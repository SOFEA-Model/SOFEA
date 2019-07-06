// Disable select warnings with MSVC and Boost.Accumulators 1.66.0
#ifdef _MSC_VER
#pragma warning(disable:4244) // Level 3, C4244: conversion from 'coordinate_type' to 'promoted_type'
#endif

#include "Analysis.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <memory>
#include <type_traits>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/rolling_count.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/rolling_sum.hpp>
#include <boost/accumulators/statistics/p_square_cumul_dist.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <fmt/format.h>
#include <netcdf.h>

extern "C" {
#include <nctime.h> // for cdRel2Iso
}

#include <QProgressDialog>
#include <QDebug>

// General netCDF error handler.
void handleError(int rc)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Analysis");
    if (rc != NC_NOERR) {
        const char* errmsg = nc_strerror(rc);
        BOOST_LOG_TRIVIAL(error) << "netCDF: " << errmsg;
        throw std::exception(errmsg);
    }
}

// Generic wrapper for netCDF type conversion.
template <int T> struct TypeMap {};
template <> struct TypeMap<NC_BYTE> {
    typedef char value_type;
    typedef int result_type;
};
template <> struct TypeMap<NC_CHAR> {
    typedef char value_type;
    typedef char result_type;
};
template <> struct TypeMap<NC_SHORT> {
    typedef short value_type;
    typedef int result_type;
};
template <> struct TypeMap<NC_INT> {
    typedef int value_type;
    typedef int result_type;
};
template <> struct TypeMap<NC_INT64> {
    typedef long long value_type;
    typedef long long result_type;
};
template <> struct TypeMap<NC_FLOAT> {
    typedef float value_type;
    typedef double result_type;
};
template <> struct TypeMap<NC_DOUBLE> {
    typedef double value_type;
    typedef double result_type;
};
template <> struct TypeMap<NC_UBYTE> {
    typedef unsigned char value_type;
    typedef unsigned char result_type;
};
template <> struct TypeMap<NC_USHORT> {
    typedef unsigned short value_type;
    typedef unsigned int result_type;
};
template <> struct TypeMap<NC_UINT> {
    typedef unsigned int value_type;
    typedef unsigned int result_type;
};
template <> struct TypeMap<NC_UINT64> {
    typedef unsigned long long value_type;
    typedef unsigned long long result_type;
};

std::string get_att_string(const int ncid, const int varid, const char *name, const int xtype, const size_t len)
{
    // Currently only support NC_CHAR attributes with length > 1.
    if (xtype != NC_CHAR && len > 1)
        return std::string();

    // Only selected types supported. Others will return empty string.
    switch (xtype)
    {
        case NC_CHAR: {
            std::string buffer;
            buffer.resize(len);
            handleError(nc_get_att_text(ncid, varid, name, &buffer[0]));
            return buffer;
        }
        case NC_SHORT: {
            TypeMap<NC_SHORT>::value_type value;
            handleError(nc_get_att_short(ncid, varid, name, &value));
            return boost::lexical_cast<std::string>(value);
        }
        case NC_INT: {
            TypeMap<NC_INT>::value_type value;
            handleError(nc_get_att_int(ncid, varid, name, &value));
            return boost::lexical_cast<std::string>(value);
        }
        case NC_FLOAT: {
            TypeMap<NC_FLOAT>::value_type value;
            handleError(nc_get_att_float(ncid, varid, name, &value));
            return boost::lexical_cast<std::string>(value);
        }
        case NC_DOUBLE: {
            TypeMap<NC_DOUBLE>::value_type value;
            handleError(nc_get_att_double(ncid, varid, name, &value));
            return boost::lexical_cast<std::string>(value);
        }
        default: {
            return std::string();
        }
    }
}

//-----------------------------------------------------------------------------
// Analysis
//-----------------------------------------------------------------------------

Analysis::Analysis(const std::string& file)
    : filename(file), ncid(0)
{
    // Open the netCDF file for reading.
    handleError(nc_open(filename.c_str(), NC_NOWRITE, &ncid));
    init();
}

Analysis::~Analysis()
{
    nc_close(ncid);
}

// EXPECTED DIMENSIONS:
// - rec, grp, ave, time, strlen
//
// EXPECTED VARIABLES:
// - double x(rec)
// - double y(rec)
// - double zelev(rec)
// - double zhill(rec)
// - double zflag(rec)
// - int rec(rec)
// - char grp(grp, strlen)
// - int ave(ave)
// - int time(time)
// - byte clmsg(time)

void Analysis::init()
{
    int ndims = 0, nvars = 0, natts = 0, unlimdimid = 0;
    handleError(nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid));

    // Get dimension information.
    for (int dimid = 0; dimid < ndims; ++dimid) {
        dim_info_t info;
        info.id = dimid;
        char name[NC_MAX_NAME];
        handleError(nc_inq_dim(ncid, dimid, name, &info.len));
        dims[std::string(name)] = info;
    }

    // Get variable information.
    for (int varid = 0; varid < nvars; ++varid) {
        var_info_t vinfo;
        vinfo.id = varid;
        int vndims = 0, vnatts = 0;
        char vname[NC_MAX_NAME];
        handleError(nc_inq_varndims(ncid, varid, &vndims));
        vinfo.dimids.resize(vndims);
        handleError(nc_inq_var(ncid, varid, vname, &vinfo.xtype, NULL,
                               &vinfo.dimids[0], &vnatts));

        // Get variable attributes.
        for (int attid = 0; attid < vnatts; ++attid) {
            att_info_t ainfo;
            ainfo.id = attid;
            char aname[NC_MAX_NAME];
            handleError(nc_inq_attname(ncid, varid, attid, aname));
            handleError(nc_inq_att(ncid, varid, aname, &ainfo.xtype, &ainfo.len));
            ainfo.value = get_att_string(ncid, varid, aname, ainfo.xtype, ainfo.len);
            vinfo.atts[std::string(aname)] = ainfo;
        }

        vars[std::string(vname)] = vinfo;
    }

    // Get global attributes.
    for (int attid = 0; attid < natts; ++attid) {
        att_info_t ainfo;
        ainfo.id = attid;
        char aname[NC_MAX_NAME];
        handleError(nc_inq_attname(ncid, NC_GLOBAL, attid, aname));
        handleError(nc_inq_att(ncid, NC_GLOBAL, aname, &ainfo.xtype, &ainfo.len));
        ainfo.value = get_att_string(ncid, NC_GLOBAL, aname, ainfo.xtype, ainfo.len);
        global_atts[std::string(aname)] = ainfo;
    }
}

std::string Analysis::getModelVersion() const
{
    if (global_atts.count("source"))
        return global_atts.at("source").value;
    else
        return std::string();
}

std::string Analysis::getModelOptions() const
{
    if (global_atts.count("options"))
        return global_atts.at("options").value;
    else
        return std::string();
}

std::string Analysis::getTitle() const
{
    if (global_atts.count("title"))
        return global_atts.at("title").value;
    else
        return std::string();
}

int Analysis::getTimeStepCount() const
{
    if (dims.count("time")) {
        const dim_info_t& dim_time = dims.at("time");
        return static_cast<int>(dim_time.len);
    }
    else {
        return 0;
    }
}

int Analysis::getReceptorCount() const
{
    if (dims.count("rec")) {
        const dim_info_t& dim_rec = dims.at("rec");
        return static_cast<int>(dim_rec.len);
    }
    else {
        return 0;
    }
}

std::vector<int> Analysis::getAveragingPeriods() const
{
    std::vector<int> periods;

    if (dims.count("ave") && vars.count("ave")) {
        const dim_info_t& dim_ave = dims.at("ave");
        const var_info_t& var_ave = vars.at("ave");
        if (var_ave.dimids.size() == 1 &&
            var_ave.dimids[0] == dim_ave.id) {
            periods.resize(dim_ave.len);
            nc_get_var_int(ncid, var_ave.id, &periods[0]);
        }
    }

    return periods;
}

std::vector<std::string> Analysis::getSourceGroups() const
{
    std::vector<std::string> names;

    if (dims.count("grp") && dims.count("strlen") && vars.count("grp")) {
        const dim_info_t& dim_grp = dims.at("grp");
        const dim_info_t& dim_strlen = dims.at("strlen");
        const var_info_t& var_grp = vars.at("grp");

        // The last dimension should be strlen.
        if (var_grp.dimids.size() == 2 &&
            var_grp.dimids[0] == dim_grp.id &&
            var_grp.dimids[1] == dim_strlen.id) {

            // Create a character buffer.
            std::string buffer;
            size_t slen = dim_strlen.len;
            size_t ngrp = dim_grp.len;
            buffer.resize(ngrp * slen);

            // Read the entire array.
            nc_get_var_text(ncid, var_grp.id, &buffer[0]);

            // Iterate over the buffer and extract fixed-width strings.
            names.reserve(ngrp);
            for (int i = 0; i < ngrp; ++i) {
                std::string name = buffer.substr(i * slen, slen);
                names.push_back(name);
            }
        }
    }

    return names;
}

std::vector<std::pair<std::string, std::string>> Analysis::getTypes() const
{
    std::vector<std::pair<std::string, std::string>> res;
    std::vector<std::string> types = {"conc", "depos", "ddep", "wdep"};

    for (const auto& type : types) {
        if (vars.count(type)) {
            std::string units = getUnits(type);
            boost::algorithm::to_lower(units);
            res.push_back(std::make_pair(type, units));
        }
    }

    return res;
}

std::string Analysis::getUnits(const std::string& varname) const
{
    if (vars.count(varname)) {
        const var_info_t& var_type = vars.at(varname);
        if (var_type.atts.count("units")) {
            std::string res = var_type.atts.at("units").value;
            return res;
        }
    }

    return std::string();
}

std::string Analysis::getTimeString(const double reltime) const
{
    // Based on nctime_val_tostring in netCDF dumplib.c

    const int PRIM_LEN = 100;
    std::string relunits = getUnits("time");
    int separator = 'T';
    std::string buffer;
    buffer.resize(PRIM_LEN);

    // Do the conversion and remove extra null terminators.
    cdRel2Iso(cdStandard, &relunits[0], separator, reltime, &buffer[0]);
    buffer.resize(strlen(buffer.c_str()));

    // If separator is not found (first hour), add "T00"
    if (buffer.find('T') == std::string::npos)
        buffer.append("T00");

    return buffer;
}

std::string Analysis::libraryVersion()
{
    std::string ver = nc_inq_libvers();
    return ver;
}

//-----------------------------------------------------------------------------
// Calculations
//-----------------------------------------------------------------------------

void Analysis::exportTimeSeries(GeneralAnalysisOpts genOpts, const std::string& filename)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Analysis");

    auto vave = getAveragingPeriods();
    auto vgrp = getSourceGroups();
    size_t ntime = getTimeStepCount();
    size_t nrecs = getReceptorCount();

    // Find the averaging period and source group indexes.
    size_t iave = std::distance(vave.begin(),
        std::find(vave.begin(), vave.end(), genOpts.avePeriod));
    size_t igrp = std::distance(vgrp.begin(),
        std::find(vgrp.begin(), vgrp.end(), genOpts.sourceGroup));

    int aveper = genOpts.avePeriod;
    int min_aveper = *std::min_element(vave.begin(), vave.end());

    QProgressDialog progress("Exporting Time Series...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Read the coordinate arrays.
    std::vector<int> recs(nrecs), time(ntime);
    std::vector<unsigned char> clmsg(ntime);
    std::vector<double> xc(nrecs), yc(nrecs), zelev(nrecs), zhill(nrecs), zflag(nrecs);
    handleError(nc_get_var_int(ncid, vars.at("rec").id, &recs[0]));
    handleError(nc_get_var_int(ncid, vars.at("time").id, &time[0]));
    handleError(nc_get_var_ubyte(ncid, vars.at("clmsg").id, &clmsg[0]));
    handleError(nc_get_var_double(ncid, vars.at("x").id, &xc[0]));
    handleError(nc_get_var_double(ncid, vars.at("y").id, &yc[0]));
    handleError(nc_get_var_double(ncid, vars.at("zelev").id, &zelev[0]));
    handleError(nc_get_var_double(ncid, vars.at("zhill").id, &zhill[0]));
    handleError(nc_get_var_double(ncid, vars.at("zflag").id, &zflag[0]));

    // Read the data array. The last dimension varies fastest.
    // (ave, grp, rec, time)
    size_t start[4] = {iave, igrp, 0, 0};
    size_t count[4] = {1, 1, nrecs, ntime};

    auto values = std::vector<double>(ntime * nrecs, 0);
    handleError(nc_get_vara_double(ncid, vars.at(genOpts.type).id,
                                   start, count, &values[0]));

    // Remove fill values. The resulting size should be:
    // floor(ntime / (aveper / min_aveper)) * nrecs
    auto remove = std::remove(values.begin(), values.end(), NC_FILL_DOUBLE);
    values.erase(remove, values.end());
    if (values.size() != (ntime / (aveper / min_aveper)) * nrecs) {
        throw std::exception("Data array has unexpected missing values.");
    }
    ntime = ntime / (aveper / min_aveper);

    // Rescale using user-defined scale factor.
    for (double &value : values)
        value *= genOpts.scaleFactor;

    // Write csv file.
    std::ofstream ofs(filename);
    if (aveper == 1) {
        ofs << "receptor,time,calm_missing,x,y,zelev,zhill,zflag," << genOpts.type << "\n";
    }
    else {
        ofs << "receptor,time,x,y,zelev,zhill,zflag," << genOpts.type << "\n";
    }
    for (int i = 0; i < nrecs; ++i) {
        progress.setValue(i);
        if (progress.wasCanceled()) {
            throw std::exception("Export cancelled.");
        }

        fmt::MemoryWriter w;
        for (int t = 0; t < ntime; ++t) {
            double timeval = static_cast<double>(time.at(t * aveper / min_aveper));
            double val = values.at(i * ntime + t);
            std::string timestr = getTimeString(timeval);
            if (aveper == 1) {
                w.write("{},{},{},{},{},{},{},{},{}\n",
                        recs.at(i), timestr, clmsg.at(t), xc.at(i), yc.at(i),
                        zelev.at(i), zhill.at(i), zflag.at(i), val);
            }
            else {
                w.write("{},{},{},{},{},{},{},{}\n",
                        recs.at(i), timestr, xc.at(i), yc.at(i),
                        zelev.at(i), zhill.at(i), zflag.at(i), val);
            }
        }
        ofs << w.str();
    }
    ofs.close();
}

void Analysis::calcReceptorStats(GeneralAnalysisOpts genOpts, ReceptorAnalysisOpts opts, ReceptorStats& out) const
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Analysis");

    using namespace boost::accumulators;

    auto vave = getAveragingPeriods();
    auto vgrp = getSourceGroups();
    size_t ntime = getTimeStepCount();
    size_t nrecs = getReceptorCount();

    // Find the averaging period and source group indexes.
    size_t iave = std::distance(vave.begin(),
        std::find(vave.begin(), vave.end(), genOpts.avePeriod));
    size_t igrp = std::distance(vgrp.begin(),
        std::find(vgrp.begin(), vgrp.end(), genOpts.sourceGroup));

    int aveper = genOpts.avePeriod;
    int min_aveper = *std::min_element(vave.begin(), vave.end());

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Read the coordinate arrays.
    std::vector<int> id(nrecs);
    std::vector<double> xc(nrecs), yc(nrecs), zc(nrecs);
    handleError(nc_get_var_int(ncid, vars.at("rec").id, &id[0]));
    handleError(nc_get_var_double(ncid, vars.at("x").id, &xc[0]));
    handleError(nc_get_var_double(ncid, vars.at("y").id, &yc[0]));
    handleError(nc_get_var_double(ncid, vars.at("zelev").id, &zc[0]));

    // Read the calm/missing flag array.
    auto clmsg = std::vector<unsigned char>(ntime);
    if (opts.calcRecMaxRM && opts.recWindowSizes.size() > 0 && aveper == 1) {
        handleError(nc_get_var_ubyte(ncid, vars.at("clmsg").id, &clmsg[0]));
    }

    // Read the data array. The last dimension varies fastest.
    // (ave, grp, rec, time)
    size_t start[4] = {iave, igrp, 0, 0};
    size_t count[4] = {1, 1, nrecs, ntime};
    auto values = std::vector<double>(ntime * nrecs, 0);
    handleError(nc_get_vara_double(ncid, vars.at(genOpts.type).id,
                                   start, count, &values[0]));

    // Remove fill values. The resulting size should be:
    // floor(ntime / (aveper / min_aveper)) * nrecs
    values.erase(std::remove(values.begin(), values.end(), NC_FILL_DOUBLE), values.end());
    if (values.size() != (ntime / (aveper / min_aveper)) * nrecs) {
        throw std::exception("Data array has unexpected missing values.");
    }

    ntime = ntime / (aveper / min_aveper);

    // Rescale using user-defined scale factor.
    for (double &value : values)
        value *= genOpts.scaleFactor;

    // Define the statistical accumulator types.
    using avgacc_t = accumulator_set<double, features<tag::mean>>;
    using maxacc_t = accumulator_set<double, features<tag::max>>;
    using varacc_t = accumulator_set<double, features<tag::variance>>;
    using p2acc_t = accumulator_set<double, stats<tag::p_square_quantile>>;
    using rmacc_t = accumulator_set<double, stats<tag::rolling_mean>>;
    using rsacc_t = accumulator_set<double, stats<tag::rolling_sum, tag::rolling_count>>;

    // Initialize the output coordinate vectors.
    out.id = id;
    out.x = xc;
    out.y = yc;
    out.z = zc;

    // Initialize the output vectors for basic statistics.
    if (opts.calcRecMean)
        out.avg.resize(nrecs);
    if (opts.calcRecMax)
        out.max.resize(nrecs);
    if (opts.calcRecStdDev)
        out.std.resize(nrecs);

    // Initialize the output vectors for P2 calculations.
    if (opts.calcRecP2 && opts.recPercentiles.size() > 0) {
        for (double p : opts.recPercentiles) {
            std::vector<double> v(nrecs, 0);
            out.p2.push_back(v);
        }
    }

    // Initialize the output vectors for max RM calculations.
    if (opts.calcRecMaxRM && opts.recWindowSizes.size() > 0) {
        for (double w : opts.recWindowSizes) {
            std::vector<double> v(nrecs, 0);
            out.rm.push_back(v);
        }
    }

    // Analyze the data array for each receptor.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled()) {
            throw std::exception("Analysis cancelled.");
        }

        // Create the accumulators.
        avgacc_t avgacc;
        maxacc_t maxacc;
        varacc_t varacc;
        std::vector<p2acc_t> p2accs; 
        std::vector<rmacc_t> rmaccs;
        std::vector<std::pair<rsacc_t, rsacc_t>> rsaccs;
        std::vector<maxacc_t> maxrmaccs;

        if (opts.calcRecP2 && opts.recPercentiles.size() > 0) {
            for (double p : opts.recPercentiles) {
                p2acc_t acc(quantile_probability = p);
                p2accs.push_back(acc);
            }
        }

        if (opts.calcRecMaxRM && opts.recWindowSizes.size() > 0) {
            for (double w : opts.recWindowSizes) {
                // Determine the window size. This depends on the averaging period.
                int aveper_hours = aveper;
                int window_hours = static_cast<int>(w) * 24;
                // This should evaluate true for any short-term averaging periods:
                // (1, 2, 3, 4, 6, 8, 12, 24)
                if (window_hours % aveper_hours == 0) {
                    int wsize = window_hours / aveper_hours;
                    rsacc_t rsacc1(tag::rolling_window::window_size = wsize); // concentration
                    rsacc_t rsacc2(tag::rolling_window::window_size = wsize); // valid hours
                    rsaccs.push_back(std::make_pair(rsacc1, rsacc2));
                    rmacc_t rmacc(tag::rolling_window::window_size = wsize);
                    rmaccs.push_back(rmacc);
                    maxacc_t maxrmacc;
                    maxrmaccs.push_back(maxrmacc);
                }
                else {
                    throw std::exception("Invalid averaging period.");
                }
            }
        }

        // Main update loop.
        for (int t = 0; t < ntime; ++t)
        {
            double val = values.at(i * ntime + t);

            if (opts.calcRecMean) {
                avgacc(val);
            }
            if (opts.calcRecMax) {
                maxacc(val);
            }
            if (opts.calcRecStdDev) {
                varacc(val);
            }
            if (opts.calcRecP2) {
                for (p2acc_t &acc : p2accs)
                    acc(val);
            }
            if (opts.calcRecMaxRM) {
                if (aveper == 1) {
                    // Apply EPA averaging policy for 1 hour averaging period.
                    int cmflag = clmsg.at(t * (aveper / min_aveper));
                    double cm = (cmflag == 0) ? 0 : 1;

                    for (int j = 0; j < rsaccs.size(); ++j) {
                        rsaccs[j].first(val);
                        rsaccs[j].second(cm);

                        // Current number of elements in the rolling window.
                        double n = rolling_count(rsaccs[j].first);

                        double numerator = rolling_sum(rsaccs[j].first);

                        if (n <= 24) {
                            // Short-term average. See SUBROUTINE AVER in AERMOD calc2.f
                            double denominator = std::max(n - rolling_sum(rsaccs[j].second), std::round(n * 0.75 + 0.4));
                            maxrmaccs[j](numerator / denominator);
                        }
                        else {
                            // Long-term average. See SUBROUTINE PERAVE in AERMOD output.f
                            double denominator = n - rolling_sum(rsaccs[j].second);
                            maxrmaccs[j](numerator / denominator);
                        }
                    }
                }
                else {
                    // Calculate standard rolling mean. Averaging already applied.
                    for (int j = 0; j < rmaccs.size(); ++j) {
                        rmaccs[j](val);
                        maxrmaccs[j](rolling_mean(rmaccs[j]));
                    }
                }
            }
        }

        // Extract results.
        if (opts.calcRecMean)
            out.avg[i] = mean(avgacc);
        if (opts.calcRecMax)
            out.max[i] = max(maxacc);
        if (opts.calcRecStdDev)
            out.std[i] = sqrt(variance(varacc));
        if (opts.calcRecP2) {
            for (int j = 0; j < p2accs.size(); ++j)
                out.p2.at(j).at(i) = p_square_quantile(p2accs[j]);
        }
        if (opts.calcRecMaxRM) {
            for (int j = 0; j < maxrmaccs.size(); ++j)
                out.rm.at(j).at(i) = max(maxrmaccs[j]);
        }
    }

    return;
}

void Analysis::calcHistogram(GeneralAnalysisOpts genOpts, HistogramAnalysisOpts opts, Histogram& out) const
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Analysis");

    using namespace boost::accumulators;

    auto vave = getAveragingPeriods();
    auto vgrp = getSourceGroups();
    size_t ntime = getTimeStepCount();
    size_t nrecs = getReceptorCount();

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Find the averaging period and source group indexes.
    size_t iave = std::distance(vave.begin(),
        std::find(vave.begin(), vave.end(), genOpts.avePeriod));
    size_t igrp = std::distance(vgrp.begin(),
        std::find(vgrp.begin(), vgrp.end(), genOpts.sourceGroup));

    int aveper = genOpts.avePeriod;
    int min_aveper = *std::min_element(vave.begin(), vave.end());

    // Read the data array. The last dimension varies fastest.
    // (ave, grp, rec, time)
    size_t start[4] = {iave, igrp, 0, 0};
    size_t count[4] = {1, 1, nrecs, ntime};

    auto values = std::vector<double>(ntime * nrecs, 0);
    handleError(nc_get_vara_double(ncid, vars.at(genOpts.type).id,
                                   start, count, &values[0]));

    // Remove fill values. The resulting size should be:
    // floor(ntime / (aveper / min_aveper)) * nrecs
    auto remove = std::remove(values.begin(), values.end(), NC_FILL_DOUBLE);
    values.erase(remove, values.end());
    if (values.size() != (ntime / (aveper / min_aveper)) * nrecs) {
        throw std::exception("Data array has unexpected missing values.");
    }
    ntime = ntime / (aveper / min_aveper);

    // Rescale using user-defined scale factor.
    for (double &value : values)
        value *= genOpts.scaleFactor;

    // Create the accumulators.
    typedef accumulator_set<double, stats<tag::p_square_cumulative_distribution>> cdfacc_t;
    cdfacc_t cdfacc(tag::p_square_cumulative_distribution::num_cells = opts.cdfBins);

    typedef accumulator_set<double, stats<tag::density>> pdfacc_t;
    pdfacc_t pdfacc(tag::density::num_bins = opts.pdfBins,
                    tag::density::cache_size = opts.pdfCacheSize);

    // Analyze the data array.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled()) {
            throw std::exception("Analysis cancelled.");
        }

        for (int t = 0; t < ntime; ++t) {
            double val = values.at(i * ntime + t);
            if (opts.calcCDF)
                cdfacc(val);
            if (opts.calcPDF)
                pdfacc(val);
        }
    }

    // Extract results.
    typedef boost::iterator_range<std::vector<std::pair<double, double>>::iterator> histogram_t;
    if (opts.calcCDF) {
        histogram_t cdf = p_square_cumulative_distribution(cdfacc);
        out.cdf.insert(out.cdf.end(), make_move_iterator(cdf.begin()), make_move_iterator(cdf.end()));
    }
    if (opts.calcPDF) {
        histogram_t pdf = density(pdfacc);
        out.pdf.insert(out.pdf.end(), make_move_iterator(pdf.begin()), make_move_iterator(pdf.end()));
    }

    return;
}

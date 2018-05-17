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
#include <string>
#include <vector>

#include <netcdf.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include <boost/accumulators/statistics/p_square_cumul_dist.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/log/trivial.hpp>

#include <QProgressDialog>

// netCDF error handling macro
#define CHECK(x) do {              \
  int retval = (x);                \
  if (retval != NC_NOERR) {        \
    errmsg = nc_strerror(retval);  \
    return false;                  \
  }                                \
} while (0)


Analysis::Analysis(const std::string& file) : filename(file)
{
}

bool Analysis::openFile(int& ncid)
{
    // Open the netCDF file for reading.
    CHECK(nc_open(filename.c_str(), NC_NOWRITE, &ncid));

    // Get the variable IDs.
    CHECK(nc_inq_varid(ncid, "x", &var_x));
    CHECK(nc_inq_varid(ncid, "y", &var_y));
    CHECK(nc_inq_varid(ncid, "z", &var_z));
    CHECK(nc_inq_varid(ncid, "time", &var_time));
    CHECK(nc_inq_varid(ncid, "type", &var_type));
    CHECK(nc_inq_varid(ncid, "receptor", &var_recs));
    CHECK(nc_inq_varid(ncid, "data", &var_conc));

    // Get the dimension IDs.
    CHECK(nc_inq_dimid(ncid, "time", &dim_time));
    CHECK(nc_inq_dimid(ncid, "type", &dim_type));
    CHECK(nc_inq_dimid(ncid, "receptor", &dim_recs));

    // Get the record counts.
    CHECK(nc_inq_dimlen(ncid, dim_time, &ntime));
    CHECK(nc_inq_dimlen(ncid, dim_type, &ntype));
    CHECK(nc_inq_dimlen(ncid, dim_recs, &nrecs));

    // Ensure the file contains only one output type.
    if (ntype != 1) {
        errmsg = nc_strerror(NC_EDIMSIZE);
        return false;
    }

    // Check for the expected dimension count.
    int conc_ndims;
    int conc_dimid[3];
    CHECK(nc_inq_varndims(ncid, var_conc, &conc_ndims));
    if (conc_ndims != 3) {
        errmsg = nc_strerror(NC_EDIMSIZE);
        return false;
    }

    // Check for the expected dimensions.
    CHECK(nc_inq_vardimid(ncid, var_conc, conc_dimid));
    if ((conc_dimid[0] != dim_type) ||
        (conc_dimid[1] != dim_recs) ||
        (conc_dimid[2] != dim_time)) {
        errmsg = nc_strerror(NC_EBADDIM);
        return false;
    }

    return true;
}

bool Analysis::calcReceptorStats(int& ncid, AnalysisOptions opts, ReceptorStats& out)
{
    using namespace boost::accumulators;

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Read the coordinate arrays.
    std::vector<int> id(nrecs);
    std::vector<double> xc(nrecs), yc(nrecs), zc(nrecs);
    CHECK(nc_get_var_int(ncid, var_recs, &id[0]));
    CHECK(nc_get_var_double(ncid, var_x, &xc[0]));
    CHECK(nc_get_var_double(ncid, var_y, &yc[0]));
    CHECK(nc_get_var_double(ncid, var_z, &zc[0]));

    // Read the concentration array. The last dimension varies fastest.
    auto conc = std::make_unique<std::vector<double>>(ntime * nrecs, 0);
    CHECK(nc_get_var_double(ncid, var_conc, &conc->data()[0]));

    // Close the file.
    CHECK(nc_close(ncid));

    // Define the statistical accumulator types.
    typedef accumulator_set<double, features<tag::mean>> avgacc_t;
    typedef accumulator_set<double, features<tag::max>> maxacc_t;
    typedef accumulator_set<double, features<tag::variance>> varacc_t;
    typedef accumulator_set<double, stats<tag::p_square_quantile>> p2acc_t;
    typedef accumulator_set<double, stats<tag::rolling_mean>> rmacc_t;

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

    // Analyze the concentration array for each receptor.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled())
            return false;

        // Create the accumulators.
        avgacc_t avgacc;
        maxacc_t maxacc;
        varacc_t varacc;
        std::vector<p2acc_t> p2accs;
        std::vector<rmacc_t> rmaccs;
        std::vector<maxacc_t> maxrmaccs;

        if (opts.calcRecP2 && opts.recPercentiles.size() > 0) {
            for (double p : opts.recPercentiles) {
                p2acc_t acc(quantile_probability = p);
                p2accs.push_back(acc);
            }
        }
        if (opts.calcRecMaxRM && opts.recWindowSizes.size() > 0) {
            for (double w : opts.recWindowSizes) {
                int hours = static_cast<int>(w) * 24;
                rmacc_t acc1(tag::rolling_window::window_size = hours);
                rmaccs.push_back(acc1);
                maxacc_t acc2;
                maxrmaccs.push_back(acc2);
            }
        }

        // Main update loop.
        for (int t = 0; t < ntime; ++t) {
            double val = conc->at(i * ntime + t);

            if (opts.calcRecMean)
                avgacc(val);
            if (opts.calcRecMax)
                maxacc(val);
            if (opts.calcRecStdDev)
                varacc(val);
            if (opts.calcRecP2) {
                for (p2acc_t &acc : p2accs)
                    acc(val);
            }
            if (opts.calcRecMaxRM) {
                for (int j = 0; j < rmaccs.size(); ++j) {
                    rmaccs[j](val);
                    maxrmaccs[j](rolling_mean(rmaccs[j]));
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

    return true;
}

bool Analysis::calcHistogram(int& ncid, AnalysisOptions opts, Histogram& out)
{
    using namespace boost::accumulators;

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Read the concentration array.
    auto conc = std::make_unique<std::vector<double>>(ntime * nrecs, 0);
    CHECK(nc_get_var_double(ncid, var_conc, &conc->data()[0]));

    // Close the file.
    CHECK(nc_close(ncid));

    // Create the accumulators.
    typedef accumulator_set<double, stats<tag::p_square_cumulative_distribution>> cdfacc_t;
    cdfacc_t cdfacc(tag::p_square_cumulative_distribution::num_cells = opts.cdfBins);

    typedef accumulator_set<double, stats<tag::density>> pdfacc_t;
    pdfacc_t pdfacc(tag::density::num_bins = opts.pdfBins,
                    tag::density::cache_size = opts.pdfCacheSize);

    // Analyze the concentration array.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled())
            return false;

        for (int t = 0; t < ntime; ++t) {
            double val = conc->at(i * ntime + t);
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

    return true;
}

bool Analysis::closeFile(int& ncid)
{
    CHECK(nc_close(ncid));
    return true;
}

long long Analysis::getTimeStepCount() const
{
    return static_cast<long long>(ntime);
}

long long Analysis::getReceptorCount() const
{
    return static_cast<long long>(nrecs);
}

std::string Analysis::getLastError() const
{
    return errmsg;
}

std::string Analysis::libraryVersion() const
{
    std::string ver = nc_inq_libvers();
    return ver;

}

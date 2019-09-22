#include "Analysis.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <filesystem>
#include <fstream>
#include <memory>
#include <type_traits>
#include <string>
#include <utility>
#include <vector>

// FIXME: use std::function callback
#include <QProgressDialog>

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
//#include <boost/log/trivial.hpp>
//#include <boost/log/attributes/scoped_attribute.hpp>

#include <fmt/format.h>

namespace ncpost {

std::string library_version()
{
    return ncpp::library_version();
}

// Variadic template to check if all vectors have equal length.
template <typename... Args>
bool equal_length(const std::vector<Args>&... args) {
    if constexpr (sizeof...(Args) == 0) {
        return true;
    } else {
        return [](const auto& v0, const auto&... rest) {
            return ((v0.size() == rest.size()) && ...);
        }(args...);
    }
}

// EXPECTED DIMENSIONS:
//   rec, arc, net, grp, ave, time, strlen
//
// EXPECTED VARIABLES:
//   double x(rec)
//   double y(rec)
//   double zelev(rec)
//   double zhill(rec)
//   double zflag(rec)
//   int rec(rec)
//   int arc(rec)
//   char arcid(arc, strlen)
//   int net(rec)
//   char netid(net, strlen)
//   char grp(grp, strlen)
//   int ave(ave)
//   int time(time)
//   byte clmsg(time)

analysis::analysis(const std::string& filepath)
    : file_(filepath, ncpp::file::read), ds_(file_), filepath_(filepath)
{
    // Global attributes
    if (ds_.atts.contains("source"))
        ds_.atts["source"].read(metadata_.model_version);
    if (ds_.atts.contains("options"))
        ds_.atts["options"].read(metadata_.model_options);
    if (ds_.atts.contains("title"))
        ds_.atts["title"].read(metadata_.title);

    // Output types
    std::array<std::string, 4> types{"conc", "depos", "ddep", "wdep"};
    for (const auto& type : types) {
        if (ds_.vars.contains(type)) {
            metadata_type::output_type v;
            v.name = type;
            ds_.vars[type].atts["units"].read(v.units);
            boost::algorithm::to_lower(v.units);
            metadata_.output_types.push_back(v);
        }
    }

    // Averaging periods
    metadata_.averaging_periods = ds_.vars["ave"].values<int>();

    // Source groups
    metadata_.source_groups = ds_.vars["grp"].values<std::string>();

    // Receptor coordinates
    const auto recid = ds_.vars["rec"].values<int>();
    const auto x = ds_.vars["x"].values<double>();
    const auto y = ds_.vars["y"].values<double>();
    const auto zelev = ds_.vars["zelev"].values<double>();
    const auto zhill = ds_.vars["zhill"].values<double>();
    const auto zflag = ds_.vars["zflag"].values<double>();

    if (!equal_length(recid, x, y, zelev, zhill, zflag))
        throw std::out_of_range("Receptor coordinates are not equal length.");

    metadata_.receptors.reserve(recid.size());
    for (int i = 0; i < recid.size(); ++i) {
        metadata_.receptors.emplace_back(
            metadata_type::receptor{recid[i], x[i], y[i], zelev[i], zhill[i], zflag[i]}
        );
    }

    // Receptor network and arc information
    if (ds_.vars.contains("net") && ds_.vars.contains("netid")) {
        const auto net = ds_.vars["net"].values<int>();
        const auto netid = ds_.vars["netid"].values<std::string>();
        if (net.size() == recid.size()) {
            for (int i = 0; i < net.size(); ++i) {
                if (net[i] > 0)
                    metadata_.receptor_netid[recid[i]] = netid.at(net[i] - 1);
            }
        }
    }

    if (ds_.vars.contains("arc") && ds_.vars.contains("arcid") ) {
        const auto arc = ds_.vars["arc"].values<int>();
        const auto arcid = ds_.vars["arcid"].values<std::string>();
        if (arc.size() == recid.size()) {
            for (int i = 0; i < arc.size(); ++i) {
                if (arc[i] > 0)
                    metadata_.receptor_arcid[recid[i]] = arcid.at(arc[i] - 1);
            }
        }
    }

    // Time steps
    if (ds_.vars.contains("time"))
        metadata_.time_steps = ds_.vars["time"].values<date::sys_seconds>();
}

metadata_type analysis::metadata() const
{
    return metadata_;
}

void analysis::export_time_series(const options::general& opts, const std::string& filepath) const
{
    // Open the output stream.
    std::ofstream ofs(filepath);

    // Read the data array.
    std::vector<double> values;
    std::size_t ntime = 0;
    std::size_t nrecs = 0;
    int minave = 0;
    read_values(opts, values, ntime, nrecs, minave);

    // Read the calm/missing flag array.
    std::vector<unsigned char> clmsg;
    if (opts.averaging_period == 1) {
        clmsg = ds_.vars["clmsg"].values<unsigned char>();
    }

    QProgressDialog progress("Exporting Time Series...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Write CSV file header.
    if (opts.averaging_period == 1)
        ofs << "receptor_group,receptor,time,calm_missing,x,y,zelev,zhill,zflag," << opts.output_type << "\n";
    else
        ofs << "receptor_group,receptor,time,x,y,zelev,zhill,zflag," <<  opts.output_type << "\n";

    // Write CSV records.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled())
            throw std::exception("Export cancelled.");

        fmt::memory_buffer w;
        for (int t = 0; t < ntime; ++t)
        {
            const auto timepoint = metadata_.time_steps.at(t * opts.averaging_period / minave);
            const std::string timestr = date::format("%F %R", timepoint);

            const auto& rec = metadata_.receptors.at(i);
            std::string recgrp;
            if (metadata_.receptor_netid.count(rec.id))
                recgrp = metadata_.receptor_netid.at(rec.id);
            else if (metadata_.receptor_arcid.count(rec.id))
                recgrp = metadata_.receptor_arcid.at(rec.id);

            const double val = values.at(i * ntime + t);

            if (opts.averaging_period == 1)
                fmt::format_to(w, "{},{},{},{},{},{},{},{},{},{}\n",
                    recgrp, rec.id, timestr, clmsg.at(t), rec.x, rec.y,
                    rec.zelev, rec.zhill, rec.zflag, val);
            else
                fmt::format_to(w, "{},{},{},{},{},{},{},{},{}\n",
                   recgrp, rec.id, timestr, rec.x, rec.y,
                   rec.zelev, rec.zhill, rec.zflag, val);
        }

        ofs << fmt::to_string(w);
    }

    ofs.close();
}

void analysis::calc_receptor_stats(const options::general& opts, const options::statistics& statopts, statistics_type& out) const
{
    using namespace boost::accumulators;

    if (!statopts.calc_avg && !statopts.calc_max && !statopts.calc_std &&
        !statopts.percentiles.size() && !statopts.maxrm_windows.size())
        return;

    // Define the statistical accumulator types.
    using avgacc_t = accumulator_set<double, features<tag::mean>>;
    using maxacc_t = accumulator_set<double, features<tag::max>>;
    using varacc_t = accumulator_set<double, features<tag::variance>>;
    using p2acc_t = accumulator_set<double, stats<tag::p_square_quantile>>;
    using rmacc_t = accumulator_set<double, stats<tag::rolling_mean>>;
    using rsacc_t = accumulator_set<double, stats<tag::rolling_sum, tag::rolling_count>>;

    // Read the data array.
    std::vector<double> values;
    std::size_t ntime = 0;
    std::size_t nrecs = 0;
    int minave = 0;
    read_values(opts, values, ntime, nrecs, minave);

    // Read the calm/missing flag array.
    std::vector<unsigned char> clmsg;
    if (opts.averaging_period == 1) {
        clmsg = ds_.vars["clmsg"].values<unsigned char>();
    }

    // Initialize the output vectors for basic statistics.
    if (statopts.calc_avg)
        out.avg.resize(nrecs);
    if (statopts.calc_max)
        out.max.resize(nrecs);
    if (statopts.calc_std)
        out.std.resize(nrecs);

    // Initialize the output vectors for P2 calculations.
    for (double p : statopts.percentiles) {
        std::vector<double> v(nrecs, 0);
        out.p2.push_back(v);
    }

    // Initialize the output vectors for max RM calculations.
    for (double w : statopts.maxrm_windows) {
        std::vector<double> v(nrecs, 0);
        out.rm.push_back(v);
    }

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

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

        for (double p : statopts.percentiles) {
            p2acc_t acc(quantile_probability = p);
            p2accs.push_back(acc);
        }

        for (double w : statopts.maxrm_windows) {
            // Determine the window size. This depends on the averaging period.
            int aveper_hours = opts.averaging_period;
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
                throw std::runtime_error("Invalid averaging period.");
            }
        }

        // Main update loop.
        for (std::size_t t = 0; t < ntime; ++t)
        {
            double val = values.at(i * ntime + t);

            if (statopts.calc_avg)
                avgacc(val);
            if (statopts.calc_max)
                maxacc(val);
            if (statopts.calc_std)
                varacc(val);
            if (statopts.percentiles.size() > 0) {
                for (p2acc_t &acc : p2accs)
                    acc(val);
            }
            if (statopts.maxrm_windows.size() > 0) {
                if (opts.averaging_period == 1) {
                    // Apply EPA averaging policy for 1 hour averaging period.
                    int cmflag = clmsg.at(t * (opts.averaging_period / minave));
                    double cm = (cmflag == 0) ? 0 : 1;

                    for (int j = 0; j < rsaccs.size(); ++j) {
                        rsaccs[j].first(val);
                        rsaccs[j].second(cm);

                        // Current number of elements in the rolling window.
                        double n = static_cast<double>(rolling_count(rsaccs[j].first));

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
        if (statopts.calc_avg)
            out.avg[i] = mean(avgacc);
        if (statopts.calc_max)
            out.max[i] = max(maxacc);
        if (statopts.calc_std)
            out.std[i] = sqrt(variance(varacc));
        if (statopts.percentiles.size() > 0) {
            for (int j = 0; j < p2accs.size(); ++j)
                out.p2.at(j).at(i) = p_square_quantile(p2accs[j]);
        }
        if (statopts.maxrm_windows.size() > 0) {
            for (int j = 0; j < maxrmaccs.size(); ++j)
                out.rm.at(j).at(i) = max(maxrmaccs[j]);
        }
    }
}

void analysis::calc_histogram(const options::general& opts, const options::histogram& histopts, histogram_type& out) const
{
    using namespace boost::accumulators;

    if (!histopts.calc_cdf && !histopts.calc_pdf)
        return;

    // Create the accumulators.
    using cdfacc_t = accumulator_set<double, stats<tag::p_square_cumulative_distribution>>;
    cdfacc_t cdfacc(tag::p_square_cumulative_distribution::num_cells = static_cast<std::size_t>(histopts.cdf_bins));

    using pdfacc_t = accumulator_set<double, stats<tag::density>>;
    pdfacc_t pdfacc(tag::density::num_bins = static_cast<std::size_t>(histopts.pdf_bins),
                    tag::density::cache_size = static_cast<std::size_t>(histopts.pdf_cache_size));

    // Read the data array.
    std::vector<double> values;
    std::size_t ntime = 0;
    std::size_t nrecs = 0;
    int minave = 0;
    read_values(opts, values, ntime, nrecs, minave);

    QProgressDialog progress("Processing...", "Abort", 0, static_cast<int>(nrecs));
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(1000);

    // Analyze the data array.
    for (int i = 0; i < nrecs; ++i)
    {
        progress.setValue(i);
        if (progress.wasCanceled()) {
            throw std::exception("Analysis cancelled.");
        }

        for (int t = 0; t < ntime; ++t) {
            double val = values.at(i * ntime + t);
            if (histopts.calc_cdf)
                cdfacc(val);
            if (histopts.calc_pdf)
                pdfacc(val);
        }
    }

    // Extract results.
    typedef boost::iterator_range<std::vector<std::pair<double, double>>::iterator> histogram_t;
    if (histopts.calc_cdf) {
        histogram_t cdf = p_square_cumulative_distribution(cdfacc);
        out.cdf.insert(out.cdf.end(), boost::make_move_iterator(cdf.begin()), boost::make_move_iterator(cdf.end()));
    }
    if (histopts.calc_cdf) {
        histogram_t pdf = density(pdfacc);
        out.pdf.insert(out.pdf.end(), boost::make_move_iterator(pdf.begin()), boost::make_move_iterator(pdf.end()));
    }
}

void analysis::read_values(const options::general& opts, std::vector<double>& values,
                           std::size_t& ntime, std::size_t& nrecs, int& minave) const
{
    // Select analysis subset.
    auto slice = ds_.vars[opts.output_type]
        .select<int>({
            {"ave", opts.averaging_period, opts.averaging_period}
        })
        .select<std::string>({
            {"grp", opts.source_group, opts.source_group}
        });

    // Get array sizes.
    ntime = ds_.vars["time"].size();
    nrecs = ds_.vars["rec"].size();
    auto ave = ds_.vars["ave"].values<int>();
    minave = *std::min_element(ave.begin(), ave.end());

    // Read the data array.
    values = slice.values<double>();

    // Remove fill values. The resulting size should be:
    // floor(ntime / (ave / minave)) * nrecs
    values.erase(std::remove(values.begin(), values.end(), NC_FILL_DOUBLE), values.end());
    if (values.size() != (ntime / (opts.averaging_period / minave)) * nrecs) {
        throw std::runtime_error("Data array has unexpected missing values.");
    }
    ntime = ntime / (opts.averaging_period / minave);

    // Rescale using user-defined scale factor.
    for (double &value : values)
        value *= opts.scale_factor;

}

} // namespace ncpost

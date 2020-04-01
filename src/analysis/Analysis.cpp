// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
#include <QDebug>

// TODO: replace Boost.Accumulators with MKL
// https://software.intel.com/en-us/mkl-ssnotes-computing-quantiles-for-streaming-data

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

#include <fmt/format.h>

namespace ncpost {

// Utility function to determine if all vectors have equal length.
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
    : file_(filepath, ncpp::file::read), ds_(file_)
{
    static progress_function_type fn = [](std::size_t){};
    progressfn_ = fn;
}

std::string analysis::library_version()
{
    return ncpp::library_version();
}

std::string analysis::model_version() const
{
    std::string source_string;
    if (ds_.atts.contains("source"))
        ds_.atts["source"].read(source_string);
    return source_string;
}

std::string analysis::model_options() const
{
    std::string options_string;
    if (ds_.atts.contains("options"))
        ds_.atts["options"].read(options_string);
    return options_string;
}

std::string analysis::title() const
{
    std::string title_string;
    if (ds_.atts.contains("title"))
        ds_.atts["title"].read(title_string);
    return title_string;
}

std::vector<output_type_t> analysis::output_types() const
{
    std::vector<output_type_t> result;
    std::array<std::string, 4> vars{"conc", "depos", "ddep", "wdep"};
    for (const auto& var : vars) {
        if (ds_.vars.contains(var)) {
            output_type_t item;
            item.name = var;
            ds_.vars[var].atts["units"].read(item.units);
            boost::algorithm::to_lower(item.units);
            result.push_back(item);
        }
    }
    return result;
}

std::vector<int> analysis::averaging_periods() const
{
    if (ds_.vars.contains("ave"))
        return ds_.vars["ave"].values<int>();
    return {};
}

std::vector<std::string> analysis::source_groups() const
{
    if (ds_.vars.contains("grp"))
        return ds_.vars["grp"].values<std::string>();
    return {};
}

std::vector<std::string> analysis::receptor_arcids() const
{
    if (ds_.vars.contains("arcid"))
        return ds_.vars["arcid"].values<std::string>();
    return {};
}

std::vector<std::string> analysis::receptor_netids() const
{
    if (ds_.vars.contains("netid"))
        return ds_.vars["netid"].values<std::string>();
    return {};
}

std::size_t analysis::receptor_count() const
{
    if (ds_.vars.contains("rec"))
        return ds_.vars["rec"].size();
    return 0;
}

std::vector<receptor_t> analysis::receptors() const
{
    if (receptor_count() == 0)
        return {};

    const auto recs  = ds_.vars["rec"].values<int>();
    const auto x     = ds_.vars["x"].values<double>();
    const auto y     = ds_.vars["y"].values<double>();
    const auto zelev = ds_.vars["zelev"].values<double>();
    const auto zhill = ds_.vars["zhill"].values<double>();
    const auto zflag = ds_.vars["zflag"].values<double>();

    if (!equal_length(recs, x, y, zelev, zhill, zflag))
        throw std::out_of_range("Receptor coordinate length mismatch.");

    const auto arcids = receptor_arcids();
    const auto netids = receptor_netids();

    std::vector<int> arcs;
    if (!arcids.empty() && ds_.vars.contains("arc") && ds_.vars["arc"].size() == recs.size()) {
        arcs = ds_.vars["arc"].values<int>();
    }

    std::vector<int> nets;
    if (!netids.empty() && ds_.vars.contains("net") && ds_.vars["net"].size() == recs.size()) {
        nets = ds_.vars["net"].values<int>();
    }

    std::vector<receptor_t> result;

    result.reserve(recs.size());
    for (std::size_t i = 0; i < recs.size(); ++i)
    {
        std::string arcid;
        std::string netid;

        if (!arcs.empty() && arcs[i] > 0)
            arcid = arcids.at(static_cast<std::size_t>(arcs[i] - 1));
        if (!nets.empty() && nets[i] > 0)
            netid = netids.at(static_cast<std::size_t>(nets[i] - 1));

        result.emplace_back(receptor_t{ recs[i], x[i], y[i], zelev[i], zhill[i], zflag[i], arcid, netid });
    }

    return result;
}

std::size_t analysis::time_step_count() const
{
    if (ds_.vars.contains("time"))
        return ds_.vars["time"].size();
    return 0;
}

std::vector<time_step_t> analysis::time_steps() const
{
    if (time_step_count() == 0)
        return {};

    auto time = ds_.vars["time"].values<date::sys_seconds>();

    std::vector<unsigned char> clmsg;
    if (ds_.vars.contains("clmsg"))
        clmsg = ds_.vars["clmsg"].values<unsigned char>();

    std::vector<time_step_t> result;
    result.reserve(time.size());
    for (std::size_t i = 0; i < time.size(); ++i) {
        unsigned char iclmsg = clmsg.empty() ? 0xff : clmsg[i];
        result.emplace_back(time_step_t{ time[i], iclmsg });
    }

    return result;
}

namespace accumulators {

namespace ba = boost::accumulators;

struct epa_rolling_mean
{
    using rmacc_t = ba::accumulator_set<double, ba::stats<ba::tag::rolling_mean>>;
    using rsacc_t = ba::accumulator_set<double, ba::stats<ba::tag::rolling_sum, ba::tag::rolling_count>>;
    using maxacc_t = ba::accumulator_set<double, ba::features<ba::tag::max>>;

    epa_rolling_mean(int ave, int hours)
        : ave_(ave), value_(0),
          rsacc1_(rsacc_t(ba::tag::rolling_window::window_size = hours / ave)),
          rsacc2_(rsacc_t(ba::tag::rolling_window::window_size = hours / ave)),
          rmacc_(rmacc_t(ba::tag::rolling_window::window_size = hours / ave))
    {
        // This should evaluate true for any short-term averaging periods:
        // (1, 2, 3, 4, 6, 8, 12, 24)
        if (hours % ave != 0)
            throw std::runtime_error("Invalid averaging period.");
    }

    void operator()(double val, unsigned char cmflag)
    {
        if (ave_ == 1) {
            // Apply EPA averaging policy for 1 hour averaging period.
            double cm = (cmflag == 0) ? 0 : 1;
            rsacc1_(val);
            rsacc2_(cm);

            // Current number of elements in the rolling window.
            double n = static_cast<double>(ba::rolling_count(rsacc1_));

            if (n <= 24) {
                // Short-term average. See SUBROUTINE AVER in AERMOD calc2.f
                double numerator = ba::rolling_sum(rsacc1_);
                double denominator = std::max(n - ba::rolling_sum(rsacc2_), std::round(n * 0.75 + 0.4));
                value_ = numerator / denominator;
            }
            else {
                // Long-term average. See SUBROUTINE PERAVE in AERMOD output.f
                double numerator = ba::rolling_sum(rsacc1_);
                double denominator = n - ba::rolling_sum(rsacc2_);
                value_ = numerator / denominator;
            }
        }
        else {
            // Calculate standard rolling mean. Averaging already applied by AERMOD.
            rmacc_(val);
            value_ = ba::rolling_mean(rmacc_);
        }

        // Track the maximum value.
        maxacc_(value_);
    }

    double value() const
    {
        return value_;
    }

    double max() const
    {
        return ba::max(maxacc_);
    }

private:
    int ave_;
    double value_;
    rsacc_t rsacc1_; // rolling sum concentration (ave == 1)
    rsacc_t rsacc2_; // rolling sum valid hours (ave == 1)
    rmacc_t rmacc_;  // rolling mean concentration (ave != 1)
    maxacc_t maxacc_;
};

} // namespace accumulators


void analysis::set_progress_function(const progress_function_type& fn)
{
    progressfn_ = fn;
}

void analysis::export_time_series(const options::general& opts, const options::tsexport& exopts) const
{
    using rmacc_t = accumulators::epa_rolling_mean;

    // Read the output matrix.
    auto recs = receptors();
    auto arcids = receptor_arcids();
    auto netids = receptor_netids();
    auto times = time_steps();
    auto matrix = output_matrix(opts.averaging_period, opts.source_group, opts.output_type, opts.scale_factor);
    auto allave = ds_.vars["ave"].values<int>();
    int minave = *std::min_element(allave.begin(), allave.end());

    std::ofstream ofs(exopts.output_file);

    // Write CSV header.
    fmt::memory_buffer header;

    if (opts.averaging_period == 1)
        fmt::format_to(header, "arcid,netid,receptor,time,calm_missing,x,y,zelev,zhill,zflag,{}", opts.output_type);
    else
        fmt::format_to(header, "arcid,netid,receptor,time,x,y,zelev,zhill,zflag,{}", opts.output_type);

    if (exopts.rm_windows.size() > 0) {
        for (const auto& w : exopts.rm_windows) {
            fmt::format_to(header, ",{}[RM{}]", opts.output_type, w);
        }
    }

    fmt::format_to(header, "\n");
    ofs << fmt::to_string(header);

    // Write CSV records.
    for (std::size_t j = 0; j < matrix.size2(); ++j) // receptors
    {
        progressfn_(j);

        const auto& rec = recs.at(j);

        // Create the rolling mean accumulators.
        std::vector<rmacc_t> rmaccs;
        rmaccs.reserve(exopts.rm_windows.size());
        for (double w : exopts.rm_windows) {
            rmaccs.emplace_back(rmacc_t(opts.averaging_period, static_cast<int>(w) * 24));
        }

        fmt::memory_buffer buffer1;
        fmt::format_to(buffer1, "{},{},{:d},{:.2f},{:.2f},{:.2f},{:.2f},{:.2f}",
            rec.arcid, rec.netid, rec.id, rec.x, rec.y,
            rec.zelev, rec.zhill, rec.zflag);

        for (std::size_t i = 0; i < matrix.size1(); ++i) // time steps
        {
            std::vector<double> rmvals(exopts.rm_windows.size(), 0);

            const auto ts = times.at(i * opts.averaging_period / minave);
            const double val = matrix.at_element(i, j); // ntime * nrecs, column-major

            for (auto&& rmacc : rmaccs)
                rmacc(val, ts.calm_missing);

            fmt::memory_buffer buffer2;

            if (opts.averaging_period == 1)
                fmt::format_to(buffer2, ",{},{},{:.6g}", date::format("%F %R", ts.time), ts.calm_missing, val);
            else
                fmt::format_to(buffer2, ",{},{:.6g}", date::format("%F %R", ts.time), val);

            for (const auto& rmacc : rmaccs)
                fmt::format_to(buffer2, ",{}", rmacc.value());

            ofs << fmt::to_string(buffer1) << fmt::to_string(buffer2) << "\n";
        }
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
    using rmacc_t = accumulators::epa_rolling_mean;

    // Read the output matrix.
    std::size_t nrecs = receptor_count();
    auto times = time_steps();
    auto matrix = output_matrix(opts.averaging_period, opts.source_group, opts.output_type, opts.scale_factor);
    auto allave = ds_.vars["ave"].values<int>();
    int minave = *std::min_element(allave.begin(), allave.end());

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

    // Initialize the output vectors for rolling mean calculations.
    for (double w : statopts.maxrm_windows) {
        std::vector<double> v(nrecs, 0);
        out.rm.push_back(v);
    }

    // Analyze the data array for each receptor.
    for (std::size_t j = 0; j < matrix.size2(); ++j) // receptors
    {
        progressfn_(j);

        // Create the accumulators.
        avgacc_t avgacc;
        maxacc_t maxacc;
        varacc_t varacc;

        std::vector<p2acc_t> p2accs;
        p2accs.reserve(statopts.percentiles.size());
        for (double p : statopts.percentiles)
            p2accs.emplace_back(p2acc_t(quantile_probability = p));

        std::vector<rmacc_t> rmaccs;
        rmaccs.reserve(statopts.maxrm_windows.size());
        for (double w : statopts.maxrm_windows)
            rmaccs.emplace_back(rmacc_t(opts.averaging_period, static_cast<int>(w) * 24));

        // Main update loop.
        for (std::size_t i = 0; i < matrix.size1(); ++i) // time steps
        {
            const auto ts = times.at(i * opts.averaging_period / minave);
            const double val = matrix.at_element(i, j); // ntime * nrecs, column-major

            if (statopts.calc_avg)
                avgacc(val);
            if (statopts.calc_max)
                maxacc(val);
            if (statopts.calc_std)
                varacc(val);
            for (auto&& p2acc : p2accs)
                p2acc(val);
            for (auto&& rmacc : rmaccs)
                rmacc(val, ts.calm_missing);
        }

        // Extract results.
        if (statopts.calc_avg)
            out.avg[j] = mean(avgacc);
        if (statopts.calc_max)
            out.max[j] = max(maxacc);
        if (statopts.calc_std)
            out.std[j] = sqrt(variance(varacc));
        for (std::size_t iacc = 0; iacc < p2accs.size(); ++iacc)
            out.p2.at(iacc).at(j) = p_square_quantile(p2accs[iacc]);
        for (std::size_t iacc = 0; iacc < p2accs.size(); ++iacc)
            out.rm.at(iacc).at(j) = rmaccs[iacc].max();
    }
}

void analysis::calc_histogram(const options::general& opts, const options::histogram& histopts, histogram_type& out) const
{
    using namespace boost::accumulators;

    using cdfacc_t = accumulator_set<double, stats<tag::p_square_cumulative_distribution>>;
    using pdfacc_t = accumulator_set<double, stats<tag::density>>;

    if (!histopts.calc_cdf && !histopts.calc_pdf)
        return;

    // Read the data array.
    auto matrix = output_matrix(opts.averaging_period, opts.source_group, opts.output_type, opts.scale_factor);

    // Create the accumulators.
    cdfacc_t cdfacc(tag::p_square_cumulative_distribution::num_cells = static_cast<std::size_t>(histopts.cdf_bins));

    // If selected cache size exceeds sample count, use all samples.
    std::size_t cache_size = std::max(static_cast<std::size_t>(histopts.pdf_cache_size), matrix.data().size());

    pdfacc_t pdfacc(tag::density::num_bins = static_cast<std::size_t>(histopts.pdf_bins),
                    tag::density::cache_size = cache_size);

    // Analyze the data array.
    for (std::size_t j = 0; j < matrix.size2(); ++j) // receptors
    {
        progressfn_(j);

        for (std::size_t i = 0; i < matrix.size1(); ++i) { // time steps
            const double val = matrix.at_element(i, j); // ntime * nrecs, column-major
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
        out.cdf.insert(out.cdf.end(), std::make_move_iterator(cdf.begin()), std::make_move_iterator(cdf.end()));
    }
    if (histopts.calc_pdf) {
        histogram_t pdf = density(pdfacc);
        out.pdf.insert(out.pdf.end(), std::make_move_iterator(pdf.begin()), std::make_move_iterator(pdf.end()));
    }
}

matrix_t analysis::output_matrix(int ave, const std::string& grp, const std::string& var, double sf) const
{
    auto slice = ds_.vars[var].select(
        ncpp::selection<int>{"ave", ave, ave},
        ncpp::selection<std::string>{"grp", grp, grp});

    // Get full matrix dimensions.
    std::size_t ntime = ds_.vars["time"].size();
    std::size_t nrecs = ds_.vars["rec"].size();
    auto allave = ds_.vars["ave"].values<int>();
    int minave = *std::min_element(allave.begin(), allave.end());

    // Recalculate ntime to exclude fill values for multiple averaging periods.
    ntime = ntime / static_cast<std::size_t>(ave / minave);

    // Read the data array and remove fill values.
    auto values = slice.values<double, aligned_allocator_t>();
    values.erase(std::remove(values.begin(), values.end(), NC_FILL_DOUBLE), values.end());
    if (values.size() != ntime * nrecs)
        throw std::runtime_error("Data array has unexpected missing values.");

    matrix_t result(ntime, nrecs); // column-major
    std::copy(values.begin(), values.end(), result.data().begin());

    //matrix_t result = slice.matrix<double, aligned_allocator_t>();
    //auto end = std::remove(result.data().begin(), result.data().end(), NC_FILL_DOUBLE);
    //if (std::distance(result.data().begin(), end) != ntime * nrecs)
    //    throw std::runtime_error("Data array has unexpected missing values.");
    //result.resize(ntime, nrecs); // column-major

    // Apply scale factor.
    if (sf != 1.0)
        result *= sf;

    return result;
}

} // namespace ncpost

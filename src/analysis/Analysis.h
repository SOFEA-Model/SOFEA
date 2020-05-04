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

#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/align/aligned_allocator.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <date/date.h>

#define NCPP_USE_BOOST
#define NCPP_USE_DATE_H
#include <ncpp/ncpp.hpp>

#include "AnalysisOptions.h"

namespace ncpost {

struct output_type_t {
    std::string name;
    std::string units;
};

struct receptor_t {
    int id;
    double x;
    double y;
    double zelev;
    double zhill;
    double zflag;
    std::string arcid;
    std::string netid;
};

struct time_step_t {
    date::sys_seconds time;
    unsigned char calm_missing;
};

// 64-byte alignment is preferred for MKL.
using aligned_allocator_t = boost::alignment::aligned_allocator<double, 64>;

using matrix_t = boost::numeric::ublas::matrix<double,
    boost::numeric::ublas::column_major,
    boost::numeric::ublas::unbounded_array<double, aligned_allocator_t>>;

struct statistics_type
{
    std::vector<double> avg;
    std::vector<double> max;
    std::vector<double> std;
    std::vector<std::vector<double>> p2;
    std::vector<std::vector<double>> rm;
};

struct histogram_type
{
    std::vector<std::pair<double, double>> cdf;
    std::vector<std::pair<double, double>> pdf;
};

class analysis
{
public:
    explicit analysis(const std::string& filepath);

    static std::string library_version();
    std::string model_version() const;
    std::string model_options() const;
    std::string title() const;
    std::vector<output_type_t> output_types() const;
    std::vector<int> averaging_periods() const;
    std::vector<std::string> source_groups() const;
    std::vector<std::string> receptor_arcids() const;
    std::vector<std::string> receptor_netids() const;
    std::size_t receptor_count() const;
    std::vector<receptor_t> receptors() const;
    std::size_t time_step_count() const;
    std::vector<time_step_t> time_steps() const;

    void set_progress_function(const std::function<void(std::size_t)>& fn);
    void export_time_series(const options::general& opts, const options::tsexport& exopts) const;
    void calc_receptor_stats(const options::general& opts, const options::statistics& statopts, statistics_type& out) const;
    void calc_histogram(const options::general& opts, const options::histogram& histopts, histogram_type& out) const;

private:
    matrix_t output_matrix(int ave, const std::string& grp, const std::string& var, double sf = 1.0) const;

    ncpp::file file_;
    ncpp::dataset ds_;

    std::function<void(std::size_t)> progressfn_;
};

} // namespace ncpost


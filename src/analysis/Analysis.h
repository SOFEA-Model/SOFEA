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
#include <string>
#include <utility>
#include <vector>

#include <date/date.h>

#define NCPP_USE_DATE_H
#include <ncpp/ncpp.hpp>

#include "AnalysisOptions.h"

namespace ncpost {

std::string library_version();

struct metadata_type
{
    struct output_type {
        std::string name;
        std::string units;
    };

    struct receptor {
        int id;
        double x;
        double y;
        double zelev;
        double zhill;
        double zflag;
    };

    std::string model_version;
    std::string model_options;
    std::string title;
    std::vector<output_type> output_types;
    std::vector<int> averaging_periods;
    std::vector<std::string> source_groups;
    std::vector<receptor> receptors;
    std::map<int, std::string> receptor_netid;
    std::map<int, std::string> receptor_arcid;
    std::vector<date::sys_seconds> time_steps;
};

struct statistics_type
{
    int id;
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

    metadata_type metadata() const;
    void export_time_series(const options::general& opts, const options::tsexport& exopts) const;
    void calc_receptor_stats(const options::general& opts, const options::statistics& statopts, statistics_type& out) const;
    void calc_histogram(const options::general& opts, const options::histogram& histopts, histogram_type& out) const;

private:
    void read_values(const options::general& opts, std::vector<double>& values,
                     std::size_t& ntime, std::size_t& nrec, int& minave) const;

    std::string filepath_;
    ncpp::file file_;
    ncpp::dataset ds_;
    metadata_type metadata_;
};

} // namespace ncpost


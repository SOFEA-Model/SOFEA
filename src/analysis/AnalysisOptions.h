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

#include <string>
#include <vector>

#include <date/date.h>

namespace ncpost {
namespace options {

struct general
{
    std::string output_type;
    int averaging_period;
    std::string source_group;
    double scale_factor = 1;
    date::sys_seconds start_time;
    date::sys_seconds end_time;
};

struct statistics
{
    bool calc_avg = true;
    bool calc_max = true;
    bool calc_std = false;
    std::vector<double> percentiles;
    std::vector<int> maxrm_windows;
};

struct histogram
{
    bool calc_cdf = false;
    bool calc_pdf = false;
    int cdf_bins = 100;
    int pdf_bins = 100;
    int pdf_cache_size = 1000;
};

} // namespace options
} // namespace ncpost

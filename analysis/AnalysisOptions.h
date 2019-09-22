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

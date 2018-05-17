#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct AnalysisOptions
{
    bool calcRecMean = false;
    bool calcRecMax = false;
    bool calcRecStdDev = false;
    bool calcRecP2 = false;
    std::vector<double> recPercentiles;
    bool calcRecMaxRM = false;
    std::vector<double> recWindowSizes;
    bool calcCDF = false;
    bool calcPDF = false;
    int cdfBins = 0;
    int pdfBins = 0;
    int pdfCacheSize = 0;
};

struct ReceptorStats
{
    std::vector<int> id;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
    std::vector<double> avg;
    std::vector<double> max;
    std::vector<double> std;
    std::vector<std::vector<double>> p2;
    std::vector<std::vector<double>> rm;
};

struct Histogram
{
    std::vector<std::pair<double, double>> cdf;
    std::vector<std::pair<double, double>> pdf;
};

class Analysis
{
public:
    explicit Analysis(const std::string &file);

    bool openFile(int& ncid);
    bool calcReceptorStats(int& ncid, AnalysisOptions opts, ReceptorStats& out);
    bool calcHistogram(int& ncid, AnalysisOptions opts, Histogram& out);
    bool closeFile(int& ncid);

    long long getTimeStepCount() const;
    long long getReceptorCount() const;
    std::string getLastError() const;
    std::string libraryVersion() const;

private:
    std::string filename;
    std::string errmsg;

    int dim_time, dim_type, dim_recs;   // netCDF dimension IDs
    int var_x, var_y, var_z, var_conc;  // netCDF variable IDs
    int var_time, var_type, var_recs;   // netCDF coordinate variable IDs

    size_t ntime = 0;
    size_t ntype = 0;
    size_t nrecs = 0;
};

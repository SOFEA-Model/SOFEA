#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <utility>

struct GeneralAnalysisOpts
{
    std::string type;
    int avePeriod = 1;
    double scaleFactor = 1;
    std::string sourceGroup;
};

struct ReceptorAnalysisOpts
{
    bool calcRecMean = false;
    bool calcRecMax = false;
    bool calcRecStdDev = false;
    bool calcRecP2 = false;
    std::vector<double> recPercentiles;
    bool calcRecMaxRM = false;
    std::vector<double> recWindowSizes;
};

struct HistogramAnalysisOpts
{
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
    struct dim_info_t {
        int id;
        size_t len = 0;
    };

    struct att_info_t {
        int id;
        int xtype;
        size_t len = 0;
        std::string value;
    };

    struct var_info_t {
        int id;
        int xtype;
        std::vector<int> dimids;
        std::map<std::string, att_info_t> atts;
    };

public:
    explicit Analysis(const std::string &file);
    ~Analysis();

    void exportTimeSeries(GeneralAnalysisOpts genOpts, const std::string& filename);
    void calcReceptorStats(GeneralAnalysisOpts genOpts, ReceptorAnalysisOpts opts, ReceptorStats& out) const;
    void calcHistogram(GeneralAnalysisOpts genOpts, HistogramAnalysisOpts opts, Histogram& out) const;

    std::string getModelVersion() const;
    std::string getModelOptions() const;
    std::string getTitle() const;
    int getTimeStepCount() const;
    int getReceptorCount() const;
    std::vector<int> getAveragingPeriods() const;
    std::vector<std::string> getSourceGroups() const;
    std::vector<std::pair<std::string, std::string>> getTypes() const;
    std::string getUnits(const std::string& varname) const;
    std::string getTimeString(const double reltime) const;
    static std::string libraryVersion();

private:
    void setAttributeValue(int varid, att_info_t &info, const char *name) const;
    void init();

    std::string filename;
    int ncid;
    std::map<std::string, dim_info_t> dims;
    std::map<std::string, var_info_t> vars;
    std::map<std::string, att_info_t> global_atts;
};

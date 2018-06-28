#pragma once

#include <map>
#include <string>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

#include <QDateTime>

#include "SourceGroup.h"

struct SurfaceInfo
{
    // Header
    std::string mplat;
    std::string mplon;
    std::string ualoc;
    std::string sfloc;
    std::string osloc;
    std::string versno;

    // Derived
    int nrec = 0;
    int ncalm = 0;
    int nmiss = 0;

    boost::posix_time::ptime tmin = boost::posix_time::ptime();
    boost::posix_time::ptime tmax = boost::posix_time::ptime();
    std::vector<std::string> intervals;
};

struct Scenario
{
    Scenario();

    void resetSurfaceFileInfo();
    std::string writeInput() const;
    void writeInputFile(const std::string& path) const;
    void writeFluxFile(const std::string& path) const;

    static const std::map<int, std::string> chemicalMap;

    // General
    unsigned int id;
    std::string title;
    int fumigantId;
    double decayCoefficient;

    // Meteorological Data
    std::string surfaceFile;
    std::string upperAirFile;
    double anemometerHeight;
    double windRotation;

    // Surface File Info
    SurfaceInfo sfInfo;
    QDateTime minTime;
    QDateTime maxTime;
    std::string surfaceId;
    std::string upperAirId;

    // AERMOD
    bool aermodFlat;
    bool aermodFastArea;
    bool aermodLowWind;
    double svMin;    // minimum sigma-v
    double wsMin;    // minimum wind speed
    double franMax;  // maximum meander factor

    // ISCST3
    bool iscNonDefault;
    bool iscNoCalm;
    bool iscMsgPro;

    // SourceGroup Container
    boost::ptr_vector<SourceGroup> sourceGroups;
};

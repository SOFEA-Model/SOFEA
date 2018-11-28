#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <QDateTime>

#include "FluxProfile.h"
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
    double flagpoleHeight;

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

    // AERMOD Non-DFAULT
    bool aermodFlat;                        // assume flat terrain (FLAT)
    bool aermodFastArea;                    // optimize for AREA sources (FASTAREA)
    bool aermodDryDeposition;               // enable dry deposition (DDEP)
    bool aermodDryDplt;                     // enable dry depletion (DRYDPLT/NODRYDPLT)
    bool aermodAreaDplt;                    // enable optimized plume depletion for AREA sources (AREADPLT)
    bool aermodWetDeposition;               // enable wet deposition (WDEP)
    bool aermodWetDplt;                     // enable wet depletion (WETDPLT/NOWETDPLT)
    std::array<int, 12> aermodGDSeasons;    // season (1-5) by month (GDSEASON)
    std::array<int, 36> aermodGDLandUse;    // land use category (1-9) by sector (10 degrees) (GDLANUSE)
    double aermodGDReact;                   // pollutant reactivity (GASDEPDF)
    std::array<double, 5> aermodGDSeasonF;  // fraction of max green LAI by season (1-5) (GASDEPDF)
    bool aermodGDVelocityEnabled;           // enable user-specified deposition velocity
    double aermodGDVelocity;                // user-specified deposition velocity (GASDEPVD)

    // AERMOD ALPHA
    bool aermodLowWind;
    double aermodSVmin;                     // minimum sigma-v
    double aermodWSmin;                     // minimum wind speed
    double aermodFRANmax;                   // maximum meander factor

    // Output
    std::vector<int> averagingPeriods;

    // Receptors
    //std::vector<ReceptorRing> rings;
    //std::vector<ReceptorNode> nodes;
    //std::vector<ReceptorGrid> grids;

    // Containers
    std::vector<std::shared_ptr<FluxProfile>> fluxProfiles;
    boost::ptr_vector<SourceGroup> sourceGroups;
};

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
#include "Receptor.h"
#include "Projection.h"

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
    Scenario(const Scenario& rhs);

    void resetSurfaceFileInfo();
    std::string writeInput() const;
    void writeInputFile(const std::string& path) const;
    void writeFluxFile(const std::string& path) const;

    static const std::map<int, std::string> chemicalMap;

    // General
    unsigned int id;
    std::string name;
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
    std::vector<ReceptorGroup> receptors;

    // Flux Profiles
    using FluxProfilePtr = std::shared_ptr<FluxProfile>;
    std::vector<FluxProfilePtr> fluxProfiles;

    // Source Groups
    using SourceGroupPtr = std::shared_ptr<SourceGroup>;
    std::vector<SourceGroupPtr> sourceGroups;

    // Coordinate System
    Projection::Area domain;
    std::string conversionCode;
    std::string hUnitsCode;
    std::string hDatumCode;
    std::string vUnitsCode;
    std::string vDatumCode;
};

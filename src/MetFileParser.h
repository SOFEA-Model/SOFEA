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

#ifndef METFILEPARSER_H
#define METFILEPARSER_H

#include <string>
#include <tuple>
#include <vector>

#include <QString>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>

#include "core/Scenario.h"

//-----------------------------------------------------------------------------
// SurfaceData
//-----------------------------------------------------------------------------

struct SurfaceData
{


    struct SurfaceHeader
    {
        // Format from MPMET.FOR in AERMET source.
        // Required header only. Excludes THRESH1SPD, CC_TT_Subs, MMIF_Vers.

        std::string mplat;      // T2,A8    latitude
        std::string mplon;      // T12,A8   longitude
        std::string ualoc;      // T37,A8   upper air station identifier
        std::string sfloc;      // T54,A8   surface station identifier
        std::string osloc;      // T71,A8   site-specific identifier
        std::string versno;     // T93,A6   AERMET version
    };

    struct SurfaceRecord
    {
        // Format from MPOUT.FOR in AERMET source.
        // Variable definitions from MP2.INC in AERMET source.

        int mpyr;               // I2,1X    year
        int mpcmo;              // I2,1X    month
        int mpcdy;              // I2,1X    day
        int mpjdy;              // I3,1X    Julian day
        int j;                  // I2,1X    hour
        double hflux;           // F6.1,1X  sensible heat flux (W/m2)
        double ustar;           // F6.3,1X  surface friction velocity (m/s)
        double wstar;           // F6.3,1X  convective velocity scale (m/s)
        double vptg;            // F6.3,1X  vertical potential temperature gradient above PBL
        double ziconv;          // F5.0,1X  height of convectively-generated boundary layer (m)
        double zimech;          // F5.0,1X  height of mechanically-generated boundary layer (m)
        double mol;             // F8.1,1X  Monin-Obukhov length (m)
        double z0;              // F7.4,1X  surface roughness length (m)
        double bowen;           // F6.2,1X  Bowen ratio
        double albedo;          // F6.2,1X  albedo
        double wspd;            // F7.2,1X  wind speed (m/s)
        double wdir;            // F6.1,1X  wind direction (degrees)
        double zref;            // F6.1,1X  reference height for Ws and Wd (m)
        double t;               // F6.1,1X  temperature (K)
        double ztref;           // F6.1,1X  reference height for temp (m)
        int ipcode;             // I5,1X    precipitation code
        double pamt;            // F6.2,1X  precipitation amount (mm)
        double rh;              // F6.0,1X  relative humidity
        double p;               // F6.0,1X  surface pressure
        int ccvr;               // I5,1X    cloud cover
        std::string wsadj;      // A7,1X    flag for WS adjustment
        std::string subs;       // A12      flags for CCVR and TEMP substitutions
        bool calm;              //          derived
        bool miss;              //          derived

        friend bool operator<(const SurfaceRecord& lhs, const SurfaceRecord& rhs) {
            return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
                 < std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
        }
    };

    SurfaceHeader header;
    std::vector<SurfaceRecord> records;
    boost::icl::interval_set<boost::posix_time::ptime> intervals;
    int nrec = 0;
    int ncalm = 0;
    int nmiss = 0;
};

//-----------------------------------------------------------------------------
// ProfileData
//-----------------------------------------------------------------------------

struct ProfileData
{
    struct ProfileRecord
    {
        int mpyr;               // I2,1X    year
        int mpcmo;              // I2,1X    month
        int mpcdy;              // I2,1X    day
        int j;                  // I2,1X    hour
        double ht;              // F6.1,1X  measurement height (m)
        int top;                // I1,1X    top of profile flag
        double wdir;            // F5.0,1X  wind direction at the current level (degrees)
        double wspd;            // F7.2,1X  wind speed at the current level (m/s)
        double t;               // F7.1,1X  temperature at the current level (C)
        double sa;              // F6.1,1X  sigma-A (degrees)
        double sw;              // F7.2     sigma-W (m/s)

        friend bool operator<(const ProfileRecord& lhs, const ProfileRecord& rhs) {
            return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
                 < std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
        }
    };

    std::vector<ProfileRecord> records;
    boost::icl::interval_set<boost::posix_time::ptime> intervals;
    int nrec = 0;
};

//-----------------------------------------------------------------------------
// MetFileParser
//-----------------------------------------------------------------------------

class MetFileParser
{
public:
    MetFileParser(const QString& filename);
    MetFileParser(const std::string& filename);

    SurfaceInfo getSurfaceInfo() const;
    std::shared_ptr<SurfaceData> getSurfaceData() const;

private:
    std::shared_ptr<SurfaceData> sd = nullptr;
};

#endif // METFILEPARSER_H

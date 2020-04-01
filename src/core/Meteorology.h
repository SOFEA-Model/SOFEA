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

#include <filesystem>
#include <istream>
#include <locale>
#include <memory>
#include <ostream>
#include <utility>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>

struct SurfaceRecord;
struct UpperAirRecord;

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

    friend std::istream& operator>>(std::istream&, SurfaceHeader&);

    friend bool operator==(const SurfaceRecord& lhs, const UpperAirRecord& rhs);
    friend bool operator==(const UpperAirRecord& lhs, const SurfaceRecord& rhs);
};

struct SurfaceRecord
{
    // Format from MPOUT.FOR in AERMET source.
    // Variable definitions from MP2.INC in AERMET source.

    unsigned short mpyr;    // I2,1X    year
    unsigned short mpcmo;   // I2,1X    month
    unsigned short mpcdy;   // I2,1X    day
    unsigned short mpjdy;   // I3,1X    Julian day
    unsigned short j;       // I2,1X    hour
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
    int wsadj;              // A7,1X    flag for WS adjustment
    int subs;               // A12      flags for CCVR and TEMP substitutions
    bool calm;              //          derived
    bool missing;           //          derived

    friend std::istream& operator>>(std::istream&, SurfaceRecord&);

    friend bool operator<(const SurfaceRecord& lhs, const SurfaceRecord& rhs) {
        return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
             < std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
    }

    friend bool operator==(const SurfaceRecord& lhs, const UpperAirRecord& rhs);
    friend bool operator==(const SurfaceRecord& lhs, const UpperAirRecord& rhs);
};

struct UpperAirRecord
{
    unsigned short mpyr;    // I2,1X    year
    unsigned short mpcmo;   // I2,1X    month
    unsigned short mpcdy;   // I2,1X    day
    unsigned short j;       // I2,1X    hour
    double ht;              // F6.1,1X  measurement height (m)
    int top;                // I1,1X    top of profile flag
    double wdir;            // F5.0,1X  wind direction at the current level (degrees)
    double wspd;            // F7.2,1X  wind speed at the current level (m/s)
    double t;               // F7.1,1X  temperature at the current level (C)
    double sa;              // F6.1,1X  sigma-A (degrees)
    double sw;              // F7.2     sigma-W (m/s)

    friend std::istream& operator>>(std::istream&, UpperAirRecord&);

    friend bool operator<(const UpperAirRecord& lhs, const UpperAirRecord& rhs) {
        return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
             < std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
    }
};

// Comparison Operators

inline bool operator==(const SurfaceRecord& lhs, const UpperAirRecord& rhs) {
   return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
       == std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
}

inline bool operator==(const UpperAirRecord& lhs, const SurfaceRecord& rhs) {
   return std::tie(lhs.mpyr, lhs.mpcmo, lhs.mpcdy, lhs.j)
       == std::tie(rhs.mpyr, rhs.mpcmo, rhs.mpcdy, rhs.j);
}

// Output Stream Operators

inline std::ostream& operator<<(std::ostream& os, const boost::icl::discrete_interval<boost::posix_time::ptime>& rhs)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    ptime lower = rhs.lower();
    ptime upper = rhs.upper();
    time_period period(lower, upper);

    time_facet *facet = new time_facet(); // std::locale handles destruction
    facet->format("%Y-%m-%d %H:%M");
    period_formatter formatter(period_formatter::AS_OPEN_RANGE, ", ", "[", ")", "]");
    facet->period_formatter(formatter);

    std::ostringstream ss;
    ss.imbue(std::locale(std::locale::classic(), facet));
    ss << period;
    os << ss.str();

    return os;
}

struct MetFile
{
    MetFile(const std::filesystem::path& p) : path_(p) {}

    bool isValid() const {
        namespace fs = std::filesystem;
        std::error_code ec;
        bool exists = fs::exists(path_, ec);
        if (ec.value()) return false;
        bool isfile = fs::is_regular_file(path_, ec);
        if (ec.value()) return false;
        return exists && isfile;
    }

    std::string path() const {
        return path_.string();
    }

    std::filesystem::file_time_type lastWriteTime() const {
        namespace fs = std::filesystem;
        std::error_code ec;
        return fs::last_write_time(path_, ec);
    }

    boost::posix_time::ptime minTime() const {
        if (intervals_.empty())
            return boost::posix_time::ptime();
        return boost::icl::first(intervals_);
    }

    boost::posix_time::ptime maxTime() const {
        if (intervals_.empty())
            return boost::posix_time::ptime();
        return boost::icl::last(intervals_);
    }

    boost::icl::interval_set<boost::posix_time::ptime> intervals() const {
        return intervals_;
    }

protected:
    std::size_t nrows_ = 0;
    std::filesystem::path path_;
    boost::icl::interval_set<boost::posix_time::ptime> intervals_;
};

struct SurfaceFile : MetFile
{
    SurfaceFile(const std::filesystem::path& p);

    std::size_t calmHours() const {
        return ncalm_;
    }

    std::size_t missingHours() const {
        return nmissing_;
    }

    const SurfaceHeader& header() const {
        return header_;
    }

    std::vector<SurfaceRecord> records() const;

private:
    SurfaceHeader header_;
    std::size_t ncalm_ = 0;
    std::size_t nmissing_ = 0;
};

struct UpperAirFile : MetFile
{
    UpperAirFile(const std::filesystem::path& p);

    std::vector<UpperAirRecord> records() const;
};

struct Meteorology
{
    Meteorology(const std::filesystem::path& sfc,
                const std::filesystem::path& pfl);

    std::string name;
    SurfaceFile surfaceFile;
    UpperAirFile upperAirFile;
    double terrainElevation = 0;
    double anemometerHeight = 10;
    double windRotation = 0;
};

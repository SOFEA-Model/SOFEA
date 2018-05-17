#include "MetFileParser.h"

#include <fstream>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>

#include "fmt/format.h"

using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::icl;

struct SurfaceHeader
{
    std::string mplat;      // T2,A8    latitude
    std::string mplon;      // T12,A8   longitude
    std::string ualoc;      // T37,A8   upper air station identifier
    std::string sfloc;      // T54,A8   surface station identifier
    std::string osloc;      // T71,A8   site-specific identifier
    std::string versno;     // T93,A6   AERMET version
    double thresh1spd;      // F5.2     optional
    std::string cc_tt_subs; // A17      optional
    std::string mmif_vers;  // A40      optional
};

struct SurfaceRecord
{
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
};

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
};

SurfaceFileInfo MetFileParser::parseSurfaceFile(const std::string& filename)
{
    SurfaceFileInfo info;
    SurfaceHeader sh;
    SurfaceRecord sr;
    std::vector<SurfaceRecord> srvec;
    
    // For tracking contiguous time intervals.
    interval_set<ptime> tset;

    std::ifstream ifs(filename, std::ios_base::in);
    if (!ifs)
        return info;

    std::string line; // buffer
    
    if (std::getline(ifs, line) && line.size() >= 99)
    {
        sh.mplat  = line.substr(2,  8); // T2,A8
        sh.mplon  = line.substr(12, 8); // T12,A8
        sh.ualoc  = line.substr(37, 8); // T37,A8
        sh.sfloc  = line.substr(54, 8); // T54,A8
        sh.osloc  = line.substr(71, 8); // T71,A8
        sh.versno = line.substr(93, 6); // T93,A6
    }

    info.mplat  = boost::trim_copy(sh.mplat);
    info.mplon  = boost::trim_copy(sh.mplon);
    info.ualoc  = boost::trim_copy(sh.ualoc);
    info.sfloc  = boost::trim_copy(sh.sfloc);
    info.osloc  = boost::trim_copy(sh.osloc);
    info.versno = boost::trim_copy(sh.versno);
    
    int ncalm = 0; // track calm hours

    while (ifs.good())
    {
        std::getline(ifs, line);

        std::istringstream iss(line);
        iss >> sr.mpyr >> sr.mpcmo >> sr.mpcdy >> sr.mpjdy >> sr.j
            >> sr.hflux >> sr.ustar >> sr.wstar >> sr.vptg >> sr.ziconv
            >> sr.zimech >> sr.mol >> sr.z0 >> sr.bowen >> sr.albedo
            >> sr.wspd >> sr.wdir >> sr.zref >> sr.t >> sr.ztref;

        if (iss.good())
        {
            if (sr.wspd == 0)
                ncalm++;
            
            // Use a window of 1950 to 2049 for 2-digit years per AERMOD 
            // convention. See v99211 (July 30, 1999) release notes.
            int offset = ((sr.mpyr > 49) ? 1900 : 2000);
            sr.mpyr += offset;
            
            // Append record to output vector
            srvec.push_back(sr);

            // Interval calculations
            ptime t(date(sr.mpyr, sr.mpcmo, sr.mpcdy), hours(sr.j - 1));
            tset += discrete_interval<ptime>::right_open(t, t + hours(1));
        }
    }

    info.tmin = first(tset);
    info.tmax = last(tset);

    // Calculate total number of hours between start and end
    time_duration td = info.tmax - info.tmin;

    info.nrec = srvec.size();
    info.ncalm = ncalm;
    info.nmiss = td.hours() - srvec.size() + 1;

    // Convert intervals to string vector
    for (const auto &interval : tset) {
        std::ostringstream oss;
        oss << interval;
        info.intervals.push_back(oss.str());
    }
    
    return info;
}

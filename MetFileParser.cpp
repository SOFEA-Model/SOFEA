#include "MetFileParser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QString>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>

#include <fmt/format.h>

inline bool checkCalm(const SurfaceData::SurfaceRecord& sr)
{
    // Based on subroutine CHKCLM in METEXT.F

    if (sr.wspd == 0)
        return true;
    else
        return false;
}

inline bool checkMiss(const SurfaceData::SurfaceRecord& sr)
{
    // Based on subroutine CHKMSG in METEXT.F

    // Wind speed (meters/second)
    if (sr.wspd >= 90.0 || sr.wspd < 0.0) {
        return true;
    }
    // Wind direction (degrees from north)
    else if (sr.wdir > 900.0 || sr.wdir <= -9.0) {
        return true;
    }
    // Ambient temperature (kelvins)
    else if (sr.t > 900.0 || sr.t <= 0.0) {
        return true;
    }
    // Monin-Obukhov length (meters)
    else if (sr.mol < -99990.0) {
        return true;
    }
    // Convective Mixing height (meters)
    else if (sr.mol < 0.0 && (sr.ziconv > 90000.0 || sr.ziconv < 0.0)) {
        return true;
    }
    // Mechanical Mixing height (meters)
    else if (sr.zimech > 90000.0 || sr.zimech < 0.0) {
        return true;
    }
    // Surface friction velocity (meters/second)
    else if (sr.ustar < 0.0 || sr.ustar >= 9.0) {
        return true;
    }
    // Convective velocity scale (meters/second)
    else if (sr.wstar < 0.0 && (sr.mol < 0.0 && sr.mol > -99990.0)) {
        return true;
    }
    else {
        return false;
    }
}

MetFileParser::MetFileParser(const QString& filename)
    : MetFileParser(filename.toStdString())
{}

MetFileParser::MetFileParser(const std::string& filename)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;

    std::string path = absolutePath(filename);
    std::ifstream ifs(path, std::ios_base::in);
    if (!ifs)
        return;

    sd = std::make_shared<SurfaceData>();

    std::string line; // buffer

    if (std::getline(ifs, line) && line.size() >= 99)
    {
        sd->header.mplat  = line.substr(2,  8); // T2,A8
        sd->header.mplon  = line.substr(12, 8); // T12,A8
        sd->header.ualoc  = line.substr(37, 8); // T37,A8
        sd->header.sfloc  = line.substr(54, 8); // T54,A8
        sd->header.osloc  = line.substr(71, 8); // T71,A8
        sd->header.versno = line.substr(93, 6); // T93,A6
    }

    while (ifs.good())
    {
        SurfaceData::SurfaceRecord sr;
        std::getline(ifs, line);

        std::istringstream iss(line);
        iss >> sr.mpyr >> sr.mpcmo >> sr.mpcdy >> sr.mpjdy >> sr.j
            >> sr.hflux >> sr.ustar >> sr.wstar >> sr.vptg >> sr.ziconv
            >> sr.zimech >> sr.mol >> sr.z0 >> sr.bowen >> sr.albedo
            >> sr.wspd >> sr.wdir >> sr.zref >> sr.t >> sr.ztref;

        if (iss.good())
        {
            sr.calm = checkCalm(sr);
            sr.miss = checkMiss(sr);

            if (sr.calm)
                sd->ncalm++;
            else if (sr.miss)
                sd->nmiss++;

            // Use a window of 1950 to 2049 for 2-digit years per AERMOD
            // convention. See v99211 (July 30, 1999) release notes.
            int offset = ((sr.mpyr > 49) ? 1900 : 2000);
            sr.mpyr += offset;

            // Append record to output vector
            sd->records.push_back(sr);

            // Interval calculations to track contiguous time intervals.
            ptime t(date(sr.mpyr, sr.mpcmo, sr.mpcdy), hours(sr.j - 1));
            sd->intervals += discrete_interval<ptime>::right_open(t, t + hours(1));
        }
    }

    sd->nrec = sd->records.size();
}

SurfaceInfo MetFileParser::getSurfaceInfo() const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;

    SurfaceInfo info;

    if (sd == nullptr)
        return info;

    // Header Info
    info.mplat  = boost::trim_copy(sd->header.mplat);
    info.mplon  = boost::trim_copy(sd->header.mplon);
    info.ualoc  = boost::trim_copy(sd->header.ualoc);
    info.sfloc  = boost::trim_copy(sd->header.sfloc);
    info.osloc  = boost::trim_copy(sd->header.osloc);
    info.versno = boost::trim_copy(sd->header.versno);

    // Record Info
    info.tmin = first(sd->intervals);
    info.tmax = last(sd->intervals);
    info.nrec = sd->nrec;
    info.ncalm = sd->ncalm;
    info.nmiss = sd->nmiss;

    // Intervals
    for (const discrete_interval<ptime>& i : sd->intervals) {
        std::ostringstream oss;
        ptime lower = i.lower();
        ptime upper = i.upper();
        time_period period(lower, upper);

        // I/O Setup
        time_facet *facet = new time_facet(); // std::locale handles destruction
        facet->format("%Y-%m-%d %H:%M");
        period_formatter formatter(period_formatter::AS_OPEN_RANGE, ", ", "[", ")", "]");
        facet->period_formatter(formatter);
        oss.imbue(std::locale(std::locale::classic(), facet));

        // Output
        oss << period;
        info.intervals.push_back(oss.str());
    }

    return info;
}

std::shared_ptr<SurfaceData> MetFileParser::getSurfaceData() const
{
    return sd;
}

std::string MetFileParser::absolutePath(const std::string& filename)
{
    // If this is a relative path, assume current project directory.
    QString path = QString::fromStdString(filename);
    QFileInfo fi(path);
    if (!path.isEmpty() && fi.isRelative()) {
        QSettings settings;
        QString dir = settings.value("DefaultDirectory", QDir::currentPath()).toString();
        path = QDir::cleanPath(dir + QDir::separator() + path);
    }

    return path.toStdString();
}

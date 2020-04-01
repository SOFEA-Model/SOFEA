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

#include "core/Meteorology.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/spirit/include/qi.hpp>

#include <fmt/format.h>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

#ifdef _MSC_VER
#pragma warning(disable:4068) // Level 1, C4068: unknown pragma
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

BOOST_FUSION_ADAPT_STRUCT(
    SurfaceRecord,
    (unsigned short, mpyr)
    (unsigned short, mpcmo)
    (unsigned short, mpcdy)
    (unsigned short, mpjdy)
    (unsigned short, j)
    (double, hflux)
    (double, ustar)
    (double, wstar)
    (double, vptg)
    (double, ziconv)
    (double, zimech)
    (double, mol)
    (double, z0)
    (double, bowen)
    (double, albedo)
    (double, wspd)
    (double, wdir)
    (double, zref)
    (double, t)
    (double, ztref)
    (int, ipcode)
    (double, pamt)
    (double, rh)
    (double, p)
    (int, ccvr)
    (int, wsadj)
    (int, subs)
)

BOOST_FUSION_ADAPT_STRUCT(
    UpperAirRecord,
    (unsigned short, mpyr)
    (unsigned short, mpcmo)
    (unsigned short, mpcdy)
    (unsigned short, j)
    (double, ht)
    (int, top)
    (double, wdir)
    (double, wspd)
    (double, t)
    (double, sa)
    (double, sw)
)

#pragma clang diagnostic pop

#ifdef _MSC_VER
#pragma warning(default:4068)
#endif

// Based on subroutine CHKCLM in AERMOD METEXT.F
inline void updateCalm(SurfaceRecord& sr)
{
    sr.calm = std::fpclassify(sr.wspd) == FP_ZERO;
}

// Based on subroutine CHKMSG in AERMOD METEXT.F
inline void updateMissing(SurfaceRecord& sr)
{
    // Wind speed (meters/second)
    if (sr.wspd >= 90.0 || sr.wspd < 0.0)
        sr.missing = true;
    // Wind direction (degrees from north)
    else if (sr.wdir > 900.0 || sr.wdir <= -9.0)
        sr.missing = true;
    // Ambient temperature (kelvins)
    else if (sr.t > 900.0 || sr.t <= 0.0)
        sr.missing = true;
    // Monin-Obukhov length (meters)
    else if (sr.mol < -99990.0)
        sr.missing = true;
    // Convective Mixing height (meters)
    else if (sr.mol < 0.0 && (sr.ziconv > 90000.0 || sr.ziconv < 0.0))
        sr.missing = true;
    // Mechanical Mixing height (meters)
    else if (sr.zimech > 90000.0 || sr.zimech < 0.0)
        sr.missing = true;
    // Surface friction velocity (meters/second)
    else if (sr.ustar < 0.0 || sr.ustar >= 9.0)
        sr.missing = true;
    // Convective velocity scale (meters/second)
    else if (sr.wstar < 0.0 && (sr.mol < 0.0 && sr.mol > -99990.0))
        sr.missing = true;
    else
        sr.missing = false;
}

// Date Parsers

static const qi::uint_parser<unsigned short, 10, 2, 2> yy_;
static const qi::uint_parser<unsigned short, 10, 1, 2> mm_, dd_, hh_;
static const qi::uint_parser<unsigned short, 10, 1, 3> jjj_;

// Symbol Parsers

struct WSAdj : qi::symbols<char, int> {
    WSAdj() { this->add
        ("NAD",          0x01)  // 0000 0001
        ("NAD-OS",       0x11)  // 0001 0001
        ("NAD-A1",       0x21)  // 0010 0001
        ("NAD-SFC",      0x41)  // 0100 0001
        ("ADJ",          0x02)  // 0000 0010
        ("ADJ-OS",       0x12)  // 0001 0010
        ("ADJ-A1",       0x22)  // 0010 0010
        ("ADJ-SFC",      0x42)  // 0100 0010
        ("MMIF-OS",      0x14); // 0001 0100
    }
} static const wsadj_;

struct Subs : qi::symbols<char, int> {
    Subs() { this->add
        ("Sub_CC-TT",    0x05)  // 0000 0101 ITMPSUB, ICCSUB
        ("NoPersC_SubT", 0x06)  // 0000 0110 ITMPSUB, ICNoPers
        ("Sub_TT",       0x04)  // 0000 0100 ITMPSUB
        ("SubC-NoPersT", 0x09)  // 0000 1001 ITNoPers, ICCSUB
        ("NoPers_CC-TT", 0x0A)  // 0000 1010 ITNoPers, ICNoPers
        ("NoPers_TT",    0x08)  // 0000 1000 ITNoPers
        ("Sub_CC",       0x01)  // 0000 0001 ICCSUB
        ("NoPers_CC",    0x02)  // 0000 0010 ICNoPers
        ("NoSubs",       0x10); // 0001 0000
    }
} static const subs_;

template <typename Iterator, typename Skip = ascii::blank_type>
struct SurfaceRecordParser : qi::grammar<Iterator, SurfaceRecord(), Skip>
{
    SurfaceRecordParser() : SurfaceRecordParser::base_type(start)
    {
        using namespace qi;

        start = yy_ > mm_ > dd_ > jjj_ > hh_ >
                double_ > double_ > double_ > double_ > double_ >
                double_ > double_ > double_ > double_ > double_ >
                double_ > double_ > double_ > double_ > double_ >
                -int_ > -double_ > -double_ > -double_ > -int_ >
                -wsadj_ > -subs_;
    }

private:
    qi::rule<Iterator, SurfaceRecord(), Skip> start;
};

template <typename Iterator, typename Skip = ascii::blank_type>
struct UpperAirRecordParser : qi::grammar<Iterator, UpperAirRecord(), Skip>
{
    UpperAirRecordParser() : UpperAirRecordParser::base_type(start)
    {
        using namespace qi;
        start = yy_ > mm_ > dd_ > hh_ > double_ > int_ >
                double_ > double_ > double_ > double_ > double_;
    }

private:
    qi::rule<Iterator, UpperAirRecord(), Skip> start;
};

// Operators

std::istream& operator>>(std::istream& is, SurfaceHeader& header)
{
    std::string line;
    if (std::getline(is, line)) {
        if (line.size() >= 99) {
            header.mplat  = boost::trim_copy(line.substr(2,  8)); // T2,A8
            header.mplon  = boost::trim_copy(line.substr(12, 8)); // T12,A8
            header.ualoc  = boost::trim_copy(line.substr(37, 8)); // T37,A8
            header.sfloc  = boost::trim_copy(line.substr(54, 8)); // T54,A8
            header.osloc  = boost::trim_copy(line.substr(71, 8)); // T71,A8
            header.versno = boost::trim_copy(line.substr(93, 6)); // T93,A6
        }
    }
    return is;
}

std::istream& operator>>(std::istream& is, SurfaceRecord& record)
{
    using iterator_type = std::string::const_iterator;
    static const SurfaceRecordParser<iterator_type> g;

    std::string line;
    if (std::getline(is, line) && !line.empty()) {
        iterator_type it0 = line.begin();
        iterator_type it1 = line.end();
        qi::phrase_parse(it0, it1, g, ascii::blank, record);

        // Use a window of 1950 to 2049 for 2-digit years per AERMOD
        // convention. See v99211 (July 30, 1999) release notes.
        int offset = ((record.mpyr > 49) ? 1900 : 2000);
        record.mpyr += offset;
        updateCalm(record);
        updateMissing(record);
    }
    return is;
}

std::istream& operator>>(std::istream& is, UpperAirRecord& record)
{
    using iterator_type = std::string::const_iterator;
    static const UpperAirRecordParser<iterator_type> g;

    std::string line;
    if (std::getline(is, line) && !line.empty()) {
        iterator_type it0 = line.begin();
        iterator_type it1 = line.end();
        qi::phrase_parse(it0, it1, g, ascii::blank, record);

        // Use a window of 1950 to 2049 for 2-digit years per AERMOD
        // convention. See v99211 (July 30, 1999) release notes.
        int offset = ((record.mpyr > 49) ? 1900 : 2000);
        record.mpyr += offset;
    }
    return is;
}

SurfaceFile::SurfaceFile(const std::filesystem::path& p) : MetFile(p)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;

    std::ifstream ifs(p, std::ios_base::in);

    ifs >> header_;

    while (ifs.good()) {
        SurfaceRecord x;
        try {
            ifs >> x; // may throw qi::expectation_failure<std::string::const_iterator>
            ptime t(date(x.mpyr, x.mpcmo, x.mpcdy), hours(x.j - 1)); // may throw std::out_of_range
            intervals_ += discrete_interval<ptime>::right_open(t, t + hours(1));
        } catch (std::exception& e) {
            std::string msg = fmt::format("SFC file parse error at line {}: {}", nrows_ + 2, e.what());
            throw std::runtime_error(msg);
        }

        nrows_++;
        if (x.calm) ncalm_++;
        if (x.missing) nmissing_++;
        if (ifs.eof())
            break;
    }
}

std::vector<SurfaceRecord> SurfaceFile::records() const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    std::vector<SurfaceRecord> result;

    std::ifstream ifs(path_, std::ios_base::in);
    if (!ifs)
        return result;

    SurfaceHeader h;
    ifs >> h;
    while (ifs.good()) {
        SurfaceRecord x;
        ifs >> x; // may throw qi::expectation_failure<std::string::const_iterator>
        ptime t(date(x.mpyr, x.mpcmo, x.mpcdy), hours(x.j - 1)); // may throw std::out_of_range
        result.push_back(x);
        if (ifs.eof())
            break;
    }

    return result;
}

UpperAirFile::UpperAirFile(const std::filesystem::path& p) : MetFile(p)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace boost::icl;

    std::ifstream ifs(p, std::ios_base::in);

    while (ifs.good()) {
        UpperAirRecord x;
        try {
            ifs >> x; // may throw qi::expectation_failure<std::string::const_iterator>
            ptime t(date(x.mpyr, x.mpcmo, x.mpcdy), hours(x.j - 1)); // may throw std::out_of_range
            intervals_ += discrete_interval<ptime>::right_open(t, t + hours(1));
        } catch (std::exception& e) {
            std::string msg = fmt::format("PFL file parse error at line {}: {}", nrows_ + 1, e.what());
            throw std::runtime_error(msg);
        }

        nrows_++;
        if (ifs.eof())
            break;
    }
}

std::vector<UpperAirRecord> UpperAirFile::records() const
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    std::vector<UpperAirRecord> result;

    std::ifstream ifs(path_, std::ios_base::in);
    if (!ifs)
        return result;

    while (ifs.good()) {
        UpperAirRecord x;
        ifs >> x; // may throw qi::expectation_failure<std::string::const_iterator>
        ptime t(date(x.mpyr, x.mpcmo, x.mpcdy), hours(x.j - 1)); // may throw std::out_of_range
        result.push_back(x);
        if (ifs.eof())
            break;
    }

    return result;
}

Meteorology::Meteorology(const std::filesystem::path& sfc,
                         const std::filesystem::path& pfl)
    : surfaceFile(sfc), upperAirFile(pfl)
{}

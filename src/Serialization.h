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

#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include <QBrush>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QPen>
#include <QString>
#include <QVariant>
#include <QDebug>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#define CEREAL_XML_STRING_VALUE "sofea"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
//#include <cereal/archives/portable_binary.hpp> // Not currently working with flux profiles
#include <cereal/types/array.hpp>
#include <cereal/types/boost_variant.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include <fmt/format.h>

#include "Scenario.h"
#include "SourceGroup.h"

// Current version information
CEREAL_CLASS_VERSION(FluxProfile, 1)
CEREAL_CLASS_VERSION(ReceptorNode, 4)
CEREAL_CLASS_VERSION(ReceptorRing, 3)
CEREAL_CLASS_VERSION(ReceptorGrid, 3)
CEREAL_CLASS_VERSION(ReceptorNodeGroup, 1)
CEREAL_CLASS_VERSION(ReceptorRingGroup, 1)
CEREAL_CLASS_VERSION(ReceptorGridGroup, 1)
CEREAL_CLASS_VERSION(BufferZone, 1)
CEREAL_CLASS_VERSION(AreaSource, 4)
CEREAL_CLASS_VERSION(AreaCircSource, 4)
CEREAL_CLASS_VERSION(AreaPolySource, 4)
CEREAL_CLASS_VERSION(SourceGroup, 7)
CEREAL_CLASS_VERSION(Scenario, 6)


namespace traits {

// Concept check for QDataStream support
template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<T,
    std::void_t<decltype(std::declval<QDataStream&>() << std::declval<T>())>>
    : std::true_type
{};

} // namespace traits


template <typename T>
std::string save_minimal_streamable(const T& var)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream ds(&buffer);
    ds.setVersion(QDataStream::Qt_5_13);
    ds << var;
    QString base64 = buffer.data().toBase64();
    return base64.toStdString();
}

template <typename T>
void load_minimal_streamable(T& var, const std::string& value)
{
    QByteArray base64(value.data(), value.size());
    QByteArray bytes = QByteArray::fromBase64(base64);
    QDataStream ds(bytes);
    ds >> var;
}


namespace cereal {

//-----------------------------------------------------------------------------
// Boost.PointerContainer Types
//-----------------------------------------------------------------------------

// External save function for boost::ptr_vector<T>
template <class Archive, class T>
void save(Archive& ar, const boost::ptr_vector<T>& pv)
{
    ar(pv.size());
    for (const auto& element : pv)
        ar(element);
}

// External load function for boost::ptr_vector<T>
template <class Archive, class T>
void load(Archive& ar, boost::ptr_vector<T>& pv)
{
    std::size_t n;
    ar(n);
    pv.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        pv.push_back(new T);
        ar(pv.back());
    }
}

// External save function for boost::ptr_vector<Source>
// FIXME: workaround for polymorphic class serialization
template <class Archive>
void save(Archive& ar, const boost::ptr_vector<Source>& pv)
{
    ar(pv.size());
    for (const auto& s : pv) {
        SourceType st = s.getType();
        ar(st);
        if (st == SourceType::AREA) {
            auto as = dynamic_cast<const AreaSource*>(&s);
            ar(*as);
        }
        if (st == SourceType::AREACIRC) {
            auto acs = dynamic_cast<const AreaCircSource*>(&s);
            ar(*acs);
        }
        if (st == SourceType::AREAPOLY) {
            auto aps = dynamic_cast<const AreaPolySource*>(&s);
            ar(*aps);
        }
    }
}

// External load function for boost::ptr_vector<Source>
// FIXME: workaround for polymorphic class serialization
template <class Archive>
void load(Archive& ar, boost::ptr_vector<Source>& pv)
{
    std::size_t n;
    ar(n);
    pv.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        SourceType st;
        ar(st);
        if (st == SourceType::AREA) {
            AreaSource *s = new AreaSource;
            ar(*s);
            pv.push_back(s);
        }
        if (st == SourceType::AREACIRC) {
            AreaCircSource *s = new AreaCircSource;
            ar(*s);
            pv.push_back(s);
        }
        if (st == SourceType::AREAPOLY) {
            AreaPolySource *s = new AreaPolySource;
            ar(*s);
            pv.push_back(s);
        }
    }
}

//-----------------------------------------------------------------------------
// Qt Types
//-----------------------------------------------------------------------------

// External save function for QDateTime
template <class Archive>
std::string save_minimal(Archive const&, QDateTime const& dt)
{
    QString qs = dt.toString(Qt::ISODate);
    return qs.toStdString();
}

// External load function for QDateTime
template <class Archive>
void load_minimal(Archive const&, QDateTime& dt, std::string const& value)
{
    QString qs = QString::fromStdString(value);
    dt = QDateTime::fromString(qs, Qt::ISODate);
    dt.setTimeSpec(Qt::UTC);
}

// External save function for QDate
template <class Archive>
std::string save_minimal(const Archive&, const QDate& d)
{
    QString qs = d.toString(Qt::ISODate);
    return qs.toStdString();
}

// External load function for QDate
template <class Archive>
void load_minimal(const Archive&, QDate& d, const std::string& value)
{
    QString qs = QString::fromStdString(value);
    d = QDate::fromString(qs, Qt::ISODate);
}

// External save function for QPolygonF
template <class Archive>
void save(Archive& ar, const QPolygonF& p)
{
    ar(p.size());
    for (const QPointF& pt : p)
        ar(std::make_pair(pt.x(), pt.y()));
}

// External load function for QPolygonF
template <class Archive>
void load(Archive& ar, QPolygonF& p)
{
    int n;
    ar(n);
    p.reserve(n);
    for (int i = 0; i < n; ++i) {
        std::pair<double, double> pt;
        ar(pt);
        p.push_back(QPointF(pt.first, pt.second));
    }
}

// External save function for QColor
template <class Archive>
void save(Archive& ar, const QColor& color)
{
    quint64 rgba = color.rgba64();
    ar(rgba);
}

// External load function for QColor
template <class Archive>
void load(Archive& ar, QColor& color)
{
    quint64 rgba;
    ar(rgba);
    color = QColor::fromRgba64(qRgba64(rgba));
}

// External save function for QPen
template <class Archive>
std::string save_minimal(const Archive&, const QPen& pen) {
    return ::save_minimal_streamable(pen);
}

// External load function for QPen
template <class Archive>
void load_minimal(const Archive&, QPen& pen, const std::string& value) {
    ::load_minimal_streamable(pen, value);
}

// External save function for QBrush
template <class Archive>
std::string save_minimal(const Archive&, const QBrush& brush) {
    return ::save_minimal_streamable(brush);
}

// External load function for QBrush
template <class Archive>
void load_minimal(const Archive&, QBrush& brush, const std::string& value) {
    ::load_minimal_streamable(brush, value);
}

//-----------------------------------------------------------------------------
// Boost.DateTime Types
//-----------------------------------------------------------------------------

// External save function for boost::gregorian::date
template<class Archive>
void save(Archive& ar, const boost::gregorian::date& d)
{
    std::string ds = boost::gregorian::to_iso_string(d);
    ar(cereal::make_nvp("date", ds));
}

// External load function for boost::gregorian::date
template<class Archive>
void load(Archive& ar, boost::gregorian::date& d)
{
    std::string ds;
    ar(cereal::make_nvp("date", ds));
    try {
        d = boost::gregorian::from_undelimited_string(ds);
    }
    catch(boost::bad_lexical_cast&) {
        boost::gregorian::special_values sv = boost::gregorian::special_value_from_string(ds);
        if (sv == boost::gregorian::not_special) {
            throw; // no match found, rethrow original exception
        }
        else {
            d = boost::gregorian::date(sv);
        }
    }
}

// External save functions for boost::posix_time::time_duration
template <class TimeResTraitsSize, class Archive>
void save_td(Archive& ar, const boost::posix_time::time_duration& td)
{
    TimeResTraitsSize h = boost::numeric_cast<TimeResTraitsSize>(td.hours());
    TimeResTraitsSize m = boost::numeric_cast<TimeResTraitsSize>(td.minutes());
    TimeResTraitsSize s = boost::numeric_cast<TimeResTraitsSize>(td.seconds());
    boost::posix_time::time_duration::fractional_seconds_type fs = td.fractional_seconds();
    ar(cereal::make_nvp("time_duration_hours", h));
    ar(cereal::make_nvp("time_duration_minutes", m));
    ar(cereal::make_nvp("time_duration_seconds", s));
    ar(cereal::make_nvp("time_duration_fractional_seconds", fs));
}

template <class Archive>
void save(Archive& ar, const boost::posix_time::time_duration& td)
{
    // serialize a bool so we know how to read this back in later
    bool is_special = td.is_special();
    ar(cereal::make_nvp("is_special", is_special));
    if (is_special) {
        std::string s = to_simple_string(td);
        ar(cereal::make_nvp("sv_time_duration", s));
    }
    else {
        save_td<int64_t>(ar, td);
    }
}

// External load functions for boost::posix_time::time_duration
template <class TimeResTraitsSize, class Archive>
void load_td(Archive& ar, boost::posix_time::time_duration& td)
{
    TimeResTraitsSize h(0);
    TimeResTraitsSize m(0);
    TimeResTraitsSize s(0);
    boost::posix_time::time_duration::fractional_seconds_type fs(0);
    ar(cereal::make_nvp("time_duration_hours", h));
    ar(cereal::make_nvp("time_duration_minutes", m));
    ar(cereal::make_nvp("time_duration_seconds", s));
    ar(cereal::make_nvp("time_duration_fractional_seconds", fs));
    td = boost::posix_time::time_duration(h, m, s, fs);
}

template <class Archive>
void load(Archive& ar, boost::posix_time::time_duration& td)
{
    bool is_special = false;
    ar(cereal::make_nvp("is_special", is_special));
    if (is_special) {
        std::string s;
        ar & make_nvp("sv_time_duration", s);
        boost::posix_time::special_values sv = boost::gregorian::special_value_from_string(s);
        td = boost::posix_time::time_duration(sv);
    }
    else {
        BOOST_STATIC_ASSERT(sizeof(boost::posix_time::time_duration::hour_type) == sizeof(boost::int64_t));
        BOOST_STATIC_ASSERT(sizeof(boost::posix_time::time_duration::min_type) == sizeof(boost::int64_t));
        BOOST_STATIC_ASSERT(sizeof(boost::posix_time::time_duration::sec_type) == sizeof(boost::int64_t));
        BOOST_STATIC_ASSERT(sizeof(boost::posix_time::time_duration::fractional_seconds_type) == sizeof(boost::int64_t));
        load_td<int64_t>(ar, td);
    }
}

// External save function for boost::posix_time::ptime
template <class Archive>
void save(Archive& ar, const boost::posix_time::ptime& pt)
{
    // from_iso_string does not include fractional seconds
    // therefore date and time_duration are used
    boost::posix_time::ptime::date_type d = pt.date();
    ar(cereal::make_nvp("ptime_date", d));
    if (!pt.is_special()) {
        boost::posix_time::ptime::time_duration_type td = pt.time_of_day();
        ar(cereal::make_nvp("ptime_time_duration", td));
    }
}

// External load function for boost::posix_time::ptime
template <class Archive>
void load(Archive& ar, boost::posix_time::ptime& pt)
{
    // from_iso_string does not include fractional seconds
    // therefore date and time_duration are used
    boost::posix_time::ptime::date_type d(boost::posix_time::not_a_date_time);
    boost::posix_time::ptime::time_duration_type td;
    ar(cereal::make_nvp("ptime_date", d));
    if (!d.is_special()) {
        ar(cereal::make_nvp("ptime_time_duration", td));
        pt = boost::posix_time::ptime(d, td);
    }
    else {
        pt = boost::posix_time::ptime(d.as_special());
    }
}

//-----------------------------------------------------------------------------
// Boost.uBLAS Types
//-----------------------------------------------------------------------------

// External save function for boost::numeric::ublas::unbounded_array
template <class Archive, class T, class ALLOC>
void save(Archive& ar, const boost::numeric::ublas::unbounded_array<T, ALLOC>& ua)
{
    ar(cereal::make_nvp("size", ua.size()));
    for (const auto& element : ua)
        ar(element);
}

// External load function for boost::numeric::ublas::unbounded_array
template <class Archive, class T, class ALLOC>
void load(Archive& ar, boost::numeric::ublas::unbounded_array<T, ALLOC>& ua)
{
    typename ALLOC::size_type s(ua.size());
    ar(cereal::make_nvp("size", s));
    ua.resize(s);
    for (auto&& element : ua)
        ar(element);
}

// External save function for boost::numeric::ublas::compressed_matrix
template <class Archive, class T, class L, std::size_t IB, class IA, class TA>
void save(Archive& ar, const boost::numeric::ublas::compressed_matrix<T, L, IB, IA, TA>& m)
{
    ar(cereal::make_nvp("size1", m.size1()));
    ar(cereal::make_nvp("size2", m.size2()));
    ar(cereal::make_nvp("capacity", m.nnz_capacity()));
    ar(cereal::make_nvp("filled1", m.filled1()));
    ar(cereal::make_nvp("filled2", m.filled2()));
    ar(cereal::make_nvp("index1_data", m.index1_data()));
    ar(cereal::make_nvp("index2_data", m.index2_data()));
    ar(cereal::make_nvp("value_data", m.value_data()));
}

// External load function for boost::numeric::ublas::compressed_matrix
template <class Archive, class T, class L, std::size_t IB, class IA, class TA>
void load(Archive& ar, boost::numeric::ublas::compressed_matrix<T, L, IB, IA, TA>& m)
{
    typename IA::value_type s1(m.size1()), s2(m.size2());
    typename IA::size_type filled1, filled2;
    auto& index1_data = m.index1_data();
    auto& index2_data = m.index2_data();
    auto& value_data = m.value_data();

    ar(cereal::make_nvp("size1", s1));
    ar(cereal::make_nvp("size2", s2));
    m.resize(s1, s2, false);

    ar(cereal::make_nvp("filled1", filled1));
    ar(cereal::make_nvp("filled2", filled2));
    ar(cereal::make_nvp("index1_data", index1_data));
    ar(cereal::make_nvp("index2_data", index2_data));
    ar(cereal::make_nvp("value_data", value_data));
    m.set_filled(filled1, filled2);
}

// Resolve ambiguities with Boost Serialization functions in uBLAS.
template <class Archive, class T, class ALLOC>
struct specialize<
    Archive,
    boost::numeric::ublas::unbounded_array<T, ALLOC>,
    cereal::specialization::non_member_load_save>
{};

template <class Archive, class T, class L, std::size_t IB, class IA, class TA>
struct specialize<
    Archive,
    boost::numeric::ublas::compressed_matrix<T, L, IB, IA, TA>,
    cereal::specialization::non_member_load_save>
{};

} // namespace cereal

//-----------------------------------------------------------------------------
// Boost.ICL Types
//-----------------------------------------------------------------------------

// External save function for boost::icl::discrete_interval<DomainT>
template <class Archive, class DomainT>
void save(Archive& ar, boost::icl::discrete_interval<DomainT> const& di)
{
    auto const& bb = di.bounds().bits();
    auto const& l  = di.lower();
    auto const& u  = di.upper();
    ar(bb, l, u);
}

// External load function for boost::icl::discrete_interval<DomainT>
template <class Archive, class DomainT>
void load(Archive& ar, boost::icl::discrete_interval<DomainT>& di)
{
    auto bb = di.bounds().bits();
    DomainT l, u;
    ar(bb, l, u);
    di = boost::icl::discrete_interval<DomainT>(l, u, boost::icl::interval_bounds(bb));
}

// External save function for boost::icl::interval_map<DomainT, CodomainT>
template <class Archive>
void save(Archive& ar, IntervalMap const& im)
{
    auto sz = im.iterative_size();
    ar(sz);
    for (auto& value : im) {
        ar(value.first(), value.second());
    }
}

// Enternal load function for boost::icl::interval_map<DomainT, CodomainT>
template <class Archive>
void load(Archive& ar, IntervalMap& im)
{
    im.clear();
    size_t sz;
    ar(sz);
    size_t counter = sz;
    while (counter--) {
        typename IntervalMap::value_type value;
        ar(value);
        im.insert(im.end(), value);
    }
}

//-----------------------------------------------------------------------------
// SOFEA Classes
//-----------------------------------------------------------------------------

// External serialize function for FluxProfile
template <class Archive>
void serialize(Archive& archive, FluxProfile& fp, const std::uint32_t version)
{
    // TODO: ADD fp.constantFlux

    if (version >= 1) {
        archive(cereal::make_nvp("name", fp.name),
                cereal::make_nvp("ts_method", fp.tsMethod),
                cereal::make_nvp("ds_method", fp.dsMethod),
                cereal::make_nvp("ref_flux", fp.refFlux),
                cereal::make_nvp("ref_app_rate", fp.refAppRate),
                cereal::make_nvp("ref_start", fp.refStart),
                cereal::make_nvp("ref_depth", fp.refDepth),
                cereal::make_nvp("ref_vol_loss", fp.refVL),
                cereal::make_nvp("max_vol_loss", fp.maxVL),
                cereal::make_nvp("warm_season_start", fp.warmSeasonStart),
                cereal::make_nvp("warm_season_end", fp.warmSeasonEnd),
                cereal::make_nvp("warm_season_sf", fp.warmSeasonSF),
                cereal::make_nvp("amplitude", fp.amplitude),
                cereal::make_nvp("center_amplitude", fp.centerAmplitude),
                cereal::make_nvp("phase", fp.phase),
                cereal::make_nvp("wavelength", fp.wavelength));
    }
}

// External serialize function for ReceptorNode
template <class Archive>
void serialize(Archive& archive, ReceptorNode& rn, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V3:  adds support for elevation and color
    // - V4:  adds support for zFlag; color support moved to receptor groups

    if (version >= 1) {
        archive(cereal::make_nvp("x", rn.x),
                cereal::make_nvp("y", rn.y));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("zelev", rn.zElev),
                cereal::make_nvp("zhill", rn.zHill));
    }
    if (version >= 4){
        archive(cereal::make_nvp("zflag", rn.zFlag));
    }
}

// External serialize function for ReceptorRing
// *** Depreciated, For Serialization Only ***
template <class Archive>
void serialize(Archive& archive, ReceptorRing& rr, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V3:  adds support for color

    if (version >= 1) {
        archive(cereal::make_nvp("arc_id", rr.arcid),
                cereal::make_nvp("buffer", rr.buffer),
                cereal::make_nvp("spacing", rr.spacing),
                cereal::make_nvp("zelev", rr.zElev),
                cereal::make_nvp("zhill", rr.zHill),
                cereal::make_nvp("points", rr.points));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("color", rr.color));
    }
}


// External serialize function for ReceptorGrid
// *** Depreciated, For Serialization Only ***
template <class Archive>
void serialize(Archive& archive, ReceptorGrid& rg, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V3:  adds support for elevation and color

    if (version >= 1) {
        archive(cereal::make_nvp("net_id", rg.netid),
                cereal::make_nvp("xinit", rg.xInit),
                cereal::make_nvp("yinit", rg.yInit),
                cereal::make_nvp("xdelta", rg.xDelta),
                cereal::make_nvp("ydelta", rg.yDelta),
                cereal::make_nvp("xcount", rg.xCount),
                cereal::make_nvp("ycount", rg.yCount));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("zelev", rg.zElev),
                cereal::make_nvp("zhill", rg.zHill),
                cereal::make_nvp("color", rg.color));
    }
}

// External serialize function for ReceptorNodeGroup
template <class Archive>
void serialize(Archive& archive, ReceptorNodeGroup& rg, const std::uint32_t version)
{
    // VERSION HISTORY:
    // -

    if (version >= 1) {
        archive(cereal::make_nvp("grpid", rg.grpid),
                cereal::make_nvp("color", rg.color),
                cereal::make_nvp("nodes", rg.nodes));
    }
}

// External serialize function for ReceptorRingGroup
template <class Archive>
void serialize(Archive& archive, ReceptorRingGroup& rg, const std::uint32_t version)
{
    // VERSION HISTORY:
    // -

    if (version >= 1) {
        archive(cereal::make_nvp("grpid", rg.grpid),
                cereal::make_nvp("color", rg.color),
                cereal::make_nvp("sgptr", rg.sgPtr),
                cereal::make_nvp("buffer", rg.buffer),
                cereal::make_nvp("spacing", rg.spacing),
                cereal::make_nvp("polygons", rg.polygons),
                cereal::make_nvp("nodes", rg.nodes));
    }
}

// External serialize function for ReceptorGridGroup
template <class Archive>
void serialize(Archive& archive, ReceptorGridGroup& rg, const std::uint32_t version)
{
    // VERSION HISTORY:
    // -

    if (version >= 1) {
        archive(cereal::make_nvp("grpid", rg.grpid),
                cereal::make_nvp("color", rg.color),
                cereal::make_nvp("xinit", rg.xInit),
                cereal::make_nvp("yinit", rg.yInit),
                cereal::make_nvp("xcount", rg.xCount),
                cereal::make_nvp("ycount", rg.yCount),
                cereal::make_nvp("xdelta", rg.xDelta),
                cereal::make_nvp("ydelta", rg.yDelta),
                cereal::make_nvp("zelevm", rg.zElevM),
                cereal::make_nvp("zhillm", rg.zHillM),
                cereal::make_nvp("zflagm", rg.zFlagM));
    }
}

// External serialize function for BufferZone
template <class Archive>
void serialize(Archive& archive, BufferZone& z, const std::uint32_t version)
{
    if (version >= 1) {
        archive(cereal::make_nvp("area_threshold", z.areaThreshold),
                cereal::make_nvp("app_rate_threshold", z.appRateThreshold),
                cereal::make_nvp("distance", z.distance),
                cereal::make_nvp("duration", z.duration));
    }
}

// External serialize function for AreaSource
template <class Archive>
void serialize(Archive& archive, AreaSource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition
    // - V3:  adds support for flux profile
    // - V4:  adds support for pen/brush

    if (version >= 1) {
        archive(cereal::make_nvp("src_id", s.srcid),
                cereal::make_nvp("xs", s.xs),
                cereal::make_nvp("ys", s.ys),
                cereal::make_nvp("zs", s.zs),
                cereal::make_nvp("app_start", s.appStart),
                cereal::make_nvp("app_rate", s.appRate),
                cereal::make_nvp("incorp_depth", s.incorpDepth),
                cereal::make_nvp("geometry", s.geometry),
                cereal::make_nvp("aremis", s.aremis),
                cereal::make_nvp("relhgt", s.relhgt),
                cereal::make_nvp("xinit", s.xinit),
                cereal::make_nvp("yinit", s.yinit),
                cereal::make_nvp("angle", s.angle),
                cereal::make_nvp("szinit", s.szinit));
    }
    if (version >= 2) {
        archive(cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("flux_profile", s.fluxProfile));
    }
    if (version >= 4) {
        archive(cereal::make_nvp("pen", s.pen),
                cereal::make_nvp("brush", s.brush));
    }
}

// External serialize function for AreaCircSource
template <class Archive>
void serialize(Archive& archive, AreaCircSource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition
    // - V3:  adds support for flux profile
    // - V4:  adds support for pen/brush

    if (version >= 1) {
        archive(cereal::make_nvp("src_id", s.srcid),
                cereal::make_nvp("xs", s.xs),
                cereal::make_nvp("ys", s.ys),
                cereal::make_nvp("zs", s.zs),
                cereal::make_nvp("app_start", s.appStart),
                cereal::make_nvp("app_rate", s.appRate),
                cereal::make_nvp("incorp_depth", s.incorpDepth),
                cereal::make_nvp("geometry", s.geometry),
                cereal::make_nvp("aremis", s.aremis),
                cereal::make_nvp("relhgt", s.relhgt),
                cereal::make_nvp("radius", s.radius),
                cereal::make_nvp("nverts", s.nverts),
                cereal::make_nvp("szinit", s.szinit));
    }
    if (version >= 2) {
        archive(cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("flux_profile", s.fluxProfile));
    }
    if (version >= 4) {
        archive(cereal::make_nvp("pen", s.pen),
                cereal::make_nvp("brush", s.brush));
    }
}

// External serialize function for AreaPolySource
template <class Archive>
void serialize(Archive& archive, AreaPolySource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition
    // - V3:  adds support for flux profile
    // - V4:  adds support for pen/brush

    if (version >= 1) {
        archive(cereal::make_nvp("src_id", s.srcid),
                cereal::make_nvp("xs", s.xs),
                cereal::make_nvp("ys", s.ys),
                cereal::make_nvp("zs", s.zs),
                cereal::make_nvp("app_start", s.appStart),
                cereal::make_nvp("app_rate", s.appRate),
                cereal::make_nvp("incorp_depth", s.incorpDepth),
                cereal::make_nvp("geometry", s.geometry),
                cereal::make_nvp("aremis", s.aremis),
                cereal::make_nvp("relhgt", s.relhgt),
                cereal::make_nvp("szinit", s.szinit));
    }
    if (version >= 2) {
        archive(cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("flux_profile", s.fluxProfile));
    }
    if (version >= 4) {
        archive(cereal::make_nvp("pen", s.pen),
                cereal::make_nvp("brush", s.brush));
    }
}

// External serialize function for SourceGroup
template <class Archive>
void serialize(Archive& archive, SourceGroup& sg, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for discrete receptors
    // - V3:  adds support for deposition
    // - V4:  adds support for app. start distribution
    // - V5:  adds support for flux profile distribution
    // - V6:  new dynamic buffer zone implementation
    // - V7:  receptor nodes and grids moved to scenario

    if (version >= 1) {
        archive(cereal::make_nvp("group_id", sg.grpid),
                cereal::make_nvp("app_method", sg.appMethod),
                cereal::make_nvp("app_factor", sg.appFactor),
                cereal::make_nvp("validation_mode", sg.validationMode));
    }
    if (version >= 4) {
        archive(cereal::make_nvp("dist_app_start", sg.appStart));
    }
    if (version >= 1) {
        archive(cereal::make_nvp("dist_app_rate", sg.appRate),
                cereal::make_nvp("dist_inc_depth", sg.incorpDepth));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("air_diffusion", sg.airDiffusion),
                cereal::make_nvp("water_diffusion", sg.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", sg.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", sg.henryConstant));
    }
    if (version >= 5) {
        archive(cereal::make_nvp("flux_profile", sg.fluxProfile));
    }
    if (version >= 6) {
        archive(cereal::make_nvp("enable_buffer_zones", sg.enableBufferZones),
                cereal::make_nvp("buffer_zones", sg.zones));
    }
    if (version >= 1) {
        archive(cereal::make_nvp("sources", sg.sources));
    }
    if (version >= 1 && version < 7) {
        archive(cereal::make_nvp("receptor_rings", sg.rings),
                cereal::make_nvp("receptor_grids", sg.grids)); // for compatibility only
    }
    if (version >= 2 && version < 7) {
        archive(cereal::make_nvp("receptor_nodes", sg.nodes)); // for compatibility only
    }
}

// External serialize function for Scenario
template <class Archive>
void serialize(Archive& archive, Scenario& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for flagpole height
    // - V3:  adds support for deposition, averaging periods
    // - V4:  adds support for flux profiles
    // - V5:  receptor groups replace previous receptor classes
    //        boost::ptr_vector<SourceGroup> replaced with std::vector<std::shared_ptr<SourceGroup>>
    // - V6:  remove flagpole height from scenario, assign directly to receptors
    //        adds support for projections

    if (version >= 1) {
        archive(cereal::make_nvp("title", s.name),
                cereal::make_nvp("fumigant_id", s.fumigantId),
                cereal::make_nvp("decay_coefficient", s.decayCoefficient));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("averaging_periods", s.averagingPeriods));
    }
    if (version >= 4) {
        archive(cereal::make_nvp("flux_profiles", s.fluxProfiles));
    }
    if (version >= 1) {
        archive(cereal::make_nvp("aermet_sf_file", s.surfaceFile),
                cereal::make_nvp("aermet_ua_file", s.upperAirFile),
                cereal::make_nvp("aermet_sf_id", s.surfaceId),
                cereal::make_nvp("aermet_ua_id", s.upperAirId),
                cereal::make_nvp("min_time", s.minTime),
                cereal::make_nvp("max_time", s.maxTime),
                cereal::make_nvp("anemometer_height", s.anemometerHeight),
                cereal::make_nvp("wind_rotation", s.windRotation),
                cereal::make_nvp("aermod_flat", s.aermodFlat),
                cereal::make_nvp("aermod_fast_area", s.aermodFastArea));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("aermod_dry_deposition", s.aermodDryDeposition),
                cereal::make_nvp("aermod_dry_depletion", s.aermodDryDplt),
                cereal::make_nvp("aermod_area_depletion", s.aermodAreaDplt),
                cereal::make_nvp("aermod_gd_user_velocity_enabled", s.aermodGDVelocityEnabled),
                cereal::make_nvp("aermod_gd_user_velocity", s.aermodGDVelocity),
                cereal::make_nvp("aermod_gd_reactivity", s.aermodGDReact),
                cereal::make_nvp("aermod_gd_season_f", s.aermodGDSeasonF),
                cereal::make_nvp("aermod_wet_deposition", s.aermodWetDeposition),
                cereal::make_nvp("aermod_wet_depletion", s.aermodWetDplt));
    }
    if (version >= 1) {
        archive(cereal::make_nvp("aermod_low_wind", s.aermodLowWind),
                cereal::make_nvp("aermod_low_wind_svmin", s.aermodSVmin),
                cereal::make_nvp("aermod_low_wind_wsmin", s.aermodWSmin),
                cereal::make_nvp("aermod_low_wind_franmax", s.aermodFRANmax));

    }

    if (version >= 5) {
        archive(cereal::make_nvp("source_groups", s.sourceGroups));
    }
    else {
        // Move source groups from boost::ptr_vector when loading earlier class versions.
        if constexpr (std::is_base_of_v<cereal::detail::InputArchiveBase, Archive>)
        {
            boost::ptr_vector<SourceGroup> pv;
            archive(cereal::make_nvp("source_groups", pv));
            s.sourceGroups.clear();
            s.sourceGroups.reserve(pv.size());
            for (auto&& sg : pv) {
                s.sourceGroups.emplace_back(std::make_shared<SourceGroup>(std::move(sg)));
            }
        }
    }

    if (version >= 5) {
        archive(cereal::make_nvp("receptor_groups", s.receptors));
    }
    else {
        // Move receptors from source group and convert to new group format.
        if constexpr (std::is_base_of_v<cereal::detail::InputArchiveBase, Archive>)
        {
            //double zflag = 0;
            //if (version >= 2) {
            //    archive(cereal::make_nvp("flagpole_height", zflag));
            //}

            int isg = 1;
            for (auto sgptr : s.sourceGroups) {
                if (!sgptr->nodes.empty()) {
                    ReceptorNodeGroup ng;
                    ng.grpid = fmt::format("G{:0=3}DISC", isg);
                    for (const auto& node : sgptr->nodes) {
                        ng.nodes.insert(node);
                    }
                    s.receptors.push_back(ng);
                    sgptr->nodes.clear();
                }

                int iring = 1;
                for (const auto& ring : sgptr->rings) {
                    ReceptorRingGroup rg;
                    rg.grpid = fmt::format("G{:0=3}R{:0=3}", isg, iring++);
                    rg.setSourceGroup(sgptr);
                    rg.setBuffer(ring.buffer);
                    rg.setSpacing(ring.spacing);
                    rg.updateGeometry();
                    s.receptors.push_back(rg);
                }
                sgptr->rings.clear();

                int igrid = 1;
                for (const auto& grid : sgptr->grids) {
                    ReceptorGridGroup gg;
                    gg.grpid = fmt::format("G{:0=3}C{:0=3}", isg, igrid++);
                    gg.setDimensions(grid.xCount, grid.yCount);
                    gg.setOrigin(grid.xInit, grid.yInit);
                    gg.setSpacing(grid.xDelta, grid.yDelta);
                    s.receptors.push_back(gg);
                }
                sgptr->rings.clear();

                isg++;
            }
        }
    }

    if (version >= 6) {
        archive(cereal::make_nvp("conversion_code", s.conversionCode),
                cereal::make_nvp("horizontal_units_code", s.hUnitsCode),
                cereal::make_nvp("horizontal_datum_code", s.hDatumCode),
                cereal::make_nvp("vertical_units_code", s.vUnitsCode),
                cereal::make_nvp("vertical_datum_code", s.vDatumCode));
    }
}


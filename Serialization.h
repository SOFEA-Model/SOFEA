#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QDebug>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_set.hpp>
#include "boost/numeric/conversion/cast.hpp"
#include <boost/ptr_container/ptr_vector.hpp>

#define CEREAL_XML_STRING_VALUE "sofea"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/boost_variant.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Scenario.h"
#include "SourceGroup.h"

// Current version information
CEREAL_CLASS_VERSION(FluxProfile, 1)
CEREAL_CLASS_VERSION(ReceptorRing, 3)
CEREAL_CLASS_VERSION(ReceptorNode, 3)
CEREAL_CLASS_VERSION(ReceptorGrid, 3)
CEREAL_CLASS_VERSION(AreaSource, 2)
CEREAL_CLASS_VERSION(AreaCircSource, 2)
CEREAL_CLASS_VERSION(AreaPolySource, 2)
CEREAL_CLASS_VERSION(SourceGroup, 4)
CEREAL_CLASS_VERSION(Scenario, 3)

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
std::string save_minimal(Archive const&, QDate const& d)
{
    QString qs = d.toString(Qt::ISODate);
    return qs.toStdString();
}

// External load function for QDate
template <class Archive>
void load_minimal(Archive const&, QDate& d, std::string const& value)
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
void save_td(Archive& ar, boost::posix_time::time_duration const& td)
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
void save(Archive& ar, boost::posix_time::time_duration const& td)
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
void save(Archive& ar, boost::posix_time::ptime const& pt)
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
// Boost.ICL Types
//-----------------------------------------------------------------------------
/*
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
*/

} // namespace cereal

//-----------------------------------------------------------------------------
// SOFEA Classes
//-----------------------------------------------------------------------------

// External serialize function for FluxProfile
template <class Archive>
void serialize(Archive& archive, FluxProfile& fp, const std::uint32_t version)
{
    if (version >= 1) {
        archive(cereal::make_nvp("ts_method", fp.tsMethod),
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

// External serialize function for ReceptorRing
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

// External serialize function for ReceptorNode
template <class Archive>
void serialize(Archive& archive, ReceptorNode& rn, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V3:  adds support for elevation and color

    if (version >= 1) {
        archive(cereal::make_nvp("x", rn.x),
                cereal::make_nvp("y", rn.y));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("zelev", rn.zElev),
                cereal::make_nvp("zhill", rn.zHill),
                cereal::make_nvp("color", rn.color));
    }
}

// External serialize function for ReceptorGrid
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
                cereal::make_nvp("ycount", rg.yCount),
                cereal::make_nvp("points", rg.points));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("zelev", rg.zElev),
                cereal::make_nvp("zhill", rg.zHill),
                cereal::make_nvp("color", rg.color));
    }
}

// External serialize function for AreaSource
template <class Archive>
void serialize(Archive& archive, AreaSource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition and xshift, yshift

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
        archive(cereal::make_nvp("xshift", s.xshift),
                cereal::make_nvp("yshift", s.yshift),
                cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
    }
}

// External serialize function for AreaCircSource
template <class Archive>
void serialize(Archive& archive, AreaCircSource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition and xshift, yshift

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
        archive(cereal::make_nvp("xshift", s.xshift),
                cereal::make_nvp("yshift", s.yshift),
                cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
    }
}

// External serialize function for AreaPolySource
template <class Archive>
void serialize(Archive& archive, AreaPolySource& s, const std::uint32_t version)
{
    // VERSION HISTORY:
    // - V2:  adds support for deposition and xshift, yshift

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
        archive(cereal::make_nvp("xshift", s.xshift),
                cereal::make_nvp("yshift", s.yshift),
                cereal::make_nvp("air_diffusion", s.airDiffusion),
                cereal::make_nvp("water_diffusion", s.waterDiffusion),
                cereal::make_nvp("cuticular_resistance", s.cuticularResistance),
                cereal::make_nvp("henrys_law_constant", s.henryConstant));
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
    if (version >= 1) {
        archive(cereal::make_nvp("sources", sg.sources),
                cereal::make_nvp("buffer_zones", sg.zones),
                cereal::make_nvp("receptor_rings", sg.rings),
                cereal::make_nvp("receptor_grids", sg.grids));
    }
    if (version >= 2) {
        archive(cereal::make_nvp("receptor_nodes", sg.nodes));
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

    if (version >= 1) {
        archive(cereal::make_nvp("title", s.title),
                cereal::make_nvp("fumigant_id", s.fumigantId),
                cereal::make_nvp("decay_coefficient", s.decayCoefficient));
    }
    if (version >= 2) {
        archive(cereal::make_nvp("flagpole_height", s.flagpoleHeight));
    }
    if (version >= 3) {
        archive(cereal::make_nvp("averaging_periods", s.averagingPeriods));
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
                cereal::make_nvp("aermod_low_wind_franmax", s.aermodFRANmax),
                cereal::make_nvp("source_groups", s.sourceGroups));
    }
}


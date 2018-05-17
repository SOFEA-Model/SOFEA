#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QDebug>

#include <boost/ptr_container/ptr_vector.hpp>

#define CEREAL_XML_STRING_VALUE "sofea"
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/boost_variant.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Scenario.h"
#include "SourceGroup.h"

// Version information
CEREAL_CLASS_VERSION(FluxScaling, 1);
CEREAL_CLASS_VERSION(ReceptorRing, 1);
CEREAL_CLASS_VERSION(ReceptorNode, 1);
CEREAL_CLASS_VERSION(ReceptorGrid, 1);
CEREAL_CLASS_VERSION(AreaSource, 1);
CEREAL_CLASS_VERSION(AreaCircSource, 1);
CEREAL_CLASS_VERSION(AreaPolySource, 1);
CEREAL_CLASS_VERSION(SourceGroup, 2);
CEREAL_CLASS_VERSION(Scenario, 1);

// External save function for boost::ptr_vector<T>
template<class Archive, class T>
void save(Archive& ar, const boost::ptr_vector<T>& pv)
{
    ar(pv.size());
    for (const auto& element : pv)
        ar(element);
}

// External load function for boost::ptr_vector<T>
template<class Archive, class T>
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
template<class Archive>
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
template<class Archive>
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

// External save function for QDateTime
template<class Archive>
std::string save_minimal(Archive const&, QDateTime const& dt)
{
    QString qs = dt.toString(Qt::ISODate);
    return qs.toStdString();
}

// External load function for QDateTime
template<class Archive>
void load_minimal(Archive const&, QDateTime& dt, std::string const& value)
{
    QString qs = QString::fromStdString(value);
    dt = QDateTime::fromString(qs, Qt::ISODate);
}

// External save function for QDate
template<class Archive>
std::string save_minimal(Archive const&, QDate const& d)
{
    QString qs = d.toString(Qt::ISODate);
    return qs.toStdString();
}

// External load function for QDate
template<class Archive>
void load_minimal(Archive const&, QDate& d, std::string const& value)
{
    QString qs = QString::fromStdString(value);
    d = QDate::fromString(qs, Qt::ISODate);
}

// External save function for QPolygonF
template<class Archive>
void save(Archive& ar, const QPolygonF& p)
{
    ar(p.size());
    for (const QPointF& pt : p)
        ar(std::make_pair(pt.x(), pt.y()));
}

// External load function for QPolygonF
template<class Archive>
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

/****************************************************************************
** Main Classes
****************************************************************************/

// External serialize function for FluxScaling
template<class Archive>
void serialize(Archive& archive, FluxScaling& fs, const std::uint32_t version)
{
    archive(cereal::make_nvp("ts_method", fs.tsMethod),
            cereal::make_nvp("ds_method", fs.dsMethod),
            cereal::make_nvp("ref_app_rate", fs.refAppRate),
            cereal::make_nvp("ref_start", fs.refStart),
            cereal::make_nvp("ref_depth", fs.refDepth),
            cereal::make_nvp("ref_vol_loss", fs.refVL),
            cereal::make_nvp("max_vol_loss", fs.maxVL),
            cereal::make_nvp("warm_season_start", fs.warmSeasonStart),
            cereal::make_nvp("warm_season_end", fs.warmSeasonEnd),
            cereal::make_nvp("warm_season_sf", fs.warmSeasonSF),
            cereal::make_nvp("amplitude", fs.amplitude),
            cereal::make_nvp("center_amplitude", fs.centerAmplitude),
            cereal::make_nvp("phase", fs.phase),
            cereal::make_nvp("wavelength", fs.wavelength));
}

// External serialize function for ReceptorRing
template<class Archive>
void serialize(Archive& archive, ReceptorRing& rr, const std::uint32_t version)
{
    archive(cereal::make_nvp("arc_id", rr.arcid),
            cereal::make_nvp("buffer", rr.buffer),
            cereal::make_nvp("spacing", rr.spacing),
            cereal::make_nvp("zelev", rr.zElev),
            cereal::make_nvp("zhill", rr.zHill),
            cereal::make_nvp("zflag", rr.zFlag),
            cereal::make_nvp("points", rr.points));
}

// External serialize function for ReceptorNode
template<class Archive>
void serialize(Archive& archive, ReceptorNode& rn, const std::uint32_t version)
{
    archive(cereal::make_nvp("x", rn.x),
            cereal::make_nvp("y", rn.y));
}

// External serialize function for ReceptorGrid
template<class Archive>
void serialize(Archive& archive, ReceptorGrid& rg, const std::uint32_t version)
{
    archive(cereal::make_nvp("net_id", rg.netid),
            cereal::make_nvp("xinit", rg.xInit),
            cereal::make_nvp("yinit", rg.yInit),
            cereal::make_nvp("xdelta", rg.xDelta),
            cereal::make_nvp("ydelta", rg.yDelta),
            cereal::make_nvp("xcount", rg.xCount),
            cereal::make_nvp("ycount", rg.yCount),
            cereal::make_nvp("points", rg.points));
}

// External serialize function for AreaSource
template<class Archive>
void serialize(Archive& archive, AreaSource& s, const std::uint32_t version)
{
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

// External serialize function for AreaCircSource
template<class Archive>
void serialize(Archive& archive, AreaCircSource& s, const std::uint32_t version)
{
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

// External serialize function for AreaPolySource
template<class Archive>
void serialize(Archive& archive, AreaPolySource& s, const std::uint32_t version)
{
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

// External serialize function for SourceGroup
template<class Archive>
void serialize(Archive& archive, SourceGroup& sg, const std::uint32_t version)
{
    archive(cereal::make_nvp("group_id", sg.grpid),
            cereal::make_nvp("app_method", sg.appMethod),
            cereal::make_nvp("app_factor", sg.appFactor),
            cereal::make_nvp("validation_mode", sg.validationMode),
            cereal::make_nvp("dist_app_start", sg.appStart),
            cereal::make_nvp("dist_app_rate", sg.appRate),
            cereal::make_nvp("dist_inc_depth", sg.incorpDepth),
            cereal::make_nvp("flux_profile", sg.refFlux),
            cereal::make_nvp("flux_scaling", sg.fluxScaling),
            cereal::make_nvp("sources", sg.sources),
            cereal::make_nvp("buffer_zones", sg.zones),
            cereal::make_nvp("receptor_rings", sg.rings),
            cereal::make_nvp("receptor_grids", sg.grids));

    if (version == 2) {
        archive(cereal::make_nvp("receptor_nodes", sg.nodes));
    }
}

// External serialize function for Scenario
template<class Archive>
void serialize(Archive& archive, Scenario& s, const std::uint32_t version)
{
    archive(cereal::make_nvp("title", s.title),
            cereal::make_nvp("fumigant_id", s.fumigantId),
            cereal::make_nvp("decay_coefficient", s.decayCoefficient),
            cereal::make_nvp("aermet_sf_file", s.surfaceFile),
            cereal::make_nvp("aermet_ua_file", s.upperAirFile),
            cereal::make_nvp("aermet_sf_id", s.surfaceId),
            cereal::make_nvp("aermet_ua_id", s.upperAirId),
            cereal::make_nvp("min_time", s.minTime),
            cereal::make_nvp("max_time", s.maxTime),
            cereal::make_nvp("anemometer_height", s.anemometerHeight),
            cereal::make_nvp("wind_rotation", s.windRotation),
            cereal::make_nvp("aermod_flat", s.aermodFlat),
            cereal::make_nvp("aermod_fast_area", s.aermodFastArea),
            cereal::make_nvp("aermod_low_wind", s.aermodLowWind),
            cereal::make_nvp("aermod_low_wind_svmin", s.svMin),
            cereal::make_nvp("aermod_low_wind_wsmin", s.wsMin),
            cereal::make_nvp("aermod_low_wind_franmax", s.franMax),
            cereal::make_nvp("source_groups", s.sourceGroups));
}


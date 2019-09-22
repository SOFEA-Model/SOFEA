#ifndef RUNSTREAM_H
#define RUNSTREAM_H

#include <QPolygonF>
#include <QString>
#include <QHash>

#define BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/variant.hpp>

// QString hash operator for hashed_index
namespace boost
{
    template <> struct hash<QString> {
        size_t operator()(QString const& v) const {
            return qHash(v);
        }
    };
}

namespace runstream
{
    namespace source
    {
        namespace detail
        {
            namespace tag {
                struct point {};
                struct pointcap {};
                struct pointhor {};
                struct volume {};
                struct area {};
                struct areapoly {};
                struct areacirc {};
                struct openpit {};
                struct line {};
                struct buoyline {};
            };

            template <typename T>
            struct generic {};

            template <> struct generic<tag::point> {
                double xs;
                double ys;
                double zs = 0;
                double ptemis; // point emission rate in g/s
                double stkhgt; // release height above ground in meters
                double stktmp; // stack gas exit temperature in degrees K; 0.0 for ambient
                double stkvel; // stack gas exit velocity in m/s
                double stkdia; // stack inside diameter in meters
                bool complete;
            };

            template <> struct generic<tag::pointcap> {
                double xs;
                double ys;
                double zs = 0;
                double ptemis; // point emission rate in g/s
                double stkhgt; // release height above ground in meters
                double stktmp; // stack gas exit temperature in degrees K; 0.0 for ambient
                double stkvel; // stack gas exit velocity in m/s
                double stkdia; // stack inside diameter in meters
                bool complete;
            };

            template <> struct generic<tag::pointhor> {
                double xs;
                double ys;
                double zs = 0;
                double ptemis; // point emission rate in g/s
                double stkhgt; // release height above ground in meters
                double stktmp; // stack gas exit temperature in degrees K; 0.0 for ambient
                double stkvel; // stack gas exit velocity in m/s
                double stkdia; // stack inside diameter in meters
                bool complete;
            };

            template <> struct generic<tag::volume> {
                double xs;
                double ys;
                double zs = 0;
                double vlemis; // volume emission rate in g/s
                double relhgt; // release height (center of volume) above ground, in meters
                double syinit; // initial lateral dimension of the volume in meters
                double szinit; // initial vertical dimension of the volume in meters
                bool complete;
            };

            template <> struct generic<tag::area> {
                double xs;
                double ys;
                double zs = 0;
                double aremis;     // area emission rate in g/(s-m2)
                double relhgt;     // release height above ground in meters
                double xinit;      // length of X side of the area (in the east-west direction if Angle is 0 degrees) in meters
                double yinit;      // length of Y side of the area (in the north-south direction if Angle is 0 degrees) in meters
                double angle = 0;  // orientation angle for the rectangular area in degrees from North, measured positive in the clockwise direction
                double szinit = 0; // initial vertical dimension of the area source plume in meters
                bool complete;
            };

            template <> struct generic<tag::areapoly> {
                double xs;
                double ys;
                double zs = 0;
                double aremis;     // area emission rate in g/(s-m2)
                double relhgt;     // release height above ground in meters
                int nverts;        // number of vertices (or sides) of the area source polygon
                double szinit = 0; // initial vertical dimension of the area source plume in meters
                QPolygonF areavert;
                bool complete;
            };

            template <> struct generic<tag::areacirc> {
                double xs;
                double ys;
                double zs = 0;
                double aremis;     // area emission rate in g/(s-m2)
                double relhgt;     // release height above ground in meters
                double radius;     // radius of the circular area in meters
                int nverts = 20;   // number of vertices (or sides) of the area source polygon
                double szinit = 0; // initial vertical dimension of the area source plume in meters
                bool complete;
            };

            template <> struct generic<tag::openpit> {
                double xs;
                double ys;
                double zs = 0;
                double opemis; // open pit emission rate in g/(s-m2)
                double relhgt; // average release height above the base of the pit in meters
                double xinit;  // length of X side of the open pit (in the east-west direction if Angle is 0 degrees) in meters
                double yinit;  // length of Y side of the open pit (in the north-south direction if Angle is 0 degrees) in meters
                double pitvol; // volume of open pit in cubic meters
                double angle;  // orientation angle for the rectangular open pit in degrees from North, measured positive in the clockwise direction
                bool complete;
            };

            template <> struct generic<tag::line> {
                double xs1;
                double ys1;
                double xs2;
                double ys2;
                double zs = 0;
                double lnemis; // line source emission rate in g/(s-m2)
                double relhgt; // average release height above ground in meters
                double width;  // width of the source in meters (with a minimum width of 1m)
                double szinit; // initial vertical dimension of the line source in meters
                bool complete;
            };

            template <> struct generic<tag::buoyline> {
                double xs1;
                double ys1;
                double xs2;
                double ys2;
                double zs = 0;
                double blemis;    // buoyant line emission rate in g/(s-m2) for the individual line
                double relhgt;    // average release height of the individual line above ground in meters
                bool complete;
            };
        }

        using point    = detail::generic<detail::tag::point>;
        using pointcap = detail::generic<detail::tag::pointcap>;
        using pointhor = detail::generic<detail::tag::pointhor>;
        using volume   = detail::generic<detail::tag::volume>;
        using area     = detail::generic<detail::tag::area>;
        using areapoly = detail::generic<detail::tag::areapoly>;
        using areacirc = detail::generic<detail::tag::areacirc>;
        using openpit  = detail::generic<detail::tag::openpit>;
        using line     = detail::generic<detail::tag::line>;
        using buoyline = detail::generic<detail::tag::buoyline>;

        typedef boost::variant<
            detail::generic<detail::tag::point>,
            detail::generic<detail::tag::pointcap>,
            detail::generic<detail::tag::pointhor>,
            detail::generic<detail::tag::volume>,
            detail::generic<detail::tag::area>,
            detail::generic<detail::tag::areapoly>,
            detail::generic<detail::tag::areacirc>,
            detail::generic<detail::tag::openpit>,
            detail::generic<detail::tag::line>,
            detail::generic<detail::tag::buoyline> > variant;

        struct kvp {
            kvp() {}
            kvp(QString srcid, variant var) : srcid(srcid), var(var) {}
            QString srcid;
            variant var;
        };

        using container = boost::multi_index_container<
            kvp,
            boost::multi_index::indexed_by<
                boost::multi_index::random_access<>,
                boost::multi_index::hashed_unique<
                    boost::multi_index::member<kvp, QString, &kvp::srcid>
                >
            >
        >;
    }
}

#endif // RUNSTREAM_H

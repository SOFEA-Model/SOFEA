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

#include <QPolygonF>
#include <QString>

#include <boost/variant.hpp>

namespace runstream {
namespace source {
namespace detail {

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
        struct rline {};
        struct rlinext {};
    } // namespace tag

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
        double lnemis;     // line source emission rate in g/(s-m2)
        double relhgt;     // average release height above ground in meters
        double width;      // width of the source in meters (with a minimum width of 1m)
        double szinit = 0; // initial vertical dimension of the line source in meters
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

    template <> struct generic<tag::rline> {
        double xs1;
        double ys1;
        double xs2;
        double ys2;
        double zs = 0;
        double lnemis;     // line source emission rate in g/s/m2
        double relhgt;     // average release height above ground in meters
        double width;      // width of the source in meters (with a minimum width of 1m)
        double szinit = 0; // initial vertical dimension of the line source in meters
        bool complete;
    };

    template <> struct generic<tag::rlinext> {
        double xs1;
        double ys1;
        double zs1;
        double xs2;
        double ys2;
        double zs2;
        double zs = 0;
        double qemis;      // roadway source emission rate in g/s/m
        double dcl;        // distance from the roadway centerline to the center of the source in meters
        double width;      // width for each source in meters
        double szinit;     // initial vertical dimension of the line source in meters
        bool complete;
    };

} // namespace detail

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
using rline    = detail::generic<detail::tag::rline>;
using rlinext  = detail::generic<detail::tag::rlinext>;

using variant = boost::variant<
    detail::generic<detail::tag::point>,
    detail::generic<detail::tag::pointcap>,
    detail::generic<detail::tag::pointhor>,
    detail::generic<detail::tag::volume>,
    detail::generic<detail::tag::area>,
    detail::generic<detail::tag::areapoly>,
    detail::generic<detail::tag::areacirc>,
    detail::generic<detail::tag::openpit>,
    detail::generic<detail::tag::line>,
    detail::generic<detail::tag::buoyline>,
    detail::generic<detail::tag::rline>,
    detail::generic<detail::tag::rlinext>>;

} // namespace source
} // namespace runstream

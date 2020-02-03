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

#include "Source.h"
#include "GeometryOp.h"

#include <fmt/format.h>

double Source::area() const
{
    if (geometry.size() == 0)
        return 0;
    return GeometryOp::area(geometry);
}

/****************************************************************************
** AREA
****************************************************************************/

void AreaSource::setGeometry()
{
    QPolygonF p;

    QPointF v1(xs, ys);
    QPointF v2(xs, ys + yinit);
    QPointF v3(xs + xinit, ys + yinit);
    QPointF v4(xs + xinit, ys);

    p << v1 << v2 << v3 << v4 << v1;

    // Check geometry and rotate
    if (GeometryOp::is_simple(p) && GeometryOp::is_valid(p)) {
        if (angle == 0) geometry = p;
        else {
            QPolygonF rotated;
            GeometryOp::rotate(p, rotated, QPointF(xs, ys), angle);
            geometry = rotated;
        }
        xs = p.first().x();
        ys = p.first().y();
        return;
    }
    else if (GeometryOp::correct(p)) {
        if (angle == 0) geometry = p;
        else {
            QPolygonF rotated;
            GeometryOp::rotate(p, rotated, QPointF(xs, ys), angle);
            geometry = rotated;
        }
        xs = p.first().x();
        ys = p.first().y();
        return;
    }
    else {
        return;
    }
}

std::string AreaSource::toString(std::size_t isrc) const
{
    fmt::memory_buffer w;
    fmt::format_to(w, "** Source {} (S{:0=3})\n", srcid, isrc);
    fmt::format_to(w, "   LOCATION S{:0=3} AREA     {: 10.2f} {: 10.2f} {: 6.2f}\n", isrc, xs, ys, zs);
    fmt::format_to(w, "   SRCPARAM S{:0=3} {} {} {} {} {:5.1f}\n", isrc, aremis, relhgt, xinit, yinit, angle);
    fmt::format_to(w, "   HOUREMIS flux.dat S{:0=3}\n", isrc);
    return fmt::to_string(w);
}

/****************************************************************************
** AREACIRC
****************************************************************************/

void AreaCircSource::setGeometry()
{
    QPolygonF p;

    if (nverts < 3 || radius == 0)
        return;

    // Based on SUBROUTINE GENCIR in AERMOD
    double nsides = nverts;
    double angInc = 2. * M_PI / nsides;

    // Calculate new radius that will provide an equal-area polygon
    double area = M_PI * radius * radius;
    double triArea = area / nsides;
    double opp = std::sqrt(triArea * std::tan(angInc / 2.));
    double newRad = opp / std::sin(angInc / 2.);

    // Generate vertices for circular area
    for (int i=0; i < nsides; ++i) {
        double ang = angInc * i;
        double x = (newRad * std::sin(ang)) + xs;
        double y = (newRad * std::cos(ang)) + ys;
        p << QPointF(x, y);
    }

    // Close the polygon
    p << p.at(0);

    geometry = p;
}

std::string AreaCircSource::toString(std::size_t isrc) const
{
    fmt::memory_buffer w;
    fmt::format_to(w, "** Source {} (S{:0=3})\n", srcid, isrc);
    fmt::format_to(w, "   LOCATION S{:0=3} AREACIRC {: 10.2f} {: 10.2f} {: 6.2f}\n", isrc, xs, ys, zs);
    fmt::format_to(w, "   SRCPARAM S{:0=3} {} {} {} {}\n", isrc, aremis, relhgt, radius, nverts);
    fmt::format_to(w, "   HOUREMIS FLUX.DAT S{:0=3}\n", isrc);
    return fmt::to_string(w);
}

/****************************************************************************
** AREAPOLY
****************************************************************************/

void AreaPolySource::setGeometry()
{
    QPolygonF p = geometry;

    if (GeometryOp::is_simple(p) && GeometryOp::is_valid(p)) {
        xs = geometry.first().x();
        ys = geometry.first().y();
    }
    else if (GeometryOp::correct(p)) {
        geometry = p;
        xs = geometry.first().x();
        ys = geometry.first().y();
    }
    else {
        geometry = QPolygonF(4);
        xs = 0;
        ys = 0;
    }
}

std::string AreaPolySource::toString(std::size_t isrc) const
{
    double xs = geometry.first().x();
    double ys = geometry.first().y();
    int nverts = geometry.size();

    fmt::memory_buffer w;
    fmt::format_to(w, "** Source {} (S{:0=3})\n", srcid, isrc);
    fmt::format_to(w, "   LOCATION S{:0=3} AREAPOLY {: 10.2f} {: 10.2f} {: 6.2f}\n", isrc, xs, ys, zs);
    fmt::format_to(w, "   SRCPARAM S{:0=3} {} {} {}\n", isrc, aremis, relhgt, nverts);
    fmt::format_to(w, "   AREAVERT S{:0=3}", isrc);
    for (const QPointF &p : geometry)
        fmt::format_to(w, " {: 10.2f} {: 10.2f}", p.x(), p.y());
    fmt::format_to(w, "\n   HOUREMIS FLUX.DAT S{:0=3}\n", isrc);
    return fmt::to_string(w);
}


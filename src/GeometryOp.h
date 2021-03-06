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

#include <vector>

#include <QPointF>
#include <QPolygonF>

#ifndef Q_MOC_RUN // workaround for Boost #6687 / QTBUG-22829
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#endif

class GeometryOp
{
    typedef boost::geometry::model::d2::point_xy<double> point_t;
    typedef boost::geometry::model::polygon<point_t> polygon_t;
    typedef boost::geometry::model::multi_polygon<polygon_t> mpolygon_t;
    typedef boost::geometry::model::ring<point_t> ring_t;

public:
    GeometryOp();

    static double area(QPolygonF const& polygon);
    static bool is_simple(QPolygonF const& polygon);
    static bool is_valid(QPolygonF const& polygon);
    static bool correct(QPolygonF& polygon);

    static QPolygonF rotate(const QPolygonF& polygon, const QPointF& pivot, double angle);
    static void rotate(const QPolygonF& polygon, QPolygonF& result, const QPointF& pivot, double angle);

    static void buffer(std::vector<QPolygonF> const& mpolygon, std::vector<QPolygonF>& result, double distance, double min_length, std::size_t points_per_circle);
    static void buffer(mpolygon_t const& mpolygon, mpolygon_t& result, double distance, double min_length, std::size_t points_per_circle);

    static void convert(mpolygon_t const& mpolygon, std::vector<QPolygonF> &result);
    static void convert(std::vector<QPolygonF> const& mpolygon, mpolygon_t& result);
};

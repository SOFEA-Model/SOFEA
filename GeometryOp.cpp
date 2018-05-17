// Disable select warnings with MSVC and Boost.Geometry 1.66.0
#ifdef _MSC_VER
#pragma warning(disable:4100) // Level 4, C4100: unreferenced formal parameter
#pragma warning(disable:4244) // Level 3, C4244: conversion from 'coordinate_type' to 'promoted_type'
#pragma warning(disable:4267) // Level 3, C4267: conversion from 'size_t' to 'const int'
#pragma warning(disable:4503) // Level 1, C4503: decorated name length exceeded, name was truncated
#endif

#include "GeometryOp.h"

#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/geometries/register/multi_polygon.hpp>

#include <boost/log/trivial.hpp>

// Adapt QPointF to the Boost.Geometry Point Concept
BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(QPointF, double, cs::cartesian, x, y, setX, setY)
// Adapt QPolygonF to the Boost.Geometry Ring/Polygon Concept
BOOST_GEOMETRY_REGISTER_RING(QPolygonF)

namespace bg = boost::geometry;

// bg::strategy::buffer::side_straight with extra vertices
class side_straight_ext
{
public:
    explicit inline side_straight_ext(double min_length = 0)
        : m_min_length(min_length)
    {}

    template<typename Point, typename OutputRange, typename DistanceStrategy>
    inline bg::strategy::buffer::result_code apply(
        Point const& input_p1, Point const& input_p2,
        bg::strategy::buffer::buffer_side_selector side,
        DistanceStrategy const& distance,
        OutputRange& output_range) const
    {
        if (bg::math::smaller_or_equals(m_min_length, 0))
            return bg::strategy::buffer::result_no_output;

        // Generate a block along (left or right of) the segment
        double const dx = bg::get<0>(input_p2) - bg::get<0>(input_p1);
        double const dy = bg::get<1>(input_p2) - bg::get<1>(input_p1);

        // For normalization [0,1] (=dot product d.d, sqrt)
        double const length = bg::math::sqrt(dx * dx + dy * dy);

        if (bg::math::equals(length, 0))
            return bg::strategy::buffer::result_no_output;

        double const d = distance.apply(input_p1, input_p2, side);

        // Generate the normalized perpendicular p, to the left (ccw)
        double const px = -dy / length;
        double const py = dx / length;

        // Calculate required number of segments
        std::size_t ns = (std::max)(static_cast<std::size_t>(
            ceil(length / m_min_length)), std::size_t(1));

        // Generate additional vertices
        Point step = input_p2;
        bg::subtract_point(step, input_p1); // input_p1 not modified
        bg::divide_value(step, ns);
        Point accu = input_p1; // accumulator

        output_range.reserve(ns + 1);

        for (std::size_t i = 0; i <= ns; ++i)
        {
            typename boost::range_value<OutputRange>::type output_point;
            bg::set<0>(output_point, bg::get<0>(accu) + px * d);
            bg::set<1>(output_point, bg::get<1>(accu) + py * d);
            output_range.push_back(output_point);
            bg::add_point(accu, step);
        }

        return bg::strategy::buffer::result_normal;
    }

private:
    double m_min_length;
};

GeometryOp::GeometryOp()
{
}

double GeometryOp::area(QPolygonF const& polygon)
{
    // 1 square meter = 0.0001 hectares
    double area = boost::geometry::area(polygon) * 0.0001;
    return area;
}

bool GeometryOp::is_simple(QPolygonF const& polygon)
{
    return boost::geometry::is_simple(polygon);
}

bool GeometryOp::is_valid(QPolygonF const& polygon)
{
    return boost::geometry::is_valid(polygon);
}

bool GeometryOp::correct(QPolygonF &polygon)
{
    boost::geometry::correct(polygon);
    std::string message;
    if (!boost::geometry::is_valid(polygon, message)) {
        BOOST_LOG_TRIVIAL(error) << message;
        return false;
    }
    else {
        return true;
    }
}

void GeometryOp::rotate(const QPolygonF& polygon, QPolygonF& result, const QPointF& pivot, double angle)
{
    bg::strategy::transform::translate_transformer<double, 2, 2> translate1(-pivot.x(), -pivot.y());
    bg::strategy::transform::rotate_transformer<bg::degree, double, 2, 2> rotate(angle);
    bg::strategy::transform::translate_transformer<double, 2, 2> translate2(pivot.x(), pivot.y());

    QPolygonF res1, res2;

    bg::transform(polygon, res1,   translate1); // move from pivot point to origin
    bg::transform(res1,    res2,   rotate);     // rotate about the origin
    bg::transform(res2,    result, translate2); // move from origin to pivot point
}

void GeometryOp::buffer(std::vector<QPolygonF> const& mpolygon, std::vector<QPolygonF> &result, double distance, double min_length, std::size_t points_per_circle)
{
    mpolygon_t mp_in;
    mpolygon_t mp_out;

    convert(mpolygon, mp_in);
    buffer(mp_in, mp_out, distance, min_length, points_per_circle);
    convert(mp_out, result);
}

void GeometryOp::buffer(mpolygon_t const& mpolygon, mpolygon_t& result, double distance, double min_length, std::size_t points_per_circle)
{
    bg::strategy::buffer::distance_symmetric<double> distance_strategy(distance);
    bg::strategy::buffer::join_round join_strategy(points_per_circle);
    bg::strategy::buffer::end_round end_strategy(points_per_circle);
    bg::strategy::buffer::point_circle point_strategy(points_per_circle);
    side_straight_ext side_strategy(min_length); // Replaces bg::strategy::buffer::side_straight

    try {
        bg::buffer(mpolygon, result,
                   distance_strategy, side_strategy, join_strategy, end_strategy, point_strategy);
    } catch (const bg::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }
}

void GeometryOp::convert(mpolygon_t const& mpolygon, std::vector<QPolygonF> &result)
{
    for (polygon_t const& p : mpolygon)
    {
        ring_t const& ring = bg::exterior_ring(p); // outer ring
        QPolygonF qring;
        bg::transform(ring, qring);
        result.push_back(qring);
    }
}

void GeometryOp::convert(std::vector<QPolygonF> const& mpolygon, mpolygon_t& result)
{
    for (QPolygonF const& ring : mpolygon)
    {
        polygon_t p;
        bg::convert(ring, p);
        result.push_back(p);
    }
}

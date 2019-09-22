#include "GeosOp.h"

#include <cstdio>
#include <string>
#include <memory>

#include <QDebug>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#ifdef GEOS_INLINE
#undef GEOS_INLINE
#endif

#include <geos_c.h>

static void noticeHandler(const char *fmt, ...)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Geometry");

    char buf[1024] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    BOOST_LOG_TRIVIAL(info) << "geos: " << buf;

    QString message = QString::fromLocal8Bit(buf);
    qDebug() << "geos:";
    qDebug() << message;

    va_end(ap);
}

static void errorHandler(const char *fmt, ...)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Geometry");

    char buf[1024] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    BOOST_LOG_TRIVIAL(error) << "geos: " << buf;

    QString message = QString::fromLocal8Bit(buf);
    qDebug() << "geos:";
    qDebug() << message;

    va_end(ap);
}

//-----------------------------------------------------------------------------
// Adapter Functions
//-----------------------------------------------------------------------------

static GEOSGeometry* polygonsToGeos(const std::vector<QPolygonF>& input)
{
    std::vector<GEOSGeometry*> geoms;
    geoms.reserve(input.size());

    for (const QPolygonF& polygon : input)
    {
        // Add points to coordinate sequence
        GEOSCoordSequence *seq = GEOSCoordSeq_create(polygon.size(), 2);
        for (int i = 0; i < polygon.size(); ++i) {
            const QPointF& point = polygon.at(i);
            GEOSCoordSeq_setX(seq, i, point.x());
            GEOSCoordSeq_setY(seq, i, point.y());
        }

        // Create polygon from coordinate sequence
        GEOSGeometry *shell = GEOSGeom_createLinearRing(seq);
        if (shell) {
            geoms.push_back(GEOSGeom_createPolygon(shell, nullptr, 0));
        }
    }

    // Collection takes ownership of individual geometries
    return GEOSGeom_createCollection(GEOS_MULTIPOLYGON,
        geoms.data(), static_cast<int>(geoms.size()));
}

static GEOSGeometry* polygonToGeos(const QPolygonF& polygon)
{
    // Add points to coordinate sequence
    GEOSCoordSequence *seq = GEOSCoordSeq_create(polygon.size(), 2);
    for (int i = 0; i < polygon.size(); ++i) {
        const QPointF& point = polygon.at(i);
        GEOSCoordSeq_setX(seq, i, point.x());
        GEOSCoordSeq_setY(seq, i, point.y());
    }

    // Create polygon from coordinate sequence
    GEOSGeometry *shell = GEOSGeom_createLinearRing(seq);
    return GEOSGeom_createPolygon(shell, nullptr, 0);
}

static QPolygonF geosToPolygon(const GEOSGeometry* input)
{
    const GEOSCoordSequence *seq;
    const GEOSGeometry *g;

    switch (GEOSGeomTypeId(input))
    {
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
        seq = GEOSGeom_getCoordSeq(input);
        break;
    case GEOS_POLYGON:
        g = GEOSGetExteriorRing(input);
        seq = GEOSGeom_getCoordSeq(g);
        break;
    default:
        seq = nullptr;
        break;
    }

    // Create polygon from valid coordinate sequence
    QPolygonF result;
    unsigned int size = 0;
    if (GEOSCoordSeq_getSize(seq, &size)) {
        for (unsigned int i = 0; i < size; ++i) {
            double px, py;
            GEOSCoordSeq_getX(seq, i, &px);
            GEOSCoordSeq_getY(seq, i, &py);
            QPointF point(px, py);
            result.push_back(point);
        }
    }

    return result;
}

static std::vector<QPolygonF> geosToPolygons(const GEOSGeometry* input)
{
    std::vector<QPolygonF> result;

    switch (GEOSGeomTypeId(input))
    {
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
    case GEOS_POLYGON: {
        QPolygonF polygon = geosToPolygon(input);
        result.push_back(polygon);
        break;
    }
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION: {
        int ngeom = GEOSGetNumGeometries(input);
        result.reserve(ngeom);
        for (int i = 0; i < ngeom; ++i) {
            const GEOSGeometry *g = GEOSGetGeometryN(input, i);
            QPolygonF polygon = geosToPolygon(g);
            result.push_back(polygon);
        }
        break;
    }
    default:
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
// Geometry Operations
//-----------------------------------------------------------------------------

GeosOp::GeosOp()
{
    initGEOS(noticeHandler, errorHandler);
}

GeosOp::~GeosOp()
{
    finishGEOS();
}

QPointF GeosOp::interiorPoint(const QPolygonF& polygon)
{
    GEOSGeometry *geom = polygonToGeos(polygon);
    GEOSGeometry *p = GEOSPointOnSurface(geom);

    double px, py;
    GEOSGeomGetX(p, &px);
    GEOSGeomGetY(p, &py);
    QPointF result{px, py};

    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(p);

    return result;
}

std::vector<QPolygonF> GeosOp::unionCascaded(const std::vector<QPolygonF>& mpolygon)
{
    std::vector<QPolygonF> result;

    if (mpolygon.empty())
        return result;

    GEOSGeometry* input = polygonsToGeos(mpolygon);
    GEOSGeometry* output = GEOSUnionCascaded(input);
    result = geosToPolygons(output);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(output);

    return result;
}

std::vector<QPolygonF> GeosOp::buffer(std::vector<QPolygonF> const& mpolygon, double distance, int points_per_circle)
{
    std::vector<QPolygonF> result;

    if (mpolygon.empty() || distance <= 0)
        return result;

    GEOSGeometry* input = polygonsToGeos(mpolygon);
    GEOSGeometry* output = GEOSBuffer(input, distance, points_per_circle);
    result = geosToPolygons(output);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(output);

    return result;
}

std::vector<QPolygonF> GeosOp::measurePoints(std::vector<QPolygonF> const& mpolygon, double spacing)
{
    // FIXME: The C API GEOSInterpolate function generates a new linear
    // index each time it is called. Use the C++ API to prevent this.

    std::vector<QPolygonF> result;

    if (mpolygon.empty() || spacing <= 0)
        return result;

    GEOSGeometry* input = polygonsToGeos(mpolygon);

    for (int i = 0; i < GEOSGetNumGeometries(input); ++i)
    {
        const GEOSGeometry *g = GEOSGetGeometryN(input, i);
        const GEOSGeometry *ring = GEOSGetExteriorRing(g);

        // Calculate required number of segments
        double length = 0;
        GEOSLength(ring, &length);
        int ns = floor(length / spacing);

        // Generate points along ring using linear referencing
        QPolygonF points;
        points.reserve(ns + 1);
        for (int i = 0; i <= ns; ++i) {
            double px, py;
            double pd = static_cast<double>(i) * spacing;
            GEOSGeometry *p = GEOSInterpolate(ring, pd);
            GEOSGeomGetX(p, &px);
            GEOSGeomGetY(p, &py);
            points.push_back(QPointF(px, py));
            GEOSGeom_destroy(p);
        }

        result.push_back(points);
    }

    GEOSGeom_destroy(input);

    return result;
}

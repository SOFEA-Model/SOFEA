#include "ShapefileParser.h"
#include "ShapefileAttributeDialog.h"
#include "ShapefileAttributeInfo.h"
#include "GeometryOp.h"

#include <QPolygonF>
#include <QDebug>

#include <fmt/format.h>
#include <shapelib/shapefil.h>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

const int MAX_ENTITIES = 999;
const double MAX_COORDINATE_ABS_VALUE = 1.0e4;

std::string shapeTypeString(int shpt)
{
    switch (shpt) {
    case(SHPT_NULL):        return "NULL";
    case(SHPT_POINT):       return "POINT";
    case(SHPT_ARC):         return "ARC";
    case(SHPT_POLYGON):     return "POLYGON";
    case(SHPT_MULTIPOINT):  return "MULTIPOINT";
    case(SHPT_POINTZ):      return "POINTZ";
    case(SHPT_ARCZ):        return "ARCZ";
    case(SHPT_POLYGONZ):    return "POLYGONZ";
    case(SHPT_MULTIPOINTZ): return "MULTIPOINTZ";
    case(SHPT_POINTM):      return "POINTM";
    case(SHPT_ARCM):        return "ARCM";
    case(SHPT_POLYGONM):    return "POLYGONM";
    case(SHPT_MULTIPOINTM): return "MULTIPOINTM";
    case(SHPT_MULTIPATCH):  return "MULTIPATCH";
    default:                return "UNKNOWN";
    }
}

void ShapefileParser::parseSources(const QString& filename, SourceGroup *sgPtr)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Import");

    int pnEntities, pnShapeType;
    double adfMinBound[4], adfMaxBound[4]; // X, Y, Z, M
    double xshift = 0, yshift = 0;

    // Open the file with read-only access.
    SHPHandle handle = SHPOpen(filename.toStdString().c_str(), "rb");

    if (handle <= 0) {
        BOOST_LOG_TRIVIAL(error) << "shapelib: error opening file";
        return;
    }

    SHPGetInfo(handle, &pnEntities, &pnShapeType, adfMinBound, adfMaxBound);

    // Check the number of entities.
    if (pnEntities <= 0) {
        BOOST_LOG_TRIVIAL(error) << "shapelib: empty file";
        SHPClose(handle);
        return;
    }

    if (pnEntities >= MAX_ENTITIES) {
        BOOST_LOG_TRIVIAL(warning) << "Maximum " << MAX_ENTITIES << " polygons will be read";
    }

    // Check the shape type.
    if ((pnShapeType != SHPT_POLYGON) &&
        (pnShapeType != SHPT_POLYGONZ) &&
        (pnShapeType != SHPT_POLYGONM)) {
        BOOST_LOG_TRIVIAL(error) << "shapelib: unsupported shape type: "
                                 << shapeTypeString(pnShapeType);
        SHPClose(handle);
        return;
    }

    // Get info from the associated DBF file, if available.
    ShapefileAttributeInfo attrInfo(filename);
    QMap<ShapefileSelectedIndex, int> indexMap;

    // Ensure the DBF and SHP record counts match and open a dialog to get field mapping.
    if (attrInfo.fieldCount > 0 && attrInfo.recordCount == pnEntities) {
        ShapefileAttributeDialog dialog(attrInfo, &indexMap);
        dialog.exec();
    }

    // Determine whether shift is required.
    if (fabs(adfMinBound[0]) >= MAX_COORDINATE_ABS_VALUE ||
        fabs(adfMinBound[1]) >= MAX_COORDINATE_ABS_VALUE) {
        // Calculate reasonable shift values, rounding off to the nearest hundred.
        xshift = -adfMinBound[0];
        yshift = -adfMinBound[1];
        xshift = static_cast<int>(xshift / 100) * 100.0;
        yshift = static_cast<int>(yshift / 100) * 100.0;
        BOOST_LOG_TRIVIAL(info) << "Coordinates are too large. Applying shift: ("
                                << static_cast<int>(xshift) << ", "
                                << static_cast<int>(yshift) << ")";
    }

    // Read only polygons, and only those without holes.
    int nProcessed = 0;
    for (int i = 0; i < pnEntities; ++i)
    {
        if (i > MAX_ENTITIES)
            break;

        SHPObject *psShape = SHPReadObject(handle, i);
        if (psShape == nullptr)
            continue;

        // Check that the geometry is valid.
        bool validType = (psShape->nSHPType == SHPT_POLYGON) ||
                         (psShape->nSHPType == SHPT_POLYGONZ) ||
                         (psShape->nSHPType == SHPT_POLYGONZ);
        bool validPartCount = (psShape->nParts == 1);
        bool validVertexCount = (psShape->nVertices >= 4);

        if (!validType)
            BOOST_LOG_TRIVIAL(warning) << "Polygon at index " << i << " has invalid type";
        else if (!validPartCount)
            BOOST_LOG_TRIVIAL(warning) << "Polygon at index " << i << " contains holes";
        else if (!validVertexCount)
            BOOST_LOG_TRIVIAL(warning) << "Polygon at index " << i << " has invalid vertex count";

        // Create the AREAPOLY source and read vertices.
        if (validType && validPartCount && validVertexCount) {
            QPolygonF p;
            for (int j = 0; j < psShape->nVertices; ++j) {
                double x = psShape->padfX[j] + xshift;
                double y = psShape->padfY[j] + yshift;
                p << QPointF(x, y);
            }

            // Validate geometry.
            bool validGeometry = true;
            if (!GeometryOp::is_simple(p) || !GeometryOp::is_valid(p)) {
                validGeometry = GeometryOp::correct(p);
            }

            // Create the source.
            if (validGeometry) {
                AreaPolySource *source = new AreaPolySource;
                source->geometry = p;
                source->xshift = xshift;
                source->yshift = yshift;
                source->xs = p.first().x();
                source->ys = p.first().y();

                // Set attributes.
                int idxSourceID = indexMap.value(ShapefileSelectedIndex::SourceID, -1);
                int idxAppDate = indexMap.value(ShapefileSelectedIndex::AppDate, -1);
                int idxAppTime = indexMap.value(ShapefileSelectedIndex::AppTime, -1);
                int idxAppRate = indexMap.value(ShapefileSelectedIndex::AppRate, -1);
                int idxIncDepth = indexMap.value(ShapefileSelectedIndex::IncDepth, -1);

                if (idxSourceID > -1)
                    source->srcid = attrInfo.getStringField(idxSourceID, i).toStdString();
                else
                    source->srcid = fmt::format("Poly{:0=4}", i + 1);

                if (idxAppDate > -1) {
                    QString dateStr = attrInfo.getStringField(idxAppDate, i);
                    QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
                    if (idxAppTime > -1) {
                        QString timeStr = attrInfo.getStringField(idxAppTime, i);
                        QTime time = QTime::fromString(timeStr, "hhmm");
                        source->appStart = QDateTime(date, time, Qt::UTC);
                    }
                    else {
                        source->appStart = QDateTime(date);
                    }
                }
                else {
                    sgPtr->initSourceAppStart(source);
                }

                if (idxAppRate > -1)
                    source->appRate = attrInfo.getNumericField(idxAppRate, i);
                else
                    sgPtr->initSourceAppRate(source);

                if (idxIncDepth > -1)
                    source->incorpDepth = attrInfo.getNumericField(idxIncDepth, i);
                else
                    sgPtr->initSourceIncorpDepth(source);

                // Add the source to the source group.
                sgPtr->sources.push_back(source);
                nProcessed++;
            }
        }

        SHPDestroyObject(psShape);
    }

    BOOST_LOG_TRIVIAL(info) << nProcessed << "/" << pnEntities << " polygons read from shapefile";

    // Close the file and free resources.
    SHPClose(handle);
}


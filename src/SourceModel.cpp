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

#include "SourceModel.h"
#include "RunstreamParser.h"
#include "ShapefileParser.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QFileInfo>
#include <QSettings>

#include <QDebug>

#include <algorithm>

#include <boost/ptr_container/clone_allocator.hpp>
#include <fmt/format.h>

SourceModel::SourceModel(Scenario *s, SourceGroup *sg, QObject *parent)
    : QAbstractTableModel(parent), sPtr(s), sgPtr(sg)
{}

void SourceModel::reset()
{
    beginResetModel();
    endResetModel();
}

void SourceModel::import()
{
    QString fileFilter = QString("%1;;%2;;%3").arg(
        "All Supported Files (*.shp *.inp *.dat)",
        "ESRI Shapefile (*.shp)",
        "Runstream File (*.inp *.dat)");

    QSettings settings;
    QString settingsKey = "DefaultImportDirectory";
    QString defaultDirectory = settings.value(settingsKey, qApp->applicationDirPath()).toString();

    const QString filename = QFileDialog::getOpenFileName(nullptr,
        tr("Import Source Data"), defaultDirectory, fileFilter);

    if (filename.isEmpty())
        return;

    QFileInfo fi(filename);
    QString dir = fi.absoluteDir().absolutePath();
    settings.setValue(settingsKey, dir);

    beginResetModel();
    QString ext = fi.completeSuffix();
    if (ext == "shp")
        ShapefileParser::parseSources(filename, sgPtr);
    else if (ext == "inp" || ext == "dat")
        RunstreamParser::parseSources(filename, sgPtr);
    endResetModel();
}

void SourceModel::setProjection(const Projection::Generic& p)
{
    // Geographic to projected coordinates
    transform = Projection::Transform(p.geodeticCRS(), p.compoundCRS(), sPtr->domain);
}

Source* SourceModel::sourceFromIndex(const QModelIndex &index) const
{
    if (index.isValid() && index.row() < rowCount()) {
        Source &s = sgPtr->sources.at(index.row());
        return &s;
    }
    return nullptr;
}

QModelIndex SourceModel::vertexIndex(int row, int i) const
{
    quintptr internalId = static_cast<quintptr>(i);
    return createIndex(row, Column::VertexData, internalId);
}

void SourceModel::emitDataChanged(const QModelIndex& index)
{
    emit dataChanged(index, index);
}

void SourceModel::setColumnHidden(int column, bool hidden)
{
    columnHidden[column] = hidden;
}

bool SourceModel::isColumnHidden(int column) const
{
    return columnHidden.value(column, false);
}

void SourceModel::addAreaSource()
{
    static unsigned int seqArea = 1;
    AreaSource *s = new AreaSource;
    s->srcid = fmt::format("Area{:0=4}", seqArea);
    s->setGeometry();
    sgPtr->initSource(s);
    seqArea++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    sgPtr->sources.push_back(s);
    endInsertRows();
}

void SourceModel::addAreaCircSource()
{
    static unsigned int seqCirc = 1;
    AreaCircSource *s = new AreaCircSource;
    s->srcid = fmt::format("Circ{:0=4}", seqCirc);
    s->setGeometry();
    sgPtr->initSource(s);
    seqCirc++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    sgPtr->sources.push_back(s);
    endInsertRows();
}

void SourceModel::addAreaPolySource()
{
    static unsigned int seqPoly = 1;
    AreaPolySource *s = new AreaPolySource;
    s->srcid = fmt::format("Poly{:0=4}", seqPoly);

    QPointF v1(s->xs, s->ys);
    QPointF v2(s->xs, s->ys + 100);
    QPointF v3(s->xs + 100, s->ys + 100);
    QPointF v4(s->xs + 100, s->ys);
    s->geometry << v1 << v2 << v3 << v4 << v1;
    sgPtr->initSource(s);
    seqPoly++;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    sgPtr->sources.push_back(s);
    endInsertRows();
}

int SourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return sgPtr->sources.size();
}

int SourceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 21;
}

QModelIndex SourceModel::index(int row, int column, const QModelIndex &parent) const
{
    // Create an index for extended columns (column >= columnCount).

    switch (column) {
    case Column::XInit:
    case Column::YInit:
    case Column::Angle:
    case Column::Radius:
    case Column::VertexCount:
    case Column::EmissionRate:
    case Column::ReleaseHeight:
    case Column::SigmaZ0:
    case Column::VertexData:
        return createIndex(row, column);
    default:
        return QAbstractTableModel::index(row, column, parent);
    }
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    const Source &s = sgPtr->sources.at(index.row());
    SourceType stype = qvariant_cast<SourceType>(headerData(index.row(), Qt::Vertical, Qt::UserRole));
    quintptr internalId = index.internalId();

    switch (role) {
    case Qt::ForegroundRole:
        // Blue: Monte Carlo
        // Red:  Calculated
        switch (index.column()) {
        case Column::ID:                  return QVariant();
        case Column::X:                   return QVariant();
        case Column::Y:                   return QVariant();
        case Column::Z:                   return QVariant();
        case Column::Longitude:           return QVariant();
        case Column::Latitude:            return QVariant();
        case Column::Area:                return QVariant(QColor(Qt::red));
        case Column::Start:               return QVariant(QColor(Qt::blue));
        case Column::AppRate:             return QVariant(QColor(Qt::blue));
        case Column::IncDepth:            return QVariant(QColor(Qt::blue));
        case Column::MassAF:              return QVariant(QColor(Qt::red));
        case Column::FluxProfile:         return QVariant(QColor(Qt::blue));
        case Column::TimeSF:              return QVariant(QColor(Qt::red));
        case Column::DepthSF:             return QVariant(QColor(Qt::red));
        case Column::OverallSF:           return QVariant(QColor(Qt::red));
        case Column::BZDistance:          return QVariant();
        case Column::BZDuration:          return QVariant();
        case Column::AirDiffusion:        return QVariant(QColor(Qt::blue));
        case Column::WaterDiffusion:      return QVariant(QColor(Qt::blue));
        case Column::CuticularResistance: return QVariant(QColor(Qt::blue));
        case Column::HenryConstant:       return QVariant(QColor(Qt::blue));
        default: return QVariant();
        }
    case Qt::UserRole:
        switch (index.column()) {
        case Column::FluxProfile: {
            // Used with ComboBoxDelegate
            if (const auto fp = s.fluxProfile.lock()) {
                auto it = std::find(sPtr->fluxProfiles.begin(), sPtr->fluxProfiles.end(), fp);
                if (it != sPtr->fluxProfiles.end())
                    return static_cast<int>(std::distance(sPtr->fluxProfiles.begin(), it));
            }
            return QVariant();
        }
        case Column::VertexData:
            // Used with VertexEditorDelegate
            if (internalId < static_cast<quintptr>(s.geometry.size()))
                return s.geometry.at(static_cast<int>(internalId));
            return QVariant();
        default:
            return QVariant();
        }
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column()) {
        case Column::ID:                  return QString::fromStdString(s.srcid);
        case Column::X:                   return s.xs;
        case Column::Y:                   return s.ys;
        case Column::Z:                   return s.zs;
        case Column::Longitude:
        case Column::Latitude:
            if (transform.isValid()) {
                double lon, lat, elev;
                int rc = transform.inverse(s.xs, s.ys, s.zs, lon, lat, elev);
                if (rc != 0)
                    return QVariant();

                if (index.column() == Column::Longitude)
                    return lon;
                if (index.column() == Column::Latitude)
                    return lat;
            }
            return QVariant();
        case Column::Area:                return s.area();
        case Column::Start:               return s.appStart.toString("yyyy-MM-dd HH:mm");
        case Column::AppRate:             return s.appRate;
        case Column::IncDepth:            return s.incorpDepth;
        case Column::MassAF:              return s.appRate * s.area() * sgPtr->appFactor;
        case Column::FluxProfile:
            if (const auto fp = s.fluxProfile.lock())
                return QString::fromStdString(fp->name);
            return QVariant();
        case Column::TimeSF:
            if (const auto fp = s.fluxProfile.lock())
                return fp->timeScaleFactor(s.appStart);
            return QVariant();
        case Column::DepthSF:
            if (const auto fp = s.fluxProfile.lock())
                return fp->depthScaleFactor(s.incorpDepth);
            return QVariant();
        case Column::OverallSF:
            if (const auto fp = s.fluxProfile.lock())
                return fp->fluxScaleFactor(s.appRate, s.appStart, s.incorpDepth);
            return QVariant();
        case Column::BZDistance:
        case Column::BZDuration:
            if (sgPtr->enableBufferZones) {
                BufferZone zref;
                zref.appRateThreshold = s.appRate;
                zref.areaThreshold = s.area();

                auto it = std::find_if(sgPtr->zones.begin(), sgPtr->zones.end(), [&zref](const BufferZone& z) {
                   return (z.areaThreshold >= zref.areaThreshold) && (z.appRateThreshold >= zref.appRateThreshold);
                });

                if (it != sgPtr->zones.end()) {
                    if (index.column() == Column::BZDistance)
                        return it->distance;
                    if (index.column() == Column::BZDuration)
                        return it->duration;
                }
            }
            return QVariant();
        case Column::AirDiffusion:        return s.airDiffusion;
        case Column::WaterDiffusion:      return s.waterDiffusion;
        case Column::CuticularResistance: return s.cuticularResistance;
        case Column::HenryConstant:       return s.henryConstant;
        case Column::XInit:               return (stype == SourceType::AREA) ? static_cast<const AreaSource *>(&s)->xinit : QVariant();
        case Column::YInit:               return (stype == SourceType::AREA) ? static_cast<const AreaSource *>(&s)->yinit : QVariant();
        case Column::Angle:               return (stype == SourceType::AREA) ? static_cast<const AreaSource *>(&s)->angle : QVariant();
        case Column::Radius:              return (stype == SourceType::AREACIRC) ? static_cast<const AreaCircSource *>(&s)->radius : QVariant();
        case Column::VertexCount:
            switch (stype) {
            case SourceType::AREA:        return s.geometry.size();
            case SourceType::AREACIRC:    return static_cast<const AreaCircSource *>(&s)->nverts;
            case SourceType::AREAPOLY:    return s.geometry.size();
            default:                      return QVariant();
            }
        case Column::EmissionRate:
            switch (stype) {
            case SourceType::AREA:        return static_cast<const AreaSource *>(&s)->aremis;
            case SourceType::AREACIRC:    return static_cast<const AreaCircSource *>(&s)->aremis;
            case SourceType::AREAPOLY:    return static_cast<const AreaPolySource *>(&s)->aremis;
            default:                      return QVariant();
            }
        case Column::ReleaseHeight:
            switch (stype) {
            case SourceType::AREA:        return static_cast<const AreaSource *>(&s)->relhgt;
            case SourceType::AREACIRC:    return static_cast<const AreaCircSource *>(&s)->relhgt;
            case SourceType::AREAPOLY:    return static_cast<const AreaPolySource *>(&s)->relhgt;
            default:                      return QVariant();
            }
        case Column::SigmaZ0:
            switch (stype) {
            case SourceType::AREA:        return static_cast<const AreaSource *>(&s)->szinit;
            case SourceType::AREACIRC:    return static_cast<const AreaCircSource *>(&s)->szinit;
            case SourceType::AREAPOLY:    return static_cast<const AreaPolySource *>(&s)->szinit;
            default:                      return QVariant();
            }
        case Column::VertexData:          return s.geometry;
        default: return QVariant();
        }
    default:
        return QVariant();
    }
}

bool SourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    Source &s = sgPtr->sources.at(static_cast<std::size_t>(row));
    SourceType stype = qvariant_cast<SourceType>(headerData(index.row(), Qt::Vertical, Qt::UserRole));
    quintptr internalId = index.internalId();

    if (role == Qt::UserRole)
    {
        switch (index.column()) {
        case Column::FluxProfile:
            // Used with ComboBoxDelegate
            if (value.canConvert(QVariant::Int)) {
                int fpIndex = value.toInt();
                if (fpIndex >= 0 && fpIndex < sPtr->fluxProfiles.size())
                    s.fluxProfile = sPtr->fluxProfiles[fpIndex];
                emit dataChanged(index, index);
                return true;
            }
            break;
        case Column::VertexData:
            // Used with VertexEditorDelegate
            if (value.canConvert(QVariant::PointF)) {
                const QPointF p = value.toPointF();
                int i = static_cast<int>(internalId);

                if (i < 0 || i >= s.geometry.size())
                    return false;

                s.geometry.replace(i, p);

                // Last vertex must equal first vertex.
                if (i == 0)
                    s.geometry.back() = p;

                s.setGeometry();
                QModelIndex xindex = index.siblingAtColumn(Column::X);
                QModelIndex yindex = index.siblingAtColumn(Column::Y);
                emit dataChanged(index, index);
                emit dataChanged(xindex, yindex);
                return true;
            }
            break;
        default:
            break;
        }
    }
    else if (role == Qt::EditRole)
    {
        if ((index.column() == Column::Longitude || index.column() == Column::Latitude) &&
            transform.isValid())
        {
            double lon, lat;
            if (index.column() == Column::Longitude) {
                lon = value.toDouble();
                lat = data(index.siblingAtColumn(Column::Latitude), Qt::DisplayRole).toDouble();
            }
            else {
                lon = data(index.siblingAtColumn(Column::Longitude), Qt::DisplayRole).toDouble();
                lat = value.toDouble();
            }

            double x, y, z;
            int rc = transform.forward(lon, lat, 0, x, y, z);
            if (rc != 0) {
                return false;
            }

            s.xs = x;
            s.ys = y;
            s.setGeometry();
            QModelIndex xindex = index.siblingAtColumn(Column::X);
            QModelIndex yindex = index.siblingAtColumn(Column::Y);
            emit dataChanged(index, index);
            emit dataChanged(xindex, yindex);
            return true;
        }

        switch (index.column()) {
        case Column::ID: {
            QString srcid = value.toString();
            // Remove any non-alphanumeric characters and truncate to max length (100)
            srcid.remove(QRegExp("[^A-Za-z0-9_]+"));
            srcid.truncate(100);
            s.srcid = srcid.toStdString();
            break;
        }
        case Column::X:
            s.xs = value.toDouble();
            s.setGeometry();
            break;
        case Column::Y:
            s.ys = value.toDouble();
            s.setGeometry();
            break;
        case Column::Z:
            s.zs = value.toDouble();
            break;
        case Column::Start:
            s.appStart = value.toDateTime();
            break;
        case Column::AppRate:
            s.appRate = value.toDouble();
            break;
        case Column::IncDepth:
            s.incorpDepth = value.toDouble();
            break;
        //case Column::BZDistance: // FIXME
        //case Column::BZDuration: // FIXME
        case Column::AirDiffusion:
            s.airDiffusion = value.toDouble();
            break;
        case Column::WaterDiffusion:
            s.waterDiffusion = value.toDouble();
            break;
        case Column::CuticularResistance:
            s.cuticularResistance = value.toDouble();
            break;
        case Column::HenryConstant:
            s.henryConstant = value.toDouble();
            break;
        case Column::XInit:
            if (stype == SourceType::AREA && value.canConvert(QVariant::Double)) {
                static_cast<AreaSource *>(&s)->xinit = value.toDouble();
                s.setGeometry();
            }
            break;
        case Column::YInit:
            if (stype == SourceType::AREA && value.canConvert(QVariant::Double)) {
                static_cast<AreaSource *>(&s)->yinit = value.toDouble();
                s.setGeometry();
            }
            break;
        case Column::Angle:
            if (stype == SourceType::AREA && value.canConvert(QVariant::Double)) {
                static_cast<AreaSource *>(&s)->angle = value.toDouble();
                s.setGeometry();
            }
            break;
        case Column::Radius:
            if (stype == SourceType::AREACIRC && value.canConvert(QVariant::Double)) {
                static_cast<AreaCircSource *>(&s)->radius = value.toDouble();
                s.setGeometry();
            }
            break;
        case Column::VertexCount:
            switch (stype) {
            case SourceType::AREACIRC:
                if (value.canConvert(QVariant::Int)) {
                    static_cast<AreaCircSource *>(&s)->nverts = value.toInt();
                    s.setGeometry();
                }
                break;
            case SourceType::AREAPOLY:
                if (value.canConvert(QVariant::Int)) {
                    int n = value.toInt();
                    if (n > 0) {
                        s.geometry.resize(n);
                        s.geometry.back() = s.geometry.front();
                    }
                }
            default:
                break;
            }
            break;
        case EmissionRate:
            switch (stype) {
            case SourceType::AREA:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaSource *>(&s)->aremis = value.toDouble();
                break;
            case SourceType::AREACIRC:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaCircSource *>(&s)->aremis = value.toDouble();
                break;
            case SourceType::AREAPOLY:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaPolySource *>(&s)->aremis = value.toDouble();
                break;
            default:
                break;
            }
            break;
        case ReleaseHeight:
            switch (stype) {
            case SourceType::AREA:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaSource *>(&s)->relhgt = value.toDouble();
                break;
            case SourceType::AREACIRC:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaCircSource *>(&s)->relhgt = value.toDouble();
                break;
            case SourceType::AREAPOLY:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaPolySource *>(&s)->relhgt = value.toDouble();
                break;
            default:
                break;
            }
            break;
        case SigmaZ0:
            switch (stype) {
            case SourceType::AREA:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaSource *>(&s)->szinit = value.toDouble();
                break;
            case SourceType::AREACIRC:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaCircSource *>(&s)->szinit = value.toDouble();
                break;
            case SourceType::AREAPOLY:
                if (value.canConvert(QVariant::Double))
                    static_cast<AreaPolySource *>(&s)->szinit = value.toDouble();
                break;
            default:
                break;
            }
            break;
        case VertexData:
            if (value.canConvert(QVariant::PolygonF)) {
                s.geometry = qvariant_cast<QPolygonF>(value);
                s.setGeometry();
                QModelIndex xindex = index.siblingAtColumn(Column::X);
                QModelIndex yindex = index.siblingAtColumn(Column::Y);
                emit dataChanged(index, index);
                emit dataChanged(xindex, yindex);
            }
            break;
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant SourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // SourceType:
    // qvariant_cast<SourceType>(headerData(index.row(), Qt::Vertical, Qt::UserRole));

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Column::ID:                   return QString("ID");
            case Column::X:                    return QString("X (m)");
            case Column::Y:                    return QString("Y (m)");
            case Column::Z:                    return QString("Z (m)");
            case Column::Longitude:            return QString("Longitude");
            case Column::Latitude:             return QString("Latitude");
            case Column::Area:                 return QString("Area (ha)");
            case Column::Start:                return QString("Start");
            case Column::AppRate:              return QString("App. Rate (kg/ha)");
            case Column::IncDepth:             return QString("Inc. Depth (cm)");
            case Column::MassAF:               return QLatin1String("Mass \xd7 AF (kg)");
            case Column::FluxProfile:          return QString("Flux Profile");
            case Column::TimeSF:               return QString("Time SF");
            case Column::DepthSF:              return QString("Depth SF");
            case Column::OverallSF:            return QString("Overall SF");
            case Column::BZDistance:           return QString("BZ Distance (m)");
            case Column::BZDuration:           return QString("BZ Duration (hr)");
            case Column::AirDiffusion:         return QLatin1String("Da (cm\xb2/sec)");
            case Column::WaterDiffusion:       return QLatin1String("Dw (cm\xb2/sec)");
            case Column::CuticularResistance:  return QLatin1String("rcl (s/cm)");
            case Column::HenryConstant:        return QLatin1String("H (Pa-m\xb3/mol)");
            case Column::XInit:                return QString("XInit");
            case Column::YInit:                return QString("YInit");
            case Column::Angle:                return QString("Angle");
            case Column::Radius:               return QString("Radius");
            case Column::VertexCount:          return QString("Vertex Count");
            default: return QVariant();
            }
        }
    }
    else if (orientation == Qt::Vertical) {
        if ((section >= rowCount()) || (section < 0) || (static_cast<std::size_t>(section) >= sgPtr->sources.size()))
            return QVariant();

        const Source &s = sgPtr->sources.at(static_cast<std::size_t>(section));

        if (role == Qt::UserRole)
            return QVariant::fromValue(s.getType());
        else if (role == Qt::ForegroundRole)
            return s.pen;
        else if (role == Qt::BackgroundRole)
            return s.brush;
    }

    return QVariant();
}


bool SourceModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::ForegroundRole && role != Qt::BackgroundRole)
        return false;

    if (orientation != Qt::Vertical)
        return false;

    if (section >= rowCount() || section < 0)
        return false;

    Source &s = sgPtr->sources.at(static_cast<std::size_t>(section));

    if (role == Qt::ForegroundRole && value.canConvert<QPen>()) {
        s.pen = qvariant_cast<QPen>(value);
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    else if (role == Qt::BackgroundRole && value.canConvert<QBrush>()) {
        s.brush = qvariant_cast<QBrush>(value);
        emit headerDataChanged(orientation, section, section);
        return true;
    }

    return false;
}

Qt::ItemFlags SourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    SourceType stype = qvariant_cast<SourceType>(headerData(index.row(), Qt::Vertical, Qt::UserRole));

    Qt::ItemFlags xyFlags = QAbstractTableModel::flags(index);
    Qt::ItemFlags llFlags = QAbstractTableModel::flags(index);
    if (stype != SourceType::AREAPOLY) { // AREAPOLY coordinates must be set using the source editor.
        xyFlags |= Qt::ItemIsEditable;
        if (transform.isValid()) // The GeoTransform must be valid to set latitude/longitude.
            llFlags |= Qt::ItemIsEditable;
    }

    // Set editable flag for selected columns.
    switch (index.column()) {
    case Column::X:
    case Column::Y:
        return xyFlags;
    case Column::Longitude:
    case Column::Latitude:
        return llFlags;
    case Column::Z:
    case Column::ID:
    case Column::Start:
    case Column::AppRate:
    case Column::IncDepth:
    case Column::FluxProfile:
    //case Column::BZDistance: // FIXME
    //case Column::BZDuration: // FIXME
    case Column::AirDiffusion:
    case Column::WaterDiffusion:
    case Column::CuticularResistance:
    case Column::HenryConstant:
    case Column::XInit:
    case Column::YInit:
    case Column::Angle:
    case Column::Radius:
    case Column::VertexCount:
    case Column::EmissionRate:
    case Column::ReleaseHeight:
    case Column::SigmaZ0:
    case Column::VertexData:
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    default:
        return QAbstractTableModel::flags(index);
    }
}

bool SourceModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || static_cast<std::size_t>(row) > sgPtr->sources.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    auto it0 = std::next(sgPtr->sources.begin(), row);
    auto it1 = std::next(it0, count);
    sgPtr->sources.erase(it0, it1);
    endRemoveRows();

    return true;
}

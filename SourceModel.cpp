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
    Q_UNUSED(parent);
    return sgPtr->sources.size();
}

int SourceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 17;
}

QVariant SourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    const Source &s = sgPtr->sources.at(index.row());

    if (role == Qt::UserRole)
    {
        return QVariant();
    }
    else if (role == Qt::ForegroundRole)
    {
        // Monte Carlo parameters: blue
        // Calculated parameters: red
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
    }

    QVariant fluxProfileName;
    QVariant timeScaleFactor;
    QVariant depthScaleFactor;
    QVariant fluxScaleFactor;

    const auto fp = s.fluxProfile.lock();
    if (fp) {
        fluxProfileName = QString::fromStdString(fp->name);
        timeScaleFactor = fp->timeScaleFactor(s.appStart);
        depthScaleFactor = fp->depthScaleFactor(s.incorpDepth);
        fluxScaleFactor = fp->fluxScaleFactor(s.appRate, s.appStart, s.incorpDepth);
    }

    double area = s.area();
    QVariant bufferDistance = QString();
    QVariant bufferDuration = QString();

    if (sgPtr->enableBufferZones) {
        BufferZone zref;
        zref.appRateThreshold = s.appRate;
        zref.areaThreshold = area;

        auto it = std::find_if(sgPtr->zones.begin(), sgPtr->zones.end(), [&zref](const BufferZone& z) {
           return (z.areaThreshold >= zref.areaThreshold) && (z.appRateThreshold >= zref.appRateThreshold);
        });

        if (it != sgPtr->zones.end()) {
            bufferDistance = it->distance;
            bufferDuration = it->duration;
        }
    }

    if (transform.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole) &&
        (index.column() == Column::Longitude || index.column() == Column::Latitude))
    {
        double lon, lat, elev;
        int rc = transform.inverse(s.xs, s.ys, s.zs, lon, lat, elev);
        if (rc != 0)
            return QVariant();

        switch (index.column()) {
        case Column::Longitude:           return lon;
        case Column::Latitude:            return lat;
        default:
            return QVariant();
        }
    }

    if (role == Qt::EditRole)
    {
        switch (index.column()) {
        case Column::ID:                  return QString::fromStdString(s.srcid);
        case Column::X:                   return s.xs;
        case Column::Y:                   return s.ys;
        case Column::Z:                   return s.zs;
        case Column::Start:               return s.appStart;
        case Column::AppRate:             return s.appRate;
        case Column::IncDepth:            return s.incorpDepth;
        case Column::FluxProfile:         return fluxProfileName;
        default: return QVariant();
        }
    }
    else if (role == Qt::DisplayRole)
    {
        switch (index.column()) {
        case Column::ID:                  return QString::fromStdString(s.srcid);
        case Column::X:                   return s.xs;
        case Column::Y:                   return s.ys;
        case Column::Z:                   return s.zs;
        case Column::Area:                return area;
        case Column::Start:               return s.appStart.toString("yyyy-MM-dd HH:mm");
        case Column::AppRate:             return s.appRate;
        case Column::IncDepth:            return s.incorpDepth;
        case Column::MassAF:              return s.appRate * area * sgPtr->appFactor;
        case Column::FluxProfile:         return fluxProfileName;
        case Column::TimeSF:              return timeScaleFactor;
        case Column::DepthSF:             return depthScaleFactor;
        case Column::OverallSF:           return fluxScaleFactor;
        case Column::BZDistance:          return bufferDistance;
        case Column::BZDuration:          return bufferDuration;
        case Column::AirDiffusion:        return s.airDiffusion;
        case Column::WaterDiffusion:      return s.waterDiffusion;
        case Column::CuticularResistance: return s.cuticularResistance;
        case Column::HenryConstant:       return s.henryConstant;
        default: return QVariant();
        }
    }

    return QVariant();
}

bool SourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    Source &s = sgPtr->sources.at(row);

    if (role == Qt::EditRole)
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
        case Column::X: {
            s.xs = value.toDouble();
            s.setGeometry();
            break;
        }
        case Column::Y: {
            s.ys = value.toDouble();
            s.setGeometry();
            break;
        }
        case Column::Z: {
            s.zs = value.toDouble();
            break;
        }
        case Column::Start: {
            s.appStart = value.toDateTime();
            break;
        }
        case Column::AppRate: {
            s.appRate = value.toDouble();
            break;
        }
        case Column::IncDepth: {
            s.incorpDepth = value.toDouble();
            break;
        }
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }
    else if (role == Qt::UserRole)
    {
        switch (index.column()) {
            case 8: {
                int fpIndex = value.toInt();
                if (fpIndex >= 0 && fpIndex < sPtr->fluxProfiles.size()) {
                    s.fluxProfile = sPtr->fluxProfiles[fpIndex];
                }
            }
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant SourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case Column::ID:                  return QString("ID");
            case Column::X:                   return QString("X (m)");
            case Column::Y:                   return QString("Y (m)");
            case Column::Z:                   return QString("Z (m)");
            case Column::Longitude:           return QString("Longitude");
            case Column::Latitude:            return QString("Latitude");
            case Column::Area:                return QString("Area (ha)");
            case Column::Start:               return QString("Start");
            case Column::AppRate:             return QString("App. Rate (kg/ha)");
            case Column::IncDepth:            return QString("Inc. Depth (cm)");
            case Column::MassAF:              return QLatin1String("Mass \xd7 AF (kg)");
            case Column::FluxProfile:         return QString("Flux Profile");
            case Column::TimeSF:              return QString("Time SF");
            case Column::DepthSF:             return QString("Depth SF");
            case Column::OverallSF:           return QString("Overall SF");
            case Column::BZDistance:          return QString("BZ Distance (m)");
            case Column::BZDuration:          return QString("BZ Duration (hr)");
            case Column::AirDiffusion:        return QLatin1String("Da (cm\xb2/sec)");
            case Column::WaterDiffusion:      return QLatin1String("Dw (cm\xb2/sec)");
            case Column::CuticularResistance: return QLatin1String("rcl (s/cm)");
            case Column::HenryConstant:       return QLatin1String("H (Pa-m\xb3/mol)");
            default: return QVariant();
            }
        }
    }
    else {
        if (section >= rowCount() || section < 0)
            return QVariant();

        const Source &s = sgPtr->sources.at(section);

        if (role == Qt::UserRole) {
            return QVariant::fromValue(s.getType());
        }
        else if (role == Qt::ForegroundRole) {
            return s.pen;
        }
        else if (role == Qt::BackgroundRole) {
            return s.brush;
        }
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

    Source &s = sgPtr->sources.at(section);

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

    // AREAPOLY coordinates must be set using the source editor,
    // and the geo transform must be valid to set latitude/longitude.
    QVariant sourceTypeVar = headerData(index.row(), Qt::Vertical, Qt::UserRole);
    SourceType sourceType = sourceTypeVar.value<SourceType>();
    Qt::ItemFlags xyFlags = QAbstractTableModel::flags(index);
    Qt::ItemFlags llFlags = QAbstractTableModel::flags(index);
    if (sourceType != SourceType::AREAPOLY) {
        xyFlags |= Qt::ItemIsEditable;
        if (transform.isValid())
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
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    default: return QAbstractTableModel::flags(index);
    }
}

bool SourceModel::removeRows(int row, int count, const QModelIndex &)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);

    auto it = sgPtr->sources.begin() + row;
    for (int i = 0; i < count; ++i)
        it = sgPtr->sources.erase(it);

    endRemoveRows();

    return true;
}

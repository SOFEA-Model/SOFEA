#include "SourceModel.h"
#include "RunstreamParser.h"
#include "ShapefileParser.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QFileInfo>
#include <QSettings>

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
    return 14;
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

    if (role == Qt::ForegroundRole)
    {
        // Monte Carlo parameters: blue
        // Calculated parameters: red
        switch (index.column()) {
            case 0:  return QVariant();
            case 1:  return QVariant();
            case 2:  return QVariant();
            case 3:  return QVariant(QColor(Qt::red));
            case 4:  return QVariant(QColor(Qt::blue));
            case 5:  return QVariant(QColor(Qt::blue));
            case 6:  return QVariant(QColor(Qt::blue));
            case 7:  return QVariant(QColor(Qt::red));
            case 8:  return QVariant(QColor(Qt::blue));
            case 9:  return QVariant(QColor(Qt::red));
            case 10: return QVariant(QColor(Qt::red));
            case 11: return QVariant(QColor(Qt::red));
            case 12: return QVariant();
            case 13: return QVariant();
            case 14: return QVariant(QColor(Qt::blue));
            case 15: return QVariant(QColor(Qt::blue));
            case 16: return QVariant(QColor(Qt::blue));
            case 17: return QVariant(QColor(Qt::blue));
            default: return QVariant(QColor(Qt::blue));
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

    if (role == Qt::EditRole)
    {
        switch (index.column()) {
            case 0:  return QString::fromStdString(s.srcid);
            case 4:  return s.appStart;
            case 5:  return s.appRate;
            case 6:  return s.incorpDepth;
            case 8:  return fluxProfileName;
            default: return QVariant();
        }
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column()) {
            case 0:  return QString::fromStdString(s.srcid);
            case 1:  return s.xs;
            case 2:  return s.ys;
            case 3:  return area;
            case 4:  return s.appStart.toString("yyyy-MM-dd HH:mm");
            case 5:  return s.appRate;
            case 6:  return s.incorpDepth;
            case 7:  return s.appRate * area * sgPtr->appFactor;
            case 8:  return fluxProfileName;
            case 9:  return timeScaleFactor;
            case 10: return depthScaleFactor;
            case 11: return fluxScaleFactor;
            case 12: return bufferDistance;
            case 13: return bufferDuration;
            case 14: return s.airDiffusion;
            case 15: return s.waterDiffusion;
            case 16: return s.cuticularResistance;
            case 17: return s.henryConstant;
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

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case 0: {
                QString srcid = value.toString();
                // Remove any non-alphanumeric characters and truncate to max length (100)
                srcid.remove(QRegExp("[^A-Za-z0-9_]+"));
                srcid.truncate(100);
                s.srcid = srcid.toStdString();
                break;
            }
            case 4: {
                QDateTime appStart = value.toDateTime();
                s.appStart = appStart;
                break;
            }
            case 5: {
                double appRate = value.toDouble();
                s.appRate = appRate;
                break;
            }
            case 6: {
                double incorpDepth = value.toDouble();
                s.incorpDepth = incorpDepth;
                break;
            }
            default:
                return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    if (role == Qt::UserRole) {
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
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString("ID");
                case 1:  return QString("X (m)");
                case 2:  return QString("Y (m)");
                case 3:  return QString("Area (ha)");
                case 4:  return QString("Start");
                case 5:  return QString("App. Rate (kg/ha)");
                case 6:  return QString("Inc. Depth (cm)");
                case 7:  return QLatin1String("Mass \xd7 AF (kg)");
                case 8:  return QString("Flux Profile");
                case 9:  return QString("Time SF");
                case 10: return QString("Depth SF");
                case 11: return QString("Overall SF");
                case 12: return QString("BZ Distance (m)");
                case 13: return QString("BZ Duration (hr)");
                case 14: return QLatin1String("Da (cm\xb2/sec)");
                case 15: return QLatin1String("Dw (cm\xb2/sec)");
                case 16: return QLatin1String("rcl (s/cm)");
                case 17: return QLatin1String("H (Pa-m\xb3/mol)");
                default: return QVariant();
            }
        }
    }

    if (role == Qt::DecorationRole) {
        if (orientation == Qt::Vertical) {
            if (section >= rowCount() || section < 0)
                return QVariant();

            const Source &s = sgPtr->sources.at(section);

            switch (s.getType()) {
                case SourceType::AREA:
                    return QIcon(":/images/Rectangle_16x.png");
                case SourceType::AREACIRC:
                    return QIcon(":/images/Circle_16x.png");
                case SourceType::AREAPOLY:
                    return QIcon(":/images/Polygon_16x.png");
                default:
                    return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags SourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    // Set editable flag for selected columns.
    switch (index.column()) {
        case 0:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 4:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 5:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 6:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        case 8:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
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

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

#include "MeteorologyModel.h"
#include "MeteorologyStationData.h"
#include "utilities/DateTimeConversion.h"

#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QMetaType>
#include <QMimeData>
#include <QPalette>
#include <QSet>
#include <QSettings>
#include <QString>

#include <QDebug>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <fmt/format.h>

#include <iterator>

MeteorologyModel::MeteorologyModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void MeteorologyModel::addUrls(const QList<QUrl>& urls)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import")

    // Check for both SFC/PFL and extract unique file names without extension.
    QSet<QString> basePaths;
    for (const auto& url : urls)
    {
        QString path = url.toLocalFile();
        if (path.isEmpty())
            continue;

        QFileInfo fi(url.toLocalFile());
        if (!fi.isFile())
            continue;

        QString basePath = QDir::cleanPath(fi.absolutePath() + QDir::separator() + fi.completeBaseName());
        if (basePaths.contains(basePath))
            continue;

        QString ext = fi.suffix().toLower();
        QString paired;
        if (ext == "sfc")
            paired = basePath + ".pfl";
        else if (ext == "pfl")
            paired = basePath + ".sfc";
        else
            continue;

        if (!QFileInfo::exists(paired))
            BOOST_LOG_TRIVIAL(warning) << fmt::format("SFC/PFL pair not found for file '{}'", fi.fileName().toStdString());

        basePaths.insert(basePath);
    }

    for (const auto& path : basePaths) {
        std::string sfc = QString(path + ".sfc").toStdString();
        std::string pfl = QString(path + ".pfl").toStdString();
        try {
            Meteorology item{sfc, pfl};
            initFromDb(item);
            appendRow(item);
        } catch (const std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
    }
}

void MeteorologyModel::appendRow(const Meteorology& item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    data_.push_back(item);
    endInsertRows();
}

void MeteorologyModel::initFromDb(Meteorology& item)
{
    QString id = QString::fromStdString(item.surfaceFile.header().sfloc);
    QDateTime dt = sofea::utilities::convert<QDateTime>(item.surfaceFile.minTime());

    QSqlQuery stationQuery = MeteorologyStationData::stationQuery(id, dt, dt);
    if (stationQuery.exec() && stationQuery.next()) {
        item.name = stationQuery.value(StationQueryModel::Column::StationName).toString().toStdString();
        item.terrainElevation = stationQuery.value(StationQueryModel::Column::GroundElevation).toDouble();

        //qDebug() << stationQuery.value(StationQueryModel::Column::GroundElevationUnit).toString();
        //qDebug() << stationQuery.value(StationQueryModel::Column::HorizontalDatum).toString(); // 35039, 185, NAD83, NAD27, OLD HAWAIIAN, WGS84
        //qDebug() << stationQuery.value(StationQueryModel::Column::VerticalDatum).toString(); // NAGVD29, NAVD88, MSL(HAWAII)
        //qDebug() << stationQuery.value(StationQueryModel::Column::LatLonSource).toString();
    }

    QSqlQuery anemometerQuery = MeteorologyStationData::anemometerQuery(id, dt, dt);
    if (anemometerQuery.exec() && anemometerQuery.next()) {
        item.anemometerHeight = anemometerQuery.value(AnemometerQueryModel::Column::AnemometerHeight).toDouble();

        //qDebug() << anemometerQuery.value(AnemometerQueryModel::Column::AnemometerHeightUnit).toString();
    }
}

int MeteorologyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(data_.size());
}

int MeteorologyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 9;
}

QModelIndex MeteorologyModel::index(int row, int column, const QModelIndex &parent) const
{
    // Create an index for extended columns (column >= columnCount).

    switch (column) {
    case Column::TerrainElevation:
    case Column::AnemometerHeight:
    case Column::WindRotation:
    case Column::SurfaceFile:
    case Column::UpperAirFile:
        return createIndex(row, column);
    default:
        return QAbstractTableModel::index(row, column, parent);
    }
}

QVariant MeteorologyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    auto item = data_.at(index.row());

    if (role == Qt::UserRole) {
        return QVariant();
    }
    else if (role == Qt::EditRole || role == Qt::DisplayRole) {
        switch (index.column()) {
        case Column::Name:             return QString::fromStdString(item.name);
        case Column::SurfaceStation:   return QString::fromStdString(item.surfaceFile.header().sfloc);
        case Column::UpperAirStation:  return QString::fromStdString(item.surfaceFile.header().ualoc);
        case Column::OnSiteStation:    return QString::fromStdString(item.surfaceFile.header().osloc);
        case Column::StartTime: {
            QDateTime dt = sofea::utilities::convert<QDateTime>(item.surfaceFile.minTime());
            return dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : QVariant();
        }
        case Column::EndTime: {
            QDateTime dt = sofea::utilities::convert<QDateTime>(item.surfaceFile.maxTime());
            return dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : QVariant();
        }
        case Column::TotalHours:       return item.surfaceFile.totalHours();
        case Column::CalmHours:        return item.surfaceFile.calmHours();
        case Column::MissingHours:     return item.surfaceFile.missingHours();
        case Column::TerrainElevation: return QString::number(item.terrainElevation, 'f', 1);
        case Column::AnemometerHeight: return QString::number(item.anemometerHeight, 'f', 1);
        case Column::WindRotation:     return QString::number(item.windRotation, 'f', 1);
        case Column::SurfaceFile:      return QString::fromStdString(item.surfaceFile.path());
        case Column::UpperAirFile:     return QString::fromStdString(item.upperAirFile.path());
        default: return QVariant();
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case Column::Name:
            return Qt::AlignLeft;
        case Column::SurfaceStation:
        case Column::UpperAirStation:
        case Column::OnSiteStation:
        case Column::StartTime:
        case Column::EndTime:
        case Column::TotalHours:
        case Column::CalmHours:
        case Column::MissingHours:
            return Qt::AlignCenter;
        default: return QVariant();
        }
    }
    else if (role == Qt::BackgroundRole) {
        switch (index.column()) {
        case Column::SurfaceStation:
        case Column::UpperAirStation:
        case Column::OnSiteStation:
        case Column::StartTime:
        case Column::EndTime:
        case Column::TotalHours:
        case Column::CalmHours:
        case Column::MissingHours:
            return QApplication::palette().color(QPalette::Window).lighter(104);
        default: return QVariant();
        }
    }
    return QVariant();
}

bool MeteorologyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (index.row() >= rowCount() || index.row() < 0)
        return false;

    if (role == Qt::EditRole || role == Qt::DisplayRole)
    {
        auto item = data_.at(static_cast<std::size_t>(index.row()));

        switch (index.column()) {
        case Column::Name:
            item.name = value.toString().simplified().toStdString();
            break;
        case Column::TerrainElevation:
            item.terrainElevation = value.toDouble();
            break;
        case Column::AnemometerHeight:
            item.anemometerHeight = value.toDouble();
            break;
        case Column::WindRotation:
            item.windRotation = value.toDouble();
            break;
        case Column::SurfaceFile:
            //item.surfaceFilePath = value.toString().toStdString();
            break;
        case Column::UpperAirFile:
            //item.upperAirFilePath = value.toString().toStdString();
            break;
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool MeteorologyModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    auto it = roles.find(Qt::EditRole);
    if (it == roles.end())
        return false;

    return setData(index, it.value(), Qt::EditRole);
}

QVariant MeteorologyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case Column::Name:             return tr("Name");
            case Column::SurfaceStation:   return tr("Surface ID");
            case Column::UpperAirStation:  return tr("Upper Air ID");
            case Column::OnSiteStation:    return tr("On Site ID");
            case Column::StartTime:        return tr("Start Time");
            case Column::EndTime:          return tr("End Time");
            case Column::TotalHours:       return tr("Total Hours");
            case Column::CalmHours:        return tr("Calm Hours");
            case Column::MissingHours:     return tr("Missing Hours");
            case Column::TerrainElevation: return tr("Terrain Elevation");
            case Column::AnemometerHeight: return tr("Anemometer Height");
            case Column::WindRotation:     return tr("Wind Rotation");
            case Column::SurfaceFile:      return tr("Surface File");
            case Column::UpperAirFile:     return tr("Upper Air File");
            default: return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags MeteorologyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;

    switch (index.column()) {
    case Column::Name:
    case Column::TerrainElevation:
    case Column::AnemometerHeight:
    case Column::WindRotation:
    case Column::SurfaceFile:
    case Column::UpperAirFile:
        return defaultFlags | Qt::ItemIsEditable;
    default:
        return defaultFlags;
    }
}

bool MeteorologyModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > data_.size())
        return false;

    auto item = data_.at(row);

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    auto it0 = std::next(data_.begin(), row);
    auto it1 = std::next(it0, count);
    data_.erase(it0, it1);
    endRemoveRows();

    return true;
}

bool MeteorologyModel::moveRows(const QModelIndex &, int sourceFirst, int count,
                                const QModelIndex &, int destinationFirst)
{
    if (count <= 0 || sourceFirst == destinationFirst)
        return false;

    int sourceLast = sourceFirst + count;
    int destinationLast = destinationFirst + count;

    if (std::min(sourceFirst, destinationFirst) < 0 ||
        std::max(sourceLast, destinationLast) > static_cast<int>(data_.size()))
        return false;

    // Get the extraction range.
    decltype(data_) range(data_.begin() + sourceFirst,
                          data_.begin() + sourceLast);

    // Notify views of new state.
    // See documentation for QAbstractItemModel::beginMoveRows.
    int destinationChild = destinationFirst < sourceFirst ? destinationFirst
                                                          : destinationLast;
    beginMoveRows(QModelIndex(), sourceFirst, sourceLast - 1,
                  QModelIndex(), destinationChild);

    // Erase and insert.
    data_.erase(data_.begin() + sourceFirst,
                data_.begin() + sourceLast);

    data_.insert(data_.begin() + destinationFirst,
                 range.begin(), range.end());

    endMoveRows();

    return true;
}

Qt::DropActions MeteorologyModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

bool MeteorologyModel::canDropMimeData(const QMimeData *mimeData, Qt::DropAction, int, int, const QModelIndex&) const
{
    if (mimeData->hasUrls())
        return true;
    return false;
}

bool MeteorologyModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction, int, int, const QModelIndex&)
{
    return true;
}

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

#include <QApplication>
#include <QMetaType>
#include <QSettings>
#include <QString>

#include <fmt/format.h>

MeteorologyModel::MeteorologyModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

int MeteorologyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return static_cast<int>(data_.size());
}

int MeteorologyModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 10;
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
        case Column::SurfaceFile:      return QString::fromStdString(item.surfaceFilePath);
        case Column::UpperAirFile:     return QString::fromStdString(item.upperAirFilePath);
        case Column::AnemometerHeight: return item.anemometerHeight;
        case Column::WindRotation:     return item.windRotation;
        default: return QVariant();
        }
    }

    return QVariant();
}

bool MeteorologyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid())
    {
        auto item = data_.at(index.row());

        switch (index.column()) {
        case Column::Name:
            item.name = value.toString().simplified().toStdString();
            break;
        case Column::SurfaceFile:
            item.surfaceFilePath = value.toString().toStdString();
            break;
        case Column::UpperAirFile:
            item.upperAirFilePath = value.toString().toStdString();
            break;
        case Column::AnemometerHeight:
            item.anemometerHeight = value.toDouble();
            break;
        case Column::WindRotation:
            item.windRotation = value.toDouble();
            break;
        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant MeteorologyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case Column::Name:             return QString("Name");
            case Column::SurfaceFile:      return QString("Surface File");
            case Column::UpperAirFile:     return QString("Upper Air File");
            case Column::AnemometerHeight: return QString("Anemometer Height");
            case Column::WindRotation:     return QString("Wind Rotation");
            case Column::StartTime:        return QString("Start Time");
            case Column::EndTime:          return QString("End Time");
            case Column::SurfaceStation:   return QString("Surface Station");
            case Column::UpperAirStation:  return QString("Upper Air Station");
            case Column::OnSiteStation:    return QString("On Site Station");
            default: return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags MeteorologyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    switch (index.column()) {
    case Column::Name:
    case Column::SurfaceFile:
    case Column::UpperAirFile:
    case Column::AnemometerHeight:
    case Column::WindRotation:
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    default:
        return QAbstractTableModel::flags(index);
    }
}

bool MeteorologyModel::insertRows(int row, int count, const QModelIndex &)
{
    static unsigned int seq = 0;

    if (row < 0 || row > data_.size())
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        Meteorology item;
        item.name = fmt::format("MetData{:0=2}", ++seq);
        data_.push_back(item);
    }
    endInsertRows();

    return true;
}

bool MeteorologyModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > data_.size())
        return false;

    auto item = data_.at(row);

    auto it = data_.begin() + row;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        it = data_.erase(it);
    }
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

    if (sourceLast > data_.size() || destinationLast > data_.size())
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

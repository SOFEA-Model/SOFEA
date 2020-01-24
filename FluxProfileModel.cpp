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

#include "FluxProfileModel.h"
#include "FluxProfileDialog.h"

#include <QApplication>
#include <QMetaType>
#include <QSettings>
#include <QString>

#include <QDebug>

#include <fmt/format.h>

QString dsMethodString(FluxProfile::DSMethod method)
{
    switch (method) {
        case FluxProfile::DSMethod::Disabled:      return QString("Disabled");
        case FluxProfile::DSMethod::LinearCDPR:    return QString("Linear (CDPR)");
        case FluxProfile::DSMethod::LinearGeneral: return QString("Linear (General)");
        case FluxProfile::DSMethod::Nonlinear:     return QString("Nonlinear");
    }

    return QString();
}

QString tsMethodString(FluxProfile::TSMethod method)
{
    switch (method) {
        case FluxProfile::TSMethod::Disabled:      return QString("Disabled");
        case FluxProfile::TSMethod::Seasonal:      return QString("Seasonal (CDPR)");
        case FluxProfile::TSMethod::Sinusoidal:    return QString("Sinusoidal");
    }

    return QString();
}

FluxProfileModel::FluxProfileModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void FluxProfileModel::save(std::vector<std::shared_ptr<FluxProfile>>& profiles)
{
    profiles.clear();
    profiles.reserve(localData.size());

    for (auto local : localData) {
        auto source = localToSource.at(local);
        if (local != nullptr) {
            *source = *local;
            profiles.push_back(source);
        }
    }
}

void FluxProfileModel::load(const std::vector<std::shared_ptr<FluxProfile>>& profiles)
{
    beginResetModel();

    // Initialize local model data with deep copy.

    localData.clear();
    localToSource.clear();
    sourceToLocal.clear();

    localData.reserve(profiles.size());
    for (const auto source : profiles) {
        auto local = std::make_shared<FluxProfile>(*source);
        localToSource[local] = source;
        sourceToLocal[source] = local;
        localData.push_back(local);
    }

    endResetModel();
}

void FluxProfileModel::showEditor(const QModelIndex &index, QWidget *parent)
{
    auto fp = localData.at(index.row());
    FluxProfileDialog dialog(fp, parent);
    int rc = dialog.exec();
    if (rc == QDialog::Accepted) {
        QModelIndex topLeft = this->index(index.row(), 0);
        QModelIndex bottomRight = this->index(index.row(), this->columnCount() - 1);
        emit dataChanged(topLeft, bottomRight);
    }
}

std::shared_ptr<FluxProfile> FluxProfileModel::fluxProfileFromIndex(const QModelIndex &index)
{
    // Returns the original flux profile.
    auto local = localData.at(index.row());
    return localToSource.at(local);
}

int FluxProfileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(localData.size());
}

int FluxProfileModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant FluxProfileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    auto fp = localData.at(index.row());

    if (role == Qt::UserRole) {
        return QVariant();
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(fp->name);
            default: return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:  return QString::fromStdString(fp->name);
            case 1:  return fp->totalHours();
            case 2:  return tsMethodString(fp->tsMethod);
            case 3:  return dsMethodString(fp->dsMethod);
            default: return QVariant();
        }
    }

    return QVariant();
}

bool FluxProfileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid())
    {
        auto fp = localData.at(index.row());
        switch (index.column()) {
            case 0: {
                QString name = value.toString();
                // Remove whitespace and truncate to max length (100)
                name = name.simplified();
                name.truncate(100);
                fp->name = name.toStdString();
                break;
            }
            default:
                return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant FluxProfileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:  return QString("Profile");
                case 1:  return QString("Hours");
                case 2:  return QString("Time Scaling");
                case 3:  return QString("Depth Scaling");
                default: return QVariant();
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags FluxProfileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    // Set editable flag for name column.
    switch (index.column()) {
        case 0:  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
        default: return QAbstractTableModel::flags(index);
    }
}

bool FluxProfileModel::insertRows(int row, int count, const QModelIndex &)
{
    static unsigned int seq = 0;

    if (row < 0 || row > localData.size())
        return false;

    auto it = localData.begin() + row;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i=0; i < count; ++i) {
        auto fp = std::make_shared<FluxProfile>();
        fp->name = fmt::format("Profile{:0=2}", ++seq);
        localToSource[fp] = fp;
        sourceToLocal[fp] = fp;
        localData.insert(it, fp);
    }
    endInsertRows();

    return true;
}

bool FluxProfileModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || row > localData.size())
        return false;

    auto local = localData.at(row);
    auto source = localToSource.at(local);
    sourceToLocal.at(source) = nullptr;

    auto it = localData.begin() + row;
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        it = localData.erase(it);
    }
    endRemoveRows();

    return true;
}

bool FluxProfileModel::moveRows(const QModelIndex &, int sourceFirst, int count,
                                const QModelIndex &, int destinationFirst)
{
    if (count <= 0 || sourceFirst == destinationFirst)
        return false;

    int sourceLast = sourceFirst + count;
    int destinationLast = destinationFirst + count;

    if (sourceLast > localData.size() || destinationLast > localData.size())
        return false;

    // Get the extraction range.
    decltype(localData) range(localData.begin() + sourceFirst,
                              localData.begin() + sourceLast);

    // Notify views of new state.
    // See documentation for QAbstractItemModel::beginMoveRows.
    int destinationChild = destinationFirst < sourceFirst ? destinationFirst
                                                          : destinationLast;
    beginMoveRows(QModelIndex(), sourceFirst, sourceLast - 1,
                  QModelIndex(), destinationChild);

    // Erase and insert.
    localData.erase(localData.begin() + sourceFirst,
                    localData.begin() + sourceLast);

    localData.insert(localData.begin() + destinationFirst,
                     range.begin(), range.end());

    endMoveRows();

    return true;
}

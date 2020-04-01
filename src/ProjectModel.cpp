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

#include "ProjectModel.h"
#include "core/Project.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

#include <QApplication>
#include <QFont>
#include <QDebug>

const std::set<ProjectModel::ItemType> itemTypes {
    ProjectModel::ItemType::Scenarios,
    ProjectModel::ItemType::Meteorology,
    ProjectModel::ItemType::FluxProfiles,
    ProjectModel::ItemType::SourceGroups,
    ProjectModel::ItemType::RunConfigurations
};

ProjectModel::ProjectModel(QObject *parent)
    : QAbstractItemModel(parent)
{}

ProjectModel::~ProjectModel()
{}

void ProjectModel::loadProject(const QString& filename)
{

}

void ProjectModel::saveProject(const QString& filename)
{

}

void ProjectModel::setProject(std::shared_ptr<Project> project)
{
    beginResetModel();
    project_ = project;
    endResetModel();
}

std::shared_ptr<Project> ProjectModel::project()
{
    return project_;
}

void ProjectModel::clear()
{
    beginResetModel();
    project_ = std::make_shared<Project>();
    endResetModel();
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return static_cast<int>(itemTypes.size());
    }
    else if (parent.internalId() == 0) {
        ItemType internalId = *std::next(itemTypes.begin(), parent.row());
        switch (internalId) {
        case ItemType::Scenarios:
            return static_cast<int>(project_->scenarios.size());
        case ItemType::Meteorology:
            return static_cast<int>(project_->meteorology.size());
        case ItemType::FluxProfiles:
            return static_cast<int>(project_->fluxProfiles.size());
        case ItemType::SourceGroups:
            return static_cast<int>(project_->sourceGroups.size());
        case ItemType::RunConfigurations:
            return 0;
        }
    }

    return 0;
}

int ProjectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

bool ProjectModel::hasChildren(const QModelIndex &index) const
{
    if (index.internalId() == 0) {
        return true;
    }
    return false;
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid() && row >= 0 && row < itemTypes.size()) {
        // Parent internal ID is always zero.
        return createIndex(row, column, (quintptr) 0);
    }
    else if (parent.isValid() && parent.row() < itemTypes.size()) {
        // Child internal ID is the parent item type.
        ItemType internalId = *std::next(itemTypes.begin(), parent.row());
        return createIndex(row, column, internalId);
    }

    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const
{
    if (index.isValid() && index.internalId() > 0) {
        auto it = itemTypes.find(static_cast<ProjectModel::ItemType>(index.internalId()));
        if (it != itemTypes.end()) {
            int parentRow = std::distance(itemTypes.begin(), it);
            return createIndex(parentRow, 0, (quintptr) 0);
        }
    }

    return QModelIndex();
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::FontRole)
        return QVariant();

    if (role == Qt::FontRole) {
        QFont font = QApplication::font();
        font.setPointSizeF(font.pointSizeF() + 1);
        font.setBold(!index.parent().isValid());
        return font;
    }

    if (!index.parent().isValid() && index.row() < itemTypes.size()) {
        ItemType internalId = *std::next(itemTypes.begin(), index.row());
        switch (internalId) {
        case ItemType::Scenarios:
            return QString(tr("Scenarios"));
        case ItemType::Meteorology:
            return QString(tr("Meteorology"));
        case ItemType::FluxProfiles:
            return QString(tr("Flux Profiles"));
        //case ItemType::BufferZones:
        //    return QString(tr("Buffer Zones"));
        //case ItemType::Distributions:
        //    return QString(tr("Distributions"));
        case ItemType::SourceGroups:
            return QString(tr("Source Groups"));
        //case ItemType::ReceptorGroups:
        //    return QString(tr("Receptor Groups"));
        case ItemType::RunConfigurations:
            return QString(tr("Run Configurations"));
        }
    }
    else if (itemTypes.count(static_cast<ProjectModel::ItemType>(index.internalId()))) {
        switch (index.internalId()) {
        case ItemType::Scenarios:
            return QString::fromStdString(project_->scenarios.at(index.row())->name);
        case ItemType::Meteorology:
            return QString::fromStdString(project_->meteorology.at(index.row())->name);
        case ItemType::FluxProfiles:
            return QString::fromStdString(project_->fluxProfiles.at(index.row())->name);
        //case ItemType::BufferZones:
        //    return QString::fromStdString(project_->bufferZones.at(index.row())->name);
        //case ItemType::Distributions:
        //    return QString::fromStdString(project_->distributions.at(index.row())->name);
        case ItemType::SourceGroups:
            return QString::fromStdString(project_->sourceGroups.at(index.row())->grpid);
        //case ItemType::ReceptorGroups:
        //    return QString::fromStdString(project_->receptorGroups.at(index.row())->name);
        case ItemType::RunConfigurations:
            return QVariant();
        }
    }

    return QVariant();
}

bool ProjectModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!index.isValid() || !index.parent().isValid())
        return false;

    if (!itemTypes.count(static_cast<ProjectModel::ItemType>(index.internalId())))
        return false;

    switch (index.internalId()) {
    case ItemType::Scenarios:
        project_->scenarios.at(index.row())->name = value.toString().toStdString();
        break;
    case ItemType::Meteorology:
        project_->meteorology.at(index.row())->name = value.toString().toStdString();
        break;
    case ItemType::FluxProfiles:
        project_->fluxProfiles.at(index.row())->name = value.toString().toStdString();
        break;
    case ItemType::SourceGroups:
        project_->sourceGroups.at(index.row())->grpid = value.toString().toStdString();
        break;
    case ItemType::RunConfigurations:
        break;
    }

    project_->modified = true;
    return true;
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (!index.parent().isValid())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

template <typename T>
inline void insertRange(std::vector<std::shared_ptr<T>>& container, int row, int count)
{
    auto it = std::next(container.begin(), row);
    container.insert(it, count, std::make_shared<T>());
}

bool ProjectModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid() || parent.internalId() != 0 || parent.row() >= itemTypes.size())
        return false;

    if (row < 0 || count < 1 || row + count > rowCount(parent))
        return false;

    beginInsertRows(parent, row, row + count - 1);

    ItemType internalId = *std::next(itemTypes.begin(), parent.row());
    switch (internalId) {
    case ItemType::Scenarios:
        insertRange(project_->scenarios, row, count);
        break;
    case ItemType::Meteorology:
        //insertRange(project_->meteorology, row, count);
        break;
    case ItemType::FluxProfiles:
        insertRange(project_->fluxProfiles, row, count);
        break;
    case ItemType::SourceGroups:
        insertRange(project_->sourceGroups, row, count);
        break;
    case ItemType::RunConfigurations:
        break;
    }

    endInsertRows();

    project_->modified = true;
    return true;
}

template <typename T>
inline void removeRange(std::vector<std::shared_ptr<T>>& container, int row, int count)
{
    auto it0 = std::next(container.begin(), row);
    auto it1 = std::next(container.begin(), row + count);
    container.erase(it0, it1);
}

bool ProjectModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!parent.isValid() || parent.internalId() != 0 || parent.row() >= itemTypes.size())
        return false;

    if (row < 0 || count < 1 || row + count > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);

    ItemType internalId = *std::next(itemTypes.begin(), parent.row());
    switch (internalId) {
    case ItemType::Scenarios:
        removeRange(project_->scenarios, row, count);
        break;
    case ItemType::Meteorology:
        removeRange(project_->meteorology, row, count);
        break;
    case ItemType::FluxProfiles:
        removeRange(project_->fluxProfiles, row, count);
        break;
    case ItemType::SourceGroups:
        removeRange(project_->sourceGroups, row, count);
        break;
    case ItemType::RunConfigurations:
        break;
    }

    endRemoveRows();

    project_->modified = true;
    return true;
}

template <typename T>
inline void moveRange(std::vector<std::shared_ptr<T>>& container,
                      int sourceFirst, int count, int destinationFirst)
{
    // Get the extraction range.
    auto it0 = std::next(container.begin(), sourceFirst);
    auto it1 = std::next(container.begin(), sourceFirst + count);
    std::vector<std::shared_ptr<T>> range(it0, it1);

    // Erase and insert.
    container.erase(it0, it1);
    auto it = std::next(container.begin(), destinationFirst);
    container.insert(it, range.begin(), range.end());
}

bool ProjectModel::moveRows(const QModelIndex &sourceParent, int sourceFirst, int count,
                            const QModelIndex &destinationParent, int destinationFirst)
{
    if (count < 1 || sourceFirst < 0 || destinationFirst < 0)
        return false;

    if (sourceFirst == destinationFirst)
        return false;

    int sourceLast = sourceFirst + count;
    int destinationLast = destinationFirst + count;

    if (sourceLast > rowCount(sourceParent) ||
        destinationLast > rowCount(destinationParent))
        return false;

    if (sourceParent != destinationParent)
        return false;

    // Notify views of new state.
    // See documentation for QAbstractItemModel::beginMoveRows.
    int destinationChild = destinationFirst < sourceFirst ? destinationFirst
                                                          : destinationLast;

    beginMoveRows(sourceParent, sourceFirst, sourceLast - 1,
                  destinationParent, destinationChild);

    ItemType internalId = *std::next(itemTypes.begin(), sourceParent.row());
    switch (internalId) {
    case ItemType::Scenarios:
        moveRange(project_->scenarios, sourceFirst, count, destinationFirst);
        break;
    case ItemType::Meteorology:
        moveRange(project_->scenarios, sourceFirst, count, destinationFirst);
        break;
    case ItemType::FluxProfiles:
        moveRange(project_->scenarios, sourceFirst, count, destinationFirst);
        break;
    case ItemType::SourceGroups:
        moveRange(project_->scenarios, sourceFirst, count, destinationFirst);
        break;
    case ItemType::RunConfigurations:
        return false;
    default:
        return false;
    }

    endMoveRows();

    project_->modified = true;
    return true;
}

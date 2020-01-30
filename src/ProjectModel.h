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

#pragma once

#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QString>
#include <QVariant>

#include "Project.h"

class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum ItemType: quintptr {
        Scenarios = 1,
        Meteorology,
        FluxProfiles,
        SourceGroups,
        RunConfigurations
    };

    ProjectModel(QObject *parent = nullptr);
    ~ProjectModel();

    void loadProject(const QString& filename);
    void saveProject(const QString& filename);
    void setProject(std::shared_ptr<Project> project);
    std::shared_ptr<Project> project();
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& index = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceFirst, int count,
                  const QModelIndex &destinationParent, int destinationFirst) override;

private:
    std::shared_ptr<Project> project_;
};

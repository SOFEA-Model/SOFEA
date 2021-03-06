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
#include <QVariant>

#include "core/Receptor.h"

struct SourceGroup;

class ReceptorModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        Color,
        Group,
        X,
        Y,
        Z,
        ZHill,
        ZFlag
    };

    ReceptorModel(QObject *parent = nullptr);
    ~ReceptorModel();

    void save(std::vector<ReceptorGroup>& groups) const;
    void load(const std::vector<ReceptorGroup>& groups);
    void addGroup(const ReceptorGroup& group);
    void addReceptor(const QModelIndex& parent, const ReceptorNode& node);
    void updateRing(const QModelIndex& index, double buffer, double spacing, std::shared_ptr<SourceGroup> sg);
    void updateGrid(const QModelIndex& index, double xInit, double yInit, int xCount, int yCount, double xDelta, double yDelta);
    void updateZElev(const QModelIndex& index, double zElev);
    void updateZHill(const QModelIndex& index, double zHill);
    void updateZFlag(const QModelIndex& index, double zFlag);
    ReceptorGroup& groupFromIndex(const QModelIndex& index);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& index = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    std::vector<ReceptorGroup> localData;
};

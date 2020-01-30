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

#ifndef BUFFERZONEMODEL_H
#define BUFFERZONEMODEL_H

#include <set>

#include <QAbstractTableModel>
#include <QVariant>

#include "BufferZone.h"

class BufferZoneModel : public QAbstractTableModel
{
public:
    explicit BufferZoneModel(QObject *parent = nullptr);

    void save(std::set<BufferZone>& zones) const;
    void load(const std::set<BufferZone>& zones);
    void insert(const BufferZone& zone);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;

private:
    std::set<BufferZone> m_zones;
};

#endif // BUFFERZONEMODEL_H

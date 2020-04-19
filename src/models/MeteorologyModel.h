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

#ifndef METEOROLOGYMODEL_H
#define METEOROLOGYMODEL_H

#include <map>
#include <memory>
#include <vector>

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QUrl>
#include <QVariant>

#include "core/Meteorology.h"

class MeteorologyModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        Name,
        SurfaceStation,
        UpperAirStation,
        OnSiteStation,
        StartTime,
        EndTime,
        TotalHours,
        CalmHours,
        MissingHours,
        TerrainElevation,
        AnemometerHeight,
        WindRotation,
        SurfaceFile,
        UpperAirFile,
    };

    MeteorologyModel(QObject *parent = nullptr);

    void appendRow(const Meteorology& item);
    void addUrls(const QList<QUrl>& urls);
    void initFromDb(Meteorology& item);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    bool setItemData(const QModelIndex& index, const QMap<int, QVariant> &roles) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& index = QModelIndex()) override;
    bool moveRows(const QModelIndex& sourceParent, int sourceFirst, int count,
                  const QModelIndex& destinationParent, int destinationFirst) override;
    Qt::DropActions supportedDropActions() const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                         const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                      const QModelIndex& parent) override;

private:
    std::vector<Meteorology> data_;
};

#endif // METEOROLOGYMODEL_H

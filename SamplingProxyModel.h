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

#ifndef SAMPLINGPROXYMODEL_H
#define SAMPLINGPROXYMODEL_H

#include <map>

#include <QIdentityProxyModel>
#include <QItemSelection>

class SamplingProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    explicit SamplingProxyModel(QObject *parent = nullptr);

    int extraColumnForProxyColumn(int proxyColumn) const;
    int proxyColumnForExtraColumn(int extraColumn) const;

    // Overrides
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &i) const override;

public slots:
    void normalize();

private:
    bool normalizeInternal(std::map<QModelIndex, double>& map) const;

    std::map<QModelIndex, double> m_modelData;
};

#endif // SAMPLINGPROXYMODEL_H

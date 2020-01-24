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

#include "SamplingProxyModel.h"

#include <QDebug>

SamplingProxyModel::SamplingProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{}

int SamplingProxyModel::extraColumnForProxyColumn(int proxyColumn) const
{
    const int sourceColumnCount = sourceModel()->columnCount();
    if (proxyColumn >= sourceColumnCount) {
        return proxyColumn - sourceColumnCount;
    }
    return -1;
}

int SamplingProxyModel::proxyColumnForExtraColumn(int extraColumn) const
{
    return sourceModel()->columnCount() + extraColumn;
}

void SamplingProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QIdentityProxyModel::setSourceModel(sourceModel);
}

QModelIndex SamplingProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();

    if (proxyIndex.column() >= sourceModel()->columnCount()) {
        // Create a fake index for the extra column.
        const QModelIndex newIndex = createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());
        return QIdentityProxyModel::mapToSource(newIndex);
    }

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

int SamplingProxyModel::columnCount(const QModelIndex &parent) const
{
    return QIdentityProxyModel::columnCount(parent) + 1;
}

QVariant SamplingProxyModel::data(const QModelIndex &index, int role) const
{
    const int extraCol = extraColumnForProxyColumn(index.column());
    if (extraCol < 0)
        return sourceModel()->data(mapToSource(index), role);

    if (extraCol == 0) {
        if (role == Qt::EditRole || role == Qt::DisplayRole) {
            if (m_modelData.find(index) != m_modelData.end()) {
                double value = m_modelData.at(index);
                return QVariant(value);
            } else {
                return QVariant(0.0);
            }
        }
    }

    return QVariant();
}

bool SamplingProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Only allow editing extra columns.
    const int extraCol = extraColumnForProxyColumn(index.column());
    if (extraCol != 0)
        return false;

    if (role == Qt::EditRole) {
        bool ok = false;
        double p = value.toDouble(&ok);
        if (ok) {
            m_modelData[index] = value.toDouble();
            emit dataChanged(index, index);
            return true;
        }
    }

    return false;
}

QVariant SamplingProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const int extraCol = extraColumnForProxyColumn(section);
    if (extraCol < 0) {
        return QIdentityProxyModel::headerData(section, orientation, role);
    }

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (extraCol == 0) {
                return QString("Probability");
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags SamplingProxyModel::flags(const QModelIndex &index) const
{
    const int extraCol = extraColumnForProxyColumn(index.column());
    if (extraCol < 0) {
        return sourceModel()->flags(mapToSource(index)) & (~Qt::ItemIsEditable) & (~Qt::ItemIsEnabled);
    }
    else if (extraCol == 0) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    return Qt::NoItemFlags;
}

QModelIndex SamplingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    const int extraCol = extraColumnForProxyColumn(column);
    if (extraCol >= 0) {
        // We store the internal pointer of the index for column 0 in the proxy index for extra columns.
        // This will be useful in the parent method.
        return createIndex(row, column, QIdentityProxyModel::index(row, 0, parent).internalPointer());
    }
    return QIdentityProxyModel::index(row, column, parent);
}

QModelIndex SamplingProxyModel::sibling(int row, int column, const QModelIndex &i) const
{
    if (row == i.row() && column == i.column()) {
        return i;
    }
    return index(row, column, parent(i));
}

void SamplingProxyModel::normalize()
{
    if (!normalizeInternal(m_modelData))
        return;

    int column = proxyColumnForExtraColumn(0);
    QModelIndex firstIndex = index(0, column);
    QModelIndex lastIndex = index(0, rowCount() - 1);
    emit dataChanged(firstIndex, lastIndex);
}

bool SamplingProxyModel::normalizeInternal(std::map<QModelIndex, double>& map) const
{
    const double sum = std::accumulate(map.begin(), map.end(), 0.0,
        [](auto &a, auto &i) { return a + i.second; });

    if (sum == 0)
        return false;

    for (auto &i : map)
        i.second = i.second / sum;

    return true;
}

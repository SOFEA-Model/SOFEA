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

#include "FilterProxyModel.h"

#include <QObject>

#include <QDebug>

FilterProxyModel::FilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

void FilterProxyModel::setFilteredRows(const std::set<int>& filtered)
{
    filteredRows = filtered;
    invalidateFilter();
}

void FilterProxyModel::clearFilteredRows()
{
    filteredRows.clear();
    invalidateFilter();
}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool accept = filteredRows.find(sourceRow) == filteredRows.end();
    return accept;
}

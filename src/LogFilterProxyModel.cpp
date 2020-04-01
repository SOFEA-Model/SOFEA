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

#include "LogFilterProxyModel.h"

#include <QModelIndex>

#include <boost/log/trivial.hpp>

LogFilterProxyModel::LogFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void LogFilterProxyModel::setTagVisible(const QString& tag, bool visible)
{
    tagVisibility[tag] = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setErrorsVisible(bool visible)
{
    errorsVisible = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setMessagesVisible(bool visible)
{
    messagesVisible = visible;
    invalidateFilter();
}

void LogFilterProxyModel::setWarningsVisible(bool visible)
{
    warningsVisible = visible;
    invalidateFilter();
}

bool LogFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    // Severity is the UserRole associated with column 0.
    QModelIndex severityIndex = sourceModel()->index(row, 0, parent);
    int severity = sourceModel()->data(severityIndex, Qt::UserRole).toInt();

    QModelIndex tagIndex = sourceModel()->index(row, filterKeyColumn(), parent);
    QString tag = sourceModel()->data(tagIndex, Qt::DisplayRole).toString();

    bool accept = true;
    switch (severity) {
    case boost::log::trivial::trace:
    case boost::log::trivial::debug:
    case boost::log::trivial::info:
        accept = messagesVisible;
        break;
    case boost::log::trivial::warning:
        accept = warningsVisible;
        break;
    case boost::log::trivial::error:
    case boost::log::trivial::fatal:
        accept = errorsVisible;
        break;
    default:
        break;
    }

    // Exclude any tags where visibility is explicitly set to false.
    accept &= tagVisibility.value(tag, true);

    return accept;
}

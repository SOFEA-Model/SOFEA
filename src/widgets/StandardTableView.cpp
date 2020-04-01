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

#include "StandardTableView.h"

#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include <QHeaderView>
#include <QModelIndex>
#include <QStandardItem>

#include <QDebug>

#include "FilterHeaderView.h"

StandardTableView::StandardTableView(QWidget *parent) : QTableView(parent)
{
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setHorizontalHeader(new FilterHeaderView(Qt::Horizontal, this));
    initHeaderStyles();
}

void StandardTableView::setAutoFilterEnabled(bool enabled)
{
    FilterHeaderView *horizontal = qobject_cast<FilterHeaderView *>(horizontalHeader());
    if (horizontal) {
        horizontal->setAutoFilterEnabled(enabled);
        setSortingEnabled(enabled);
    }
}

void StandardTableView::initHeaderStyles()
{
    QHeaderView *horizontal = horizontalHeader();
    QHeaderView *vertical = verticalHeader();

    QFont headerFont = QApplication::font();
    //QFontMetrics fm{headerFont};
    //int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, this);
    //int height = fm.height() + margin * 2;

    horizontal->setFont(headerFont);
    horizontal->setFixedHeight(32);
    horizontal->setStretchLastSection(true);

    vertical->setFont(headerFont);
    vertical->setSectionResizeMode(QHeaderView::Fixed);
    vertical->setDefaultSectionSize(24);
    vertical->setVisible(false);
}

QModelIndexList StandardTableView::selectedRows() const
{
    QModelIndexList rows;
    if (!selectionModel()->hasSelection())
        return rows;

    if (selectionBehavior() == QAbstractItemView::SelectRows) {
        // Use QItemSelectionModel::selectedRows
        rows = selectionModel()->selectedRows();
    }
    else {
        // Use QItemSelectionModel::selectedIndexes and remove duplicates
        rows = selectionModel()->selectedIndexes();
        auto it = std::unique(rows.begin(), rows.end(),
            [](const QModelIndex& a, const QModelIndex& b) {
            return a.row() == b.row();
        });
        rows.erase(it, rows.end());
    }

    if (rows.count() > 1) {
        // Ensure rows are ordered.
        std::sort(rows.begin(), rows.end(),
            [](const QModelIndex& a, const QModelIndex& b) {
            return a.row() < b.row();
        });
    }

    return rows;
}

bool StandardTableView::appendRow()
{
    if (!model())
        return false;

    int nrows = model()->rowCount();
    return model()->insertRow(nrows);
}

void StandardTableView::selectLastRow()
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    if (nrows > 0) {
        scrollToBottom();
        selectRow(nrows - 1);
    }
}

bool StandardTableView::removeSelectedRows()
{
    if (!model() || !selectionModel())
        return false;

    if (!selectionModel()->hasSelection())
        return false;

    QModelIndexList selectedRows;
    if (selectionBehavior() == QAbstractItemView::SelectRows) {
        selectedRows = selectionModel()->selectedRows();
    }
    else {
        selectedRows = selectionModel()->selectedIndexes();
        auto it = std::unique(selectedRows.begin(), selectedRows.end(),
            [](const QModelIndex& a, const QModelIndex& b)->bool {
            return a.row() == b.row();
        });
        selectedRows.erase(it, selectedRows.end());
    }

    if (selectedRows.size() == 0)
        return false;

    // Sort selection descending.
    if (selectedRows.size() > 1) {
        std::sort(selectedRows.begin(), selectedRows.end(),
            [](const auto& a, const auto& b) {
                return a.row() > b.row();
            });
    }

    // Partition selected rows into contiguous ranges.
    std::vector<std::pair<int, int>> remove;
    auto current = selectedRows.begin();
    while (current != selectedRows.end()) {
        auto next = std::adjacent_find(current, selectedRows.end(),
            [](const auto& a, const auto& b) {
                return a.row() - b.row() > 1;
            });

        if (next != selectedRows.end()) {
            auto start = next;
            int count = static_cast<int>(std::distance(current, std::next(next)));
            remove.emplace_back(std::make_pair(start->row(), count));
            current = std::next(next);
        }
        else {
            auto start = std::prev(next);
            int count = static_cast<int>(std::distance(current, next));
            remove.emplace_back(std::make_pair(start->row(), count));
            break;
        }
    }

    // Remove all rows in the selection.
    QModelIndex parent = selectedRows.front().parent();
    for (const auto& p : remove) {
        model()->removeRows(p.first, p.second, parent);
    }

    return true;
}

bool StandardTableView::moveSelectedRows(int offset)
{
    if (!model() || !selectionModel() || offset == 0)
        return false;

    if (!selectionModel()->hasSelection())
        return false;

    QModelIndexList selectedRows;
    if (selectionBehavior() == QAbstractItemView::SelectRows) {
        selectedRows = selectionModel()->selectedRows();
    }
    else {
        selectedRows = selectionModel()->selectedIndexes();
        auto it = std::unique(selectedRows.begin(), selectedRows.end(),
            [](const QModelIndex& a, const QModelIndex& b)->bool {
            return a.row() == b.row();
        });
        selectedRows.erase(it, selectedRows.end());
    }

    if (selectedRows.size() == 0)
        return false;

    // Sort selection descending.
    if (selectedRows.size() > 1) {
        std::sort(selectedRows.begin(), selectedRows.end(),
            [](const auto& a, const auto& b) {
                return a.row() > b.row();
            });
    }

    int nrows = model()->rowCount();
    if (nrows == 1)
        return false;

    if (offset < 0) {
        // Check for valid move up.
        int start = selectedRows.last().row();
        if (start + offset < 0)
            return false;
    }
    else {
        // Check for valid move down.
        int start = selectedRows.first().row();
        if (start + offset >= nrows)
            return false;
    }

    // Partition selected rows into contiguous ranges.
    std::vector<std::pair<int, int>> move; // start, count
    auto current = selectedRows.begin();
    while (current != selectedRows.end()) {
        auto next = std::adjacent_find(current, selectedRows.end(),
            [](const auto& a, const auto& b) {
                return (a.row() - b.row()) > 1;
            });

        if (next != selectedRows.end()) {
            auto start = next;
            int count = static_cast<int>(std::distance(current, std::next(next)));
            move.emplace_back(std::make_pair(start->row(), count));
            current = std::next(next);
        }
        else {
            auto start = std::prev(next);
            int count = static_cast<int>(std::distance(current, next));
            move.emplace_back(std::make_pair(start->row(), count));
            break;
        }
    }

    // Move all rows in the selection.
    QModelIndex parent = selectedRows.front().parent();
    if (offset > 0) {
        // Use reverse iterator for move up.
        for (auto p = move.crbegin(); p != move.crend(); ++p)
            model()->moveRows(parent, p->first, p->second, parent, p->first + offset);
    }
    else {
        // Use forward iterator for move down.
        for (auto p = move.cbegin(); p != move.cend(); ++p)
            model()->moveRows(parent, p->first, p->second, parent, p->first + offset);
    }

    return true;
}

void StandardTableView::copyClipboard()
{
    QString clipboardString;
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();

    QMap<int, bool> columnMap;
    for (QModelIndex& current : selectedIndexes)
        columnMap[current.column()] = true;
    QList<int> columns = columnMap.uniqueKeys();

    for (int column : columns) {
        clipboardString += model()->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
        if (column != columns.last())
            clipboardString += QLatin1Char('\t');
    }

    clipboardString += QLatin1Char('\n');

    for (int i = 0; i < selectedIndexes.size(); ++i)
    {
        QModelIndex currentIndex = selectedIndexes[i];
        QString displayText = currentIndex.data(Qt::DisplayRole).toString();

        // Check if this is second to last column.
        if (i + 1 < selectedIndexes.size())
        {
            QModelIndex nextIndex = selectedIndexes[i+1];
            // Append newline if the new column is on a new row.
            if (nextIndex.row() != currentIndex.row())
                displayText.append(QLatin1Char('\n'));
            else
                displayText.append(QLatin1Char('\t'));
        }

        clipboardString.append(displayText);
    }

    QApplication::clipboard()->setText(clipboardString);
}

template <typename T>
std::vector<T> StandardTableView::columnData(int column)
{
    std::vector<T> values;

    if (!model())
        return values;

    int nrows = model()->rowCount();
    int ncols = model()->columnCount();

    if (column < 0 || column >= ncols)
        return values;

    values.reserve(nrows);

    for (int row = 0; row < nrows; ++row) {
        QModelIndex currentIndex = model()->index(row, column);
        QVariant data = model()->data(currentIndex);
        if (data.canConvert<T>())
            values.emplace_back(data.value<T>());
    }

    return values;
}

template std::vector<int> StandardTableView::columnData<int>(int);
template std::vector<double> StandardTableView::columnData<double>(int);

template <typename T>
void StandardTableView::setColumnData(int column, const std::vector<T> &values)
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    int ncols = model()->columnCount();

    if (column < 0 || column >= ncols)
        return;

    if (values.size() != static_cast<std::size_t>(nrows))
        return;

    for (int row = 0; row < nrows; ++row) {
        QModelIndex currentIndex = model()->index(row, column);
        const std::size_t i = static_cast<std::size_t>(row);
        model()->setData(currentIndex, values[i], Qt::DisplayRole);
    }

    return;
}

template void StandardTableView::setColumnData<int>(int, const std::vector<int> &);
template void StandardTableView::setColumnData<double>(int, const std::vector<double> &);

void StandardTableView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyClipboard();
        event->accept();
    }
    else if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Plus:
            moveSelectedRows(1); // move down
            event->accept();
            break;
        case Qt::Key_Minus:
            moveSelectedRows(-1); // move up
            event->accept();
            break;
        default:
            break;
        }
    }

    QTableView::keyPressEvent(event);
}

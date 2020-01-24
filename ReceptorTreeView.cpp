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

#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QStylePainter>
#include <QDebug>

#include <algorithm>

#include "ReceptorDelegate.h"
#include "ReceptorTreeView.h"

ReceptorTreeView::ReceptorTreeView(ReceptorModel *model, QWidget *parent)
    : QTreeView(parent)
{
    using Column = ReceptorModel::Column;

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);
    setPalette(QApplication::palette());
    setAutoFillBackground(true);
    setFrameStyle(QFrame::NoFrame);

    QFont font = QApplication::font();
    QHeaderView *header = this->header();
    header->setFont(font);
    header->setFixedHeight(32);

    setModel(model);

    setItemDelegateForColumn(Column::Color, new ReceptorColorDelegate);
    setItemDelegateForColumn(Column::Group, new ReceptorGroupDelegate);
    setItemDelegateForColumn(Column::X, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::Y, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::Z, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::ZHill, new ReceptorGridDelegate);
    setItemDelegateForColumn(Column::ZFlag, new ReceptorGridDelegate);

    const int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    QFontMetrics fm(font);
    const int charWidth = fm.averageCharWidth();

    header->setSectionResizeMode(Column::Color, QHeaderView::Fixed);
    header->resizeSection(Column::Color, indentation() + iconSize * 1.5);

    setColumnWidth(Column::Group, charWidth * 9);
    setColumnWidth(Column::X, charWidth * 9);
    setColumnWidth(Column::Y, charWidth * 9);
    setColumnWidth(Column::Z, charWidth * 7);
    setColumnWidth(Column::ZHill, charWidth * 7);
    setColumnWidth(Column::ZFlag, charWidth * 7);
}

ReceptorTreeView::~ReceptorTreeView()
{}

void ReceptorTreeView::selectLastRow()
{
    if (!model())
        return;

    int nrows = model()->rowCount();
    if (nrows > 0) {
        QItemSelectionModel *sm = selectionModel();
        QModelIndex index = model()->index(nrows - 1, 0, QModelIndex());
        sm->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        scrollToBottom();
    }
}

void ReceptorTreeView::copyClipboard()
{
    QModelIndexList selectedRows = selectionModel()->selectedRows();
    if (selectedRows.empty())
        return;

    int columnCount = model()->columnCount();
    QString clipboardString;

    for (int j = ReceptorModel::Column::Group; j < columnCount; ++j) {
        clipboardString += model()->headerData(j, Qt::Horizontal, Qt::DisplayRole).toString();
        clipboardString += QLatin1Char('\t');
    }

    clipboardString += QLatin1Char('\n');

    // Sort by parent row then child row.
    std::sort(selectedRows.begin(), selectedRows.end(), [&](const QModelIndex& a, const QModelIndex& b) {
        return (a.internalId() < b.internalId()) ||
               (a.internalId() == b.internalId() && a.row() < b.row());
    });

    // Remove parent indexes.
    selectedRows.erase(std::remove_if(selectedRows.begin(), selectedRows.end(), [&](const QModelIndex& index) {
        return index.internalId() == 0;
    }), selectedRows.end());

    for (const QModelIndex& rowIndex : selectedRows) {
        QModelIndex parent = rowIndex.parent();
        QString group = parent.siblingAtColumn(ReceptorModel::Column::Group).data(Qt::DisplayRole).toString();
        clipboardString += group + QLatin1Char('\t');
        for (int column = ReceptorModel::Column::X; column < columnCount; ++column) {
            QModelIndex index = rowIndex.siblingAtColumn(column);
            QString displayText = index.data(Qt::DisplayRole).toString();
            clipboardString += displayText + QLatin1Char('\t');
        }
        clipboardString += QLatin1Char('\n');
    }

    QApplication::clipboard()->setText(clipboardString);
}

void ReceptorTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = viewOptions();

    // Fill the branch rect.
    QRect bgrect = QRect(rect.topLeft(), QPoint(rect.left() + indentation() - 1, rect.bottom()));
    painter->fillRect(bgrect, opt.palette.window());

    QTreeView::drawBranches(painter, rect, index);
}

void ReceptorTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;

    // Always use active style for selections.
    if (selectionModel()->isSelected(index))
        opt.state |= QStyle::State_Active;

    QTreeView::drawRow(painter, opt, index);

    // Draw horizontal gridline below the row.
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(color);
    painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());
    painter->restore();
}

void ReceptorTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyClipboard();
        event->accept();
    }
    else {
        QTreeView::keyPressEvent(event);
    }
}

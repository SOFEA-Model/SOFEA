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

#include "AppStyle.h"
#include "FilterEditor.h"
#include "FilterHeaderView.h"

#include <QApplication>
#include <QBoxLayout>
#include <QEvent>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QSortFilterProxyModel>
#include <QStyleOptionMenuItem>
#include <QWidgetAction>

#include <QDebug>

//-----------------------------------------------------------------------------
// FilterHeaderView
//-----------------------------------------------------------------------------

FilterHeaderView::FilterHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    const QIcon sortAscendingIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_HeaderSortAscending));
    const QIcon sortDescendingIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_HeaderSortDescending));
    const QIcon deleteFilterIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_HeaderDeleteFilter));

    actSortAscending = new QAction(sortAscendingIcon, tr("Sort A to Z"), this);
    actSortDescending = new QAction(sortDescendingIcon, tr("Sort Z to A"), this);
    actClearFilter = new QAction(deleteFilterIcon, tr("Clear Filter"), this);

    filterMenu = new QMenu(this);

    filterEditor = new FilterEditor;
    filterEditor->setMouseTracking(true); // QMenu Fix

    actEditFilter = new QWidgetAction(filterMenu);
    actEditFilter->setDefaultWidget(filterEditor);

    filterMenu->addAction(actSortAscending);
    filterMenu->addAction(actSortDescending);
    filterMenu->addSeparator();
    filterMenu->addAction(actClearFilter);
    filterMenu->addAction(actEditFilter);
}

void FilterHeaderView::setAutoFilterEnabled(bool enabled)
{
    if (enabled && autoFilterEnabled())
        return;

    if (enabled) {
        setSectionsClickable(false);
        setHighlightSections(false);
        setMouseTracking(true);
        setSortIndicator(-1, Qt::AscendingOrder);
        setSortIndicatorShown(false);
        autoFilter = true;
    }
    else {
        setSectionsClickable(true);
        setHighlightSections(true);
        setMouseTracking(false);
        autoFilter = false;
    }
}

bool FilterHeaderView::autoFilterEnabled() const
{
    return autoFilter;
}

void FilterHeaderView::showAutoFilter(int section)
{
    // Align with the right edge of the column.
    int width = filterMenu->sizeHint().width();
    int x = sectionViewportPosition(section) + sectionSize(section) - width;
    int y = viewport()->height();
    QPoint pos = QPoint(x, y);
    QPoint globalPos = viewport()->mapToGlobal(pos);

    // Use the source model for filter queries.
    QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel *>(model());
    QAbstractItemModel *sourceModel = proxyModel ? proxyModel->sourceModel() : model();

    // Set the current column.
    filterEditor->clearSearchText();
    filterEditor->setSourceModel(sourceModel);
    filterEditor->setFilterKeyColumn(section);
    filterEditor->updateCheckState();

    bool changed = false;
    connect(filterEditor, &FilterEditor::filterChanged, [&]{
        changed = true;
    });

    QAction *selected = filterMenu->exec(globalPos);

    if (selected == actSortAscending) {
        columnFlags[section].setFlag(SortAscending).setFlag(SortDescending, false);
        setSortIndicator(section, Qt::AscendingOrder);
    }
    else if (selected == actSortDescending) {
        columnFlags[section].setFlag(SortDescending).setFlag(SortAscending, false);
        setSortIndicator(section, Qt::DescendingOrder);
    }
    else if (selected == actClearFilter) {
        columnFlags[section].setFlag(Filter, false);
        filterEditor->clearFilter(section);
        emit filterStateChanged();
        this->viewport()->update();
    }
    else if (changed) {
        columnFlags[section].setFlag(Filter, true);
        emit filterStateChanged();
        this->viewport()->update();
    }

    return;
}

std::set<int> FilterHeaderView::filteredRows() const
{
    return filterEditor->proxyModel()->filteredRows();
}

void FilterHeaderView::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    if (autoFilter && orientation() == Qt::Horizontal) {
        int prevSection = hoverSection;
        hoverSection = -1;
        headerDataChanged(Qt::Horizontal, prevSection, prevSection);
    }
}

void FilterHeaderView::mouseMoveEvent(QMouseEvent *e)
{
    QHeaderView::mouseMoveEvent(e);

    if (autoFilter && orientation() == Qt::Horizontal) {
        int pos = e->x();
        int section = logicalIndexAt(pos);
        if (section != hoverSection && section >= 0) {
            int prevSection = hoverSection;
            hoverSection = section;
            headerDataChanged(Qt::Horizontal, section, section);
            headerDataChanged(Qt::Horizontal, prevSection, prevSection);
        }
    }
}

void FilterHeaderView::mousePressEvent(QMouseEvent *e)
{
    if (autoFilter && (orientation() == Qt::Horizontal || e->button() == Qt::LeftButton)) {
        int pos = e->x();
        int section = logicalIndexAt(pos);
        if (isPointOnIcon(section, e->pos())) {
            return;
        }
    }

    QHeaderView::mousePressEvent(e);
}

void FilterHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    if (autoFilter && (orientation() == Qt::Horizontal || e->button() == Qt::LeftButton)) {
        int pos = e->x();
        int section = logicalIndexAt(pos);
        if (isPointOnIcon(section, e->pos())) {
            showAutoFilter(section);
            return;
        }
    }

    QHeaderView::mouseReleaseEvent(e);
}

bool FilterHeaderView::isPointOnIcon(int section, const QPoint& pos) const
{
    if (orientation() != Qt::Horizontal)
        return false;

    QRect sectionRect = QRect(sectionViewportPosition(section), 0,
                              sectionSize(section), viewport()->height());

    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.section = section;
    opt.rect = sectionRect;

    QRect iconRect = style()->subElementRect(QStyle::SE_HeaderArrow, &opt, this);
    return iconRect.contains(pos);
}

void FilterHeaderView::paintIndicator(QPainter *painter, const QRect &rect, SortFilterFlags flags) const
{
    static const QPointF filter[6] = {
        QPointF(0.7500, 0.2500),
        QPointF(0.2500, 0.2500),
        QPointF(0.4375, 0.5625),
        QPointF(0.4375, 0.8125),
        QPointF(0.5625, 0.8125),
        QPointF(0.5625, 0.5625)
    };

    static const QPointF arrowUp[9] = {
        QPointF(0.5000, 0.1875),
        QPointF(0.3125, 0.3750),
        QPointF(0.3125, 0.5000),
        QPointF(0.4375, 0.3750),
        QPointF(0.4375, 0.8125),
        QPointF(0.5625, 0.8125),
        QPointF(0.5625, 0.3750),
        QPointF(0.6875, 0.5000),
        QPointF(0.6875, 0.3750)
    };

    static const QPointF arrowDown[9] = {
        QPointF(0.5000, 0.8750),
        QPointF(0.3125, 0.6875),
        QPointF(0.3125, 0.5625),
        QPointF(0.4375, 0.6875),
        QPointF(0.4375, 0.2500),
        QPointF(0.5625, 0.2500),
        QPointF(0.5625, 0.6875),
        QPointF(0.6875, 0.5625),
        QPointF(0.6875, 0.6875)
    };

    static const QPointF overlayFilter[6] = {
        QPointF(0.5625, 0.2500),
        QPointF(0.0625, 0.2500),
        QPointF(0.2500, 0.5625),
        QPointF(0.2500, 0.8125),
        QPointF(0.3750, 0.8125),
        QPointF(0.3750, 0.5625)
    };

    static const QPointF overlayArrowUp[9] = {
        QPointF(0.7500, 0.2500),
        QPointF(0.5625, 0.4375),
        QPointF(0.5625, 0.5625),
        QPointF(0.6875, 0.4375),
        QPointF(0.6875, 0.8125),
        QPointF(0.8125, 0.8125),
        QPointF(0.8125, 0.4375),
        QPointF(0.9375, 0.5625),
        QPointF(0.9375, 0.4375)
    };

    static const QPointF overlayArrowDown[9] = {
        QPointF(0.7500, 0.8125),
        QPointF(0.5625, 0.6250),
        QPointF(0.5625, 0.5000),
        QPointF(0.6875, 0.6250),
        QPointF(0.6875, 0.2500),
        QPointF(0.8125, 0.2500),
        QPointF(0.8125, 0.6250),
        QPointF(0.9375, 0.5000),
        QPointF(0.9375, 0.6250)
    };

    const QBrush arrowBrush = QColor(0, 83, 156);
    const QBrush filterBrush = QColor(66, 66, 66);

    int side = qMax(rect.width(), rect.height());
    const int x = rect.center().x() - side / 2;
    const int y = rect.center().y() - side / 2;

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->translate(x, y);
    painter->scale(side, side);
    painter->setPen(Qt::NoPen);

    switch (flags) {
    case SortAscending:
        painter->setBrush(arrowBrush);
        painter->drawPolygon(arrowUp, sizeof arrowUp / sizeof arrowUp[0]);
        break;
    case SortAscendingFilter:
        painter->setBrush(arrowBrush);
        painter->drawPolygon(overlayArrowUp, sizeof overlayArrowUp / sizeof overlayArrowUp[0]);
        painter->setBrush(filterBrush);
        painter->drawPolygon(overlayFilter, sizeof overlayFilter / sizeof overlayFilter[0]);
        break;
    case SortDescending:
        painter->setBrush(arrowBrush);
        painter->drawPolygon(arrowDown, sizeof arrowDown / sizeof arrowDown[0]);
        break;
    case SortDescendingFilter:
        painter->setBrush(arrowBrush);
        painter->drawPolygon(overlayArrowDown, sizeof overlayArrowDown / sizeof overlayArrowDown[0]);
        painter->setBrush(filterBrush);
        painter->drawPolygon(overlayFilter, sizeof overlayFilter / sizeof overlayFilter[0]);
        break;
    case Filter:
        painter->setBrush(filterBrush);
        painter->drawPolygon(filter, sizeof filter / sizeof filter[0]);
        break;
    default:
        break;
    }

    painter->restore();
}

void FilterHeaderView::paintSection(QPainter *painter, const QRect &rect, int section) const
{
    // Use custom icon in place of PE_IndicatorHeaderArrow
    // See: qcommonstyle.cpp, drawControl, CE_Header

    if (!autoFilter) {
        QHeaderView::paintSection(painter, rect, section);
        return;
    }

    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.section = section;
    opt.rect = rect;

    // Draw the header.
    style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

    // Calculate icon rect and label rects with/without icon.
    QRect iconRect = style()->subElementRect(QStyle::SE_HeaderArrow, &opt, this);
    opt.sortIndicator = QStyleOptionHeader::None;
    QRect labelRect0 = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
    opt.sortIndicator = QStyleOptionHeader::SortUp;
    QRect labelRect1 = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);

    //int pm = style()->pixelMetric(QStyle::PM_HeaderMarkSize, nullptr, this);
    //int pm = style()->pixelMetric(QStyle::PM_ToolBarIconSize, nullptr, this);
    //int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, this);

    // Draw sort, filter or hover indicators?
    bool filterIndicator = false;
    if (columnFlags.find(section) != columnFlags.end())
        filterIndicator = columnFlags.at(section).testFlag(Filter);
    bool sortIndicator = (isSortIndicatorShown() && sortIndicatorSection() == section);
    bool drawIcon = (section == hoverSection || sortIndicator || filterIndicator);

    // Set the header text and elide if necessary.
    QAbstractItemModel *m = model();
    if (m) {
        QString headerText = m->headerData(section, orientation(), Qt::DisplayRole).toString();
        QVariant textAlignment = m->headerData(section, orientation(), Qt::TextAlignmentRole);
        opt.textAlignment = Qt::Alignment(textAlignment.isValid() ?
            Qt::Alignment(textAlignment.toInt()) : defaultAlignment());

        // Use the label rect without icon if possible, to maintain position.
        QFontMetrics fm(font());
        if (drawIcon) {
            opt.text = fm.elidedText(headerText, Qt::ElideRight, labelRect1.width());
            QRect elidedRect = fm.boundingRect(labelRect0, opt.textAlignment, opt.text);
            opt.rect = (elidedRect.right() <= labelRect1.right()) ?
                labelRect0 : labelRect1;
        }
        else {
            opt.text = fm.elidedText(headerText, Qt::ElideRight, labelRect0.width());
            opt.rect = labelRect0;
        }

        if (opt.rect.isValid()) {
            style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);
        }
    }

    if (drawIcon) {
        if (sortIndicator) {
            SortFilterFlags flags = filterIndicator ? Filter : NoSortFilter;
            if (sortIndicatorOrder() == Qt::AscendingOrder)
                flags |= SortAscending;
            else if (sortIndicatorOrder() == Qt::DescendingOrder)
                flags |= SortDescending;
            paintIndicator(painter, iconRect, flags);
        }
        else {
            paintIndicator(painter, iconRect, Filter);
        }
    }
}

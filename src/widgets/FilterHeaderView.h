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

#include <QFlags>
#include <QHeaderView>

class FilterEditor;

QT_BEGIN_NAMESPACE
class QAction;
class QEvent;
class QMenu;
class QMouseEvent;
class QPainter;
class QWidgetAction;
QT_END_NAMESPACE

#include <map>
#include <set>

//-----------------------------------------------------------------------------
// FilterHeaderView
//-----------------------------------------------------------------------------

class FilterHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    enum SortFilterFlag
    {
        NoSortFilter = 0x00,         // 0000
        SortAscending = 0x01,        // 0001
        SortDescending = 0x02,       // 0010
        Filter = 0x04,               // 0100
        SortAscendingFilter = 0x05,  // 0101
        SortDescendingFilter = 0x06  // 0110
    };

    Q_DECLARE_FLAGS(SortFilterFlags, SortFilterFlag)

    FilterHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void setAutoFilterEnabled(bool enabled);
    bool autoFilterEnabled() const;
    std::set<int> filteredRows() const;

signals:
    void filterStateChanged();

private slots:
    void showAutoFilter(int section);

protected:
    virtual void leaveEvent(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect, int section) const override;

private:    
    bool isPointOnIcon(int section, const QPoint& pos) const;
    void paintIndicator(QPainter *painter, const QRect &rect, SortFilterFlags flags) const;

    QAction *actSortAscending;
    QAction *actSortDescending;
    QAction *actClearFilter;
    FilterEditor *filterEditor;
    QWidgetAction *actEditFilter;
    QMenu *filterMenu;

    bool autoFilter = false;
    int hoverSection = -1;
    std::map<int, SortFilterFlags> columnFlags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FilterHeaderView::SortFilterFlags)


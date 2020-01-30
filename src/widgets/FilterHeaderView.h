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

#ifndef FILTERHEADERVIEW_H
#define FILTERHEADERVIEW_H

#include <QHeaderView>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QEvent;
class QMouseEvent;
class QPainter;
class QLineEdit;
class QListView;
class QWidgetAction;
QT_END_NAMESPACE

class FilterEditorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FilterEditorProxyModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void setFilterWildcard(const QString& pattern);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    mutable QHash<QString, Qt::CheckState> checkStateMap;
};

class FilterEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FilterEditor(QWidget *parent = nullptr);
    //void setSourceModel(QAbstractItemModel *sourceModel);
    //void setFilterKeyColumn(int column);
    //bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    QLineEdit *searchBox;
    QListView *listView;
    FilterEditorProxyModel *proxyModel;
};

class FilterHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    FilterHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setAutoFilterEnabled(bool enabled);
    bool autoFilterEnabled() const;
    //void setSourceModel(QAbstractItemModel *sourceModel);

private slots:
    void showAutoFilter(int section);

protected:
    virtual void leaveEvent(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect, int section) const override;

private:
    enum class SortFilterState
    {
        None,
        SortAscending,
        SortAscendingFilter,
        SortDescending,
        SortDescendingFilter,
        Filter
    };

    bool isPointOnIcon(int section, const QPoint& pos) const;
    void paintIndicator(QPainter *painter, const QRect &rect, SortFilterState state) const;

    QAction *sortAscendingAct;
    QAction *sortDescendingAct;
    //QAction *clearFilterAct;
    //QAction *editFilterAct;
    //FilterEditor *filterEditor;
    //QWidgetAction *filterAction;

    bool autoFilter = false;
    int hoverSection = -1;
    std::map<int, SortFilterState> columnStateMap;
};

#endif // FILTERHEADERVIEW_H

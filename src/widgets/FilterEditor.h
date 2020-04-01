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

#include <QSortFilterProxyModel>
#include <QVariant>
#include <QWidget>

#include <map>
#include <set>

#ifndef BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#define BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#endif

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QCheckBox;
class QLineEdit;
class QListView;
class QPushButton;
QT_END_NAMESPACE

struct FilterState
{
    FilterState(QModelIndex index, const QVariant& value)
        : index(index), data(value) {}

    QModelIndex index;
    QVariant data;
    mutable Qt::CheckState state = Qt::Checked;
};

using FilterStateContainer = boost::multi_index_container<
  FilterState,
  boost::multi_index::indexed_by<
    // Ordered unique on index
    boost::multi_index::ordered_unique<
      boost::multi_index::member<FilterState, QModelIndex, &FilterState::index>
    >,
    // Ordered non-unique on value, index
    boost::multi_index::ordered_non_unique<
      boost::multi_index::composite_key<
        FilterState,
        boost::multi_index::member<FilterState, QVariant, &FilterState::data>,
        boost::multi_index::member<FilterState, QModelIndex, &FilterState::index>
      >
    >
  >
>;

//-----------------------------------------------------------------------------
// FilterEditorProxyModel
//-----------------------------------------------------------------------------

class FilterEditorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FilterEditorProxyModel(QObject *parent = nullptr);

    void setFilterKeyColumn(int column);
    void clearFilter(int column);
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    std::set<int> filteredRows() const;

signals:
    void filterChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    FilterStateContainer states;
};

//-----------------------------------------------------------------------------
// FilterEditor
//-----------------------------------------------------------------------------

class FilterEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FilterEditor(QWidget *parent = nullptr);
    FilterEditorProxyModel * proxyModel();
    void setSourceModel(QAbstractItemModel *sourceModel);
    void setFilterKeyColumn(int column);
    void clearFilter(int column);
    void clearSearchText();
    void updateCheckState();

signals:
    void filterChanged();

private slots:
    void onSearchTextEdited(const QString& text);
    void onCheckStateChanged(int state);
    void onDataChanged(const QModelIndex&, const QModelIndex&);

private:
    QLineEdit *leSearch;
    QCheckBox *cbSelectAll;
    QListView *view;
    FilterEditorProxyModel *proxy;
};

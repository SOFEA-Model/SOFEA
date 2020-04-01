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

#include <QBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSortFilterProxyModel>

#include <QDebug>

#include <tuple>
#include <utility>

//-----------------------------------------------------------------------------
// FilterEditorProxyModel
//-----------------------------------------------------------------------------

FilterEditorProxyModel::FilterEditorProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

void FilterEditorProxyModel::setFilterKeyColumn(int column)
{
    if (!sourceModel())
        QSortFilterProxyModel::setFilterKeyColumn(column);

    // Update the container with filter states for all rows.
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        QModelIndex sourceIndex = sourceModel()->index(row, column);
        auto& unique_index_view = states.get<0>();
        auto it = unique_index_view.find(sourceIndex);
        if (it == unique_index_view.end()) { // not found
            QVariant displayData = sourceModel()->data(sourceIndex, Qt::DisplayRole);
            states.insert(FilterState(sourceIndex, displayData));
        }
    }

    QSortFilterProxyModel::setFilterKeyColumn(column);
}

void FilterEditorProxyModel::clearFilter(int column)
{
    if (!sourceModel())
        return;

    // Reset the check state for all rows to default (Qt::Checked).
    for (int row = 0; row < sourceModel()->rowCount(); ++row) {
        QModelIndex sourceIndex = sourceModel()->index(row, column);
        auto& unique_index_view = states.get<0>();
        auto it = unique_index_view.find(sourceIndex);
        while (it != unique_index_view.end()) {
            if (it->index.column() == filterKeyColumn())
                it->state = Qt::Checked;
            ++it;
        }
    }
}

QVariant FilterEditorProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != filterKeyColumn())
        return QVariant();

    // Return the check state for the source index if available.
    switch (role) {
    case Qt::CheckStateRole: {
        QModelIndex sourceIndex = mapToSource(index);
        auto& unique_index_view = states.get<0>();
        auto it = unique_index_view.find(sourceIndex);
        if (it != unique_index_view.end())
            return QVariant(it->state);
        else
            return QVariant(Qt::Checked);
    }
    case Qt::FontRole:
    case Qt::TextAlignmentRole:
    case Qt::ForegroundRole:
    case Qt::BackgroundRole:
        return QVariant();
    default:
        break;
    }

    return QSortFilterProxyModel::data(index, role);
}

bool FilterEditorProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.column() != filterKeyColumn())
        return false;

    // Update the check state for every source index in the column with matching value.
    if (role == Qt::CheckStateRole && value.canConvert(QVariant::Int))
    {
        QVariant displayData = sourceModel()->data(mapToSource(index), Qt::DisplayRole);
        Qt::CheckState state = qvariant_cast<Qt::CheckState>(value);

        auto& composite_key_view = states.get<1>();
        auto key = std::make_tuple(displayData);
        auto p = composite_key_view.equal_range(key);
        while (p.first != p.second) {
            if (p.first->index.column() == filterKeyColumn())
                p.first->state = state;
            ++p.first;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

Qt::ItemFlags FilterEditorProxyModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || index.column() != filterKeyColumn())
        return QSortFilterProxyModel::flags(index);

    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

bool FilterEditorProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
    QVariant displayData = sourceModel()->data(sourceIndex, Qt::DisplayRole);

    // Accept only the first index in each (data, index) group.
    auto& composite_key_view = states.get<1>();
    auto key = std::make_tuple(displayData);
    auto p = composite_key_view.equal_range(key);
    if (p.first != p.second && p.first->index == sourceIndex)
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    else
        return false;
}

std::set<int> FilterEditorProxyModel::filteredRows() const
{
    std::set<int> result;
    auto& unique_index_view = states.get<0>();
    auto it = unique_index_view.begin();
    while (it != unique_index_view.end()) {
        if (it->state == Qt::Unchecked)
            result.insert(it->index.row());
        ++it;
    }
    return result;
}

//-----------------------------------------------------------------------------
// FilterEditor
//-----------------------------------------------------------------------------

FilterEditor::FilterEditor(QWidget *parent)
    : QWidget(parent)
{
    leSearch = new QLineEdit;
    leSearch->setPlaceholderText(tr("Search"));
    leSearch->setClearButtonEnabled(true);

    QRegularExpression re("\\S+"); // non-whitespace characters
    QRegularExpressionValidator *searchValidator = new QRegularExpressionValidator(re, this);
    leSearch->setValidator(searchValidator);

    cbSelectAll = new QCheckBox(tr("Select All"));
    cbSelectAll->setTristate(true);

    proxy = new FilterEditorProxyModel(this);

    view = new QListView;
    view->setFixedHeight(200);
    view->setSelectionMode(QAbstractItemView::NoSelection);

    connect(leSearch, &QLineEdit::textEdited, this, &FilterEditor::onSearchTextEdited);
    connect(cbSelectAll, &QCheckBox::stateChanged, this, &FilterEditor::onCheckStateChanged);
    connect(proxy, &FilterEditorProxyModel::dataChanged, this, &FilterEditor::onDataChanged);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    const int scaleFactor = static_cast<int>(this->logicalDpiX() / 96.0);
    const int xOffset = 21 * scaleFactor + 16; // Check CE_MenuItem in QFusionStyle
    mainLayout->setContentsMargins(xOffset, 6, 6, 6);
    mainLayout->addWidget(leSearch);
    mainLayout->addWidget(cbSelectAll);
    mainLayout->addWidget(view);
    setLayout(mainLayout);
}

FilterEditorProxyModel * FilterEditor::proxyModel()
{
    return proxy;
}

void FilterEditor::setSourceModel(QAbstractItemModel *sourceModel)
{
    proxy->setSourceModel(sourceModel);
    view->setModel(proxy);
}

void FilterEditor::setFilterKeyColumn(int column)
{
    proxy->setFilterKeyColumn(column);
    view->setModelColumn(column);
}

void FilterEditor::clearFilter(int column)
{
    proxy->clearFilter(column);
    proxy->setFilterRegularExpression("");
}

void FilterEditor::clearSearchText()
{
    leSearch->clear();
    proxy->setFilterRegularExpression("");
}

void FilterEditor::onSearchTextEdited(const QString& text)
{
    QRegularExpression re(text, QRegularExpression::CaseInsensitiveOption);
    proxy->setFilterRegularExpression(re);
}

void FilterEditor::onCheckStateChanged(int state)
{
    disconnect(proxy, &FilterEditorProxyModel::dataChanged, this, &FilterEditor::onDataChanged);

    if (state == Qt::PartiallyChecked) {
        state = Qt::Checked;
        disconnect(cbSelectAll, &QCheckBox::stateChanged, this, &FilterEditor::onCheckStateChanged);
        cbSelectAll->setCheckState(static_cast<Qt::CheckState>(state));
        connect(cbSelectAll, &QCheckBox::stateChanged, this, &FilterEditor::onCheckStateChanged);
    }

    int column = view->modelColumn();
    for (int row = 0; row < proxy->rowCount(); ++row) {
        QModelIndex index = proxy->index(row, column);
        proxy->setData(index, state, Qt::CheckStateRole);
    }

    connect(proxy, &FilterEditorProxyModel::dataChanged, this, &FilterEditor::onDataChanged);
    emit filterChanged();
}

void FilterEditor::onDataChanged(const QModelIndex&, const QModelIndex&)
{
    updateCheckState();
    emit filterChanged();
}

void FilterEditor::updateCheckState()
{
    Qt::CheckState prevState;
    int column = view->modelColumn();
    for (int row = 0; row < proxy->rowCount(); ++row) {
        QModelIndex index = proxy->index(row, column);
        Qt::CheckState state = qvariant_cast<Qt::CheckState>(proxy->data(index, Qt::CheckStateRole));
        if (row == 0) {
            prevState = state;
        }
        else if (row > 0 && prevState != state) {
            prevState = Qt::PartiallyChecked;
            break;
        }
    }

    disconnect(cbSelectAll, &QCheckBox::stateChanged, this, &FilterEditor::onCheckStateChanged);
    cbSelectAll->setCheckState(prevState);
    connect(cbSelectAll, &QCheckBox::stateChanged, this, &FilterEditor::onCheckStateChanged);
}

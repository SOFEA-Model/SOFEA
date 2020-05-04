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

#include "StandardTableEditor.h"
#include "StandardTableView.h"

#include <QDir>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

#include <algorithm>
#include <iterator>

StandardTableEditor::StandardTableEditor(Qt::Orientation orientation, StandardButtons buttons, QWidget *parent)
    : QWidget(parent)
{
    filter_ = nullptr;
    caption_ = tr("Import File");

    // Layout
    QBoxLayout *mainLayout;
    switch (orientation) {
    case Qt::Vertical:
        mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        break;
    case Qt::Horizontal:
        mainLayout = new QBoxLayout(QBoxLayout::LeftToRight);
        break;
    default:
        return;
    }

    mainLayout->setMargin(0);

    if (orientation == Qt::Horizontal) {
        mainLayout->addStretch(1);
    }

    if (buttons.testFlag(Add)) {
        btnAdd = new QPushButton("Add");
        mainLayout->addWidget(btnAdd);
    }
    if (buttons.testFlag(Remove)) {
        btnRemove = new QPushButton("Remove");
        mainLayout->addWidget(btnRemove);
    }
    if (buttons.testFlag(Rename)) {
        btnRename = new QPushButton("Rename");
        mainLayout->addWidget(btnRename);
    }

    if (buttons.testFlag(MoveUp) | buttons.testFlag(MoveDown)) {
        mainLayout->addSpacing(10);
    }

    if (buttons.testFlag(MoveUp)) {
        btnMoveUp = new QPushButton("Move Up");
        mainLayout->addWidget(btnMoveUp);
    }
    if (buttons.testFlag(MoveDown)) {
        btnMoveDown = new QPushButton("Move Down");
        mainLayout->addWidget(btnMoveDown);
    }

    if (buttons.testFlag(Edit) | buttons.testFlag(Import)) {
        mainLayout->addSpacing(10);
    }

    if (buttons.testFlag(Edit)) {
        btnEdit = new QPushButton("Edit...");
        mainLayout->addWidget(btnEdit);
    }
    if (buttons.testFlag(Import)) {
        btnImport = new QPushButton("Import...");
        mainLayout->addWidget(btnImport);
    }

    if (orientation == Qt::Vertical) {
        mainLayout->addStretch(1);
    }

    setLayout(mainLayout);
}

void StandardTableEditor::setView(StandardTableView *view)
{
    view_ = view;

    if (view_->selectionModel()->selectedIndexes().isEmpty()) {
        if (btnRemove)   btnRemove->setEnabled(false);
        if (btnMoveUp)   btnMoveUp->setEnabled(false);
        if (btnMoveDown) btnMoveDown->setEnabled(false);
    }

    if (view_->selectionModel()->selectedRows().count() != 1) {
        if (btnEdit)     btnEdit->setEnabled(false);
        if (btnRename)   btnRename->setEnabled(false);
    }

    if (btnAdd)      btnAdd->setFocusProxy(view);
    if (btnRemove)   btnRemove->setFocusProxy(view);
    if (btnRename)   btnRename->setFocusProxy(view);
    if (btnMoveUp)   btnMoveUp->setFocusProxy(view);
    if (btnMoveDown) btnMoveDown->setFocusProxy(view);
    if (btnEdit)     btnEdit->setFocusProxy(view);
    if (btnImport)   btnImport->setFocusProxy(view);

    // Connections
    if (btnAdd)      connect(btnAdd,      &QPushButton::clicked, this, &StandardTableEditor::onAddItemClicked);
    if (btnRemove)   connect(btnRemove,   &QPushButton::clicked, this, &StandardTableEditor::onRemoveItemClicked);
    if (btnRename)   connect(btnRename,   &QPushButton::clicked, this, &StandardTableEditor::onRenameItemClicked);
    if (btnMoveUp)   connect(btnMoveUp,   &QPushButton::clicked, this, &StandardTableEditor::onMoveUpClicked);
    if (btnMoveDown) connect(btnMoveDown, &QPushButton::clicked, this, &StandardTableEditor::onMoveDownClicked);
    if (btnEdit)     connect(btnEdit,     &QPushButton::clicked, this, &StandardTableEditor::onEditItemClicked);
    if (btnImport)   connect(btnImport,   &QPushButton::clicked, this, &StandardTableEditor::onImportClicked);

    connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &StandardTableEditor::onSelectionChanged);

    connect(view_->model(), &QAbstractItemModel::rowsMoved,
            this, &StandardTableEditor::onRowsMoved);
}

void StandardTableEditor::setImportFilter(const QString& filter)
{
    filter_ = filter;
}

void StandardTableEditor::setImportCaption(const QString& caption)
{
    caption_ = caption;
}

QAbstractButton * StandardTableEditor::button(ButtonRole role)
{
    switch (role) {
    case ButtonRole::AddRole:      return btnAdd;
    case ButtonRole::RemoveRole:   return btnRemove;
    case ButtonRole::RenameRole:   return btnRename;
    case ButtonRole::MoveUpRole:   return btnMoveUp;
    case ButtonRole::MoveDownRole: return btnMoveDown;
    case ButtonRole::EditRole:     return btnEdit;
    case ButtonRole::ImportRole:   return btnImport;
    }
    return nullptr;
}

void StandardTableEditor::onAddItemClicked()
{
    if (!view_)
        return;

    if (view_->appendRow()) {
        view_->selectLastRow();
    }
}

void StandardTableEditor::onRemoveItemClicked()
{
    if (!view_)
        return;

    view_->removeSelectedRows();
}

void StandardTableEditor::onRenameItemClicked()
{
    if (!view_)
        return;

    if (view_->selectionBehavior() == QAbstractItemView::SelectItems) {
        QModelIndexList selectedItems = view_->selectionModel()->selectedIndexes();
        if (selectedItems.count() == 1)
            view_->edit(selectedItems.first());
    }
    else if (view_->selectionBehavior() == QAbstractItemView::SelectRows) {
        QModelIndexList selectedRows = view_->selectionModel()->selectedRows();
        if (selectedRows.count() == 1)
            view_->edit(selectedRows.first());
    }
}

void StandardTableEditor::onMoveUpClicked()
{
    if (!view_)
        return;

    view_->moveSelectedRows(-1);
}

void StandardTableEditor::onMoveDownClicked()
{
    if (!view_)
        return;

    view_->moveSelectedRows(1);
}

void StandardTableEditor::onEditItemClicked()
{
    if (!view_)
        return;

    if (view_->selectionBehavior() == QAbstractItemView::SelectItems) {
        QModelIndexList selectedItems = view_->selectionModel()->selectedIndexes();
        if (selectedItems.count() == 1)
            emit editRequested(selectedItems.first());
    }
    else if (view_->selectionBehavior() == QAbstractItemView::SelectRows) {
        QModelIndexList selectedRows = view_->selectionModel()->selectedRows();
        if (selectedRows.count() == 1)
            emit editRequested(selectedRows.first());
    }
}

void StandardTableEditor::onImportClicked()
{
    if (!view_)
        return;

    int cols = view_->model()->columnCount();
    if (cols == 0)
        return;

    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this,
                       caption_,
                       QDir::currentPath(),
                       filter_);

    if (filename.isEmpty())
        return;

    emit importRequested(filename);
}

void StandardTableEditor::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    checkSelection();
}

void StandardTableEditor::onRowsMoved(const QModelIndex&, int sourceFirst, int sourceLast,
                                      const QModelIndex&, int destinationFirst)
{
    // Need to query the current selection after each move.
    // This slot will be invoked multiple times for non-contiguous selections.

    checkSelection();
}

void StandardTableEditor::checkSelection()
{
    bool hasSelection = view_->selectionModel()->hasSelection();

    if (btnRemove) btnRemove->setEnabled(hasSelection);
    if (!hasSelection) {
        if (btnMoveUp)   btnMoveUp->setEnabled(false);
        if (btnMoveDown) btnMoveDown->setEnabled(false);
        if (btnEdit)     btnEdit->setEnabled(false);
        if (btnRename)   btnRename->setEnabled(false);
        return;
    }

    QModelIndexList selectedRows = view_->selectedRows();

    // Edit and Rename requires single selection.
    bool singleSelection = selectedRows.count() == 1;
    if (btnEdit)     btnEdit->setEnabled(singleSelection);
    if (btnRename)   btnRename->setEnabled(singleSelection);

    int nrows = view_->model()->rowCount();

    if (btnMoveUp) {
        int start = selectedRows.first().row();
        btnMoveUp->setEnabled(nrows > 1 && start - 1 >= 0);
    }

    if (btnMoveDown) {
        int start = selectedRows.last().row();
        btnMoveDown->setEnabled(nrows > 1 && start + 1 < nrows);
    }
}

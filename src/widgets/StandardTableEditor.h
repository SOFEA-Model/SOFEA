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

#include <QBoxLayout>
#include <QFlags>
#include <QString>
#include <QWidget>

class StandardTableView;

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
class QAbstractButton;
QT_END_NAMESPACE

class StandardTableEditor : public QWidget
{
    Q_OBJECT

public:
    enum StandardButton {
        Add       = 0x01,
        Remove    = 0x02,
        AddRemove = 0x03,
        Rename    = 0x04,
        MoveUp    = 0x08,
        MoveDown  = 0x10,
        Edit      = 0x20,
        Import    = 0x40,
        All       = 0xff
    };

    enum ButtonRole {
        AddRole,
        RemoveRole,
        RenameRole,
        MoveUpRole,
        MoveDownRole,
        EditRole,
        ImportRole
    };

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    StandardTableEditor(Qt::Orientation orientation = Qt::Vertical,
        StandardButtons buttons = AddRemove, QWidget *parent = nullptr);

    void setView(StandardTableView *view);
    void setImportFilter(const QString& filter);
    void setImportCaption(const QString& caption_);
    QAbstractButton * button(ButtonRole role);

signals:
    void moveRequested(const QModelIndex &sourceParent, int sourceRow, int count,
                       const QModelIndex &destinationParent, int destinationRow);
    void editRequested(const QModelIndex &index);
    void importRequested(const QString& filename);

public slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onRenameItemClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onEditItemClicked();
    void onImportClicked();
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
    void onRowsMoved(const QModelIndex&, int sourceFirst, int sourceLast,
                     const QModelIndex&, int destinationFirst);

private:
    void checkSelection();

    StandardTableView *view_;
    QString filter_;
    QString caption_;

    QAbstractButton *btnAdd = nullptr;
    QAbstractButton *btnRemove = nullptr;
    QAbstractButton *btnRename = nullptr;
    QAbstractButton *btnMoveUp = nullptr;
    QAbstractButton *btnMoveDown = nullptr;
    QAbstractButton *btnEdit = nullptr;
    QAbstractButton *btnImport = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StandardTableEditor::StandardButtons)

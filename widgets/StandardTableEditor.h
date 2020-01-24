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
#include <QPushButton>

#include "StandardTableView.h"

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

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)

    StandardTableEditor(Qt::Orientation orientation = Qt::Vertical,
        StandardButtons buttons = AddRemove, QWidget *parent = nullptr);

    void init(StandardTableView *standardTableView);
    void setImportFilter(const QString& filter);
    void setImportCaption(const QString& caption);

    QPushButton *btnAdd = nullptr;
    QPushButton *btnRemove = nullptr;
    QPushButton *btnRename = nullptr;
    QPushButton *btnMoveUp = nullptr;
    QPushButton *btnMoveDown = nullptr;
    QPushButton *btnEdit = nullptr;
    QPushButton *btnImport = nullptr;

signals:
    void moveRequested(const QModelIndex &sourceParent, int sourceRow, int count,
                       const QModelIndex &destinationParent, int destinationRow);
    void editRequested(const QModelIndex &index);
    void importRequested(const QString& filename);

public slots:
    void onAddItemClicked();
    void onRemoveItemClicked();
    void onRenameItemClicked();
    void onMoveRequested();
    void onEditItemClicked();
    void onImportClicked();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    StandardTableView *m_standardTableView;
    QString m_importFilter;
    QString m_importCaption;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StandardTableEditor::StandardButtons)

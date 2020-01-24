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

#ifndef SOURCETABLE_H
#define SOURCETABLE_H

#include "Scenario.h"
#include "SourceGroup.h"
#include "SourceEditor.h"
#include "SourceModel.h"
#include "FluxProfileModel.h"
#include "widgets/StandardTableView.h"

#include <QAction>
#include <QColor>
#include <QItemSelection>
#include <QLabel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QVector>

class SourceTable : public QWidget
{
    Q_OBJECT

public:
    SourceTable(Scenario *s, SourceGroup *sg, QWidget *parent = nullptr);

    double getTotalMass() const;

signals:
    void dataChanged();

public slots:
    void refresh();

private slots:
    void headerContextMenuRequested(const QPoint &pos);
    void contextMenuRequested(const QPoint &pos);
    void onSelectionChanged(const QItemSelection&, const QItemSelection&);
    void onDataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&);
    void onRowsInserted(const QModelIndex&, int, int);
    void onRowsRemoved(const QModelIndex&, int, int);

private:
    QColor colorFromIndex(const QModelIndex& index) const;
    void openColorDialog(const QModelIndexList& selection);
    void plotFluxProfile(const Source *s);
    void setColumnVisible(int column, bool visible);
    bool isColumnVisible(int column) const;

    Scenario *sPtr;
    SourceGroup *sgPtr;

    QLabel *massLabel;
    StandardTableView *table;
    SourceModel *model;
    SourceEditor *sourceEditor;
    QSortFilterProxyModel *proxyModel;
    FluxProfileModel *fpEditorModel;

    // Table Actions
    QAction *actAddArea;
    QAction *actAddAreaCirc;
    QAction *actAddAreaPoly;
    QAction *actImport;
    QAction *actEdit;
    QAction *actColor;
    QAction *actFlux;
    QAction *actRemove;
    QAction *actResampleAppStart;
    QAction *actResampleAppRate;
    QAction *actResampleIncorpDepth;
    QAction *actResampleFluxProfile;
};

#endif // SOURCETABLE_H

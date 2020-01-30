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

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QMenu>
#include <QPoint>

#include "AppStyle.h"
#include "ProjectTreeView.h"
#include "ProjectModel.h"

ProjectTreeView::ProjectTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setPalette(QApplication::palette());
    setAutoFillBackground(true);
    setHeaderHidden(true);
    setUniformRowHeights(true);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    setupConnections();
}

ProjectTreeView::~ProjectTreeView()
{}

void ProjectTreeView::setupConnections()
{
    connect(this, &QTreeView::customContextMenuRequested,
            this, &ProjectTreeView::showContextMenu);
}

void ProjectTreeView::showContextMenu(const QPoint& pos)
{
    const QIcon cloneIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionClone));
    const QIcon renameIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionRename));

    ProjectModel *projectModel = qobject_cast<ProjectModel *>(this->model());
    if (!projectModel)
        return;

    QPoint globalPos = this->viewport()->mapToGlobal(pos);
    QModelIndex index = this->indexAt(pos);

    QMenu contextMenu;
    QAction *actClone = contextMenu.addAction(cloneIcon, tr("Clone"));
    QAction *actRename = contextMenu.addAction(renameIcon, tr("Rename"));
    QAction *actRemove = contextMenu.addAction(tr("Remove"));
    contextMenu.addSeparator();

    QAction *selected = contextMenu.exec(globalPos);
    if (!selected)
        return;


}

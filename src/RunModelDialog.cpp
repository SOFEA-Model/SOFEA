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
#include <QBoxLayout>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QThread>
#include <QToolButton>
#include <QSettings>
#include <QUrl>

#include <QDebug>

#include "AppStyle.h"
#include "IPCServer.h"
#include "RunModelDialog.h"
#include "core/Common.h"
#include "core/Scenario.h"
#include "delegates/ProgressBarDelegate.h"
#include "models/ProcessModel.h"
#include "widgets/StandardTableView.h"

RunModelDialog::RunModelDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_RunSettings)));
    setWindowTitle("Run Model");
    setWindowFlag(Qt::WindowMaximizeButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setModal(false);

    leWorkingDir = new QLineEdit;
    leWorkingDir->setReadOnly(true);
    btnWorkingDir = new QToolButton;
    btnWorkingDir->setText("...");

    lblThreads = new QLabel(tr("Available processor cores: %1").arg(QThread::idealThreadCount()));
    btnSelectAll = new QPushButton(tr("Select All"));
    btnDeselectAll = new QPushButton(tr("Deselect All"));
    btnRun = new QPushButton(tr("Run"));
    btnPause = new QPushButton(tr("Pause"));
    btnStop = new QPushButton(tr("Stop"));

    model = new ProcessModel(this);

    table = new StandardTableView;
    table->setModel(model);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setItemDelegateForColumn(ProcessModel::Progress, new ProgressBarDelegate);

    int startingWidth = table->font().pointSize();
    table->setColumnWidth(0, startingWidth * 12);
    table->setColumnWidth(1, startingWidth * 12);
    table->setColumnWidth(2, startingWidth * 20);
    table->setColumnWidth(3, startingWidth * 10);
    table->setColumnWidth(4, startingWidth * 20);
    table->setMinimumWidth(startingWidth * 100);
    table->setColumnHidden(5, true);

    // Context Menu Actions
    QIcon openFolderIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_OpenFolder));
    openFolderAction = new QAction(openFolderIcon, tr("Open Folder"), this);

	// Button Box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Output directory: "), 0);
    layout1->addWidget(leWorkingDir, 1);
    layout1->addWidget(btnWorkingDir, 0);

    // Controls Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(btnSelectAll);
    controlsLayout->addWidget(btnDeselectAll);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(btnRun);
    controlsLayout->addWidget(btnPause);
    controlsLayout->addWidget(btnStop);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(table);
    mainLayout->addWidget(lblThreads);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setupConnections();
    setWorkingDirectory(QDir::currentPath());
}

void RunModelDialog::setupConnections()
{
    connect(model, &ProcessModel::progressValueChanged,
            this, &RunModelDialog::progressValueChanged);

    connect(table, &QTableView::customContextMenuRequested,
            this, &RunModelDialog::contextMenuRequested);

    // UI Signals
    connect(btnWorkingDir, &QToolButton::clicked, this, [=]{
        QSettings settings;
        QString selectedDir = QFileDialog::getExistingDirectory(
            this, tr("Select Working Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly);
        if (!selectedDir.isEmpty())
            setWorkingDirectory(selectedDir);
    });

    connect(btnSelectAll, &QPushButton::clicked, this, [=]{
        table->selectAll();
        table->setFocus();
    });

    connect(btnDeselectAll, &QPushButton::clicked, this, [=]{
        table->clearSelection();
        table->setFocus();
    });

    connect(btnRun, &QPushButton::clicked, this, &RunModelDialog::runSelected);
    connect(btnPause, &QPushButton::clicked, this, &RunModelDialog::pauseSelected);
    connect(btnStop, &QPushButton::clicked, this, &RunModelDialog::stopSelected);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RunModelDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RunModelDialog::reject);
}

void RunModelDialog::setWorkingDirectory(const QString& path)
{
    model->setWorkingDirectory(path);
    leWorkingDir->setText(path);
}

void RunModelDialog::addScenario(Scenario *s)
{
    model->addScenario(s);
}

void RunModelDialog::removeScenario(Scenario *s)
{
    model->removeScenario(s);
}

void RunModelDialog::runSelected()
{
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();

    std::sort(selectedRows.begin(), selectedRows.end(), [&](const QModelIndex& a, const QModelIndex& b) {
        return a.row() < b.row();
    });

    for (const auto& index : selectedRows)
        model->startJob(index.row());
}

void RunModelDialog::pauseSelected()
{
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();

    std::sort(selectedRows.begin(), selectedRows.end(), [&](const QModelIndex& a, const QModelIndex& b) {
        return a.row() < b.row();
    });

    for (const auto& index : selectedRows)
        model->suspendJob(index.row());
}

void RunModelDialog::stopSelected()
{
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();

    std::sort(selectedRows.begin(), selectedRows.end(), [&](const QModelIndex& a, const QModelIndex& b) {
        return a.row() < b.row();
    });

    for (const auto& index : selectedRows)
        model->stopJob(index.row());
}


void RunModelDialog::contextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = table->viewport()->mapToGlobal(pos);
    QModelIndex index = table->indexAt(pos);
    if (!index.isValid())
        return;

    int i = index.row();
    if (i < 0 || i >= model->rowCount())
        return;

    index = model->index(i, ProcessModel::Path, QModelIndex());
    QString path = model->data(index, Qt::DisplayRole).toString();

    qDebug() << path;
    QDir dir(path);
    openFolderAction->setEnabled(!path.isEmpty() && dir.exists());

    QMenu contextMenu;
    contextMenu.addAction(openFolderAction);

    QAction *selectedAction = contextMenu.exec(globalPos);
    if (selectedAction == openFolderAction) {
        QDesktopServices::openUrl(QUrl(dir.path()));
    }
}

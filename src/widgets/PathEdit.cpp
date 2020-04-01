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

#include "PathEdit.h"
#include "MenuLineEdit.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMenu>
#include <QStyle>
#include <QToolButton>

PathEdit::PathEdit(QWidget *parent)
    : QWidget(parent)
{
    lePath = new MenuLineEdit;
    lePath->setMaxLength(260); // MAX_PATH
    lePath->setContextMenuPolicy(Qt::CustomContextMenu);

    actMakeRelative = new QAction(tr("Make Relative"));
    actMakeAbsolute = new QAction(tr("Make Absolute"));

    actionMenu = new QMenu;
    actionMenu->addAction(actMakeRelative);
    actionMenu->addAction(actMakeAbsolute);

    lePath->setMenu(actionMenu);

    btnBrowse = new QToolButton;
    btnBrowse->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    btnBrowse->setText("...");

    connect(lePath, &QWidget::customContextMenuRequested, this, &PathEdit::onContextMenuRequested);
    connect(btnBrowse, &QAbstractButton::clicked, this, &PathEdit::browse);
    connect(actionMenu, &QMenu::aboutToShow, this, &PathEdit::updateActions);
    connect(actMakeRelative, &QAction::triggered, [&](bool) { makeRelative(); });
    connect(actMakeAbsolute, &QAction::triggered, [&](bool) { makeAbsolute(); });

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(lePath, 1);
    mainLayout->addWidget(btnBrowse, 0);
    setLayout(mainLayout);

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::LineEdit);
    setSizePolicy(sizePolicy);
}

QString PathEdit::currentPath() const {
    return lePath->text();
}

QString PathEdit::dialogCaption() const {
    return dialogCaption_;
}

QString PathEdit::dialogDirectory() const {
    return dialogDirectory_;
}

QString PathEdit::dialogFilter() const {
    return dialogFilter_;
}

QFileDialog::FileMode PathEdit::dialogFileMode() const {
    return dialogFileMode_;
}

void PathEdit::setCurrentPath(const QString& path)
{
    lePath->setText(path);
    emit currentPathChanged(path);
}

void PathEdit::setDialogCaption(const QString& caption) {
    dialogCaption_ = caption;
}

void PathEdit::setDialogDirectory(const QString& dir) {
    dialogDirectory_ = dir;
}

void PathEdit::setDialogFilter(const QString& filter) {
    dialogFilter_ = filter;
}

void PathEdit::setDialogFileMode(QFileDialog::FileMode mode) {
    dialogFileMode_ = mode;
}

void PathEdit::setReadOnly(bool readOnly)
{
    lePath->setReadOnly(readOnly);
    btnBrowse->setDisabled(readOnly);
    actMakeAbsolute->setDisabled(readOnly);
    actMakeRelative->setDisabled(readOnly);
}

void PathEdit::browse()
{
    QString initDir = dialogDirectory_;
    QString path = currentPath();
    if (!path.isEmpty()) {
        QFileInfo fi(path);
        if (fi.isRelative()) {
            QDir currentDir(dialogDirectory_ + QDir::separator() + fi.path());
            if (currentDir.exists())
                initDir = currentDir.canonicalPath();
        }
    }

    QFileDialog dialog(nullptr, dialogCaption_, initDir, dialogFilter_);
    dialog.setFileMode(dialogFileMode_);
    if (dialog.exec())
        setCurrentPath(dialog.selectedFiles().front());
}

void PathEdit::makeRelative()
{
    QDir base(QDir::currentPath());
    setCurrentPath(base.relativeFilePath(currentPath()));
}

void PathEdit::makeAbsolute()
{
    QDir base(QDir::currentPath());
    QFileInfo fi(base.absoluteFilePath(currentPath()));
    setCurrentPath(fi.canonicalFilePath());
}

void PathEdit::onContextMenuRequested(const QPoint &pos)
{
    updateActions();

    QMenu *menu = lePath->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(actMakeRelative);
    menu->addAction(actMakeAbsolute);
    menu->exec(this->mapToGlobal(pos));
    menu->deleteLater();
}

void PathEdit::updateActions()
{
    QString path = currentPath();
    if (path.isEmpty() || lePath->isReadOnly()) {
        actMakeRelative->setEnabled(false);
        actMakeAbsolute->setEnabled(false);
    }
    else {
        QFileInfo fi(path);
        bool isFile = fi.isFile();
        actMakeRelative->setEnabled(isFile && fi.isAbsolute());
        actMakeAbsolute->setEnabled(isFile && fi.isRelative());
    }
}

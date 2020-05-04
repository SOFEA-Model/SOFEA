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

#include <QDir>
#include <QFileDialog>
#include <QWidget>

class MenuLineEdit;

QT_BEGIN_NAMESPACE
class QAction;
class QLineEdit;
class QToolButton;
class QMenu;
QT_END_NAMESPACE

class PathEdit : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged USER true)

public:
    explicit PathEdit(QWidget *parent = nullptr);

    QString currentPath() const;
    QString dialogCaption() const;
    QString dialogDirectory() const;
    QString dialogFilter() const;
    QFileDialog::FileMode dialogFileMode() const;
    QFileDialog::AcceptMode dialogAcceptMode() const;
    QString dialogDefaultSuffix() const;

    void setDialogCaption(const QString& caption);
    void setDialogDirectory(const QString& dir);
    void setDialogFilter(const QString& filter);
    void setDialogFileMode(QFileDialog::FileMode mode);
    void setDialogAcceptMode(QFileDialog::AcceptMode mode);
    void setDialogDefaultSuffix(const QString& suffix);
    void setReadOnly(bool readOnly);

signals:
    void currentPathChanged(const QString& path);

public slots:
    void setCurrentPath(const QString& path);

private slots:
    void browse();
    void makeAbsolute();
    void makeRelative();
    void onContextMenuRequested(const QPoint &pos);

private:
    void updateActions();

    MenuLineEdit *lePath;
    QToolButton *btnBrowse;
    QAction *actMakeRelative;
    QAction *actMakeAbsolute;
    QMenu *actionMenu;

    QString dialogCaption_;
    QString dialogDirectory_;
    QString dialogFilter_;
    QFileDialog::FileMode dialogFileMode_ = QFileDialog::ExistingFile;
    QFileDialog::AcceptMode dialogAcceptMode_ = QFileDialog::AcceptOpen;
    QString dialogDefaultSuffix_;
};

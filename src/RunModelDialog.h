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

#ifndef RUNMODELDIALOG_H
#define RUNMODELDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QList>
#include <QMap>
#include <QProcess>
#include <QStandardItemModel>
#include <QTimer>

class ProcessModel;
class Scenario;
class StandardTableView;

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
QT_END_NAMESPACE

class RunModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunModelDialog(QWidget *parent = nullptr);
    void setWorkingDirectory(const QString& path);
    void addScenario(Scenario *);
    void removeScenario(Scenario *);
    void runSelected();
    void pauseSelected();
    void stopSelected();

signals:
    void progressValueChanged(int value);

private slots:
    void contextMenuRequested(const QPoint &pos);

private:
    void setupConnections();

    ProcessModel *model;

    QLineEdit *leWorkingDir;
    QToolButton *btnWorkingDir;
    QLabel *lblThreads;
    QPushButton *btnSelectAll;
    QPushButton *btnDeselectAll;
    QPushButton *btnRun;
    QPushButton *btnPause;
    QPushButton *btnStop;
    StandardTableView *table;
    QAction *openFolderAction;
    QDialogButtonBox *buttonBox;
};

#endif // RUNMODELDIALOG_H

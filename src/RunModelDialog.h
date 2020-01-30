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

#include <boost/ptr_container/ptr_vector.hpp>

#include "Scenario.h"
#include "IPCServer.h"
#include "widgets/StandardTableView.h"

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class RunModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunModelDialog(QWidget *parent = nullptr);
    void setWorkingDirectory(const QString& path);
    void addScenario(Scenario *);

signals:
    void progressValueChanged(int value);
    void progressVisibilityChanged(bool visible);

public slots:
    void runSelected();
    void stopSelected();
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processStateChanged(QProcess::ProcessState newState);
    void processErrorOccurred(QProcess::ProcessError error);
    void updateProgress(quint32 pid, quint32 msg);
    void setElapsedTime();
    void reject() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void executeJob(int i);
    void setupConnections();
    int indexForProcess(const QProcess *) const;
    static QString getProcessState(const QProcess::ProcessState state);
    static QString getProcessError(const QProcess::ProcessError error);

    int maxThreads = 0;
    IPCServer *ipcServer;
    QString workingDir;
    QTimer *timer;

    // Job Manager
    struct Job {
        Scenario *scenario;
        QProcess *process = nullptr;
        QString subDir;
        QString path;
        QDateTime startTime;
        QTime elapsedTime;
        bool queued = false;
    };

    QList<Job *> jobs;
    int activeJobs = 0;
    QMap<qint64, Job *> pidToJob;
    QMap<Job *, qint64> jobToPid;

    QLineEdit *leWorkingDir;
    QLabel *lblThreads;
    QPushButton *btnSelectAll;
    QPushButton *btnDeselectAll;
    QPushButton *btnRun;
    QPushButton *btnStop;
    QStandardItemModel *runModel;
    StandardTableView *runTable;
    QDialogButtonBox *buttonBox;
};

#endif // RUNMODELDIALOG_H

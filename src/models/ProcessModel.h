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

#include <QAbstractTableModel>
#include <QDateTime>
#include <QElapsedTimer>
#include <QProcess>
#include <QString>
#include <QVariant>

#include <map>
#include <memory>
#include <vector>

class IPCServer;
class Scenario;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class ProcessModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ProcessModel(QObject *parent = nullptr);

    enum Column {
        Name,
        Status,
        Started,
        Elapsed,
        Progress,
        Path
    };

    QString workingDirectory() const;
    void setWorkingDirectory(const QString& path);
    void addScenario(Scenario *s);
    void removeScenario(Scenario *s);
    void startJob(int row);
    void suspendJob(int row);
    void resumeJob(int row);
    void stopJob(int row);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceFirst, int count,
                  const QModelIndex &destinationParent, int destinationFirst) override;

signals:
    void progressValueChanged(int value);

private slots:
    void startProcess(int row);
    void onTimeout();
    void onMessageReceived(quint32 pid, quint32 msg);
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessStateChanged(QProcess::ProcessState state);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    struct Job {
        Scenario *scenario = nullptr;
        QProcess *process = nullptr;
        QString status;
        QString subDir;
        QString path;
        QDateTime started;
        QElapsedTimer timer;
        qint64 elapsed = 0;
        bool queued = false;
        bool paused = false;
        int progress = 0;
        int maxProgress = 0;
    };

    void updateTotalProgress();
    static QString processStateString(const QProcess::ProcessState state);
    static QString processErrorString(const QProcess::ProcessError error);

    int activeJobs_ = 0;
    QTimer *timer_;
    IPCServer *ipc_;
    QString workingDir_;
    std::vector<Job> data_;
    std::map<qint64, int> pidToIndex_;
    std::map<int, qint64> indexToPid_;
};

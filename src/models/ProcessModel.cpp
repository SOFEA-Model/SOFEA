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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h> // DebugActiveProcess

#include "IPCServer.h"
#include "ProcessModel.h"
#include "core/Common.h"
#include "core/Scenario.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QThread>

#include <cmath>
#include <algorithm>

ProcessModel::ProcessModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(1000);

    ipc_ = new IPCServer(this);

    connect(timer_, &QTimer::timeout, this, &ProcessModel::onTimeout);
    connect(ipc_, &IPCServer::messageReceived, this, &ProcessModel::onMessageReceived);

    ipc_->start();
    timer_->start();
}

QString ProcessModel::workingDirectory() const
{
    return workingDir_;
}

void ProcessModel::setWorkingDirectory(const QString& path)
{
    workingDir_ = path;
}

void ProcessModel::addScenario(Scenario *s)
{
    // Check for duplicates.
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.scenario == s;
    });

    if (it != data_.end())
        return;

    Job job;
    job.scenario = s;
    job.status = "Not Started";

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    data_.push_back(job);
    endInsertRows();
}

void ProcessModel::removeScenario(Scenario *s)
{
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.scenario == s;
    });

    if (it == data_.end())
        return;

    int row = static_cast<int>(std::distance(data_.begin(), it));

    removeRow(row);
}


void ProcessModel::startJob(int row)
{
    if (row < 0 || row > rowCount())
        return;

    auto& job = data_.at(static_cast<std::size_t>(row));

    // Resume the job if suspended.
    if (job.paused) {
        resumeJob(row);
        return;
    }

    // Do nothing if the job is already running.
    if (job.process != nullptr) {
        auto state = job.process->state();
        if (state == QProcess::Starting || state == QProcess::Running)
            return;
    }

    job.queued = true;
    job.status = "Queued";
    auto statusIndex = this->index(row, Column::Status);
    auto progressIndex = this->index(row, Column::Progress);
    emit dataChanged(statusIndex, progressIndex);

    startProcess(row);
}

void ProcessModel::suspendJob(int row)
{
    if (row < 0 || row > rowCount())
        return;

    auto& job = data_.at(static_cast<std::size_t>(row));

    if (job.process != nullptr && job.process->state() == QProcess::Running && !job.paused)
    {
        auto state = job.process->state();
        if (state != QProcess::Running)
            return;

        DWORD pid = static_cast<DWORD>(job.process->processId());
        BOOL rc = DebugActiveProcess(pid);
        if (rc != 0) {
            job.status = "Paused";
            job.paused = true;
            job.elapsed += job.timer.elapsed();
            job.timer.invalidate();
            auto statusIndex = this->index(row, Column::Status);
            auto progressIndex = this->index(row, Column::Progress);
            emit dataChanged(statusIndex, progressIndex);
        }
    }
}

void ProcessModel::resumeJob(int row)
{
    if (row < 0 || row > rowCount())
        return;

    auto& job = data_.at(static_cast<std::size_t>(row));

    if (job.process != nullptr && job.process->state() == QProcess::Running && job.paused)
    {
        DWORD pid = static_cast<DWORD>(job.process->processId());
        BOOL rc = DebugActiveProcessStop(pid);
        if (rc != 0) {
            job.status = "Running";
            job.paused = false;
            job.timer.start();
            auto statusIndex = this->index(row, Column::Status);
            auto progressIndex = this->index(row, Column::Progress);
            emit dataChanged(statusIndex, progressIndex);
        }
    }
}

void ProcessModel::stopJob(int row)
{
    if (row < 0 || row > rowCount())
        return;

    auto& job = data_.at(static_cast<std::size_t>(row));

    // Remove job from queue to prevent restart on finished() signal.
    job.queued = false;

    if (job.process != nullptr && job.process->state() == QProcess::Running)
    {
        // Console applications on Windows that do not run an event loop,
        // or whose event loop does not handle the WM_CLOSE message, can
        // only be terminated by calling kill().
        job.process->kill();

        // Wait until finished signal or one second as a precaution.
        job.process->waitForFinished(1000);
    }
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(data_.size());
}

int ProcessModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rowCount() || index.row() < 0)
        return QVariant();

    auto& job = data_.at(index.row());

    if (role == Qt::EditRole || role == Qt::DisplayRole)
    {
        switch (index.column()) {
        case Column::Name:
            return job.scenario ? QString::fromStdString(job.scenario->name) : QVariant();
        case Column::Status:
            return job.status;
        case Column::Started:
            return job.started.isValid() ? job.started.toString("yyyy-MM-dd hh:mm:ss") : QVariant();
        case Column::Elapsed: {
            auto msec = job.timer.isValid() ? job.elapsed + job.timer.elapsed() :
                                              job.elapsed;
            auto sec = msec / 1000;
            auto min = (sec / 60) % 60;
            auto hr = (sec / 3600);
            sec = sec % 60;
            return QString("%1:%2:%3").arg(hr, 2, 10, QLatin1Char('0'))
                                      .arg(min, 2, 10, QLatin1Char('0'))
                                      .arg(sec, 2, 10, QLatin1Char('0'));
        }
        case Column::Progress:
        {
            double percent = (double)job.progress / (double)job.maxProgress;
            int progress = std::clamp<int>(std::lround(percent * 100), 0, 100);
            return progress;
        }
        case Column::Path:
            return job.path;
        default: return QVariant();
        }
    }
    else if (role == Qt::DecorationRole && index.column() == Column::Progress)
    {
        // Use a custom highlight color for ProgressBarDelegate.
        if (job.paused)
            return QColor(255, 240, 0); // Yellow
        else
            return QColor(77, 230, 57); // Green
    }

    return QVariant();
}

QVariant ProcessModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Column::Name:     return tr("Name");
        case Column::Status:   return tr("Status");
        case Column::Started:  return tr("Started");
        case Column::Elapsed:  return tr("Elapsed");
        case Column::Progress: return tr("Progress");
        case Column::Path:     return tr("Path");
        default: return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags ProcessModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index);
}

bool ProcessModel::removeRows(int row, int count, const QModelIndex &)
{
    if (row < 0 || count < 1 || row + count > rowCount())
        return false;

    // Stop any running processes.
    for (int i = row; i < row + count; ++i)
        stopJob(i);

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    auto it0 = std::next(data_.begin(), row);
    auto it1 = std::next(it0, count);
    data_.erase(it0, it1);
    endRemoveRows();

    return true;
}

bool ProcessModel::moveRows(const QModelIndex &, int sourceFirst, int count,
                                const QModelIndex &, int destinationFirst)
{
    if (count <= 0 || sourceFirst == destinationFirst)
        return false;

    int sourceLast = sourceFirst + count;
    int destinationLast = destinationFirst + count;

    if (std::min(sourceFirst, destinationFirst) < 0 ||
        std::max(sourceLast, destinationLast) > static_cast<int>(data_.size()))
        return false;

    // Get the extraction range.
    decltype(data_) range(data_.begin() + sourceFirst,
                          data_.begin() + sourceLast);

    // Notify views of new state.
    // See documentation for QAbstractItemModel::beginMoveRows.
    int destinationChild = destinationFirst < sourceFirst ? destinationFirst
                                                          : destinationLast;
    beginMoveRows(QModelIndex(), sourceFirst, sourceLast - 1,
                  QModelIndex(), destinationChild);

    // Erase and insert.
    data_.erase(data_.begin() + sourceFirst,
                data_.begin() + sourceLast);

    data_.insert(data_.begin() + destinationFirst,
                 range.begin(), range.end());

    endMoveRows();

    return true;
}

void ProcessModel::startProcess(int i)
{
    if (i >= rowCount() || i < 0)
        return;

    auto& job = data_.at(static_cast<std::size_t>(i));

    // Keep in queue if the maximum thread count has been reached.
    int maxThreads = QThread::idealThreadCount();
    if (maxThreads >= 0 && activeJobs_ >= maxThreads)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();

    job.status = "Initializing";
    job.started = timestamp;
    job.timer.start();
    job.elapsed = 0;
    job.queued = false;
    emit dataChanged(index(i, 0), index(i, columnCount() - 1));

    // Create the output directory.
    auto s = job.scenario;
    if (!s) {
        job.status = "Scenario Removed";
        auto statusIndex = this->index(i, Column::Status);
        emit dataChanged(statusIndex, statusIndex);
        return;
    }

    job.subDir = QString::fromStdString(s->name) + "_" + timestamp.toString("yyyyMMddhhmmsszzz");
    QDir outputDir(workingDir_);
    if (!outputDir.mkdir(job.subDir)) {
        setData(index(i, Column::Status), processErrorString(QProcess::WriteError));
        return;
    }

    job.path = QDir::cleanPath(workingDir_ + QDir::separator() + job.subDir);

    // Generate the input files.
    QString fluxPath = QDir::cleanPath(job.path + QDir::separator() + "flux.dat");
    QString inputPath = QDir::cleanPath(job.path + QDir::separator() + "aermod.inp");
    s->writeFluxFile(fluxPath.toStdString());
    s->writeInputFile(inputPath.toStdString());

    // Get number of records in surface file for progress calculation.
    job.maxProgress = s->meteorology.surfaceFile.totalHours();

    // Create the process.
    QString exePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + AERMOD_EXE);
    job.process = new QProcess(this);
    job.process->setProgram(exePath);
    job.process->setWorkingDirectory(job.path);

    connect(job.process, &QProcess::started,
            this, &ProcessModel::onProcessStarted);
    connect(job.process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessModel::onProcessFinished);
    connect(job.process, &QProcess::stateChanged,
            this, &ProcessModel::onProcessStateChanged);
    connect(job.process, &QProcess::errorOccurred,
            this, &ProcessModel::onProcessErrorOccurred);

    // Start the process.
    job.process->start();
}

void ProcessModel::onTimeout()
{
    QModelIndex first = index(0, Column::Elapsed);
    QModelIndex last = index(rowCount() - 1, Column::Elapsed);
    emit dataChanged(first, last);
}

void ProcessModel::onMessageReceived(quint32 pid, quint32 msg)
{
    auto it = pidToIndex_.find(pid);
    if (it == pidToIndex_.end())
        return;

    int row = it->second;
    auto& job = data_.at(static_cast<std::size_t>(it->second));

    // Skip update if progress cannot be calculated.
    if (job.maxProgress == 0)
        return;

    job.progress = msg;
    auto progressIndex = index(row, Column::Progress);
    emit dataChanged(progressIndex, progressIndex);

    updateTotalProgress();
}

void ProcessModel::onProcessStarted()
{
    QProcess *process = qobject_cast<QProcess *>(QObject::sender());
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.process == process;
    });

    if (it == data_.end())
        return;

    int row = static_cast<int>(std::distance(data_.begin(), it));

    // Emit progress signal.
    updateTotalProgress();

    // Update the PID maps.
    qint64 pid = process->processId();
    pidToIndex_[pid] = row;
    indexToPid_[row] = pid;

    // Send output directory to IPC server for logging.
    ipc_->addPid(pid, it->subDir.toStdString());

    activeJobs_++;
}

void ProcessModel::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess *>(QObject::sender());
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.process == process;
    });

    if (it == data_.end())
        return;

    int row = static_cast<int>(std::distance(data_.begin(), it));

    // Stop the timer.
    it->elapsed += it->timer.elapsed();
    it->timer.invalidate();

    // Update progress.
    if (it->maxProgress > 0) {
        if (process->exitStatus() == QProcess::NormalExit) {
            it->progress = it->maxProgress;
            auto progressIndex = index(row, Column::Progress);
            emit dataChanged(progressIndex, progressIndex);
        }
        if (process->exitStatus() == QProcess::CrashExit) {
            it->progress = 0;
            auto progressIndex = index(row, Column::Progress);
            emit dataChanged(progressIndex, progressIndex);
        }
    }

    // Emit progress signal.
    updateTotalProgress();

    // Update the PID maps.
    qint64 pid = indexToPid_[row];
    pidToIndex_.erase(pid);
    indexToPid_.erase(row);
    ipc_->removePid(pid);

    activeJobs_--;

    // Start the next job in queue.
    for (auto&& job : data_) {
        if (job.queued) {
            startProcess(row);
            break;
        }
    }
}

void ProcessModel::onProcessStateChanged(QProcess::ProcessState state)
{
    QProcess *process = qobject_cast<QProcess *>(QObject::sender());
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.process == process;
    });

    if (it == data_.end())
        return;

    it->status = processStateString(state);

    int row = static_cast<int>(std::distance(data_.begin(), it));
    auto statusIndex = index(row, Column::Status);
    emit dataChanged(statusIndex, statusIndex);
}

void ProcessModel::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess *>(QObject::sender());
    auto it = std::find_if(data_.begin(), data_.end(), [&](const auto& job) {
        return job.process == process;
    });

    if (it == data_.end())
        return;

    it->status = processErrorString(error);

    int row = static_cast<int>(std::distance(data_.begin(), it));
    auto statusIndex = index(row, Column::Status);
    emit dataChanged(statusIndex, statusIndex);
}

void ProcessModel::updateTotalProgress()
{
    // Calculate aggregate progress for all active jobs.
    int sumProgress = 0, sumMaxProgress = 0;
    for (const auto& job : data_) {
        if (job.process && job.process->state() == QProcess::Running) {
            sumProgress += job.progress;
            sumMaxProgress += job.maxProgress;
        }
    }

    double percent = (double)sumProgress / (double)sumMaxProgress;
    int totalProgress = std::clamp<int>(std::lround(percent * 100), 0, 100);
    emit progressValueChanged(totalProgress);
}

QString ProcessModel::processStateString(const QProcess::ProcessState state)
{
    switch (state) {
    case QProcess::NotRunning:
        return QString("Stopped");
    case QProcess::Starting:
        return QString("Starting");
    case QProcess::Running:
        return QString("Running");
    default:
        return QString();
    }
}

QString ProcessModel::processErrorString(const QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        return QString("Failed to Start");
    case QProcess::Crashed:
        return QString("Crashed");
    case QProcess::Timedout:
        return QString("Timeout");
    case QProcess::WriteError:
        return QString("Write Error");
    case QProcess::ReadError:
        return QString("Read Error");
    case QProcess::UnknownError:
        return QString("Unknown Error");
    default:
        return QString();
    }
}



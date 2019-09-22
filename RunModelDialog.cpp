#include <QtWidgets>

#include "RunModelDialog.h"
#include "delegate/ProgressBarDelegate.h"

const QString AERMODEXE = "sofea_aermod.exe";

RunModelDialog::RunModelDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(QIcon(":/images/ApplicationRunning_32x.png"));
    setWindowTitle("Run Model");
    setWindowFlag(Qt::WindowMaximizeButtonHint);

    //  Returns -1 if the number of processor cores could not be detected.
    maxThreads = QThread::idealThreadCount();
    ipcServer = new IPCServer(this);
    timer = new QTimer(this);
    timer->setInterval(1000);

    leWorkingDir = new QLineEdit;
    leWorkingDir->setReadOnly(true);
    lblThreads = new QLabel(QString("Available processor cores: %1").arg(maxThreads));
    btnSelectAll = new QPushButton("Select All");
    btnDeselectAll = new QPushButton("Deselect All");
    btnRun = new QPushButton("Run");
    btnStop = new QPushButton("Stop");

    runModel = new QStandardItemModel;
    runModel->setColumnCount(5);
    runModel->setHorizontalHeaderLabels(QStringList{"Name", "Status", "Started", "Elapsed", "Progress"});

    runTable = new StandardTableView;
    runTable->setModel(runModel);
    runTable->setAlternatingRowColors(true);
    runTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    runTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    runTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    runTable->setItemDelegateForColumn(4, new ProgressBarDelegate);

    int startingWidth = runTable->font().pointSize();
    runTable->setColumnWidth(0, startingWidth * 12);
    runTable->setColumnWidth(1, startingWidth * 12);
    runTable->setColumnWidth(2, startingWidth * 20);
    runTable->setColumnWidth(3, startingWidth * 10);
    runTable->setColumnWidth(4, startingWidth * 20);
    runTable->setMinimumWidth(startingWidth * 100);

	// Button Box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel("Output directory: "), 0);
    layout1->addWidget(leWorkingDir, 1);

    // Controls Layout
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(btnSelectAll);
    controlsLayout->addWidget(btnDeselectAll);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(btnRun);
    controlsLayout->addWidget(btnStop);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(runTable);
    mainLayout->addWidget(lblThreads);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setupConnections();
    ipcServer->start();
    timer->start();
}

void RunModelDialog::setupConnections()
{
    // UI Signals
    connect(btnSelectAll, &QPushButton::clicked, this, [=]{
        runTable->selectAll();
        runTable->setFocus();
    });
    connect(btnDeselectAll, &QPushButton::clicked, this, [=]{
        runTable->clearSelection();
        runTable->setFocus();
    });
    connect(btnRun, &QPushButton::clicked, this, &RunModelDialog::runSelected);
    connect(btnStop, &QPushButton::clicked, this, &RunModelDialog::stopSelected);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RunModelDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RunModelDialog::accept);

    // IPC Signals
    connect(ipcServer, &IPCServer::messageReceived, this, &RunModelDialog::updateProgress);

    // Timer Signals
    connect(timer, &QTimer::timeout, this, &RunModelDialog::setElapsedTime);

    installEventFilter(this);
}

void RunModelDialog::setWorkingDirectory(const QString& path)
{
    workingDir = path;
    leWorkingDir->setText(workingDir);
}

void RunModelDialog::setTaskbarButton(QWinTaskbarButton *button)
{
    taskbarButton = button;
    taskbarProgress = taskbarButton->progress();
    taskbarProgress->setVisible(true);
}

void RunModelDialog::addScenario(Scenario *s)
{
    Job *job = new Job;
    job->scenario = s;
    jobs.push_back(job);

    // Initialize QStandardItem for all columns
    int i = runModel->rowCount();
    for (int j = 0; j < runModel->columnCount(); ++j)
        runModel->setItem(i, j, new QStandardItem);

    QString name = QString::fromStdString(s->title);
    runModel->item(i, 0)->setText(name);
    runModel->item(i, 4)->setData(0, Qt::DisplayRole); // 0% complete
}

void RunModelDialog::runSelected()
{
    QModelIndexList selectedRows = runTable->selectionModel()->selectedRows();
    for (auto mi = selectedRows.constBegin(); mi != selectedRows.constEnd(); ++mi)
    {
        int i = (*mi).row();

        // Do nothing if the job is already running.
        if (jobs[i]->process != nullptr) {
            if ((jobs[i]->process->state() == QProcess::Starting) ||
                (jobs[i]->process->state() == QProcess::Running)) {
                continue;
            }
        }

        jobs[i]->queued = true;
        runModel->item(i, 1)->setText("Queued");

        executeJob(i);
    }
}

void RunModelDialog::executeJob(int i)
{
    // Keep in queue if the maximum thread count has been reached.
    if (maxThreads >= 0 && activeJobs >= maxThreads)
        return;

    QDateTime timestamp = QDateTime::currentDateTime();

    runModel->item(i, 1)->setText("Processing");
    runModel->item(i, 2)->setText(timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    runModel->item(i, 3)->setText("00:00:00");
    runModel->item(i, 4)->setData(0, Qt::DisplayRole); // 0% complete

    // Start the timer.
    jobs[i]->elapsedTime.start();

    // Remove from queue.
    jobs[i]->queued = false;

    // Create the output directory.
    jobs[i]->subDir = QString::fromStdString(jobs[i]->scenario->title) + "_" + timestamp.toString("yyyyMMddhhmmsszzz");
    QDir outputDir(workingDir);
    if (!outputDir.mkdir(jobs[i]->subDir)) {
        runModel->item(i, 1)->setText(getProcessError(QProcess::WriteError));
        return;
    }
    jobs[i]->path = QDir::cleanPath(workingDir + QDir::separator() + jobs[i]->subDir);

    // Ensure surface file data is updated.
    jobs[i]->scenario->resetSurfaceFileInfo();

    // Generate the input files.
    QString fluxPath = QDir::cleanPath(jobs[i]->path + QDir::separator() + "flux.dat");
    QString inputPath = QDir::cleanPath(jobs[i]->path + QDir::separator() + "aermod.inp");
    jobs[i]->scenario->writeFluxFile(fluxPath.toStdString());
    jobs[i]->scenario->writeInputFile(inputPath.toStdString());

    // Create the process.
    QString exePath = qApp->applicationDirPath() + QDir::separator() + AERMODEXE;
    jobs[i]->process = new QProcess(this);
    jobs[i]->process->setProgram(exePath);
    jobs[i]->process->setWorkingDirectory(jobs[i]->path);

    connect(jobs[i]->process, &QProcess::started,
            this, &RunModelDialog::processStarted);
    connect(jobs[i]->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RunModelDialog::processFinished);
    connect(jobs[i]->process, &QProcess::stateChanged,
            this, &RunModelDialog::processStateChanged);
    connect(jobs[i]->process, &QProcess::errorOccurred,
            this, &RunModelDialog::processErrorOccurred);

    // Start the process.
    jobs[i]->process->start();
}

void RunModelDialog::stopSelected()
{
    QModelIndexList selectedRows = runTable->selectionModel()->selectedRows();
    for (auto mi = selectedRows.constBegin(); mi != selectedRows.constEnd(); ++mi)
    {
        int i = (*mi).row();
        runModel->item(i, 1)->setText("Stopping");

        // Remove job from queue to prevent restart on finished() signal.
        jobs[i]->queued = false;

        if (jobs[i]->process != nullptr)
        {
            // Console applications on Windows that do not run an event loop,
            // or whose event loop does not handle the WM_CLOSE message, can
            // only be terminated by calling kill().
            jobs[i]->process->kill();

            // Wait until finished signal or one second as a precaution.
            jobs[i]->process->waitForFinished(1000);
        }
    }
}

void RunModelDialog::processStarted()
{
    //qDebug() << "Process Started";

    QProcess *process = qobject_cast<QProcess*>(QObject::sender());
    int i = indexForProcess(process);

    // Update the PID maps.
    qint64 pid = process->processId();
    pidToJob[pid] = jobs[i];
    jobToPid[jobs[i]] = pid;

    // Send output directory to IPC server for logging.
    ipcServer->addPid(pid, jobs[i]->subDir.toStdString());

    // Update counters.
    activeJobs++;

    // Disable close button.
    buttonBox->button(QDialogButtonBox::Close)->setEnabled(false);
}

void RunModelDialog::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    //qDebug() << "Process Finished";

    QProcess *process = qobject_cast<QProcess*>(QObject::sender());
    int i = indexForProcess(process);

    // Stop the timer.
    jobs[i]->elapsedTime = QTime();

    // Update progress.
    if (process->exitStatus() == QProcess::NormalExit)
        runModel->item(i, 4)->setData(100, Qt::DisplayRole); // 100% complete
    if (process->exitStatus() == QProcess::CrashExit)
        runModel->item(i, 4)->setData(0, Qt::DisplayRole); // 0% complete

    // Update the PID maps.
    qint64 pid = jobToPid[jobs[i]];
    pidToJob.remove(pid);
    jobToPid.remove(jobs[i]);
    ipcServer->removePid(pid);

    // Update counters.
    activeJobs--;

    // Start the next job in queue.
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->queued == true) {
            executeJob(i);
            break;
        }
    }

    // Enable close button after all jobs are finished.
    if (activeJobs == 0)
        buttonBox->button(QDialogButtonBox::Close)->setEnabled(true);
}

void RunModelDialog::processStateChanged(QProcess::ProcessState newState)
{
    //qDebug() << "Process State Changed: " << getProcessState(newState);

    QProcess *process = qobject_cast<QProcess*>(QObject::sender());
    int i = indexForProcess(process);
    if (i >= 0)
        runModel->item(i, 1)->setText(getProcessState(newState));
}

void RunModelDialog::processErrorOccurred(QProcess::ProcessError error)
{
    //qDebug() << "Process Error Occurred: " << getProcessError(error);

    QProcess *process = qobject_cast<QProcess*>(QObject::sender());
    int i = indexForProcess(process);
    if (i >= 0)
        runModel->item(i, 1)->setText(getProcessError(error));
}

void RunModelDialog::updateProgress(quint32 pid, quint32 msg)
{
    //qDebug() << "Message: PID=" << pid << " NTOTHRS=" << msg;

    if (pidToJob.contains(pid)) {
        Job *job = pidToJob.value(pid);
        int nrec = job->scenario->sfInfo.nrec;

        // Skip update if nrec has not been calculated.
        if (nrec == 0)
            return;

        int i = jobs.indexOf(job);
        if (i >= 0) {
            double percent = static_cast<double>(msg) / static_cast<double>(nrec);
            int progress = std::lrint(percent * 100);
            progress = std::min(std::max(progress, 0), 100);
            runModel->item(i, 4)->setData(progress, Qt::DisplayRole);

            if (taskbarProgress) {
                taskbarProgress->setValue(progress);
            }
        }
    }
}

void RunModelDialog::setElapsedTime()
{
    if (activeJobs == 0)
        return;

    for (int i = 0; i < jobs.size(); ++i) {
        int msec = jobs[i]->elapsedTime.elapsed();
        if (msec == 0)
            continue;

        int sec = msec / 1000;
        int min = (sec / 60) % 60;
        int hr = (sec / 3600);
        sec = sec % 60;
        QString elapsedText = QString("%1:%2:%3")
                .arg(hr, 2, 10, QLatin1Char('0'))
                .arg(min, 2, 10, QLatin1Char('0'))
                .arg(sec, 2, 10, QLatin1Char('0'));

        runModel->item(i, 3)->setText(elapsedText);
    }
}

void RunModelDialog::reject()
{
    if (activeJobs > 0)
        return;

    // Make sure all processes are stopped.
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->process != nullptr) {
            if (jobs[i]->process->state() != QProcess::NotRunning) {
                return;
            }
        }
    }

    // Clear the progress indicator.
    if (taskbarProgress) {
        taskbarProgress->setVisible(false);
    }

    QDialog::reject();
}

bool RunModelDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        // Ignore escape key
        if (ke->key() == Qt::Key_Escape) {
            return true;
        }
    }
    else {
        return QObject::eventFilter(obj, event);
    }

    return QObject::eventFilter(obj, event);
}

/****************************************************************************
** Utility Functions
****************************************************************************/

int RunModelDialog::indexForProcess(const QProcess *process) const
{
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i]->process == process)
            return i;
    }
    return -1;
}

QString RunModelDialog::getProcessState(const QProcess::ProcessState state)
{
    switch (state) {
    case QProcess::NotRunning:
        return QString("Stopped");
    case QProcess::Starting:
        return QString("Starting");
    case QProcess::Running:
        return QString("Running");
    default:
        return QString("");
    }
}

QString RunModelDialog::getProcessError(const QProcess::ProcessError error)
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
        return QString("");
    }
}

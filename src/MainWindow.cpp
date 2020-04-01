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
#include <QBoxLayout>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#include "AppStyle.h"
#include "MainWindow.h"
//#include "RibbonDefinition.h"
#include "ReceptorDialog.h"
#include "RunModelDialog.h"
#include "core/Serialization.h"
#include "core/Validation.h"
#include "core/Common.h"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <csv/csv.h>

#include <fstream>

MainWindow::MainWindow()
{
    //setWindowIcon(QIcon(":/res/SOFEA_64x.png"));
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    int largeIconSize = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
    setIconSize(QSize(largeIconSize, largeIconSize));
    setAcceptDrops(true);

    centralTabWidget = new QTabWidget;
    centralTabWidget->setTabsClosable(true);
    centralTabWidget->setDocumentMode(true);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(5);
    centralLayout->addWidget(centralTabWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *central = new QWidget;
    central->setLayout(centralLayout);
    setCentralWidget(central);

    createMenus();
    createActions();
    createToolbar();
    createPanels();
    setupConnections();
    setupLogging();
    loadSettings();

    QStringList argv = QCoreApplication::arguments();
    if (argv.size() > 1 && QFile::exists(argv.last())) {
        openProjectFile(argv.last());
    }

    projectModified = false;

    statusBar()->showMessage(tr("Ready"), 3000);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createActions()
{
    // Resources
    const QIcon newIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_NewFile));
    const QIcon openIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_OpenFile));
    const QIcon saveIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_Save));
    const QIcon saveAsIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_SaveAs));
    const QIcon validateIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_Checkmark));
    const QIcon runIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_Run));
    const QIcon analyzeIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_Measure));
    const QIcon helpIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_StatusHelp));

    const QIcon cloneIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionClone));
    const QIcon renameIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionRename));
    const QIcon addGroupIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionAddBuildQueue));
    const QIcon importDataIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionImport));
    const QIcon editReceptorsIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionEditorZone));
    const QIcon showTableIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionTable));
    const QIcon showInputFileIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionTextFile));
    const QIcon exportFluxFileIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionExportFile));

    // File Menu actions
    newAct = new QAction(newIcon, tr("&New Scenario"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new scenario"));

    openAct = new QAction(openIcon, tr("&Open Project..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing project"));

    saveAct = new QAction(saveIcon, tr("&Save Project"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current project"));

    saveAsAct = new QAction(saveAsIcon, tr("&Save Project As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the current project to a new file"));

    closeAct = new QAction(tr("Close Project"), this);
    closeAct->setStatusTip(tr("Close the current project"));

    exitAct = new QAction(tr("E&xit"), this);
    //exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
    exitAct->setStatusTip(tr("Exit the application"));

    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    // Tools Menu actions
    validateAct = new QAction(validateIcon, tr("Check Project"), this);
    validateAct->setStatusTip(tr("Check project for errors"));

    runAct = new QAction(runIcon, tr("&Run Model..."), this);
    runAct->setStatusTip(tr("Run the current project"));

    analyzeAct = new QAction(analyzeIcon, tr("Analyze Results..."), this);
    analyzeAct->setStatusTip(tr("Analyze results"));

    optionsAct = new QAction(tr("Options..."), this);
    optionsAct->setStatusTip(tr("Edit global options"));
    optionsAct->setDisabled(true); // FIXME

    toolsMenu->addAction(validateAct);
    toolsMenu->addAction(runAct);
    toolsMenu->addAction(analyzeAct);
    toolsMenu->addSeparator();
    toolsMenu->addAction(optionsAct);

    // Help Menu actions
    helpAct = new QAction(helpIcon, tr("Open User's Guide"), this);
    helpAct->setShortcuts(QKeySequence::HelpContents);
    helpAct->setStatusTip(tr("Open User's Guide"));

    aboutAct = new QAction(tr("About SOFEA"), this);
    aboutAct->setStatusTip(tr("About SOFEA"));

    helpMenu->addAction(helpAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);

    // Scenario Context Menu actions
    scenarioCloneAct = new QAction(cloneIcon, tr("Clone"), this);
    scenarioRenameAct = new QAction(renameIcon, tr("Rename"), this);
    scenarioRemoveAct = new QAction(tr("Remove"), this);
    scenarioAddSourceGroupAct = new QAction(addGroupIcon, tr("Add Source Group"), this);
    scenarioImportValidationAct = new QAction(importDataIcon, tr("Import Retrospective Data..."), this);
    scenarioPropertiesAct = new QAction(tr("Properties..."), this);
    scenarioInputFileAct = new QAction(showInputFileIcon, tr("Show Input File"), this);
    scenarioFluxFileAct = new QAction(exportFluxFileIcon, tr("Export Flux File..."), this);

    // Source Group Context Menu actions
    sourceGroupCloneAct = new QAction(cloneIcon, tr("Clone"), this);
    sourceGroupRenameAct = new QAction(renameIcon, tr("Rename"), this);
    sourceGroupRemoveAct = new QAction(tr("Remove"), this);
    sourceGroupPropertiesAct = new QAction(tr("Properties..."), this);
    editReceptorsAct = new QAction(editReceptorsIcon, tr("Edit Receptors..."), this);
    sourceTableAct = new QAction(showTableIcon, tr("Show Source Table"), this);
}

void MainWindow::createToolbar()
{
    toolbar = addToolBar(tr("Standard"));
    toolbar->setObjectName("MainToolbar");
    toolbar->addAction(newAct);
    toolbar->addAction(openAct);
    toolbar->addAction(saveAct);
    toolbar->addSeparator();
    toolbar->addAction(validateAct);
    toolbar->addAction(runAct);
    toolbar->addAction(analyzeAct);

    // Override drawPrimitive(PE_PanelButtonCommand)?
    //toolbar->setContentsMargins(0, 0, 0, 0);
    //toolbar->layout()->setSpacing(0);
    //toolbar->layout()->setContentsMargins(0, 0, 0, 20);
}

void MainWindow::createPanels()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    // Project Browser
    dwProjectTree = new QDockWidget(tr("Project Browser"), this);
    dwProjectTree->setObjectName("ProjectBrowser");
    dwProjectTree->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);


    //QDockWidget *dwProjectTreeView = new QDockWidget(tr("Project Browser"), this);
    //dwProjectTreeView->setObjectName("ProjectBrowser");
    //dwProjectTreeView->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //
    //projectTreeView = new ProjectTreeView(dwProjectTreeView);
    //projectModel = new ProjectModel(this);
    //auto project = std::make_shared<Project>();
    //project->scenarios.emplace_back(std::make_shared<Scenario>());
    //projectModel->setProject(project);
    //projectTreeView->setModel(projectModel);
    //
    //dwProjectTreeView->setWidget(projectTreeView);
    //addDockWidget(Qt::LeftDockWidgetArea, dwProjectTreeView);


    projectTree = new QTreeWidget(dwProjectTree);
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() + 1);
    projectTree->setFont(font);
    projectTree->setColumnCount(1);
    projectTree->setHeaderHidden(true);
    projectTree->setUniformRowHeights(true);
    projectTree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    projectTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
    projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
    //projectTree->setDragDropMode(QAbstractItemView::InternalMove);
    //projectTree->setAcceptDrops(true);
    //projectTree->setDragEnabled(true);

    dwProjectTree->setWidget(projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, dwProjectTree);

    // Validation LogWidget
    dwValidation = new QDockWidget(tr("Validation"), this);
    dwValidation->setObjectName("Validation");
    dwValidation->setAllowedAreas(Qt::BottomDockWidgetArea);
    lwValidation = new LogWidget(dwValidation);
    dwValidation->setWidget(lwValidation);
    addDockWidget(Qt::BottomDockWidgetArea, dwValidation);

    // Model Output LogWidget
    dwOutput = new QDockWidget(tr("Model Output"), this);
    dwValidation->setObjectName("Model Output");
    dwValidation->setAllowedAreas(Qt::BottomDockWidgetArea);
    lwOutput = new LogWidget(dwOutput);
    dwOutput->setWidget(lwOutput);
    addDockWidget(Qt::BottomDockWidgetArea, dwOutput);

    viewMenu->addAction(dwProjectTree->toggleViewAction());
    viewMenu->addAction(dwValidation->toggleViewAction());
    viewMenu->addAction(dwOutput->toggleViewAction());

    // Tabify LogWidgets
    tabifyDockWidget(dwValidation, dwOutput);
    dwValidation->raise();
}

void MainWindow::setupConnections()
{
    connect(centralTabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::deleteTab);

    //connect(this, &MainWindow::commandActivated, this, &MainWindow::onCommandActivated);
    //connect(this, &MainWindow::commandToggled, this, &MainWindow::onCommandToggled);

    connect(this, &MainWindow::scenarioUpdated, this, &MainWindow::onScenarioUpdated);
    connect(this, &MainWindow::sourceGroupUpdated, this, &MainWindow::onSourceGroupUpdated);


    // File Menu
    connect(newAct, &QAction::triggered, this, &MainWindow::newScenario);
    connect(openAct, &QAction::triggered, this, &MainWindow::openProject);
    connect(saveAct,  &QAction::triggered, this, &MainWindow::saveProject);
    connect(saveAsAct,  &QAction::triggered, this, &MainWindow::saveProject);
    connect(closeAct, &QAction::triggered, this, &MainWindow::closeProject);
    connect(exitAct, &QAction::triggered, this, &MainWindow::exitApplication);

    // Model Menu
    connect(validateAct, &QAction::triggered, this, &MainWindow::validate);
    connect(runAct, &QAction::triggered, this, &MainWindow::runModel);
    connect(analyzeAct, &QAction::triggered, this, &MainWindow::analyzeOutput);

    // Help Menu
    connect(helpAct, &QAction::triggered, this, &MainWindow::showHelp);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    // Project Tree
    connect(projectTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::contextMenuRequested);
    connect(projectTree, &QTreeWidget::itemChanged, this, &MainWindow::onItemChanged);
    //connect(projectTree, &QTreeWidget::currentItemChanged, this, &MainWindow::onCurrentItemChanged);
}

void MainWindow::setupLogging()
{
    // Configure columns and filter menu for each log widget.
    QFontMetrics fm = fontMetrics();
    int maxCharWidth = fm.horizontalAdvance(QChar('W'));
    int avgCharWidth = fm.averageCharWidth();

    lwValidation->setColumn(0, "Message", "", 0);
    lwValidation->setColumn(1, "Source", "Source", avgCharWidth * 50);
    lwValidation->setFilterKeyColumn(1);
    lwValidation->setFilterValues(QStringList{"Distribution", "Geometry", "Import", "Export", "Model", "Analysis", "General"});

    lwOutput->setColumn(0, "Message", "", 0);
    lwOutput->setColumn(1, "PW", "Pathway", maxCharWidth * 2);
    lwOutput->setColumn(2, "Code", "ErrorCode", maxCharWidth * 4);
    lwOutput->setColumn(3, "Line", "Line", maxCharWidth * 8);
    lwOutput->setColumn(4, "Module", "Module", maxCharWidth * 12);
    lwOutput->setColumn(5, "Directory", "Dir", avgCharWidth * 50);
    lwOutput->setFilterKeyColumn(1);
    lwOutput->setFilterValues(QStringList{"CO", "SO", "RE", "ME", "EV", "OU", "MX", "CN"});
    lwOutput->setUniformRowHeights(true);

    // Get a pointer to the global logging core.
    auto core = boost::log::core::get();

    // Set a global severity filter.
#ifndef SOFEA_DEBUG
    core->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

    // Register custom sinks, with attribute presence filter on "Dir".
    // The Dir attribute should be present for all model output messages.
    typedef boost::log::sinks::synchronous_sink<LogWidgetBackend> sink_t;

    auto messageBackend = boost::make_shared<LogWidgetBackend>(lwValidation);
    messageBackend->setKeywords(QStringList{"Source"});
    auto messageSink = boost::make_shared<sink_t>(messageBackend);
    messageSink->set_filter(!boost::log::expressions::has_attr("Dir"));
    core->add_sink(messageSink);

    auto outputBackend = boost::make_shared<LogWidgetBackend>(lwOutput);
    outputBackend->setKeywords(QStringList{"Dir", "Pathway", "ErrorCode", "Line", "Module"});
    auto outputSink = boost::make_shared<sink_t>(outputBackend);
    outputSink->set_filter(boost::log::expressions::has_attr("Dir"));
    core->add_sink(outputSink);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    // Default main window size is 75% main screen size
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QSize defaultSize = primaryScreen->availableGeometry().size() * 0.75;

    // Load settings
    QByteArray state = settings.value("State", QByteArray()).toByteArray();
    QByteArray geometry = settings.value("Geometry", QByteArray()).toByteArray();
    bool maximized = settings.value("Maximized", false).toBool();
    QSize size = settings.value("Size", defaultSize).toSize();

    // Restore size/state of main window
    this->restoreState(state);
    this->restoreGeometry(geometry);
    this->resize(size);
    if (maximized)
        this->showMaximized();

    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.remove("");

    // Save size/state of main window
    settings.setValue("State", this->saveState());
    settings.setValue("Geometry", this->saveGeometry());
    bool maximized = this->isMaximized();
    settings.setValue("Maximized", maximized);
    if (!maximized)
        settings.setValue("Size", this->size());

    settings.endGroup();
}

/****************************************************************************
** Slots
****************************************************************************/
/*
void MainWindow::onCommandActivated(int commandId)
{
    switch (commandId) {
    case IDC_RIBBONHELP:
        showHelp();
        break;
    case IDC_OPEN:
    case IDC_OPENPROJECT:
        openProject();
        break;
    case IDC_VALIDATE:
        validate();
        break;
    case IDC_RUN:
        runModel();
        break;
    case IDC_ANALYZE:
        analyzeOutput();
        break;
    case IDC_INPUTFILE:
    case IDC_SOURCETABLE:
    case IDC_RECEPTOREDITOR:
        break;
    case IDC_SELECTALL:
        emit selectAllClicked();
        break;
    case IDC_SELECTNONE:
        emit selectNoneClicked();
        break;
    case IDC_SELECTINVERSE:
        emit selectInverseClicked();
        break;
    default:
        break;
    }
}

void MainWindow::onCommandToggled(int commandId, bool checked)
{
    switch (commandId) {
    case IDC_SHOWPROJECTBROWSER:
        dwProjectTree->setVisible(checked);
        break;
    case IDC_SHOWVALIDATION:
        dwValidation->setVisible(checked);
        break;
    case IDC_SHOWMODELOUTPUT:
        dwOutput->setVisible(checked);
        break;
    default:
        break;
    }
}

void MainWindow::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)

    if (current && treeWidgetToScenario.count(current) > 0) {
        //Scenario *s = treeWidgetToScenario[current];
        setCommandEnabled(IDC_PROPERTIES, true);
        setCommandEnabled(IDC_INPUTFILE, true);
        setCommandEnabled(IDC_SOURCETABLE, false);
        setCommandEnabled(IDC_RECEPTOREDITOR, true);
        setCommandEnabled(IDC_WINDROSE, true);
    }
    else if (current && treeWidgetToSourceGroup.count(current) > 0) {
        //SourceGroup *sg = treeWidgetToSourceGroup[current];
        setCommandEnabled(IDC_PROPERTIES, true);
        setCommandEnabled(IDC_INPUTFILE, false);
        setCommandEnabled(IDC_SOURCETABLE, true);
        setCommandEnabled(IDC_RECEPTOREDITOR, false);
        setCommandEnabled(IDC_WINDROSE, false);
    }
    else {
        setCommandEnabled(IDC_PROPERTIES, false);
        setCommandEnabled(IDC_INPUTFILE, false);
        setCommandEnabled(IDC_SOURCETABLE, false);
        setCommandEnabled(IDC_RECEPTOREDITOR, false);
        setCommandEnabled(IDC_WINDROSE, false);
    }
}
*/
void MainWindow::onItemChanged(QTreeWidgetItem *item, int)
{
    // Handle rename, making sure name is valid.
    QString text = item->text(0); // new text

    // Remove any non-alphanumeric characters and truncate to max length (100)
    text.remove(QRegExp("[^A-Za-z0-9_]+"));
    text.truncate(100);
    item->setText(0, text);

    if (treeWidgetToScenario.count(item) > 0) {
        Scenario *s = treeWidgetToScenario[item];
        s->name = text.toStdString();
        emit scenarioUpdated(s);
        return;
    }

    if (treeWidgetToSourceGroup.count(item) > 0) {
        SourceGroup *sg = treeWidgetToSourceGroup[item];
        sg->grpid = text.toStdString();
        emit sourceGroupUpdated(sg);
        return;
    }
}

void MainWindow::contextMenuRequested(const QPoint& pos)
{
    QPoint globalPos = projectTree->viewport()->mapToGlobal(pos);
    QMenu contextMenu;

    QTreeWidgetItem *item = projectTree->itemAt(pos);
    QAction *selectedItem;

    // Scenario Context Menu
    // TODO: Import Sources
    if (item && treeWidgetToScenario.count(item) > 0)
    {
        Scenario *s = treeWidgetToScenario[item];
        contextMenu.addAction(scenarioCloneAct);
        contextMenu.addAction(scenarioRenameAct);
        contextMenu.addAction(scenarioRemoveAct);
        contextMenu.addSeparator();
        contextMenu.addAction(scenarioAddSourceGroupAct);
        contextMenu.addAction(scenarioImportValidationAct);
        contextMenu.addAction(scenarioFluxFileAct);
        contextMenu.addSeparator();
        contextMenu.addAction(scenarioPropertiesAct);
        contextMenu.addSeparator();
        contextMenu.addAction(editReceptorsAct);
        contextMenu.addSeparator();
        contextMenu.addAction(scenarioInputFileAct);

        selectedItem = contextMenu.exec(globalPos);
        if (selectedItem && selectedItem == scenarioCloneAct) {
            cloneScenario(s);
        }
        else if (selectedItem && selectedItem == scenarioRenameAct) {
            projectTree->editItem(item);
        }
        else if (selectedItem && selectedItem == scenarioRemoveAct) {
            removeScenario(s);
        }
        else if (selectedItem && selectedItem == scenarioAddSourceGroupAct) {
            newSourceGroup(s);
        }
        else if (selectedItem && selectedItem == scenarioImportValidationAct) {
            importValidationData(s);
        }
        else if (selectedItem && selectedItem == scenarioPropertiesAct) {
            showScenarioProperties(s);
        }
        else if (selectedItem && selectedItem == editReceptorsAct) {
            showReceptorEditor(s);
        }
        else if (selectedItem && selectedItem == scenarioInputFileAct) {
            showInputViewer(s);
        }
        else if (selectedItem && selectedItem == scenarioFluxFileAct) {
            exportFluxFile(s);
        }
        return;
    }

    // Source Group Context Menu
    if (item && treeWidgetToSourceGroup.count(item) > 0)
    {
        SourceGroup *sg = treeWidgetToSourceGroup[item];
        contextMenu.addAction(sourceGroupCloneAct);
        contextMenu.addAction(sourceGroupRenameAct);
        contextMenu.addAction(sourceGroupRemoveAct);
        contextMenu.addSeparator();
        contextMenu.addAction(sourceGroupPropertiesAct);
        contextMenu.addSeparator();
        contextMenu.addAction(sourceTableAct);

        selectedItem = contextMenu.exec(globalPos);
        if (selectedItem && selectedItem == sourceGroupCloneAct) {
            cloneSourceGroup(sg);
        }
        else if (selectedItem && selectedItem == sourceGroupRenameAct) {
            projectTree->editItem(item);
        }
        else if (selectedItem && selectedItem == sourceGroupRemoveAct) {
            removeSourceGroup(sg);
        }
        else if (selectedItem && selectedItem == sourceGroupPropertiesAct) {
            showSourceGroupProperties(sg);
        }
        else if (selectedItem && selectedItem == sourceTableAct) {
            showSourceTable(sg);
        }
        return;
    }
}

void MainWindow::onScenarioUpdated(Scenario *s)
{
    projectModified = true;
    if (scenarioToInputViewer.count(s) > 0) {
        InputViewer *viewer = scenarioToInputViewer[s];
        int viewerIndex = centralTabWidget->indexOf(viewer);
        if (viewerIndex >= 0) {
            viewer->refresh();
            QString viewerTitle = QString::fromStdString(s->name);
            centralTabWidget->setTabText(viewerIndex, viewerTitle);
        }
    }
    for (const auto &item : sourceTableToSourceGroup) {
        SourceTable *table = item.first;
        SourceGroup *sg = item.second;
        if (sourceGroupToScenario[sg] == s) {
            table->refresh();
        }
    }
}

void MainWindow::onSourceGroupUpdated(SourceGroup *sg)
{
    projectModified = true;
    if (sourceGroupToSourceTable.count(sg) > 0) {
        SourceTable *table = sourceGroupToSourceTable[sg];
        int tableIndex = centralTabWidget->indexOf(table);
        if (tableIndex >= 0) {
            table->refresh();
            QString tableTitle = QString::fromStdString(sg->grpid);
            centralTabWidget->setTabText(tableIndex, tableTitle);
        }
    }
}

void MainWindow::openProject()
{
    QSettings settings;
    QString openFile;
    QString examplesDir = QDir::cleanPath(qApp->applicationDirPath() + QDir::separator() + "examples");
    QString currentDir = settings.value("DefaultDirectory", examplesDir).toString();
    openFile = QFileDialog::getOpenFileName(this,
                                            tr("Open Project"),
                                            currentDir,
                                            tr("SOFEA Project (*.sofea)"));

    if (openFile.isEmpty())
        return;

    openProjectFile(openFile);
}

void MainWindow::openProjectFile(const QString& openFile)
{
    QFileInfo fi(openFile);
    if (fi.suffix().toLower() != "sofea")
        return;

    // Close current project.
    closeProject();

    // Read the scenario data.
    std::string ifile = openFile.toStdString();

    try {
        std::ifstream is(ifile);
        // Simple check for JSON: first character is '{'
        if (is.peek() == 0x7b) {
            cereal::JSONInputArchive ia(is);
            ia(scenarios);
        }
        else {
            //cereal::PortableBinaryInputArchive ia(is);
            //ia(scenarios);
        }
    } catch (const cereal::Exception &e) {
        QMessageBox::critical(this, "Parse Error", QString::fromLocal8Bit(e.what()));
        return;
    }

    // Update containers and tree.
    for (Scenario &s : scenarios) {
        addScenarioToTree(&s);
        for (auto sgptr : s.sourceGroups) {
            sourceGroupToScenario[sgptr.get()] = &s;
            addSourceGroupToTree(sgptr.get(), &s);
        }
    }

    // Update project file and working directory.
    projectFile = openFile;
    QString projectDir = fi.canonicalPath();

    QSettings settings;
    settings.setValue("DefaultDirectory", projectDir);

    projectModified = false;
}

void MainWindow::saveProject()
{
    if (scenarios.size() == 0)
        return;

    // saveAct or saveAsAct
    QAction *sender = qobject_cast<QAction *>(QObject::sender());

    if (projectFile.isEmpty() || sender == saveAsAct) {
        QSettings settings;
        QString saveFile;
        QString currentDir = settings.value("DefaultDirectory", QDir::currentPath()).toString();
        saveFile = QFileDialog::getSaveFileName(this,
                                                tr("Save Project"),
                                                currentDir,
                                                tr("SOFEA Project (*.sofea)"));

        if (saveFile.isEmpty())
            return;
    }

    saveProjectFile(projectFile);
}

void MainWindow::saveProjectFile(const QString& saveFile)
{
    if (scenarios.size() == 0)
        return;

    // Write the scenario data.
    std::string ofile = saveFile.toStdString();

    try {
        std::ofstream os(ofile);
        cereal::JSONOutputArchive oa(os, cereal::JSONOutputArchive::Options::NoIndent());
        oa(scenarios);
    } catch (const cereal::Exception &e) {
        QMessageBox::critical(this, "Parse Error", QString::fromLocal8Bit(e.what()));
        return;
    }

    // Update project file and working directory.
    projectFile = saveFile;
    QFileInfo fi(saveFile);
    QString projectDir = fi.canonicalPath();

    QSettings settings;
    settings.setValue("DefaultDirectory", projectDir);

    projectModified = false;
}

void MainWindow::closeProject()
{
    if (scenarios.size() == 0)
        return;

    if (projectModified) {
        // Get user confirmation.
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The current project has unsaved changes. Are you sure you want to close?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int rc = msgBox.exec();
        if (rc == QMessageBox::No)
            return;
    }

    // Clear messages.
    lwValidation->clear();
    lwOutput->clear();

    // Remove all scenarios.
    for (auto it = scenarios.rbegin(); it != scenarios.rend(); ++it)
        removeScenario(&(*it));

    // Clear project data.
    projectFile.clear();

    projectModified = false;
}

void MainWindow::exitApplication()
{
    QApplication::closeAllWindows();
}

void MainWindow::newScenario()
{
    Scenario *s = new Scenario();
    scenarios.push_back(s);
    addScenarioToTree(s);
}

void MainWindow::newSourceGroup(Scenario *s)
{
    auto sgptr = std::make_shared<SourceGroup>();
    s->sourceGroups.push_back(sgptr);
    sourceGroupToScenario[sgptr.get()] = s;
    addSourceGroupToTree(sgptr.get(), s);
}

void MainWindow::importValidationData(Scenario *s)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");

    QSettings settings;
    QString csvfile;
    QString examplesDir = QDir::cleanPath(qApp->applicationDirPath() + QDir::separator() + "examples");
    QString currentDir = settings.value("DefaultDirectory", examplesDir).toString();
    csvfile = QFileDialog::getOpenFileName(this,
                                           tr("Import Retrospective Data"),
                                           currentDir,
                                           tr("Retrospective Data (*.csv)"));

    if (csvfile.isEmpty())
        return;

    // Create a new SourceGroup with area sources
    auto sgptr = std::make_shared<SourceGroup>();
    s->sourceGroups.push_back(sgptr);
    sourceGroupToScenario[sgptr.get()] = s;

    // Set validation mode by default
    sgptr->validationMode = true;

    // Read the CSV file
    std::string srcid;
    double xs, ys, xinit, yinit, apprate;
    std::string start;
    double incdepth;

    try {
        io::CSVReader<8> in(csvfile.toStdString());
        in.read_header(io::ignore_extra_column, "srcid", "xs", "ys", "xinit", "yinit", "apprate", "start", "incdepth");
        while (true) {
            bool ok = in.read_row(srcid, xs, ys, xinit, yinit, apprate, start, incdepth);
            if (!ok)
                break;
            AreaSource *as = new AreaSource;
            as->srcid = srcid;
            as->xs = xs;
            as->ys = ys;
            as->xinit = xinit;
            as->yinit = yinit;
            as->appRate = apprate;
            as->appStart = QDateTime::fromString(QString::fromStdString(start), Qt::ISODate);
            as->appStart.setTimeSpec(Qt::UTC);
            as->incorpDepth = incdepth;
            as->setGeometry();
            sgptr->sources.push_back(as);
        }
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "Import Failed", QString::fromLocal8Bit(e.what()));
        return;
    }

    addSourceGroupToTree(sgptr.get(), s);
}

void MainWindow::addScenarioToTree(Scenario *s)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;

    scenarioToTreeWidget[s] = item;
    treeWidgetToScenario[item] = s;

    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
    item->setText(0, QString::fromStdString(s->name));

    QFont rootFont = item->font(0);
    rootFont.setBold(true);
    item->setFont(0, rootFont);

    projectTree->addTopLevelItem(item);

    projectModified = true;
}

void MainWindow::addSourceGroupToTree(SourceGroup *sg, Scenario *s)
{
    QTreeWidgetItem *parent = scenarioToTreeWidget[s];
    QTreeWidgetItem *item = new QTreeWidgetItem;

    sourceGroupToTreeWidget[sg] = item;
    treeWidgetToSourceGroup[item] = sg;

    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    item->setText(0, QString::fromStdString(sg->grpid));

    parent->addChild(item);
    parent->setExpanded(true);

    projectModified = true;
}

void MainWindow::removeScenario(Scenario *s)
{
    // Remove scenario tabs.
    if (scenarioToInputViewer.count(s) > 0) {
        InputViewer *viewer = scenarioToInputViewer[s];
        int index = centralTabWidget->indexOf(viewer);
        deleteTab(index);
    }

    // Remove all source groups.
    for (auto it = s->sourceGroups.rbegin(); it != s->sourceGroups.rend(); ++it)
        removeSourceGroup((*it).get());

    // Remove from tree.
    QTreeWidgetItem *item = scenarioToTreeWidget[s];
    scenarioToTreeWidget.erase(s);
    treeWidgetToScenario.erase(item);
    delete item;

    // Remove from ptr_vector using erase–remove idiom.
    // Object will be automatically deallocated.
    for (auto it = scenarios.begin(); it != scenarios.end(); ) {
        if (&(*it) == s)
            it = scenarios.erase(it);
        else
            ++it;
    }

    projectModified = true;
}

void MainWindow::removeSourceGroup(SourceGroup *sg)
{
    // Remove tabs.
    if (sourceGroupToSourceTable.count(sg) > 0) {
        SourceTable *table = sourceGroupToSourceTable[sg];
        int index = centralTabWidget->indexOf(table);
        deleteTab(index);
    }

    // Find parent.
    Scenario *s = sourceGroupToScenario.find(sg)->second;

    // Remove from tree.
    QTreeWidgetItem *item = sourceGroupToTreeWidget[sg];
    sourceGroupToTreeWidget.erase(sg);
    treeWidgetToSourceGroup.erase(item);
    delete item;

    // Remove from maps.
    auto it = sourceGroupToScenario.find(sg);
    if (it != sourceGroupToScenario.end())
        sourceGroupToScenario.erase(it);

    // Remove from ptr_vector using erase–remove idiom.
    // Object will be automatically deallocated.
    for (auto it = s->sourceGroups.begin(); it != s->sourceGroups.end(); ) {
        if ((*it).get() == sg)
            it = s->sourceGroups.erase(it);
        else
            ++it;
    }

    projectModified = true;
}

void MainWindow::cloneScenario(Scenario *s)
{
    Scenario *clone = new Scenario(*s); // copy constructor
    scenarios.push_back(clone);
    addScenarioToTree(clone);
    for (auto sgptr : clone->sourceGroups) {
        sourceGroupToScenario[sgptr.get()] = clone;
        addSourceGroupToTree(sgptr.get(), clone);
    }
}

void MainWindow::cloneSourceGroup(SourceGroup *sg)
{
    Scenario *s = sourceGroupToScenario.find(sg)->second;
    auto clone = std::make_shared<SourceGroup>(*sg); // copy constructor
    s->sourceGroups.push_back(clone);
    sourceGroupToScenario[clone.get()] = s;
    addSourceGroupToTree(clone.get(), s);
}

void MainWindow::showScenarioProperties(Scenario *s)
{
    ScenarioProperties *dialog = new ScenarioProperties(s, this);
    connect(dialog, &ScenarioProperties::saved, [&]() {
        emit scenarioUpdated(s);
    });
    dialog->exec();
}

void MainWindow::showSourceGroupProperties(SourceGroup *sg)
{
    Scenario *s = sourceGroupToScenario[sg];
    SourceGroupProperties *dialog = new SourceGroupProperties(s, sg, this);
    connect(dialog, &SourceGroupProperties::saved, [&]() {
        emit scenarioUpdated(s);
        emit sourceGroupUpdated(sg);
    });
    dialog->exec();
}

void MainWindow::showReceptorEditor(Scenario *s)
{
    ReceptorDialog *dialog = new ReceptorDialog(s, this);
    int rc = dialog->exec();
    if (rc == QDialog::Accepted) {
        emit scenarioUpdated(s);
    }
}

void MainWindow::showInputViewer(Scenario *s)
{
    // If the tab already exists, refresh and set current.
    // Otherwise, create a new tab.

    if (scenarioToInputViewer.count(s) > 0) {
        InputViewer *viewer = scenarioToInputViewer[s];
        viewer->refresh();
        int index = centralTabWidget->indexOf(viewer);
        if (index >= 0)
            centralTabWidget->setCurrentIndex(index);
    }
    else {
        InputViewer *viewer = new InputViewer(s);
        viewer->refresh();

        QString title = QString::fromStdString(s->name);
        centralTabWidget->addTab(viewer, title);
        centralTabWidget->setCurrentIndex(centralTabWidget->count() - 1);

        scenarioToInputViewer[s] = viewer;
        inputViewerToScenario[viewer] = s;
    }
}

void MainWindow::showSourceTable(SourceGroup *sg)
{
    // If the tab already exists, refresh and set current.
    // Otherwise, create a new tab.

    if (sourceGroupToSourceTable.count(sg) > 0) {
        SourceTable *table = sourceGroupToSourceTable[sg];
        int index = centralTabWidget->indexOf(table);
        if (index >= 0)
            centralTabWidget->setCurrentIndex(index);
    }
    else {
        Scenario *s = sourceGroupToScenario[sg];
        SourceTable *table = new SourceTable(s, sg);

        QString title = QString::fromStdString(sg->grpid);
        centralTabWidget->addTab(table, title);
        centralTabWidget->setCurrentIndex(centralTabWidget->count() - 1);

        sourceGroupToSourceTable[sg] = table;
        sourceTableToSourceGroup[table] = sg;
    }

    connect(sourceGroupToSourceTable[sg], &SourceTable::dataChanged, [=]() {
        projectModified = true;
    });
}

void MainWindow::exportFluxFile(Scenario *s)
{
    QSettings settings;
    QString saveFile;
    QString currentDir = settings.value("DefaultDirectory", QDir::currentPath()).toString();
    saveFile = QFileDialog::getSaveFileName(this,
                                            tr("Export Flux File"),
                                            currentDir,
                                            tr("Flux File (*.dat)"));

    if (!saveFile.isEmpty()) {
        s->writeFluxFile(saveFile.toStdString());
    }
}

void MainWindow::deleteTab(int index)
{
    // Remove tab from containers, and schedule the widget for deletion.
    // This will also remove the tab.

    if (index < 0)
        return;

    QWidget *widget = centralTabWidget->widget(index);

    InputViewer *viewer = qobject_cast<InputViewer *>(widget);
    if (viewer != nullptr) {
        Scenario *s = inputViewerToScenario[viewer];
        scenarioToInputViewer.erase(s);
        inputViewerToScenario.erase(viewer);
        widget->deleteLater();
        return;
    }

    SourceTable *table = qobject_cast<SourceTable *>(widget);
    if (table != nullptr) {
        SourceGroup *sg = sourceTableToSourceGroup[table];
        sourceGroupToSourceTable.erase(sg);
        sourceTableToSourceGroup.erase(table);
        widget->deleteLater();
        return;
    }
}

void MainWindow::validate()
{
    lwValidation->clear();
    dwValidation->raise();

    for (Scenario &s : scenarios) {
        auto res = Validation::ValidateScenario(s);
    }
}

void MainWindow::runModel()
{
    RunModelDialog *dialog = new RunModelDialog(this);

    QSettings settings;
    QString defaultDir = settings.value("DefaultDirectory", QDir::currentPath()).toString();
    dialog->setWorkingDirectory(defaultDir);

    for (Scenario &s : scenarios) {
        dialog->addScenario(&s);
    }

#ifdef Q_OS_WIN
    if (taskbarProgress) {
        connect(dialog, &RunModelDialog::progressValueChanged,
                taskbarProgress, &QWinTaskbarProgress::setValue);
        connect(dialog, &RunModelDialog::progressVisibilityChanged,
                taskbarProgress, &QWinTaskbarProgress::setVisible);
    }
#endif

    dwOutput->raise();
    dialog->exec();
}

void MainWindow::analyzeOutput()
{
    if (analysisWindow == nullptr) {
        analysisWindow = new AnalysisWindow(this);
    }

    analysisWindow->show();

    // Centering
    analysisWindow->move(window()->frameGeometry().topLeft() +
                         window()->rect().center() -
                         analysisWindow->rect().center());
}

void MainWindow::showHelp()
{
    QDesktopServices::openUrl(QUrl(SOFEA_DOCUMENTATION_URL));
}

void MainWindow::about()
{
    QString aboutText;
    aboutText += "<h3>Soil Fumigant Exposure Assessment (SOFEA) Modeling System</h3>";
    aboutText += QString("<h4>Version %1 <i>ALPHA</i>").arg(SOFEA_VERSION_STRING);
#if _WIN64
    aboutText += " 64-bit</h4>";
#elif _WIN32
    aboutText += " 32-bit</h4>";
#else
    aboutText += "</h4>";
#endif
    aboutText += "<p>&copy; 2004-2020 Dow Inc. All rights reserved.</p>";

    // Compiler Info
    aboutText += QString("Built with MSVC %1").arg(_MSC_VER) +
                 QString(" and Qt %1").arg(QT_VERSION_STR) +
                 QString(" on %1").arg(__DATE__);

    // Dispersion Model Info
    aboutText += "<h4>Model Version Information:</h4>";
    aboutText += QString("AERMOD %1").arg(AERMOD_VERSION_STRING);

    // Contributor Info
    aboutText += "<h4>Developers:</h4>";
    aboutText += "John Buonagurio (jbuonagurio@exponent.com)<br>";
    aboutText += "Steven A. Cryer (steven.cryer@corteva.com)<br>";
    aboutText += "Ian van Wesenbeeck (ian@illahe-environmental.com)";

    QMessageBox::about(this, tr("About SOFEA"), aboutText);
}

/****************************************************************************
** Events
****************************************************************************/

void MainWindow::showEvent(QShowEvent *event)
{
    //setCommandEnabled(IDC_PROPERTIES, false);
    //setCommandEnabled(IDC_INPUTFILE, false);
    //setCommandEnabled(IDC_SOURCETABLE, false);
    //setCommandEnabled(IDC_RECEPTOREDITOR, false);
    //setCommandEnabled(IDC_WINDROSE, false);

    //setCommandChecked(IDC_SHOWPROJECTBROWSER, true);
    //setCommandChecked(IDC_SHOWVALIDATION, true);
    //setCommandChecked(IDC_SHOWMODELOUTPUT, true);

    //setTabGroupContextAvailability(IDC_TABGROUP_SOURCETOOLS, ContextAvailability::Available);

#ifdef Q_OS_WIN
    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(windowHandle());
    taskbarProgress = taskbarButton->progress();
#endif

    QMainWindow::showEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (projectModified) {
        // Get user confirmation.
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The current project has unsaved changes. Are you sure you want to exit?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        int rc = msgBox.exec();
        if (rc == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    event->accept();
    QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
    QMainWindow::dragMoveEvent(event);
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
    QMainWindow::dragLeaveEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString openFile = urlList.first().toLocalFile();
        openProjectFile(openFile);
    }

    event->acceptProposedAction();
    QMainWindow::dropEvent(event);
}

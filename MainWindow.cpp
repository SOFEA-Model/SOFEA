#include <QtWidgets>

#include "Common.h"
#include "Serialization.h"
#include "MainWindow.h"
#include "ReceptorDialog.h"
#include "RunModelDialog.h"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <csv/csv.h>

#include <fstream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/images/Corteva_64x.png"));
    setUnifiedTitleAndToolBarOnMac(true);

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

    //QStringList argv = QCoreApplication::arguments();
    //if (argv.size() > 1 && QFile::exists(argv.last())) {
    //    openProject(argv.last());
    //}

    projectModified = false;

    statusBar()->showMessage(tr("Ready"), 2000);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    modelMenu = menuBar()->addMenu(tr("&Model"));
    helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createActions()
{
    // Resources
    const QIcon newIcon = QIcon(":/images/NewFile_32x.png");
    const QIcon openIcon = QIcon(":/images/OpenFolder_32x.png");
    const QIcon saveIcon = QIcon(":/images/Save_32x.png");
    const QIcon saveAsIcon = QIcon(":/images/SaveAs_32x.png");
    const QIcon runIcon = QIcon(":/images/Run_32x.png");
    const QIcon analyzeIcon = QIcon(":/images/RunPerformance_32x.png");
    const QIcon helpIcon = QIcon(":/images/MSHelpTableOfContent_32x.png");
    const QIcon cloneIcon = QIcon(":/images/CopyItem_16x.png");
    const QIcon renameIcon = QIcon(":/images/Rename_16x.png");
    const QIcon addGroupIcon = QIcon(":/images/AddBuildQueue_16x.png");
    const QIcon importIcon = QIcon(":/images/ImportFile_16x.png");
    const QIcon receptorsIcon = QIcon(":/images/EditReceptors_16x.png");
    const QIcon tableIcon = QIcon(":/images/Table_16x.png");
    const QIcon textFileIcon = QIcon(":/images/TextFile_16x.png");
    const QIcon exportFileIcon = QIcon(":/images/ExportFile_16x.png");

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

    // Model Menu actions
    runAct = new QAction(runIcon, tr("&Run Model..."), this);
    runAct->setStatusTip(tr("Run the current project"));

    analyzeAct = new QAction(analyzeIcon, tr("Analyze Results..."), this);
    analyzeAct->setStatusTip(tr("Analyze results"));

    modelMenu->addAction(runAct);
    modelMenu->addAction(analyzeAct);

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
    scenarioImportValidationAct = new QAction(importIcon, tr("Import Retrospective Data..."), this);
    scenarioPropertiesAct = new QAction(tr("Properties..."), this);
    scenarioInputFileAct = new QAction(textFileIcon, tr("Show Input File"), this);
    scenarioFluxFileAct = new QAction(exportFileIcon, tr("Export Flux File..."), this);

    // Source Group Context Menu actions
    sourceGroupCloneAct = new QAction(cloneIcon, tr("Clone"), this);
    sourceGroupRenameAct = new QAction(renameIcon, tr("Rename"), this);
    sourceGroupRemoveAct = new QAction(tr("Remove"), this);
    sourceGroupPropertiesAct = new QAction(tr("Properties..."), this);
    editReceptorsAct = new QAction(receptorsIcon, tr("Edit Receptors..."), this);
    sourceTableAct = new QAction(tableIcon, tr("Show Source Table"), this);
}

void MainWindow::createToolbar()
{
    toolbar = addToolBar(tr("Standard"));
    toolbar->setObjectName("MainToolbar");
    toolbar->setIconSize(QSize(32, 32));
    toolbar->addAction(newAct);
    toolbar->addAction(openAct);
    toolbar->addAction(saveAct);
    toolbar->addSeparator();
    toolbar->addAction(runAct);
    toolbar->addAction(analyzeAct);
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

    projectTree = new QTreeWidget(dwProjectTree);
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
    viewMenu->addAction(dwProjectTree->toggleViewAction());

    // Messages LogWidget
    dwMessages = new QDockWidget(tr("Messages"), this);
    dwMessages->setObjectName("Messages");
    dwMessages->setAllowedAreas(Qt::BottomDockWidgetArea);
    lwMessages = new LogWidget(dwMessages);
    dwMessages->setWidget(lwMessages);
    addDockWidget(Qt::BottomDockWidgetArea, dwMessages);
    viewMenu->addAction(dwMessages->toggleViewAction());

    // Model Output LogWidget
    dwOutput = new QDockWidget(tr("Model Output"), this);
    dwMessages->setObjectName("Model Output");
    dwMessages->setAllowedAreas(Qt::BottomDockWidgetArea);
    lwOutput = new LogWidget(dwOutput);
    dwOutput->setWidget(lwOutput);
    addDockWidget(Qt::BottomDockWidgetArea, dwOutput);
    viewMenu->addAction(dwOutput->toggleViewAction());

    // Tabify LogWidgets
    tabifyDockWidget(dwMessages, dwOutput);
    dwMessages->raise();
}

void MainWindow::setupConnections()
{
    connect(centralTabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::deleteTab);

    connect(this, &MainWindow::scenarioUpdated, [=](Scenario *s)
    {
        projectModified = true;
        if (scenarioToInputViewer.count(s) > 0) {
            InputViewer *viewer = scenarioToInputViewer[s];
            int index = centralTabWidget->indexOf(viewer);
            if (index >= 0) {
                viewer->refresh();
                QString title = QString::fromStdString(s->title);
                centralTabWidget->setTabText(index, title);
            }
        }
    });

    connect(this, &MainWindow::sourceGroupUpdated, [=](SourceGroup *sg)
    {
        projectModified = true;
        if (sourceGroupToSourceTable.count(sg) > 0) {
            SourceTable *table = sourceGroupToSourceTable[sg];
            int index = centralTabWidget->indexOf(table);
            if (index >= 0) {
                table->refresh();
                QString title = QString::fromStdString(sg->grpid);
                centralTabWidget->setTabText(index, title);
            }
        }
    });

    // File Menu
    connect(newAct, &QAction::triggered, this, &MainWindow::newScenario);
    connect(openAct, &QAction::triggered, this, &MainWindow::openProject);
    connect(saveAct,  &QAction::triggered, this, &MainWindow::saveProject);
    connect(saveAsAct,  &QAction::triggered, this, &MainWindow::saveProject);
    connect(closeAct, &QAction::triggered, this, &MainWindow::closeProject);
    connect(exitAct, &QAction::triggered, this, &MainWindow::exitApplication);

    // Model Menu
    connect(runAct, &QAction::triggered, this, &MainWindow::runModel);
    connect(analyzeAct, &QAction::triggered, this, &MainWindow::analyzeOutput);

    // Help Menu
    //connect(helpAct, &QAction::triggered, this, &MainWindow::viewHelp);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    // Project Tree
    connect(projectTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::contextMenuRequested);
    connect(projectTree, &QTreeWidget::itemChanged, this, &MainWindow::handleItemChanged);
}

void MainWindow::setupLogging()
{
    // Configure columns and filter menu for each log widget.
    QFontMetrics fm = fontMetrics();
    int maxCharWidth = fm.horizontalAdvance(QChar('W'));
    int avgCharWidth = fm.averageCharWidth();

    lwMessages->setColumn(0, "Message", "", 0);
    lwMessages->setColumn(1, "Source", "Source", avgCharWidth * 50);
    lwMessages->setFilterKeyColumn(1);
    lwMessages->setFilterValues(QStringList{"Distribution", "Geometry", "Import", "Model", "Analysis"});

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
    typedef boost::log::sinks::synchronous_sink<LogWidgetBackend> sink_t;

    auto messageBackend = boost::make_shared<LogWidgetBackend>(lwMessages);
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
    QDesktopWidget desktop;
    QSize defaultSize = desktop.availableGeometry(desktop.primaryScreen()).size() * 0.75;

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

void MainWindow::openProject()
{
    QSettings settings;
    QString openFile;
    QString examplesDir = qApp->applicationDirPath() + QDir::separator() + "examples";
    QString currentDir = settings.value("DefaultDirectory", examplesDir).toString();
    openFile = QFileDialog::getOpenFileName(this,
                                            tr("Open Project"),
                                            currentDir,
                                            tr("SOFEA Project (*.sofea)"));

    if (openFile.isEmpty())
        return;

    // Close current project.
    closeProject();

    // Read the scenario data.
    std::string ifile = openFile.toStdString();
    std::ifstream is(ifile);
    cereal::JSONInputArchive ia(is);

    try {
        ia(scenarios);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Parse Error", QString::fromLocal8Bit(e.what()));
        return;
    }

    // Update containers and tree.
    for (Scenario &s : scenarios) {
        addScenarioToTree(&s);
        for (SourceGroup &sg : s.sourceGroups) {
            sourceGroupToScenario[&sg] = &s;
            addSourceGroupToTree(&sg, &s);
        }
    }

    // Update project file and working directory.
    projectFile = openFile;
    QFileInfo fi(openFile);
    projectDir = fi.canonicalPath();
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
        QString currentDir = settings.value("DefaultDirectory", QDir::rootPath()).toString();
        saveFile = QFileDialog::getSaveFileName(this,
                                                tr("Save Project"),
                                                currentDir,
                                                tr("SOFEA Project (*.sofea)"));

        if (saveFile.isEmpty())
            return;

        // Update project file and working directory.
        projectFile = saveFile;
        QFileInfo fi(saveFile);
        projectDir = fi.canonicalPath();
        settings.setValue("DefaultDirectory", projectDir);
    }

    // Write the scenario data.
    std::string ofile = projectFile.toStdString();
    std::ofstream os(ofile);
    cereal::JSONOutputArchive oa(os);
    oa(scenarios);

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

    // Clear project data.
    projectFile.clear();
    projectDir.clear();

    // Clear messages.
    lwMessages->clear();

    // Remove all scenarios.
    for (auto it = scenarios.rbegin(); it != scenarios.rend(); ++it)
        removeScenario(&(*it));

    projectModified = false;
}

void MainWindow::exitApplication()
{
    QApplication::closeAllWindows();
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
}

void MainWindow::newScenario()
{
    Scenario *s = new Scenario();
    scenarios.push_back(s);
    addScenarioToTree(s);
}

void MainWindow::newSourceGroup(Scenario *s)
{
    SourceGroup *sg = new SourceGroup;
    sourceGroupToScenario[sg] = s;
    s->sourceGroups.push_back(sg);
    addSourceGroupToTree(sg, s);
}

void MainWindow::importValidationData(Scenario *s)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");

    QSettings settings;
    QString csvfile;
    QString examplesDir = qApp->applicationDirPath() + QDir::separator() + "examples";
    QString currentDir = settings.value("DefaultDirectory", examplesDir).toString();
    csvfile = QFileDialog::getOpenFileName(this,
                                           tr("Import Retrospective Data"),
                                           currentDir,
                                           tr("Retrospective Data (*.csv)"));

    if (csvfile.isEmpty())
        return;

    // Create a new SourceGroup with area sources
    SourceGroup *sg = new SourceGroup;
    sourceGroupToScenario[sg] = s;
    s->sourceGroups.push_back(sg);

    // Set validation mode by default
    sg->validationMode = true;

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
            sg->sources.push_back(as);
        }
    }
    catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        return;
    }

    addSourceGroupToTree(sg, s);
}

void MainWindow::addScenarioToTree(Scenario *s)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;

    scenarioToTreeWidget[s] = item;
    treeWidgetToScenario[item] = s;

    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
    item->setText(0, QString::fromStdString(s->title));

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
        removeSourceGroup(&(*it));

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
        if (&(*it) == sg)
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
    for (SourceGroup &sg : clone->sourceGroups) {
        sourceGroupToScenario[&sg] = clone;
        addSourceGroupToTree(&sg, clone);
    }
}

void MainWindow::cloneSourceGroup(SourceGroup *sg)
{
    SourceGroup *clone = new SourceGroup(*sg); // copy constructor
    Scenario *s = sourceGroupToScenario.find(sg)->second;
    sourceGroupToScenario[clone] = s;
    s->sourceGroups.push_back(clone);
    addSourceGroupToTree(clone, s);
}

void MainWindow::showScenarioProperties(Scenario *s)
{
    ScenarioProperties *dialog = new ScenarioProperties(s, this);
    connect(dialog, &ScenarioProperties::saved, [&]() {
        emit scenarioUpdated(s); // input file update
    });
    dialog->exec();
}

void MainWindow::showSourceGroupProperties(SourceGroup *sg)
{
    Scenario *s = sourceGroupToScenario[sg];
    SourceGroupProperties *dialog = new SourceGroupProperties(s, sg, this);
    connect(dialog, &SourceGroupProperties::saved, [&]() {
        emit scenarioUpdated(s); // input file update
        emit sourceGroupUpdated(sg); // source table update
    });
    dialog->exec();
}

void MainWindow::showReceptorEditor(SourceGroup *sg)
{
    Scenario *s = sourceGroupToScenario[sg];
    ReceptorDialog *dialog = new ReceptorDialog(s, sg, this);
    int rc = dialog->exec();
    if (rc == QDialog::Accepted) {
        emit scenarioUpdated(s); // input file update
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

        QString title = QString::fromStdString(s->title);
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
        SourceTable *table = new SourceTable(sg);

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
    QString currentDir = settings.value("DefaultDirectory", QDir::rootPath()).toString();
    saveFile = QFileDialog::getSaveFileName(this,
                                            tr("Export Flux File"),
                                            currentDir,
                                            tr("Flux File (*.dat)"));

    if (!saveFile.isEmpty()) {
        s->writeFluxFile(saveFile.toStdString());
    }
}

void MainWindow::contextMenuRequested(QPoint const& pos)
{
    QPoint globalPos = projectTree->viewport()->mapToGlobal(pos);
    QMenu contextMenu;

    QTreeWidgetItem *item = projectTree->itemAt(pos);
    QAction *selectedItem;

    // Scenario Context Menu
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
        contextMenu.addAction(editReceptorsAct);
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
        else if (selectedItem && selectedItem == editReceptorsAct) {
            showReceptorEditor(sg);
        }
        else if (selectedItem && selectedItem == sourceTableAct) {
            showSourceTable(sg);
        }
        return;
    }
}

void MainWindow::handleItemChanged(QTreeWidgetItem *item, int)
{
    // Handle rename, making sure name is valid.
    QString text = item->text(0); // new text

    // Remove any non-alphanumeric characters and truncate to max length (100)
    text.remove(QRegExp("[^A-Za-z0-9_]+"));
    text.truncate(100);
    item->setText(0, text);

    if (treeWidgetToScenario.count(item) > 0) {
        Scenario *s = treeWidgetToScenario[item];
        s->title = text.toStdString();
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

void MainWindow::runModel()
{
    if (projectDir.isEmpty() || !QDir(projectDir).exists())
    {
        QSettings settings;
        QString currentDir = settings.value("DefaultDirectory", QDir::rootPath()).toString();
        QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Select Working Directory"),
                                                                currentDir,
                                                                QFileDialog::ShowDirsOnly);

        if (selectedDir.isEmpty())
            return;

        projectDir = selectedDir;
        settings.setValue("DefaultDirectory", projectDir);
    }

    RunModelDialog dialog(this);
    dialog.setWorkingDirectory(projectDir);
    for (Scenario &s : scenarios) {
        dialog.addScenario(&s);
    }

    dwOutput->raise();
    dialog.exec();
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

void MainWindow::about()
{
    QString aboutText;
    aboutText += "<h3>Corteva Agriscience<br />Soil Fumigant Exposure Assessment (SOFEA) Modeling System</h3>";
    aboutText += QString("<h4>Version %1 <i>BETA</i>").arg(SOFEA_VERSION_STRING);
#if _WIN64
    aboutText += " 64-bit</h4>";
#elif _WIN32
    aboutText += " 32-bit</h4>";
#else
    aboutText += "</h4>";
#endif
    aboutText += "<p>&copy; 2004-2018 DowDuPont Inc. All rights reserved.</p>";

    // Compiler Info
    aboutText += QString("Built with MSVC %1").arg(_MSC_VER) +
                 QString(" and Qt %1").arg(QT_VERSION_STR) +
                 QString(" on %1").arg(__DATE__);

    // Dispersion Model Info
    aboutText += "<h4>Model Version Information:</h4>";
    aboutText += QString("AERMOD %1").arg(AERMOD_VERSION_STRING);

    // Contributor Info
    aboutText += "<h4>Developers:</h4>";
    aboutText += "Steven A. Cryer (sacryer@dow.com)<br>";
    aboutText += "Ian van Wesenbeeck (ijvanwesenbeeck@dow.com)<br>";
    aboutText += "John Buonagurio (jbuonagurio@exponent.com)";

    QMessageBox::about(this, tr("About SOFEA"), aboutText);
}

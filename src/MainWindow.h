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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AnalysisWindow.h"
#include "InputViewer.h"
#include "LogWidget.h"
#include "ProjectModel.h"
#include "ProjectTreeView.h"
#include "ScenarioProperties.h"
#include "SourceGroupProperties.h"
#include "SourceTable.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <map>
#include <memory>

QT_BEGIN_NAMESPACE
class QAction;
class QDir;
class QDockWidget;
class QMenu;
class QTabWidget;
class QTextEdit;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
#ifdef Q_OS_WIN
class QWinTaskbarButton;
class QWinTaskbarProgress;
#endif
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

signals:
    void scenarioUpdated(Scenario *);
    void sourceGroupUpdated(SourceGroup *);
    void selectAllClicked();
    void selectNoneClicked();
    void selectInverseClicked();

private slots:
    //void onCommandActivated(int commandId);
    //void onCommandToggled(int commandId, bool checked);
    //void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onItemChanged(QTreeWidgetItem *item, int);
    void contextMenuRequested(const QPoint& pos);
    void onScenarioUpdated(Scenario *s);
    void onSourceGroupUpdated(SourceGroup *s);

    void newScenario();
    void openProject();
    void saveProject();
    void closeProject();
    void exitApplication();
    void deleteTab(int index);
    void validate();
    void runModel();
    void analyzeOutput();
    void showHelp();
    void about();

private:
    void createActions();
    void createMenus();
    void createToolbar();
    void createPanels();
    void setupConnections();
    void setupLogging();
    void loadSettings();
    void saveSettings();
    void openProjectFile(const QString& openFile);
    void saveProjectFile(const QString& saveFile);

    void newSourceGroup(Scenario *s);
    void importValidationData(Scenario *s);
    void addScenarioToTree(Scenario *s);
    void addSourceGroupToTree(SourceGroup *sg, Scenario *s);
    void removeScenario(Scenario *s);
    void removeSourceGroup(SourceGroup *sg);
    void cloneScenario(Scenario *s);
    void cloneSourceGroup(SourceGroup *sg);
    void showScenarioProperties(Scenario *s);
    void showSourceGroupProperties(SourceGroup *sg);
    void showReceptorEditor(Scenario *s);
    void showInputViewer(Scenario *s);
    void showSourceTable(SourceGroup *sg);
    void exportFluxFile(Scenario *s);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QTabWidget *centralTabWidget;
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;
    QToolBar *toolbar;
    QDockWidget *dwProjectTree;
    QDockWidget *dwValidation;
    QDockWidget *dwOutput;
    ProjectTreeView *projectTreeView;
    ProjectModel *projectModel;
    QTreeWidget *projectTree;
    LogWidget *lwValidation;
    LogWidget *lwOutput;
    AnalysisWindow *analysisWindow = nullptr;

#ifdef Q_OS_WIN
    QWinTaskbarButton *taskbarButton = nullptr;
    QWinTaskbarProgress *taskbarProgress = nullptr;
#endif

    // Scenario/SourceGroup Containers
    // TODO: implement ProjectModel.

    boost::ptr_vector<Scenario> scenarios;
    std::map<SourceGroup *, Scenario *> sourceGroupToScenario;

    // Scenario-Tree Map
    std::map<Scenario *, QTreeWidgetItem *> scenarioToTreeWidget;
    std::map<QTreeWidgetItem *, Scenario *> treeWidgetToScenario;
    // SourceGroup-Tree Map
    std::map<SourceGroup *, QTreeWidgetItem *> sourceGroupToTreeWidget;
    std::map<QTreeWidgetItem *, SourceGroup *> treeWidgetToSourceGroup;
    // Scenario-InputViewer Map
    std::map<Scenario *, InputViewer *> scenarioToInputViewer;
    std::map<InputViewer *, Scenario *> inputViewerToScenario;
    // SourceGroup-SourceTable Map
    std::map<SourceGroup *, SourceTable *> sourceGroupToSourceTable;
    std::map<SourceTable *, SourceGroup *> sourceTableToSourceGroup;

    // Project Properties
    QString projectFile;
    mutable bool projectModified;

    // Main Actions
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *exitAct;
    QAction *validateAct;
    QAction *runAct;
    QAction *analyzeAct;
    QAction *optionsAct;
    QAction *helpAct;
    QAction *aboutAct;

    // Scenario Actions
    QAction *scenarioCloneAct;
    QAction *scenarioRenameAct;
    QAction *scenarioRemoveAct;
    QAction *scenarioAddSourceGroupAct;
    QAction *scenarioImportValidationAct;
    QAction *scenarioPropertiesAct;
    QAction *scenarioInputFileAct;
    QAction *scenarioFluxFileAct;

    // Source Group Actions
    QAction *sourceGroupCloneAct;
    QAction *sourceGroupRenameAct;
    QAction *sourceGroupRemoveAct;
    QAction *sourceGroupPropertiesAct;
    QAction *editReceptorsAct;
    QAction *sourceTableAct;
};

#endif // MAINWINDOW_H

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

#include "core/Scenario.h"
#include "core/SourceGroup.h"
#include "core/Project.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <map>
#include <memory>

class AnalysisWindow;
class InputViewer;
class LogWidget;
class MeteorologyEditor;
class ProgressBar;
class ProjectModel;
class ProjectTreeView;
class RunModelDialog;
class ScenarioProperties;
class SourceGroupProperties;
class SourceTable;

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
    void showMeteorologyEditor();
    void validate();
    void runModel();
    void showAnalysisWindow();
    void showHelp();
    void about();

private:
    void createMenus();
    void createActions();
    void createToolBar();
    void createProgressBar();
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
    void showElevationEditor(Scenario *s);
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
    ProgressBar *progressBar;
#ifdef Q_OS_WIN
    QWinTaskbarButton *taskbarButton = nullptr;
    QWinTaskbarProgress *taskbarProgress = nullptr;
#endif
    QTreeWidget *projectTree; // TODO: Replace
    ProjectModel *projectModel;
    ProjectTreeView *projectTreeView;
    LogWidget *lwValidation;
    LogWidget *lwOutput;
    RunModelDialog *runModelDialog = nullptr;
    AnalysisWindow *analysisWindow = nullptr;
    MeteorologyEditor *meteorologyEditor = nullptr;

    // Scenario/SourceGroup Containers
    // TODO: implement ProjectModel, use shared_ptr, weak_ptr

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
    QAction *meteorologyAct;
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

    QAction *receptorEditorAct;
    QAction *elevationEditorAct;
    QAction *sourceTableAct;
};

#endif // MAINWINDOW_H

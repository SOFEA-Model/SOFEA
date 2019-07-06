#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AnalysisWindow.h"
#include "InputViewer.h"
#include "LogWidget.h"
#include "ScenarioProperties.h"
#include "SourceGroupProperties.h"
#include "SourceTable.h"
#include "Utilities.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <map>

QT_BEGIN_NAMESPACE
class QAction;
class QDir;
class QMenu;
class QTabWidget;
class QTextEdit;
class QToolBar;
class QTreeWidget;
class QTreeWidgetItem;
class QWinTaskbarButton;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:
    void scenarioUpdated(Scenario *);
    void sourceGroupUpdated(SourceGroup *);

private slots:
    void newScenario();
    void openProject();
    void saveProject();
    void closeProject();
    void exitApplication();
    void runModel();
    void analyzeOutput();
    void about();
    void contextMenuRequested(QPoint const& pos);
    void handleItemChanged(QTreeWidgetItem *item, int);
    void deleteTab(int index);
    void onScenarioUpdated(Scenario *s);
    void onSourceGroupUpdated(SourceGroup *s);

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
    void showReceptorEditor(SourceGroup *sg);
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
    QWinTaskbarButton *taskbarButton;
    QTabWidget *centralTabWidget;
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *modelMenu;
    QMenu *helpMenu;
    QToolBar *toolbar;
    DockWidget *dwProjectTree;
    DockWidget *dwMessages;
    DockWidget *dwOutput;
    QTreeWidget *projectTree;
    LogWidget *lwMessages;
    LogWidget *lwOutput;
    AnalysisWindow *analysisWindow = nullptr;

    // Scenario/SourceGroup Containers
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
    QString projectDir;
    mutable bool projectModified;

    // Main Actions
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *closeAct;
    QAction *exitAct;
    QAction *runAct;
    QAction *analyzeAct;
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

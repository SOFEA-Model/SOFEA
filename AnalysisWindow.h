#ifndef ANALYSISWINDOW_H
#define ANALYSISWINDOW_H

#include <QMainWindow>

#include "Analysis.h"
#include "ListEditor.h"
#include "StandardTableView.h"
#include "UDUnitsLineEdit.h"
#include "Utilities.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QToolBox;
class QToolButton;
QT_END_NAMESPACE

/****************************************************************************
** ReceptorAnalysisTool
****************************************************************************/

class ReceptorAnalysisTool : public QWidget
{
    Q_OBJECT

public:
    ReceptorAnalysisTool(QWidget *parent = nullptr);
    ReceptorAnalysisOpts analysisOpts() const;

signals:
    void calcRequested();

public slots:
    void hideWarning() { lblWarning->setVisible(false); }
    void showWarning() { lblWarning->setVisible(true); }

private:
    StatusLabel *lblWarning;
    QGroupBox *gbSummary;
    QCheckBox *cbMean;
    QCheckBox *cbMax;
    QCheckBox *cbStdDev;
    QGroupBox *gbPercentile;
    ListEditor *pctEditor;
    QGroupBox *gbMovingAverage;
    ListEditor *windowEditor;
    QPushButton *btnCalc;
};

/****************************************************************************
** HistogramAnalysisTool
****************************************************************************/

class HistogramAnalysisTool : public QWidget
{
    Q_OBJECT

public:
    HistogramAnalysisTool(QWidget *parent = nullptr);
    HistogramAnalysisOpts analysisOpts() const;

signals:
    void calcRequested();

private:
    QGroupBox *gbCDF;
    QSpinBox *sbBinsCDF;
    QGroupBox *gbPDF;
    QSpinBox *sbBinsPDF;
    QSpinBox *sbCacheSizePDF;
    QPushButton *btnCalc;
};

/****************************************************************************
** OptionsPanel
****************************************************************************/

class OptionsPanel : public QWidget
{
    Q_OBJECT

public:
    OptionsPanel(QWidget *parent = nullptr);
    GeneralAnalysisOpts analysisOpts() const;
    ReceptorAnalysisOpts receptorAnalysisOpts() const;
    HistogramAnalysisOpts histogramAnalysisOpts() const;
    QString currentFile() const;
    void addType(const std::string& type, const std::string& units);
    void resetType();
    void setAveragingPeriods(std::vector<int>& periods);
    void setSourceGroups(std::vector<std::string>& groups);

signals:
    void receptorAnalysisRequested();
    void histogramAnalysisRequested();
    void currentFileChanged();

public slots:
    void enableTools();
    void disableTools();
    void clearOptions();

private slots:
    void selectFile();
    void exportTimeSeries();

private:
    void setupConnections();

    ReadOnlyLineEdit *leFile;
    QToolButton *btnBrowse;
    QComboBox *cboAvePeriod;
    QComboBox *cboSourceGroup;
    QComboBox *cboType;
    UDUnitsLineEdit *leUnitIn;
    ReadOnlyLineEdit *leUnitInVal;
    UDUnitsLineEdit *leUnitOut;
    ReadOnlyLineEdit *leUnitOutVal;
    ReadOnlyLineEdit *leScaleFactor;
    QPushButton *btnExport;
    QToolBox *toolbox;
    ReceptorAnalysisTool *receptorTool;
    HistogramAnalysisTool *histogramTool;
};

/****************************************************************************
** FileInfoPanel
****************************************************************************/

class FileInfoPanel : public QWidget
{
public:
    FileInfoPanel(QWidget *parent = nullptr);
    bool setFile(const QString& filename);
    void setTitle(const QString& title);
    void setOptions(const QString& options);
    void setVersion(const QString& version);
    void setReceptorCount(const int nrec);
    void setTimeStepCount(const int ntime);
    void clearContents();

private:
    QTreeWidget *infoTree;
    QTreeWidgetItem *itemTitle;
    QTreeWidgetItem *itemOptions;
    QTreeWidgetItem *itemVersion;
    QTreeWidgetItem *itemReceptors;
    QTreeWidgetItem *itemTimeSteps;
};

/****************************************************************************
** AnalysisWindow
****************************************************************************/

class AnalysisWindow : public QMainWindow
{
    Q_OBJECT

public:
    AnalysisWindow(QWidget *parent = nullptr);

private slots:
    void setCurrentFile();
    void calcReceptorStats();
    void calcHistogram();

private:
    void showReceptorStats(const GeneralAnalysisOpts genOpts, const ReceptorAnalysisOpts opts, const ReceptorStats& out);
    void showHistogram(const GeneralAnalysisOpts genOpts, const HistogramAnalysisOpts opts, const Histogram& out);
    void setupConnections();

    QString filename;
    OptionsPanel *optionsPanel;
    FileInfoPanel *fileInfoPanel;
    StandardTableView *outputTable(QWidget *parent = nullptr);

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

protected:
    //bool eventFilter(QObject *obj, QEvent *event);
};

#endif // ANALYSISWINDOW_H

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

#ifndef ANALYSISWINDOW_H
#define ANALYSISWINDOW_H

#include <QMainWindow>

#include "analysis/Analysis.h"
#include "analysis/AnalysisOptions.h"

class ListEditor;
class ReadOnlyLineEdit;
class StandardTableView;
class StatusLabel;
class UDUnitsLineEdit;

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
** ExportTool
****************************************************************************/

class ExportTool : public QWidget
{
    Q_OBJECT

public:
    ExportTool(QWidget *parent = nullptr);
    ncpost::options::tsexport exportOpts() const;

signals:
    void exportRequested();

private slots:
    void onExportClicked();

private:
    QGroupBox *gbMovingAverage;
    ListEditor *windowEditor;
    QPushButton *btnExport;
    QString selectedFile;
};


/****************************************************************************
** ReceptorAnalysisTool
****************************************************************************/

class ReceptorAnalysisTool : public QWidget
{
    Q_OBJECT

public:
    ReceptorAnalysisTool(QWidget *parent = nullptr);
    ncpost::options::statistics analysisOpts() const;

signals:
    void calcRequested();

public slots:
    void hideWarning();
    void showWarning();

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
    ncpost::options::histogram analysisOpts() const;

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
    ncpost::options::general analysisOpts() const;
    ncpost::options::tsexport exportOpts() const;
    ncpost::options::statistics receptorAnalysisOpts() const;
    ncpost::options::histogram histogramAnalysisOpts() const;
    QString currentFile() const;
    void addType(const std::string& type, const std::string& units);
    void resetType();
    void setAveragingPeriods(std::vector<int>& periods);
    void setSourceGroups(std::vector<std::string>& groups);

signals:
    void exportRequested();
    void receptorAnalysisRequested();
    void histogramAnalysisRequested();
    void currentFileChanged();

public slots:
    void enableTools();
    void disableTools();
    void clearOptions();

private slots:
    void selectFile();

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

    QToolBox *toolbox;
    ExportTool *exportTool;
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
    void exportTimeSeries();
    void calcReceptorStats();
    void calcHistogram();

private:
    void showReceptorStats(const ncpost::options::general& opts,
        const ncpost::options::statistics& statopts, const ncpost::statistics_type& out,
        const std::vector<ncpost::receptor_t>& recs);
    void showHistogram(const ncpost::options::general& opts,
        const ncpost::options::histogram& histopts, const ncpost::histogram_type& out);
    void setupConnections();

    QString filename;
    OptionsPanel *optionsPanel;
    FileInfoPanel *fileInfoPanel;
    StandardTableView *outputTable(QWidget *parent = nullptr);

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
};

#endif // ANALYSISWINDOW_H

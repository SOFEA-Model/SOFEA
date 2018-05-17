#ifndef ANALYSISWINDOW_H
#define ANALYSISWINDOW_H

#include <QMainWindow>

#include "Analysis.h"
#include "ListEditor.h"
#include "StandardTableView.h"
#include "Utilities.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QToolBox;
QT_END_NAMESPACE

class AnalysisWindow : public QMainWindow
{
    Q_OBJECT

public:
    AnalysisWindow(const QString& file, QWidget *parent = nullptr);

private slots:
    void calcReceptorStats();
    void calcHistogram();

private:
    void showReceptorStats(const AnalysisOptions opts, const ReceptorStats& out);
    void showHistogram(const AnalysisOptions opts, const Histogram& out);
    void setupConnections();
    AnalysisOptions getOptions() const;
    bool setFileInformation();

    // Widget Generators
    QWidget* infoControl(QWidget *parent = nullptr);
    QWidget* receptorsControl(QWidget *parent = nullptr);
    QWidget* histogramControl(QWidget *parent = nullptr);
    StandardTableView* outputTable(QWidget *parent = nullptr);

    std::string filename;
    int maxThreads = 0;

    // File Information
    ReadOnlyLineEdit *lePath;
    ReadOnlyLineEdit *leVersion;
    ReadOnlyLineEdit *leReceptors;
    ReadOnlyLineEdit *leTimeSteps;

    // Receptors
    QGroupBox *gbSummary;
    QCheckBox *cbMean;
    QCheckBox *cbMax;
    QCheckBox *cbStdDev;
    QGroupBox *gbPercentile;
    ListEditor *pctEditor;
    QGroupBox *gbMovingAverage;
    ListEditor *windowEditor;
    QPushButton *btnCalcReceptors;

    // Histogram
    QGroupBox *gbCDF;
    QSpinBox *sbBinsCDF;
    QGroupBox *gbPDF;
    QSpinBox *sbBinsPDF;
    QSpinBox *sbCacheSizePDF;
    QPushButton *btnCalcHistogram;

    QToolBox *toolBox;
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

protected:
    //bool eventFilter(QObject *obj, QEvent *event);
};

#endif // ANALYSISWINDOW_H

#include <QtWidgets>

#include "AnalysisWindow.h"

AnalysisWindow::AnalysisWindow(const QString &file, QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Analysis");
    setWindowIcon(QIcon(":/images/RunPerformance_32x.png"));
    setAttribute(Qt::WA_DeleteOnClose, false);

    maxThreads = QThread::idealThreadCount();
    filename = file.toStdString();

    QDockWidget *dock;

    // File Information
    dock = new QDockWidget(tr("File Information"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);

    QWidget *infoWidget = infoControl(dock);
    dock->setWidget(infoWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dock);


    // Options
    dock = new QDockWidget(tr("Options"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);

    toolBox = new QToolBox(dock);
    toolBox->setContentsMargins(10, 10, 10, 10);
    toolBox->setMinimumSize(400, 500);
    toolBox->addItem(receptorsControl(), "Receptors");
    toolBox->addItem(histogramControl(), "Overall Distribution");

    dock->setWidget(toolBox);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    // Central
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMinimumSize(800, 500);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(5);
    centralLayout->addWidget(tabWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *central = new QWidget;
    central->setLayout(centralLayout);
    setCentralWidget(central);

    setupConnections();

    bool enabled = setFileInformation();
    btnCalcReceptors->setEnabled(enabled);
    btnCalcHistogram->setEnabled(enabled);
}

void AnalysisWindow::setupConnections()
{
    connect(btnCalcReceptors, &QPushButton::clicked, this, &AnalysisWindow::calcReceptorStats);
    connect(btnCalcHistogram, &QPushButton::clicked, this, &AnalysisWindow::calcHistogram);
    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int i) {
        QWidget *widget = tabWidget->widget(i);
        if (widget != nullptr)
            widget->deleteLater();
    });
}

AnalysisOptions AnalysisWindow::getOptions() const
{
    AnalysisOptions opts;

    opts.calcRecMean = cbMean->isChecked();
    opts.calcRecMax = cbMax->isChecked();
    opts.calcRecStdDev = cbStdDev->isChecked();
    opts.calcRecP2 = gbPercentile->isChecked();
    opts.recPercentiles = pctEditor->values();
    opts.calcRecMaxRM = gbMovingAverage->isChecked();
    opts.recWindowSizes = windowEditor->values();
    opts.calcCDF = gbCDF->isChecked();
    opts.calcPDF = gbPDF->isChecked();
    opts.cdfBins = sbBinsCDF->value();
    opts.pdfBins = sbBinsPDF->value();
    opts.pdfCacheSize = sbCacheSizePDF->value();

    return opts;
}

void AnalysisWindow::calcReceptorStats()
{
    Analysis analysis(filename);
    AnalysisOptions opts = getOptions();
    ReceptorStats out;
    int ncid = 0;

    if (!analysis.openFile(ncid)) {
        QString errmsg = QString::fromStdString(analysis.getLastError());
        QMessageBox::critical(this, "File Error", errmsg);
        analysis.closeFile(ncid);
        return;
    }

    if (!analysis.calcReceptorStats(ncid, opts, out))
        return;

    showReceptorStats(opts, out);
}

void AnalysisWindow::calcHistogram()
{
    Analysis analysis(filename);
    AnalysisOptions opts = getOptions();
    Histogram out;
    int ncid = 0;

    if (!analysis.openFile(ncid)) {
        QString errmsg = QString::fromStdString(analysis.getLastError());
        QMessageBox::critical(this, "File Error", errmsg);
        analysis.closeFile(ncid);
        return;
    }

    if (!analysis.calcHistogram(ncid, opts, out))
        return;

    showHistogram(opts, out);
}

void AnalysisWindow::showReceptorStats(const AnalysisOptions opts, const ReceptorStats &out)
{
    QStringList headers;
    headers << "Receptor" << "X" << "Y" << "Z";
    if (opts.calcRecMean) headers << "Mean";
    if (opts.calcRecMax)  headers << "Max";
    if (opts.calcRecStdDev) headers << "Std. Dev.";
    if (opts.calcRecP2) {
        for (double p : opts.recPercentiles)
            headers << QString("P%1").arg(p * 100);
    }
    if (opts.calcRecMaxRM) {
        for (double w : opts.recWindowSizes)
            headers << QString("Max[MA%1]").arg(w);
    }

    StandardTableView *table = outputTable();
    int nrows = static_cast<int>(out.id.size());
    QStandardItemModel *model = new QStandardItemModel(table);
    table->setModel(model);
    model->setRowCount(nrows);
    model->setColumnCount(headers.size());
    model->setHorizontalHeaderLabels(headers);

    int col = 0;
    table->setColumnData(col++, out.id);
    table->setColumnData(col++, out.x);
    table->setColumnData(col++, out.y);
    table->setColumnData(col++, out.z);
    if (opts.calcRecMean) table->setColumnData(col++, out.avg);
    if (opts.calcRecMax) table->setColumnData(col++, out.max);
    if (opts.calcRecStdDev) table->setColumnData(col++, out.std);
    if (opts.calcRecP2) {
        for (std::vector<double> v : out.p2)
            table->setColumnData(col++, v);
    }
    if (opts.calcRecMaxRM) {
        for (std::vector<double> v : out.rm)
            table->setColumnData(col++, v);
    }

    tabWidget->addTab(table, "Receptors");
    tabWidget->setCurrentIndex(tabWidget->count() - 1);
}

void AnalysisWindow::showHistogram(const AnalysisOptions opts, const Histogram& out)
{
    if (opts.calcCDF)
    {
        StandardTableView *table = outputTable();
        int nrows = static_cast<int>(out.cdf.size());
        QStandardItemModel *model = new QStandardItemModel(table);
        table->setModel(model);
        model->setRowCount(nrows);
        model->setColumnCount(2);
        model->setHorizontalHeaderLabels(QStringList{"x", "F(x)"});

        for (int i = 0; i < nrows; ++i) {
            model->setData(model->index(i, 0), out.cdf[i].first, Qt::DisplayRole);
            model->setData(model->index(i, 1), out.cdf[i].second, Qt::DisplayRole);
        }

        tabWidget->addTab(table, "CDF");
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }

    if (opts.calcPDF)
    {
        StandardTableView *table = outputTable();
        int nrows = static_cast<int>(out.pdf.size());
        QStandardItemModel *model = new QStandardItemModel(table);
        table->setModel(model);
        model->setRowCount(nrows);
        model->setColumnCount(2);
        model->setHorizontalHeaderLabels(QStringList{"x", "f(x)"});

        for (int i = 0; i < nrows; ++i) {
            model->setData(model->index(i, 0), out.pdf[i].first, Qt::DisplayRole);
            model->setData(model->index(i, 1), out.pdf[i].second, Qt::DisplayRole);
        }

        tabWidget->addTab(table, "PDF");
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }
}

bool AnalysisWindow::setFileInformation()
{
    Analysis analysis(filename);
    int ncid = 0;
    bool rc;

    if (analysis.openFile(ncid)) {
        lePath->setText(QString::fromStdString(filename));
        long long nr = analysis.getReceptorCount();
        long long nt = analysis.getTimeStepCount();
        leReceptors->setText(QString::number(nr));
        leTimeSteps->setText(QString::number(nt));
        leVersion->setText(QString::fromStdString(analysis.libraryVersion()));
        rc = true;
    }
    else {
        QString errmsg = QString::fromStdString(analysis.getLastError());
        QMessageBox::critical(this, "File Error", errmsg);
        rc = false;
    }

    analysis.closeFile(ncid);
    return rc;
}

StandardTableView* AnalysisWindow::outputTable(QWidget *parent)
{
    StandardTableView *table = new StandardTableView;

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setFrameStyle(QFrame::NoFrame);

    return table;
}

QWidget* AnalysisWindow::receptorsControl(QWidget *parent)
{
    QWidget *control = new QWidget(parent);

    cbMean = new QCheckBox("Mean");
    cbMean->setChecked(true);
    cbMax = new QCheckBox("Maximum");
    cbMax->setChecked(true);
    cbStdDev = new QCheckBox("Std. Dev.");
    cbStdDev->setChecked(true);

    pctEditor = new ListEditor;
    pctEditor->setValidator(0.001, 0.999, 3);
    std::vector<double> pctDefault{0.5, 0.95, 0.99};
    pctEditor->setValues(pctDefault);
    pctEditor->resetLayout();
    pctEditor->setComboBoxItems(QStringList{"0.5","0.75","0.9","0.95","0.99","0.999"});

    windowEditor = new ListEditor;
    windowEditor->setValidator(1, 100, 0);
    std::vector<double> windowDefault{1, 28, 90};
    windowEditor->setValues(windowDefault);
    windowEditor->setComboBoxItems(QStringList{"1","28","90"});
    windowEditor->resetLayout();

    btnCalcReceptors = new QPushButton("Calculate");

    // Layout
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(cbMean);
    layout1->addWidget(cbMax);
    layout1->addWidget(cbStdDev);
    layout1->addStretch(1);
    gbSummary = new QGroupBox("Summary statistics");
    gbSummary->setFlat(true);
    gbSummary->setLayout(layout1);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addWidget(pctEditor);
    gbPercentile = new QGroupBox(QLatin1String("Percentile estimates (P\xb2 method)"));
    gbPercentile->setCheckable(true);
    gbPercentile->setChecked(false);
    gbPercentile->setFlat(true);
    gbPercentile->setLayout(layout2);

    QVBoxLayout *layout3 = new QVBoxLayout;
    layout3->addWidget(new QLabel("Window size (days):"));
    layout3->addWidget(windowEditor);
    gbMovingAverage = new QGroupBox(QLatin1String("Maximum moving average"));
    gbMovingAverage->setCheckable(true);
    gbMovingAverage->setChecked(false);
    gbMovingAverage->setFlat(true);
    gbMovingAverage->setLayout(layout3);

    QVBoxLayout *layoutControl = new QVBoxLayout;
    layoutControl->setContentsMargins(0, 0, 0, 0);
    layoutControl->addSpacing(5);
    layoutControl->addWidget(gbSummary);
    layoutControl->addSpacing(5);
    layoutControl->addWidget(gbPercentile);
    layoutControl->addSpacing(5);
    layoutControl->addWidget(gbMovingAverage);
    QHBoxLayout *layoutCalc = new QHBoxLayout;
    layoutCalc->addStretch(1);
    layoutCalc->addWidget(btnCalcReceptors);
    layoutControl->addLayout(layoutCalc);
    layoutControl->addStretch(1);
    control->setLayout(layoutControl);

    return control;
}

QWidget* AnalysisWindow::infoControl(QWidget *parent)
{
    QWidget *control = new QWidget(parent);

    lePath = new ReadOnlyLineEdit;
    leReceptors = new ReadOnlyLineEdit;
    leTimeSteps = new ReadOnlyLineEdit;
    leVersion = new ReadOnlyLineEdit;

    QFormLayout *mainLayout = new QFormLayout;
    mainLayout->addRow("File Path: ", lePath);
    mainLayout->addRow("Receptors: ", leReceptors);
    mainLayout->addRow("Time Steps: ", leTimeSteps);
    mainLayout->addRow("Library Version: ", leVersion);
    control->setLayout(mainLayout);

    return control;
}

QWidget* AnalysisWindow::histogramControl(QWidget *parent)
{
    QWidget *control = new QWidget(parent);

    sbBinsCDF = new QSpinBox;
    sbBinsCDF->setRange(10, 1000);
    sbBinsCDF->setValue(100);

    sbBinsPDF = new QSpinBox;
    sbBinsPDF->setRange(10, 1000);
    sbBinsPDF->setValue(100);

    sbCacheSizePDF = new QSpinBox;
    sbCacheSizePDF->setRange(100, 100000);
    sbCacheSizePDF->setValue(10000);

    btnCalcHistogram = new QPushButton("Calculate");

    // Layout
    QFormLayout *layout1 = new QFormLayout;
    layout1->addRow("Number of bins: ", sbBinsCDF);
    gbCDF = new QGroupBox(QLatin1String("CDF estimate (P\xb2 method)"));
    gbCDF->setFlat(true);
    gbCDF->setCheckable(true);
    gbCDF->setChecked(false);
    gbCDF->setLayout(layout1);

    QFormLayout *layout2 = new QFormLayout;
    layout2->addRow("Number of bins: ", sbBinsPDF);
    layout2->addRow("Cache size: ", sbCacheSizePDF);
    gbPDF = new QGroupBox("PDF estimate");
    gbPDF->setFlat(true);
    gbPDF->setCheckable(true);
    gbPDF->setChecked(false);
    gbPDF->setLayout(layout2);

    QVBoxLayout *layoutControl = new QVBoxLayout;
    layoutControl->setContentsMargins(0, 0, 0, 0);
    layoutControl->addSpacing(5);
    layoutControl->addWidget(gbCDF);
    layoutControl->addSpacing(5);
    layoutControl->addWidget(gbPDF);
    QHBoxLayout *layoutCalc = new QHBoxLayout;
    layoutCalc->addStretch(1);
    layoutCalc->addWidget(btnCalcHistogram);
    layoutControl->addLayout(layoutCalc);
    layoutControl->addStretch(1);
    control->setLayout(layoutControl);

    return control;
}

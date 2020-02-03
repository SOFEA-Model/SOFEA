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

#include <QApplication>
#include <QtWidgets>

#include "AppStyle.h"
#include "AnalysisWindow.h"

/****************************************************************************
** ExportTool
****************************************************************************/

ExportTool::ExportTool(QWidget *parent)
    : QWidget(parent)
{
    windowEditor = new ListEditor;
    windowEditor->setValidator(1, 180, 0);
    std::vector<double> windowDefault{1, 28, 90};
    windowEditor->setValues(windowDefault);
    windowEditor->setComboBoxItems(QStringList{"1","28","90"});

    btnExport = new QPushButton("Export...");

    // Layout
    QVBoxLayout *layout1 = new QVBoxLayout;
    layout1->addWidget(new QLabel("Window size (days):"));
    layout1->addWidget(windowEditor);
    gbMovingAverage = new QGroupBox(QLatin1String("Enable moving average"));
    gbMovingAverage->setCheckable(true);
    gbMovingAverage->setChecked(false);
    gbMovingAverage->setFlat(true);
    gbMovingAverage->setLayout(layout1);

    QHBoxLayout *layoutCalc = new QHBoxLayout;
    layoutCalc->addStretch(1);
    layoutCalc->addWidget(btnExport);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(gbMovingAverage);
    mainLayout->addLayout(layoutCalc);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    // Connections
    connect(btnExport, &QPushButton::clicked,
            this, &ExportTool::onExportClicked);
}

void ExportTool::onExportClicked()
{
    QSettings settings;
    QString settingsKey = "DefaultPostFileDirectory";

    QString defaultDirectory = settings.value(settingsKey, qApp->applicationDirPath()).toString();

    const QString csvfile = QFileDialog::getSaveFileName(
        this, tr("Export Time Series"), defaultDirectory, tr("CSV File (*.csv)"));

    if (csvfile.isEmpty())
        return;

    QFileInfo fi(csvfile);
    QString dir = fi.absoluteDir().absolutePath();
    settings.setValue(settingsKey, dir);

    selectedFile = csvfile;

    emit exportRequested();
}

ncpost::options::tsexport ExportTool::exportOpts() const
{
    ncpost::options::tsexport opts;
    opts.output_file = selectedFile.toStdString();
    if (gbMovingAverage->isChecked()) {
        for (const auto& w : windowEditor->values()) {
            opts.rm_windows.push_back(static_cast<int>(w));
        }
    }
    return opts;
}

/****************************************************************************
** ReceptorAnalysisTool
****************************************************************************/

ReceptorAnalysisTool::ReceptorAnalysisTool(QWidget *parent)
    : QWidget(parent)
{
    lblWarning = new StatusLabel;
    lblWarning->setStatusType(StatusLabel::Alert);
    lblWarning->setText("Summary statistics calculated on the 1-hour time series will include zero concentrations for calm and missing hours.");

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
    pctEditor->setComboBoxItems(QStringList{"0.5","0.75","0.9","0.95","0.99","0.999"});

    windowEditor = new ListEditor;
    windowEditor->setValidator(1, 180, 0);
    std::vector<double> windowDefault{1, 28, 90};
    windowEditor->setValues(windowDefault);
    windowEditor->setComboBoxItems(QStringList{"1","28","90"});

    btnCalc = new QPushButton("Calculate");

    // Layout
    QHBoxLayout *cbLayout = new QHBoxLayout;
    cbLayout->addWidget(cbMean);
    cbLayout->addWidget(cbMax);
    cbLayout->addWidget(cbStdDev);
    cbLayout->addStretch(1);
    QVBoxLayout *layout1 = new QVBoxLayout;
    layout1->addWidget(lblWarning);
    layout1->addLayout(cbLayout);

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

    QHBoxLayout *layoutCalc = new QHBoxLayout;
    layoutCalc->addStretch(1);
    layoutCalc->addWidget(btnCalc);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(gbSummary);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(gbPercentile);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(gbMovingAverage);
    mainLayout->addLayout(layoutCalc);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    // Connections
    connect(btnCalc, &QPushButton::clicked,
            this, &ReceptorAnalysisTool::calcRequested);

    hideWarning();
}

ncpost::options::statistics ReceptorAnalysisTool::analysisOpts() const
{
    ncpost::options::statistics opts;
    opts.calc_avg = cbMean->isChecked();
    opts.calc_max = cbMax->isChecked();
    opts.calc_std = cbStdDev->isChecked();
    if (gbPercentile->isChecked()) {
        opts.percentiles = pctEditor->values();
    }
    if (gbMovingAverage->isChecked()) {
        for (const auto& w : windowEditor->values()) {
            opts.maxrm_windows.push_back(static_cast<int>(w));
        }
    }
    return opts;
}

/****************************************************************************
** HistogramAnalysisTool
****************************************************************************/

HistogramAnalysisTool::HistogramAnalysisTool(QWidget *parent)
    : QWidget(parent)
{
    sbBinsCDF = new QSpinBox;
    sbBinsCDF->setRange(5, 1000);
    sbBinsCDF->setValue(100);

    sbBinsPDF = new QSpinBox;
    sbBinsPDF->setRange(5, 1000);
    sbBinsPDF->setValue(100);

    sbCacheSizePDF = new QSpinBox;
    sbCacheSizePDF->setRange(5, 10000000);
    sbCacheSizePDF->setValue(1000);

    btnCalc = new QPushButton("Calculate");

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

    QHBoxLayout *layoutCalc = new QHBoxLayout;
    layoutCalc->addStretch(1);
    layoutCalc->addWidget(btnCalc);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(gbCDF);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(gbPDF);
    mainLayout->addLayout(layoutCalc);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    // Connections
    connect(btnCalc, &QPushButton::clicked,
            this, &HistogramAnalysisTool::calcRequested);
}

ncpost::options::histogram HistogramAnalysisTool::analysisOpts() const
{
    ncpost::options::histogram opts;
    opts.calc_cdf = gbCDF->isChecked();
    opts.calc_pdf = gbPDF->isChecked();
    opts.cdf_bins = sbBinsCDF->value();
    opts.pdf_bins = sbBinsPDF->value();
    opts.pdf_cache_size = sbCacheSizePDF->value();
    return opts;
}

/****************************************************************************
** OptionsPanel
****************************************************************************/

OptionsPanel::OptionsPanel(QWidget *parent)
    : QWidget(parent)
{
    leFile = new ReadOnlyLineEdit;
    btnBrowse = new QToolButton;
    btnBrowse->setText("...");

    cboType = new QComboBox;
    cboAvePeriod = new QComboBox;
    cboSourceGroup = new QComboBox;

    QFont font = QApplication::font();
    cboType->setFont(font);
    cboAvePeriod->setFont(font);
    cboSourceGroup->setFont(font);

    // Unit Controls
    leUnitIn = new UDUnitsLineEdit;
    leUnitIn->setReadOnly(true);
    leUnitIn->setBasePalette();
    leUnitInVal = new ReadOnlyLineEdit;
    leUnitInVal->setBasePalette();

    leUnitOut = new UDUnitsLineEdit;
    leUnitOutVal = new ReadOnlyLineEdit;
    leUnitOutVal->setBasePalette();

    leScaleFactor = new ReadOnlyLineEdit;
    leScaleFactor->setBasePalette();
    leScaleFactor->setText("1");

    const QIcon syncIcon = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_SyncArrow));
    int iconSize = this->style()->pixelMetric(QStyle::PM_SmallIconSize);
    const QPixmap syncPixmap = syncIcon.pixmap(QSize{iconSize, iconSize});
    QLabel *syncLabel = new QLabel;
    syncLabel->setPixmap(syncPixmap);

    // Toolbox
    exportTool = new ExportTool;
    receptorTool = new ReceptorAnalysisTool;
    histogramTool = new HistogramAnalysisTool;

    toolbox = new QToolBox;
    toolbox->setContentsMargins(10, 0, 10, 10);
    toolbox->setMinimumSize(400, 500);
    toolbox->addItem(exportTool, "Time Series Export");
    toolbox->addItem(receptorTool, "Receptors");
    toolbox->addItem(histogramTool, "Overall Distribution");

    // Unit Layouts
    QHBoxLayout *unitLayout1 = new QHBoxLayout;
    unitLayout1->addWidget(leUnitIn);
    unitLayout1->addWidget(syncLabel);
    unitLayout1->addWidget(leUnitInVal);

    QHBoxLayout *unitLayout2 = new QHBoxLayout;
    unitLayout2->addWidget(leUnitOut);
    unitLayout2->addWidget(syncLabel);
    unitLayout2->addWidget(leUnitOutVal);

    // Grid Layout
    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->setContentsMargins(10, 10, 10, 10);
    controlsLayout->setRowMinimumHeight(4, 10);
    controlsLayout->setRowMinimumHeight(8, 5);
    controlsLayout->addWidget(new QLabel("Output file: "), 0, 0);
    controlsLayout->addWidget(leFile, 0, 1);
    controlsLayout->addWidget(btnBrowse, 0, 2);
    controlsLayout->addWidget(new QLabel("Output type: "), 1, 0);
    controlsLayout->addWidget(cboType, 1, 1, 1, 2);
    controlsLayout->addWidget(new QLabel("Averaging period: "), 2, 0);
    controlsLayout->addWidget(cboAvePeriod, 2, 1, 1, 2);
    controlsLayout->addWidget(new QLabel("Source group: "), 3, 0);
    controlsLayout->addWidget(cboSourceGroup, 3, 1, 1, 2);
    controlsLayout->addWidget(new QLabel("Output units: "), 5, 0);
    controlsLayout->addLayout(unitLayout1, 5, 1, 1, 2);
    controlsLayout->addWidget(new QLabel("Analysis units: "), 6, 0);
    controlsLayout->addLayout(unitLayout2, 6, 1, 1, 2);
    controlsLayout->addWidget(new QLabel("Scale factor: "), 7, 0);
    controlsLayout->addWidget(leScaleFactor, 7, 1, 1, 2);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(toolbox);

    setLayout(mainLayout);
    setupConnections();
    disableTools();
}

void OptionsPanel::setupConnections()
{
    connect(btnBrowse, &QToolButton::clicked,
            this, &OptionsPanel::selectFile);

    connect(cboType, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {
        QString units = cboType->itemData(index, Qt::UserRole + 1).toString();
        leUnitIn->setText(units);
        leUnitOut->setText(units);
    });

    connect(cboAvePeriod, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {
        if (cboAvePeriod->currentData().toInt() == 1)
            receptorTool->showWarning();
        else
            receptorTool->hideWarning();
    });

    // Units Controls
    connect(leUnitIn, &UDUnitsLineEdit::unitsChanged, [&]() {
        leUnitInVal->setText(leUnitIn->parsedText());
        leUnitOut->setConvertFrom(leUnitIn->text());
        double sf = leUnitOut->scaleFactor();
        leScaleFactor->setText(QString::number(sf));
    });

    connect(leUnitOut, &UDUnitsLineEdit::unitsChanged, [&]() {
        leUnitOutVal->setText(leUnitOut->parsedText());
        double sf = leUnitOut->scaleFactor();
        leScaleFactor->setText(QString::number(sf));
    });

    // Tool Actions
    connect(exportTool, &ExportTool::exportRequested,
            this, &OptionsPanel::exportRequested);

    connect(receptorTool, &ReceptorAnalysisTool::calcRequested,
            this, &OptionsPanel::receptorAnalysisRequested);

    connect(histogramTool, &HistogramAnalysisTool::calcRequested,
            this, &OptionsPanel::histogramAnalysisRequested);
}

ncpost::options::general OptionsPanel::analysisOpts() const
{
    ncpost::options::general opts;
    opts.output_type = cboType->currentData(Qt::UserRole).toString().toStdString();
    opts.averaging_period = cboAvePeriod->currentData().toInt();
    opts.scale_factor = leUnitOut->scaleFactor();
    opts.source_group = cboSourceGroup->currentText().toStdString();
    return opts;
}

ncpost::options::tsexport OptionsPanel::exportOpts() const
{
    return exportTool->exportOpts();
}

ncpost::options::statistics OptionsPanel::receptorAnalysisOpts() const
{
    return receptorTool->analysisOpts();
}

ncpost::options::histogram OptionsPanel::histogramAnalysisOpts() const
{
    return histogramTool->analysisOpts();
}

QString OptionsPanel::currentFile() const
{
    return leFile->text();
}

void OptionsPanel::addType(const std::string& type, const std::string& units)
{
    QString text;
    if (type == "conc") text = "CONC (concentration)";
    else if (type == "depos") text = "DEPOS (total deposition)";
    else if (type == "ddep") text = "DDEP (dry deposition)";
    else if (type == "wdep") text = "WDEP (wet deposition)";
    else return;

    QString qstype = QString::fromStdString(type);
    QString qsunits = QString::fromStdString(units);

    cboType->insertItem(cboType->count(), text, Qt::DisplayRole);
    cboType->setItemData(cboType->count() - 1, qstype, Qt::UserRole);
    cboType->setItemData(cboType->count() - 1, qsunits, Qt::UserRole + 1);
}

void OptionsPanel::resetType()
{
    cboType->setCurrentIndex(0);
    emit cboType->currentIndexChanged(0);
}

void OptionsPanel::setAveragingPeriods(std::vector<int>& periods)
{
    for (int period : periods) {
        QString text = QString::number(period);
        if (period == 1)
            text += " hour";
        else
            text += " hours";
        cboAvePeriod->insertItem(cboAvePeriod->count(), text, QVariant(period));
    }
}

void OptionsPanel::setSourceGroups(std::vector<std::string>& groups)
{
    for (const std::string& group : groups) {
        cboSourceGroup->insertItem(cboSourceGroup->count(), QString::fromStdString(group));
    }
}

void OptionsPanel::enableTools()
{
    exportTool->setEnabled(true);
    receptorTool->setEnabled(true);
    histogramTool->setEnabled(true);
}

void OptionsPanel::disableTools()
{
    exportTool->setEnabled(false);
    receptorTool->setEnabled(false);
    histogramTool->setEnabled(false);
}

void OptionsPanel::clearOptions()
{
    cboType->clear();
    cboAvePeriod->clear();
    cboSourceGroup->clear();
    leUnitIn->clear();
    leUnitInVal->clear();
    leUnitOut->clear();
    leUnitOutVal->clear();
    leScaleFactor->setText("1");
}

void OptionsPanel::selectFile()
{
    QSettings settings;
    QString defaultDirectory;
    QString settingsKey = "DefaultPostFileDirectory";

    QFileInfo fi(currentFile());
    if (fi.exists())
        defaultDirectory = fi.canonicalPath();
    else
        defaultDirectory = settings.value(settingsKey, qApp->applicationDirPath()).toString();

    const QString openfile = QFileDialog::getOpenFileName(
        this, tr("Select Output File"), defaultDirectory, tr("netCDF POSTFILE (*.nc)"));

    if (!openfile.isEmpty()) {
        QFileInfo fi(openfile);
        QString dir = fi.absoluteDir().absolutePath();
        settings.setValue(settingsKey, dir);

        leFile->setText(openfile);
        emit currentFileChanged();
    }
}

/****************************************************************************
** FileInfoPanel
****************************************************************************/

FileInfoPanel::FileInfoPanel(QWidget *parent)
    : QWidget(parent)
{
    infoTree = new QTreeWidget;
    infoTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    infoTree->setRootIsDecorated(false);
    infoTree->setColumnCount(2);
    infoTree->header()->setVisible(false);
    QFont font = infoTree->font();
    font.setPointSizeF(QApplication::font().pointSizeF());
    infoTree->setFont(font);

    itemTitle = new QTreeWidgetItem;
    itemTitle->setText(0, "Title:  ");
    itemTitle->setTextAlignment(0, Qt::AlignTop);
    itemTitle->setTextAlignment(1, Qt::AlignTop);
    itemOptions = new QTreeWidgetItem;
    itemOptions->setText(0, "Model Options:  ");
    itemVersion = new QTreeWidgetItem;
    itemVersion->setText(0, "Model Version:  ");
    itemReceptors = new QTreeWidgetItem;
    itemReceptors->setText(0, "Receptors:  ");
    itemTimeSteps = new QTreeWidgetItem;
    itemTimeSteps->setText(0, "Time Steps:  ");

    infoTree->insertTopLevelItem(0, itemTitle);
    infoTree->insertTopLevelItem(1, itemOptions);
    infoTree->insertTopLevelItem(2, itemVersion);
    infoTree->insertTopLevelItem(3, itemReceptors);
    infoTree->insertTopLevelItem(4, itemTimeSteps);

    infoTree->resizeColumnToContents(0);
    infoTree->setMinimumHeight(infoTree->sizeHintForRow(0) * 6);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(infoTree);

    setLayout(mainLayout);
}

void FileInfoPanel::setTitle(const QString& title)
{
    itemTitle->setText(1, title);
}
void FileInfoPanel::setOptions(const QString& options)
{
    itemOptions->setText(1, options);
}
void FileInfoPanel::setVersion(const QString& version)
{
    itemVersion->setText(1, version);
}
void FileInfoPanel::setReceptorCount(const int nrec)
{
    itemReceptors->setText(1, QString::number(nrec));
}
void FileInfoPanel::setTimeStepCount(const int ntime)
{
    itemTimeSteps->setText(1, QString::number(ntime));
}
void FileInfoPanel::clearContents()
{
    itemTitle->setText(1, "");
    itemOptions->setText(1, "");
    itemVersion->setText(1, "");
    itemReceptors->setText(1, "");
    itemTimeSteps->setText(1, "");
}

/****************************************************************************
** AnalysisWindow
****************************************************************************/

AnalysisWindow::AnalysisWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Analysis");
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_Measure)));
    setAttribute(Qt::WA_DeleteOnClose, false);

    // File Information
    QDockWidget *dwInfo = new QDockWidget(tr("File Information"), this);
    dwInfo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dwInfo->setFeatures(QDockWidget::DockWidgetMovable |
                        QDockWidget::DockWidgetFloatable);

    fileInfoPanel = new FileInfoPanel(dwInfo);
    dwInfo->setWidget(fileInfoPanel);
    addDockWidget(Qt::LeftDockWidgetArea, dwInfo);

    // Analysis Options
    QDockWidget *dwOpts = new QDockWidget(tr("Analysis Options"), this);
    dwOpts->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dwOpts->setFeatures(QDockWidget::DockWidgetMovable |
                        QDockWidget::DockWidgetFloatable);

    optionsPanel = new OptionsPanel(dwOpts);
    dwOpts->setWidget(optionsPanel);
    addDockWidget(Qt::LeftDockWidgetArea, dwOpts);

    // Central
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setAutoFillBackground(true);
    tabWidget->setMinimumWidth(1200);
    //tabWidget->setBackgroundRole(QPalette::AlternateBase);
    //tabWidget->setDocumentMode(true);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(5);
    centralLayout->addWidget(tabWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *central = new QWidget;
    central->setLayout(centralLayout);
    setCentralWidget(central);

    setupConnections();
}

void AnalysisWindow::setupConnections()
{
    connect(optionsPanel, &OptionsPanel::exportRequested,
            this, &AnalysisWindow::exportTimeSeries);

    connect(optionsPanel, &OptionsPanel::receptorAnalysisRequested,
            this, &AnalysisWindow::calcReceptorStats);

    connect(optionsPanel, &OptionsPanel::histogramAnalysisRequested,
            this, &AnalysisWindow::calcHistogram);

    connect(optionsPanel, &OptionsPanel::currentFileChanged,
            this, &AnalysisWindow::setCurrentFile);

    connect(tabWidget, &QTabWidget::tabCloseRequested, [=](int i) {
        QWidget *widget = tabWidget->widget(i);
        if (widget != nullptr)
            widget->deleteLater();
    });
}

void AnalysisWindow::setCurrentFile()
{
    filename = optionsPanel->currentFile();
    fileInfoPanel->clearContents();
    optionsPanel->clearOptions();
    optionsPanel->disableTools();

    try
    {
        ncpost::analysis analysis(filename.toStdString());
        auto metadata = analysis.metadata();
        int nr = static_cast<int>(metadata.receptors.size());
        int nt = static_cast<int>(metadata.time_steps.size());
        std::string title = metadata.title;
        std::string options = metadata.model_options;
        std::string version = metadata.model_version;
        auto types = metadata.output_types;
        auto periods = metadata.averaging_periods;
        auto groups = metadata.source_groups;

        fileInfoPanel->setTitle(QString::fromStdString(title).trimmed());
        fileInfoPanel->setOptions(QString::fromStdString(options));
        fileInfoPanel->setVersion(QString::fromStdString(version));
        fileInfoPanel->setReceptorCount(nr);
        fileInfoPanel->setTimeStepCount(nt);

        // Stop now if we don't have types or averaging periods.
        if (types.size() == 0 || periods.size() == 0)
            return;

        for (const auto& type : types) {
            optionsPanel->addType(type.name, type.units);
        }

        optionsPanel->setAveragingPeriods(periods);
        optionsPanel->setSourceGroups(groups);
        optionsPanel->resetType();
        optionsPanel->enableTools();
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "File Error", QString::fromLocal8Bit(e.what()));
    }
}

void AnalysisWindow::exportTimeSeries()
{
    auto opts = optionsPanel->analysisOpts();
    auto exopts = optionsPanel->exportOpts();

    try {
        ncpost::analysis analysis(filename.toStdString());
        analysis.export_time_series(opts, exopts);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Export Error", QString::fromLocal8Bit(e.what()));
        return;
    }
}

void AnalysisWindow::calcReceptorStats()
{
    if (filename.isEmpty())
        return;

    auto opts = optionsPanel->analysisOpts();
    auto statopts = optionsPanel->receptorAnalysisOpts();
    ncpost::statistics_type out;

    try {
        ncpost::analysis analysis(filename.toStdString());
        analysis.calc_receptor_stats(opts, statopts, out);
        showReceptorStats(opts, statopts, out, analysis.metadata());
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Analysis Error", QString::fromLocal8Bit(e.what()));
        return;
    }
}

void AnalysisWindow::calcHistogram()
{
    if (filename.isEmpty())
        return;

    auto opts = optionsPanel->analysisOpts();
    auto histopts = optionsPanel->histogramAnalysisOpts();
    ncpost::histogram_type out;

    try {
        ncpost::analysis analysis(filename.toStdString());
        analysis.calc_histogram(opts, histopts, out);
        showHistogram(opts, histopts, out, analysis.metadata());
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Analysis Error", QString::fromLocal8Bit(e.what()));
        return;
    }
}

void AnalysisWindow::showReceptorStats(const ncpost::options::general& opts, const ncpost::options::statistics& statopts,
                                       const ncpost::statistics_type& out, const ncpost::metadata_type& metadata)
{
    QStringList headers;
    headers << "Group" << "Receptor" << "X" << "Y" << "Z" << "Z Hill" << "Z Flag";
    if (statopts.calc_avg) headers << "Mean";
    if (statopts.calc_max)  headers << "Max";
    if (statopts.calc_std) headers << "Std. Dev.";
    for (double p : statopts.percentiles)
        headers << QString("P%1").arg(p * 100);
    for (double w : statopts.maxrm_windows)
        headers << QString("Max[MA%1]").arg(w);

    StandardTableView *table = outputTable();
    const int nrows = static_cast<int>(metadata.receptors.size());
    QStandardItemModel *model = new QStandardItemModel(table);
    table->setModel(model);
    model->setRowCount(nrows);
    model->setColumnCount(headers.size());
    model->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < nrows; ++i) {
        int col = 0;
        const auto& rec = metadata.receptors.at(i);
        QString recgrp;
        if (metadata.receptor_netid.count(rec.id))
            recgrp = QString::fromStdString(metadata.receptor_netid.at(rec.id));
        else if (metadata.receptor_arcid.count(rec.id))
            recgrp = QString::fromStdString(metadata.receptor_arcid.at(rec.id));

        model->setData(model->index(i, col++), recgrp, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.id, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.x, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.y, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.zelev, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.zhill, Qt::DisplayRole);
        model->setData(model->index(i, col++), rec.zflag, Qt::DisplayRole);
        if (statopts.calc_avg)
            model->setData(model->index(i, col++), out.avg.at(i), Qt::DisplayRole);
        if (statopts.calc_max)
            model->setData(model->index(i, col++), out.max.at(i), Qt::DisplayRole);
        if (statopts.calc_std)
            model->setData(model->index(i, col++), out.std.at(i), Qt::DisplayRole);
        for (std::vector<double> v : out.p2)
            model->setData(model->index(i, col++), v.at(i), Qt::DisplayRole);
        for (std::vector<double> v : out.rm)
            model->setData(model->index(i, col++), v.at(i), Qt::DisplayRole);
    }

    QString title = QString("Receptors (%1/%2/%3)")
            .arg(QString::fromStdString(opts.output_type).toUpper())
            .arg(opts.averaging_period)
            .arg(QString::fromStdString(opts.source_group).trimmed());

    tabWidget->addTab(table, title);
    tabWidget->setCurrentIndex(tabWidget->count() - 1);
}

void AnalysisWindow::showHistogram(const ncpost::options::general& opts, const ncpost::options::histogram& histopts,
                                   const ncpost::histogram_type& out, const ncpost::metadata_type& metadata)
{
    if (histopts.calc_cdf)
    {
        StandardTableView *table = outputTable();
        const int nrows = static_cast<int>(out.cdf.size());
        QStandardItemModel *model = new QStandardItemModel(table);
        table->setModel(model);
        model->setRowCount(nrows);
        model->setColumnCount(2);
        model->setHorizontalHeaderLabels(QStringList{"x", "F(x)"});

        for (int i = 0; i < nrows; ++i) {
            model->setData(model->index(i, 0), out.cdf[i].first, Qt::DisplayRole);
            model->setData(model->index(i, 1), out.cdf[i].second, Qt::DisplayRole);
        }

        QString title = QString("CDF (%1/%2/%3)")
                .arg(QString::fromStdString(opts.output_type).toUpper())
                .arg(opts.averaging_period)
                .arg(QString::fromStdString(opts.source_group).trimmed());

        tabWidget->addTab(table, title);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }

    if (histopts.calc_pdf)
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

        QString title = QString("PDF (%1/%2/%3)")
                .arg(QString::fromStdString(opts.output_type).toUpper())
                .arg(opts.averaging_period)
                .arg(QString::fromStdString(opts.source_group).trimmed());

        tabWidget->addTab(table, title);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }
}

StandardTableView *AnalysisWindow::outputTable(QWidget *parent)
{
    StandardTableView *table = new StandardTableView(parent);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setFrameStyle(QFrame::NoFrame);
    table->setSelectionBehavior(QAbstractItemView::SelectItems);
    table->setAutoFilterEnabled(true);

    return table;
}

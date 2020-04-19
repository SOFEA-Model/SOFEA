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

#include <QtWidgets>

#include "AppStyle.h"
#include "StandardPlot.h"
#include "FluxProfileDialog.h"
#include "FluxProfilePlot.h"
#include "delegates/DoubleItemDelegate.h"
#include "delegates/SpinBoxDelegate.h"
#include "widgets/BackgroundFrame.h"
#include "widgets/GridLayout.h"

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <csv/csv.h>

const QMap<FluxProfile::DSMethod, QString> FluxProfileDialog::dsMethodMap = {
    {FluxProfile::DSMethod::Disabled,      "Disabled"},
    {FluxProfile::DSMethod::LinearCDPR,    "Linear (CDPR)"},
    {FluxProfile::DSMethod::LinearGeneral, "Linear (General)"},
    {FluxProfile::DSMethod::Nonlinear,     "Nonlinear"}
};

const QMap<FluxProfile::TSMethod, QString> FluxProfileDialog::tsMethodMap = {
    {FluxProfile::TSMethod::Disabled,      "Disabled"},
    {FluxProfile::TSMethod::Seasonal,      "Seasonal (CDPR)"},
    {FluxProfile::TSMethod::Sinusoidal,    "Sinusoidal"}
};

FluxProfileDialog::FluxProfileDialog(std::shared_ptr<FluxProfile> fp, QWidget *parent)
    : QDialog(parent), wptr(fp)
{
    setWindowTitle(QString::fromStdString(fp->name));
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionStepChart)));
    setMinimumSize(650, 500);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    // Reference
    sbRefAppRate = new QDoubleSpinBox;
    sbRefAppRate->setRange(0, 10000000);
    sbRefAppRate->setDecimals(2);
    sbRefAppRate->setSingleStep(1);

    //bgFluxMode = new QButtonGroup(this);
    //radioVariableFlux = new QRadioButton(tr("Variable"));
    //radioConstantFlux = new QRadioButton(tr("Constant"));
    //bgFluxMode->addButton(radioVariableFlux, 1);
    //bgFluxMode->addButton(radioConstantFlux, 2);

    //radioConstantFlux->setDisabled(true); // FIXME - NOT YET IMPLEMENTED
    //radioVariableFlux->setDisabled(true); // FIXME - NOT YET IMPLEMENTED

    //lblConstantFluxInfo = new StatusLabel;
    //lblConstantFluxInfo->setStatusType(StatusLabel::InfoTip);
    //lblConstantFluxInfo->setText(
    //    "Constant flux is determined from the mean scaled hourly flux profile "
    //    "for each source, and applies to the full simulation period. "
    //    "Application start time is used for flux scaling only."
    //);
    //lblConstantFluxInfo->hide();

    refTable = new StandardTableView;

    StandardTableEditor::StandardButtons refEditorOpts =
        StandardTableEditor::Add |
        StandardTableEditor::Remove |
        StandardTableEditor::Import;

    refEditor = new StandardTableEditor(Qt::Vertical, refEditorOpts);
    btnPlot = new QPushButton("Plot...");

    // Temporal Scaling
    cboTemporalScaling = new QComboBox;
    deRefDate = new QDateTimeEdit;
    deRefDate->setTimeSpec(Qt::UTC);
    deRefDate->setDisplayFormat("yyyy-MM-dd HH:mm");
    deRefDate->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC));

    // Temporal Scaling - Linear
    deStartDate = new QDateEdit;
    deStartDate->setDisplayFormat("MM/dd");
    deEndDate = new QDateEdit;
    deEndDate->setDisplayFormat("MM/dd");
    sbScaleFactor = new QDoubleSpinBox;
    sbScaleFactor->setMinimum(0);
    sbScaleFactor->setSingleStep(0.1);

    // Temporal Scaling - Sinusoidal
    sbAmplitude = new QDoubleSpinBox;
    sbAmplitude->setSingleStep(0.1);
    sbCenterAmplitude = new QDoubleSpinBox;
    sbCenterAmplitude->setSingleStep(0.1);
    sbPhase = new QDoubleSpinBox;
    sbPhase->setSuffix(QLatin1String("\xb0"));
    sbPhase->setRange(0, 359.99);
    sbPhase->setSingleStep(30);
    sbPhase->setWrapping(true);
    sbWavelength = new QDoubleSpinBox;
    sbWavelength->setRange(0.01, 1461); // 365+365+365+366
    sbWavelength->setSingleStep(10);
    btnCalcPhase = new QPushButton("Estimate");
    btnPlotTS = new QPushButton("Plot...");

    // Incorporation Depth Scaling
    cboDepthScaling = new QComboBox;
    sbRefDepth = new QDoubleSpinBox;
    sbRefDepth->setRange(2.54, 100);
    sbRefDepth->setDecimals(2);
    sbRefDepth->setSingleStep(0.01);
    sbRefVL = new QDoubleSpinBox;
    sbRefVL->setRange(0.01, 100);
    sbRefVL->setDecimals(2);
    sbRefVL->setSuffix("%");
    sbRefVL->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbMaxVL = new QDoubleSpinBox;
    sbMaxVL->setRange(0.01, 100);
    sbMaxVL->setDecimals(2);
    sbMaxVL->setSuffix("%");
    sbMaxVL->setButtonSymbols(QAbstractSpinBox::NoButtons);
    btnPlotDS = new QPushButton("Plot...");

    // Reference Layout
    //QHBoxLayout *fluxModeLayout = new QHBoxLayout;
    //fluxModeLayout->addWidget(radioVariableFlux);
    //fluxModeLayout->addWidget(radioConstantFlux);
    //fluxModeLayout->setAlignment(radioVariableFlux, Qt::AlignLeft);
    //fluxModeLayout->setAlignment(radioConstantFlux, Qt::AlignLeft);

    GridLayout *refInputLayout = new GridLayout;
    refInputLayout->setMargin(0);
    refInputLayout->addWidget(new QLabel(tr("Application rate (kg/ha):")), 1, 0);
    refInputLayout->addWidget(sbRefAppRate, 1, 1);
    //refInputLayout->addWidget(new QLabel(tr("Flux calculation mode:")), 2, 0);
    //refInputLayout->addLayout(fluxModeLayout, 2, 1, Qt::AlignLeft);
    //refInputLayout->addWidget(lblConstantFluxInfo, 3, 0, 1, 2);

    QVBoxLayout *refControlsLayout = new QVBoxLayout;
    refControlsLayout->setMargin(0);
    refControlsLayout->addWidget(refEditor);
    refControlsLayout->addStretch(1);
    refControlsLayout->addWidget(btnPlot);

    QHBoxLayout *refTableLayout = new QHBoxLayout;
    refTableLayout->setMargin(0);
    refTableLayout->addWidget(refTable);
    refTableLayout->addLayout(refControlsLayout);

    QVBoxLayout *refLayout = new QVBoxLayout;
    refLayout->addLayout(refInputLayout);
    refLayout->addSpacing(5);
    refLayout->addLayout(refTableLayout);

    QWidget *referenceTab = new QWidget;
    referenceTab->setLayout(refLayout);

    // Temporal Scaling - CDPR Layout
    GridLayout *stackInner1 = new GridLayout;
    stackInner1->setMargin(0);
    stackInner1->addWidget(new QLabel(tr("Warm season start date:")), 0, 0);
    stackInner1->addWidget(deStartDate, 0, 1);
    stackInner1->addWidget(new QLabel(tr("Warm season end date:")), 1, 0);
    stackInner1->addWidget(deEndDate, 1, 1);
    stackInner1->addWidget(new QLabel(tr("Warm season scale factor:")), 2, 0);
    stackInner1->addWidget(sbScaleFactor, 2, 1);
    stackInner1->addWidget(new QLabel(tr("")), 3, 0);
    QVBoxLayout *stackOuter1 = new QVBoxLayout;
    stackOuter1->setMargin(0);
    stackOuter1->addLayout(stackInner1);
    QWidget *stackWidget1 = new QWidget;
    stackWidget1->setLayout(stackOuter1);

    // Temporal Scaling - Sinusoidal Layout
    QHBoxLayout *phaseLayout = new QHBoxLayout;
    phaseLayout->addWidget(sbPhase, 1);
    phaseLayout->addWidget(btnCalcPhase, 0);

    GridLayout *stackInner2 = new GridLayout;
    stackInner2->setMargin(0);
    stackInner2->addWidget(new QLabel(tr("Amplitude (A):")), 0, 0);
    stackInner2->addWidget(sbAmplitude, 0, 1);
    stackInner2->addWidget(new QLabel(tr("Center amplitude (D):")), 1, 0);
    stackInner2->addWidget(sbCenterAmplitude, 1, 1);
    stackInner2->addWidget(new QLabel(tr("Phase (\xcf\x95):")), 2, 0);
    stackInner2->addLayout(phaseLayout, 2, 1);
    stackInner2->addWidget(new QLabel(tr("Wavelength (\xce\xbb):")), 3, 0);
    stackInner2->addWidget(sbWavelength, 3, 1);
    QVBoxLayout *stackOuter2 = new QVBoxLayout;
    stackOuter2->setMargin(0);
    stackOuter2->addLayout(stackInner2);
    QWidget *stackWidget2 = new QWidget;
    stackWidget2->setLayout(stackOuter2);

    // Temporal Scaling Layout
    GridLayout *temporalParamLayout = new GridLayout;
    temporalParamLayout->addWidget(new QLabel(tr("Flux scaling method:")), 0, 0);
    temporalParamLayout->addWidget(cboTemporalScaling, 0, 1);
    temporalParamLayout->addWidget(new QLabel(tr("Reference start date:")), 1, 0);
    temporalParamLayout->addWidget(deRefDate, 1, 1);
    temporalStack = new QStackedWidget;
    temporalStack->addWidget(new QWidget);
    temporalStack->addWidget(stackWidget1);
    temporalStack->addWidget(stackWidget2);
    QHBoxLayout *temporalPlotLayout = new QHBoxLayout();
    temporalPlotLayout->addStretch(1);
    temporalPlotLayout->addWidget(btnPlotTS);
    QVBoxLayout *temporalLayout = new QVBoxLayout;
    temporalLayout->addLayout(temporalParamLayout);
    temporalLayout->addWidget(temporalStack);
    temporalLayout->addStretch(1);
    temporalLayout->addLayout(temporalPlotLayout);

    // Incorporation Depth Scaling Layout
    GridLayout *depthParamLayout = new GridLayout;
    depthParamLayout->addWidget(new QLabel(tr("Flux scaling method:")), 0, 0);
    depthParamLayout->addWidget(cboDepthScaling, 0, 1);
    depthParamLayout->addWidget(new QLabel(tr("Reference depth (cm):")), 1, 0);
    depthParamLayout->addWidget(sbRefDepth, 1, 1);
    depthParamLayout->addWidget(new QLabel(tr("Measured volatilization loss:")), 2, 0);
    depthParamLayout->addWidget(sbRefVL, 2, 1);
    depthParamLayout->addWidget(new QLabel(tr("Maximum volatilization loss:")), 3, 0);
    depthParamLayout->addWidget(sbMaxVL, 3, 1);
    QHBoxLayout *depthPlotLayout = new QHBoxLayout();
    depthPlotLayout->addStretch(1);
    depthPlotLayout->addWidget(btnPlotDS);
    QVBoxLayout *depthLayout = new QVBoxLayout;
    depthLayout->addLayout(depthParamLayout);
    depthLayout->addStretch(1);
    depthLayout->addLayout(depthPlotLayout);

    // Time Scaling Tab Layout
    QWidget *timeScalingTab = new QWidget;
    timeScalingTab->setLayout(temporalLayout);

    // Depth Scaling Tab Layout
    QWidget *depthScalingTab = new QWidget;
    depthScalingTab->setLayout(depthLayout);

    // Tab Widget
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(referenceTab, tr("Reference"));
    tabWidget->addTab(timeScalingTab, tr("Time Scaling"));
    tabWidget->addTab(depthScalingTab, tr("Depth Scaling"));

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    init();
}

void FluxProfileDialog::init()
{
    using TSMethod = FluxProfile::TSMethod;
    using DSMethod = FluxProfile::DSMethod;

    for (TSMethod key : tsMethodMap.keys()) {
        QString val = tsMethodMap[key];
        cboTemporalScaling->addItem(val, static_cast<int>(key));
    }

    for (DSMethod key : dsMethodMap.keys()) {
        QString val = dsMethodMap[key];
        cboDepthScaling->addItem(val, static_cast<int>(key));
    }

    // Initialize StandardItemModel for Reference Flux
    refModel = new QStandardItemModel;
    refModel->setColumnCount(2);
    QString fluxHeader = QLatin1String("Flux (g/m\xb2/s)"); // [g m-2 s-1]
    refModel->setHorizontalHeaderLabels(QStringList{"Duration (hr)", fluxHeader});

    refTable->setModel(refModel);
    refTable->setItemDelegateForColumn(0, new SpinBoxDelegate(1, 1000, 1));
    refTable->setItemDelegateForColumn(1, new DoubleItemDelegate(0, 10000000, 8, false));
    refTable->setMinimumWidth(400);

    refEditor->setView(refTable);
    refEditor->setImportFilter("Flux Profile (*.csv *.txt)");
    refEditor->setImportCaption(tr("Import Flux Profile"));

    // Connections
    //connect(radioConstantFlux, &QRadioButton::toggled, lblConstantFluxInfo, &QLabel::setVisible);
    connect(refEditor, &StandardTableEditor::importRequested, this, &FluxProfileDialog::importFluxProfile);
    connect(btnCalcPhase, &QPushButton::clicked, this, &FluxProfileDialog::calcPhase);
    connect(btnPlot, &QPushButton::clicked, this, &FluxProfileDialog::plotFluxProfile);
    connect(btnPlotTS, &QPushButton::clicked, this, &FluxProfileDialog::plotTemporalScaling);
    connect(btnPlotDS, &QPushButton::clicked, this, &FluxProfileDialog::plotDepthScaling);
    connect(cboTemporalScaling, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            temporalStack, &QStackedWidget::setCurrentIndex);

    // Constraints
    connect(sbMaxVL, &QDoubleSpinBox::editingFinished, this, [this] {
        const double newValue = sbMaxVL->value();
        if (sbRefVL->value() > newValue)
            sbRefVL->setValue(newValue);
    });

    connect(sbRefVL, &QDoubleSpinBox::editingFinished, this, [this] {
        const double newValue = sbRefVL->value();
        if (sbMaxVL->value() < newValue)
            sbMaxVL->setValue(newValue);
    });

    connect(deEndDate, &QDateEdit::dateChanged,
        [=](const QDate &newDate) {
            if (deStartDate->date() > newDate)
                deStartDate->setDate(newDate);
        }
    );

    connect(deStartDate, &QDateEdit::dateChanged,
        [=](const QDate &newDate) {
            if (deEndDate->date() < newDate)
                deEndDate->setDate(newDate);
        }
    );

    load();

    connect(refModel, &QStandardItemModel::rowsInserted, this, &FluxProfileDialog::onRowsInserted);
}

void FluxProfileDialog::save()
{
    std::shared_ptr<FluxProfile> fp = wptr.lock();
    if (!fp) {
        BOOST_LOG_TRIVIAL(error) << "Failed to acquire write lock";
        return;
    }

    std::string name = fp->name;
    *fp = currentProfile();
    fp->name = name;
}

void FluxProfileDialog::load()
{
    std::shared_ptr<FluxProfile> fp = wptr.lock();
    if (!fp) {
        BOOST_LOG_TRIVIAL(error) << "Failed to acquire read lock";
        return;
    }

    refModel->removeRows(0, refModel->rowCount());
    for (const auto& xy : fp->refFlux) {
        int currentRow = refModel->rowCount();
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(xy.first, Qt::DisplayRole);
        item1->setData(xy.second, Qt::DisplayRole);
        refModel->setItem(currentRow, 0, item0);
        refModel->setItem(currentRow, 1, item1);
    }

    //if (fp->constantFlux)
    //    radioConstantFlux->setChecked(true);
    //else
    //    radioVariableFlux->setChecked(true);
    deRefDate->setDateTime(fp->refStart);
    sbRefAppRate->setValue(fp->refAppRate);
    sbRefDepth->setValue(fp->refDepth);
    sbRefVL->setValue(fp->refVL * 100.);
    sbMaxVL->setValue(fp->maxVL * 100.);

    // Temporal Scaling
    int tsMethod = static_cast<int>(fp->tsMethod);
    int tsIndex = cboTemporalScaling->findData(tsMethod);
    cboTemporalScaling->setCurrentIndex(tsIndex);
    temporalStack->setCurrentIndex(tsIndex);

    deStartDate->setDate(fp->warmSeasonStart);
    deEndDate->setDate(fp->warmSeasonEnd);
    sbScaleFactor->setValue(fp->warmSeasonSF);
    sbAmplitude->setValue(fp->amplitude);
    sbCenterAmplitude->setValue(fp->centerAmplitude);
    sbPhase->setValue(fp->phase);
    sbWavelength->setValue(fp->wavelength);

    // Depth Scaling
    int dsMethod = static_cast<int>(fp->dsMethod);
    int dsIndex = cboDepthScaling->findData(dsMethod);
    cboDepthScaling->setCurrentIndex(dsIndex);
}

FluxProfile FluxProfileDialog::currentProfile()
{
    FluxProfile fp;

    for (int i = 0; i < refModel->rowCount(); ++i) {
        if (refModel->item(i, 0) && refModel->item(i, 1)) {
            QModelIndex xi = refModel->index(i, 0);
            QModelIndex yi = refModel->index(i, 1);
            int x = refModel->data(xi).toInt();
            double y = refModel->data(yi).toDouble();
            if (x > 0 && y >= 0) {
                fp.refFlux.push_back(std::make_pair(x, y));
            }
        }
    }

    using TSMethod = FluxProfile::TSMethod;
    using DSMethod = FluxProfile::DSMethod;

    fp.tsMethod = static_cast<TSMethod>(cboTemporalScaling->currentData().toInt());
    fp.dsMethod = static_cast<DSMethod>(cboDepthScaling->currentData().toInt());
    //fp.constantFlux = radioConstantFlux->isChecked();
    fp.refStart = deRefDate->dateTime();
    fp.refAppRate = sbRefAppRate->value();
    fp.refDepth = sbRefDepth->value();
    fp.refVL = sbRefVL->value() / 100.;
    fp.maxVL = sbMaxVL->value() / 100.;
    fp.warmSeasonStart = deStartDate->date();
    fp.warmSeasonEnd = deEndDate->date();
    fp.warmSeasonSF = sbScaleFactor->value();
    fp.amplitude = sbAmplitude->value();
    fp.centerAmplitude = sbCenterAmplitude->value();
    fp.phase = sbPhase->value();
    fp.wavelength = sbWavelength->value();

    return fp;
}

void FluxProfileDialog::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    for (int i = first; i <= last; ++i) {
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData("1", Qt::DisplayRole);
        item1->setData("0", Qt::DisplayRole);
        refModel->setItem(i, 0, item0);
        refModel->setItem(i, 1, item1);
    }
}

void FluxProfileDialog::importFluxProfile(const QString &filename)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import")

    using ReferenceFlux = FluxProfile::ReferenceFlux;

    // Remove existing rows
    refModel->removeRows(0, refModel->rowCount());

    ReferenceFlux imported;

    try {
        io::CSVReader<2> in(filename.toStdString());
        while (true) {
            int hour;
            double flux;
            if (in.read_row(hour, flux))
                imported.push_back(std::make_pair(hour, flux));
            else
                break;
        }
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "Import Failed", QString::fromLocal8Bit(e.what()));
        return;
    }

    for (const auto &xy : imported) {
        int currentRow = refModel->rowCount();
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(xy.first, Qt::DisplayRole);
        item1->setData(xy.second, Qt::DisplayRole);
        refModel->setItem(currentRow, 0, item0);
        refModel->setItem(currentRow, 1, item1);
    }
}

void FluxProfileDialog::calcPhase()
{
    // Calculate the phase shift (phi) based on reference date
    QDate refDate = deRefDate->date();
    if (!refDate.isValid())
        return;

    int numerator = refDate.dayOfYear() - 1; // Jan 1 is zero
    int denominator = refDate.daysInYear(); // 365 or 366
    double prop = static_cast<double>(numerator) / static_cast<double>(denominator);
    double phi = 270.0 - (prop * 360.0);

    // Adjust to 0-360
    phi = fmod(phi, 360.0);
    if(phi < 0)
        phi += 360.0;

    sbPhase->setValue(phi);
}

void FluxProfileDialog::plotFluxProfile()
{
    FluxProfile fp = currentProfile();
    FluxProfilePlot *plotWidget = new FluxProfilePlot(fp);
    plotWidget->setAppStart(fp.refStart);
    plotWidget->setAppRate(fp.refAppRate);
    plotWidget->setIncorpDepth(fp.refDepth);
    plotWidget->setupConnections();
    plotWidget->updatePlot();

    QDialog *plotDialog = new QDialog(this);
    QDialogButtonBox *plotDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(plotDialogButtonBox, &QDialogButtonBox::rejected, plotDialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotWidget);
    mainLayout->addWidget(plotDialogButtonBox);

    plotDialog->setWindowTitle("Reference Flux Profile");
    plotDialog->setWindowFlag(Qt::Tool);
    plotDialog->setAttribute(Qt::WA_DeleteOnClose);
    plotDialog->setLayout(mainLayout);
    plotDialog->exec();
}

void FluxProfileDialog::plotTemporalScaling()
{
    using TSMethod = FluxProfile::TSMethod;
    FluxProfile fp = currentProfile();

    if (!fp.refStart.isValid()) {
        QMessageBox::critical(this, "Calculation Error", "Field study reference date is invalid.");
        return;
    }

    QDateTime dt0 = QDateTime(QDate(fp.refStart.date().year(), 1, 1));
    QDateTime dt1 = QDateTime(dt0.addYears(1));

    dt0.setTimeSpec(Qt::UTC);
    dt1.setTimeSpec(Qt::UTC);

    // Plot Limits
    double x0 = QwtDate::toDouble(dt0);
    double x1 = QwtDate::toDouble(dt1);
    double y0 = 0.0;
    double y1 = 1.0;

    if (fp.tsMethod == TSMethod::Seasonal) {
        y0 = 1.0;
        y1 = fp.warmSeasonSF;
    }
    if (fp.tsMethod == TSMethod::Sinusoidal) {
        y0 = fp.centerAmplitude - fp.amplitude;
        y1 = fp.centerAmplitude + fp.amplitude;
    }

    QVector<QPointF> series;
    int n = fp.refStart.date().daysInYear(); // 365 or 366
    series.reserve(n);

    for (int i = 0; i < n; ++i) {
        QDateTime x = dt0.addDays(i);
        double y = fp.timeScaleFactor(x);
        double xf = QwtDate::toDouble(x);
        QPointF p(xf, y);
        series.push_back(p);
    }

    StandardPlot *plot = new StandardPlot;
    plot->setCurveTracker(true);
    QwtPlotCurve *sfCurve = new QwtPlotCurve;
    sfCurve->setPen(Qt::blue, 1.5);
    if (fp.tsMethod == TSMethod::Seasonal)
        sfCurve->setStyle(QwtPlotCurve::Steps);
    else
        sfCurve->setStyle(QwtPlotCurve::Lines);
    sfCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    sfCurve->setSamples(series);
    sfCurve->attach(plot);

    // Use reference date on X-axis
    QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    scaleDraw->setDateFormat(QwtDate::Month, QString("MMM"));
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine(Qt::UTC);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    // Set plot limits
    plot->setAxisScale(QwtPlot::xBottom, x0, x1);
    plot->setAxisScale(QwtPlot::yLeft, y0, y1);

    // Add reference date
    double xref = QwtDate::toDouble(QDateTime(fp.refStart.date()));
    plot->addRefVLine(xref, 0, " Field Study");

    // Layout
    BackgroundFrame *plotFrame = new BackgroundFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);

    QDialog *dialog = new QDialog(this);
    QDialogButtonBox *plotDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(plotDialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotFrame);
    mainLayout->addWidget(plotDialogButtonBox);

    dialog->setWindowTitle("Time Scale Factor");
    dialog->setWindowFlag(Qt::Tool);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setLayout(mainLayout);
    dialog->exec();
}

void FluxProfileDialog::plotDepthScaling()
{
    FluxProfile fp = currentProfile();

    double x0 = 0.0;
    double x1 = fp.refDepth * 1.5;
    double y0 = fp.depthScaleFactor(x1);
    double y1 = fp.depthScaleFactor(x0);

    // Prevent setting axis limits with y0 = y1
    if (std::abs(y1 - y0) < std::numeric_limits<double>::epsilon()) {
        y0 = 0.0;
        y1 = 1.0;
    }

    double step = 0.01;

    QVector<QPointF> series;
    for (double x = x0; x < x1; x += step) {
        double y = fp.depthScaleFactor(x);
        QPointF p(x, y);
        series.push_back(p);
    }

    StandardPlot *plot = new StandardPlot;
    plot->setXAxisTitle("Depth (cm)");
    plot->setCurveTracker(true);
    QwtPlotCurve *sfCurve = new QwtPlotCurve;
    sfCurve->setPen(Qt::blue, 1.5);
    sfCurve->setStyle(QwtPlotCurve::Lines);
    sfCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    sfCurve->setSamples(series);
    sfCurve->attach(plot);

    // Set plot limits
    plot->setAxisScale(QwtPlot::xBottom, x0, x1);
    plot->setAxisScale(QwtPlot::yLeft, y0, y1);

    // Add reference depth
    plot->addRefVLine(fp.refDepth, 0, " Field Study");

    // Layout
    BackgroundFrame *plotFrame = new BackgroundFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);

    QDialog *dialog = new QDialog(this);
    QDialogButtonBox *plotDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(plotDialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotFrame);
    mainLayout->addWidget(plotDialogButtonBox);

    dialog->setWindowTitle("Depth Scale Factor");
    dialog->setWindowFlag(Qt::Tool);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setLayout(mainLayout);
    dialog->exec();
}

void FluxProfileDialog::accept()
{
    save();
    QDialog::done(QDialog::Accepted);
}

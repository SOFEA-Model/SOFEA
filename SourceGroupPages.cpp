#include <QtWidgets>

#include "StandardPlot.h"
#include "SourceGroupPages.h"
#include "Utilities.h"

#include <boost/log/trivial.hpp>

#include "csv/csv.h"

/****************************************************************************
** Application
****************************************************************************/

const QMap<SourceGroup::AppMethod, QString> ApplicationPage::appMethodMap = {
    {SourceGroup::AppMethod::Other,                  "Other"},
    {SourceGroup::AppMethod::TIFDripIrrigation,      "TIF Drip Irrigation"},
    {SourceGroup::AppMethod::TIFShankInjection,      "TIF Shank Injection"},
    {SourceGroup::AppMethod::NonTIFDripIrrigation,   "Non-TIF Drip Irrigation"},
    {SourceGroup::AppMethod::NonTIFShankInjection,   "Non-TIF Shank Injection"},
    {SourceGroup::AppMethod::UntarpedDripIrrigation, "Untarped Drip Irrigation"},
    {SourceGroup::AppMethod::UntarpedShankInjection, "Untarped Shank Injection"}
};

ApplicationPage::ApplicationPage(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    cboAppMethod = new QComboBox;

    sbAppFactor = new QDoubleSpinBox;
    sbAppFactor->setRange(0, 10);
    sbAppFactor->setDecimals(2);
    sbAppFactor->setSingleStep(0.1);

    bgCalcMode = new QButtonGroup;
    radioProspective = new QRadioButton(tr("Prospective"));
    radioValidation = new QRadioButton(tr("Retrospective"));
    bgCalcMode->addButton(radioProspective, 1);
    bgCalcMode->addButton(radioValidation, 2);

    // NOTE: uodate SourceTable delegates with any format changes.

    mcAppStart = new MonteCarloDateTimeEdit;
    mcAppStart->setDisplayFormat("yyyy-MM-dd HH:mm");

    mcAppRate = new MonteCarloLineEdit;
    mcAppRate->setRange(0, 10000000);
    mcAppRate->setDecimals(2);

    mcIncorpDepth = new MonteCarloLineEdit; // FIXME: constrain to 2.54 - 100
    mcIncorpDepth->setRange(0, 100);
    mcIncorpDepth->setDecimals(2);

    QHBoxLayout *calcModeLayout = new QHBoxLayout;
    calcModeLayout->addWidget(radioProspective);
    calcModeLayout->addWidget(radioValidation);

    GridLayout *layout1 = new GridLayout;
    layout1->setColumnStretch(0, 1);
    layout1->setColumnStretch(1, 2);
    layout1->addWidget(new QLabel(tr("Application method:")), 0, 0);
    layout1->addWidget(cboAppMethod, 0, 1);
    layout1->addWidget(new QLabel(tr("Application factor:")), 1, 0);
    layout1->addWidget(sbAppFactor, 1, 1);
    layout1->addWidget(new QLabel(tr("Calculation mode:")), 2, 0);
    layout1->addLayout(calcModeLayout, 2, 1);

    GridLayout *layout2 = new GridLayout;
    layout2->setColumnStretch(0, 1);
    layout2->setColumnStretch(1, 2);
    layout2->addWidget(new QLabel(tr("Application start time:")), 0, 0);
    layout2->addWidget(mcAppStart, 0, 1);
    layout2->addWidget(new QLabel(tr("Application rate (kg/ha):")), 1, 0);
    layout2->addWidget(mcAppRate, 1, 1);
    layout2->addWidget(new QLabel(tr("Incorporation depth (cm):")), 2, 0);
    layout2->addWidget(mcIncorpDepth, 2, 1);

    gbMonteCarlo = new QGroupBox("Monte Carlo Parameters");
    gbMonteCarlo->setFlat(true);
    gbMonteCarlo->setLayout(layout2);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(gbMonteCarlo);
    mainLayout->addStretch(1);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void ApplicationPage::init()
{
    for (SourceGroup::AppMethod key : appMethodMap.keys()) {
        QString val = appMethodMap[key];
        cboAppMethod->addItem(val, static_cast<int>(key));
    }

    connect(radioValidation, &QRadioButton::toggled, [=](bool checked) {
        gbMonteCarlo->setDisabled(checked);
    });

    load();
}

void ApplicationPage::save()
{
    sgPtr->appMethod = static_cast<SourceGroup::AppMethod>(cboAppMethod->currentIndex());
    sgPtr->appFactor = sbAppFactor->value();
    sgPtr->validationMode = radioValidation->isChecked();
    sgPtr->appStart = mcAppStart->dateTime(); // FIXME: mcAppStart->getDistribution();
    sgPtr->appRate = mcAppRate->getDistribution();
    sgPtr->incorpDepth = mcIncorpDepth->getDistribution();
}

void ApplicationPage::load()
{
    cboAppMethod->setCurrentIndex(static_cast<int>(sgPtr->appMethod));
    sbAppFactor->setValue(sgPtr->appFactor);
    if (sgPtr->validationMode)
        radioValidation->setChecked(true);
    else
        radioProspective->setChecked(true);
    mcAppStart->setDateTime(sgPtr->appStart); // FIXME: mcAppStart->setDistribution(sgPtr->appStart);
    mcAppRate->setDistribution(sgPtr->appRate);
    mcIncorpDepth->setDistribution(sgPtr->incorpDepth);
}

/****************************************************************************
** Flux Profile
****************************************************************************/

const QMap<FluxScaling::DSMethod, QString> FluxProfilePage::dsMethodMap = {
    {FluxScaling::DSMethod::Disabled,      "Disabled"},
    {FluxScaling::DSMethod::LinearCDPR,    "Linear (CDPR)"},
    {FluxScaling::DSMethod::LinearGeneral, "Linear (General)"},
    {FluxScaling::DSMethod::Nonlinear,     "Nonlinear"}
};

const QMap<FluxScaling::TSMethod, QString> FluxProfilePage::tsMethodMap = {
    {FluxScaling::TSMethod::Disabled,   "Disabled"},
    {FluxScaling::TSMethod::Seasonal,   "Seasonal (CDPR)"},
    {FluxScaling::TSMethod::Sinusoidal, "Sinusoidal"}
};

FluxProfilePage::FluxProfilePage(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    // Reference
    sbRefAppRate = new QDoubleSpinBox;
    sbRefAppRate->setRange(0, 10000000);
    sbRefAppRate->setDecimals(2);
    sbRefAppRate->setSingleStep(1);
    refTable = new StandardTableView;
    refEditor = new StandardTableEditor(QBoxLayout::TopToBottom);

    // Temporal Scaling
    cboTemporalScaling = new QComboBox;
    deRefDate = new QDateTimeEdit;
    deRefDate->setTimeSpec(Qt::UTC);
    deRefDate->setDisplayFormat("yyyy-MM-dd HH:mm");
    deRefDate->setDate(QDate(2000, 1, 1));

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
    btnPlotTS = new QPushButton("Plot");

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
    btnPlotDS = new QPushButton("Plot");

    // Reference Layout
    GridLayout *refInputLayout = new GridLayout;
    refInputLayout->setMargin(0);
    refInputLayout->addWidget(new QLabel(tr("Application rate (kg/ha):")), 1, 0);
    refInputLayout->addWidget(sbRefAppRate, 1, 1);

    QHBoxLayout *refTableLayout = new QHBoxLayout;
    refTableLayout->setMargin(0);
    refTableLayout->addWidget(refTable);
    refTableLayout->addWidget(refEditor);

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
    stackInner2->setColumnStretch(0, 1);
    stackInner2->setColumnStretch(1, 1);
    stackInner2->addWidget(new QLabel(tr("Amplitude (A):")), 0, 0);
    stackInner2->addWidget(sbAmplitude, 0, 1);
    stackInner2->addWidget(new QLabel(tr("Center Amplitude (D):")), 1, 0);
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
    temporalParamLayout->addWidget(new QLabel(tr("Reference start date:")), 0, 0);
    temporalParamLayout->addWidget(deRefDate, 0, 1);
    temporalParamLayout->addWidget(new QLabel(tr("Flux scaling method:")), 1, 0);
    temporalParamLayout->addWidget(cboTemporalScaling, 1, 1);
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
    depthParamLayout->addWidget(new QLabel(tr("Reference depth (cm):")), 0, 0);
    depthParamLayout->addWidget(sbRefDepth, 0, 1);
    depthParamLayout->addWidget(new QLabel(tr("Measured volatilization loss:")), 1, 0);
    depthParamLayout->addWidget(sbRefVL, 1, 1);
    depthParamLayout->addWidget(new QLabel(tr("Maximum volatilization loss:")), 2, 0);
    depthParamLayout->addWidget(sbMaxVL, 2, 1);
    depthParamLayout->addWidget(new QLabel(tr("Flux scaling method:")), 3, 0);
    depthParamLayout->addWidget(cboDepthScaling, 3, 1);
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
    mainLayout->setMargin(0);

    setLayout(mainLayout);
    init();
}

void FluxProfilePage::init()
{
    using TSMethod = FluxScaling::TSMethod;
    using DSMethod = FluxScaling::DSMethod;

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
    QString fluxHeader = QLatin1String("Flux (g/m\xb2/second)"); // [g m-2 s-1]
    refModel->setHorizontalHeaderLabels(QStringList{"Interval (hr)", fluxHeader});

    refTable->setModel(refModel);
    refTable->setSpinBoxForColumn(0, 1, 1000);
    refTable->setDoubleLineEditForColumn(1, 0, 10000000, 8, false);

    refEditor->init(refTable);
    refEditor->setImportEnabled(true);
    refEditor->setImportFilter("Flux Profile (*.csv *.txt)");
    refEditor->setImportCaption(tr("Import Flux Profile"));

    // Connections
    connect(refEditor, &StandardTableEditor::importRequested, this, &FluxProfilePage::importFluxProfile);
    connect(btnCalcPhase, &QPushButton::clicked, this, &FluxProfilePage::calcPhase);
    connect(btnPlotTS, &QPushButton::clicked, this, &FluxProfilePage::plotTemporalScaling);
    connect(btnPlotDS, &QPushButton::clicked, this, &FluxProfilePage::plotDepthScaling);
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

    connect(refModel, &QStandardItemModel::rowsInserted, this, &FluxProfilePage::onRowsInserted);
}

void FluxProfilePage::save()
{
    // Flux Profile
    sgPtr->refFlux.clear();
    for (int i = 0; i < refModel->rowCount(); ++i) {
        if (refModel->item(i, 0) && refModel->item(i, 1)) {
            QModelIndex xi = refModel->index(i, 0);
            QModelIndex yi = refModel->index(i, 1);
            int x = refModel->data(xi).toInt();
            double y = refModel->data(yi).toDouble();
            if (x > 0 && y >= 0) {
                sgPtr->refFlux.push_back(std::make_pair(x, y));
            }
        }
    }

    using TSMethod = FluxScaling::TSMethod;
    using DSMethod = FluxScaling::DSMethod;
    FluxScaling fs;

    fs.tsMethod = static_cast<TSMethod>(cboTemporalScaling->currentData().toInt());
    fs.dsMethod = static_cast<DSMethod>(cboDepthScaling->currentData().toInt());
    fs.refStart.setDate(deRefDate->date());
    fs.refAppRate = sbRefAppRate->value();
    fs.refDepth = sbRefDepth->value();
    fs.refVL = sbRefVL->value() / 100.;
    fs.maxVL = sbMaxVL->value() / 100.;
    fs.warmSeasonStart = deStartDate->date();
    fs.warmSeasonEnd = deEndDate->date();
    fs.warmSeasonSF = sbScaleFactor->value();
    fs.amplitude = sbAmplitude->value();
    fs.centerAmplitude = sbCenterAmplitude->value();
    fs.phase = sbPhase->value();
    fs.wavelength = sbWavelength->value();

    sgPtr->fluxScaling = fs;
}

void FluxProfilePage::load()
{
    refModel->removeRows(0, refModel->rowCount());
    for (const auto& xy : sgPtr->refFlux) {
        int currentRow = refModel->rowCount();
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(xy.first, Qt::DisplayRole);
        item1->setData(xy.second, Qt::DisplayRole);
        refModel->setItem(currentRow, 0, item0);
        refModel->setItem(currentRow, 1, item1);
    }

    using TSMethod = FluxScaling::TSMethod;
    using DSMethod = FluxScaling::DSMethod;
    FluxScaling fs = sgPtr->fluxScaling;

    deRefDate->setDateTime(fs.refStart);
    sbRefAppRate->setValue(fs.refAppRate);
    sbRefDepth->setValue(fs.refDepth);
    sbRefVL->setValue(fs.refVL * 100.);
    sbMaxVL->setValue(fs.maxVL * 100.);

    // Temporal Scaling
    int tsMethod = static_cast<int>(fs.tsMethod);
    int tsIndex = cboTemporalScaling->findData(tsMethod);
    cboTemporalScaling->setCurrentIndex(tsIndex);

    deStartDate->setDate(fs.warmSeasonStart);
    deEndDate->setDate(fs.warmSeasonEnd);
    sbScaleFactor->setValue(fs.warmSeasonSF);
    sbAmplitude->setValue(fs.amplitude);
    sbCenterAmplitude->setValue(fs.centerAmplitude);
    sbPhase->setValue(fs.phase);
    sbWavelength->setValue(fs.wavelength);

    // Depth Scaling
    int dsMethod = static_cast<int>(fs.dsMethod);
    int dsIndex = cboDepthScaling->findData(dsMethod);
    cboDepthScaling->setCurrentIndex(dsIndex);
}

void FluxProfilePage::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);

    for (int i = first; i <= last; ++i) {
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData("1", Qt::DisplayRole);
        item1->setData("0", Qt::DisplayRole);
        refModel->setItem(i, 0, item0);
        refModel->setItem(i, 1, item1);
    }

    refTable->scrollToBottom();
    refTable->selectLastRow();
    refTable->setFocus();
}

void FluxProfilePage::importFluxProfile()
{
    // Remove existing rows
    refModel->removeRows(0, refModel->rowCount());

    QString csvfile = refEditor->importFile();
    int hour;
    double flux;
    std::vector<std::pair<int, double> > profile;

    io::CSVReader<2> in(csvfile.toStdString());

    while (true) {
        try {
            if (in.read_row(hour, flux))
                profile.push_back(std::make_pair(hour, flux));
            else
                break;
        } catch (const std::exception &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
            break;
        }
    }

    for (const auto &xy : profile) {
        int currentRow = refModel->rowCount();
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(xy.first, Qt::DisplayRole);
        item1->setData(xy.second, Qt::DisplayRole);
        refModel->setItem(currentRow, 0, item0);
        refModel->setItem(currentRow, 1, item1);
    }
}

void FluxProfilePage::calcPhase()
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

void FluxProfilePage::plotTemporalScaling()
{
    using TSMethod = FluxScaling::TSMethod;
    FluxScaling fs;

    fs.tsMethod = static_cast<TSMethod>(cboTemporalScaling->currentData().toInt());
    fs.refStart.setDate(deRefDate->date());
    fs.refDepth = sbRefDepth->value();
    fs.refVL = sbRefVL->value() / 100.;
    fs.maxVL = sbMaxVL->value() / 100.;
    fs.warmSeasonStart = deStartDate->date();
    fs.warmSeasonEnd = deEndDate->date();
    fs.warmSeasonSF = sbScaleFactor->value();
    fs.amplitude = sbAmplitude->value();
    fs.centerAmplitude = sbCenterAmplitude->value();
    fs.phase = sbPhase->value();
    fs.wavelength = sbWavelength->value();

    if (!fs.refStart.isValid()) {
        QMessageBox::critical(this, "Calculation Error", "Field study reference date is invalid.");
        return;
    }

    QDateTime dt0 = QDateTime(QDate(fs.refStart.date().year(), 1, 1));
    QDateTime dt1 = QDateTime(dt0.addYears(1));

    dt0.setTimeSpec(Qt::UTC);
    dt1.setTimeSpec(Qt::UTC);

    // Plot Limits
    double x0 = QwtDate::toDouble(dt0);
    double x1 = QwtDate::toDouble(dt1);
    double y0 = 0.0;
    double y1 = 1.0;

    if (fs.tsMethod == TSMethod::Seasonal) {
        y0 = 1.0;
        y1 = fs.warmSeasonSF;
    }
    if (fs.tsMethod == TSMethod::Sinusoidal) {
        y0 = fs.centerAmplitude - fs.amplitude;
        y1 = fs.centerAmplitude + fs.amplitude;
    }

    QVector<QPointF> series;
    int n = fs.refStart.date().daysInYear(); // 365 or 366
    series.reserve(n);

    for (int i = 0; i < n; ++i) {
        QDateTime x = dt0.addDays(i);
        double y = fs.timeScaleFactor(x);
        double xf = QwtDate::toDouble(x);
        QPointF p(xf, y);
        series.push_back(p);
    }

    StandardPlot *plot = new StandardPlot;
    plot->setCurveTracker(true);
    QwtPlotCurve *sfCurve = new QwtPlotCurve;
    sfCurve->setPen(Qt::blue, 1.5);
    if (fs.tsMethod == TSMethod::Seasonal)
        sfCurve->setStyle(QwtPlotCurve::Steps);
    else
        sfCurve->setStyle(QwtPlotCurve::Lines);
    sfCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    sfCurve->setSamples(series);
    sfCurve->attach(plot);

    // Use reference date on X-axis
    QwtDateScaleDraw *scaleDraw = new QwtDateScaleDraw;
    scaleDraw->setDateFormat(QwtDate::Month, QString("MMM"));
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    QwtDateScaleEngine *scaleEngine = new QwtDateScaleEngine;
    plot->setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    plot->setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    // Set plot limits
    plot->setAxisScale(QwtPlot::xBottom, x0, x1);
    plot->setAxisScale(QwtPlot::yLeft, y0, y1);

    // Add reference date
    double xref = QwtDate::toDouble(QDateTime(fs.refStart.date()));
    plot->addRefVLine(xref, 0, " Field Study");

    // Layout
    BackgroundFrame *plotFrame = new BackgroundFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);

    QDialog *dialog = new QDialog(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotFrame);
    mainLayout->addWidget(buttonBox);

    dialog->setWindowTitle("Time Scale Factor");
    dialog->setWindowFlag(Qt::Tool);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setLayout(mainLayout);
    dialog->exec();
}

void FluxProfilePage::plotDepthScaling()
{
    using DSMethod = FluxScaling::DSMethod;
    FluxScaling fs;

    fs.dsMethod = static_cast<DSMethod>(cboDepthScaling->currentData().toInt());
    fs.refStart.setDate(deRefDate->date());
    fs.refDepth = sbRefDepth->value();
    fs.refVL = sbRefVL->value() / 100.;
    fs.maxVL = sbMaxVL->value() / 100.;
    fs.warmSeasonStart = deStartDate->date();
    fs.warmSeasonEnd = deEndDate->date();
    fs.warmSeasonSF = sbScaleFactor->value();
    fs.amplitude = sbAmplitude->value();
    fs.centerAmplitude = sbCenterAmplitude->value();
    fs.phase = sbPhase->value();
    fs.wavelength = sbWavelength->value();

    double x0 = 0.0;
    double x1 = fs.refDepth * 1.5;
    double y0 = fs.depthScaleFactor(x1);
    double y1 = fs.depthScaleFactor(x0);

    // Prevent setting axis limits with y0 = y1
    if (std::abs(y1 - y0) < std::numeric_limits<double>::epsilon()) {
        y0 = 0.0;
        y1 = 1.0;
    }

    double step = 0.01;

    QVector<QPointF> series;
    for (double x = x0; x < x1; x += step) {
        double y = fs.depthScaleFactor(x);
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
    plot->addRefVLine(fs.refDepth, 0, " Field Study");

    // Layout
    BackgroundFrame *plotFrame = new BackgroundFrame;
    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addWidget(plot);
    plotFrame->setLayout(plotLayout);

    QDialog *dialog = new QDialog(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(plotFrame);
    mainLayout->addWidget(buttonBox);

    dialog->setWindowTitle("Depth Scale Factor");
    dialog->setWindowFlag(Qt::Tool);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setLayout(mainLayout);
    dialog->exec();
}


/****************************************************************************
** Buffer Zone
****************************************************************************/

BufferZonePage::BufferZonePage(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    sbBuffer = new QDoubleSpinBox;
    sbBuffer->setRange(0, 99000); // 99km
    sbBuffer->setValue(0);
    sbBuffer->setDecimals(2);

    sbReentry = new QSpinBox;
    sbReentry->setMinimum(1);
    sbReentry->setMaximum(1000);
    sbReentry->setSingleStep(1);
    sbReentry->setValue(1);

    zoneModel = new QStandardItemModel;
    zoneModel->setColumnCount(2);
    zoneModel->setHorizontalHeaderLabels(QStringList{"Distance (m)", "Reentry (hr)"});

    zoneTable = new StandardTableView;
    zoneTable->setModel(zoneModel);
    zoneTable->setDoubleLineEditForColumn(0, 0, 99000, 7, false);
    zoneTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    zoneTable->setSelectionMode(QAbstractItemView::ContiguousSelection);

    zoneEditor = new StandardTableEditor(QBoxLayout::LeftToRight);

    // Layout
    GridLayout *zoneInputLayout = new GridLayout;
    zoneInputLayout->setColumnStretch(0, 1);
    zoneInputLayout->setMargin(0);
    zoneInputLayout->addWidget(new QLabel(tr("Buffer zone distance (m):")), 0, 0);
    zoneInputLayout->addWidget(sbBuffer, 0, 1);
    zoneInputLayout->addWidget(new QLabel(tr("Reentry period (hr):")), 1, 0);
    zoneInputLayout->addWidget(sbReentry, 1, 1);
    zoneInputLayout->setRowMinimumHeight(2, 5);
    zoneInputLayout->addWidget(zoneEditor, 3, 1);

    QVBoxLayout *zoneTableLayout = new QVBoxLayout;
    zoneTableLayout->setMargin(0);
    zoneTableLayout->addWidget(zoneTable);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(zoneInputLayout);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(zoneTableLayout);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void BufferZonePage::init()
{
    connect(zoneEditor->btnAdd,    &QPushButton::clicked, this, &BufferZonePage::onAddZoneClicked);
    connect(zoneEditor->btnRemove, &QPushButton::clicked, this, &BufferZonePage::onRemoveZoneClicked);

    zoneEditor->init(zoneTable);
    zoneEditor->disconnectActions();

    load();
}

void BufferZonePage::save()
{
    sgPtr->zones.clear();
    sgPtr->zones = zones;
}

void BufferZonePage::load()
{
    zones.clear();
    zones = sgPtr->zones;

    zoneModel->removeRows(0, zoneModel->rowCount());
    for (auto const& z : zones)
    {
        // Insert item at front.
        int currentRow = zoneModel->rowCount();
        zoneModel->insertRow(currentRow);
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(z.first, Qt::DisplayRole);
        item1->setData(z.second, Qt::DisplayRole);
        zoneModel->setItem(currentRow, 0, item0);
        zoneModel->setItem(currentRow, 1, item1);
    }

    //zoneModel->sort(0, Qt::AscendingOrder);
}

void BufferZonePage::onAddZoneClicked()
{
    double buffer = sbBuffer->value();
    int reentry = sbReentry->value();

    // Check if a buffer zone with the current distance already exists.
    auto it = std::find_if(std::begin(zones), std::end(zones),
        [&](const std::pair<double, int> &z) { return z.first == buffer; });

    if (it != std::end(zones)) {
        // Buffer zone exists; update it.
        (*it).second = reentry;

        int position = std::distance(std::begin(zones), it);
        QStandardItem *item = zoneModel->item(position, 1);
        item->setData(reentry, Qt::DisplayRole);
    }
    else {
        // Add a new buffer zone.
        zones.push_back(std::make_pair(buffer, reentry));

        // insert item at back.
        int currentRow = zoneModel->rowCount();
        QStandardItem *item0 = new QStandardItem;
        QStandardItem *item1 = new QStandardItem;
        item0->setData(buffer, Qt::DisplayRole);
        item1->setData(reentry, Qt::DisplayRole);
        zoneModel->setItem(currentRow, 0, item0);
        zoneModel->setItem(currentRow, 1, item1);
    }
}

void BufferZonePage::onRemoveZoneClicked()
{
    QModelIndexList selection = zoneTable->selectionModel()->selectedRows();

    int count = selection.count();
    int row = selection.first().row();

    auto it = zones.begin() + row;
    for (int i = 0; i < count; ++i)
        it = zones.erase(it);

    zoneTable->removeSelectedRows();
}


/****************************************************************************
** Fields
****************************************************************************/

const QMap<SourceType, QString> FieldPage::sourceTypeMap = {
    {SourceType::POINT,    "POINT"},
    {SourceType::POINTCAP, "POINTCAP"},
    {SourceType::POINTHOR, "POINTHOR"},
    {SourceType::VOLUME,   "VOLUME"},
    {SourceType::AREA,     "AREA"},
    {SourceType::AREAPOLY, "AREAPOLY"},
    {SourceType::AREACIRC, "AREACIRC"},
    {SourceType::OPENPIT,  "OPENPIT"},
    {SourceType::LINE,     "LINE"},
    {SourceType::BUOYLINE, "BUOYLINE"}
};

FieldPage::FieldPage(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    btnAddSource = new QPushButton("Add");
    btnRemoveSource = new QPushButton("Remove");
    btnImport = new QPushButton("Import...");

    btnAddSourceMenu = new QMenu(this);
    const QIcon icoArea = QIcon(":/images/Rectangle_16x.png");
    const QIcon icoAreaCirc = QIcon(":/images/Circle_16x.png");
    const QIcon icoAreaPoly = QIcon(":/images/Polygon_16x.png");
    actAddArea = new QAction(icoArea, "Area");
    actAddAreaCirc = new QAction(icoAreaCirc, "Circular");
    actAddAreaPoly = new QAction(icoAreaPoly, "Polygon");
    btnAddSourceMenu->addAction(actAddArea);
    btnAddSourceMenu->addAction(actAddAreaCirc);
    btnAddSourceMenu->addAction(actAddAreaPoly);
    btnAddSource->setMenu(btnAddSourceMenu);

    sourceTable = new StandardTableView;
    sourceTable->setSelectionMode(QAbstractItemView::ContiguousSelection);
    sourceTable->verticalHeader()->setVisible(true);
    sourceTable->horizontalHeader()->setVisible(false);

    sourceEditor = new SourceEditor;
    sourceEditor->setVisible(false);

    // Layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(btnAddSource);
    buttonLayout->addWidget(btnRemoveSource);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(btnImport);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(sourceTable, 1);
    mainLayout->addWidget(sourceEditor, 0);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void FieldPage::init()
{
    // Defaults
    btnRemoveSource->setEnabled(false);

    // Initialize SourceModel
    sourceModel = new SourceModel(sgPtr, this);
    sourceTable->setModel(sourceModel);

    // Hide all columns except source ID.
    for (int i=1; i < sourceModel->columnCount(); ++i)
        sourceTable->setColumnHidden(i, true);

    // Connections
    connect(actAddArea,     &QAction::triggered, sourceModel, &SourceModel::addAreaSource);
    connect(actAddAreaCirc, &QAction::triggered, sourceModel, &SourceModel::addAreaCircSource);
    connect(actAddAreaPoly, &QAction::triggered, sourceModel, &SourceModel::addAreaPolySource);
    connect(btnRemoveSource, &QPushButton::clicked, this, &FieldPage::removeSelectedRows);
    connect(btnImport, &QPushButton::clicked, this, &FieldPage::import);

    connect(sourceTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FieldPage::onSelectionChanged);
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this, &FieldPage::onRowsInserted);
    connect(sourceModel, &QAbstractItemModel::rowsRemoved,  this, &FieldPage::onRowsRemoved);

    load();
}

void FieldPage::save()
{
    sourceModel->save();
}

void FieldPage::load()
{
    sourceModel->load();
}

void FieldPage::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    const QItemSelectionModel *selectionModel = sourceTable->selectionModel();

    if (!selectionModel)
        return;

    bool hasSelection = selectionModel->hasSelection();
    btnRemoveSource->setEnabled(hasSelection);

    // Map to source editor; only enabled if exactly one row selected
    QModelIndexList selectedRows = selectionModel->selectedRows();
    bool enableEditor = (hasSelection && selectedRows.count() == 1);

    // Update source editor
    if (enableEditor) {
        const QModelIndex &index = selectedRows.first();
        Source *sPtr = sourceModel->getSource(index);
        sourceEditor->setSource(sPtr);
        sourceEditor->setVisible(true);
    }
    else {
        sourceEditor->setVisible(false);
    }
}

void FieldPage::removeSelectedRows()
{
    const QItemSelectionModel *selectionModel = sourceTable->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();
    int row = selectedRows.first().row();
    int count = selectedRows.count();
    sourceModel->removeRows(row, count);
}

void FieldPage::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    sourceTable->scrollToBottom();
    sourceTable->selectLastRow();
    sourceTable->setFocus();
}

void FieldPage::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    return;
}

void FieldPage::import()
{
    QSettings settings;
    QString currentDir = settings.value("DefaultDirectory", QDir::rootPath()).toString();
    QString fileFilter = "Runstream File (*.inp *.dat)";
    QString defaultDirectory = QDir::rootPath();
    const QString file = QFileDialog::getOpenFileName(this,
                         tr("Import Geometry"),
                         currentDir,
                         fileFilter);

    if (!file.isEmpty())
        sourceModel->import(file);
}

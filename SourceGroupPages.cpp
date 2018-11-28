#include <QtWidgets>

#include "SourceGroupPages.h"

#include <boost/log/trivial.hpp>

#include <csv/csv.h>

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

    // NOTE: Update SourceTable delegates with any format changes.
    // FIXME: Formats should be stored in one place.

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
    sgPtr->appStart = mcAppStart->getDistribution();
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
    mcAppStart->setDistribution(sgPtr->appStart);
    mcAppRate->setDistribution(sgPtr->appRate);
    mcIncorpDepth->setDistribution(sgPtr->incorpDepth);
}


/****************************************************************************
** Deposition
****************************************************************************/

DepositionPage::DepositionPage(SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg)
{
    mcAirDiffusion = new MonteCarloLineEdit;
    mcAirDiffusion->setRange(0.000001, 0.5);
    mcAirDiffusion->setDecimals(8);

    mcWaterDiffusion = new MonteCarloLineEdit;
    mcWaterDiffusion->setRange(0.000001, 0.5);
    mcWaterDiffusion->setDecimals(8);

    mcCuticularResistance = new MonteCarloLineEdit;
    mcCuticularResistance->setRange(0, 1000000);
    mcCuticularResistance->setDecimals(8);

    mcHenryConstant = new MonteCarloLineEdit;
    mcHenryConstant->setRange(0, 10000);
    mcHenryConstant->setDecimals(8);

    lblDepoNotEnabled = new StatusLabel;
    lblDepoNotEnabled->setSeverity(3);
    lblDepoNotEnabled->setText("Deposition must be enabled for these parameters to take effect.");

    lblDepoUserVelocity = new StatusLabel;
    lblDepoUserVelocity->setSeverity(3);
    lblDepoUserVelocity->setText("Custom gas dry deposition velocity must be disabled for these parameters to take effect.");

    // Layout
    GridLayout *layout1 = new GridLayout;
    layout1->addWidget(new QLabel(QLatin1String("Air diffusion (cm\xb2/sec):")), 0, 0);
    layout1->addWidget(mcAirDiffusion, 0, 1);
    layout1->addWidget(new QLabel(QLatin1String("Water diffusion (cm\xb2/sec):")), 1, 0);
    layout1->addWidget(mcWaterDiffusion, 1, 1);
    layout1->addWidget(new QLabel(QLatin1String("Cuticular resistance (s/cm):")), 2, 0);
    layout1->addWidget(mcCuticularResistance, 2, 1);
    layout1->addWidget(new QLabel(QLatin1String("Henry's law constant (Pa-m\xb3/mol):")), 3, 0);
    layout1->addWidget(mcHenryConstant, 3, 1);

    QGroupBox *gbMonteCarlo = new QGroupBox("Monte Carlo Parameters");
    gbMonteCarlo->setFlat(true);
    gbMonteCarlo->setLayout(layout1);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gbMonteCarlo);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(lblDepoNotEnabled);
    mainLayout->addWidget(lblDepoUserVelocity);
    mainLayout->addStretch(1);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void DepositionPage::init()
{
    lblDepoNotEnabled->setVisible(false);
    lblDepoUserVelocity->setVisible(false);

    load();
}

void DepositionPage::warnDepoNotEnabled()
{
    lblDepoNotEnabled->setVisible(true);
    lblDepoUserVelocity->setVisible(false);
}

void DepositionPage::warnUserVelocity()
{
    lblDepoNotEnabled->setVisible(false);
    lblDepoUserVelocity->setVisible(true);
}

void DepositionPage::save()
{
    sgPtr->airDiffusion = mcAirDiffusion->getDistribution();
    sgPtr->waterDiffusion = mcWaterDiffusion->getDistribution();
    sgPtr->cuticularResistance = mcCuticularResistance->getDistribution();
    sgPtr->henryConstant = mcHenryConstant->getDistribution();
}

void DepositionPage::load()
{
    mcAirDiffusion->setDistribution(sgPtr->airDiffusion);
    mcWaterDiffusion->setDistribution(sgPtr->waterDiffusion);
    mcCuticularResistance->setDistribution(sgPtr->cuticularResistance);
    mcHenryConstant->setDistribution(sgPtr->henryConstant);
}

/****************************************************************************
** Flux Profile
****************************************************************************/

FluxProfilePage::FluxProfilePage(Scenario *s, SourceGroup *sg, QWidget *parent)
    : QWidget(parent), sgPtr(sg), sPtr(s)
{
    model = new FluxProfileModel;
    model->load(sPtr->fluxProfiles, false);

    editor = new SamplingDistributionEditor(model);
    editor->setColumnHidden(1);
    editor->setColumnHidden(2);
    editor->setColumnHidden(3);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editor);
    mainLayout->addStretch(1);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void FluxProfilePage::init()
{
    load();
}

void FluxProfilePage::save()
{
    // Probabilities will be normalized on save.
    editor->normalize();

    sgPtr->fluxProfile.data.clear();

    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        auto fp = model->fluxProfileFromIndex(index);
        double probability = editor->getProbability(row);
        sgPtr->fluxProfile.data[fp] = probability;
    }
}

void FluxProfilePage::load()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        auto fp = model->fluxProfileFromIndex(index);

        if (sgPtr->fluxProfile.data.count(fp) > 0) {
            double probability = sgPtr->fluxProfile.data.at(fp);
            editor->setProbability(row, probability);
        }
    }
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

    zoneEditor = new StandardTableEditor(Qt::Vertical);

    // Layout
    GridLayout *zoneInputLayout = new GridLayout;
    zoneInputLayout->setColumnStretch(0, 1);
    zoneInputLayout->setMargin(0);
    zoneInputLayout->addWidget(new QLabel(tr("Buffer zone distance (m):")), 0, 0);
    zoneInputLayout->addWidget(sbBuffer, 0, 1);
    zoneInputLayout->addWidget(new QLabel(tr("Reentry period (hr):")), 1, 0);
    zoneInputLayout->addWidget(sbReentry, 1, 1);

    QHBoxLayout *zoneTableLayout = new QHBoxLayout;
    zoneTableLayout->setMargin(0);
    zoneTableLayout->addWidget(zoneTable);
    zoneTableLayout->addWidget(zoneEditor);

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
    disconnect(zoneEditor->btnAdd,    &QPushButton::clicked, zoneEditor, &StandardTableEditor::onAddItemClicked);
    disconnect(zoneEditor->btnRemove, &QPushButton::clicked, zoneEditor, &StandardTableEditor::onRemoveItemClicked);

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
    for (const auto& z : zones)
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


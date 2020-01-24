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
#include "ScenarioPages.h"
#include "MetFileParser.h"
#include "MetFileInfoDialog.h"
#include "MagneticDeclinationDialog.h"

#include "ctk/ctkCollapsibleGroupBox.h"
#include "widgets/GridLayout.h"
#include "widgets/BackgroundFrame.h"

/****************************************************************************
** General
****************************************************************************/

GeneralPage::GeneralPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    cboFumigant = new QComboBox;
    cboFumigant->setObjectName("Fumigant");

    leDecayCoefficient = new DoubleLineEdit;
    leDecayCoefficient->setObjectName("DecayCoefficient");

    periodEditor = new ListEditor;
    periodEditor->setObjectName("AveragingPeriod");
    periodEditor->setValidator(1, 24, 0);
    periodEditor->addValue(1);
    periodEditor->addValue(24);
    periodEditor->setComboBoxItems(QStringList{"1","2","3","4","6","8","12","24"});
    periodEditor->setEditable(false);

    // Layouts
    GridLayout *mainLayout = new GridLayout;
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->addWidget(new QLabel(tr("Fumigant type:")), 0, 0);
    mainLayout->addWidget(cboFumigant, 0, 1);
    mainLayout->addWidget(new QLabel(tr("Decay coefficient (1/sec):")), 1, 0);
    mainLayout->addWidget(leDecayCoefficient, 1, 1);
    mainLayout->addWidget(new QLabel(tr("Averaging periods (hr): ")), 2, 0);
    mainLayout->addWidget(periodEditor, 2, 1, 2, 1);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);

    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void GeneralPage::init()
{
    for (const auto& item : scenario->chemicalMap) {
        QString text = QString::fromStdString(item.second);
        cboFumigant->addItem(text, item.first); // ID
    }

    load();
}

void GeneralPage::save()
{
    scenario->fumigantId = cboFumigant->currentData().toInt();
    scenario->decayCoefficient = leDecayCoefficient->text().toDouble();

    scenario->averagingPeriods.clear();
    std::vector<double> periods = periodEditor->values();
    for (const double period : periods) {
        int value = static_cast<int>(period);
        scenario->averagingPeriods.push_back(value);
    }
}

void GeneralPage::load()
{
    int fumigantId = static_cast<int>(scenario->fumigantId);
    int fumigantIndex = cboFumigant->findData(fumigantId);
    cboFumigant->setCurrentIndex(fumigantIndex);
    leDecayCoefficient->setText(QString::number(scenario->decayCoefficient));

    periodEditor->clearValues();
    for (const int value : scenario->averagingPeriods) {
        double period = static_cast<double>(value);
        periodEditor->addValue(period);
    }
}

/****************************************************************************
** Projection
****************************************************************************/

ProjectionPage::ProjectionPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    editor = new ProjectionEditor;

    // Layouts
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->addWidget(editor);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);

    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void ProjectionPage::init()
{
    load();
}

void ProjectionPage::save()
{
    scenario->conversionCode = editor->conversionCode();
    scenario->hUnitsCode = editor->hUnitsCode();
    scenario->hDatumCode = editor->hDatumCode();
    scenario->vUnitsCode = editor->vUnitsCode();
    scenario->vDatumCode = editor->vDatumCode();
}

void ProjectionPage::load()
{
    editor->setConversionCode(scenario->conversionCode);
    editor->setHUnitsCode(scenario->hUnitsCode);
    editor->setHDatumCode(scenario->hDatumCode);
    editor->setVUnitsCode(scenario->vUnitsCode);
    editor->setVDatumCode(scenario->vDatumCode);
}

/****************************************************************************
** Meteorological Data
****************************************************************************/

MetDataPage::MetDataPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    leSurfaceDataFile = new QLineEdit;
    leSurfaceDataFile->setObjectName("SurfaceDataFile");
    leSurfaceDataFile->setMaxLength(260); // MAX_PATH
    btnSurfaceDataFile = new QToolButton;
    btnSurfaceDataFile->setText("...");

    leUpperAirDataFile = new QLineEdit;
    leUpperAirDataFile->setObjectName("UpperAirDataFile");
    leUpperAirDataFile->setMaxLength(260); // MAX_PATH
    btnUpperAirDataFile = new QToolButton;
    btnUpperAirDataFile->setText("...");

    sbAnemometerHeight = new QDoubleSpinBox;
    sbAnemometerHeight->setObjectName("AnemometerHeight");
    sbAnemometerHeight->setMinimum(0.1);
    sbAnemometerHeight->setMaximum(1000);
    sbAnemometerHeight->setSingleStep(0.1);
    sbAnemometerHeight->setDecimals(1);

    sbWindRotation = new QDoubleSpinBox;
    sbWindRotation->setObjectName("WindRotation");
    sbWindRotation->setMinimum(-180.0);
    sbWindRotation->setMaximum(180.0);
    sbWindRotation->setSingleStep(0.1);
    sbWindRotation->setDecimals(1);

    const QIcon icoCalc = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_CalculateButton));
    btnDeclinationCalc = new QToolButton;
    btnDeclinationCalc->setIcon(icoCalc);
    btnDeclinationCalc->setToolButtonStyle(Qt::ToolButtonIconOnly);

    leSurfaceStationId = new ReadOnlyLineEdit;
    leUpperAirStationId = new ReadOnlyLineEdit;

    lwIntervals = new QListWidget;
    lwIntervals->setSelectionMode(QAbstractItemView::NoSelection);
    lwIntervals->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    lwIntervals->setUniformItemSizes(true);

    btnDiagnostics = new QPushButton("Diagnostics...");
    btnUpdate = new QPushButton("Update");

    // Layouts
    QLatin1String rotationText = QLatin1String("Wind rotation (\u00b0CW):");
    GridLayout *inputLayout = new GridLayout;
    inputLayout->setColumnStretch(0, 0);
    inputLayout->setColumnStretch(1, 1);
    inputLayout->setColumnStretch(2, 0);
    inputLayout->addWidget(new QLabel(tr("Surface data file:")),        0, 0);
    inputLayout->addWidget(leSurfaceDataFile,                           0, 1);
    inputLayout->addWidget(btnSurfaceDataFile,                          0, 2);
    inputLayout->addWidget(new QLabel(tr("Upper air data file:")),      1, 0);
    inputLayout->addWidget(leUpperAirDataFile,                          1, 1);
    inputLayout->addWidget(btnUpperAirDataFile,                         1, 2);
    inputLayout->addWidget(new QLabel(tr("Anemometer height (m):")),    2, 0);
    inputLayout->addWidget(sbAnemometerHeight,                          2, 1, 1, 2);
    inputLayout->addWidget(new QLabel(rotationText),                    3, 0);
    inputLayout->addWidget(sbWindRotation,                              3, 1);
    inputLayout->addWidget(btnDeclinationCalc,                          3, 2);

    // File Information
    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->addSpacing(5);
    statsLayout->addWidget(new QLabel(tr("Time intervals:")));
    statsLayout->addWidget(lwIntervals);

    GridLayout *infoLayout = new GridLayout;
    infoLayout->setColumnStretch(0, 0);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->addWidget(new QLabel(tr("Surface station ID:")),       0, 0);
    infoLayout->addWidget(leSurfaceStationId,                          0, 1);
    infoLayout->addWidget(new QLabel(tr("Upper air station ID:")),     1, 0);
    infoLayout->addWidget(leUpperAirStationId,                         1, 1);
    infoLayout->addLayout(statsLayout,                                 2, 0, 1, 2);

    QGroupBox *gbFileInfo = new QGroupBox("File Information");
    gbFileInfo->setLayout(infoLayout);

    QMargins infoMargins = infoLayout->contentsMargins();
    int minWidth = infoLayout->columnMinimumWidth(0) - infoMargins.left();
    infoLayout->setColumnMinimumWidth(0, minWidth);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(btnUpdate);
    buttonLayout->addWidget(btnDiagnostics);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(inputLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(gbFileInfo);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void MetDataPage::init()
{
    // Reset height for QListWidget after draw.
    lwIntervals->addItem("");
    lwIntervals->setFixedHeight(4 * lwIntervals->sizeHintForRow(0) + 2 * lwIntervals->frameWidth());

    connect(btnSurfaceDataFile, &QToolButton::clicked, this, &MetDataPage::browseMetDataFile);
    connect(btnUpperAirDataFile, &QToolButton::clicked, this, &MetDataPage::browseMetDataFile);
    connect(btnDeclinationCalc, &QToolButton::clicked, this, &MetDataPage::showDeclinationCalc);
    connect(btnDiagnostics, &QPushButton::clicked, this, &MetDataPage::showInfoDialog);
    connect(btnUpdate, &QPushButton::clicked, this, &MetDataPage::update);

    load();
}

void MetDataPage::save()
{
    scenario->surfaceFile = leSurfaceDataFile->text().toStdString();
    scenario->upperAirFile = leUpperAirDataFile->text().toStdString();
    scenario->anemometerHeight = sbAnemometerHeight->value();
    scenario->windRotation = sbWindRotation->value();

    // Input:   surfaceFile
    // Output:  minTime, maxTime, surfaceId, upperAirId
    scenario->resetSurfaceFileInfo();
}

void MetDataPage::load()
{
    leSurfaceDataFile->setText(QString::fromStdString(scenario->surfaceFile));
    leUpperAirDataFile->setText(QString::fromStdString(scenario->upperAirFile));
    sbAnemometerHeight->setValue(scenario->anemometerHeight);
    sbWindRotation->setValue(scenario->windRotation);
    leSurfaceStationId->setText(QString::fromStdString(scenario->surfaceId));
    leUpperAirStationId->setText(QString::fromStdString(scenario->upperAirId));

    lwIntervals->clear();
    for (const std::string& i : scenario->sfInfo.intervals) {
        QString is = QString::fromStdString(i);
        lwIntervals->addItem(is);
    }
}


void MetDataPage::browseMetDataFile()
{
    QSettings settings;
    QLineEdit *metFileLineEdit;
    QString metFileCurrentPath;
    QString metFileFilter;
    QString defaultDirectory;
    QString settingsKey = "DefaultMetFileDirectory";

    // determine current path and filter from sender
    QObject* obj = sender();
    if (obj == btnSurfaceDataFile) {
        metFileLineEdit = leSurfaceDataFile;
        metFileFilter = "AERMET Surface Data (*.sfc)";
    }
    else if (obj == btnUpperAirDataFile) {
        metFileLineEdit = leUpperAirDataFile;
        metFileFilter = "AERMET Profile Data (*.pfl)";
    }
    else {
        return;
    }

    // set default directory from the second data file, if available
    // otherwise, get default from QSettings
    metFileCurrentPath = metFileLineEdit->text();
    QFileInfo fi(metFileCurrentPath);
    if (fi.exists())
        defaultDirectory = fi.canonicalPath();
    else
        defaultDirectory = settings.value(settingsKey, QDir::currentPath()).toString();

    // execute the dialog
    const QString metFile = QFileDialog::getOpenFileName(this,
                            tr("Choose File"),
                            defaultDirectory,
                            metFileFilter);

    if (!metFile.isEmpty()) {
        // save selection as new default
        QFileInfo mfi(metFile);
        QString dir = mfi.absoluteDir().absolutePath();
        settings.setValue(settingsKey, dir);

        // update line edit
        metFileLineEdit->setText(metFile);
    }
}

void MetDataPage::showInfoDialog()
{
    QString surfaceFile = leSurfaceDataFile->text();

    MetFileParser parser(surfaceFile);
    std::shared_ptr<SurfaceData> sd = parser.getSurfaceData();

    if (sd == nullptr)
        return;

    MetFileInfoDialog dialog(sd, this);
    dialog.exec();
}

void MetDataPage::showDeclinationCalc()
{
    // Declination is positive when magnetic north is east of true north,
    // and negative when it is to the west. WDROTATE is *subtracted* from
    // wind direction.
    //
    // WDREF = WD - ROTANG (CW) --> Final rotation is counter-clockwise
    // WDREF = WD - Declination

    MagneticDeclinationDialog *dialog = new MagneticDeclinationDialog(this);
    connect(dialog, &MagneticDeclinationDialog::declinationUpdated, [&](double value) {
        sbWindRotation->setValue(value);
    });
    dialog->exec();
}

void MetDataPage::update()
{
    QString surfaceFile = leSurfaceDataFile->text();

    MetFileParser parser(surfaceFile);
    SurfaceInfo sfInfo = parser.getSurfaceInfo();

    leSurfaceStationId->setText(QString::fromStdString(sfInfo.sfloc));
    leUpperAirStationId->setText(QString::fromStdString(sfInfo.ualoc));

    lwIntervals->clear();
    for (const std::string& i : sfInfo.intervals) {
        QString is = QString::fromStdString(i);
        lwIntervals->addItem(is);
    }
}

/****************************************************************************
** Flux Profiles
****************************************************************************/

FluxProfilesPage::FluxProfilesPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    model = new FluxProfileModel(this);

    table = new StandardTableView;
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setModel(model);

    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);

    int startingWidth = table->font().pointSize();
    table->setColumnWidth(1, startingWidth * 8);
    table->setColumnWidth(2, startingWidth * 18);
    table->setColumnWidth(3, startingWidth * 18);

    StandardTableEditor::StandardButtons editorOpts = StandardTableEditor::All & ~StandardTableEditor::Import;
    editor = new StandardTableEditor(Qt::Vertical, editorOpts);
    editor->init(table);

    connect(editor, &StandardTableEditor::moveRequested, model, &FluxProfileModel::moveRows);
    connect(editor, &StandardTableEditor::editRequested, this, &FluxProfilesPage::editFluxProfile);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(table);
    mainLayout->addWidget(editor);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->addWidget(frame);
    frameLayout->setMargin(0);

    setLayout(frameLayout);
    init();
}

void FluxProfilesPage::init()
{
    load();
}

void FluxProfilesPage::save()
{
    model->save(scenario->fluxProfiles);
}

void FluxProfilesPage::load()
{
    model->load(scenario->fluxProfiles);
}

void FluxProfilesPage::editFluxProfile(const QModelIndex& index)
{
    model->showEditor(index, this);
}

/****************************************************************************
** Dispersion Model
****************************************************************************/

DispersionPage::DispersionPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    // AERMOD Group 1: Non-DFAULT
    chkFlat = new QCheckBox("Assume flat terrain (FLAT)");
    chkFastArea = new QCheckBox("Optimize model runtime for area sources (FASTAREA)");

    // AERMOD Group 2: ALPHA
    chkDryDplt = new QCheckBox("Enable dry depletion processes (DRYDPLT)");
    chkDryDeposition = new QCheckBox("Enable dry deposition (DDEP)");
    chkAreaDplt = new QCheckBox("Optimize plume depletion for AREA sources (AREADPLT)");
    chkGDVelocity = new QCheckBox("Custom gas dry deposition velocity (GASDEPVD):");
    chkWetDplt = new QCheckBox("Enable wet depletion processes (WETDPLT)");
    chkWetDeposition = new QCheckBox("Enable wet deposition (WDEP)");

    sbGDVelocity = new QDoubleSpinBox;
    sbGDVelocity->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbGDVelocity->setRange(0.001, 0.05);
    sbGDVelocity->setDecimals(3);
    sbGDVelocity->setSingleStep(0.001);
    sbGDVelocity->setSuffix(" m/s");

    QHBoxLayout *velocityLayout = new QHBoxLayout;
    velocityLayout->setContentsMargins(0, 0, 0, 0);
    velocityLayout->addWidget(chkGDVelocity);
    velocityLayout->addWidget(sbGDVelocity);
    velocityLayout->addStretch(1);

    // Low Wind Parameters
    //lblSVmin = new QLabel("Minimum \xcf\x83\xce\xbd:");
    //lblWSmin = new QLabel("Minimum wind speed:");
    //lblFRANmax = new QLabel("Maximum meander factor:");
    chkLowWind = new QCheckBox("Optimize model for low wind speeds (LOW_WIND)");
    lblSVmin = new QLabel("SVmin:");
    lblWSmin = new QLabel("WSmin:");
    lblFRANmax = new QLabel("FRANmax:");

    sbSVmin = new QDoubleSpinBox;
    sbSVmin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbSVmin->setRange(0.01, 1.0);
    sbSVmin->setDecimals(4);
    sbSVmin->setSingleStep(0.0001);
    sbSVmin->setSuffix(" m/s");

    sbWSmin = new QDoubleSpinBox;
    sbWSmin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbWSmin->setRange(0.01, 1.0);
    sbWSmin->setDecimals(4);
    sbWSmin->setSingleStep(0.0001);
    sbWSmin->setSuffix(" m/s");

    sbFRANmax = new QDoubleSpinBox;
    sbFRANmax->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbFRANmax->setRange(0.0, 1.0);
    sbFRANmax->setDecimals(4);
    sbFRANmax->setSingleStep(0.0001);

    // Non-DFAULT Layout
    ctkCollapsibleGroupBox *gbNonDefault = new ctkCollapsibleGroupBox("Non-DFAULT Options");
    gbNonDefault->setFlat(true);

    QGridLayout *gbNonDefaultLayout = new QGridLayout;
    gbNonDefaultLayout->setColumnMinimumWidth(0, 16);
    gbNonDefaultLayout->setColumnMinimumWidth(1, 16);
    gbNonDefaultLayout->setColumnStretch(0, 0);
    gbNonDefaultLayout->setColumnStretch(1, 0);
    gbNonDefaultLayout->addWidget(chkFlat,          0, 0, 1, 3);
    gbNonDefaultLayout->addWidget(chkFastArea,      1, 0, 1, 3);
    gbNonDefaultLayout->addWidget(chkDryDeposition, 2, 0, 1, 3);
    gbNonDefaultLayout->addWidget(chkDryDplt,       3, 1, 1, 2);
    gbNonDefaultLayout->addWidget(chkAreaDplt,      4, 2, 1, 1);
    gbNonDefaultLayout->addLayout(velocityLayout,   5, 1, 1, 2);
    gbNonDefaultLayout->addWidget(chkWetDeposition, 6, 0, 1, 3);
    gbNonDefaultLayout->addWidget(chkWetDplt,       7, 1, 1, 2);
    gbNonDefault->setLayout(gbNonDefaultLayout);

    // ALPHA Layout
    ctkCollapsibleGroupBox *gbAlpha = new ctkCollapsibleGroupBox("ALPHA Options");
    gbAlpha->setFlat(true);

    QGridLayout *gbAlphaLayout = new QGridLayout;
    gbAlphaLayout->setColumnStretch(5, 1);
    gbAlphaLayout->addWidget(chkLowWind, 0, 0, 1, 5);
    gbAlphaLayout->addWidget(lblSVmin,   1, 1, 1, 1);
    gbAlphaLayout->addWidget(sbSVmin,    1, 2, 1, 1);
    gbAlphaLayout->addWidget(lblWSmin,   2, 1, 1, 1);
    gbAlphaLayout->addWidget(sbWSmin,    2, 2, 1, 1);
    gbAlphaLayout->addWidget(lblFRANmax, 1, 3, 1, 1);
    gbAlphaLayout->addWidget(sbFRANmax,  1, 4, 1, 1);
    gbAlpha->setLayout(gbAlphaLayout);

    QVBoxLayout *aermodLayout = new QVBoxLayout;
    aermodLayout->addWidget(gbNonDefault);
    aermodLayout->addWidget(gbAlpha);
    aermodLayout->addStretch(1);

    // Tabs
    QWidget *aermodTab = new QWidget;
    aermodTab->setLayout(aermodLayout);

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(aermodTab, "AERMOD");

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->setMargin(0);

    setLayout(mainLayout);
    init();
}

void DispersionPage::resetState()
{
    // If DDEP is unchecked, advanced options should be unchecked.
    if (!chkDryDeposition->isChecked()) {
        chkGDVelocity->setChecked(false);
        chkAreaDplt->setChecked(false);
    }

    if (chkDryDeposition->isChecked()) {
        // DRYDPLT depends on DDEP
        chkDryDplt->setEnabled(true);

        // AREADPLT depends on DRYDPLT
        chkAreaDplt->setEnabled(chkDryDplt->isChecked());

        // GASDEPVD depends on DDEP
        // WDEP and GASDEPVD mutually exclusive
        if (chkGDVelocity->isChecked()) {
            chkWetDeposition->setChecked(false);
            chkWetDeposition->setEnabled(false);
        }
        else {
            chkWetDeposition->setEnabled(true);
        }
        if (chkWetDeposition->isChecked()) {
            chkGDVelocity->setChecked(false);
            chkGDVelocity->setEnabled(false);
        }
        else {
            chkGDVelocity->setEnabled(true);
        }
    }
    else {
        chkDryDplt->setEnabled(false);
        chkAreaDplt->setEnabled(false);
        chkGDVelocity->setEnabled(false);
        sbGDVelocity->setEnabled(false);
        chkWetDeposition->setEnabled(true);
    }

    sbGDVelocity->setEnabled(chkGDVelocity->isEnabled());

    // WETDPLT depends on WDEP
    chkWetDplt->setEnabled(chkWetDeposition->isChecked());

    // LOW_WIND parameters
    lblSVmin->setEnabled(chkLowWind->isChecked());
    lblWSmin->setEnabled(chkLowWind->isChecked());
    lblFRANmax->setEnabled(chkLowWind->isChecked());
    sbSVmin->setEnabled(chkLowWind->isChecked());
    sbWSmin->setEnabled(chkLowWind->isChecked());
    sbFRANmax->setEnabled(chkLowWind->isChecked());
}

void DispersionPage::init()
{
    load();

    resetState();

    connect(chkDryDeposition, &QCheckBox::toggled,
            this, &DispersionPage::resetState);
    connect(chkDryDplt, &QCheckBox::toggled,
            this, &DispersionPage::resetState);
    connect(chkWetDeposition, &QCheckBox::toggled,
            this, &DispersionPage::resetState);
    connect(chkGDVelocity, &QCheckBox::toggled,
            this, &DispersionPage::resetState);
    connect(chkLowWind, &QCheckBox::toggled,
            this, &DispersionPage::resetState);
}

void DispersionPage::save()
{
    scenario->aermodFlat = chkFlat->isChecked();
    scenario->aermodFastArea = chkFastArea->isChecked();
    scenario->aermodDryDeposition = chkDryDeposition->isChecked();
    scenario->aermodDryDplt = chkDryDplt->isChecked();
    scenario->aermodAreaDplt = chkAreaDplt->isChecked();
    scenario->aermodGDVelocityEnabled = chkGDVelocity->isChecked();
    scenario->aermodGDVelocity = sbGDVelocity->value();
    scenario->aermodWetDeposition = chkWetDeposition->isChecked();
    scenario->aermodWetDplt = chkWetDplt->isChecked();
    scenario->aermodLowWind = chkLowWind->isChecked();
    scenario->aermodSVmin = sbSVmin->value();
    scenario->aermodWSmin = sbWSmin->value();
    scenario->aermodFRANmax = sbFRANmax->value();
}

void DispersionPage::load()
{
    chkFlat->setChecked(scenario->aermodFlat);
    chkFastArea->setChecked(scenario->aermodFastArea);
    chkDryDeposition->setChecked(scenario->aermodDryDeposition);
    chkDryDplt->setChecked(scenario->aermodDryDplt);
    chkAreaDplt->setChecked(scenario->aermodAreaDplt);
    chkGDVelocity->setChecked(scenario->aermodGDVelocityEnabled);
    sbGDVelocity->setValue(scenario->aermodGDVelocity);
    chkWetDeposition->setChecked(scenario->aermodWetDeposition);
    chkWetDplt->setChecked(scenario->aermodWetDplt);
    chkLowWind->setChecked(scenario->aermodLowWind);
    sbSVmin->setValue(scenario->aermodSVmin);
    sbWSmin->setValue(scenario->aermodWSmin);
    sbFRANmax->setValue(scenario->aermodFRANmax);
}

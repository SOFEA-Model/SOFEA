#include <QtWidgets>

#include "ScenarioPages.h"
#include "MetFileParser.h"
#include "MetFileInfoDialog.h"
#include "FluxProfileDialog.h"

/****************************************************************************
** General
****************************************************************************/

GeneralPage::GeneralPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    cboFumigant = new QComboBox;

    leDecayCoefficient = new DoubleLineEdit;

    sbFlagpoleHeight = new QDoubleSpinBox;
    sbFlagpoleHeight->setRange(0, 1000);
    sbFlagpoleHeight->setDecimals(2);

    periodEditor = new ListEditor;
    periodEditor->setValidator(1, 24, 0);
    periodEditor->addValue(1);
    periodEditor->addValue(24);
    periodEditor->resetLayout();
    periodEditor->setComboBoxItems(QStringList{"1","2","3","4","6","8","12","24"});
    periodEditor->setEditable(false);

    // Layouts
    GridLayout *layout1 = new GridLayout;
    layout1->addWidget(new QLabel(tr("Fumigant type:")), 0, 0);
    layout1->addWidget(cboFumigant, 0, 1);
    layout1->addWidget(new QLabel(tr("Decay coefficient (1/sec):")), 1, 0);
    layout1->addWidget(leDecayCoefficient, 1, 1);
    layout1->addWidget(new QLabel(tr("Default receptor height (m):")), 2, 0);
    layout1->addWidget(sbFlagpoleHeight, 2, 1);
    layout1->addWidget(new QLabel(tr("Averaging periods (hr): ")), 3, 0);
    layout1->addWidget(periodEditor, 3, 1, 2, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout1);
    mainLayout->addStretch(1);

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
    scenario->flagpoleHeight = sbFlagpoleHeight->value();

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
    sbFlagpoleHeight->setValue(scenario->flagpoleHeight);

    periodEditor->clearValues();
    for (const int value : scenario->averagingPeriods) {
        double period = static_cast<double>(value);
        periodEditor->addValue(period);
    }
}

/****************************************************************************
** Meteorological Data
****************************************************************************/

MetDataPage::MetDataPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    leSurfaceDataFile = new QLineEdit;
    leSurfaceDataFile->setMaxLength(260); // MAX_PATH
    btnSurfaceDataFile = new QToolButton;
    btnSurfaceDataFile->setText("...");

    leUpperAirDataFile = new QLineEdit;
    leUpperAirDataFile->setMaxLength(260); // MAX_PATH
    btnUpperAirDataFile = new QToolButton;
    btnUpperAirDataFile->setText("...");

    sbAnemometerHeight = new QDoubleSpinBox;
    sbAnemometerHeight->setMinimum(0.1);
    sbAnemometerHeight->setMaximum(1000);
    sbAnemometerHeight->setSingleStep(0.1);
    sbAnemometerHeight->setDecimals(1);

    sbWindRotation = new QDoubleSpinBox;
    sbWindRotation->setMinimum(0);
    sbWindRotation->setMaximum(359.9);
    sbWindRotation->setSingleStep(1);
    sbWindRotation->setDecimals(1);
    sbWindRotation->setWrapping(true);

    leSurfaceStationId = new ReadOnlyLineEdit;
    leUpperAirStationId = new ReadOnlyLineEdit;

    lwIntervals = new QListWidget;
    lwIntervals->setSelectionMode(QAbstractItemView::NoSelection);
    lwIntervals->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    btnDiagnostics = new QPushButton("Diagnostics...");
    btnUpdate = new QPushButton("Update");

    // Layouts
    QLatin1String rotationText = QLatin1String("Wind rotation (\u00b0CCW):");
    GridLayout *layout1 = new GridLayout;
    layout1->setColumnStretch(0, 0);
    layout1->setColumnStretch(1, 1);
    layout1->setColumnStretch(2, 0);
    layout1->addWidget(new QLabel(tr("Surface data file:")),        0, 0);
    layout1->addWidget(leSurfaceDataFile,                           0, 1);
    layout1->addWidget(btnSurfaceDataFile,                          0, 2);
    layout1->addWidget(new QLabel(tr("Upper air data file:")),      1, 0);
    layout1->addWidget(leUpperAirDataFile,                          1, 1);
    layout1->addWidget(btnUpperAirDataFile,                         1, 2);
    layout1->addWidget(new QLabel(tr("Anemometer height (m):")),    2, 0);
    layout1->addWidget(sbAnemometerHeight,                          2, 1, 1, 2);
    layout1->addWidget(new QLabel(rotationText),                    3, 0);
    layout1->addWidget(sbWindRotation,                              3, 1, 1, 2);

    // File Information
    GridLayout *infoLayout = new GridLayout;
    int minWidth = infoLayout->columnMinimumWidth(0) - 12;
    infoLayout->setColumnMinimumWidth(0, minWidth);
    infoLayout->setColumnStretch(0, 0);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->addWidget(new QLabel(tr("Surface station ID:")),       0, 0);
    infoLayout->addWidget(leSurfaceStationId,                          0, 1);
    infoLayout->addWidget(new QLabel(tr("Upper air station ID:")),     1, 0);
    infoLayout->addWidget(leUpperAirStationId,                         1, 1);

    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->addSpacing(5);
    statsLayout->addWidget(new QLabel(tr("Time intervals:")));
    statsLayout->addWidget(lwIntervals);
    infoLayout->addLayout(statsLayout, 2, 0, 1, 2);
    QGroupBox *gbFileInfo = new QGroupBox("File Information");
    gbFileInfo->setLayout(infoLayout);

    // Buttons
    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->addStretch(1);
    layout4->addWidget(btnUpdate);
    layout4->addWidget(btnDiagnostics);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(gbFileInfo);
    mainLayout->addLayout(layout4);
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

void MetDataPage::showInfoDialog()
{
    std::string surfaceFile = leSurfaceDataFile->text().toStdString();

    MetFileParser parser(surfaceFile);
    std::shared_ptr<SurfaceData> sd = parser.getSurfaceData();

    if (sd == nullptr)
        return;

    MetFileInfoDialog dialog(sd, this);
    dialog.exec();
}

void MetDataPage::update()
{
    std::string surfaceFile = leSurfaceDataFile->text().toStdString();

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
        defaultDirectory = settings.value(settingsKey, qApp->applicationDirPath()).toString();

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

/****************************************************************************
** Flux Profiles
****************************************************************************/

FluxProfilesPage::FluxProfilesPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    model = new FluxProfileModel;

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

    StandardTableEditor::StandardButtons editorOpts = StandardTableEditor::All;
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
    auto fp = model->fluxProfileFromIndex(index);
    FluxProfileDialog dialog(fp, this);
    int rc = dialog.exec();
    if (rc == QDialog::Accepted) {
        emit model->dataChanged(index, index);
    }
}

/****************************************************************************
** Dispersion Model
****************************************************************************/

DispersionPage::DispersionPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    // AERMOD Group 1: Non-DFAULT
    QGroupBox *gbNonDefault = new QGroupBox("Non-DFAULT Options");
    gbNonDefault->setFlat(true);
    chkFlat = new QCheckBox("Assume flat terrain (FLAT)");
    chkFastArea = new QCheckBox("Optimize model runtime for area sources (FASTAREA)");

    // Deposition Parameters
    chkDryDeposition = new QCheckBox("Enable dry deposition (DDEP)");
    chkDryDplt = new QCheckBox("Enable dry depletion processes (DRYDPLT)");
    chkAreaDplt = new QCheckBox("Optimize plume depletion for AREA sources (AREADPLT)");
    chkGDVelocity = new QCheckBox("Custom gas dry deposition velocity (GASDEPVD):");
    chkWetDeposition = new QCheckBox("Enable wet deposition (WDEP)");
    chkWetDplt = new QCheckBox("Enable wet depletion processes (WETDPLT)");

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

    // AERMOD Group 2: ALPHA
    QGroupBox *gbAlpha = new QGroupBox("ALPHA Options");
    gbAlpha->setFlat(true);
    chkLowWind = new QCheckBox("Optimize model for low wind speeds (LOW_WIND)");

    // Low Wind Parameters
    //lblSVmin = new QLabel("Minimum \xcf\x83\xce\xbd:");
    //lblWSmin = new QLabel("Minimum wind speed:");
    //lblFRANmax = new QLabel("Maximum meander factor:");
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
    QGridLayout *gbAlphaLayout = new QGridLayout;
    gbAlphaLayout->setColumnMinimumWidth(0, 16);
    gbAlphaLayout->setColumnStretch(0, 0);
    gbAlphaLayout->setColumnStretch(1, 0);
    gbAlphaLayout->setColumnStretch(2, 1);
    gbAlphaLayout->setColumnStretch(3, 0);
    gbAlphaLayout->setColumnStretch(4, 1);
    gbAlphaLayout->setColumnStretch(5, 2);
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

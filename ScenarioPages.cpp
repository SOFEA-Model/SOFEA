#include <QtWidgets>

#include "ScenarioPages.h"
#include "MetFileParser.h"

/****************************************************************************
** General
****************************************************************************/

GeneralPage::GeneralPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    cboFumigant = new QComboBox;

    leDecayCoefficient = new DoubleLineEdit;
    leDecayCoefficient->setText("0");

    // Layouts
    GridLayout *layout1 = new GridLayout;
    layout1->addWidget(new QLabel(tr("Fumigant type:")), 0, 0);
    layout1->addWidget(cboFumigant, 0, 1);
    layout1->addWidget(new QLabel(tr("Decay coefficient (1/sec):")), 1, 0);
    layout1->addWidget(leDecayCoefficient, 1, 1);

    layout1->setRowMinimumHeight(0, 28);
    layout1->setRowMinimumHeight(1, 28);

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
    for (auto const& item : scenario->chemicalMap) {
        QString text = QString::fromStdString(item.second);
        cboFumigant->addItem(text, item.first); // ID
    }

    load();
}

void GeneralPage::save()
{
    scenario->fumigantId = cboFumigant->currentData().toInt();
    scenario->decayCoefficient = leDecayCoefficient->text().toDouble();
}

void GeneralPage::load()
{
    int fumigantId = static_cast<int>(scenario->fumigantId);
    int fumigantIndex = cboFumigant->findData(fumigantId);
    cboFumigant->setCurrentIndex(fumigantIndex);
    leDecayCoefficient->setText(QString::number(scenario->decayCoefficient));
}

/****************************************************************************
** Meteorological Data
****************************************************************************/

MetDataPage::MetDataPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    leSurfaceDataFile = new QLineEdit;
    leSurfaceDataFile->setMaxLength(260); // MAX_PATH
    btnSurfaceDataFile = new QPushButton("Browse...");

    leUpperAirDataFile = new QLineEdit;
    leUpperAirDataFile->setMaxLength(260); // MAX_PATH
    btnUpperAirDataFile = new QPushButton("Browse...");

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
    //leTotalHours = new ReadOnlyLineEdit;
    //leCalmHours = new ReadOnlyLineEdit;
    //leMissingHours = new ReadOnlyLineEdit;

    lwIntervals = new QListWidget;
    lwIntervals->setSelectionMode(QAbstractItemView::NoSelection);


    //StandardTableView *statsTable = new StandardTableView;
    //QStandardItemModel *statsModel = new QStandardItemModel;
    //statsModel->setColumnCount(4);
    //statsModel->setHorizontalHeaderLabels(QStringList({"Interval","Total Hours","Calm Hours"}));
    //statsTable->setModel(statsModel);
    //
    //QFontMetrics fm = fontMetrics();
    //QHeaderView *header = statsTable->horizontalHeader();
    //header->setStretchLastSection(false);
    //header->resizeSection(0, fm.width("[2010-Jan-01 00:00:00,2012-Jan-01 00:00:00) "));
    //header->resizeSection(1, fm.width("Total Hours"));
    //header->resizeSection(2, fm.width("Calm Hours"));


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
    //infoLayout->addWidget(new QLabel(tr("Total hours:")),              2, 0);
    //infoLayout->addWidget(leTotalHours,                                2, 1);
    //infoLayout->addWidget(new QLabel(tr("Calm hours:")),               3, 0);
    //infoLayout->addWidget(leCalmHours,                                 3, 1);
    //infoLayout->addWidget(new QLabel(tr("Missing hours:")),            4, 0);
    //infoLayout->addWidget(leMissingHours,                              4, 1);

    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->addSpacing(5);
    statsLayout->addWidget(new QLabel(tr("Time intervals:")));
    statsLayout->addWidget(lwIntervals);
    infoLayout->addLayout(statsLayout, 2, 0, 1, 2);
    QGroupBox *gbFileInfo = new QGroupBox("File Information");
    gbFileInfo->setLayout(infoLayout);

    // Update Button
    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->addStretch(1);
    layout4->addWidget(btnUpdate);

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

    connect(btnSurfaceDataFile, &QPushButton::clicked, this, &MetDataPage::browseMetDataFile);
    connect(btnUpperAirDataFile, &QPushButton::clicked, this, &MetDataPage::browseMetDataFile);
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

void MetDataPage::update()
{
    std::string surfaceFile = leSurfaceDataFile->text().toStdString();
    SurfaceFileInfo sfInfo = MetFileParser::parseSurfaceFile(surfaceFile);

    leSurfaceStationId->setText(QString::fromStdString(sfInfo.sfloc));
    leUpperAirStationId->setText(QString::fromStdString(sfInfo.ualoc));
    //leTotalHours->setText(QString::number(sfInfo.nrec));
    //leCalmHours->setText(QString::number(sfInfo.ncalm));
    //leMissingHours->setText(QString::number(sfInfo.nmiss));

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
** Dispersion Model
****************************************************************************/

DispersionPage::DispersionPage(Scenario *s, QWidget *parent)
    : QWidget(parent), scenario(s)
{
    // TODO:
    // - Allow non-sequential meteorological data files (WARNCHKD)
    // - No date checking for met data (NOCHKD)

    // AERMOD Group 1: Non-DFAULT
    QGroupBox *gbNonDefault;
    gbNonDefault = new QGroupBox("Non-DFAULT Options");
    gbNonDefault->setFlat(true);
    chkFlat = new QCheckBox("Assume flat terrain (FLAT)");
    chkFastArea = new QCheckBox("Optimize model runtime for area sources (FASTAREA)");

    // AERMOD Group 2: ALPHA
    QGroupBox *gbAlpha;
    gbAlpha = new QGroupBox("ALPHA Options");
    gbAlpha->setFlat(true);
    chkLowWind = new QCheckBox("Optimize model for low wind speeds (LOW_WIND)");

    // Low Wind Parameters
    sbSVmin = new QDoubleSpinBox;
    sbSVmin->setRange(0.01, 1.0);
    sbSVmin->setDecimals(4);
    sbSVmin->setSingleStep(0.0001);

    sbWSmin = new QDoubleSpinBox;
    sbWSmin->setRange(0.01, 1.0);
    sbWSmin->setDecimals(4);
    sbWSmin->setSingleStep(0.0001);

    sbFRANmax = new QDoubleSpinBox;
    sbFRANmax->setRange(0.0, 1.0);
    sbFRANmax->setDecimals(4);
    sbFRANmax->setSingleStep(0.0001);

    sbSVmin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbWSmin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sbFRANmax->setButtonSymbols(QAbstractSpinBox::NoButtons);

    QGridLayout *lowWindLayout = new QGridLayout;
    lowWindLayout->setMargin(0);
    lowWindLayout->addWidget(new QLabel("Minimum \xcf\x83\xce\xbd (m/s): "), 0, 0, 1, 1);
    lowWindLayout->addWidget(new QLabel("Minimum wind speed (m/s): "),       1, 0, 1, 1);
    lowWindLayout->addWidget(new QLabel("Maximum meander factor (m/s): "),   2, 0, 1, 1);
    lowWindLayout->addWidget(sbSVmin,      0, 1, 1, 1);
    lowWindLayout->addWidget(sbWSmin,      1, 1, 1, 1);
    lowWindLayout->addWidget(sbFRANmax,    2, 1, 1, 1);
    lowWindParams = new QWidget;
    lowWindParams->setLayout(lowWindLayout);
    lowWindParams->setDisabled(true);

    // ISCST3 Group 1: Non-DFAULT
    gbIscNonDefault = new QGroupBox("Non-DFAULT Options");
    gbIscNonDefault->setFlat(true);
    chkIscNoCalm = new QCheckBox("Bypass calms processing routine (NOCALM)");
    chkIscMsgPro = new QCheckBox("Use missing data processing routines (MSGPRO)");

    // Non-DFAULT Layout
    QGridLayout *gbNonDefaultLayout = new QGridLayout;
    gbNonDefaultLayout->addWidget(chkFlat, 0, 0);
    gbNonDefaultLayout->addWidget(chkFastArea, 1, 0);

    // ALPHA Layout
    QGridLayout *gbAlphaLayout = new QGridLayout;
    gbAlphaLayout->setColumnMinimumWidth(0, 15);
    gbAlphaLayout->setColumnStretch(0, 0);
    gbAlphaLayout->addWidget(chkLowWind,    0, 0, 1, 2);
    gbAlphaLayout->addWidget(lowWindParams, 1, 1, 1, 1);

    QVBoxLayout *gbIscNonDefaultLayout = new QVBoxLayout;
    gbIscNonDefaultLayout->addWidget(chkIscNoCalm);
    gbIscNonDefaultLayout->addWidget(chkIscMsgPro);

    gbNonDefault->setLayout(gbNonDefaultLayout);
    gbAlpha->setLayout(gbAlphaLayout);
    gbIscNonDefault->setLayout(gbIscNonDefaultLayout);

    QVBoxLayout *aermodLayout = new QVBoxLayout;
    aermodLayout->addWidget(gbNonDefault);
    aermodLayout->addWidget(gbAlpha);
    aermodLayout->addStretch(1);

    QWidget *aermodTab = new QWidget;
    aermodTab->setLayout(aermodLayout);

    QVBoxLayout *iscLayout = new QVBoxLayout;
    iscLayout->addWidget(gbIscNonDefault);
    iscLayout->addStretch(1);

    QWidget *iscTab = new QWidget;
    iscTab->setLayout(iscLayout);

    // Tab Widget
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(aermodTab, "AERMOD");
    tabWidget->addTab(iscTab, "ISCST3");
    tabWidget->setTabEnabled(1, false); // FIXME

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->setMargin(0);

    setLayout(mainLayout);
    init();
}

void DispersionPage::init()
{
    connect(chkLowWind, &QCheckBox::toggled,
        [=](bool checked) {
        lowWindParams->setDisabled(!checked);
    });

    load();
}

void DispersionPage::save()
{
    scenario->aermodFlat = chkFlat->isChecked();
    scenario->aermodFastArea = chkFastArea->isChecked();
    scenario->aermodLowWind = chkLowWind->isChecked();
    scenario->svMin = sbSVmin->value();
    scenario->wsMin = sbWSmin->value();
    scenario->franMax = sbFRANmax->value();

    // FIXME: ISCST3
}

void DispersionPage::load()
{
    chkFlat->setChecked(scenario->aermodFlat);
    chkFastArea->setChecked(scenario->aermodFastArea);
    chkLowWind->setChecked(scenario->aermodLowWind);
    sbSVmin->setValue(scenario->svMin);
    sbWSmin->setValue(scenario->wsMin);
    sbFRANmax->setValue(scenario->franMax);

    // FIXME: ISCST3
}

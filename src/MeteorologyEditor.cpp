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

#include "AppStyle.h"
#include "MagneticDeclinationDialog.h"
#include "MeteorologyEditor.h"
#include "MeteorologyFileDialog.h"
#include "MeteorologyModel.h"
#include "MeteorologyStationData.h"
#include "MeteorologyTableView.h"
#include "widgets/ButtonLineEdit.h"
#include "widgets/PathEdit.h"
#include "widgets/StandardTableEditor.h"
#include "widgets/StatusLabel.h"

#include <string>

#include <QAction>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDataWidgetMapper>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>

#include <QDebug>

MeteorologyEditor::MeteorologyEditor(MeteorologyModel *m, QWidget *parent)
    : QWidget(parent), model(m)
{
    lblInfoTip = new StatusLabel;
    lblInfoTip->setStatusType(StatusLabel::InfoTip);
    lblInfoTip->setText(tr("Drag-and-drop to easily add multiple files."));

    table = new MeteorologyTableView(model, this);

    StandardTableEditor::StandardButtons buttons =
        StandardTableEditor::Add | StandardTableEditor::Remove | StandardTableEditor::Rename |
        StandardTableEditor::MoveUp | StandardTableEditor::MoveDown;

    tableEditor = new StandardTableEditor(Qt::Vertical, buttons);
    tableEditor->setView(table);
    tableEditor->button(StandardTableEditor::AddRole)->setText(tr("Add..."));

    QSettings settings;
    QString defaultDir = settings.value("DefaultMetFileDirectory", QDir::currentPath()).toString();

    leSurfaceFile = new PathEdit;
    leSurfaceFile->setDialogCaption(tr("Select Surface File"));
    leSurfaceFile->setDialogDirectory(defaultDir);
    leSurfaceFile->setDialogFilter("AERMET Surface Data (*.sfc)");

    leUpperAirFile = new PathEdit;
    leUpperAirFile->setDialogCaption(tr("Select Upper Air File"));
    leUpperAirFile->setDialogDirectory(defaultDir);
    leUpperAirFile->setDialogFilter("AERMET Profile Data (*.pfl)");

    sbTerrainElevation = new QDoubleSpinBox;
    sbTerrainElevation->setMinimum(0);
    sbTerrainElevation->setMaximum(100000);
    sbTerrainElevation->setSingleStep(0.1);
    sbTerrainElevation->setDecimals(1);

    sbAnemometerHeight = new QDoubleSpinBox;
    sbAnemometerHeight->setMinimum(0);
    sbAnemometerHeight->setMaximum(100000);
    sbAnemometerHeight->setSingleStep(0.1);
    sbAnemometerHeight->setDecimals(1);

    sbWindRotation = new QDoubleSpinBox;
    sbWindRotation->setMinimum(-180.0);
    sbWindRotation->setMaximum(180.0);
    sbWindRotation->setSingleStep(0.1);
    sbWindRotation->setDecimals(1);

    const QIcon icoCalc = this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_CalculateButton));
    btnDeclinationCalc = new QToolButton;
    btnDeclinationCalc->setIcon(icoCalc);
    btnDeclinationCalc->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btnDeclinationCalc->setToolTip(tr("Magnetic Declination Calculator"));

    btnUpdate = new QPushButton(tr("Update"));

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(model);
    mapper->setOrientation(Qt::Horizontal);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(leSurfaceFile, MeteorologyModel::Column::SurfaceFile, "currentPath");
    mapper->addMapping(leUpperAirFile, MeteorologyModel::Column::UpperAirFile, "currentPath");
    mapper->addMapping(sbTerrainElevation, MeteorologyModel::Column::TerrainElevation);
    mapper->addMapping(sbAnemometerHeight, MeteorologyModel::Column::AnemometerHeight);
    mapper->addMapping(sbWindRotation, MeteorologyModel::Column::WindRotation);

    // Connections
    connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
            mapper, &QDataWidgetMapper::setCurrentModelIndex);

    connect(tableEditor->button(StandardTableEditor::AddRole), &QPushButton::clicked,
            this, &MeteorologyEditor::onAddClicked);

    connect(btnDeclinationCalc, &QToolButton::clicked,
            this, &MeteorologyEditor::onDeclinationCalcClicked);

    connect(btnUpdate, &QPushButton::clicked, mapper, &QDataWidgetMapper::submit);

    // Layout
    QHBoxLayout *tableLayout = new QHBoxLayout;
    tableLayout->setContentsMargins(0, 8, 0, 8);
    tableLayout->addWidget(table);
    tableLayout->addSpacing(16);
    tableLayout->addWidget(tableEditor);

    QGridLayout *fileLayout = new QGridLayout;
    fileLayout->setContentsMargins(0, 8, 0, 8);
    fileLayout->setColumnStretch(0, 0);
    fileLayout->setColumnStretch(1, 1);
    fileLayout->addWidget(new QLabel(tr("Surface data file:")), 0, 0);
    fileLayout->addWidget(leSurfaceFile, 0, 1);
    fileLayout->addWidget(new QLabel(tr("Upper air data file:")), 1, 0);
    fileLayout->addWidget(leUpperAirFile, 1, 1);

    QGridLayout *inputLayout = new QGridLayout;
    inputLayout->setContentsMargins(0, 8, 0, 8);
    inputLayout->setColumnStretch(0, 0);
    inputLayout->setColumnStretch(1, 1);
    inputLayout->setColumnStretch(2, 0);
    inputLayout->addWidget(new QLabel(tr("Terrain elevation (ft):")), 0, 0);
    inputLayout->addWidget(sbTerrainElevation, 0, 1, 1, 2);
    inputLayout->addWidget(new QLabel(tr("Anemometer height (ft):")), 1, 0);
    inputLayout->addWidget(sbAnemometerHeight, 1, 1, 1, 2);
    inputLayout->addWidget(new QLabel(QLatin1String("Wind rotation (\u00b0CW):")), 2, 0);
    inputLayout->addWidget(sbWindRotation, 2, 1);
    inputLayout->addWidget(btnDeclinationCalc, 2, 2);

    QHBoxLayout *updateLayout = new QHBoxLayout;
    updateLayout->addWidget(btnUpdate, 0, Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->addWidget(lblInfoTip);
    mainLayout->addLayout(tableLayout, 1);
    mainLayout->addLayout(fileLayout);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(updateLayout);

    QPalette palette = this->palette();
    QColor bgcolor = QWidget::palette().window().color();
    palette.setColor(QPalette::Window, bgcolor.lighter(106));
    setPalette(palette);
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    setLayout(mainLayout);
}

void MeteorologyEditor::onCurrentPathChanged(const QString& path)
{
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    QString defaultDir = fi.absoluteDir().absolutePath();
    QSettings settings;
    settings.setValue("DefaultMetFileDirectory", defaultDir);

    leUpperAirFile->setDialogDirectory(defaultDir);
    leSurfaceFile->setDialogDirectory(defaultDir);
}

void MeteorologyEditor::onAddClicked()
{
    QSettings settings;
    QVariant defaultPath = settings.value("DefaultMetFileDirectory", QDir::currentPath());
    QUrl dialogDir = QUrl::fromLocalFile(defaultPath.toString());

    QString dialogFilter = QString("%1;;%2;;%3").arg(
        "All Supported Files (*.sfc *.pfl)",
        "AERMET Surface Data (*.sfc)",
        "AERMET Profile Data (*.pfl)");

    QList<QUrl> urls = QFileDialog::getOpenFileUrls(nullptr, "Select Files",
        dialogDir, dialogFilter, nullptr, QFileDialog::Options(), QStringList{ "file" });

    model->addUrls(urls);
}

void MeteorologyEditor::onDeclinationCalcClicked()
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


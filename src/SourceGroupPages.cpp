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

#include "SourceGroupPages.h"

#include "BufferZoneModel.h"
#include "BufferZoneEditor.h"
#include "FluxProfileModel.h"
#include "MonteCarloLineEdit.h"
#include "MonteCarloDateTimeEdit.h"
#include "SamplingDistributionEditor.h"
#include "delegate/DoubleSpinBoxDelegate.h"
#include "delegate/SpinBoxDelegate.h"
#include "widgets/BackgroundFrame.h"
#include "widgets/GridLayout.h"
#include "widgets/StandardTableView.h"
#include "widgets/StatusLabel.h"

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <csv/csv.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QRadioButton>
#include <QString>

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

    bgCalcMode = new QButtonGroup(this);
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
    calcModeLayout->setAlignment(radioProspective, Qt::AlignLeft);
    calcModeLayout->setAlignment(radioValidation, Qt::AlignLeft);

    GridLayout *layout1 = new GridLayout;
    layout1->addWidget(new QLabel(tr("Application method:")), 0, 0);
    layout1->addWidget(cboAppMethod, 0, 1);
    layout1->addWidget(new QLabel(tr("Application factor:")), 1, 0);
    layout1->addWidget(sbAppFactor, 1, 1);
    layout1->addWidget(new QLabel(tr("Calculation mode:")), 2, 0);
    layout1->addLayout(calcModeLayout, 2, 1, Qt::AlignLeft);

    GridLayout *layout2 = new GridLayout;
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

    connect(radioValidation, &QRadioButton::toggled, gbMonteCarlo, &QGroupBox::setDisabled);

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
    lblDepoNotEnabled->setStatusType(StatusLabel::Alert);
    lblDepoNotEnabled->setText("Deposition must be enabled for these parameters to take effect.");

    lblDepoUserVelocity = new StatusLabel;
    lblDepoUserVelocity->setStatusType(StatusLabel::Alert);
    lblDepoUserVelocity->setText("Custom gas dry deposition velocity must be disabled for these parameters to take effect.");

    // Layout
    QGridLayout *layout1 = new QGridLayout;
    layout1->addWidget(new QLabel(QLatin1String("Air diffusivity [Da] (cm\xb2/sec):")), 0, 0);
    layout1->addWidget(mcAirDiffusion, 0, 1);
    layout1->addWidget(new QLabel(QLatin1String("Water diffusivity [Dw] (cm\xb2/sec):")), 1, 0);
    layout1->addWidget(mcWaterDiffusion, 1, 1);
    layout1->addWidget(new QLabel(QLatin1String("Cuticular resistance [rcl] (s/cm):")), 2, 0);
    layout1->addWidget(mcCuticularResistance, 2, 1);
    layout1->addWidget(new QLabel(QLatin1String("Henry's law constant (Pa-m\xb3/mol):")), 3, 0);
    layout1->addWidget(mcHenryConstant, 3, 1);

    QGroupBox *gbMonteCarlo = new QGroupBox("Monte Carlo Parameters");
    gbMonteCarlo->setFlat(true);
    gbMonteCarlo->setLayout(layout1);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gbMonteCarlo);
    mainLayout->addStretch(1);
    mainLayout->addWidget(lblDepoNotEnabled);
    mainLayout->addWidget(lblDepoUserVelocity);
    mainLayout->addSpacing(5);

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
    : QWidget(parent), sPtr(s), sgPtr(sg)
{
    model = new FluxProfileModel(this);
    model->load(sPtr->fluxProfiles);

    editor = new SamplingDistributionEditor(model);
    editor->setColumnHidden(1);
    editor->setColumnHidden(2);
    editor->setColumnHidden(3);

    lblNoFluxProfile = new StatusLabel;
    lblNoFluxProfile->setStatusType(StatusLabel::Warning);
    lblNoFluxProfile->setText("No flux profiles have been defined in the scenario.");
    lblNoFluxProfile->setVisible(model->rowCount() == 0);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editor);
    mainLayout->addStretch(1);
    mainLayout->addWidget(lblNoFluxProfile);
    mainLayout->addSpacing(5);

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
        double probability = editor->probability(row);

        // SourceGroup holds a weak_ptr to Scenario flux profile.
        std::weak_ptr<FluxProfile> weak = fp;
        sgPtr->fluxProfile.data[weak] = probability;
    }
}

void FluxProfilePage::load()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, 0);
        auto fp = model->fluxProfileFromIndex(index);

        // SourceGroup holds a weak_ptr to Scenario flux profile.
        std::weak_ptr<FluxProfile> weak = fp;

        if (sgPtr->fluxProfile.data.count(fp) > 0) {
            double probability = sgPtr->fluxProfile.data.at(weak);
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
    model = new BufferZoneModel(this);
    editor = new BufferZoneEditor(model);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(editor);

    BackgroundFrame *frame = new BackgroundFrame;
    frame->setLayout(mainLayout);
    QVBoxLayout *frameLayout = new QVBoxLayout;
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(frame);

    setLayout(frameLayout);
    init();
}

void BufferZonePage::init()
{
    load();
}

void BufferZonePage::save()
{
    sgPtr->enableBufferZones = editor->isEnableChecked();
    model->save(sgPtr->zones);
}

void BufferZonePage::load()
{
    editor->setEnableChecked(sgPtr->enableBufferZones);
    model->load(sgPtr->zones);
}


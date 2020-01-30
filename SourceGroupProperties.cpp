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

#include <QPushButton>
#include <QWidget>
#include <QIcon>

#include "AppStyle.h"
#include "SourceGroupProperties.h"

SourceGroupProperties::SourceGroupProperties(Scenario *s, SourceGroup *sg, QWidget *parent)
    : SettingsDialog(parent), sgPtr(sg)
{
    setWindowTitle(QString::fromStdString(sg->grpid));
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionSettings)));

    applicationPage = new ApplicationPage(sg);
    depositionPage = new DepositionPage(sg);
    fluxProfilePage = new FluxProfilePage(s, sg);
    bufferZonePage = new BufferZonePage(sg);

    // Data from parent scenario.
    if (!s->aermodDryDeposition && !s->aermodWetDeposition) {
        depositionPage->warnDepoNotEnabled();
    }
    if (s->aermodGDVelocityEnabled) {
        depositionPage->warnUserVelocity();
    }

    addPage("Application", applicationPage);
    addPage("Deposition", depositionPage);
    addPage("Flux Profiles", fluxProfilePage);
    addPage("Buffer Zones", bufferZonePage);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SourceGroupProperties::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SourceGroupProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SourceGroupProperties::reject);
}

void SourceGroupProperties::apply()
{
    applicationPage->save();
    depositionPage->save();
    fluxProfilePage->save();
    bufferZonePage->save();

    // Recalculate source values if distribution is modified and in prospective mode.
    // TODO: remove isModified when random seed support is added. Add a menu option to explicitly resample selected parameters.
    if (!sgPtr->validationMode)
    {
        if (applicationPage->mcAppStart->isModified())
            sgPtr->resampleAppStart();
        if (applicationPage->mcAppRate->isModified())
            sgPtr->resampleAppRate();
        if (applicationPage->mcIncorpDepth->isModified())
            sgPtr->resampleIncorpDepth();
    }

    // Recalculate deposition values if modified, regardless of mode.
    if (depositionPage->mcAirDiffusion->isModified())
        sgPtr->resampleAirDiffusion();
    if (depositionPage->mcWaterDiffusion->isModified())
        sgPtr->resampleWaterDiffusion();
    if (depositionPage->mcCuticularResistance->isModified())
        sgPtr->resampleCuticularResistance();
    if (depositionPage->mcHenryConstant->isModified())
        sgPtr->resampleHenryConstant();

    emit saved();
}

void SourceGroupProperties::accept()
{
    apply();
    QDialog::done(QDialog::Accepted);
}

void SourceGroupProperties::reject()
{
    QDialog::done(QDialog::Rejected);
}

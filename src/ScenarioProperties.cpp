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
#include "ScenarioProperties.h"
#include "Scenario.h"

ScenarioProperties::ScenarioProperties(Scenario *s, QWidget *parent)
    : SettingsDialog(parent), sPtr(s)
{
    setWindowTitle(QString::fromStdString(s->name));
    setWindowIcon(this->style()->standardIcon(static_cast<QStyle::StandardPixmap>(AppStyle::CP_ActionSettings)));

    generalPage = new GeneralPage(s);
    projectionPage = new ProjectionPage(s);
    metDataPage = new MetDataPage(s);
    fluxProfilesPage = new FluxProfilesPage(s);
    dispersionPage = new DispersionPage(s);

    addPage("General Settings", generalPage);
    addPage("Coordinate System", projectionPage);
    addPage("Meteorological Data", metDataPage);
    addPage("Flux Profiles", fluxProfilesPage);
    addPage("Dispersion Model", dispersionPage);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScenarioProperties::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ScenarioProperties::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ScenarioProperties::reject);
}

void ScenarioProperties::apply()
{
    generalPage->save();
    projectionPage->save();
    metDataPage->save();
    fluxProfilesPage->save();
    dispersionPage->save();
    emit saved();
}

void ScenarioProperties::accept()
{
    apply();
    QDialog::done(QDialog::Accepted);
}

void ScenarioProperties::reject()
{
    QDialog::done(QDialog::Rejected);
}

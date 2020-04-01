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

#ifndef SCENARIOPROPERTIES_H
#define SCENARIOPROPERTIES_H

#include "core/Scenario.h"
#include "ScenarioPages.h"
#include "SettingsDialog.h"

class ScenarioProperties : public SettingsDialog
{
    Q_OBJECT

public:
    ScenarioProperties(Scenario *s, QWidget *parent = nullptr);

signals:
    void saved();

public slots:
    void apply();
    void accept() override;
    void reject() override;

private:
    Scenario *sPtr;
    GeneralPage *generalPage;
    ProjectionPage *projectionPage;
    MetDataPage *metDataPage;
    FluxProfilesPage *fluxProfilesPage;
    DispersionPage *dispersionPage;
};

#endif // SCENARIOPROPERTIES_H

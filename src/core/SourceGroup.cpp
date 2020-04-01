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

#include "core/SourceGroup.h"

#include <cassert>
#include <cmath>
#include <utility>

#include <fmt/format.h>

SourceGroup::SourceGroup()
{
    static unsigned int sequenceNumber = 1;
    id = sequenceNumber;
    sequenceNumber++;

    grpid = fmt::format("Crop{:0=2}", id);
    appMethod = AppMethod::Other;
    appFactor = 1.0;
    validationMode = false;

    appStart = QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC);
    appRate = Distribution::Constant();
    incorpDepth = Distribution::Constant();
    airDiffusion = Distribution::Constant();
    waterDiffusion = Distribution::Constant();
    cuticularResistance = Distribution::Constant();
    henryConstant = Distribution::Constant();

    enableBufferZones = false;
}

// Used in RunstreamParser, SourceModel
void SourceGroup::initSource(Source *s)
{
    s->appStart = appStart(&gen);
    s->appRate = appRate(&gen);
    s->incorpDepth = incorpDepth(&gen);
    s->airDiffusion = airDiffusion(&gen);
    s->waterDiffusion = waterDiffusion(&gen);
    s->cuticularResistance = cuticularResistance(&gen);
    s->henryConstant = henryConstant(&gen);
    s->fluxProfile = fluxProfile(&gen);
}

// Used in ShapefileParser, SourceTable
void SourceGroup::initSourceAppStart(Source *s)
{
    s->appStart = appStart(&gen);
}
void SourceGroup::initSourceAppRate(Source *s)
{
    s->appRate = appRate(&gen);
}
void SourceGroup::initSourceIncorpDepth(Source *s)
{
    s->incorpDepth = incorpDepth(&gen);
}
void SourceGroup::initSourceAirDiffusion(Source *s)
{
    s->airDiffusion = airDiffusion(&gen);
}
void SourceGroup::initSourceWaterDiffusion(Source *s)
{
    s->waterDiffusion = waterDiffusion(&gen);
}
void SourceGroup::initSourceCuticularResistance(Source *s)
{
    s->cuticularResistance = cuticularResistance(&gen);
}
void SourceGroup::initSourceHenryConstant(Source *s)
{
    s->henryConstant = henryConstant(&gen);
}
void SourceGroup::initSourceFluxProfile(Source *s)
{
    s->fluxProfile = fluxProfile(&gen);
}

// Used in SourceGroupProperties
void SourceGroup::resampleAppStart()
{
    for (Source &s : sources)
        s.appStart = appStart(&gen);
}
void SourceGroup::resampleAppRate()
{
    for (Source &s : sources)
        s.appRate = appRate(&gen);
}
void SourceGroup::resampleIncorpDepth()
{
    for (Source &s : sources)
        s.incorpDepth = incorpDepth(&gen);
}
void SourceGroup::resampleAirDiffusion()
{
    for (Source &s : sources)
        s.airDiffusion = airDiffusion(&gen);
}
void SourceGroup::resampleWaterDiffusion()
{
    for (Source &s : sources)
        s.waterDiffusion = waterDiffusion(&gen);
}
void SourceGroup::resampleCuticularResistance()
{
    for (Source &s : sources)
        s.cuticularResistance = cuticularResistance(&gen);
}
void SourceGroup::resampleHenryConstant()
{
    for (Source &s : sources)
        s.henryConstant = henryConstant(&gen);
}
void SourceGroup::resampleFluxProfile()
{
    for (Source &s : sources)
        s.fluxProfile = fluxProfile(&gen);
}


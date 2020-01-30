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

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include <QDateTime>

#include "DateTimeDistribution.h"
#include "GenericDistribution.h"
#include "SamplingDistribution.h"
#include "Source.h"
#include "Receptor.h"
#include "BufferZone.h"
#include "FluxProfile.h"

struct SourceGroup
{
    SourceGroup();
    SourceGroup(const SourceGroup& rhs) = default;

    void initSource(Source *s);
    void initSourceAppStart(Source *s);
    void initSourceAppRate(Source *s);
    void initSourceIncorpDepth(Source *s);
    void initSourceAirDiffusion(Source *s);
    void initSourceWaterDiffusion(Source *s);
    void initSourceCuticularResistance(Source *s);
    void initSourceHenryConstant(Source *s);
    void initSourceFluxProfile(Source *s);

    void resampleAppStart();
    void resampleAppRate();
    void resampleIncorpDepth();
    void resampleAirDiffusion();
    void resampleWaterDiffusion();
    void resampleCuticularResistance();
    void resampleHenryConstant();
    void resampleFluxProfile();

    enum class AppMethod {
        Other,
        TIFDripIrrigation,
        TIFShankInjection,
        NonTIFDripIrrigation,
        NonTIFShankInjection,
        UntarpedDripIrrigation,
        UntarpedShankInjection
    };

    // Random Generator
    boost::random::mt19937 gen;

    // General
    unsigned int id;
    std::string grpid;
    AppMethod appMethod;
    double appFactor;

    // Monte Carlo Parameters
    bool validationMode;
    DateTimeDistribution appStart;
    GenericDistribution appRate;
    GenericDistribution incorpDepth;

    // Deposition
    GenericDistribution airDiffusion;
    GenericDistribution waterDiffusion;
    GenericDistribution cuticularResistance;
    GenericDistribution henryConstant;

    // Buffer Zones
    bool enableBufferZones;
    std::set<BufferZone> zones;

    // Flux Profile
    using FluxProfilePtr = std::weak_ptr<FluxProfile>;
    using FluxProfileDistribution = SamplingDistribution<FluxProfilePtr>;
    FluxProfileDistribution fluxProfile;

    // Sources
    boost::ptr_vector<Source> sources;

    // Receptors
    std::vector<ReceptorRing> rings; // *** Depreciated, For Serialization Only ***
    std::vector<ReceptorNode> nodes; // *** Depreciated, For Serialization Only ***
    std::vector<ReceptorGrid> grids; // *** Depreciated, For Serialization Only ***
};


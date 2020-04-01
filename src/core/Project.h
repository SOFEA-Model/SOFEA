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
#include <string>
#include <vector>

#include <boost/random/seed_seq.hpp>

#include "core/Scenario.h"
#include "core/SourceGroup.h"
#include "core/FluxProfile.h"
#include "core/Receptor.h"
#include "core/Meteorology.h"

struct Project
{
    int maxThreads = -1;
    int debugLevel = 0;

    std::string projectFilePath;
    std::string outputDirectory;
    bool modified = false;

    // Dispersion Model Control
    //std::vector<int> averagingPeriods;
    //double decayCoefficient;

    // Coordinate System
    //Projection::Area domain;
    //std::string conversionCode;
    //std::string hUnitsCode;
    //std::string hDatumCode;
    //std::string vUnitsCode;
    //std::string vDatumCode;
    //double xShift = 0;
    //double yShift = 0;

    //   * Project Options
    //   * Meteorological Data
    //     - Station01 ...
    //   * Flux Profiles
    //     - Profile01 ...
    //   * Buffer Zones
    //     - Zone01 (MeBr) ...
    //   * Distributions
    //     - 70%/30% PE/TIF Flux
    //     - Buffer Zone Mix
    //     - DateTime
    //     - General
    //   * Source Groups
    //     - Group01 ...
    //   * Receptor Groups
    //     - Ring01 ...
    //   * Run Configurations
    //     - Met Data + Source Groups + Receptor Groups + Random Seeds

    using ScenarioPtr = std::shared_ptr<Scenario>; // Replace
    using MeteorologyPtr = std::shared_ptr<Meteorology>;
    using SourceGroupPtr = std::shared_ptr<SourceGroup>;
    using FluxProfilePtr = std::shared_ptr<FluxProfile>;
    using ReceptorGroupPtr = std::shared_ptr<ReceptorGroup>;

    using MeteorologyWeakPtr = std::weak_ptr<Meteorology>;
    using SourceGroupWeakPtr = std::weak_ptr<SourceGroup>;
    using FluxProfileWeakPtr = std::weak_ptr<FluxProfile>;
    using ReceptorGroupWeakPtr = std::weak_ptr<ReceptorGroup>;

    std::vector<MeteorologyPtr> meteorology;
    std::vector<FluxProfilePtr> fluxProfiles;
    //std::vector<BufferZonePtr> bufferZones;
    //std::vector<GenericDistribution> genericDistributions;
    //std::vector<DateTimeDistribution> dateTimeDistributions;
    //std::vector<FluxProfileDistribution> fluxProfileDistributions;
    //std::vector<BufferZoneDistribution> bufferZoneDistributions;
    std::vector<SourceGroupPtr> sourceGroups;
    std::vector<ReceptorGroupPtr> receptors;
    std::vector<ScenarioPtr> scenarios; // Replace

    struct RunConfiguration {
        std::string name;
        MeteorologyWeakPtr meteorology;
        std::vector<SourceGroupWeakPtr> sourceGroups;
        std::vector<ReceptorGroupWeakPtr> receptors;
        bool bufferZonesEnabled;
        boost::random::seed_seq randomSeed;
    };

    std::vector<RunConfiguration> runConfigurations;

    // TODO:
    // - Isolate distributions
    //   - Source level
    //     - Statistical for numeric params (GenericDistribution)
    //     - Discrete (GenericDistribution) - need to create editor
    //     - Application start date (DateTimeDistribution)
    //     - Flux profile (SamplingDistribution<std::weak_ptr<FluxProfile>>)
    //
    // - Implement Run Configurations using many to many relationships.
    //   * 1 FluxProfile -> MANY Source
    //     * SourceGroup holds weak_ptr to FluxProfile - move to Source level
    //   * 1 BufferZone -> MANY Source
    //     * TODO: SourceGroup holds weak_ptr to BufferZone - move to Source level
    //     * TODO: Option to globally disable BufferZone for RunConfiguration
    //   * 1 SourceGroup -> MANY RunConfiguration
    //   * 1 Meteorology -> MANY RunConfiguration
    //   * 1 ReceptorNodeGroup -> MANY RunConfiguration
    //   * 1 ReceptorGridGroup -> MANY RunConfiguration
    //   * 1 ReceptorRing -> MANY RunConfiguration
    //     * ReceptorRing holds weak_ptr to SourceGroup
    //
    // - Show Input File and source calcs. for initialized Run Configurations only
    //
    // - Automatically set elevations for receptors and sources
    //
    // - Visibility checkable combobox for source groups in receptor editor
    //
    // - Enter latitude/longitude for receptors
    //
    // - Receptor table import from CSV or AERMOD runstream
    //
    // - Implement ProjectOptions
    //   * unitSystem + precision, rounding (ProjectOptionsDialog)
    //   * prng
    //   * outputDirectory
    //
    // - Run/Analysis variables
    //   - output vector
    //     - directory
    //     - postfile
    //     - processExitStatus
    //     - aermodExitStatus
    //     - startTime
    //     - stopTime
    //
    // - Update postfile format with version, receptor group
    // - Add projection, xShift, yShift metadata to input file
    // - Add ME STARTEND
};

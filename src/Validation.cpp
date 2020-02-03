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

#include "Validation.h"

#include "Scenario.h"
#include "MetFileParser.h"
#include "ReceptorVisitor.h"

#include <algorithm>
#include <filesystem>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <fmt/format.h>

// ERRORS LIST:
// FluxProfileDialog.cpp
//     BOOST_LOG_TRIVIAL(error) << "Failed to acquire write lock";
//     BOOST_LOG_TRIVIAL(error) << "Failed to acquire read lock";
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");
//         BOOST_LOG_TRIVIAL(error) << e.what();
// GenericDistribution.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Distribution");
//         BOOST_LOG_TRIVIAL(warning) << e.what();
// GenericDistributionDialog.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Distribution");
//         BOOST_LOG_TRIVIAL(warning) << e.what();
// GeometryOp.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Geometry");
//         BOOST_LOG_TRIVIAL(warning) << message;
//         BOOST_LOG_TRIVIAL(error) << e.what();
// GeosOp.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Geometry");
//         BOOST_LOG_TRIVIAL(info) << "geos: " << buf;
//         BOOST_LOG_TRIVIAL(error) << "geos: " << buf;
// IPCServer.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Model");
//         BOOST_LOG_TRIVIAL(error) << "Unable to start the IPC server: "
//         BOOST_LOG_TRIVIAL(error) << "Unable to stop the IPC server: "
// RunstreamParser.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");
//         *Many*
// Scenario.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Model");
//         BOOST_LOG_TRIVIAL(error) << "Invalid time range";
//         BOOST_LOG_TRIVIAL(error) << "Missing flux profile for source " << s.srcid;
//         BOOST_LOG_TRIVIAL(error) << "Failed to acquire read lock";
// ShapefileParser.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");
//         *Many*
// SourceGroupPages.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Import");
//         BOOST_LOG_TRIVIAL(error) << e.what();
// UDUnitsInterface.cpp
//     BOOST_LOG_TRIVIAL(error) << "udunits2: " << buf;
// MetFileParser.cpp
//     BOOST_LOG_SCOPED_THREAD_TAG("Source", "Model");
//     BOOST_LOG_TRIVIAL(error) << e.what();

namespace Validation {

ValidateScenario::ValidateScenario(const Scenario& s)
    : s_(s)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", s.name)

    // Warn is projection is "Local"
    // Check projection validity
    // Check that domain intersects projection area

    validateMeteorology();
    validateFluxProfiles();
    validateReceptors();
    validateSourceGroups();
}

void ValidateScenario::validateMeteorology()
{
    namespace fs = std::filesystem;

    if (s_.surfaceFile.empty()) {
        BOOST_LOG_TRIVIAL(error) << "Surface file is missing";
    }
    else {
        //try {
        //    fs::path path(s_.surfaceFile);
        //    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        //        BOOST_LOG_TRIVIAL(error) << "Surface file does not exist; check path";
        //    }
        //}
        //catch (fs::filesystem_error& e) {
        //    BOOST_LOG_TRIVIAL(error) << "Surface file error: " << e.what();
        //}
    }

    if (s_.upperAirFile.empty()) {
        BOOST_LOG_TRIVIAL(error) << "Upper air file is missing";
    }
    else {
        //try {
        //    fs::path path(s_.upperAirFile);
        //    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        //        BOOST_LOG_TRIVIAL(error) << "Upper air file does not exist; check path";
        //    }
        //}
        //catch (fs::filesystem_error& e) {
        //    BOOST_LOG_TRIVIAL(error) << "Upper air file error: " << e.what();
        //}
    }

    // Try to parse file
    // Check for calm/missing hours exceed 10% threshold
    // Check for matching records in upper air file
}

void ValidateScenario::validateFluxProfiles()
{
    if (s_.fluxProfiles.empty()) {
        BOOST_LOG_TRIVIAL(error) << "No flux profiles have been defined";
        return;
    }

    for (std::shared_ptr<FluxProfile> fp : s_.fluxProfiles) {
        if (!fp)
            continue;

        if (fp->refAppRate < 0)
            BOOST_LOG_TRIVIAL(error) << "Reference application rate for flux profile '" << fp->name << "' is negative";

        if (fp->refAppRate == 0)
            BOOST_LOG_TRIVIAL(warning) << "Reference application rate for flux profile '" << fp->name << "' is zero; reference flux profile will not be scaled by source application rate";

        if (fp->refFlux.empty())
            BOOST_LOG_TRIVIAL(error) << "Flux profile '" << fp->name << "' has no reference flux data; flux will be zero for all hours";

        if (fp->dsMethod != FluxProfile::DSMethod::Disabled && fp->refVL == fp->maxVL)
            BOOST_LOG_TRIVIAL(warning) << "Depth scaling enabled for flux profile '" << fp->name << "' and reference volatilization loss equals maximum volatilization loss; depth scaling will not be used";
    }
}

void ValidateScenario::validateReceptors()
{
    if (s_.receptors.empty()) {
        BOOST_LOG_TRIVIAL(error) << "No receptors have been defined";
        return;
    }

    double zElevMin = 0;
    double zElevMax = 0;

    for (const auto& rg : s_.receptors) {
        std::string name = boost::apply_visitor(ReceptorGroupNameVisitor(), rg);
        std::size_t n = boost::apply_visitor(ReceptorNodeCountVisitor(), rg);
        if (n == 0)
            BOOST_LOG_TRIVIAL(error) << "Receptor group '" << name << "' contains no receptors";

        for (std::size_t i = 0; i < n; ++i) {
            ReceptorNode node = boost::apply_visitor(ReceptorGroupNodeVisitor(i), rg);
            zElevMin = std::min(zElevMin, node.zElev);
            zElevMax = std::max(zElevMax, node.zElev);
        }
    }

    if (!s_.aermodFlat && zElevMin == zElevMax) {
        BOOST_LOG_TRIVIAL(warning) << "ELEV is enabled, but all receptors have equal elevation";
    }

    // Receptor group 'G' contains receptors outside of modeling domain
}

void ValidateScenario::validateSourceGroups()
{
    if (s_.sourceGroups.empty()) {
        BOOST_LOG_TRIVIAL(error) << "No source groups have been defined";
    }

    for (const std::shared_ptr<SourceGroup>& sg : s_.sourceGroups) {
        if (!sg)
            continue;

        if (sg->sources.empty())
            BOOST_LOG_TRIVIAL(error) << "No sources have been defined for source group '" << sg->grpid << "'";

        if (sg->fluxProfile.data.empty())
            BOOST_LOG_TRIVIAL(error) << "Flux profile probabilities have not been set for source group '" << sg->grpid << "'";
    }

    // Sources 'A', 'B', 'C' ... have overlapping geometry within the same emission period
    // Receptor ring 'R' not updated after source geometry changed
    // Receptor ring 'R' references a missing source group
    // Missing flux profile for source 'S'
    // Emission period for Source 'S' overlaps met file start
    // Emission period for Source 'S' overlaps met file end
    // Emission period for source 'S' is outside of met file range; flux will be zero for all hours
    // Application rate for source 'S' is zero; flux will be zero for all hours (only if reference flux application rate is non-zero)
    // Buffer zone could not be assigned to source 'S': [area, application rate] exceeds threshold
    // Monte Carlo parameter [name] generated values outside of acceptable limits for source 'S'
}

} // namespace Validation




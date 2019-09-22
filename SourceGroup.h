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
    std::vector<ReceptorRing> rings; // depreciated, for serialization only
    std::vector<ReceptorNode> nodes; // depreciated, for serialization only
    std::vector<ReceptorGrid> grids; // depreciated, for serialization only
};


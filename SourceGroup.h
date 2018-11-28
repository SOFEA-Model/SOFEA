#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include <QDateTime>
#include <QPointF>
#include <QPolygonF>
#include <QColor>

#include "DateTimeDistribution.h"
#include "GenericDistribution.h"
#include "SamplingDistribution.h"
#include "Source.h"
#include "FluxProfile.h"

struct Scenario;

struct ReceptorRing
{
    std::string arcid; // length 8
    double buffer;
    double spacing;
    double zElev = 0;
    double zHill = 0;
    QPolygonF points;
    std::vector<QPolygonF> polygons;
    QColor color;
};

struct ReceptorNode
{
    double x;
    double y;
    double zElev = 0;
    double zHill = 0;
    QPointF point;
    QColor color;
};

struct ReceptorGrid
{
    std::string netid; // length 8
    double xInit;
    double yInit;
    double xDelta;
    double yDelta;
    int xCount;
    int yCount;
    double zElev = 0;
    double zHill = 0;
    QPolygonF points;
    QColor color;
};

struct SourceGroup
{
    SourceGroup();

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

    void resetGeometry();

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
    std::vector<std::pair<double, int>> zones; // distance, hours

    // Flux Profile
    using FluxProfileDistribution = SamplingDistribution<std::shared_ptr<FluxProfile>>;
    FluxProfileDistribution fluxProfile;

    // Containers
    boost::ptr_vector<Source> sources;
    std::vector<ReceptorRing> rings;
    std::vector<ReceptorNode> nodes;
    std::vector<ReceptorGrid> grids;
};


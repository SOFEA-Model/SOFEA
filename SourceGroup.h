#pragma once

#include <string>
#include <utility>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include <QDateTime>
#include <QPointF>
#include <QPolygonF>
#include <QColor>

#include "GenericDistribution.h"
#include "Source.h"
#include "FluxScaling.h"

struct Scenario;

typedef std::vector<std::pair<double, int>> BufferZones;    // distance, hours
typedef std::vector<std::pair<int, double>> ReferenceFlux;  // hours, flux
typedef std::vector<std::pair<QDateTime, double>> Flux;     // time, flux

struct ReceptorRing
{
    std::string toString(int igrp, int iarc) const;
    std::string arcid; // length 8
    double buffer;
    double spacing;
    double zElev = 0;
    double zHill = 0;
    double zFlag = 0;
    QPolygonF points;
    std::vector<QPolygonF> polygons;
    QColor color;
};

struct ReceptorNode
{
    std::string toString() const;
    double x;
    double y;
    double zElev = 0;
    double zHill = 0;
    double zFlag = 0;
    QPointF point;
    QColor color;
};

struct ReceptorGrid
{
    std::string toString(int igrp, int inet) const;
    std::string netid; // length 8
    double xInit;
    double yInit;
    double xDelta;
    double yDelta;
    int xCount;
    int yCount;
    double zElev = 0;
    double zHill = 0;
    double zFlag = 0;
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
    void resampleAppStart();
    void resampleAppRate();
    void resampleIncorpDepth();
    void resampleAirDiffusion();
    void resampleWaterDiffusion();
    void resampleCuticularResistance();
    void resampleHenryConstant();
    void resetGeometry();
    Flux fluxProfile(const Source *s) const;

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
    QDateTime appStart; // FIXME
    GenericDistribution appRate;
    GenericDistribution incorpDepth;

    // Deposition
    GenericDistribution airDiffusion;
    GenericDistribution waterDiffusion;
    GenericDistribution cuticularResistance;
    GenericDistribution henryConstant;

    // Buffer Zones
    BufferZones zones;

    // Flux Profile
    ReferenceFlux refFlux;

    // Flux Scaling
    FluxScaling fluxScaling;

    // Containers
    boost::ptr_vector<Source> sources;
    std::vector<ReceptorRing> rings;
    std::vector<ReceptorNode> nodes;
    std::vector<ReceptorGrid> grids;
};

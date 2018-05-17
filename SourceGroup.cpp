#include "SourceGroup.h"

#include <cassert>
#include <cmath>
#include <utility>

#include "fmt/format.h"

SourceGroup::SourceGroup()
{
    static unsigned int sequenceNumber = 1;
    id = sequenceNumber;
    sequenceNumber++;

    grpid = fmt::format("Crop{:0=2}", id);
    appMethod = AppMethod::Other;
    appFactor = 1.0;
    validationMode = false;
    appStart = QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0));
    appRate = Distribution::Constant();
    incorpDepth = Distribution::Constant();
}

void SourceGroup::initSource(Source *s)
{
    s->appStart = appStart;
    s->appRate = appRate(&gen);
    s->incorpDepth = incorpDepth(&gen);
}

void SourceGroup::resampleAppStart()
{
    for (Source &s : sources)
        s.appStart = appStart; // FIXME
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

void SourceGroup::resetGeometry()
{
    for (Source &s : sources)
        s.setGeometry();
}

Flux SourceGroup::fluxProfile(const Source *s) const
{
    Flux res;

    // Expand the reference flux profile to one point per hour.
    std::vector<double> refFluxEx;
    for (const auto& xy : refFlux) {
        std::fill_n(std::back_inserter(refFluxEx), xy.first, xy.second);
    }

    // Determine the number of hours in the flux profile.
    int n = refFluxEx.size();

    // Calculate the scale factor.
    double sf = fluxScaling.fluxScaleFactor(s->appRate, s->appStart, s->incorpDepth);

    // Generate the flux profile.
    res.reserve(n);
    for (int t = 0; t < n; ++t) {
        QDateTime dt = s->appStart.addSecs(t * 60 * 60);
        res.push_back(std::make_pair(dt, refFluxEx[t] * sf));
    }

    return res;
}

std::string ReceptorRing::toString(int igrp, int iarc) const
{
    fmt::MemoryWriter w;
    w.write("** Distance = {}, Spacing = {}\n", buffer, spacing);
    for (const auto &p : points) {
        w.write("   EVALCART {: 10.2f} {: 10.2f}", p.x(), p.y());
        w.write("{: 6.1f} {: 6.1f} {: 6.1f} G{:0=3}R{:0=3}\n",
                zElev, zHill, zFlag, igrp, iarc);
    }
    w.write("\n");
    return w.str();
}

std::string ReceptorNode::toString() const
{
    fmt::MemoryWriter w;
    w.write("   DISCCART {: 10.2f} {: 10.2f}\n", x, y);
    return w.str();
}

std::string ReceptorGrid::toString(int igrp, int inet) const
{
    fmt::MemoryWriter w;
    w.write("   GRIDCART G{:0=3}C{:0=3} STA\n", igrp, inet);
    w.write("   GRIDCART G{:0=3}C{:0=3} XYINC ", igrp, inet);
    w.write("{} {} {} ",  xInit, xCount, xDelta);
    w.write("{} {} {}\n", yInit, yCount, yDelta);
    w.write("   GRIDCART G{:0=3}C{:0=3} END\n\n", igrp, inet);
    return w.str();
}

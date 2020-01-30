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

#include <string>
#include <utility>
#include <vector>

#include <QDateTime>
#include <QMetaType>

struct FluxProfile
{
    FluxProfile() :
        tsMethod(TSMethod::Disabled),
        dsMethod(DSMethod::Disabled),
        constantFlux(false),
        refStart(QDate(2000, 1, 1), QTime(0, 0, 0), Qt::UTC),
        refAppRate(0),
        refDepth(2.54),
        refVL(1),
        maxVL(1),
        warmSeasonStart(QDate::fromString("06/22/2000", "MM/dd/yyyy")),
        warmSeasonEnd(QDate::fromString("09/21/2000", "MM/dd/yyyy")),
        warmSeasonSF(1.6),
        amplitude(0.3),
        centerAmplitude(1.3),
        phase(270),
        wavelength(365.25)
    {}

    using ReferenceFlux = std::vector<std::pair<int, double>>;
    using GeneratedFlux = std::vector<std::pair<QDateTime, double>>;

    enum class TSMethod {
        Disabled,
        Seasonal,
        Sinusoidal
    };

    enum class DSMethod {
        Disabled,
        LinearCDPR,
        LinearGeneral,
        Nonlinear
    };

    // General
    std::string name;
    bool constantFlux;

    // Reference
    ReferenceFlux refFlux;
    double refAppRate;

    // Time Scaling
    TSMethod tsMethod;
    QDateTime refStart;
    QDate warmSeasonStart;
    QDate warmSeasonEnd;
    double warmSeasonSF;
    double amplitude;
    double centerAmplitude;
    double phase;
    double wavelength;

    // Depth Scaling
    DSMethod dsMethod;
    double refDepth;
    double refVL;
    double maxVL;

    inline double timeScaleFactor(QDateTime dt) const
    {
        if (!dt.isValid())
            return 1.0;

        switch (tsMethod) {
            case TSMethod::Disabled : {
                return 1.0;
            }
            case TSMethod::Seasonal : {
                QDate d = dt.date();
                QDate dr = refStart.date();
                QDate d0 = QDate(d.year(), warmSeasonStart.month(), warmSeasonStart.day());
                QDate d1 = QDate(d.year(), warmSeasonEnd.month(), warmSeasonEnd.day());

                if (!d0.isValid() || !d1.isValid()) {
                    return 1.0;
                }

                if ((d >= d0) && (d <= d1)) {
                    // application is in the warm season
                    if ((dr >= d0) && (dr <= d1)) {
                        // reference application in the warm season; scale down
                        return 1.0;
                    }
                    else {
                        // reference application in the cool season; scale up
                        return warmSeasonSF;
                    }
                }
                else {
                    // application is in the cool season
                    if ((dr >= d0) && (dr <= d1)) {
                        // reference application in the warm season; scale up
                        return warmSeasonSF;
                    }
                    else {
                        // reference application in the cool season; scale down
                        return 1.0;
                    }
                }
            }
            case TSMethod::Sinusoidal : {
                // QTime assumes 86400 seconds/day, no leap seconds
                double frac = static_cast<double>(dt.time().msecsSinceStartOfDay()) / 86400000.;
                double x = static_cast<double>(dt.date().dayOfYear()) + frac;

                double A = amplitude;
                double D = centerAmplitude;
                double phi = phase * (M_PI / 180.);
                double lambda = wavelength;

                double p = A * std::sin((2.0 * M_PI * x / lambda) + phi) + D;
                return p;
            }
            default: {
                return 1.0;
            }
        }
    }

    inline double depthScaleFactor(double depth) const
    {
        if ((refVL <= 0) || (refDepth <= 0))
            return 1.0;

        // Ensure minimum depth is 2.54 cm (1 in) to prevent errors
        // in slope calculations.

        depth = std::max(2.54, depth);

        // For clarity, some code is duplicated below.

        switch (dsMethod) {
            case DSMethod::Disabled : {
                return 1.0;
            }
            case DSMethod::LinearGeneral : {
                double slope = (maxVL - refVL) / -refDepth;
                double p = (slope * depth) + maxVL;
                p = std::min(std::max(p, 0.), 1.);
                double sf = p / refVL;
                return sf;
            }
            case DSMethod::LinearCDPR : {
                if (depth > refDepth) // critical depth
                    depth = refDepth;

                double slope = (maxVL - refVL) / -refDepth;
                double p = (slope * depth) + maxVL;
                p = std::min(std::max(p, 0.), 1.);
                double sf = p / refVL;
                return sf;
            }
            case DSMethod::Nonlinear : {
                double k = std::log(refVL / maxVL) / refDepth;
                double p = maxVL * std::exp(k * depth);
                p = std::min(std::max(p, 0.), 1.);
                double sf = p / refVL;
                return sf;
            }
            default: {
                return 1.0;
            }
        }
    }

    inline double fluxScaleFactor(double appRate, QDateTime dt, double depth) const
    {
        double sf;
        if (refAppRate > 0)
            sf = (appRate / refAppRate) * timeScaleFactor(dt) * depthScaleFactor(depth);
        else
            sf = timeScaleFactor(dt) * depthScaleFactor(depth);
        return sf;
    }

    int totalHours() const
    {
        const int sum = std::accumulate(refFlux.begin(), refFlux.end(), 0,
            [](auto i, const auto &xy) { return i + xy.first; });

        return sum;
    }

    GeneratedFlux expandedFlux(QDateTime appStart) const
    {
        // Expand the reference flux profile to one point per hour.
        std::vector<double> exRefFlux;
        for (const auto& xy : refFlux) {
            std::fill_n(std::back_inserter(exRefFlux), xy.first, xy.second);
        }

        // Determine the number of hours in the flux profile.
        std::size_t n = exRefFlux.size();

        // Generate the flux profile.
        GeneratedFlux flux;
        flux.reserve(n);
        for (int t = 0; t < n; ++t) {
            QDateTime dt = appStart.addSecs(t * 60 * 60);
            flux.push_back(std::make_pair(dt, exRefFlux[t]));
        }

        return flux;
    }

    GeneratedFlux scaledFlux(double appRate, QDateTime appStart, double incorpDepth) const
    {
        // Get the expanded reference flux profile.
        GeneratedFlux flux = expandedFlux(appStart);

        // Calculate and apply the scale factor.
        double sf = fluxScaleFactor(appRate, appStart, incorpDepth);
        for (auto&& xy : flux) {
            xy.second *= sf;
        }

        return flux;
    }

    double meanScaledFlux(double appRate, QDateTime appStart, double incorpDepth) const
    {
        GeneratedFlux flux = scaledFlux(appRate, appStart, incorpDepth);
        if (flux.empty())
            return 0;

        const double sum = std::accumulate(flux.begin(), flux.end(), 0.0,
            [](auto i, const auto &xy) { return i + xy.second; });

        return sum / flux.size();
    }
};

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

#include <cmath>
#include <memory>
#include <string>

#include <QDateTime>
#include <QMetaType>
#include <QPolygonF>
#include <QPen>
#include <QBrush>

#include "core/FluxProfile.h"

enum class SourceType
{
    POINT,
    POINTCAP,
    POINTHOR,
    VOLUME,
    AREA,
    AREAPOLY,
    AREACIRC,
    OPENPIT,
    LINE,
    BUOYLINE,
    RLINE,
    RLINEXT
};

Q_DECLARE_METATYPE(SourceType)

struct Source
{
    Source() {}
    virtual ~Source() {}

    double area() const;

    virtual void setGeometry() = 0;
    virtual std::string toString(std::size_t isrc) const = 0;
    virtual SourceType getType() const = 0;
    virtual Source* clone() const = 0;

    std::string srcid;
    double xs = 0;
    double ys = 0;
    double zs = 0;
    QDateTime appStart;
    double appRate = 0;
    double incorpDepth = 0;
    double airDiffusion = 0;
    double waterDiffusion = 0;
    double cuticularResistance = 0;
    double henryConstant = 0;
    QPolygonF geometry;
    QPen pen;
    QBrush brush;

    using FluxProfilePtr = std::weak_ptr<FluxProfile>;
    FluxProfilePtr fluxProfile;
};

// Implement clonable concept for boost::ptr_vector
inline Source* new_clone(const Source& c)
{
    return c.clone();
}

/****************************************************************************
** AREA Source
****************************************************************************/

struct AreaSource : Source
{
    void setGeometry() override;
    std::string toString(std::size_t isrc) const override;
    SourceType getType() const override {
        return SourceType::AREA;
    }

    double aremis = 1;
    double relhgt = 0;
    double xinit = 100;
    double yinit = 100;
    double angle = 0;
    double szinit = 0;

    AreaSource* clone() const override {
        return new AreaSource(*this);
    }
};

/****************************************************************************
** AREACIRC Source
****************************************************************************/

struct AreaCircSource : Source
{
    void setGeometry() override;
    std::string toString(std::size_t isrc) const override;
    SourceType getType() const override {
        return SourceType::AREACIRC;
    }

    double aremis = 1;
    double relhgt = 0;
    double radius = 100;
    int nverts = 20;
    double szinit = 0;

    AreaCircSource* clone() const override {
        return new AreaCircSource(*this);
    }
};

/****************************************************************************
** AREAPOLY Source
****************************************************************************/

struct AreaPolySource : Source
{
    using Source::Source;

    void setGeometry() override;
    std::string toString(std::size_t isrc) const override;
    SourceType getType() const override {
        return SourceType::AREAPOLY;
    }

    double aremis = 1;
    double relhgt = 0;
    double szinit = 0;

    AreaPolySource* clone() const override {
        return new AreaPolySource(*this);
    }
};



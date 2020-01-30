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

#include <QPointF>
#include <QPolygonF>
#include <vector>

class GeosOp
{
public:
    GeosOp();
    ~GeosOp();

    bool isValid(const QPolygonF& polygon);
    bool overlaps(const QPolygonF& polygon1, const QPolygonF& polygon2);
    QPointF interiorPoint(const QPolygonF& polygon);
    std::vector<QPolygonF> unionCascaded(const std::vector<QPolygonF>& mpolygon);
    std::vector<QPolygonF> buffer(std::vector<QPolygonF> const& mpolygon, double distance, int points_per_circle);
    std::vector<QPolygonF> measurePoints(std::vector<QPolygonF> const& mpolygon, double spacing);
};

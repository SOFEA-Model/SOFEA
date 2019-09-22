#pragma once

#include <QPointF>
#include <QPolygonF>
#include <vector>

class GeosOp
{
public:
    GeosOp();
    ~GeosOp();

    QPointF interiorPoint(const QPolygonF& polygon);
    std::vector<QPolygonF> unionCascaded(const std::vector<QPolygonF>& mpolygon);
    std::vector<QPolygonF> buffer(std::vector<QPolygonF> const& mpolygon, double distance, int points_per_circle);
    std::vector<QPolygonF> measurePoints(std::vector<QPolygonF> const& mpolygon, double spacing);
};

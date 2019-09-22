#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <tuple>

#include <QPointF>
#include <QPolygonF>
#include <QColor>

#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/variant.hpp>

#include "GeosOp.h"

struct SourceGroup;

struct ReceptorNode
{
    double x;
    double y;
    mutable double zElev = 0;
    mutable double zHill = 0;
    mutable double zFlag = 0;

    QPointF point() const {
        return QPointF{x, y};
    }

    friend bool operator<(const ReceptorNode& lhs, const ReceptorNode& rhs) {
        return std::tie(lhs.x, lhs.y) < std::tie(rhs.x, rhs.y);
    }
};

// Discrete receptors are stored in a std::set in order to prevent duplicates
// and provide automatic sorting. boost::container::flat_set is an alternative,
// which would provide faster lookup and iteration at the cost of slower
// insertions.

struct ReceptorNodeGroup
{
    ReceptorNodeGroup();

    std::string grpid; // arcid or netid; length 8
    QColor color;
    std::set<ReceptorNode> nodes;

    bool setZElev(int index, double zElev);
    bool setZHill(int index, double zHill);
    bool setZFlag(int index, double zFlag);
    bool removeNodes(int start, int count);
    ReceptorNode getNode(int index) const;
    std::size_t nodeCount() const;
    std::string format() const;
};

// Receptor rings are stored in a std::vector, to preserve the order of
// receptors generated in the linear referencing calculations and provide fast
// insertion. A std::weak_ptr is used to track the source group, as it could be
// removed after receptor rings are generated.

struct ReceptorRingGroup
{
    ReceptorRingGroup();
    explicit operator ReceptorNodeGroup() const; // conversion operator

    std::string grpid; // arcid or netid; length 8
    QColor color;
    std::weak_ptr<SourceGroup> sgPtr;
    double buffer = 100;
    double spacing = 100;
    std::vector<QPolygonF> polygons;
    std::vector<ReceptorNode> nodes;

    bool setSourceGroup(std::shared_ptr<SourceGroup> sg);
    bool setBuffer(double value);
    bool setSpacing(double value);
    bool setZElev(int index, double zElev);
    bool setZHill(int index, double zHill);
    bool setZFlag(int index, double zFlag);
    bool removeNodes(int start, int count);
    bool updateGeometry();
    ReceptorNode getNode(int index) const;
    std::size_t nodeCount() const;
    QPolygonF points() const;
    std::string format() const;
};

// Cartesian grid receptor geometry is generated on-demand. Elevations, which
// may be set for individual receptors, use compressed row storage
// (boost::numeric::ublas::compressed_matrix). This reduces project file size
// and provides a convenient method to write the receptor grid specification to
// the AERMOD runstream using shorthand notation, e.g. "ELEV 1 8*10."

struct ReceptorGridGroup
{
    ReceptorGridGroup();
    explicit operator ReceptorNodeGroup() const; // conversion operator

    std::string grpid; // arcid or netid; length 8
    QColor color;
    double xInit = 0;
    double yInit = 0;
    int xCount = 0;
    int yCount = 0;
    double xDelta = 100.0;
    double yDelta = 100.0;

    boost::numeric::ublas::compressed_matrix<double> zElevM;
    boost::numeric::ublas::compressed_matrix<double> zHillM;
    boost::numeric::ublas::compressed_matrix<double> zFlagM;

    bool setDimensions(int nrows, int ncols);
    bool setOrigin(double x, double y);
    bool setSpacing(double x, double y);
    bool setZElev(int i, int j, double zElev);
    bool setZElev(int index, double zElev);
    bool setZHill(int i, int j, double zHill);
    bool setZHill(int index, double zHill);
    bool setZFlag(int i, int j, double zFlag);
    bool setZFlag(int index, double zFlag);
    bool removeNodes(int start, int count);
    ReceptorNode getNode(int index) const;
    std::size_t nodeCount() const;
    QPolygonF points() const;
    std::string format() const;
};

using ReceptorGroup = boost::variant<
    ReceptorNodeGroup,
    ReceptorRingGroup,
    ReceptorGridGroup>;


// DEPRECIATED
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

    friend bool operator<(const ReceptorRing& lhs, const ReceptorRing& rhs) {
        return lhs.buffer < rhs.buffer;
    }
};

// DEPRECIATED
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
    QColor color;

    QPolygonF geometry() const {
        QPolygonF result;
        result.reserve(xCount * yCount);
        for (int i=0; i < xCount; ++i) {
            for (int j=0; j < yCount; ++j) {
                double x = xInit + (xDelta * i);
                double y = yInit + (yDelta * j);
                result.push_back(QPointF{x, y});
            }
        }
        return result;
    }

    friend bool operator<(const ReceptorGrid& lhs, const ReceptorGrid& rhs) {
        return std::tie(lhs.xInit, lhs.yInit, lhs.xDelta, lhs.yDelta, lhs.xCount, lhs.yCount) <
               std::tie(rhs.xInit, rhs.yInit, rhs.xDelta, rhs.yDelta, rhs.xCount, rhs.yCount);
    }
};

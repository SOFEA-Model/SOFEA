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

#include "Receptor.h"
#include "SourceGroup.h"
#include "GeosOp.h"

#include <QDebug>

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <fmt/format.h>

//-----------------------------------------------------------------------------
// ReceptorNodeGroup
//-----------------------------------------------------------------------------

ReceptorNodeGroup::ReceptorNodeGroup()
{}

bool ReceptorNodeGroup::setZElev(int index, double zElev)
{
    if (index < 0 || index >= nodes.size())
        return false;

    auto it = std::next(nodes.begin(), index);
    it->zElev = zElev;
    return true;
}

bool ReceptorNodeGroup::setZHill(int index, double zHill)
{
    if (index < 0 || index >= nodes.size())
        return false;

    auto it = std::next(nodes.begin(), index);
    it->zHill = zHill;
    return true;
}

bool ReceptorNodeGroup::setZFlag(int index, double zFlag)
{
    if (index < 0 || index >= nodes.size())
        return false;

    auto it = std::next(nodes.begin(), index);
    it->zFlag = zFlag;
    return true;
}

bool ReceptorNodeGroup::removeNodes(int start, int count)
{
    if (start < 0 || count < 0 || start + count > nodeCount())
        return false;

    auto it = std::next(nodes.begin(), start);
    for (int i = 0; i < count; ++i)
        it = nodes.erase(it);

    return true;
}

ReceptorNode ReceptorNodeGroup::getNode(int index) const
{
    if (index < 0 || index > nodeCount())
        return ReceptorNode();

    return *std::next(nodes.begin(), index);
}

std::size_t ReceptorNodeGroup::nodeCount() const
{
    return nodes.size();
}

std::string ReceptorNodeGroup::format() const
{
    fmt::memory_buffer w;

    fmt::format_to(w, "** Discrete Receptor Group {}\n", grpid);
    for (const auto& node : nodes) {
        fmt::format_to(w, "   EVALCART {: 10.2f} {: 10.2f} ", node.x, node.y);
        fmt::format_to(w, "{:>6.2f} {:>6.2f} {:>6.2f} {}\n",
                node.zElev, node.zHill, node.zFlag, grpid);
    }

    return fmt::to_string(w);
}

//-----------------------------------------------------------------------------
// ReceptorRingGroup
//-----------------------------------------------------------------------------

ReceptorRingGroup::ReceptorRingGroup()
{}

ReceptorRingGroup::operator ReceptorNodeGroup() const
{
    ReceptorNodeGroup ng;
    ng.grpid = grpid;
    ng.color = color;
    ng.nodes.insert(this->nodes.begin(), this->nodes.end());
    return ng;
}

bool ReceptorRingGroup::setSourceGroup(std::shared_ptr<SourceGroup> sg)
{
    sgPtr = sg;
    return true;
}

bool ReceptorRingGroup::setBuffer(double value)
{
    if (value < 0)
        return false;

    buffer = value;
    return true;
}

bool ReceptorRingGroup::setSpacing(double value)
{
    if (value < 0)
        return false;

    spacing = value;
    return true;
}

bool ReceptorRingGroup::setZElev(int index, double zElev)
{
    if (index < 0 || index >= nodes.size())
        return false;

    ReceptorNode& n = nodes.at(index);
    n.zElev = zElev;
    return true;
}

bool ReceptorRingGroup::setZHill(int index, double zHill)
{
    if (index < 0 || index >= nodes.size())
        return false;

    ReceptorNode& n = nodes.at(index);
    n.zHill = zHill;
    return true;
}

bool ReceptorRingGroup::setZFlag(int index, double zFlag)
{
    if (index < 0 || index >= nodes.size())
        return false;

    ReceptorNode& n = nodes.at(index);
    n.zFlag = zFlag;
    return true;
}

bool ReceptorRingGroup::removeNodes(int start, int count)
{
    if (start != 0 && count != nodeCount())
        return false;

    nodes.clear();
    polygons.clear();
    return true;
}

bool ReceptorRingGroup::updateGeometry()
{
    // TODO: cache the rings if source group geometry has not changed.

    const std::shared_ptr<SourceGroup> sg = sgPtr.lock();
    if (!sg)
        return false;

    if (spacing == 0)
        return false;

    nodes.clear();
    polygons.clear();

    // Create the source mpolygon for the buffer.
    std::vector<QPolygonF> source_mpolygon;

    for (const auto &s : sg->sources)
        source_mpolygon.push_back(s.geometry);

    // Calculate number of points in circle using receptor spacing as chord length.
    double thetaR = 2.0 * std::asin(spacing / (2.0 * buffer));
    double thetaD = thetaR * (180.0 / M_PI);
    int npc = (std::max)(static_cast<int>(ceil(360.0 / thetaD)), 8);

    // Calculate the buffer and generate receptors along ring.
    GeosOp geosOp;
    std::vector<QPolygonF> union_mpolygon = geosOp.unionCascaded(source_mpolygon);
    std::vector<QPolygonF> buffer_mpolygon = geosOp.buffer(union_mpolygon, buffer, npc);
    std::vector<QPolygonF> points = geosOp.measurePoints(buffer_mpolygon, spacing);

    for (const QPolygonF &p : buffer_mpolygon) {
        polygons.push_back(p);
    }

    for (const QPolygonF &p : points) {
        for (const QPointF &pt : p) {
            ReceptorNode n;
            n.x = pt.x();
            n.y = pt.y();
            n.zElev = 0;
            n.zHill = 0;
            n.zFlag = 0;
            nodes.push_back(n);
        }
    }

    return true;
}

ReceptorNode ReceptorRingGroup::getNode(int index) const
{
    if (index < 0 || index > nodeCount())
        return ReceptorNode();

    return nodes.at(index);
}

std::size_t ReceptorRingGroup::nodeCount() const
{
    return nodes.size();
}

QPolygonF ReceptorRingGroup::points() const
{
    QPolygonF result;
    result.reserve(nodeCount());

    for (const auto& node : nodes) {
        result.push_back(node.point());
    }

    return result;
}

std::string ReceptorRingGroup::format() const
{
    fmt::memory_buffer w;

    fmt::format_to(w, "** Receptor Ring {}\n", grpid);
    fmt::format_to(w, "** Distance = {}, Spacing = {}\n", buffer, spacing);
    for (const auto& node : nodes) {
        fmt::format_to(w, "   EVALCART {: 10.2f} {: 10.2f} ", node.x, node.y);
        fmt::format_to(w, "{:>6.2f} {:>6.2f} {:>6.2f} {}\n",
            node.zElev, node.zHill, node.zFlag, grpid);
    }

    return fmt::to_string(w);
}

//-----------------------------------------------------------------------------
// ReceptorGridGroup
//-----------------------------------------------------------------------------

ReceptorGridGroup::ReceptorGridGroup()
{}

ReceptorGridGroup::operator ReceptorNodeGroup() const
{
    ReceptorNodeGroup ng;
    ng.grpid = grpid;
    ng.color = color;
    for (int i=0; i < xCount; ++i) {
        for (int j=0; j < yCount; ++j) {
            ReceptorNode node;
            node.x = xInit + (xDelta * i);
            node.y = yInit + (yDelta * j);
            node.zElev = zElevM(i, j);
            node.zHill = zHillM(i, j);
            node.zFlag = zFlagM(i, j);
            ng.nodes.insert(node);
        }
    }
    return ng;
}

bool ReceptorGridGroup::setDimensions(int nrows, int ncols)
{
    if (nrows < 0 || ncols < 0)
        return false;

    xCount = nrows;
    yCount = ncols;

    if (nrows <= 0 || ncols <= 0) {
        zElevM.clear();
        zHillM.clear();
        zFlagM.clear();
    }
    else {
        zElevM.resize(nrows, ncols);
        zHillM.resize(nrows, ncols);
        zFlagM.resize(nrows, ncols);
    }

    return true;
}

bool ReceptorGridGroup::setOrigin(double x, double y)
{
    xInit = x;
    yInit = y;
    return true;
}

bool ReceptorGridGroup::setSpacing(double x, double y)
{
    if (x <= 0 || y <= 0)
        return false;

    xDelta = x;
    yDelta = y;
    return true;
}

bool ReceptorGridGroup::setZElev(int i, int j, double zElev)
{
    if (i < 0 || j < 0 || i >= xCount || j >= yCount)
        return false;

    zElevM(i, j) = zElev;
    return true;
}

bool ReceptorGridGroup::setZElev(int index, double zElev)
{
    if (index < 0 || index >= nodeCount())
        return false;

    int j = index % yCount;
    int i = (index - j) / yCount;
    return setZElev(i, j, zElev);
}

bool ReceptorGridGroup::setZHill(int i, int j, double zHill)
{
    if (i < 0 || j < 0 || i >= xCount || j >= yCount)
        return false;

    zHillM(i, j) = zHill;
    return true;
}

bool ReceptorGridGroup::setZHill(int index, double zHill)
{
    if (index < 0 || index >= nodeCount())
        return false;

    int j = index % yCount;
    int i = (index - j) / yCount;
    return setZHill(i, j, zHill);
}

bool ReceptorGridGroup::setZFlag(int i, int j, double zFlag)
{
    if (i < 0 || j < 0 || i >= xCount || j >= yCount)
        return false;

    zFlagM(i, j) = zFlag;
    return true;
}

bool ReceptorGridGroup::setZFlag(int index, double zFlag)
{
    if (index < 0 || index >= nodeCount())
        return false;

    int j = index % yCount;
    int i = (index - j) / yCount;
    return setZFlag(i, j, zFlag);
}

bool ReceptorGridGroup::removeNodes(int start, int count)
{
    if (start != 0 && count != nodeCount())
        return false;

    setDimensions(0, 0);
    return true;
}

ReceptorNode ReceptorGridGroup::getNode(int index) const
{
    // This is a helper function to work with ReceptorModel,
    // since the other receptor types use ReceptorNode.

    if (nodeCount() == 0 || index < 0 || index > nodeCount())
        return ReceptorNode();

    int j = index % yCount;
    int i = (index - j) / yCount;

    ReceptorNode node;
    node.x = xInit + (xDelta * i);
    node.y = yInit + (yDelta * j);
    node.zElev = zElevM(i, j);
    node.zHill = zHillM(i, j);
    node.zFlag = zFlagM(i, j);
    return node;
}

std::size_t ReceptorGridGroup::nodeCount() const
{
    return xCount * yCount;
}

QPolygonF ReceptorGridGroup::points() const
{
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

void formatMatrix(fmt::memory_buffer& w, const std::string& keyword, const std::string& grpid,
                  const boost::numeric::ublas::compressed_matrix<double>& m)
{
    namespace ublas = boost::numeric::ublas;

    for (std::size_t i = 0; i < m.size1(); ++i) {
        // Row numbers start at 1, and increase with the y-coordinate.
        fmt::format_to(w, "   GRIDCART {0:<8} {1:} {2:<5}", keyword, grpid, i + 1);

        std::size_t ncols = m.size2();
        ublas::compressed_vector<double> v(ncols);
        ublas::noalias(v) = ublas::row(m, i);
        std::size_t nnz = v.nnz(); // number of non-zero entries

        // Write contiguous values in shorthand format.
        std::size_t repeat = 1;
        for (std::size_t j = 1; j < ncols; ++j) {
            if (nnz == 0) {
                std::size_t nz = ncols - nnz;
                fmt::format_to(w, "{0:>5}*{1:<6.2f}", nz, 0.0);
                break;
            }

            double first = std::as_const(v)[j - 1];
            double second = std::as_const(v)[j];

            if (first == second) {
                ++repeat;
            }
            else {
                fmt::format_to(w, "{0:>5}*{1:<6.2f}", repeat, first);
                repeat = 1;
            }

            if (j == ncols - 1) {
                fmt::format_to(w, "{0:>5}*{1:<6.2f}", repeat, second);
                break;
            }
        }

        fmt::format_to(w, "\n");
    }
}

std::string ReceptorGridGroup::format() const
{
    if (xCount <= 0 || yCount <= 0)
        return std::string();

    fmt::memory_buffer w;

    fmt::format_to(w, "** Cartesian Grid {}\n", grpid);
    fmt::format_to(w, "   GRIDCART {:<8} STA\n", grpid);
    fmt::format_to(w, "   GRIDCART {:<8} XYINC ", grpid);
    fmt::format_to(w, "{} {} {} ",  xInit, xCount, xDelta);
    fmt::format_to(w, "{} {} {}\n", yInit, yCount, yDelta);
    formatMatrix(w, "ELEV", grpid, zElevM);
    formatMatrix(w, "HILL", grpid, zHillM);
    formatMatrix(w, "FLAG", grpid, zFlagM);
    fmt::format_to(w, "   GRIDCART {:<8} END\n", grpid);

    return fmt::to_string(w);
}

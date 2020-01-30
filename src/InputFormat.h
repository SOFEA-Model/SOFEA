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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <fmt/format.h>

#include "Receptor.h"

namespace fmt {

template <>
struct formatter<ReceptorNodeGroup>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const ReceptorNodeGroup& group, FormatContext& ctx)
    {
        auto it = format_to(ctx.out(),
            "** Discrete Receptor Group {}\n", group.grpid);

        for (const auto& node : group.nodes) {
            it = format_to(ctx.out(),
                "   EVALCART {: 10.2f} {: 10.2f} {:>6.2f} {:>6.2f} {:>6.2f} {}\n",
                node.x, node.y, node.zElev, node.zHill, node.zFlag, group.grpid);
        }

        return it;
    }
};

template <>
struct formatter<ReceptorRingGroup>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const ReceptorRingGroup& group, FormatContext& ctx)
    {
        auto it = format_to(ctx.out(),
            "** Receptor Ring {}\n** Distance = {}, Spacing = {}\n",
            group.grpid, group.buffer, group.spacing);

        for (const auto& node : group.nodes) {
            it = format_to(ctx.out(),
                "   EVALCART {: 10.2f} {: 10.2f} {:>6.2f} {:>6.2f} {:>6.2f} {}\n",
                node.x, node.y, node.zElev, node.zHill, node.zFlag, group.grpid);
        }

        return it;
    }
};

template <>
struct formatter<ReceptorGridGroup>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format_matrix(FormatContext& ctx, const std::string& keyword, const std::string& grpid,
                       const boost::numeric::ublas::compressed_matrix<double>& m)
    {
        namespace ublas = boost::numeric::ublas;

        auto it = ctx.out();
        for (std::size_t i = 0; i < m.size1(); ++i) {
            // Row numbers start at 1, and increase with the y-coordinate.
            it = fmt::format_to(ctx.out(), "   GRIDCART {0:<8} {1:<5} {2:<5}", grpid, keyword, i + 1);

            std::size_t ncols = m.size2();
            ublas::compressed_vector<double> v(ncols);
            ublas::noalias(v) = ublas::row(m, i);
            std::size_t nnz = v.nnz(); // number of non-zero entries

            // Write contiguous values in shorthand format.
            std::size_t repeat = 1;
            for (std::size_t j = 1; j < ncols; ++j) {
                if (nnz == 0) {
                    std::size_t nz = ncols - nnz;
                    it = fmt::format_to(ctx.out(), "{0:>5}*{1:<6.2f}", nz, 0.0);
                    break;
                }

                double first = std::as_const(v)[j - 1];
                double second = std::as_const(v)[j];

                if (first == second) {
                    ++repeat;
                }
                else {
                    it = fmt::format_to(ctx.out(), "{0:>5}*{1:<6.2f}", repeat, first);
                    repeat = 1;
                }

                if (j == ncols - 1) {
                    it = fmt::format_to(ctx.out(), "{0:>5}*{1:<6.2f}", repeat, second);
                    break;
                }
            }

            it = fmt::format_to(ctx.out(), "\n");
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const ReceptorGridGroup& group, FormatContext& ctx)
    {
        if (group.xCount <= 0 || group.yCount <= 0)
            return ctx.out();

        auto it = format_to(ctx.out(),
            "** Cartesian Grid {0}\n"
            "   GRIDCART {0:<8} STA\n"
            "   GRIDCART {0:<8} XYINC {1} {2} {3} {4} {5} {6}\n",
            group.grpid,
            group.xInit, group.xCount, group.xDelta,
            group.yInit, group.yCount, group.yDelta);
        it = format_matrix(ctx, "ELEV", group.grpid, group.zElevM);
        it = format_matrix(ctx, "HILL", group.grpid, group.zHillM);
        it = format_matrix(ctx, "FLAG", group.grpid, group.zFlagM);
        it = format_to(ctx.out(),
            "   GRIDCART {0:<8} END\n", group.grpid);

        return it;
    }
};

template <>
struct formatter<ReceptorGroup>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FormatContext>
    struct group_format_visitor {
        explicit group_format_visitor(FormatContext& ctx)
            : ctx_(ctx)
        {}

        template <typename T>
        auto operator()(const T& group) {
            return format_to(ctx_.out(), "{}", group);
        }

        FormatContext& ctx_;
    };

    template <typename FormatContext>
    auto format(const ReceptorGroup& variant, FormatContext& ctx) {
        return boost::apply_visitor(group_format_visitor(ctx), variant);
    }
};

} // namespace fmt

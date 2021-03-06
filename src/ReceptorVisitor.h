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

#include <QColor>
#include <QRectF>
#include <QString>
#include <QVariant>

#include <string>

#include <boost/variant.hpp>

#include "core/Receptor.h"

struct ReceptorGroupColorVisitor
{
    template <typename T>
    QColor operator()(const T& group) {
        return group.color;
    }
};

struct ReceptorGroupNameVisitor
{
    template <typename T>
    std::string operator()(const T& group) const {
        return group.grpid;
    }
};

struct ReceptorGroupTypeVisitor
    : public boost::static_visitor<ReceptorGroupType>
{
    ReceptorGroupType operator()(const ReceptorNodeGroup&) const {
        return ReceptorGroupType::Node;
    }
    ReceptorGroupType operator()(const ReceptorRingGroup&) const {
        return ReceptorGroupType::Ring;
    }
    ReceptorGroupType operator()(const ReceptorGridGroup&) const {
        return ReceptorGroupType::Grid;
    }
};

struct ReceptorGroupNodeVisitor
{
    explicit ReceptorGroupNodeVisitor(int index)
        : index_(index)
    {}

    template <typename T>
    ReceptorNode operator()(const T& group) const {
        return group.getNode(index_);
    }

    const int index_;
};

struct ReceptorNodeCountVisitor
{
    template <typename T>
    std::size_t operator()(const T& group) const {
        return group.nodeCount();
    }
};

struct ReceptorGroupRectVisitor
{
    template <typename T>
    QRectF operator()(const T& group) const {
        return group.boundingRect();
    }
};

struct SetReceptorGroupColor
{
    explicit SetReceptorGroupColor(const QColor& color)
        : color_(color)
    {}

    template <typename T>
    bool operator()(T& group) const {
        group.color = color_;
        return true;
    }

    const QColor& color_;
};

struct SetReceptorGroupName
{
    explicit SetReceptorGroupName(const std::string& grpid)
        :  grpid_(grpid)
    {}

    template <typename T>
    bool operator()(T& group) const {
        group.grpid = grpid_;
        return true;
    }

    const std::string& grpid_;
};

struct SetReceptorZElev
{
    explicit SetReceptorZElev(int index, double zElev)
        : index_(index), zElev_(zElev)
    {}

    template <typename T>
    bool operator()(T& group) const {
        return group.setZElev(index_, zElev_);
    }

    const int index_;
    const double zElev_;
};

struct SetReceptorZHill
{
    explicit SetReceptorZHill(int index, double zHill)
        : index_(index), zHill_(zHill)
    {}

    template <typename T>
    bool operator()(T& group) const {
        return group.setZHill(index_, zHill_);
    }

    const int index_;
    const double zHill_;
};

struct SetReceptorZFlag
{
    explicit SetReceptorZFlag(int index, double zFlag)
        : index_(index), zFlag_(zFlag)
    {}

    template <typename T>
    bool operator()(T& group) const {
        return group.setZFlag(index_, zFlag_);
    }

    const int index_;
    const double zFlag_;
};

struct RemoveReceptorGroupNodes
{
    explicit RemoveReceptorGroupNodes(int start, int count)
        : start_(start), count_(count)
    {}

    bool operator()(ReceptorNodeGroup& group) const {
        return group.removeNodes(start_, count_);
    }

    bool operator()(ReceptorRingGroup& group) const {
        // TODO: Allow removing receptors, convert to ReceptorNodeGroup.
        return group.removeNodes(start_, count_);
    }

    bool operator()(ReceptorGridGroup& group) const {
        // TODO: Allow removing receptors, convert to ReceptorNodeGroup.
        return group.removeNodes(start_, count_);
    }

    const int start_;
    const int count_;
};


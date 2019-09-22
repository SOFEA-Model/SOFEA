#pragma once

#include <QColor>
#include <QString>
#include <QVariant>

#include <string>

#include <boost/variant.hpp>

#include "Receptor.h"


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
    : public boost::static_visitor<QString>
{
    QString operator()(const ReceptorNodeGroup&) const {
        return QString("Discrete");
    }
    QString operator()(const ReceptorRingGroup&) const {
        return QString("Ring");
    }
    QString operator()(const ReceptorGridGroup&) const {
        return QString("Grid");
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


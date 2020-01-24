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

#ifdef _MSC_VER
#pragma warning(disable:4267) // Level 3, C4267: conversion from 'size_t' to 'int'
#endif

#include <functional> // std::less
#include <string>
#include <sstream>
#include <utility>

#include <QDateTime>

#include <boost/bind.hpp>
#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

// Workaround to disable perfect forwarding for Boost.Variant derived class.
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
#define BOOST_NO_CXX11_RVALUE_REFERENCES
#include <boost/variant.hpp>
#undef BOOST_NO_CXX11_RVALUE_REFERENCES
#endif

// Combine functor to overwrite values on set union.
// See: https://lists.boost.org/boost-users/2015/11/85298.php

namespace boost {
namespace icl {
template <typename Type> struct inplace_assign
    : public boost::icl::identity_based_inplace_combine<Type>
{
    typedef inplace_assign<Type> type;
    void operator()(Type& object, const Type& operand) const {
        object = operand;
    }
};
} // namespace icl
} // namespace boost

typedef boost::icl::interval_map<
    boost::posix_time::ptime,     // DomainT
    double,                       // CodomainT
    boost::icl::partial_absorber, // Traits
    std::less,                    // Compare
    boost::icl::inplace_assign    // Combine
> IntervalMap;

namespace Distribution
{
using ConstantDateTime = QDateTime;

struct DiscreteDateTime : public IntervalMap
{
    using IntervalMap::IntervalMap;

    bool operator==(const DiscreteDateTime& rhs) const {
        return ((IntervalMap)*this == (IntervalMap)rhs);
    }

    bool operator!=(const DiscreteDateTime& rhs) const {
        return ((IntervalMap)*this != (IntervalMap)rhs);
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        size_t is = this->iterative_size();
        ar(is);
        for(auto it = begin(); it != end(); ++it) {
            value_type row = *it;
            auto bounds = row.first.bounds().bits();
            domain_type lower = row.first.lower();
            domain_type upper = row.first.upper();
            codomain_type value = row.second;
            ar(bounds, lower, upper, value);
        }
    }

    template<class Archive>
    void load(Archive& ar)
    {
        clear();
        size_t is;
        ar(is);
        size_t index = is;
        while (index--) {
            value_type row;
            auto bounds = row.first.bounds().bits();
            domain_type lower, upper;
            codomain_type value;
            ar(bounds, lower, upper, value);
            auto di = boost::icl::discrete_interval<domain_type>(
                lower, upper, boost::icl::interval_bounds(bounds));
            insert(end(), std::make_pair(di, value));
        }
    }

    template <typename Engine>
    QDateTime operator()(const Engine& rng)
    {
        using namespace boost::random;
        using namespace boost::gregorian;
        using namespace boost::posix_time;
        using namespace boost::icl;

        if (empty())
            return QDateTime();

        // Extract the probabilities from the interval_map.
        std::vector<discrete_interval<ptime>> intervals;
        std::vector<double> probabilities;

        intervals.reserve(this->iterative_size());
        probabilities.reserve(this->iterative_size());

        for(auto it = begin(); it != end(); ++it) {
            intervals.push_back(it->first);
            probabilities.push_back(it->second);
        }

        // Select the interval_map index.
        discrete_distribution<> index_dist(probabilities.begin(), probabilities.end());
        variate_generator<Engine, discrete_distribution<>> index_vgen(rng, index_dist);
        int index = index_vgen();

        // Extract the selected interval and calculate duration.
        discrete_interval<ptime> i = intervals.at(index);
        ptime lower = i.lower();
        ptime upper = i.upper();
        int nhours = (upper - lower).hours();

        // Sample from open range.
        uniform_int_distribution<> hours_dist(0, nhours - 1);
        variate_generator<Engine, uniform_int_distribution<>> hours_vgen(rng, hours_dist);
        ptime sample = lower + hours(hours_vgen());

        // Return value as QDateTime.
        date gd = sample.date();
        time_duration td = sample.time_of_day();
        return QDateTime(QDate(gd.year(), gd.month(), gd.day()),
                         QTime(td.hours(), 0, 0), Qt::UTC);
    }
};

using DateTimeVariant = boost::variant<ConstantDateTime, DiscreteDateTime>;

// Static visitor to call random variate generators.
struct DateTimeVisitor : boost::static_visitor<QDateTime> {
    template <typename Engine>
    QDateTime operator()(ConstantDateTime& d, const Engine& rng) const {
        return d;
    }
    template <typename Engine>
    QDateTime operator()(DiscreteDateTime& d, const Engine& rng) const {
        return d(rng);
    }
};

} // namespace Distribution


class DateTimeDistribution : public Distribution::DateTimeVariant
{
    using Base = Distribution::DateTimeVariant;
public:
    using Base::Base;

    bool operator==(const DateTimeDistribution& rhs) const {
        return Base::operator==(static_cast<const Base&>(rhs));
    }
    bool operator!=(const DateTimeDistribution& rhs) const {
        return Base::operator!=(static_cast<const Base&>(rhs));
    }

    template <typename Engine>
    QDateTime operator()(const Engine& rng) {
        auto f = boost::bind(Distribution::DateTimeVisitor(), ::_1, rng);
        return boost::apply_visitor(f, *this);
    }
};

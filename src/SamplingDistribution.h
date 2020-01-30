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

#include <map>
#include <memory>
#include <vector>

#include <boost/random/discrete_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>

template <class T, class Compare = std::owner_less<T>>
struct SamplingDistribution
{
    using ProbabilityMap = std::map<T, double, Compare>;

    SamplingDistribution() {}

    SamplingDistribution(const ProbabilityMap& map)
        : data(map) {}

    bool operator==(const SamplingDistribution& rhs) const {
        return (data == rhs.data);
    }

    bool operator!=(const SamplingDistribution& rhs) const {
        return (data != rhs.data);
    }

    template <class Archive>
    void save(Archive& ar) const {
        ar(data);
    }

    template<class Archive>
    void load(Archive& ar) {
        ar(data);
    }

    template<class Engine>
    T operator()(const Engine& rng)
    {
        using namespace boost::random;

        if (data.empty())
            return T();

        std::vector<T> keys;
        std::vector<double> probabilities;

        keys.reserve(data.size());
        probabilities.reserve(data.size());

        // If there is only one value, return it.
        if (data.size() == 1) {
            auto it = data.begin();
            return it->first;
        }

        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it->second > 0) {
                keys.push_back(it->first);
                probabilities.push_back(it->second);
            }
        }

        // Return default if probabilities are zero.
        if (probabilities.empty())
            return T();

        // Select the map index.
        discrete_distribution<> index_dist(probabilities.begin(), probabilities.end());
        variate_generator<Engine, discrete_distribution<>> index_vgen(rng, index_dist);
        int index = index_vgen();

        return keys.at(index);
    }

    ProbabilityMap data;
};

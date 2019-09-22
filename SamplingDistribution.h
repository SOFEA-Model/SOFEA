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

#pragma once

#ifdef _MSC_VER
#pragma warning(disable:4267) // Level 3, C4267: conversion from 'size_t' to 'const int'
#endif

#include <string>
#include <sstream>

#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/beta_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/chi_squared_distribution.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/extreme_value_distribution.hpp>
#include <boost/random/fisher_f_distribution.hpp>
#include <boost/random/gamma_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/laplace_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/negative_binomial_distribution.hpp>
#include <boost/random/non_central_chi_squared_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/piecewise_constant_distribution.hpp>
#include <boost/random/piecewise_linear_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/student_t_distribution.hpp>
#include <boost/random/triangle_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/weibull_distribution.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

#include <boost/bind.hpp>
#include <boost/mpl/list.hpp>
#include <boost/variant.hpp>

#include <QVector>

// Internal split minimal serialization functions
#define DISTRIBUTION_SERIALIZATION                                      \
    template <class Archive>                                            \
    std::string save_minimal(Archive const&) const {                    \
        std::ostringstream oss;                                         \
        oss << *this;                                                   \
        return oss.str();                                               \
    }                                                                   \
    template <class Archive>                                            \
    void load_minimal(Archive const&, std::string const& value) {       \
        std::istringstream iss(value);                                  \
        iss >> *this;                                                   \
    }

namespace Distribution
{
namespace Detail
{
    namespace br = boost::random;

    struct UniformInt : br::uniform_int_distribution<> {
        using Base = br::uniform_int_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct UniformReal : br::uniform_real_distribution<> {
        using Base = br::uniform_real_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Binomial : br::binomial_distribution<> {
        using Base = br::binomial_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Geometric : br::geometric_distribution<> {
        using Base = br::geometric_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct NegativeBinomial : br::negative_binomial_distribution<> {
        using Base = br::negative_binomial_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Poisson : br::poisson_distribution<> {
        using Base = br::poisson_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Exponential : br::exponential_distribution<> {
        using Base = br::exponential_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Gamma : br::gamma_distribution<> {
        using Base = br::gamma_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Weibull : br::weibull_distribution<> {
        using Base = br::weibull_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct ExtremeValue : br::extreme_value_distribution<> {
        using Base = br::extreme_value_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Beta : br::beta_distribution<> {
        using Base = br::beta_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Laplace : br::laplace_distribution<> {
        using Base = br::laplace_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Normal : br::normal_distribution<> {
        using Base = br::normal_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Lognormal : br::lognormal_distribution<> {
        using Base = br::lognormal_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct ChiSquared : br::chi_squared_distribution<> {
        using Base = br::chi_squared_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct NCChiSquared : br::non_central_chi_squared_distribution<> {
        using Base = br::non_central_chi_squared_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Cauchy : br::cauchy_distribution<> {
        using Base = br::cauchy_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct FisherF : br::fisher_f_distribution<> {
        using Base = br::fisher_f_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct StudentT : br::student_t_distribution<> {
        using Base = br::student_t_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Discrete : br::discrete_distribution<> {
        using Base = br::discrete_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct PiecewiseConstant : br::piecewise_constant_distribution<> {
        using Base = br::piecewise_constant_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct PiecewiseLinear : br::piecewise_linear_distribution<> {
        using Base = br::piecewise_linear_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Triangle : br::triangle_distribution<> {
        using Base = br::triangle_distribution<>;
        using Base::Base;
        DISTRIBUTION_SERIALIZATION
    };
    struct Constant {
        // Minimal placeholder class for a constant/scalar value.
        typedef double result_type;
        explicit Constant(double a_arg = 0.0) : _a(a_arg) {}
        template<class Engine> double operator()(Engine &eng) { return _a; }
        double a() const { return _a; }
        friend std::ostream& operator<<(std::ostream &os, const Constant &cd) {
            os << cd._a;
            return os;
        }
        friend std::istream& operator>>(std::istream &is, Constant &cd) {
            is >> cd._a;
            return is;
        }
        DISTRIBUTION_SERIALIZATION
    private:
        double _a;
    };

    /*
    using DiscreteTypes = boost::mpl::list<
        UniformInt,
        Binomial,
        Geometric,
        NegativeBinomial,
        Poisson,
        Discrete
    >;
    using ContinuousTypes = boost::mpl::list<
        UniformReal,
        Exponential,
        Gamma,
        Weibull,
        ExtremeValue,
        Beta,
        Laplace,
        Normal,
        Lognormal,
        ChiSquared,
        NCChiSquared,
        Cauchy,
        FisherF,
        StudentT,
        PiecewiseConstant,
        PiecewiseLinear,
        Triangle,
        Constant
    >;
    using Variant = boost::make_variant_over<
        boost::mpl::joint_view<
            DiscreteTypes,
            ContinuousTypes
        >::type
    >::type;
    */

    // FIXME
    // To workaround issue with boost::variant serialization in cereal,
    // increase MPL type limit and define variant as usual.

    using Variant = boost::variant<
        UniformReal,
        UniformInt,
        NegativeBinomial,
        Geometric,
        Binomial,
        Laplace,
        Beta,
        ExtremeValue,
        Weibull,
        Gamma,
        Exponential,
        Poisson,
        StudentT,
        FisherF,
        Cauchy,
        NCChiSquared,
        ChiSquared,
        Lognormal,
        Normal,
        PiecewiseLinear,
        PiecewiseConstant,
        Discrete,
        Triangle,
        Constant>;

    struct vgen_generic_visitor : boost::static_visitor<double> {
        template <typename Dist, typename Engine>
        typename Dist::result_type operator()(Dist const& d, Engine const& rng) const {
            br::variate_generator<Engine, Dist> vgen(rng, d);
            return vgen();
        }
    };
    
    template <typename... Dist, typename Engine>
    auto vgen_generic(Engine const& rng, boost::variant<Dist...> const& operand)
    {    
        auto f = boost::bind(vgen_generic_visitor(), ::_1, rng);
        return boost::apply_visitor(f, operand);
    }

} // namespace Detail

using UniformInt              = Detail::UniformInt;
using UniformReal             = Detail::UniformReal;
using Binomial                = Detail::Binomial;
using Geometric               = Detail::Geometric;
using NegativeBinomial        = Detail::NegativeBinomial;
using Poisson                 = Detail::Poisson;
using Exponential             = Detail::Exponential;
using Gamma                   = Detail::Gamma;
using Weibull                 = Detail::Weibull;
using ExtremeValue            = Detail::ExtremeValue;
using Beta                    = Detail::Beta;
using Laplace                 = Detail::Laplace;
using Normal                  = Detail::Normal;
using Lognormal               = Detail::Lognormal;
using ChiSquared              = Detail::ChiSquared;
using NCChiSquared            = Detail::NCChiSquared;
using Cauchy                  = Detail::Cauchy;
using FisherF                 = Detail::FisherF;
using StudentT                = Detail::StudentT;
using Discrete                = Detail::Discrete;
using PiecewiseConstant       = Detail::PiecewiseConstant;
using PiecewiseLinear         = Detail::PiecewiseLinear;
using Triangle                = Detail::Triangle;
using Constant                = Detail::Constant;

} // namespace Distribution


class GenericDistribution : public Distribution::Detail::Variant
{
    using Base = Distribution::Detail::Variant;

public:
    using Base::Base;
    
    template <typename Engine>
    QVector<double> sample(Engine const& rng, std::size_t const n) const
    {
        QVector<double> rv(static_cast<int>(n));
        for (auto&& x : rv)
            x = Distribution::Detail::vgen_generic(rng, *this);
        return rv;
    }
    
    template <typename Engine>
    double operator()(Engine const& rng)
    {
        return Distribution::Detail::vgen_generic(rng, *this);
    }
    
    void pdf(QVector<double> const& x, QVector<double> &y) const;
};

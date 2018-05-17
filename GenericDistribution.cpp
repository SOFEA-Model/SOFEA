//#define BOOST_MATH_ASSERT_UNDEFINED_POLICY false
//#define BOOST_MATH_OVERFLOW_ERROR_POLICY errno_on_error

#include <boost/math/distributions/bernoulli.hpp>
#include <boost/math/distributions/beta.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <boost/math/distributions/cauchy.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/exponential.hpp>
#include <boost/math/distributions/extreme_value.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/gamma.hpp>
#include <boost/math/distributions/geometric.hpp>
#include <boost/math/distributions/laplace.hpp>
#include <boost/math/distributions/lognormal.hpp>
#include <boost/math/distributions/negative_binomial.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/poisson.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/triangular.hpp>
#include <boost/math/distributions/uniform.hpp>
#include <boost/math/distributions/weibull.hpp>

#include <boost/log/trivial.hpp>

#include "GenericDistribution.h"

namespace Distribution
{
namespace Detail
{
    namespace bm = boost::math;

    struct pdf_visitor : boost::static_visitor<>
    {
        pdf_visitor(QVector<double> const& x, QVector<double>& y) :
            _x(x), _y(y) {}

        // Uniform Distributions
        void operator()(UniformInt const& d) const {
            auto m = bm::uniform_distribution<>(d.a(), d.b());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(UniformReal const& d) const {
            auto m = bm::uniform_distribution<>(d.a(), d.b());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Binomial const& d) const {
            auto m = bm::binomial_distribution<>(d.t(), d.p());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Geometric const& d) const {
            auto m = bm::geometric_distribution<>(d.p());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(NegativeBinomial const& d) const {
            auto m = bm::negative_binomial_distribution<>(d.k(), d.p());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        // Poisson Distributions
        void operator()(Poisson const& d) const {
            auto m = bm::poisson_distribution<>(d.mean());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Exponential const& d) const {
            auto m = bm::exponential_distribution<>(d.lambda());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Gamma const& d) const {
            auto m = bm::gamma_distribution<>(d.alpha(), d.beta());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Weibull const& d) const {
            auto m = bm::weibull_distribution<>(d.a(), d.b());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(ExtremeValue const& d) const {
            auto m = bm::extreme_value_distribution<>(d.a(), d.b());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Beta const& d) const {
            auto m = bm::beta_distribution<>(d.alpha(), d.beta());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Laplace const& d) const {
            auto m = bm::laplace_distribution<>(d.mean(), d.beta());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        // Normal Distributions
        void operator()(Normal const& d) const {
            auto m = bm::normal_distribution<>(d.mean(), d.sigma());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Lognormal const& d) const {
            auto m = bm::lognormal_distribution<>(d.m(), d.s());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(ChiSquared const& d) const {
            auto m = bm::chi_squared_distribution<>(d.n());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(NCChiSquared const& d) const {
            auto m = bm::non_central_chi_squared_distribution<>(d.k(), d.lambda());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(Cauchy const& d) const {
            auto m = bm::cauchy_distribution<>(d.median(), d.sigma());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(FisherF const& d) const {
            auto m = bm::fisher_f_distribution<>(d.m(), d.n());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        void operator()(StudentT const& d) const {
            auto m = bm::students_t_distribution<>(d.n());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        // Sampling Distributions
        void operator()(Discrete const& d) const {
            _y = QVector<double>::fromStdVector(d.probabilities());
        }
        void operator()(PiecewiseConstant const& d) const {
            _y = QVector<double>::fromStdVector(d.densities());
        }
        void operator()(PiecewiseLinear const& d) const {
            _y = QVector<double>::fromStdVector(d.densities());
        }
        // Miscellaneous Distributions
        void operator()(Triangle const& d) const {
            auto m = bm::triangular_distribution<>(d.a(), d.b(), d.c());
            for(const auto &x : _x) _y.push_back(bm::pdf(m, x));
        }
        // Placeholder
        void operator()(Constant const& d) const {
            for(const auto &x : _x) _y.push_back(d.a());
        }
        
    private:
        const QVector<double> &_x;
        QVector<double> &_y;
    };

} // namespace Distribution
} // namespace Detail

void GenericDistribution::pdf(QVector<double> const& x, QVector<double> &y) const
{
    y.reserve(x.size());
    try {
        boost::apply_visitor(Distribution::Detail::pdf_visitor(x, y), *this);
    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        return;
    }
};

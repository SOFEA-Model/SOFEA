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

#include <string>
#include <system_error>

#include <fmt/core.h>

namespace sofea {
namespace error {

enum class MeteorologyError
{
    NoError = 0,
    SurfaceFileNotFound,
    UpperAirFileNotFound,
    SurfaceFileParseError,
    UpperAirFileParseError,
    CalmMissingExceedsThreshold
};

enum class FluxProfileError
{
    NoError = 0,
    NegativeReferenceAppRate,
    ZeroReferenceAppRate,
    NoReferenceFlux,
    DepthScalingAndMaxVolatilizationLoss
};

} // namespace error
} // namespace sofea

namespace std {

template<>
struct is_error_code_enum<sofea::error::FluxProfileError> : public true_type {};

} // namespace std

namespace sofea {
namespace error {
namespace detail {

class FluxProfileCategory : public std::error_category
{
public:
    virtual const char * name() const noexcept override final { return "Flux Profile Error"; }

    virtual std::string message(int e) const override final
    {
        switch (static_cast<FluxProfileError>(e)) {
        case FluxProfileError::NoError:
            return "no error";
        case FluxProfileError::NegativeReferenceAppRate:
            return "reference application rate is negative";
        case FluxProfileError::ZeroReferenceAppRate:
            return "reference application rate is zero";
        case FluxProfileError::NoReferenceFlux:
            return "no reference flux data; flux will be zero for all hours";
        case FluxProfileError::DepthScalingAndMaxVolatilizationLoss:
            return "depth scaling enabled and reference volatilization loss equals maximum volatilization loss; depth scaling will not be used";
        default:
            return "unknown error";
        }
    }
};

} // namespace detail

extern inline const detail::FluxProfileCategory &FluxProfileCategory()
{
    static detail::FluxProfileCategory c;
    return c;
}

inline std::error_code make_error_code(FluxProfileError e)
{
    return {static_cast<int>(e), FluxProfileCategory()};
}

} // namespace error
} // namespace sofea

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

#include <set>
#include <tuple>

struct BufferZone
{
    double areaThreshold = 0;
    double appRateThreshold = 0;
    mutable double distance = 0;
    mutable int duration = 0;

    friend bool operator<(const BufferZone& lhs, const BufferZone& rhs) {
        return std::tie(lhs.areaThreshold, lhs.appRateThreshold)
             < std::tie(rhs.areaThreshold, rhs.appRateThreshold);
    }
};

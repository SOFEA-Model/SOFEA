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

#include <memory>
#include <utility>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

struct Meteorology
{
    std::string name;
    std::string surfaceFilePath;
    std::string upperAirFilePath;
    double anemometerHeight = 10;
    double windRotation = 0;
    boost::posix_time::ptime minTime = boost::posix_time::ptime();
    boost::posix_time::ptime maxTime = boost::posix_time::ptime();
};

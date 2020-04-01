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

#include <QDateTime>
#include <QString>

#include <locale>
#include <sstream>
#include <string>
#include <type_traits>

#include <boost/icl/gregorian.hpp>
#include <boost/icl/ptime.hpp>
#include <boost/icl/discrete_interval.hpp>

namespace sofea {
namespace utilities {

// QDateTime to posix_time::ptime

template <class T>
std::enable_if_t<std::is_same_v<T, boost::posix_time::ptime>, T> convert(const QDateTime& dt)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    if (!dt.isValid())
        return ptime();

    int y = dt.date().year();
    int m = dt.date().month();
    int d = dt.date().day();
    int h = dt.time().hour();

    ptime pt(date(y, m, d), hours(h));
    return pt;
}

// posix_time::ptime to QDateTime

template <class T>
std::enable_if_t<std::is_same_v<T, QDateTime>, T> convert(const boost::posix_time::ptime& pt)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    if (pt.is_not_a_date_time())
        return QDateTime();

    date gd = pt.date();
    time_duration td = pt.time_of_day();

    return QDateTime(QDate(gd.year(), gd.month(), gd.day()),
                     QTime(td.hours(), 0, 0), Qt::UTC);
}

// icl::discrete_interval<posix_time::ptime> to std::string

template <class T>
std::enable_if_t<std::is_same_v<T, std::string>, T> convert(const boost::icl::discrete_interval<boost::posix_time::ptime>& i)
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;

    std::ostringstream oss;
    ptime lower = i.lower();
    ptime upper = i.upper();
    time_period period(lower, upper);

    time_facet *facet = new time_facet(); // std::locale handles destruction
    facet->format("%Y-%m-%d %H:%M");
    period_formatter formatter(period_formatter::AS_OPEN_RANGE, ", ", "[", ")", "]");
    facet->period_formatter(formatter);
    oss.imbue(std::locale(std::locale::classic(), facet));

    oss << period;
    return oss.str();
}

} // namespace utilities
} // namespace sofea

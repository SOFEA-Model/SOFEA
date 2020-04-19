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

#include "UDUnitsInterface.h"
#include "core/Common.h"

#include <cstdio>
#include <mutex>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QStringList>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <udunits2/udunits2.h>

namespace UDUnits {

// Returns a message describing the last reported status code.
static QString statusText(ut_status status_code)
{
    switch (status_code) {
    case UT_SUCCESS:         return "Success";
    case UT_BAD_ARG:         return "An argument violates the function's contract";
    case UT_EXISTS:          return "Unit, prefix, or identifier already exists";
    case UT_NO_UNIT:         return "No such unit exists";
    case UT_OS:              return "Operating system error";
    case UT_NOT_SAME_SYSTEM: return "The units belong to different unit-systems";
    case UT_MEANINGLESS:     return "The operation on the unit(s) is meaningless";
    case UT_NO_SECOND:       return "The unit-system doesn't have a unit named \"second\"";
    case UT_VISIT_ERROR:     return "An error occurred while visiting a unit";
    case UT_CANT_FORMAT:     return "The unit can't be formatted in the desired manner";
    case UT_SYNTAX:          return "string unit representation contains syntax error";
    case UT_UNKNOWN:         return "string unit representation contains unknown word";
    case UT_OPEN_ARG:        return "Can't open argument-specified unit database";
    case UT_OPEN_ENV:        return "Can't open environment-specified unit database";
    case UT_OPEN_DEFAULT:    return "Can't open default unit database";
    case UT_PARSE:           return "Error parsing unit specification";
    default:                 return "Unknown error";
    }
}

// Custom error handler to replace ut_write_to_stderr
static int errorHandler(const char* fmt, va_list args)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General")

    char buf[1024] = { '\0' };
    int nbytes = vsnprintf(buf, 1024, fmt, args);

    ut_status status_code = ut_get_status();

    // Only log errors for critical status codes.
    // Skip UT_PARSE, as spurious warnings are generated
    // when parsing the default UDUNITS-2 XML files.

    switch (status_code) {
    case UT_OS:
    case UT_NO_SECOND:
    case UT_VISIT_ERROR:
    case UT_OPEN_ARG:
    case UT_OPEN_ENV:
    case UT_OPEN_DEFAULT:
        BOOST_LOG_TRIVIAL(error) << "udunits2: " << buf;
    case UT_BAD_ARG:
    case UT_EXISTS:
    case UT_NO_UNIT:
    case UT_NOT_SAME_SYSTEM:
    case UT_MEANINGLESS:
    case UT_CANT_FORMAT:
    case UT_SYNTAX:
    case UT_UNKNOWN:
    case UT_PARSE:
        message = statusText(status_code);
    case UT_SUCCESS:
    default:
        break;
    }

    return nbytes;
}

//-----------------------------------------------------------------------------
// UDUnits::UnitSystem
//-----------------------------------------------------------------------------

ut_system* UnitSystem::instance()
{
    static QString xmlPath = QDir::cleanPath(QCoreApplication::applicationDirPath()
        + QDir::separator() + SOFEA_UDUNITS_XML_PATH);

    static std::once_flag flag;
    std::call_once(flag, ut_set_error_message_handler, &errorHandler);

    static std::shared_ptr<ut_system> us(
        ut_read_xml(xmlPath.toLocal8Bit().constData()),
        [](ut_system *p) { if (p) ut_free_system(p); });

    return us.get();
}

//-----------------------------------------------------------------------------
// UDUnits::Unit
//-----------------------------------------------------------------------------

Unit::Unit(const QString& unit)
{
    ut_system *us = UnitSystem::instance();
    if (us == nullptr)
        return;

    const char* cstr = unit.toUtf8().constData();
    m_unit = ut_parse(us, cstr, UT_UTF8);
}

Unit::~Unit()
{
    if (m_unit) ut_free(m_unit);
}

bool Unit::isValid() const
{
    return (m_unit ? true : false);
}

QString Unit::toString() const
{
    if (m_unit == nullptr)
        return QString();

    char buf[128];
    //unsigned int opts = UT_UTF8 | UT_DEFINITION;
    unsigned int opts = UT_UTF8;
    int len = ut_format(m_unit, buf, sizeof(buf), opts);
    if (len == -1)
        return QString();

    QString str = QString::fromUtf8(buf);

    // This is a workaround for excess decimals in ut_format.
    // Check if the first part is a number, and reformat.

    QStringList split = str.split(" ", QString::SkipEmptyParts);
    if (split.length() > 1) {
        bool ok;
        double val = split[0].toDouble(&ok);
        if (ok) {
            split[0] = QString::number(val);
            return split.join(" ");
        }
    }

    return str;
}

//-----------------------------------------------------------------------------
// UDUnits::Converter
//-----------------------------------------------------------------------------

Converter::Converter(const Unit& from, const Unit& to)
{
    if (from.m_unit == nullptr || to.m_unit == nullptr)
        return;

    m_converter = ut_get_converter(from.m_unit, to.m_unit);
}

Converter::~Converter()
{
    if (m_converter) cv_free(m_converter);
}

bool Converter::isConvertible(const Unit& from, const Unit& to)
{
    if (from.m_unit == nullptr || to.m_unit == nullptr)
        return false;

    int rc = ut_are_convertible(from.m_unit, to.m_unit);
    if (rc != 0) {
        return true;
    }
    else {
        ut_status status_code = ut_get_status();
        // UT_SUCCESS indicates an error for ut_are_convertible.
        if (status_code == UT_SUCCESS) {
            message = "Conversion between the units is not possible";
        }
        return false;
    }
}

double Converter::convert(const double value) const
{
    if (m_converter == nullptr)
        return value;
    else
        return cv_convert_double(m_converter, value);
}

} // namespace UDUnits

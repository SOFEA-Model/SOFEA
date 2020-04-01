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

#include <QString>

struct ut_system;
union ut_unit;
union cv_converter;

namespace UDUnits {

// Last reported error status text.
static QString message;

//-----------------------------------------------------------------------------
// UDUnits::UnitSystem
//-----------------------------------------------------------------------------

class UnitSystem
{
public:
    static ut_system* instance();

    UnitSystem(const UnitSystem&) = delete;
    UnitSystem& operator=(const UnitSystem&) = delete;
    UnitSystem(UnitSystem&&) = delete;
    UnitSystem& operator=(UnitSystem&&) = delete;

private:
    UnitSystem() = default;
    ~UnitSystem() = default;
};

//-----------------------------------------------------------------------------
// UDUnits::Unit
//-----------------------------------------------------------------------------

class Unit
{
friend class Converter;
public:
    explicit Unit(const QString& unit);
    ~Unit();

    bool isValid() const;
    QString toString() const;

private:
    ut_unit* m_unit = nullptr;
};

//-----------------------------------------------------------------------------
// UDUnits::Converter
//-----------------------------------------------------------------------------

class Converter
{
public:
    Converter(const Unit& from, const Unit& to);
    ~Converter();

    static bool isConvertible(const Unit& from, const Unit& to);
    double convert(const double value) const;

private:
    cv_converter* m_converter = nullptr;
};

} // namespace UDUnits

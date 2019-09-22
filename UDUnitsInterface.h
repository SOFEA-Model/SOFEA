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

struct UnitSystem final
{
    static ut_system* get();

private:
    UnitSystem() = default;
    ~UnitSystem() = default;

    UnitSystem(const UnitSystem&) = delete;
    UnitSystem& operator=(const UnitSystem&) = delete;
    UnitSystem(UnitSystem&&) = delete;
    UnitSystem& operator=(UnitSystem&&) = delete;
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

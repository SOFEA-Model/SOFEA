#pragma once

#include <memory>
#include <string>
#include <vector>

// proj.h opaque pointers
struct PJconsts;
struct PJ_AREA;
struct projCtx_t;

namespace Projection {

enum class Type {
    Local = 1,
    SPCS83,
    UTM
};

enum class HDatum {
    Local = 1,
    NAD83,
    WGS84
};

enum class Units {
    Meter,
    InternationalFoot,
    USSurveyFoot
};

struct Area
{
    double xmin = -180.0;
    double ymin = -90.0;
    double xmax = 180.0;
    double ymax = 90.0;
};

//-----------------------------------------------------------------------------
// Projection::Context
//-----------------------------------------------------------------------------

struct Context final
{
    static projCtx_t* get();
    static void setDatabasePath(const std::string& path);
    static std::string databasePath();

private:
    Context() = default;
    ~Context() = default;

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;
};

//-----------------------------------------------------------------------------
// Projection::Generic
//-----------------------------------------------------------------------------

struct Generic
{
    Generic(const std::string& conversionCode,
            const std::string& hDatumCode, const std::string& hUnitsCode,
            const std::string& vDatumCode, const std::string& vUnitsCode);

    bool isValid() const;
    const std::string& conversionCode() const;
    const std::string& hDatumCode() const;
    const std::string& hUnitsCode() const;
    const std::string& vDatumCode() const;
    const std::string& vUnitsCode() const;
    const std::string& geodeticCRS() const;
    const std::string& compoundCRS() const;

private:
    bool valid_ = false;
    const std::string conversionCode_;
    const std::string hDatumCode_;
    const std::string hUnitsCode_;
    const std::string vDatumCode_;
    const std::string vUnitsCode_;
    std::string compoundCRS_;
    std::string geodeticCRS_;
};

//-----------------------------------------------------------------------------
// Projection::Transform
//-----------------------------------------------------------------------------

struct Transform
{
    Transform();
    Transform(const std::string& from, const std::string& to, const Area& bbox = Area{});
    bool isValid() const;
    int forward(double lon, double lat, double elev, double& x, double& y, double& z) const;
    int inverse(double x, double y, double z, double& lon, double& lat, double& elev) const;
    static std::string errorString(int err);

private:
    bool valid_ = false;
    std::shared_ptr<PJ_AREA> area_;
    std::shared_ptr<PJconsts> pdata_;
};

} // namespace Projection

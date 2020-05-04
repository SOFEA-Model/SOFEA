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

#include "core/Projection.h"

#include <proj.h>
#include <proj_experimental.h>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

// proj_internal.h
extern void PROJ_DLL pj_context_set_user_writable_directory(PJ_CONTEXT* ctx, const std::string& path);

namespace Projection {

struct PJUniquePtrDeleter {
    void operator()(PJ *p) const { proj_destroy(p); }
};

struct PJAreaUniquePtrDeleter {
    void operator()(PJ_AREA *p) const { proj_area_destroy(p); }
};

using PJUniquePtr = std::unique_ptr<PJ, PJUniquePtrDeleter>;

using PJAreaUniquePtr = std::unique_ptr<PJ_AREA, PJAreaUniquePtrDeleter>;

inline void contextLogFunction(void *data, int level, const char *msg)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Projection");

    switch (level) {
    case PJ_LOG_NONE:
        break;
    case PJ_LOG_TRACE:
        BOOST_LOG_TRIVIAL(trace) << "PROJ: " << msg;
        break;
    case PJ_LOG_DEBUG:
        BOOST_LOG_TRIVIAL(debug) << "PROJ: " << msg;
        break;
    case PJ_LOG_ERROR:
        BOOST_LOG_TRIVIAL(error) << "PROJ: " << msg;
        break;
    default:
        break;
    }
}

inline PJ_CONTEXT * context()
{
    thread_local std::shared_ptr<PJ_CONTEXT> ctx;

    if (ctx == nullptr) {
        ctx = std::shared_ptr<PJ_CONTEXT>(
            proj_context_create(),
            [](PJ_CONTEXT *p) { proj_context_destroy(p); });

        // Cache directory is not inherited. Initialize from the default context.
        setCacheDirectory(cacheDirectory(nullptr), ctx.get());

        // Enable networking.
        proj_grid_cache_set_enable(ctx.get(), true);
        proj_context_set_enable_network(ctx.get(), true);

        // Set the log callback.
        proj_log_level(ctx.get(), PJ_LOG_TRACE);
        proj_log_func(ctx.get(), nullptr, contextLogFunction);
    }

    return ctx.get();
}

//-----------------------------------------------------------------------------
// Context Functions
//-----------------------------------------------------------------------------

// Set the search path for the default context. Inherited by contexts created later.
void setSearchPath(const std::string& path) noexcept
{
    const char *paths[] = { path.c_str(), nullptr };
    proj_context_set_search_paths(nullptr, 1, paths);
}

void setDatabasePath(const std::string& path, PJ_CONTEXT *ctx) noexcept
{
    proj_context_set_database_path(ctx, path.c_str(), nullptr, nullptr);
}

std::string databasePath(PJ_CONTEXT *ctx) noexcept
{
    return proj_context_get_database_path(ctx);
}

void setCacheDirectory(const std::string& path, PJ_CONTEXT *ctx) noexcept
{
    pj_context_set_user_writable_directory(ctx, path.c_str());
}

std::string cacheDirectory(PJ_CONTEXT *ctx) noexcept
{
    return proj_context_get_user_writable_directory(ctx, false);
}

//-----------------------------------------------------------------------------
// Object Inquiry Functions
//-----------------------------------------------------------------------------

std::string getObjectTypeString(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return "NULL";

    switch (proj_get_type(object.get())) {
    case PJ_TYPE_ELLIPSOID:                        return "PJ_TYPE_ELLIPSOID";
    case PJ_TYPE_PRIME_MERIDIAN:                   return "PJ_TYPE_PRIME_MERIDIAN";
    case PJ_TYPE_GEODETIC_REFERENCE_FRAME:         return "PJ_TYPE_GEODETIC_REFERENCE_FRAME";
    case PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME: return "PJ_TYPE_DYNAMIC_GEODETIC_REFERENCE_FRAME";
    case PJ_TYPE_VERTICAL_REFERENCE_FRAME:         return "PJ_TYPE_VERTICAL_REFERENCE_FRAME";
    case PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME: return "PJ_TYPE_DYNAMIC_VERTICAL_REFERENCE_FRAME";
    case PJ_TYPE_DATUM_ENSEMBLE:                   return "PJ_TYPE_DATUM_ENSEMBLE";
    case PJ_TYPE_GEOGRAPHIC_2D_CRS:                return "PJ_TYPE_GEOGRAPHIC_2D_CRS";
    case PJ_TYPE_GEOGRAPHIC_3D_CRS:                return "PJ_TYPE_GEOGRAPHIC_3D_CRS";
    case PJ_TYPE_GEOCENTRIC_CRS:                   return "PJ_TYPE_GEOCENTRIC_CRS";
    case PJ_TYPE_GEODETIC_CRS:                     return "PJ_TYPE_GEODETIC_CRS";
    case PJ_TYPE_VERTICAL_CRS:                     return "PJ_TYPE_VERTICAL_CRS";
    case PJ_TYPE_PROJECTED_CRS:                    return "PJ_TYPE_PROJECTED_CRS";
    case PJ_TYPE_COMPOUND_CRS:                     return "PJ_TYPE_COMPOUND_CRS";
    case PJ_TYPE_TEMPORAL_CRS:                     return "PJ_TYPE_TEMPORAL_CRS";
    case PJ_TYPE_ENGINEERING_CRS:                  return "PJ_TYPE_ENGINEERING_CRS";
    case PJ_TYPE_BOUND_CRS:                        return "PJ_TYPE_BOUND_CRS";
    case PJ_TYPE_OTHER_CRS:                        return "PJ_TYPE_OTHER_CRS";
    case PJ_TYPE_CONVERSION:                       return "PJ_TYPE_CONVERSION";
    case PJ_TYPE_TRANSFORMATION:                   return "PJ_TYPE_TRANSFORMATION";
    case PJ_TYPE_CONCATENATED_OPERATION:           return "PJ_TYPE_CONCATENATED_OPERATION";
    case PJ_TYPE_OTHER_COORDINATE_OPERATION:       return "PJ_TYPE_OTHER_COORDINATE_OPERATION";
    default: return "PJ_TYPE_UNKNOWN";
    }
}

std::string getObjectName(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_get_name(object.get());
}

std::string getObjectAuthName(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_get_id_auth_name(object.get(), 0);
}

std::string getObjectCode(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_get_id_code(object.get(), 0);
}

GeographicExtent getObjectAreaOfUse(std::shared_ptr<PJ> object) noexcept
{
    GeographicExtent result;

    if (object == nullptr)
        return result;

    proj_get_area_of_use(context(), object.get(),
                         &result.westLongitude,
                         &result.southLatitude,
                         &result.eastLongitude,
                         &result.northLatitude,
                         nullptr);

    return result;
}

// CRS and Coordinate Operation Functions

std::shared_ptr<PJ> createObjectFromWKT(const std::string& wkt) noexcept
{
    if (wkt.empty())
        return nullptr;

    PJ *object = proj_create_from_wkt(context(), wkt.c_str(), nullptr, nullptr, nullptr);
    return std::shared_ptr<PJ>(object, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> createObjectFromString(const std::string& definition) noexcept
{
    if (definition.empty())
        return nullptr;

    PJ *object = proj_create(context(), definition.c_str());
    return std::shared_ptr<PJ>(object, [](PJ *p) { proj_destroy(p); });
}

//-----------------------------------------------------------------------------
// CRS and Coordinate Operation Functions
//-----------------------------------------------------------------------------

UnitOfMeasure createUnitOfMeasure(const std::string& code)
{
    UnitOfMeasure result;

    const char *name = nullptr;
    double factor;
    const char *category = nullptr;

    int rc = proj_uom_get_info_from_database(context(),
        "EPSG", code.c_str(), &name, &factor, &category);

    if (rc) {
        result.name = name;
        result.factor = factor;
        result.category = category;
    }

    return result;
}

std::shared_ptr<PJ> createGeodeticCRS(const std::string& datumCode) noexcept
{
    PJ *crs = proj_create_from_database(context(),
        "EPSG", datumCode.c_str(), PJ_CATEGORY_CRS, false, nullptr);

    if (crs == nullptr)
        return nullptr;

    const PJ_TYPE crstype = proj_get_type(crs);
    switch (crstype) {
    case PJ_TYPE_GEOGRAPHIC_2D_CRS:
    case PJ_TYPE_GEOGRAPHIC_3D_CRS:
    case PJ_TYPE_GEOCENTRIC_CRS:
    case PJ_TYPE_GEODETIC_CRS:
        return std::shared_ptr<PJ>(crs, [](PJ *p) { proj_destroy(p); });
    default:
        return nullptr;
    }
}

std::shared_ptr<PJ> createConversion(const std::string& conversionCode) noexcept
{
    PJ *conversion = proj_create_from_database(context(),
        "EPSG", conversionCode.c_str(), PJ_CATEGORY_COORDINATE_OPERATION, false, nullptr);

    return std::shared_ptr<PJ>(conversion, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> createProjectedCRS(std::shared_ptr<PJ> geodeticCRS,
                                       std::shared_ptr<PJ> conversion,
                                       const std::string& unitCode)
{
    if (geodeticCRS == nullptr || conversion == nullptr)
        return nullptr;

    auto uom = createUnitOfMeasure(unitCode);

    PJUniquePtr cs(proj_create_cartesian_2D_cs(context(),
        PJ_CART2D_EASTING_NORTHING, uom.name.c_str(), uom.factor));

    if (cs == nullptr)
        return nullptr;

    PJ *crs = proj_create_projected_crs(context(),
        nullptr, geodeticCRS.get(), conversion.get(), cs.get());

    return std::shared_ptr<PJ>(crs, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> createVerticalCRS(const std::string& datumCode,
                                      const std::string& unitCode,
                                      const std::string& geoidName)
{
    PJUniquePtr datum(proj_create_from_database(context(),
        "EPSG", datumCode.c_str(), PJ_CATEGORY_DATUM, false, nullptr));

    if (datum == nullptr)
        return nullptr;

    const char *datumName = proj_get_name(datum.get());
    const char *datumAuthName = proj_get_id_auth_name(datum.get(), 0);
    auto uom = createUnitOfMeasure(unitCode);

    std::string crsName = datumName;
    if (!geoidName.empty())
         crsName.append(std::string(" (" + geoidName + ")"));

    PJ *crs = proj_create_vertical_crs_ex(context(), crsName.c_str(),
        datumName, datumAuthName, datumCode.c_str(),
        uom.name.c_str(), uom.factor, geoidName.c_str(),
        nullptr, nullptr, nullptr, nullptr);

    return std::shared_ptr<PJ>(crs, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> createCompoundCRS(std::shared_ptr<PJ> horizontalCRS,
                                      std::shared_ptr<PJ> verticalCRS) noexcept
{
    if (horizontalCRS == nullptr || verticalCRS == nullptr)
        return nullptr;

    PJ *crs = proj_create_compound_crs(context(),
        nullptr, horizontalCRS.get(), verticalCRS.get());

    return std::shared_ptr<PJ>(crs, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> getComponentCRS(std::shared_ptr<PJ> compoundCRS,
                                    int index) noexcept
{
    if (compoundCRS == nullptr)
        return nullptr;

    PJ *crs = proj_crs_get_sub_crs(context(), compoundCRS.get(), index);

    return std::shared_ptr<PJ>(crs, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> getGeodeticCRS(std::shared_ptr<PJ> crs) noexcept
{
    if (crs == nullptr)
        return nullptr;

    PJ *gcrs = proj_crs_get_geodetic_crs(context(), crs.get());

    return std::shared_ptr<PJ>(gcrs, [](PJ *p) { proj_destroy(p); });
}

std::shared_ptr<PJ> createPipeline(std::shared_ptr<PJ> from, std::shared_ptr<PJ> to, const GeographicExtent& bbox) noexcept
{
    if (from == nullptr || to == nullptr)
        return nullptr;

    PJAreaUniquePtr area(proj_area_create());
    proj_area_set_bbox(area.get(), bbox.westLongitude, bbox.southLatitude, bbox.eastLongitude, bbox.northLatitude);

    PJUniquePtr object(proj_create_crs_to_crs_from_pj(context(), from.get(), to.get(), area.get(), nullptr));
    if (object == nullptr)
        return nullptr;

    return std::shared_ptr<PJ>(
        proj_normalize_for_visualization(context(), object.get()),
        [](PJ *p) { proj_destroy(p); });
}

std::string exportToPROJString(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_as_proj_string(context(), object.get(), PJ_PROJ_5, nullptr);
}

std::string exportToWKT(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_as_wkt(context(), object.get(), PJ_WKT2_2019, nullptr);
}

std::string exportToJSON(std::shared_ptr<PJ> object)
{
    if (object == nullptr)
        return std::string();
    return proj_as_projjson(context(), object.get(), nullptr);
}

//-----------------------------------------------------------------------------
// Pipeline
//-----------------------------------------------------------------------------

Pipeline::Pipeline(std::shared_ptr<PJ> from, std::shared_ptr<PJ> to, const GeographicExtent& bbox) noexcept
{
    if (from == nullptr || to == nullptr)
        return;

    PJAreaUniquePtr area(proj_area_create());
    proj_area_set_bbox(area.get(), bbox.westLongitude, bbox.southLatitude, bbox.eastLongitude, bbox.northLatitude);

    PJUniquePtr object(proj_create_crs_to_crs_from_pj(context(), from.get(), to.get(), area.get(), nullptr));
    if (object == nullptr)
        return;

    coordop_ = std::shared_ptr<PJ>(
        proj_normalize_for_visualization(context(), object.get()),
        [](PJ *p) { proj_destroy(p); });
}

bool Pipeline::valid() noexcept
{
    if (coordop_ == nullptr)
        return false;
    return proj_coordoperation_is_instantiable(context(), coordop_.get());
}

std::vector<Grid> Pipeline::grids()
{
    std::vector<Grid> result;

    if (coordop_ == nullptr)
        return result;

    int ngrids = proj_coordoperation_get_grid_used_count(context(), coordop_.get());
    if (ngrids == 0)
        return result;

    result.reserve(ngrids);
    for (int i = 0; i < ngrids; ++i) {
        const char *shortName = nullptr;
        const char *fullName = nullptr;
        const char *packageName = nullptr;
        const char *url = nullptr;
        int directDownload = 0;
        int openLicense = 0;
        int available = 0;

        int rc = proj_coordoperation_get_grid_used(context(), coordop_.get(), i,
            &shortName, &fullName, &packageName, &url, &directDownload, &openLicense, &available);

        if (rc) {
            Grid g;
            g.shortName = shortName;
            g.fullName = fullName;
            g.packageName = packageName;
            g.url = url;
            g.directDownload = directDownload;
            g.openLicense = openLicense;
            g.available = available;
            result.push_back(g);
        }
    }

    return result;
}

double Pipeline::accuracy() noexcept
{
    if (coordop_ == nullptr)
        return 0.;
    return proj_coordoperation_get_accuracy(context(), coordop_.get());
}

int Pipeline::forward(double x0, double y0, double z0, double& x1, double& y1, double& z1) const noexcept
{
    if (coordop_ == nullptr)
        return -1; // PJD_ERR_NO_ARGS

    PJ_COORD a, b;
    a = proj_coord(x0, y0, z0, 0.);

    proj_errno_reset(coordop_.get());
    b = proj_trans(coordop_.get(), PJ_FWD, a);
    int err = proj_errno(coordop_.get());
    if (err)
        return err;

    x1 = b.xyz.x;
    y1 = b.xyz.y;
    z1 = b.xyz.z;
    return err;
}

int Pipeline::inverse(double x0, double y0, double z0, double& x1, double& y1, double& z1) const noexcept
{
    if (coordop_ == nullptr)
        return -1; // PJD_ERR_NO_ARGS

    PJ_COORD a, b;
    a = proj_coord(x0, y0, z0, 0.);

    proj_errno_reset(coordop_.get());
    b = proj_trans(coordop_.get(), PJ_INV, a);
    int err = proj_errno(coordop_.get());
    if (err)
        return err;

    x1 = b.xyz.x;
    y1 = b.xyz.y;
    z1 = b.xyz.z;
    return err;
}

std::string Pipeline::errorString(int err)
{
    if (err >= 0)
        return std::string();
    return proj_errno_string(err);
}

} // namespace Projection


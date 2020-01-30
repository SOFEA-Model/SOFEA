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

#include "Projection.h"

#include <proj.h>
#include <proj/common.hpp>
#include <proj/coordinateoperation.hpp>
#include <proj/coordinatesystem.hpp>
#include <proj/crs.hpp>
#include <proj/datum.hpp>
#include <proj/io.hpp>
#include <proj/util.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>

using namespace osgeo::proj;

namespace Projection {

//-----------------------------------------------------------------------------
// Projection::Context
//-----------------------------------------------------------------------------

projCtx_t* Context::get()
{
    thread_local std::shared_ptr<projCtx_t> ctx(
        proj_context_create(),
        [](projCtx_t *p) { proj_context_destroy(p); });

    return ctx.get();
}

void Context::setDatabasePath(const std::string& path)
{
    proj_context_set_database_path(Context::get(), path.c_str(), nullptr, nullptr);
}

std::string Context::databasePath()
{
    return proj_context_get_database_path(Context::get());
}

//-----------------------------------------------------------------------------
// Projection::Generic
//-----------------------------------------------------------------------------

Generic::Generic(const std::string& conversionCode,
                 const std::string& hDatumCode, const std::string& hUnitsCode,
                 const std::string& vDatumCode, const std::string& vUnitsCode)
    : conversionCode_(conversionCode),
      hDatumCode_(hDatumCode), hUnitsCode_(hUnitsCode),
      vDatumCode_(vDatumCode), vUnitsCode_(vUnitsCode)
{
    thread_local auto dbContext = io::DatabaseContext::create(
        Context::databasePath(), std::vector<std::string>(), Context::get());

    thread_local auto epsgFactory = io::AuthorityFactory::create(dbContext, "EPSG");

    try {
        // Horizontal
        auto hconv = epsgFactory->createConversion(conversionCode);
        auto hdatum = epsgFactory->createGeodeticCRS(hDatumCode);
        auto hunits = epsgFactory->createUnitOfMeasure(hUnitsCode);
        cs::CartesianCSNNPtr hcs = cs::CartesianCS::createEastingNorthing(*hunits);
        crs::ProjectedCRSNNPtr hcrs = crs::ProjectedCRS::create(util::PropertyMap(), hdatum, hconv, hcs);

        // Vertical
        auto vdatum = epsgFactory->createVerticalDatum(vDatumCode);
        auto vunits = epsgFactory->createUnitOfMeasure(vUnitsCode);
        cs::VerticalCSNNPtr vcs = cs::VerticalCS::createGravityRelatedHeight(*vunits);
        crs::VerticalCRSNNPtr vcrs = crs::VerticalCRS::create(util::PropertyMap(), vdatum, vcs);

        // Compound
        crs::CompoundCRSNNPtr ccrs = crs::CompoundCRS::create(util::PropertyMap(), { hcrs, vcrs });

        geodeticCRS_ = hdatum->exportToPROJString(io::PROJStringFormatter::create().get());
        compoundCRS_ = ccrs->exportToPROJString(io::PROJStringFormatter::create().get());
        valid_ = true;
    }
    catch (const std::exception&) {
        geodeticCRS_ = std::string();
        compoundCRS_ = std::string();
        valid_ = false;
    }
}

bool Generic::isValid() const {
    return valid_;
}

const std::string& Generic::conversionCode() const {
    return conversionCode_;
}

const std::string& Generic::hDatumCode() const {
    return hDatumCode_;
}

const std::string& Generic::hUnitsCode() const {
    return hUnitsCode_;
}

const std::string& Generic::vDatumCode() const {
    return vDatumCode_;
}

const std::string& Generic::vUnitsCode() const {
    return vUnitsCode_;
}

const std::string& Generic::geodeticCRS() const {
    return geodeticCRS_;
}

const std::string& Generic::compoundCRS() const {
    return compoundCRS_;
}

//-----------------------------------------------------------------------------
// Projection::Transform
//-----------------------------------------------------------------------------

// If the CRS is invalid/local, we can still use a pipeline (coordinate operation) to convert units:
// +proj=pipeline
// +step +init=predefined_pipelines:projectandswap
// +step +proj=unitconvert +xy_in=m +xy_out=us-ft
//
// ConversionNNPtr createChangeVerticalUnit(constutil::PropertyMap &properties, constcommon::Scale &factor)
// ConversionNNPtr createGeographicGeocentric(constutil::PropertyMap &properties)
//
// auto op = CoordinateOperationFactory::create()->createOperation(crs->baseCRS(), NN_NO_CHECK(crs));

Transform::Transform()
{}

Transform::Transform(const std::string& from, const std::string& to, const Area& bbox)
{
    area_ = std::shared_ptr<PJ_AREA>(
        proj_area_create(),
        [](PJ_AREA *a) { proj_area_destroy(a); });

    proj_area_set_bbox(area_.get(), bbox.xmin, bbox.ymin, bbox.xmax, bbox.ymax);

    pdata_ = std::shared_ptr<PJconsts>(
        proj_create_crs_to_crs(Context::get(), from.c_str(), to.c_str(), area_.get()),
        [](PJ *p) { if (p != nullptr) proj_destroy(p); });

    if (pdata_ != nullptr)
        valid_ = true;
}

bool Transform::isValid() const
{
    return valid_;
}

int Transform::forward(double lon, double lat, double elev, double& x, double& y, double& z) const
{
    if (!valid_)
        return -1; // no arguments in initialization list

    PJ_COORD a, b;
    a.lpz.lam = lon;
    a.lpz.phi = lat;
    a.lpz.z = elev;

    proj_errno_reset(pdata_.get());
    b = proj_trans(pdata_.get(), PJ_FWD, a);
    int err = proj_errno(pdata_.get());
    if (err)
        return err;

    x = b.xyz.x;
    y = b.xyz.y;
    z = b.xyz.z;
    return err;
}

int Transform::inverse(double x, double y, double z, double& lon, double& lat, double& elev) const
{
    if (!valid_)
        return -1; // no arguments in initialization list

    PJ_COORD a, b;
    a.xyz.x = x;
    a.xyz.y = y;
    a.xyz.z = z;

    proj_errno_reset(pdata_.get());
    b = proj_trans(pdata_.get(), PJ_INV, a);
    int err = proj_errno(pdata_.get());
    if (err)
        return err;

    lon = b.lpz.lam;
    lat = b.lpz.phi;
    elev = b.lpz.z;
    return err;
}

std::string Transform::errorString(int err)
{
    if (err == 0)
        return std::string();

    return proj_errno_string(err);
}

} // namespace Projection


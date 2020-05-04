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

#define SOFEA_VERSION_STRING "4.1.5"
#define AERMOD_VERSION_STRING "18081"
#define ISCST3_VERSION_STRING "02035"

#define AERMOD_EXE "sofea_aermod.exe"

#define SOFEA_DOCUMENTATION_URL "https://sofea-model.github.io/user-guide/index.html"

#define SOFEA_PROJ_DATA_PATH "./share/proj"
#define SOFEA_GDAL_DATA_PATH "./share/gdal"
#define SOFEA_UDUNITS_XML_PATH "./share/udunits/udunits2.xml"

// Geomagnetic Model Endpoints
#define GEOMAG_API_URL_WMM "https://www.ngdc.noaa.gov/geomag-web/calculators/calculateDeclination?model=WMM"
#define GEOMAG_API_URL_IGRF "https://www.ngdc.noaa.gov/geomag-web/calculators/calculateDeclination?model=IGRF"
#define GEOMAG_API_URL_EMM "https://emmcalc.geomag.info/?magneticComponent=d"

// USGS 3DEP WCS Server Endpoint
#define USGS_3DEP_WCS_ENDPOINT "https://elevation.nationalmap.gov:443/arcgis/services/3DEPElevation/ImageServer/WCSServer?version=2.0.1&coverage=DEP3Elevation"

// Common GeodeticCRS Codes (Geographic2D)
#define EPSG_HDATUM_WGS84 "4326"
#define EPSG_HDATUM_NAD83_1986 "4269"
#define EPSG_HDATUM_NAD83_FBN "8860"
#define EPSG_HDATUM_NAD83_HARN "4152"
#define EPSG_HDATUM_NAD83_NSRS2007 "4759"
#define EPSG_HDATUM_NAD83_2011 "6318"
#define EPSG_HDATUM_NAD27 "4267"

// Common GeodeticCRS Codes (Geographic3D)
#define EPSG_HDATUM_3D_WGS84 "4979"
#define EPSG_HDATUM_3D_NAD83_FBN "8542"
#define EPSG_HDATUM_3D_NAD83_HARN "4957"
#define EPSG_HDATUM_3D_NAD83_NSRS2007 "4893"
#define EPSG_HDATUM_3D_NAD83_2011 "6319"

// Common VerticalDatum Codes
#define EPSG_VDATUM_NAVD88 "5103"
#define EPSG_VDATUM_NGVD29 "5102"
#define EPSG_VDATUM_EGM2008 "1027"
#define EPSG_VDATUM_EGM96 "5171"
#define EPSG_VDATUM_EGM84 "5203"

// Common UnitOfMeasure Codes
#define EPSG_UOM_METER "9001"
#define EPSG_UOM_IFT "9002"
#define EPSG_UOM_USFT "9003"

namespace sofea {
namespace constants {

inline constexpr double MIN_X_COORDINATE = -10000000.;
inline constexpr double MIN_Y_COORDINATE = -10000000.;
inline constexpr double MIN_Z_COORDINATE = 0.;
inline constexpr double MIN_X_DIMENSION = 0.;
inline constexpr double MIN_Y_DIMENSION = 0.;
inline constexpr double MIN_RADIUS = 0.;
inline constexpr double MIN_ROTATION_ANGLE = 0.;

inline constexpr double MAX_X_COORDINATE = 10000000.;
inline constexpr double MAX_Y_COORDINATE = 10000000.;
inline constexpr double MAX_Z_COORDINATE = 9999.99;
inline constexpr double MAX_X_DIMENSION = 10000.;
inline constexpr double MAX_Y_DIMENSION = 10000.;
inline constexpr double MAX_RADIUS = 10000.;
inline constexpr double MAX_ROTATION_ANGLE = 359.9;

inline constexpr int LONGITUDE_PRECISION = 8;
inline constexpr int LATITUDE_PRECISION = 8;

inline constexpr int X_COORDINATE_PRECISION = 2;
inline constexpr int Y_COORDINATE_PRECISION = 2;
inline constexpr int Z_COORDINATE_PRECISION = 2;
inline constexpr int X_DIMENSION_PRECISION = 2;
inline constexpr int Y_DIMENSION_PRECISION = 2;
inline constexpr int RADIUS_PRECISION = 2;
inline constexpr int ROTATION_ANGLE_PRECISION = 2;

} // namespace constants
} // namespace sofea

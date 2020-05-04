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
#include <string>
#include <vector>

// proj.h opaque pointers

struct PJconsts;
typedef struct PJconsts PJ;

struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;

namespace Projection {

struct GeographicExtent {
    double westLongitude = -180.0;
    double southLatitude = -90.0;
    double eastLongitude = 180.0;
    double northLatitude = 90.0;
};

struct ProjectedExtent {
    double xmin = 0;
    double ymin = 0;
    double xmax = 0;
    double ymax = 0;
};

struct UnitOfMeasure
{
    std::string name;
    double factor = 1;
    std::string category;
};

struct Grid {
    std::string shortName;
    std::string fullName;
    std::string packageName;
    std::string url;
    bool directDownload;
    bool openLicense;
    bool available;
};

//-----------------------------------------------------------------------------
// Context Functions
//-----------------------------------------------------------------------------

void setSearchPath(const std::string& path) noexcept;

void setDatabasePath(const std::string& path, PJ_CONTEXT *ctx = nullptr) noexcept;

std::string databasePath(PJ_CONTEXT *ctx = nullptr) noexcept;

void setCacheDirectory(const std::string& path, PJ_CONTEXT *ctx = nullptr) noexcept;

std::string cacheDirectory(PJ_CONTEXT *ctx = nullptr) noexcept;

//-----------------------------------------------------------------------------
// Object Inquiry Functions
//-----------------------------------------------------------------------------

std::string getObjectTypeString(std::shared_ptr<PJ> object);

std::string getObjectName(std::shared_ptr<PJ> object);

std::string getObjectAuthName(std::shared_ptr<PJ> object);

std::string getObjectCode(std::shared_ptr<PJ> object);

GeographicExtent getObjectAreaOfUse(std::shared_ptr<PJ> object) noexcept;

std::shared_ptr<PJ> createObjectFromWKT(const std::string& wkt) noexcept;

std::shared_ptr<PJ> createObjectFromString(const std::string& definition) noexcept;

//-----------------------------------------------------------------------------
// CRS and Coordinate Operation Functions
//-----------------------------------------------------------------------------

UnitOfMeasure createUnitOfMeasure(const std::string& code);

std::shared_ptr<PJ> createGeodeticCRS(const std::string& datumCode) noexcept;

std::shared_ptr<PJ> createConversion(const std::string& conversionCode) noexcept;

std::shared_ptr<PJ> createProjectedCRS(std::shared_ptr<PJ> geodeticCRS,
                                       std::shared_ptr<PJ> conversion,
                                       const std::string& unitCode);

std::shared_ptr<PJ> createVerticalCRS(const std::string& datumCode,
                                      const std::string& unitCode,
                                      const std::string& geoidName = {});

std::shared_ptr<PJ> createCompoundCRS(std::shared_ptr<PJ> horizontalCRS,
                                      std::shared_ptr<PJ> verticalCRS) noexcept;

std::shared_ptr<PJ> getComponentCRS(std::shared_ptr<PJ> compoundCRS,
                                    int index) noexcept;

std::shared_ptr<PJ> getGeodeticCRS(std::shared_ptr<PJ> crs) noexcept;

std::shared_ptr<PJ> createPipeline(std::shared_ptr<PJ> from,
                                   std::shared_ptr<PJ> to,
                                   const GeographicExtent& bbox = {}) noexcept;

std::string exportToPROJString(std::shared_ptr<PJ> object);

std::string exportToWKT(std::shared_ptr<PJ> object);

std::string exportToJSON(std::shared_ptr<PJ> object);

//-----------------------------------------------------------------------------
// Pipeline
//-----------------------------------------------------------------------------

struct Pipeline
{
    Pipeline() noexcept {}
    Pipeline(std::shared_ptr<PJ> from,
             std::shared_ptr<PJ> to,
             const GeographicExtent& bbox = {}) noexcept;

    bool valid() noexcept;
    std::vector<Grid> grids();
    double accuracy() noexcept;
    int forward(double x0, double y0, double z0, double& x1, double& y1, double& z1) const noexcept;
    int inverse(double x0, double y0, double z0, double& x1, double& y1, double& z1) const noexcept;
    static std::string errorString(int err);

private:
    std::shared_ptr<PJ> coordop_;
};

} // namespace Projection

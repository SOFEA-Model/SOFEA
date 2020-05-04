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

#include "core/Projection.h"
#include "core/TaskControl.h"

#include <array>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GDALDataset;
class GDALRasterBand;

namespace Raster {

void setConfigOption(const std::string& option, const std::string& value);

std::string getConfigOption(const std::string& option);

//-----------------------------------------------------------------------------
// GeoTransform
//-----------------------------------------------------------------------------

struct GeoTransform
{
    GeoTransform() = default;

    GeoTransform(const std::array<double, 6>& array)
        : ulx(array[0]), xres(array[1]), xrot(array[2]),
          uly(array[3]), yrot(array[4]), yres(array[5])
    {}

    std::array<double, 6> array()
    {
        return { ulx, xres, xrot, uly, yrot, yres };
    }

    double ulx = 0;
    double xres = 0;
    double xrot = 0;
    double uly = 0;
    double yrot = 0;
    double yres = 0;
};

//-----------------------------------------------------------------------------
// Window
//-----------------------------------------------------------------------------

struct Window
{
    int xmin = 0;
    int ymin = 0;
    int xmax = 0;
    int ymax = 0;

    int nx() const { return xmax - xmin; }
    int ny() const { return ymax - ymin; }
};

//-----------------------------------------------------------------------------
// Dataset
//-----------------------------------------------------------------------------

class Dataset
{
public:
    Dataset(const std::string& filename,
            const std::string& drivername,
            const std::vector<std::string>& options = {});

    ~Dataset();

    GDALRasterBand * band();
    GeoTransform geoTransform() const;
    std::shared_ptr<PJ> projection() const;
    std::array<int, 2> dimensions() const;
    std::array<int, 2> blockDimensions() const;
    double offset() const;
    double scale() const;
    double noDataValue() const;
    Window window() const;
    Window blockWindow() const;
    void setBoundingBox(std::shared_ptr<PJ> crs, const Projection::ProjectedExtent& bbox);
    void exportGeoTIFF(const std::string& filename) const;
    std::future<std::vector<float>> readBlocks(TaskControl& control);

private:
    static std::vector<float> readBlocksInternal(TaskControl& control, Dataset *p);

    Window window_;
    Window blockWindow_;
    GDALDataset *dataset_;
    GDALRasterBand *band_;
    std::vector<float> values_;
};

} // namespace Raster

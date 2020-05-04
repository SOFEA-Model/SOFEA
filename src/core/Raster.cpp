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

#include "core/Raster.h"

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <fmt/format.h>

#include <cpl_error.h>
#include <gdal_priv.h>
#include <gdal_alg.h>

namespace Raster {

inline void errorCallback(CPLErr err, CPLErrorNum num, const char *message)
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "General");

    switch (err) {
    case CE_None:
        break;
    case CE_Debug:
        BOOST_LOG_TRIVIAL(debug) << "GDAL: " << message;
        break;
    case CE_Warning:
        BOOST_LOG_TRIVIAL(warning) << "GDAL: " << message;
        break;
    case CE_Failure:
        BOOST_LOG_TRIVIAL(error) << "GDAL: " << message;
        break;
    case CE_Fatal:
        BOOST_LOG_TRIVIAL(fatal) << "GDAL: " << message;
        break;
    default:
        break;
    }
}

void setConfigOption(const std::string& option, const std::string& value)
{
    CPLSetConfigOption(option.c_str(), value.c_str());
}

std::string getConfigOption(const std::string& option)
{
    return CPLGetConfigOption(option.c_str(), "");
}

//-----------------------------------------------------------------------------
// DriverManager
//-----------------------------------------------------------------------------

class DriverManager
{
public:
    static GDALDriverManager * instance()
    {
        static std::once_flag flag;
        std::call_once(flag, [](){
            GDALAllRegister();
            CPLSetConfigOption("CPL_CURL_GZIP", "YES");
            CPLSetConfigOption("GTIFF_REPORT_COMPD_CS", "YES");
            CPLSetErrorHandler(errorCallback);
        });

        static std::shared_ptr<GDALDriverManager> manager(
            GetGDALDriverManager(),
            [](GDALDriverManager *p) { GDALDestroyDriverManager(); });

        return manager.get();
    }

    DriverManager(const DriverManager&) = delete;
    DriverManager& operator=(const DriverManager&) = delete;
    DriverManager(DriverManager&&) = delete;
    DriverManager& operator=(DriverManager&&) = delete;

private:
    DriverManager() = default;
    ~DriverManager() = default;
};

//-----------------------------------------------------------------------------
// Dataset
//-----------------------------------------------------------------------------

Dataset::Dataset(const std::string& filename,
                 const std::string& drivername,
                 const std::vector<std::string>& options)
{
    GDALDriver *driver = DriverManager::instance()->GetDriverByName(drivername.c_str());
    if (driver == nullptr)
        throw std::runtime_error(fmt::format("Failed to load {} driver.", drivername));

    DriverManager::instance()->RegisterDriver(driver);

    const unsigned int openFlags = GDAL_OF_READONLY | GDAL_OF_RASTER;
    const char *allowedDrivers[] = { drivername.c_str(), nullptr };

    std::vector<const char *> optionsArray;
    for (const auto& option : options)
        optionsArray.push_back(option.c_str());
    optionsArray.push_back(nullptr);

    dataset_ = GDALDataset::Open(filename.c_str(), openFlags, allowedDrivers, optionsArray.data());

    if (dataset_ == nullptr)
        throw std::runtime_error("Failed to open dataset");

    if (dataset_->GetRasterCount() != 1)
        throw std::runtime_error("Invalid band count");

    band_ = dataset_->GetRasterBand(1);

    if (band_ == nullptr)
        throw std::runtime_error("Failed to open raster band");
}

Dataset::~Dataset()
{
    GDALClose(dataset_);
}

GDALRasterBand * Dataset::band()
{
    return band_;
}

GeoTransform Dataset::geoTransform() const
{
    std::array<double, 6> gtarray = { 0 }; // ulx xres xrot uly yrot yres
    CPLErr rc = dataset_->GetGeoTransform(gtarray.data());
    if (rc != CE_None)
        throw std::runtime_error("Failed to read dataset geotransform");

    return GeoTransform(gtarray);
}

std::shared_ptr<PJ> Dataset::projection() const
{
    const char *wkt = dataset_->GetProjectionRef();
    if (wkt == nullptr)
        return nullptr;

    return Projection::createObjectFromWKT(wkt);
}

std::array<int, 2> Dataset::dimensions() const
{
    const int nx = band_->GetXSize();
    const int ny = band_->GetYSize();
    return { nx, ny };
}

std::array<int, 2> Dataset::blockDimensions() const
{
    int nx = 0;
    int ny = 0;
    band_->GetBlockSize(&nx, &ny);
    return { nx, ny };
}

double Dataset::offset() const
{
    // Returns 0 if unknown.
    return band_->GetOffset();
}

double Dataset::scale() const
{
    // Returns 1 if unknown.
    return band_->GetScale();
}

double Dataset::noDataValue() const
{
    // Scale and offset are not applied.
    return band_->GetNoDataValue();
}

Window Dataset::window() const
{
    return window_;
}

Window Dataset::blockWindow() const
{
    return blockWindow_;
}

void Dataset::setBoundingBox(std::shared_ptr<PJ> crs, const Projection::ProjectedExtent& bbox)
{
    // Transform bounding box coordinates to dataset projection.
    auto op = Projection::Pipeline(crs, projection());
    double llx = 0, lly = 0, llz = 0;
    double lrx = 0, lry = 0, lrz = 0;
    double ulx = 0, uly = 0, ulz = 0;
    double urx = 0, ury = 0, urz = 0;
    op.forward(bbox.xmin, bbox.ymin, 0, llx, lly, llz); // SW corner
    op.forward(bbox.xmax, bbox.ymin, 0, lrx, lry, lrz); // SE corner
    op.forward(bbox.xmin, bbox.ymax, 0, ulx, uly, ulz); // NW corner
    op.forward(bbox.xmax, bbox.ymax, 0, urx, ury, urz); // NE corner

    // Get the maximum source window covering the projected bounding box.
    double x0 = std::min(llx, ulx); // SW, NW: West
    double y0 = std::min(lly, lry); // SW, SE: South
    double x1 = std::max(lrx, urx); // SE, NE: East
    double y1 = std::max(uly, ury); // NW, NE: North

    // Calculate pixel offsets and window dimensions.
    auto gt = geoTransform();
    double xoffset = (x0 - gt.ulx) / gt.xres; // xoff = (ulx - xorigin) / xres
    double yoffset = (y1 - gt.uly) / gt.yres; // yoff = (uly - yorigin) / yres
    double nx = (x1 - x0) / gt.xres;           // nx = (lrx - ulx) / xres
    double ny = (y0 - y1) / gt.yres;           // ny = (lry - uly) / yres

    // Round to integer pixels.
    xoffset = std::floor(xoffset + 0.001);
    yoffset = std::floor(yoffset + 0.001);
    nx = std::floor(nx + 0.5);
    ny = std::floor(ny + 0.5);

    // Update the dataset window.
    window_.xmin = static_cast<int>(xoffset);      // xmin = x offset, left
    window_.ymax = static_cast<int>(yoffset);      // ymax = y offset, top
    window_.xmax = static_cast<int>(xoffset + nx); // xmax = x offset, right
    window_.ymin = static_cast<int>(yoffset - ny); // ymin = y offset, bottom

    // Update the block window using the natural block size.
    auto blockDims = blockDimensions();
    blockWindow_.xmin = window_.xmin - (window_.xmin % blockDims[0]);
    blockWindow_.ymin = window_.ymin - (window_.ymin % blockDims[1]);
    blockWindow_.xmax = window_.xmax - (window_.xmax % blockDims[0]) + blockDims[0];
    blockWindow_.ymax = window_.ymax - (window_.ymax % blockDims[1]) + blockDims[1];
}

std::vector<float> Dataset::readBlocksInternal(TaskControl& control, Dataset *p)
{
    control.started();

    auto band = p->band();
    auto window = p->window();
    auto blockWindow = p->blockWindow();
    auto blockDims = p->blockDimensions();

    const int xbmin = blockWindow.xmin;
    const int ybmax = blockWindow.ymax;
    const int nxbuf = blockWindow.nx();
    const int nybuf = blockWindow.ny();
    const int xbsize = blockDims[0];
    const int ybsize = blockDims[1];
    const int nxblocks = nxbuf / xbsize;
    const int nyblocks = nybuf / ybsize;

    const int xoffset = window.xmin - blockWindow.xmin;
    const int yoffset = blockWindow.ymax - window.ymax;

    if (nxbuf <= 0 || nybuf <= 0) {
        control.finished();
        throw std::runtime_error("Invalid buffer size");
    }

    // Create the buffer to hold block data.
    float fillValue = static_cast<float>((p->noDataValue() * p->scale()) + p->offset());
    std::vector<float> buffer(nxbuf * nybuf, fillValue);

    // Setup internal progress callback.
    struct ProgressData {
        ProgressData(TaskControl& control) : control(control) {}
        TaskControl& control;
        int currentBlock = 0;
        int totalBlocks = 0;
    };

    ProgressData pdata(control);
    pdata.currentBlock = 0;
    pdata.totalBlocks = nxblocks * nyblocks;

    static auto pfn = [](double complete, const char *message, void *data) {
        auto pd = static_cast<ProgressData *>(data);
        if (pd->control.interruptRequested())
            return 0;

        double start = (double)pd->currentBlock / (double)pd->totalBlocks;
        pd->control.progress(start + complete / pd->totalBlocks);
        return 1;
    };

    // Set nearest neighbor resampling and progress function.
    GDALRasterIOExtraArg extraArg;
    INIT_RASTERIO_EXTRA_ARG(extraArg);
    extraArg.eResampleAlg = GRIORA_NearestNeighbour;
    extraArg.pfnProgress = pfn;
    extraArg.pProgressData = &pdata;

    // Notify driver of upcoming read requests.
    const GDALDataType dataType = band->GetRasterDataType();
    band->AdviseRead(xbmin, ybmax, nxbuf, nybuf, nxbuf, nybuf, dataType, nullptr);

    for (int ixb = 0; ixb < nxblocks; ++ixb)
    {
        for (int iyb = 0; iyb < nyblocks; ++iyb)
        {
            if (control.interruptRequested()) {
                control.finished();
                throw std::runtime_error("Canceled");
            }

            pdata.currentBlock = iyb + ixb * nyblocks;
            control.message(fmt::format("Reading Block {} of {}", pdata.currentBlock + 1, pdata.totalBlocks));

            // Calculate block index in source raster.
            const int xb = xbmin / xbsize + ixb;
            const int yb = ybmax / ybsize + iyb;

            // Read the block using DirectRasterIO. For WCS, this is faster than
            // GDALRasterBand::GetLockedBlockRef or GDALRasterBand::ReadBlock.
            std::vector<float> block(xbsize * ybsize);
            const int xstart = xb * xbsize;
            const int ystart = yb * ybsize;

            CPLErr rc = band->RasterIO(GF_Read, xstart, ystart, xbsize, ybsize, block.data(), xbsize, ybsize, GDT_Float32, 0, 0, &extraArg);
            if (rc != CE_None) {
                control.finished();
                if (control.interruptRequested())
                    throw std::runtime_error("Canceled");
                else
                    throw std::runtime_error("I/O error");
            }

            for (int ix = 0; ix < xbsize; ++ix) {
                for (int iy = 0; iy < ybsize; ++iy) {
                    int pos = (ix + ixb * xbsize) + (iy + iyb * ybsize) * nxbuf;
                    float value = static_cast<float>((block.at(ix + iy * xbsize) * p->scale()) + p->offset());
                    buffer.at(pos) = value;
                }
            }
        }
    }

    // Copy results to output buffer within source window.
    //float *start = &buffer.at(nxbuf * yoffset + xoffset);
    //const GSpacing linespace = sizeof(float) * nxbuf;
    //band->RasterIO(GF_Write, 0, 0, nx, ny, start, nx, ny, GDT_Float32, 0, linespace);

    control.message("Generating DEM");

    std::vector<float> result;
    result.reserve(window.nx() * window.ny());
    for (int ix = xoffset; ix < window.nx(); ++ix) {
        for (int iy = yoffset; iy < window.ny(); ++iy) {
            result.push_back(buffer.at(ix + iy * xbsize));
        }
    }

    control.finished();
    return result;
}

std::future<std::vector<float>> Dataset::readBlocks(TaskControl& control)
{
    return std::async(std::launch::async, readBlocksInternal, std::ref(control), this);
}

void Dataset::exportGeoTIFF(const std::string& filename) const
{
    GDALDriver *driver = DriverManager::instance()->GetDriverByName("GTiff");
    if (driver == nullptr)
        throw std::runtime_error("Failed to load GeoTIFF driver");

    const int nx = window_.nx();
    const int ny = window_.ny();

    if (nx <= 0 || ny <= 0 || nx * ny != values_.size())
        throw std::runtime_error("Invalid window size");

    GDALDatasetUniquePtr dataset(driver->Create(filename.c_str(), nx, ny, 1, GDT_Float32, nullptr));
    GeoTransform gt = geoTransform();
    gt.ulx += window_.xmin; // ulx + offset
    gt.uly -= window_.ymax; // uly + offset
    dataset->SetGeoTransform(gt.array().data());
    dataset->SetProjection(dataset_->GetProjectionRef());

    GDALRasterBand *band = dataset->GetRasterBand(1);
    band->SetUnitType("m");

    float *start = const_cast<float *>(values_.data());
    band->RasterIO(GF_Write, 0, 0, nx, ny, start, nx, ny, GDT_Float32, 0, 0);
}




} // namespace Raster

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

#include "core/TerrainProcessor.h"

#include <array>
#include <cstdlib>
#include <memory>
#include <vector>

#include <boost/compute/system.hpp>
#include <boost/compute/buffer.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/utility/dim.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

// NVIDIA extension: CL_NV_CREATE_BUFFER
typedef cl_bitfield cl_mem_flags_NV;
CL_API_ENTRY cl_mem CL_API_CALL clCreateBufferNV(cl_context, cl_mem_flags, cl_mem_flags_NV, std::size_t, void *, cl_int *);

#ifndef CL_MEM_LOCATION_HOST_NV
#define CL_MEM_LOCATION_HOST_NV (1 << 0)
#endif

#ifndef CL_MEM_PINNED_NV
#define CL_MEM_PINNED_NV (1 << 1)
#endif

namespace compute = boost::compute;

TerrainProcessor::TerrainProcessor()
{
    using compute::int_;
    using compute::float2_;
    using compute::float4_;

    const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE(

        // Interpolate elevation from DEM and set initial critical hill height (Hc).
        __kernel void calculate_elev(__read_only image2d_t dem,
                                     __global float4 * restrict receptor,
                                     const float2 offset,
                                     const float2 res)
        {
            const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
            const uint gid = get_global_id(0);

            float2 pos;
            pos.x = ((float) receptor[gid].x - offset.x) / res.x;
            pos.y = ((float) receptor[gid].y - offset.y) / res.y;

            float elev = read_imagef(dem, sampler, pos).z;

            receptor[gid].z = elev;
            receptor[gid].w = elev;
        }

        // Iterate over DEM nodes and calculate critical hill height (Hc) using AERMAP method (sub_calchc.f).
        __kernel void calculate_hc(__read_only image2d_t dem,
                                   __global float4 * restrict receptor,
                                   const float2 offset,
                                   const float2 res,
                                   volatile __global int *progress)
        {
            const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
            const uint gid = get_global_id(0);
            const int2 dims = (int2)(get_image_width(dem), get_image_height(dem));
            float hcmax = -FLT_MAX;
            float hc;

            float2 node;
            for (int i = 0; i != dims.x; ++i) {
                node.x = offset.x + ((float) i * res.x) + (res.x * 0.5f);
                for (int j = 0; j != dims.y; ++j) {
                    node.y = offset.y + ((float) j * res.y) + (res.y * 0.5f);

                    // Update Hc for DEM node elevation >= 10% slope from receptor.
                    // Horizontal and vertical coordinate system must have the same units.
                    float znode = read_imagef(dem, sampler, (int2)(i, j)).z;
                    float bp = 0.1f * fast_distance(node, receptor[gid].xy);

                    if (znode - receptor[gid].z >= bp) {
                        hc = znode;
                        if (hc > hcmax) {
                            hcmax = hc;
                            receptor[gid].w = hc;
                        }
                    }
                }
            }

            // Requires OpenCL 1.2 for "cl_khr_local_int32_base_atomics" extension.
            atomic_inc(progress);
        }
    );

    compute::device gpu = compute::system::default_device();
    compute::platform platform = gpu.platform();
    compute::context context(gpu);
    compute::command_queue queue(context, gpu);
    compute::program program = compute::program::build_with_source(source, context);

    // Create pinned memory buffer for progress monitoring.
    // Requires clCreateBufferNV() extension function on NVIDIA.

    int_ *progress = nullptr;
    compute::buffer progress_buffer;

    auto clCreateBufferNVPtr = reinterpret_cast<decltype(&clCreateBufferNV)>(platform.get_extension_function_address("clCreateBufferNV"));

    if (clCreateBufferNVPtr != nullptr) {
        int_ error = 0;
        auto mem = clCreateBufferNVPtr(context, CL_MEM_READ_WRITE, CL_MEM_LOCATION_HOST_NV, sizeof(int_), nullptr, &error);
        if (mem != nullptr) {
            progress_buffer = compute::buffer(mem);
        }
        else {
            progress_buffer = compute::buffer(context, sizeof(int_),
                compute::memory_object::alloc_host_ptr | compute::memory_object::read_write);
        }
    }
    else {
        progress_buffer = compute::buffer(context, sizeof(int_),
            compute::memory_object::alloc_host_ptr | compute::memory_object::read_write);
    }

    progress = (int_ *)queue.enqueue_map_buffer(progress_buffer,
        compute::command_queue::map_write, 0, sizeof(int_));

    queue.enqueue_unmap_buffer(progress_buffer, progress);
}

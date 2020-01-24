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

#include "boost/endian/arithmetic.hpp"

namespace IPCMessage
{
    struct header_t {
        boost::endian::little_int32_t cbsize;
        boost::endian::little_int32_t procid;
        boost::endian::little_int32_t msgtyp;
    };

    // Message Type 1
    struct errmsg_buffer_t {
        header_t header;
        char pathwy[2];
        char errtyp;
        char errcod[3];
        boost::endian::little_int32_t lineno;
        char modnam[12];
        char errmg1[50];
        char errmg2[12];
    };

    // Message Type 2
    struct tothrs_buffer_t {
        header_t header;
        boost::endian::little_int32_t tothrs;
    };

    static_assert(sizeof(header_t) == 12u, "IPCMessage::header_t bad size");
    static_assert(sizeof(errmsg_buffer_t) == 96u, "IPCMessage::errmsg_buffer_t bad size");
    static_assert(sizeof(tothrs_buffer_t) == 16u, "IPCMessage::tothrs_buffer_t bad size");
}

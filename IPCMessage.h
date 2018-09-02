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

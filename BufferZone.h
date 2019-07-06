#pragma once

#include <set>
#include <tuple>

struct BufferZone
{
    double areaThreshold = 0;
    double appRateThreshold = 0;
    mutable double distance = 0;
    mutable int duration = 0;

    friend bool operator<(const BufferZone& lhs, const BufferZone& rhs) {
        return std::tie(lhs.areaThreshold, lhs.appRateThreshold)
             < std::tie(rhs.areaThreshold, rhs.appRateThreshold);
    }
};

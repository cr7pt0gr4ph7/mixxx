#pragma once

#include <algorithm>

#include "util/types.h"

// Limits the range of a CSAMPLE value to [-CSAMPLE_PEAK, CSAMPLE_PEAK].
constexpr CSAMPLE CSAMPLE_clamp(CSAMPLE in) {
    static_assert(-CSAMPLE_PEAK <= CSAMPLE_PEAK);
    return std::clamp(in, -CSAMPLE_PEAK, CSAMPLE_PEAK);
}

// Limits the range of a CSAMPLE_GAIN value to [CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX].
constexpr CSAMPLE_GAIN CSAMPLE_GAIN_clamp(CSAMPLE_GAIN in) {
    static_assert(CSAMPLE_GAIN_MIN <= CSAMPLE_GAIN_MAX);
    return std::clamp(in, CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX);
}

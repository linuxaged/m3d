/*
* Copyright (C) 2016 Tracy Ma
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <cmath>

namespace m3d {
namespace math {
    static inline float InvSqrt(float f)
    {
        // TODO: quake 3, neon
        return std::sqrt(1.0f / f);
    }

    struct Range {
    public:
        float min;
        float max;

    public:
        inline float length() { return (max - min); }
    };

    static inline float MapRange(float x, Range fromRange, Range toRange)
    {
        return (x - fromRange.min) * toRange.length() / fromRange.length() + toRange.min;
    }
}
}

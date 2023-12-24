#pragma once

#include "common.h"

namespace Math
{
    float sign(float value);
    float dot(const Vec3D &a, const Vec3D &b);
    Vec3D cross(const Vec3D &a, const Vec3D &b);
} // namespace Math

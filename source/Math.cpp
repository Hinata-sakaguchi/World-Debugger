#include "Math.hpp"

namespace Math
{
    float sign(float value)
    {
        return (value > 0.0f) - (value < 0.0f);
    }

    float dot( const Vec3D& a, const Vec3D& b){
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vec3D cross( const Vec3D& a, const Vec3D& b ){
        return Vec3D{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
} // namespace Math

#pragma once

#include <citro3d.h>

struct Position {
    float x;
    float y;
    float z;

    Position operator+(const Position& p) const
    {
        return Position{x + p.x, y + p.y, z + p.z};
    }

    Position operator-(const Position& p) const
    {
        return Position{x - p.x, y - p.y, z - p.z};
    }

    Position operator*(float f) const
    {
        return Position{x * f, y * f, z * f};
    }

    void operator=(const C3D_FVec& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    C3D_FVec toC3DVec(void) const
    {
        return FVec3_New(x, y, z);
    }
};

using Vec3D = Position;

struct Line {
    Position start;
    Position end;
};

struct Triangle {
    Position vtx[3];
};

struct Vertex { 
    Position position; float texcoord[2]; float normal[3]; 
};

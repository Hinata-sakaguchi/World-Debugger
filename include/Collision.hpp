#pragma once

#include <Object.hpp>

namespace Collision
{
    bool checkCollision(const Object& obj1, const Position &oldPos, const Position &newPos);
}
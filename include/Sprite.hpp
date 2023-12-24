#pragma once

#include <citro3d.h>
#include <citro2d.h>
#include <3ds.h>
#include <string>

#include "common.h"

class SpriteObj {
private:
    C2D_SpriteSheet spriteSheet;

public:
    SpriteObj(const std::string& path);
    ~SpriteObj();
    void setSprite(C2D_Sprite* sprite, size_t index, int posX, int posY);
};
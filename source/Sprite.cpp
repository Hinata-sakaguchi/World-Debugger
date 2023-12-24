#include "Sprite.hpp"

SpriteObj::SpriteObj(const std::string& path) {
    spriteSheet = C2D_SpriteSheetLoad(path.c_str());
}

SpriteObj::~SpriteObj() {
    C2D_SpriteSheetFree(spriteSheet);
}

void SpriteObj::setSprite(C2D_Sprite* sprite, size_t index, int posX, int posY) {
    C2D_SpriteFromSheet(sprite, spriteSheet, index);
    C2D_SpriteSetCenter(sprite, 0.5, 0.5);
	C2D_SpriteSetPos(sprite, posX, posY);
}

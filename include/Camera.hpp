#pragma once

#include <citro3d.h>
#include <3ds.h>

#include "common.h"
#include "Game.hpp"

class Game;
class Camera {
public:
    using MoveCallback = void (*)(Position&);

    Camera(Game *owner, float x, float y, float z);
    ~Camera();

    void control(u32 kDown, u32 kHeld, u32 kUp);
    void updateView(void);

    void moveAxisV(float speed);
    void moveAxisH(float speed);

    float getAngleX() const;
    float getAngleY() const;
    Position getPosition() const;

    void setMoveCallback(MoveCallback callback);

private:
    Game *m_owner;
    C3D_Mtx m_projection;
    Position m_position;
    float m_angleX;
    float m_angleY;
    MoveCallback m_moveCallback;
};
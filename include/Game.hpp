#pragma once

#include <citro3d.h>
#include <citro2d.h>
#include <3ds.h>

#include "Camera.hpp"
#include "Uniforms.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "Sprite.hpp"

class Camera;
class Game {
public:
    Game(void);
    ~Game(void);

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    void control(u32 kDown, u32 kHeld, u32 kUp);
    void renderTop(void);
    void renderBottom(void);

    static Game &getInstance(void);
    
private:
    friend class Camera;

    DVLB_s* m_dvlb;
    shaderProgram_s m_shader;
    Object *m_room;
    Object *m_car;
    Object *m_sea;
    Object *m_ground;
    Object *m_stop;
    Object *m_titleTown;
    Object *m_light;
    UniformManager *m_uniformManager;
    LightManager *m_lightManager;
    Camera *m_camera;
    C2D_TextBuf m_textBuf;
    C3D_FogLut fog_Lut;
    SpriteObj *spriteSheet;
    C2D_Sprite collaboraterSprite;
    C2D_Sprite companySprite;
    C2D_Sprite logoSprite;
    time_t programStartTime;
    time_t currentTime;
    static Game *m_instance;

    static void moveCallback(Position &pos);
    void renderPrepare(void);
};
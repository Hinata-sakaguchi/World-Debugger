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
    void handleInput(u32 kDown, u32 kHeld, u32 kUp);
    void DeleteStage(void);
    void titleMenu(u32 kDown, u32 kHeld, u32 kUp);
    void startTown(void);
    void renderTop(u32 kDown, u32 kHeld, u32 kUp);
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
    Object *m_lamp;
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
    C2D_Sprite ChapterSprite;
    C2D_Sprite NewGameSprite;
    C2D_Sprite SettingSprite;
    C2D_Sprite StaffRoleSprite;
    bool NewGameIsSelect;
    bool ChapterIsSelect;
    bool SettingIsSelect;
    bool StaffRoleIsSelect;

    bool judge;
    static Game *m_instance;

    typedef enum {
        MENU_NEW_GAME,
        MENU_CHAPTER,
        MENU_SETTING,
        MENU_STAFF_ROLE
    } MenuOption;

    typedef enum {
        TITLE_MENU,
        START_TOWN
    } NowStage;

    MenuOption selectedOption;
    NowStage   nowStage;
    

    static void moveCallback(Position &pos);
    void renderPrepare(void);
};
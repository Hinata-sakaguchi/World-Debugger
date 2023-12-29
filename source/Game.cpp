#include "Game.hpp"
#include "all_vertex.hpp"
#include "vshader_shbin.h"
#include "Utils.hpp"


#define SAFE_DELETE(x) if(x) { delete x; x = nullptr; }

Game *Game::m_instance = nullptr;

Game::Game(void)
{
    if(m_instance != nullptr)
        svcBreak(USERBREAK_PANIC);

    m_instance = this;

    m_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&m_shader);
    shaderProgramSetVsh(&m_shader, &m_dvlb->DVLE[0]);

    m_uniformManager = new UniformManager(m_shader.vertexShader);
    m_uniformManager->registerAsInstance();

    m_car = new Object("romfs:/gfx/car.t3x", Vertices::carVertices, Vertices::carVertexCount);
    m_sea = new Object("romfs:/gfx/sea.t3x", Vertices::seaVertices, Vertices::seaVertexCount);
    m_ground = new Object("romfs:/gfx/ground.t3x", Vertices::groundVertices, Vertices::groundVertexCount);
    m_stop = new Object("romfs:/gfx/stop.t3x", Vertices::stopVertices, Vertices::stopVertexCount);
    m_titleTown = new Object("romfs:/gfx/titleTown.t3x", Vertices::titleTownVertices, Vertices::titleTownVertexCount);
    m_lamp = new Object("romfs:/gfx/lamp.t3x", Vertices::lampVertices, Vertices::lampVertexCount);
    spriteSheet = new SpriteObj("romfs:/gfx/title.t3x");

    m_lightManager = new LightManager();

    m_camera = new Camera(this, 0, 2.0f, 0);
    m_camera->setMoveCallback(moveCallback);


    m_textBuf = C2D_TextBufNew(4096);

    NewGameIsSelect = true;
    ChapterIsSelect = false;
    SettingIsSelect = false;
    StaffRoleIsSelect = false;


    selectedOption = MENU_NEW_GAME;

    //Defalut lights
    for(u32 i = 2; i <= 8; i++)
    {
        m_lightManager->enable(i);
    }
    spriteSheet->setSprite(&companySprite, 0, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&collaboraterSprite, 1, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&logoSprite, 2, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&ChapterSprite, 3, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&NewGameSprite, 4, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&SettingSprite, 5, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&StaffRoleSprite, 6, 400 / 2, 240 / 2);

    FogLut_Exp(&fog_Lut, 0.01f, 2.5f, 0.01f, 60.0f);
	C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
	C3D_FogColor(0x7C542C);
	C3D_FogLutBind(&fog_Lut);
}

Game::~Game(void)
{

}

void Game::DeleteStage(void)
{
    C2D_TextBufDelete(m_textBuf);
    SAFE_DELETE(m_car);
    SAFE_DELETE(m_sea);
    SAFE_DELETE(m_ground);
    SAFE_DELETE(m_lightManager);
    SAFE_DELETE(m_stop);
    SAFE_DELETE(m_titleTown);
    // SAFE_DELETE(m_camera);
    SAFE_DELETE(m_lamp);
    SAFE_DELETE(spriteSheet);
    //SAFE_DELETE(m_uniformManager);
    // shaderProgramFree(&m_shader);
    // DVLB_Free(m_dvlb);
}

void Game::moveCallback(Position &pos)
{
    /*
        No.3 ~ 7は常に点灯
    */
    LightManager &lm = *(getInstance().m_lightManager);

    // //lm.disable(3);
    // lm.disable(4);
    //lm.disable(5);
    lm.disable(6);
    lm.disable(7);
    lm.disable(1);
    lm.disable(2);
    lm.disable(8);
    lm.disable(9);
    lm.disable(10);
}

void Game::control(u32 kDown, u32 kHeld, u32 kUp)
{
    m_camera->control(kDown, kHeld, kUp);
}

void Game::handleInput(u32 kDown, u32 kHeld, u32 kUp) 
{
    switch (selectedOption)
    {
        case MENU_NEW_GAME:
            C2D_DrawSprite(&NewGameSprite);
            if (kDown & KEY_DDOWN) {
                selectedOption = MENU_CHAPTER;
            } else if (kDown & KEY_A) {
                nowStage = START_TOWN;
                DeleteStage();
            }
            break;
        case MENU_CHAPTER:
            C2D_DrawSprite(&ChapterSprite);
            if (kDown & KEY_DUP) {
                selectedOption = MENU_NEW_GAME;
            } else if (kDown & KEY_DDOWN) {
                selectedOption = MENU_SETTING;
            } else if (kDown & KEY_A) {

            }
            break;
        case MENU_SETTING:
            C2D_DrawSprite(&SettingSprite);
            if (kDown & KEY_DUP) {
                selectedOption = MENU_CHAPTER;
            } else if (kDown & KEY_DDOWN) {
                selectedOption = MENU_STAFF_ROLE;
            } else if (kDown & KEY_A) {
                // Settingが選択された時の処理
            }
            break;
        case MENU_STAFF_ROLE:
            C2D_DrawSprite(&StaffRoleSprite);
            if (kDown & KEY_DUP) {
                selectedOption = MENU_SETTING;
            } else if (kDown & KEY_DDOWN) {
                selectedOption = MENU_NEW_GAME;
            } else if (kDown & KEY_A) {
                // Staff Roleが選択された時の処理
            }
            break;
        default:
            break;
    }
}

void Game::renderPrepare(void)
{
    // Bind a shader program
    C3D_BindProgram(&m_shader);
    
    // Setup attributes
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal
    
    // Setup TexEnv
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_SECONDARY_COLOR, (GPU_TEVSRC)0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

    for(u32 i = 1; i < 5; i++) {
        C3D_TexEnvInit(C3D_GetTexEnv(i));
    }

    // Setup Light
    m_lightManager->bind();

    C3D_CullFace(GPU_CULL_BACK_CCW);
    //C3D_CullFace(GPU_CULL_NONE);
}



void Game::renderTop(u32 kDown, u32 kHeld, u32 kUp)
{
    switch (nowStage)
    {
        case TITLE_MENU:
            titleMenu(kDown, kHeld, kUp);
            break;

        case START_TOWN:
            control(kDown, kHeld, kUp);
            startTown();
            break;
        
        default:
            break;
    }
}

void Game::renderBottom(void)
{
    C2D_Prepare();

    static char buffer[1024];
    C2D_Text text;

    C2D_TextBufClear(m_textBuf);

    Position pos = m_camera->getPosition();

    sprintf(buffer, "FPS: %d\nAngleX: %f AngleY: %f\nX: %f Y: %f Z: %f\n\n", Utils::fpsCounter(), m_camera->getAngleX(), m_camera->getAngleY(), pos.x, pos.y, pos.z);
    std::string str(buffer);

    for(u32 i = 0; i < m_lightManager->getLightDefCount(); i++) {
        int enabled = m_lightManager->isEnabled(i);

        str += std::to_string(enabled) + " ";
    }
    
    C2D_TextParse(&text, m_textBuf, str.c_str());
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor, 0, 0, 0, 0.5f, 0.5f, 0xFFFFFFFF);
}

Game &Game::getInstance(void)
{
    return *m_instance;
}
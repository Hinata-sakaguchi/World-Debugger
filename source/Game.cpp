#include "Game.hpp"
#include "all_vertex.hpp"
#include "vshader_shbin.h"
#include "Utils.hpp"


#define SAFE_DELETE(x) if(x) { delete x; x = nullptr; }

Game *Game::m_instance = nullptr;

Game::Game(void)
{
    programStartTime = time(NULL);
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
    spriteSheet = new SpriteObj("romfs:/gfx/title.t3x");

    m_lightManager = new LightManager();

    m_camera = new Camera(this, 0, 2.0f, 0);
    // m_camera->setMoveCallback(moveCallback);

    m_textBuf = C2D_TextBufNew(4096);

    //Defalut lights
    for(u32 i = 2; i <= 8; i++)
    {
        m_lightManager->enable(i);
    }

    spriteSheet->setSprite(&collaboraterSprite, 1, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&companySprite, 0, 400 / 2, 240 / 2);
    spriteSheet->setSprite(&logoSprite, 2, 400 / 2, 240 / 2);


    FogLut_Exp(&fog_Lut, 0.05f, 1.5f, 0.01f, 20.0f);
	C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
	C3D_FogColor(0x7C542C);
	C3D_FogLutBind(&fog_Lut);

    //m_titleTown->setRotateY(C3D_AngleFromDegrees(180.0f));
}

Game::~Game(void)
{
    C2D_TextBufDelete(m_textBuf);

    SAFE_DELETE(m_car);
    SAFE_DELETE(m_sea);
    SAFE_DELETE(m_ground);
    SAFE_DELETE(m_light);
    SAFE_DELETE(m_stop);
    SAFE_DELETE(m_titleTown);
    SAFE_DELETE(m_uniformManager);
    SAFE_DELETE(m_camera);
    SAFE_DELETE(spriteSheet);
    shaderProgramFree(&m_shader);
    DVLB_Free(m_dvlb);
}

void Game::moveCallback(Position &pos)
{
    /*
        No.3 ~ 7は常に点灯
    */
    LightManager &lm = *(getInstance().m_lightManager);
    // Loop
    if(pos.z < -32.5)
    {
        lm.disable(8);
        lm.disable(9);
        lm.disable(10);

        lm.enable(0);
        lm.enable(1);
        lm.enable(2);

        pos.x += 30;
        pos.z = 32.5;
    }
    else if(pos.z > 32.5)
    {
        lm.disable(0);
        lm.disable(1);
        lm.disable(2);

        lm.enable(8);
        lm.enable(9);
        lm.enable(10);
    
        pos.x -= 30;
        pos.z = -32.5;
    }

    /* Corner2のライト */
    if(pos.z < 0)
    {
        lm.disable(1);
        lm.enable(9);
    }
    if(pos.z > 0)
    {
        lm.disable(9);
        lm.enable(1);
    }

    if(pos.z < -22.5f)
    {
        lm.disable(2);
        lm.enable(10);
    }
    if(pos.z > -22.5f)
    {
        lm.disable(10);
        lm.enable(2);
    }
    if(pos.z > 22.5f)
    {
        lm.disable(8);
        lm.enable(0);
    }
    if(pos.z < 22.5f)
    {
        lm.disable(0);
        lm.enable(8);
    }
}

void Game::control(u32 kDown, u32 kHeld, u32 kUp)
{
    m_camera->control(kDown, kHeld, kUp);
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
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, (GPU_TEVSRC)0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

    for(u32 i = 1; i < 5; i++) {
        C3D_TexEnvInit(C3D_GetTexEnv(i));
    }

    // Setup Light
    m_lightManager->bind();

    C3D_CullFace(GPU_CULL_BACK_CCW);
    //C3D_CullFace(GPU_CULL_NONE);
}

void Game::renderTop(void)
{
    currentTime = time(NULL);
	if (difftime(currentTime, programStartTime) >= 0)
	{
        // C2D_Prepare();
		// C2D_DrawSprite(&companySprite);
		if (difftime(currentTime, programStartTime) >= 4)
		{
			// C2D_DrawSprite(&collaboraterSprite);
			if (difftime(currentTime, programStartTime) > 7)
            {
				//C2D_DrawSprite(&logoSprite);
                
                renderPrepare();        
                m_camera->updateView();
                // m_car->render();
                m_sea->render();
                m_ground->render();
                // m_stop->render();
                //m_titleTown->render(); //テストとしてここは表示させない
            }    
		}
        C2D_Flush();
	}

    m_light->setScale(2.0f, 1.5f, 1.5f);

    for(u32 i = 0; i < m_lightManager->getLightDefCount(); i++) {
        const auto &lightDef = m_lightManager->getLightDef(i);
        Position pos = lightDef.pos;
        m_light->setPosition(pos.x, pos.y - 1.4f, pos.z);
        m_light->setRotateY(C3D_AngleFromDegrees(lightDef.dir));
        m_light->render();
    }

    // m_numBoard->setScale(0.9);
    // m_numBoard->setRotateY(C3D_AngleFromDegrees(180.0f));

    // m_numBoard->setPosition(-12.6f, 0.5f, -30.0f);
    // m_numBoard->render();

    // m_numBoard->setPosition(17.4f, 0.5f, 35.0f);
    // m_numBoard->render();

    // m_ceilGuide->setPosition(0, 4.1f, 0);
    // m_ceilGuide->render();

    
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
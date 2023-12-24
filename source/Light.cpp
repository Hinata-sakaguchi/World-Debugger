#include "Light.hpp"

constexpr float Ceil = 5.78f;

static const C3D_Material material =
{
    { 0.2f, 0.2f, 0.2f }, // 環境
    { 0.9f, 0.9f, 0.9f }, // 拡散
    { 0.8f, 0.8f, 0.8f }, // 鏡面1
    { 0.0f, 0.0f, 0.0f }, // 鏡面2
    { 0.9f, 0.9f, 0.9f }, // 放出
};

LightManager::LightManager(void): m_busyIndex(0)
{
    C3D_LightEnvInit(&m_env);
    C3D_LightEnvMaterial(&m_env, &material);

    LightLut_Phong(&m_lut, 0.8f);
    C3D_LightEnvLut(&m_env, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &m_lut);

    LightLutDA_Quadratic(&m_lutDa, 1.0f, 10.f, 0.021, 0.021);
    LightLutDA_Quadratic(&m_lutDaHighPower, 1.0f, 14.f, 0.011, 0.011);

    // Init lights
    for(u32 i = 0; i < 8; i++) {
        C3D_Light *light = new C3D_Light;

        C3D_LightInit(light, &m_env);
        C3D_LightColor(light, 0.96f, 0.96f, 1.0f);

        //C3D_LightDistAttn(light, &m_lutDa);
        C3D_LightDistAttnEnable(light, true);

        C3D_LightEnable(light, false);
        
        m_lights.push_back(light);
    }

    initLightDefs();
}

LightManager::~LightManager(void)
{

}

void LightManager::initLightDefs(void)
{
    m_lightDefs = {
        {{22.5f, Ceil, 40.0f}, 90, true},  // 0
        // Corner3
        {{15.f, Ceil, 32.50f}, 0},     // 1
        // Corner4
        {{7.5f, Ceil, 25.0f}, 90.f},   // 2
        // Main street
        {{0.0f, Ceil, 20.0f}, 0},     // 3
        {{0.0f, Ceil, 10.0f}, 0},     // 4
        {{0.0f, Ceil, 0.0f}, 0},       // 5 
        {{0.0f, Ceil, -10.0f}, 0},      // 6
        {{0.0f, Ceil, -20.0f}, 0},      // 7
        // Corner5
        {{-7.5f, Ceil, -25.0f}, 90.f}, // 8
        // Corner6
        {{-15.f, Ceil, -32.50f}, 0},   // 9
        // Corner7
        {{-22.5f, Ceil, -40.0f}, 90, true},  // 10
    };
}

void LightManager::bind(void)
{
    C3D_LightEnvBind(&m_env);
}

u32 LightManager::getLightDefCount(void) const
{
    return m_lightDefs.size();
}

const LightDef &LightManager::getLightDef(u32 index) const
{
    return m_lightDefs[index];
}

void LightManager::enable(u32 index)
{
    if(isEnabled(index)) {
        return;
    }

    LightDef &lightDef = m_lightDefs[index];
    C3D_FVec pos = lightDef.pos.toC3DVec();
    s8 lightIndex = -1;

    pos.w = 1.0f;

    // Find free light
    for(u32 i = 0; i < m_lights.size(); i++) {
        bool isEnabled = m_lights[i]->flags & 1;

        if(!isEnabled) {
            lightIndex = i;
            break;
        }
    }

    // Force disable older light
    if(lightIndex == -1) {
        lightIndex = m_busyIndex;
        C3D_LightEnable(m_lights[lightIndex], false);

        if(++m_busyIndex >= (s8)m_lights.size()) {
            m_busyIndex = 0;
        }
    }

    C3D_Light *light = m_lights[lightIndex];

    if(lightDef.highPower) {
        C3D_LightDistAttn(light, &m_lutDaHighPower);
    }
    else {
        C3D_LightDistAttn(light, &m_lutDa);
    }

    C3D_LightPosition(light, &pos);
    C3D_LightEnable(light, true);
    lightDef.lightIndex = lightIndex;
}

void LightManager::disable(u32 index)
{
    if(m_lightDefs[index].lightIndex == -1) {
        return;
    }

    C3D_Light *light = m_lights[m_lightDefs[index].lightIndex];

    C3D_LightEnable(light, false);

    m_lightDefs[index].lightIndex = -1;
}

bool LightManager::isEnabled(u32 index)
{
    return m_lightDefs[index].lightIndex != -1;
}
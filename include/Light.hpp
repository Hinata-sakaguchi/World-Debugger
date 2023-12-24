#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <vector>

#include "common.h"

struct LightDef {
    Position pos;
    float dir; // Degree
    bool highPower = false;
    s8 lightIndex = -1;
};

class LightManager {
public:
    LightManager(void);
    ~LightManager(void);

    void bind(void);

    u32 getLightDefCount(void) const;
    const LightDef &getLightDef(u32 index) const;

    void enable(u32 index);
    void disable(u32 index);
    bool isEnabled(u32 index);

private:
    C3D_LightLut m_lut;
    C3D_LightEnv m_env;
    C3D_LightLutDA m_lutDa;
    C3D_LightLutDA m_lutDaHighPower;
    s8 m_busyIndex;

    std::vector<C3D_Light *> m_lights;
    std::vector<LightDef> m_lightDefs;

    void initLightDefs(void);
    C3D_Light *getLightById(s8 id);
};
#pragma once

#include <3ds.h>
#include <map>
#include <citro3d.h>

class UniformManager {
public:
    enum class Id {
        World = 0,
        View,
        Projection,
    };

    UniformManager(shaderInstance_s *shader);
    ~UniformManager();

    void registerAsInstance(void);

    s8 getLocation(Id id);
    void setMatrix4x4(Id id, const C3D_Mtx &matrix);
    void setFVector(Id id, const C3D_FVec &vector);

    static UniformManager &getInstance();

private:
    std::map<Id, s8> m_locations;
    shaderInstance_s *m_shader;

    static UniformManager *m_instance;

    void addUniform(Id id, const char *unifName);
};
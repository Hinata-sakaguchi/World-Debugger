#include "Uniforms.hpp"

UniformManager *UniformManager::m_instance = nullptr;

UniformManager::UniformManager(shaderInstance_s *shader): m_shader(shader)
{
    addUniform(Id::Projection, "projection");
    addUniform(Id::View, "view");
    addUniform(Id::World, "world");
}

UniformManager::~UniformManager() {}

void UniformManager::registerAsInstance(void)
{
    m_instance = this;
}

void UniformManager::addUniform(Id id, const char *unifName)
{
    m_locations[id] = shaderInstanceGetUniformLocation(m_shader, unifName);
}

s8 UniformManager::getLocation(Id id)
{
    return m_locations[id];
}

void UniformManager::setMatrix4x4(Id id, const C3D_Mtx &matrix)
{
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, m_locations[id], &matrix);
}

void UniformManager::setFVector(Id id, const C3D_FVec &vector)
{
    C3D_FVUnifSet(GPU_VERTEX_SHADER, m_locations[id], vector.x, vector.y, vector.z, vector.w);
}

UniformManager &UniformManager::getInstance(void)
{
    return *m_instance;
}
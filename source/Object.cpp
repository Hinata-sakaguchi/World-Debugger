#include "Object.hpp"
#include "Uniforms.hpp"

#include <tex3ds.h>

static bool loadTextureFromFile(C3D_Tex* tex, C3D_TexCube* cube, const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f)
        return false;

    Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, tex, cube, false);
    fclose(f);
    if (!t3x)
        return false;

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

Object::Object(const std::string &texturePath, const Vertex *vertices, u32 numVertices) : 
    m_pos({0.0f, 0.0f, 0.0f}), m_scale({1.0f, 1.0f, 1.0f}), m_rotateX(0.0f), m_rotateY(0.0f),
    m_rotateZ(0.0f), m_vertices(vertices), m_numVertices(numVertices), m_textureIndex(0)
{
    addTexture(texturePath);

    // Create VBO
    m_vbo = linearAlloc(numVertices * sizeof(Vertex));
    memcpy(m_vbo, vertices, numVertices * sizeof(Vertex));
}

Object::~Object() {
    if(m_vbo)
    {
        linearFree(m_vbo);
        m_vbo = nullptr;
    }

    for(auto &tex : m_textures)
    {
        C3D_TexDelete(&tex);
    }
}

void Object::setPosition(const Position &pos) { 
    m_pos = pos; 
}

void Object::setPosition(float x, float y, float z) {
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
}

void Object::setScale(const Vec3D &scale) {
    m_scale = scale;
}

void Object::setScale(float x, float y, float z) {
    m_scale.x = x;
    m_scale.y = y;
    m_scale.z = z;
}

void Object::setScale(float scale) {
    m_scale.x = scale;
    m_scale.y = scale;
    m_scale.z = scale;
}

void Object::setRotateX(float angle) {
    m_rotateX = angle;
}

void Object::setRotateY(float angle) {
    m_rotateY = angle;
}

void Object::setRotateZ(float angle) {
    m_rotateZ = angle;
}

Position Object::getPosition() const {
    return m_pos;
}

Position Object::getScale() const {
    return m_scale;
}

float Object::getRotateX() const {
    return m_rotateX;
}

float Object::getRotateY() const {
    return m_rotateY;
}

float Object::getRotateZ() const {
    return m_rotateZ;
}

u32 Object::getNumVertices() const {
    return m_numVertices;
}

const Vertex *Object::getVertices() const {
    return m_vertices;
}

void Object::addTexture(const std::string &texturePath) {
    u32 index = m_textures.size();

    m_textures.resize(index + 1);

    if(!loadTextureFromFile(&m_textures[index], nullptr, texturePath.c_str()))
        svcBreak(USERBREAK_PANIC);

    C3D_TexSetFilter(&m_textures[index], GPU_LINEAR, GPU_NEAREST);
}

void Object::setTexture(u32 index) {
    if(index < m_textures.size()) {
        m_textureIndex = index;
    }
}

void Object::render(void) {

    // Set matrix
    C3D_Mtx world;
    Mtx_Identity(&world);
    Mtx_RotateZ(&world, m_rotateZ, false);
    Mtx_RotateY(&world, m_rotateY, false);
    Mtx_RotateX(&world, m_rotateX, false);
    Mtx_Scale(&world, m_scale.x, m_scale.y, m_scale.z);
    Mtx_Translate(&world, m_pos.x, m_pos.y, m_pos.z, false);

    // Apply matrix
    UniformManager::getInstance().setMatrix4x4(UniformManager::Id::World, world);

    // Set VBO
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, m_vbo, sizeof(Vertex), 3, 0x210);

    // Set texture
    C3D_TexBind(0, &m_textures[m_textureIndex]);

    // Draw
    C3D_DrawArrays(GPU_TRIANGLES, 0, m_numVertices);
}
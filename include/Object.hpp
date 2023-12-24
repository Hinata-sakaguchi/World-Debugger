#pragma once

#include <citro3d.h>
#include <3ds.h>
#include <memory>
#include <string>
#include <vector>

#include "common.h"

class Object {
public:
    Object(const std::string &texturePath, const Vertex *vertices, u32 numVertices);
    ~Object();

    Object(const Object &) = delete;
    Object &operator=(const Object &) = delete;

    void render(void);

    void setPosition(const Position &pos);
    void setPosition(float x, float y, float z);
    void setScale(const Vec3D &scale);
    void setScale(float x, float y, float z);
    void setScale(float scale);
    void setRotateX(float angle);
    void setRotateY(float angle);
    void setRotateZ(float angle);

    Position getPosition() const;
    Vec3D getScale() const;
    float getRotateX() const;
    float getRotateY() const;
    float getRotateZ() const;
    u32 getNumVertices() const;
    const Vertex *getVertices() const;

    void addTexture(const std::string &texturePath);
    void setTexture(u32 index);

private:
    Position m_pos;
    Vec3D m_scale;
    float m_rotateX;
    float m_rotateY;
    float m_rotateZ;
    const Vertex *m_vertices;
    void *m_vbo;
    u32 m_numVertices;
    std::vector<C3D_Tex> m_textures;
    u32 m_textureIndex;
};
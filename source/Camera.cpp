#include "Camera.hpp"
#include "Uniforms.hpp"
#include "Math.hpp"
#include "Collision.hpp"

#include <algorithm>

Camera::Camera(Game *owner, float x, float y, float z): m_owner(owner), m_position{x, y, z}, m_angleX(0.0f), m_angleY(0.0f), m_moveCallback(nullptr)
{
    Mtx_PerspTilt(&m_projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.001f, 1000.0f, false);
}

Camera::~Camera() {}

void Camera::moveAxisV(float speed)
{
    float newPosX = m_position.x - speed * sin(m_angleY);
    float newPosZ = m_position.z + speed * cos(m_angleY);

    constexpr auto distance = 0.2f;

    if(Collision::checkCollision(*m_owner->m_sea, m_position, Position{newPosX - distance * Math::sign(sin(m_angleY)) * Math::sign(speed), m_position.y, m_position.z}))
    {
        m_position.x = newPosX;
    }

    if(Collision::checkCollision(*m_owner->m_sea, m_position, Position{m_position.x, m_position.y, newPosZ + distance * Math::sign(cos(m_angleY) * Math::sign(speed))}))
    {
        m_position.z = newPosZ;
    }
}

void Camera::moveAxisH(float speed)
{
    float newPosX = m_position.x + speed * cos(m_angleY);
    float newPosZ = m_position.z + speed * sin(m_angleY);

    constexpr auto distance = 0.2f;

    if(Collision::checkCollision(*m_owner->m_sea, m_position, Position{newPosX + distance * Math::sign(cos(m_angleY)) * Math::sign(speed), m_position.y, m_position.z}))
    {
        m_position.x = newPosX;
    }

    if(Collision::checkCollision(*m_owner->m_sea, m_position, Position{m_position.x, m_position.y, newPosZ + distance * Math::sign(sin(m_angleY)) * Math::sign(speed)}))
    {
        m_position.z = newPosZ;
    }
}

void Camera::control(u32 kDown, u32 kHeld, u32 kUp)
{
    constexpr float moveSpeed = 0.15f;
    float speedV = 0.0f, speedH = 0.0f;

    if(kHeld & KEY_DLEFT)  m_angleY -= C3D_AngleFromDegrees(2.0f);
    if(kHeld & KEY_DRIGHT) m_angleY += C3D_AngleFromDegrees(2.0f);
    if(kHeld & KEY_DUP)    m_angleX -= C3D_AngleFromDegrees(2.0f);
    if(kHeld & KEY_DDOWN)  m_angleX += C3D_AngleFromDegrees(2.0f);
    if(kHeld & KEY_CPAD_UP)    speedV = moveSpeed;
    if(kHeld & KEY_CPAD_DOWN)  speedV = -moveSpeed;
    if(kHeld & KEY_CPAD_LEFT)  speedH = moveSpeed;
    if(kHeld & KEY_CPAD_RIGHT) speedH = -moveSpeed;

    if(speedV != 0.0f)
        moveAxisV(speedV);

    if(speedH != 0.0f)
        moveAxisH(speedH);

    if(m_moveCallback)
        m_moveCallback(m_position);
    
    // clamp angles
    m_angleX = std::clamp((double)m_angleX, -M_PI / 2, M_PI / 2);
}

void Camera::updateView(void)
{
    auto &um = UniformManager::getInstance();
    C3D_Mtx view;

    Mtx_Identity(&view);
    Mtx_LookAt(&view, m_position.toC3DVec(), FVec3_New(m_position.x, m_position.y, m_position.z + 0.1), FVec3_New(0, 1, 0), false);
    Mtx_RotateY(&view, m_angleY, false);
    Mtx_RotateX(&view, m_angleX, false);

    um.setMatrix4x4(UniformManager::Id::Projection, m_projection);
    um.setMatrix4x4(UniformManager::Id::View, view);
}

float Camera::getAngleX() const
{
    return m_angleX;
}

float Camera::getAngleY() const
{
    return m_angleY;
}

Position Camera::getPosition() const
{
    return m_position;
}

void Camera::setMoveCallback(MoveCallback callback)
{
    m_moveCallback = callback;
}
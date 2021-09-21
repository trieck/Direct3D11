#include "stdafx.h"
#include "Camera.h"

void Camera::SetPosition(XMFLOAT3 position)
{
    m_position = std::move(position);
}

void Camera::SetRotation(XMFLOAT3 rotation)
{
    m_rotation = std::move(rotation);
}

const XMFLOAT3& Camera::position() const
{
    return m_position;
}

const XMFLOAT3& Camera::rotation() const
{
    return m_rotation;
}

XMMATRIX Camera::view() const
{
    // setup vector that points up
    XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };

    // setup where the camera is looking by default
    XMFLOAT3 lookAt = { 0.0f, 0.0f, 1.0f };

    // Set the pitch (X axis), yaw (Y axis) and roll (Z axis) rotations in radians
    auto pitch = XMConvertToRadians(m_rotation.x);
    auto yaw = XMConvertToRadians(m_rotation.y);
    auto roll = XMConvertToRadians(m_rotation.z);

    auto rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // // Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin
    auto vLookAt = XMVector3Transform(XMLoadFloat3(&lookAt), rotation);
    auto vUp = XMVector3Transform(XMLoadFloat3(&up), rotation);

    // Translate the rotated camera position to the location of the viewer
    auto vPosition = XMLoadFloat3(&m_position);
    vLookAt += vPosition;

    // create the view matrix from the three updated vectors
    auto view = XMMatrixLookAtLH(vPosition, vLookAt, vUp);

    return view;
}

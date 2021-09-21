#pragma once

class Camera
{
public:
    Camera() = default;
    ~Camera() = default;

    void SetPosition(XMFLOAT3 position);
    void SetRotation(XMFLOAT3 rotation);

    const XMFLOAT3& position() const;
    const XMFLOAT3& rotation() const;
    XMMATRIX view() const;
    
private:
    XMFLOAT3 m_position{};
    XMFLOAT3 m_rotation{};
};

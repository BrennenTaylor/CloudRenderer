#include "Transform.h"

namespace Farlor
{
    Transform::Transform()
        : m_position{0.0f, 0.0f, 0.0f}
        , m_rotation{0.0f, Vector3(0.0f, 0.0f, 0.0f)}
        , m_scale{1.0f, 1.0f, 1.0f}
        , m_world{}
    {
        UpdateWorld();
    }

    Transform::~Transform()
    {
    }

    void Transform::UpdateWorld()
    {
        m_world = Matrix4x4::TranslationMatrix(m_position) * Matrix4x4::RotationMatrix(m_rotation) * Matrix4x4::ScaleMatrix(m_scale);
    }

    void Transform::SetPosition(Vector3 &position)
    {
        m_position = position;
    }

    void Transform::SetRotation(Quaternion &rotation)
    {
        m_rotation = rotation;
    }

    void Transform::SetScale(Vector3& scale)
    {
        m_scale = scale;
    }

    Transform Transform::s_Identity = Transform();
}
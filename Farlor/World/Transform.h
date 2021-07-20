#pragma once

#include <FMath/FMath.h>

#include <map>
#include <unordered_map>
#include <memory>

namespace Farlor
{
    class Transform
    {
    public:
        Transform();
        ~Transform();

        void UpdateWorld();
        const Matrix4x4& GetWorld() const
        {
            return m_world;
        }

        void SetPosition(Vector3& position);
        void SetRotation(Quaternion& rotation);
        void SetScale(Vector3& scale);

    public:
        static Transform s_Identity;

    private:
        Vector3 m_position;
        Quaternion m_rotation;
        Vector3 m_scale;
        Matrix4x4 m_world;
    };
}
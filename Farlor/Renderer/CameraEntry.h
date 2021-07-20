#pragma once

#include <Fmath/FMath.h>

namespace Farlor
{
    // Only information needed about camera
    struct CameraEntry
    {
        Vector3 m_position;
        float m_fov;
        Vector3 m_target;
        float m_cameraMoved;
        Vector3 m_worldUp;

        Matrix4x4 m_view;
        Matrix4x4 m_proj;

        CameraEntry()
            : m_position(0.0, 0.0, 0.0)
            , m_fov(60.0f)
            , m_target(0.0, 0.0, 1.0)
            , m_cameraMoved(true)
            , m_worldUp(0.0, 1.0, 0.0)
            , m_view(Farlor::Matrix4x4::s_Identity)
            , m_proj(Farlor::Matrix4x4::s_Identity)
        {
        }
    };
}
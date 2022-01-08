#pragma once

#include <FMath\FMath.h>

#include <cstdint>

namespace Farlor
{
    // Random number seed pass
    struct RandomGeneratorControl
    {
        uint32_t m_frameCount;
        Vector3 m_pad;
    };

    // Iteration
    struct PathTraceRay
    {
        Vector3 m_start;
        float m_t;
        Vector3 m_dir;
        float m_pad;
    };
}
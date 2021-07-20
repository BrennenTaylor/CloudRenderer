#pragma once

#include "Mesh.h"

#include <FMath\Vector3.h>

namespace Farlor
{
    class AreaLight
    {
    public:
        AreaLight();
        AreaLight(const Farlor::Vector3& intensity);

        void SetGeometryId(uint32_t geometryId);

    private:
        Farlor::Vector3 m_intensity;
        uint32_t m_geometryId;
    };
}
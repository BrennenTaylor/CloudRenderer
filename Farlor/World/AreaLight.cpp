#include "AreaLight.h"

namespace Farlor
{
    AreaLight::AreaLight()
        : m_intensity{ 0.0f, 0.0f, 0.0f }
        , m_geometryId{ 0 }
    {
    }

    AreaLight::AreaLight(const Farlor::Vector3& intensity)
        : m_intensity{ intensity }
        , m_geometryId{ 0 }
    {
    }

    void AreaLight::SetGeometryId(uint32_t geometryId)
    {
        m_geometryId = geometryId;
    }
}
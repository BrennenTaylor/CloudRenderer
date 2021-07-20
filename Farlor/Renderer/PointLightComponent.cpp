#include "PointLightComponent.h"

namespace Farlor
{
    PointLightComponent::PointLightComponent(uint32_t objectId)
        : m_id{objectId}
        , m_position(0.0f, 0.0f, 0.0f)
        , m_color(0.0f, 0.0f, 0.0f)
    {
    }

    PointLightComponent::~PointLightComponent()
    {
    }
}
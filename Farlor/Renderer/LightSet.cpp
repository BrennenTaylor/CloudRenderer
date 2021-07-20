#include "LightSet.h"

namespace Farlor
{
    LightSet::LightSet()
        : m_pointLightEntries{}
        , m_directionalLightEntries{}
    {
    }
        
    LightSet::~LightSet()
    {
    }

    void LightSet::AddPointLightComponent(const PointLightComponent& pointLightComponent)
    {
        PointLightEntry pointLightEntry;
        pointLightEntry.color = pointLightComponent.m_color;
        m_pointLightEntries.push_back(pointLightEntry);
    }
        
    void LightSet::AddDirectionalLightComponent(const DirectionalLightComponent& directionalLightComponent)
    {
        DirectionalLightEntry directionalLightEntry;
        directionalLightEntry.direction = directionalLightComponent.m_direction;
        directionalLightEntry.color = directionalLightComponent.m_color;
        m_directionalLightEntries.push_back(directionalLightEntry);
    }
}
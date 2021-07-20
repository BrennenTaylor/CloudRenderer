#pragma once

#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"

#include <FMath/FMath.h>

#include <vector>

namespace Farlor
{
    class LightSet
    {
    public:
        struct PointLightEntry
        {
            Vector3 color;
        };

        struct DirectionalLightEntry
        {
            Vector3 direction;
            Vector3 color;
        };

    public:
        LightSet();
        ~LightSet();

        void AddPointLightComponent(const PointLightComponent& pointLightComponent);
        void AddDirectionalLightComponent(const DirectionalLightComponent& directionalLightComponent);

        const std::vector<PointLightEntry>& GetPointLightEntries() const
        {
            return m_pointLightEntries;
        }

        const std::vector<DirectionalLightEntry>& GetDirectionalLightEntries() const
        {
            return m_directionalLightEntries;
        }

    private:
        std::vector<PointLightEntry> m_pointLightEntries;
        std::vector<DirectionalLightEntry> m_directionalLightEntries;
    };
}
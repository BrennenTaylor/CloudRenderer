#pragma once

#include <FMath/FMath.h>

namespace Farlor
{
    class Renderer;

    class DirectionalLightComponent
    {
        friend Renderer;
    public:
        explicit DirectionalLightComponent(uint32_t id);
        ~DirectionalLightComponent();

    public:
        Vector3 m_direction;
        Vector3 m_color;

    private:
        uint32_t m_id;
    };
}
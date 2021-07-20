#pragma once

#include <FMath/FMath.h>

namespace Farlor
{
    class Renderer;

    // Outside class can set the public parameters and then calls update to
    // handle the rest
    class PointLightComponent
    {
        friend Renderer;
    public:
        explicit PointLightComponent(uint32_t id);
        ~PointLightComponent();

    public:
        Vector3 m_position;
        Vector3 m_color;

    private:
        uint32_t m_id;
    };
}
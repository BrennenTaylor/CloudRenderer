#pragma once

#include "Geometry.h"

#include <cstdint>

namespace Farlor
{
    class RenderComponent
    {
    public:
        RenderComponent();
        explicit RenderComponent(Geometry* pGeometry, const Matrix4x4& transform);
        ~RenderComponent();

        Geometry::GeometryHandle GetAgnosticHandle() const
        {
            return m_agnosticHandle;
        }

        void SetIsVisible(bool visible);
        bool IsVisible() const;

    private:
        Geometry* m_pGeometry;
        Matrix4x4 m_worldTransform;
        bool m_isVisible;
        Geometry::GeometryHandle m_agnosticHandle;
    };
}
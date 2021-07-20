#pragma once

#include "Geometry.h"
#include "RenderComponent.h"
#include "Transform.h"

#include <vector>

namespace Farlor
{
    class Renderer;

    class VisibleSet
    {
    public:
        struct VisibleEntry
        {
            Geometry::GeometryHandle m_agnosticHandle;
            Matrix4x4 m_transformMatrix;
        };

    public:
        VisibleSet();
        ~VisibleSet();

        const std::vector<VisibleEntry>& GetVisibleEntries() const;

        void AddRenderComponent(const RenderComponent& renderComponent, const Transform& transform);

    private:
        std::vector<VisibleEntry> m_entries;
    };
}
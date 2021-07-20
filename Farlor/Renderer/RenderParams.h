#pragma once

#include "IGraphicsBackend.h"

#include "Geometry.h"

namespace Farlor
{
    // This will cross the dll boundary, so it is important to keep it as POD as possible. No fancy storage containers for us... :(
    struct RenderParams
    {
        Geometry::GeometryHandle* pGeometryHandles;
        uint32_t m_geometryCount;
    };
}
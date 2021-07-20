#pragma once

#include "Vertex.h"
#include <cstdint>

namespace Farlor
{
    class Geometry
    {
    public:
        using GeometryHandle = uint32_t;
        using GeometryVertex = VertexPositionUVNormalTan;

        const static GeometryHandle s_InvalidHandle = -1;

    public:
        Geometry();
        virtual ~Geometry();

        virtual const Geometry::GeometryVertex *GetVertexData() const = 0;
        virtual uint32_t GetNumVertices() const = 0;

        virtual const uint32_t* GetIndexData() const = 0;
        virtual uint32_t GetNumIndices() const = 0;
    };
}
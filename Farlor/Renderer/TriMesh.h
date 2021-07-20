#pragma once

#include "Geometry.h"

namespace Farlor
{
    class TriMesh : public Geometry
    {
    public:
        TriMesh();
        ~TriMesh();

        virtual const Geometry::GeometryVertex *GetVertexData() const override;
        virtual uint32_t GetNumVertices() const override;

        virtual const uint32_t *GetIndexData() const override;
        virtual uint32_t GetNumIndices() const override;

        void AddTriangle(const Geometry::GeometryVertex &v0, const Geometry::GeometryVertex &v2, const Geometry::GeometryVertex &v3);

      private:
        std::vector<Geometry::GeometryVertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
}
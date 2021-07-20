#pragma once

#include "Geometry.h"

namespace Farlor
{
    class ObjMesh : public Geometry
    {
    public:
        ObjMesh();
        ~ObjMesh();

        virtual const Geometry::GeometryVertex *GetVertexData() const override;
        virtual uint32_t GetNumVertices() const override;

        virtual const uint32_t *GetIndexData() const override;
        virtual uint32_t GetNumIndices() const override;

        bool Load(std::string resourceRootDir, std::string filename);

    private:
        std::vector<Geometry::GeometryVertex> m_vertices;
        std::vector<uint32_t> m_indices;
    };
}
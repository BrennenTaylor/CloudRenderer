#include "TriMesh.h"

namespace Farlor
{
    TriMesh::TriMesh()
    {
    }

    TriMesh::~TriMesh()
    {
    }

    const Geometry::GeometryVertex *TriMesh::GetVertexData() const
    {
        return m_vertices.data();
    }
    uint32_t TriMesh::GetNumVertices() const
    {
        return static_cast<uint32_t>(m_vertices.size());
    }

    const uint32_t *TriMesh::GetIndexData() const
    {
        return m_indices.data();
    }
    uint32_t TriMesh::GetNumIndices() const
    {
        return static_cast<uint32_t>(m_indices.size());
    }

    void TriMesh::AddTriangle(const Geometry::GeometryVertex& v0, const Geometry::GeometryVertex& v1, const Geometry::GeometryVertex& v2)
    {
        m_vertices.push_back(v0);
        m_vertices.push_back(v1);
        m_vertices.push_back(v2);
        m_indices.push_back(static_cast<uint32_t>(m_indices.size()));
        m_indices.push_back(static_cast<uint32_t>(m_indices.size()));
        m_indices.push_back(static_cast<uint32_t>(m_indices.size()));
    }
}
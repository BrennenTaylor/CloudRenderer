#include "ObjMesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Farlor
{
    ObjMesh::ObjMesh()
    {
    }

    ObjMesh::~ObjMesh()
    {
    }

    const Geometry::GeometryVertex *ObjMesh::GetVertexData() const
    {
        return m_vertices.data();
    }

    uint32_t ObjMesh::GetNumVertices() const
    {
        return static_cast<uint32_t>(m_vertices.size());
    }

    const uint32_t *ObjMesh::GetIndexData() const
    {
        return m_indices.data();
    }

    uint32_t ObjMesh::GetNumIndices() const
    {
        return static_cast<uint32_t>(m_indices.size());
    }

    bool ObjMesh::Load(std::string resourceRootDir, std::string filename)
    {
        std::vector<Geometry::GeometryVertex> vertices;
        std::vector<uint32_t> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string wrn;
        std::string err;

        std::string fullModelPath = resourceRootDir + filename;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &wrn, &err, fullModelPath.c_str()))
        {
            std::cout << "Failed to load model:" << fullModelPath << std::endl;
            return { nullptr };
        }

        std::unordered_map<VertexPositionUVNormalTan, int> uniqueVertices = {};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                VertexPositionUVNormalTan vertex = {};

                vertex.m_position =
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (attrib.texcoords.size() > 0)
                {
                    vertex.m_uv =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (attrib.normals.size() > 0)
                {
                    vertex.m_normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = (int)vertices.size();
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        // Generate normals if the values are not loaded.
        // TODO: Check that the values are not loaded.

        for (uint32_t i = 0; i < (uint32_t)indices.size(); i += 3)
        {
            uint32_t index0 = indices[i];
            uint32_t index1 = indices[i + 1];
            uint32_t index2 = indices[i + 2];

            Vector3 pos0 = vertices[index0].m_position;
            Vector3 pos1 = vertices[index1].m_position;
            Vector3 pos2 = vertices[index2].m_position;

            Vector3 a = pos1 - pos0;
            Vector3 b = pos2 - pos0;
            Vector3 faceNormal = a.Cross(b);

            vertices[index0].m_normal += faceNormal;
            vertices[index1].m_normal += faceNormal;
            vertices[index2].m_normal += faceNormal;
        }

        for (uint32_t i = 0; i < (uint32_t)vertices.size(); ++i)
        {
            vertices[i].m_normal = vertices[i].m_normal.Normalized();
        }


        // We need to generate tangent
        // Tangent
        for (uint32_t i = 0; i < (uint32_t)indices.size(); i += 3)
        {
            uint32_t index0 = indices[i];
            uint32_t index1 = indices[i + 1];
            uint32_t index2 = indices[i + 2];

            Vector3 pos0 = vertices[index0].m_position;
            Vector3 pos1 = vertices[index1].m_position;
            Vector3 pos2 = vertices[index2].m_position;

            Vector2 uv0 = vertices[index0].m_uv;
            Vector2 uv1 = vertices[index1].m_uv;
            Vector2 uv2 = vertices[index2].m_uv;

            Vector3 aPos = pos1 - pos0;
            Vector3 bPos = pos2 - pos0;

            Vector2 aUV = uv1 - uv0;
            Vector2 bUV = uv2 - uv0;

            Vector3 tangent = aUV.y * bPos - bUV.y * aPos;
            Vector3 bitangent = aUV.x * bPos - bUV.x * aPos;

            vertices[index0].m_tangent += tangent;
            vertices[index1].m_tangent += tangent;
            vertices[index2].m_tangent += tangent;
        }

        for (uint32_t i = 0; i < (uint32_t)vertices.size(); ++i)
        {
            vertices[i].m_tangent = vertices[i].m_tangent.Normalized();
        }

        // Move our vertices over to where we want them
        m_vertices = std::move(vertices);
        m_indices = std::move(indices);
    }
}
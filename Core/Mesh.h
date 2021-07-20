#pragma once

#include <FMath\Vector2.h>
#include <FMath\Vector3.h>
#include <FMath\Vector4.h>

#include <cstdint>
#include <vector>

namespace Farlor
{
    class Mesh
    {
    public:
        struct Vertex
        {
        public:
            Vector3 m_position;
            Vector3 m_normal;
            Vector2 m_uv;
            Vector3 m_tangent;
            Vector3 m_bitangent;
        };

        enum class VertexFlags : uint32_t
        {
            Position = 1 << 0,
            Normal = 1 << 1,
            UV = 1 << 2,
            Tangent = 1 << 3,
            Bitangent = 1 << 4
        };

        explicit Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indicies, uint32_t meshFlags);

    public:
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indicies;
        uint32_t m_meshFlags;
    };
}
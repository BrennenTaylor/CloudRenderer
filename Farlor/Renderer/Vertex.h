#pragma once

#include <FMath/FMath.h>

#include <unordered_map>

namespace Farlor
{
    struct VertexPosition
    {
        Vector3 m_position;

        VertexPosition();
        VertexPosition(float x, float y, float z);
    };

    struct VertexFont
    {
        Vector4 m_position;
        Vector4 m_uv;
        Vector4 m_color;

        VertexFont();
        VertexFont(float screenX, float screenY, float screenWidth, float screenHeight,
            float u, float v, float texCoordWidth, float texCoordHeight,
            float r, float g, float b, float a);
    };

    struct VertexPositionUVColor
    {
        Vector3 m_position;
        Vector2 m_uv;
        Vector3 m_color;

        VertexPositionUVColor();
        VertexPositionUVColor(float x, float y, float z, float u, float v, float r, float g, float b);

        bool operator==(const VertexPositionUVColor& other) const;
    };

    struct VertexPositionUVNormal
    {
        Vector3 m_position;
        Vector2 m_uv;
        Vector3 m_normal;

        VertexPositionUVNormal();
        VertexPositionUVNormal(float x, float y, float z, float u, float v, float nx, float ny, float nz);

        bool operator==(const VertexPositionUVNormal& other) const;
    };

    struct VertexPositionUVNormalTan
    {
        Vector3 m_position;
        Vector2 m_uv;
        Vector3 m_normal;
        Vector3 m_tangent;

        VertexPositionUVNormalTan();
        VertexPositionUVNormalTan(float x, float y, float z, float u, float v, float nx, float ny, float nz, float tx, float ty, float tz);

        bool operator==(const VertexPositionUVNormalTan& other) const;
    };
}

namespace std
{
    template<> struct hash<Farlor::VertexPositionUVNormal>
    {
        size_t operator()(Farlor::VertexPositionUVNormal const& vertex) const
        {
            return
                ((std::hash<Farlor::Vector3>()((Farlor::Vector3)vertex.m_position))
                    ^ ((std::hash<Farlor::Vector3>()((Farlor::Vector3)vertex.m_normal) << 1)) >> 1)
                ^ ((std::hash<Farlor::Vector2>()((Farlor::Vector2)vertex.m_uv) << 1));
        }
    };

    template<> struct hash<Farlor::VertexPositionUVNormalTan>
    {
        size_t operator()(Farlor::VertexPositionUVNormalTan const& vertex) const
        {
            return ((std::hash<Farlor::Vector3>()((Farlor::Vector3)vertex.m_position) ^ (std::hash<Farlor::Vector3>()((Farlor::Vector3)vertex.m_normal) << 1)) >> 1) ^ (std::hash<Farlor::Vector2>()((Farlor::Vector2)vertex.m_uv) << 1);
        }
    };
}
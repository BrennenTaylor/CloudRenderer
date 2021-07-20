#include "Vertex.h"

namespace Farlor
{
    //// Vertex Position
    //D3D11_INPUT_ELEMENT_DESC VertexPosition::s_layout[] =
    //{
    //    {
    //        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //};

    //u32 VertexPosition::s_numElements = 1;

    VertexPosition::VertexPosition()
        : m_position{ 0.0f, 0.0f, 0.0f }
    {
    }

    VertexPosition::VertexPosition(float x, float y, float z)
        : m_position{ x, y, z }
    {
    }

    // Vertex Font
    // This is a special packed vertex
    //D3D11_INPUT_ELEMENT_DESC VertexFont::s_layout[] =
    //{
    //    {
    //        "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //};

    //u32 VertexFont::s_numElements = 3;

    VertexFont::VertexFont()
        : m_position{ 0.0f, 0.0f, 0.0f, 0.0f }
        , m_uv{ 0.0f, 0.0f, 0.0f, 0.0f }
        , m_color{ 0.0f, 0.0f, 0.0f, 0.0f }
    {
    }

    VertexFont::VertexFont(float screenX, float screenY, float screenWidth, float screenHeight,
        float u, float v, float texCoordWidth, float texCoordHeight,
        float r, float g, float b, float a)
        : m_position{ screenX, screenY, screenWidth, screenHeight }
        , m_uv{ u, v, texCoordWidth, texCoordHeight }
        , m_color{ r, g, b, a }
    {
    }


    // Vertex Position UV Color
    //D3D11_INPUT_ELEMENT_DESC VertexPositionUVColor::s_layout[] =
    //{
    //    {
    //        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //};

    //u32 VertexPositionUVColor::s_numElements = 3;

    VertexPositionUVColor::VertexPositionUVColor()
        : m_position{ 0.0f, 0.0f, 0.0f }
        , m_uv{ 0.0f, 0.0f }
        , m_color{ 0.0f, 0.0f, 0.0f }
    {
    }

    VertexPositionUVColor::VertexPositionUVColor(float x, float y, float z, float u, float v, float r, float g, float b)
        : m_position{ x, y, z }
        , m_uv{ u, v }
        , m_color{ r, g, b }
    {
    }

    bool VertexPositionUVColor::operator==(const VertexPositionUVColor& other) const {
        return (Vector3)m_position == (Vector3)other.m_position && (Vector3)m_color == (Vector3)other.m_color && (Vector2)m_uv == (Vector2)other.m_uv;
    }

    // Vertex Position UV Normal
    //D3D11_INPUT_ELEMENT_DESC VertexPositionUVNormal::s_layout[] =
    //{
    //    {
    //        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //};

    //u32 VertexPositionUVNormal::s_numElements = 3;

    VertexPositionUVNormal::VertexPositionUVNormal()
        : m_position{ 0.0f, 0.0f, 0.0f }
        , m_uv{ 0.0f, 0.0f }
        , m_normal{ 0.0f, 0.0f, 0.0f }
    {
    }

    VertexPositionUVNormal::VertexPositionUVNormal(float x, float y, float z, float u, float v, float nx, float ny, float nz)
        : m_position{ x, y, z }
        , m_uv{ u, v }
        , m_normal{ nx, ny, nz }
    {
    }

    bool VertexPositionUVNormal::operator==(const VertexPositionUVNormal& other) const {
        return (Vector3)m_position == (Vector3)other.m_position && (Vector3)m_normal == (Vector3)other.m_normal && (Vector2)m_uv == (Vector2)other.m_uv;
    }

    // Vertex Position UV Normal Tangent Bitangent
    //D3D11_INPUT_ELEMENT_DESC VertexPositionUVNormalTan::s_layout[] =
    //{
    //    {
    //        "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    },
    //    {
    //        "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0
    //    }
    //};

    //u32 VertexPositionUVNormalTan::s_numElements = 3;

    VertexPositionUVNormalTan::VertexPositionUVNormalTan()
        : m_position{ 0.0f, 0.0f, 0.0f }
        , m_uv{ 0.0f, 0.0f }
        , m_normal{ 0.0f, 0.0f, 0.0f }
        , m_tangent{ 0.0f, 0.0f, 0.0f }
    {
    }

    VertexPositionUVNormalTan::VertexPositionUVNormalTan(float x, float y, float z, float u, float v, float nx, float ny, float nz, float tx, float ty, float tz)
        : m_position{ x, y, z }
        , m_uv{ u, v }
        , m_normal{ nx, ny, nz }
        , m_tangent{ tx, ty, tz }
    {
    }

    bool VertexPositionUVNormalTan::operator==(const VertexPositionUVNormalTan& other) const {
        return (Vector3)m_position == (Vector3)other.m_position && (Vector3)m_normal == (Vector3)other.m_normal && (Vector2)m_uv == (Vector2)other.m_uv &&
            (Vector3)m_tangent == (Vector3)other.m_tangent;
    }
}
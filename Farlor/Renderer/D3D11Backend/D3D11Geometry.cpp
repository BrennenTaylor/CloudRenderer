#include "D3D11Geometry.h"

namespace Farlor
{
    D3D11Geometry::D3D11Geometry(Microsoft::WRL::ComPtr<ID3D11Buffer> cpVertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> cpIndexBuffer,
        uint32_t numVertices, uint32_t numIndices, uint32_t vertexStride, uint32_t vertexOffset)
        : m_cpVertexBuffer{ cpVertexBuffer }
        , m_cpIndexBuffer{ cpIndexBuffer }
        , m_numVertices{numVertices}
        , m_numIndices{numIndices}
        , m_vertexStride{vertexStride}
        , m_vertexOffset{vertexOffset}
    {
    }

    D3D11Geometry::~D3D11Geometry()
    {
    }

    ID3D11Buffer* D3D11Geometry::GetRawVertexBuffer() const
    {
        return m_cpVertexBuffer.Get();
    }
        
    ID3D11Buffer* D3D11Geometry::GetRawIndexBuffer() const
    {
        return m_cpIndexBuffer.Get();
    }
}
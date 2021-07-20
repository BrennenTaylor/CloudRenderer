#pragma once

#include <Geometry.h>

#include <d3d11.h>
#include <wrl.h>

namespace Farlor
{
    // Specialization of geometry class for D3D11
    // Basically holds d3d specific buffer information
    class D3D11Geometry
    {
    public:
        D3D11Geometry(Microsoft::WRL::ComPtr<ID3D11Buffer> cpVertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> cpIndexBuffer,
            uint32_t numVertices, uint32_t numIndices, uint32_t vertexStride, uint32_t vertexOffset);
        ~D3D11Geometry();

        ID3D11Buffer* GetRawVertexBuffer() const;
        ID3D11Buffer* GetRawIndexBuffer() const;

        uint32_t GetNumVertices() const
        {
            return m_numVertices;
        }
        uint32_t GetNumIndices() const
        {
            return m_numIndices;
        }
        uint32_t GetVertexStride() const
        {
            return m_vertexStride;
        }
        uint32_t GetVertexOffset() const
        {
            return m_vertexOffset;
        }

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpVertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpIndexBuffer;
        uint32_t m_numVertices;
        uint32_t m_numIndices;
        uint32_t m_vertexStride;
        uint32_t m_vertexOffset;
    };
}
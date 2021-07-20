#include "D3D11ResourceFactory.h"

namespace Farlor
{
    D3D11ResourceFactory::D3D11ResourceFactory(Microsoft::WRL::ComPtr<ID3D11Device> cpDevice)
        : m_cpDevice{cpDevice}
    {
    }

    D3D11ResourceFactory::~D3D11ResourceFactory()
    {
    }

    std::unique_ptr<D3D11Geometry> D3D11ResourceFactory::CreateBackendGeometry(Geometry* pFrontendGeometry)
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;

        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
        // Currently hardcoded for buffer type, should expose as a paramater
        vertexBufferDesc.ByteWidth = sizeof(VertexPositionUVNormalTan) * pFrontendGeometry->GetNumVertices();
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = sizeof(VertexPositionUVNormalTan);

        D3D11_SUBRESOURCE_DATA vertexData;
        ZeroMemory(&vertexData, sizeof(vertexData));

        vertexData.pSysMem = pFrontendGeometry->GetVertexData();
        vertexData.SysMemPitch = 0;
        vertexData.SysMemSlicePitch = 0;

        HRESULT result = m_cpDevice->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf());
        if (FAILED(result))
        {
            return nullptr;
        }

        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
        // Currently hardcoded for buffer type, should expose as a paramater
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * pFrontendGeometry->GetNumIndices();
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;
        indexBufferDesc.StructureByteStride = sizeof(uint32_t);

        D3D11_SUBRESOURCE_DATA indexData;
        ZeroMemory(&indexData, sizeof(indexData));

        indexData.pSysMem = pFrontendGeometry->GetIndexData();
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        result = m_cpDevice->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf());
        if (FAILED(result))
        {
            return nullptr;
        }

        const uint32_t stride = sizeof(VertexPositionUVNormalTan);
        const uint32_t offset = 0;
        return std::make_unique<D3D11Geometry>(vertexBuffer, indexBuffer, pFrontendGeometry->GetNumVertices(), pFrontendGeometry->GetNumIndices(), stride, offset);
    }
}
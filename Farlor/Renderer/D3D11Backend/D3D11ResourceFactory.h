#pragma once

#include "D3D11Geometry.h"

#include <d3d11.h>
#include <wrl.h>

#include <memory>

namespace Farlor
{
    class D3D11ResourceFactory
    {
    public:
        explicit D3D11ResourceFactory(Microsoft::WRL::ComPtr<ID3D11Device> cpDevice);
        ~D3D11ResourceFactory();

        std::unique_ptr<D3D11Geometry> CreateBackendGeometry(Geometry* pFrontendGeometry);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_cpDevice;
    };
}
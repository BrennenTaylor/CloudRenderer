#pragma once

#include "../IGraphicsBackend.h"

#include "D3D11Geometry.h"
#include "D3D11ResourceFactory.h"

#include <Geometry.h>

#include <d3d11.h>
#include <Windows.h>
#include <wrl.h>

#include <map>

namespace Farlor
{
    class D3D11Backend : public IGraphicsBackend
    {
    public:
        D3D11Backend(const Renderer& renderer);
        virtual ~D3D11Backend();

        virtual void Initialize(IWindow *pWindow, std::string& resourceDir) override;
        virtual void Shutdown() override;

        virtual void Render(const VisibleSet& visibleSet, const LightSet& lightSet, const CameraEntry& currentCameraEntry, float deltaTime, float totalTime) override;

        virtual Geometry::GeometryHandle RegisterGeometry(Geometry* pGeometry) override;

    protected:
        bool m_isInitialized;

        HWND m_windowHandle;
        uint32_t m_clientWidth;
        uint32_t m_clientHeight;
        std::string m_resourceDir;

        // D3D11 Resources
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_cpSwapChain;
        Microsoft::WRL::ComPtr<ID3D11Device> m_cpDevice;
        Microsoft::WRL::ComPtr<ID3D11Debug> m_cpDebugDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_cpDeviceContext;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_cpBackBufferRTV;

        std::unique_ptr<D3D11ResourceFactory> m_upResourceFactory;
        std::map < Geometry::GeometryHandle, std::unique_ptr<D3D11Geometry>> m_idToBackendGeometry;
        Geometry::GeometryHandle m_currentGeometryId;
        IGraphicsBackend::BackendGraphicsHandle m_previousBackendHandle;

        // The base backend handles device creation and sets up a common set of resources that most applications use.
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_cpDefaultDSV;

        // Blending Modes
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_cpAdditiveBlending;
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_cpDisableBlending;

        // Samplers
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cpCommonSampler;
    };
}
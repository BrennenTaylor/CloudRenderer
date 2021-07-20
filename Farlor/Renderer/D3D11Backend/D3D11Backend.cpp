#include "D3D11Backend.h"
#include "CBStructures.h"

#include <Renderer.h>

#include <StringUtil.h>

#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <D3D11Utils\DebugUtils.h>

namespace Farlor
{
    constexpr bool DebugD3D11Mode = true;

    D3D11Backend::D3D11Backend(const Renderer& renderer)
        : IGraphicsBackend(renderer)
        , m_isInitialized{false}
        , m_windowHandle{ 0 }
        , m_clientWidth{ 0 }
        , m_clientHeight{ 0 }
        , m_cpSwapChain{ nullptr }
        , m_cpDevice{ nullptr }
        , m_cpDeviceContext{ nullptr }
        , m_cpBackBufferRTV{ nullptr }
        , m_upResourceFactory{ nullptr }
        , m_idToBackendGeometry{}
        , m_currentGeometryId{0}
        , m_previousBackendHandle{IGraphicsBackend::InvalidBackendGraphicsHandle + 1}
        , m_cpDefaultDSV{nullptr}
        , m_cpAdditiveBlending{nullptr}
        , m_cpDisableBlending{nullptr}
        , m_cpCommonSampler{nullptr}
    {
    }

    D3D11Backend::~D3D11Backend()
    {
    }

    void D3D11Backend::Initialize(IWindow *pWindow, std::string& resourceDir)
    {
        m_windowHandle = *reinterpret_cast<const HWND*>(pWindow->GetNativeHandle());
        m_clientWidth = pWindow->GetClientWidth();
        m_clientHeight = pWindow->GetClientHeight();
        m_resourceDir = resourceDir;

        HRESULT result;

        // Create the device, device context, and back buffer RTV
        {
            DXGI_MODE_DESC backBufferDesc{};
            ZeroMemory(&backBufferDesc, sizeof(backBufferDesc));
            backBufferDesc.Width = m_clientWidth;
            backBufferDesc.Height = m_clientHeight;
            backBufferDesc.RefreshRate.Numerator = 60;
            backBufferDesc.RefreshRate.Denominator = 1;
            backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            backBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            backBufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

            DXGI_SWAP_CHAIN_DESC swapChainDesc{};
            ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
            swapChainDesc.BufferDesc = backBufferDesc;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 1;
            swapChainDesc.OutputWindow = m_windowHandle;
            swapChainDesc.Windowed = true;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            uint32_t flags = 0;

            if constexpr (DebugD3D11Mode)
            {
                flags |= D3D11_CREATE_DEVICE_DEBUG;
            }

            result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0,
                D3D11_SDK_VERSION, &swapChainDesc, m_cpSwapChain.GetAddressOf(), m_cpDevice.GetAddressOf(), 0, m_cpDeviceContext.GetAddressOf());

            if (FAILED(result))
            {
                std::cout << "Create Device and Swap Chain call failed" << std::endl;
                return;
            }

            if (!m_cpDevice)
            {
                // TODO: FATAL ERROR
                std::cout << "Failed to aquire d3d11 device" << std::endl;
            }

            if (!m_cpDeviceContext)
            {
                // TODO: FATAL ERROR
                std::cout << "Failed to aquire d3d11 device context" << std::endl;
            }

            if constexpr (DebugD3D11Mode)
            {
                result = m_cpDevice->QueryInterface(IID_PPV_ARGS(m_cpDebugDevice.GetAddressOf()));
                // We want to assume that if we have the debug interface turned on, we can have a debug device
                assert(m_cpDevice);
            }
        }

        m_upResourceFactory = std::make_unique<D3D11ResourceFactory>(m_cpDevice);

        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> cpBackBuffer = nullptr;
            result = m_cpSwapChain->GetBuffer(0, IID_PPV_ARGS(cpBackBuffer.GetAddressOf()));
            if (FAILED(result))
            {
                return;
            }
            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(cpBackBuffer.Get(), std::string("D3D11Backend_BackbufferTexture"));
            }

            result = m_cpDevice->CreateRenderTargetView(cpBackBuffer.Get(), 0, m_cpBackBufferRTV.GetAddressOf());
            if (FAILED(result))
            {
                return;
            }
            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpBackBufferRTV.Get(), std::string("D3D11Backend_BackbufferRTV"));
            }
        }

        // Create default depth stencil views
        {
            D3D11_TEXTURE2D_DESC mainDeferredDSTextureDesc;
            ZeroMemory(&mainDeferredDSTextureDesc, sizeof(mainDeferredDSTextureDesc));
            mainDeferredDSTextureDesc.Width = m_clientWidth;
            mainDeferredDSTextureDesc.Height = m_clientHeight;
            mainDeferredDSTextureDesc.MipLevels = 1;
            mainDeferredDSTextureDesc.ArraySize = 1;
            mainDeferredDSTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            mainDeferredDSTextureDesc.SampleDesc.Count = 1;
            mainDeferredDSTextureDesc.SampleDesc.Quality = 0;
            mainDeferredDSTextureDesc.Usage = D3D11_USAGE_DEFAULT;
            mainDeferredDSTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            mainDeferredDSTextureDesc.CPUAccessFlags = 0;
            mainDeferredDSTextureDesc.MiscFlags = 0;

            Microsoft::WRL::ComPtr<ID3D11Texture2D> cpDeferredMainDST = nullptr;
            result = m_cpDevice->CreateTexture2D(&mainDeferredDSTextureDesc, 0, cpDeferredMainDST.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }
            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(cpDeferredMainDST.Get(), std::string("D3D11Backend_DefaultDSTexture"));
            }

            D3D11_DEPTH_STENCIL_VIEW_DESC mainDeferredDSVDesc;
            ZeroMemory(&mainDeferredDSVDesc, sizeof(mainDeferredDSVDesc));
            mainDeferredDSVDesc.Format = mainDeferredDSTextureDesc.Format;
            mainDeferredDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            mainDeferredDSVDesc.Texture2D.MipSlice = 0;

            result = m_cpDevice->CreateDepthStencilView(cpDeferredMainDST.Get(), &mainDeferredDSVDesc, m_cpDefaultDSV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpDefaultDSV.Get(), std::string("D3D11Backend_DefaultDSV"));
            }
        }

        // Create required blend states
        {
            D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
            ZeroMemory(&renderTargetBlendDesc, sizeof(renderTargetBlendDesc));
            renderTargetBlendDesc.BlendEnable = false;
            renderTargetBlendDesc.SrcBlend = D3D11_BLEND_ONE;
            renderTargetBlendDesc.DestBlend = D3D11_BLEND_ZERO;
            renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
            renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
            renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            D3D11_BLEND_DESC blendDesc;
            ZeroMemory(&blendDesc, sizeof(blendDesc));
            blendDesc.AlphaToCoverageEnable = false;
            blendDesc.IndependentBlendEnable = false;
            blendDesc.RenderTarget[0] = renderTargetBlendDesc;

            result = m_cpDevice->CreateBlendState(&blendDesc, m_cpDisableBlending.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpDisableBlending.Get(), std::string("D3D11Backend_DisabledBlendState"));
            }

            ZeroMemory(&renderTargetBlendDesc, sizeof(renderTargetBlendDesc));
            renderTargetBlendDesc.BlendEnable = true;
            renderTargetBlendDesc.SrcBlend = D3D11_BLEND_ONE;
            renderTargetBlendDesc.DestBlend = D3D11_BLEND_ONE;
            renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
            renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
            renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            ZeroMemory(&blendDesc, sizeof(blendDesc));
            blendDesc.AlphaToCoverageEnable = false;
            blendDesc.IndependentBlendEnable = false;
            blendDesc.RenderTarget[0] = renderTargetBlendDesc;

            result = m_cpDevice->CreateBlendState(&blendDesc, m_cpAdditiveBlending.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpAdditiveBlending.Get(), std::string("D3D11Backend_AdditiveBlendState"));
            }
        }

        {
            // Create and set viewport
            D3D11_VIEWPORT viewport = { 0 };
            ZeroMemory(&viewport, sizeof(viewport));
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.Width = static_cast<float>(m_clientWidth);
            viewport.Height = static_cast<float>(m_clientHeight);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            m_cpDeviceContext->RSSetViewports(1, &viewport);
        }

        {
            D3D11_SAMPLER_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD = 0;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            result = m_cpDevice->CreateSamplerState(&desc, m_cpCommonSampler.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCommonSampler.Get(), std::string("D3D11Backend_CommonSampler"));
            }
        }
    }

    void D3D11Backend::Shutdown()
    {
    }

    void D3D11Backend::Render(const VisibleSet& visibleSet, const LightSet& lightSet, const CameraEntry& currentCameraEntry, float deltaTime, float totalTime)
    {
    }

    Geometry::GeometryHandle D3D11Backend::RegisterGeometry(Geometry* pGeometry)
    {
        // Ensure we have geometry
        if (!pGeometry)
        {
            return -1;
        }

        std::unique_ptr<D3D11Geometry> upD3D11Geometry = m_upResourceFactory->CreateBackendGeometry(pGeometry);
        if (upD3D11Geometry)
        {
            m_idToBackendGeometry.insert(std::make_pair(m_currentGeometryId, std::move(upD3D11Geometry)));
            ++m_currentGeometryId;
            return (m_currentGeometryId - 1);
        }

        // Return invalid handle if failure
        return -1;
    }
}
#include "D3D11SpatiotemporalFilter.h"

#include "PathTracerCbs.h"
#include "RenderPasses.h"
#include "ShaderStructures.h"

#include <Renderer.h>

#include <DebugUtils.h>
#include <GenericCbs.h>
#include <StringUtil.h>

#include <d3dcompiler.h>

#include <DDSTextureLoader.h>

#include <cassert>
#include <string>

namespace Farlor
{
    constexpr bool DebugD3D11Mode = true;
    constexpr uint32_t ProfilerNBufferCount = 5;

    D3D11SpatiotemporalFilterBackend::D3D11SpatiotemporalFilterBackend(const Renderer& renderer)
        : D3D11Backend(renderer)
        , m_gpuProfiler{ static_cast<uint32_t>(ProfileEvent::NumEvents), ProfilerNBufferCount }
        , m_frameCount{ 0 }
        , m_iterativeFrameCount{ 0 }
        , m_previousCamera{}
        , m_cpGeometryNormalOSRTV{ nullptr }
        , m_cpGeometryNormalWSRTV{ nullptr }
        , m_cpGeometryVelocityBufferRTV{ nullptr }
        , m_cpLocalVertexSRV{ nullptr }
        , m_cpIndexSRV{ nullptr }
        , m_cpLocalToWorldSRV{ nullptr }
        , m_cpLocalToWorldInvSRV{ nullptr }
        , m_cpTransformedVertexSRV{ nullptr }
        , m_cpGeometryNormalOSSRV{ nullptr }
        , m_cpGeometryNormalWSSRV{ nullptr }
        , m_cpGeometryVelocityBufferSRV{ nullptr }
        , m_cpCloudBufferUAV{ nullptr }
        , m_cpCloudBufferSRV{ nullptr }
        , m_cpClearHDRImageBufferCS{ nullptr }
        , m_cpCloudTraceCS{ nullptr }
        , m_cpGBufferVS{ nullptr }
        , m_cpGBufferPS{ nullptr }
        , m_cpTonemappingVS{ nullptr }
        , m_cpTonemappingPS{ nullptr }
        , m_cpGeometryDeferredInputLayout{ nullptr }
        , m_cpCameraCb{ nullptr }
        , m_cpPathTracerControlCb{ nullptr }
        , m_cpNewCameraCb{ nullptr }
        , m_cpOldCameraCb{ nullptr }
        , m_cpTimeValuesCb{nullptr}
        , m_cpGeometryDeferredPerObjectCb{ nullptr }
        , m_cpGeometryDeferredPerFrameCb{ nullptr }
        , m_cpTonemapPassCb{ nullptr }
        , m_cpSamplerWrap{nullptr}
    {
    }

    D3D11SpatiotemporalFilterBackend::~D3D11SpatiotemporalFilterBackend()
    {
    }

    void D3D11SpatiotemporalFilterBackend::Initialize(IWindow *pWindow, std::string& resourceDir)
    {
        D3D11Backend::Initialize(pWindow, resourceDir);

        m_gpuProfiler.Initialize(m_cpDevice.Get(), m_cpDeviceContext.Get());

        HRESULT result;
        {
            // Create required depth stencil views
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
                D3D11DebugUtils::SetDebugName(cpDeferredMainDST.Get(), std::string("Deferred Depth Stencil Target"));
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
                D3D11DebugUtils::SetDebugName(m_cpDefaultDSV.Get(), std::string("Deferred Depth Stencil Target DSV"));
            }
        }

        // GBuffer Resources
        // Geometry NormalOS Texture and Views
        {
            D3D11_TEXTURE2D_DESC textureDesc;
            ZeroMemory(&textureDesc, sizeof(textureDesc));
            textureDesc.Width = m_clientWidth;
            textureDesc.Height = m_clientHeight;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            // Create required render targets
            Microsoft::WRL::ComPtr<ID3D11Texture2D> cpRTT = nullptr;
            result = m_cpDevice->CreateTexture2D(&textureDesc, 0, cpRTT.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
            ZeroMemory(&RTVDesc, sizeof(RTVDesc));
            RTVDesc.Format = textureDesc.Format;
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            RTVDesc.Texture2D.MipSlice = 0;
            result = m_cpDevice->CreateRenderTargetView(cpRTT.Get(), &RTVDesc, m_cpGeometryNormalOSRTV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryNormalOSRTV.Get(), std::string("Geometry NormalOS RTV"));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            ZeroMemory(&SRVDesc, sizeof(SRVDesc));
            SRVDesc.Format = textureDesc.Format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = 1;
            SRVDesc.Texture2D.MostDetailedMip = 0;
            result = m_cpDevice->CreateShaderResourceView(cpRTT.Get(), &SRVDesc, m_cpGeometryNormalOSSRV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryNormalOSSRV.Get(), std::string("Geometry NormalWS SRV"));
            }
        }

        // Geometry NormalWS Texture and Views
        {
            D3D11_TEXTURE2D_DESC textureDesc;
            ZeroMemory(&textureDesc, sizeof(textureDesc));
            textureDesc.Width = m_clientWidth;
            textureDesc.Height = m_clientHeight;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            // Create required render targets
            Microsoft::WRL::ComPtr<ID3D11Texture2D> cpRTT = nullptr;
            result = m_cpDevice->CreateTexture2D(&textureDesc, 0, cpRTT.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
            ZeroMemory(&RTVDesc, sizeof(RTVDesc));
            RTVDesc.Format = textureDesc.Format;
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            RTVDesc.Texture2D.MipSlice = 0;
            result = m_cpDevice->CreateRenderTargetView(cpRTT.Get(), &RTVDesc, m_cpGeometryNormalWSRTV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryNormalWSRTV.Get(), std::string("Geometry NormalWS RTV"));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            ZeroMemory(&SRVDesc, sizeof(SRVDesc));
            SRVDesc.Format = textureDesc.Format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = 1;
            SRVDesc.Texture2D.MostDetailedMip = 0;
            result = m_cpDevice->CreateShaderResourceView(cpRTT.Get(), &SRVDesc, m_cpGeometryNormalWSSRV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryNormalWSSRV.Get(), std::string("Geometry NormalWS SRV"));
            }
        }

        // Geometry Velocity Texture and Views
        {
            D3D11_TEXTURE2D_DESC textureDesc;
            ZeroMemory(&textureDesc, sizeof(textureDesc));
            textureDesc.Width = m_clientWidth;
            textureDesc.Height = m_clientHeight;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            // Create required render targets
            Microsoft::WRL::ComPtr<ID3D11Texture2D> cpRTT = nullptr;
            result = m_cpDevice->CreateTexture2D(&textureDesc, 0, cpRTT.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
            ZeroMemory(&RTVDesc, sizeof(RTVDesc));
            RTVDesc.Format = textureDesc.Format;
            RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            RTVDesc.Texture2D.MipSlice = 0;
            result = m_cpDevice->CreateRenderTargetView(cpRTT.Get(), &RTVDesc, m_cpGeometryVelocityBufferRTV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryVelocityBufferRTV.Get(), std::string("Geometry Velocity RTV"));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            ZeroMemory(&SRVDesc, sizeof(SRVDesc));
            SRVDesc.Format = textureDesc.Format;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = 1;
            SRVDesc.Texture2D.MostDetailedMip = 0;
            result = m_cpDevice->CreateShaderResourceView(cpRTT.Get(), &SRVDesc, m_cpGeometryVelocityBufferSRV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryVelocityBufferSRV.Get(), std::string("Geometry Velocity SRV"));
            }
        }

        // Cloud buffer and views
        {
            D3D11_BUFFER_DESC pathTracerBufferDesc;
            ZeroMemory(&pathTracerBufferDesc, sizeof(pathTracerBufferDesc));
            pathTracerBufferDesc.ByteWidth = m_clientWidth * m_clientHeight * sizeof(float) * 4;
            pathTracerBufferDesc.Usage = D3D11_USAGE_DEFAULT;
            pathTracerBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            pathTracerBufferDesc.CPUAccessFlags = 0;
            pathTracerBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            pathTracerBufferDesc.StructureByteStride = sizeof(float) * 4;

            // Create required render targets
            Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpPathTracerBuffer = nullptr;
            result = m_cpDevice->CreateBuffer(&pathTracerBufferDesc, 0, m_cpPathTracerBuffer.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpPathTracerBuffer.Get(), std::string("Path Tracer Color Buffer"));
            }

            // Create UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC pathTracingUAVDesc;
            ZeroMemory(&pathTracingUAVDesc, sizeof(pathTracingUAVDesc));
            pathTracingUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
            pathTracingUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            pathTracingUAVDesc.Buffer.FirstElement = 0;
            pathTracingUAVDesc.Buffer.NumElements = pathTracerBufferDesc.ByteWidth / pathTracerBufferDesc.StructureByteStride;
            result = m_cpDevice->CreateUnorderedAccessView(m_cpPathTracerBuffer.Get(), &pathTracingUAVDesc, m_cpCloudBufferUAV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCloudBufferUAV.Get(), std::string("Cloud Buffer UAV"));
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC pathTracingSRVDesc;
            ZeroMemory(&pathTracingSRVDesc, sizeof(pathTracingSRVDesc));
            pathTracingSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
            pathTracingSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            pathTracingSRVDesc.BufferEx.FirstElement = 0;
            pathTracingSRVDesc.BufferEx.NumElements = pathTracerBufferDesc.ByteWidth / pathTracerBufferDesc.StructureByteStride;
            result = m_cpDevice->CreateShaderResourceView(m_cpPathTracerBuffer.Get(), &pathTracingSRVDesc, m_cpCloudBufferSRV.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG ERROR
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCloudBufferSRV.Get(), std::string("Cloud Buffer SRV"));
            }
        }

        // Geometry Per Frame
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbGeometryDeferredPerObject);
            bufferDesc.StructureByteStride = sizeof(CBs::cbGeometryDeferredPerObject);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbGeometryDeferredPerObject data;
            data.WorldMatrix = Matrix4x4::s_Identity;
            data.InvWorldMatrix = Matrix4x4::s_Identity;
            data.MeshID = 0;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpGeometryDeferredPerObjectCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryDeferredPerObjectCb.Get(), std::string("Geometry Per Object CB"));
            }
        }

        // Geometry Per Frame
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbGeometryDeferredPerFrame);
            bufferDesc.StructureByteStride = sizeof(CBs::cbGeometryDeferredPerFrame);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbGeometryDeferredPerFrame data;
            data.ViewMatrix = Matrix4x4::s_Identity;
            data.ProjMatrix = Matrix4x4::s_Identity;
            data.ViewMatrix = Matrix4x4::s_Identity; //m_previousView;
            data.ProjMatrix = Matrix4x4::s_Identity; //m_previousProj;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpGeometryDeferredPerFrameCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGeometryDeferredPerFrameCb.Get(), std::string("Geometry Per Frame CB"));
            }
        }

        // Create all the needed shaders

        // Clear HDR Image Buffer
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/ClearHDRStructuredBuffer.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            D3D_SHADER_MACRO macros[] =
            {
                "USE_DEFAULT_THREAD_COUNTS", "true",
                0, 0
            };

            result = D3DCompileFromFile(filename.c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpClearHDRImageBufferCS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpClearHDRImageBufferCS.Get(), std::string("Clear HDR Image Buffer CS"));
            }
        }

        // Path Tracer CS
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/CloudTrace.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            D3D_SHADER_MACRO macros[] =
            {
                "USE_DEFAULT_THREAD_COUNTS", "true",
                0, 0
            };

            result = D3DCompileFromFile(filename.c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpCloudTraceCS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCloudTraceCS.Get(), std::string("Cloud Trace CS"));
            }
        }

        // G-Buffer VS
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/STD_GeometryDeferred.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            result = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpGBufferVS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGBufferVS.Get(), std::string("GBufferPass VS"));
            }

            //Vector3 m_position;
            //Vector2 m_uv;
            //Vector3 m_normal;
            //Vector3 m_tangent;
            const uint32_t numElements = 4;
            D3D11_INPUT_ELEMENT_DESC elementDesc[numElements] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            result = m_cpDevice->CreateInputLayout(elementDesc, numElements, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), m_cpGeometryDeferredInputLayout.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }
        }

        // G-Buffer PS
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/STD_GeometryDeferred.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            result = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpGBufferPS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpGBufferPS.Get(), std::string("GBufferPass PS"));
            }
        }

        // Tonemapping VS
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/TonemappingPass.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            result = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpTonemappingVS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpTonemappingVS.Get(), std::string("Tonemapper VS"));
            }
        }

        // Tonemapping PS
        {
            std::wstring filename = Utility::StringUtil::StringToWideString(m_resourceDir) + std::wstring(L"/shaders/TonemappingPass.hlsl");
            Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
            result = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
            if (FAILED(result))
            {
                std::string error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
                std::cout << "Failed to compile shader: " << error << std::endl;
                return;
            }

            result = m_cpDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, m_cpTonemappingPS.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: Log error
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpTonemappingPS.Get(), std::string("Tonemapper PS"));
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
                D3D11DebugUtils::SetDebugName(m_cpDisableBlending.Get(), std::string("Disable Blending Blend State"));
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
                D3D11DebugUtils::SetDebugName(m_cpAdditiveBlending.Get(), std::string("Additive Blending Blend State"));
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

        // Camera Control CB
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbCamera);
            bufferDesc.StructureByteStride = sizeof(CBs::cbCamera);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbCamera data;
            data.position = Vector3(0.0f, 0.0f, 0.0f);

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpCameraCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCameraCb.Get(), std::string("Rasterization Camera cb"));
            }
        }

        // Path Tracer Control CB
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbPathTracerControl);
            bufferDesc.StructureByteStride = sizeof(CBs::cbPathTracerControl);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbPathTracerControl data;
            
            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpPathTracerControlCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpPathTracerControlCb.Get(), std::string("Path Tracer Control cb"));
            }
        }

        // New Camera CB
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbPathTracerCamera);
            bufferDesc.StructureByteStride = sizeof(CBs::cbPathTracerCamera);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbPathTracerCamera data;
            data.cameraPos = Farlor::Vector3(0.0f, 0.0f, 0.0f);
            data.cameraTarget = Farlor::Vector3(0.0f, 0.0f, 1.0f);
            data.worldUp = Farlor::Vector3(0.0f, 1.0f, 0.0f);
            data.screenWidth = m_clientWidth;
            data.screenHeight = m_clientHeight;
            data.FOV_Horizontal = 45.0f;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpNewCameraCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpNewCameraCb.Get(), std::string("New Camera cb"));
            }
        }

        // Old Camera CB
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbPathTracerCamera);
            bufferDesc.StructureByteStride = sizeof(CBs::cbPathTracerCamera);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbPathTracerCamera data;
            data.cameraPos = Farlor::Vector3(0.0f, 0.0f, 0.0f);
            data.cameraTarget = Farlor::Vector3(0.0f, 0.0f, 1.0f);
            data.worldUp = Farlor::Vector3(0.0f, 1.0f, 0.0f);
            data.screenWidth = m_clientWidth;
            data.screenHeight = m_clientHeight;
            data.FOV_Horizontal = 45.0f;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpOldCameraCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpOldCameraCb.Get(), std::string("Old Camera cb"));
            }
        }

        // Time Vals CB
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbTimeValues);
            bufferDesc.StructureByteStride = sizeof(CBs::cbTimeValues);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbTimeValues data;
            data.DeltaTime = 0.0f;
            data.TotalTime = 0.0f;
            data._pad[0] = 0.0f;
            data._pad[1] = 0.0f;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpTimeValuesCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr (DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpOldCameraCb.Get(), std::string("Time Vals cb"));
            }
        }

        // Tonemapping Pass Parameters
        {
            D3D11_BUFFER_DESC bufferDesc;
            ZeroMemory(&bufferDesc, sizeof(bufferDesc));
            bufferDesc.ByteWidth = sizeof(CBs::cbTonemapPassSettings);
            bufferDesc.StructureByteStride = sizeof(CBs::cbTonemapPassSettings);
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

            CBs::cbTonemapPassSettings data;
            data.ScreenWidth = m_clientWidth;
            data.ScreenHeight = m_clientHeight;
            data.PTC_Pad0 = 0;
            data.PTC_Pad1 = 0;

            D3D11_SUBRESOURCE_DATA initialData;
            initialData.pSysMem = &data;

            result = m_cpDevice->CreateBuffer(&bufferDesc, &initialData, m_cpTonemapPassCb.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpTonemapPassCb.Get(), std::string("Tonemap Pass cb"));
            }
        }

        {
            D3D11_SAMPLER_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD = -FLT_MAX;
            desc.MaxLOD = D3D11_FLOAT32_MAX;

            result = m_cpDevice->CreateSamplerState(&desc, m_cpCommonSampler.GetAddressOf());
            if (FAILED(result))
            {
                // TODO: LOG FAILURE
                return;
            }

            if constexpr(DebugD3D11Mode)
            {
                D3D11DebugUtils::SetDebugName(m_cpCommonSampler.Get(), std::string("Common Sampler"));
            }
        }

        // Load up the DDS textures
        // Load low frequency data
        {
            result = DirectX::CreateDDSTextureFromFile(m_cpDevice.Get(),
                L"./assets/textures/LowFrequency/LowFrequency.dds",
                m_cpLowFrequencyResource.GetAddressOf(), m_cpLowFrequencySRV.GetAddressOf());
            if (FAILED(result))
            {
                std::cout << "Failed to load low frequency texture" << std::endl;
            }
        }

        {
            result = DirectX::CreateDDSTextureFromFile(m_cpDevice.Get(),
                L"./assets/textures/HighFrequency/HighFrequency.dds",
                m_cpHighFrequencyResource.GetAddressOf(), m_cpHighFrequencySRV.GetAddressOf());
            if (FAILED(result))
            {
                std::cout << "Failed to load high frequency texture" << std::endl;
            }
        }

        {
            result = DirectX::CreateDDSTextureFromFile(m_cpDevice.Get(),
                L"./assets/textures/curlNoise.dds",
                m_cpCurlResource.GetAddressOf(), m_cpCurlSRV.GetAddressOf());
            if (FAILED(result))
            {
                std::cout << "Failed to load curl noise texture" << std::endl;
            }
        }

        {
            result = DirectX::CreateDDSTextureFromFile(m_cpDevice.Get(),
                L"./assets/textures/weatherMap.dds",
                m_cpWeatherResource.GetAddressOf(), m_cpWeatherSRV.GetAddressOf());
            if (FAILED(result))
            {
                std::cout << "Failed to load weather map texture" << std::endl;
            }
        }

        // Creat sampler
        {
            D3D11_SAMPLER_DESC samplerDesc;
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 1;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            samplerDesc.BorderColor[0] = 0;
            samplerDesc.BorderColor[1] = 0;
            samplerDesc.BorderColor[2] = 0;
            samplerDesc.BorderColor[3] = 0;
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

            result = m_cpDevice->CreateSamplerState(&samplerDesc, m_cpSamplerWrap.GetAddressOf());
            if (FAILED(result))
            {
                std::cout << "Failed to create sampler state" << std::endl;
                return;
            }
        }


        m_isInitialized = true;
    }

    void D3D11SpatiotemporalFilterBackend::Shutdown()
    {
    }

    void D3D11SpatiotemporalFilterBackend::Render(const VisibleSet& visibleSet, const LightSet& lightSet, const CameraEntry& currentCameraEntry, float deltaTime, float totalTime)
    {
        assert(m_isInitialized);

        struct VertexLocal
        {
            VertexLocal()
                : m_localPosition{}
                , m_matrixIndex{ static_cast<uint32_t>(-1) }
                , m_localNormal{ 0.0f, 1.0f, 0.0f }
                , _pad{ 0 }
            {
            }

            Farlor::Vector3 m_localPosition;
            uint32_t m_matrixIndex;
            Farlor::Vector3 m_localNormal;
            float _pad;
        };

        // This is the actual start of rendering... this should probably change to be the first thing
        m_gpuProfiler.StartTimingFrame(m_frameCount);

        m_gpuProfiler.StartTimingEvent(static_cast<uint32_t>(ProfileEvent::ClearBuffers));
        // Clear render target
        {
            float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            m_cpDeviceContext->ClearRenderTargetView(m_cpBackBufferRTV.Get(), color);
        }

        if (currentCameraEntry.m_cameraMoved)
        {
            m_iterativeFrameCount = 0;

            //Clear Cloud Buffer
            {
                {
                    // Set the Primary Ray Generation Pass State
                    m_cpDeviceContext->CSSetShader(m_cpClearHDRImageBufferCS.Get(), 0, 0);

                    const uint32_t numUAVS = 1;
                    ID3D11UnorderedAccessView* pUnorderedAccessViews[numUAVS];
                    pUnorderedAccessViews[0] = m_cpCloudBufferUAV.Get();
                    m_cpDeviceContext->CSSetUnorderedAccessViews(0, numUAVS, pUnorderedAccessViews, 0);
                }

                // Dispatch the Clear
                {
                    const uint32_t threadGroupSizeX = 256;
                    uint32_t numDispatchX = (m_clientWidth * m_clientHeight + (threadGroupSizeX - 1)) / threadGroupSizeX;
                    m_cpDeviceContext->Dispatch(numDispatchX, 1, 1);
                }

                // Pop the Clear
                {
                    m_cpDeviceContext->CSSetShader(nullptr, 0, 0);

                    const uint32_t numUAVS = 1;
                    ID3D11UnorderedAccessView* pUnorderedAccessViews[numUAVS];
                    pUnorderedAccessViews[0] = nullptr;
                    m_cpDeviceContext->CSSetUnorderedAccessViews(0, numUAVS, pUnorderedAccessViews, 0);
                }
            }
        }

        // Clear Depth Stencil Buffers
        {
            float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            m_cpDeviceContext->ClearDepthStencilView(m_cpDefaultDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_DEPTH, 1.0f, 0);
        }
        m_gpuProfiler.EndTimingEvent(static_cast<uint32_t>(ProfileEvent::ClearBuffers));

        // Clear all the other buffers!!
        {
            {
                float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                m_cpDeviceContext->ClearRenderTargetView(m_cpGeometryNormalOSRTV.Get(), color);
            }

            {
                float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                m_cpDeviceContext->ClearRenderTargetView(m_cpGeometryNormalWSRTV.Get(), color);
            }

            {
                float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                m_cpDeviceContext->ClearRenderTargetView(m_cpGeometryVelocityBufferRTV.Get(), color);
            }
        }

        // Set camera constant buffer parameters
        {
            CBs::cbCamera cameraData;
            cameraData.position = currentCameraEntry.m_position;

            // Update the per object buffer
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            ZeroMemory(&mappedResource, sizeof(mappedResource));
            m_cpDeviceContext->Map(m_cpCameraCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            memcpy(mappedResource.pData, &cameraData, sizeof(CBs::cbCamera));
            m_cpDeviceContext->Unmap(m_cpCameraCb.Get(), 0);
        }

        // We need to do the g buffer generation here
        m_gpuProfiler.StartTimingEvent(static_cast<uint32_t>(ProfileEvent::GBuffer));
        // Geometry Pass
        {
            auto visibleEntries = visibleSet.GetVisibleEntries();
            // Push Geometry Pass State
            {
                ID3D11RenderTargetView* pRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                for (uint32_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                {
                    pRenderTargets[i] = nullptr;
                }
                pRenderTargets[0] = m_cpGeometryNormalOSRTV.Get();
                pRenderTargets[1] = m_cpGeometryNormalWSRTV.Get();
                pRenderTargets[2] = m_cpGeometryVelocityBufferRTV.Get();
                ID3D11DepthStencilView* pDepthStencil = m_cpDefaultDSV.Get();
                m_cpDeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRenderTargets, pDepthStencil);

                m_cpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                // Set the correct shader
                m_cpDeviceContext->VSSetShader(m_cpGBufferVS.Get(), 0, 0);
                m_cpDeviceContext->PSSetShader(m_cpGBufferPS.Get(), 0, 0);
                m_cpDeviceContext->IASetInputLayout(m_cpGeometryDeferredInputLayout.Get());
            }

            {
                // Here, we want to calculate the matrices
                CBs::cbGeometryDeferredPerFrame perFrameData;
                // We access the current camera's view and projection matricies as well!
                perFrameData.ViewMatrix = currentCameraEntry.m_view;
                perFrameData.ProjMatrix = currentCameraEntry.m_proj;
                perFrameData.PrevViewMatrix = m_previousCamera.m_view;
                perFrameData.PrevProjMatrix = m_previousCamera.m_proj;

                // Update the per object buffer
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                ZeroMemory(&mappedResource, sizeof(mappedResource));
                m_cpDeviceContext->Map(m_cpGeometryDeferredPerFrameCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                memcpy(mappedResource.pData, &perFrameData, sizeof(CBs::cbGeometryDeferredPerFrame));
                m_cpDeviceContext->Unmap(m_cpGeometryDeferredPerFrameCb.Get(), 0);

                uint32_t startSlot = 1;
                uint32_t numVSConstBuffers = 1;
                m_cpDeviceContext->VSSetConstantBuffers(startSlot, numVSConstBuffers, m_cpGeometryDeferredPerFrameCb.GetAddressOf());
            }

            uint32_t meshId = 1;

            // Here, we want to have the backend handles
            for (const auto& geometry : visibleEntries)
            {
                const IGraphicsBackend::BackendGraphicsHandle backendHandle = m_renderer.GetBackendHandleFromAgnosticHandle(geometry.m_agnosticHandle);

                // Apply the correct mesh to the pipeline
                // Only do if we have changed mesh as optimization
                if (backendHandle != m_previousBackendHandle)
                {
                    m_previousBackendHandle = backendHandle;

                    auto itr = m_idToBackendGeometry.find(backendHandle);
                    if (itr == m_idToBackendGeometry.end())
                    {
                        // Log invalid case
                        continue;
                    }

                    // We can apply the mesh to the pipeline
                    ID3D11Buffer* pVertexBuffer = itr->second->GetRawVertexBuffer();
                    ID3D11Buffer* pIndexBuffer = itr->second->GetRawIndexBuffer();

                    // As of now, no batching is used
                    // TODO: This should be changed for static geometry at some point!!
                    uint32_t stride = itr->second->GetVertexStride();
                    uint32_t offset = itr->second->GetVertexOffset();
                    m_cpDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
                    m_cpDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                }

                // Now, check if we have a valid geometry handle!
                // If so, execute a draw. We already have batched the mesh draws
                if (m_previousBackendHandle != IGraphicsBackend::InvalidBackendGraphicsHandle)
                {
                    auto itr = m_idToBackendGeometry.find(m_previousBackendHandle);
                    if (itr == m_idToBackendGeometry.end())
                    {
                        // Log invalid case
                        continue;
                    }

                    {
                        // Here, we want to calculate the matrices
                        CBs::cbGeometryDeferredPerObject perObjData;
                        perObjData.WorldMatrix = geometry.m_transformMatrix; // Note: I believe this is correct. Verified with render doc.
                        // We access the current camera's view and projection matricies as well!
                        perObjData.InvWorldMatrix = geometry.m_transformMatrix.Inversed().Transposed();
                        perObjData.MeshID = meshId;

                        // Update the per object buffer
                        D3D11_MAPPED_SUBRESOURCE mappedResource;
                        ZeroMemory(&mappedResource, sizeof(mappedResource));
                        m_cpDeviceContext->Map(m_cpGeometryDeferredPerObjectCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                        memcpy(mappedResource.pData, &perObjData, sizeof(CBs::cbGeometryDeferredPerObject));
                        m_cpDeviceContext->Unmap(m_cpGeometryDeferredPerObjectCb.Get(), 0);

                        uint32_t startSlot = 0;
                        uint32_t numVSConstBuffers = 1;
                        m_cpDeviceContext->VSSetConstantBuffers(startSlot, numVSConstBuffers, m_cpGeometryDeferredPerObjectCb.GetAddressOf());
                    }

                    m_cpDeviceContext->DrawIndexed(itr->second->GetNumIndices(), 0, 0);
                }
                meshId++;
            }

            // Pop Geometry State
            {
                ID3D11RenderTargetView* pRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                for (uint32_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                {
                    pRenderTargets[i] = nullptr;
                }
                ID3D11DepthStencilView* pDepthStencil = nullptr;
                m_cpDeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRenderTargets, pDepthStencil);
            }

            {
                m_cpDeviceContext->VSSetShader(nullptr, 0, 0);
                m_cpDeviceContext->PSSetShader(nullptr, 0, 0);

                m_cpDeviceContext->IASetInputLayout(nullptr);

                m_cpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                const uint32_t numConstBuffers = 2;
                ID3D11Buffer* constantBuffers[numConstBuffers];
                constantBuffers[0] = nullptr;
                constantBuffers[1] = nullptr;
                const uint32_t startSlot = 0;
                m_cpDeviceContext->VSSetConstantBuffers(startSlot, numConstBuffers, constantBuffers);
            }
        }
        m_gpuProfiler.EndTimingEvent(static_cast<uint32_t>(ProfileEvent::GBuffer));

        // Push the Cloud Render Pass State
        m_gpuProfiler.StartTimingEvent(static_cast<uint32_t>(ProfileEvent::CloudTrace));
        {
            // New camera
            {
                CBs::cbPathTracerCamera pathTracerCamera;
                pathTracerCamera.cameraPos = currentCameraEntry.m_position;
                pathTracerCamera.cameraTarget = currentCameraEntry.m_target;
                pathTracerCamera.worldUp = currentCameraEntry.m_worldUp;
                pathTracerCamera.screenWidth = m_clientWidth;
                pathTracerCamera.screenHeight = m_clientHeight;
                pathTracerCamera.FOV_Horizontal = currentCameraEntry.m_fov;

                D3D11_MAPPED_SUBRESOURCE mappedResource;
                ZeroMemory(&mappedResource, sizeof(mappedResource));
                m_cpDeviceContext->Map(m_cpNewCameraCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                memcpy(mappedResource.pData, &pathTracerCamera, sizeof(CBs::cbPathTracerCamera));
                m_cpDeviceContext->Unmap(m_cpNewCameraCb.Get(), 0);
            }

            // Old Camera
            {
                CBs::cbPathTracerCamera pathTracerCamera;
                pathTracerCamera.cameraPos = m_previousCamera.m_position;
                pathTracerCamera.cameraTarget = m_previousCamera.m_target;
                pathTracerCamera.worldUp = m_previousCamera.m_worldUp;
                pathTracerCamera.screenWidth = m_clientWidth;
                pathTracerCamera.screenHeight = m_clientHeight;
                pathTracerCamera.FOV_Horizontal = m_previousCamera.m_fov;

                D3D11_MAPPED_SUBRESOURCE mappedResource;
                ZeroMemory(&mappedResource, sizeof(mappedResource));
                m_cpDeviceContext->Map(m_cpOldCameraCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                memcpy(mappedResource.pData, &pathTracerCamera, sizeof(CBs::cbPathTracerCamera));
                m_cpDeviceContext->Unmap(m_cpOldCameraCb.Get(), 0);
            }

            // Time Values
            {
                CBs::cbTimeValues timeValues;
                timeValues.DeltaTime = deltaTime;
                timeValues.TotalTime = totalTime;
                timeValues._pad[0] = 0.0f;
                timeValues._pad[1] = 0.0f;

                D3D11_MAPPED_SUBRESOURCE mappedResource;
                ZeroMemory(&mappedResource, sizeof(mappedResource));
                m_cpDeviceContext->Map(m_cpTimeValuesCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                memcpy(mappedResource.pData, &timeValues, sizeof(CBs::cbTimeValues));
                m_cpDeviceContext->Unmap(m_cpTimeValuesCb.Get(), 0);
            }

            // Set the shader
            m_cpDeviceContext->CSSetShader(m_cpCloudTraceCS.Get(), 0, 0);

            const uint32_t numUAVS = 1;
            ID3D11UnorderedAccessView* pUnorderedAccessViews[numUAVS];
            pUnorderedAccessViews[0] = m_cpCloudBufferUAV.Get();
            m_cpDeviceContext->CSSetUnorderedAccessViews(0, numUAVS, pUnorderedAccessViews, 0);

            const uint32_t numShaderResourceViews = 4;
            ID3D11ShaderResourceView* pShaderResourceViews[numShaderResourceViews];
            pShaderResourceViews[0] = m_cpLowFrequencySRV.Get();
            pShaderResourceViews[1] = m_cpHighFrequencySRV.Get();
            pShaderResourceViews[2] = m_cpCurlSRV.Get();
            pShaderResourceViews[3] = m_cpWeatherSRV.Get();
            m_cpDeviceContext->CSSetShaderResources(0, numShaderResourceViews, pShaderResourceViews);

            const uint32_t numConstBuffers = 3;
            ID3D11Buffer* constantBuffers[numConstBuffers];
            constantBuffers[0] = m_cpNewCameraCb.Get();
            constantBuffers[1] = m_cpOldCameraCb.Get();
            constantBuffers[2] = m_cpTimeValuesCb.Get();
            m_cpDeviceContext->CSSetConstantBuffers(0, numConstBuffers, constantBuffers);

            const uint32_t numSamplerStates = 1;
            ID3D11SamplerState* samplerStates[numSamplerStates];
            samplerStates[0] = m_cpSamplerWrap.Get();
            m_cpDeviceContext->CSSetSamplers(0, numSamplerStates, samplerStates);
        }

        // Dispatch the Cloud Render pass
        {
            const uint32_t threadGroupX = 32;
            const uint32_t threadGroupY = 16;

            uint32_t xDispatch = m_clientWidth / threadGroupX;
            if (m_clientWidth % threadGroupX)
                xDispatch++;

            uint32_t yDispatch = m_clientHeight / threadGroupY;
            if (m_clientHeight % threadGroupY)
                yDispatch++;

            m_cpDeviceContext->Dispatch(xDispatch, yDispatch, 1);
        }

        // Pop the path tracing pass state
        {
            m_cpDeviceContext->CSSetShader(nullptr, 0, 0);

            const uint32_t numUAVS = 1;
            ID3D11UnorderedAccessView* pUnorderedAccessViews[numUAVS];
            pUnorderedAccessViews[0] = nullptr;
            m_cpDeviceContext->CSSetUnorderedAccessViews(0, numUAVS, pUnorderedAccessViews, 0);

            const uint32_t numShaderResourceViews = 4;
            ID3D11ShaderResourceView* pShaderResourceViews[numShaderResourceViews];
            pShaderResourceViews[0] = nullptr;
            pShaderResourceViews[1] = nullptr;
            pShaderResourceViews[2] = nullptr;
            pShaderResourceViews[3] = nullptr;
            m_cpDeviceContext->CSSetShaderResources(0, numShaderResourceViews, pShaderResourceViews);

            const uint32_t numConstBuffers = 3;
            ID3D11Buffer* constantBuffers[numConstBuffers];
            constantBuffers[0] = nullptr;
            constantBuffers[1] = nullptr;
            constantBuffers[2] = nullptr;
            m_cpDeviceContext->CSSetConstantBuffers(0, numConstBuffers, constantBuffers);

            const uint32_t numSamplerStates = 1;
            ID3D11SamplerState* samplerStates[numSamplerStates];
            samplerStates[0] = nullptr;
            m_cpDeviceContext->CSSetSamplers(0, numSamplerStates, samplerStates);
        }
        m_gpuProfiler.EndTimingEvent(static_cast<uint32_t>(ProfileEvent::CloudTrace));

        // Tonemap Pass
        m_gpuProfiler.StartTimingEvent(static_cast<uint32_t>(ProfileEvent::Tonemap));
        {
            {
                ID3D11RenderTargetView* pRenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                for (uint32_t i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                {
                    pRenderTargets[i] = nullptr;
                }
                pRenderTargets[0] = m_cpBackBufferRTV.Get();
                ID3D11DepthStencilView* pDepthStencil = nullptr;
                m_cpDeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRenderTargets, pDepthStencil);
            }

            // Set Shaders
            m_cpDeviceContext->VSSetShader(m_cpTonemappingVS.Get(), nullptr, 0);
            m_cpDeviceContext->PSSetShader(m_cpTonemappingPS.Get(), nullptr, 0);

            // Update constant buffer
            {
                CBs::cbTonemapPassSettings tonemapPassSettings;
                tonemapPassSettings.ScreenWidth = m_clientWidth;
                tonemapPassSettings.ScreenHeight = m_clientHeight;
                tonemapPassSettings.PTC_Pad0 = 0;
                tonemapPassSettings.PTC_Pad1 = 0;

                // Update the per object buffer
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                ZeroMemory(&mappedResource, sizeof(mappedResource));
                m_cpDeviceContext->Map(m_cpTonemapPassCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                memcpy(mappedResource.pData, &tonemapPassSettings, sizeof(CBs::cbTonemapPassSettings));
                m_cpDeviceContext->Unmap(m_cpTonemapPassCb.Get(), 0);
            }

            {
                const uint32_t numConstBuffers = 1;
                ID3D11Buffer* constantBuffers[numConstBuffers];
                constantBuffers[0] = m_cpTonemapPassCb.Get();
                uint32_t startSlot = 0;
                m_cpDeviceContext->PSSetConstantBuffers(startSlot, numConstBuffers, constantBuffers);
            }

            // Set shader resources
            // Pixel Shader
            {
                const uint32_t numPixelSRVs = 1;
                ID3D11ShaderResourceView* pPixelSRVs[numPixelSRVs];
                pPixelSRVs[0] = m_cpCloudBufferSRV.Get();
                m_cpDeviceContext->PSSetShaderResources(0, 1, pPixelSRVs);
                m_cpDeviceContext->PSSetSamplers(0, 1, m_cpCommonSampler.GetAddressOf());
            }

            m_cpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Draw fullscreen triangle for tonemapping pass
            m_cpDeviceContext->Draw(3, 0);

            // Pop shader resources
            {
                {
                    m_cpDeviceContext->VSSetShader(nullptr, nullptr, 0);
                    m_cpDeviceContext->PSSetShader(nullptr, nullptr, 0);
                    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
                    m_cpDeviceContext->PSSetShaderResources(0, 1, nullSRV);
                }

                {
                    const uint32_t numConstBuffers = 1;
                    ID3D11Buffer* constantBuffers[numConstBuffers];
                    constantBuffers[0] = nullptr;
                    m_cpDeviceContext->PSSetConstantBuffers(0, numConstBuffers, constantBuffers);
                }
            }
        }
        m_gpuProfiler.EndTimingEvent(static_cast<uint32_t>(ProfileEvent::Tonemap));


        // Present the back buffer
        m_cpSwapChain->Present(0, 0);

        m_gpuProfiler.EndTimingFrame(m_frameCount);

        // Cache the camera data as previous camera data
        m_previousCamera = currentCameraEntry;

        ++m_frameCount;
        ++m_iterativeFrameCount;
    }
}
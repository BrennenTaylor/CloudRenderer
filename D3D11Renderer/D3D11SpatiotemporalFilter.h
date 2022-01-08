#pragma once

#include <D3D11Backend\D3D11Backend.h>

#include <D3D11GpuProfiler.h>

#include <Geometry.h>

#include <map>

#include <wrl.h>
#include <d3d11.h>

#include <Windows.h>

namespace Farlor
{
    class Renderer;

    class D3D11SpatiotemporalFilterBackend : public D3D11Backend
    {
    public:
        D3D11SpatiotemporalFilterBackend(const Renderer& renderer);
        virtual ~D3D11SpatiotemporalFilterBackend();

        virtual void Initialize(IWindow *pWindow, std::string& resourceDir) override;
        virtual void Shutdown() override;

        virtual void Render(const VisibleSet& visibleSet, const LightSet& lightSet, const CameraEntry& currentCameraEntry, float deltaTime, float totalTime) override;

    private:
        D3D11GpuProfiler m_gpuProfiler;
        uint32_t m_frameCount;
        uint32_t m_iterativeFrameCount;

        // Temporal cached data
        Farlor::CameraEntry m_previousCamera;

        // Render target views
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_cpGeometryNormalOSRTV;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_cpGeometryNormalWSRTV;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_cpGeometryVelocityBufferRTV;

        // Shader resource views
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpLocalVertexSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpIndexSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpLocalToWorldSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpLocalToWorldInvSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpTransformedVertexSRV;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpGeometryNormalOSSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpGeometryNormalWSSRV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpGeometryVelocityBufferSRV;

        // Cloud Buffer
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_cpCloudBufferUAV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpCloudBufferSRV;

        // Clear HDR image buffer
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_cpClearHDRImageBufferCS;

        // PathTracing Compute
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_cpCloudTraceCS;
            
        // G-Buffer Pass
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_cpGBufferVS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_cpGBufferPS;

        // Tonemapping
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_cpTonemappingVS;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_cpTonemappingPS;

        // Input Layouts
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_cpGeometryDeferredInputLayout;

        // Constant buffers
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpCameraCb;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpPathTracerControlCb;
        
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpNewCameraCb;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpOldCameraCb;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpTimeValuesCb;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpGeometryDeferredPerObjectCb;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpGeometryDeferredPerFrameCb;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpTonemapPassCb;

        Microsoft::WRL::ComPtr<ID3D11Resource> m_cpLowFrequencyResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpLowFrequencySRV;

        Microsoft::WRL::ComPtr<ID3D11Resource> m_cpHighFrequencyResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpHighFrequencySRV;

        Microsoft::WRL::ComPtr<ID3D11Resource> m_cpCurlResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpCurlSRV;

        Microsoft::WRL::ComPtr<ID3D11Resource> m_cpWeatherResource;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpWeatherSRV;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cpSamplerWrap;
    };
}
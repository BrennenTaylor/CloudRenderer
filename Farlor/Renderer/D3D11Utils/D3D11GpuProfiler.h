#pragma once

#include <d3d11.h>
#include <wrl.h>

#include <cstdint>
#include <map>
#include <vector>

namespace Farlor
{
    class D3D11GpuProfiler
    {
        struct PerEvent
        {
            uint32_t m_eventId;
            bool m_startActiveForFrame;
            bool m_endActiveForFrame;
            Microsoft::WRL::ComPtr<ID3D11Query> m_cpStartQuery;
            Microsoft::WRL::ComPtr<ID3D11Query> m_cpEndQuery;
        };

        struct PerFrame
        {
            uint32_t m_frameCount;
            Microsoft::WRL::ComPtr<ID3D11Query> m_cpDisjointQuery;
            Microsoft::WRL::ComPtr<ID3D11Query> m_cpBeginFrameQuery;
            Microsoft::WRL::ComPtr<ID3D11Query> m_cpEndFrameQuery;
            std::vector<PerEvent> m_eventTimings;
        };

    public:
        D3D11GpuProfiler(uint32_t numEventsPerFrame, uint32_t frameDelay = 3);
        
        void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        void StartTimingFrame(uint32_t frameCount);
        void EndTimingFrame(uint32_t frameCount);

        void StartTimingEvent(uint32_t event);
        void EndTimingEvent(uint32_t event);

        void PrintTimingInfo(uint32_t frameToPrint);

    private:
        uint32_t m_numEventsPerFrame;
        uint32_t m_frameDelay;

        uint32_t m_currentFrame;
        
        ID3D11Device* m_pDevice;
        ID3D11DeviceContext* m_pDeviceContext;

        std::vector<PerFrame> m_frameTiming;
    };
}
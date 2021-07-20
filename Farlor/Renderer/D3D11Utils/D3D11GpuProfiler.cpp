#include "D3D11GpuProfiler.h"

#include <iostream>

namespace Farlor
{
    D3D11GpuProfiler::D3D11GpuProfiler(uint32_t numEventsPerFrame, uint32_t frameDelay)
        : m_numEventsPerFrame{ numEventsPerFrame }
        , m_frameDelay{ frameDelay }
        , m_currentFrame{ static_cast<uint32_t>(-1) }
        , m_pDevice{ nullptr }
        , m_pDeviceContext{ nullptr }
        , m_frameTiming{}
    {
    }

    void D3D11GpuProfiler::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
    {
        m_pDevice = pDevice;
        m_pDeviceContext = pDeviceContext;

        for (uint32_t i = 0; i < m_frameDelay; ++i)
        {
            PerFrame perFrame{};
            {
                D3D11_QUERY_DESC queryDesc{};
                queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
                queryDesc.MiscFlags = 0;
                m_pDevice->CreateQuery(&queryDesc, perFrame.m_cpDisjointQuery.GetAddressOf());
            }
            {
                D3D11_QUERY_DESC queryDesc{};
                queryDesc.Query = D3D11_QUERY_TIMESTAMP;
                queryDesc.MiscFlags = 0;
                m_pDevice->CreateQuery(&queryDesc, perFrame.m_cpBeginFrameQuery.GetAddressOf());
                m_pDevice->CreateQuery(&queryDesc, perFrame.m_cpEndFrameQuery.GetAddressOf());
            }

            for (uint32_t j = 0; j < m_numEventsPerFrame; ++j)
            {
                PerEvent perEvent{};
                perEvent.m_startActiveForFrame = false;
                perEvent.m_endActiveForFrame = false;
                perEvent.m_eventId = j;

                {
                    D3D11_QUERY_DESC queryDesc{};
                    queryDesc.Query = D3D11_QUERY_TIMESTAMP;
                    queryDesc.MiscFlags = 0;
                    m_pDevice->CreateQuery(&queryDesc, perEvent.m_cpStartQuery.GetAddressOf());
                    m_pDevice->CreateQuery(&queryDesc, perEvent.m_cpEndQuery.GetAddressOf());
                }
                
                perFrame.m_eventTimings.push_back(perEvent);
            }

            // Add this timing to the list
            m_frameTiming.push_back(perFrame);
        }
    }

    void D3D11GpuProfiler::StartTimingFrame(uint32_t frameNum)
    {
        auto& frameInfo = m_frameTiming[frameNum % m_frameDelay];
        frameInfo.m_frameCount = frameNum;
        m_currentFrame = frameNum;

        // Reset active events
        for (auto& eventInfo : frameInfo.m_eventTimings)
        {
            eventInfo.m_startActiveForFrame = false;
            eventInfo.m_endActiveForFrame = false;
        }

        m_pDeviceContext->Begin(frameInfo.m_cpDisjointQuery.Get());
        m_pDeviceContext->End(frameInfo.m_cpBeginFrameQuery.Get());
    }

    void D3D11GpuProfiler::EndTimingFrame(uint32_t frameNum)
    {
        auto& frameInfo = m_frameTiming[frameNum % m_frameDelay];
        m_pDeviceContext->End(frameInfo.m_cpEndFrameQuery.Get());
        m_pDeviceContext->End(frameInfo.m_cpDisjointQuery.Get());
    }

    void D3D11GpuProfiler::StartTimingEvent(uint32_t eventNum)
    {
        auto& eventInfo = m_frameTiming[m_currentFrame % m_frameDelay].m_eventTimings[eventNum];
        m_pDeviceContext->End(eventInfo.m_cpStartQuery.Get());
        eventInfo.m_startActiveForFrame = true;
    }

    void D3D11GpuProfiler::EndTimingEvent(uint32_t eventNum)
    {
        auto& eventInfo = m_frameTiming[m_currentFrame % m_frameDelay].m_eventTimings[eventNum];
        m_pDeviceContext->End(eventInfo.m_cpEndQuery.Get());
        eventInfo.m_endActiveForFrame = true;
    }

    void D3D11GpuProfiler::PrintTimingInfo(uint32_t frameToPrint)
    {
        const auto& frameInfo = m_frameTiming[frameToPrint % m_frameDelay];
        if (frameInfo.m_frameCount != frameToPrint)
        {
            std::cout << "Timings overwritten for frame: " << frameToPrint << std::endl;
            return;
        }

        std::cout << "Timings for Frame: " << frameToPrint << std::endl;

        while (m_pDeviceContext->GetData(frameInfo.m_cpDisjointQuery.Get(), 0, 0, 0) == S_FALSE)
        {
            std::cout << "Error printing frame: " << frameToPrint << " , info not ready" << std::endl;
        }

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointTimestep{};
        m_pDeviceContext->GetData(frameInfo.m_cpDisjointQuery.Get(), &disjointTimestep, sizeof(disjointTimestep), 0);
        if (disjointTimestep.Disjoint)
        {
            std::cout << "Cant print info... disjoint for frame: " << frameToPrint << std::endl;
        }

        auto TimingDifference = [=](UINT64 begin, UINT64 end) -> float
        {
            return float(end - begin) / float(disjointTimestep.Frequency) * 1000.0f;
        };

        UINT64 beginFrameTS = 0;
        UINT64 endFrameTS = 0;
        m_pDeviceContext->GetData(frameInfo.m_cpBeginFrameQuery.Get(), &beginFrameTS, sizeof(beginFrameTS), 0);
        m_pDeviceContext->GetData(frameInfo.m_cpEndFrameQuery.Get(), &endFrameTS, sizeof(endFrameTS), 0);

        std::cout << "Total Frame Time: " << TimingDifference(beginFrameTS, endFrameTS) << "ms" << std::endl;

        for (const auto& eventInfo : frameInfo.m_eventTimings)
        {
            if (!eventInfo.m_startActiveForFrame || !eventInfo.m_endActiveForFrame)
            {
                std::cout << "\tEvent Inactive: " << eventInfo.m_eventId << std::endl;
                continue;
            }

            UINT64 beginEventTS = 0;
            UINT64 endEventTS = 0;
            m_pDeviceContext->GetData(eventInfo.m_cpStartQuery.Get(), &beginEventTS, sizeof(beginEventTS), 0);
            m_pDeviceContext->GetData(eventInfo.m_cpEndQuery.Get(), &endEventTS, sizeof(endEventTS), 0);
            std::cout << "\tEvent " << eventInfo.m_eventId << " Time: " << TimingDifference(beginEventTS, endEventTS) << "ms" << std::endl;
        }
    }
}
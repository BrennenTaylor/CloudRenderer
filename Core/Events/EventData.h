#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "..\..\Util\StringID.h"

#include <memory>

namespace Farlor
{
    typedef StrID EventType;

    struct EventData
    {
        EventData()
        {
            LARGE_INTEGER counter;
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&counter);
            m_timestamp = (double)counter.QuadPart / (double)frequency.QuadPart;
        }

        virtual EventType GetEventType() = 0;

        double m_timestamp;
    };

    typedef std::shared_ptr<EventData> EventDataPtr;
}

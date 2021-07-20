#pragma once

#include <Windows.h>

namespace Farlor
{
    class Timer
    {
    public:
        Timer();

        float DeltaTimeInSeconds();
        float TotalTimeInSeconds();
        float CurrentTime();

        void Reset();
        void Start();
        void Stop();
        void Tick();

        void Toggle();

    private:
        double m_deltaTimeInSeconds;
        double m_secondsPerTick; // Store it in this order to avoid constant divide

        __int64 m_baseTime;
        __int64 m_pausedTime;
        __int64 m_stopTime;
        __int64 m_prevTime;
        __int64 m_currentTime;

        bool m_stopped;
    };
}

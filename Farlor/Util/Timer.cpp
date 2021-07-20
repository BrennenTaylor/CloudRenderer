#include "Timer.h"

namespace Farlor
{
    Timer::Timer()
        : m_deltaTimeInSeconds(0.0)
        , m_secondsPerTick(0.0)
        , m_baseTime(0)
        , m_stopTime(0)
        , m_pausedTime(0)
        , m_prevTime(0)
        , m_currentTime(0)
        , m_stopped(false)
    {
        __int64 countsPerSec;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
        if (static_cast<double>(countsPerSec))
        {
            m_secondsPerTick = 0.0f;
        }
        m_secondsPerTick = 1.0f / static_cast<double>(countsPerSec);
    }

    float Timer::DeltaTimeInSeconds()
    {
        return static_cast<float>(m_deltaTimeInSeconds);
    }

    float Timer::CurrentTime()
    {
        __int64 currentTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        return (float)(((currentTime - m_pausedTime) - m_baseTime) * m_secondsPerTick);
    }

    float Timer::TotalTimeInSeconds()
    {
        if (m_stopped)
        {
            return (float)(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerTick);
        }
        else
        {
            return (float)(((m_currentTime - m_pausedTime) - m_baseTime) * m_secondsPerTick);
        }
    }

    void Timer::Reset()
    {
        __int64 currentTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        m_baseTime = currentTime;
        m_prevTime = currentTime;
        m_stopTime = 0;
        m_stopped = false;
    }

    void Timer::Start()
    {
        __int64 startTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

        if (m_stopped)
        {
            m_pausedTime += (startTime - m_stopTime);

            m_prevTime = startTime;

            m_stopTime = 0;
            m_stopped = false;
        }
    }

    void Timer::Stop()
    {
        if (!m_stopped)
        {
            __int64 currentTime;
            QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
            m_stopTime = currentTime;
            m_stopped = true;
        }
    }

    void Timer::Toggle()
    {
        if (m_stopped)
        {
            Start();
        }
        else
        {
            Stop();
        }
    }

    void Timer::Tick()
    {
        if (m_stopped)
        {
            m_deltaTimeInSeconds = 0.0;
            return;
        }

        // Get the time this frame
        __int64 currentTime;
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        m_currentTime = currentTime;

        // Time difference between this and previous time
        m_deltaTimeInSeconds = (m_currentTime - m_prevTime) * m_secondsPerTick;

        // Prepare for next frame
        m_prevTime = m_currentTime;

        // Force non negative
        if (m_deltaTimeInSeconds < 0.0)
        {
            m_deltaTimeInSeconds = 0.0;
        }

        return;
    }
}

#pragma once

namespace Farlor
{
    class FixedUpdate
    {
    public:
        explicit FixedUpdate(float updateRate);
        bool ShouldUpdate();

        void AccumulateTime(float frameTime);

        float GetUpdateRate();

        float m_updateRate;

    public:
        float m_accumulator;
    };
}

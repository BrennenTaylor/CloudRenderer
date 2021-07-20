#pragma once

#include "../Farlor.h"

namespace Farlor
{
    template <class T>
    class StateMonitor
    {
    public:
        StateMonitor(T initialState);
        ~StateMonitor();

        void SetSister(StateMonitor<T> *pSister);
        bool SameAsSister();

        void SetState(T state);
        T GetState() const;

        bool IsUpdateNeeded();
        void InitializeState();
        void ResetTracking();

    private:
        bool m_gpuUpdateNeeded;

        T m_initialState;
        T m_state;

        StateMonitor<T> *m_pSisterState;
    };
}

#include "StateMonitor.inl"
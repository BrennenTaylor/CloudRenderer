#pragma once

namespace Farlor
{
    template <class T, unsigned int N>
    class StateMonitorArray
    {
    public:

        StateMonitorArray(T initialState);
        ~StateMonitorArray();

        void SetSister(StateMonitorArray<T, N> *pSister);
        bool SameAsSister(unsigned int slot);

        void SetState(unsigned int slot, T state);

        bool IsUpdateNeeded();
        unsigned int GetStartSlot();
        unsigned int GetEndSlot();
        unsigned int GetRange();

        T GetState(unsigned int slot);
        T* GetFirstSlotLocation();
        T* GetSlotLocation(unsigned int slot);

        void InitializeStates();
        void ResetTracking();

    private:
        void SetStartFromBelow();
        void SetEndFromAbove();

    private:
        // Upload Data
        bool m_gpuUploadNeeded;
        unsigned int m_uploadStartSlot;
        unsigned int m_uploadEndSlot;

        // State Data
        T m_initialState;
        T m_states[N];

        // Sister State
        StateMonitorArray<T, N> *m_pSister;
    };
}

#include "StateMonitorArray.inl"
namespace Farlor
{
    template <class T, unsigned int N>
    StateMonitorArray<T, N>::StateMonitorArray(T initialState)
        : m_uploadStartSlot{0}
        , m_uploadEndSlot{0}
        , m_gpuUploadNeeded{false}
        , m_initialState{initialState}
        , m_pSister{nullptr}
    {
        InitializeStates();
        ResetTracking();
    }

    template <class T, unsigned int N>
    StateMonitorArray<T, N>::~StateMonitorArray()
    {
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::SetSister(StateMonitorArray<T, N> *pSister)
    {
        m_pSister = pSister;
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::SetState(unsigned int slot, T state)
    {
        assert(slot < N);

        m_states[slot] = state;

        // Ensure that a sister state has been set, and if so then use it to manage
        // which slots need to be set in the next update to the pipeline.

        // If there is no sister state, then we default to always requiring an upload
        // of the entire array.

        if (m_pSister == nullptr)
        {
            m_gpuUploadNeeded = true;
            m_uploadStartSlot = 0;
            m_uploadEndSlot = N - 1;

            return;
        }

        bool sameAsSister = SameAsSister(slot);
        // Begin by checking if any sates changes are pending
        if (!m_gpuUploadNeeded && !sameAsSister)
        {
            m_gpuUploadNeeded = true;
            m_uploadStartSlot = slot;
            m_uploadEndSlot = slot;
        }

        // If one is already needed, then we need to update the state of our list
        if (m_gpuUploadNeeded)
        {
            // Below current range
            if (slot < m_uploadStartSlot)
            {
                 if (!sameAsSister)
                 {
                     m_uploadStartSlot = slot;
                 }
            }
            // Updating flow at lower end point
            else if (slot == m_uploadStartSlot)
            {
                if (sameAsSister)
                {
                    SetStartFromBelow();
                }
            }

            // Updating slot inside current range
            else if (m_uploadStartSlot < slot && slot < m_uploadEndSlot)
            {
                // This case does not matter
            }

            // Updating slot at upper end point
            else if (slot == m_uploadEndSlot)
            {
                if (sameAsSister)
                {
                    SetEndFromAbove();
                }
            }
            
            // Updating slot above current range
            else if (m_uploadEndSlot < slot)
            {
                if (!sameAsSister)
                {
                    m_uploadEndSlot = slot;
                }
            }
        }
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::SetStartFromBelow()
    {
        for (; m_uploadStartSlot < m_uploadEndSlot; m_uploadStartSlot++)
        {
            if (!SameAsSister(m_uploadStartSlot))
            {
                break;
            }
        }

        // IF no difference has been found, we compare last avalible slot
        if (m_uploadStartSlot == m_uploadEndSlot && SameAsSister(m_uploadStartSlot))
        {
            ResetTracking();
        }
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::SetEndFromAbove()
    {
        for (; m_uploadEndSlot > m_uploadStartSlot; m_uploadEndSlot--)
        {
            if (!SameAsSister(m_uploadEndSlot))
            {
                break;
            }
        }

        // IF no difference has been found, we compare last avalible slot
        if (m_uploadStartSlot == m_uploadEndSlot && SameAsSister(m_uploadEndSlot))
        {
            ResetTracking();
        }
    }

    template <class T, unsigned int N>
    bool StateMonitorArray<T, N>::SameAsSister(unsigned int slot)
    {
        assert(slot < N);
        return (m_states[slot] == m_pSister->m_states[slot]);
    }

    template <class T, unsigned int N>
    bool StateMonitorArray<T, N>::IsUpdateNeeded()
    {
        return m_gpuUploadNeeded;
    }

    template <class T, unsigned int N>
    unsigned int StateMonitorArray<T, N>::GetStartSlot()
    {
        return m_uploadStartSlot;
    }

    template <class T, unsigned int N>
    unsigned int StateMonitorArray<T, N>::GetEndSlot()
    {
        return m_uploadEndSlot;
    }

    template <class T, unsigned int N>
    unsigned int StateMonitorArray<T, N>::GetRange()
    {
        return m_uploadEndSlot - m_uploadStartSlot + 1;
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::InitializeStates()
    {
        for (unsigned int i = 0; i < N; i++)
        {
            SetState(i, m_initialState);
        }
    }

    template <class T, unsigned int N>
    void StateMonitorArray<T, N>::ResetTracking()
    {
        m_uploadStartSlot = 0;
        m_uploadEndSlot = 0;
        m_gpuUploadNeeded = false;
    }

    template <class T, unsigned int N>
    T StateMonitorArray<T, N>::GetState(unsigned int slot)
    {
        assert(slot < N);

        return m_states[slot];
    }

    template <class T, unsigned int N>
    T* StateMonitorArray<T, N>::GetFirstSlotLocation()
    {
        return &m_states[m_uploadStartSlot];
    }

    template <class T, unsigned int N>
    T* StateMonitorArray<T, N>::GetSlotLocation(unsigned int slot)
    {
        assert (slot < N);
        return &m_states[slot];
    }
}
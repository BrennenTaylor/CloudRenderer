namespace Farlor
{
    template <class T>
    StateMonitor<T>::StateMonitor(T initialState)
        : m_gpuUpdateNeeded{false}
        , m_initialState{initialState}
        , m_state{initialState}
        , m_pSisterState{nullptr}
    {
        InitializeState();
        ResetTracking();
    }

    template <class T>
    StateMonitor<T>::~StateMonitor()
    {
    }

    template <class T>
    void StateMonitor<T>::SetSister(StateMonitor<T> *pSister)
    {
        m_pSisterState = pSister;
    }

    template <class T>
    void StateMonitor<T>::SetState(T state)
    {
        m_state = state;

        // Ensure that sister state has been set

        if (m_pSisterState == nullptr)
        {
            m_gpuUpdateNeeded = true;
            return;
        }

        // As we only manage a single state, we update the status
        // based on a simple compare after the state has been updated

        // i.e. is not the same as the sister
        m_gpuUpdateNeeded = !SameAsSister();
    }

    template <class T>
    bool StateMonitor<T>::SameAsSister()
    {
        return (m_state == m_pSisterState->m_state);
    }

    template <class T>
    bool StateMonitor<T>::IsUpdateNeeded()
    {
        return m_gpuUpdateNeeded;
    }

    template <class T>
    void StateMonitor<T>::InitializeState()
    {
        SetState(m_initialState);
    }

    template <class T>
    void StateMonitor<T>::ResetTracking()
    {
        m_gpuUpdateNeeded = false;
    }

    template <class T>
    T StateMonitor<T>::GetState() const
    {
        return m_state;
    }
}
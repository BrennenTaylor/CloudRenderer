namespace Farlor
{
    template <class T>
    MultithreadedVector<T>::MultithreadedVector()
        : m_vector{}
        , m_mutex{}
        , m_conditionVariable{}
    {
    }

    template <class T>
    MultithreadedVector<T>::~MultithreadedVector()
    {
    }

    template <class T>
    bool MultithreadedVector<T>::Empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_vector.empty();
    }

    template <class T>
    size_t MultithreadedVector<T>::Size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    template <class T>
    void MultithreadedVector<T>::Push(T& t)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(std::move(t));
        }
        m_conditionVariable.notify_one();
    }

    template <class T>
    T MultithreadedVector<T>::GetElementWait(u32 index)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        T value = m_vector.at();
        m_queue.pop();
        return value;
    }
}
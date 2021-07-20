namespace Farlor
{
    template <class T>
    MultithreadedQueue<T>::MultithreadedQueue()
        : m_queue{}
        , m_mutex{}
        , m_conditionVariable{}
    {
    }

    template <class T>
    MultithreadedQueue<T>::~MultithreadedQueue()
    {
    }

    template <class T>
    bool MultithreadedQueue<T>::Empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    template <class T>
    size_t MultithreadedQueue<T>::Size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    template <class T>
    void MultithreadedQueue<T>::Push(T& t)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(std::move(t));
        }
        m_conditionVariable.notify_one();
    }

    template <class T>
    bool MultithreadedQueue<T>::TryPop(T& result)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;

        result = m_queue.front();
        m_queue.pop();
        return true;
    }

    template <class T>
    T MultithreadedQueue<T>::WaitPop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            m_conditionVariable.wait(lock);
        }

        T value = m_queue.front();
        m_queue.pop();
        return value;
    }
}
#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Farlor
{
    template <class T>
    class MultithreadedQueue
    {
    public:
        MultithreadedQueue();
        ~MultithreadedQueue();

        bool Empty();
        void Push(T& t);
        bool TryPop(T& result);
        T WaitPop();
        std::size_t Size();

    private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_conditionVariable;
    };
}

#include "MultithreadQueue.inc"
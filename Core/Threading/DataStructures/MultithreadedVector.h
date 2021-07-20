#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

namespace Farlor
{
    template <class T>
    class MultithreadedVector
    {
    public:
        MultithreadedVector();
        ~MultithreadedVector();

        std::size_t Size();

        void PushBack(T t);
        void Remove(T t);

        T GetElementWait(u32 index);


    private:
        std::vector<T> m_vector;
        mutable std::mutex m_mutex;
        std::condition_variable m_conditionVariable;
    };
}

#include "MultithreadedVector.inc"
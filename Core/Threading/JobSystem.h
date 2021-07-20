#pragma once

#include "DataStructures\MultithreadQueue.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <thread>

#include <Windows.h>

typedef void(*JobFunction)(void*);

namespace Farlor
{
    namespace FarlorJobs
    {
        // One job system is in charge of a single job queue. Only one of these should be made probably...
        // but I havenet decided.
        class JobSystem
        {
        public:
            using FiberPtr = void*;
            using ThreadHandle = HANDLE;
            using JobArgument = void*;
            
            struct Counter;
            
            struct Job
            {
                explicit Job()
                    : m_jobFunction{}
                    , m_jobArgument{}
                    , m_spCounter{ nullptr }
                {
                }

                JobFunction m_jobFunction;
                JobArgument m_jobArgument;
                std::shared_ptr<Counter> m_spCounter;
            };

            struct Counter
            {
                explicit Counter()
                    : m_atomicCounter{ 0 }
                    , m_desiredValue{ 0 }
                {
                }

                // Stores the current count for the counter
                std::atomic<uint32_t> m_atomicCounter;
                // What we want the counter to be
                uint32_t m_desiredValue;
            };

            // Wraps a system fiber.
            struct Fiber
            {
                explicit Fiber()
                    : m_pFiber{ nullptr }
                    , m_jobSystemId{ static_cast<uint32_t>(-1) }
                    , m_pJob{ nullptr }
                {
                }

                // OS fiber handle
                FiberPtr m_pFiber;
                // Id Assigned by the job system
                // Ranges [1, max fibers]
                uint32_t m_jobSystemId;
                // Points to the job we are execting
                Job* m_pJob;
            };

            // Wraps a system thread.
            struct Thread
            {
                explicit Thread()
                    : m_thread{ 0 }
                    , m_jobSystemId{ 0 }
                    , m_osThreadId{ 0 }
                {
                }

                // OS thread handle
                ThreadHandle m_thread;
                // Id assigned by the job system
                // Ranges [1, numthreads]
                uint32_t m_jobSystemId;
                // Id assigned by the os
                uint32_t m_osThreadId;
            };

            // Storage local to an individual thread
            struct ThreadLocalStorage
            {
                explicit ThreadLocalStorage()
                    : m_currentFiberIndex{ 0 }
                    , m_threadFiber{nullptr}
                {
                }

                uint32_t m_currentFiberIndex;
                FiberPtr m_threadFiber;
            };

            // Storage local to an individual fiber
            struct FiberLocalStorage
            {
                explicit FiberLocalStorage()
                    : m_spWaitingCounter{ nullptr }
                    , m_markWaitingIndex{ 0 }
                    , m_markFreeIndex{ 0 }
                {
                }

                std::shared_ptr<Counter> m_spWaitingCounter;
                uint32_t m_markWaitingIndex;
                uint32_t m_markFreeIndex;
            };

        public:
            JobSystem(uint32_t numFibers = 100, uint32_t maxNumThreads = 4);
            void BootstrapMainTask(JobFunction mainTask, void* pMainTaskArg);

            // Non-owning counter return
            std::shared_ptr<JobSystem::Counter> SubmitJobs(Job* pJobs, uint32_t numJobs);
            void Wait(std::shared_ptr<Counter> spCounter);

            uint32_t GetCurrentJobSystemThreadIndex();
            uint32_t GetCurrentJobSystemFiberIndex();

        public:
            struct MainFiberFuncArg
            {
                JobSystem* pJobSystem;
                Job bootstrapJob;
            };
            static void CALLBACK MainFiberFunc(void* pAarg);

            struct FiberFuncArg
            {
                JobSystem* pJobSystem;
            };

            static void CALLBACK FiberFunc(void* pAarg);

            struct ThreadFuncArg
            {
                JobSystem* pJobSystem;
            };
            static DWORD WINAPI ThreadFunc(void* pArg);

        private:
            // This is the job queue. It stores all inserted jobs that will be run.
            MultithreadedQueue<Job> m_jobQueue;

            // This is the numbers of job fibers to create.
            uint32_t m_numFibers;
            uint32_t m_numHwThreads;

            std::atomic<bool> m_quitting;

            // Wrappers around thread and fiber structures
            std::vector<Thread> m_threads;
            std::vector<Fiber> m_fibers;
            
            
            std::vector<std::atomic<bool>> m_freeFibers;
            std::vector<std::atomic<bool>> m_waitingFibers;

            // Local storage for fibers and fibers
            std::vector<ThreadLocalStorage> m_threadLocalStorage;
            std::vector<FiberLocalStorage> m_fiberLocalStorage;

            // Id mapping from OS to JobSystem
            std::unordered_map<uint32_t, uint32_t> m_threadIdMap;
            std::unordered_map<FiberPtr, uint32_t> m_fiberIdMap;
        };
    }
}
#include "JobSystem.h"

#include "DataStructures/MultithreadQueue.h"

#include <assert.h>
#include <cstdint>
#include <iostream>

namespace Farlor
{
    namespace FarlorJobs
    {
        JobSystem::JobSystem(uint32_t numFibers, uint32_t maxNumThreads)
            : m_jobQueue{}
            , m_numFibers{ numFibers }
            , m_quitting{ false }
            , m_threads {}
            , m_threadIdMap{}
            , m_fiberIdMap{}
            , m_threadLocalStorage{}
            , m_fiberLocalStorage{}
            , m_fibers( m_numFibers )
            , m_freeFibers( m_numFibers )
            , m_waitingFibers( m_numFibers )
        {
            m_numHwThreads = std::thread::hardware_concurrency();
            // Cap the number of threads to a given max number
            if (m_numHwThreads > maxNumThreads && maxNumThreads != 0)
            {
                m_numHwThreads = maxNumThreads;
            }

            // We want total number of fibers to be desired + num hw threads
            for (uint32_t i = 0; i < m_numFibers; ++i)
            {
                m_freeFibers[i].store(false);
                m_waitingFibers[i].store(false);
            }

            for (uint32_t i = 0; i < m_numHwThreads; ++i)
            {
                m_threads.emplace_back();
                m_threadLocalStorage.emplace_back();
            }

            for (uint32_t i = 0; i < m_numFibers; ++i)
            {
                m_fiberLocalStorage.emplace_back();
            }
        }

        uint32_t JobSystem::GetCurrentJobSystemThreadIndex()
        {
            // Calls os specific call to get the current thread id, which hashes into the map to get the job system index
            return m_threadIdMap[GetCurrentThreadId()];
        }

        uint32_t JobSystem::GetCurrentJobSystemFiberIndex()
        {
            // Calls os specific call to get the current fiber id, which hashes into the map to get the job system index
            return m_fiberIdMap[GetCurrentFiber()];
        }

        // This sets up all the job threads
        void JobSystem::BootstrapMainTask(JobFunction mainTask, void* pMainTaskArg)
        {
            // Structures to ensure that the values are destroyed at some point
            std::vector<std::unique_ptr<MainFiberFuncArg>> mainFiberFuncArgs;
            std::vector<std::unique_ptr<FiberFuncArg>> fiberFuncArgs;
            std::vector<std::unique_ptr<ThreadFuncArg>> threadFuncArgs;

            // First, we want to create the main fiber thread and bootstrap to the main thread
            // Create a counter for the main job
            std::shared_ptr<Counter> spCounter = std::make_shared<Counter>();
            spCounter->m_atomicCounter.store(0);
            spCounter->m_desiredValue = 1;

            // We bootstrap the main job with this
            Job mainJob;
            mainJob.m_jobFunction = mainTask;
            mainJob.m_jobArgument = pMainTaskArg;
            mainJob.m_spCounter = spCounter;

            std::unique_ptr<MainFiberFuncArg> upMainFiberFuncArg = std::make_unique<MainFiberFuncArg>();
            upMainFiberFuncArg->pJobSystem = this;
            upMainFiberFuncArg->bootstrapJob = mainJob;
            
            // Fiber creation
            {
                const uint32_t fiberStackSize = 512 * 1024;
                // Only the first fiber in the array is special
                FiberPtr mainThreadFiber = CreateFiber(fiberStackSize, &JobSystem::MainFiberFunc, upMainFiberFuncArg.get());
                mainFiberFuncArgs.push_back(std::move(upMainFiberFuncArg));
                if (!mainThreadFiber)
                {
                    // TODO: Log cause we have hell to pay
                    // Blow up
                    assert(false);
                }

                // The first fiber is the main thread fiber
                // Note: Always one more than the actual array index we want when using arrays
                uint32_t jobSystemFiberIndex = 1;
                uint32_t jobSystemFiberArrayIndex = jobSystemFiberIndex - 1;

                m_fibers[jobSystemFiberArrayIndex].m_pFiber = mainThreadFiber;
                // Require that its run on the creation thread
                m_fibers[jobSystemFiberArrayIndex].m_jobSystemId = jobSystemFiberIndex;
                // NOTE: Can this start as false to ensure its avalible when we need it
                m_freeFibers[jobSystemFiberArrayIndex].store(false);
                m_waitingFibers[jobSystemFiberArrayIndex].store(false);
                m_fiberIdMap.insert(std::make_pair(mainThreadFiber, jobSystemFiberIndex));
                // Increment to the next fiber index
                ++jobSystemFiberIndex;
                jobSystemFiberArrayIndex = jobSystemFiberIndex - 1;

                // Created all the fibers we need
                for (uint32_t i = 1; i < m_numFibers; ++i)
                {
                    std::unique_ptr<FiberFuncArg> upFiberFuncArg = std::make_unique<FiberFuncArg>();
                    upFiberFuncArg->pJobSystem = this;
                    FiberPtr newOsFiber = CreateFiber(fiberStackSize, &JobSystem::FiberFunc, upFiberFuncArg.get());
                    fiberFuncArgs.push_back(std::move(upFiberFuncArg));
                    if (!newOsFiber)
                    {
                        // Log, we have hell to pay
                        assert(false);
                        continue;
                    }

                    m_fibers[jobSystemFiberArrayIndex].m_pFiber = newOsFiber;
                    m_fibers[jobSystemFiberArrayIndex].m_jobSystemId = jobSystemFiberIndex;
                    m_fibers[jobSystemFiberArrayIndex].m_pJob = nullptr;
                    // We want this to start in a free state
                    m_freeFibers[jobSystemFiberArrayIndex].store(true);
                    m_waitingFibers[jobSystemFiberArrayIndex].store(false);
                    m_fiberIdMap.insert(std::make_pair(newOsFiber, jobSystemFiberIndex));

                    ++jobSystemFiberIndex;
                    jobSystemFiberArrayIndex = jobSystemFiberIndex - 1;
                }
            }

            // Creation of threads
            {
                uint32_t jobSystemThreadId = 1;
                uint32_t jobSystemThreadArrayId = jobSystemThreadId - 1;
                Thread mainThread;
                mainThread.m_thread = GetCurrentThread();
                mainThread.m_jobSystemId = jobSystemThreadId;
                mainThread.m_osThreadId = GetCurrentThreadId();
                m_threads[jobSystemThreadArrayId] = mainThread;
                m_threadIdMap.insert(std::make_pair(mainThread.m_osThreadId, mainThread.m_jobSystemId));

                ++jobSystemThreadId;
                jobSystemThreadArrayId = jobSystemThreadId - 1;

                for (uint32_t i = 1; i < m_numHwThreads; ++i)
                {
                    std::unique_ptr<ThreadFuncArg> upThreadFuncArg = std::make_unique<ThreadFuncArg>();
                    upThreadFuncArg->pJobSystem = this;

                    Thread workerThread;
                    workerThread.m_thread = CreateThread(0, 0, FarlorJobs::JobSystem::ThreadFunc, upThreadFuncArg.get(), 0, nullptr);
                    workerThread.m_osThreadId = GetThreadId(workerThread.m_thread);
                    workerThread.m_jobSystemId = jobSystemThreadId;
                    threadFuncArgs.push_back(std::move(upThreadFuncArg));
                    m_threads[jobSystemThreadArrayId] = workerThread;
                    m_threadIdMap.insert(std::make_pair(workerThread.m_osThreadId, workerThread.m_jobSystemId));

                    if (workerThread.m_thread == 0)
                    {
                        std::cout << "Failed to create thread: " << i << std::endl;
                    }

                    ++jobSystemThreadId;
                    jobSystemThreadArrayId = jobSystemThreadId - 1;
                }
            }

            // Main thread must convert to a thread
            FiberPtr mainThreadFiber = ConvertThreadToFiber(0);
            if (mainThreadFiber == 0)
            {
                std::cout << "Error, could not convert thread to fiber" << std::endl;
                assert(false);
            }

            m_threadLocalStorage[0].m_threadFiber = mainThreadFiber;

            // Once we do this, we want to switch to the main thread
            SwitchToFiber(m_fibers[0].m_pFiber);

            // Once we return from this, we want to switch back to a thread and finish out
            if (!ConvertFiberToThread())
            {
                std::cout << "Error, could not convert main fiber back to thread" << std::endl;
                assert(false);
            }

            // Finally, wait for all the threads that arent the main thread
            const uint32_t childThreadStartIndex = 1;
            for (uint32_t i = childThreadStartIndex; i < m_numHwThreads; ++i)
            {
                WaitForSingleObject(m_threads[i].m_thread, INFINITE);
            }

            // Finally, we can return as all threads are shutdown at this point
        }

        // This is the main fiber job
        // This is only for the fiber created from the thread that calls the bootstrap method on the job system
        void CALLBACK JobSystem::MainFiberFunc(void* pArg)
        {
            // NOTE: This fiber should start not being free
            MainFiberFuncArg* pFuncArg = static_cast<MainFiberFuncArg*>(pArg);
            JobSystem& jobSystem = *pFuncArg->pJobSystem;

            // Grab the main job and run
            Job& mainJob = pFuncArg->bootstrapJob;
            mainJob.m_jobFunction(mainJob.m_jobArgument);

            // We are done, quit the job system
            jobSystem.m_quitting.store(true);

            // We need to switch back to the main function
            const uint32_t mainThreadIndex = 0;
            SwitchToFiber(jobSystem.m_threadLocalStorage[mainThreadIndex].m_threadFiber);
        }

        // This is the loop that each fiber executes
        void CALLBACK JobSystem::FiberFunc(void* pArg)
        {
            const FiberFuncArg* pFuncArg = static_cast<FiberFuncArg*>(pArg);
            JobSystem& jobSystem = *pFuncArg->pJobSystem;

            // Until we are quitting, run this loop
            while (jobSystem.m_quitting.load() == false)
            {
                // We enter back here, so mark the previous fiber as waiting
                auto currentFiberIndex = jobSystem.GetCurrentJobSystemFiberIndex();
                uint32_t markWaitingIndex = jobSystem.m_fiberLocalStorage[currentFiberIndex - 1].m_markWaitingIndex;
                if (markWaitingIndex > 0)
                {
                    jobSystem.m_waitingFibers[markWaitingIndex - 1].store(true);
                    jobSystem.m_fiberLocalStorage[currentFiberIndex - 1].m_markWaitingIndex = 0;
                }

                // We want to see if we can switch to a waiting fiber
                bool switchToWaitFiber = false;
                FiberPtr waitFiberSwitch = nullptr;

                // We want to check if we should switch to a waiting thread
                // Iterate over all waiting threads
                for (uint32_t i = 0; i < jobSystem.m_numFibers; ++i)
                {
                    // We want to check that waiting thread is set to true
                    bool expected = true;

                    // Get current thread id
                    uint32_t currentJobSystemThreadId = jobSystem.GetCurrentJobSystemThreadIndex();
                    if (jobSystem.m_waitingFibers[i].compare_exchange_weak(expected, false))
                    {
                        uint32_t jobSystemFiberId = jobSystem.m_fiberIdMap[jobSystem.m_fibers[i].m_pFiber];

                        if (jobSystem.m_fiberLocalStorage[jobSystemFiberId - 1].m_spWaitingCounter->m_atomicCounter.load() == jobSystem.m_fiberLocalStorage[jobSystemFiberId - 1].m_spWaitingCounter->m_desiredValue)
                        {
                            waitFiberSwitch = jobSystem.m_fibers[i].m_pFiber;
                            switchToWaitFiber = true;
                            break;
                        }
                        else
                        {
                            jobSystem.m_waitingFibers[i].store(true);
                            continue;
                        }
                    }
                }

                // If we found a fiber to switch to, we want to do that switch
                if (switchToWaitFiber)
                {
                    uint32_t waitFiberId = jobSystem.m_fiberIdMap[waitFiberSwitch];
                    auto currentFiberJobIndex = jobSystem.GetCurrentJobSystemFiberIndex();
                    jobSystem.m_fiberLocalStorage[waitFiberId - 1].m_markFreeIndex = currentFiberJobIndex;

                    SwitchToFiber(waitFiberSwitch);
                    // Coming back from this, we are now a free fiber again
                    continue;
                }


                // If we are not running a waiting thread, lets go ahead and grab a job off the stack
                // Currently, this does not support checking if a job can execute on the current thread
                Job job;
                if (jobSystem.m_jobQueue.TryPop(job))
                {
                    // We might need to have some logic here for thread afinity for jobs...?
                    // TODO: If the job cannot be run on this thread, throw it back in the job queue
                    // For now, we have nothing.

                    uint32_t fiberId = jobSystem.GetCurrentJobSystemFiberIndex();
                    job.m_jobFunction(job.m_jobArgument);
                    job.m_spCounter->m_atomicCounter.fetch_add(1);
                }
            }

            // Now we are done with the game
            // Switch to the main thread fiber for shutdown of thread
            uint32_t currentTheadIndex = jobSystem.GetCurrentJobSystemThreadIndex();
            SwitchToFiber(jobSystem.m_threadLocalStorage[currentTheadIndex - 1].m_threadFiber);
        }

        // This is the main function run by each thread
        DWORD WINAPI JobSystem::ThreadFunc(void* pArg)
        {
            ThreadFuncArg* pThreadMainArg = static_cast<ThreadFuncArg*>(pArg);
            JobSystem& jobSystem = *pThreadMainArg->pJobSystem;

            auto currentThreadId = GetCurrentThreadId();
            auto currentJobSystemThreadId = jobSystem.m_threadIdMap[currentThreadId];

            // Threads must be converted to fibers
            FiberPtr fiberHandle = ConvertThreadToFiber(0);
            if (fiberHandle == 0)
            {
                std::cout << "Error, could not convert thread to fiber" << std::endl;
                assert(false);
            }

            // We want to store the fiber in thread local storage
            // It is important for the main fiber to be saved so the thread goes back to the same fiber it started as
            jobSystem.m_threadLocalStorage[currentJobSystemThreadId - 1].m_threadFiber = fiberHandle;


            // Now, we need to get a new free fiber to jump to next. It will simply start running jobs and will return to this point when finished
            FiberPtr fiberToStartOn = 0;
            for (uint32_t i = 0; i < jobSystem.m_numFibers; ++i)
            {
                bool expected = true;
                if (jobSystem.m_freeFibers[i].compare_exchange_weak(expected, false))
                {
                    fiberToStartOn = jobSystem.m_fibers[i].m_pFiber;
                    break;
                }
            }
            
            SwitchToFiber(fiberToStartOn);


            if (!ConvertFiberToThread())
            {
                std::cout << "Failed to convert fiber back to thread" << std::endl;
            }

            return 0;
        }

        std::shared_ptr<JobSystem::Counter> JobSystem::SubmitJobs(Job* pJobs, uint32_t numJobs)
        {
            // Get a new counter
            std::shared_ptr<Counter> spCounter = std::make_shared<Counter>();
            spCounter->m_atomicCounter.store(0);
            spCounter->m_desiredValue = numJobs;

            for (uint32_t i = 0; i < numJobs; ++i)
            {
                // We copy the job into the queue
                Job job = pJobs[i];
                job.m_spCounter = spCounter;
                m_jobQueue.Push(job);
            }

            return spCounter;
        }

        // Calling thus function waits on a counter until it is equal to a correct value.
        // This is used to yield a thread from a job and instaead switch to a free fiber.
        void JobSystem::Wait(std::shared_ptr<Counter> spCounter)
        {
            // Wait for counter to be correct value
            // If we are done, simply return
            if (spCounter->m_atomicCounter.load() == spCounter->m_desiredValue)
            {
                return;
            }

            // We need to wait, so find a free fiber and switch to it
            FiberPtr currentFiber = GetCurrentFiber();
            uint32_t jobSystemFiberId = m_fiberIdMap[currentFiber];

            // Get and switch to local fiber
            FiberPtr freeFiber = nullptr;
            uint32_t freeFiberIndex = 0;
            for (uint32_t i = 0; i < m_numFibers; ++i)
            {
                bool expected = true;
                if (m_freeFibers[i].compare_exchange_weak(expected, false))
                {
                    freeFiber = m_fibers[i].m_pFiber;
                    freeFiberIndex = i + 1;
                    break;
                }
            }

            if (freeFiber == nullptr)
            {
                std::cout << "We are out of freaking fibers" << std::endl;
                assert(false);
            }

            m_fiberLocalStorage[freeFiberIndex - 1].m_markWaitingIndex = jobSystemFiberId;
            m_fiberLocalStorage[jobSystemFiberId - 1].m_spWaitingCounter = spCounter;

            // TODO: This might just be a race condition... we might want to do something to mitigate this... I am unsure.
            // Could we defer this...? Maybe on a unique fiber that is in charge of performing these kinds of updates. Or thread?
            // This must be marked or it will never be restarted.
            SwitchToFiber(freeFiber);

            // We want to mark the fiber we came from as a free thread again
            auto currentFiberIndex = GetCurrentJobSystemFiberIndex();
            uint32_t markFreeIndex = m_fiberLocalStorage[currentFiberIndex - 1].m_markFreeIndex;
            if (markFreeIndex > 0)
            {
                m_freeFibers[markFreeIndex - 1].store(true);
                m_fiberLocalStorage[currentFiberIndex - 1].m_markFreeIndex = 0;
            }

            m_fiberLocalStorage[currentFiberIndex - 1].m_spWaitingCounter = nullptr;
        }
    }
}
#include "stdafx.h"

#include "JobScheduler.h"
#include "MPMCQueue.h"


struct WorkerContext
{
     mpmc_bounded_queue<Job>* m_queue;
     volatile bool m_exit;
};


unsigned __stdcall worker_thread_func(void* a_context)
{
    WorkerContext* context = (WorkerContext*)a_context;

    Job job;
    while (!context->m_exit)
    {
        if(context->m_queue->dequeue(job))
        {
            job.m_function(job.m_data);
        }
        else
        {
            SwitchToThread();
        }
    }

    return 0;
}


JobScheduler::JobScheduler(int a_numThreads, int a_maxJobsInQueue)
    : m_numThreads(a_numThreads)
    , m_jobQueue(new mpmc_bounded_queue<Job>(a_maxJobsInQueue))
    , m_threads(new void*[a_numThreads])
    , m_contexts(new WorkerContext[a_numThreads])
{
    for (int i = 0; i < m_numThreads; ++i)
    {
        m_contexts[i].m_exit = false;
        m_contexts[i].m_queue = m_jobQueue;
        m_threads[i] = (HANDLE)_beginthreadex(0, 0, worker_thread_func, &m_contexts[i], 0, 0);
    }
}


JobScheduler::~JobScheduler()
{
    for (int i = 0; i < m_numThreads; ++i)
    {
        m_contexts[i].m_exit = true;
    }
}


bool JobScheduler::SubmitJob(const Job& a_job)
{
    return m_jobQueue->enqueue(a_job);
}


void JobScheduler::YieldCurrentThread()
{
    SwitchToThread();
}
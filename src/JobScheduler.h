#ifndef _JobScheduler_h_
#define _JobScheduler_h_

#include"Scoped.h"
#include"ScopedArray.h"


struct WorkerContext;

template<class T>
class mpmc_bounded_queue;


struct Job
{
    void* m_data;
    void (*m_function)(void*);
};


// A simple scheduler that creates a set of worker threads.
// Jobs are submitted to a queue (can be from multiple threads).
// The threads do the work in the queue but stay busy even when there 
// is no work to do, so an improvement would be to add signals to wake
// up threads.
class JobScheduler
{
public:
    // Creates a_numThreads worker threads. a_maxJobsInQueue must be a power of 2.
    JobScheduler(int a_numThreads, int a_maxJobsInQueue);

    // Tells worker threads to exit once they have finished the current job
    ~JobScheduler();

    // Multiple threads may call this
    bool SubmitJob(const Job& a_job);

    inline int NumThreads() const { return m_numThreads; }

    // A utiltiy for idle threads to call while waiting on jobs.
    static void YieldCurrentThread();

private:

    int m_numThreads;
    Scoped<mpmc_bounded_queue<Job> > m_jobQueue;
    ScopedArray<void*> m_threads;
    ScopedArray<WorkerContext> m_contexts;
};

#endif



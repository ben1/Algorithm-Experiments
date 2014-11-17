#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>


struct Job
{
    void* m_data;
    void (*m_function)(void*);
};


// A simple scheduler that creates a set of worker threads.
// Jobs are submitted to a queue (can be from multiple threads).
// The threads do the work in the queue.
class JobQueue
{
public:
    // Creates a_numThreads worker threads.
    JobQueue(int a_numThreads);

    // Tells worker threads to exit once they have finished the current job
    ~JobQueue();

    // Multiple threads may call this
    void SubmitJob(const Job& a_job);

    inline int NumThreads() const { return m_threads.size(); }

private:

	int WorkerFunction();

	std::mutex m_mutex;
	std::condition_variable m_condition;
	std::queue<Job> m_queue;
	std::vector<std::thread> m_threads;
	volatile bool m_exit;
};




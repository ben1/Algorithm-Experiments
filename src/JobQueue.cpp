#include "stdafx.h"

#include "JobQueue.h"


JobQueue::JobQueue(int a_numThreads)
	: m_exit(false)
{
	// start all the threads
	m_threads.resize(a_numThreads);
	for (uint32_t i = 0; i < m_threads.size(); ++i)
    {
		m_threads[i] = std::thread(&JobQueue::WorkerFunction, this);
    }
}


JobQueue::~JobQueue()
{
	// We don't allow jobs to be started once the queue is being destroyed.
	// Allow the threads to exit when current jobs in progress are finished.
	m_exit = true;

	// notify all threads that they can wake up
	m_condition.notify_all();

	// wait for all threads to exit
	for (uint32_t i = 0; i < m_threads.size(); ++i)
	{
		m_threads[i].join();
	}
}


void JobQueue::SubmitJob(const Job& a_job)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue.push(a_job);
	m_condition.notify_one();
}


int JobQueue::WorkerFunction()
{
	// keep the lock while testing the queue and not waiting
	std::unique_lock<std::mutex> lock(m_mutex);

	while (!m_exit)
	{
		if (m_queue.size() > 0)
		{
			// pop the next job
			Job job = m_queue.front();
			m_queue.pop();

			// run the job without a lock on the queue
			lock.unlock();
			job.m_function(job.m_data);
			lock.lock();
		}
		else
		{
			// Wait (releases the lock) until signalled, or spuriously woken (either is OK, because we will simply wait again if there is no work).
			m_condition.wait(lock);
		}
	}

	return 0;
}



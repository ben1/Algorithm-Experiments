#include <condition_variable>
#include <mutex>


class CountdownLatch
{
public:

	// any threads that called Wait() will be woken after a_count Notify() calls.
	CountdownLatch(int a_count)
		: m_count(a_count)
	{
	}

	// releases any threads waiting, and resets the count 
	void Reset(int a_count)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condition.notify_all();
		m_count = a_count;
	}

	void Notify()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_count--;
		if (m_count == 0)
		{
			m_condition.notify_all();
		}
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_count > 0)
		{
			m_condition.wait(lock);
		}
	}

private:
	int m_count;
	std::condition_variable m_condition;
	std::mutex m_mutex;
};
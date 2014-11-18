#include <condition_variable>
#include <mutex>

// Any threads that called Wait(a_count) will be woken after a_count Notify() calls.
class CountLatch
{
public:

	CountLatch()
		: m_count(0)
	{
	}

	// releases any threads waiting, and resets the count of notifications
	void Reset()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_condition.notify_all();
		m_count = 0;
	}

	void Notify()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_count++;
		m_condition.notify_all();
	}

	void Wait(int a_count)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_count < a_count)
		{
			m_condition.wait(lock);
		}
	}

private:
	int m_count;
	std::condition_variable m_condition;
	std::mutex m_mutex;
};
#pragma once
template<class T, int MAX_FREE_ITEMS>
class MemoryChain
{
public:
	MemoryChain()
		: m_nextFree(0)
		, m_numFree(0)
	{
	}
	~MemoryChain()
	{
		DeleteFree();
	}

private:
	struct Item
	{
		Item* m_nextFree;
	};

public:

	inline T* Allocate()
	{
		if (m_nextFree)
		{
			m_numFree--;
			Item* i = m_nextFree;
			m_nextFree = m_nextFree->m_nextFree;
			return (T*)i;
		}
		else
		{
			return (T*) new char[sizeof(T)];
		}
	}

	inline void Free(T* a_ptr)
	{
		if (m_numFree < MAX_FREE_ITEMS)
		{
			m_numFree++;
			Item* i = (Item*)a_ptr;
			i->m_nextFree = m_nextFree;
			m_nextFree = i;
		}
		else
		{
			delete[] (char*)a_ptr;
		}
	}

	void DeleteFree()
	{
		while (m_nextFree)
		{
			char* i = (char*)m_nextFree;
			m_nextFree = m_nextFree->m_nextFree;
			delete[] i;
		}
		m_numFree = 0;
	}

private:

	Item* m_nextFree;
	uint32_t m_numFree;
};


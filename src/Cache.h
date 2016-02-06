#pragma once

#include <unordered_map>
#include <list>


template<class KEY, class VALUE>
class Cache
{
public:

	Cache(std::size_t a_maxItems)
		: m_maxItems(a_maxItems)
	{
	}

	inline bool get(const KEY& a_key, VALUE& a_value)
	{
		auto i = m_cache.find(a_key);
		if (i == m_cache.end())
		{
			// not in cache
			return false;
		}

		// make it the most recently used
		m_lru.erase(i->second.m_iter);
		m_lru.push_front(a_key);
		i->second.m_iter = m_lru.begin();

		// copy the value and indicate that we found it
		a_value = i->second.m_value;
		return true;
	}

	inline void put(const KEY& a_key, const VALUE& a_value)
	{
		auto i = m_cache.find(a_key);
		if (i == m_cache.end())
		{
			// inserting a new item, so reduce the size to one less than the limit
			while (m_cache.size() >= m_maxItems)
			{
				m_cache.erase(m_lru.back());
				m_lru.pop_back();
			}

			// add the new item to the cache and make it the most recently used
			m_lru.push_front(a_key);
			Item item;
			item.m_value = a_value;
			item.m_iter = m_lru.begin();
			m_cache[a_key] = item;
		}
		else
		{
			// updating an existing item, so update the value and make it the most recently used
			m_lru.erase(i->second.m_iter);
			m_lru.push_front(a_key);
			i->second.m_iter = m_lru.begin();
			i->second.m_value = a_value;
		}
	}

	inline const size_t size() const
	{
		return m_cache.size();
	}

private:

	struct Item
	{
		VALUE m_value;
		typename std::list<KEY>::iterator m_iter;
	};

	std::size_t m_maxItems;
	std::list<KEY> m_lru;
	std::unordered_map<KEY, Item> m_cache;
};


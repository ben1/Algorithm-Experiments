#pragma once

#if 0

template<class T, int ITEMS_PER_BLOCK>
class MemoryPoolChain
{
private:
	union Item
	{
		T m_data;
		Item* m_nextFree;
	};
public:
	MemoryPoolChain();
	~MemoryPoolChain();

	T* Allocate()
	{
		if (m_nextFreeBlock)
		{
			MemoryPoolBlock* block = m_nextFreeBlock;
			block->m_usedCount++;
			if (block->m_usedCount == ITEMS_PER_BLOCK)
			{
				// unhook the block because it is all used.
			}
			// unhook the item
			return m_nextFreeBlock->m_nextFreeItem.m_data;
		}
		else
		{
			// allocate a new block
			// hook the block and all the other items
			// NOTE: this is the slowest operation and needs to be improved.
			// TODO: add a block member called int m_nextUnusedIndex which can also be used to get free items without having to hook them all up
		}
	}
	void Free(T* a_ptr)
	{
		MemoryPoolBlock* block = a_ptr->GetBlock();
		block->m_usedCount--;
		if (block->m_usedCount == 0)
		{
			// unhook the block, and delete it

		}
		else
		{
			if (block->m_usedCount == ITEMS_PER_BLOCK - 1)
			{
				// hook the block because now it has a free item.
			}
			// hook up the item that was freed
		}
	}

	float CalculatePercentageUsed();

	class Tag
	{
	public:
		inline Tag(uint8_t a_index)
		{
			m_index = a_index;
		}
		inline MemoryPoolBlock* GetBlock(T* self)
		{
			return (MemoryPoolBlock*)(self - m_index);
		}
	private:
		uint8_t m_index;
	};

private:

	class MemoryPoolBlock
	{
	public:
		MemoryPoolBlock();
		~MemoryPoolBlock();
	private:
		Item m_data[ITEMS_PER_BLOCK];

		int m_usedCount;
		MemoryPoolBlock* m_prevBlock;
		MemoryPoolBlock* m_nextBlock;
		Item* m_nextFreeItem;
	};

	MemoryPoolBlock* m_nextFreeBlock;
};

#endif
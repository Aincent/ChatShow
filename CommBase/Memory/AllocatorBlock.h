/*
 * AllocatorBlock.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef MEMORY_ALLOCATORBLOCK_H_
#define MEMORY_ALLOCATORBLOCK_H_

#include "../Common.h"

namespace CommBaseOut
{

class AllocatorBlock
{
	struct SBlock
	{
		SBlock *next_;

		SBlock()
		{
			next_ = 0;
		}

		SBlock(SBlock *next) : next_(next)
		{

		}
	};

	struct SMemory_Block
	{
		char *memory_block_;
		SMemory_Block *next_;

		SMemory_Block() : memory_block_(0),next_(0){}
	};

public:
	AllocatorBlock(size_t block_size,size_t block_number = 32);

	~AllocatorBlock(void);

	size_t  get_allocator_size(void)
	{
		int memory_block_number = 0;
		for (SMemory_Block* block = this->memory_list_.next_;block != 0;block = block->next_)
		{
			++memory_block_number;
		}
		return  memory_block_number * this->block_number_ * this->block_size_;
	}

	void* malloc(size_t sz);

	void* malloc(size_t sz,size_t &r_sz);

	void free(void *p,size_t sz);

	void* malloc();

	void free(void *p);

private:
	void allocat_size();

public:
	size_t block_size_;
	size_t block_number_;

private:
	SBlock free_;
	CSimLock lock_;
	SMemory_Block memory_list_;
};

}

#endif /* MEMORY_ALLOCATORBLOCK_H_ */

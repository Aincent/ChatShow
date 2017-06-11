/*
 * AllocatorBlock.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */
#include "AllocatorBlock.h"
#include <new>

namespace CommBaseOut
{

AllocatorBlock::AllocatorBlock(size_t block_size,size_t block_number)
{
	block_size_ = block_size > sizeof(void*) ? block_size : sizeof(void*);
	block_size_ += 1;
	block_number_ = block_number;
	allocat_size();
}

AllocatorBlock::~AllocatorBlock(void)
{
	while(memory_list_.next_)
	{
		SMemory_Block * block = memory_list_.next_;

		memory_list_.next_ = block->next_;
		delete[] block->memory_block_;
		delete block;
	}
}


void* AllocatorBlock::malloc(size_t sz)
{
	if(sz > block_size_)
		return 0;
	else
		return malloc();
}

void* AllocatorBlock::malloc(size_t sz,size_t &r_sz)
{
	if(sz > block_size_)
	{
		r_sz = 0;

		return NULL;
	}
	else
	{
		r_sz = block_size_;

		return malloc();
	}
}

void AllocatorBlock::free(void *p,size_t sz)
{
	free(p);
}

void* AllocatorBlock::malloc()
{
	GUARD(CSimLock, obj, &lock_);

	if(free_.next_)
	{
		SBlock *result = free_.next_;

		free_.next_ = free_.next_->next_;

		return result;
	}
	else
	{
		allocat_size();
		if(free_.next_)
		{
			SBlock *result = NULL;

			result = free_.next_;
			free_.next_ = result->next_;

			return result;
		}
		else
		{
			return 0;
		}
	}
}

void AllocatorBlock::free(void *p)
{
	GUARD(CSimLock, obj, &lock_);

	*(SBlock*)p = free_.next_;
	free_.next_ =(SBlock*)p;
}

void AllocatorBlock::allocat_size()
{
	size_t total = block_size_ * block_number_;
	char* buffer = new(std::nothrow)char[total];
	if(!buffer)
		return;


	SMemory_Block *new_block = new(std::nothrow) SMemory_Block;
	if(!new_block)
	{
		delete[] buffer;
		return;
	}

	new_block->next_ = memory_list_.next_;
	new_block->memory_block_ = buffer;
	memory_list_.next_ = new_block;

	SBlock *header = new(buffer)SBlock;
	SBlock *block = 0;

	header->next_ = 0;
	buffer += block_size_;
	for(size_t i = 1 ; i < block_number_ ; ++i)
	{
		block = (SBlock*)buffer;
		buffer += block_size_;
		block->next_ = header;
		header = block;
	}
	free_.next_ = header;
}


}


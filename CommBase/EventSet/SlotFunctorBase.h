/*
 * SlotFunctorBase.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_SLOTFUNCTORBASE_H_
#define EVENTSET_SLOTFUNCTORBASE_H_

#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{
	class EventArgs;

	class SlotFunctorBase
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
	{
	public:
		virtual ~SlotFunctorBase() {};
		virtual bool operator()(const EventArgs& args) = 0;
	};
}

#endif /* EVENTSET_SLOTFUNCTORBASE_H_ */

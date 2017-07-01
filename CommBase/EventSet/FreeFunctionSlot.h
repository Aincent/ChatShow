/*
 * FreeFunctionSlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_FREEFUNCTIONSLOT_H_
#define EVENTSET_FREEFUNCTIONSLOT_H_

#include "SlotFunctorBase.h"

namespace CommBaseOut
{
	class FreeFunctionSlot : public SlotFunctorBase
	{
	public:
		typedef bool (SlotFunction)(const EventArgs&);

		FreeFunctionSlot(SlotFunction* func) :
		d_function(func)
		{}

		virtual bool operator()(const EventArgs& args)
		{
			return d_function(args);
		}

	private:
		SlotFunction* d_function;
	};
}

#endif /* EVENTSET_FREEFUNCTIONSLOT_H_ */

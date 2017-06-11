/*
 * MemberFunctionSlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_MEMBERFUNCTIONSLOT_H_
#define EVENTSET_MEMBERFUNCTIONSLOT_H_

#include "SlotFunctorBase.h"

namespace CommBaseOut
{
	template<typename T>
	class MemberFunctionSlot : public SlotFunctorBase
	{
	public:
		typedef bool(T::*MemberFunctionType)(const EventArgs&);

		MemberFunctionSlot(MemberFunctionType func, T* obj) :
		d_function(func),
			d_object(obj)
		{}

		virtual bool operator()(const EventArgs& args)
		{
			return (d_object->*d_function)(args);
		}

	private:
		MemberFunctionType d_function;
		T* d_object;
	};
}

#endif /* EVENTSET_MEMBERFUNCTIONSLOT_H_ */

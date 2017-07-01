/*
 * FunctorPointSlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_FUNCTORPOINTSLOT_H_
#define EVENTSET_FUNCTORPOINTSLOT_H_

#include "SlotFunctorBase.h"

namespace CommBaseOut
{
	template<typename T>
	class FunctorPointerSlot : public SlotFunctorBase
	{
	public:
		FunctorPointerSlot(T* functor) :
		  d_functor(functor)
		  {}

		  virtual bool operator()(const EventArgs& args)
		  {
			  return (*d_functor)(args);
		  }

	private:
		T* d_functor;
	};
}

#endif /* EVENTSET_FUNCTORPOINTSLOT_H_ */

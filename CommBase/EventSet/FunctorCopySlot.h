/*
 * FunctorCopySlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_FUNCTORCOPYSLOT_H_
#define EVENTSET_FUNCTORCOPYSLOT_H_

#include "SlotFunctorBase.h"

namespace CommBaseOut
{
	template<typename T>
	class FunctorCopySlot : public SlotFunctorBase
	{
	public:
		FunctorCopySlot(const T& functor) :
		  d_functor(functor)
		  {}

		  virtual bool operator()(const EventArgs& args)
		  {
			  return d_functor(args);
		  }

	private:
		T d_functor;
	};
}

#endif /* EVENTSET_FUNCTORCOPYSLOT_H_ */

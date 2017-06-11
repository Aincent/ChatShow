/*
 * FunctorReferenceSlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_FUNCTORREFERENCESLOT_H_
#define EVENTSET_FUNCTORREFERENCESLOT_H_

#include "SlotFunctorBase.h"

namespace CommBaseOut
{
	template<typename T>
	class FunctorReferenceSlot : public SlotFunctorBase
	{
	public:
		FunctorReferenceSlot(T& functor) :
		  d_functor(functor)
		  {}

		  virtual bool operator()(const EventArgs& args)
		  {
			  return d_functor(args);
		  }

	private:
		T& d_functor;
	};
}

#endif /* EVENTSET_FUNCTORREFERENCESLOT_H_ */

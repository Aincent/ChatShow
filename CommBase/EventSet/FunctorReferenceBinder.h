/*
 * FunctorReferenceBinder.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_FUNCTORREFERENCEBINDER_H_
#define EVENTSET_FUNCTORREFERENCEBINDER_H_

namespace CommBaseOut
{
	template<typename T>
	struct FunctorReferenceBinder
	{
		FunctorReferenceBinder(T& functor) :
	d_functor(functor)
	{}

	T& d_functor;
	};
}


#endif /* EVENTSET_FUNCTORREFERENCEBINDER_H_ */

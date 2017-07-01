/*
 * SubscriberSlot.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_SUBSCRIBERSLOT_H_
#define EVENTSET_SUBSCRIBERSLOT_H_

#include "FreeFunctionSlot.h"
#include "FunctorPointSlot.h"
#include "FunctorCopySlot.h"
#include "FunctorReferenceBinder.h"
#include "FunctorReferenceSlot.h"
#include "MemberFunctionSlot.h"
#include "../Memory/MemAllocator.h"


namespace CommBaseOut
{
class SubscriberSlot
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:

	SubscriberSlot() : m_functorImpl(0)
	{
	}

	SubscriberSlot(FreeFunctionSlot::SlotFunction* func): m_functorImpl(NEW FreeFunctionSlot(func))
	{
	}

	 ~SubscriberSlot()
	{
	}


	bool operator()(const EventArgs& args) const
	{
		return (*m_functorImpl)(args);
	}

	bool Connected() const
	{
		return m_functorImpl != 0;
	}

	void Cleanup()
	{
		delete m_functorImpl;
		m_functorImpl = 0;
	}

	// 类的成员函数
	template<typename T>
		SubscriberSlot(bool (T::*function)(const EventArgs&), T* obj) : m_functorImpl(NEW MemberFunctionSlot<T>(function, obj))
	{

	}

	// 引用
	template<typename T>
		SubscriberSlot(const FunctorReferenceBinder<T>& binder) : m_functorImpl(NEW FunctorReferenceSlot<T>(binder.d_functor))
	{

	}

	// 复制
	template<typename T>
		SubscriberSlot(const T& functor) : m_functorImpl(NEW FunctorCopySlot<T>(functor))
	{

	}

	// 指针
	template<typename T>
		SubscriberSlot(T* functor) : m_functorImpl(NEW FunctorPointerSlot<T>(functor))
	{
	}

private:
	SlotFunctorBase* m_functorImpl;
};
}

#endif /* EVENTSET_SUBSCRIBERSLOT_H_ */

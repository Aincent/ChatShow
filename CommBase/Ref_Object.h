/*
 * Ref_Object.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef REF_OBJECT_H_
#define REF_OBJECT_H_

#include "Common.h"


namespace CommBaseOut
{

class Ref_Object
{
public:
	Ref_Object(void) : ref_count_(0)
	{
	}

	Ref_Object(const Ref_Object& val)
	{
		ref_count_ = 0;
	}
public:
	virtual ~Ref_Object(void)
	{
	}

	void addref(void)
	{
		GUARD(CSimLock, obj, &ref_lock_);
		++ref_count_;
	}

	bool release(void)
	{
		GUARD(CSimLock, obj, &ref_lock_);
		--ref_count_;

		if(ref_count_ <= 0)
		{
			obj.UnLock();
			delete this;
			return true;
		}

		return false;
	}

	Ref_Object& operator = (const Ref_Object& val)
	{
		if(this == &val)
			return *this;

		GUARD(CSimLock, obj, &ref_lock_);
		ref_count_ = 0;
		obj.UnLock();

		return *this;
	}
protected:
	volatile long ref_count_;
	CSimLock ref_lock_;
};


}

#endif /* REF_OBJECT_H_ */

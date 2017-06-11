/*
 * ClassBase.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef CLASSBASE_H_
#define CLASSBASE_H_

#include "Ref_Object.h"
#include "Memory/MemAllocator.h"

namespace CommBaseOut
{
class NullBase : public Ref_Object
{
public:
	virtual ~NullBase(){}
};

}

#endif /* CLASSBASE_H_ */

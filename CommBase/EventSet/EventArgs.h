/*
 * EventArgs.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef EVENTSET_EVENTARGS_H_
#define EVENTSET_EVENTARGS_H_

#include "../define.h"

namespace CommBaseOut
{

class EventArgs
{
public:
	friend class CEEvent;

	EventArgs(void) : handled(0), bfire(false) {}
	virtual ~EventArgs(void) {}

	DWORD handled;

	bool IsFire() const { return bfire; }
private:
	bool bfire;
};
}

#endif /* EVENTSET_EVENTARGS_H_ */

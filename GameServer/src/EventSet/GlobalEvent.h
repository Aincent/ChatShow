/*
 * GlobalEvent.h
 *
 *  Created on: Jul 22, 2017
 *      Author: root
 */

#ifndef GLOBALEVENT_H_
#define GLOBALEVENT_H_

#include "EventSet/EventSet.h"
#include "EventSet/SubscriberSlot.h"
#include "EventSet/EventArgs.h"
#include "Singleton.h"

using namespace CommBaseOut;

class GlobalEvent : public Singleton<GlobalEvent>, public EventSet
{
public:

};

#endif /* GLOBALEVENT_H_ */

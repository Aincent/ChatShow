/*
 * ServerEventDefine.h
 *
 *  Created on: Jul 27, 2017
 *      Author: root
 */

#ifndef SERVEREVENTDEFINE_H_
#define SERVEREVENTDEFINE_H_

#include "./EventSet/EventArgs.h"
#include "Smart_Ptr.h"
#include "CharDefine.h"
#include "Object/Player/Player.h"

using namespace CommBaseOut;

class PlayerArgs: public EventArgs
{
public:
	PlayerArgs(Smart_Ptr<Player> player):m_Player(player){}

	Smart_Ptr<Player> m_Player;
};

const int PLAYER_ONLINE		= 1000; //玩家上线
const int PLAYER_OFFLINE	= 1001; //玩家下线

#endif /* SERVEREVENTDEFINE_H_ */

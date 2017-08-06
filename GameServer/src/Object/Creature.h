/*
 * Creature.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef CREATURE_H_
#define CREATURE_H_

#include "Object.h"
#include "define.h"
#include "Smart_Ptr.h"
#include "EventSet/EventSet.h"
#include "StateMgr.h"

using namespace std;
using namespace CommBaseOut;

//--------------------------------------------------------------------------
// 角色状态标识 -- 该状态改变，需要向周围玩家广播
//--------------------------------------------------------------------------
enum ECreatureState
{

};

class CreatureObj : public Object, public EventSet
{
public:
	CreatureObj(eObjType type);
	virtual ~CreatureObj();

	void Release();

	int64 GetID()	{ return m_ID; }
	void SetID(int64 id)	{ m_ID = id; }

	string &GetName()	{ return m_name; }
	void SetName(const string &name)	{ m_name = name; }
public:
	typedef State<DWORD64, ECreatureState>				CreatureState;

private:
	//-------------------------------------------------------------------------------------
	// 玩家当前状态
	//-------------------------------------------------------------------------------------
	CreatureState			m_CreatureState;						// 玩家状态	-- 变化后，需通知周围玩家

	int64 m_ID;
	string m_name;
};

#endif /* CREATURE_H_ */

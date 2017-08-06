/*
 * Creature.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */
#include "Creature.h"
#include "SvrConfig.h"

CreatureObj::CreatureObj(eObjType type):Object(type),m_ID(-1)
{

}

CreatureObj::~CreatureObj()
{

}

void CreatureObj::Release()
{
	Object::Release();

	m_ID = -1;
	CreatureState		state;
	m_CreatureState = state;
}


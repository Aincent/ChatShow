/*
 * ObjectFactory.cpp

 *
 *  Created on: Jun 30, 2017
 *      Author: root
 */
#include "ObjectFactory.h"
#include "Log/Logger.h"

ObjectFactory * ObjectFactory::m_instance = 0;

ObjectFactory::ObjectFactory():m_onlyID(1)
{
	//GUARD(CSimLock, obj, &m_idLock);

	//CreateObiect(ePlayer, ePlayerInitCount);
	//CreateObiect(eMonster, eMonsterInitCount);
	//CreateObiect(ePet, ePetInitCount);
	//CreateObiect(eDropItem, eDropItemInitCount);
	//CreateObiect(eSpring, 500);
}


ObjectFactory::~ObjectFactory()
{

}

Smart_Ptr<Object> ObjectFactory::AllocObject(eObjType eObjecType)
{
	int key = 0;

	GUARD(CSimLock, obj, &m_idLock);
	key = m_onlyID++;
	if(m_onlyID <= 0)
	{
		m_onlyID = 1;
	}

	Smart_Ptr<Object>	object = GetObject(eObjecType);
	if(object.Get() != NULL)
	{
		object->SetKey(key);
	}

	return object;
}

void ObjectFactory::Recycle(Smart_Ptr<Object>& object)
{
	GUARD(CSimLock, obj, &m_idLock);

	switch(object->GetType())
	{
		case ePlayer:
		{
			m_players.push(object);
			break;
		}
		default:
			break;
	}
}

void ObjectFactory::CreateObiect(eObjType eObjecType, int size)
{
	switch(eObjecType)
	{
		case ePlayer:
		{
			for(int i=0; i<size; ++i)
			{
				Player* pObject = new Player(ePlayer);
				Smart_Ptr<Player> obj = pObject;
				m_players.push(obj);
			}
			break;
		}
		default:
		{
			LOG_ERROR(FILEINFO, "create unknown object");
		}
	}

}

Smart_Ptr<Object>	ObjectFactory::GetObject(eObjType eObjecType)
{
	switch(eObjecType)
	{
		case ePlayer:
		{
			//if(m_players.size() == 0)
			//{
			//	CreateObiect(ePlayer, ePlayerInitCount/2);
			//printf("\n 创建%d个对象ePlayer  \n", ePlayerInitCount/2 );
			//}

			//Smart_Ptr<Object> obj = m_players.front();
			//m_players.pop();

			Smart_Ptr<Object> obj = new Player(ePlayer);

			return obj;

			break;
		}
		default:
		{
			return NULL;
		}
	}

	return NULL;
}




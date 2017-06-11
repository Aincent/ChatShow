/*
 * LuaPool.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "LuaPool.h"
#include "../Log/Logger.h"


namespace CommBaseOut
{

LuaPool * LuaPool::m_instance = 0;

LuaPool::LuaPool():m_content(5),m_rate(3)
{

}

LuaPool::~LuaPool()
{
	m_pool.clear();
}


int LuaPool::Init(int content, int rate, string &filename)
{
	int res = 0;

	m_content = content;
	m_rate = rate;
	m_file = filename;

	res = Create(content);

	return res;
}

int LuaPool::Create(int count)
{
	for(int i=0; i<count; ++i)
	{
		try
		{
			Smart_Ptr<CLuaInterface> luaInter = NEW CLuaInterface();

			luaInter->OpenLua();
			if(luaInter->LoadFile(m_file) != 0 || luaInter->Call() != 0)
			{
				LOG_BASE(FILEINFO, "luapool loadfile[%s] error", m_file.c_str());

				return -1;
			}

			m_pool.push_back(luaInter);
		}
		catch(exception &e)
		{
			LOG_BASE(FILEINFO, "luapool loadfile[%s] error", e.what());

			return -1;
		}
		catch(...)
		{
			LOG_BASE(FILEINFO, "luapool loadfile error");

			return -1;
		}
	}

	return 0;
}

Smart_Ptr<CLuaInterface> LuaPool::GetLuaInterface()
{
	GUARD(CSimLock, obj, &m_poolLock);
	if(m_pool.size() == 0)
	{
		if(Create(m_rate) != 0)
		{
			LOG_BASE(FILEINFO, "LuaPool::GetLuaInterface add content error");

			return 0;
		}
	}

	Smart_Ptr<CLuaInterface> tmp = m_pool.front();

	m_pool.pop_front();

	return tmp;
}

void LuaPool::ReleaseLuaInterface(Smart_Ptr<CLuaInterface> & lua)
{
	GUARD(CSimLock, obj, &m_poolLock);
	m_pool.push_back(lua);
}

}

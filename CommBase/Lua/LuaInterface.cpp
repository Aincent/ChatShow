/*
 * LuaInterface.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "LuaInterface.h"
#include "../util.h"
#include "../Log/Logger.h"
namespace CommBaseOut
{


CLuaInterface::CLuaInterface()
{
	m_nStackSize = 0;
	m_luaState = NULL;
}

CLuaInterface::~CLuaInterface()
{
	if(m_luaState != NULL)
	{
		lua_close(m_luaState);
		m_luaState = NULL;
	}
}

void CLuaInterface::OpenLua()
{
	if(m_luaState == NULL)
	{
		m_luaState = luaL_newstate();
		luaL_openlibs(m_luaState);
	}
}

void CLuaInterface::CloseLua()
{
	if(m_luaState != NULL)
	{
		lua_close(m_luaState);
		m_luaState = NULL;
	}
}

int CLuaInterface::LoadBuffer(const char *strLua, const char *strName)
{
	if(strLua == NULL || strLua[0] == 0 || strName == NULL)
	{
		return -1;
	}
	if(luaL_loadbuffer(m_luaState, strLua, strlen(strLua), strName))
	{
		LOG_BASE(FILEINFO, "Load buffer[strname=%s] error", strName);

		Pop(1);
		return -1;
	}
	return 0;
}

int CLuaInterface::LoadFile(const std::string &strFileName)
{
	std::string strFullFileName;
	CUtil::GetFullPath(strFileName, strFullFileName);

	if(luaL_loadfile(m_luaState, strFullFileName.c_str()))
	{
		LOG_BASE(FILEINFO, "Load file[filepath=%s] error", strFileName.c_str());

		Pop(1);
		return -1;
	}

	return 0;
}

int CLuaInterface::Call(int nArgs, int nRets)
{
	int res = lua_pcall(m_luaState, nArgs, nRets, 0);
	if(res)
	{
		LOG_BASE(FILEINFO, "call error");

		Pop(1);
		return -1;
	}

	return 0;
}

int CLuaInterface::SetGlobalInt(const char *strGlobalName, int nValue)
{
	if(strGlobalName == NULL)
	{
		LOG_BASE(FILEINFO, "global name is null");

		return -1;
	}

	lua_pushinteger(m_luaState, nValue);
	lua_setglobal(m_luaState, strGlobalName);

	return 0;
}

int CLuaInterface::SetGlobalString(const char *strGlobalName, const char *strGlobalValue)
{
	if(strGlobalName == NULL || strGlobalValue == NULL)
	{
		LOG_BASE(FILEINFO, "global name is null");

		return -1;
	}
	lua_pushstring(m_luaState, strGlobalValue);
	lua_setglobal(m_luaState, strGlobalName);

	return 0;
}

int CLuaInterface::SetGlobalLightUserData(const char *strGlobalName, void *pValue)
{
	if(strGlobalName == NULL || pValue == NULL)
	{
		LOG_BASE(FILEINFO, "global name is null");

		return -1;
	}
	lua_pushlightuserdata(m_luaState, pValue);
	lua_setglobal(m_luaState, strGlobalName);

	return 0;
}

int CLuaInterface::RegisterFun(const char *strFunName, lua_CFunction fun)
{
	lua_register(m_luaState, strFunName, fun);
	return 0;
}

int CLuaInterface::SetLightUserData(void *pValue)
{
	lua_pushlightuserdata(m_luaState, pValue);
	return 0;
}

int CLuaInterface::SetInt(int nValue)
{
	lua_pushinteger(m_luaState, nValue);
	return 0;
}

int CLuaInterface::SetFloat(float fValue)
{
	lua_pushnumber(m_luaState, fValue);
	return 0;
}

int CLuaInterface::SetString(const char *strValue)
{
	lua_pushstring(m_luaState, strValue);
	return 0;
}

int CLuaInterface::GetInt(const char *strParam, int nIndex)
{
	if(strParam != NULL)
	{
		lua_getglobal(m_luaState, strParam);
	}

	if(!lua_isnumber(m_luaState, nIndex))
	{
		Pop(1);
		return 0;
	}

	int n = lua_tointeger(m_luaState, nIndex);
	Pop(1);

	return n;
}

float CLuaInterface::GetFloat(const char *strParam, int nIndex)
{
	if(strParam != NULL)
	{
		lua_getglobal(m_luaState, strParam);
	}

	if(!lua_isnumber(m_luaState, nIndex))
	{
		Pop(1);
		return 0;
	}

	float fValue = lua_tonumber(m_luaState, -1);
	Pop(1);

	return fValue;
}

int CLuaInterface::GetString(char *buf, int nLen, const char* strParam, int nIndex)
{
	if(buf == NULL)
	{
		LOG_BASE(FILEINFO, "buf is null");

		return -1;
	}
	if(strParam != NULL)
	{
		lua_getglobal(m_luaState, strParam);
	}
	if(!lua_isstring(m_luaState, nIndex))
	{
		LOG_BASE(FILEINFO, "value isn't string");

		Pop(1);
		return -1;
	}

	size_t nStrLen;
	const char *strValue = lua_tolstring(m_luaState, nIndex, &nStrLen);
	if(nStrLen >= (DWORD)nLen)
	{
		LOG_BASE(FILEINFO, "buf len is too min");

		return -1;
	}
	strncpy(buf, strValue, nStrLen + 1);
	Pop(1);

	return 0;
}

int CLuaInterface::GetTable(const char *str)
{
	if(str == NULL)
	{
		LOG_BASE(FILEINFO, "global name is null");

		return -1;
	}
	lua_getglobal(m_luaState, str);

	if(lua_istable(m_luaState, -1))
	{
		return 0;
	}
	else
	{
		LOG_BASE(FILEINFO, "value isn't table");

		Pop(1);
		return -1;
	}
}

int CLuaInterface::GetTableFieldCount()
{
	if (!lua_istable(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't table");

		return -1;
	}

	return lua_rawlen(m_luaState, -1);
}/**/

void CLuaInterface::CloseTable()
{
	Pop(1);
}

int CLuaInterface::GetFieldInt(const char *strKey)
{
	if(strKey == NULL)
	{
		return 0;
	}
	lua_pushstring(m_luaState, strKey);
	lua_gettable(m_luaState, -2);
	if(!lua_isnumber(m_luaState, -1))
	{
		Pop(1);

		return 0;
	}
	int n = (int)lua_tointeger(m_luaState, -1);
	Pop(1);

	return n;
}

int CLuaInterface::GetFieldInt(int nKey)
{
	lua_pushinteger(m_luaState, nKey);
	lua_gettable(m_luaState, -2);
	if(!lua_isnumber(m_luaState, -1))
	{
		Pop(1);

		return 0;
	}
	int n = (int)lua_tointeger(m_luaState, -1);
	Pop(1);

	return n;
}

float CLuaInterface::GetFieldFloat(int nKey)
{
	lua_pushinteger(m_luaState, nKey);
	lua_gettable(m_luaState, -2);
	if(!lua_isnumber(m_luaState, -1))
	{
		Pop(1);

		return 0;
	}
	float fValue = lua_tonumber(m_luaState, -1);
	Pop(1);

	return fValue;
}

int CLuaInterface::GetFieldTable(int nKey)
{
	lua_pushinteger(m_luaState, nKey);
	lua_gettable(m_luaState, -2);

	if(!lua_istable(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't table");

		Pop(1);

		return -1;
	}

	return 0;
}

int CLuaInterface::GetFieldTable(const char *strKey)
{
	if(strKey == NULL)
	{
		LOG_BASE(FILEINFO, "key is null");

		return -1;
	}

	lua_pushstring(m_luaState, strKey);
	lua_gettable(m_luaState, -2);
	if(!lua_istable(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't table");

		Pop(1);

		return -1;
	}

	return 0;
}

int CLuaInterface::GetFieldIntInTableList(int nFirstBoundKey, int nSecondBoundKey)
{
	lua_pushinteger(m_luaState, nFirstBoundKey);
	lua_gettable(m_luaState, -2);
	lua_pushinteger(m_luaState, nSecondBoundKey);
	lua_gettable(m_luaState, -2);
	if(!lua_isnumber(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't number");

		Pop(2);

		return -1;
	}
	int n = (int)lua_tointeger(m_luaState, -1);
	Pop(2);

	return n;
}

int CLuaInterface::GetFieldString(char *buf, int nLen, const char *strKey)
{
	if(buf == NULL || strKey == NULL)
	{
		LOG_BASE(FILEINFO, "buf point is null or key is null");

		return -1;
	}
	lua_pushstring(m_luaState, strKey);
	lua_gettable(m_luaState, -2);

	size_t nStrLen;
	const char *strValue = lua_tolstring(m_luaState, -1, &nStrLen);

	if(!lua_isstring(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't string");

		Pop(1);
		return -1;
	}

	if(nStrLen >= (DWORD)nLen)
	{
		LOG_BASE(FILEINFO, "buf len is too min");

		Pop(1);

		return -1;
	}
	strncpy(buf, strValue, nLen);
	Pop(1);

	return 0;
}

int CLuaInterface::GetFieldString(char *buf, int nLen, int nKey)
{
	if(buf == NULL)
	{
		LOG_BASE(FILEINFO, "buf point is null");

		return -1;
	}
	lua_pushinteger(m_luaState, nKey);
	lua_gettable(m_luaState, -2);

	size_t nStrLen;
	const char *strValue = lua_tolstring(m_luaState, -1, &nStrLen);

	if(!lua_isstring(m_luaState, -1))
	{
		LOG_BASE(FILEINFO, "value isn't string");

		Pop(1);
		return -1;
	}

	if(nStrLen >= (DWORD)nLen)
	{
		LOG_BASE(FILEINFO, "buf len is too min");

		return -1;
	}

	strncpy(buf, strValue, nLen);
	Pop(1);

	return 0;
}

int CLuaInterface::GetFieldStringInTableList(char *buf, int nLen, int nFirstBoundKey, int nSecondBoundKey)
{
	if(buf == NULL)
	{
		LOG_BASE(FILEINFO, "buf point is null");

		return -1;
	}

	lua_pushinteger(m_luaState, nFirstBoundKey);
	lua_gettable(m_luaState, -2);
	lua_pushinteger(m_luaState, nSecondBoundKey);
	lua_gettable(m_luaState, -2);

	size_t nStrLen;
	const char *strValue = lua_tolstring(m_luaState, -1, &nStrLen);

	if(nStrLen >= (DWORD)nLen)
	{
		LOG_BASE(FILEINFO, "buf len is too min");

		Pop(2);

		return -1;
	}
	strncpy(buf, strValue, nLen);
	Pop(2);

	return 0;
}

void CLuaInterface::Pop(int n)
{
	lua_pop(m_luaState, n);
}

int CLuaInterface::SetLuaFunction(const char *strFunName)
{
	if(strFunName == NULL)
	{
		LOG_BASE(FILEINFO, "global name is null");

		return -1;
	}

	lua_getglobal(m_luaState, strFunName);

	return 0;
}

int CLuaInterface::GetStackTop()
{
	return lua_gettop(m_luaState);
}

void CLuaInterface::CreateNewTable()
{
	lua_newtable(m_luaState);
}

int CLuaInterface::SetFieldInt(const char *strFieldName, int nValue)
{
	if(strFieldName == NULL)
	{
		LOG_BASE(FILEINFO, "field name is null");

		return -1;
	}

	lua_pushinteger(m_luaState, nValue);
	lua_setfield(m_luaState, -2, strFieldName);
	return 0;
}

int CLuaInterface::SetFieldInt(int nIndex, int nValue)
{
	lua_pushinteger(m_luaState, nValue);
	lua_rawseti(m_luaState, -2, nIndex);
	return 0;
}

int CLuaInterface::SetFieldTable(int nIndex)
{
	lua_rawseti(m_luaState, -2, nIndex);
	return 0;
}

int CLuaInterface::SetFieldTable(const char *strTableName)
{
	lua_setfield(m_luaState, -2, strTableName);
	return 0;
}

int CLuaInterface::FinishTable(const char *strTableName)
{
	if(strTableName == NULL)
	{
		LOG_BASE(FILEINFO, "table name is null");

		return -1;
	}

	lua_setglobal(m_luaState, strTableName);
	return 0;
}

void CLuaInterface::SavedStackSize()
{
	m_nStackSize = lua_gettop(m_luaState);
	lua_pushnil(m_luaState);
}
}


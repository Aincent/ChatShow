/*
 * LuaPoolInterface.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef LUA_LUAPOOLINTERFACE_H_
#define LUA_LUAPOOLINTERFACE_H_

#include "../Smart_Ptr.h"
#include "lua.hpp"
#include "LuaInterface.h"

namespace CommBaseOut
{

class LuaPoolInterface
{
public:
	LuaPoolInterface();
	~LuaPoolInterface();

	/**
	*  执行已经加载lua程序或者lua函数
	*	return : E_LuaRet
	*	param : nArgs lua函数参数个数    nRets lua函数返回结果
	*	 note : 执行一段lua的chunk时，nArgs和nRets都为0
	*/
	int Call(int nArgs = 0, int nRets = 0);

	int SetGlobalString(const char *strGlobalName, const char *strGlobalValue);

	int SetGlobalLightUserData(const char *strGlobalName, void *pValue);

	int SetGlobalInt(const char *strGlobalName, int nValue);

	void CreateNewTable();

	int SetFieldInt(const char *strFieldName, int nValue);

	int SetFieldInt(int nIndex, int nValue);

	int SetFieldTable(int nIndex);

	int SetFieldTable(const char *strTableName);

	int FinishTable(const char *strTableName);

	int RegisterFun(const char *strFunName, lua_CFunction fun);

	int SetLuaFunction(const char *strFunName);

	int SetInt(int nValue);

	int SetFloat(float fValue);

	int SetString(const char *strValue);

	int SetLightUserData(void *pValue);

	int GetInt(const char *strParamName = NULL, int nIndex = -1);

	float GetFloat(const char *strParam = NULL, int nIndex = -1);

	int GetString(char *buf, int nLen, const char* strParamName = NULL, int nIndex = -1);

	int GetTable(const char *strTableName);

	void CloseTable(void);

	int GetTableFieldCount(void);

	int GetFieldIntInTableList(int nKey1, int nKey2);

	int GetFieldStringInTableList(char *buf, int nLen, int nKey1, int nKey2);

	int GetFieldInt(const char *strKey);

	int GetFieldInt(int nKey);

	float GetFieldFloat(int nKey);

	int GetFieldTable(int nKey);

	int GetFieldTable(const char *sKey);

	int GetFieldString(char *buf, int nLen, const char *strKey);

	int GetFieldString(char *buf, int nLen, int nKey);

	void Pop(int n);

	int GetStackTop();

	void SavedStackSize();

	Smart_Ptr<CLuaInterface> & GetLua()
	{
		return m_lua;
	}

private:
	Smart_Ptr<CLuaInterface> m_lua;
};

}

#endif /* LUA_LUAPOOLINTERFACE_H_ */

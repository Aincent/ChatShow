/*
 * LuaPoolInterface.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "LuaPoolInterface.h"
#include "LuaPool.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

LuaPoolInterface::LuaPoolInterface()
{
	m_lua = LuaPool::GetInstance()->GetLuaInterface();
	if(!m_lua)
	{
		LOG_BASE(FILEINFO, "LuaPoolInterface GetLuaInterface error");
	}
}

LuaPoolInterface::~LuaPoolInterface()
{
	if(!m_lua)
	{
		return;
	}

	LuaPool::GetInstance()->ReleaseLuaInterface(m_lua);
}


int LuaPoolInterface::Call(int nArgs, int nRets)
{
	return m_lua->Call(nArgs, nRets);
}

int LuaPoolInterface::SetGlobalString(const char *strGlobalName, const char *strGlobalValue)
{
	return m_lua->SetGlobalString(strGlobalName, strGlobalValue);
}

int LuaPoolInterface::SetGlobalLightUserData(const char *strGlobalName, void *pValue)
{
	return m_lua->SetGlobalLightUserData(strGlobalName, pValue);
}

int LuaPoolInterface::SetGlobalInt(const char *strGlobalName, int nValue)
{
	return m_lua->SetGlobalInt(strGlobalName, nValue);
}

void LuaPoolInterface::CreateNewTable()
{
	m_lua->CreateNewTable();
}

int LuaPoolInterface::SetFieldInt(const char *strFieldName, int nValue)
{
	return m_lua->SetFieldInt(strFieldName, nValue);
}

int LuaPoolInterface::SetFieldInt(int nIndex, int nValue)
{
	return m_lua->SetFieldInt(nIndex, nValue);
}

int LuaPoolInterface::SetFieldTable(int nIndex)
{
	return m_lua->SetFieldTable(nIndex);
}

int LuaPoolInterface::SetFieldTable(const char *strTableName)
{
	return m_lua->SetFieldTable(strTableName);
}

int LuaPoolInterface::FinishTable(const char *strTableName)
{
	return m_lua->FinishTable(strTableName);
}

int LuaPoolInterface::RegisterFun(const char *strFunName, lua_CFunction fun)
{
	return m_lua->RegisterFun(strFunName, fun);
}

int LuaPoolInterface::SetLuaFunction(const char *strFunName)
{
	return m_lua->SetLuaFunction(strFunName);
}

int LuaPoolInterface::SetInt(int nValue)
{
	return m_lua->SetInt(nValue);
}

int LuaPoolInterface::SetFloat(float fValue)
{
	return m_lua->SetFloat(fValue);
}

int LuaPoolInterface::SetString(const char *strValue)
{
	return m_lua->SetString(strValue);
}

int LuaPoolInterface::SetLightUserData(void *pValue)
{
	return m_lua->SetLightUserData(pValue);
}

int LuaPoolInterface::GetInt(const char *strParamName, int nIndex)
{
	return m_lua->GetInt(strParamName, nIndex);
}

float LuaPoolInterface::GetFloat(const char *strParam, int nIndex)
{
	return m_lua->GetFloat(strParam, nIndex);
}

int LuaPoolInterface::GetString(char *buf, int nLen, const char* strParamName, int nIndex)
{
	return m_lua->GetString(buf, nLen, strParamName, nIndex);
}

int LuaPoolInterface::GetTable(const char *strTableName)
{
	return m_lua->GetTable(strTableName);
}

void LuaPoolInterface::CloseTable(void)
{
	m_lua->CloseTable();
}

int LuaPoolInterface::GetTableFieldCount(void)
{
	return m_lua->GetTableFieldCount();
}

int LuaPoolInterface::GetFieldIntInTableList(int nKey1, int nKey2)
{
	return m_lua->GetFieldIntInTableList(nKey1, nKey2);
}

int LuaPoolInterface::GetFieldStringInTableList(char *buf, int nLen, int nKey1, int nKey2)
{
	return m_lua->GetFieldStringInTableList(buf, nLen, nKey1, nKey2);
}

int LuaPoolInterface::GetFieldInt(const char *strKey)
{
	return m_lua->GetFieldInt(strKey);
}

int LuaPoolInterface::GetFieldInt(int nKey)
{
	return m_lua->GetFieldInt(nKey);
}

float LuaPoolInterface::GetFieldFloat(int nKey)
{
	return m_lua->GetFieldFloat(nKey);
}

int LuaPoolInterface::GetFieldTable(int nKey)
{
	return m_lua->GetFieldTable(nKey);
}

int LuaPoolInterface::GetFieldTable(const char *sKey)
{
	return m_lua->GetFieldTable(sKey);
}

int LuaPoolInterface::GetFieldString(char *buf, int nLen, const char *strKey)
{
	return m_lua->GetFieldString(buf, nLen, strKey);
}

int LuaPoolInterface::GetFieldString(char *buf, int nLen, int nKey)
{
	return m_lua->GetFieldString(buf, nLen, nKey);
}

void LuaPoolInterface::Pop(int n)
{
	return m_lua->Pop(n);
}

int LuaPoolInterface::GetStackTop()
{
	return m_lua->GetStackTop();
}

void LuaPoolInterface::SavedStackSize()
{
	m_lua->SavedStackSize();
}

}

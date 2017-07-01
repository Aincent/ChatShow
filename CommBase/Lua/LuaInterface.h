/*
 * LuaInterface.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef LUA_LUAINTERFACE_H_
#define LUA_LUAINTERFACE_H_

#include "../define.h"
#include "lua.hpp"
#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{

enum E_LuaRet
{
	eLuaSuccess,
};

/**
* 	类的功能：提供与Lua交互接口
*	说明：打开关闭lua环境，所有与lua交互均通过这个类
*/
class CLuaInterface
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	CLuaInterface();

	 ~CLuaInterface();

	/**
	*	开启lua环境，打开lua_State
	*	return :
	*	param :
	*/
	void OpenLua();

	/**
	*	关闭lua环境，关闭lua_State
	*	return :
	*	param :
	*/
	void CloseLua();

	/**
	* 从内存buf中加载lua程序
	*return : E_LuaRet
	*param : strLua 需要加载的lua程序buf    strName buf命名（调试时使用）
	*/
	int LoadBuffer(const char *strLua, const char *strName);

	/**
	*	从文件中加载lua程序
	*	return : E_LuaRet
	*	param : strFileName 需要加载的程序文件名
	*/
	int LoadFile(const std::string &strFileName);

	/**
	* 执行已经加载lua程序或者lua函数
	*	return : E_LuaRet
	*	param : nArgs lua函数参数个数    nRets lua函数返回结果
	*	 note : 执行一段lua的chunk时，nArgs与nRets均为0
	*/
	int Call(int nArgs = 0, int nRets = 0);

	/**
	*	 设置一个全局变量的值为字符串值
	*	return : E_LuaRet
	*	param : strGlobalName 全局变量名     strGlobalValue 全局变量值
	*/
	int SetGlobalString(const char *strGlobalName, const char *strGlobalValue);

	/**
	* 设置一个全局变量的值为lightuserdata
	*	return : E_LuaRet
	*param : strGlobalName 全局变量名      pValue lightuserdata值
	*/
	int SetGlobalLightUserData(const char *strGlobalName, void *pValue);

	/**
	* 设置一个全局变量的值为int
	*	return : E_LuaRet
	*	param : strGlobalName 全局变量名       nValue 要设置的值
	*/
	int SetGlobalInt(const char *strGlobalName, int nValue);

	/**
	* 在lua中新建一个表
	*/
	void CreateNewTable();

	/**
	* 在lua表中设置字段和值
	*	return : E_LuaRet
	*param : strFieldName 表字段名           nValue 字段的值
	*/
	int SetFieldInt(const char *strFieldName, int nValue);

	/**
	* 在lua表中设置字段和值
	*	return : E_LuaRet
	* param : nIndex 字段索引     nValue 字段的值
	*/
	int SetFieldInt(int nIndex, int nValue);

	/**
	*	 在lua表中增加一个二级表
	*	return : E_LuaRet
	*	param : nIndex 表字段索引
	*/
	int SetFieldTable(int nIndex);

	/**
	*	 在lua表中增加一个二级表
	*	return : E_LuaRet
	*	param : strTableName 表字段索引
	*/
	int SetFieldTable(const char *strTableName);

	/**
	*	 结束table创建
	*	return : E_LuaRet
	*	param : strTableName 创建table名称
	*/
	int FinishTable(const char *strTableName);

	/**
	*	 在lua中注册一个C函数
	*	return : E_LuaRet
	*	param : strFunName 要注册函数名称      lua_CFunction fun 要注册函数
	*/
	int RegisterFun(const char *strFunName, lua_CFunction fun);

	/**
	*	 要调用lua函数名入栈
	*	return : E_LuaRet
	*	param strFunName 要调用lua函数名
	*/
	int SetLuaFunction(const char *strFunName);

	/**
	*	   将整数入栈
	*	 return: E_LuaRet
	*	 param : nValue 要入栈的值
	*	 note 调用完成占用一个栈空间
	*/
	int SetInt(int nValue);

	/**
	*	   将一个浮点数入栈
	*	 return : E_LuaRet
	*	 param : fValue 要入栈的值
	*	 note 调用完成占用一个栈空间
	*/
	int SetFloat(float fValue);

	/**
	*	   将字符串入栈
	*	 return : E_LuaRet
	*	 param : strValue 要入栈的值
	*	 note 调用完成占用一个栈空间
	*/
	int SetString(const char *strValue);

	/**
	*	   入栈一个lightuserdata，一般用于指针入栈
	*	 return : E_LuaRet
	*	 param : pValue 要入栈的值
	*	 note 调用完成占用一个栈空间
	*/
	int SetLightUserData(void *pValue);

	/**
	*	   获取一个整数值
	*	 return : 返回变量值
	*	 param : strParamName 要获取的变量名  nIndex 获取变量所在栈位置，默认从栈顶取
	*/
	int GetInt(const char *strParamName = NULL, int nIndex = -1);

	/**
	*	   获取一个浮点数
	*	 param : strParamName 要获取的变量名    nIndex 获取变量所在栈位置，默认从栈顶取
	*	 return : 返回变量值
	*/
	float GetFloat(const char *strParam = NULL, int nIndex = -1);

	/**
	*	   获取字符串的值
	*	 return : E_LuaRet
	*	 param : buf 获取到的值存放缓冲区   nLen 缓冲区长度   strParamName 要获取变量名，为NULL时返回栈顶值  nIndex 获取变量所在栈位置，默认从栈顶取
	*/
	int GetString(char *buf, int nLen, const char* strParamName = NULL, int nIndex = -1);

	/**
	*	   将一个table入栈
	*	 return : E_LuaRet
	*	 param : strTableName 要入栈table名
	*	 note 调用完成后占用栈空间加1，使用完后需要调用CloseTable
	*/
	int GetTable(const char *strTableName);

	/**
	*	   最近调用的table出栈
	*/
	void CloseTable(void);

	/**
	*	   获取表的元素个数
	*	 return : 成功返回table的字段数，失败返回小于0
	*	 param :
	*/
	int GetTableFieldCount(void);

	/**
	*	   获取二维表中的数值, 如取表info={{1, 9, 3}, {29, 89, 55}}中的值9
	*	 return :
	* param : nKey1 表中第一维的索引, 如: nKey1=1，则取第一维的表元素为: {1, 9, 3}
		         nKey2 为表中第二维的索引，如: nKey1=1，nKey2=2，则取第一维表元素: {1, 9, 3}中的元素: 9。
	*/
	int GetFieldIntInTableList(int nKey1, int nKey2);

	/**
	*	   获取二维表中的字符串, 如取表info={{1, "abc", 3}, {29, 89, 55}}中的字符串"abc"
	* return :
	* param : nKey1 表中第一维的索引, 如: nKey1=1，则取第一维的表元素为: {1, "abc", 3}
			          nKey2 为表中第二维的索引，如: nKey1=1，nKey2=2，则取第一维表元素: {1, "abc", 3}中的元素: "abc"。
	*/
	int GetFieldStringInTableList(char *buf, int nLen, int nKey1, int nKey2);

	/**
	*	   获取table的一个field的整数值
	*	 return : 返回key对应的值大小
	*	 param : strKey 要获取的key名称
	*	 note 当table的key为字符串时调用此函数
	*/
	int GetFieldInt(const char *strKey);

	/**
	*	   获取table的一个field的整数值
	*	 return : 返回key对应的值大小
	*	 param : nKey 要获取的key索引
	*	 note 当table的key为整数时调用此函数
	*/
	int GetFieldInt(int nKey);

	/**
	*	   获取table的一个field的浮点数值
	*	 return : 返回key对应的值
	*	 param : nKey 要获取的key索引
	*	 note 当table的key为整数时调用此函数
	*/
	float GetFieldFloat(int nKey);

	/**
	*	 table的一个field为table时，调用此方法获取table的一个field
	*	 return : E_LuaRet
	*	 param : nKey 要获取的key索引
	*	 note 当table的key为整数时调用此函数，调用完成占用栈空间加1
	*/
	int GetFieldTable(int nKey);

	/**
	*	 table的一个field为table时，调用此方法获取table的一个field
	*	 param : nKey 要获取的key索引
	*	 return : E_LuaRet
	*	 note 当table的key为整数时调用此函数，调用完成占用栈空间加1
	*/
	int GetFieldTable(const char *sKey);

	/**
	*	   获取table的一个field的字符串值
	*	 return : E_LuaRet
	*	 param : buf 获取字符串缓冲区   nLen 缓冲区大小   strKey 要获取的key名称
	*	 note 当table的key为字符串时调用此函数
	*/
	int GetFieldString(char *buf, int nLen, const char *strKey);

	/**
	*	   获取table的一个field的字符串值
	*	 return : E_LuaRet
	*	 param : buf 获取字符串缓冲区  nLen 缓冲区大小  nKey 要获取的key索引
	*	 note 当table的key为整数时调用此函数
	*/
	int GetFieldString(char *buf, int nLen, int nKey);

	/**
	*	   出栈
	*	 return :
	*	 param : n 要出栈数目
	*/
	void Pop(int n);

	/**
	*	   获取stack top
	*	 return : stack top
	*	 param :
	*/
	int GetStackTop();

	void SavedStackSize();

	int GetError()
	{
		return lua_error(m_luaState);
	}

protected:
	lua_State *m_luaState;
private:
	int m_nStackSize;//获取当前栈的的大小：索引
};
}

#endif /* LUA_LUAINTERFACE_H_ */

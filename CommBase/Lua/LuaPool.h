/*
 * LuaPool.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef LUA_LUAPOOL_H_
#define LUA_LUAPOOL_H_

#include "LuaInterface.h"
#include <deque>
#include "../Smart_Ptr.h"
#include "../Common.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{
/*
 * 用于多线程频繁对同一个lua文件进行操作的时
 * 当只是验证性的操作，那么开启一个池，可以加快多线程访问的速度
 * @todo : 由于没有对luainterface进行自我回收，不能单独使用，必须使用luapoolinterface来调用它
 */
class LuaPool
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:

	 ~LuaPool();

	static LuaPool * GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = NEW LuaPool();
		}

		return m_instance;
	}

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	/*
	 * 初始化lua池
	 * return : 0成功  －1失败
	 * param : content 初始的池大小   rate每次增加的池的大小     filename 加载的lua的文件名
	 */
	int Init(int content, int rate, string &filename);

	/*
	 * 创建lua接口
	 * return ： 0 成功    －1 失败
	 * param ： count 创建的个数
	 * @todo : 里面有加锁，在使用之前必须要对其m_poolLock枷锁
	 */
	int Create(int count);

	/*
	 * 获取一个lua接口
	 * return ： 返回的lua接口对象
	 * param ：
	 */
	Smart_Ptr<CLuaInterface> GetLuaInterface();

	/*
	 * 释放lua接口，唯一一个回收的接口
	 * return ：
	 * param ： 回收的lua接口
	 */
	void ReleaseLuaInterface(Smart_Ptr<CLuaInterface> & lua);

private:
	LuaPool();

private:

	static LuaPool * m_instance;
	int m_content;  // 当前大小
	int m_rate;  // 增长率
	string m_file; // 需要打开的lua文件
	deque<Smart_Ptr<CLuaInterface> > m_pool;
	CSimLock m_poolLock;
};
}

#endif /* LUA_LUAPOOL_H_ */

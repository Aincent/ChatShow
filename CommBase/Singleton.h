/*
 * Singleton.h
 *
 *  Created on: Jun 9, 2017
 *      Author: root
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include "./Memory/MemAllocator.h"

namespace CommBaseOut
{

template<typename T>
class Singleton
{
public:
	virtual ~Singleton(void){}

	static T * GetInstance()
	{
		if(0 == m_instance)
		{
			m_instance = new T;
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

public:
	Singleton(void){}

protected:
 static	T * m_instance;
};

template<typename T>
T * Singleton<T>::m_instance = 0;

}


#endif /* SINGLETON_H_ */

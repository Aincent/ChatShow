/*
 * Object.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include "define.h"

using namespace CommBaseOut;

enum eObjType
{
	eObject = ( 1<<1 ),
	ePlayer = ( eObject | ( 1<<2 ) ),
	eMax
};

class Object
{
public:
	Object(eObjType type):m_type(type),m_key(0)
	{	}
	virtual ~Object()
	{	}

	void Release()
	{
		m_key = 0;
	}

	void SetKey(int key) { 	m_key = key; }
	int GetKey() { 	return m_key; }
	int GetType() { return m_type; }

private:
	int m_type; //eObjType
	int m_key;
};



#endif /* OBJECT_H_ */

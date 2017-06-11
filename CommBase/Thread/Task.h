/*
 * Task.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef THREAD_TASK_H_
#define THREAD_TASK_H_

#include <pthread.h>

namespace CommBaseOut
{

enum E_TaskRet
{
	eTaskSuccess=0,
	eCreateThreadErr,
};

class Task
{
public:
	Task();
	virtual ~Task();

	int Start(int num, pthread_attr_t *attr = 0);

	int End();

	virtual void Close(){}

private:

	static void * ThreadRun(void *p);

	virtual int svr();

protected:

	bool m_flag;

private:
	int m_count;
	pthread_t *m_pID;
};

}

#endif /* THREAD_TASK_H_ */

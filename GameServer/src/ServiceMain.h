/*
 * ServiceMain.h
 *
 *  Created on: Jun 24, 2017
 *      Author: root
 */

#ifndef SERVICEMAIN_H_
#define SERVICEMAIN_H_
#include "ServerConfig.h"
using namespace CommBaseOut;

class CServiceMain
{
public:
	CServiceMain();
	~CServiceMain();

	int Init();

	int InitFile();

	int InitInstance();

	void UnInit();

	void Start();

	static void Stop(int signal);

	short int GetLocalID()
	{
		return m_conf.GetLocalID();
	}

protected:


private:
	CServerConf m_conf;
	static bool m_flag;
};

#endif /* SERVICEMAIN_H_ */

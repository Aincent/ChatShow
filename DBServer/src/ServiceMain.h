/*
 * ServiceMain.h
 *
 *  Created on: Jun 28, 2017
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

	int InitInstance();

	void UnInitInstance();

	void Start();

	static void Stop(int signal);

protected:


private:
	CServerConf m_conf;
};

#endif /* SERVICEMAIN_H_ */

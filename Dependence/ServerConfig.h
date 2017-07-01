/*
 * ServerConfig.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef SERVERCONFIG_H_
#define SERVERCONFIG_H_

#include "Lua/LuaInterface.h"
#include "SvrConfig.h"
#include "Network/NetWorkConfig.h"
#include <vector>
#include "Memory/MemAllocator.h"
#include "Password/Password.h"

using namespace std;
using namespace CommBaseOut;

class CServerConf
{
public:
	CServerConf():m_blockThread(0),m_ioThread(0),m_serverID(-1),m_firstPort(0)
	{
		m_con.clear();
		m_pLua.OpenLua();
	}
	~CServerConf()
	{
		m_pLua.CloseLua();
	}

	int LoadConfig(const string &strPath)
	{
		if(m_pLua.LoadFile(strPath) != 0 || m_pLua.Call() != 0)
		{
			return -1;
		}

		if(ReadLogConf() ||
				ReadConnConf() ||
				ReadSendConnConf() ||
				ReadAccConfig() ||
				ReadThreadConfig() ||
				ReadDBConfig() ||
				ReadServerID() )
		{
			return -1;
		}

		return 0;
	}

	int ReadServerID()
	{
		m_pLua.SetLuaFunction("GetServerID");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		m_serverID = m_pLua.GetInt();

		return 0;
	}

	int ReadLogConf()
	{
		m_pLua.SetLuaFunction("GetLogConfig");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		if (m_pLua.GetFieldTable(1) != 0)
		{
			return -1;
		}

		m_log.base = m_pLua.GetFieldInt("base");
		m_log.debug = m_pLua.GetFieldInt("debug");
		m_log.display = m_pLua.GetFieldInt("display");
		m_log.error = m_pLua.GetFieldInt("error");
		m_log.fatal = m_pLua.GetFieldInt("fatal");
		m_log.info = m_pLua.GetFieldInt("info");
		m_log.warning = m_pLua.GetFieldInt("warning");
		m_log.fileLen = m_pLua.GetFieldInt("fileLen");

		char path[32] = {0};
		char module[32] = {0};
		m_pLua.GetFieldString(path, 32, "filePath");
		m_pLua.GetFieldString(module, 32, "module");

		m_log.filePath = path;
		m_log.module = module;

		m_pLua.CloseTable();
		m_pLua.CloseTable();

		return 0;
	}

	int ReadConnConf()
	{
		m_pLua.SetLuaFunction("GetConnConfig");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		int tableCount = m_pLua.GetTableFieldCount();
		for(int i=1; i<=tableCount; ++i)
		{
			if (m_pLua.GetFieldTable(i) != 0)
			{
				return -1;
			}

			ConnectionConfig con;
			char ip[32] = {0};
			int port = -1;

			con.aes_algorithm_type_ = m_pLua.GetFieldInt("aesType");
			con.channel_keep_time_ = m_pLua.GetFieldInt("keepTime");
			con.check_wait_time_ = m_pLua.GetFieldInt("checkTime");
			con.local_id_ = m_pLua.GetFieldInt("localID");
			con.local_type_ = m_pLua.GetFieldInt("localType");
			con.re_connect_ = m_pLua.GetFieldInt("connect");
			con.remote_id_ = m_pLua.GetFieldInt("remoteID");
			con.remote_type_ = m_pLua.GetFieldInt("remoteType");
			con.security_ = m_pLua.GetFieldInt("security");
			con.group_count = m_pLua.GetFieldInt("groupCount");
			port = m_pLua.GetFieldInt("port");
			m_pLua.GetFieldString(ip, 32, "ip");

			if(strlen(ip) == 0)
			{
				string netName = "lo";

				CUtil::SafeMemmove(ip, 31, CUtil::GetIpByName(netName).c_str(), CUtil::GetIpByName(netName).size());
				strcpy(ip, CUtil::GetIpByName(netName).c_str());
			}

			con.remote_address_ = NEW Inet_Addr(ip, port);

			m_con.push_back(con);
			m_pLua.CloseTable();
		}

		m_pLua.CloseTable();

		return 0;
	}

	int ReadSendConnConf()
	{
		m_pLua.SetLuaFunction("GetSendConnConfig");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		int tableCount = m_pLua.GetTableFieldCount();
		for(int i=1; i<=tableCount; ++i)
		{
			if (m_pLua.GetFieldTable(i) != 0)
			{
				return -1;
			}

			char ip[128] = {0};

			m_firstPort = m_pLua.GetFieldInt("port");
			m_pLua.GetFieldString(ip, 127, "ip");
			m_firstIP = ip;

			m_pLua.CloseTable();
		}

		m_pLua.CloseTable();

		return 0;
	}

	int ReadThreadConfig()
	{
		m_pLua.SetLuaFunction("GetThreadConfig");

		if(m_pLua.Call(0, 2))
		{
			return -1;
		}

		m_ioThread = m_pLua.GetInt();
		m_blockThread = m_pLua.GetInt();

		return 0;
	}

	int ReadAccConfig()
	{
		m_pLua.SetLuaFunction("GetAccConfig");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		int tableCount = m_pLua.GetTableFieldCount();
		for(int i=1; i<=tableCount; ++i)
		{
			if (m_pLua.GetFieldTable(i) != 0)
			{
				return -1;
			}

			AcceptorConfig acc;
			char ip[32] = {0};

			acc.aes_algorithm_type_ = m_pLua.GetFieldInt("aesType");
			acc.channel_keep_time_ = m_pLua.GetFieldInt("keepTime");
			acc.is_packet_keepalive = m_pLua.GetFieldInt("keepAlive");
			acc.local_id_ = m_pLua.GetFieldInt("localID");
			acc.local_type_ = m_pLua.GetFieldInt("localType");
			acc.max_allow_accept_ = m_pLua.GetFieldInt("maxAcc");
			acc.recv_packet_overfllower_count_ = m_pLua.GetFieldInt("overCount");
			acc.security_ = m_pLua.GetFieldInt("security");
			acc.listen_port_ = m_pLua.GetFieldInt("port");
			m_pLua.GetFieldString(ip, 32, "ip");
			acc.listen_ip_ = ip;

			m_acc.push_back(acc);
			m_pLua.CloseTable();
		}

		m_pLua.CloseTable();

		return 0;
	}

	int ReadDBConfig()
	{
		m_pLua.SetLuaFunction("GetDBConfig");

		if(m_pLua.Call(0, 1))
		{
			return -1;
		}

		int tableCount = m_pLua.GetTableFieldCount();
		for(int i=1; i<=tableCount; ++i)
		{
			if (m_pLua.GetFieldTable(i) != 0)
			{
				return -1;
			}

			char host[32] = {0};
			char db[32] = {0};
			char user[32] = {0};
			char passwd[128] = {0};

			m_dbConfig.port = m_pLua.GetFieldInt("port");
			m_dbConfig.size = m_pLua.GetFieldInt("size");
			m_dbConfig.rate = m_pLua.GetFieldInt("rate");
			m_pLua.GetFieldString(host, 32, "host");
			m_pLua.GetFieldString(db, 32, "db");
			m_pLua.GetFieldString(user, 32, "user");
			m_pLua.GetFieldString(passwd, 128, "passwd");
			m_dbConfig.host = host;
			m_dbConfig.db = db;
			string tempStr = passwd;
			m_dbConfig.passwd = tempStr; // Password::getStr(tempStr);
			m_dbConfig.user = user;

			m_pLua.CloseTable();
		}

		m_pLua.CloseTable();

		return 0;
	}

	LogConf &GetLogConf()
	{
		return m_log;
	}

	vector<ConnectionConfig> &GetConnCof()
	{
		return m_con;
	}

	short int GetLocalID()
	{
		vector<ConnectionConfig>::iterator it = m_con.begin();
		if(it != m_con.end())
		{
			return it->local_id_;
		}

		vector<AcceptorConfig>::iterator itAcc = m_acc.begin();
		if(itAcc != m_acc.end())
		{
			return it->local_id_;
		}

		return -1;
	}

	vector<AcceptorConfig> &GetAccCof()
	{
		return m_acc;
	}

	string &GetFirstAccIP()
	{
		return m_firstIP;
	}

	short int GetFirstAccPort()
	{
		return m_firstPort;
	}

	int GetBlockThread()
	{
		return m_blockThread;
	}

	int GetIOThread()
	{
		return m_ioThread;
	}

	int GetServerID()
	{
		return m_serverID;
	}

	DBConf & GetDBConf()
	{
		return m_dbConfig;
	}

private:
	CLuaInterface m_pLua;
	LogConf m_log;
	DBConf m_dbConfig;
	vector<ConnectionConfig> m_con;
	vector<AcceptorConfig> m_acc;
	int m_blockThread;
	int m_ioThread;
	int m_serverID;
	string m_firstIP;
	short int m_firstPort;
};

#endif /* SERVERCONFIG_H_ */

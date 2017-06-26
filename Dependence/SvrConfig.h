/*
 * SvrConfig.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef SVRCONFIG_H_
#define SVRCONFIG_H_

#include <string>

using namespace std;

typedef struct sLogConf
{
	bool base;
	bool info;
	bool debug;
	bool warning;
	bool error;
	bool fatal;
	bool display;
	int fileLen;
	string filePath;
	string module;
}LogConf;

typedef struct sDBConf
{
	string host;
	string db;
	string user;
	string passwd;
	int port;
	int size;
	int rate;
}DBConf;

enum EMapType
{
	eOnlyMap=1,//唯一地图
	eStaticMap,//静态地图，每个gateserver下面都可以管理多个相同的，开服时候创建
	eCopyMap, //副本，可以创建多个，动态创建
	eBeyondMap, //夸服副本
	ePVEMap, //pve副本
	eMapMax
};

enum ECopyMapType
{
	eCopyMax
};

enum ServerMapType
{
	eSceneType=0, 		//场景id
	eMapType, 					//地图id
	eStartPoint, 			//传送点
	eBePointType, 		//带有传送点的类型（帮派战，在传送开始时就决定了到哪里）
	eChangeMapLine,	//换线传送
};

//创建场景id（64位），t为地图的类型，a为gateserver的id（保持loginserver这里场景id的唯一性，夸服此字段为0，为了保持所有服组场景id的唯一）
//g为gameserver的id，x为地图的id，i为编号（副本需要id标识，其他i为0）
#define CREATE_MAP(t, a, g, x, i) (((CommBaseOut::int64)t << 40) | ((CommBaseOut::int64)a << 32) | ((CommBaseOut::int64)g << 24) | ((CommBaseOut::int64)x << 16) | i)

//获取场景类型
#define GET_MAP_TYPE(m) ((m >> 40) & 0xff)

//获取地图id
#define GET_MAP_ID(m) ((m >> 16) & 0xff)

//获取场景所在的gateserver
#define GET_GATESVR_ID(m) ((m >> 32) & 0xff)

//获取场景所在的gameserver
#define GET_SERVER_ID(m) ((m >> 24) & 0xff)

//客户端连接网关超时时间
#define CONN_GATESERVER_TIMEOUT 15

// gameserver的id的唯一id合成     t为gateserver的id，x为gs的id
#define CREATE_GSID(t, x) ((t << 8) | x)

#define CREATE_GATEID_GS(x, t) ((x << 16) | t)

#define CREATE_DBID_GS(x, t) ((x << 16) | t)

#define CREATE_CHARID_GS(x, t) (((int64)x << 48) | t)
//根据CHARID获得服组ID
#define GET_SERVER_CHARID(m) ((m >> 48) &0xffff)
//根据数据查询ID获取服组ID
#define GET_SERVER_DBID(m) ((m >> 32) &0xffff)
//去除服组ID的CHARID
#define GET_PLAYER_CHARID(m) ((m << 16 ) >> 16)

#define SAVE_CHARCACHE_TODB 1*60*1000

#define DELETE_CHARCACHE_TIMEOUT 2*60*1000

#define CLIENT_HEART_BEAT 200*1000

#define CLIENT_SYNCH_POS 1 * 1000

#define CLIENT_POS_OFFSET 2

#define SECTOR_LENGTH 50

//距离传送点的最大距离
#define STARTPOINT_LENGTH 6

// 获取某类型size的二进制位数数量
#define GET_TYPE_BINARYDIGITS(T)	(sizeof(T) * 8)

#endif /* SVRCONFIG_H_ */

/*
 * ServerMsgDefine.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef SERVERMSGDEFINE_H_
#define SERVERMSGDEFINE_H_

const int MSG_REQ_GT2GM_PUSHSERVERID = 1;
const int MSG_REQ_DB2GM_PUSHSERVERID = 2;

const int MSG_REQ_LS2GT_WILLLOGIN = 1000;
const int MSG_REQ_C2GT_HEARTBEAT = 1002;
const int MSG_REQ_C2GT_PLAYERINFO = 1003;
const int MSG_REQ_GT2GM_PLAYERINFO = 1004;
const int MSG_REQ_GM2DB_PLAYERINFO = 1005;
const int MSG_SIM_GT2LS_PLAYERLOGIN = 1006;
const int MSG_SIM_GM2GT_PLAYER_EXIT = 1007;  //GameServer踢玩家下线
const int MSG_SIM_GT2GM_SAVE_PLAYERINFO = 1008;
const int MSG_REQ_GS2DB_SAVE_PLAYERDATA = 1009;
const int MSG_REQ_GT2GM_CLIENTIN = 1010;
const int MSG_REQ_C2GT_CLIENTIN = 1011;
const int MSG_REQ_GT2GM_PLAYEREXIT = 1012;
const int MSG_REQ_C2GT_SYN_ATTR = 1013;
const int MSG_REQ_GT2GM_SYN_ATTR = 1014;
const int MSG_REQ_GT2GM_FRIEND_LIST = 1015;

#endif /* SERVERMSGDEFINE_H_ */

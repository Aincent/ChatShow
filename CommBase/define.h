/*
 * define.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef DEFINE_H_
#define DEFINE_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include <iostream>
#include <map>
#include <queue>
#include <vector>

namespace CommBaseOut
{
//base unsigned type
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long long DWORD64;

typedef long long int64;

//base str len
const int IPSTR_LEN = 16;
const int HOST_NAME_LEN = 64;
const int FILENAME_LEN = 128;

//24*60*60
const int SECONDS_OF_DAY = 86400;
//7*24*60*60
const int SECONDS_OF_WEEK = 604800;
//30*24*60*60
const DWORD SECONDS_OF_MONTH = 2592000;

//send buff type
enum SendBufType
{
	eClientBuf,
	eServerBuf,
	eBufMax
};
const int MAX_NET_MSG_SIZE[eBufMax] = {1024 * 512, 1024 * 1024};

//max msg size
const int MAX_RECV_MSG_SIZE = 1024 * 64;
const int MAX_RECV_MSG_CONTENT_SIZE = 1024;
const int MAX_MSG_PACKET_SIZE = 1024 * 64;

const int MAX_LISTEN_SIZE = 1000;
const int MAX_EPOLL_fd = 10000;
const int MAX_EPOLL_EVENT = 10000;

//epoll_wait
const int EPOLL_TIMEOUT = 1000;

const int KEEP_IDLE = 180;
const int KEEP_INTERVAL = 5;
const int KEEP_COUNT = 3;

//timer slot
const int SLOT_COUNT = 3000;
const int SLOT_CHANGE_TIME = 100;

//mysql execute len
const int MYSQL_EXECUTE_BUF = 32 * 1024;
}

#endif /* DEFINE_H_ */

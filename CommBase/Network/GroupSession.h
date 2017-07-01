/*
 * GroupSession.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_GROUPSESSION_H_
#define NETWORK_GROUPSESSION_H_

#include "Context.h"
#include <vector>
#include <map>
#include "../define.h"
#include "../Common.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

/*
 *     组连接
 *  为解决单通道缓冲区满的问题
 *   可以通过key来定制通道，如果没有定制，会按照默认的通道发送
 */

class GroupSession
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
	struct GroupSessionInfo
	{
		vector<int> channel; //组的channel
		BYTE count; //组的成员数目
		WORD slot; //轮

		GroupSessionInfo():count(0),slot(0)
		{

		}

		int GetChannel(int key)
		{
			int tSlot = key % channel.size();// % key;

//			printf("\n++++++++++++++++++++slot=%d, channel=%d++++++++++++++\n", tSlot, channel[tSlot]);

			return channel[tSlot];
		}

		bool DeleteChannel(int channelID)
		{
			vector<int>::iterator it = channel.begin();
			for(; it!=channel.end(); ++it)
			{
				if(channelID == *it)
				{
					channel.erase(it);
					break;
				}
			}

			if(channel.size() <= 0)
				return false;

			return true;
		}
	};

public:
	GroupSession(Context * c);

	 ~GroupSession();

	void DeleteGroupSession(int group, int channel);
	int AddGroupSession(int type, int id, int channel, int count, bool &isSuccess);

	int BindSession(int64 key, int group);
	void UnBindSession(int64 key, int group);

private:

	map<WORD, int> m_group;
	map<int, GroupSessionInfo> m_groupChannel;
	CRWLock m_groupLock;
	Context * m_c;
};
}

#endif /* NETWORK_GROUPSESSION_H_ */

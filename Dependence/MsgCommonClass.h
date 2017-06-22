/*
 * MsgCommonClass.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef MSGCOMMONCLASS_H_
#define MSGCOMMONCLASS_H_

#include "Common.h"
#include "Smart_Ptr.h"
#include "Network/MessageManager.h"
#include "define.h"

using namespace CommBaseOut;

class requestAct : public NullBase
{
public:
	requestAct(Safe_Smart_Ptr<CommBaseOut::Message> &message):mesReq(message)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
};

class FirstAct : public NullBase
{
public:
	FirstAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, int64 dataf):mesReq(message),mesDataf(dataf)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	int64 mesDataf;
};


class commonAct : public NullBase
{
public:
	commonAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, int64 dataf, int64 datas):mesReq(message),mesDataf(dataf),mesDatas(datas)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	int64 mesDataf;
	int64 mesDatas;
};

class ThreeAct : public NullBase
{
public:
	ThreeAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, int64 dataf, int64 datas, int64 datat):mesReq(message),mesDataf(dataf),mesDatas(datas),mesDatat(datat)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	int64 mesDataf;
	int64 mesDatas;
	int64 mesDatat;
};

class FourAct : public NullBase
{
public:
	FourAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, int64 dataf, int64 datas, int64 datat, string &datafo):mesReq(message),mesDataf(dataf),mesDatas(datas),mesDatat(datat),mesDatafo(datafo)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	int64 mesDataf;
	int64 mesDatas;
	int64 mesDatat;
	string mesDatafo;
};

class IntAct : public NullBase
{
public:
	IntAct(int64 data):mesData(data)
	{}

	int64 mesData;
};

class DoubleIntAct : public NullBase
{
public:
	DoubleIntAct(int64 dataf, int64 datas):mesDataf(dataf),mesDatas(datas)
	{}

	int64 mesDataf;
	int64 mesDatas;
};

class BoolAct : public NullBase
{
public:
	BoolAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, bool dataf):mesReq(message),mesDataf(dataf)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	bool mesDataf;
};

class StringIntAct : public NullBase
{
public:
	StringIntAct(Safe_Smart_Ptr<CommBaseOut::Message> &message, int64 dataf, int64 datas, string &datafo):mesReq(message),mesDataf(dataf),mesDatas(datas),mesDatafo(datafo)
	{}

	Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
	int64 mesDataf;
	int64 mesDatas;
	string mesDatafo;
};

#endif /* MSGCOMMONCLASS_H_ */

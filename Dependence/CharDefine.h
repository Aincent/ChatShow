/*
 * CharDefine.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef CHARDEFINE_H_
#define CHARDEFINE_H_

const int MAX_NAME_LEN = 16;

enum ECharSex
{
	eSexMale=0,
	eSexFemale,
};

enum ECharBaseStruct
{
	eBaseInfo = 0,
	eFriendInfo,
	eCharStructMax,
};

enum ECharAttrType
{
	eCharName = 0,
	eWchatName,
	eHeadPhoto,
	eCharSex,
	eCountry,
	eProvince,
	eCity,
	eCharSignature,
	eCharMoney,
	eCharTurnMoney,
	eQRCode,
	ePhone,
	eYear,
	eMonth,
	eDay,
	eCharAttrMax,
};

#endif /* CHARDEFINE_H_ */

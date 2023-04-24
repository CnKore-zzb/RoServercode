#ifndef _XTELEGRAM_H_
#define _XTELEGRAM_H_

#include "xDefine.h"
#include "SceneDefine.h"
#include "xPos.h"

enum TelegramTypeEnum
{
	TG_TYPE_NULL = -1,
	TG_TYPE_MOVE_TO_POS,
	TG_TYPE_ATTACK,
	TG_TYPE_GUIDE,
	TG_TYPE_FOLLOW_USER,
	TG_TYPE_STAY,
	TG_TYPE_CLEAR,
	TG_TYPE_ADD_TASK,

	TG_TYPE_MAX,
};

// Telegram sender type
enum TelegramSenderEnum
{
	TELE_SENDER_NULL = -1,
	TELE_SENDER_USER = SCENE_ENTRY_USER,
	TELE_SENDER_NPC = SCENE_ENTRY_NPC,
	TELE_SENDER_GM,
	TELE_SENDER_SCENE,
};

struct Telegram
{
	BYTE type;	//TelegramTypeEnum
	Telegram(const BYTE &_type) : type(_type) {}
	~Telegram() {}
};

struct DelayedTelegram : Telegram
{
	DWORD dispatchTime;
	BYTE senderType;	//TelegramSenderEnum
	QWORD senderID;
	BYTE receiverType;	//SCENE_ENTRY_USER 或 SCENE_ENTRY_NPC
	std::vector<QWORD> receivers;	//接收者的ID
	DelayedTelegram() : Telegram(TG_TYPE_NULL)
	{
		dispatchTime = 0;
		senderType = TELE_SENDER_NULL;
		senderID = 0;
		receiverType = SCENE_ENTRY_NPC;
	}
	inline void addReceiver(const QWORD &id) { receivers.push_back(id); }
};

struct MoveToPosTelegram : Telegram
{
	xPos pos;
	bool attackFlag;
	DWORD clearFlag;
	DWORD toRegion;
	MoveToPosTelegram() : Telegram(TG_TYPE_MOVE_TO_POS)
	{
		attackFlag = false;
		clearFlag = 0;
		toRegion = 0;
	}
};

struct AttackTelegram : Telegram
{
	QWORD id;
	AttackTelegram() : Telegram(TG_TYPE_ATTACK) 
	{
		id = 0;
	}
};

struct GuideTelegram : Telegram
{
	xPos pos;
	QWORD id;
	GuideTelegram() : Telegram(TG_TYPE_GUIDE) 
	{
		id = 0;
	}
};

struct FollowUserTelegram : Telegram
{
	QWORD userID;
	WORD mode;
	FollowUserTelegram() : Telegram(TG_TYPE_FOLLOW_USER) 
	{
		userID = 0;
		mode = 0;
	}
};

struct StayTelegram : Telegram
{
	DWORD time;
	bool attackFlag;
	StayTelegram() : Telegram(TG_TYPE_STAY) 
	{
		time = 0;
		attackFlag = true;
	}
};

struct ClearTelegram : Telegram
{
	ClearTelegram() : Telegram(TG_TYPE_CLEAR) 
	{
	}
};

struct AddTaskTelegram : Telegram
{
	std::vector<DWORD> newTaskIDs;
	AddTaskTelegram() : Telegram(TG_TYPE_ADD_TASK)
	{
	}
};

#endif

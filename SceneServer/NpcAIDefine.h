#pragma once

//	NPC巡逻方式
enum NpcPattrolEnum
{
	NPC_PATTROL_NULL,
	NPC_PATTROL_RANDOM,
	NPC_PATTROL_LINE,
	NPC_PATTROL_CIRCLE,
	NPC_PATTROL_GUARD,
	NPC_PATTROL_SPEED_RANDOM,
};

// NPC搜索方式
enum NpcSearchEnum
{
	NPC_SEARCH_NULL = 0,
	NPC_SEARCH_USER_ACTIVE = 1,	//主动搜索玩家
	NPC_SEARCH_NPC_ACTIVE = 2,	//主动搜索NPC(不同国家)
	NPC_SEARCH_ALL_ACTIVE = 3,	//主动搜索玩家和NPC(不同国家)
	NPC_SEARCH_NPC_USER = 4,	//优先搜索NPC再玩家(不同国家)
	NPC_SEARCH_USER_NPC = 5,	//优先搜索玩家再NPC(不同国家)
	NPC_SEARCH_PASSIVE = 6,	//被动搜索
	NPC_SEARCH_NORMAL_NPC_AND_FOLLOWER = 7,	//搜索普通NPC和属于玩家的NPC，但不包括宠物
  NPC_SEARCH_TEAM_ACTIVE = 8, // 主动搜索所属某个队伍的玩家
  NPC_SEARCH_PERSONAL_ACTIVE = 9, //只攻击某个玩家
};

//NPC被攻击模式
enum NpcBeAttackEnum
{
	NPC_BEATTACK_NULL = 0,
	NPC_BEATTACK_NOUSER = 1,	//不能被玩家攻击
	NPC_BEATTACK_NO_CIVIL_USER = 2,	//不能被本国玩家攻击
	NPC_BEATTACK_NO_FOREIGN_USER = 3,	//不能被外国玩家攻击
};

// NPC切换目标方式
enum NpcSwitchTargetEnum
{
	NPC_SWITCH_TARGET_RANDOM = 0,	//随机
	NPC_SWITCH_TARGET_CLOSEST = 1,	//距离最近者
	NPC_SWITCH_TARGET_LAST_ATTACKER = 2,	//最后攻击者
	NPC_SWITCH_TARGET_LOWEST_LEVEL = 3,		//等级最低者
	NPC_SWITCH_TARGET_HIGHEST_LEVEL = 4,	//等级最高者
	NPC_SWITCH_TARGET_LEAST_HP = 5,		//血量最少者
	NPC_SWITCH_TARGET_MOST_HP = 6,		//防御最低者
	NPC_SWITCH_TARGET_HIGHEST_DAM = 7,	//攻击力最高者(它造成了最大的伤害)
	NPC_SWITCH_TARGET_TEAM = 8,	// 帮助自己队友
};

const DWORD MAX_NPC_PURSUIT_TIME = 15;	//最大的NPC追逐时间(过了时间便切换目标)

// NPC的攻击标识
enum NpcAttackFlag
{
	NPC_ATTACK_NULL = 0, //不可攻击 
	NPC_ATTACK_CITY = 1, //不可被玩家攻击，可以被npc攻击 
	NPC_ATTACK_OTHER_CITY = 2, //可被城市以外人攻击 
	NPC_ATTACK_COUNTRY = 4, //可被国家人攻击 
	NPC_ATTACK_OTHER_COUNTRY = 8, //可被国家以外人攻击(npc所在场景所属国家以外的人可以攻击) 
	NPC_ATTACK_ZHONGLI = 16, //桃源人可攻击 
	NPC_ATTACK_WEI = 32, //魏国人可攻击 
	NPC_ATTACK_SHU = 64, //蜀国人可攻击  
	NPC_ATTACK_WU = 128, //吴国人可攻击
};


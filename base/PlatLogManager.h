#pragma once
/*
@brief:平台日志
*/
#include "xlib/xSingleton.h"
#include "LogCmd.pb.h"
#include "xServer.h"
#include "SceneManual.pb.h"
#include "GuidManager.h"
#include <sstream>
#include "xlib/json/json2pb.h"
#include "BaseConfig.h"

class xServer;
using namespace Cmd;

enum EVENT_TYPE {
  EventType_Income = 1,       //游戏币获得     (1+1)*100 +source     废弃
  EventType_Consume = 2,      //游戏币消耗     (2+1)*100 +source     废弃

  EventType_Item = 3,         //物品获取       (3+1)*100 +source    废弃 
  EventType_Props = 4,        //物品使用       (4+1)*100 +source    废弃

  EventType_Change_Skill = 5,   //升级技能等级
  EventType_Change_Move = 6,    //移动跨地图
  EventType_Change_Follow = 7,  //跟随跨地图
  EventType_Change_TeamGlobal = 8,  //更改队伍目标
  EventType_Change_TeamName = 9,   //更改队伍名称
  //EventType_Change_TeamLeader = 10,  //更改队伍队长     //废弃
  EventType_Change_GuildGlobal = 11, //更改公会目标
  EventType_Change_GuildJob = 12, //更改公会职业
  EventType_Change_GuildLeader = 13,  //更改公会会长
  EventType_Change_GuildLv = 14,  //更改公会等级

  EventType_EquipOn = 15,     //穿上装备
  EventType_EquipOff = 16,   //卸下装备

  EventType_CardOn = 17,     //装备插卡
  EventType_CardOff = 18,    //装备卸卡

  EventType_Hand = 19,        //牵手

  EventType_AddFriend = 25,   //添加好友
  EventType_GuildCreate = 26, //创建公会
  EventType_GuildApply = 27,  //申请加入公会
  EventType_GuildJoin = 28,   //加入公会
  EventType_GuildLeave = 29,  //离开公会
  EventType_GuildKick = 30,   //公会踢出玩家
  EventType_GuildMoney = 31,  //公会资金

  EventType_Quest_Start = 32,  //开始任务
  EventType_Quest_Stop = 33,//完成任务
  EventType_Quest_Summon = 34,//召唤npc
  EventType_Quest_VisitNpc = 35,//访问npc
  EventType_Quest_Reward = 36,//任务奖励

  EventType_Manual_Exp = 37,//冒险手册获得经验
  EventType_Manual_Title = 38,//冒险手册获得称号
  EventType_Manual_Skill = 39,//冒险手册获得技能

  EventType_Complete_Board = 40,//完成看板任务
  EventType_Complete_Seal = 41,//完成封印
  EventType_Complete_DailyQuest = 42,//完成抗击魔潮
  EventType_Complete_Laboratory = 43,//完成研究所

  EventType_PickItem = 44,// 拾取物品
  EventType_PlayMusic = 45,//使用唱片
  EventType_Kill = 46,//杀怪
  EventType_Tower_Complete = 47,//完成无限塔
  EventType_Tower_Leave = 48,//离开无限塔
  EventType_EquipStrength = 49, //装备强化
  EventType_EquipRefine = 50, //装备精炼

  EventType_Reward = 51, //Reward 
  EventType_Mail = 52,  //领取邮件
  EventType_Dojo = 53,  //道场通关
  EventType_Enchant = 54, //装备附魔
  EventType_GuildPray = 55, //公会祈祷
  EventType_GuildSetDismiss = 56,//发起解散
  EventType_GuildCancelDismiss = 57,//取消解散
  EventType_GuildDismiss = 58,  //公会解散

  EventType_Manual_PhotoMonster = 59, //冒险手册拍照魔物
  EventType_Manual_PhotoScenery = 60, //冒险手册拍照景点
  EventType_Manual_AchieveReward = 61,       //冒险手册成就奖励

  EventType_UseSkill = 62,          //使用技能
  EventType_Compose = 63,           //合成
  EventType_AddMail = 64,           //新增邮件
  EventType_Jumpzone = 65,          //跳线
  EventType_TempPackAdd = 66,       //临时背包获得
  EventType_TempPackSub = 67,       //临时背包被领取
  EventType_TempPackDismiss = 68,   //临时背包消失

  EventType_DelFriend = 69,         //删除好友
  EventType_Change_Attr = 70,       //属性点加点
  EventType_Change_ResetSkillPoint = 71,  //重置技能点
  EventType_Change_ResetAttrPoint = 72,   //重置属性点

  EventType_UserRename = 73, // 角色道具
  EventType_GuildRename = 74, // 公会改名

  EventType_Team_Create = 2000,     //创建队伍
  EventType_Team_Join = 2001,       //加入队伍
  EventType_Team_Apply = 2002,      //申请加入队伍
  EventType_Team_Exit = 2003,       //退出队伍
  EventType_Team_Kick = 2004,       //踢出队伍
  EventType_Team_Dismiss = 2005,    //解散队伍
  EventType_Team_ChangeLeader = 2006,       //队伍更好队长
  EventType_Team_ChangeTempLeader = 2007,  //队伍更好临时队长

  EventType_Pet_Change = 2100,      //宠物变化
  EventType_Pet_Adventure = 2101    //宠物冒险

  
  //1000 etype 根据esource确定
  //1001 通过默认
  //1002 通过背包
  //1003 通过任务
  //1004 通过装备
  //1005 通过卡片
  //1006 通过主动技能
  //1007 通过被动技能
  //1009 通过合成
  //1010 通过奖励
  //1011 通过怪物死亡
  //1012 通过GM
  //1013 通过副本
  //1014 通过充值
  //1015 通过研究所
  //1016 通过地图传送
  //1017 通过理发点
  //1018 通过强化
  //1019 通过商店购买
  //1021 通过商店出售
  //1022 通过拾取
  //1024 通过base升级
  //1025 通过交易所
  //1026 通过装备继承
  //1027 通过封印
  //1028 通过公会祈祷
  //1029 通过附魔
  //1030 通过公会创建
  //1031 通过公会升级
  //1032 通过复活
  //1033 通过装备修理
  //1034 通过仓库
  //1035 通过音乐盒
  //1036 通过无限塔
  //1037 通过道场首通
  //1038 通过道场协通
  //1039 通过冒险手册
  //1040 通过技能返回
  //1041 通过掠夺证
  //1042 通过看板任务
  //1043 通过捐赠
  //1044 通过怪物ai
  //1045 通过北森寻宝
  //1046 通过摩天轮
  //1047 通过白幽灵事件
  //1048 通过切线
  //1049 通过运营活动
  //1050 通过单身狗活动
  //1051 通过礼包码
  //1052 通过问答活动
  //1053 通过分解
  //1054 通过帮助
  //1055 通过开仓库
  //1056 通过存仓库
  //1057 通过出仓库
  //1058 通过装备置换
  //1059 通过强化返回
  //1060 通过抗击魔潮
  //1061 通过主动使用道具
  //1069 通过存个人仓库
  //1070 通过出个人仓库
  //1071 通过存公共仓库
  //1072 通过出公共仓库
};

enum ELEVEL_TYPE {
  ELevelType_BaseLv = 1,
  ELevelType_JobLv = 2,
  ELevelType_ManualLv = 3
};

#define   EChangeFlag_Profession  "profession"
#define   EChangeFlag_User_Name   "user_name"
#define   EChangeFlag_Guild_Name  "guild_name"
#define   EChangeFlag_Safe_PWD    "safe_pwd"

enum ECHANGE_TYPE {
  EChange_Skill           = 1,
  EChange_Move            = 2,
  EChange_Follow          = 3,
  EChange_TeamGlobal      = 4,
  EChange_TeamName        = 5,
  EChange_TeamLeader      = 6,
  EChange_GuildGlobal     = 7,
  EChange_GuildJob        = 8,
  EChange_GuildLeader     = 9,
  EChange_GuildLv         =10,
  EChange_Attr            =11,
  EChange_ResetSkillPoint =12,
  EChange_ResetAttrPoint  =13,
};

enum EEQUIP_TYPE {
  EEquip_EquipOn = 1,
  EEquip_EquipOff = 2,
  EEquip_Inherait = 3,      //装备继承
  EEquip_Strength = 4,
  EEquip_Refine = 5,
};

enum ESOCIAL_TYPE {
  ESocial_Hand        = 1,    //牵手
  ESocial_TeamCreate  = 2,    //队伍创建
  ESocial_TeamApply   = 3,    //队伍申请
  ESocial_TeamJoin    = 4,    //队伍加入
  ESocial_TeamLeave   = 5,    //离开队伍
  ESocial_TeamKick    = 6,    //剔出队员
  ESocial_AddFriend   = 7,    //添加好友
  ESocial_GuildCreate = 8,    //剔出队员
  ESocial_GuildApply  = 9,    //
  ESocial_GuildJoin   = 10,   //
  ESocial_GuildLeave  = 11,   //
  ESocial_GuildKick   = 12,   //踢出会员
  ESocial_GuildMoney  = 13,    //公会资金
  ESocial_GuildSetDismiss = 14,//发起解散
  ESocial_GuildCancelDismiss = 15,//取消解散
  ESocial_GuildDismiss = 16,   //公会解散
  ESocial_DelFriend = 17,      //删除好友
  ESocial_GuildRename = 18,    //公会改名
};

enum EQUEST_TYPE {
  EQuestType_START = 1,
  EQuestType_STOP = 2,
  EQuestType_SUMMON = 3,
  EQuestType_VISITNPC = 4,
  EQuestType_REWARD = 4,
};

enum EMANUAL_TYPE {
  EManual_Exp = 1,        //冒险经验
  EManual_Title = 2,      //冒险称号
  EManual_Skill = 3,       //冒险技能
  EManual_PhotoMonster = 4, //拍照魔物
  EManual_PhotoScenery = 5, //拍照景点
  EManual_AchieveReward = 6     //成就奖励
};

enum ECOMPLETE_TYPE {
  ECompleteType_Board = 1,          //看板任务
  ECompleteType_Tower = 2,          //无限塔
  ECompleteType_Seal = 3,           //封印
  ECompleteType_DailyQuest = 4,     //抗击魔潮
  ECompleteType_Laboratory = 5      //研究所
};

enum EITEMOPER_TYPE {
  EItemOperType_Pick = 1,
  EItemOperType_PlayMusic = 2,
};

enum EDOJOPASS_TYPE {
  EDOJOPASSTYPE_FIRST = 1,
  EDOJOPASSTYPE_HELP = 2,
  EDOJOPASSTYPE_NORMAL = 3,
};

enum ETRADE_TYPE {
  ETRADETYPE_NONE = 0,              //上架
  ETRADETYPE_SELL = 1,              //上架
  ETRADETYPE_CANCEL = 2,            //撤单下架
  ETRADETYPE_TRUEBUY = 3,           //成功购买
  ETRADETYPE_TRUESELL = 4,          //成功出售
  ETRADETYPE_RESELL = 5,            //重新上架
  ETRADETYPE_PUBLICITY_SELL = 6,    //公示上架
  ETRADETYPE_RESELL_AUTO = 7,       //公示后自动上架
  ETRADETYPE_PUBLICITY_SEIZURE = 8,           //公示期扣押钱
  ETRADETYPE_PUBLICITY_RETURN = 9,            //公示期返回钱
  ETRADETYPE_PUBLICITY_BUY = 10,              //公示期购买成功
  ETRADETYPE_TAKE = 11,                       //交易所领取 废弃
  ETRADETYPE_TAKE_SELL_MONEY = 12,            //领取出售所得银币
  ETRADETYPE_TAKE_BUY_ITEM = 13,              //领取购买所得道具
  ETRADETYPE_TAKE_RETURN_MONEY = 14,          //领取公示期返还所得银币
  ETRADETYPE_TAKE_PUBLICITY_BUY_ITEM = 15,    //领取公示期所得道具
  ETRADETYPE_TAKE_PUBLICITY_SELL_MONEY = 16,  //领取公示期出售所得银币
  ETRADETYPE_AUTO_OFFSHELF = 17,  //不可交易自动下架
  ETRADETYPE_BOOTH_SELL                      = 18, //上架
  ETRADETYPE_BOOTH_CANCEL                    = 19, //撤单下架
  ETRADETYPE_BOOTH_TRUE_BUY                  = 20, //成功购买
  ETRADETYPE_BOOTH_TRUE_SELL                 = 21, //成功出售
  ETRADETYPE_BOOTH_RESELL                    = 22, //重新上架
  ETRADETYPE_BOOTH_PUBLICITY_SELL            = 23, //公示上架
  ETRADETYPE_BOOTH_RESELL_AUTO               = 24, //公示后自动上架
  ETRADETYPE_BOOTH_PUBLICITY_SEIZURE         = 25, //公示期扣押钱
  ETRADETYPE_BOOTH_PUBLICITY_RETURN          = 26, //公示期返回钱
  ETRADETYPE_BOOTH_PUBLICITY_BUY             = 27, //公示期购买成功
  ETRADETYPE_BOOTH_PUBLICITY_TRUE_SELL       = 28, //公示期出售成功
  ETRADETYPE_BOOTH_TAKE                      = 29, //交易所领取
  ETRADETYPE_BOOTH_TAKE_SELL_MONEY           = 30, //领取出售所得银币
  ETRADETYPE_BOOTH_TAKE_BUY_ITEM             = 31, //领取购买所得道具
  ETRADETYPE_BOOTH_TAKE_RETURN_MONEY         = 32, //领取公示期返还所得银币
  ETRADETYPE_BOOTH_TAKE_PUBLICITY_BUY_ITEM   = 33, //领取公示期所得道具
  ETRADETYPE_BOOTH_TAKE_PUBLICITY_SELL_MONEY = 34, //领取公示期出售所得银币
};

enum EMONSTER_TYPE {
  EMONSTERTYPE_MONSTER = 1,
  EMONSTERTYPE_MINI = 2,
  EMONSTERTYPE_MVP = 3,
};

#define LOGID(log, zoneid)   \
   std::string logGuid =  GuidManager::getMe().newGuidStr(zoneid, 0);        \
   log.set_logid(logGuid)

#define LOGBUFF(log)\
  try   \
  {   \
    std::string jsonStr = pb2json(log);   \
    std::stringstream log_ss;     \
    log_ss.str(""); \
    log_ss << "[" << now() << "," << jsonStr << "]";   \
    addLog(log.param(), log_ss.str());  \
  } \
  catch (...) \
  { \
    XERR << "[fluent-日志] 发送：param:" << log.param() << log.ShortDebugString() << "转出json失败捕获到异常" << XEND; \
  } 
#define EXPIRETIME            5 * 60
#define MAX_SEND_CACHE_SIZE   100000
#define MAX_UNSEND_SIZE       100000 
//#define MAX_LOG_COUNT         500
//#define MAX_RETRY_COUNT       4

struct AckCache {
  DWORD dwExpireTime = 0;
  string log;
  DWORD dwRetryCount = 0;
};

class LogProcesser :private xNoncopyable
{ 
public:
  void setTag(Cmd::LogParam cmd, const string& tag) { m_dwCmd = cmd; m_strTag = tag; }

  void sendLog();
  void recvAck(QWORD ack);
  void addLog(const string& log);
  void processRetry(DWORD curSec);
private:
  void concatLog(string& log);
private:
  std::string m_strTag;
  DWORD m_dwCmd = 0;
  QWORD m_qwKey = 0;
  std::list<string> m_listUnsend;
  std::map<QWORD, AckCache> m_mapAckCache;
  std::map<QWORD, string> m_mapRetry;
  DWORD m_dwNextRetryTime = 0;
};

class PlatLogManager : public xSingleton<PlatLogManager>
{
  friend class xSingleton<PlatLogManager>;

public:
  virtual ~PlatLogManager() {}
  void init(xServer* pServer);
private:
  PlatLogManager();

public:  
  /*登录、登出日志*/
  void loginLog(xServer* pServer, DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    const std::string& ip,
    DWORD dwCharge,
    int logType,
    DWORD dwRoleLv,
    const std::string& strSign, const std::string& strDevice, int isGuest, const std::string& strMac, const std::string& strAgent,
    DWORD dwMapId,
    DWORD dwOnlineTime,
    DWORD dwTeamTimeLen,
    bool isNew);

  /*角色创建日志*/
  void createCharLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    const std::string& strName,
    int isGuest,
    const std::string& ip,
    const std::string& strDevice,
    const std::string& strMac,
    const std::string& strAgent, 
    DWORD dwGender,
    DWORD dwHair,
    DWORD dwHairColor);

  /*角色删除日志*/
  void deleteCharLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId);

  /*玩家属性变化日志*/
  template<typename T>
  void changeFlagLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwCharId,
    const std::string& flag, T from, T to, 
    QWORD qwParam1 = 0)
  {
    if (pServer == nullptr)
      return;

    std::stringstream ss1;
    ss1 << from;
    std::stringstream ss2;
    ss2 << to;
    changeFlagLog(pServer, dwPlatFormId, dwZoneId, dwCharId, flag, ss1.str(), ss2.str(), qwParam1);
  }

  /*充值日志*/
  void chargeLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    DWORD dwRoleLv,
    float dwValue,
    const std::string& strName,
    DWORD dwCreateTime,
    DWORD dwItemId,
    DWORD dwCoins,
    const std::string strOrderId,
    const std::string strType,
    const std::string strCurrency,
    const std::string strProvider,
    const std::string strIp,
    const std::string strDevice);

  /*玩家操作事件日志*/
  void eventLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD dwType,
    DWORD dwSubType,
    DWORD dwCount);
  /*玩家操作事件日志*/
  void eventLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    string strAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD dwType,
    DWORD dwSubType,
    DWORD dwCount);

  /*游戏币获取日志*/
  void incomeMoneyLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD coinType,
    QWORD dwValue,
    QWORD dwAfter,
    DWORD dwSource);

  /*游戏币消耗日志*/
  void outcomeMoneyLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD coinType,
    QWORD dwValue,
    QWORD dwAfter,
    DWORD dwSource,
    QWORD qwChargeMoney,
    QWORD qwLeftChargeMoney);

  /*物品获取日志*/
  void gainItemLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD dwItemId,
    DWORD dwValue,
    DWORD dwAfter,
    DWORD dwSource,
    DWORD dwEventType =0);

  /*物品使用日志*/
  void consumeItemLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwCharge,
    DWORD dwItemId,
    DWORD dwValue,
    DWORD dwAfter,
    DWORD dwSource,
    DWORD dwEventType =0,
    const std::string& iteminfo = std::string());


  /*聊天日志 带语音的*/
  void chatLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    const std::string strSenderName,
    QWORD dwCharId,
    QWORD qwRecvAccid,
    const std::string strRecvName,
    QWORD qwRecvCharid,
    DWORD type,
    DWORD dwCharge,
    const std::string strContent,
    DWORD dwRoleLv,
    DWORD voiceTime = 0);

  /*等级升级日志*/
  void levelUpLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    DWORD dwFromLv,
    DWORD dwToLv,
    DWORD dwCharge,
    ELEVEL_TYPE type,
    DWORD dwCostTime = 0);

  /*vip 升级日志 暂无*/
  void vipLvUpLog() {}

  /*在线玩家数*/
  void onlineCountLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    DWORD dwMinTime,
    DWORD dwTotalCount,
    DWORD dwAndroidCount,
    DWORD dwClientCount,
    DWORD dwIosCount);

  /*每月付费玩家快照接口 暂无*/

  /*关卡通过日志*/
  void levelPassLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwCharId,
    QWORD qwEid,
    DWORD dwMapId,
    DWORD dwCharge,
    DWORD dwIsFirst);

  /*各种排行日志 暂无*/

  
  //------------
  /*变化日志*/
  template<typename T>
  void changeLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    ECHANGE_TYPE flag, T from, T to,
    QWORD qwParam1 = 0)
  {
    if (pServer == nullptr)
      return;

    std::stringstream ss1;
    ss1 << from;
    std::stringstream ss2;
    ss2 << to;
    changeLog(pServer, dwPlatFormId, dwZoneId, dwAccid,dwCharId, eType, qwEid, flag, ss1.str(), ss2.str(), qwParam1);
  }

  /*装备日志*/
  void EquipLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD type,           //1：穿上 2：脱下  3:transfer
    DWORD dwOldEquipId,
    const std::string& oldEquipGuid,
    DWORD dwOldStrengthLv,
    DWORD dwOldRefineLv,
    bool bOldIsDamanged,
    DWORD dwNewEquipId,
    const std::string& newEquipGuid,
    DWORD dwNewStrengthLv,
    DWORD dwNewRefineLv,
    bool bNewIsDamanged);

  /*插卡日志*/
  void CardLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwEquipId,
    const std::string& equipGid,
    DWORD type,   //1:on 2：off
    DWORD dwCardId,
    const std::string& cardGid,
    DWORD dwUseslot,
    DWORD dwMaxSlot);

  /*装备加强日志*/
  void EquipUpLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD type,           //1：强化 2：精炼 
    DWORD count,
    QWORD qwEquipId,
    const std::string& equipGid,
    DWORD dwOldLv,
    DWORD dwNewLv,
    bool bIsFail,
    std::string costMoney,
    std::string costItem,
    bool bIsDamanged );

  /*social日志*/
  void SocialLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    ESOCIAL_TYPE type,
    QWORD qwInId,
    QWORD qwAnotherId,
    QWORD qwParam1,
    QWORD qwParam2);

  /*任务日志*/
  void QuestLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    QWORD qwQuestId,
    EQUEST_TYPE type,
    QWORD qwTargetId,
    DWORD dwBaseExp,
    DWORD dwJobExp,
    std::string& rewardItem,
    DWORD dwLevel);

  /*冒险手册日志*/
  void ManualLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    EMANUAL_TYPE type,
    DWORD by,
    QWORD qwWhat,
    QWORD qwParam1);

  /*完成活动相关日志*/
  void CompleteLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    ECOMPLETE_TYPE type,
    QWORD qwTargetId,
    DWORD dwTodayCount,
    DWORD dwRewardType,
    DWORD dwRewardCount,
    DWORD dwLevel);

  /*完成活动相关日志*/
  void TowerLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD type,         //1：完成  2：离开
    DWORD dwCurLayer,
    DWORD dwMaxLayer,
    QWORD qwTeamId,
    DWORD dwLevel);

  /*物品操作相关日志*/
  void ItemOperLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    EITEMOPER_TYPE type,
    DWORD dwItemid,
    DWORD dwCount
  );

  /*杀怪*/
  void KillLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwMonsterId,
    QWORD qwMonsterGid,
    DWORD dwMonsterGroup,
    DWORD dwBaseExp,
    DWORD dwJobExp,
    bool isMvp,
    EMONSTER_TYPE type,
    DWORD dwLv,
    DWORD killType /*1:monster die, 2:user die*/
  );

  /*reward*/
  void RewardLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwId,
    DWORD dwProfession,
    const string& rewarditem
  );

  /*mail*/
  void MailLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    QWORD qwId,
    QWORD qwSysid,
    DWORD dwMailType,
    const string& title,
    const string& rewarditem
  );

  /*通关道场*/
  void DojoLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dojoId,
    DWORD dwMapId,
    EDOJOPASS_TYPE passType,
    DWORD dwLevel);

  /*附魔*/
  void EnchantLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    const string& equipGuid,
    DWORD dwItemId,
    DWORD enchantType,
    const string& oldAttr, 
    const string& newAttr,
    const string& oldBuffid,
    const string& newBuffid,
    DWORD dwCostItemId,
    DWORD dwItemCount,
    DWORD dwMoney
  );


  /*公会祈祷*/
  void GuildPrayLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwPrayId,
    DWORD dwAddattr, /*祈祷后的等级*/
    DWORD dwCostItem,
    DWORD dwCostMoney,
    DWORD dwCostCon
  );

  /*使用技能*/
  void UseSkillLog(xServer* pServer,
    DWORD dwPlatFormId,
    DWORD dwZoneId,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwSkillID
  );

  /*交易*/
  void TradeLog(xServer* pServer,
    DWORD dwPlatFormId,
    QWORD dwCharId,
    ETRADE_TYPE type,
    DWORD dwItemId,
    DWORD dwCount,
    DWORD dwPrice,
    DWORD dwTax,
    DWORD moneyCount,
    string& itemInfo,
    QWORD otherId = 0
  );
  
  /*money item 来源转换成etype*/
  DWORD EventTypeConvert(DWORD source) {    
    return 1000 + source;
  }

  /*合成*/
  void ComposeLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwItemid,
    std::string itemGuid,
    std::string cost
  );

  /*跳线*/
  void JumpzoneLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    DWORD dwOldZoneid,
    DWORD dwNewZoneid,
    DWORD dwIsFirst,
    std::string cost
  );

  /*Team日志*/
  void TeamLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    QWORD qwInId,
    QWORD qwAnotherId);

  void PetChangeLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,

    EPetChangeType eChangeType,
    DWORD dwMonsterId,
    string& strName,
    DWORD dwBefore,
    DWORD dwAfter,
    string& strBefore,
    string& strAfter);

  void PetAdventureLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    EVENT_TYPE eType,
    QWORD qwEid,
    
    EPetAdventureLogType eAdventureType,
    DWORD dwId,
    string strNames,
    DWORD dwCond
    );
  void InactiveUserLog(xServer* pServer,
    QWORD dwAccid,
    QWORD dwCharId,
    const string& name,
    DWORD dwJob,
    DWORD dwLevel,
    QWORD qwLeftZeny,
    DWORD dwMapid,
    QWORD qwGuildId,
    DWORD dwCreateTime,
    DWORD dwSendCount    
  );

  void TradeUntakeLog(xServer* pServer,
    DWORD dwPlatFormId,
    QWORD dwCharId,
    const std::string& strName,
    QWORD qwZeny,
    const std::string& strGuildName
  );

  void CreditLog(xServer* pServer,
    QWORD qwCharId,
    const std::string& strName,
    Cmd::ECreditType type,
    QWORD qwBefore,
    QWORD qwAfter
  );

  void TradeGiveLog(xServer* pServer,
    QWORD dwCharId,
    Cmd::EGiveEvent event, 
    DWORD dwItemId,
    QWORD qwQuota,
    const std::string& itemInfo,
    QWORD qwOtherCharId,
    const std::string& strName,
    const std::string& strOtherName,
    DWORD givetime,
    Cmd::ELogGiveType type,
    DWORD dwItemCount
  );

  /* 额度变化 */
  void QuotaLog(xServer* pServer,
                QWORD qwAccid,
                QWORD qwCharId,
                EQuotaOptType eOptType,
                EQuotaType eQuotaType,
                QWORD qwChanged,
                QWORD qwQuota,
                QWORD qwLock
    );

  /* 公会道具变化 */
  void GuildItemLog(xServer* pServer,
                    QWORD qwGuildId,
                    DWORD dwItemId,
                    SQWORD sqwChanged,
                    DWORD dwCount,
                    DWORD dwSource
    );


  std::string generateKey(DWORD platform, QWORD charId, DWORD itemid, DWORD source);
  
  void timeTick(DWORD curSec);
  bool sendLog(const string& log, bool retry=false, bool bLog = false);
  bool doCmd(BYTE* buf, WORD len);
  LogProcesser* getLogProcesser(DWORD cmd);
  private:
    void reg(Cmd::LogParam cmd, const string& tag);
    void addLog(Cmd::LogParam cmd, const string&log);
    void processItem();
  private:
    xTimer m_oFiveSecTimer;
    xTimer m_oOneMinTimer;

    std::map<DWORD, LogProcesser*> m_mapLogProcesser; 
    xServer *m_pServer = nullptr;

    std::map<string, ConsumeLogCmd> m_mapConsumeLog;
    std::map<string, IncomeLogCmd> m_mapIncomeLog;
    std::map<string, ItemLogCmd> m_mapItemLog;
    std::map<string, PropsLogCmd> m_mapPropsLog;
};

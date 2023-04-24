#pragma once
/************************************************************************/
/* 统计程序宏                                                                    */
/************************************************************************/
#include "xDefine.h"
#include "xServer.h"
#include "StatCmd.pb.h"
#include <bitset>
  
#define SKILL_POINT_REST      1       //重置技能点
#define ATTR_POINT_REST       2       //重置属性点

enum STAT_TYPE {
  ESTATTYPE_BEGIN = 1,
  ESTATTYPE_MONSTER_KILL = 1,           //怪物击杀次数
  ESTATTYPE_MONSTER_ITEM_REWARD = 2,    //怪物物品掉落次数*衰减
  ESTATTYPE_MONSTER_EXP_REWARD = 3,     //怪物经验掉落次数*衰减
  ESTATTYPE_MAX_ATTR = 4,               //level skey:属性
  ESTATTYPE_AVG_ATTR = 5,               //level skey：属性 subkey：职业

  ESTATTYPE_MIN_TRADE_PRICE = 6,        //某个物品交易所购买最低价格，不分等级
  ESTATTYPE_AVG_TRADE_PRICE = 7,        //某个物品交易所购买平均价格，和交易次数 不分等级
  ESTATTYPE_MAX_TRADE_PRICE = 8,        //某个物品交易所购买最高价格，不分等级  
  ESTATTYPE_TRADE_SELL = 9,             //某个物品交易所上架数量（默认都是分等级）
  ESTATTYPE_TRADE_BUY = 10,             //某个物品交易所购买数量
  ESTATTYPE_WANT_QUEST_COUNT = 11,      //看板总次数
  ESTATTYPE_SEAL_COUNT = 12,            //封印总次数
  ESTATTYPE_DAILY_COUNT = 13,           //抗魔总次数
  ESTATTYPE_YJS_TRY_COUNT = 14,         //研究所挑战总次数

  ESTATTYPE_YJS_PASS_COUNT = 15,        //研究所通关总次数
  ESTATTYPE_ROB_COUNT = 16,             //掠夺总怪物数
  ESTATTYPE_NORMAL_KILL_COUNT = 17,     //野外普通怪物击杀总数
  ESTATTYPE_MINI_KILL_COUNT = 18,       //mini怪击杀总数
  ESTATTYPE_MVP_KILL_COUNT = 19,        //mvp怪击杀总数
  ESTATTYPE_REFINE_USER_COUNT = 20,     //精炼人数， level, 每个人之推送一次 sum   MODIFY
  ESTATTYPE_REFINE_COUNT = 21,          //精炼总次数
  ESTATTYPE_REFINE_SUCCESS_COUNT = 22,  //精炼成功总次数
  ESTATTYPE_REFINE_FAIL_COUNT = 23,     //精炼失败总次数
  ESTATTYPE_REFINE_DAMAGE_COUNT = 24,   //精炼损坏总次数

  ESTATTYPE_REPAIR_COUNT = 25,          //精炼修理总次数
  ESTATTYPE_REFINE_LEVEL_COUNT = 26,    //精炼后每个等级的装备个数，key 装备的guid  
  ESTATTYPE_ENCHANT_USER_COUNT = 27,    //附魔人数，level, 每个人之推送一次 sum subkey：附魔等级  TODO
  ESTATTYPE_ENCHANT_COUNT = 28,         //每个附魔等级附魔总次数，key：附魔等级     
  ESTATTYPE_TOWER_COUNT = 29,           //无限塔每层通过次数总和，key：0  subkey: 层数  按周统计   TODO
  ESTATTYPE_DOJO_COUNT = 30,            //道场每层通过人数，     subkey: dojoid  sum
  ESTATTYPE_ITEM_COUNT = 31,            //每个物品获得数量，key：itemid  subkey:source
  ESTATTYPE_PROPS_COUNT = 32,           //每个物品消耗数量，key：itemid  subkey:source
  ESTATTYPE_INCOME_COUNT = 33,          //每个钱币获得数量，key：moneytype  subkey:source
  ESTATTYPE_CONSUME_COUNT = 34,         //每个钱币消耗数量，key：moneytype  subkey:source

  //替换 ESTATTYPE_ONLINE_INFO = 35,           //每个在线玩家，key：charid subkey:职业 value1:在线时长， 人数都在此统计  //sum
  ESTATTYPE_DIE_COUNT = 36,             //玩家死亡次数 总和 TODO
  ESTATTYPE_TEAM_TIME = 37,             //玩家的组队时间时长，总和。总和/总在线人数 = 平均   TODO
  ESTATTYPE_NANMEN_TIME = 38,           //玩家在南门的时长，总和。 总和/总在线人数 = 平均    TODO
  ESTATTYPE_CREATE_GUILD_COUNT = 39,    //玩家创建公会的次数，总和。 TODO
  ESTATTYPE_JOIN_GUILD_COUNT = 40,      //玩家加入公会次数，总和  TODO
  ESTATTYPE_CHAT_COUNT = 41,            //玩家聊天的次数，总和。key:聊天类型  TODO 
  ESTATTYPE_PHOTO_COUNT = 42,           //玩家拍照的次数，总和。   TODO
  ESTATTYPE_DANMU_COUNT = 43,           //玩家弹幕的次数，总和。   TODO
  ESTATTYPE_CARIIER_COUNT = 44,         //玩家载具乘坐的次数，总和 
  ESTATTYPE_SKILL_LEARN = 45,           //每个玩家学的技能， key:0 subkey: (skillid/100)*1000 + sourceid, subkey2:0：不在技能栏1:手动2：自动 level:skill level     value1:1 每日只推送一个  TODO
//替换  ESTATTYPE_RESET_POINT = 46,           //重置技能或者素质点的人数， key:charid subkey:1：技能点 2：素质点   subkey2:profession  value1:1
//替换  ESTATTYPE_HEAL_COUNT = 47,            //艾莉儿加血的总次数和人数， key:charid  value1:1  求和
//替换  ESTATTYPE_STRENGTH = 48,              //强化， key:charid, subkey:装备类型  value1:1  次数
  ESTATTYPE_REMAIN_ZENY_SUM = 49,       //玩家下线时总的剩余zeny币数量， 求和，value1: 银币数量 TODO
  //new
  ESTATTYPE_ONLINE_COUNT = 50,          //每日在线玩家个数,  level key:0 subkey:职业
  ESTATTYPE_ONLINE_TIME = 51,           //每日总在线时间     level

  ESTATTYPE_RESET_POINT = 55,           //重置技能或者素质点的人数， key:charid subkey:1：技能点 2：素质点   subkey2:profession  value1:1
  ESTATTYPE_RESET_POINT_SUM = 56,       //重置技能或者素质点的总次数， key:charid subkey:1：技能点 2：素质点   subkey2:profession  value1:1

 // ESTATTYPE_RESET_SKILL_POINT_USER_COUNT = 55,      //技能洗点总人数， subkey2:profession  value1:1  一天推送一次 TODO
  //ESTATTYPE_RESET_ATTR_POINT_USER_COUNT = 56,       //属性洗点总人数， subkey2:profession  value1:1  一天推送一次 TODO
//  ESTATTYPE_RESET_SKILL_POINT_SUM = 57,             //技能洗点总数， subkey2:profession  value1:1  TODO
//  ESTATTYPE_RESET_ATTR_POINT_SUM = 58,              //属性洗点总数， subkey2:profession  value1:1  TODO
  ESTATTYPE_HEAL_COUNT_USER_COUNT = 59,             //被艾利尔加血总人数
  ESTATTYPE_HEAL_COUNT_SUM = 60,                    //被艾利尔加血总次数
  ESTATTYPE_STRENGTH_USER_COUNT = 61,               //强化总人数
  ESTATTYPE_STRENGTH_SUM = 62,                      //强化总次数

  ESTATTYPE_EQUIP_REFINE_DAMAGE_COUNT = 63,        //某个装备精炼损坏总次数  key：itemid
  ESTATTYPE_EQUIP_COMPOSE_COUNT = 64,             //某个装备制作总次数  key：itemid
  ESTATTYPE_EQUIP_EXCHANGE_COUNT = 65,            //某个装备打洞消耗总次数  key：itemid

  ESTATTYPE_VISIT_CAT_COUNT = 66,            //点击招财猫次数 key：itemid
  ESTATTYPE_VISIT_CAT_USER_COUNT = 67,            //点击招财猫人数 key：userid
  ESTATTYPE_SKILL_DAMAGE_USER = 68,                //key：skillid  subkey:damage value1:人数
  ESTATTYPE_SKILL_DAMAGE_MONSTER = 69,                //key：skillid  subkey:damage value1:人数
  ESTATTYPE_FASHION = 70,                     //key：itemid  value1:装备了，1 value2:存量数，1
  ESTATTYPE_MAX_KILL_MONSTER = 71,            //统计一天中一分钟杀怪的最大数量
  ESTATTYPE_END = 72,
};

class StatisticsDefine
{
public:
  static void sendStatLog(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, DWORD level, QWORD value1, bool isFloat =false)
  {
    if (pServer == nullptr)
      return;
    Cmd::StatCmd log;
    //1-10级合并为10级
    DWORD lv = 0;
    if (level > 0)
      lv = (((level - 1) / 10) + 1) * 10;
    log.set_type(type);
    log.set_key(key);
    log.set_subkey(subKey);
    log.set_level(lv);
    log.set_value1(value1);
    if (isFloat)
      log.set_isfloat(true);
    else
      log.set_isfloat(false);

    PROTOBUF(log, send, len);
    pServer->sendCmd(ClientType::stat_server, (BYTE*)send, len);
  }

  static void sendStatLog2(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, DWORD value1, bool isFloat = false)
  {
    if (pServer == nullptr)
      return;
    Cmd::StatCmd log;
    //1-10级合并为10级
    DWORD lv = 0;
    if (level > 0)
      lv = (((level - 1) / 10) + 1) * 10;
    log.set_type(type);
    log.set_key(key);
    log.set_subkey(subKey);
    log.set_subkey2(subKey2);
    log.set_level(lv);
    log.set_value1(value1);
    if (isFloat)
      log.set_isfloat(true);
    else
      log.set_isfloat(false);

    PROTOBUF(log, send, len);
    pServer->sendCmd(ClientType::stat_server, (BYTE*)send, len);
  }


  static void sendStatLog(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, DWORD level, float value)
  {
    QWORD qwValue = QWORD(value * 1000);
    if (pServer == nullptr)
      return;
    sendStatLog(pServer, type, key, subKey, level, qwValue, true);
  }

  static void sendStatLog(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, DWORD level, DWORD value)
  {
    QWORD qwValue = QWORD(value);
    if (pServer == nullptr)
      return;
    sendStatLog(pServer, type, key, subKey, level, qwValue, false);
  }

  static void sendStatLog2(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, float value)
  {
    QWORD qwValue = QWORD(value * 1000);
    if (pServer == nullptr)
      return;
    sendStatLog2(pServer, type, key, subKey, subKey2, level, qwValue, true);
  }    

  static void sendStatLog4(xServer* pServer, STAT_TYPE type, QWORD key, QWORD subKey, DWORD level, QWORD value1, QWORD value2)
  {
    if (pServer == nullptr)
      return;
    Cmd::StatCmd log;
    //1-10级合并为10级
    DWORD lv = 0;
    if (level > 0)
      lv = (((level - 1) / 10) + 1) * 10;
    log.set_type(type);
    log.set_key(key);
    log.set_subkey(subKey);
    log.set_level(lv);
    log.set_value1(value1);
    log.set_value2(value2);
    log.set_isfloat(false);
    PROTOBUF(log, send, len);
    pServer->sendCmd(ClientType::stat_server, (BYTE*)send, len);
  }

};

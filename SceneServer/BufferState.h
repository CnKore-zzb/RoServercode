/**
 * @file BufferState.h
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#pragma once

#include "xDefine.h"
#include <map>
#include <set>
#include <memory>
#include "xPos.h"
#include "BuffCondition.h"
#include "BufferManager.h"
#include "Item.h"
#include "xSceneEntry.h"
#include "SceneItem.pb.h"
#include "ProtoCommon.pb.h"
#include "SceneMap.pb.h"
#include <list>
#include <Package.h>
#include "LuaManager.h"
#include <queue>
#include "Transform.h"
#include "SkillStatus.h"
#include "RewardConfig.h"

using std::set;
using std::vector;
using std::pair;
using std::queue;

namespace Cmd
{
  class BlobBuffer;
};
class xSceneEntryDynamic;
class SceneUser;
class SceneNpc;
class BaseSkill;
struct SBufferData;
typedef std::set<xSceneEntryDynamic*> TSetSceneEntrys;
typedef std::map<EAttrType, DWORD> TMapAttrControl;
struct SSpecSkillInfo;

enum EBuffTargetEntryType
{
  EBUFF_TARGET_ENTRY_TYPE_NPC  = 1 << 0,
  EBUFF_TARGET_ENTRY_TYPE_USER = 1 << 1,
};

enum BuffReloadType
{
  BUFFRELOAD_NONE = 0, //reload 无作用
  BUFFRELOAD_COVER = 1, //reload 覆盖原有
  BUFFRELOAD_TOGETHER = 2, //reload 并存
  BUFFRELOAD_OFFSET = 3 //reload 抵消
};
enum EBuffType
{
  EBUFFTYPE_MIN = 0,
  EBUFFTYPE_NORM = 1,
  EBUFFTYPE_TRANSFORM = 2, //变装
  EBUFFTYPE_STATUS = 3, //眩晕,冰冻..
  EBUFFTYPE_HIDE = 4, // 隐身
  EBUFFTYPE_ELEMENT = 5, // 水,火 etc 元素
  EBUFFTYPE_DSTATUS = 6, // 免疫状态
  EBUFFTYPE_PARTTRANSFORM = 7, // 部分变装
  EBUFFTYPE_ROBREWARD = 8, // 额外掉落
  EBUFFTYPE_HANDEMOJI = 9, // 表情达人
  EBUFFTYPE_TAUNT = 10, // 嘲讽
  EBUFFTYPE_CHDEFINE = 11, // 改变npc define
  EBUFFTYPE_FORCEATTR = 12, // 强制属性
  EBUFFTYPE_DISABLE = 13, // diable other buff
  EBUFFTYPE_ATTRCHANGE = 14, // 基础属性变化 not hp sp
  EBUFFTYPE_HSPCHANGE = 15, // hp sp 变化buff
  EBUFFTYPE_RECOVERHP = 16, // 自动回血
  EBUFFTYPE_USESKILL = 17 , // 触发技能类buff
  EBUFFTYPE_DROPITEM = 18, // 掉落道具类buff
  EBUFFTYPE_GETSKILL = 19, // 获得技能类buff
  EBUFFTYPE_DRIVEOUT = 20, // 驱散
  EBUFFTYPE_ADDBUFF = 21, // 添加其他buff
  EBUFFTYPE_SPRECOVER = 22, // 回蓝buff
  EBUFFTYPE_PLAYACTION = 23, // 播放动作
  EBUFFTYPE_DELME = 24, // 删除自身(npc)
  EBUFFTYPE_HPREDUCE = 25, // 流血类buff
  EBUFFTYPE_DELBUFF = 26, // 删除其他buff
  EBUFFTYPE_DELSTATUS = 27, // 删除状态类buff
  EBUFFTYPE_SEETRAP = 28, // 陷阱可见
  EBUFFTYPE_TREASURE = 29, // 寻宝, 可见
  EBUFFTYPE_LOSETARGET = 30, // 使怪物丢失目标
  EBUFFTYPE_PRIORATTACK = 31, // 使怪物优先攻击某些目标
  EBUFFTYPE_HPPROTECT = 32, // 掉血保护
  EBUFFTYPE_MULTITIME = 33, // 双(多)倍卡
  EBUFFTYPE_CHANGESCALE = 34, // 改变体型
  EBUFFTYPE_CHANGESKILL = 35, // 改变普攻技能
  EBUFFTYPE_SHAREDAM = 36, // 被承担伤害
  EBUFFTYPE_BREAKEQUIP = 37, // 损坏装备
  EBUFFTYPE_PACKAGESLOT = 38, // 改变背包格子数量
  EBUFFTYPE_AFFACTSKILL = 39, // 影响指定技能属性
  EBUFFTYPE_TRADEINFO = 40, // 影响交易所数据
  EBUFFTYPE_EXTRADROP = 41, // 额外掉落
  EBUFFTYPE_DROPEXP = 42, // 改变死亡经验惩罚值
  EBUFFTYPE_MARKHEAL = 43, // 标记治愈术额外目标
  EBUFFTYPE_SPEEDUP = 44, // 加速其他Buff
  EBUFFTYPE_TRIGTRAP = 45, // 引爆陷阱buff
  EBUFFTYPE_REPLACESKILL = 46, // 改变技能
  EBUFFTYPE_ATTRLIMIT = 47, // 属性上下限限定
  EBUFFTYPE_LIMITSKILL = 48, // 限制某些技能对指定目标使用
  EBUFFTYPE_SKILLCD = 49, // 使指定技能进入CD
  EBUFFTYPE_LIMITUSEITEM = 50, // 限制不能使用某些道具类型
  EBUFFTYPE_TRANSFERBUFF = 51, // 添加者与被添加者转移某BUFF
  EBUFFTYPE_AUTOBLOCK = 55, // 自动防御
  EBUFFTYPE_GM = 56, // 执行gm指令
  EBUFFTYPE_SKILLLEVEL = 57, // 技能等级
  EBUFFTYPE_COOKER_KINFE = 58, // 美食家餐刀获得
  EBUFFTYPE_UNDEAD = 59, // 收到致死攻击不会死亡
  EBUFFTYPE_RESISTSTATUS = 60, // 免疫异常状态, 先判概率
  EBUFFTYPE_DROPFOODREWARD = 63, // 杀怪掉落食材奖励
  EBUFFTYPE_DELSPEFFECT = 62, // 删除特效
  EBUFFTYPE_FORCESTATUS = 64, // 给对方添加异常状态时, 该状态不能被驱散
  EBUFFTYPE_MULTIEXP = 65, // 多倍经验卡, 时间正常流失, 不影响战斗时长(区别于锁链)
  EBUFFTYPE_GVGREWARD = 66, // GVG延迟奖励
  EBUFFTYPE_IMMUNE = 67, // 护盾, 免疫攻击与敌方buff
  EBUFFTYPE_PICKUP = 68, // 捡道具
  EBUFFTYPE_DELSKILLSTATUS = 69, // 移除技能状态
  EBUFFTYPE_DEEPHIDE = 70, // 深度隐身, 仅在自己释放技能时删除隐身
  EBUFFTYPE_OFFEQUIP = 71, // 装备脱卸
  EBUFFTYPE_PROTECTEQUIP = 72, // 装备保护
  EBUFFTYPE_FIXEQUIP = 73, // 装备修复
  EBUFFTYPE_SKILLTARGET = 74, // 技能额外目标
  EBUFFTYPE_EXTRAREWARD = 75, // 回归额外奖励buff
  EBUFFTYPE_SCENERY = 76, // 景点buff
  EBUFFTYPE_WEAPONBLOCK = 77, // 武器防御
  EBUFFTYPE_STATUSREBOUND = 78, //技能反弹
  EBUFFTYPE_HPSTORAGE = 79, //hp存储库
  EBUFFTYPE_SPSTORAGE = 80, //sp存储库
  EBUFFTYPE_PETADVENTURE = 81, //宠物冒险buff
  EBUFFTYPE_ATTRCONTROL = 82, // 属性控制
  EBUFFTYPE_RIDEWOLF   = 83, //标记为骑狼状态
  EBUFFTYPE_FORBIDEQUIP = 84, // 禁用装备位
  EBUFFTYPE_CHASE = 85, //锁定目标(怪物用)
  EBUFFTYPE_IMMUNETAUNT = 86, //免疫嘲讽
  EBUFFTYPE_SPEXCHANGE = 87, //交换SP
  EBUFFTYPE_UNLOCKRECIPE = 88, // 概率解锁食谱
  EBUFFTYPE_ATTRTRANSFER = 89, // 属性转移
  EBUFFTYPE_DHIDE = 90, // 反隐
  EBUFFTYPE_CHECKADDLINE = 91, // 检查队员距离添加连线
  EBUFFTYPE_NOMAPEXIT = 92, // 不可使用地图传送阵
  EBUFFTYPE_INFECT = 93, //异常状态传染
  EBUFFTYPE_MAX
};

enum EBuffStatusType
{
  EBUFFSTATUS_MIN = 0,
  EBUFFSTATUS_POSION = 1,
  EBUFFSTATUS_BLOOD = 2,
  EBUFFSTATUS_BURN = 3,
  EBUFFSTATUS_DIZZY = 4,
  EBUFFSTATUS_FREEZE = 5,
  EBUFFSTATUS_STONE = 6,
  EBUFFSTATUS_SLEEP = 7,
  EBUFFSTATUS_FEAR = 8,
  EBUFFSTATUS_NOMOVE = 9,
  EBUFFSTATUS_SILENCE = 10,
  EBUFFSTATUS_CURSE = 11,
  EBUFFSTATUS_DARK = 12,
  EBUFFSTATUS_MAX = 13,
};

enum EBuffDelType
{
  EBUFFDELTYPE_MIN = 0,
  EBUFFDELTYPE_TIME = 1, // 按时间删除
  EBUFFDELTYPE_COUNT = 2, // 按次数删除
  EBUFFDELTYPE_FOREVER = 3, // 永久类型, 不删除
  EBUFFDELTYPE_FAKEDEAD = 4, // 非装死状态时删除
  EBUFFDELTYPE_LAYER = 5, // 层数为0时删除
};

enum EBuffTargetType
{
  EBUFFTARGET_NONE = 0,
  EBUFFTARGET_SELF = 1,
  EBUFFTARGET_ENEMY = 2,
  EBUFFTARGET_TEAM = 3,
  EBUFFTARGET_FRIEND = 4,
  EBUFFTARGET_BUFFADDER= 5, // buff添加者
};

enum EIgnoreAttrControl
{
  EIGNOREATTRCONTROL_MIN = 0,
  EIGNOREATTRCONTROL_ALL = 1,
  EIGNOREATTRCONTROL_INC = 2,
  EIGNOREATTRCONTROL_DEC = 3,
};

enum EAttrControl
{
  EATTRCONTROL_IGNORE_INC = 1, // 忽略增加的属性, 按位处理
  EATTRCONTROL_IGNORE_DEC = 2, // 忽略减少的属性
};

#define TARGET_NUM_LIMIT 10

struct SBufferData;
class BufferState
{
  friend class BufferStateList;
  public:
    BufferState();
    virtual ~BufferState();

  public:
    DWORD getID() const { return m_dwID; }

    DWORD getBuffOdds(xSceneEntryDynamic *me, UINT fromID, DWORD lv);
    QWORD getBuffLastTime(xSceneEntryDynamic* me, UINT fromID, DWORD lv);
    DWORD getLimitAddLayers(xSceneEntryDynamic* me, UINT fromID, DWORD lv);
    DWORD getEffectOdds(xSceneEntryDynamic* me, xSceneEntryDynamic* pTarget, UINT fromID, DWORD lv);
  public:
    EBuffType getBuffType() const { return m_eBuffType; }
    BuffTriggerType getTrigType() const { return buffCondition == nullptr ? BUFFTRIGGER_NONE : buffCondition->getTrigType(); }

    //bool isGainBuff() const { return m_isgain == 1; }
    bool isOverlay() const { return m_bIsOverLay; }
    bool isTimeBuff() const { return !(m_eDelType == EBUFFDELTYPE_MIN || m_eDelType == EBUFFDELTYPE_FOREVER); }
    bool isRingBuff() const { return m_dwIconType == 2; }
    bool isDeathCanAdd() const { return m_bDeathAdd; }

    bool isPermanentBuff() const { return m_eDelType == EBUFFDELTYPE_FOREVER; }
    bool isDelByLayer() const { return m_eDelType == EBUFFDELTYPE_LAYER; }
    bool isDelByCount() const { return m_eDelType == EBUFFDELTYPE_COUNT; }

    QWORD getInterval() const { return m_dwInterval; }
    DWORD getProtectTime() const { return m_dwProtectTime; }

    TPtrBuffCond getCondition() const { return buffCondition; }
    EBuffStatusType getStatus() const { return m_eStatusType; }

    //DWORD getLimitAddLayers() const { return m_dwLimitLayers; }
    DWORD getShaderColor() const { return m_dwShaderColor; }
    const string& getBuffTips() const { return m_strBuffTips; }
    const string& getBuffName() const { return m_strBuffName; }
    DWORD getFailBuff() const { return m_dwFailBuff; }
    const string& getBuffTypeName() const { return m_strBuffTypeName; }

    DWORD getAtkDefPri() const { return m_dwAtkDef_Priority; }
    bool needSyncNine() const;
    EIgnoreAttrControl getIgnoreAttrControl() const { return m_eIgnoreAttrControl; }

    bool isStaticBuff() const { return !m_bCheckEffect && !m_bCheckDel && !m_bDynamicAttr; }
    bool hasAttr() const { return m_bHasAttr; }
    void sortInit(); /* 创建成功, init 结束调用一次*/
  public:
    bool canStart(SBufferData& bData, DWORD cur);// { return buffCondition == nullptr ? false : buffCondition->checkCondition(bData, cur); }
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool canContinue(const SBufferData& bData, QWORD curm);
    virtual bool needDel(const SBufferData& bData, QWORD cur);
    virtual void ondel(xSceneEntryDynamic *me) {};
    virtual void ondelLater(xSceneEntryDynamic *me) {};
    virtual void onInvalid(SBufferData& bData);
    virtual bool isCalcOnce() const {return m_bCalcOnce;}
    virtual bool needRefreshAllBuff() const { return false; }

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);// {me->refreshDataAtonce(); return true;}
    virtual bool getMyAttr(xSceneEntryDynamic* me, const SBufferData* pData, TVecAttrSvrs& getMyAttr);// { return m_data.getMyAttr(skillID, charid); }
  protected:
    const TSetSceneEntrys& getBuffTargets(const SBufferData& bData, xSceneEntryDynamic*me, xSceneEntryDynamic* enemy = nullptr);
    const TSetSceneEntrys& getBuffTargetsByTargetType(const SBufferData& bData, xSceneEntryDynamic*me, EBuffTargetType eType, xSceneEntryDynamic* enemy = nullptr, float range = 0.0f);
    bool needCheckEffect() const;
    bool needCheckDel() const;
  protected:
    DWORD m_dwID = 0;
    QWORD m_dwInterval = 0;
    DWORD m_maxCnt = 0;
    bool m_bDelayDoEffect = false; /*添加时不立即执行, 必须经过interval时间*/
    //DWORD m_isgain = 0;

    // 选择目标
    EBuffTargetType m_eTargetType = EBUFFTARGET_NONE;
    DWORD m_dwTargetNumLmt = 0; // 数量限制
    float m_fTargetRange = 0; // 静态范围
    float m_fTargetInnerRange = 0; // 内圈范围
    FmData m_FmRange; // 动态范围
    TSetSceneEntrys m_setTarget; // 目标结果, 每次选择先擦除再返回
    DWORD m_dwTeamCount = 0; // 可被施加buff的队友数量上限
    DWORD m_dwTargetLmtDistanceCnt = 0; // 一定距离内可被施加buff的对象数量上限
    float m_fTargetLmtDistance = 0; // 限制数量的距离范围
    DWORD m_dwEntryType = 0; // 搜索的目标类型 1.npc 2.user 4....   同时作用npc,user 用 1 | 2
    std::set<DWORD> m_setMonsterRace; // 限制monster类型 参考CommonFun.Race
    bool m_bIncludeSelf = true;  //是否包括自己
    // 选择目标

    DWORD m_dwProtectTime = 0;
    //DWORD m_dwDelType = 0; // 0 装备卡片被动技能; 1 按时间删除; 2 按次数删除; 3 属性娃娃,永不删除
    EBuffDelType m_eDelType = EBUFFDELTYPE_MIN;
    DWORD m_dwIconType = 0;
    bool m_bAttackDel = false;
    bool m_bDeathKeep = false;
    bool m_bMultiTime = false;
    bool m_bOfflineKeep = false;
    bool m_bDeathAdd = false;
    DWORD m_dwOffSetBuffID = 0; // 互相抵消的Buff

    // 删除条件
    DWORD m_dwAttackCount = 0;
    DWORD m_dwBeAttackCount = 0;
    float m_fDamagePer = 0;

    EBuffStatusType m_eStatusType = EBUFFSTATUS_MIN;
    bool m_bNeedFakeDead = false;
    DWORD m_dwNeedAction = 0;
    DWORD m_dwNeedEffectLineID = 0;

    FmData m_FmOdds;
    FmData m_FmTime;
    TVecFmData m_FmAttrs;
    TVecAttrSvrs m_oAttr;

    FmData m_FmLayer;
    FmData m_FmEffectOdds;
    FmData m_FmBeAttackCnt;
    FmData m_FmDamagerPer;

    TPtrBuffCond buffCondition;

    EBuffType m_eBuffType = EBUFFTYPE_NORM;
    BuffReloadType reloadType = BUFFRELOAD_NONE;
    ESource sourceType = ESOURCE_NORMAL;

    bool m_bIsOverLay = false;
    DWORD m_dwLimitLayers = 0;
    DWORD m_dwShaderColor = 0;

    string m_strBuffTips;
    string m_strBuffName;
    string m_strBuffTypeName;
    DWORD m_dwFailBuff = 0;
    bool m_bNeedSourceName = false;
    DWORD m_dwTargetMsg = 0;

    DWORD m_dwEndLayerBuff = 0;
    TSetDWORD m_setEndExtraBuff;

    bool m_bGainBuff = false; /* 是否是增益buff */
    bool m_bCanDisperse = false; /* 是否可以被驱散 */
    DWORD m_dwResistImmune = 0; // 1免疫, 2抵抗, 3,免疫and抵抗 : 表示是否需特效表现

    // 攻击防御属性 优先级
    DWORD m_dwAtkDef_Priority = 0; // 数字小, 优先级高

    bool m_bLayerDiffTime = false; /* 每层拥有不同结束时间*/
    bool m_bOverLayerNoAdd = false ; /* 超出层数不可以添加*/

    bool m_bNotSave = false;
    bool m_bCheckDel = false;
    bool m_bCheckEffect = false;
    bool m_bDynamicAttr = false; /*是否有动态属性,需要实时计算*/
    bool m_bHasAttr = false; /*是否有除Hp、Sp外的常规属性*/

    DWORD m_dwStateEffectID = 0; // 对应buff表现特效id, bufferstate表
    TSetDWORD m_setDelBuffID;
    bool m_bCalcOnce = false;
    bool m_bIsForceStatus = false;

    EIgnoreAttrControl m_eIgnoreAttrControl = EIGNOREATTRCONTROL_MIN;
    bool m_bNotDelOffSetBuff = false;
};

//属性改变buff
class BuffAttrChange : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAttrChange();
    virtual ~BuffAttrChange();
};

class BuffHSPChange : public BufferState
{
  friend class BufferStateList;
  public:
    BuffHSPChange();
    virtual ~BuffHSPChange();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    bool m_bCalcHeal = false;
};

class BuffAttrChangePer : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAttrChangePer()
    {
    }
    virtual ~BuffAttrChangePer()
    {
    }
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
    {
      return true;
    }
};

//隐匿
class BuffHiding : public BufferState
{
  friend class BufferStateList;
  public:
    BuffHiding();
    virtual ~BuffHiding();
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);
  public:
    void onClientMove(SBufferData& bData);
  private:
    DWORD m_dwEndBuff = 0;
    DWORD m_dwMoveDelTimeMs = 0; // ms
};

//反击 施加buff者 (嘲讽,...)
class BuffBeatBack : public BufferState
{
  friend class BufferStateList;
  public:
    BuffBeatBack();
    virtual ~BuffBeatBack();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);
};

//使用指定技能
class BuffUseSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffUseSkill();
    virtual ~BuffUseSkill();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    DWORD m_odds = 0;
    DWORD m_skillID = 0;
    bool isActive = true;
    bool m_bUseSelect = false;
    DWORD m_dwSkillOpt = 0;
    bool m_bUseCondSkill = false;
    bool m_bUseReplaced = false;
};

//变身
class BuffTransform : public BufferState
{
  friend class BufferStateList;
  public:
    BuffTransform();
    virtual ~BuffTransform();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual void ondel(xSceneEntryDynamic *me);
    virtual void onInvalid(SBufferData& bData);
    DWORD getTransform(EUserDataType uType);
    bool isMoveDel() { return m_bMoveDel; }
    bool doTransform(SBufferData& bData);
  private:
    void refreshTransform(xSceneEntryDynamic *me);
    DWORD m_dwFigureID = 0;
    map<EUserDataType, DWORD> m_mapFigure;
    vector<pair<DWORD, DWORD>> m_vecScale;
    DWORD m_dwTransformID = 0;
    bool m_bMoveDel = false;
    ETransformType m_eType = ETRANSFORM_NORMAL;
};

//回血
class BuffRecoverHp : public BufferState
{
  friend class BufferStateList;
  public:
    BuffRecoverHp();
    virtual ~BuffRecoverHp();
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    //type=0, recover anyway; type=1, recover when still
    DWORD m_type = 0;
    bool m_bCanBeDisable = false;
    xLuaData m_effParams;
};

//允许指定职业骑乘
class BuffAllowRide : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAllowRide();
    virtual ~BuffAllowRide();
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    const TVecDWORD& getRideProfes() const { return m_vecProfes; };
  private:
    TVecDWORD m_vecProfes;
};

//杀死指定怪物 掉落指定物品 
class BuffDropItem : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDropItem();
    virtual ~BuffDropItem();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    DWORD m_dwItemID = 0;
    DWORD m_dwItemNum = 0;
    DWORD m_dwRate = 0;//生效概率
};

//获得某个技能 装备带来的属性,仅执行一次但不会被删除(others del after count >= m_maxcnt)
class BuffGetSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffGetSkill();
    virtual ~BuffGetSkill();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id, xLuaData &data, TPtrBuffCond buffCond);
    virtual void ondel(xSceneEntryDynamic *me);
  private:
    DWORD m_sourceItem = 0;
    DWORD m_dwSkillID = 0;
    ESource m_eSource = ESOURCE_EQUIP;
    DWORD m_dwLevelUp = 0;
    DWORD m_dwExtraLv = 0;
};

//驱散
class BuffDriveout : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDriveout();
    virtual ~BuffDriveout();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    EBuffType m_delType = EBUFFTYPE_MIN;
    bool m_bDelFakeDead = false;

    bool m_bDelGain = false;
    FmData m_FmNum;
};

//AddBuff
class BuffAddBuff : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAddBuff();
    virtual ~BuffAddBuff();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    TVecDWORD m_vecBuffIDs;
    bool m_bRandom = false;
    bool m_bSelectOneValid = false;
    DWORD m_dwOdds = 0;
    map<DWORD, DWORD> m_mapID2Layer;
    TVecDWORD m_vecWeightAccum; // 累加的权重. 如30,30,40的权重,数组为30,60,100
    TVecDWORD m_vecWeightSkillids; // 每个权重对应的skillid
};

//DelBuff
class BuffDelBuff : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDelBuff();
    virtual ~BuffDelBuff();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    TVecDWORD m_vecBuffIDs;
    map<DWORD, DWORD> m_mapID2Layer;
    bool m_bSameSource = false; // buff来源相同
    bool m_bDeadClear = false; // 按照死亡清除buff的规则删除所有buff
    TSetDWORD m_setDeadClearExcept; // 死亡清除buff时不删的特定buffid
};

//状态改变
class BuffStatusChange : public BufferState
{
  friend class BufferStateList;
  public:
    BuffStatusChange();
    virtual ~BuffStatusChange();
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual void ondel(xSceneEntryDynamic* me);
};

//sp恢复
class BuffSpRecover : public BufferState
{
  friend class BufferStateList;
  public:
    BuffSpRecover();
    virtual ~BuffSpRecover();

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    bool m_bCanBeDisable = false;
};

// 播放表情
class BuffPlayAction : public BufferState
{
  friend class BufferStateList;
  public:
    BuffPlayAction();
    virtual ~BuffPlayAction();

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
  private:
    DWORD m_dwEmoji = 0;
};

// disable other buff
class BuffDisable : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDisable();
    virtual ~BuffDisable();

    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual void ondel(xSceneEntryDynamic *me);

    const TSetDWORD& getDisIDs() { return m_setIDs; }
  private:
    TSetDWORD m_setIDs;
};

// 元素属性变化
class BuffBattleAttr : public BufferState
{
  friend class BufferStateList;
  public:
    BuffBattleAttr();
    virtual ~BuffBattleAttr();

    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual void ondel(xSceneEntryDynamic *me);
    bool isDisOther() const { return !m_FmTime.empty(); }
    bool haveAtkAttr() const { return m_bAtkAttr; }
    bool haveDefAttr() const { return m_bDefAttr; }
    DWORD getPriority() const { return m_dwPriority; }
  private:
    bool m_bAtkAttr = false;
    bool m_bDefAttr = false;
    DWORD m_dwPriority = 0;
};

// delete self buff , only for npc
class BuffDelMe : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDelMe();
    virtual ~BuffDelMe();

    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
};

// 损血
class BuffHpReduce : public BufferState
{
  friend class BufferStateList;
  public:
    BuffHpReduce();
    virtual ~BuffHpReduce();

    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    //float m_fDelHpPer = 0.0f;
    FmData m_fmDelHpPer;
    //DWORD m_dwDelValue = 0;
};

// 删除状态
class BuffDelStatus : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDelStatus();
    virtual ~BuffDelStatus();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    DWORD m_dwRandomNum = 0;
    TSetDWORD m_setStatus;
};

// 免疫状态
class BuffImmuneStatus : public BufferState
{
  friend class BufferStateList;
  public:
    BuffImmuneStatus();
    virtual ~BuffImmuneStatus();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

    bool isImmuneStatus(DWORD status) const { return m_setStatus.find(status) != m_setStatus.end(); }
  protected:
    TSetDWORD m_setStatus;
};

// 照明箭, 使隐藏的陷阱自己与友方可见
class BuffSeeTrap : public BufferState
{
  enum ETrapType
  {
    ETRAPTYPE_NORMALTRAP = 1,
    ETRAPTYPE_SPECSKILL = 2,
  };
  friend class BufferStateList;
  public:
    BuffSeeTrap();
    virtual ~BuffSeeTrap();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    ETrapType m_eType = ETRAPTYPE_NORMALTRAP;
    DWORD m_dwRange = 0;
    TSetDWORD m_setSeeSkills;
};

// 部分变身
class BuffPartTransform : public BufferState
{
  friend class BufferStateList;
  public:
    BuffPartTransform();
    virtual ~BuffPartTransform();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);

    bool has(EUserDataType eType) const { return m_mapFigure.find(eType) != m_mapFigure.end(); }
    DWORD get(EUserDataType eType, xSceneEntryDynamic* entry = nullptr) const;
  private:
    map<EUserDataType, DWORD> m_mapFigure;
    map<DWORD, DWORD> m_mapPartner2Mount;
    map<DWORD, DWORD> m_mapMount2Body;
};

// 杀死怪物额外掉落
class BuffRobReward : public BufferState
{
  friend class BufferStateList;
  public:
    BuffRobReward();
    virtual ~BuffRobReward();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    void onKillMonster(SceneUser* user, SceneNpc* npc, SBufferData& bData);
    void onAddBattleTime(SBufferData& bData, DWORD time);

    DWORD getRate() { return m_dwBattleTime <= 0 ? 0 : m_dwExtraCnt; };
  private:
    DWORD m_dwExtraCnt = 0;
    DWORD m_dwBattleTime = 0;
};

// 表情达人
class BuffHandEmoji : public BufferState
{
  friend class BufferStateList;
  public:
    BuffHandEmoji();
    virtual ~BuffHandEmoji();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  public:
    void onPlayEmoji(DWORD id, SBufferData& bData);

  private:
    vector<pair<DWORD, DWORD>> m_vecEmoji2Buff;
    FmData m_cdFm;
};

// npc change define
class BuffChangeDefine : public BufferState
{
  friend class BufferStateList;
  public:
    BuffChangeDefine();
    virtual ~BuffChangeDefine();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);
    virtual bool onStart(SBufferData& bData, DWORD cur);
  private:
    xLuaData m_oData;
};

// force attr
class BuffForceAttr : public BufferState
{
  friend class BufferStateList;
  public:
    BuffForceAttr();
    virtual ~BuffForceAttr();

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);
};

// treasure
class BuffTreasure : public BufferState
{
  friend class BufferStateList;
  public:
    BuffTreasure();
    virtual ~BuffTreasure();

    virtual void ondel(xSceneEntryDynamic *me);
    virtual bool onStart(SBufferData& bData, DWORD cur);
};

// lose target
class BuffLoseTarget : public BufferState
{
  friend class BufferStateList;
  public:
    BuffLoseTarget();
    virtual ~BuffLoseTarget();

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    bool m_bNoMvp = false;
    bool m_bNoMini = false;
};

// 优先攻击指定职业, 性别(only monster)
class BuffPriorAttack : public BufferState
{
  friend class BufferStateList;
  public:
    BuffPriorAttack();
    virtual ~BuffPriorAttack();

    virtual void ondel(xSceneEntryDynamic *me);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    TSetDWORD m_setPriorProfession;
    EGender m_ePriorGender = EGENDER_MIN;
    TSetDWORD m_setIgnoreProfession;
};

// 一定时间内掉血保护buff
class BuffHpProtect : public BufferState
{
  friend class BufferStateList;
  public:
    BuffHpProtect();
    virtual ~BuffHpProtect();

    virtual void ondel(xSceneEntryDynamic *me);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    DWORD m_dwProtectInterval;
    float m_fProtectHpPer;
};

// 双(多)倍卡
class BuffMultiTime : public BufferState
{
  friend class BufferStateList;
  public:
    BuffMultiTime();
    virtual ~BuffMultiTime();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  public:
    void onGetExp(SBufferData& bData, SceneNpc* npc, DWORD baseExp, DWORD jobExp);
    void onPickUpItem(SBufferData& bData, const ItemInfo& oItem);
    void onAddBattleTime(SBufferData& bData, DWORD time);

    DWORD getRate() { return m_dwRate < 2 ? 0 : (m_dwRate - 1); }
  private:
    bool isValidGet(SceneUser* user, const SNpcCFG* npcCFG);
  private:
    DWORD m_dwRate = 0;
};

// 体型变化
class BuffChangeScale : public BufferState
{
  friend class BufferStateList;
  public:
    BuffChangeScale();
    virtual ~BuffChangeScale();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);

    float getChangePer() const { return m_fChangePer; }
    float getAddPer() const { return m_fAddPer; }
  private:
    float m_fChangePer = 0;
    float m_fAddPer = 0;
};

// 改变普攻
class BuffChangeSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffChangeSkill();
    virtual ~BuffChangeSkill();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
    virtual void onInvalid(SBufferData& bData);
  private:
    map<DWORD, DWORD> m_mapPro2SkillID;
    DWORD m_dwMonsterSkill = 0;
};

// 被别人承担伤害
class BuffShareDam : public BufferState
{
  friend class BufferStateList;
  public:
    BuffShareDam();
    virtual ~BuffShareDam();
};

// 装备破坏
class BuffBreakEquip : public BufferState
{
  friend class BufferStateList;
public:
  BuffBreakEquip();
  virtual ~BuffBreakEquip();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);

private:
  DWORD m_dwCalcType = 0;
  DWORD m_dwDuration = 0;
};

// 改变背包格子数量
class BuffPackageSlot : public BufferState
{
  friend class BufferStateList;
  public:
    BuffPackageSlot();
    virtual ~BuffPackageSlot();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic* me);
    DWORD getSlotNum(EPackType eType) { return m_eType == eType ? m_dwNum : 0; }
  private:
    EPackType m_eType = EPACKTYPE_MAIN;
    DWORD m_dwNum = 0;
};

// 影响指定技能属性
class BuffAffactSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAffactSkill();
    virtual ~BuffAffactSkill();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
    virtual void onInvalid(SBufferData& bData);

    bool haveSkill(xSceneEntryDynamic* me, DWORD familyid);
    void getSpecSkillInfo(SSpecSkillInfo& info, DWORD layer);
    bool isAllSkill() { return m_bAllSkill; }
    bool getLastConcertSkill() { return m_bLastConcertSkill; }
  private:
    int m_intCount = 0;
    int m_intTargetNum = 0;
    float m_fDurationPer = 0;
    float m_fRange = 0;
    float m_fReady = 0;
    TSetDWORD m_setSkillIDs;
    map<EAttrType, float> m_mapSkillAttrs;
    map<DWORD, int> m_mapItem2Num;
    map<DWORD, float> m_mapItem2Per;
    DWORD m_intLimitCount = 0;
    bool m_bNeedNoItem = false;
    bool m_bAllSkill = false;
    bool m_bLastConcertSkill = false;
};

// 影响交易所数据
class BuffTradeInfo : public BufferState
{
  friend class BufferStateList;
  public:
    BuffTradeInfo();
    virtual ~BuffTradeInfo();

    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
};

// 影响摆摊数据
class BuffBoothInfo : public BufferState
{
  friend class BufferStateList;
  public:
    BuffBoothInfo();
    virtual ~BuffBoothInfo();

    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
};

// 额外掉落
class BuffExtraDrop : public BufferState
{
  friend class BufferStateList;
public:
  BuffExtraDrop();
  virtual ~BuffExtraDrop();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

  void onPickUpItem(SBufferData& bData, const ItemInfo& oItem);

private:
  DWORD m_dwCd = 0;               /* cd时间(秒) */
  DWORD m_dwProb = 0;             /* 触发概率 */
  DWORD m_dwLimit = 0;            /* 数量上限 */
  DWORD m_dwItemId = 0;          /* 道具类型 */
  DWORD m_dwItemNum = 0;            /* 获取数量 */
};

// 改变死亡经验惩罚值
class BuffDropExp : public BufferState
{
  friend class BufferStateList;
public:
  BuffDropExp();
  virtual ~BuffDropExp();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

  SDWORD onDropBaseExp(ESource source);

private:
  SDWORD m_sdwRate = 0;               /* 减少比例, 千分, 正数表示减少, 负数表示增加 */
};

// 标记治愈术额外目标
class BuffMarkHeal : public BufferState
{
  friend class BufferStateList;
  public:
    BuffMarkHeal();
    virtual ~BuffMarkHeal();

    virtual void ondel(xSceneEntryDynamic *me);
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  private:
    DWORD m_dwSkillFamilyID = 0;
};

// 加速其他Buff
class BuffSpeedUp : public BufferState
{
  friend class BufferStateList;
  public:
    BuffSpeedUp();
    virtual ~BuffSpeedUp();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    DWORD m_dwBuffID = 0; // 目标BuffID
    int m_intMs = 0; // 加速时间, 毫秒
};

// 结束时引爆陷阱
class BuffTrigTrap : public BufferState
{
  friend class BufferStateList;
  public:
    BuffTrigTrap();
    virtual ~BuffTrigTrap();

    virtual void ondel(xSceneEntryDynamic* me);
};

class BuffReplaceSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffReplaceSkill();
    virtual ~BuffReplaceSkill();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
  private:
    DWORD m_dwOldFamilyID = 0;
    DWORD m_dwNewFamilyID = 0;
};

// 属性上下限
class BuffAttrLimit : public BufferState
{
  friend class BufferStateList;
  public:
    BuffAttrLimit();
    virtual ~BuffAttrLimit();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);

    const map<EAttrType, float>& getLimitMaxAttrs() { return m_mapLimitMaxValue; }
    const map<EAttrType, float>& getLimitMinAttrs() { return m_mapLimitMinValue; }
  private:
    map<EAttrType, float> m_mapLimitMaxValue;
    map<EAttrType, float> m_mapLimitMinValue;
};

// 自动防御
class BuffAutoBlock : public BufferState
{
  friend class BufferStateList;
public:
  BuffAutoBlock();
  virtual ~BuffAutoBlock();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  DWORD getStiff() { return m_dwStiff; }
private:
  DWORD m_dwStiff = 0;
};

// 执行gm指令
class BuffGM : public BufferState
{
  friend class BufferStateList;
public:
  BuffGM();
  virtual ~BuffGM();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
private:
  xLuaData m_oParam;
  bool m_bSource;
};

// 执行gm指令
class BuffSkillLevel : public BufferState
{
  friend class BufferStateList;
public:
  BuffSkillLevel()
  {
    m_eBuffType = EBUFFTYPE_SKILLLEVEL;
  }
  virtual ~BuffSkillLevel() {}
};

// 限制对某目标使用技能
class BuffLimitSkill : public BufferState
{
  friend class BufferStateList;
  public:
    BuffLimitSkill();
    virtual ~BuffLimitSkill();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    const TSetDWORD& getLimitSkill() { return m_setSkillIDs; } // 可以使用的技能
    const TSetDWORD& getLimitNotSkill() { return m_setNotSkillIDs; } // 不可使用的技能
  private:
    TSetDWORD m_setSkillIDs;
    TSetDWORD m_setNotSkillIDs;
    DWORD m_dwIgnoreTarget = 0;
};

// 使指定技能进入CD
class BuffSkillCD : public BufferState
{
  friend class BufferStateList;
  public:
    BuffSkillCD();
    virtual ~BuffSkillCD();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);
  private:
    map<DWORD, FmData> m_mapSkill2CDMs;
};

// 限制不能使用某些类型的道具
class BuffLimitUseItem : public BufferState
{
  friend class BufferStateList;
  public:
    BuffLimitUseItem();
    virtual ~BuffLimitUseItem();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  public:
    bool canUseItem(DWORD itemtype) const;
  private:
    TSetDWORD m_setOkTypes;
    TSetDWORD m_setForbidTypes;
    bool m_bForbidAll = false;
};

// 转移BUFF
class BuffTransfer : public BufferState
{
  friend class BufferStateList;
  public:
    BuffTransfer();
    virtual ~BuffTransfer();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool onStart(SBufferData& bData, DWORD cur);

  private:
    DWORD m_dwBuffID = 0;
    DWORD m_dwMaxLayer = 0;
};

// 美食家餐刀掉落
class BuffCookerKnife : public BufferState
{
  friend class BufferStateList;
  public:
    BuffCookerKnife();
    virtual ~BuffCookerKnife();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

    void onPickUpItem(SBufferData& bData, const ItemInfo& oItem);

  private:
    DWORD m_dwCd = 0;               /* cd时间(秒) */
    DWORD m_dwProb = 0;             /* 触发概率 */
    DWORD m_dwLimit = 0;            /* 数量上限 */
    DWORD m_dwItemId = 0;          /* 道具类型 */
    DWORD m_dwItemNum = 0;            /* 获取数量 */
};

// 受到致命攻击, 不会死亡
class BuffUndead : public BufferState
{
  friend class BufferStateList;
  public:
    BuffUndead();
    virtual ~BuffUndead();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool canContinue(const SBufferData& bData, QWORD curm) { return false; } /*外部驱动, 不在timer执行*/
  public:
    DWORD getProtectBuff() const { return m_dwProtectBuff; }
  private:
    DWORD m_dwProtectBuff = 0;
};

// 免疫异常状态 一定次数
class BuffResistStatus : public BuffImmuneStatus
{
  friend class BufferStateList;
  public:
    BuffResistStatus();
    virtual ~BuffResistStatus();
    virtual bool canContinue(const SBufferData& bData, QWORD curm) { return false; } /*外部驱动, 不在timer执行*/
};

// 删除特效
class BuffDelSpEffect : public BufferState
{
  friend class BufferStateList;
public:
  BuffDelSpEffect();
  virtual ~BuffDelSpEffect();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
private:
  set<DWORD> m_setSpIds;
  bool m_bIgnoreSource = false;
};

// 施加的异常状态不可被驱散
class BuffForceStatus : public BufferState
{
  friend class BufferStateList;
  public:
    BuffForceStatus();
    virtual ~BuffForceStatus();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    bool haveStatus(DWORD status) { return m_setStatus.find(status) != m_setStatus.end(); }
  private:
    TSetDWORD m_setStatus;
};

// 多倍经验
class BuffMultiExp : public BufferState
{
  friend class BufferStateList;
  public:
    BuffMultiExp();
    virtual ~BuffMultiExp();
    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    void onGetExp(SBufferData& bData, SceneNpc* npc, DWORD baseExp, DWORD jobExp);
  private:
    float m_fRate = 0;
};

// gvg延迟奖励
class BuffGvgReward : public BufferState
{
  friend class BufferStateList;
  public:
    BuffGvgReward();
    virtual ~BuffGvgReward();

    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
};

//  捡道具
class BuffPickUp : public BufferState
{
  friend class BufferStateList;
  public:
    BuffPickUp();
    virtual ~BuffPickUp();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    float m_fRange = 0;
    DWORD m_dwItemID = 0;
};

// 护盾
class BuffImmuneAttack : public BufferState
{
  friend class BufferStateList;
  public:
    BuffImmuneAttack();
    virtual ~BuffImmuneAttack();
};

//杀死怪物 掉落指定食材奖励物品
class BuffDropFoodReward : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDropFoodReward();
    virtual ~BuffDropFoodReward();
    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    void onKillMonster(SceneUser* user, SceneNpc* npc, SBufferData& bData);
  private:
    DWORD m_dwIntervalTime = 0;//间隔时间
};

//移除技能状态
class BuffDelSkillStatus : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDelSkillStatus();
    virtual ~BuffDelSkillStatus();

    virtual bool init(DWORD id,xLuaData &data,TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  private:
    ESkillStatus m_eStatus = ESKILLSTATUS_MIN;
};

// 深度隐身
class BuffDeepHide : public BuffHiding
{
  friend class BufferStateList;
  public:
    BuffDeepHide();
    virtual ~BuffDeepHide();
};

// 装备脱卸
class BuffOffEquip : public BufferState
{
  friend class BufferStateList;
public:
  BuffOffEquip();
  virtual ~BuffOffEquip();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);

private:
  DWORD m_dwCalcType = 0;
};

// 装备保护
class BuffProtectEquip : public BufferState
{
  friend class BufferStateList;
public:
  BuffProtectEquip();
  virtual ~BuffProtectEquip();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  virtual void ondel(xSceneEntryDynamic *me);

private:
  set<EEquipPos> m_setPos;
  bool m_bAlways = false;
};

// 装备修理
class BuffFixEquip : public BufferState
{
  friend class BufferStateList;
public:
  BuffFixEquip();
  virtual ~BuffFixEquip();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);

private:
  set<EEquipPos> m_setPos;
};

// 技能额外目标
class BuffSkillTarget : public BufferState
{
  enum EBuffSkillTargetType
  {
    EBUFFSKILLTARGET_TEAMHPMIN = 1,
  };
  friend class BufferStateList;
  public:
    BuffSkillTarget();
    virtual ~BuffSkillTarget();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    QWORD getTarget(SBufferData& data, DWORD skillid, const TSetQWORD& nowTargets);
  private:
    EBuffSkillTargetType m_eTargetType = EBUFFSKILLTARGET_TEAMHPMIN;
    DWORD m_dwSkillID = 0;
};

// 回归额外奖励buff
class BuffExtraReward : public BufferState
{
  friend class BufferStateList;
  public:
    BuffExtraReward();
    virtual ~BuffExtraReward();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    EExtraRewardType getRewardType() { return m_eType; }
    float getExtraZenyRatio() const { return m_fZenyRatio; }
    float getExtraExpRatio() const { return m_fExpRatio; }
  private:
    EExtraRewardType m_eType = EEXTRAREWARD_MIN;
    float m_fZenyRatio = 0;
    float m_fExpRatio = 0;
};

// 景点buff
class BuffScenery : public BufferState
{
  friend class BufferStateList;
public:
  BuffScenery();
  virtual ~BuffScenery();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

  DWORD getSceneryID() { return m_dwSceneryID; }

private:
  DWORD m_dwSceneryID = 0;
};

// hp总存储buff
class BuffHPStorage: public BufferState
{
  friend class BufferStateList;
public:
  BuffHPStorage();
  virtual ~BuffHPStorage();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  virtual bool onStart(SBufferData& bData, DWORD cur);
private:
  DWORD m_dwHPAdd = 0;
  DWORD m_dwHPTriggerPercent = 0; // buff效果能否添加放在doBuffEffect中判断
};

// sp总存储buff
class BuffSPStorage: public BufferState
{
  friend class BufferStateList;
public:
  BuffSPStorage();
  virtual ~BuffSPStorage();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);

  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  virtual bool onStart(SBufferData& bData, DWORD cur);
private:
  DWORD m_dwSPAdd = 0;
  DWORD m_dwSPTriggerPercent = 0; // buff效果能否添加放在doBuffEffect中判断
};

/**
 * @brief 宠物冒险Buffer,主要影响冒险额外奖励
 */ 
class BuffPetAdventure : public BufferState 
{
  friend class BufferStateList;
public:
  BuffPetAdventure();
  virtual ~BuffPetAdventure();
  
  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  
  float getBase(){ return m_fBase ; }
  float getJob() { return m_fJob;  }
  float getRatioByQualify(DWORD qualify)
  {
    if (m_mapMaterial.find(qualify) == m_mapMaterial.end())
      return 0.0f;
    return m_mapMaterial[qualify].first ;
  }
  DWORD getNumByQualify(DWORD qualify)
  {
    if (m_mapMaterial.find(qualify) == m_mapMaterial.end())
      return 0;
    return m_mapMaterial[qualify].second ;
  }
  DWORD getRatioNotConsume() {return m_dwNotCostBall;}
  float getRatioReduceBattleTime() { return m_fFightTime ; }
  float getRatioReduceConsumeTime() {return m_fConsumeTime ; }
  float getRatioKind() { return m_fKind; }
  float getRatioCard() { return m_fCard;}
  DWORD getRatioRareBox() { return m_dwRareBox;}
private:
  // <材料品质， 额外收益率>
  typedef std::pair<float, DWORD> RatioNumPair;
  typedef std::map<DWORD, RatioNumPair> TMapQualifyRatio; 
  TMapQualifyRatio m_mapMaterial;
  float m_fCard           = 0.0f; //稀有卡牌 
  float m_fConsumeTime    = 0.0f; //外出时间减少 
  float m_fFightTime      = 0.0f; //战斗时间减少 
  float m_fBase           = 0.0f; //Base经验     
  float m_fJob            = 0.0f; //Job经验       
  DWORD m_dwNotCostBall   = 0; //不消耗丸子    
  float m_fKind           = 0.0f; //素材种类
  DWORD m_dwRareBox       = 0; //稀有箱子
};

/**
 * @brief 技能反弹 
 */
class BuffStatusRebound : public BufferState
{
  friend class BufferStateList;
  public:
    BuffStatusRebound();
    virtual ~BuffStatusRebound();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    // 判断是否状态可以反弹
    bool canRebound(EBuffStatusType status);
  private:
    typedef std::set<EBuffStatusType> TSetBuffStatus;
    TSetBuffStatus m_setStatus;    
    DWORD m_dwOdds = 0;
};

// 武器防御
class BuffWeaponBlock : public BufferState
{
  friend class BufferStateList;
public:
  BuffWeaponBlock();
  virtual ~BuffWeaponBlock();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  DWORD getStiff() { return m_dwStiff; }
private:
  DWORD m_dwStiff = 0;
};

// 属性控制
class BuffAttrControl : public BufferState
{
  friend class BufferStateList;
public:
  BuffAttrControl();
  virtual ~BuffAttrControl();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool needRefreshAllBuff() const { return true; }
  const TMapAttrControl& getAttr() { return m_mapAttr; }
private:
  TMapAttrControl m_mapAttr;
};

class BuffRideWolf : public BufferState 
{
  friend class BufferStateList;
public:
  BuffRideWolf();
  virtual ~BuffRideWolf();
  bool onStart(SBufferData& bData, DWORD cur);
  virtual void ondelLater(xSceneEntryDynamic *me);
private:
  DWORD dwPartnerID = 0;
};

// 禁用装备位
class BuffForbidEquip : public BufferState
{
  friend class BufferStateList;
public:
  BuffForbidEquip();
  virtual ~BuffForbidEquip();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  bool isEquipForbidOn(EPackType pack, EEquipPos pos) const;
  bool isEquipForbidOff(EPackType pack, EEquipPos pos) const;

private:
  map<EPackType, set<EEquipPos>> m_mapPack2ForbidOnPos;
  map<EPackType, set<EEquipPos>> m_mapPack2ForbidOffPos;
  map<EPackType, set<EEquipPos>> m_mapPack2OffPos;
};

// 概率解锁食谱
class BuffUnlockRecipe : public BufferState
{
  friend class BufferStateList;
public:
  BuffUnlockRecipe();
  virtual ~BuffUnlockRecipe();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);

private:
  DWORD m_dwRecipeID = 0;
  DWORD m_dwProbability = 0;
};

// 属性转移
class BuffAttrTransfer : public BufferState
{
  friend class BufferStateList;
public:
  BuffAttrTransfer();
  virtual ~BuffAttrTransfer();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
  virtual void ondel(xSceneEntryDynamic *me);
  virtual bool getMyAttr(xSceneEntryDynamic* me, const SBufferData* pData, TVecAttrSvrs& getMyAttr) { return true; }
};

// 锁定目标(怪物用)
class BuffChase : public BufferState
{
  friend class BufferStateList;
public:
  BuffChase()
  {
    m_eBuffType = EBUFFTYPE_CHASE;
  }
  virtual ~BuffChase() {}
};

// 免疫嘲讽
class BuffImmuneTaunt : public BufferState
{
  friend class BufferStateList;
public:
  BuffImmuneTaunt()
  {
    m_eBuffType = EBUFFTYPE_IMMUNETAUNT;
  }
  virtual ~BuffImmuneTaunt() {}
};

//交换SP
class BuffSPExchange : public BufferState
{
  friend class BufferStateList;
public:
  BuffSPExchange();
  virtual ~BuffSPExchange();
  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
private:
  DWORD m_dwSPRecoverPercent = 0;
};

// 反隐
class BuffDHide : public BufferState
{
  friend class BufferStateList;
  public:
    BuffDHide();
    virtual ~BuffDHide();

    bool onStart(SBufferData& bData, DWORD cur);
    virtual void ondel(xSceneEntryDynamic *me);
};

// 队员距离近, 添加连线
class BuffCheckAddLine : public BufferState
{
  friend class BufferStateList;
  public:
    BuffCheckAddLine();
    virtual ~BuffCheckAddLine();

    virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
    virtual bool doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData);
    virtual void ondel(xSceneEntryDynamic *me);
  private:
    DWORD m_dwLineID = 0;
    float m_fRange = 0;
    TSetDWORD m_setAddBuffs;
};

// 不可使用地图传送阵
class BuffNoMapExit : public BufferState
{
  friend class BufferStateList;
  public:
    BuffNoMapExit();
    virtual ~BuffNoMapExit();
};

// 异常状态传染
class BuffInfect : public BufferState
{
  friend class BufferStateList;
public:
  BuffInfect();
  virtual ~BuffInfect();

  virtual bool init(DWORD id, xLuaData& data, TPtrBuffCond buffCond);
  virtual void ondel(xSceneEntryDynamic* me);
private:
  float m_fRange = 0;
};

//---------------------------------------------
//---------------------------------------------
//---------------------------------------------

//buff 数据结构
struct SBufferData
{
  DWORD id = 0;
  DWORD layers = 0;

  QWORD endTime = 0;
  DWORD count = 0;

  DWORD lv = 0;
  bool activeFlag = false;
  bool bSyncNine = false;

  xSceneEntryDynamic *me;
  UINT fromID = 0;
  string strFromName;

  TPtrBufferState pBuff;

  DWORD addTime = 0;
  DWORD hpOnAdd = 0;

  xPos oldPos;
  QWORD timeTick = 0;

  DWORD dwCommonData = 0;
  int dwDamage = 0;

  DWORD dwTotalDamage = 0;
  DWORD dwAttackCount = 0;
  DWORD dwBeAttackCount = 0;

  DWORD dwSpareTime = 0;
  TVecAttrSvrs m_oAttr;
  TVecDWORD vecCommonData;
  queue<QWORD> queueEndTime;

  DWORD condSkillID = 0;

  bool canNotTrig = false;

  const TVecAttrSvrs& getAttr();
  void calcAttr();

  void setStatusNoDisp() { if (vecCommonData.empty()) vecCommonData.push_back(0); vecCommonData[0] |= 1; }
  bool isStatusNoDisp() { return !vecCommonData.empty() && (vecCommonData[0] & 1); }

  bool canRemove() { return endTime != 0; }
  DWORD getLimitLayer();
  void setCondSkillID(DWORD skillid) { condSkillID = skillid; }
  void setCanNotTrig(bool flag) { canNotTrig = flag; }
};
typedef std::vector<SBufferData> TVecSBuffData;

struct SBufferUpdateData
{
  DWORD id = 0;

  QWORD fromID = 0;
  DWORD lv = 0;
  int dwDamage = 0;
  QWORD endTime = 0;
  DWORD layer = 0;

  bool bAllLayer = false;
  bool bIgnoreOdds = false;
};
typedef std::list<pair<bool, SBufferUpdateData>> TListUpdateData;

typedef std::map<DWORD, DWORD> TMapBuffID2Lv;

class ItemBase;

enum EHideState
{
  EHideState_None = 0,
  EHideState_Hide = 1,
  EHideState_Hided = 2,  
  EHideState_Show = 3,
};

class BufferStateList
{
  public:
    BufferStateList(xSceneEntryDynamic *entry);
    ~BufferStateList();

  public:
    bool add(DWORD id, xSceneEntryDynamic *fromEntry = nullptr, DWORD lv = 0, int damage = 0, QWORD endtime = 0, bool ignoreOdds = false);
    bool del(DWORD id, bool allLayer = false);// allLayer = true, 表示无视当前层数直接删除, = false 表示删除一层
    bool delLayer(DWORD id, DWORD layer); // 删除指定层数
    bool active(DWORD id);
    bool canAdd(TPtrBufferState buffPtr);
    void addLayers(DWORD id, DWORD layers);

    bool isLayerEnough(EBuffType eType);

    //void addNext(const SBufferData& sData);
    //void delNext(DWORD id) { m_vecNeedDel.push_back(id); }
    bool haveBuff(DWORD id) { return getBuffData(id) != nullptr; }
    bool haveBuffType(EBuffType eType) { return m_mapType2SetBuff.find(eType) != m_mapType2SetBuff.end(); }
    void getAllSkillSpec(SSpecSkillInfo& info);
    void getVecBuffDataByBuffType(EBuffType eType, TVecSBuffData& vecBuffData);

    void timer(QWORD curm);

    void enableBuff(DWORD id);
    void disableBuff(DWORD id);
    void enableType(EBuffType eType);
    void disableType(EBuffType eType);
    void changeBuffSpeed(DWORD buffid, int speedMs);

    void save(Cmd::BlobBuffer *data);
    void load(const Cmd::BlobBuffer &data);
    void loadFromUser();

    template<class T> void foreach(T func) { for_each(m_mapID2BuffData.begin(), m_mapID2BuffData.end(), [func](std::map<DWORD, SBufferData>::value_type r) { func(r.second);}); 
      for_each(m_mapID2StaticBuff.begin(), m_mapID2StaticBuff.end(), [func](std::map<DWORD, SBufferData>::value_type r) { func(r.second); });
    }

    template<class T> void foreachDynamicBuff(T func){
      for_each(m_mapID2BuffData.begin(), m_mapID2BuffData.end(), [func](std::map<DWORD, SBufferData>::value_type &r) { func(r.second); });
    }
    template<class T> void foreachStaticBuff(T func){
      for_each(m_mapID2StaticBuff.begin(), m_mapID2StaticBuff.end(), [func](std::map<DWORD, SBufferData>::value_type &r) { func(r.second); });
    }

    void onLeaveScene();
    void onEnterScene();
  private:
    bool realAdd(const SBufferUpdateData& stData);
    bool realDel(DWORD id, bool bAllLayer, DWORD layer);
    void onAdd(TPtrBufferState buffPtr, QWORD fromid, DWORD lv, bool repeat);
    void onAddLater(TPtrBufferState buffPtr, QWORD fromid, DWORD level, QWORD endtime, bool ignoreOdds);
    bool checkOffSet(TPtrBufferState buffPtr);
  public:
    // 触发buff生效条件
    void onEquipChange(ItemBase* pItem, EEquipOper oper);
    void onCardChange(DWORD dwTypeID, bool isAdd);
    void onMountChange();
    void onProfesChange(EProfession oldProfes);
    void onAttrChange();
    void onAttackMe(xSceneEntryDynamic* attacker, const BaseSkill* pSkill, DamageType damtype, DWORD damage = 0);
    void onAttack(xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, DWORD damage);
    void onUseSkill(const BaseSkill* pSkill);
    void onChantStatusChange();
    void onHpChange();
    void onSpChange();
    void onEmoji(DWORD id);
    void onRelive();
    void onClientMove();
    void onBeShoot();
    void onTeamChange();
    void onHandChange();
    void onApproachDie();

    void onGetMonsterExp(SceneNpc* npc, DWORD baseExp, DWORD jobExp);
    void onPickUpItem(ItemInfo& oItem);
    void onKillMonster(SceneNpc* npc, EBuffType buffType);
    void onAddBattleTime(DWORD time);
    void onBattleStatusChange();
    void onEquipRefineChange(ItemBase* pItem, bool success);
    SDWORD onDropBaseExp(ESource source);
    void onBeingChange();
    void onRideMountChange(ItemBase* pItem);
    void onChangeGender();
    void onChangeMap();
    void onSkillStatusChange();

    void onChangeElement();

  public:
    bool delSkillBuff(DWORD skillid); //passive skill
    bool addSkillBuff(DWORD skillid); //passive skill

    bool delEquipBuff(ItemBase* pItem);
    bool addEquipBuff(ItemBase* pItem);
    bool addEquipBuffBreakValid(ItemBase *pItem);
    bool addEquipBuffBreakInvalid(ItemBase *pItem);
    bool addEquipBuffPVP(ItemEquip *pEquip);
    bool delEquipBuffBreakValid(ItemBase* pItem);
    bool delEquipBuffBreakInvalid(ItemBase* pItem, bool bWhenBreak = false);
    bool delEquipBuffPVP(ItemEquip* pEquip);

    bool delCardBuff(const SItemCFG* pItem);
    bool addCardBuff(const SItemCFG* pItem);

  private:
    bool addSuitRefineBuff(SceneUser* pUser, const SSuitCFG* pCFG, ItemEquip* pEquip, EquipPackage* pEquipPack);
    bool delSuitRefineBuff(SceneUser* pUser, const SSuitCFG* pCFG, ItemEquip* pEquip, EquipPackage* pEquipPack);

  public:
    bool isInStatus(DWORD statusID);
    bool isImmuneStatus(DWORD statusID);
    bool delStatus(DWORD statusID);
    bool delStatus(const TSetDWORD& setStatus);
    void getStatus(set<DWORD>& stSet);
    DWORD getStatusAttr();
    DWORD getLayerByID(DWORD id);
    DWORD getEndTimeByID(DWORD id); // 返回buff剩下多少毫秒结束
    QWORD getBuffDelTime(DWORD id);

    bool delBuffByType(EBuffType type);

    void addBuffDamage(float damage);
    void delBuffByGain(bool bGain, DWORD num);
  public:
    // 魔物变身
    DWORD getTransform(EUserDataType uType);
    void setTransform(bool flag) { if (m_bTransformNoBuff == flag) return; m_bDelSkillEquipBuff = flag; m_bAddSkillEquipBuff = !flag; m_bTransformNoBuff = flag;}

    // 部分变装
    void setPartTransform(bool flag){ m_bPartTransform = flag; }
    bool hasPartTransform(EUserDataType eType);
    DWORD getPartTransform(EUserDataType eType);
    float getBodyScalePer();

    void setStatusChange() { m_bIsInStatus = true; }

    // body color
    DWORD getShaderColor();

    DWORD getPackageSlot(EPackType eType);
    void getSpecSkillInfo(DWORD familySkillID, SSpecSkillInfo& info);
  public:
    void reloadAllBuff();
    void setClearState() { m_bClear = true; }

    void setForceAttr(bool flag);
    bool haveForceAttr() { return m_bHaveForceAttr; }
    void getForceAttr(TVecAttrSvrs& attrs);

    bool haveLimitAttr() const;
    void getLimitAttrs(map<EAttrType, float>& minattrs, map<EAttrType, float>& maxattrs);

    bool haveLimitSkill() const;
    //const TSetDWORD& getLimitSkill();
    bool isSkillNotLimited(DWORD skillid);
    QWORD getLimitSkillTarget(DWORD skillId);

    QWORD getSkillExtraTarget(DWORD skillid, const TSetQWORD& nowTargets);

    void addDisableID(DWORD buffid) { m_setUpdateActive.insert(buffid); /*m_setDisables.insert(buffid);*/ }
    void delDisableID(DWORD buffid) { m_setUpdateActive.insert(buffid); /*m_setDisables.erase(buffid);*/ }

    bool haveLimitUseItem() const { return m_mapType2SetBuff.find(EBUFFTYPE_LIMITUSEITEM) != m_mapType2SetBuff.end(); }
    bool canUseItem(DWORD itemtype);

    bool isUndead();// { return m_mapType2SetBuff.find(EBUFFTYPE_UNDEAD) != m_mapType2SetBuff.end(); }
    bool isForceAddStatus(DWORD status);

    void addUpdate(const SBufferData& upData);

    //类外谨慎调用, 将缓存buff数据刷到m_mapID2BuffData, 用于上下线保存
    void update(QWORD curm);
    void updateOneSecond(QWORD curm);
    
    DWORD getMultiTimeRate();
    DWORD getRobRewardRate();
    void clientHideBuff(DWORD buffid);
    void clearClientHideBuff();
    bool checkHasHideBuff();

    float getExtraExpRatio(EExtraRewardType eType);
    float getExtraZenyRatio(EExtraRewardType eType);
    void onFinishEvent(EExtraRewardType eType);

    void removeTauntBuffByFromID(QWORD fromid);
    void clear(const TSetDWORD& excepts = TSetDWORD{});

    void getControlledAttr(TMapAttrControl& attrs);
    bool isEquipForbid(EPackType pack, EEquipPos pos, EEquipOper oper);
  
    void refreshBuffAtonce();
    bool hasConcertAffactSkillBuff();
  //lua 调用
  public:
    void getBuffListByName(const char* name, SLuaNumberArray& result);
    QWORD getBuffFromID(DWORD buffid);
    DWORD getLevelByID(DWORD id);

    DWORD getAutoBlockStiff();
    bool hasSceneryID(DWORD sceneryid);
    DWORD getWeaponBlockStiff();

  public:
    void loadSkillBuff();

  private:
    void loadEquipBuff();
    void loadRuneBuff();
    void loadManualBuff();
    void loadConfigBuff();

    void updateBuffToNine();
    void updatePermanentBuff();

    void delSkillEquipBuff();
    void addSkillEquipBuff();

    void setDataMark(TPtrBufferState buffPtr);

    void addTypeBuff(TPtrBufferState buffPtr, DWORD id);
    void delTypeBuff(TPtrBufferState buffPtr, DWORD id);

    SBufferData* getBuffData(DWORD id);
  private:
    std::map<DWORD, SBufferData> m_mapID2BuffData;
    std::map<DWORD, SBufferData> m_mapID2StaticBuff;
    std::map<DWORD, DWORD> m_mapID2Protecttime;
    std::map<EBuffType, TSetDWORD> m_mapType2SetBuff;
    std::map<BuffTriggerType, TSetDWORD> m_mapTrigType2Set;

    xSceneEntryDynamic *m_pEntry;

    TVecSBuffData m_vecAddUpdates;

    //TVecSBuffData m_vecNeedAdd;
    //set<DWORD> m_setNeedDel;
    //TVecDWORD m_vecNeedDel;
    TListUpdateData m_listUpdateData;

    bool m_bTransformNoBuff = false;
    bool m_bDelSkillEquipBuff = false;
    bool m_bAddSkillEquipBuff = false;

    TSetDWORD m_setPmtBuffUpdates; // 永久属性

    bool m_bPartTransform = false;
    QWORD m_qwTimer = 0; // 100ms 单位
    QWORD m_qwOneSecond = 0;

    bool m_bIsInStatus = false;

    bool m_bClear = false;

    bool m_bHaveForceAttr = false;
    bool m_bNoBuffMsg = false;
    bool m_bLoadingFromUser = false;
  private:
    void updateBuffStatus(DWORD cur);
    void updateDisable(DWORD cur);
    bool canAddStatus(DWORD status) const;
    bool canReboundStatus(EBuffStatusType status); //判断状态是否可以反弹
    void checkBuffEnable(EBuffType eType);
    void checkBuffEnable(BuffTriggerType eTrigType);
    void checkBuffEnable(const TSetDWORD& ids);
  private:
    DWORD m_dwStatusTimeTick = 0;
    map<DWORD, pair<DWORD, DWORD>> m_mapStatus2Record; // status -> pair(time, period)

    TSetDWORD m_setDisables; // 无效的buff
    TSetDWORD m_setUpdateActive; // 刷新有效性的列表, 用完清空

    TSetDWORD m_setClientHideBuff;
    EHideState m_eHideState = EHideState_None;
};


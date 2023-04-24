/**
 * @file Skill.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-08-07
 */

#pragma once

#include <string>
#include "SceneSkill.pb.h"
#include "xEntry.h"
#include "xSceneEntry.h"
#include "xLuaTable.h"
#include "xPos.h"
#include "NpcConfig.h"
#include "SkillConfig.h"

using std::string;
using std::pair;
using std::vector;
using std::set;
using namespace Cmd;

#define RANGE_ERROR 1
#define DAMAGE_ERROR 5
#define INVALID_DAMAGE -987654321
#define CD_ERROR 800
#define CHECK_DIS_PARAM(skilldist, checkdist)\
        float checkdist;\
      if (skilldist <= 3) checkdist = (skilldist + 2) * 1.5;\
      else checkdist = skilldist * 1.5 + 2;\

class xSceneEntryDynamic;
class SceneUser;
class SkillRunner;

enum ESkillNumber
{
  ESKILLNUMBER_CAIJI = -2,  // 采集类
  ESKILLNUMBER_BREAK = -1,  // 打断技能
  ESKILLNUMBER_CHANT = 0,   // 吟唱
  ESKILLNUMBER_ATTACK = 1,  // 攻击
  ESKILLNUMBER_RET = 2,     // 暴风雪类技能
  ESKILLNUMBER_MOVECHANT = 3,// 移动吟唱
};

enum ESkillTrap
{
  ESKILLTRAP_MIN = 0,
  ESKILLTRAP_STORM,
  ESKILLTRAP_FIREWALL,
  ESKILLTRAP_MAX
};

struct SSkillCondition
{
  DWORD dwJobLv = 0;
  DWORD dwMenuID = 0;
  DWORD dwTitleID = 0;

  TVecDWORD vecAllQuest;
  TVecDWORD vecOneQuest;
  TVecDWORD vecPreSkillIDs;

  SSkillCondition() {}
};

enum ESkillLeadType
{
  ESKILLLEADTYPE_MIN = 0,
  ESKILLLEADTYPE_ONE = 1,
  ESKILLLEADTYPE_TWO = 2,
  ESKILLLEADTYPE_THREE = 3,
  ESKILLLEADTYPE_LEAD = 4,
  ESKILLLEADTYPE_MAX,
};
struct SSkillLeadType
{
  ESkillLeadType eType = ESKILLLEADTYPE_MIN;

  float fReadyTime = 0.0f;
  float fDuration = 0.0f;
  float fCCT = 0.0f;
  float fFCT = 0.0f;
  float fDCT = 0.0f;

  SSkillLeadType() {}
};

enum ESkillTrapType
{
  ESKILLTRAPTYPE_MIN = 0,
  ESKILLTRAPTYPE_TRANS = 1,
};

struct SSkillLogicParam
{
  QWORD qwDuration = 0;
  QWORD qwInterval = 0;

  float fRange = 0.0f;
  float fWidth = 0;
  float fDistance = 0;
  DWORD dwCount = 0;
  DWORD dwHitTime = 0;
  DWORD dwPetID = 0;
  float fOffect = 0;
  DWORD dwTrapNpcID = 0;
  DWORD dwRangeMaxNum = 0;
  DWORD dwMaxSkillCount = 0;
  DWORD m_dwDispHideType = 0;

  bool bIsCountTrap = false;
  bool bIsTimeTrap = false;

  bool bIsNpcTrap = false;

  bool bClientNoSelect = false;
  bool bSelectTarget = false;
  bool bIncludeSelf = false;

  float fTeamRange = 0;
  xLuaData m_oParam;

  ENpcType eInvalidTarType = ENPCTYPE_MIN;
  ESkillTrapType eTrapType = ESKILLTRAPTYPE_MIN;

  DWORD dwSuckSpType = 0; // 吸蓝类型, != 0 时表示有吸蓝效果
  bool m_bBuffOnlyMajor = false; // buff 只影响主目标
  bool m_bNoHitBackMajor = false; // 不会击退主目标
  bool m_bZeroDamHitBack = false; //伤害为0时(伤害类型非miss等)仍然可以击退
  bool m_bImmunedBySuspend = false; //是否被悬浮免疫
  bool m_bNotImmunedByFieldArea = false; //是否不能被地领域免疫

  bool m_bForceHeal = false;
  bool m_bIsMagicMachine = false;

  TVecDWORD m_vecChatid; // 释放技能时候发出的对话
  bool m_bSelectHide = false; //是否选择隐匿单位

  SSkillLogicParam() {}
};

enum EReleaseCond
{
  ERELEASECOND_MIN = 0,
  ERELEASECOND_USESKILL = 1,
  ERELEASECOND_EQUIP = 2,
  ERELEASECOND_HPLESS = 3,
  ERELEASECOND_ATTR = 4,
  ERELEASECOND_PET = 5,
  ERELEASECOND_BUFF = 6,
  ERELEASECOND_HAVESKILL = 7,
  ERELEASECOND_BEING = 8,
  ERELEASECOND_OFFEQUIP = 9,
  ERELEASECOND_DAMEQUIP = 10,
  ERELEASECOND_MAX,
};

struct SSkillReleaseCond
{
  EReleaseCond eType = ERELEASECOND_MIN;

  DWORD param1 = 0;
  DWORD param2 = 0;
  UserAttrSvr uAttr;
};

struct SRealeaseConds
{
  bool bNeedBoth = false;
  vector<SSkillReleaseCond> vecConds;
  DWORD dwReleaseLuaFunc = 0;
};

enum ESkillAttrType
{
  ESKILLATTRTYPE_MIN = 0,
  ESKILLATTRTYPE_NORMAL = 1,
  ESKILLATTRTYPE_MAGIC = 2,
  ESKILLATTRTYPE_MAX,
};

enum ESkillState
{
  ESKILLSTATE_MIN = 0,
  ESKILLSTATE_PREPARE,
  ESKILLSTATE_CHANT,
  ESKILLSTATE_RUN,
  ESKILLSTATE_END,
};

enum ESkillCost
{
  ESKILLCOST_MIN = 0,
  ESKILLCOST_ARROW = 1,
  ESKILLCOST_BUFF = 2,
  ESKILLCOST_MAX,
};

struct SExtraSkillCost
{
  ESkillCost eType = ESKILLCOST_MIN;
  DWORD dwCostID = 0;
  DWORD dwCostNum = 0;
};
typedef vector<SExtraSkillCost> TVecExtraSkillCost;

struct SSkillCost
{
  DWORD dwSp = 0;
  DWORD dwHp = 0;
  vector<pair<DWORD, DWORD>> vecItem2Num;
  vector<pair<DWORD, DWORD>> vecBuff2Num;

  DWORD dwDynamicCostType = 0;
  DWORD dwMaxSpPer = 0;
};

enum ETransportType
{
  ETRANSPORTTYPE_MIN = 0,
  ETRANSPORTTYPE_GUILD = 1,
  ETRANSPORTTYPE_WEDDING_HONEYMOON = 2,
  ETRANSPORTTYPE_TRANSFER = 3,
  ETRANSPORTTYPE_MAX = 3,
};

// base skill
class BaseSkill : public xEntry
{
  public:
    BaseSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~BaseSkill();

    virtual bool init(xLuaData& params);

    // 技能逻辑参数
    QWORD getDuration(xSceneEntryDynamic* attacker) const; //{ return (isTrap() == true) ? m_stLogicParam.qwDuration : 0; }
    QWORD getInterval() const { return m_stLogicParam.qwInterval; }
    DWORD getCount(xSceneEntryDynamic* attacker) const; //{ return m_stLogicParam.dwCount; }
    DWORD getPetID() const { return m_stLogicParam.dwPetID; }
    DWORD getLimitCount(xSceneEntryDynamic* attacker) const;
    DWORD getTargetNumLimit(xSceneEntryDynamic* me) const;
    bool isContinueSkill() const { return m_stLogicParam.qwDuration || m_stLogicParam.dwCount > 1; }
    const xLuaData& getLuaParam() const { return m_stLogicParam.m_oParam; }
    bool canImmunedByFieldArea() const { return !m_stLogicParam.m_bNotImmunedByFieldArea; }

    // 技能属性
    const string& getName() const { return m_strName; }
    DWORD getSkillID() const { return m_dwSkillID; }
    DWORD getNextSkillID() const { return m_dwNextSkillID; }
    DWORD getBreakSkillID() const { return m_dwBreakSkillID; }
    DWORD getDeadSkillID() const { return m_dwDeadSkillID; }
    DWORD getDeadLvReq() const { return m_dwDeadLv; }
    DWORD getLevelCost() const { return m_dwLevelCost; }
    DWORD getFamilyID() const { return m_dwSkillID / 1000 * 1000; }
    ESkillType getSkillType() const { return m_eType; }
    ESkillLogic getLogicType() const { return m_eLogic; }
    bool haveNoTargets() const { return m_eLogic == ESKILLLOGIC_MIN && !m_stLogicParam.bSelectTarget && !m_stLogicParam.bIncludeSelf; }
    ESkillCamp getSkillCamp() const { return m_eCamp; }
    ESkillLeadType getLeadType() const { return m_stLeadType.eType; }
    ESkillTrapType getTrapType() const { return m_stLogicParam.eTrapType; }
    bool haveDirectBuff() const { return m_eType == ESKILLTYPE_PASSIVE || m_eType == ESKILLTYPE_FUNCTION; }
    bool isSkillNoDelay() const { return m_stLeadType.eType == ESKILLLEADTYPE_LEAD; }
    bool noHpBreak() const { return m_bNoHpBreak; }
    DWORD getBuffBreakLimit() const { return m_dwBuffBreakLimit; }

    //伤害所需参数
    DWORD getAtkAttr() const { return m_dwAtkAttrType; }
    DWORD getSuperUse() const { return m_dwSuperUse; }
    DWORD getSuperUseEff() const { return m_dwSuperUseEff; }

    // 技能释放属性
    float getReadyTime(xSceneEntryDynamic* attacker, bool bOriginal = false) const;
    float getLaunchRange(xSceneEntryDynamic* attacker) const;
    DWORD getCD(xSceneEntryDynamic* attacker) const;
    DWORD getDelayCD(xSceneEntryDynamic* attacker) const;

    bool checkSkillCost(xSceneEntryDynamic* attacker) const;
    void doSkillCost(xSceneEntryDynamic* attacker, SkillRunner& oRunner) const;
    DWORD getSpCost(SceneUser* pUser) const;
    bool isCostBeforeChant() const { return m_eType == ESKILLTYPE_LEAD || m_eType == ESKILLTYPE_SWORDBREAK; }

    //技能自身属性
    bool isConsumeSkill() const { return m_isConsumer; }
    bool isNormalSkill(xSceneEntryDynamic* attacker) const;
    bool isNearNormalSkill() const { return m_bNearNormalSkill; }
    bool isOutRangeBreak() const { return (m_eLogic == ESKILLLOGIC_LOCKEDTARGET || m_eLogic == ESKILLLOGIC_MISSILE)
      && (m_stLeadType.eType == ESKILLLEADTYPE_ONE || m_stLeadType.eType == ESKILLLEADTYPE_TWO); }
    //bool canBeBreak(xSceneEntryDynamic* attacker) const;
    bool isCDBeforeChant() const { return m_stLeadType.eType == ESKILLLEADTYPE_THREE; }
    bool isLongSkill() const { return m_dwLaunchRange >= 3; }
    bool isPointSkill() const { return m_eLogic == ESKILLLOGIC_POINTRECT || m_eLogic == ESKILLLOGIC_POINTRANGE; }
    bool haveDamage() const { return m_bHaveDamage; }
    bool isShareSkill() const { return m_bShareSkill; }
    bool hasAttMove() const { return m_fAttMove != 0; }

    bool canEquipAuto() const { return m_bCanEquipAuto; }
    bool checkCondition(xSceneEntryDynamic* entry) const;
    bool checkCanUse(xSceneEntryDynamic* entry) const;
    bool checkUseItem(xSceneEntryDynamic* entry, QWORD targetid) const;

    //技能携带的自身buff
    const TSetDWORD& getSelfBuffs() const { return m_selfbuffs; }
    const TSetDWORD& getEnemyBuffs() const { return m_enemybuffs; }
    const TSetDWORD& getPetMasterBuffs(bool pvp = false) const { return pvp ? m_setPetMasterBuffs : m_pvpPetMasterBuffs; }

    // test print
    DWORD printDamage(xSceneEntryDynamic* atter, xSceneEntryDynamic* defer, const char* message1, const char* message2,const char* message3,const char* message4,DWORD type);
  public:
    // 技能释放流程
    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool chant(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
    virtual bool trap(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;

    virtual void onBeBreak(SkillRunner& oRunner) const {}
    virtual void runSpecEffect(SkillRunner& oRunner) const {}
    virtual bool canUseToTarget(xSceneEntryDynamic* attacker, const SkillBroadcastUserCmd& oCmd) const { return true; }
    virtual void onReset(xSceneEntryDynamic* pEntry) const {};
  protected:
    bool checkAttacker(xSceneEntryDynamic* &attacker) const;
    xSceneEntryDynamic* getRealAttacker(xSceneEntryDynamic* attacker) const;

    void handleDamage(xSceneEntryDynamic* enemy, xSceneEntryDynamic* attacker, HitedTarget* hit, DWORD targetsNum, SkillRunner& oRunner) const;

    bool collectTarget(SkillRunner& oRunner) const;
    bool collectDamage(SkillRunner& oRunner) const;
    bool runEffect(SkillRunner& oRunner) const;
    bool addBuff(SkillRunner& oRunner) const;

    void sendSkillStatusChant(SkillRunner& oRunner) const;
    void sendSkillStatusRun(SkillRunner& oRunner) const;
    void sendSkillStatusHit(SkillRunner& oRunner) const;

  protected:
    bool isCountTrap() const { return m_stLogicParam.bIsCountTrap; }
    bool isTimeTrap() const { return m_stLogicParam.bIsTimeTrap; }
    bool isNpcTrap() const { return m_stLogicParam.bIsNpcTrap; }
    bool isTrap() const { return isCountTrap() || isTimeTrap() ;}
    bool isClientNoSelect() const { return m_stLogicParam.bClientNoSelect || m_eLogic == ESKILLLOGIC_RANDOMRANGE; }
    bool isRangeSkill() const { return m_eLogic == ESKILLLOGIC_POINTRECT || m_eLogic == ESKILLLOGIC_POINTRANGE || m_eLogic == ESKILLLOGIC_SELFRANGE || m_eLogic == ESKILLLOGIC_FORWARDRECT; }
    bool havebuff() { return !m_selfbuffs.empty() || !m_teambuffs.empty() || !m_enemybuffs.empty() || !m_friendbuffs.empty() || !m_selfSkillBuffs.empty() || !m_setPetTeamBuffs.empty() || !m_setBeingSelfBuffs.empty() || !m_setPetMasterBuffs.empty() || !m_setSelfOnceBuffs.empty(); }
    bool isClientNeedTarget() const { return m_bHaveDamage || m_eLogic == ESKILLLOGIC_LOCKEDTARGET; }
    bool isLeadSkill() const { return m_stLeadType.eType == ESKILLLEADTYPE_LEAD; }
    bool checkHitback(int damage, DamageType etype) const;
    bool checkNoHitback(xSceneEntryDynamic* attacker) const;

  protected:
    void getSkillItemCost(map<DWORD, int>& item2num, SceneUser* pUser, DWORD beingid) const;
  public:
    /*是不是魔法技能*/
    bool isMagicSkill()const { return m_eAttrType == ESKILLATTRTYPE_MAGIC;} 
    /*是不是物理技能*/
    bool isPhySkill()const { return m_eAttrType == ESKILLATTRTYPE_NORMAL; } 

  protected:
    string m_strName;
    DWORD m_dwSkillID = 0;
    DWORD m_dwNextSkillID = 0;
    DWORD m_dwBreakSkillID = 0;
    DWORD m_dwDeadSkillID = 0;
    DWORD m_dwDeadLv = 0;
    DWORD m_dwLevelCost = 0;
    DWORD m_dwSuperUse = 0;
    DWORD m_dwSuperUseEff = 0;
    DWORD m_dwForbidUse = 0;
    float m_dwLaunchRange = 0.0f;

    bool m_bNoHpBreak = false;
    DWORD m_dwBuffBreakLimit = 0;

    DWORD m_dwAtkAttrType = 0;

    DWORD m_dwDamTime = 0;
    float m_fCD = 0;
    float m_fHitBack = 0.0f;
    float m_fHitSpeed = 0.0f;
    float m_fDelayCD = 0.0f;
    float m_fAttMove = 0.0f;

    bool m_isConsumer = false;
    bool m_bNearNormalSkill = false;
    bool m_bCanEquipAuto = false;
    bool m_bPvpNoHitBack = false;
    bool m_bHaveDamage = false;
    bool m_bNoOverlay = false;
    bool m_bForceDelHide = false;
    bool m_bFixCD = false;
    bool m_bShareSkill = false;
    bool m_bHaveBuff = false;

    ESkillType m_eType = ESKILLTYPE_MIN;
    ESkillLogic m_eLogic = ESKILLLOGIC_MIN;
    ESkillCamp m_eCamp = ESKILLCAMP_MIN;
    ESkillTrap m_eTrap = ESKILLTRAP_MIN;
    ESkillAttrType m_eAttrType = ESKILLATTRTYPE_MIN;

    TSetDWORD m_selfbuffs; // 只要释放技能就会给自己添加的buff
    TSetDWORD m_selfSkillBuffs; // 只有自己在技能范围目标中时,才会给自己添加的buff（ex:暗之壁障)
    TSetDWORD m_enemybuffs;
    TSetDWORD m_teambuffs;
    TSetDWORD m_friendbuffs;
    TSetDWORD m_setPetSelfBuffs;
    TSetDWORD m_setPetTeamBuffs;
    TSetDWORD m_setPetMasterBuffs;
    TSetDWORD m_setBeingSelfBuffs;
    TSetDWORD m_setGuildBuffs;
    TSetDWORD m_setSelfOnDamageBuffs;
    TSetDWORD m_setSelfOnceBuffs;
    map<DWORD, pair<DWORD, DWORD>> m_mapBuff2MaxNum; //buffid -> (static num, dynamic num type)

    TSetDWORD m_pvpSelfBuffs;
    TSetDWORD m_pvpSelfSkillBuffs;
    TSetDWORD m_pvpEnemyBuffs;
    TSetDWORD m_pvpTeamBuffs;
    TSetDWORD m_pvpFriendBuffs;
    TSetDWORD m_pvpPetSelfBuffs;
    TSetDWORD m_pvpPetTeamBuffs;
    TSetDWORD m_pvpPetMasterBuffs;
    TSetDWORD m_pvpGuildBuffs;
    TSetDWORD m_pvpSelfOnDamageBuffs;
    TSetDWORD m_pvpSetSelfOnceBuffs;
    map<DWORD, pair<DWORD, DWORD>> m_mapPvpBuff2MaxNum;

    SSkillCondition m_stCondition;
    SSkillLeadType m_stLeadType;
    SSkillLogicParam m_stLogicParam;
    SRealeaseConds m_stRealseConds;
    TVecExtraSkillCost m_vecExtraSkillCost;
    SSkillCost m_stSkillCost;
    set<EProfession> m_setIgnoreProfession;
};

// attack skill
class AttackSkill : public BaseSkill
{
  public:
    AttackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~AttackSkill();

    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
};

// buff skill
class BuffSkill : public BaseSkill
{
  public:
    BuffSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~BuffSkill();
};

// passive skill
class PassiveSkill : public BaseSkill
{
  public:
    PassiveSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~PassiveSkill();
};

// heal skill
class HealSkill : public BaseSkill
{
  public:
    HealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~HealSkill();
};

// teleport skill
class TelesportSkill : public BaseSkill
{
  public:
    TelesportSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TelesportSkill();


    virtual bool init(xLuaData& params);
    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
  private:
    bool getRandPos(xSceneEntryDynamic* attacker, xPos& p) const;

    bool m_bContinueAttack = false;
};

// bowlingbash skill
class BowBashSkill : public BaseSkill
{
  public:
    BowBashSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~BowBashSkill();
};

/*class PurifySkill : public BaseSkill
{
  public:
    PurifySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~PurifySkill();

    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
  private:
    bool purify(SkillRunner& oRunner, bool haveChant = true) const;
};*/

// transport skill
class TransportSkill : public BaseSkill
{
  public:
    TransportSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TransportSkill();

    virtual bool run(SkillRunner& oRunner) const;
    virtual bool init(xLuaData& params);
  private:
    bool m_bIncludeFollower = false;
};

// collect skill
class CollectSkill : public BaseSkill
{
  enum ECollctSkillType
  {
    ECOLLECTSKILL_MIN = 0,         // 普通
    ECOLLECTSKILL_MAGICBALL = 1,   //元素法球
    ECOLLECTSKILL_BUFFITEM = 2,    //buff道具
  };
  public:
    CollectSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~CollectSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
  private:
    ECollctSkillType m_eCollectType = ECOLLECTSKILL_MIN;
    TSetDWORD m_setBuffIDs;
};

// summon skill
class SummonSkill : public BaseSkill
{
  public:
    SummonSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~SummonSkill();

    virtual bool init(xLuaData& params);

    virtual bool run(SkillRunner& oRunner) const;
  private:
    DWORD m_dwSumSkillID = 0;
    DWORD m_dwRateType = 0;
    struct SOtherSkillItem
    {
      DWORD dwSumSkillID = 0;
      DWORD dwRateType = 0;
    };
    std::vector<SOtherSkillItem> m_vecOtherSkill;

};

// suicide attack skill
class SuicideSkill : public BaseSkill
{
  public:
    SuicideSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~SuicideSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

class FlashSkill : public BaseSkill
{
  public:
    FlashSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~FlashSkill();

    virtual bool init(xLuaData& params);

    virtual bool run(SkillRunner& oRunner) const;
  private:
    DWORD ghostBuff = 0;
    DWORD demonBuff = 0;
};

class FakeDeadSkill : public BaseSkill
{
  public:
    FakeDeadSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~FakeDeadSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

class ExpelSkill : public BaseSkill
{
  public:
    ExpelSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~ExpelSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

class ReBirthSkill : public BaseSkill
{
  public:
    ReBirthSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~ReBirthSkill();

    virtual bool init(xLuaData& params);
    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
    virtual void onBeBreak(SkillRunner& oRunner) const;
    virtual bool canUseToTarget(xSceneEntryDynamic* attacker, const SkillBroadcastUserCmd& oCmd) const;
  private:
    bool m_bIgnoreGVGCd = false;
};

class GroupBirthSkill : public BaseSkill
{
  public:
    GroupBirthSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~GroupBirthSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

class TrapSkill : public BaseSkill
{
  public:
    TrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TrapSkill();

    virtual bool init(xLuaData& params);

    virtual bool run(SkillRunner& oRunner) const;
  private:
    bool trap(SkillRunner& oRunner) const;
    bool trapfire(SkillRunner& oRunner) const;

    DWORD m_dwTrapSkillID = 0;
};

class RepairSkill : public BaseSkill
{
  public:
    RepairSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~RepairSkill();
};

class BeatBackSkill : public BaseSkill
{
  public:
    BeatBackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~BeatBackSkill();

    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
    virtual void onBeBreak(SkillRunner& oRunner) const;
};

class TrapBarrierSkill : public BaseSkill
{
  public:
    TrapBarrierSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TrapBarrierSkill();

    virtual bool run(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
};

class TriggerTrapSkill : public BaseSkill
{
  public:
    TriggerTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TriggerTrapSkill();
    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
  private:
    void triggerTraps(xSceneEntryDynamic* attacker, bool isAll = true) const; // 触发陷阱,isAll为true触发所有,为false触发m_setNpcid中对应的陷阱
    TSetDWORD m_setNpcid;
};

class StealSkill : public BaseSkill
{
  public:
    StealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~StealSkill();
    virtual bool run(SkillRunner& oRunner) const;
    virtual bool init(xLuaData& params);
  private:
    DWORD m_dwExpression = 0;
    vector<pair<DWORD, DWORD>> m_vecItem2Count;
};

class FunctionSkill : public BaseSkill
{
  public:
    FunctionSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~FunctionSkill();
};

class MarkHealSkill : public BaseSkill
{
  public:
    MarkHealSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~MarkHealSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

class LeadSkill : public BaseSkill
{
  public:
    LeadSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~LeadSkill();

    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool chant(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
};

enum ETrapNpcFuncType
{
  ETRAPNPCFUNC_MIN = 0,
  ETRAPNPCFUNC_BTRANS = 1,
};

class TrapNpcSkill : public BaseSkill
{
  public:
    TrapNpcSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TrapNpcSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
  public:
    void onTriggerNpc(SkillRunner& oRunner, SceneUser* user) const;
  private:
    bool trap(SkillRunner& oRunner) const;
    ETrapNpcFuncType m_eFuncType = ETRAPNPCFUNC_MIN;
};

class SwordBreakSkill : public BaseSkill
{
  public:
    SwordBreakSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~SwordBreakSkill();

    virtual void onBeBreak(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
};

// 表现技能
class ShowSkill : public BaseSkill
{
  public:
    ShowSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~ShowSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 抚摸宠物
class TouchPetSkill : public BaseSkill
{
  public:
    TouchPetSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TouchPetSkill();

    virtual bool prepare(SkillRunner& oRunner) const;
    virtual bool run(SkillRunner& oRunner) const;
    virtual void onBeBreak(SkillRunner& oRunner) const;
    virtual void end(SkillRunner& oRunner) const;
};

// 波利乱斗击落金苹果技能
class PoliAttackSkill : public BaseSkill
{
  public:
    PoliAttackSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~PoliAttackSkill();

  protected:
    virtual void runSpecEffect(SkillRunner& oRunner) const;
};

// 闪现
class BlinkSkill : public BaseSkill
{
  enum EBlinkType
  {
    EBLINKFORWARD = 1,
  };
  public:
    BlinkSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~BlinkSkill();
    virtual bool run(SkillRunner& oRunner) const;
};

// 招怪技能
class TrapMonsterSkill : public BaseSkill
{
  enum ESummonPosType
  {
    ESUMMONPOS_FORWARD = 1,
    ESUMMONPOS_BACK = 2,
  };
  public:
    TrapMonsterSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~TrapMonsterSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 偷钱技能
class StealMoneySkill: public BaseSkill
{
  public:
    StealMoneySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~StealMoneySkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 胁持
class SeizeSkill : public BaseSkill
{
  public:
    SeizeSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~SeizeSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 召唤生命体
class SummonBeingSkill : public BaseSkill
{
public:
  SummonBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~SummonBeingSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;

  DWORD getFirstBeingID() const { return m_setBeingIDs.empty() ? 0 : *(m_setBeingIDs.begin()); }
private:
  set<DWORD> m_setBeingIDs;
};

// 复活生命体
class ReviveBeingSkill : public BaseSkill
{
public:
  ReviveBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~ReviveBeingSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
private:
  DWORD m_dwHpPercent = 0;
};

// 召唤地狱植物
class HellPlantSkill : public BaseSkill
{
public:
  HellPlantSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~HellPlantSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
  virtual void end(SkillRunner& oRunner) const;
private:
  bool trap(SkillRunner& oRunner) const;
  bool trapfire(SkillRunner& oRunner) const;

  DWORD m_dwTrapSkillID = 0;
};

// 给生命体添加buff
class BeingBuffSkill : public BaseSkill
{
public:
  BeingBuffSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~BeingBuffSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
private:
  TSetDWORD m_setSelfBuff;
  map<DWORD, TSetDWORD> m_mapBeingBuff;
};

// 使生命体放技能
class UseBeingSkill : public BaseSkill
{
public:
  UseBeingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~UseBeingSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
private:
  DWORD m_dwBeingSkillID = 0;
};

// 随机释放技能
class RandomSkill : public BaseSkill
{
public:
  RandomSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~RandomSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
private:
  TVecDWORD m_vecSkillIDs;
};

// 控制技能, (擒拿)
class ControlSkill : public BaseSkill
{
  public:
    ControlSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~ControlSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 陷阱拆除
class RemoveTrapSkill : public BaseSkill
{
  public:
    RemoveTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~RemoveTrapSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 抄袭技能
class CopySkill : public BaseSkill
{
  public:
    CopySkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~CopySkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 闪现技能
class SpaceLeapSkill : public BaseSkill
{
  public:
    SpaceLeapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~SpaceLeapSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

// 临危不乱
class FastRestoreSkill : public BaseSkill
{
  public:
    FastRestoreSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~FastRestoreSkill();

    virtual bool run(SkillRunner& oRunner) const;
    virtual bool init(xLuaData& params);
  private:
    TSetDWORD m_setPanaceaHeal;
    DWORD m_dwPanaceaItemID = 0;/*万能药*/
    DWORD m_dwRepairItemID = 0;/*修理石*/
};

// 传送地图
class GoMapSkill : public BaseSkill
{
  public:
    GoMapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~GoMapSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
  private:
    DWORD m_dwMapID = 0;
    ETransportType m_eTransType = ETRANSPORTTYPE_MIN;
};

// 婚姻
class WeddingSkill : public BaseSkill
{
  public:
    WeddingSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~WeddingSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
    virtual bool canUseToTarget(xSceneEntryDynamic* attacker, const SkillBroadcastUserCmd& oCmd) const;
};

// 清除技能效果
class ClearEffectSkill : public BaseSkill
{
  public:
    ClearEffectSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~ClearEffectSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
  private:
    DWORD m_dwClearNum = 0;
    TSetDWORD m_setForceClear;
};

// 咒缚阵
class CursedCircleSkill : public BaseSkill
{
  public:
    CursedCircleSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~CursedCircleSkill();

    virtual bool run(SkillRunner& oRunner) const;
};

//骑乘或者变身技能(骑狼术，魔导机械)
class RideChangeSkill : public BaseSkill
{
  public:
    RideChangeSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    virtual ~RideChangeSkill();

    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
    //魔导机械 自爆
    bool selfDestructor(SkillRunner& oRunner) const;
    void addSelfBuffs(SkillRunner& oRunner) const ;
    void delSelfBuffs(SkillRunner& oRunner) const ;
    virtual void onReset(xSceneEntryDynamic* pEntry) const;
  private:
    bool m_bSelfDestructor = false;
    DWORD m_dwMachineSkill = 0;
};

//复活技能, 用于大主教的圣灵降临,逻辑与GroupBirthSkill类似
class ReviveSkill : public BaseSkill
{
  public:
    ReviveSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    ~ReviveSkill();
    virtual bool run(SkillRunner& oRunner) const;
    virtual bool init(xLuaData& params);
  private:
    TSetDWORD m_setOnceBuff;

};

class SummonElementSkill : public BaseSkill
{
  public:
    SummonElementSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    ~SummonElementSkill();
    virtual bool run(SkillRunner& oRunner) const;
    virtual bool init(xLuaData& params);

  private:
    TSetDWORD m_setElementIDs; //可召唤的元素 npcid
    DWORD m_dwBaseLastSeconds = 0; //元素的基础生命时长 秒
    DWORD m_dwEffectSkillID = 0;  //影响元素生命时长的技能ID
    DWORD m_fEffectRatio = 0;    //影响元素生命时长系数  基础时长 + 技能等级 * 系数
};

class ElementTrapSkill : public BaseSkill
{
  public:
    ElementTrapSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
    ~ElementTrapSkill();
    virtual bool init(xLuaData& params);
    virtual bool run(SkillRunner& oRunner) const;
  private:
    bool m_bClearTrap = false;
};

// 独奏
class SoloSkill : public BaseSkill
{
public:
  SoloSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~SoloSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
  virtual void end(SkillRunner& oRunner) const;

private:
  DWORD m_dwCostSp = 0;
  set<EItemType> m_setWeaponItemType;
};

// 合奏
class EnsembleSkill : public BaseSkill
{
public:
  EnsembleSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~EnsembleSkill();

  virtual bool init(xLuaData& params);
  virtual bool run(SkillRunner& oRunner) const;
  virtual void end(SkillRunner& oRunner) const;

  xSceneEntryDynamic* getPartner(xSceneEntryDynamic* user, DWORD& skillid) const;
  DWORD getCostSp(SceneUser* user) const;

private:
  DWORD m_dwCostSp = 0;
  DWORD m_dwPartnerSkillID = 0;
  set<EItemType> m_setWeaponItemType;
};

// 停止演奏
class StopConcertSkill : public BaseSkill
{
public:
  StopConcertSkill(DWORD id, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp);
  virtual ~StopConcertSkill();

  virtual bool run(SkillRunner& oRunner) const;
};

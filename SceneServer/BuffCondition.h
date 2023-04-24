/**
 * @file BuffCondition.h
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#pragma once

#include "xDefine.h"
#include <map>
#include <set>
#include "Item.h"
#include "ProtoCommon.pb.h"
#include "NpcConfig.h"
#include "SceneUser.pb.h"

using std::set;

class xSceneEntryDynamic;
class BaseSkill;

enum BuffTriggerType
{
  BUFFTRIGGER_NONE      = 0,
  BUFFTRIGGER_HPLESSPER = 1,
  BUFFTRIGGER_ATTACK    = 2,
  BUFFTRIGGER_BEATTACK  = 3,
  BUFFTRIGGER_EQUIP     = 4,
  BUFFTRIGGER_MOUNT     = 5,
  BUFFTRIGGER_PROFES    = 6,
  BUFFTRIGGER_KILLNPC   = 7,
  BUFFTRIGGER_ALLEQUIP  = 8,
  BUFFTRIGGER_ALLCARD   = 9,
  BUFFTRIGGER_EQREFINE  = 10,
  BUFFTRIGGER_ATTR      = 11,
  BUFFTRIGGER_TIME      = 12,
  BUFFTRIGGER_DAMAGE    = 13,
  BUFFTRIGGER_REBORN    = 14,
  BUFFTRIGGER_BESHOOT   = 15,
  BUFFTRIGGER_TEAMNUM   = 16,
  BUFFTRIGGER_HAND      = 17,
  BUFFTRIGGER_USESKILL  = 18,
  BUFFTRIGGER_DISTANCE  = 19,
  BUFFTRIGGER_BUFF      = 20,
  BUFFTRIGGER_MAP       = 21,
  BUFFTRIGGER_NPCPRESENT= 22,
  BUFFTRIGGER_MAPTYPE   = 23,
  BUFFTRIGGER_USESKILLKILL = 24, //使用特定技能杀死目标
  BUFFTRIGGER_SUSPEND   = 26,
  BUFFTRIGGER_GENDER    = 27,
  BUFFTRIGGER_ABNORMAL  = 28,
  BUFFTRIGGER_CONCERT   = 31,
  BUFFTRIGGER_SPLESSPER = 32,
  BUFFTRIGGER_ELEMENTELF= 33,
  BUFFTRIGGER_CHANT     = 34,
};

enum BuffFormula
{
  NO_FORMULA = 0,
  SLV_FORMULA = 1,
  LEVSAR_FORMULA = 2,
  SLVDEX_FORMULA = 3,
  SLVAGI_FORMULA = 4,
  HPRECOVER_FORMULA = 5,
  HPREDUCE_FORMULA = 6,
  SPRECOVER_FORMULA = 7,
  SPECSKILL_FORMULA = 8,
  INSTATUS_FORMULA = 10,
  TURNUNDEAD_FORMULA = 11,
  GETSTATUS_FORMULA = 12,
  INACTION_FORMULA = 13,
  FOURTEEN_FORMULA = 14,
  FIFTEEN_FORMULA = 15,
  SKILLDAMAGE_FORMULA = 16,
  HIDE_FORMULA = 17,
  EIGHTEEN_FORMULA = 18,
  NINETEEN_FROMULA = 19,
};

struct SBufferData;
struct FmData
{
  BuffFormula formula = NO_FORMULA;
  EAttrType eType = EATTRTYPE_MIN;
  std::vector<float> params;
  float value = 0.0f;
  bool load(xLuaData &data)
  {
    value = data.getFloat();
    DWORD type = data.getTableInt("type");
    formula = static_cast <BuffFormula> (type);
    float a = data.getTableFloat("a");
    params.push_back(a);
    float b = data.getTableFloat("b");
    params.push_back(b);
    float c = data.getTableFloat("c");
    params.push_back(c);
    float d = data.getTableFloat("d");
    params.push_back(d);
    return true;
  }
  float getParams(DWORD index)
  {
    if (index >= params.size())
      return 0.0f;
    return params[index];
  }
  bool hasDynamicAttr() const
  {
    // 有计算公式
    if (formula != NO_FORMULA)
      return true;
    // 有攻防属性, 计算属性时, 需要放到同一个buff列表中
    if (eType == EATTRTYPE_ATKATTR || eType == EATTRTYPE_DEFATTR)
      return true;
    return false;
  }
  bool empty() const { return value==0 && formula == NO_FORMULA; }
  float getFmValue(xSceneEntryDynamic* me, const SBufferData* pData);
};
typedef std::vector<FmData> TVecFmData;

struct SBufferData;
class BaseCondition
{
  public:
    BaseCondition();
    virtual ~BaseCondition();
  public:
    BuffTriggerType getTrigType() { return trigType; }
  protected:
    enum EMapType
    {
      EMAPTYPE_PVE = 1,
      EMAPTYPE_PVP = 2,
      EMAPTYPE_GVG = 3,
      EMAPTYPE_TEMAPWS = 4,
    };
    BuffTriggerType trigType = BUFFTRIGGER_NONE;
    bool checkBase(xSceneEntryDynamic* entry);
  public:
    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool canContinue() { return true; }
  private:
    TSetDWORD m_setProfession;
    TSetDWORD m_setEquipType;
    set<EMapType> m_setNoMapTypes;
};

//时间条件
class TimeCondition : public BaseCondition
{
  public:
    TimeCondition()
    {
      trigType = BUFFTRIGGER_TIME;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    DWORD afterTime = 0;
    bool fakeDead = false;
};

//属性条件
class AttrCondition : public BaseCondition
{
  public:
    AttrCondition()
    {
      trigType = BUFFTRIGGER_ATTR;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    TVecAttrSvrs m_uAttr;
};

//装备物品
class ItemCondition : public BaseCondition
{
  public:
    ItemCondition()
    {
      trigType = BUFFTRIGGER_EQUIP;
      itemValue = EITEMTYPE_MIN;
      m_dwItemID = 0;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
    bool delWhenInvalid() { return m_bDelWhenInvalid; }
  private:
    EItemType itemValue;
    DWORD m_dwItemID;
    vector<EProfession> m_vecProfession;
    TSetDWORD m_itemTypes;
    TSetDWORD m_noWeaponTypes;
    TSetDWORD m_setNoItemIDs;
    bool m_bNoWeaponOk = false;
    bool m_bDelWhenInvalid = false; // 失效时删除
};

//转职为某种职业
class ProfesCondition : public BaseCondition
{
  public:
    ProfesCondition()
    {
      trigType = BUFFTRIGGER_PROFES;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    //EProfession profes;
    vector<EProfession> m_vecProfession;
};

//状态条件
class StatusCondition : public BaseCondition
{
  public:
    StatusCondition();
    virtual ~StatusCondition();
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);

  private:
    DWORD m_dwStatus = 1;
};

//骑乘条件
class RideCondition : public BaseCondition
{
  public:
    RideCondition()
    {
      trigType = BUFFTRIGGER_MOUNT;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    TSetDWORD m_setMountID;
};

class AllEquipCondition : public BaseCondition
{
  public:
    AllEquipCondition()
    {
      trigType = BUFFTRIGGER_ALLEQUIP;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    vector<DWORD> m_vecEquipID;
};

class AllCardCondition : public BaseCondition
{
  public:
    AllCardCondition()
    {
      trigType = BUFFTRIGGER_ALLCARD;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
    virtual bool init(xLuaData &data);
  private:
    vector<DWORD> m_vecCardID;
};

class EqRefineCondition : public BaseCondition
{
  public:
    EqRefineCondition()
    {
      trigType = BUFFTRIGGER_EQREFINE;
    }
    virtual bool init(xLuaData &data);
    DWORD getRefineLv() { return refineLv; }
  private:
    DWORD refineLv = 0;
};

// 受到百分比伤害时
class DamageCondition : public BaseCondition
{
  public:
    DamageCondition();
    virtual ~DamageCondition();

    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
  private:
    float m_dwDamagePer = 0.0f;
};

class HpLessPerCondition : public BaseCondition
{
  public:
    HpLessPerCondition();
    virtual ~HpLessPerCondition();

    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
  private:
    float m_fHpLessPer = 0.0f;
    float m_fHpMoreInvalidPer = 0.0f;
};

class SpLessPerCondition : public BaseCondition
{
  public:
    SpLessPerCondition();
    virtual ~SpLessPerCondition();

    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
  private:
    float m_fSpLessPer = 0.0f;
    float m_fSpMoreInvalidPer = 0.0f;
};

class TeamNumCondition : public BaseCondition
{
  public:
    TeamNumCondition()
    {
      trigType = BUFFTRIGGER_TEAMNUM;
    };

    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
  private:
    DWORD m_dwNum = 0;
};

class HandCondition : public BaseCondition
{
  public:
    HandCondition();
    virtual ~HandCondition();
  public:
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
};

// 超出距离不生效
class DistanceCondition : public BaseCondition
{
public:
  DistanceCondition()
  {
    trigType = BUFFTRIGGER_DISTANCE;
  }
  virtual ~DistanceCondition();

  virtual bool init(xLuaData& data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  DWORD m_dwDistance = 0;
  bool m_bTeammate = false;
};

class HasBuffCondition : public BaseCondition
{
  public:
    HasBuffCondition();
    virtual ~HasBuffCondition();
  public:
    virtual bool init(xLuaData &data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
  private:
    TSetDWORD m_setNeedBuffIDs;
    TSetDWORD m_setNotNeedBuffIDs;
};

// npc(生命体...)在场生效,离场失效
class NpcPresentCondition : public BaseCondition
{
public:
  NpcPresentCondition()
  {
    trigType = BUFFTRIGGER_NPCPRESENT;
  }
  virtual ~NpcPresentCondition() {}

  virtual bool init(xLuaData& data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  DWORD m_dwBeingID = 0;
};

class MapTypeCondition : public BaseCondition
{
public:
  MapTypeCondition();
  virtual ~MapTypeCondition();

  virtual bool init(xLuaData &data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  bool hasMapType(EMapType type) { return m_setMapType.find(type) != m_setMapType.end(); }

  set<EMapType> m_setMapType;
};

// 检查是否是魔导悬浮状态
class SuspendCondition : public BaseCondition
{
  public:
    SuspendCondition();
    virtual ~SuspendCondition();
    virtual bool checkCondition(SBufferData& bData, DWORD cur);
};
// 按性别生效
class GenderCondition : public BaseCondition
{
public:
  GenderCondition()
  {
    trigType = BUFFTRIGGER_GENDER;
  }
  virtual ~GenderCondition() {}

  virtual bool init(xLuaData& data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  EGender m_eGender = EGENDER_MIN;
};

// 是否在演奏中
class ConcertCondition : public BaseCondition
{
public:
  ConcertCondition()
  {
    trigType = BUFFTRIGGER_CONCERT;
  }
  virtual ~ConcertCondition() {}

  virtual bool init(xLuaData& data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  DWORD m_dwStyle = 0;
};

/*------------------------------不可持续, call in outside------------------------------*/
// 受到攻击时调用, check always return false
class BeAttackCondition : public BaseCondition
{
  public:
    BeAttackCondition();
    virtual ~BeAttackCondition();

    virtual bool checkCondition(SBufferData& bData, DWORD cur) { return true; }
    virtual bool canContinue() { return false; }
    virtual bool init(xLuaData &data);
    bool isCheckOk(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, const SBufferData* pData);

  private:
    bool m_bNormalSkillOk = false;
    bool m_bPhySkillOk = false;
    bool m_bMagicSkillOk = false;
    bool m_bAnySkillOk = false;
    TVecDWORD m_vecSkillIDs;

    bool m_bJustLockSkill = false;
    bool m_bJustLongSkill = false;
    bool m_bJustNearSkill = false;

    bool m_bNoNormalSkill = false;
    bool m_bNeedMiss = false;
    DWORD m_dwNeedAtkAttr = 0;
    DWORD m_dwLessDistance = 0;
    TSetDWORD m_setNotSkillIDs;

    FmData m_fmHpMore;
    FmData m_fmHpLess;
    TSetDWORD m_setNeedRaces;
};

//Buff条件：杀死怪物
class KillCondition : public BaseCondition
{
  public:
    KillCondition()
    {
      trigType = BUFFTRIGGER_KILLNPC;
    }
    virtual bool checkCondition(SBufferData& bData, DWORD cur) { return true; }
    virtual bool canContinue() { return false; }
    virtual bool init(xLuaData &data);
    bool isTheRace(xSceneEntryDynamic* pMon, const BaseSkill* pSkill);
  private:
    vector<ERaceType> m_vecRaceType;
    bool m_bNormalSkillOk = false;
    bool m_bPhySkillOk = false;
    bool m_bMagicSkillOk = false;
    bool m_bAnySkillOk = false;

    bool m_bJustLockSkill = false;
    bool m_bJustLongSkill = false;
    bool m_bJustNearSkill = false;

};

// 普攻时调用, check always return false
class AttackCondition : public BaseCondition
{
  public:
    AttackCondition();
    virtual ~AttackCondition();

    virtual bool checkCondition(SBufferData& bData, DWORD cur) { return true; }
    virtual bool canContinue() { return false; }
    virtual bool init(xLuaData &data);
    bool isCheckOk(xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy, const BaseSkill* pSkill, DamageType damtype, const SBufferData* pData);

  protected:
    bool checkSkill(const BaseSkill* pSkill);
  protected:
    FmData m_stCriOdds;
    FmData m_stNomOdds;
    bool m_bNormalSkillOk = false;
    bool m_bPhySkillOk = false;
    bool m_bMagicSkillOk = false;
    bool m_bAnySkillOk = false;

    bool m_bJustLockSkill = false;
    bool m_bJustLongSkill = false;
    bool m_bJustNearSkill = false;
    bool m_bJustPointSkill = false;
    bool m_bJustReadySkill = false;

    bool m_bNoNormalSkill = false;
    DWORD m_dwNeedAtkAttr = 0;
    DWORD m_dwNeedDamage = 0;
    FmData m_fmHpMore;
    FmData m_fmHpLess;
    FmData m_fmTarHpThan;
    FmData m_fmTarHpLess;
    TSetDWORD m_setNeedRaces;
    TSetDWORD m_setNeedSkillID;
};

// 使用技能
class UseSkillCondition : public AttackCondition
{
  public:
    UseSkillCondition();
    virtual ~UseSkillCondition();
    bool isCheckOk(const BaseSkill* pSkill, xSceneEntryDynamic* attacker);
};

// 复活时调用, check always return false
class RebornCondition : public BaseCondition
{
  public:
    RebornCondition();
    virtual ~RebornCondition();

    virtual bool checkCondition(SBufferData& bData, DWORD cur) { return true; }
    virtual bool canContinue() { return false; }
};

// 被拍摄时, check always return false
class BeShootCondition : public BaseCondition
{
  public:
    BeShootCondition();
    virtual ~BeShootCondition();

    virtual bool checkCondition(SBufferData& bData, DWORD cur) { return true; }
    virtual bool canContinue() { return false; }
};

// 具体地图生效
class MapBuffCondition : public BaseCondition
{
public:
  MapBuffCondition()
  {
    trigType = BUFFTRIGGER_MAP;
  }
  virtual ~MapBuffCondition();

  virtual bool init(xLuaData& data);
  virtual bool checkCondition(SBufferData& bData, DWORD cur);

private:
  TSetDWORD m_setMapId;
};

// 使用技能杀死目标
class UseSkillKillCondition : public UseSkillCondition
{
  public:
    UseSkillKillCondition();
    virtual ~UseSkillKillCondition();
    bool isCheckOk(const BaseSkill* pSkill, xSceneEntryDynamic* attacker, xSceneEntryDynamic* enemy);
};

//异常状态
class AbnormalStateCondition : public BaseCondition
{
public:
  AbnormalStateCondition();
  virtual ~AbnormalStateCondition();
  virtual bool checkCondition(SBufferData& bData, DWORD cur);
};

// 当前元素被召唤生效
class ElementElfCondition : public BaseCondition
{
  public:
    ElementElfCondition()
    {
      trigType = BUFFTRIGGER_ELEMENTELF;
    }
    virtual ~ElementElfCondition();
    virtual bool init(xLuaData& data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);

  private:
    DWORD m_dwElementElfId = 0;
};

//当前吟唱状态生效
class ChantStatusCondition : public BaseCondition
{
  public:
    ChantStatusCondition()
    {
      trigType = BUFFTRIGGER_CHANT;
    }
    virtual ~ChantStatusCondition();
    virtual bool init(xLuaData& data);
    virtual bool checkCondition(SBufferData& bData, DWORD cur);

  private:
    bool m_bMoveChant = false;
};

#pragma once

#include <set>
#include "xSceneEntry.h"
#include "xPos.h"
#include "Scene.h"
#include "xBitOperator.h"
#include "SkillProcessor.h"
#include "Attribute.h"
#include "xTime.h"
#include "MoveAction.h"
#include "BufferState.h"
#include "CDTime.h"
#include "Follower.h"
//#include "ChangeSkill.h"
#include "GatewayCmd.h"
#include "SpEffect.h"
#include "SkillStatus.h"
#include "GSocial.h"
#include "Action.h"

using namespace Cmd;

class SceneUser;
class SceneNpc;
class Skill;

class xSceneEntryDynamic : public xSceneEntry
{
  public:
    xSceneEntryDynamic(QWORD eid, QWORD etempid);
    virtual ~xSceneEntryDynamic();

  public:
    static xSceneEntryDynamic* getEntryByID(const UINT entryId);
    virtual QWORD getTempID() const { return id; }
    // 场景
  public:
    virtual bool enterScene(Scene* scene) = 0;
    virtual void leaveScene() = 0;
    virtual void sendMeToNine() = 0;
    virtual void sendToNineMe() {}        // SceneUser 实现
    virtual void sendCmdToMe(const void* cmd, DWORD len) {}
    virtual void sendCmdToNine(const void* cmd, DWORD len, GateIndexFilter filter=GateIndexFilter());
    virtual void sendCmdToScope(const void* cmd, DWORD len) = 0;
    virtual bool isAlive() { return getStatus() == ECREATURESTATUS_LIVE; }
    bool isMask() const { return false; }
    bool check2PosInNine(xSceneEntryDynamic* target);
    void setScenePos(xPos p);
    virtual DWORD getMapID() const { return getScene() == nullptr ? 0 : getScene()->getMapID(); }

    // 定时器
  protected:
    virtual void onOneSecTimeUp(QWORD curMSec);
    virtual void onFiveSecTimeUp(QWORD curMSec) {}
    virtual void onTenSecTimeUp(QWORD curMSec) {}
    virtual void onOneMinTimeUp(QWORD curMSec) {}
    virtual void onTenMinTimeUp(QWORD curMSec) {}
    virtual void onDailyRefresh(QWORD curMSec) {}
  public:
    virtual void refreshMe(QWORD curMSec);
  private:
    xTimer m_oOneSecTimer;
    xTimer m_oFiveSecTimer;
    xTimer m_oTenSecTimer;
    xTimer m_oOneMinTimer;
    xTimer m_oTenMinTimer;
    xDayTimer m_oDayTimer;

    // 攻击相关
  public:
    bool isMyEnemy(xSceneEntryDynamic* target);
    bool canAttack(xSceneEntryDynamic* target);
    bool attackMe(QWORD damage, xSceneEntryDynamic* attacker);

  protected:
    virtual bool beAttack(QWORD damage, xSceneEntryDynamic* attacker) = 0;
  public:
    virtual bool isMyTeamMember(QWORD id) { return this->id == id; }
    bool canSeeHideBy(xSceneEntryDynamic* entry);
    //使用技能
  public:
    /**
     * @brief 使用技能,目标可以使场景实体或地上一点
     *
     * @param skillId 技能id
     * @param targetId 目标id
     * @param pos 目标点
     *
     * @return 是否使用成功
     */
    bool useSkill(const UINT skillId, const UINT targetId, const xPos& pos, bool bSpecPos = false);
    virtual bool canUseSkill(const BaseSkill* skill) = 0;
  public:
    //virtual const BaseSkill* getFixedSkill(const UINT skillId) const;

    /*************************************************************//**
     *                      移动 
     ****************************************************************/
  public:
    MoveAction m_oMove;
    SkillProcessor m_oSkillProcessor;
    BufferStateList m_oBuff;
    //ChangeSkill m_oChangeSkill;
    virtual void goTo(xPos pos, bool isGoMap=false, bool noCheckScene = false);

    /*************************************************************//**
     *                      属性 
     ****************************************************************/
  public:
    virtual bool initAttr() = 0;

    void updateAttribute() { m_pAttribute->updateAttribute(); }
    void setCollectMark(ECollectType eType) { m_pAttribute->setCollectMark(eType); }
    void setCollectMarkAllBuff() { m_pAttribute->setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF); m_pAttribute->setCollectMark(ECOLLECTTYPE_STATIC_BUFF); }

    Attribute* getAttribute() { return m_pAttribute; }

    float getBaseAttr(EAttrType eType) const;
    float getBuffAttr(EAttrType eType) const;
    float getOtherAttr(EAttrType eType) const;
    float getAttr(EAttrType eType) const;
    float getTimeBuffAttr(EAttrType eType) const;
    float getPointAttr(EAttrType eType) const;
    bool setAttr(EAttrType eType, float value);
    bool setPointAttr(EAttrType eType, float value);
    bool setShowAttr(EAttrType eType, float value);
    //bool setGuildAttr(EAttrType eType, float value);
    //bool setEquipAttr(EAttrType eType, float value);
    //float getAttr(const string& str);
    //bool setAttr(const string& str, float value);
    bool setGMAttr(EAttrType eType, float value);
    DWORD getEvo();

    void modifyCollect(ECollectType eCollect, EAttrType eType, float value, EAttrOper eOper)
    {
      if (m_pAttribute == nullptr)
        return;
      UserAttrSvr oAttr;
      oAttr.set_type(eType);
      oAttr.set_value(value);
      m_pAttribute->modifyCollect(eCollect, oAttr, eOper);
    }
    bool testAttr(EAttrType eType)
    {
      if (m_pAttribute == nullptr)
        return false;
      return m_pAttribute->test(eType);
    }
  protected:
    Attribute* m_pAttribute = nullptr;
  public:
    virtual float getMoveSpeed() { return getAttr(EATTRTYPE_MOVESPD); }
    virtual SQWORD changeHp(SQWORD hp, xSceneEntryDynamic* entry, bool bshare = false, bool bforce = false) = 0;
    virtual DWORD getLevel() const { return 0; }
    virtual void setAction(DWORD id) = 0;
    virtual DWORD getAction() const = 0;
    // 不处理伤害逻辑, 仅处理技能属性攻击逻辑
    void onBeSkillAttack(const BaseSkill* pSkill, xSceneEntryDynamic* attacker);
    void sendBuffDamage(int damage);
    void sendSpDamage(int damage);

    void doBuffAttack(int damage, xSceneEntryDynamic* attacker);
  public:
    CDTimeM m_oCDTime;
    Follower m_oFollower;

  public:
    virtual void refreshDataAtonce() { m_dwDataTick = 0; }
  protected:
    DWORD m_dwDataTick = 0;
    DWORD m_dwSecTick = 0;
    DWORD m_hpRecoverTick = 0;
  protected:
    //void autoRecoverHp(DWORD cur);
    // 状态
  public:
    ECreatureStatus getStatus() const { return m_eStatus; }
    virtual void setStatus(ECreatureStatus eStatus) { if (m_eStatus == eStatus) return; m_eStatus = eStatus; setDataMark(EUSERDATATYPE_STATUS); refreshDataAtonce(); }
  protected:
    ECreatureStatus m_eStatus = ECREATURESTATUS_LIVE;
    // 测试变量
  public:
    bool m_blGod = false;
    bool m_blKiller = false;
    // 数据
  public:
    virtual void setDataMark(EUserDataType eType);
  protected:
    std::bitset<EUSERDATATYPE_MAX> m_bitset;
  public:
    // lua 调用
    virtual DWORD getBodySize() const { return 2; }
    virtual DWORD getRaceType() = 0;
    virtual DWORD getWeaponType() { return 0; }
    virtual DWORD getSkillLv(DWORD skillGroupID) { return 0; }
    virtual DWORD getNormalSkill() const { return 0 ;}
    virtual EProfession getProfession() {return EPROFESSION_MIN;}
    virtual DWORD getArrowID() { return 0; }

    virtual bool isImmuneSkill(DWORD skillid) { return false; }
    virtual bool bDamageAlways1() { return false; }
    bool isOnPvp() { return getScene() && getScene()->isPVPScene(); }
    bool isInRaid() { return getScene() && getScene()->isDScene(); }
    bool isOnGvg() { return getScene() && getScene()->isGvg(); }

    bool isValidAttrName(const char* name);
    float getStrAttr(const char* name);

    const char* getName() const { return this->name; }
    SceneNpc* getNpcObject();
    SceneUser* getUserObject();
    QWORD getGuid() const { return this->id; }

    int getTempDamageType() { int dType = static_cast<int>(m_eTempDamageType); m_eTempDamageType = DAMAGE_TYPE_INVALID; return dType; }
    void setTempDamageType(int type) { DamageType eType = static_cast<DamageType>(type); m_eTempDamageType = eType; } 
    DWORD getTempAtkAttr() { return m_dwTempAtkAttr; }
    void setTempAtkAttr(DWORD atkattr) { m_dwTempAtkAttr = atkattr; }

    bool isMissStillBuff() { bool f = m_bMissStillBuff; m_bMissStillBuff = false; return f; }
    void setMissStillBuff() { m_bMissStillBuff = true; }

    DWORD getRaidType() const;
    DWORD getSkillStatus() { return m_oSkillProcessor.getRunner().getState(); }

    //virtual DWORD getItemUseCnt(DWORD itemid) { return 0; }
    virtual bool isBuffLayerEnough(EBuffType eType) { return false; }
    virtual bool isReliveByOther() { return false; }
    // 随机数, lua调用
    virtual DWORD getRandomNum() { return randBetween(0, 99); }
    virtual void setRandIndex(DWORD index) {}

    void addBuffDamage(float damage) { m_oBuff.addBuffDamage(damage); }
    BufferStateList& getBuff() { return m_oBuff; }
    float getDisWithEntry(QWORD tarid);
    void doExtraDamage(float damage);
    void delSkillBuff(DWORD id, DWORD layer);
    virtual bool inGuildZone() { return false; }
    bool addBuff(DWORD buffid, QWORD targetid);
    virtual bool inSuperGvg() { return false; }
    bool isNoDamMetalNpc() { return !inGuildZone() || inSuperGvg(); } // 不可攻击华丽金属

  private:
    DamageType m_eTempDamageType = DAMAGE_TYPE_INVALID;
    DWORD m_dwTempAtkAttr = 0; // 记录当前技能 最终攻击属性
    bool m_bMissStillBuff = false; // 记录当前技能, 虽然Miss,但依旧添加debuff

    // 表情
  public:
    void playEmoji(DWORD emojiID);
    void checkEmoji(const char *t);
    void checkLockMeEmoji(const char *t);
    // attreffect 属性效果
  public:
    bool isIgnoreMAtt() const;
    bool isIgnoreAtt() const;
    bool isNoHpRecover() const;
    bool isNoSpRecover() const;
    bool isSkillNoBreak() const;
    bool isIgnoreRaceDam() const;
    bool isIgnoreBodyDam() const;
    bool isIgnoreElementDam() const;
    bool isDNearNormalSkill() const;
    bool isForceHitCri() const;
    bool isAttackCanMount() const;
    bool isPointSkillFar() const;
    bool isCantHeal() const;
    bool isNoDie() const;
    bool isNoAddSp() const; // sp 不能增加
    bool isSkillNoReady() const; // 技能瞬发
    bool isNoActiveMonster() const; // 不会被怪物主动攻击
    bool isDiffZoneNoDam() const; // 公会非本线成员无法对其造成伤害
    bool isNoEnemySkilled() const; //不可被地方选为技能目标

    bool isGod() const;

    bool isNoAttacked() const;

    bool isRideWolf() const; //是否是骑狼状态 18
    bool isNotCure() const ;// 是否 不允许治疗 19
    bool isBeMagicMachine() const; //是否 在魔导机械状态  20
    bool isNotHide()const;// 是否 不允许隐身 21  
    bool isSuspend() const; //是否 在悬浮状态 22 
    bool isImmuneGainBuff() const; //是否 免疫增益buff 24
    bool isImmuneReductionBuff() const; //是否 免疫减益buff 25
    bool isImmuneTrap() const; //是否 免疫地面魔法 26
    // attrfuntion 属性效果
  public:
    bool isHandEnable() const;
    bool isShootGhost() const;
    bool isCameraDizzy() const;
    bool isJustInViceZone() const;
    // 属性效果
  public:
    bool isAttrCanMove() const;
    bool isAttrCanSkill() const;
    bool isAttrCanSkill(const BaseSkill* pSkill) const;
    bool isAttrOnlyNormalSkill() const;
    bool isNoHitBack() const; // 不能被击退
    bool isNoAttMove() const; // 不能主动产生技能位移
    bool isCanMoveChant() const; //是否可以移动吟唱施法

  public:
    // 功能限制
    bool canGoGuild() const; // 能否进入公会领地

    // 特殊特效
  public:
    SpEffect& getSpEffect() { return m_oSpEffect; }
    void effect(DWORD dwId, DWORD dwIndex = 0);
    void stopEffect(DWORD dwIndex = 0);
  protected:
    SpEffect m_oSpEffect;

    // 技能额外目标 
  public:
    void setExtraSkillTarget(QWORD entryid, DWORD familySkill) { m_mapExtraSkillTarget[familySkill] = entryid; }
    QWORD getExtraSkillTarget(DWORD skillid) const;
    void clearExtraSkillTarget(DWORD familySkill);
  private:
    std::map<DWORD, QWORD> m_mapExtraSkillTarget;
  public:
    SkillStatus m_oSkillStatus;
    // lua
  public:
    GSocial& getSocial();
    Action& getUserAction();
  public:
    /**
     * @brief 获取周围的敌人数量
     * @param distance 距离
     */ 
    DWORD getRangeEnemy(float distance);
    // 判断是否激活地图使用苍蝇翅膀所需的传送阵
    bool canUseWingOfFly();
  private:
    std::map<DWORD, pair<QWORD, TVecAttrSvrs>> m_mapBuffID2TransferSrc; // this作为转移来源
    std::map<DWORD, pair<QWORD, TVecAttrSvrs>> m_mapBuffID2TransferTar; // this作为转移目标
  public:
    void setTransferAttr(DWORD buffid, QWORD charid, const TVecAttrSvrs& attr, bool src);
    void delTransferAttr(DWORD buffid, bool src, bool updateattr);
    void delTransferAttr(DWORD buffid, QWORD charid, bool src, bool includeother, bool updateattr);
    void getTransferAttr(TVecAttrSvrs& attrs);
    bool hasTransferAttr(DWORD buffid, bool src);
    bool canTeamUseWingOfFly();
};


#include "xSceneEntryDynamic.h"
#include "SceneNpcManager.h"
#include "SceneUserManager.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "RoleDataConfig.h"
#include "xTime.h"
#include "CommonConfig.h"
#include "UserEvent.pb.h"
#include "GMCommandRuler.h"
#include "DressUpStageMgr.h"

xSceneEntryDynamic::xSceneEntryDynamic(QWORD eid, QWORD etempid):
  m_oOneSecTimer(1)
  , m_oFiveSecTimer(5)
  , m_oTenSecTimer(10)
  , m_oOneMinTimer(60)
  , m_oTenMinTimer(600)
  , m_oDayTimer(0, 5)
  , m_oMove(this)
  , m_oSkillProcessor(this)
  , m_oBuff(this)
  //, m_oChangeSkill(this)
  , m_pAttribute(NULL),
  m_oCDTime(this),
  m_oFollower(this), 
  m_oSpEffect(this),
  m_oSkillStatus(this)
{
  set_id(eid);
  set_tempid(etempid);
  m_blGod = false;
  m_blKiller = false;
}

xSceneEntryDynamic::~xSceneEntryDynamic()
{
  SAFE_DELETE(m_pAttribute);
}

inline bool isNpcTempId(const UINT id)
{
  return (id >> 32) == 0 || (id >> 32) >= ONE_THOUSAND;
}

xSceneEntryDynamic* xSceneEntryDynamic::getEntryByID(const UINT entryId)
{
  if (isNpcTempId(entryId))
  {
    return (xSceneEntryDynamic *)SceneNpcManager::getMe().getNpcByTempID(entryId);
  }
  else
  {
    return (xSceneEntryDynamic *)SceneUserManager::getMe().getUserByID(entryId);
  }
}

void xSceneEntryDynamic::sendCmdToNine(const void* cmd, DWORD len, GateIndexFilter filter)
{
  if (!getScene()) return;

  switch (((xCommand *)cmd)->cmd)
  {
    case SCENE_USER_PROTOCMD:
      {
        switch (((xCommand *)cmd)->param)
        {
          case SKILL_BROADCAST_USER_CMD:
          case RET_MOVE_USER_CMD:
            {
              sendCmdToScope(cmd, len);
              return;
            }
            break;
          default:
            break;
        }
      }
      break;
    case SCENE_USER2_PROTOCMD:
      {
        switch (((xCommand *)cmd)->param)
        {
          case USER2PARAM_EFFECT: //EffectUserCmd
            {
              sendCmdToScope(cmd, len);
              return;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }

  getScene()->sendCmdToNine(getPos(), cmd, len, filter);
}

void xSceneEntryDynamic::onOneSecTimeUp(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  //m_oChangeSkill.timer(curSec);
  m_oSpEffect.timer(curSec);
}

void xSceneEntryDynamic::refreshMe(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  if (SCENE_ENTRY_USER == getEntryType())
  {
    {
      ExecutionTime_Scope;
      m_oBuff.timer(curMSec);
    }
    {
      ExecutionTime_Scope;
      m_oSkillProcessor.timer(curMSec);
    }
  }
  else
  {
    m_oBuff.timer(curMSec);
    m_oSkillProcessor.timer(curMSec);
  }

  if (m_oOneSecTimer.timeUp(curSec))
  {
    onOneSecTimeUp(curMSec);
    if (m_oFiveSecTimer.timeUp(curSec))
    {
      onFiveSecTimeUp(curMSec);
    }
    if (m_oTenSecTimer.timeUp(curSec))
    {
      onTenSecTimeUp(curMSec);
    }
    if (m_oOneMinTimer.timeUp(curSec))
    {
      onOneMinTimeUp(curMSec);
      if (m_oTenMinTimer.timeUp(curSec))
        onTenMinTimeUp(curMSec);
      if (m_oDayTimer.timeUp(curSec))
        onDailyRefresh(curMSec);
    }
  }
}

bool xSceneEntryDynamic::check2PosInNine(xSceneEntryDynamic* target)
{
  return (target && getScene() && target->getScene() && getScene()==target->getScene() && getScene()->check2PosInNine(getPos(), target->getPos()));
}

void xSceneEntryDynamic::goTo(xPos pos, bool isGoMap, bool noCheckScene)
{
  if (pos == getPos()) return;

  if (SCENE_ENTRY_NPC ==getEntryType())
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (this);
    if(npc && npc->getScene() && npc->getScene()->getMapID() == MiscConfig::getMe().getDressStageCFG().m_dwStaticMap)
    {
      if(DressUpStageMgr::getMe().isInStageRange(this, pos, 0, nullptr) == true)
        return;
    }
  }

  Scene *pScene = getScene();
  if (!pScene) return;
  if (!noCheckScene && pScene->canUseGoToPos() == false)
    return;

  if (pos.empty())
  {
    const SceneObject *pObject = pScene->getSceneObject();
    if (pObject)
    {
      const xPos* pBorn = pObject->getBornPoint(1);
      if (pBorn != nullptr)
        pos = *pBorn;
    }
  }

  if (pScene->isValidPos(pos) == false)
  {
    XERR << "[瞬移], 坐标非法, 对象:" << name << id << "地图:" << pScene->name << pScene->id << "坐标:" << pos.x << pos.y << pos.z << XEND;
    return;
  }

  Cmd::GoToUserCmd message;
  message.set_charid(id);
  message.set_isgomap(isGoMap);
  ScenePos *p = message.mutable_pos();
  p->set_x(pos.getX());
  p->set_y(pos.getY());
  p->set_z(pos.getZ());
  PROTOBUF(message, send, len);
  sendCmdToNine(send, len);

  setScenePos(pos);

  if (SCENE_ENTRY_USER==getEntryType())
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(this);
    if (pUser != nullptr)
    {
      if (pUser->m_oHands.has() && pUser->m_oHands.isInWait() == false)
        pUser->m_oHands.changeHandStatus(false, pUser->m_oHands.getOtherID());
      if (pUser->getStatus() == ECREATURESTATUS_FAKEDEAD)
        pUser->setStatus(ECREATURESTATUS_LIVE);
      if (pUser->getAction() != 0)
      {
        pUser->setAction(0);
        UserActionNtf cmd;
        cmd.set_charid(id);
        cmd.set_value(0);
        cmd.set_type(EUSERACTIONTYPE_MOTION);
        PROTOBUF(cmd, send2, len2);
        sendCmdToNine(send2, len2);
      }
      pUser->getWeaponPet().onUserGoTo(pos);
      pUser->getUserPet().onUserGoTo(pos);
      pUser->getUserBeing().onUserGoTo(pos);
      pUser->getUserElementElf().onUserGoTo(pos);
      pUser->getServant().onUserGoTo(pos);
    }
    //((SceneUser *)this)->m_oHands.move(pos);
  }
  m_oFollower.goTo(pos);
}

// 属性
float xSceneEntryDynamic::getAttr(EAttrType eType) const
{
  if (m_pAttribute == nullptr)
    return 0;

  return m_pAttribute->getAttr(eType);
}

float xSceneEntryDynamic::getBaseAttr(EAttrType eType) const
{
  if (m_pAttribute == nullptr)
    return 0.0f;
  return m_pAttribute->getBaseAttr(eType);
}

float xSceneEntryDynamic::getBuffAttr(EAttrType eType) const
{
  if (m_pAttribute == nullptr)
    return 0;

  return m_pAttribute->getBuffAttr(eType);
}

float xSceneEntryDynamic::getTimeBuffAttr(EAttrType eType) const
{
  if (m_pAttribute == nullptr)
    return 0;

  return m_pAttribute->getTimeBuffAttr(eType);
}

float xSceneEntryDynamic::getPointAttr(EAttrType eType) const
{
  if (m_pAttribute == nullptr)
    return 0;
  return m_pAttribute->getPointAttr(eType);
}

float xSceneEntryDynamic::getOtherAttr(EAttrType eType) const
{
  return m_pAttribute == nullptr ? 0.0f : m_pAttribute->getOtherAttr(eType);
}

bool xSceneEntryDynamic::setAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;

  return m_pAttribute->setAttr(eType, value);
}

bool xSceneEntryDynamic::setPointAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;

  return m_pAttribute->setPointAttr(eType, value);
}

bool xSceneEntryDynamic::setShowAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;

  return m_pAttribute->setShowAttr(eType, value);
}

/*bool xSceneEntryDynamic::setGuildAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;
  return m_pAttribute->setGuildAttr(eType, value);
}*/

/*bool xSceneEntryDynamic::setEquipAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;
  return m_pAttribute->setEquipAttr(eType, value);
}*/

/*
float xSceneEntryDynamic::getAttr(const string& str)
{
  if (m_pAttribute == nullptr)
    return 0;

  return m_pAttribute->getAttr(str);
}

bool xSceneEntryDynamic::setAttr(const string& str, float value)
{
  if (m_pAttribute == nullptr)
    return false;

  return m_pAttribute->setAttr(str, value);
}
*/

bool xSceneEntryDynamic::isValidAttrName(const char* name)
{
  return RoleDataConfig::getMe().getIDByName(name) != 0;
}

float xSceneEntryDynamic::getStrAttr(const char* name)
{
  DWORD id = RoleDataConfig::getMe().getIDByName(name);
  const RoleData* pData = RoleDataConfig::getMe().getRoleData(id);
  if (pData == nullptr)
    return 0;
  EAttrType eType = static_cast<EAttrType>(id);
  float value = getAttr(eType);
  if (!pData->bPercent)
    return (int)(value);
  return value;
}

DWORD xSceneEntryDynamic::getRaidType() const
{
  DScene* pDScene = dynamic_cast<DScene*>(getScene());
  return pDScene == nullptr ? 0 : pDScene->getRaidType();
}

bool xSceneEntryDynamic::setGMAttr(EAttrType eType, float value)
{
  if (m_pAttribute == nullptr)
    return false;

  return m_pAttribute->setGMAttr(eType, value);
}

DWORD xSceneEntryDynamic::getEvo()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(this);
  if (pUser != nullptr)
    return pUser->getEvo();
  return 0;
}

void xSceneEntryDynamic::setDataMark(EUserDataType eType)
{
  if (eType == EUSERDATATYPE_MAX)
  {
    m_bitset.set();
    return;
  }

  if (eType <= EUSERDATATYPE_MIN || eType >= EUSERDATATYPE_MAX)
    return;

  m_bitset.set(eType);
}

void xSceneEntryDynamic::setScenePos(xPos p)
{
  if (getScene())
  {
    setPos(p);
    if (getOldPosI()!=getPosI())
    {
      getScene()->changeScreen(this);
    }
    if (getEntryType() == SCENE_ENTRY_USER)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (this);
      if (user)
        user->syncPosAtonce();
    }
  }
  m_oMove.clear();
}
/*void xSceneEntryDynamic::autoRecoverHp(DWORD cur)
{
  if (!this->m_oMove.empty())
    return;
  DWORD gap = cur - m_hpRecoverTick;
  if (gap > getAttr(EATTRTYPE_REHPINTERVAL))
  {
    float maxHp = getAttr(EATTRTYPE_MAXHP);
    float nowHp = getAttr(EATTRTYPE_HP);
    float reValue = maxHp * (1.0f + getAttr(EATTRTYPE_REHPVALUE) / 100);
    reValue = reValue + nowHp > maxHp ? maxHp : reValue + nowHp;
    setAttr(EATTRTYPE_HP, reValue);
    m_hpRecoverTick = cur;
  }
}*/

void xSceneEntryDynamic::playEmoji(DWORD emojiID)
{
  if (!getScene()) return;
  if (!emojiID) return;

  /* 2016-09-12 重熙 去掉
  if (getAttr(EATTRTYPE_HIDE) != 0)
    return;
  */

  UserActionNtf cmd;
  cmd.set_charid(id);
  cmd.set_value(emojiID);
  cmd.set_type(EUSERACTIONTYPE_EXPRESSION);
  PROTOBUF(cmd, send, len);
  this->sendCmdToNine(send, len);
}

void xSceneEntryDynamic::checkEmoji(const char *t)
{
  if (!t) return;

  if (SCENE_ENTRY_NPC==getEntryType())
  {
    SceneNpc *npc = (SceneNpc *)this;
    npc->m_oEmoji.check(t);
  }
}

void xSceneEntryDynamic::checkLockMeEmoji(const char *t)
{
  if (!t) return;

  if (SCENE_ENTRY_USER==getEntryType())
  {
    SceneUser *pUser = (SceneUser *)this;
    pUser->lockMeCheckEmoji(t);
  }
}

void xSceneEntryDynamic::onBeSkillAttack(const BaseSkill* pSkill, xSceneEntryDynamic* attacker)
{
  DWORD eAttrType = pSkill->getAtkAttr();
  if (!eAttrType)
    eAttrType = attacker->getAttr(EATTRTYPE_ATKATTR);
  switch (eAttrType)
  {
    case EELEMENTTYPE_WIND:
      {
        checkEmoji("BeWind");
      }
      break;
    case EELEMENTTYPE_EARTH:
      {
        checkEmoji("BeEarth");
      }
      break;
    case EELEMENTTYPE_WATER:
      {
        checkEmoji("BeWater");
      }
      break;
    case EELEMENTTYPE_FIRE:
      {
        checkEmoji("BeFire");
      }
      break;
    case EELEMENTTYPE_NEUTRAL:
      {
      }
      break;
    case EELEMENTTYPE_HOLY:
      {
        checkEmoji("BeHoly");
      }
      break;
    case EELEMENTTYPE_SHADOW:
      {
        checkEmoji("BeDark");
      }
      break;
    case EELEMENTTYPE_GHOST:
      {
      }
      break;
    case EELEMENTTYPE_UNDEAD:
      {
        checkEmoji("BeUndead");
      }
      break;
    case EELEMENTTYPE_POSION:
      {
      }
      break;
    default:
      break;
  }

  if (m_oSkillProcessor.getRunner().getCFG() && m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT)
  {
    switch(m_oSkillProcessor.getRunner().getCFG()->getSkillType())
    {
      case ESKILLTYPE_BEATBACK:
        if (pSkill->isNearNormalSkill())
          m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_SKILL, attacker->id);
        else
          m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_SKILL, 0);
        break;
      case ESKILLTYPE_SWORDBREAK: // 真剑破百道 可被近距离普攻触发有效打断, 远距离普攻仅打断
        {
          // boss, mini 直接打断技能 不进状态
          SceneNpc* npc = dynamic_cast<SceneNpc*> (attacker);
          if (npc && (npc->getNpcType() == ENPCTYPE_MVP || npc->getNpcType() == ENPCTYPE_MINIBOSS))
          {
            m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_SKILL, attacker->id);
            break;
          }

          bool validbreak = false;
          if (pSkill->isNearNormalSkill())
          {
            validbreak = true;
          }
          else if (pSkill->isNormalSkill(attacker))
          {
            if (getXZDistance(getPos(), attacker->getPos()) <= 2.0)
              validbreak = true;
            else
              m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_SKILL, attacker->id);
          }
          if (validbreak)
          {
            m_oSkillProcessor.setValidBreak();
            m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_SKILL, attacker->id);
          }
        }
        break;
      default:
        break;
    }
  }

  SceneNpc* pNpc = dynamic_cast<SceneNpc*>(this);
  if (pNpc)
  {
    pNpc->m_ai.addAttMeUserSkill(attacker->id, pSkill->getSkillID());
    pNpc->m_sai.checkSig("beattack");
  }
}

void xSceneEntryDynamic::sendBuffDamage(int damage)
{
  if (damage == 0)
    return;
  if (getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser*pUser = dynamic_cast<SceneUser*>(this);
    if (pUser && pUser->isInPollyScene())
      return;
  }

  BuffDamageUserEvent cmd;
  cmd.set_charid(id);
  cmd.set_damage(damage);

  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);
}

void xSceneEntryDynamic::sendSpDamage(int damage)
{
  if (damage == 0)
    return;
  if (getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser*pUser = dynamic_cast<SceneUser*>(this);
    if (pUser && pUser->isInPollyScene())
      return;
  }

  BuffDamageUserEvent cmd;
  cmd.set_charid(id);
  cmd.set_damage(damage);
  cmd.set_etype(DAMAGE_TYPE_NORMALSP);

  PROTOBUF(cmd, send, len);
  sendCmdToNine(send, len);
}

void xSceneEntryDynamic::doExtraDamage(float damage)
{
  changeHp(damage, this);
  sendBuffDamage(damage);
}

SceneNpc* xSceneEntryDynamic::getNpcObject()
{
  if (this->getEntryType() != SCENE_ENTRY_NPC)
    return nullptr;
  return (SceneNpc*)(this);
}

SceneUser* xSceneEntryDynamic::getUserObject()
{
  if (this->getEntryType() != SCENE_ENTRY_USER)
    return nullptr;
  return (SceneUser*)(this);
}

float xSceneEntryDynamic::getDisWithEntry(QWORD tarid)
{
  xSceneEntryDynamic* pTar = getEntryByID(tarid);
  if (pTar == nullptr)
    return 99999; // 返回到lua, 表示无效值
  if (pTar->getScene() != getScene())
    return 99999;
  return getXZDistance(pTar->getPos(), getPos());
}

// 属性效果 attreffect
bool xSceneEntryDynamic::isIgnoreMAtt() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 0);
}

bool xSceneEntryDynamic::isIgnoreAtt() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 1);
}

bool xSceneEntryDynamic::isNoHpRecover() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 2);
}

bool xSceneEntryDynamic::isNoSpRecover() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 3);
}

bool xSceneEntryDynamic::isSkillNoBreak() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 4);
}

bool xSceneEntryDynamic::isIgnoreRaceDam() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 5);
}

bool xSceneEntryDynamic::isIgnoreBodyDam() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 6);
}

bool xSceneEntryDynamic::isIgnoreElementDam() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 7);
}

// 无视近战普攻伤害
bool xSceneEntryDynamic::isDNearNormalSkill() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 8);
}

// 攻击必定命中且暴击
bool xSceneEntryDynamic::isForceHitCri() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 9);
}

// 攻击时不下坐骑
bool xSceneEntryDynamic::isAttackCanMount() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 10);
}

// 区域型技能施法距离无限制
bool xSceneEntryDynamic::isPointSkillFar() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 1);
}

// 使用恢复类道具无效果
bool xSceneEntryDynamic::isCantHeal() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 2);
}

// 不会死亡, 至少保留一血
bool xSceneEntryDynamic::isNoDie() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 3);
}

// sp 无法增加
bool xSceneEntryDynamic::isNoAddSp() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 5);
}

// 技能瞬发
bool xSceneEntryDynamic::isSkillNoReady() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 7);
}

// 不会被怪物主动攻击
bool xSceneEntryDynamic::isNoActiveMonster() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 8);
}

// 公会非本线成员无法对其造成伤害
bool xSceneEntryDynamic::isDiffZoneNoDam() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 13);
}

// 不可被敌方选择为技能目标
bool xSceneEntryDynamic::isNoEnemySkilled() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 15);
}

// 是否是骑狼状态 
bool xSceneEntryDynamic::isRideWolf() const 
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 17);
}

// 是否禁止被治愈  
bool xSceneEntryDynamic::isNotCure() const 
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 18);
}

// 是否是魔导机械状态  
bool xSceneEntryDynamic::isBeMagicMachine() const 
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 19);
}

// 是否 不允许隐身  
bool xSceneEntryDynamic::isNotHide() const 
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 20);
}

// 是否是悬空状态 
bool xSceneEntryDynamic::isSuspend() const 
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 21);
}

//是否 免疫增益buff
bool xSceneEntryDynamic::isImmuneGainBuff() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 23);
}

//是否 免疫减益buff
bool xSceneEntryDynamic::isImmuneReductionBuff() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 24);
}

//是否 免疫地面魔法
bool xSceneEntryDynamic::isImmuneTrap() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT2) & (1 << 25);
}

bool xSceneEntryDynamic::isGod() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTREFFECT) & (1 << 19);
}

// 不可被选择为技能目标
bool xSceneEntryDynamic::isNoAttacked() const
{
  return getAttr(EATTRTYPE_NOATTACKED) != 0;
}

// 属性效果 -- AttrFunction
bool xSceneEntryDynamic::isHandEnable() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTRFUNCTION) & (1 << 0);
}

bool xSceneEntryDynamic::isShootGhost() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTRFUNCTION) & (1 << 1);
}

bool xSceneEntryDynamic::isCameraDizzy() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTRFUNCTION) & (1 << 2);
}

bool xSceneEntryDynamic::isJustInViceZone() const
{
  return (DWORD)getAttr(EATTRTYPE_ATTRFUNCTION) & (1 << 3);
}


// 属性效果
bool xSceneEntryDynamic::isAttrCanMove() const
{
  return !getAttr(EATTRTYPE_NOMOVE) && !getAttr(EATTRTYPE_NOACT) && !getAttr(EATTRTYPE_FREEZE);
}

bool xSceneEntryDynamic::isAttrCanSkill() const
{
  return !getAttr(EATTRTYPE_NOATTACK) && !getAttr(EATTRTYPE_NOACT) && !getAttr(EATTRTYPE_FREEZE);
}

// superuse
bool xSceneEntryDynamic::isAttrCanSkill(const BaseSkill* pSkill) const
{
  if (pSkill == nullptr)
    return false;

  if (pSkill->getSkillType() == ESKILLTYPE_FLASH)
    return true;

  if (getAttr(EATTRTYPE_NOMAGICSKILL) && pSkill->isMagicSkill()) //魔力霜冻状态无法使用魔法攻击
    return false;

  DWORD superuse = pSkill->getSuperUse();
  if (superuse == 0)
    return isAttrCanSkill();

  bool checksuperuse = false;
  if ((getAttr(EATTRTYPE_NOSKILL) && pSkill->getSkillID() != getNormalSkill()) || (getAttr(EATTRTYPE_NOMAGICSKILL) && pSkill->isMagicSkill()))
  {
    checksuperuse = true;
    if ((superuse & 1) == 0)
      return false;
  }
  if (getAttr(EATTRTYPE_NOACT))
  {
    checksuperuse = true;
    if ((superuse & 2) == 0)
      return false;
  }
  if (getAttr(EATTRTYPE_FREEZE))
  {
    checksuperuse = true;
    if ((superuse & 4) == 0)
      return false;
  }
  if (getAttr(EATTRTYPE_FEARRUN))
  {
    checksuperuse = true;
    if ((superuse & 8) == 0)
      return false;
  }
  if (getAttr(EATTRTYPE_NOATTACK))
  {
    checksuperuse = true;
    if ((superuse & 16) == 0)
      return false;
  }

  if (checksuperuse)
  {
    // SuperUseEffect 表示仅有SuperUseEffect包含的异常状态 带来的技能限制(noskill, noact, freeze等), 可以被SuperUse解除
    // 若, 有其他异常则不能使用
    DWORD superuseeff = pSkill->getSuperUseEff();
    DWORD stateeff = getAttr(EATTRTYPE_STATEEFFECT);
    if ((superuseeff & stateeff) != stateeff)
      return false;
  }

  return true;
}

bool xSceneEntryDynamic::isAttrOnlyNormalSkill() const
{
  return getAttr(EATTRTYPE_NOSKILL) != 0;
}

bool xSceneEntryDynamic::isNoHitBack() const
{
  return (DWORD)getAttr(EATTRTYPE_NOEFFECTMOVE) & 1;
}

bool xSceneEntryDynamic::isNoAttMove() const
{
  return (DWORD)getAttr(EATTRTYPE_NOEFFECTMOVE) & 2;
}

bool xSceneEntryDynamic::isCanMoveChant() const
{
  return getAttr(EATTRTYPE_MOVECHANT) != 0;
}

QWORD xSceneEntryDynamic::getExtraSkillTarget(DWORD skillid) const
{
  skillid /= ONE_THOUSAND;
  auto it = m_mapExtraSkillTarget.find(skillid);
  return it != m_mapExtraSkillTarget.end() ? it->second : 0;
}

void xSceneEntryDynamic::clearExtraSkillTarget(DWORD familySkill)
{
  auto it = m_mapExtraSkillTarget.find(familySkill);
  if (it == m_mapExtraSkillTarget.end())
    return;
  m_mapExtraSkillTarget.erase(it);
}

GSocial& xSceneEntryDynamic::getSocial()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(this);
  if (pUser != nullptr)
    return pUser->getSocial();
  static GSocial oSocial;
  return oSocial;
}

Action& xSceneEntryDynamic::getUserAction()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(this);
  if (pUser != nullptr)
    return pUser->getUserAction();
  static Action oAction(nullptr);
  return oAction;
}

void xSceneEntryDynamic::delSkillBuff(DWORD id, DWORD layer)
{
  DWORD orilayer = m_oBuff.getLayerByID(id);
  m_oSkillProcessor.setBuffLayer(id, orilayer); // 删除前, 记录到技能中
  m_oBuff.delLayer(id, layer);
}

// ---- FuncLimit 属性, 功能限制 -----

// 进入公会领地
bool xSceneEntryDynamic::canGoGuild() const
{
  return ((DWORD)getAttr(EATTRTYPE_FUNCLIMIT) & (1 << 0)) == 0;
}

void xSceneEntryDynamic::effect(DWORD dwId, DWORD dwIndex /*= 0*/)
{
  const SEffect* pCfg = TableManager::getMe().getEffectCFG(dwId);
  if (pCfg == nullptr)
  {
    XERR << "[播放特效] 失败，找不到特效id" << id << dwId << XEND;
    return;
  }
  xLuaData data = pCfg->getGMData();
  data.setData("index", dwIndex);
  GMCommandRuler::effect(this, data);
}

void xSceneEntryDynamic::stopEffect(DWORD dwIndex /*= 0*/)
{
  xLuaData data;
  data.setData("opt", "delete");
  data.setData("index", dwIndex);
  GMCommandRuler::effect(this, data);
}

void xSceneEntryDynamic::doBuffAttack(int damage, xSceneEntryDynamic* attacker)
{
  if (attacker == nullptr)
    return;
  if (isDiffZoneNoDam()) // 华丽金属
  {
    if (attacker->isNoDamMetalNpc())
    return;
  }
  if (isGod())
    return;

  attackMe(damage, attacker);
  sendBuffDamage(damage);
}

DWORD xSceneEntryDynamic::getRangeEnemy(float distance)
{
  if (!getScene()) 
    return 0;
  xSceneEntrySet uSet;
  getScene()->getEntryList(getPos(), distance, uSet);
  DWORD count = 0;
  for (auto& u : uSet)
  {
    if (!u)
      continue;
    if (u->id == this->id)
      continue;
    xSceneEntryDynamic *p = (xSceneEntryDynamic *)(u);
    if (isMyEnemy(p))
      ++count;
  }
  return count;
}

bool xSceneEntryDynamic::addBuff(DWORD buffid, QWORD targetid)
{
  if (targetid)
  {
    xSceneEntryDynamic* target = xSceneEntryDynamic::getEntryByID(targetid);
    if (!target)
      return false;
    return target->m_oBuff.add(buffid, this);
  }
  return m_oBuff.add(buffid, this);
}

bool xSceneEntryDynamic::canUseWingOfFly()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(this);
  if (pUser != nullptr)
    return pUser->canUseWingOfFly();
  return true;
}

void xSceneEntryDynamic::setTransferAttr(DWORD buffid, QWORD charid, const TVecAttrSvrs& attr, bool src)
{
  if (src)
  {
    m_mapBuffID2TransferSrc[buffid] = make_pair(charid, attr);
  }
  else
  {
    m_mapBuffID2TransferTar[buffid] = make_pair(charid, attr);
  }
  setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  refreshDataAtonce();
}

void xSceneEntryDynamic::delTransferAttr(DWORD buffid, bool src, bool updateattr)
{
  delTransferAttr(buffid, 0, src, true, updateattr);
}

void xSceneEntryDynamic::delTransferAttr(DWORD buffid, QWORD charid, bool src, bool includeother, bool updateattr)
{
  if (src)
  {
    auto it = m_mapBuffID2TransferSrc.find(buffid);
    if (it == m_mapBuffID2TransferSrc.end() || it->second.first <= 0)
      return;
    if (charid && charid != it->second.first)
      return;
    if (includeother)
    {
      xSceneEntryDynamic* tar = xSceneEntryDynamic::getEntryByID(it->second.first);
      if (tar)
        tar->delTransferAttr(buffid, id, false, false, updateattr);
    }
    it->second.first = 0;
    it->second.second.clear();
  }
  else
  {
    auto it = m_mapBuffID2TransferTar.find(buffid);
    if (it == m_mapBuffID2TransferTar.end() || it->second.first <= 0)
      return;
    if (charid && charid != it->second.first)
      return;
    if (includeother)
    {
      xSceneEntryDynamic* psrc = xSceneEntryDynamic::getEntryByID(it->second.first);
      if (psrc)
        psrc->delTransferAttr(buffid, id, true, false, updateattr);
    }
    it->second.first = 0;
    it->second.second.clear();
  }
  setCollectMark(ECOLLECTTYPE_DYNAMIC_BUFF);
  refreshDataAtonce();
  if (updateattr)
    updateAttribute();
}

void xSceneEntryDynamic::getTransferAttr(TVecAttrSvrs& attrs)
{
  for (auto& v : m_mapBuffID2TransferSrc)
  {
    for (auto& attr : v.second.second)
      attrs.push_back(attr);
  }
  for (auto& v : m_mapBuffID2TransferTar)
  {
    for (auto& attr : v.second.second)
      attrs.push_back(attr);
  }
}

bool xSceneEntryDynamic::hasTransferAttr(DWORD buffid, bool src)
{
  if (src)
  {
    auto it = m_mapBuffID2TransferSrc.find(buffid);
    if (it != m_mapBuffID2TransferSrc.end() && it->second.first)
      return true;
    return false;
  }
  auto it = m_mapBuffID2TransferTar.find(buffid);
  if (it != m_mapBuffID2TransferTar.end() && it->second.first)
    return true;
  return false;
}

bool xSceneEntryDynamic::canSeeHideBy(xSceneEntryDynamic* entry)
{
  if (!entry)
    return false;
  if (isMyTeamMember(entry->id))
    return true;

  if (entry->m_oBuff.haveBuffType(EBUFFTYPE_DHIDE))
    return true;
  return false;
}

bool xSceneEntryDynamic::canTeamUseWingOfFly()
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(this);
  if (pUser != nullptr)
    return pUser->canTeamUseWingOfFly();
  return true;
}

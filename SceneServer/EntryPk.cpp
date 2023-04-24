#include "xSceneEntryDynamic.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "SkillItem.h"
#include "SkillManager.h"
#include "UserEvent.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "MsgManager.h"
#include "UserEvent.pb.h"
#include "DScene.h"

bool xSceneEntryDynamic::isMyEnemy(xSceneEntryDynamic* target)
{
  if (target == nullptr || target == this)
    return false;

  auto bGvgFire = [&]()->bool
  {
    if (getScene() && getScene()->getSceneType() == SCENE_TYPE_GUILD_FIRE)
    {
      GuildFireScene* pGScene = dynamic_cast<GuildFireScene*> (getScene());
      if (pGScene && pGScene->getFireState() == EGUILDFIRE_FIRE)
        return true;
    }
    return false;
  };

  if (getEntryType() == SCENE_ENTRY_USER && target->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(this);
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(target);
    if (pUser == nullptr || pNpc == nullptr)
      return false;
    if (pNpc->isWeaponPet() || pNpc->getNpcType() == ENPCTYPE_SKILLNPC || pNpc->getNpcType() == ENPCTYPE_PETNPC || pNpc->getNpcType() == ENPCTYPE_BEING || pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
      SceneUser* pMaster = pNpc->getMasterUser();
      if (pMaster != nullptr && pMaster->isMyEnemy(pUser))
        return true;
      return false;
    }
    if (pNpc->getNpcType() == ENPCTYPE_FRIEND)
      return false;
    if (pNpc->isMonster() == false)
      return false;
    if (pNpc->define.m_oVar.m_qwQuestOwnerID && pNpc->define.m_oVar.m_qwQuestOwnerID != pUser->id)
      return false;
    if (pNpc->define.m_oVar.m_qwFollowerID)
      return false;
    if (pNpc->define.m_oVar.m_qwGuildID)
    {
      if (bGvgFire() && pUser->getGuild().id() != pNpc->define.m_oVar.m_qwGuildID)
        return true;
      if (getScene() && getScene()->isSuperGvg() && pUser->getGuild().id() != pNpc->define.m_oVar.m_qwGuildID)
        return true;
      return false;
    }

    return true;
  }
  else if (getEntryType() == SCENE_ENTRY_USER && target->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser* pSelf = dynamic_cast<SceneUser*>(this);
    SceneUser* pTarget = dynamic_cast<SceneUser*>(target);
    if (pSelf == nullptr || pTarget == nullptr)
      return false;

    if (!getScene() || !getScene()->getSceneBase())
      return false;

    if (getScene()->base->isTransformMap())
    {
      if (pSelf->getTransform().isMonster() || pTarget->getTransform().isMonster())
        return true;
    }
    if (getScene()->isPVPScene())
    {
      if (pSelf->id == pTarget->id)
        return false;
      if (pSelf->isMyTeamMember(pTarget->id))
        return false;
      return true;
    }
    if (bGvgFire())
    {
      if (isMyTeamMember(target->id))
        return false;
      QWORD guild1 = pSelf->getGuild().id();
      QWORD guild2 = pTarget->getGuild().id();
      if (guild1 == 0 || guild2 == 0)
        return true;
      return guild1 != guild2;
    }
    if (getScene()->isSuperGvg())
    {
      QWORD guild1 = pSelf->getGuild().id();
      QWORD guild2 = pTarget->getGuild().id();
      if (guild1 == 0 || guild2 == 0)
        return false;
      return guild1 != guild2;
    }
    if (getScene()->isPollyScene())
      return true;
    return false;
  }
  else if (getEntryType() == SCENE_ENTRY_NPC && target->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(this);
    SceneUser* pUser = dynamic_cast<SceneUser*>(target);
    if (pUser == nullptr || pNpc == nullptr)
      return false;

    if (pNpc->isWeaponPet() || pNpc->getNpcType() == ENPCTYPE_SKILLNPC || pNpc->getNpcType() == ENPCTYPE_PETNPC || pNpc->getNpcType() == ENPCTYPE_BEING || pNpc->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
      SceneUser* pMaster = pNpc->getMasterUser();
      if (pMaster != nullptr && pMaster->isMyEnemy(pUser))
        return true;
      return false;
    }
    if (pNpc->getNpcType() == ENPCTYPE_FRIEND)
      return false;
    if (pNpc->isWeaponPet())
      return false;
    if (pNpc->isMonster() == false)
      return false;
    if (pNpc->define.m_oVar.m_qwQuestOwnerID && pNpc->define.m_oVar.m_qwQuestOwnerID != pUser->id)
      return false;
    if (pNpc->define.m_oVar.m_qwFollowerID)
      return false;
    if (pUser->m_oCarrier.has())
      return false;
    if (pNpc->define.m_oVar.m_qwGuildID)
    {
      if (bGvgFire() && pUser->getGuild().id() != pNpc->define.m_oVar.m_qwGuildID)
        return true;
      if (getScene() && getScene()->isSuperGvg() && pUser->getGuild().id() != pNpc->define.m_oVar.m_qwGuildID)
        return true;
      return false;
    }

    return true;
  }
  else if (getEntryType() == SCENE_ENTRY_NPC && target->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneNpc* pSelf = dynamic_cast<SceneNpc*>(this);
    SceneNpc* pTarget = dynamic_cast<SceneNpc*>(target);
    if (pSelf == nullptr || pTarget == nullptr)
      return false;

    if (pTarget->isMonster() == false)
      return false;

    if (pSelf->isWeaponPet() || pSelf->getNpcType() == ENPCTYPE_SKILLNPC || pSelf->getNpcType() == ENPCTYPE_PETNPC || pSelf->getNpcType() == ENPCTYPE_BEING || pSelf->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
      SceneUser* pMaster = pSelf->getMasterUser();
      if (pMaster != nullptr && pMaster->isMyEnemy(pTarget))
        return true;
      return false;
    }
    if (pTarget->isWeaponPet() || pTarget->getNpcType() == ENPCTYPE_SKILLNPC || pTarget->getNpcType() == ENPCTYPE_PETNPC || pTarget->getNpcType() == ENPCTYPE_BEING || pSelf->getNpcType() == ENPCTYPE_ELEMENTELF)
    {
      SceneUser* pMaster = pTarget->getMasterUser();
      if (pMaster != nullptr && pMaster->isMyEnemy(pSelf))
        return true;
      return false;
    }

    if ((pSelf->getNpcType() == ENPCTYPE_FRIEND) ^ (pTarget->getNpcType() == ENPCTYPE_FRIEND))
      return true;

    if (pSelf->isMonster() == false)
      return true;

    return false;
  }

  return false;
}

bool xSceneEntryDynamic::canAttack(xSceneEntryDynamic* target)
{
  if (target == nullptr)
    return false;
  if (target->isAlive() == false)
    return false;
  if (target->m_blGod || target->isGod())
    return false;
  if (isMyEnemy(target) == false)
    return false;
  if (target->getAttr(EATTRTYPE_NOATTACKED))
    return false;
  if (target->isNoEnemySkilled())
    return false;

  if (getEntryType() == SCENE_ENTRY_USER && target->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneNpc* pTarget = dynamic_cast<SceneNpc*>(target);
    if (pTarget == nullptr || pTarget->define.isGear())
      return false;
  }
  /*else if (getEntryType() == SCENE_ENTRY_USER && target->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(target);
    if (pUser == nullptr)
      return false;
    if (!pUser->m_oHands.canBeAttack())
      return false;
  }
  */
  else if (getEntryType() == SCENE_ENTRY_NPC && target->getEntryType() == SCENE_ENTRY_USER)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(this);
    SceneUser* pUser = dynamic_cast<SceneUser*>(target);
    if (pUser == nullptr || pNpc == nullptr)
      return false;

    if (pUser->getAttr(EATTRTYPE_NOATTACKED) == 1)
      return false;
    //if (!pUser->m_oHands.canBeAttack())
      //return false;
  }
  else if (getEntryType() == SCENE_ENTRY_NPC && target->getEntryType() == SCENE_ENTRY_NPC)
  {
    SceneNpc* pSelf = dynamic_cast<SceneNpc*>(this);
    SceneNpc* pTarget = dynamic_cast<SceneNpc*>(target);
    if (pSelf == nullptr || pTarget == nullptr)
      return false;
    if (/*pSelf->getNpcID() < 10000 || */pTarget->getNpcID() < 10000)
      return false;
    if (pTarget->getDefine().isGear())
      return false;
  }

  return true;
}

bool xSceneEntryDynamic::attackMe(QWORD damage, xSceneEntryDynamic* attacker)
{
  if (!attacker || isAlive() == false)
    return false;

  QWORD realDamage = (QWORD)getAttr(EATTRTYPE_HP) > damage ? damage : (QWORD)getAttr(EATTRTYPE_HP);

  // put before changehp, (changehp may trigger using skill)
  m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_HP, attacker->id, static_cast<DWORD>(damage));

  changeHp(0 - (SQWORD)realDamage, attacker);
  return beAttack(realDamage, attacker);
}

bool xSceneEntryDynamic::useSkill(const UINT skillId, const UINT targetId, const xPos& pos, bool bSpecPos /*=false*/)
{
  //TODO 一般条件检查,如状态冲突等
  if (!m_oCDTime.skilldone(skillId)) 
    return false;
  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(skillId);
  if (pSkillCFG == nullptr)
    return false;
  if (m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT)
    return false;
  if (targetId == this->id && pSkillCFG->getSkillCamp() == ESKILLCAMP_ENEMY)
    return false;

  Cmd::SkillBroadcastUserCmd cmd;
  cmd.set_skillid(skillId);
  Cmd::PhaseData *pData = cmd.mutable_data();
  Cmd::ScenePos *p = pData->mutable_pos();
  p->set_x(pos.getX());
  p->set_y(pos.getY());
  p->set_z(pos.getZ());
  if (targetId)
  {
    Cmd::HitedTarget *pT = pData->add_hitedtargets();
    pT->set_charid(targetId);
  }

  if (this->getEntryType() == SCENE_ENTRY_NPC)
  {
    DWORD dir = 0;
    if (getDistance(getPos(), pos) > 0.5)
    {
      dir = calcAngle(getPos(), pos) * ONE_THOUSAND;
    }
    else
    {
      dir = randBetween(1, 360) * ONE_THOUSAND;
    }
    pData->set_dir(dir);

    if (!bSpecPos)
    {
      switch(pSkillCFG->getLogicType())
      {
        case ESKILLLOGIC_MIN:
        case ESKILLLOGIC_FORWARDRECT:
        case ESKILLLOGIC_MISSILE:
        case ESKILLLOGIC_POINTRANGE:
        case ESKILLLOGIC_POINTRECT:
        case ESKILLLOGIC_RANDOMRANGE:
          break;
        case ESKILLLOGIC_SELFRANGE:
          p->set_x(getPos().getX());
          p->set_y(getPos().getY());
          p->set_z(getPos().getZ());
          break;
        default:
          break;
      }
    }
  }
  else
  {
    if (pSkillCFG->getPetID() != 0)
    {
      SceneUser* user = dynamic_cast<SceneUser*> (this);
      if (user == nullptr)
        return false;
      if (user->getPet().getPartnerID() == 0)
        return false;
    }
    //m_oSkillProcessor.clearActionTime();
    m_oSkillProcessor.addServerTrigSkill(skillId);
  }
  if (!m_oSkillProcessor.setActiveSkill(cmd))
    return false;

  return true;
}

//const BaseSkill* xSceneEntryDynamic::getFixedSkill(const UINT skillId) const
//{
//  return SkillManager::getMe().getSkillCFG(skillId);
//}

bool SceneNpc::canUseSkill(const BaseSkill* skill)
{
  if (skill->getSkillType() == ESKILLTYPE_PASSIVE)
    return false;
  if (isAttrCanSkill(skill) == false)
    return false;
  /*if (isAttrOnlyNormalSkill())
    return getNormalSkill() == skill->getSkillID();*/

  return skill != nullptr ? skill->checkCanUse(this) : false;
}

//const BaseSkill* SceneUser::getFixedSkill(const UINT skillId) const
//{
  //TODO 处理技能转换等
  //return xSceneEntryDynamic::getFixedSkill(skillId);
//}

bool SceneUser::canUseSkill(const BaseSkill* skill)
{
  if (skill == nullptr)
    return false;
  if (m_pCurFighter == nullptr)
    return false;

  //if (m_oHands.isFollower()) return false;

  if (isAlive() == false)
  {
    XERR << "[技能], 玩家已经死亡, 不可释放技能, 玩家:" << name << id << "技能:" << skill->getSkillID() << XEND;
    return false;
  }

  // 卡片 buff , 等触发技能
  if (m_pCurFighter->getSkill().isFreeSkill(skill->getSkillID()))
    return true;

  if (isAttrCanSkill(skill) == false)
    return false;

  if (m_oBuff.haveLimitSkill())
  {
    if(m_oBuff.isSkillNotLimited(skill->getSkillID() / ONE_THOUSAND) == false)
      return false;
  }

  if (skill->getPetID() != 0)
    return m_oPet.getPartnerID() != 0 ;//== skill->getPetID();

  bool noCheckCost = false;
  if (m_oTransform.isMonster()) noCheckCost = true;
  if (skill->isCostBeforeChant() && m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT) noCheckCost = true;//引导技能等, 仅在吟唱开始前检查消耗

  if (!noCheckCost && skill->checkSkillCost(this) == false)
  {
    XERR << "[技能], 释放所需消耗不足, 不可释放此技能, 玩家:" << this->name << this->id << "技能:" << skill->getSkillID() << XEND;
    return false;
  }

  // 前置条件仅在吟唱开始前检查
  if (m_oSkillProcessor.getRunner().getState() != ESKILLSTATE_CHANT)
  {
    if (skill->checkCanUse(this) == false)
    {
      XERR << "[技能], 技能前置条件不足, 不可释放此技能, 玩家:" << this->name << this->id << "技能:" << skill->getSkillID() << XEND;
      return false;
    }
  }

  if (getScene() && getScene()->isForbidSkill(skill->getSkillID()))
  {
    XERR << "[技能], 当前地图禁用该技能, 玩家:" << name << id << "技能:" << skill->getSkillID() << XEND;
    return false;
  }

  static const std::set<ESkillType> noCheckTypes = { ESKILLTYPE_FLASH, ESKILLTYPE_SHOWSKILL, ESKILLTYPE_TOUCHPETSKILL };
  if (getTransform().isMonster())
  {
    if (getTransform().checkSkill(skill->getSkillID()) == false)
      return false;
  }
  else if (noCheckTypes.find(skill->getSkillType()) != noCheckTypes.end())
  {
    return true;
  }
  else if (getScene() && getScene()->isFreeSkill(skill->getSkillID()))
  {
    return true;
  }
  else if (getNormalSkill() != skill->getSkillID())
  {
    bool bable = m_pCurFighter->getSkill().isSkillEnable(skill->getSkillID()) || skill->getSkillType() == ESKILLTYPE_ENSEMBLE;
    bool bequip = (m_pCurFighter->getSkill().isSkillEquiped(skill->getSkillID()) || skill->getSkillType() == ESKILLTYPE_REBIRTH); //复活术 spec.
    if (!bequip)
    {
      TSetDWORD higerskill;
      MiscConfig::getMe().getAutoSkillGrp().getHigherSkill(skill->getSkillID(), higerskill);
      for (auto &s : higerskill)
      {
        if (m_pCurFighter->getSkill().isSkillFamilyEquiped(s))
        {
          bequip = true;
          break;
        }
      }
    }
    //if (m_pCurFighter->getSkill().isSkillEnable(skill->getSkillID()) == false || m_pCurFighter->getSkill().isSkillEquiped(skill->getSkillID()) == false)
    if (!bable || !bequip)
    {
      if (ItemConfig::getMe().isItemSkill(skill->getSkillID()) == false)
      {
        XERR << "[技能], 释放非法技能, 不可释放, 玩家:" << this->name << this->id << "技能:" << skill->getSkillID() << XEND;
        return false;
      }
    }
  }
  return true;
}


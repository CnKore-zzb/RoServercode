/**
 * @file BufferEffect.cpp
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#include "BufferState.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SkillItem.h"
#include "SceneItemManager.h"
#include "GMCommandRuler.h"
#include "ItemConfig.h"
#include "SkillConfig.h"
#include "MsgManager.h"
#include "SceneUserManager.h"
#include "SkillManager.h"
#include "StatisticsDefine.h"
#include "SceneMap.pb.h"
#include "FighterSkill.h"
#include "GuildCityManager.h"
#include "MailManager.h"
#include "SceneNpcManager.h"
#include "RecipeConfig.h"
#include "ActivityManager.h"

bool BufferState::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (m_FmEffectOdds.empty() == false)
  {
    xSceneEntryDynamic* target = me;
    if (uSet.empty() == false)
      target = *(uSet.begin());
    DWORD odds = getEffectOdds(me, target, bData.fromID, bData.lv);
    if ((DWORD)randBetween(1, 100) > odds)
      return false;
  }

  if (m_maxCnt)
    bData.count ++;
  if (m_dwInterval)
    bData.timeTick = xTime::getCurMSec() + m_dwInterval;
  if (m_bIsForceStatus)
    bData.setStatusNoDisp();
  return true;
}

//AttrChange
BuffAttrChange::BuffAttrChange()
{
  m_eBuffType = EBUFFTYPE_ATTRCHANGE;
}

BuffAttrChange::~BuffAttrChange()
{

}

// hp sp change
BuffHSPChange::BuffHSPChange()
{
  m_eBuffType = EBUFFTYPE_HSPCHANGE;
}

BuffHSPChange::~BuffHSPChange()
{

}

bool BuffHSPChange::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_bCalcHeal = data.getMutableData("BuffEffect").getTableInt("calc_heal") != 0;
  return true;
}

bool BuffHSPChange::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  for (auto &pEntry : uSet)
  {
    if (pEntry->isAlive() == false)
      continue;
    const TVecAttrSvrs& uAttrs = bData.getAttr();
    for (auto v = uAttrs.begin(); v != uAttrs.end(); ++v)
    {
      if (v->type() == EATTRTYPE_HP)
      {
        float value = v->value();
        if (v->value() > 0)
        {
          // 华丽金属
          if (pEntry->isDiffZoneNoDam() && me->isNoDamMetalNpc())
            continue;

          if (m_bCalcHeal && pEntry->getEntryType() == SCENE_ENTRY_USER)
          {
            DWORD hp = pEntry->getAttr(EATTRTYPE_HP);
            DWORD maxhp = pEntry->getAttr(EATTRTYPE_MAXHP);
            DWORD delta = maxhp > hp ? maxhp - hp : 0;
            DWORD dvalue = (DWORD)value;
            dvalue = dvalue > delta ? delta : dvalue;
            ((SceneUser*)pEntry)->onSkillHealMe(me, dvalue);
          }

          pEntry->changeHp(value, me);
          pEntry->sendBuffDamage(0 - value);
        }
        else if (v->value() < 0)
        {
          xSceneEntryDynamic* pFromE = xSceneEntryDynamic::getEntryByID(bData.fromID);
          if (pFromE == nullptr || (pEntry != pFromE && pEntry->isNoAttacked()))
            continue;
          pEntry->doBuffAttack(0 - value, pFromE);
        }
      }
      if (v->type() == EATTRTYPE_SP)
      {
        SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
        if (pUser == nullptr)
          continue;
        float value = v->value();
        //float dif = pUser->getAttr(EATTRTYPE_MAXSP) - pUser->getAttr(EATTRTYPE_SP);
        //value = value > dif ? dif : value;
        value = pUser->getAttr(EATTRTYPE_SP) + value;
        value = value < 0 ? 0 : value;
        value = value > pUser->getAttr(EATTRTYPE_MAXSP) ? pUser->getAttr(EATTRTYPE_MAXSP) : value;
        if (value != pUser->getAttr(EATTRTYPE_SP))
          pUser->setSp(value);
      }
    }
  }
  return true;
}

//隐匿
BuffHiding::BuffHiding()
{
  m_eBuffType = EBUFFTYPE_HIDE;
  m_maxCnt = 1;
}

BuffHiding::~BuffHiding()
{

}

bool BuffHiding::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_dwEndBuff = effdata.getTableInt("endbuff");
  if (effdata.has("move_del_time"))
    m_dwMoveDelTimeMs = effdata.getTableFloat("move_del_time") * ONE_THOUSAND;
  return true;
}

bool BuffHiding::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (!me || !me->getScene() || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  switch (me->getEntryType())
  {
    case SCENE_ENTRY_NPC:
      {
        SceneNpc* pNpc = (SceneNpc *) (me);
        if (pNpc->needCheckHideUser())
        {
          if (me->getScene()->isHideUser())
          {
            SceneUser* pUser = pNpc->getMasterUser();
            if (pUser)
            {
              xSceneEntrySet userset;
              me->getScene()->getEntryListInNine(SCENE_ENTRY_USER, me->getPos(), userset);
              for (auto &s : userset)
              {
                if (pUser->canSeeHideBy((SceneUser*)s) == false)
                  pNpc->delMeToUser((SceneUser*)s);
              }
            }
          }
        }
        if (pNpc->define.m_oVar.m_qwOwnerID)
          return true;
        xPos myPos = me->getPos();
        float r = 10;
        xPos newPos;
        me->getScene()->getRandPos(myPos, r, newPos);
        pNpc->m_ai.moveTo(newPos);
        return true;
      }
      break;
    case SCENE_ENTRY_USER:
      {
        SceneUser* pUser = (SceneUser *)(me);
        pUser->lockMeCheckEmoji("EnemyHide");
        if (me->getScene()->isHideUser())
        {
          me->getScene()->hideMeToScope(pUser);
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

void BuffHiding::ondel(xSceneEntryDynamic *me)
{
  if (!me) return;
  if (m_dwEndBuff != 0)
  {
    me->m_oBuff.add(m_dwEndBuff);
  }
  if (me->getEntryType() == SCENE_ENTRY_NPC)
  {
    ((SceneNpc*)me)->m_sai.checkSig("disp_hide");
  }

  if (me->getScene() && me->getScene()->isHideUser())
  {
    if (me->getEntryType() == SCENE_ENTRY_USER)
    {
      me->getScene()->sendUserToScope((SceneUser *)me);
    }
    else
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
      if (npc && npc->needCheckHideUser())
      {
        xSceneEntrySet userset;
        me->getScene()->getEntryListInNine(SCENE_ENTRY_USER, me->getPos(), userset);
        for (auto &s : userset)
        {
          if (npc->isVisableToSceneUser((SceneUser*)s))
            npc->sendMeToUser((SceneUser*)s);
        }
      }
    }
  }
}

void BuffHiding::onClientMove(SBufferData& bData)
{
  if (!m_dwMoveDelTimeMs)
    return;
  if (bData.dwCommonData & 1)
    return;
  bData.dwCommonData = bData.dwCommonData | 1;

  QWORD curm = xTime::getCurMSec();
  if (bData.endTime > curm + m_dwMoveDelTimeMs)
    bData.endTime = curm + m_dwMoveDelTimeMs;
}

//反击 Buff施加者
BuffBeatBack::BuffBeatBack()
{
  m_eBuffType = EBUFFTYPE_TAUNT;
  m_maxCnt = 1;
}

BuffBeatBack::~BuffBeatBack()
{

}

bool BuffBeatBack::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  if (me->m_oBuff.haveBuffType(EBUFFTYPE_IMMUNETAUNT))
    return false;

  xSceneEntryDynamic *pFrom = xSceneEntryDynamic::getEntryByID(bData.fromID);
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (me);
  if (pNpc && pFrom)
  {
    pNpc->m_ai.setTauntUser(pFrom->id);
    pNpc->m_ai.changeState(ENPCSTATE_ATTACK);
  }
  me->m_oSkillProcessor.breakSkill(EBREAKSKILLTYPE_BUFF, bData.fromID);
  return true;
}

void BuffBeatBack::ondel(xSceneEntryDynamic *me)
{
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (!npc)
    return;
  npc->m_ai.setTauntUser(0);
}

//自动回血
BuffRecoverHp::BuffRecoverHp()
{
  m_eBuffType = EBUFFTYPE_RECOVERHP;
}

BuffRecoverHp::~BuffRecoverHp()
{

}

bool BuffRecoverHp::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_type = data.getData("BuffEffect").getTableInt("value");
  m_bCanBeDisable = data.getData("BuffEffect").getTableInt("disable") != 0;
  if (!data.getData("BuffEffect").has("effect"))
    return true;
  const std::string& effect = data.getData("BuffEffect").getTableString("effect");
  m_effParams.setData("type", "effect");
  m_effParams.setData("effect", effect);
  return true;
}

bool BuffRecoverHp::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  // hp 无法自然恢复
  if (m_bCanBeDisable && me->isNoHpRecover())
    return true;

  xPos nowPos = me->getPos();
  if (m_type == 1 && bData.oldPos != nowPos)
  {
    bData.oldPos = nowPos;
    return false;
  }

  //+ hp
  bData.oldPos = nowPos;
  const TVecAttrSvrs& uAttrs = bData.getAttr();
  for (auto m = uAttrs.begin(); m != uAttrs.end(); ++m)
  {
    if (m->type() == EATTRTYPE_HP)
    {
      float value = m->value();
      float dif = me->getAttr(EATTRTYPE_MAXHP) - me->getAttr(EATTRTYPE_HP);
      value = value > dif ? dif : value;
      if (value >= 1)
      {
        me->changeHp(value, me);
        if (m_effParams.has("effect"))
          GMCommandRuler::getMe().execute(me, m_effParams);
      }
      return true;
    }
  }
  return false;
}

//UseSkill   buff添加者 对 被添加者 UseSkill
BuffUseSkill::BuffUseSkill()
{
  m_eBuffType = EBUFFTYPE_USESKILL;
}

BuffUseSkill::~BuffUseSkill()
{

}

bool BuffUseSkill::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_skillID = data.getData("BuffEffect").getTableInt("id");
  m_odds = data.getData("BuffEffect").getTableInt("Odds");
  DWORD actNum = data.getData("BuffEffect").getTableInt("IsActive");
  isActive = actNum > 0 ? true : false;
  DWORD useSelect = data.getData("BuffEffect").getTableInt("UseSelect");
  m_bUseSelect = useSelect > 0 ? true : false;
  m_dwSkillOpt = data.getData("BuffEffect").getTableInt("SkillOpt");
  DWORD useCond = data.getData("BuffEffect").getTableInt("UseCondSkill");
  m_bUseCondSkill = useCond > 0 ? true : false;
  DWORD useReplaced = data.getData("BuffEffect").getTableInt("UseReplaced");
  m_bUseReplaced = useReplaced > 0 ? true : false;

  if (SkillConfig::getMe().getSkillCFG(m_skillID) == nullptr)
  {
    XERR << "[BufferEffect-BuffUseSkill] skillid :" << m_skillID << "未在 Table_Skill.txt 表中找到" << XEND;
    return false;
  }

  return true;
}

bool BuffUseSkill::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (!me || !me->getScene() || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  if ((DWORD)randBetween(1, 100) > m_odds)
    return false;
  DWORD useSkillID = m_skillID;
  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if (pUser)
  {
    if (m_bUseSelect)
    {
      if (m_dwSkillOpt == 0)
        return false;
      useSkillID = pUser->getUserSceneData().getSkillOptValue((ESkillOption)m_dwSkillOpt);
      
      DWORD lev = pUser->getSkillLv(useSkillID);
      DWORD nowID = useSkillID * ONE_THOUSAND + lev;
      useSkillID = nowID;
    }
    else if (m_bUseCondSkill)
    {
      useSkillID = bData.condSkillID;
      if (useSkillID == 0)
      {
        return false;
      }
    }
    else if (m_bUseReplaced)
    {
      useSkillID = pUser->getFighter()->getSkill().getReplaceSkill(m_skillID/ONE_THOUSAND);
    }
    else
    {
      DWORD lev = pUser->getSkillLv(m_skillID / ONE_THOUSAND);
      DWORD nowID = m_skillID / ONE_THOUSAND * ONE_THOUSAND + lev;
      useSkillID = nowID > m_skillID ? nowID : m_skillID;
    }
  }

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(useSkillID);
  if (pSkillCFG == nullptr)
    return false;
  xSceneEntryDynamic* pAttacker = xSceneEntryDynamic::getEntryByID(bData.fromID);
  pAttacker = pAttacker != nullptr ? pAttacker : me;
  if (pAttacker->isAttrCanSkill() == false)
    return false;

  float launchrange = pSkillCFG->getLaunchRange(pAttacker);
  if (m_bUseReplaced)
  {
    if (pSkillCFG->getSkillType() == ESKILLTYPE_PASSIVE)
      return false;

    if (pSkillCFG->getSkillCamp() == ESKILLCAMP_FRIEND)
    {
      getBuffTargetsByTargetType(bData, me, EBUFFTARGET_SELF);
    }
    else if (pSkillCFG->getSkillCamp() == ESKILLCAMP_ENEMY)
    {
      if (uSet.empty())
        return false;
      getBuffTargetsByTargetType(bData, me, EBUFFTARGET_ENEMY, *uSet.begin());
    }
    else if (pSkillCFG->getSkillCamp() == ESKILLCAMP_TEAM)
    {
      getBuffTargetsByTargetType(bData, me, EBUFFTARGET_TEAM, nullptr, launchrange);
    }
    else
      return false;
  }

  const TSetSceneEntrys& tSet = m_bUseReplaced ? m_setTarget : uSet;
  for (auto &pEntry : tSet)
  {
    if (pEntry == nullptr || pEntry->getScene() == nullptr)
      continue;
    // check launching distance
    if (isActive && launchrange != 0 && getDistance(pAttacker->getPos(), pEntry->getPos()) > launchrange * 1.5)
      continue;

    bData.setCanNotTrig(true);
    pAttacker->m_oSkillProcessor.useBuffSkill(pAttacker, pEntry, useSkillID, isActive, true, true);
    bData.setCanNotTrig(false);
  }

  return true;
}

//transform
BuffTransform::BuffTransform()
{
  m_eBuffType = EBUFFTYPE_TRANSFORM;
  m_maxCnt = 1;
}

BuffTransform::~BuffTransform()
{

}

bool BuffTransform::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& buffData = data.getMutableData("BuffEffect");
  m_dwFigureID = data.getData("BuffEffect").getTableInt("id");
  m_dwTransformID = data.getData("BuffEffect").getTableInt("TransformID");
  m_bMoveDel = data.getData("BuffEffect").getTableInt("moveDel") == 1;

  xLuaData& scale = buffData.getMutableData("scale");
  auto scaleF = [this](const string& key, xLuaData& data)
  {
    pair<DWORD, DWORD> s;
    s.first = data.getTableInt("rate");
    s.second = data.getTableInt("shape");
    m_vecScale.push_back(s);
  };
  scale.foreach(scaleF);

  if (buffData.has("transform_type"))
  {
    string stype = buffData.getTableString("transform_type");
    if (stype == "Poli")
      m_eType = ETRANSFROM_POLIFIRE;
    else if (stype == "Altman")
      m_eType = ETRANSFORM_ALTMAN;
  }
  return true;
}

bool BuffTransform::doTransform(SBufferData& bData)
{
  SceneUser* pUser = dynamic_cast<SceneUser*> (bData.me);
  if (!pUser || !pUser->getScene())
    return false;

  const SNpcCFG* pBase = NpcConfig::getMe().getNpcCFG(m_dwFigureID);
  if (pBase == nullptr)
    return false;

  // set monster
  {
    pUser->getTransform().enterTransform(m_dwTransformID, m_eType);
    pUser->setCollectMark(ECOLLECTTYPE_BASE);
    //pUser->setCollectMark(ECOLLECTTYPE_SKILL);
    pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    //pUser->setCollectMark(ECOLLECTTYPE_CARD);
    pUser->setCollectMarkAllBuff();
    pUser->updateAttribute();
    pUser->setAttr(EATTRTYPE_HP, pUser->getAttr(EATTRTYPE_MAXHP));
    pUser->setSp(0);
  }

  bData.me->m_oBuff.setTransform(true);
  DWORD body = pBase->figure.body;
  DWORD haircolor = pBase->figure.haircolor;
  //DWORD bodycolor = pBase->figure.bodycolor;
  DWORD lefthand = pBase->figure.lefthand;
  DWORD righthand = pBase->figure.righthand;
  DWORD hair = pBase->figure.hair;
  DWORD head = pBase->figure.head;
  DWORD wing = pBase->figure.wing;
  DWORD mount = pBase->figure.mount;
  DWORD face = pBase->figure.face;
  DWORD tail = pBase->figure.tail;
  DWORD eye = pBase->figure.eye;
  DWORD scale = 100; //protect

  DWORD rand = randBetween(1, 100);
  DWORD vecSize = m_vecScale.size();
  for (DWORD i = 1; i <vecSize; ++i)
  {
    m_vecScale[i].first += m_vecScale[i-1].first ;
  }
  for (auto v = m_vecScale.begin(); v != m_vecScale.end(); ++v)
  {
    if (rand <= v->first)
    {
      scale = v->second;
      break;
    }
  }

  if (bData.dwCommonData != 0)
    scale = bData.dwCommonData;
  else
    bData.dwCommonData = scale;

  m_mapFigure[EUSERDATATYPE_BODY] = body;
  m_mapFigure[EUSERDATATYPE_HAIRCOLOR] = haircolor;
  //m_mapFigure[EUSERDATATYPE_BODYCOLOR] = bodycolor;
  m_mapFigure[EUSERDATATYPE_LEFTHAND] = lefthand;
  m_mapFigure[EUSERDATATYPE_RIGHTHAND] = righthand;
  m_mapFigure[EUSERDATATYPE_HAIR] = hair;
  m_mapFigure[EUSERDATATYPE_BACK] = wing;
  m_mapFigure[EUSERDATATYPE_HEAD] = head;
  m_mapFigure[EUSERDATATYPE_MOUNT] = mount;
  m_mapFigure[EUSERDATATYPE_BODYSCALE] = scale;
  m_mapFigure[EUSERDATATYPE_FACE] = face;
  m_mapFigure[EUSERDATATYPE_TAIL] = tail;
  m_mapFigure[EUSERDATATYPE_EYE] = eye;

  refreshTransform(bData.me);
  return true;
}

bool BuffTransform::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  return doTransform(bData);
}

DWORD BuffTransform::getTransform(EUserDataType uType)
{
  auto iter = m_mapFigure.find(uType);
  if (iter != m_mapFigure.end())
    return iter->second;
  return 0;
}

void BuffTransform::ondel(xSceneEntryDynamic *me)
{
  if (!me)
    return;
  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if (pUser)
  {
    pUser->getTransform().exitTransform();
    pUser->setCollectMark(ECOLLECTTYPE_BASE);
    //pUser->setCollectMark(ECOLLECTTYPE_SKILL);
    pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
    //pUser->setCollectMark(ECOLLECTTYPE_CARD);
    pUser->setCollectMarkAllBuff();
    pUser->updateAttribute();
    //MsgManager::sendMsg(pUser->id, 831);
  }
  refreshTransform(me);
  me->m_oBuff.setTransform(false);
}

void BuffTransform::onInvalid(SBufferData& bData)
{
  BufferState::onInvalid(bData);
  ondel(bData.me);
}

void BuffTransform::refreshTransform(xSceneEntryDynamic *me)
{
  if (!me)
    return;
  me->setDataMark(EUSERDATATYPE_BODY);
  me->setDataMark(EUSERDATATYPE_HAIRCOLOR);
  me->setDataMark(EUSERDATATYPE_LEFTHAND);
  me->setDataMark(EUSERDATATYPE_RIGHTHAND);
  me->setDataMark(EUSERDATATYPE_HAIR);
  me->setDataMark(EUSERDATATYPE_BACK);
  me->setDataMark(EUSERDATATYPE_HEAD);
  me->setDataMark(EUSERDATATYPE_MOUNT);
  me->setDataMark(EUSERDATATYPE_BODYSCALE);
  me->setDataMark(EUSERDATATYPE_NORMAL_SKILL);
  me->setDataMark(EUSERDATATYPE_PET_PARTNER);
  me->setDataMark(EUSERDATATYPE_FACE);
  me->setDataMark(EUSERDATATYPE_TAIL);
  me->setDataMark(EUSERDATATYPE_MOUTH);
  me->setDataMark(EUSERDATATYPE_EYE);
  me->refreshDataAtonce();
}

//允许指定职业骑乘
BuffAllowRide::BuffAllowRide()
{

}

BuffAllowRide::~BuffAllowRide()
{

}

bool BuffAllowRide::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effData = data.getMutableData("BuffEffect");
  xLuaData proData = effData.getMutableData("value");
  auto getpro = [this](std::string key, xLuaData& value)
  {
    m_vecProfes.push_back(value.getInt());
  };
  proData.foreach(getpro);
  return true;
}

//掉落物品
BuffDropItem::BuffDropItem()
{
  m_eBuffType = EBUFFTYPE_DROPITEM;
  m_maxCnt = 1;
}

BuffDropItem::~BuffDropItem()
{

}

bool BuffDropItem::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  if(static_cast<DWORD>(randBetween(0, 100)) >= m_dwRate)
     return true;

  //掉落id = m_dwItemId的道具
  SceneUser *pUser = dynamic_cast<SceneUser *>(me);
  if(nullptr == pUser || pUser->getScene() == nullptr)
    return true;

  const SSceneItemCFG& rCFG = MiscConfig::getMe().getSceneItemCFG();

  xPos posResult;
  if (pUser->getScene()->getRandPos(pUser->getPos(), rCFG.getRange(1), posResult) == false)
    return false;

  ItemInfo itemDrop;
  itemDrop.set_id(m_dwItemID);
  itemDrop.set_count(m_dwItemNum);
  itemDrop.set_source(ESOURCE_BUFF);
  SceneItem *pItem = SceneItemManager::getMe().createSceneItem(pUser->getScene(), itemDrop, posResult);
  if(pItem != nullptr)
  {
    pItem->addOwner(pUser->id);

    Cmd::AddMapItem cmd;
    pItem->fillMapItemData(cmd.add_items(), rCFG.dwDropInterval);

    // inform to client
    PROTOBUF(cmd, send, len);
    pUser->getScene()->sendCmdToNine(pUser->getPos(), send, len);
  }
  XLOG << "[Buff - 奖励掉落], 玩家:" << pUser->name << pUser->id << "buff id=" << m_dwID << "itemid=" << m_dwItemID << "num=" << m_dwItemNum << XEND;
  return true;
}

bool BuffDropItem::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if(!BufferState::init(id, data, buffCond))
    return false;
  m_dwItemID = data.getData("BuffEffect").getTableInt("ItemID");
  m_dwItemNum = data.getData("BuffEffect").getTableInt("Num");
  m_dwRate = data.getData("BuffEffect").getTableInt("Odds");

  if (ItemConfig::getMe().getItemCFG(m_dwItemID) == nullptr)
  {
    XERR << "[BuffEffect-BuffDropItem] 物品item :" << m_dwItemID << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }
  if (m_dwItemNum == 0)
  {
    XERR << "[BuffEffect-BuffDropItem] 物品item :" << m_dwItemID << "Num 为 0 个" << XEND;
    return false;
  }
  return true;
}

//get skill
BuffGetSkill::BuffGetSkill()
{
  m_eBuffType = EBUFFTYPE_GETSKILL;
  m_maxCnt = 1;
}

BuffGetSkill::~BuffGetSkill()
{

}

bool BuffGetSkill::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if (!BufferState::init(id, data, buffCond))
    return false;

  xLuaData& effectData = data.getMutableData("BuffEffect");
  m_dwSkillID = effectData.getTableInt("SkillID");
  m_sourceItem = effectData.getTableInt("Itemid");

  if (SkillConfig::getMe().getSkillCFG(m_dwSkillID) == nullptr)
  {
    XERR << "[BufferEffect-BuffUseSkill] skillid :" << m_dwSkillID << "未在 Table_Skill.txt 表中找到" << XEND;
    return false;
  }

  if (effectData.has("Source"))
  {
    DWORD dwSource = effectData.getTableInt("Source");
    if (dwSource > static_cast<DWORD>(ESOURCE_MIN) && dwSource < static_cast<DWORD>(ESOURCE_MAX))
      m_eSource = static_cast<ESource>(dwSource);
  }
  m_dwLevelUp = effectData.getTableInt("LevelUp");
  m_dwExtraLv = effectData.getTableInt("ExtraLv");

  return true;
}

bool BuffGetSkill::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  SceneUser *pUser = dynamic_cast<SceneUser*>(me);
  if(nullptr == pUser)
    return false;
  if (m_dwLevelUp)
  {
    DWORD groupid = m_dwSkillID / ONE_THOUSAND;
    DWORD lv = pUser->getSkillLv(groupid);
    DWORD oldid = groupid * ONE_THOUSAND + lv;
    DWORD newid = oldid + m_dwLevelUp;
    pUser->removeSkill(oldid, m_sourceItem, m_eSource);
    pUser->addSkill(newid, m_sourceItem, m_eSource);
  }
  else if (m_dwExtraLv)
  {
    if (pUser->getFighter())
      pUser->getFighter()->getSkill().addExtraSkillLv(m_dwSkillID / ONE_THOUSAND, m_dwExtraLv);
  }
  else
  {
    pUser->addSkill(m_dwSkillID, m_sourceItem, m_eSource);
  }
  return true;
}

void BuffGetSkill::ondel(xSceneEntryDynamic *me)
{
  if (!me)
    return;
  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if (!pUser)
    return;
  if (m_dwLevelUp)
  {
    DWORD groupid = m_dwSkillID / ONE_THOUSAND;
    DWORD lv = pUser->getSkillLv(groupid);
    DWORD oldid = groupid * ONE_THOUSAND + lv;
    pUser->removeSkill(oldid, m_sourceItem, m_eSource);
    if (lv > m_dwLevelUp)
    {
      pUser->addSkill(oldid - m_dwLevelUp, m_sourceItem, m_eSource);
    }
  }
  else if (m_dwExtraLv)
  {
    if (pUser->getFighter())
      pUser->getFighter()->getSkill().decExtraSkillLv(m_dwSkillID / ONE_THOUSAND, m_dwExtraLv);
  }
  else
  {
    // 防止同时拥有多个该buff时, 移除一个该buff, 技能就失效
    if (pUser->m_oBuff.getLayerByID(m_dwID) <= 1)
      pUser->removeSkill(m_dwSkillID, m_sourceItem, m_eSource);
  }
  return ;
}

//驱散 或 删除某类型buff
BuffDriveout::BuffDriveout()
{
  m_eBuffType = EBUFFTYPE_DRIVEOUT;
}

BuffDriveout::~BuffDriveout()
{

}

bool BuffDriveout::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if(!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effData = data.getMutableData("BuffEffect");
  m_FmNum.load(effData.getMutableData("num"));
  if (effData.has("value"))
  {
    string type = effData.getTableString("value");
    if (type == "Hide")
      m_delType = EBUFFTYPE_HIDE;
    else if (type == "FakeDead")
      m_bDelFakeDead = true;
    else if (type == "GainBuff")
      m_bDelGain = true;
  }
  return true;
}

bool BuffDriveout::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD num = 0;
  if (m_FmNum.empty() == false)
    num = m_FmNum.getFmValue(me, &bData);

  for (auto &pEntry : uSet)
  {
    if (m_delType != EBUFFTYPE_MIN)
    {
      pEntry->m_oBuff.delBuffByType(m_delType);
      return true;
    }

    if (num != 0)
      pEntry->m_oBuff.delBuffByGain(m_bDelGain, num);

    if (m_bDelFakeDead)
    {
      pEntry->setStatus(ECREATURESTATUS_LIVE);
    }
  }
  return true;
}

// AddBuff
BuffAddBuff::BuffAddBuff()
{
  m_eBuffType = EBUFFTYPE_ADDBUFF;
}

BuffAddBuff::~BuffAddBuff()
{

}

bool BuffAddBuff::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if(!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effData = data.getMutableData("BuffEffect");
  xLuaData& buffData = effData.getMutableData("id");

  auto getIDs = [this](std::string key, xLuaData& data)
  {
    m_vecBuffIDs.push_back(data.getInt());
  };
  buffData.foreach(getIDs);

  m_bRandom = effData.getTableInt("random") != 0;
  m_bSelectOneValid = effData.getTableInt("selectone") != 0;
  m_dwOdds = effData.getTableInt("Odds");
  // buff id check
  auto getlayer = [&](const string& key, xLuaData& d)
  {
    DWORD id = d.getTableInt("id");
    DWORD layer = d.getTableInt("layer");
    m_mapID2Layer[id] = layer;
  };
  effData.getMutableData("layer").foreach(getlayer);

  if(effData.has("rand_by_weight")){
    xLuaData& weightData = effData.getMutableData("rand_by_weight");
    int weightAccum = 0;
    auto getWeights = [&](const string& key, xLuaData& d){
      weightAccum += d.getInt();
      m_vecWeightSkillids.push_back(atoi(key.c_str()));
      m_vecWeightAccum.push_back(weightAccum);
    };
    weightData.foreach(getWeights);
  }

  return true;
}

bool BuffAddBuff::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || me->getScene() == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  xSceneEntryDynamic* pFromEntry = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (pFromEntry == nullptr)
    return false;

  TVecDWORD tempvec;
  for (auto &v : m_vecBuffIDs)
    tempvec.push_back(v);
  if (m_bRandom)
  {
    tempvec.clear();
    DWORD size = m_vecBuffIDs.size();
    if (size == 0)
      return true;
    DWORD index = randBetween(0, size - 1);
    tempvec.push_back(m_vecBuffIDs[index]);
  }
  // 选取一个buff 添加, 解决buff层数问题 , 伤害增压
  if (m_bSelectOneValid)
  {
    tempvec.clear();
    for (auto &v : m_vecBuffIDs)
    {
      if (!me->m_oBuff.haveBuff(v))
      {
        tempvec.push_back(v);
        break;
      }
    }
  }

  if (m_dwOdds == 0 || m_dwOdds >= (DWORD)randBetween(1, 100))
  {
    for (auto &pEntry : uSet)
    {
      for (auto &v : tempvec)
      {
        pEntry->m_oBuff.add(v, pFromEntry, bData.lv, bData.dwDamage);
      }
    }
  }

  // 添加多层Buff
  if (!m_mapID2Layer.empty())
  {
    for (auto &m : m_mapID2Layer)
    {
      for (auto &pEntry : uSet)
      {
        for (DWORD i = 0; i < m.second; ++i)
        {
          pEntry->m_oBuff.add(m.first, pFromEntry, bData.lv, bData.dwDamage);
        }
      }
    }
  }

  // 根据权重添加一个buff
  if (!m_vecWeightAccum.empty())
  {
    DWORD rand = (DWORD)randBetween(1, m_vecWeightAccum[m_vecWeightAccum.size()-1]);
    DWORD skillidIndex = 0;
    while(rand > m_vecWeightAccum[skillidIndex]){
      skillidIndex++;
    }
    if(skillidIndex >= m_vecWeightSkillids.size()){
      XERR << "[BufferEffect-AddBuff] rand_by_weight超出范围, rand: " << rand << " WeightAccumMax: " << m_vecWeightAccum[m_vecWeightAccum.size()-1];
      return false;
    }
    for(auto &pEntry : uSet)
    {
      pEntry->m_oBuff.add(m_vecWeightSkillids[skillidIndex], pFromEntry, bData.lv, bData.dwDamage);
    }
  }

  return true;
}

BuffStatusChange::BuffStatusChange()
{
  m_eBuffType = EBUFFTYPE_STATUS;
}

BuffStatusChange::~BuffStatusChange()
{

}

bool BuffStatusChange::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if(!BufferState::init(id, data, buffCond))
    return false;
  return true;
}

bool BuffStatusChange::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  //change hp
  const TVecAttrSvrs& uAttrs = bData.getAttr();
  for (auto m = uAttrs.begin(); m != uAttrs.end(); ++m)
  {
    if (m->type() != EATTRTYPE_HP || m->value() > 0)
      continue;
    xSceneEntryDynamic* attacker = nullptr;
    attacker = xSceneEntryDynamic::getEntryByID(bData.fromID);
    if (attacker == nullptr)
      return false;
    me->doBuffAttack(0 - m->value(), attacker);
  }

  return true;
}

void BuffStatusChange::ondel(xSceneEntryDynamic *me)
{
  if (me == nullptr)
    return;

  me->m_oBuff.setStatusChange();
}

BuffSpRecover::BuffSpRecover()
{
  m_eBuffType = EBUFFTYPE_SPRECOVER;
}

BuffSpRecover::~BuffSpRecover()
{

}

bool BuffSpRecover::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_bCanBeDisable = data.getData("BuffEffect").getTableInt("disable") != 0;
  return true;
}

bool BuffSpRecover::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  // sp 无法自然恢复
  if (m_bCanBeDisable && me->isNoSpRecover())
    return true;

  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if (pUser == nullptr)
    return false;

  const TVecAttrSvrs& uAttrs = bData.getAttr();
  for (auto m = uAttrs.begin(); m != uAttrs.end(); ++m)
  {
    if (m->type() == EATTRTYPE_SP)
    {
      if (m->value() >= 1)
      {
        float value = m->value();
        float nowsp = me->getAttr(EATTRTYPE_SP);
        float dif = me->getAttr(EATTRTYPE_MAXSP) - nowsp;
        value = value > dif ? dif : value;
        if (value > 0)
          pUser->setSp(nowsp + value);
        return true;
      }
    }
  }
  return true;
}

BuffPlayAction::BuffPlayAction()
{
  m_eBuffType = EBUFFTYPE_PLAYACTION;
  m_maxCnt = 1;
}

BuffPlayAction::~BuffPlayAction()
{

}

bool BuffPlayAction::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwEmoji = data.getData("BuffEffect").getTableInt("id");

  if (TableManager::getMe().getExpressionCFG(m_dwEmoji) == nullptr)
  {
    XERR << "[BufferEffect-BuffPlayAction] emoji :" << m_dwEmoji << "未在 Table_Expression.txt 表中找到" << XEND;
    return false;
  }

  return true;
}

bool BuffPlayAction::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  me->playEmoji(m_dwEmoji);
  return true;
}

BuffDisable::BuffDisable()
{
  m_eBuffType = EBUFFTYPE_DISABLE;
  m_maxCnt = 1;
}

BuffDisable::~BuffDisable()
{

}


bool BuffDisable::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  DWORD disID = data.getData("BuffEffect").getTableInt("id");
  if (disID != 0)
    m_setIDs.insert(disID);
  xLuaData& ids = data.getMutableData("BuffEffect").getMutableData("id");
  auto getid = [&](const string& key, xLuaData& value)
  {
    if (value.getInt())
      m_setIDs.insert(value.getInt());
  };
  ids.foreach(getid);
  return true;
}

bool BuffDisable::onStart(SBufferData& bData, DWORD cur)
{
  xSceneEntryDynamic* me = bData.me;
  if (me == nullptr)
    return false;
  for (auto &s : m_setIDs)
    me->m_oBuff.addDisableID(s);
  return true;
}

void BuffDisable::ondel(xSceneEntryDynamic *me)
{
  if (me == nullptr)
    return;
  for (auto &s : m_setIDs)
    me->m_oBuff.delDisableID(s);
    //me->m_oBuff.enableBuff(s);
}

BuffBattleAttr::BuffBattleAttr()
{
  m_eBuffType = EBUFFTYPE_ELEMENT;
}

BuffBattleAttr::~BuffBattleAttr()
{

}

bool BuffBattleAttr::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_bAtkAttr = data.getData("BuffEffect").has("AtkAttr");
  m_bDefAttr = data.getData("BuffEffect").has("DefAttr");
  if (!m_bAtkAttr && !m_bDefAttr)
  {
    XERR << "[Buff-配置错误], buff id:" << id << "没有配置AtkAttr 和 DefAttr" << XEND;
    return false;
  }
  m_dwPriority = data.getData("BuffEffect").getTableInt("priority");
  /*if (m_dwPriority == 0)
  {
    XERR << "[Buff-配置错误], buffid :" << id << "priority=0, 没有配置有效优先级" << XEND;
    return false;
  }
  */
  return true;
}

void BuffBattleAttr::ondel(xSceneEntryDynamic* me)
{
  if (me == nullptr)
    return;
  //if (m_bDisableOther == false)
  if (isDisOther() == false)
    return;
  //me->m_oBuff.enableType(EBUFFTYPE_ELEMENT);
}

BuffDelMe::BuffDelMe()
{
  m_eBuffType = EBUFFTYPE_DELME;
}

BuffDelMe::~BuffDelMe()
{

}

bool BuffDelMe::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  return true;
}

bool BuffDelMe::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (me);
  if (pNpc == nullptr)
    return false;
  pNpc->setClearState();
  return true;
}

BuffHpReduce::BuffHpReduce()
{
  m_eBuffType = EBUFFTYPE_HPREDUCE;
}

BuffHpReduce::~BuffHpReduce()
{

}

bool BuffHpReduce::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  //m_fDelHpPer = data.getData("BuffEffect").getTableFloat("delHpPer");
  xLuaData& effdata = data.getMutableData("BuffEffect");
  xLuaData& delhpper = effdata.getMutableData("delHpPer");
  m_fmDelHpPer.load(delhpper);
  return true;
}

bool BuffHpReduce::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  float per = me->getAttr(EATTRTYPE_HP) / me->getAttr(EATTRTYPE_MAXHP);

  float delper = 0;
  if (bData.dwCommonData)
  {
    delper = ((float)bData.dwCommonData) / 100.0f;
  }
  else
  {
    delper = m_fmDelHpPer.getFmValue(me, &bData);
    bData.dwCommonData = delper * 100;
  }

  if (per < delper)
  {
    me->m_oBuff.del(m_dwID);
    return true;
  }

  const TVecAttrSvrs& uAttrs = bData.getAttr();
  for (auto m = uAttrs.begin(); m != uAttrs.end(); ++m)
  {
    if (m->type() == EATTRTYPE_HP)
    {
      xSceneEntryDynamic* pFromE = xSceneEntryDynamic::getEntryByID(bData.fromID);
      if (pFromE == nullptr)
        return false;
      me->doBuffAttack(0-m->value(), pFromE);
      return true;
    }
  }
  return false;
}

// DelBuff
BuffDelBuff::BuffDelBuff()
{
  m_eBuffType = EBUFFTYPE_DELBUFF;
  m_maxCnt = 1;
}

BuffDelBuff::~BuffDelBuff()
{

}

bool BuffDelBuff::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if(!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effData = data.getMutableData("BuffEffect");
  xLuaData& buffData = effData.getMutableData("id");

  auto getIDs = [this](std::string key, xLuaData& data)
  {
    m_vecBuffIDs.push_back(data.getInt());
  };
  buffData.foreach(getIDs);
  
  auto getlayer = [&](const string& key, xLuaData& d)
  {
    DWORD id = d.getTableInt("id");
    DWORD layer = d.getTableInt("layer");
    m_mapID2Layer[id] = layer;
  };
  effData.getMutableData("layer").foreach(getlayer);

  m_bSameSource = effData.getTableInt("same_source") == 1;

  m_bDeadClear = effData.getTableInt("dead_clear") == 1;
  auto deadclearf = [&](const string& key, xLuaData& d)
  {
    m_setDeadClearExcept.insert(d.getInt());
  };
  effData.getMutableData("dead_clear_except").foreach(deadclearf);

  return true;
}

bool BuffDelBuff::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || me->getScene() == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  for (auto &v : m_vecBuffIDs)
  {
    for (auto &pEntry : uSet)
    {
      if (m_bSameSource == true && me->id != pEntry->m_oBuff.getBuffFromID(v))
        continue;
      pEntry->m_oBuff.del(v, true);
    }
  }

  for (auto &m : m_mapID2Layer)
  {
    for (auto &pEntry : uSet)
    {
      pEntry->m_oBuff.delLayer(m.first, m.second);
    }
  }  

  if (m_bDeadClear)
  {
    for (auto &pEntry : uSet)
      pEntry->m_oBuff.clear(m_setDeadClearExcept);
  }
  return true;
}

// 删除状态类buff
BuffDelStatus::BuffDelStatus()
{
  m_eBuffType = EBUFFTYPE_DELSTATUS;
}

BuffDelStatus::~BuffDelStatus()
{

}

bool BuffDelStatus::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_dwRandomNum = effdata.getTableInt("random_num");

  xLuaData& statusData = effdata.getMutableData("status");
  auto getstatus = [&](std::string key, xLuaData& data)
  {
    m_setStatus.insert(data.getInt());
  };
  statusData.foreach(getstatus);
  if (m_dwRandomNum > m_setStatus.size())
  {
    XERR << "[BuffConfig], id=" << id << "random_num 过大, 配置错误" << XEND;
    m_dwRandomNum = m_setStatus.size();
  }
  return true;
}

bool BuffDelStatus::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || me->getScene() == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  if (m_dwRandomNum > m_setStatus.size())
    return false;

  /*
  TSetDWORD setStatus;
  if (m_dwRandomNum == m_vecStatus.size())
  {
    setStatus.insert(m_vecStatus.begin(), m_vecStatus.end());
  }
  else
  {
    TVecDWORD vecTemp = m_vecStatus;
    std::random_shuffle(vecTemp.begin(), vecTemp.end());
    for (DWORD i = 0; i < m_dwRandomNum; ++i)
      setStatus.insert(vecTemp[i]);
  }

  for (auto &pEntry: uSet)
  {
    pEntry->m_oBuff.delStatus(setStatus);
  }
  */

  for (auto &pEntry: uSet)
  {
    TSetDWORD stSet;
    pEntry->m_oBuff.getStatus(stSet);
    if (stSet.empty())
      continue;
    TVecDWORD vecTemp(m_setStatus.size());
    auto it = std::set_intersection(m_setStatus.begin(), m_setStatus.end(), stSet.begin(), stSet.end(), vecTemp.begin());
    vecTemp.resize(it - vecTemp.begin());
    if (vecTemp.size() > m_dwRandomNum)
    {
      std::random_shuffle(vecTemp.begin(), vecTemp.end());
      vecTemp.resize(m_dwRandomNum);
    }
    for (auto &v : vecTemp)
      pEntry->m_oBuff.delStatus(v);
  }
  return true;
}


// 免疫状态类buff
BuffImmuneStatus::BuffImmuneStatus()
{
  m_eBuffType = EBUFFTYPE_DSTATUS;
}

BuffImmuneStatus::~BuffImmuneStatus()
{

}

bool BuffImmuneStatus::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  xLuaData& statusdata = effdata.getMutableData("status");
  auto getstatus = [&](const string& str, xLuaData& data)
  {
    m_setStatus.insert(data.getInt());
  };
  statusdata.foreach(getstatus);
  return true;
}

// 使陷阱现身
BuffSeeTrap::BuffSeeTrap()
{
  m_eBuffType = EBUFFTYPE_SEETRAP;
}

BuffSeeTrap::~BuffSeeTrap()
{

}

bool BuffSeeTrap::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (!BufferState::init(id, data, buffCond))
    return false;
  m_dwRange = data.getMutableData("BuffEffect").getTableInt("range");
  string stype = data.getMutableData("BuffEffect").getTableString("see_type");
  if (stype == "special_skill")
  {
    m_eType = ETRAPTYPE_SPECSKILL;
    data.getMutableData("BuffEffect").getMutableData("skillid").getIDList(m_setSeeSkills);
  }

  return true;
}

bool BuffSeeTrap::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || me->getScene() == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  xSceneEntrySet npcset;
  me->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, me->getPos(), m_dwRange, npcset);
  switch (m_eType)
  {
    case ETRAPTYPE_NORMALTRAP:
      {
        for (auto &s : npcset)
        {
          TrapNpc* npc = dynamic_cast<TrapNpc*> (s);
          if (npc == nullptr)
            continue;
          npc->addTrapSeeUser(me);
        }
      }
      break;
    case ETRAPTYPE_SPECSKILL:
      {
        for (auto &s : npcset)
        {
          SkillNpc* npc = dynamic_cast<SkillNpc*> (s);
          if (npc == nullptr)
            continue;
          DWORD skillid = npc->getSkillID() / ONE_THOUSAND;
          if (m_setSeeSkills.find(skillid) == m_setSeeSkills.end())
            continue;
          SceneUser* pMaster = npc->getMasterUser();
          if (pMaster == nullptr || me->isMyEnemy(pMaster) == false)
            continue;

          npc->removeTeamSeeLimit();
          npc->sendMeToNine();
        }
      }
      break;
    default:
      break;
  }
  return true;
}


// 部分变身
BuffPartTransform::BuffPartTransform()
{
  m_eBuffType = EBUFFTYPE_PARTTRANSFORM;
  m_maxCnt = 1;
}

BuffPartTransform::~BuffPartTransform()
{

}

bool BuffPartTransform::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (!BufferState::init(id, data, buffCond))
    return false;
  xLuaData& effData = data.getMutableData("BuffEffect");
  vector<pair<string, EUserDataType>> vecFormat;
  vecFormat.push_back(pair<string, EUserDataType>("Head", EUSERDATATYPE_HEAD));
  vecFormat.push_back(pair<string, EUserDataType>("Body", EUSERDATATYPE_BODY));
  vecFormat.push_back(pair<string, EUserDataType>("Hair", EUSERDATATYPE_HAIR));
  vecFormat.push_back(pair<string, EUserDataType>("HairColor", EUSERDATATYPE_HAIRCOLOR));
  vecFormat.push_back(pair<string, EUserDataType>("LeftHand", EUSERDATATYPE_LEFTHAND));
  vecFormat.push_back(pair<string, EUserDataType>("RightHand", EUSERDATATYPE_RIGHTHAND));
  vecFormat.push_back(pair<string, EUserDataType>("Back", EUSERDATATYPE_BACK));
  vecFormat.push_back(pair<string, EUserDataType>("Crystal", EUSERDATATYPE_ALPHA));
  vecFormat.push_back(pair<string, EUserDataType>("Mount", EUSERDATATYPE_MOUNT));
  vecFormat.push_back(pair<string, EUserDataType>("Porttrait", EUSERDATATYPE_PORTRAIT));
  vecFormat.push_back(pair<string, EUserDataType>("Tail", EUSERDATATYPE_TAIL));
  //vecFormat.push_back(pair<string, EUserDataType>("BodyScale", EUSERDATATYPE_BODYSCALE));

  for (auto &v : vecFormat)
  {
    if (!effData.has(v.first))
      continue;
    m_mapFigure[v.second] = effData.getTableInt(v.first);
  }

  auto p2mf = [&](const string& str, xLuaData& d)
  {
    m_mapPartner2Mount[atoi(str.c_str())] = d.getInt();
  };
  effData.getMutableData("partner2mount").foreach(p2mf);
  auto m2bf = [&](const string& str, xLuaData& d)
  {
    m_mapMount2Body[atoi(str.c_str())] = d.getInt();
  };
  effData.getMutableData("mount2body").foreach(m2bf);

  return true; 
}

bool BuffPartTransform::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr)
    return false;
  if (BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  for (auto &m : m_mapFigure)
  {
    me->setDataMark(m.first);
  }
  me->m_oBuff.setPartTransform(true);
  me->refreshDataAtonce();
  return true;
}

void BuffPartTransform::ondel(xSceneEntryDynamic* me)
{
  if (me == nullptr)
    return;
  for (auto &m : m_mapFigure)
  {
    me->setDataMark(m.first);
  }
  me->m_oBuff.setPartTransform(false);
  me->refreshDataAtonce();
}

DWORD BuffPartTransform::get(EUserDataType eType, xSceneEntryDynamic* entry/* = nullptr*/) const
{
  auto it = m_mapFigure.find(eType);
  if (it == m_mapFigure.end())
    return 0;
  if (entry)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(entry);
    if (user)
    {
      if (eType == EUSERDATATYPE_MOUNT)
      {
        auto p2mit = m_mapPartner2Mount.find(user->getPet().getActivePartnerID());
        if (p2mit != m_mapPartner2Mount.end())
          return p2mit->second;
      }
      else if (eType == EUSERDATATYPE_BODY)
      {
        auto m2bit = m_mapMount2Body.find(user->getEquipID(EEQUIPPOS_MOUNT));
        if (m2bit != m_mapMount2Body.end())
          return m2bit->second;
      }
    }
  }

  return it->second;
}

// 杀死怪物额外倍数掉落
BuffRobReward::BuffRobReward()
{
  m_eBuffType = EBUFFTYPE_ROBREWARD;
}

BuffRobReward::~BuffRobReward()
{

}

bool BuffRobReward::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwExtraCnt = data.getMutableData("BuffEffect").getTableInt("extra");
  if (m_dwExtraCnt == 0)
  {
    XERR << "[Buff-配置], id=" << m_dwID << "配置错误, extra为0" << XEND;
    return false;
  }
  m_dwBattleTime = data.getMutableData("BuffEffect").getTableInt("time");
  return true;
}

void BuffRobReward::onKillMonster(SceneUser* user, SceneNpc* npc, SBufferData& bData)
{
  if (user == nullptr || !user->getScene() || npc == nullptr || npc->getScene() == nullptr)
    return;
  const SNpcCFG* pCFG = npc->getCFG();
  if (pCFG == nullptr)
    return;
  if (pCFG->eNpcType == ENPCTYPE_MINIBOSS || pCFG->eNpcType == ENPCTYPE_MVP || npc->define.m_oVar.m_qwQuestOwnerID != 0 || pCFG->bPredatory == false)
    return;

  if (bData.layers == 0) return;

  if (m_dwBattleTime <= 0)
    bData.layers --;

  //statics
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_ROB_COUNT, 0, 0, user->getLevel(), (DWORD)1);

  float fRatio = LuaManager::getMe().call<float>("calcMapRewardRatio", npc, user, false, false);

  Cmd::AddMapItem cmd;
  const SSceneItemCFG& rCFG = MiscConfig::getMe().getSceneItemCFG();
  for (DWORD cnt = 0; cnt < m_dwExtraCnt; ++ cnt)
  {
    //statics
    StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_MONSTER_ITEM_REWARD, npc->getNpcID(),0, 0, fRatio);

    TVecItemInfo vecItemInfo;
    if(npc->define.getSuperAiNpc() == false)
    {
      for (auto v = pCFG->vecRewardIDs.begin(); v != pCFG->vecRewardIDs.end(); ++v)
      {
        TVecItemInfo item;
        if (RewardManager::roll(*v, nullptr, item, m_dwBattleTime <= 0 ? ESOURCE_ROB : ESOURCE_PICKUP, fRatio) == false)
          continue;
        for (auto &i : item)
          vecItemInfo.push_back(i);
      }
    }

    float fRange = rCFG.getRange(static_cast<DWORD>(vecItemInfo.size()));
    xPos posResult;
    for (DWORD i = 0; i < vecItemInfo.size(); ++i)
    {
      if (npc->getScene()->getRandPos(npc->getPos(), fRange, posResult) == false)
        continue;

      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(user->getScene(), vecItemInfo[i], posResult);
      if(pItem != nullptr)
      {
        pItem->setViewLimit();
        pItem->addOwner(user->id);

        pItem->fillMapItemData(cmd.add_items(), rCFG.dwDropInterval * (i + 1));
      }
      XDBG << "[掠夺许可证-物品掉落] charid:" << user->id << npc->id << npc->getNpcID() << npc->name << "drop sceneitem id:" << vecItemInfo[i].id() << "count:" << vecItemInfo[i].count() << XEND;
    }
  }

  // inform to client
  if (cmd.items_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }
}

void BuffRobReward::onAddBattleTime(SBufferData& bData, DWORD time)
{
  if (m_dwBattleTime <= 0)
    return;

  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->isInBattleTimeMap() == false)
    return;

  bData.layers = (bData.layers > time ? bData.layers - time : 0);
  time *= (m_dwBattleTime - 1);

  DWORD usedbattletime = user->getUserSceneData().getUsedBattleTime();
  DWORD rebattletime = user->getUserSceneData().getReBattleTime();
  if (rebattletime > usedbattletime)
  {
    DWORD addtime = (usedbattletime + time <= rebattletime ? time : rebattletime - usedbattletime);
    if (time >= addtime)
      time -= addtime;
    user->getUserSceneData().setUsedBattleTime(addtime + usedbattletime);
  }
  if (time != 0)
  {
    DWORD oldtime = user->getUserSceneData().getBattleTime();
    user->getUserSceneData().setBattleTime(oldtime + time);
  }

  XDBG << "[Buff-掠夺许可证], 玩家:" << user->name << user->id << "添加额外战斗时间:" << time << XEND;
}

// 表情达人, 牵手时做表情, 给对方加buff
BuffHandEmoji::BuffHandEmoji()
{
  m_eBuffType = EBUFFTYPE_HANDEMOJI;
}

BuffHandEmoji::~BuffHandEmoji()
{

}

bool BuffHandEmoji::init(DWORD id, xLuaData &data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  xLuaData& cd = effdata.getMutableData("cd");
  m_cdFm.load(cd);
  xLuaData& emoji = effdata.getMutableData("emoji");
  auto getdata = [&](const string& key, xLuaData& data)
  {
    pair<DWORD, DWORD> pa;
    pa.first = data.getTableInt("1");
    pa.second = data.getTableInt("2");
    m_vecEmoji2Buff.push_back(pa);
  };
  emoji.foreach(getdata);
  return true;
}

void BuffHandEmoji::onPlayEmoji(DWORD id, SBufferData& bData)
{
  // only vice hand-er
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->m_oHands.has() == false || user->m_oHands.isMaster())
    return;

  DWORD cur = now();
  // check cd
  if (bData.dwCommonData > cur)
    return;

  auto it = find_if(m_vecEmoji2Buff.begin(), m_vecEmoji2Buff.end(), [id](const pair<DWORD, DWORD>& r){
      return id == r.first;
      });
  if (it == m_vecEmoji2Buff.end())
    return;

  SceneUser* tarUser = SceneUserManager::getMe().getUserByID(user->m_oHands.getMasterID());
  if (tarUser == nullptr || tarUser->isAlive() == false)
    return;

  // add cd
  bData.dwCommonData = cur + (DWORD)m_cdFm.getFmValue(bData.me, &bData);

  // do effect
  tarUser->m_oBuff.add(it->second, user);
}


BuffChangeDefine::BuffChangeDefine()
{
  m_eBuffType = EBUFFTYPE_CHDEFINE;
  m_maxCnt = 1;
}

BuffChangeDefine::~BuffChangeDefine()
{

}

bool BuffChangeDefine::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_oData = data.getMutableData("BuffEffect");

  return true;
}

bool BuffChangeDefine::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  SceneNpc* npc = dynamic_cast<SceneNpc*> (bData.me);
  if (!npc || !npc->isAlive() || !npc->getScene())
    return false;

  if (m_oData.has("scale"))
    npc->setScale(m_oData.getTableFloat("scale"));
  return true;
}

bool BuffChangeDefine::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys &uSet, SBufferData &bData)
{
  if (BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (!npc || !npc->isAlive() || !npc->getScene())
    return false;

  if (m_oData.has("search"))
  {
    DWORD oldsearch = npc->define.getSearch();
    npc->define.setSearch(m_oData.getTableInt("search"));
    DWORD newsearch = npc->define.getSearch();
    if (m_oData.getTableInt("pri_attack") == 1)
    {
      if (oldsearch == 0 && newsearch > 0)
        npc->m_ai.setPriAttackUser(npc->m_ai.getLastAttacker());
    }
  }

  return true;
}

void BuffChangeDefine::ondel(xSceneEntryDynamic* me)
{
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (!npc) return;

  if (m_oData.has("search"))
    npc->define.setSearch(npc->m_oOriDefine.getSearch());
  return;
}

BuffForceAttr::BuffForceAttr()
{
  m_maxCnt = 1;
  m_eBuffType = EBUFFTYPE_FORCEATTR;
}

BuffForceAttr::~BuffForceAttr()
{

}

bool BuffForceAttr::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  me->m_oBuff.setForceAttr(true);
  return true;
}

void BuffForceAttr::ondel(xSceneEntryDynamic *me)
{
  if (!me) return;
  me->m_oBuff.setForceAttr(false);
}

// treasure
BuffTreasure::BuffTreasure()
{
  m_eBuffType = EBUFFTYPE_TREASURE;
}

BuffTreasure::~BuffTreasure()
{

}

void BuffTreasure::ondel(xSceneEntryDynamic *me)
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(me);
  if (pUser != nullptr && pUser->getScene() != nullptr)
    pUser->getScene()->getSceneTreasure().onLeaveScene(pUser);
}

bool BuffTreasure::onStart(SBufferData& bData, DWORD cur)
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(bData.me);
  if (pUser != nullptr && pUser->getScene() != nullptr)
    pUser->getScene()->getSceneTreasure().onEnterScene(pUser);
  return true;
}

// lose target
BuffLoseTarget::BuffLoseTarget()
{
  m_eBuffType = EBUFFTYPE_LOSETARGET;
}

BuffLoseTarget::~BuffLoseTarget()
{

}

bool BuffLoseTarget::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_bNoMvp = effdata.getTableInt("no_mvp")==1;
  m_bNoMini = effdata.getTableInt("no_mini")==1;
  return true;
}

bool BuffLoseTarget::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (npc)
  {
    if (npc->isAlive())
      npc->m_ai.setCurLockID(0);
  }
  else
  {
    SceneUser* user = dynamic_cast<SceneUser*>(me);
    if (user)
    {
      TSetQWORD monsterids = user->getLockMeList();
      for (auto &s : monsterids)
      {
        SceneNpc* p = SceneNpcManager::getMe().getNpcByTempID(s);
        if (p == nullptr || p->m_ai.getCurLockID() != user->id)
          continue;
        if (m_bNoMvp && p->getNpcType() == ENPCTYPE_MVP)
          continue;
        if (m_bNoMini && p->getNpcType() == ENPCTYPE_MINIBOSS)
          continue;
        p->m_ai.setCurLockID(0); //会操作user->getLockMeList列表
      }
    }
  }
  return true;
}

// prior attack
BuffPriorAttack::BuffPriorAttack()
{
  m_eBuffType = EBUFFTYPE_PRIORATTACK;
}

BuffPriorAttack::~BuffPriorAttack()
{

}

bool BuffPriorAttack::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effdata = data.getMutableData("BuffEffect");
  if (effdata.has("profession"))
  {
    auto getprf = [&](const string& str, xLuaData& d)
    {
      m_setPriorProfession.insert(d.getInt());
    };
    effdata.getMutableData("profession").foreach(getprf);
  }
  if (effdata.has("profession_ignore"))
  {
    auto getprf = [&](const string& str, xLuaData& d)
    {
      m_setIgnoreProfession.insert(d.getInt());
    };
    effdata.getMutableData("profession_ignore").foreach(getprf);
  }
  if (effdata.has("gender"))
  {
    DWORD gender = effdata.getTableInt("gender");
    if (gender != (DWORD)EGENDER_MALE && gender != (DWORD)EGENDER_FEMALE)
    {
      XERR << "[Buff], id:" << id << "gender配置错误" << XEND;
      return false;
    }
    m_ePriorGender = static_cast<EGender> (gender);
  }
  return true;
}

bool BuffPriorAttack::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  SceneNpc* npc = dynamic_cast<SceneNpc*> (bData.me);
  if (npc == nullptr)
    return false;

  for (auto &s : m_setPriorProfession)
  {
    npc->getNpcAI().priorAttackProfession(s);
  }
  if (m_ePriorGender != EGENDER_MIN)
  {
    npc->getNpcAI().priorAttackGender(m_ePriorGender);
  }
  for (auto &s : m_setIgnoreProfession)
  {
    npc->m_ai.ignoreAttackProfession(s);
  }
  return true;
}

void BuffPriorAttack::ondel(xSceneEntryDynamic* me)
{
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (npc == nullptr)
    return;
  if (m_setPriorProfession.empty() == false || m_ePriorGender != EGENDER_MIN)
    npc->getNpcAI().clearPriorAttack();
  if (m_setIgnoreProfession.empty() == false)
    npc->getNpcAI().clearIgnoreAttack();

  return;
}

BuffHpProtect::BuffHpProtect()
{
  m_eBuffType = EBUFFTYPE_HPPROTECT;
}

BuffHpProtect::~BuffHpProtect()
{

}

bool BuffHpProtect::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwProtectInterval = data.getMutableData("BuffEffect").getTableInt("time");
  m_fProtectHpPer = data.getMutableData("BuffEffect").getTableFloat("hpPer");
  if (m_dwProtectInterval == 0 || m_fProtectHpPer == 0)
  {
    XERR << "[Buff-配置错误], time, hp 配置不可为0, id:" << m_dwID << XEND;
    return false;
  }
  return true;
}

bool BuffHpProtect::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  SceneNpc* npc = dynamic_cast<SceneNpc*> (bData.me);
  if (npc == nullptr)
    return false;

  npc->setProtect(m_dwProtectInterval, m_fProtectHpPer * npc->getAttr(EATTRTYPE_MAXHP));
  return true;
}

void BuffHpProtect::ondel(xSceneEntryDynamic *me)
{
  SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
  if (npc == nullptr)
    return;
  npc->setProtect(0, 0);
}

BuffMultiTime::BuffMultiTime()
{
  m_eBuffType = EBUFFTYPE_MULTITIME;
}

BuffMultiTime::~BuffMultiTime()
{

}

bool BuffMultiTime::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwRate = data.getMutableData("BuffEffect").getTableInt("rate");
  if (m_dwRate < 2)
  {
    XERR << "[Buff-配置], 配置错误, id:" << m_dwID << "rate 配置错误" << m_dwRate << XEND;
    return false;
  }
  return true;
}

bool BuffMultiTime::isValidGet(SceneUser* user, const SNpcCFG* npcCFG)
{
  if (user == nullptr || npcCFG == nullptr)
    return false;
  if (user->isInBattleTimeMap() == false)
    return false;
  if (!npcCFG->isFieldMonster())
    return false;

  return true;
}

void BuffMultiTime::onGetExp(SBufferData& bData, SceneNpc* npc, DWORD baseExp, DWORD jobExp)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || npc == nullptr)
    return;
  if (isValidGet(user, npc->getCFG()) == false)
    return;
  if (m_dwRate < 2)
    return;

  DWORD extra = m_dwRate - 1;
  if (baseExp)
  {
    user->addBaseExp(baseExp * extra, ESOURCE_MONSTERKILL);
    user->getUserPet().addBaseExp((baseExp * extra));
    user->getUserBeing().addBaseExp(baseExp * extra);
    XLOG << "[Buff-多倍卡生效], 玩家" << user->name << user->id << "获得额外经验:" << baseExp * extra << XEND;
  }
  if (jobExp)
  {
    user->addJobExp(jobExp * extra, ESOURCE_MONSTERKILL);
    XLOG << "[Buff-多倍卡生效], 玩家" << user->name << user->id << "获得额外job经验:" << jobExp * extra << XEND;
  }
}

void BuffMultiTime::onPickUpItem(SBufferData& bData, const ItemInfo& oItem)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->isInBattleTimeMap() == false)
    return;

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(oItem.source_npc());
  if (pCFG == nullptr)
    return;
  if (isValidGet(user, pCFG) == false)
    return;
  if (pCFG->getFeaturesByType(ENPCFEATURESPARAM_CHAINDOUBLE) == true)
    return;

  if (m_dwRate < 2)
    return;

  for (DWORD i = 0; i < m_dwRate - 1; ++i)
  {
    ItemInfo tmpItem;
    tmpItem.CopyFrom(oItem);

    bool bshow = false;
    const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(oItem.id());
    if (pItemCFG && ItemConfig::getMe().isCard(pItemCFG->eItemType))
      bshow = true;
    //user->getPackage().addItemAvailable(tmpItem, false, true);
    user->getPackage().addItem(tmpItem, EPACKMETHOD_AVAILABLE, bshow, true);

    XLOG << "[Buff-多倍卡生效], 玩家:" << user->name << user->id << "获得额外物品:" << oItem.id() << "数量:" << oItem.count() << XEND;
  }
}

void BuffMultiTime::onAddBattleTime(SBufferData& bData, DWORD time)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->isInBattleTimeMap() == false)
    return;

  bData.layers = (bData.layers > time ? bData.layers - time : 0);
  time *= (m_dwRate - 1);

  DWORD usedbattletime = user->getUserSceneData().getUsedBattleTime();
  DWORD rebattletime = user->getUserSceneData().getReBattleTime();
  if (rebattletime > usedbattletime)
  {
    DWORD addtime = (usedbattletime + time <= rebattletime ? time : rebattletime - usedbattletime);
    if (time >= addtime)
      time -= addtime;
    user->getUserSceneData().setUsedBattleTime(addtime + usedbattletime);
  }
  if (time != 0)
  {
    DWORD oldtime = user->getUserSceneData().getBattleTime();
    user->getUserSceneData().setBattleTime(oldtime + time);
  }

  XDBG << "[Buff-多倍卡生效], 玩家:" << user->name << user->id << "添加额外战斗时间:" << time << XEND;
}

BuffChangeScale::BuffChangeScale()
{
  m_eBuffType = EBUFFTYPE_CHANGESCALE;
  m_maxCnt = 1;
}

BuffChangeScale::~BuffChangeScale()
{

}

bool BuffChangeScale::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_fChangePer = data.getMutableData("BuffEffect").getTableFloat("bodyper");
  m_fAddPer = data.getMutableData("BuffEffect").getTableFloat("addper");

  return true;
}

bool BuffChangeScale::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || !BufferState::doBuffEffect(me, uSet, bData))
    return false;
  me->setDataMark(EUSERDATATYPE_BODYSCALE);
  me->refreshDataAtonce();

  SceneUser* pUser = dynamic_cast<SceneUser*>(me);
  if (pUser != nullptr)
    pUser->getAchieve().onBody();
  return true;
}

void BuffChangeScale::ondel(xSceneEntryDynamic* me)
{
  if (!me)
    return;
  me->setDataMark(EUSERDATATYPE_BODYSCALE);
  me->refreshDataAtonce();
}

BuffChangeSkill::BuffChangeSkill()
{
  m_eBuffType = EBUFFTYPE_CHANGESKILL;
}

BuffChangeSkill::~BuffChangeSkill()
{

}

bool BuffChangeSkill::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  bool bCorrect = true;
  xLuaData& skill = data.getMutableData("BuffEffect").getMutableData("skill");
  auto getskill = [&](const string& key, xLuaData& d)
  {
    DWORD pro = d.getTableInt("1");
    if (pro <= EPROFESSION_MIN || pro >= EPROFESSION_MAX)
    {
      XERR << "[Buff-配置], id:" << id << "职业:" << pro << "不合法" << XEND;
      bCorrect = false;
      return;
    }
    DWORD skillid = d.getTableInt("2");
    m_mapPro2SkillID[pro] = skillid;
  };
  skill.foreach(getskill);

  m_dwMonsterSkill = data.getMutableData("BuffEffect").getTableInt("monster_skill");
  return bCorrect;
}


bool BuffChangeSkill::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user != nullptr)
  {
    DWORD pro = user->getProfession();
    auto it = m_mapPro2SkillID.find(pro);
    if (it == m_mapPro2SkillID.end())
      return false;

    if (user->getFighter() == nullptr)
      return false;

    user->getFighter()->getSkill().replaceNormalSkill(it->second);
  }
  else
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (bData.me);
    if (npc)
    {
      const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(m_dwMonsterSkill);
      if (pSkill == nullptr)
        return false;
      npc->replaceNormalSkill(m_dwMonsterSkill);
    }
  }

  return false;
}

void BuffChangeSkill::ondel(xSceneEntryDynamic *me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user != nullptr && user->getFighter() != nullptr)
  {
    user->getFighter()->getSkill().restoreNormalSkill();
  }
  else
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
    if (npc)
      npc->recoverNormalSkill();
  }
}

void BuffChangeSkill::onInvalid(SBufferData& bData)
{
  BufferState::onInvalid(bData);

  xSceneEntryDynamic* me = bData.me;
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user != nullptr && user->getFighter() != nullptr)
  {
    user->getFighter()->getSkill().restoreNormalSkill();
  }
  else
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (me);
    if (npc)
      npc->recoverNormalSkill();
  }
}

BuffShareDam::BuffShareDam()
{
  m_eBuffType = EBUFFTYPE_SHAREDAM;
}

BuffShareDam::~BuffShareDam()
{

}

// 一定几率损坏指定装备
BuffBreakEquip::BuffBreakEquip()
{
  m_eBuffType = EBUFFTYPE_BREAKEQUIP;
  m_maxCnt = 1;
}

BuffBreakEquip::~BuffBreakEquip()
{
}

bool BuffBreakEquip::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: type="BreakEquip",calctype=公式id,duration=持续时间
  m_dwCalcType = data.getMutableData("BuffEffect").getTableInt("calctype");
  m_dwDuration = data.getMutableData("BuffEffect").getTableInt("duration");
  return true;
}

bool BuffBreakEquip::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD cur = now(), endtime = bData.endTime / 1000;
  DWORD duration = m_dwDuration > 0 ? m_dwDuration : (cur < endtime ? endtime - cur : 0);
  if (duration <= 0)
    return false;

  xSceneEntryDynamic* src = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (src == nullptr)
    return false;

  SLuaParams sParams;
  for (auto& u : uSet)
  {
    SceneUser* pTarget = dynamic_cast<SceneUser*>(u);
    if (pTarget == nullptr)
      continue;

    for (DWORD pos = LuaManager::getMe().call<DWORD>("calcBuffValue", src, u, &sParams, m_dwCalcType); pos != 0; pos /= 100)
    {
      DWORD p = pos % 100;
      if (p <= EEQUIPPOS_MIN || p >= EEQUIPPOS_MAX || EEquipPos_IsValid(p) == false)
        continue;
      pTarget->getPackage().breakEquip(static_cast<EEquipPos>(p), duration, src);
    }
  }

  return true;
}

BuffDropExp::BuffDropExp()
{
  m_eBuffType = EBUFFTYPE_DROPEXP;
}

BuffDropExp::~BuffDropExp()
{
}

bool BuffDropExp::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
    if (BufferState::init(id, data, buffCond) == false)
    return false;

    // format: type="DropExp", rate=x
  m_sdwRate = data.getMutableData("BuffEffect").getTableInt("rate");
  if (m_sdwRate > 1000) {
    XERR << "[BuffEffect-BuffDropExp]: rate:" << m_sdwRate << "非法" << XEND;
    return false;
  }
  return true;
}

SDWORD BuffDropExp::onDropBaseExp(ESource source)
{
  if (source != ESOURCE_MONSTERKILL)
    return 0;

  return m_sdwRate;
}

BuffPackageSlot::BuffPackageSlot()
{
  m_eBuffType = EBUFFTYPE_PACKAGESLOT;
}

BuffPackageSlot::~BuffPackageSlot()
{

}

bool BuffPackageSlot::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effData = data.getMutableData("BuffEffect");
  DWORD packtype = effData.getTableInt("package");
  if (packtype <= EPACKTYPE_MIN || packtype >= EPACKTYPE_MAX)
  {
    XERR << "[Buff-配置错误], id" << m_dwID << "package:" << packtype << "类型非法" << XEND;
    return false;
  }

  m_eType = static_cast<EPackType>(packtype);
  m_dwNum = effData.getTableInt("num");

  return true;
}

bool BuffPackageSlot::onStart(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr)
    return false;

  BasePackage* pPack = user->getPackage().getPackage(m_eType);
  if (pPack == nullptr)
    return false;
  pPack->refreshSlot();

  return true;
}

void BuffPackageSlot::ondel(xSceneEntryDynamic* me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr)
    return;
  BasePackage* pPack = user->getPackage().getPackage(m_eType);
  if (pPack)
    pPack->refreshSlot();
}

BuffAffactSkill::BuffAffactSkill()
{
  m_eBuffType = EBUFFTYPE_AFFACTSKILL;
}

BuffAffactSkill::~BuffAffactSkill()
{

}

bool BuffAffactSkill::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effData = data.getMutableData("BuffEffect");
  xLuaData& skilldata = effData.getMutableData("skillID");
  auto getskill = [&](const string& key, xLuaData& d)
  {
    m_setSkillIDs.insert(d.getInt());
  };
  skilldata.foreach(getskill);

  xLuaData& attr = effData.getMutableData("attr");
  bool bCorrect = true;
  auto getattr = [&](const string& key, xLuaData& d)
  {
    DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
    if (id == 0 || EAttrType_IsValid(id) == false)
    {
      bCorrect = false;
      XERR << "[Buff-配置错误], id" << m_dwID << "属性配置错误:" << key << XEND;
      return;
    }
    EAttrType etype = (Cmd::EAttrType)id;
    m_mapSkillAttrs[etype] = d.getFloat();
  };
  attr.foreach(getattr);
  xLuaData& item = effData.getMutableData("item");
  auto getitem = [&](const string& key, xLuaData& d)
  {
    DWORD itemid = d.getTableInt("itemid");
    if (ItemConfig::getMe().getItemCFG(itemid) == nullptr)
    {
      XERR << "[Buff-配置错误], id:" << m_dwID << "itemid:" << itemid << "不合法" << XEND;
      bCorrect = false;
      return;
    }
    if (d.has("num"))
    {
      m_mapItem2Num[itemid] = d.getTableInt("num");
    }
    if (d.has("per"))
    {
      m_mapItem2Per[itemid] = d.getTableFloat("per");
    }
  };
  item.foreach(getitem);

  m_intCount = effData.getTableInt("count");
  m_fDurationPer = effData.getTableFloat("duration_per");
  m_fRange = effData.getTableFloat("range");
  m_intTargetNum = effData.getTableInt("target_count");
  m_fReady = effData.getTableFloat("readytime");
  m_intLimitCount = effData.getTableInt("limit_count");
  m_bNeedNoItem = effData.getTableInt("noitem") == 1;
  m_bAllSkill = effData.getTableInt("allskill") == 1;
  m_bLastConcertSkill = effData.getTableInt("last_concert_skill") == 1;

  return bCorrect;
}

bool BuffAffactSkill::onStart(SBufferData& bData, DWORD cur)
{
  BufferState::onStart(bData, cur);

  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->getFighter() == nullptr)
    return false;
  if(!m_bAllSkill)
  {
    for (auto &s : m_setSkillIDs)
      user->getFighter()->getSkill().markSpecSkill(s);
  }
  else
  {
    // 标记需要更新所有技能的影响
    user->getFighter()->getSkill().markSpecSkill(0);
  }

  if (m_bLastConcertSkill)
  {
    DWORD skillid = user->getFighter()->getSkill().getLastConcertSkillID();
    if (skillid)
      user->getFighter()->getSkill().markSpecSkill(skillid);
  }
  return true;
}

void BuffAffactSkill::ondel(xSceneEntryDynamic* me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr || user->getFighter() == nullptr)
    return;
  if(!m_bAllSkill)
  {
    for (auto &s : m_setSkillIDs)
      user->getFighter()->getSkill().markSpecSkill(s);
  }
  else
  {
    user->getFighter()->getSkill().markSpecSkill(0);
  }

  if (m_bLastConcertSkill)
  {
    DWORD skillid = user->getFighter()->getSkill().getLastConcertSkillID();
    if (skillid)
      user->getFighter()->getSkill().markSpecSkill(skillid);
  }
}

void BuffAffactSkill::onInvalid(SBufferData& bData)
{
  BufferState::onInvalid(bData);

  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->getFighter() == nullptr)
    return;
  if(!m_bAllSkill)
  {
    for (auto &s : m_setSkillIDs)
      user->getFighter()->getSkill().markSpecSkill(s);
  }
  else
  {
    user->getFighter()->getSkill().markSpecSkill(0);
  }

  if (m_bLastConcertSkill)
  {
    DWORD skillid = user->getFighter()->getSkill().getLastConcertSkillID();
    if (skillid)
      user->getFighter()->getSkill().markSpecSkill(skillid);
  }
}

bool BuffAffactSkill::haveSkill(xSceneEntryDynamic* me, DWORD familyid)
{
  if (m_bLastConcertSkill && familyid)
  {
    SceneUser* user = dynamic_cast<SceneUser*>(me);
    if (user == nullptr || user->getFighter() == nullptr)
      return false;
    return familyid == user->getFighter()->getSkill().getLastConcertSkillID();
  }
  return m_setSkillIDs.find(familyid) != m_setSkillIDs.end();
}

void BuffAffactSkill::getSpecSkillInfo(SSpecSkillInfo& info, DWORD layer)
{
  for (auto &m : m_mapSkillAttrs)
  {
    auto it = info.mapAttrValue.find(m.first);
    if (it == info.mapAttrValue.end())
      info.mapAttrValue[m.first] = m.second * layer;
    else
      it->second += m.second * layer;
  }

  for (auto &m : m_mapItem2Num)
  {
    auto it = info.mapItem2NumAndPer.find(m.first);
    if (it == info.mapItem2NumAndPer.end())
    {
      pair<int, float>& pa = info.mapItem2NumAndPer[m.first];
      pa.first = m.second * layer;
      pa.second = 0;
    }
    else
    {
      it->second.first += m.second * layer;
    }
  }

  for (auto &m : m_mapItem2Per)
  {
    auto it = info.mapItem2NumAndPer.find(m.first);
    if (it == info.mapItem2NumAndPer.end())
    {
      pair<int, float>& pa = info.mapItem2NumAndPer[m.first];
      pa.first = 0;
      pa.second = m.second * layer;
    }
    else
    {
      it->second.second += m.second * layer;
    }
  }

  info.fChRange += m_fRange * layer;
  info.fChDurationPer += m_fDurationPer * layer;
  info.intChCount += m_intCount * layer;
  info.intChTarCount += m_intTargetNum * layer;
  info.fChReady += m_fReady * layer;
  info.intChLimitCnt += m_intLimitCount * layer;
  info.bNeedNoItem = info.bNeedNoItem||m_bNeedNoItem;
}

BuffTradeInfo::BuffTradeInfo()
{
  m_eBuffType = EBUFFTYPE_TRADEINFO;
}

BuffTradeInfo::~BuffTradeInfo()
{

}

bool BuffTradeInfo::onStart(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr)
    return false;
  user->refreshTradeInfo();
  return true;
}

void BuffTradeInfo::ondel(xSceneEntryDynamic* me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr)
    return;
  user->refreshTradeInfo();
}

BuffBoothInfo::BuffBoothInfo()
{
  m_eBuffType = EBUFFTYPE_TRADEINFO;
}

BuffBoothInfo::~BuffBoothInfo()
{

}

bool BuffBoothInfo::onStart(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr)
    return false;
  user->refreshBoothInfo();
  return true;
}

void BuffBoothInfo::ondel(xSceneEntryDynamic* me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr)
    return;
  user->refreshBoothInfo();
}

// 额外掉落
BuffExtraDrop::BuffExtraDrop()
{
  m_eBuffType = EBUFFTYPE_EXTRADROP;
}

BuffExtraDrop::~BuffExtraDrop()
{
}

bool BuffExtraDrop::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& dataEff = data.getMutableData("BuffEffect");

  // format: type="ExtraDrop",cd=x,prob=x,limit=x,itemID=x,itemNum=x
  m_dwCd = dataEff.getTableInt("cd");
  m_dwProb = dataEff.getTableInt("prob");
  m_dwLimit = dataEff.getTableInt("limit");
  m_dwItemId = dataEff.getTableInt("itemID");
  m_dwItemNum = dataEff.getTableInt("itemNum");

  if (ItemConfig::getMe().getItemCFG(m_dwItemId) == nullptr) {
    XERR << "[BuffEffect-BuffExtraDrop] 物品item:" << m_dwItemId << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }
  if (m_dwItemNum <= 0) {
    XERR << "[BuffEffect-BuffExtraDrop] 物品item:" << m_dwItemId << "Num 为0个" << XEND;
    return false;
  }

  return true;
}

// 获得任意物品时有55%概率获得燃料*1
// 1 每30秒最多获得1个
// 2 该物品不能放入公共仓库中。可放入手推车、仓库
// 3 当该物品数量达到2000个时，无法在获得。统计范围：手推车+仓库+背包
// 4 死亡不可获得
void BuffExtraDrop::onPickUpItem(SBufferData& bData, const ItemInfo& oItem)
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(bData.me);
  if (pUser == nullptr || !pUser->isAlive())
    return;

  DWORD cur = now();

  // 检查cd时间
  if (bData.dwCommonData > cur)
    return;

  // 检查概率
  if ((DWORD)randBetween(1, 1000) > m_dwProb)
    return;

  // 检查数量限制
  DWORD total = 0;
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack != nullptr)
    total += pMainPack->getItemCount(m_dwItemId);
  TempMainPackage* pTempMainPack = dynamic_cast<TempMainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_TEMP_MAIN));
  if (pTempMainPack != nullptr)
    total += pTempMainPack->getItemCount(m_dwItemId);
  BarrowPackage* pBarrowPack = dynamic_cast<BarrowPackage*>(pUser->getPackage().getPackage(EPACKTYPE_BARROW));
  if (pBarrowPack != nullptr)
    total += pBarrowPack->getItemCount(m_dwItemId);
  StorePackage* pStorePack = dynamic_cast<StorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_STORE));
  if (pStorePack != nullptr)
    total += pStorePack->getItemCount(m_dwItemId);
  PersonalStorePackage* pPersonalStorePack = dynamic_cast<PersonalStorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_PERSONAL_STORE));
  if (pPersonalStorePack != nullptr)
    total += pPersonalStorePack->getItemCount(m_dwItemId);
  if (total >= m_dwLimit)
    return;

  // 受锁链雷锭和锁链德洛米影响
  DWORD multitime = pUser->m_oBuff.getMultiTimeRate();
  DWORD robreward = pUser->m_oBuff.getRobRewardRate();
  DWORD dropCnt = multitime > robreward ? multitime : robreward; // 两者不可能同时生效

  // 掉落
  for (DWORD i = 0; i <= dropCnt; ++i) {
    if (total >= m_dwLimit)
      break;

    // 锁链德洛米从第二次掉落开始需要重新计算概率
    if (i != 0 && robreward > 0 && (DWORD)randBetween(1, 1000) > m_dwProb)
      continue;

  ItemInfo item;
  item.set_id(m_dwItemId);
  item.set_count(m_dwItemNum);
  item.set_source(ESOURCE_BUFF);
  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(m_dwItemId);
  bool bShow = pItemCfg && ItemConfig::getMe().isCard(pItemCfg->eItemType);
  //pUser->getPackage().addItemAvailable(item, bShow, true);
  pUser->getPackage().addItem(item, EPACKMETHOD_AVAILABLE, bShow, true);

    ++total;

  XLOG << "[Buff-额外掉落]: 玩家:" << pUser->name << pUser->id << "item:" << m_dwItemId << "num:" << m_dwItemNum << XEND;
}

  // 更新cd时间
  bData.dwCommonData = cur + m_dwCd;
}

BuffMarkHeal::BuffMarkHeal()
{
  m_eBuffType = EBUFFTYPE_MARKHEAL;
}

BuffMarkHeal::~BuffMarkHeal()
{

}

bool BuffMarkHeal::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwSkillFamilyID = data.getMutableData("BuffEffect").getTableInt("skillid");
  return true;
}

void BuffMarkHeal::ondel(xSceneEntryDynamic* me)
{
  if (me == nullptr)
    return;
  me->clearExtraSkillTarget(m_dwSkillFamilyID);
}

BuffSpeedUp::BuffSpeedUp()
{
  m_eBuffType = EBUFFTYPE_SPEEDUP;
}

BuffSpeedUp::~BuffSpeedUp()
{

}

bool BuffSpeedUp::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& dataEff = data.getMutableData("BuffEffect");
  m_dwBuffID = dataEff.getTableInt("buffid");
  m_intMs = dataEff.getTableFloat("time") * ONE_THOUSAND;

  return true;
}

bool BuffSpeedUp::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  for (auto &s : uSet)
  {
    s->m_oBuff.changeBuffSpeed(m_dwBuffID, m_intMs);
  }

  return true;
}

BuffTrigTrap::BuffTrigTrap()
{
  m_eBuffType = EBUFFTYPE_TRIGTRAP;
}

BuffTrigTrap::~BuffTrigTrap()
{

}

void BuffTrigTrap::ondel(xSceneEntryDynamic* me)
{
  if (me == nullptr)
    return;
  me->m_oSkillProcessor.setTrapHoldOn(false);
  me->m_oSkillProcessor.triggerAllTrap();
}

BuffReplaceSkill::BuffReplaceSkill()
{
  m_eBuffType = EBUFFTYPE_REPLACESKILL;
}

BuffReplaceSkill::~BuffReplaceSkill()
{

}

bool BuffReplaceSkill::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwOldFamilyID = data.getMutableData("BuffEffect").getTableInt("oldid");
  m_dwNewFamilyID = data.getMutableData("BuffEffect").getTableInt("newid");

  return true;
}

bool BuffReplaceSkill::onStart(SBufferData& bData, DWORD cur)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || user->getFighter() == nullptr)
    return false;
  user->getFighter()->getSkill().setReplaceSkill(m_dwOldFamilyID, m_dwNewFamilyID);
  if (user->getTransform().isInTransform())
    user->getTransform().addReplaceSkill(m_dwOldFamilyID, m_dwNewFamilyID);
  return true;
}

void BuffReplaceSkill::ondel(xSceneEntryDynamic* me)
{
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr || user->getFighter() == nullptr)
    return;
  user->getFighter()->getSkill().delReplaceSkill(m_dwOldFamilyID, m_dwNewFamilyID);
  if (user->getTransform().isInTransform())
    user->getTransform().delReplaceSkill(m_dwOldFamilyID, m_dwNewFamilyID);
}

BuffAttrLimit::BuffAttrLimit()
{
  m_maxCnt = 1;
  m_eBuffType = EBUFFTYPE_ATTRLIMIT;
}

BuffAttrLimit::~BuffAttrLimit()
{

}

bool BuffAttrLimit::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  bool bCorrect = true;
  DWORD type = 1;
  auto getLimitAttr = [&](const string& key, xLuaData& d)
  {
    DWORD id = RoleDataConfig::getMe().getIDByName(key.c_str());
    if (Cmd::EAttrType_IsValid(id) == false)
    {
      XERR << "[Buff配置错误], id:" << m_dwID << "属性名不合法:" << key.c_str() << XEND;
      bCorrect = false;
      return;
    }
    EAttrType etype =  (Cmd::EAttrType)id;
    float value = d.getFloat();
    if (type == 1)
      m_mapLimitMaxValue[etype] = value;
    else if (type == 2)
      m_mapLimitMinValue[etype] = value;
  };
  xLuaData& maxattr = data.getMutableData("BuffEffect").getMutableData("MaxAttr");
  xLuaData& minattr = data.getMutableData("BuffEffect").getMutableData("MinAttr");
  type = 1;
  maxattr.foreach(getLimitAttr);
  type = 2;
  minattr.foreach(getLimitAttr);
  return bCorrect;
}

bool BuffAttrLimit::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;
  if (bData.me == nullptr)
    return false;
  bData.me->setCollectMark(ECOLLECTTYPE_NONE);
  bData.me->refreshDataAtonce();
  return true;
}

void BuffAttrLimit::ondel(xSceneEntryDynamic *me)
{
  if (!me) return;
  auto pAttr = me->getAttribute();
  if (pAttr)
  {
    for (auto &m : m_mapLimitMaxValue)
      pAttr->setMark(m.first);
    for (auto &m : m_mapLimitMinValue)
      pAttr->setMark(m.first);
  }
  me->setCollectMark(ECOLLECTTYPE_NONE);
  me->refreshDataAtonce();
}

BuffAutoBlock::BuffAutoBlock()
{
  m_eBuffType = EBUFFTYPE_AUTOBLOCK;
}

BuffAutoBlock::~BuffAutoBlock()
{
}

bool BuffAutoBlock::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: stiff=x
  m_dwStiff = data.getMutableData("BuffEffect").getTableFloat("stiff") * 1000;
  return true;
}

BuffGM::BuffGM()
{
  m_eBuffType = EBUFFTYPE_GM;
}

BuffGM::~BuffGM()
{
}

bool BuffGM::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: cmd={gm命令},source=1
  if (data.getMutableData("BuffEffect").has("cmd") == false)
    return false;
  m_oParam = data.getMutableData("BuffEffect").getMutableData("cmd");
  m_bSource = data.getMutableData("BuffEffect").getTableInt("source") == 1;
  return true;
}

bool BuffGM::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  if (m_bSource)
  {
    xSceneEntryDynamic *src = xSceneEntryDynamic::getEntryByID(bData.fromID);
    if (src != nullptr)
      GMCommandRuler::getMe().execute(src, m_oParam);
  }
  else
  {
    GMCommandRuler::getMe().execute(me, m_oParam);
  }
  return true;
}

BuffLimitSkill::BuffLimitSkill()
{
  m_eBuffType = EBUFFTYPE_LIMITSKILL;
}

BuffLimitSkill::~BuffLimitSkill()
{

}

bool BuffLimitSkill::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  auto getskill = [&](const string& k, xLuaData& d)
  {
    m_setSkillIDs.insert(d.getInt());
  };
  data.getMutableData("BuffEffect").getMutableData("id").foreach(getskill);
  if (data.getMutableData("BuffEffect").has("IgnoreTarget"))
    m_dwIgnoreTarget = data.getMutableData("BuffEffect").getTableInt("IgnoreTarget");
  auto getnotskill = [&](const string& k, xLuaData& d)
  {
    m_setNotSkillIDs.insert(d.getInt());
  };
  data.getMutableData("BuffEffect").getMutableData("notid").foreach(getnotskill);
  return true;
}

BuffSkillCD::BuffSkillCD()
{
  m_eBuffType = EBUFFTYPE_SKILLCD;
}

BuffSkillCD::~BuffSkillCD()
{

}

bool BuffSkillCD::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  auto getcd = [&](const string& key, xLuaData& d)
  {
    DWORD skill = d.getTableInt("id");
    if (d.getTableInt("realid") != 1)
      skill = skill * ONE_THOUSAND;
    FmData& cd = m_mapSkill2CDMs[skill];
    cd.load(d.getMutableData("time"));
  };
  data.getMutableData("BuffEffect").getMutableData("cd").foreach(getcd);
  return true;
}

bool BuffSkillCD::onStart(SBufferData& bData, DWORD cur)
{
  if (bData.me == nullptr)
    return false;
  for (auto &m : m_mapSkill2CDMs)
  {
    DWORD cd = m.second.getFmValue(bData.me, &bData) * ONE_THOUSAND;
    bData.me->m_oCDTime.add(m.first, cd, CD_TYPE_SKILL);
  }

  return true;
}

BuffLimitUseItem::BuffLimitUseItem()
{
  m_eBuffType = EBUFFTYPE_LIMITUSEITEM;
}

BuffLimitUseItem::~BuffLimitUseItem()
{

}

bool BuffLimitUseItem::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  DWORD type = 1;
  auto getid = [&](const string& key, xLuaData& d)
  {
    if (type == 1)
      m_setOkTypes.insert(d.getInt());
    else if (type == 2)
      m_setForbidTypes.insert(d.getInt());
  };
  type = 1;
  data.getMutableData("BuffEffect").getMutableData("ok_type").foreach(getid);
  type = 2;
  data.getMutableData("BuffEffect").getMutableData("forbid_type").foreach(getid);
  m_bForbidAll = data.getMutableData("BuffEffect").getTableInt("forbid_all") == 1;

  return true;
}

bool BuffLimitUseItem::canUseItem(DWORD itemtype) const
{
  if (m_bForbidAll)
    return false;
  if (m_setOkTypes.empty() == false)
    return m_setOkTypes.find(itemtype) != m_setOkTypes.end();
  if (m_setForbidTypes.empty() == false)
    return m_setForbidTypes.find(itemtype) == m_setOkTypes.end();

  return true;
}

BuffTransfer::BuffTransfer()
{
  m_eBuffType = EBUFFTYPE_TRANSFERBUFF;
}

BuffTransfer::~BuffTransfer()
{

}

bool BuffTransfer::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_dwBuffID = effdata.getTableInt("id");
  m_dwMaxLayer = effdata.getTableInt("maxlayer");
  return true;
}

bool BuffTransfer::onStart(SBufferData& bData, DWORD cur)
{
  if (bData.me == nullptr)
    return false;
  xSceneEntryDynamic* pFrom = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (pFrom == nullptr || pFrom == bData.me)
    return false;

  DWORD nowlayer = pFrom->m_oBuff.getLayerByID(m_dwBuffID);
  DWORD translayer = m_dwMaxLayer > nowlayer ? nowlayer : m_dwMaxLayer;
  if (translayer == 0)
    return false;
  pFrom->m_oBuff.delLayer(m_dwBuffID, translayer);
  for (DWORD i = 0; i < translayer; ++i)
    bData.me->m_oBuff.add(m_dwBuffID, pFrom);

  return true;
}

/************************************************************************/
/* BuffCookerKnife                                                                     */
/************************************************************************/

// 额外掉落
BuffCookerKnife::BuffCookerKnife()
{
  m_eBuffType = EBUFFTYPE_COOKER_KINFE;
}

BuffCookerKnife::~BuffCookerKnife()
{
}
bool BuffCookerKnife::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& dataEff = data.getMutableData("BuffEffect");

  // format: type="ExtraDrop",cd=x,prob=x,limit=x,itemID=x,itemNum=x
  m_dwCd = dataEff.getTableInt("cd");
  m_dwProb = dataEff.getTableInt("prob");
  m_dwLimit = dataEff.getTableInt("limit");
  m_dwItemId = dataEff.getTableInt("itemID");
  m_dwItemNum = dataEff.getTableInt("itemNum");

  if (ItemConfig::getMe().getItemCFG(m_dwItemId) == nullptr) {
    XERR << "[BuffEffect-BuffCookerKnife] 物品item:" << m_dwItemId << "未在 Table_Item.txt 表中找到" << XEND;
    return false;
  }
  if (m_dwItemNum <= 0) {
    XERR << "[BuffEffect-BuffCookerKnife] 物品item:" << m_dwItemId << "Num 为0个" << XEND;
    return false;
  }

  return true;
}

// 获得任意物品时有5%概率获得食材*1
// 1 每30秒最多获得1个
// 2 该物品不能放入公共仓库中。可放入仓库
// 3 当该物品数量达到2000个时，无法在获得。统计范围：仓库+背包
// 4 死亡不可获得
void BuffCookerKnife::onPickUpItem(SBufferData& bData, const ItemInfo& oItem)
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(bData.me);
  if (pUser == nullptr || !pUser->isAlive())
    return;

  DWORD cur = now();

  // 检查cd时间
  if (bData.dwCommonData > cur)
    return;

  // 检查概率
  if ((DWORD)randBetween(1, 1000) > m_dwProb)
    return;

  // 检查数量限制
  DWORD total = 0;
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack != nullptr)
    total += pMainPack->getItemCount(m_dwItemId);
  TempMainPackage* pTempMainPack = dynamic_cast<TempMainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_TEMP_MAIN));
  if (pTempMainPack != nullptr)
    total += pTempMainPack->getItemCount(m_dwItemId);
  BarrowPackage* pBarrowPack = dynamic_cast<BarrowPackage*>(pUser->getPackage().getPackage(EPACKTYPE_BARROW));
  if (pBarrowPack != nullptr)
    total += pBarrowPack->getItemCount(m_dwItemId);
  StorePackage* pStorePack = dynamic_cast<StorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_STORE));
  if (pStorePack != nullptr)
    total += pStorePack->getItemCount(m_dwItemId);
  PersonalStorePackage* pPersonalStorePack = dynamic_cast<PersonalStorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_PERSONAL_STORE));
  if (pPersonalStorePack != nullptr)
    total += pPersonalStorePack->getItemCount(m_dwItemId);
  if (total >= m_dwLimit)
    return;

  // 受锁链雷锭和锁链德洛米影响
  DWORD multitime = pUser->m_oBuff.getMultiTimeRate();
  DWORD robreward = pUser->m_oBuff.getRobRewardRate();
  DWORD dropCnt = multitime > robreward ? multitime : robreward; // 两者不可能同时生效

                                                                 // 掉落
  for (DWORD i = 0; i <= dropCnt; ++i) {
    if (total >= m_dwLimit)
      break;

    // 锁链德洛米从第二次掉落开始需要重新计算概率
    if (i != 0 && robreward > 0 && (DWORD)randBetween(1, 1000) > m_dwProb)
      continue;

    ItemInfo item;
    item.set_id(m_dwItemId);
    item.set_count(m_dwItemNum);
    const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(m_dwItemId);
    bool bShow = pItemCfg && ItemConfig::getMe().isCard(pItemCfg->eItemType);
    pUser->getPackage().addItem(item, EPACKMETHOD_AVAILABLE, bShow, true);
    ++total;

    XLOG << "[料理-食材获得]: 玩家:" << pUser->name << pUser->id << "item:" << m_dwItemId << "num:" << m_dwItemNum << XEND;
  }

  // 更新cd时间
  bData.dwCommonData = cur + m_dwCd;
}

BuffUndead::BuffUndead()
{
  m_eBuffType = EBUFFTYPE_UNDEAD;
}

BuffUndead::~BuffUndead()
{

}

bool BuffUndead::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwProtectBuff = data.getMutableData("BuffEffect").getTableInt("protect_buff");
  return true;
}

// 免疫异常状态一定次数
BuffResistStatus::BuffResistStatus()
{
  m_eBuffType = EBUFFTYPE_RESISTSTATUS;
}

BuffResistStatus::~BuffResistStatus()
{

}

// 杀死怪物掉落食材奖励
BuffDropFoodReward::BuffDropFoodReward()
{
  m_eBuffType = EBUFFTYPE_DROPFOODREWARD;
}

BuffDropFoodReward::~BuffDropFoodReward()
{

}

bool BuffDropFoodReward::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwIntervalTime = data.getMutableData("BuffEffect").getTableInt("interval");
  return true;
}

void BuffDropFoodReward::onKillMonster(SceneUser* user, SceneNpc* npc, SBufferData& bData)
{
  if (user == nullptr || !user->getScene() || npc == nullptr || npc->getScene() == nullptr)
    return;
  const SNpcCFG* pCFG = npc->getCFG();
  if (pCFG == nullptr)
    return;

  if (bData.layers == 0 || pCFG->setFoodRewards.empty() == true) return;

  DWORD curSec = now();
  if(curSec > bData.dwCommonData)
    bData.dwCommonData = curSec + m_dwIntervalTime;
  else
    return;

  bData.layers--;

  for(auto &s : pCFG->setFoodRewards)
  {
    user->getPackage().rollReward(s, EPACKMETHOD_AVAILABLE, false, true);
    XLOG << "[Buff - 杀怪食材奖励掉落], 玩家:" << user->name << user->id << "buff id=" << m_dwID << "reward=" << s << "layers=" << bData.layers << XEND;
  }
}

BuffDelSpEffect::BuffDelSpEffect()
{
  m_eBuffType = EBUFFTYPE_DELSPEFFECT;
}

BuffDelSpEffect::~BuffDelSpEffect()
{
}

bool BuffDelSpEffect::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: speffectid={x,x,x},ignoresource=1
  auto f = [&](const string& ket, xLuaData& data)
  {
    m_setSpIds.insert(data.getInt());
  };
  data.getMutableData("BuffEffect").getMutableData("speffectid").foreach(f);
  m_bIgnoreSource = data.getMutableData("BuffEffect").getTableInt("ignoresource") == 1;
  return true;
}

bool BuffDelSpEffect::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  QWORD ignoreid = 0;
  if (m_bIgnoreSource)
    ignoreid = bData.fromID;
  for (auto& id : m_setSpIds)
  {
    me->getSpEffect().del(id, ignoreid);
  }
  return true;
}

BuffForceStatus::BuffForceStatus()
{
  m_eBuffType = EBUFFTYPE_FORCESTATUS;
}

BuffForceStatus::~BuffForceStatus()
{
}

bool BuffForceStatus::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  data.getMutableData("BuffEffect").getMutableData("status").getIDList(m_setStatus);
  return true;
}

BuffMultiExp::BuffMultiExp()
{
  m_eBuffType = EBUFFTYPE_MULTIEXP;
}

BuffMultiExp::~BuffMultiExp()
{
}

bool BuffMultiExp::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_fRate = data.getMutableData("BuffEffect").getTableFloat("rate");
  return true;
}

void BuffMultiExp::onGetExp(SBufferData& bData, SceneNpc* npc, DWORD baseExp, DWORD jobExp)
{
  SceneUser* user = dynamic_cast<SceneUser*> (bData.me);
  if (user == nullptr || npc == nullptr || npc->getCFG() == nullptr)
    return;
  if (user->isInBattleTimeMap() == false)
    return;
  if (!npc->getCFG()->isFieldMonster())
    return;

  if (m_fRate < 1)
    return;
  float extra = m_fRate - 1;
  if (baseExp)
  {
    user->addBaseExp(baseExp * extra, ESOURCE_MONSTERKILL);
    user->getUserPet().addBaseExp((baseExp * extra));
    user->getUserBeing().addBaseExp(baseExp * extra);
    XLOG << "[Buff-经验卡生效], 玩家" << user->name << user->id << "获得额外经验:" << baseExp * extra << "BuffID:" << m_dwID << XEND;
  }
  if (jobExp)
  {
    user->addJobExp(jobExp * extra, ESOURCE_MONSTERKILL);
    XLOG << "[Buff-经验卡生效], 玩家" << user->name << user->id << "获得额外job经验:" << jobExp * extra << "BuffID:" << m_dwID << XEND;
  }
}

BuffGvgReward::BuffGvgReward()
{
  m_eBuffType = EBUFFTYPE_GVGREWARD;
}

BuffGvgReward::~BuffGvgReward()
{

}

bool BuffGvgReward::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  /*
  if (bData.dwCommonData == 0)
    bData.dwCommonData = GuildCityManager::getMe().getFireStopTime();

  // 1s 检测一次
  QWORD curm = xTime::getCurMSec();
  bData.timeTick = curm + ONE_THOUSAND;

  DWORD cur = curm / ONE_THOUSAND;
  if (cur < bData.dwCommonData)
    return true;
  SceneUser* user = dynamic_cast<SceneUser*>(me);
  if (user == nullptr)
    return false;

  me->m_oBuff.del(m_dwID);

  const SGuildFireCFG& cfg = MiscConfig::getMe().getGuildFireCFG();
  DWORD mailid = cfg.dwPartInMailID;
  XLOG << "[公会战奖励Buff], 玩家:" << user->name << user->id << "获取参与奖励, 邮件ID:" << mailid << XEND;
  if (mailid == 0)
    return true;
  MailManager::getMe().sendMail(me->id, mailid);
  */

  return true;
}

BuffPickUp::BuffPickUp()
{
  m_eBuffType = EBUFFTYPE_PICKUP;
}

BuffPickUp::~BuffPickUp()
{

}

bool BuffPickUp::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_fRange = effdata.getTableFloat("range");
  m_dwItemID = effdata.getTableInt("itemid");
  return true;
}

bool BuffPickUp::doBuffEffect(xSceneEntryDynamic *me, const TSetSceneEntrys& uSet, SBufferData& bData) 
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user == nullptr)
    return false;

  Scene* pScene = user->getScene();
  if (pScene == nullptr)
    return false;
  xSceneEntrySet itemset;
  pScene->getEntryListInBlock(SCENE_ENTRY_ITEM, user->getPos(), m_fRange, itemset);

  if (uSet.empty())
    return true;

  for (auto &s : itemset)
  {
    SceneItem* pItem = dynamic_cast<SceneItem*> (s);
    if (pItem == nullptr || pItem->getItemInfo().id() != m_dwItemID)
      continue;
    if (pItem->canPickup(user, EITEMPICKUP_BUFF) == false)
      continue;
    ItemInfo item = pItem->getItemInfo();
    ItemInfo item2;
    item2.CopyFrom(item);
    user->getPackage().addItem(item2, EPACKMETHOD_AVAILABLE, false, true);
    XLOG << "[Buff-Pickup], 成功捡到道具, 玩家:" << user->name << user->id << "道具:" << item.id() << "数量:" << item.count() << XEND;

    PickupItem cmd;
    cmd.set_success(true);
    cmd.set_playerguid(user->id);
    cmd.set_itemguid(pItem->id);

    PROTOBUF(cmd, send, len);
    user->sendCmdToNine(send, len);
    //user->setPickupTime();
    pItem->leaveScene();
  }

  return true;
}


BuffImmuneAttack::BuffImmuneAttack()
{
  m_eBuffType = EBUFFTYPE_IMMUNE;
}

BuffImmuneAttack::~BuffImmuneAttack()
{

}

BuffDelSkillStatus::BuffDelSkillStatus()
{
  m_eBuffType = EBUFFTYPE_DELSKILLSTATUS;
}

BuffDelSkillStatus::~BuffDelSkillStatus()
{

}

bool BuffDelSkillStatus::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  DWORD status = data.getMutableData("BuffEffect").getTableInt("status");
  m_eStatus = static_cast<ESkillStatus> (status);
  return true;
}

bool BuffDelSkillStatus::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (me == nullptr || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  me->m_oSkillStatus.delStatus(m_eStatus);

  return true;
}

// 深度隐身
BuffDeepHide::BuffDeepHide()
{
  m_eBuffType = EBUFFTYPE_DEEPHIDE;
}

BuffDeepHide::~BuffDeepHide()
{

}

// 装备脱卸
BuffOffEquip::BuffOffEquip()
{
  m_eBuffType = EBUFFTYPE_OFFEQUIP;
  m_maxCnt = 1;
}

BuffOffEquip::~BuffOffEquip()
{
}

bool BuffOffEquip::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: type="OffEquip",calctype=公式id
  m_dwCalcType = data.getMutableData("BuffEffect").getTableInt("calctype");
  return true;
}

bool BuffOffEquip::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD cur = now(), endtime = bData.endTime / 1000;
  DWORD duration = cur < endtime ? endtime - cur : 0;
  if (duration <= 0)
    return false;

  xSceneEntryDynamic* src = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (src == nullptr)
    return false;

  SLuaParams sParams;
  for (auto& u : uSet)
  {
    SceneUser* pTarget = dynamic_cast<SceneUser*>(u);
    if (pTarget == nullptr)
      continue;

    for (DWORD pos = LuaManager::getMe().call<DWORD>("calcBuffValue", src, u, &sParams, m_dwCalcType); pos != 0; pos /= 100)
    {
      DWORD p = pos % 100;
      if (p <= EEQUIPPOS_MIN || p >= EEQUIPPOS_MAX || EEquipPos_IsValid(p) == false)
        continue;
      pTarget->getPackage().forceOffEquip(static_cast<EEquipPos>(p), duration, src);
    }
  }

  return true;
}

// 装备保护
BuffProtectEquip::BuffProtectEquip()
{
  m_eBuffType = EBUFFTYPE_PROTECTEQUIP;
  m_maxCnt = 1;
}

BuffProtectEquip::~BuffProtectEquip()
{
}

bool BuffProtectEquip::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: type="ProtectEqiup",pos={装备位置,...}
  auto f = [&](const string& key, xLuaData& data)
  {
    DWORD pos = data.getInt();
    if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false)
      return;
    m_setPos.insert(static_cast<EEquipPos>(pos));
  };
  data.getMutableData("BuffEffect").getMutableData("pos").foreach(f);
  m_bAlways = data.getMutableData("BuffEffect").getTableInt("always") == 1;
  return true;
}

bool BuffProtectEquip::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD duration = 0;
  if (m_bAlways == false)
  {
    DWORD cur = now(), endtime = bData.endTime / 1000;
    duration = cur < endtime ? endtime - cur : 0;
    if (duration <= 0)
      return false;
  }

  for (auto& u : uSet)
  {
    SceneUser* pTarget = dynamic_cast<SceneUser*>(u);
    if (pTarget == nullptr)
      continue;
    for (auto pos : m_setPos)
      pTarget->getPackage().protectEquip(pos, duration, m_bAlways);
  }

  return true;
}

void BuffProtectEquip::ondel(xSceneEntryDynamic *me)
{
  if (m_bAlways == false)
    return;
  if (me == nullptr)
    return;
  SceneUser* user = dynamic_cast<SceneUser*>(me);
  if (user == nullptr)
    return;
  for (auto pos : m_setPos)
    user->getPackage().cancelAlwaysProtectEquip(pos);
}

// 装备修理
BuffFixEquip::BuffFixEquip()
{
  m_eBuffType = EBUFFTYPE_FIXEQUIP;
}

BuffFixEquip::~BuffFixEquip()
{
}

bool BuffFixEquip::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: type="FixEquip",pos={装备位置,...}
  auto f = [&](const string& key, xLuaData& data)
  {
    DWORD pos = data.getInt();
    if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false)
      return;
    m_setPos.insert(static_cast<EEquipPos>(pos));
  };
  data.getMutableData("BuffEffect").getMutableData("pos").foreach(f);
  return true;
}

bool BuffFixEquip::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  for (auto& u : uSet)
  {
    SceneUser* pTarget = dynamic_cast<SceneUser*>(u);
    if (pTarget == nullptr)
      continue;
    for (auto pos : m_setPos)
      pTarget->getPackage().fixBrokenEquip(pos);
  }

  return true;
}


BuffSkillTarget::BuffSkillTarget()
{
  m_eBuffType = EBUFFTYPE_SKILLTARGET;
}

BuffSkillTarget::~BuffSkillTarget()
{

}

bool BuffSkillTarget::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwSkillID = data.getMutableData("BuffEffect").getTableInt("skillid");
  return true;
}

QWORD BuffSkillTarget::getTarget(SBufferData& data, DWORD skillid, const TSetQWORD& nowTargets)
{
  if (skillid / ONE_THOUSAND != m_dwSkillID)
    return 0;
  switch(m_eTargetType)
  {
    case EBUFFSKILLTARGET_TEAMHPMIN:
      {
        SceneUser* user = dynamic_cast<SceneUser*> (data.me);
        if (user == nullptr)
          return 0;

        if (user->getTeamID() == 0)
          return 0;

        std::set<SceneUser*> userset = user->getTeamSceneUser();
        DWORD minhp = DWORD_MAX;
        QWORD tarid = 0;
        for (auto &s : userset)
        {
          if (nowTargets.find(s->id) != nowTargets.end())
            continue;
          if (s->check2PosInNine(user) == false || s->isAlive() == false)
            continue;
          DWORD hpper = s->getAttr(EATTRTYPE_HP) * 100 / s->getAttr(EATTRTYPE_MAXHP);
          if (hpper < minhp)
          {
            tarid = s->id;
            minhp = hpper;
          }
        }
        return tarid;
      }
  }

  return 0;
}

BuffExtraReward::BuffExtraReward()
{
  m_eBuffType = EBUFFTYPE_EXTRAREWARD;
}

BuffExtraReward::~BuffExtraReward()
{

}

bool BuffExtraReward::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  DWORD d = effdata.getTableInt("event_type");
  m_eType = static_cast<EExtraRewardType>(d);
  if (effdata.has("zeny_ratio"))
    m_fZenyRatio = effdata.getTableFloat("zeny_ratio");
  if (effdata.has("exp_ratio"))
    m_fExpRatio = effdata.getTableFloat("exp_ratio");

  return true;
}

BuffScenery::BuffScenery()
{
  m_eBuffType = EBUFFTYPE_SCENERY;
}

BuffScenery::~BuffScenery()
{
}

bool BuffScenery::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwSceneryID = data.getMutableData("BuffEffect").getTableInt("scenic");
  return true;
}

BuffHPStorage::BuffHPStorage()
{
  m_eBuffType = EBUFFTYPE_HPSTORAGE;
}

BuffHPStorage::~BuffHPStorage()
{
}

bool BuffHPStorage::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwHPTriggerPercent = data.getMutableData("BuffEffect").getTableInt("percent");
  m_dwHPAdd = data.getMutableData("BuffEffect").getTableInt("addvalue");
  return true;
}

bool BuffHPStorage::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if(BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD addValue = 0;
  if (bData.layers >= m_dwHPAdd)
    addValue = m_dwHPAdd;
  else
    addValue = bData.layers;

  bData.layers = bData.layers - addValue;
  me->changeHp(addValue, me);
  me->sendBuffDamage(-addValue);
  me->m_oBuff.addUpdate(bData);

#ifdef _DEBUG
  XLOG << "[HP储备库], 玩家:" << me->name << me->id << "buff id=" << bData.id << "addHP=" << addValue << "总储存值=" << bData.layers << XEND;
#endif

  return true;
}

bool BuffHPStorage::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  return true;
}

BuffSPStorage::BuffSPStorage()
{
  m_eBuffType = EBUFFTYPE_SPSTORAGE;
}

BuffSPStorage::~BuffSPStorage()
{
}

bool BuffSPStorage::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwSPTriggerPercent = data.getMutableData("BuffEffect").getTableInt("percent");
  m_dwSPAdd = data.getMutableData("BuffEffect").getTableInt("addvalue");
  return true;
}

bool BuffSPStorage::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if(BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  DWORD addValue = 0;
  if (bData.layers >= m_dwSPAdd)
    addValue = m_dwSPAdd;
  else
    addValue = bData.layers;

  bData.layers = bData.layers - addValue;
  SQWORD oldsp = me->getAttr(EATTRTYPE_SP);
  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if(pUser != nullptr)
    pUser->setSp(oldsp + addValue);
  else
  {
    XLOG << "[SP储备库], 非玩家,无法添加:" << me->name << me->id << "buff id=" << bData.id << "addHP=" << addValue << "总储存值=" << bData.layers << XEND;
    BufferState::onInvalid(bData);
    return false;
  }
  me->m_oBuff.addUpdate(bData);

#ifdef _DEBUG
  XLOG << "[SP储备库], 玩家:" << me->name << me->id << "buff id=" << bData.id << "addHP=" << addValue << "总储存值=" << bData.layers << XEND;
#endif

  return true;
}

bool BuffSPStorage::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  return true;
}

BuffPetAdventure::BuffPetAdventure()
{
  m_eBuffType = EBUFFTYPE_PETADVENTURE;
}

BuffPetAdventure::~BuffPetAdventure()
{

}

bool BuffPetAdventure::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  xLuaData& effdata = data.getMutableData("BuffEffect");
  if (effdata.has("Card"))
    m_fCard = effdata.getTableFloat("Card");
  if (effdata.has("Base"))
    m_fBase = effdata.getTableFloat("Base");
  if (effdata.has("Job"))
    m_fJob = effdata.getTableFloat("Job");
  if (effdata.has("NotCostBall"))
    m_dwNotCostBall = effdata.getTableInt("NotCostBall");
  if (effdata.has("Kind"))
    m_fKind = effdata.getTableFloat("Kind");
  if (effdata.has("ConsumeTime"))
    m_fConsumeTime = effdata.getTableFloat("ConsumeTime");
  if (effdata.has("FightTime"))
    m_fFightTime = effdata.getTableFloat("FightTime");
  if (effdata.has("RareBox"))
    m_dwRareBox =  effdata.getTableInt("RareBox");
  m_mapMaterial.clear();
  /*auto f = [&](const string& key, xLuaData& data)
  {
    if (!data.has("qualify"))
      return;
    DWORD qualify = data.getTableInt("qualify");
    if (qualify <= EQUALITYTYPE_MIN || qualify >= EQUALITYTYPE_MAX)
      return;
    //if (!data.has("ratio"))
    //  return;
    float ratio = data.getTableFloat("ratio");
    //if (ratio < 0.00001f )
    //  return;
    DWORD num = data.getTableInt("num");
    m_mapMaterial.insert({qualify, {ratio, num}});
  };*/

  xLuaData& material = data.getMutableData("BuffEffect").getMutableData("Material");
  DWORD qualify = material.getTableInt("qualify");
  float ratio = data.getTableFloat("ratio");
  DWORD num = material.getTableInt("num");
  m_mapMaterial.insert({qualify, {ratio, num}});
  return true;
}

BuffStatusRebound::BuffStatusRebound()
{
  m_eBuffType = EBUFFTYPE_STATUSREBOUND;
}

BuffStatusRebound::~BuffStatusRebound()
{

}

bool BuffStatusRebound::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  auto f = [&](const string& key, xLuaData& data)
  {
    EBuffStatusType status = (EBuffStatusType)data.getInt();
    if (status <= EBUFFSTATUS_MIN || status >= EBUFFSTATUS_MAX)
      return;
    m_setStatus.insert(status);
  };
  data.getMutableData("BuffEffect").getMutableData("status").foreach(f);
  m_dwOdds = data.getMutableData("BuffEffect").getTableInt("Odds");
  return true;
}

bool BuffStatusRebound::canRebound(EBuffStatusType status)
{
   
  if (m_setStatus.empty())
    return true;
  if (m_setStatus.find(status) == m_setStatus.end())
    return false;
  return true;
}

BuffWeaponBlock::BuffWeaponBlock()
{
  m_eBuffType = EBUFFTYPE_WEAPONBLOCK;
}

BuffWeaponBlock::~BuffWeaponBlock()
{

}

bool BuffWeaponBlock::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  // format: stiff=x
  m_dwStiff = data.getMutableData("BuffEffect").getTableFloat("stiff") * 1000;
  return true;
}

BuffAttrControl::BuffAttrControl()
{
  m_eBuffType = EBUFFTYPE_ATTRCONTROL;
}

BuffAttrControl::~BuffAttrControl()
{

}

bool BuffAttrControl::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  auto attrf = [&](const string& key, xLuaData& data)
  {
    DWORD attr = RoleDataConfig::getMe().getIDByName(key.c_str());
    if (attr <= EATTRTYPE_MIN || attr >= EATTRTYPE_MAX || EAttrType_IsValid(attr) == false)
      return;
    auto it = m_mapAttr.find(static_cast<EAttrType>(attr));
    if (it == m_mapAttr.end())
      m_mapAttr[static_cast<EAttrType>(attr)] = data.getInt();
    else
      it->second |= data.getInt();
  };
  data.getMutableData("BuffEffect").getMutableData("attr").foreach(attrf);
  return true;
}

BuffRideWolf::BuffRideWolf()
{
  m_eBuffType = EBUFFTYPE_RIDEWOLF;
}

BuffRideWolf::~BuffRideWolf()
{

}

bool BuffRideWolf::onStart(SBufferData& bData, DWORD cur)
{
  if (!BufferState::onStart(bData, cur))
    return false;
  SceneUser* pUser = dynamic_cast<SceneUser*>(bData.me);
  if(!pUser)
    return true;
  pUser->setDataMark(EUSERDATATYPE_PET_PARTNER);
  pUser->refreshDataAtonce();
  return true;
}

void BuffRideWolf::ondelLater(xSceneEntryDynamic *me)
{
  SceneUser* pUser = dynamic_cast<SceneUser*>(me);
  if(!pUser)
    return;
  pUser->setDataMark(EUSERDATATYPE_PET_PARTNER);
  pUser->refreshDataAtonce();
  return;
}

// 禁用装备位
BuffForbidEquip::BuffForbidEquip()
{
  m_eBuffType = EBUFFTYPE_FORBIDEQUIP;
  m_maxCnt = 1;
}

BuffForbidEquip::~BuffForbidEquip()
{
}

bool BuffForbidEquip::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  auto packf = [&](const string& k, xLuaData& d)
  {
    DWORD pack = atoi(k.c_str());
    if (pack <= EPACKTYPE_MIN || pack >= EPACKTYPE_MAX || EPackType_IsValid(pack) == false)
      return;

    int type = 0;
    auto posf = [&](const string& key, xLuaData& data)
    {
      DWORD pos = data.getInt();
      if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false)
        return;
      if (type == 1)
        m_mapPack2ForbidOnPos[static_cast<EPackType>(pack)].insert(static_cast<EEquipPos>(pos));
      else if (type == 2)
        m_mapPack2ForbidOffPos[static_cast<EPackType>(pack)].insert(static_cast<EEquipPos>(pos));
      else if (type == 3)
        m_mapPack2OffPos[static_cast<EPackType>(pack)].insert(static_cast<EEquipPos>(pos));
    };
    type = 1;
    d.getMutableData("forbid_on_pos").foreach(posf);
    type = 2;
    d.getMutableData("forbid_off_pos").foreach(posf);
    type = 3;
    d.getMutableData("off_pos").foreach(posf);
  };
  data.getMutableData("BuffEffect").foreach(packf);
  return true;
}

bool BuffForbidEquip::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  if (!m_mapPack2OffPos.empty())
  {
    for (auto& u : uSet)
    {
      SceneUser* pTarget = dynamic_cast<SceneUser*>(u);
      if (pTarget == nullptr)
        continue;
      for (auto& v : m_mapPack2OffPos)
      {
        if (v.first == EPACKTYPE_EQUIP)
        {
          for (auto pos : v.second)
            pTarget->getPackage().equipOff(pos);
        }
        else if (v.first == EPACKTYPE_FASHIONEQUIP)
        {
          FashionEquipPackage* pack = dynamic_cast<FashionEquipPackage*>(pTarget->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
          if (pack == nullptr)
            continue;
          for (auto pos : v.second)
          {
            ItemEquip* equip = pack->getEquip(pos);
            if (equip)
              pTarget->getPackage().equip(EEQUIPOPER_OFFFASHION, pos, equip->getGUID());
          }
        }
      }
    }
  }

  return true;
}

bool BuffForbidEquip::isEquipForbidOn(EPackType pack, EEquipPos pos) const
{
  auto it = m_mapPack2ForbidOnPos.find(pack);
  if (it == m_mapPack2ForbidOnPos.end())
    return false;
  return it->second.find(pos) != it->second.end();
}

bool BuffForbidEquip::isEquipForbidOff(EPackType pack, EEquipPos pos) const
{
  auto it = m_mapPack2ForbidOffPos.find(pack);
  if (it == m_mapPack2ForbidOffPos.end())
    return false;
  return it->second.find(pos) != it->second.end();
}

//交换SP
BuffSPExchange::BuffSPExchange()
{
  m_eBuffType = EBUFFTYPE_SPEXCHANGE;
  m_maxCnt = 1;
}

BuffSPExchange::~BuffSPExchange()
{

}

bool BuffSPExchange::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_dwSPRecoverPercent = data.getMutableData("BuffEffect").getTableInt("recoverPercent");
  return true;
}

bool BuffSPExchange::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;
  
  SceneUser* buffTarget = dynamic_cast<SceneUser*> (me);
  if(buffTarget == nullptr) 
    return false;
  SceneUser* buffFrom = dynamic_cast<SceneUser*> (xSceneEntryDynamic::getEntryByID(bData.fromID));
  if(buffFrom == nullptr) 
    return false;

  float targetSP = buffTarget->getAttr(EATTRTYPE_SP);
  float targetMAXSP = buffTarget->getAttr(EATTRTYPE_MAXSP);
  float fromSP = buffFrom->getAttr(EATTRTYPE_SP);
  float fromMAXSP = buffFrom->getAttr(EATTRTYPE_MAXSP);
  if(!fromMAXSP || !targetMAXSP) 
    return false;
  buffTarget->setSp(targetMAXSP * fromSP / fromMAXSP);
  //buffFrom->setSp((1 + m_dwSPRecoverPercent / 100) * fromMAXSP * targetSP / targetMAXSP);
  buffFrom->setSp(fromMAXSP * targetSP / targetMAXSP * m_dwSPRecoverPercent / 100.0);
  return true;
}

// 概率解锁食谱
BuffUnlockRecipe::BuffUnlockRecipe()
{
  m_eBuffType = EBUFFTYPE_UNLOCKRECIPE;
  m_maxCnt = 1;
}

BuffUnlockRecipe::~BuffUnlockRecipe()
{
}

bool BuffUnlockRecipe::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effdata = data.getMutableData("BuffEffect");
  m_dwRecipeID = effdata.getTableInt("recipe");
  m_dwProbability = effdata.getTableInt("probability");
  return true;
}

bool BuffUnlockRecipe::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  bool isOpen = ActivityManager::getMe().isOpen(GACTIVITY_FOOD_RECIPE);
  if(isOpen == false)
    return false;

  SceneUser* pUser = dynamic_cast<SceneUser*> (me);
  if(pUser == nullptr)
    return false;

  bool bRankOK = m_dwProbability >= (DWORD)randBetween(1,100);
  if(bRankOK == true)
  {
    const SRecipeCFG* pRecipeCfg = RecipeConfig::getMe().getRecipeCFG(m_dwRecipeID);
    if (pRecipeCfg == nullptr)
      return false;

    if(pUser->getSceneFood().unlockRecipe(m_dwRecipeID) == true)
      MsgManager::sendMsg(pUser->id, 25805, MsgParams(pRecipeCfg->strName));
  }
  return true;
}

BuffAttrTransfer::BuffAttrTransfer()
{
  m_eBuffType = EBUFFTYPE_ATTRTRANSFER;
}

BuffAttrTransfer::~BuffAttrTransfer()
{

}

bool BuffAttrTransfer::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_maxCnt = 1;
  return true;
}

bool BuffAttrTransfer::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (!me || BufferState::doBuffEffect(me, uSet, bData) == false || bData.fromID == me->id)
    return false;
  xSceneEntryDynamic* src = xSceneEntryDynamic::getEntryByID(bData.fromID);
  if (src == nullptr)
    return false;

  if (me->hasTransferAttr(getID(), false))
  {
    me->delTransferAttr(getID(), false, true);
  }
  if (src->hasTransferAttr(getID(), true))
  {
    src->delTransferAttr(getID(), true, true);
  }

  TVecAttrSvrs getMyAttr;
  if (BufferState::getMyAttr(me, &bData, getMyAttr) == false)
    return false;
  for (auto& attr : getMyAttr)
  {
    float v = src->getAttr(attr.type());
    if (v < attr.value())
      attr.set_value(v);
  }

  me->setTransferAttr(getID(), src->id, getMyAttr, false);
  for (auto& attr : getMyAttr)
    attr.set_value(-attr.value());
  src->setTransferAttr(getID(), me->id, getMyAttr, true);

  return true;
}

void BuffAttrTransfer::ondel(xSceneEntryDynamic *me)
{
  if (me == nullptr)
    return;
  BufferState::ondel(me);

  me->delTransferAttr(getID(), false, false);
}



BuffDHide::BuffDHide()
{
  m_eBuffType = EBUFFTYPE_DHIDE;
  m_maxCnt = 1;
}

BuffDHide::~BuffDHide()
{

}

bool BuffDHide::onStart(SBufferData& bData, DWORD cur)
{
  if (BufferState::onStart(bData, cur) == false)
    return false;

  xSceneEntryDynamic* me = bData.me;
  if (!me)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user && user->getScene())
  {
    xSceneEntrySet userset;
    user->getScene()->getEntryListInNine(SCENE_ENTRY_USER, me->getPos(), userset);
    for (auto &s : userset)
    {
      SceneUser* p = dynamic_cast<SceneUser*>(s);
      if (p && p->getAttr(EATTRTYPE_HIDE) && p->isMyEnemy(user))
        p->sendMeToUser(user);
    }

    xSceneEntrySet npcset;
    user->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, me->getPos(), npcset);
    for (auto &s : npcset)
    {
      SceneNpc* p = dynamic_cast<SceneNpc*> (s);
      if (!p || !p->getAttr(EATTRTYPE_HIDE))
        continue;
      SceneUser* master = p->getMasterUser();
      if (!master)
        continue;
      if (master->isMyEnemy(user))
        p->sendMeToUser(user);
    }
  }

  return true;
}

void BuffDHide::ondel(xSceneEntryDynamic *me)
{
  if (!me)
    return;
  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (user && user->getScene())
  {
    xSceneEntrySet userset;
    user->getScene()->getEntryListInNine(SCENE_ENTRY_USER, me->getPos(), userset);
    for (auto &s : userset)
    {
      SceneUser* p = dynamic_cast<SceneUser*>(s);
      if (p && p->getAttr(EATTRTYPE_HIDE) && p->isMyEnemy(user))
        p->delMeToUser(user);
    }

    xSceneEntrySet npcset;
    user->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, me->getPos(), npcset);
    for (auto &s : npcset)
    {
      SceneNpc* p = dynamic_cast<SceneNpc*> (s);
      if (!p || !p->getAttr(EATTRTYPE_HIDE))
        continue;
      SceneUser* master = p->getMasterUser();
      if (!master)
        continue;
      if (master->isMyEnemy(user))
        p->delMeToUser(user);
    }
  }
}

BuffCheckAddLine::BuffCheckAddLine()
{
  m_eBuffType = EBUFFTYPE_CHECKADDLINE;
}

BuffCheckAddLine::~BuffCheckAddLine()
{

}

bool BuffCheckAddLine::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;

  xLuaData& effData = data.getMutableData("BuffEffect");
  m_dwLineID = effData.getTableInt("line");
  m_fRange = effData.getTableFloat("range");
  m_setAddBuffs.clear();
  effData.getMutableData("buffs").getIDList(m_setAddBuffs);

  return true;
}

bool BuffCheckAddLine::doBuffEffect(xSceneEntryDynamic* me, const TSetSceneEntrys& uSet, SBufferData& bData)
{
  if (BufferState::doBuffEffect(me, uSet, bData) == false)
    return false;

  SceneUser* user = dynamic_cast<SceneUser*> (me);
  if (!user)
    return false;

  if (user->getSpEffect().hasLine(m_dwLineID) || user->getSpEffect().beLineByOther(m_dwLineID))
  {
    for (auto &s : m_setAddBuffs)
      user->m_oBuff.add(s);
    return true;
  }

  std::set<SceneUser*> mems = user->getTeamSceneUser();
  mems.erase(user);

  float minRange = 1000;
  SceneUser* minMem = nullptr;
  for (auto &s : mems)
  {
    float dis = getXZDistance(user->getPos(), s->getPos());
    if (dis < minRange)
    {
      minRange = dis;
      minMem = s;
    }
  }

  if (minRange <= m_fRange && minMem)
  {
    for (auto &s : m_setAddBuffs)
    {
      user->m_oBuff.add(s);
      minMem->m_oBuff.add(s);
    }
    TSetQWORD tars;
    tars.insert(minMem->id);
    user->getSpEffect().add(m_dwLineID, tars, true);
  }

  return true;
}

void BuffCheckAddLine::ondel(xSceneEntryDynamic *me)
{
  if (!me)
    return;
  if (me->getSpEffect().hasLine(m_dwLineID))
    me->getSpEffect().del(m_dwLineID, 0);
}

BuffNoMapExit::BuffNoMapExit()
{
  m_eBuffType = EBUFFTYPE_NOMAPEXIT;
}

BuffNoMapExit::~BuffNoMapExit()
{

}

BuffInfect::BuffInfect()
{
  m_eBuffType = EBUFFTYPE_INFECT;
}

BuffInfect::~BuffInfect()
{
}

bool BuffInfect::init(DWORD id, xLuaData& data, TPtrBuffCond buffCond)
{
  if (BufferState::init(id, data, buffCond) == false)
    return false;
  m_fRange = data.getMutableData("BuffEffect").getTableFloat("range");
  return true;
}

void BuffInfect::ondel(xSceneEntryDynamic *me)
{
  if (!me || me->getScene() == nullptr)
    return;

  TVecSBuffData vecBuffData;
  me->m_oBuff.getVecBuffDataByBuffType(EBUFFTYPE_STATUS, vecBuffData);
  if (vecBuffData.empty())
    return;

  xSceneEntrySet userSet;
  me->getScene()->getEntryListInBlock(SCENE_ENTRY_USER, me->getPos(), m_fRange, userSet);
  for (auto &pEntry: userSet)
  {
    if (pEntry == me)
      continue;
    SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
    if (!pUser)
      continue;
    for (auto &bData: vecBuffData)
    {
      xSceneEntryDynamic* pFromE = xSceneEntryDynamic::getEntryByID(bData.fromID);
      pUser->m_oBuff.add(bData.id, pFromE, bData.lv, bData.dwDamage, bData.endTime, true);
    }
  }
}

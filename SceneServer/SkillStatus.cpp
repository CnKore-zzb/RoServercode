#include "SkillStatus.h"
#include "xSceneEntryDynamic.h"
#include "SkillItem.h"
#include "SkillManager.h"

SkillStatus::SkillStatus(xSceneEntryDynamic* pEntry) : m_pEntry(pEntry)
{

}

SkillStatus::~SkillStatus()
{

}

bool SkillStatus::enterStatus(ESkillStatus eStatus, const TSetSceneEntrys& setOther, const BaseSkill* pSkill)
{
  if (pSkill == nullptr)
    return false;
  for (auto pOther : setOther)
    if (pOther == nullptr)
      return false;

  QWORD cur = xTime::getCurMSec();
  switch(eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
      for (auto pOther : setOther)
        if (inStatus(cur) || pOther->m_oSkillStatus.inStatus(cur))
          return false;
      break;
    case ESKILLSTATUS_CATCH:
    case ESKILLSTATUS_CURSEDCIRCLE:
      if (inStatus(cur))
        end();
      for (auto pOther : setOther)
        if (pOther->m_oSkillStatus.inStatus(cur))
          pOther->m_oSkillStatus.delStatus(eStatus);
      break;
    case ESKILLSTATUS_MIN:
    default:
      break;
  }

  switch(eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_CATCH:
    case ESKILLSTATUS_CURSEDCIRCLE:
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
      {
        if (m_pEntry->isAlive() == false)
          return false;
        for (auto pOther : setOther)
        {
          if (m_pEntry == pOther || pOther->isAlive() == false)
            return false;
        }
        QWORD time = pSkill->getLuaParam().getData("SkillStatus").getTableFloat("time") * 1000;
        m_qwStatusEndTime = cur + time;
        m_dwRelateSkillID = pSkill->getSkillID();
        m_bMaster = true;
        m_eStatus = eStatus;
        for (auto pOther : setOther)
        {
          m_setOtherID.insert(pOther->id);
          pOther->m_oSkillStatus.setEnemyStatus(eStatus, m_pEntry, m_qwStatusEndTime, m_dwRelateSkillID);
        }
      }
      break;
    case ESKILLSTATUS_MIN:
    default:
      break;
  }

  if (eStatus == ESKILLSTATUS_SOLO || eStatus == ESKILLSTATUS_ENSEMBLE)
    onConcertStatusChange();

  return true;
}


bool SkillStatus::setEnemyStatus(ESkillStatus eStatus, xSceneEntryDynamic* pMaster, QWORD endTime, DWORD skillid)
{
  if (pMaster == nullptr)
    return false;
  m_eStatus = eStatus;
  m_qwStatusEndTime = endTime;
  m_setOtherID.insert(pMaster->id);
  m_dwRelateSkillID = skillid;

  return true;
}

void SkillStatus::onLeaveScene()
{
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch(m_eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_CATCH:
    case ESKILLSTATUS_CURSEDCIRCLE:
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
      end();
      break;
    case ESKILLSTATUS_MIN:
    default:
      break;
  }
}

void SkillStatus::onDie()
{
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch(m_eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_CATCH:
    case ESKILLSTATUS_CURSEDCIRCLE:
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
      end();
      break;
    case ESKILLSTATUS_MIN:
    default:
      break;
  }
}

void SkillStatus::onUseSkill()
{
  if (m_eStatus == ESKILLSTATUS_MIN)
    return;
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch(m_eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
      //end();
      break;
    case ESKILLSTATUS_CURSEDCIRCLE:
      end();
      break;
    case ESKILLSTATUS_MIN:
    default:
      break;
  }
}

void SkillStatus::onAttack(xSceneEntryDynamic* target)
{
  if (m_eStatus == ESKILLSTATUS_MIN)
    return;
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch(m_eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_CURSEDCIRCLE:
      if (m_bMaster)
        end();
      break;
    case ESKILLSTATUS_CATCH:
      // 擒拿状态仅在使用某些技能时删除, 不在此处理
      break;
    case ESKILLSTATUS_SOLO:
    case ESKILLSTATUS_ENSEMBLE:
    case ESKILLSTATUS_MIN:
    default:
      break;
  }
}

void SkillStatus::onTeamChange(const GTeam& oldTeam, const GTeam& nowTeam)
{
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch (m_eStatus)
  {
  case ESKILLSTATUS_ENSEMBLE:
    if (nowTeam.getTeamID() == 0 || oldTeam.getTeamID() != nowTeam.getTeamID())
      end();
    break;
  default:
    break;
  }
}

void SkillStatus::onSkillReset()
{
  QWORD cur = xTime::getCurMSec();
  if (inStatus(cur) == false)
    return;

  switch (m_eStatus)
  {
  case ESKILLSTATUS_SOLO:
  case ESKILLSTATUS_ENSEMBLE:
    end();
    break;
  default:
    break;
  }
}

void SkillStatus::end()
{
  switch(m_eStatus)
  {
    case ESKILLSTATUS_SWORDBREAK:
    case ESKILLSTATUS_CATCH:
    case ESKILLSTATUS_CURSEDCIRCLE:
      {
        QWORD cur = xTime::getCurMSec();
        if (inStatus(cur) == false)
          return;

        const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(m_dwRelateSkillID);
        if (pSkill == nullptr)
          return;
        for (auto qwOtherID : m_setOtherID)
        {
          xSceneEntryDynamic* pOther = xSceneEntryDynamic::getEntryByID(qwOtherID);
          if (pOther == nullptr)
            return;
          const TSetDWORD& selfBuffs = pSkill->getSelfBuffs();
          const TSetDWORD& enemyBuffs = pSkill->getEnemyBuffs();
          DWORD buffid = pSkill->getLuaParam().getData("SkillStatus").getTableInt("limitSkillBuff");
          if (m_bMaster)
          {
            for (auto &s : selfBuffs)
              m_pEntry->m_oBuff.del(s);
            m_pEntry->m_oBuff.del(buffid);

            for (auto &s : enemyBuffs)
              pOther->m_oBuff.del(s);
          }
          else
          {
            for (auto &s : selfBuffs)
              pOther->m_oBuff.del(s);
            pOther->m_oBuff.del(buffid);

            for (auto &s : enemyBuffs)
              m_pEntry->m_oBuff.del(s);
          }
          clearStatus();
          pOther->m_oSkillStatus.clearStatus();
        }
      }
      break;
    case ESKILLSTATUS_SOLO:
    {
      clearStatus();
      const SSkillTypeCFG* typecfg = MiscConfig::getMe().getSkillCFG().getSkillTypeCFG(ESKILLTYPE_SOLO);
      if (typecfg)
        for (auto buffid : typecfg->setBuff)
          m_pEntry->m_oBuff.del(buffid);
      m_pEntry->m_oSkillProcessor.endSkill(m_dwRelateSkillID);
      m_pEntry->setGMAttr(EATTRTYPE_SOLO, 0);
      m_pEntry->refreshDataAtonce();
      onConcertStatusChange();
      break;
    }
    case ESKILLSTATUS_ENSEMBLE:
    {
      clearStatus();
      const SSkillTypeCFG* typecfg = MiscConfig::getMe().getSkillCFG().getSkillTypeCFG(ESKILLTYPE_ENSEMBLE);
      if (typecfg)
        for (auto buffid : typecfg->setBuff)
          m_pEntry->m_oBuff.del(buffid);
      for (auto qwOtherID : m_setOtherID)
      {
        xSceneEntryDynamic* pOther = xSceneEntryDynamic::getEntryByID(qwOtherID);
        if (pOther == nullptr)
          continue;
        pOther->m_oSkillStatus.clearStatus();
        if (typecfg)
          for (auto buffid : typecfg->setBuff)
            pOther->m_oBuff.del(buffid);
        pOther->m_oSkillProcessor.endSkill(m_dwRelateSkillID);
        pOther->setGMAttr(EATTRTYPE_ENSEMBLE, 0);
        pOther->refreshDataAtonce();
      }
      m_pEntry->m_oSkillProcessor.endSkill(m_dwRelateSkillID);
      m_pEntry->setGMAttr(EATTRTYPE_ENSEMBLE, 0);
      m_pEntry->refreshDataAtonce();
      onConcertStatusChange();
      break;
    }
    case ESKILLSTATUS_MIN:
    default:
      break;
  }
}

void SkillStatus::delStatus(ESkillStatus eStatus)
{
  if (m_eStatus != eStatus)
    return;
  switch(m_eStatus)
  {
  case ESKILLSTATUS_SOLO:
  case ESKILLSTATUS_ENSEMBLE:
    break;
  default:
    if (inStatus(xTime::getCurMSec()) == false)
      return;
    break;
  }
  end();
}

void SkillStatus::onConcertStatusChange()
{
  m_pEntry->m_oBuff.onSkillStatusChange();
  for (auto qwOtherID : m_setOtherID)
  {
    xSceneEntryDynamic* pOther = xSceneEntryDynamic::getEntryByID(qwOtherID);
    if (pOther == nullptr)
      continue;
    pOther->m_oBuff.onSkillStatusChange();
  }
}

#include "ActionItem.h"
#include "SceneUser.h"
#include "BufferManager.h"

// base action
BaseAction::BaseAction(SceneUser* pUser) : m_pUser(pUser)
{
  m_dwStartTime = xTime::getCurSec();
}

BaseAction::~BaseAction()
{

}

bool BaseAction::init(const SActionCFG* pCFG)
{
  if (pCFG == nullptr)
    return false;

  m_dwID = pCFG->dwID;
  m_dwTime = pCFG->oData.getTableInt("time");
  return true;
}

EActionStatus BaseAction::doAction(DWORD curSec)
{
  if (curSec - m_dwStartTime > m_dwTime)
    return EACTIONSTATUS_OVER;
  return EACTIONSTATUS_RUN;
}

// effect action
EffectAction::EffectAction(SceneUser* pUser) : BaseAction(pUser)
{

}

EffectAction::~EffectAction()
{

}

bool EffectAction::init(const SActionCFG* pCFG)
{
  if (BaseAction::init(pCFG) == false)
    return false;

  m_dwEffectID = pCFG->oData.getTableInt("id");
  if (TableManager::getMe().getEffectCFG(m_dwEffectID) == nullptr)
  {
    XERR << "[玩家-行为步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "创建EffectAction失败, id :" << m_dwEffectID << "未在 Table_Effect.txt 表中找到" << XEND;
    return false;
  }

  return true;
}

EActionStatus EffectAction::doAction(DWORD curSec)
{
  if (!m_bSet)
  {
    m_pUser->effect(m_dwEffectID);
    m_bSet = true;
  }
  return BaseAction::doAction(curSec);
}

// exp action
ExpAction::ExpAction(SceneUser* pUser) : BaseAction(pUser)
{

}

ExpAction::~ExpAction()
{

}

bool ExpAction::init(const SActionCFG* pCFG)
{
  if (BaseAction::init(pCFG) == false)
    return false;

  const string& str = pCFG->oData.getTableString("method");
  if (str == "add_base")
    m_eMethod = EEXPMETHOD_ADD_BASE;
  else if (str == "add_job")
    m_eMethod = EEXPMETHOD_ADD_JOB;
  else
  {
    XERR << "[玩家-行为步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "创建ExpAction失败, method :" << str << "不合法" << XEND;
    return false;
  }

  m_strScript = pCFG->oData.getTableString("script");
  return true;
}

EActionStatus ExpAction::doAction(DWORD curSec)
{
  if (!m_bSet)
  {
    switch (m_eMethod)
    {
      case EEXPMETHOD_ADD_BASE:
        {
          ItemData oData;
          oData.mutable_base()->set_id(ITEM_BASEEXP);
          oData.mutable_base()->set_count(LuaManager::getMe().call<DWORD>(m_strScript.c_str(), m_pUser->getLevel()));
          m_pUser->getPackage().addItem(oData);
        }
        break;
      case EEXPMETHOD_ADD_JOB:
        {
          SceneFighter* pFighter = m_pUser->getFighter();
          if (pFighter != nullptr)
          {
            ItemData oData;
            oData.mutable_base()->set_id(ITEM_JOBEXP);
            oData.mutable_base()->set_count(LuaManager::getMe().call<DWORD>(m_strScript.c_str(), pFighter->getJobLv()));
            m_pUser->getPackage().addItem(oData);
          }
        }
        break;
      default:
        break;
    }
    m_bSet = true;
  }
  return BaseAction::doAction(curSec);
}

// sys action
SysMsgAction::SysMsgAction(SceneUser* pUser) : BaseAction(pUser)
{

}

SysMsgAction::~SysMsgAction()
{

}

bool SysMsgAction::init(const SActionCFG* pCFG)
{
  if (BaseAction::init(pCFG) == false)
    return false;
  m_dwMsgID = pCFG->oData.getTableInt("id");
  return true;
}

EActionStatus SysMsgAction::doAction(DWORD curSec)
{
  if (!m_bSet)
  {
    MsgManager::sendMsg(m_pUser->id, m_dwMsgID, m_oParam);
    m_bSet = true;
  }

  return BaseAction::doAction(curSec);
}

// buff action
BuffAction::BuffAction(SceneUser* pUser) : BaseAction(pUser)
{

}

BuffAction::~BuffAction()
{

}

bool BuffAction::init(const SActionCFG* pCFG)
{
  if (BaseAction::init(pCFG) == false)
    return false;

  m_dwBuffID = pCFG->oData.getTableInt("id");
  if (BufferManager::getMe().getBuffById(m_dwBuffID) == nullptr)
  {
    XERR << "[玩家-行为步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "创建BuffAction失败, id :" << m_dwBuffID << "未在 Table_Buffer.txt 表中找到" << XEND;
    return false;
  }

  const string& str = pCFG->oData.getTableString("method");
  if (str == "add")
    m_eMethod = EBUFFMETHOD_ADD;
  else
  {
    XERR << "[玩家-行为步骤]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "创建BuffAction失败, method :" << str << "不合法" << XEND;
    return false;
  }

  return true;
}

EActionStatus BuffAction::doAction(DWORD curSec)
{
  if (!m_bSet)
  {
    if (m_eMethod == EBUFFMETHOD_ADD)
      m_pUser->m_oBuff.add(m_dwBuffID);
    m_bSet = true;
  }
  return BaseAction::doAction(curSec);
}

// equip attr action
EquipAttrAction::EquipAttrAction(SceneUser* pUser) : BaseAction(pUser)
{

}

EquipAttrAction::~EquipAttrAction()
{

}

bool EquipAttrAction::init(const SActionCFG* pCFG)
{
  if (BaseAction::init(pCFG) == false)
    return false;

  m_strGUID = pCFG->oData.getTableString("guid");

  return true;
}

EActionStatus EquipAttrAction::doAction(DWORD curSec)
{
  if (BaseAction::doAction(curSec) == EACTIONSTATUS_OVER)
  {
    EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    if (pEquipPack)
    {
      ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getItem(m_strGUID));
      if (pEquip && pEquip->isBroken() == false)
      {
        m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
        m_pUser->refreshDataAtonce();

        m_pUser->m_oBuff.addEquipBuffBreakInvalid(pEquip);
      }
    }
    return EACTIONSTATUS_OVER;
  }
  return EACTIONSTATUS_RUN;
}


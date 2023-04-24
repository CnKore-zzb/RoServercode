#include "Action.h"
#include "SceneUser.h"

Action::Action(SceneUser* pUser) : m_pUser(pUser)
{

}

Action::~Action()
{
  for (auto m = m_mapAction.begin(); m != m_mapAction.end(); ++m)
  {
    for (auto l = m->second.begin(); l != m->second.end(); ++l)
      SAFE_DELETE(*l);
  }
  m_mapAction.clear();
}

bool Action::addAction(DWORD dwID, DWORD dwActionID)
{
  const SActionCFG* pCFG = ActionConfig::getMe().getActionCFG(dwActionID);
  if (pCFG == nullptr)
  {
    XERR << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << dwID << "添加" << dwActionID << "失败,未在 Table_Action.txt 表中找到" << XEND;
    return false;
  }
  return addAction(dwID, *pCFG);
}

bool Action::addAction(DWORD dwID, const SActionCFG& rCFG)
{
  BaseAction* pAction = nullptr;
  switch (rCFG.type)
  {
    case EACTIONTYPE_MIN:
    case EACTIONTYPE_MAX:
      break;
    case EACTIONTYPE_EFFECT:
      pAction = NEW EffectAction(m_pUser);
      break;
    case EACTIONTYPE_EXP:
      pAction = NEW ExpAction(m_pUser);
      break;
    case EACTIONTYPE_SYSMSG:
      pAction = NEW SysMsgAction(m_pUser);
      break;
    case EACTIONTYPE_BUFF:
      pAction = NEW BuffAction(m_pUser);
      break;
    case EACTIONTYPE_EQUIPATTR:
      pAction = NEW EquipAttrAction(m_pUser);
      break;
  }
  if (pAction == nullptr)
  {
    XERR << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << dwID << "添加" << rCFG.dwID << "失败,创建失败" << XEND;
    return false;
  }
  if (pAction->init(&rCFG) == false)
  {
    SAFE_DELETE(pAction);
    XERR << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << dwID << "添加" << rCFG.dwID << "失败,初始化失败" << XEND;
    return false;
  }

  m_mapAction[dwID].push_back(pAction);
  XDBG << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "在" << dwID << "添加" << rCFG.dwID << "成功" << XEND;
  return true;
}

BaseAction* Action::getActionItem(DWORD dwID, DWORD dwActionID)
{
  auto m = m_mapAction.find(dwID);
  if (m == m_mapAction.end())
    return nullptr;
  auto l = find_if(m->second.begin(), m->second.end(), [&](const BaseAction* p) -> bool{
    if (p == nullptr)
      return false;
    return p->id() == dwActionID;
  });
  if (l == m->second.end())
    return nullptr;
  return *l;
}

void Action::timer(DWORD curSec)
{
  for (auto m = m_mapAction.begin(); m != m_mapAction.end();)
  {
    for (auto l = m->second.begin(); l != m->second.end();)
    {
      if ((*l)->doAction(curSec) != EACTIONSTATUS_OVER)
      {
        ++l;
        break;
      }

      XDBG << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "列表" << m->first << "行为" << (*l)->getType() << "执行完毕" << XEND;
      BaseAction* pAction = *l;
      l = m->second.erase(l);
      SAFE_DELETE(pAction);
    }

    if (m->second.empty() == false)
    {
      ++m;
      continue;
    }

    XDBG << "[玩家-行为]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "列表" << m->first << "中行为全部执行完毕" << XEND;
    m = m_mapAction.erase(m);
  }
}


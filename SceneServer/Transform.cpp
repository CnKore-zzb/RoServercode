#include "Transform.h"
#include "SceneUser.h"
#include "MiscConfig.h"
#include "SceneSkill.pb.h"
#include "DScene.h"

Transform::Transform(SceneUser* pUser)
{
  m_pUser = pUser;
}

Transform::~Transform()
{

}

void Transform::enterTransform(DWORD monsterid, ETransformType etype)
{
  m_bInTransform = true;
  m_dwMonsterID = monsterid;
  m_eType = etype;
  switch(etype)
  {
    case ETRANSFORM_NORMAL:
      break;
    case ETRANSFROM_POLIFIRE:
      {
        m_listSkillItems.clear();
        Scene* pScene = m_pUser->getScene();
        if (pScene->isPollyScene())
        {
          PollyScene* pyScene = dynamic_cast<PollyScene*> (pScene);
          if (pyScene && pyScene->canPickUp(m_pUser->id) == false)
          {
            sendSkillInfo();
            break;
          }
        }
        DWORD defaultskill = MiscConfig::getMe().getPoliFireCFG().dwDefaultSkillID;
        addSkill(defaultskill);
      }
      break;
    case ETRANSFORM_ALTMAN:
      {
        m_vecSkillItems.clear();
        Scene* pScene = m_pUser->getScene();
        if (pScene)
        {
          AltmanScene* pyScene = dynamic_cast<AltmanScene*> (pScene);
          if (pyScene)
          {
            DWORD defaultskill = MiscConfig::getMe().getAltmanCFG().dwDefaultSkillID;
            addSkill(defaultskill);
            break;
          }
        }
      }
      break;
  }
}

bool Transform::addSkill(DWORD skillid)
{
  if (m_eType == ETRANSFROM_POLIFIRE)
  {
    DWORD size = m_listSkillItems.size();
    if (size >= MiscConfig::getMe().getPoliFireCFG().dwMaxSkillPos)
    {
      XLOG << "[波利乱斗], 玩家:" << m_pUser->name << m_pUser->id << "添加技能失败, 技能格子已满, 技能id:" << skillid << XEND;
      return false;
    }
    DWORD familyid = skillid / ONE_THOUSAND;
    auto it = m_mapReplaceSkill.find(familyid);
    if (it != m_mapReplaceSkill.end())
      skillid = it->second * ONE_THOUSAND + skillid % ONE_THOUSAND;

    DWORD pos = 0;
    for (auto &s : m_listSkillItems)
    {
      if (s.dwID == skillid)
        pos = s.dwPos + 1;
    }
    if (pos)
    {
      for (auto &s : m_listSkillItems)
      {
        if (s.dwPos >= pos)
          s.dwPos += 1;
      }
    }
    else
    {
      pos = size + 1;
    }

    SSkillItem item;
    item.dwID = skillid;
    item.dwPos = pos;
    m_listSkillItems.push_back(item);

    sendSkillInfo();
    return true;
  }
  else if (m_eType == ETRANSFORM_ALTMAN)
  {
    DWORD size = m_vecSkillItems.size();
    if (size >= MiscConfig::getMe().getAltmanCFG().dwMaxSkillPos)
    {
      XLOG << "[奥特曼], 玩家:" << m_pUser->name << m_pUser->id << "添加技能失败, 技能格子已满, 技能id:" << skillid << XEND;
      return false;
    }

    SSkillItem item;
    item.dwID = skillid;
    m_vecSkillItems.insert(m_vecSkillItems.begin(), item);

    sendSkillInfo();
    return true;
  }

  return false;
}

void Transform::delSkill(DWORD skillid)
{
  if (m_eType == ETRANSFROM_POLIFIRE)
  {
    DWORD pos = 0;
    for (auto s = m_listSkillItems.begin(); s != m_listSkillItems.end(); )
    {
      if (skillid == s->dwID)
      {
        pos = s->dwPos;
        s = m_listSkillItems.erase(s);
        break;
      }
      ++s;
    }

    if (pos == 0)
      return;

    for (auto &s : m_listSkillItems)
    {
      if (s.dwPos >= pos)
        s.dwPos --;
    }

    sendSkillInfo();
  }
  else if (m_eType == ETRANSFORM_ALTMAN)
  {
    for (auto it = m_vecSkillItems.begin(); it != m_vecSkillItems.end();)
    {
      if(it->dwID == skillid)
      {
        m_vecSkillItems.erase(it);
        sendSkillInfo();
        return;
      }

      ++it;
    }
  }
}

void Transform::sendSkillInfo()
{
  if (m_eType == ETRANSFROM_POLIFIRE)
  {
    DynamicSkillCmd cmd;
    // 倒序显示
    DWORD size = m_listSkillItems.size();
    for (auto &s : m_listSkillItems)
    {
      SkillItem* pItem = cmd.add_skills();
      if (pItem == nullptr)
        continue;
      pItem->set_id(s.dwID);
      pItem->set_pos(size + 1 - s.dwPos);
    }
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  else if (m_eType == ETRANSFORM_ALTMAN)
  {
    DynamicSkillCmd cmd;
    for(DWORD i = 0; i < m_vecSkillItems.size(); ++i)
    {
      SkillItem* pItem = cmd.add_skills();
      if (pItem == nullptr)
        continue;
      pItem->set_id(m_vecSkillItems[i].dwID);
      pItem->set_pos(i + 1);
    }
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Transform::addReplaceSkill(DWORD oldid, DWORD newid)
{
  if (m_eType != ETRANSFROM_POLIFIRE)
    return;
  auto it = m_mapReplaceSkill.find(oldid);
  if (it != m_mapReplaceSkill.end())
    return;

  m_mapReplaceSkill[oldid] = newid;
  bool update = false;
  for (auto &s : m_listSkillItems)
  {
    if (s.dwID / ONE_THOUSAND == oldid)
    {
      s.dwID = newid * ONE_THOUSAND + oldid % ONE_THOUSAND;
      update = true;
    }
  }
  if (update)
    sendSkillInfo();
}

void Transform::delReplaceSkill(DWORD oldid, DWORD newid)
{
  if (m_eType != ETRANSFROM_POLIFIRE)
    return;
  auto it = m_mapReplaceSkill.find(oldid);
  if (it == m_mapReplaceSkill.end())
    return;

  m_mapReplaceSkill.erase(it);
  bool update = false;
  for (auto &s : m_listSkillItems)
  {
    if (s.dwID / ONE_THOUSAND == newid)
    {
      s.dwID = oldid * ONE_THOUSAND + newid % ONE_THOUSAND;
      update = true;
    }
  }
  if (update)
    sendSkillInfo();
}

bool Transform::checkSkill(DWORD skillid) const
{
  if (MiscConfig::getMe().getNewRoleCFG().dwTransSkill == skillid)
    return true;

  switch(m_eType)
  {
    case ETRANSFORM_NORMAL:
      {
        const SNpcCFG* pBase = NpcConfig::getMe().getNpcCFG(m_dwMonsterID);
        if (pBase == nullptr)
          return false;
        auto v = find_if(pBase->vecTransformSkill.begin(), pBase->vecTransformSkill.end(), [&](const DWORD& d){
            return d == skillid;
            });
        return v != pBase->vecTransformSkill.end();
      }
    case ETRANSFROM_POLIFIRE:
      {
        if (m_pUser->m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT)
          return true;
        auto it = find_if(m_listSkillItems.begin(), m_listSkillItems.end(), [&](const SSkillItem& r) -> bool{
            return r.dwID == skillid;
            });
        return it != m_listSkillItems.end();
      }
    case ETRANSFORM_ALTMAN:
      {
        if (m_pUser->m_oSkillProcessor.getRunner().getState() == ESKILLSTATE_CHANT)
          return true;
        auto it = find_if(m_vecSkillItems.begin(), m_vecSkillItems.end(), [&](const SSkillItem& r) -> bool{
            return r.dwID == skillid;
            });
        return it != m_vecSkillItems.end();
      }
      break;
  }

  return false;
}

void Transform::onUseSkill(DWORD skillid)
{
  if (m_eType == ETRANSFROM_POLIFIRE)
  {
    if (skillid != MiscConfig::getMe().getPoliFireCFG().dwDefaultSkillID)
      delSkill(skillid);
  }
  else if (m_eType == ETRANSFORM_ALTMAN)
  {
    if (skillid != MiscConfig::getMe().getAltmanCFG().dwDefaultSkillID)
      delSkill(skillid);
  }
}

DWORD Transform::getNormalSkillID() const
{
  switch(m_eType)
  {
    case ETRANSFORM_NORMAL:
      {
        const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(m_dwMonsterID);
        if (pCFG == nullptr)
          return 0;
        return pCFG->dwNormalSkillID;
      }
    case ETRANSFROM_POLIFIRE:
      return 0;
    case ETRANSFORM_ALTMAN:
      return 0;
  }

  return 0;
}

void Transform::getPoliFireDropSkills(TVecDWORD& vec)
{
  for (auto &s : m_listSkillItems)
    vec.push_back(s.dwID);

  // get original skillid
  if (m_mapReplaceSkill.empty() == false)
  {
    for (auto &m : m_mapReplaceSkill)
    {
      for (auto &v : vec)
      {
        if (v == m.second)
          v = m.first * ONE_THOUSAND + v % ONE_THOUSAND;
      }
    }
  }
}

void Transform::clearSkill()
{
  if (m_eType == ETRANSFROM_POLIFIRE)
  {
    m_listSkillItems.clear();
    sendSkillInfo();
  }
  else if (m_eType == ETRANSFORM_ALTMAN)
  {
    m_vecSkillItems.clear();
    sendSkillInfo();
  }
}

bool Transform::canPickUp()
{
  if (m_eType == ETRANSFORM_ALTMAN)
  {
    DWORD size = m_vecSkillItems.size();
    if (size >= MiscConfig::getMe().getAltmanCFG().dwMaxSkillPos)
      return false;
  }

  return true;
}

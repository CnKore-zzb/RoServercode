#include "GSocial.h"
#include "xLog.h"
#include "MiscConfig.h"

GSocial::GSocial()
{

}

GSocial::~GSocial()
{

}

void GSocial::parseRelationTime(const string& str, map<ESocialRelation, DWORD>& mapResult)
{
  mapResult.clear();

  stringstream sstr(str);
  ESocialRelation eRelation = ESOCIALRELATION_MIN;
  DWORD dwRelation = 0;
  DWORD dwTime = 0;

  while (sstr >> dwRelation)
  {
    if (dwRelation == 0)
      break;
    if (dwRelation <= ESOCIALRELATION_MIN || dwRelation >= ESOCIALRELATION_MAX)
      continue;

    eRelation = static_cast<ESocialRelation>(dwRelation);

    sstr >> dwTime;
    mapResult[eRelation] = dwTime;
  }
}

void GSocial::serialRelationTime(const map<ESocialRelation, DWORD>& mapTime, string& result)
{
  result.clear();

  std::ostringstream ostr;
  for (auto m = mapTime.begin(); m != mapTime.end(); ++m)
    ostr << m->first << " " << m->second << " ";
  result = ostr.str();
}

bool GSocial::toData(SyncSocialListSocialCmd& cmd)
{
  if (!m_bInit)
    return false;

  cmd.Clear();

  cmd.set_charid(m_qwCharID);
  for (auto &m : m_mapSocialList)
  {
    SocialItem* pItem = cmd.add_items();
    pItem->set_charid(m.first);
    pItem->set_relation(m.second.dwRelation);

    string str;
    serialRelationTime(m.second.mapRelationTime, str);
    pItem->set_createtime(str);
  }

  return true;
}

bool GSocial::checkRelation(QWORD qwCharID, ESocialRelation eRelation) const
{
  auto m = m_mapSocialList.find(qwCharID);
  if (m == m_mapSocialList.end())
    return false;

  if (eRelation == ESOCIALRELATION_RECALL && ((m->second.dwRelation & eRelation) != 0))
  {
    const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
    if (getRelationTime(qwCharID, eRelation) + rCFG.dwContractTime < xTime::getCurSec())
      return false;
  }

  return (m->second.dwRelation & eRelation) != 0;
}

DWORD GSocial::getRelationCount(ESocialRelation eRelation) const
{
  DWORD dwCount = 0;
  for (auto &m : m_mapSocialList)
  {
    if (checkRelation(m.first, eRelation) == true)
      ++dwCount;
  }
  return dwCount;
}

DWORD GSocial::getRelationTime(QWORD qwCharID, ESocialRelation eRelation) const
{
  if (qwCharID == 0)
  {
    for (auto &m : m_mapSocialList)
    {
      if (checkRelation(m.first, eRelation) == true)
      {
        qwCharID = m.first;
        break;
      }
    }
  }

  auto m = m_mapSocialList.find(qwCharID);
  if (m == m_mapSocialList.end())
    return 0;
  auto time = m->second.mapRelationTime.find(eRelation);
  if (time == m->second.mapRelationTime.end())
    return 0;
  return time->second;
}

void GSocial::initSocial(const SyncSocialListSocialCmd& cmd)
{
  m_bInit = true;
  m_mapSocialList.clear();
  m_qwCharID = cmd.charid();

  for (int i = 0; i < cmd.items_size(); ++i)
  {
    const SocialItem& rItem = cmd.items(i);

    SGSocialItem& rGItem = m_mapSocialList[rItem.charid()];
    rGItem.dwRelation = rItem.relation();
    parseRelationTime(rItem.createtime(), rGItem.mapRelationTime);
  }
  XDBG << "[GSocial-初始化] charid :" << cmd.charid() << "初始化社交列表" << cmd.ShortDebugString() << XEND;
}

void GSocial::updateSocial(const SocialListUpdateSocialCmd& cmd)
{
  for (int i = 0; i < cmd.dels_size(); ++i)
  {
    auto m = m_mapSocialList.find(cmd.dels(i));
    if (m != m_mapSocialList.end())
      m_mapSocialList.erase(m);
  }

  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const SocialItem& rItem = cmd.updates(i);
    auto m = m_mapSocialList.find(rItem.charid());
    if (m == m_mapSocialList.end())
    {
      m_mapSocialList.insert(std::make_pair(rItem.charid(), SGSocialItem()));
      m = m_mapSocialList.find(rItem.charid());
      if (m == m_mapSocialList.end())
        continue;
    }
    m->second.dwRelation = rItem.relation();
    parseRelationTime(rItem.createtime(), m->second.mapRelationTime);
  }

  XDBG << "[GSocial-更新] charid :" << cmd.charid() << "更新社交" << cmd.ShortDebugString() << XEND;
}

QWORD GSocial::getTutorCharID()
{
  for (auto &m : m_mapSocialList)
    if (checkRelation(m.first, ESOCIALRELATION_TUTOR) == true)
      return m.first;
  return 0;
}

void GSocial::collectRelation(ESocialRelation eRelation, TSetQWORD& setIDs)
{
  for (auto &m : m_mapSocialList)
  {
    if (checkRelation(m.first, eRelation) == true)
      setIDs.insert(m.first);
  }
}

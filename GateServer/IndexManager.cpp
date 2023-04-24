#include "IndexManager.h"
#include "GateUser.h"
#include "CommonConfig.h"

//一级索引管理器
void OneLevelIndexManager::broadcastCmd(ONE_LEVEL_INDEX_TYPE t, QWORD i, unsigned char *buf, unsigned short len, QWORD exclude, DWORD ip)
{
  if (t >= ONE_LEVEL_INDEX_TYPE_MAX) return;

  OneLevelIndex::const_iterator oneIter = indexlist[t].find(i);
  if (oneIter == indexlist[t].end()) return;

  UserSet::const_iterator userIter = oneIter->second.begin();
  for (; userIter != oneIter->second.end(); ++userIter)
  {
    if ((*userIter) && GateUser_State::running==(*userIter)->getState()
        && ((*userIter)->id!=exclude)
        && (!ip || ((*userIter)->getIP()==ip)))
      (*userIter)->sendCmdToMe(buf, len);
  }
}

void OneLevelIndexManager::addIndex(ONE_LEVEL_INDEX_TYPE t, QWORD i, GateUser *pUser)
{
  if (pUser && t < ONE_LEVEL_INDEX_TYPE_MAX)
    indexlist[t][i].insert(pUser);
}

void OneLevelIndexManager::remove(GateUser *pUser)
{
  for (DWORD i = 0; i < ONE_LEVEL_INDEX_TYPE_MAX; ++i)
  {
    for (OneLevelIndex::iterator oneIter = indexlist[i].begin(); oneIter != indexlist[i].end(); ++oneIter)
      oneIter->second.erase(pUser);
  }
}

void OneLevelIndexManager::remove(ONE_LEVEL_INDEX_TYPE t, QWORD i, GateUser *pUser)
{
  if (!pUser || t>=ONE_LEVEL_INDEX_TYPE_MAX) return;

  OneLevelIndex::iterator oneIter = indexlist[t].find(i);
  if (oneIter != indexlist[t].end())
    oneIter->second.erase(pUser);
}

//二级索引管理器
void TwoLevelIndexManager::broadcastCmd(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, unsigned char *buf, unsigned short len, GateIndexFilter &filter)
{
  if (t >= TWO_LEVEL_INDEX_TYPE_MAX) return;

  TwoLevelIndex::const_iterator twoIter = indexlist[t].find(i);
  if (twoIter == indexlist[t].end()) return;
  OneLevelIndex::const_iterator oneIter = twoIter->second.find(i2);
  if (oneIter == twoIter->second.end()) return;

  UserSet::const_iterator userIter = oneIter->second.begin(), end = oneIter->second.end();
  for (; userIter != end; ++userIter)
  {
    GateUser *pUser = (* userIter);
    if (pUser && GateUser_State::running==pUser->getState() && pUser->id!=filter.exclude)
    {
      if (!filter.group || (filter.team && (filter.team==pUser->m_qwTeamIndex)))
        (*userIter)->sendCmdToMe(buf, len);
    }
  }
}

void TwoLevelIndexManager::addIndex(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, GateUser *pUser)
{
  if (pUser && t < TWO_LEVEL_INDEX_TYPE_MAX)
    indexlist[t][i][i2].insert(pUser);
}

void TwoLevelIndexManager::remove(GateUser *pUser)
{
  if (!pUser) return;
  for (DWORD i = 0; i < TWO_LEVEL_INDEX_TYPE_MAX; ++i)
  {
    TwoLevelIndex::iterator twoIter = indexlist[i].begin();
    for (; twoIter != indexlist[i].end(); ++twoIter)
    {
      for (OneLevelIndex::iterator oneIter = twoIter->second.begin(); oneIter != twoIter->second.end(); ++oneIter)
      {
        oneIter->second.erase(pUser);
      }
    }
  }
}

void TwoLevelIndexManager::remove(TWO_LEVEL_INDEX_TYPE t, GateUser *pUser)
{
  if (!pUser || t>=TWO_LEVEL_INDEX_TYPE_MAX) return;

  TwoLevelIndex::iterator twoIter = indexlist[t].begin();
  for (; twoIter != indexlist[t].end(); ++twoIter)
  {
    for (OneLevelIndex::iterator oneIter = twoIter->second.begin(); oneIter != twoIter->second.end(); ++oneIter)
    {
      oneIter->second.erase(pUser);
    }
  }
}

void TwoLevelIndexManager::remove(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, GateUser *pUser)
{
  if (!pUser || t>=TWO_LEVEL_INDEX_TYPE_MAX) return;

  TwoLevelIndex::iterator twoIter = indexlist[t].find(i);
  if (twoIter != indexlist[t].end())
  {
    OneLevelIndex::iterator oneIter = twoIter->second.find(i2);
    if (oneIter != twoIter->second.end())
    {
      oneIter->second.erase(pUser);
    }
  }
}

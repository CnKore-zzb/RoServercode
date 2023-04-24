#include "TeamIDManager.h"
#include "TeamServer.h"
#include "TeamDataThread.h"
#include "xLog.h"

TeamIDManager::TeamIDManager()
{
}

TeamIDManager::~TeamIDManager()
{
}

#define TEAMID_INIT_NUM 10
void TeamIDManager::load()
{
  if (size() < TEAMID_INIT_NUM)
  {
    createTeamID(TEAMID_INIT_NUM - size());
  }
  XLOG_T("[队伍-ID],共有:%u个id", size());
}

QWORD TeamIDManager::getTeamID()
{
  if (m_oTeamIDQueue.empty()) return 0;

  QWORD ret = m_oTeamIDQueue.front();
  m_oTeamIDQueue.pop();

  createTeamID();

  return ret;
}

void TeamIDManager::putTeamID(QWORD teamid)
{
  if (!teamid) return;

  m_oTeamIDQueue.push(teamid);
}

void TeamIDManager::createTeamID(DWORD num)
{
  if (size() >= TEAMID_INIT_NUM)
    return;
  ThTeamData *pData = new ThTeamData(ThTeamDataAction_CreateTeamID);
  pData->m_dwCreateTeamIDNum = num;
  ThTeamDataThread::getMe().add(pData);
}

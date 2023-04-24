#include "GuidManager.h"
#include "xlib/xDBFields.h"
#include "ZoneServer.h"

extern ZoneServer* thisServer;

GuidManager::GuidManager() :
    m_oTempIDManager(((DWORD)1) << 31, (DWORD) - 502)
  , m_oRaidIDManager(1111111, 9999999)
  , m_oChatIDManager(1111111, 9999999)
  , m_oEventIDManager(((DWORD)1) << 31, (DWORD) - 502)
{

}

string GuidManager::newGuidStr(DWORD zoneID, DWORD sceneID)
{
  DWORD m_time = now();
  DWORD m_mapID = sceneID;
  DWORD m_zoneID = zoneID;
  if (m_time != zoneIndexM[sceneID].lastCreateStaticEntryTime)
  {
    zoneIndexM[sceneID].lastCreateStaticEntryTime = m_time;
    zoneIndexM[sceneID].lastCreateStaticEntryIndex = 1;
  }
  else
  {
    ++(zoneIndexM[sceneID].lastCreateStaticEntryIndex);
  }
  DWORD m_index = zoneIndexM[sceneID].lastCreateStaticEntryIndex;

  char szTemp[128] = {0};
  sprintf(szTemp, "%d-%d-%d-%d", m_index, m_zoneID, m_mapID, m_time);
  return szTemp;
}

DWORD GuidManager::getNextNpcID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextSceneItemID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextBroadID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextChatRoomID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextInterID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextActID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextTrapID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

/*DWORD GuidManager::getNextVoiceID()
{
  DWORD dwID = 0;
  m_oChatIDManager.getUniqueID(dwID);
  return dwID;
}*/

DWORD GuidManager::getNextRaidID()
{
  DWORD dwID = 0;
  m_oRaidIDManager.getUniqueID(dwID);
  dwID = dwID * 10 + thisServer->getZoneID() % 10;
  return dwID;
}

DWORD GuidManager::getNextEventID()
{
  DWORD dwID = 0;
  m_oEventIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextLetterID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}

DWORD GuidManager::getNextGuildGmSessionID()
{
  DWORD dwID = 0;
  m_oTempIDManager.getUniqueID(dwID);
  return dwID;
}


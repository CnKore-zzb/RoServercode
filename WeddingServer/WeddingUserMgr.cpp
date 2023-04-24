#include "WeddingUserMgr.h"
#include "WeddingSCmd.pb.h"
#include "WeddingServer.h"
#include "Wedding.h"
#include "WeddingManager.h"
#include "MiscConfig.h"

/************************************************************************/
/*WeddingUser                                                                      */
/************************************************************************/
bool WeddingUser::lockReserve()
{
  DWORD curSec = now();
  if (m_dwReserveLock == 0 || curSec > m_dwReserveLock)
  {
    m_dwReserveLock = curSec + MiscConfig::getMe().getWeddingMiscCFG().dwEngageInviteOverTime + 20;
    return true;
  }

  return false;
}

//预定解锁
bool WeddingUser::unlockReserve()
{
  m_dwReserveLock = 0;
  return true;
}

void WeddingUser::ntfBriefWeddingInfo2Client()
{
  NtfWeddingInfoCCmd cmd;
  Wedding*pWedding = WeddingManager::getMe().getWeddingByCharId(m_qwCharId);
  if (pWedding)
  {
    pWedding->mutableBirefWeddingInfo(cmd.mutable_info());
  }
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToClient(m_qwCharId, m_dwZoneId, send, len);
  XDBG <<"[婚礼-同步婚礼信息到客户端] NtfWeddingInfoCCmd"<<m_qwCharId <<m_dwZoneId <<cmd.ShortDebugString() << XEND;
}

bool WeddingUser::sendCmdToMe(void* buf, WORD len)
{
  if (!buf || !len) return false;
  return thisServer->sendCmdToClient(m_qwCharId, m_dwZoneId, buf, len);
}

bool WeddingUser::sendCmdToScene(void* buf, WORD len)
{
  if (!buf || !len) return false;
  return thisServer->forwardCmdToSceneServer(m_qwCharId, m_dwZoneId, buf, len);
}

void WeddingUser::sendMsg(DWORD dwMsgID, MsgParams oParams /*= MsgParams()*/, EMessageType eType /*= EMESSAGETYPE_FRAME*/)
{
  thisServer->sendMsg(m_qwCharId, m_dwZoneId, dwMsgID, oParams, eType);
}

/************************************************************************/
/*WeddingUserMgr                                                                      */
/************************************************************************/
void WeddingUserMgr::onUserOnline(const Cmd::SocialUser& rUser)
{
  WeddingUser&rWeddingUser = m_mapUser[rUser.charid()];
  rWeddingUser.m_qwCharId = rUser.charid();
  rWeddingUser.m_strName = rUser.name();
  rWeddingUser.m_dwZoneId = rUser.zoneid();  
  
  Wedding* w = WeddingManager::getMe().getWeddingByCharId(rWeddingUser.m_qwCharId);
  if (w)
  {
    w->onUserOnline(&rWeddingUser);
  }
  syncWeddingInfo2Scene(rWeddingUser.m_qwCharId, rWeddingUser.m_dwZoneId);
  rWeddingUser.ntfBriefWeddingInfo2Client();
}

void WeddingUserMgr::onUserOffline(const Cmd::SocialUser& rUser)
{
  m_mapUser.erase(rUser.charid());

  Wedding* pWedding = WeddingManager::getMe().getWeddingByCharId(rUser.charid());
  if (pWedding)
  {
    pWedding->onUserOffline(rUser.charid());
  }
}

bool WeddingUserMgr::syncWeddingInfo2Scene(QWORD qwCharId)
{
  WeddingUser* pUser = getWeddingUser(qwCharId);
  if (!pUser)
    return false;
  return syncWeddingInfo2Scene(pUser->m_qwCharId, pUser->m_dwZoneId);
}

bool WeddingUserMgr::syncWeddingInfo2Scene(QWORD qwCharId, DWORD dwZoneid)
{
  if (qwCharId == 0 || dwZoneid == 0)
    return false;

  SyncWeddingInfoSCmd cmd;
  cmd.set_charid(qwCharId);
  Wedding*pWedding = WeddingManager::getMe().getWeddingByCharId(qwCharId);
  if (pWedding)
  {
    pWedding->multableWeddingInfo(cmd.mutable_weddinginfo());
  }
  PROTOBUF(cmd, send, len);
  XDBG << "[婚礼-同步婚礼信息到Session] SyncWeddingInfoSCmd" << qwCharId << dwZoneid << cmd.ShortDebugString() << XEND;
  return thisServer->sendCmdToZone(dwZoneid, send, len);
}

WeddingUser* WeddingUserMgr::getWeddingUser(QWORD qwCharId)
{
  auto it = m_mapUser.find(qwCharId);
  if (it == m_mapUser.end())
    return nullptr;
  return &(it->second);
}

bool WeddingUserMgr::sendCmdToUser(QWORD qwCharId, void* buf, WORD len)
{
  WeddingUser* pUser = getWeddingUser(qwCharId);
  if (!pUser)
    return false;
  return pUser->sendCmdToMe(buf, len);
}

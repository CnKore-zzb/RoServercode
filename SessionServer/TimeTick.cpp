#include "SessionServer.h"
#include "SessionSceneManager.h"
#include "Boss.h"
#include "SessionUserManager.h"
#include "TowerManager.h"
#include "MsgManager.h"
#include "FerrisWheelManager.h"
#include "SessionActivityMgr.h"
#include "UserActive.h"
#include "SealManager.h"
#include "TimerM.h"
#include "GMCommandManager.h"
#include "ChatManager_SE.h"
#include "ShopMgr.h"
#include "GuildGmMgr.h"
#include "SessionThread.h"
#include "SessionUser.h"

void SessionServer::v_timetick()
{
  ZoneServer::v_timetick();

  if (getServerState() != ServerState::run) return;

  DWORD curTime = xTime::getCurSec();

  if (m_oTickOneSec.timeUp(curTime))
  {
    SessionSceneManager::getMe().delOffLineScene();
    BossList::getMe().timer(curTime);
    SessionUserManager::getMe().timer(curTime);
    TowerManager::getMe().timer(curTime);
    FerrisWheelManager::getMe().timer(curTime);
    SessionActivityMgr::getMe().timer(curTime);
    SealManager::getMe().timer(curTime);
    MsgManager::timer(curTime);
    ShopMgr::getMe().timeTick(curTime);
    UserActive::getMe().updateOnline2MatchServer();
    GuildGmMgr::getMe().timer(curTime);
    if (m_oTickOneMin.timeUp(curTime))
    {
      QuestConfig::getMe().randomWantedQuest();
      UserActive::getMe().timer(curTime);
      TimerM::getMe().timer(curTime);
      ChatManager_SE::getMe().timer(curTime);
      //GuildGmMgr::getMe().timer(curTime);
      if (m_oTickOneHour.timeUp(curTime))
      {
        BossList::getMe().save();
      }
    }
    GMCommandManager::getMe().timerTick(curTime);
  }

  processSessionThread();
}

void SessionServer::processSessionThread()
{
  SessionThreadData *data = SessionThread::getMe().get();
  while (data)
  {
    SessionThread::getMe().pop();
    doSessionThreadData(data);
    SAFE_DELETE(data);

    data = SessionThread::getMe().get();
  }
}

void SessionServer::doSessionThreadData(SessionThreadData *pData)
{
  if (!pData) return;

  switch (pData->m_oAction)
  {
    case SessionThreadAction_OfflineMsg_Load:
      {
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(pData->m_qwCharID);
        if (!pUser) return;

        switch (pData->m_dwMsgLoadType)
        {
          case OFFLINE_MSG_LOAG_TYPE_ONE:
            {
              TVecOfflineMsg vecMsg;
              if (!ChatManager_SE::getMe().loadDb(pData->m_qwCharID, vecMsg, pData->m_oRetRecordSet))
                return;

              TSetQWORD setDelID;
              for (auto v = vecMsg.begin(); v != vecMsg.end();)
              {
                OfflineMsgBase* pMsg = *v;
                if (pMsg)
                {
                  if (pMsg->send(pUser) == true)
                    pMsg->getId(setDelID);
                  SAFE_DELETE(pMsg);
                }
                v = vecMsg.erase(v);
              }
              ChatManager_SE::getMe().delDb(setDelID);

              // 处理需缓存的离线消息

              TVecDWORD vecType;
              vecType.push_back((DWORD)EOFFLINEMSG_ADD_RELATION);
              vecType.push_back((DWORD)EOFFLINEMSG_REMOVE_RELATION);

              ChatManager_SE::getMe().eraseMapCharID2Msgs(pData->m_qwCharID);

              vecType.push_back((DWORD)EOFFLINEMSG_ADD_RELATION);
              vecType.push_back((DWORD)EOFFLINEMSG_REMOVE_RELATION);

              ChatManager_SE::getMe().preLoadDb(pData->m_qwCharID, vecType, OFFLINE_MSG_LOAG_TYPE_TWO);
            }
            break;
          case OFFLINE_MSG_LOAG_TYPE_TWO:
            {
              TVecOfflineMsg vecMsg;

              if (!ChatManager_SE::getMe().loadDb(pUser->id, vecMsg, pData->m_oRetRecordSet))
                return;

              ChatManager_SE::getMe().addMapCharID2Msgs(pData->m_qwCharID, vecMsg);
            }
            break;
          case OFFLINE_MSG_LOAG_TYPE_THREE:
            {
              TVecOfflineMsg vecMsg;
              if (!ChatManager_SE::getMe().loadDb(pUser->id, vecMsg, pData->m_oRetRecordSet))
              {
                return;
              }

              TSetQWORD setDelID;
              for (auto v = vecMsg.begin(); v != vecMsg.end();)
              {
                OfflineMsgBase* pMsg = *v;
                if (pMsg)
                {
                  if (pMsg->send(pUser) == true)
                    pMsg->getId(setDelID);
                  SAFE_DELETE(pMsg);
                }
                v = vecMsg.erase(v);
              }
              ChatManager_SE::getMe().delDb(setDelID);
            }
            break;
          default:
            break;
        }
      }
      break;
    case SessionThreadAction_Mail_Load:
      {
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(pData->m_qwCharID);
        if (pUser)
        {
          pUser->getMail().loadAllMail(pData->m_oRetRecordSet);
          pUser->getMail().updateMail();
        }
      }
      break;
    case SessionThreadAction_Trade_LoadGiveToMe:
      {
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(pData->m_qwCharID);
        if (pUser)
        {
          pUser->getTradeLog().loadGiveToMeFromDb(pData->m_oRetRecordSet);
        }
      }
      break;
    case SessionThreadAction_Trade_LoadGiveToOther:
      {
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(pData->m_qwCharID);
        if (pUser)
        {
          pUser->getTradeLog().loadGiveToOtherFromDb(pData->m_oRetRecordSet);
        }
      }
      break;
    default:
      break;
  }
}


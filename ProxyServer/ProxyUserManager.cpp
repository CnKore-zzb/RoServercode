#include "ProxyUserManager.h"
#include "ProxyServer.h"
#include "xNetProcessor.h"
#include "ProxyUser.h"
#include "ProxyUserDataThread.h"
#include "SceneUser2.pb.h"
#include "LoginUserCmd.pb.h"

ProxyUserManager::ProxyUserManager():m_oOneSecTimer(1),m_oFiveSecTimer(5)
{
}

ProxyUserManager::~ProxyUserManager()
{
}

bool ProxyUserManager::add(ProxyUser *user)
{
  if (!user) return false;
  m_oProxyUserList.insert(user);
  return true;
}

bool ProxyUserManager::del(ProxyUser *user)
{
  if (!user) return false;
  m_oProxyUserList.erase(user);
  return true;
}

void ProxyUserManager::timer()
{
  DWORD cur = now();
  static DWORD dwTickDelay = 500000;
  if (m_oOneSecTimer.timeUp(cur))
  {
    m_oTickFrameTimer.elapseStart();
    for (auto &it : m_oProxyUserList)
    {
      it->timer(cur);
      if (m_oTickFrameTimer.uElapse() > dwTickDelay)
      {
        return;
      }
    }

    if (m_oFiveSecTimer.timeUp(cur))
    {
      DWORD count = m_oProxyUserList.size();
      if (count != m_dwCount)
      {
        m_dwCount = count;
        syncTaskNum();
      }
    }
  }

  ProxyUserData *data = ProxyUserDataThread::getMe().get();
  while (data)
  {
    ProxyUserDataThread::getMe().pop();
    doProxyUserData(data);
    SAFE_DELETE(data);

    data = ProxyUserDataThread::getMe().get();
  }
}

void ProxyUserManager::doProxyUserData(ProxyUserData *pData)
{
  if (!pData) return;

  if (m_oProxyUserList.find(pData->m_pUser) == m_oProxyUserList.end())
    return;

  ProxyUser *pUser = pData->m_pUser;
  if (!pUser || pUser->accid != pData->m_qwAccID) return;

  switch (pData->m_oAction)
  {
    case ProxyUserDataAction_LOGIN:
      {
        if (!pUser->get(pData))
        {
          return;
        }

        if (!pUser->checkNoLoginTime())
        {
          return;
        }

        pUser->realSendSnapShotToMe(pData);

        SafeDeviceUserCmd cmd;
        cmd.set_safe(pUser->m_bSafeDevice);
        PROTOBUF(cmd, send, len);
        pUser->forwardClientTask(send, len);
        XLOG << "[登录-安全设备]" << pUser->accid << "safedevice:" << pUser->m_bSafeDevice << "语言: " <<  pUser->m_dwLanguage << XEND;
        pUser->onLineAuthorize();
      }
      break;
    case ProxyUserDataAction_SEND:
      {
        pUser->realSendSnapShotToMe(pData);
      }
      break;
    case ProxyUserDataAction_CANCEL_DELETE:
      {
        if (pData->m_blCancelDeleteRet)
        {
          pUser->realSendSnapShotToMe(pData);
        }
      }
      break;
    case ProxyUserDataAction_SET_DELETE:
      {
        switch (pData->m_oDeleteRet)
        {
          case ProxyUserDeleteRet_NULL:
            {
            }
            break;
          case ProxyUserDeleteRet_OK:
            {
              pUser->kickChar(pUser->m_oAccBaseData.m_qwDeleteCharID);
              pUser->realSendSnapShotToMe(pData);
            }
            break;
          case ProxyUserDeleteRet_LOCKED:
            {
              pUser->notifyError(REG_ERR_DELETE_ERROR_LOCKED);
            }
            break;
          case ProxyUserDeleteRet_ERROR:
            {
              pUser->notifyError(REG_ERR_DELETE_ERROR);
            }
            break;
          case ProxyUserDeleteRet_MSG_1067:
            {
              SysMsg cmd;
              cmd.set_id(1067);
              cmd.set_type(EMESSAGETYPE_FRAME);
              cmd.set_act(EMESSAGEACT_ADD);
              cmd.set_delay(0);

              PROTOBUF(cmd, send, len);
              pUser->forwardClientTask(send, len);
            }
            break;
          default:
            break;
        }
      }
      break;
    case ProxyUserDataAction_DELETE_CHAR:
      {
        pUser->realDeleteChar();
      }
      break;
    default:
      break;
  }
}

void ProxyUserManager::process()
{
  xNetProcessor *np;

  static DWORD delay = 1000000;

  m_oFrameTimer.elapseStart();
  for (auto &it : m_oProxyUserList)
  {
    np = it->m_pGateServerTask;
    if (m_oFrameTimer.uElapse() > delay)
    {
      if (!np)
      {
        XDBG << "[ProxyUser]" << "耗时" << m_oFrameTimer.uElapse() << "微秒,未连接游戏服，跳过消息处理" << XEND;
#ifndef _ALL_SUPER_GM
        continue;
#endif
      }
    }
    if (np)
    {
      CmdPair *pair = np->getCmd();
      while (pair)
      {
        if (!it->doGateServerCmd((xCommand *)(pair->second), pair->first))
        {
          XDBG << "[GateServer消息处理]" << it->id << it->name << "error," << ((xCommand *)(pair->second))->cmd << ((xCommand *)(pair->second))->param << XEND;
        }
   //     XDBG("[GateServer消息处理],%llu,%s,(%u,%u)", it->id, it->name, ((xCommand *)(pair->second))->cmd, ((xCommand *)(pair->second))->param);
        np->popCmd();
        /*
        if (m_oFrameTimer.uElapse() > delay)
        {
          XDBG << "[ProxyUser]" << "耗时" << m_oFrameTimer.uElapse() << "微秒,GateServer处理，跳过" << XEND;
          break;
        }
        */
        pair = np->getCmd();
      }
    }
    np = it->m_pClientTask;
    if (np)
    {
      CmdPair *pair = np->getCmd();
      while (pair)
      {
        if (!it->doClientCmd((xCommand *)(pair->second), pair->first))
        {
          XDBG << "[Client消息处理]" << it->id << it->name << "error," << ((xCommand *)(pair->second))->cmd << ((xCommand *)(pair->second))->param << XEND;
        }
     //   XDBG("[Client消息处理],%llu,%s,(%u,%u)", it->id, it->name, ((xCommand *)(pair->second))->cmd, ((xCommand *)(pair->second))->param);
        np->popCmd();
        /*
        if (m_oFrameTimer.uElapse() > delay)
        {
          XDBG << "[ProxyUser]" << "耗时" << m_oFrameTimer.uElapse() << "微秒,客户度消息处理，跳过" << XEND;
          break;
        }
        */
        pair = np->getCmd();
      }
    }
  }
}

void ProxyUserManager::onClose(xNetProcessor* np)
{
  if (!np) return;

  ProxyUser *pUser = nullptr;

  for (auto &it : m_oProxyUserList)
  {
    if (it->m_pClientTask == np)
    {
      pUser = it;
      it->m_pClientTask = nullptr;
      XLOG << "[连接]" << pUser->accid << pUser->m_dwRegionID << "客户端连接关闭" << pUser->m_strDomain << pUser->m_strTaskIP << pUser->m_strDevice << XEND;
      break;
    }
    if (it->m_pGateServerTask == np)
    {
      pUser = it;
      it->m_pGateServerTask = nullptr;
      XLOG << "[连接]" << pUser->accid << pUser->m_dwRegionID << "关闭游戏服连接" << XEND;
      if (pUser->m_qwCharID && pUser->m_oReconnectController.isNeed())
      {
        return;
      }
    }
  }

  if (pUser)
  {
    m_oProxyUserList.erase(pUser);
    if (pUser->m_pGateServerTask)
    {
      thisServer->addCloseList(pUser->m_pGateServerTask, TerminateMethod::terminate_active, "m_pGateServerTask close");
      pUser->m_pGateServerTask = nullptr;
    }
    if (pUser->m_pClientTask)
    {
      thisServer->addCloseList(pUser->m_pClientTask, TerminateMethod::terminate_active, "m_pClientTask close");
      pUser->m_pClientTask = nullptr;
    }
    XLOG << "[ProxyUser]" << pUser->accid << pUser->m_dwRegionID << "删除" << XEND;
    SAFE_DELETE(pUser);
  }
}

void ProxyUserManager::syncTaskNum()
{
  Cmd::InfoProxySystemCmd message;
  message.set_proxyid(thisServer->getProxyID());
  message.set_tasknum(m_dwCount);
  PROTOBUF(message, send, len);
  thisServer->sendCmd(ClientType::g_proxy_server, send, len);
}

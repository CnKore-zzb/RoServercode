#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "GateServer.h"
#include "xXMLParser.h"
#include "xNetProcessor.h"
#include "xTaskThread.h"
#include "GateUserManager.h"
#include "GateUser.h"
#include "GatewayCmd.h"
#include "RedisManager.h"
#include "ChatFilterManager.h"

#define FLOW_STAT_INTERVAL 1800

//GateServer *thisServer = 0;
#define GM_OVERTIME   10        //session处理超时时间
void GMNetProcesser::updateTime(DWORD curSec)
{
  expiretime = curSec + GM_OVERTIME;
}

GateServer::GateServer(OptArgs &args) : ZoneServer(args)
   ,m_oOneSecTimer(1)
   ,m_oTenMinTimer(600)
{
  m_oCmdFiler.clear();
}

GateServer::~GateServer()
{
}

bool GateServer::v_init()
{
  return true;
}

void GateServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  if (np->isTask())
  {
    GateUser *pUser = GateUserManager::getMe().getUserByAccID(np->id);
    if (pUser && (pUser->client_task() == np))
    {
      GateUserManager::getMe().loginOut(pUser);
      pUser->set_client_task(NULL);
      XLOG << "[断开连接]" << pUser->tempid << pUser->id << pUser->name << "退出" << pUser->m_strProxyIP << XEND;
    }
    pUser = GateUserManager::getMe().getLoginUserByAccID(np->id);
    if (pUser && (pUser->client_task() == np))
    {
      pUser->set_client_task(NULL);
      GateUserManager::getMe().delLoginUser(pUser);
      XLOG << "[断开连接]" << pUser->tempid << pUser->id << pUser->name << "LoginUser退出" << pUser->m_strProxyIP << XEND;
      SAFE_DELETE(pUser);
    }
    
    //gm
    if (np->tempid)
    {
      delGMCon(np->tempid);
    }
  }
  else if(np->isClient())
  {
    ZoneServer::v_closeNp(np);
  }
}

void GateServer::v_closeServer(xNetProcessor *np)
{
  if (!np) return;

  if (strcmp(np->name,"SessionServer")==0)
  {

  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {
    GateUserManager::getMe().kickUserOnScene(np);
  }
}

void GateServer::v_final()
{
  ZoneServer::v_final();
}

bool GateServer::doCmd(xNetProcessor *np, BYTE *buf, WORD len)
{
  if(!np || !buf || !len) return false;

  if(np->isPlatClient())
  {
    ChatFilterManager::getMe().doCmd(buf, len);
  }
  else
  {
    ZoneServer::doCmd(np, buf, len);
  }

  return true;
}

GMNetProcesser* GateServer::getGMCon(QWORD id)
{
  auto it = m_mapGMCon.find(id);
  if (it == m_mapGMCon.end())
    return nullptr;
  return &(it->second);
}

void GateServer::delGMCon(QWORD id)
{
  m_mapGMCon.erase(id);
  XLOG << "[GM-关闭连接] id" <<id << XEND;
}

GMNetProcesser* GateServer::addGMCon(xNetProcessor* pNet)
{
  if (!pNet)
    return nullptr;
  GMNetProcesser*pGm = nullptr;
  if (pNet->tempid)
  {
    pGm = getGMCon(pNet->tempid);
  }
  
  if (pGm == nullptr)
  {
    m_qwGMId++;
    pNet->set_tempid(m_qwGMId);
    GMNetProcesser& gm = m_mapGMCon[m_qwGMId];
    gm.pNetProcess = pNet;
    pGm = &gm;
  }  

  pGm->updateTime(now());

  XLOG << "[GM-添加连接] id" << m_qwGMId << XEND;
  return pGm;
}



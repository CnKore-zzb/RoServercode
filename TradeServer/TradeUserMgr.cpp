#include "TradeUserMgr.h"
#include "UserCmd.h"
#include "xCommand.h"
#include "xTime.h"
#include "SysMsg.pb.h"
#include "TradeServer.h"
#include "StatisticsDefine.h"
#include "PlatLogManager.h"

// GlobalUser
TradeUser::TradeUser(const SocialUser& rUser) : m_oUser(rUser)
{

}

TradeUser::~TradeUser()
{

}

void TradeUser::setPermission(EPermission per, DWORD value)
{
  m_mapPer[per] = value;
}

DWORD TradeUser::getPerValue(EPermission per)
{
  auto it = m_mapPer.find(per);
  if(it == m_mapPer.end())
    return 0;
  return it->second;
}

// TradeUserMgr
TradeUserMgr::TradeUserMgr()
{

}

TradeUserMgr::~TradeUserMgr()
{

}

void TradeUserMgr::onUserOnline(const SocialUser& rUser)
{
  auto m = m_mapTradeUser.find(rUser.charid());
  if (m != m_mapTradeUser.end())
    return;

  TradeUser* pUser = NEW TradeUser(rUser);
  if (pUser == nullptr)
    return;
  m_mapTradeUser[rUser.charid()] = pUser;
}

void TradeUserMgr::onUserOffline(const SocialUser& rUser)
{
  auto m = m_mapTradeUser.find(rUser.charid());
  if (m == m_mapTradeUser.end())
    return;

  TradeUser* pUser = m->second;
  m_mapTradeUser.erase(m);
  SAFE_DELETE(pUser);
}

TradeUser* TradeUserMgr::getTradeUser(QWORD qwCharID)
{
  auto m = m_mapTradeUser.find(qwCharID);
  if (m != m_mapTradeUser.end())
    return m->second;
  return nullptr;
}


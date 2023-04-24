#include "UserActive.h"
#include "xDBConnPool.h"
#include "SessionServer.h"
#include "SessionUserManager.h"
#include "GZoneCmd.pb.h"

UserActive::UserActive()
{

}

UserActive::~UserActive()
{

}

void UserActive::init()
{
  m_list.clear();

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (field)
  {
    m_dwCheckTime = now() - DAY_T;
    char szWhere[256] = {0};
    snprintf(szWhere, 256, "zoneid=%u and onlinetime>%u", thisServer->getZoneID(), m_dwCheckTime);

    field->setValid("accid,charid,onlinetime");
    xRecordSet set;

    QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, szWhere);
    if (QWORD_MAX == retNum)
    {
      XERR << "[活跃在线]" << "数据库查询失败" << XEND;
    }
    for (DWORD i=0; i< retNum; ++i)
    {
      QWORD accid = set[i].get<QWORD>("accid");
      DWORD onlinetime = set[i].get<DWORD>("onlinetime");
      setActiveTime(accid, onlinetime, false);
    }
  }
  update();
}

void UserActive::setActiveTime(QWORD accid, DWORD t, bool isUpdate)
{
  auto it = m_list.find(accid);
  if (it == m_list.end())
  {
    m_list[accid] = t;
    return;
  }

  if (it->second > t)
    return;

  it->second = t;

  if (isUpdate)
    update();
}

void UserActive::add(QWORD accid)
{
  setActiveTime(accid, now(), true);
}

void UserActive::update()
{
  DWORD active = m_list.size() / 10 * 10;
  DWORD online = SessionUserManager::getMe().size() / 10 * 10;
  if (m_dwOnlineNum == online && m_dwActiveNum == active)
  {
    return;
  }
  m_dwOnlineNum = online;
  m_dwActiveNum = active;
  UpdateActiveOnlineGZoneCmd message;
  message.set_zoneid(thisServer->getZoneID());
  message.set_active(m_dwActiveNum);
  message.set_online(m_dwOnlineNum);
  PROTOBUF(message, send, len);
  thisServer->sendCmd(ClientType::gzone_server, send, len);
  XLOG << "[活跃在线]" << "活跃" << active << "在线" << online << XEND;
}

void UserActive::updateOnline2MatchServer()
{
  DWORD online = SessionUserManager::getMe().size();
  if (m_dwOnlineNumTemp == online)
    return;
  m_dwOnlineNumTemp = online;
  UpdateActiveOnlineGZoneCmd message;
  message.set_zoneid(thisServer->getZoneID());
  message.set_online(online);
  PROTOBUF(message, send, len);
  thisServer->sendCmd(ClientType::match_server, send, len);
}

void UserActive::timer(DWORD cur)
{
  if (!m_dwCheckTime) return;

  if ((cur > m_dwCheckTime) && (cur - m_dwCheckTime > HOUR_T))
  {
    check();
  }
}

void UserActive::check()
{
  m_dwCheckTime = now() - DAY_T;
  auto it = m_list.begin();
  auto tmp = it;
  for ( ; it != m_list.end(); )
  {
    tmp = it++;
    if (tmp->second < m_dwCheckTime)
    {
      m_list.erase(tmp);
    }
  }
  XLOG << "[活跃在线]" << "定时更新" << XEND;
  update();
}

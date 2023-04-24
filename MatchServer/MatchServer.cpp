#include <iostream>
#include "MatchServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RegCmd.h"
#include "MatchSCmd.pb.h"
#include "MatchManager.h"
#include "CommonConfig.h"
#include "GlobalShopMgr.h"
#include "RedisManager.h"
#include "MatchGMTest.h"
#include "ScoreManager.h"

void PollyZoneInfo::setUserCount(DWORD count)
{
  dwUserCount = count;
  XLOG << "[斗技场-更新线负载] zoneid" << dwZoneId << "人数" << dwUserCount << XEND;
}

MatchServer::MatchServer(OptArgs &args):RegionServer(args), m_oOneSecTimer(3)
{
}

MatchServer::~MatchServer()
{
}

void MatchServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;
  GlobalShopMgr::getMe().saveDb();
  RegionServer::v_final();

  m_oSQLThread.thread_stop();
  m_oSQLThread.thread_join();
}

void MatchServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void MatchServer::v_zoneDel(DWORD zoneId)
{
  delZoneInfo(zoneId);
}

void MatchServer::v_timetick()
{
  RegionServer::v_timetick();
 
  DWORD curTime = xTime::getCurSec();
  MatchManager::getMe().timeTick(curTime);

#ifdef _DEBUG
  MatchGMTest::getMe().timer(curTime);
#endif

  if (m_oOneSecTimer.timeUp(curTime))
  {
    if (n_bNeedSort)
      sortZone();
    ScoreManager::getMe().timer(curTime);
  }
  GlobalShopMgr::getMe().timeTick(curTime);
}

bool MatchServer::v_init()
{    
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("MatchServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， MatchServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  if (!loadConfig())
    return false;

  const xLuaData& data = getBranchConfig().getData("Redis");
  if (RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port")) == false)
  {
    XERR << "[RedisManager]" << "init error!" << XEND;
    return false;
  }

  MatchManager::getMe().init();
  GlobalShopMgr::getMe().loadDb();
  ScoreManager::getMe().init();

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }

  initDBConnPool("DataBase", getRegionDBName(), m_oSQLThread.getDBConnPool());
  if (!m_oSQLThread.thread_start())
  {
    XERR << "[xSQLThread]" << "创建失败" << XEND;
    return false;
  }

  return true;
}

bool MatchServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadMatchConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool MatchServer::sendCmdToClient(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::SessionToMeRecordTrade cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);
  return thisServer->sendCmdToZone(zoneId, send, len2);
}

bool MatchServer::forwardCmdToTeamServer(DWORD zoneId, void* buf, WORD len)
{
  Cmd::SessionForwardMatchTeam cmd;
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToZone(zoneId, send2, len2);
}

bool MatchServer::forwardCmdToSceneServer(QWORD charId, DWORD zoneId, void* buf, WORD len)
{
  Cmd::SessionForwardMatchScene cmd;
  cmd.set_charid(charId);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send2, len2);
  return thisServer->sendCmdToZone(zoneId, send2, len2);
}

bool MatchServer::isInPvpZoneId(DWORD zoneId)
{
  if (zoneId == 0)
    return false;

  if (zoneId == m_dwLLHZoneid)
    return true;

  if (zoneId == m_dwSMZLZoneid)
    return true;

  if (zoneId == m_dwHLJSZoneid)
    return true;
  return false;
}

void MatchServer::addZoneInfo(DWORD zoneId)
{
  PollyZoneInfo zi;
  zi.dwZoneId = zoneId;
  zi.dwUserCount = 0;
  m_mapZoneInfo[zoneId] = zi;
  m_dwPollyZoneid = zoneId;
  XLOG << "[斗技场-普通线] 添加，zoneid" << zoneId << XEND;
}

void MatchServer::delZoneInfo(DWORD zoneId)
{
  m_mapZoneInfo.erase(zoneId);
  XLOG << "[斗技场-普通线] 删除，zoneid" << zoneId << XEND;
}

void MatchServer::sortZone()
{
  if (m_mapZoneInfo.empty())
    return ;

  //sort
  std::vector<PollyZoneInfo> vecZone;
  vecZone.reserve(m_mapZoneInfo.size());
  std::pair<DWORD, DWORD> pr;
  for (auto it = m_mapZoneInfo.begin(); it != m_mapZoneInfo.end(); ++it)
  {
    vecZone.push_back(it->second);
  }

  std::sort(vecZone.begin(), vecZone.end(), CmpByValue1());
  m_dwPollyZoneid = vecZone.begin()->dwZoneId;
  n_bNeedSort = false;
  XLOG << "[斗技场-负载排名更新] zoneid" << m_dwPollyZoneid << XEND;
}

bool MatchServer::getMinZones(DWORD num, std::list<DWORD>& minZones)
{
  if (m_mapZoneInfo.empty())
    return false;

  std::vector<PollyZoneInfo> vec;
  vec.reserve(m_mapZoneInfo.size());

  for (auto it = m_mapZoneInfo.begin(); it != m_mapZoneInfo.end(); ++it)
    vec.push_back(it->second);

  std::sort(vec.begin(), vec.end(), CmpByValue1());
  DWORD size = vec.size();
  for (DWORD i = 0; i < num; ++i)
  {
    minZones.push_back(vec[i % size].dwZoneId);
  }

  return true;
}

DWORD MatchServer::getAZoneId()
{   
  return m_dwPollyZoneid;
}

PollyZoneInfo* MatchServer::getZoneInfo(DWORD zoneId)
{
  auto it = m_mapZoneInfo.find(zoneId);
  if (it == m_mapZoneInfo.end())
    return nullptr;
  return &(it->second);
}

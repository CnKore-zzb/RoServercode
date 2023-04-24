#include <iostream>
#include "GZoneServer.h"
#include "xNetProcessor.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "BaseConfig.h"
#include "RedisManager.h"
#include "GZoneManager.h"
#include "ActivityManager.h"

GZoneServer::GZoneServer(OptArgs &args):RegionServer(args),m_oTenSecTimer(10),m_oOneMinTimer(60),m_oTenMinTimer(600),m_oFiveMinTimer(300)
{
}

GZoneServer::~GZoneServer()
{
}

void GZoneServer::v_final()
{
  XLOG << "[GZoneServer]" << "v_final" << XEND;

  GZoneManager::getMe().final();
  RegionServer::v_final();
}

void GZoneServer::v_closeNp(xNetProcessor *np)
{
  if (!np) return;

  RegionServer::v_closeNp(np);
}

void GZoneServer::v_timetick()
{
  RegionServer::v_timetick();

  DWORD curTime = now();
  if (m_oTenSecTimer.timeUp(curTime))
  {
    xTime frameTimer;
    QWORD _e = 0;

    GZoneManager::getMe().timer();

    if (_e > 30 * 1000)
      XLOG << "[帧耗时]" << "gzone timer" << _e << " 微秒" << XEND;

    if (m_oOneMinTimer.timeUp(curTime))
    {
      GZoneManager::getMe().loadGRegionConfig();
      if (m_oTenMinTimer.timeUp(curTime))
      {
        _e = frameTimer.uElapse();
        GZoneManager::getMe().save();
        if (_e > 30 * 1000)
          XLOG << "[帧耗时]" << "gzone save" << _e << " 微秒" << XEND;
      }
    }

    if (m_oFiveMinTimer.timeUp(curTime))
    {
      ActivityManager::getMe().load();
      ActivityEventManager::getMe().load();
    }
  }
}

bool GZoneServer::v_init()
{
  const ServerData& rServerData = xServer::getServerData();
  const TIpPortPair* pSelfPair = rServerData.getIpPort("GZoneServer");
  if (!pSelfPair)
  {
    XERR << "[启动]，找不到ip port， GZoneServer" << XEND;
    return false;
  }
  setServerPort(pSelfPair->second);

  if (!loadConfig())
    return false;

  if (!listen())
  {
    XERR << "[监听]" << pSelfPair->second << "失败" << XEND;
    return false;
  }

  return true;
}

bool GZoneServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }

  const xLuaData& data = getBranchConfig().getData("Redis");
  if (RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port")) == false)
  {
    XERR << "[RedisManager]" << "init error!" << XEND;
    bResult = false;
  }

  GZoneManager::getMe().load();
  ActivityManager::getMe().load();
  ActivityEventManager::getMe().load();

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool GZoneServer::sendCmdToProxy(DWORD proxyid, const void *buf, WORD len)
{
  /*
  Cmd::ForwardToUserSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  PROTOBUF(message, send, len2);
  return sendCmdToZone(zoneid, send, len2);
  */
  return true;
}

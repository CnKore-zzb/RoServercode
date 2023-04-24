#include "RecordServer.h"
#include "xXMLParser.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "RecordUserManager.h"
#include "TableManager.h"
#include "BaseConfig.h"
#include "GatewayCmd.h"
#include "RecordTradeMgr.h"
#include "ConfigManager.h"
#include "RedisManager.h"
#include "LuaManager.h"
//#include "RecordGCityManager.h"
#include "RecordTool.h"

//RecordServer *thisServer = 0;

RecordServer::RecordServer(OptArgs &args):ZoneServer(args), m_oTickOneHour(60*60)
{
}

RecordServer::~RecordServer()
{
}

void RecordServer::v_final()
{
  XLOG << "[" << getServerName() << "],v_final" << XEND;

  while (!v_stop())
  {
    sleep(1);
  }

  RecordUserManager::getMe().final();
  ZoneServer::v_final();

  m_oRollbackThread.thread_stop();
  m_oRollbackThread.thread_join();
}

void RecordServer::v_verifyOk(xNetProcessor *task)
{
  ZoneServer::v_verifyOk(task);

  if (task == nullptr)
    return;

  /*
  if (strncmp(task->name, "SceneServer", 11) == 0)
    RecordGCityManager::getMe().syncCityInfo();
    */
}

bool RecordServer::loadConfig()
{
  bool bResult = true;

  // lua
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }
  // base配置
  if (ConfigManager::getMe().loadRecordConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool RecordServer::v_init()
{
  if (!addDataBase(REGION_DB, true))
  {
    XERR << "[数据库Trade],初始化数据库连接失败:" << REGION_DB << XEND;
    return false;
  }
  if (!loadConfig())
    return false;

  patch();

  /*
  if (RecordGCityManager::getMe().loadCityInfo() == false)
    return false;
    */

  if (getBranchConfig().has("RollbackDataBase"))
  {
    if (!initDBConnPool("RollbackDataBase", getRegionDBName(), m_oRollbackThread.getDBConnPool()))
    {
      XERR << "[m_oRollbackThread]" << "创建失败" << XEND;
      return false;
    }
    if (!m_oRollbackThread.thread_start())
    {
      XERR << "[m_oRollbackThread]" << "创建失败" << XEND;
      return false;
    }
    XLOG << "[m_oRollbackThread]" << "创建成功" << XEND;
  }
  else
  {
    XLOG << "[m_oRollbackThread]" << "未配置数据库" << XEND;
  }

  return true;
}

void RecordServer::v_closeServer(xNetProcessor *np)
{
  if (!np) return;

  if (strcmp(np->name,"SessionServer")==0)
  {
    stop();
  }
  else if (strcmp(np->name,"SuperServer")==0)
  {

  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {
  }

}

bool RecordServer::v_stop()
{
  if (!m_oServerList.empty())
  {
    for (auto it : m_oServerList)
    {
      for (auto sub_it : it.second)
      {
        if (sub_it.second->getTask() && sub_it.second->getTask()->isTask())
        {
          XLOG << "[" << getServerName() << "], 等待" << sub_it.second->getTask()->name << "断开连接" << XEND;
          sleep(5);
          return false;
        }
      }
    }
  }
  return true;
}

bool RecordServer::doGatewayCmd(const BYTE* buf, WORD len)
{
  GatewayCmd* cmd = (GatewayCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;


  return true;
}

void RecordServer::patch()
{
  //RecordUserManager::getMe().patch();
}


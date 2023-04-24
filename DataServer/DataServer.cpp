#include <iostream>
#include "DataServer.h"
#include "RegCmd.h"
#include "xNetProcessor.h"
#include "xExecutionTime.h"
#include "RegCmd.h"
#include "SystemCmd.pb.h"
#include "MsgManager.h"
#include "GuildPhotoLoader.h"
#include "ConfigManager.h"
#include "RecordGCityManager.h"

DataServer::DataServer(OptArgs &args):ZoneServer(args)
{
}

DataServer::~DataServer()
{
}

bool DataServer::v_init()
{
  // base配置
  if (ConfigManager::getMe().loadDataConfig() == false)
    CHECK_LOAD_CONFIG("ConfigManager");
  if (RecordGCityManager::getMe().loadCityInfo() == false)
    return false;
  return true;
}

bool DataServer::v_stop()
{
  XLOG << "[" << getServerName() << "],v_stop" << XEND;

  return true;
}

void DataServer::v_timetick()
{
  ZoneServer::v_timetick();
  DWORD curSec = xTime::getCurSec();
  GuildPhotoLoader::getMe().timer(curSec);
}

void DataServer::v_final()
{
  XLOG << "[主循环]" << "v_final" << XEND;

  ZoneServer::v_final();
}

void DataServer::v_closeServer(xNetProcessor *np)
{
  if (!np) return;
  if (strcmp(np->name,"SessionServer")==0)
  {

  }
  else if (strcmp(np->name,"RecordServer")==0)
  {

  }
  else if (strncmp(np->name,"GateServer", 10)==0)
  {
  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {

  }
}

void DataServer::v_verifyOk(xNetProcessor *np)
{
  if (!np) return;
  XLOG << "[VerifyOk]," << np->name << XEND;
  if (strcmp(np->name,"SessionServer")==0)
  {
  }
  else if (strcmp(np->name,"RecordServer")==0)
  {

  }
  else if (strncmp(np->name,"GateServer", 10)==0)
  {
  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {
    RecordGCityManager::getMe().syncCityInfo();
  }
}

void DataServer::initOkServer(ServerTask *task)
{
  if (!task) return;

  ZoneServer::initOkServer(task);
}

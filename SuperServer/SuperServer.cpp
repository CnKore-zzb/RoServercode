#include <iostream>
#include "SuperServer.h"
#include "RegCmd.h"
#include "xXMLParser.h"
#include "xNetProcessor.h"
#include "xExecutionTime.h"
#include "RegCmd.h"
#include "SystemCmd.pb.h"
#include "GateInfo.h"
#include "MsgManager.h"

//SuperServer *thisServer = 0;

SuperServer::SuperServer(OptArgs &args):ZoneServer(args),m_oMonitorTimer(5)
{
}

SuperServer::~SuperServer()
{
}

bool SuperServer::v_init()
{
  GateInfoM::getMe().init();

  return true;
}

bool SuperServer::v_stop()
{
  XLOG << "[" << getServerName() << "],v_stop" << XEND;

  return true;
}

void SuperServer::v_timetick()
{
  ZoneServer::v_timetick();

  if (m_oMonitorTimer.timeUp(now()))
  {
    monitor();
  }
}

void SuperServer::v_final()
{
  XLOG << "[主循环]" << "v_final" << XEND;

  GateInfoM::getMe().delAll();

  ZoneServer::v_final();
}

void SuperServer::v_closeServer(xNetProcessor *np)
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
    GateInfoM::getMe().delGate(np);
  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {

  }

  if (m_blZoneReady && getServerState()==ServerState::run)
  {
    std::ostringstream stream;
    stream.str("");
    stream << np->name << ",断开连接";
    //MsgManager::alter_msg(getFullName(), stream.str(), EPUSHMSG_CORE_DUMP);
    alter_msg(getFullName(), stream.str(), EPUSHMSG_CORE_DUMP);
  }
}

void SuperServer::v_verifyOk(xNetProcessor *np)
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
    if (m_blZoneReady)
    {
      GateInfoM::getMe().addGate(np);
    }
  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {

  }
}

void SuperServer::initOkServer(ServerTask *task)
{
  if (!task) return;

  task->setState(ServerTask_State::okay);

  for (auto it : m_oServerConfig)
  {
    if ("SuperServer" == it.first) continue;

    if (!checkConnectedServer(it.first))
      return;

    for (auto iter : it.second)
    {
      ServerTask *task = getConnectedServer(it.first, iter.first);
      if (task)
      {
        if (task->getState() != ServerTask_State::okay)
          return;
      }
    }
  }
  if (false == m_blZoneReady)
  {
    XLOG << "[初始化完成],state:" << (DWORD)getServerState() << XEND;
    /*
    if (xServer::isOuter())
    {
      pushover(getFullName(), "服务器启动成功");
    }
    */
    std::cout << "服务器启动成功,开始接收客户端登录" << std::endl;

    std::vector<ServerTask *> vec;
    if (getConnectedServer("GateServer", "", vec))
    {
      for (auto &iter : vec)
      {
        if (iter->getTask())
          GateInfoM::getMe().addGate(iter->getTask());
      }
    }
    m_blZoneReady = true;
  }
}

std::string getLinuxUserName()
{
  std::string str;
  char buf[MAX_NAMESIZE];
  if (0 == getlogin_r(buf, sizeof(buf)))
    str = std::string(buf, sizeof(buf));

  return str;
}

std::string getLinuxHostName()
{
  std::string str;
  char buf[MAX_NAMESIZE];
  if (0 == gethostname(buf, sizeof(buf)))
    str = std::string(buf, sizeof(buf));

  return str;
}

const std::string MSG_STR = "http://sdk.4001185185.com/sdk/smssdk!mt.action?sdk=18602107309&code=f1130a70e882d21e9e114e54e57b1154&pwdtype=md5&phones=18616664977,13818156950,15800551846&msg=";

void SuperServer::monitor()
{
  if (!thisServer || getServerState() >= ServerState::stop)
    return;

  Cmd::HeartBeatSystemCmd message;
  PROTOBUF(message, send, len);

  for (auto oIter : m_oServerConfig)
  {
    if (strcmp("SuperServer", oIter.first.c_str()) == 0)  continue;

    for (auto iIter = oIter.second.begin(); iIter != oIter.second.end(); ++iIter)
    {
      ServerTask *task = getConnectedServer(oIter.first, iIter->first);
      if (task)
      {
        if (task->check())
        {
          task->sendCmd(send, len);
        }
        else
        {
          XERR << "[心跳]," << oIter.first.c_str() << "," << iIter->first.c_str() << ",没有响应" << XEND;
        }
      }
    }
  }
}

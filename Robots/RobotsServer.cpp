#include "RobotsServer.h"
#include "Robot.h"
#include "RobotManager.h"
#include "xNetProcessor.h"

//ProxyServer* thisServer = 0;

RobotsServer::RobotsServer(OptArgs &args) : xServer(args)
{
}

void RobotsServer::v_final()
{
}

bool RobotsServer::init()
{
  RobotManager::getMe().init();
  return true;
}

void RobotsServer::v_closeNp(xNetProcessor* np)
{
  if (!np) return;

  RobotManager::getMe().onClose(np);
}

bool RobotsServer::v_callback()
{
  switch (getServerState())
  {
    case ServerState::create:
      {
        setServerState(ServerState::init);

        return true;
      }
      break;
    case ServerState::init:
      {
        if (!init()) stop();

        setServerState(ServerState::run);

        return true;
      }
      break;
    case ServerState::run:
      {
        return true;
      }
      break;
    case ServerState::stop:
    case ServerState::finish:
      {
        return false;
      }
      break;
    default:
      break;
  }
  return false;
}

bool RobotsServer::doCmd(xNetProcessor* np, unsigned char* buf, unsigned short len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    /*
    case Cmd::LOGIN_USER_PROTOCMD:
      {
        return doLoginUserCmd(np, buf, len);
      }
      break;
    case Cmd::SYSTEM_PROTOCMD:
      {
        using namespace Cmd;
        switch (cmd->param)
        {
        case COMMON_RELOAD_SYSCMD:
        {
          PARSE_CMD_PROTOBUF(CommonReloadSystemCmd, message);
          commonReload(message);
          return true;
        }
        break;
        }
      }
      break;
      */
    default:
      break;
  }
  return false;
}

void RobotsServer::v_timetick()
{
  xTime oFrameTimer;

  xServer::v_timetick();

  QWORD _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-timetick]," << _e << " 微秒" << XEND;

  oFrameTimer.elapseStart();
  process();
  _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-process]," << _e << " 微秒" << XEND;

  oFrameTimer.elapseStart();
  RobotManager::getMe().timer();
  _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-usertimer]," << _e << " 微秒" << XEND;
}

void RobotsServer::process()
{
  RobotManager::getMe().process();
}

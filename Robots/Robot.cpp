#include <arpa/inet.h>
#include "Robot.h"
#include "RobotsServer.h"
#include "RobotManager.h"
#include "xNetProcessor.h"
#include "ClientCmd.h"
#include "LoginUserCmd.pb.h"
#include "SceneUser2.pb.h"
#include "xlib/xSha1.h"

Robot::Robot(QWORD accid, DWORD regionid) : m_oTenSecTimer(10), m_bBoothOpen(false)
{
  m_qwAccID = accid;
  id = accid;
  m_dwRegionID = regionid;
  setState(Robot_State::create);
}

Robot::~Robot()
{
  if (m_pTask)
  {
    thisServer->addCloseList(m_pTask, TerminateMethod::terminate_active, "Robot析构");
    m_pTask = nullptr;
  }
  XLOG << "[Robot]" << m_qwAccID << id << name << "析构" << XEND;
}

bool Robot::sendCmdToServer(void* cmd, unsigned short len)
{
  if (Robot_State::quit == getState())
  {
    return false;
  }
  if (m_pTask)
  {
    xCommand *pCmd = (xCommand *)cmd;

    BUFFER_CMD(send, xCommand);
    send->cmd = pCmd->cmd;
    send->param = pCmd->param;
    WORD *noncelen = (WORD *)(send->probuf);
    Nonce mess;
    DWORD cur = now();
    if (m_dwNonceTime != cur)
    {
      m_dwNonceIndex = 0;
      m_dwNonceTime = cur;
    }

    mess.set_timestamp(m_dwNonceTime);
    mess.set_index(++m_dwNonceIndex);

    std::stringstream ss;
    ss << mess.timestamp() << "_" << mess.index() << "_" << "!^ro&";

    char sha1[SHA1_LEN + 1];
    bzero(sha1, sizeof(sha1));
    getSha1Result(sha1, ss.str().c_str(), ss.str().size());
    mess.set_sign(sha1);

    *noncelen = mess.ByteSize();
    if (!mess.SerializeToArray(&(send->probuf[2]), *noncelen))
    {
      return false;
    }
    bcopy(pCmd->probuf, &(send->probuf[*noncelen + 2]), len - 2);
    return m_pTask->sendCmd(send, len + *noncelen + 2);
  }
  else
  {
#ifdef _LX_DEBUG
    XLOG << "[forwardTask]" << m_qwAccID << id << name << "发送失败" << XEND;
#endif
  }
  return false;
}

// 处理游戏服务器返回的消息
bool Robot::doServerCmd(xCommand *cmd, unsigned short len)
{
  if (!cmd || !len) return true;

//  std::cout << m_qwAccID << " 收到消息:" << (DWORD)cmd->cmd << "," << (DWORD)cmd->param  << std::endl;

  switch (cmd->cmd)
  {
    case SCENE_USER_SEAL_PROTOCMD:
      {
        return doSceneUserSealCmd((UserCmd *)cmd, len);
      }
      break;
    case SESSION_USER_SOCIALITY_PROTOCMD:
      {
        return doSessionUserSocialityCmd((UserCmd *)cmd, len);
      }
      break;
    case SESSION_USER_WEATHER_PROTOCMD:
      {
        return doSessionUserWeatherCmd((UserCmd *)cmd, len);
      }
      break;
    case FUBEN_PROTOCMD:
      {
        return doFubenCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_MAP_PROTOCMD:
      {
        return doSceneUserMapCmd((UserCmd *)cmd, len);
      }
      break;
    case SESSION_USER_GUILD_PROTOCMD:
      {
        return doSessionUserGuildCmd((UserCmd *)cmd, len);
      }
      break;
    case ACTIVITY_PROTOCMD:
      {
        return doActivityCmd((UserCmd *)cmd, len);
      }
      break;
    case SESSION_USER_MAIL_PROTOCMD:
      {
        return doSessionUserMailCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_ASTROLABE_PROTOCMD:
      {
        return doSceneUserAstrolabeCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_FOOD_PROTOCMD:
      {
        return doSceneUserFoodCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_PHOTO_PROTOCMD:
      {
        return doSceneUserPhotoCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_ITEM_PROTOCMD:
      {
        return doSceneUserItemCmd((UserCmd *)cmd, len);
      }
      break;
    case USER_EVENT_PROTOCMD:
      {
        return doUserEventCmd((UserCmd *)cmd, len);
      }
      break;
    case INFINITE_TOWER_PROTOCMD:
      {
        return doInfiniteTowerCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_ACHIEVE_PROTOCMD:
      {
        return doSceneUserAchieveCmd((UserCmd *)cmd, len);
      }
      break;
    case LOGIN_USER_PROTOCMD:
      {
        return doLoginUserCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_PROTOCMD:
      {
        return doSceneUserCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER2_PROTOCMD:
      {
        return doSceneUser2Cmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_SKILL_PROTOCMD:
      {
        return doSceneSkillCmd((UserCmd *)cmd, len);
      }
      break;
    case SCENE_USER_QUEST_PROTOCMD:
      {
        return doSceneQuestCmd((UserCmd *)cmd, len);
      }
      break;
    case ERROR_USER_PROTOCMD:
      {
        return doErrRegUserCmd((UserCmd *)cmd, len);
      }
      break;
    case CLIENT_CMD:
      {
        switch (cmd->param)
        {
          // 消息集合 直接转发
          case CMDSET_USER_CMD:
            {
              Cmd::CmdSetUserCmd *rev = (Cmd::CmdSetUserCmd *)cmd;

              DWORD place = 0;

              for (DWORD i=0; i<rev->num; ++i)
              {
                WORD size = *(WORD *)(rev->data + place);
                place += sizeof(WORD);

                xCommand *buf = (xCommand *)(rev->data + place);

                // if (!buf->cmd && !buf->param)
                //{
                 // std::cout << m_qwAccID << " 消息集合:" << (DWORD)rev->num << "," << len << "," << place << std::endl;
               // }

                if (!doServerCmd(buf, size))
                {
                  // std::cout << id << " 消息处理错误:" << (DWORD)buf->cmd << "," << (DWORD)buf->param << "," << size << std::endl;
                }
                place += size;
              }

              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

void Robot::setState(Robot_State state)
{
  m_oRobotState = state;
  switch (m_oRobotState)
  {
    case Robot_State::create:
      XLOG << "[Robot_State]" << m_qwAccID << id << name << this << "创建" << XEND;
      break;
    case Robot_State::running:
      XLOG << "[Robot_State]" << m_qwAccID << id << name << this << "运行" << XEND;
      break;
    case Robot_State::quit:
      XLOG << "[Robot_State]" << m_qwAccID << id << name << this << "退出" << XEND;
      break;
    default:
      break;
  }
}

void Robot::timer(DWORD cur)
{
  switch (m_oRobotState)
  {
    case Robot_State::create:
      {
        connectServer();
        setState(Robot_State::running);
        ReqLoginUserCmd message;
        message.set_accid(m_qwAccID);
        message.set_timestamp(now());
        std::stringstream ss;
        ss << message.accid() << "_" << message.timestamp() << "_" << "rtyuio@#$%^&";

        char sha1[SHA1_LEN + 1];
        bzero(sha1, sizeof(sha1));
        getSha1Result(sha1, ss.str().c_str(), ss.str().size());
        message.set_sha1(sha1);

        message.set_zoneid(m_dwRegionID);
        PROTOBUF(message, send, len);

        std::cout<<"[Robot]send cmd to proxy, m_qwAccID:"<<m_qwAccID<<" m_dwRegionID:"<<m_dwRegionID<<" timestamp:"<<now()<<" sha1:"<<sha1<<std::endl;

        sendCmdToServer(send, len);
      }
      break;
    case Robot_State::running:
      {
      }
      break;
    case Robot_State::quit:
      break;
    default:
      break;
  }
  {
    HeartBeatUserCmd message;
    PROTOBUF(message, send, len);
    sendCmdToServer(send, len);
  }

  if(m_oTenSecTimer.timeUp(cur))
  {
    if(!hasBoothOpen())
    {
      Cmd::BoothReqUserCmd cmd;
      cmd.set_name(std::to_string(m_qwAccID));
      cmd.set_oper(Cmd::EBOOTHOPER_OPEN);

      PROTOBUF(cmd, send, len);
      sendCmdToServer(send, len);
    }
  }
}

void Robot::connectServer()
{
  if (m_pTask) return;

  m_pTask = thisServer->newClient();

  if (!m_pTask->connect(RobotManager::getMe().getProxyIp().c_str(), RobotManager::getMe().getProxyPort()))
  {
    std::cout<<"conncet proxy server failed! ip:"<< RobotManager::getMe().getProxyIp() << " port:" << RobotManager::getMe().getProxyPort() <<std::endl;
    SAFE_DELETE(m_pTask);
  }
  else
  {
    std::cout<<"conncet proxy server success! ip:"<< RobotManager::getMe().getProxyIp() << " port:" << RobotManager::getMe().getProxyPort() <<std::endl;
    thisServer->getTaskThreadPool().add(m_pTask);
  }
}

#include "Robot.h"
#include "SceneUser.pb.h"
#include "RobotManager.h"

bool Robot::doSceneUserCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    case SYS_TIME_USER_CMD:
    case USERPARAM_USERSYNC:
      {
        return true;
      }
      break;
    case CHANGE_SCENE_USER_CMD:
      {
        sendCmdToServer(buf, len);

        std::cout << m_qwAccID << " 进入场景" << std::endl;
        EFuncType eType = RobotManager::getMe().getFuncType();
        std::cout << m_qwAccID << " 测试类型：" << eType << std::endl;
        switch(eType)
        {
          case EFUNCTYPE_SKILL:
            {
              GoToExitPosUserCmd message;
              if (RobotManager::getMe().getMapID())
              {
                message.set_mapid(RobotManager::getMe().getMapID());
              }
              PROTOBUF(message, send, len);
              sendCmdToServer(send, len);
            }
            break;
          case EFUNCTYPE_BOOTH:
            {
              GoToRandomPosUserCmd cmd;
              DWORD mapId = RobotManager::getMe().getMapID();
              ScenePos pos;
              RobotManager::getMe().getScenePos(pos);

              cmd.set_mapid(mapId);
              cmd.mutable_pos()->CopyFrom(pos);
              std::cout << m_qwAccID << " goto mapId: " << mapId << " pos: (" << pos.x() << "," << pos.y() << "," << pos.z() << ")" << std::endl;

              PROTOBUF(cmd, send1, len1);
              sendCmdToServer(send1, len1);
            }
            break;
          default:
            break;
        }

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

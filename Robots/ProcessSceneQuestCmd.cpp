#include "Robot.h"
#include "SceneQuest.pb.h"

bool Robot::doSceneQuestCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    case QUESTPARAM_QUERYOTHERDATA:
      {
        return true;
      }
      break;
    case QUESTPARAM_QUESTLIST:
      {
        PARSE_CMD_PROTOBUF(QuestList, message);

        // std::cout << m_qwAccID << " [任务列表]:" << message.ShortDebugString() << std::endl;

        // return true;

        // std::cout << m_qwAccID << " [任务列表]:类型," << (DWORD)message.type() << ";id," << message.id() << std::endl;

        for (int i = 0; i < message.list_size(); ++i)
        {
          const QuestData &data = message.list(i);
          // std::cout << m_qwAccID << " [任务列表]:" << data.ShortDebugString() << std::endl;
          XLOG << "[任务列表]" << m_qwAccID << data.ShortDebugString() << XEND;
          /*
          for (int j = 0; j < data.steps_size(); ++j)
          {
            const QuestStep &step = data.steps(j);
            std::cout << "step:" << j << ":" << step.ShortDebugString() << ";" << std::endl;
          }
          */
        }

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

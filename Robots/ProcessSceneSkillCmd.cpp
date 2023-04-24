#include "Robot.h"
#include "SceneSkill.pb.h"

bool Robot::doSceneSkillCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    case SKILLPARAM_SKILLUPDATE:
      {
        PARSE_CMD_PROTOBUF(SkillUpdate, message);
        return true;

        for (int i = 0; i < message.update_size(); ++i)
        {
          const SkillData &data = message.update(i);
          std::cout << m_qwAccID << " 技能更新:" << data.usedpoint() << "," << data.profession() << std::endl;
          if (data.items_size())
          {
            std::cout << m_qwAccID << " 技能更新:";
            for (int j = 0; j < data.items_size(); ++j)
            {
              const SkillItem &item = data.items(j);
              std::cout << item.id() << ",";
            }
            std::cout << std::endl;
          }
        }

        for (int i = 0; i < message.del_size(); ++i)
        {
          const SkillData &data = message.del(i);
          std::cout << m_qwAccID << " 技能删除:" << data.usedpoint() << "," << data.profession() << std::endl;
          if (data.items_size())
          {
            std::cout << m_qwAccID << " 技能删除:";
            for (int j = 0; j < data.items_size(); ++j)
            {
              const SkillItem &item = data.items(j);
              std::cout << item.id() << ",";
            }
            std::cout << std::endl;
          }
        }

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

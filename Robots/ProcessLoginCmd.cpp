#include "Robot.h"
#include "LoginUserCmd.pb.h"

bool Robot::doLoginUserCmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    case SNAPSHOT_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(SnapShotUserCmd, rev);

        std::cout << m_qwAccID << " 收到快照数据" << std::endl;

        for (int i = 0; i < rev.data_size(); ++i)
        {
          const SnapShotDataPB &data = rev.data(i);
          if (data.id())
          {
            SelectRoleUserCmd message;
            message.set_id(data.id());
            PROTOBUF(message, send, len);
            sendCmdToServer(send, len);

            std::cout << m_qwAccID << " 选择角色:" << data.id() << std::endl;

            // break;
            return true;
          }
        }

        stringstream sstr;
        sstr << "r" << m_qwAccID;
        CreateCharUserCmd createCmd;
        createCmd.set_name(sstr.str());
        createCmd.set_role_sex(randBetween(EGENDER_MALE, EGENDER_FEMALE));
        createCmd.set_profession(randBetween(EPROFESSION_WARRIOR, EPROFESSION_RUNEKNIGHT));
        createCmd.set_hair(randBetween(2, 2));
        createCmd.set_haircolor(randBetween(2, 2));
        createCmd.set_clothcolor(randBetween(1, 1));
        createCmd.set_sequence(1);
        PROTOBUF(createCmd, send, len);
        sendCmdToServer(send, len);

        return true;
      }
      break;
    case SAFE_DEVICE_USER_CMD:
    case CONFIRM_AUTHORIZE_USER_CMD:
    case LOGIN_RESULT_USER_CMD:
    case HEART_BEAT_USER_CMD:
      {
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

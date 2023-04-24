#include "Robot.h"
#include "ErrorUserCmd.pb.h"

bool Robot::doErrRegUserCmd(UserCmd* buf, unsigned short len)
{
  if(!buf || !len) return false;
  switch(buf->param)
  {
    case REG_ERR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(RegErrUserCmd, rev);
        std::cout<<"[Robot] doErrRegUserCmd, cmd:"<<rev.ShortDebugString()<<std::endl;
      }
      break;
    default:
      break;
  }
  return false;
}

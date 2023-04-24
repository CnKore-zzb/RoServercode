#include "Robot.h"
#include "SceneUser2.pb.h"

bool Robot::doSceneUser2Cmd(Cmd::UserCmd* buf, unsigned short len)
{
  if (!buf || !len) return false;
  switch (buf->param)
  {
    case USER2PARAM_PRESETCHATMSG:
    case USER2PARAM_VAR:
    case USER2PARAM_MENU:
    //case USER2PARAM_NEWHAIR:
    case USER2PARAM_QUERYFIGHTERINFO:
    case USER2PARAM_GOTO_LIST:
    case USER2PARAM_QUERY_MAPAREA:
    case USER2PARAM_QUERY_ACTION:
    case USER2PARAM_QUERY_TRACE_LIST:
    case USER2PARAM_DOWNLOAD_SCENERY_PHOTO:
    case USER2PARAM_UPYUN_AUTHORIZATION:
    case USER2PARAM_SYSMSG:
    case USER2PARAM_NTF_VISIBLENPC:
    case USER2PARAM_GAMETIME:
    case USER2PARAM_SCENERY:
    case USER2PARAM_EFFECT:
    case USER2PARAM_SOUNDEFFECT:
    case USER2PARAM_BUFFERSYNC:
    case USER2PARAM_TALKINFO:
      {

        return true;
      }
      break;
    case USER2PARAM_USER_BOOTH_REQ:
      {
        PARSE_CMD_PROTOBUF(Cmd::BoothReqUserCmd, rev);
        if(rev.success())
        {
          setBoothOpen(true);
          std::cout << m_qwAccID << " 摊位开启成功" << std::endl;
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

#include "GZoneServer.h"
#include "MiscConfig.h"
#include "GZoneCmd.pb.h"
#include "GZoneManager.h"

bool GZoneServer::doGZoneCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case GZONEPARAM_UPDATEA_ACTIVE_ONLINE:
      {
        PARSE_CMD_PROTOBUF(UpdateActiveOnlineGZoneCmd, rev);

        GZoneManager::getMe().update(rev.zoneid(), rev.active(), rev.online());

        return true;
      }
      break;
    default:
      return false;
  }

  return true;
}

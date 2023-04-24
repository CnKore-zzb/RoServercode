#pragma once
#include "UserCmd.h"
//#include "Define.h"
#pragma pack(1)

namespace Cmd
{
  struct ClientCmd : public UserCmd
  {
    ClientCmd(unsigned char p=0) : UserCmd(CLIENT_CMD, p)
    {
    }
  };

// data:len|cmd|len|cmd.......
#define CMDSET_USER_CMD 1
  struct CmdSetUserCmd : public ClientCmd
  {
    WORD num;
    BYTE data[0];
    CmdSetUserCmd():ClientCmd(CMDSET_USER_CMD)
    {
      num = 0;
    }
  };

#define REFRESH_SNAPSHOT_CLIENT_USER_CMD 2
  struct RefreshSnapshotClientUserCmd : public ClientCmd
  {
    RefreshSnapshotClientUserCmd() : ClientCmd(REFRESH_SNAPSHOT_CLIENT_USER_CMD)
    {
    }
  };

#define SELECT_CHAR_CLIENT_USER_CMD 3
  struct SelectCharClientUserCmd : public ClientCmd
  {
    QWORD charid = 0;
    SelectCharClientUserCmd() : ClientCmd(SELECT_CHAR_CLIENT_USER_CMD)
    {
    }
  };

#define RECONNECT_CLIENT_USER_CMD 4
  struct ReconnectClientUserCmd : public ClientCmd
  {
    QWORD charid = 0;
    BYTE refresh = 0;
    ReconnectClientUserCmd() : ClientCmd(RECONNECT_CLIENT_USER_CMD)
    {
    }
  };

#define DISCONNECT_CLIENT_USER_CMD 5
  struct DisconnectClientUserCmd : public ClientCmd
  {
    DisconnectClientUserCmd() : ClientCmd(DISCONNECT_CLIENT_USER_CMD)
    {
    }
  };
};

#pragma pack()

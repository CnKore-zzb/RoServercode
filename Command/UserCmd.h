#pragma once
#include "xCommand.h"
#include "UserData.h"
#pragma pack(1)

#define LOGIN_USER_CMD 1
#define SCENE_USER_CMD 2
#define DATA_USER_CMD 3

namespace Cmd
{
  struct UserCmd : public xCommand
  {
    UserCmd(BYTE c, BYTE p) : xCommand(c, p)
    {
    }
  };
}

#pragma pack()

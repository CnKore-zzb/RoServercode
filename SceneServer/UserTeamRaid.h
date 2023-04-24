#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"

class SceneUser;

class UserTeamRaid : private xNoncopyable
{
  public:
    UserTeamRaid(SceneUser *u);
    ~UserTeamRaid();

  private:
    //SceneUser *m_pUser = NULL;
};

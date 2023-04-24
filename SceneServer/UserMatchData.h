#pragma once

#include "xDefine.h"
#include "MiscConfig.h"

using namespace Cmd;

class SceneUser;

class UserMatchData
{
  public:
    UserMatchData(SceneUser* user);
    virtual ~UserMatchData();

    void setPwsSeason(DWORD season) { m_dwTeamPwsSeason = season; }
    DWORD getPwsSeason() { return m_dwTeamPwsSeason; }

    void setPwsScore(DWORD score) { m_dwTeamPwsScore = score; }
    DWORD getPwsScore() { return m_dwTeamPwsScore; }
    ETeamPwsRank getPwsRank();
  private:
    SceneUser* m_pUser;
    DWORD m_dwTeamPwsSeason = 0; // 当前赛季
    DWORD m_dwTeamPwsScore = 0; // 玩家积分
};


#include "SceneUser.h"
#include "UserMatchData.h"

UserMatchData::UserMatchData(SceneUser* user) : m_pUser(user)
{

}

UserMatchData::~UserMatchData()
{

}

ETeamPwsRank UserMatchData::getPwsRank()
{
  return MiscConfig::getMe().getTeamPwsCFG().getERankByScore(m_dwTeamPwsScore);
}



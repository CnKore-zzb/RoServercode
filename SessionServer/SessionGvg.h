#pragma once

#include "GuildCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "xDefine.h"
#include "xSingleton.h"

using namespace std;
using namespace Cmd;

struct SCityShowInfo
{
  QWORD qwOldGuildID = 0;
  DWORD dwCityID = 0;
  GuildCityInfo stCityInfo;
  EGCityState eState = EGCITYSTATE_MIN;

  void toData(CityShowInfo* pData);
};

class SessionUser;
class SessionGvg : public xSingleton<SessionGvg>
{
  friend class xSingleton<SessionGvg>;
  private:
    SessionGvg();
  public:
    virtual ~SessionGvg();

    void updateCityInfoFromGuild(const GuildCityActionGuildSCmd& cmd);
    void updateCityInfoFromGuild(const CityDataUpdateGuildSCmd& cmd);
    void updateCityState(const UpdateCityStateGuildSCmd& cmd);

    void queryCityShowInfo(SessionUser* user);
    void setFireStatus(bool fire);
  private:
    bool m_bFire = false;
    map<DWORD, SCityShowInfo> m_mapCityInfo;
};

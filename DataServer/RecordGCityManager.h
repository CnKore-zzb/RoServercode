/**
 * @file RecordGCityManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-10-09
 */

#pragma once

#include "xSingleton.h"
#include "GuildSCmd.pb.h"

using std::map;
using std::pair;
using namespace Cmd;

class RecordGCityManager : public xSingleton<RecordGCityManager>
{
  friend class xSingleton<RecordGCityManager>;
  private:
    RecordGCityManager();
  public:
    virtual ~RecordGCityManager();

    bool loadCityInfo();
    void updateCityInfo(const GuildCityActionGuildSCmd& cmd);
    void syncCityInfo(DWORD dwCityID = 0, EGuildCityResult eResult = EGUILDCITYRESULT_MIN, const string& scenename = "");
    void saveCityResult(const GvgResultGuildSCmd& cmd);
  private:
    map<DWORD, GuildCityInfo> m_mapCityInfo;
};


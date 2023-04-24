#pragma once

#include "xSingleton.h"
#include "UserEvent.pb.h"
#include "MiscConfig.h"

using namespace Cmd;
using std::map;
using std::vector;
using std::string;

//称号
struct SDojoItemCfg{
  DWORD dwDojoId;
  DWORD dwFirstReward;
  DWORD dwGroupId;          //道场组id
  DWORD dwRapid;            //副本地图id
  DWORD dwHelpReward;       //协助通关奖励
  DWORD dwLevel;            //推荐等级
};

typedef map<DWORD, SDojoItemCfg> TMapDojoItem;
typedef map<DWORD, DWORD> TMapDojoRaidGroup;
typedef map<DWORD, TSetDWORD> TMapGroupDojoID;

// config
class DojoConfig : public xSingleton<DojoConfig>
{
  friend class xSingleton<DojoConfig>;
  private:
    DojoConfig();
  public:
    virtual ~DojoConfig();

    bool loadConfig();
    bool checkConfig();
    const SDojoItemCfg* getDojoItemCfg(DWORD dwID) const;
    bool isDojoMap(DWORD dwMapId);
    bool hasGroup(DWORD dwGroupID) const;

    DWORD getGroupIDByRaid(DWORD dwRaid) const;
    void collectDojoID(DWORD dwGroupID, TSetDWORD& setIDs) const;
  private:
    TMapDojoItem m_mapID2Item;
    TMapDojoRaidGroup m_mapRaid2Group;
    TMapGroupDojoID m_mapGroupDojo;
};


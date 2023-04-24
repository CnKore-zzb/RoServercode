#pragma once

#include "xSingleton.h"
#include <map>
#include <vector>
#include <set>
#include "xLuaTable.h"
#include "xPos.h"

enum ETransferType{
  Parent,Child
};

struct SDeathTransferConfig{
  DWORD mapid;
  DWORD deathMapNumber;
  DWORD npcid;
  ETransferType npcType;
  xPos pos;
};

// config
class DeathTransferConfig : public xSingleton<DeathTransferConfig>
{
  friend class xSingleton<DeathTransferConfig>;
private:
  DeathTransferConfig();
public:
  virtual ~DeathTransferConfig();
  bool loadConfig();
  bool checkConfig();

  SDeathTransferConfig* getDeathTransferCfg(DWORD dwNpcid);
  //DWORD getTransferIdByQuestId(DWORD dwQuestid);
  //dwDeathMapNumber是1和2,表示两张地图
  bool isMapAllActivated(DWORD dwDeathMapNumber, const std::set<DWORD>& setActivatedNpcid);
private:
  bool loadDeathTransferConfig();

private:
  std::map<DWORD/*npcid*/, SDeathTransferConfig> m_mapDeathTransferCfg;
  //std::map<DWORD/*questId*/, DWORD/*npcid*/> m_mapQuest2Transfer;
  std::vector<DWORD> m_vecMap1Transferid;
  std::vector<DWORD> m_vecMap2Transferid;
};


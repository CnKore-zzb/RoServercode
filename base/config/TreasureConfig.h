/**
 * @file TreasureConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-09-07
 */

#pragma once

#include "xSingleton.h"
#include "ProtoCommon.pb.h"
#include "SceneUser2.pb.h"
#include "xPos.h"
#include "xLuaTable.h"

using namespace Cmd;
using std::vector;
using std::map;

// config data
struct STreasureNpcProcess
{
  ETreeStatus eType = ETREESTATUS_MIN;

  DWORD dwNpcID = 0;
  DWORD dwCount = 0;
  DWORD dwRewardID = 0;

  xLuaData oNpcData;
};
typedef vector<STreasureNpcProcess> TVecTreasureNpcProcess;

enum ETreeType
{
  ETREETYPE_MIN = 0,
  ETREETYPE_GOLD = 1,
  ETREETYPE_MAGIC = 2,
  ETREETYPE_HIGH = 3,
  ETREETYPE_MAX,
};
typedef vector<xPos> TVecPos;
struct STreasureNpc
{
  DWORD dwNpcID = 0;
  ETreeType eType = ETREETYPE_MIN;

  TVecPos vecPos;
  TVecTreasureNpcProcess vecProcess;

  STreasureNpc() {}
  const STreasureNpcProcess* getProcess(DWORD dwIndex) const;
  DWORD randPosIndex(const TSetDWORD& setExclude) const;
};
typedef map<ETreeType, STreasureNpc> TMapTreasureNpc;

struct STreasureCFG
{
  DWORD dwMapID = 0;

  TMapTreasureNpc mapNpcType;

  STreasureCFG() {}
  const STreasureNpc* getTree(ETreeType eType) const;
};
typedef map<DWORD, STreasureCFG> TMapTreasureCFG;

// config
class TreasureConfig : public xSingleton<TreasureConfig>
{
  friend class xSingleton<TreasureConfig>;
  private:
    TreasureConfig();
  public:
    virtual ~TreasureConfig();

    bool loadConfig();

    const STreasureCFG* getTreasureCFG(DWORD dwMapID) const;
  private:
    TMapTreasureCFG m_mapTreasureCFG;
};


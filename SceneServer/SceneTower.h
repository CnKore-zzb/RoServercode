/**
 * @file SceneTower.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-12-08
 */

#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"

using namespace Cmd;
using std::string;
using std::vector;

// data
struct STowerLayerData
{
  DWORD dwLayer = 0;
  DWORD dwTime = 0;

  bool bRewardGeted = false;

  STowerLayerData() {}

  void fromData(const UserTowerLayer& rLayer);
  void toData(UserTowerLayer* pLayer);
};
typedef vector<STowerLayerData> TVecTowerLayerData;

struct STowerData
{
  DWORD dwOldMaxLayer = 0;
  DWORD dwCurMaxLayer = 0;
  DWORD dwMaxLayer = 0;
  DWORD dwLastWeekMaxLayer = 0;
  TSetDWORD setEverPassLayers;

  TVecTowerLayerData vecPassLayers;
  TVecTowerLayerData vecEverPassLayers;

  void fromData(const UserTowerInfo& rData);
  void toData(UserTowerInfo* pData);

  void reset();
};

// tower
class SceneUser;
class SceneTower
{
  public:
    SceneTower(SceneUser* pUser);
    ~SceneTower();

    bool load(const BlobUserTower& oBlob);
    bool save(BlobUserTower* pBlob);
    void toData(UserTowerInfo* pInfo);

    DWORD getCurMaxLayer() const { return m_stData.dwCurMaxLayer; }
    DWORD getMaxLayer() const { return m_stData.dwMaxLayer; }
    DWORD getPassCount() const { return m_stData.vecPassLayers.size(); }
    DWORD getLastWeekLayer() const { return m_stData.dwLastWeekMaxLayer; }
    DWORD getEverMaxLayer() const;
    bool getEverPassLayer(DWORD dwLayer) const { return m_stData.setEverPassLayers.find(dwLayer) != m_stData.setEverPassLayers.end(); }
    void sendTowerData();

    bool isRewarded(DWORD dwLayer);
    bool passLayer(DWORD dwLayer, bool realPass = true, bool teamCheck = true);
    bool canEnter(DWORD dwLayer);
    void resetData();

    DWORD calcNoMonsterLayer() const;
    void addEverPassLayer(DWORD dwLayer);
  private:
    SceneUser* m_pUser = nullptr;

    STowerData m_stData;
};


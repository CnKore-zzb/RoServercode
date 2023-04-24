#include "SceneTower.h"
#include "SceneUser.h"
#include "Menu.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "StatisticsDefine.h"
#include "ActivityEventManager.h"

// data
void STowerLayerData::fromData(const UserTowerLayer& rLayer)
{
  dwLayer = rLayer.layer();
  dwTime = rLayer.utime();
  bRewardGeted = rLayer.rewarded();
}

void STowerLayerData::toData(UserTowerLayer* pLayer)
{
  if (pLayer == nullptr)
    return;

  pLayer->set_layer(dwLayer);
  pLayer->set_utime(dwTime);
  pLayer->set_rewarded(bRewardGeted);
}

void STowerData::fromData(const UserTowerInfo& rData)
{
  dwOldMaxLayer = rData.oldmaxlayer();
  dwCurMaxLayer = rData.curmaxlayer();
  dwMaxLayer = rData.maxlayer();
  dwLastWeekMaxLayer = rData.record_layer();

  vecPassLayers.clear();
  for (int i = 0; i < rData.layers_size(); ++i)
  {
    DWORD dwLayer = rData.layers(i).layer();
    auto v = find_if(vecPassLayers.begin(), vecPassLayers.end(), [dwLayer](const STowerLayerData& r) -> bool{
      return r.dwLayer == dwLayer;
    });
    if (v != vecPassLayers.end())
    {
      XERR << "[无限塔-加载] layer:" << dwLayer << "duplicated" << XEND;
      continue;
    }

    STowerLayerData stData;
    stData.fromData(rData.layers(i));
    vecPassLayers.push_back(stData);
  }

  vecEverPassLayers.clear();
  for (int i = 0; i < rData.everpasslayers_size(); ++i)
  {
    DWORD dwLayer = rData.everpasslayers(i).layer();
    auto v = find_if(vecEverPassLayers.begin(), vecEverPassLayers.end(), [dwLayer](const STowerLayerData& r) -> bool{
      return r.dwLayer == dwLayer;
    });
    if (v != vecEverPassLayers.end())
    {
      XERR << "[无限塔-加载] everpasslayer:" << dwLayer << "duplicated" << XEND;
      continue;
    }
    setEverPassLayers.insert(dwLayer);

    STowerLayerData stData;
    stData.fromData(rData.everpasslayers(i));
    vecEverPassLayers.push_back(stData);
  }
}

void STowerData::toData(UserTowerInfo* pData)
{
  if (pData == nullptr)
    return;

  pData->set_oldmaxlayer(dwOldMaxLayer);
  pData->set_curmaxlayer(dwCurMaxLayer == 0 ? 1 : dwCurMaxLayer);
  pData->set_maxlayer(dwMaxLayer);
  pData->set_record_layer(dwLastWeekMaxLayer);

  pData->clear_layers();
  for (auto v = vecPassLayers.begin(); v != vecPassLayers.end(); ++v)
  {
    UserTowerLayer* pLayer = pData->add_layers();
    v->toData(pLayer);
  }

  pData->clear_everpasslayers();
  for (auto v = vecEverPassLayers.begin(); v != vecEverPassLayers.end(); ++v)
  {
    UserTowerLayer* pLayer = pData->add_everpasslayers();
    v->toData(pLayer);
  }
}

void STowerData::reset()
{
  dwOldMaxLayer = dwCurMaxLayer = 0;
  vecPassLayers.clear();
}

// tower
SceneTower::SceneTower(SceneUser* pUser) : m_pUser(pUser)
{

}

SceneTower::~SceneTower()
{

}

bool SceneTower::load(const BlobUserTower& oBlob)
{
  m_stData.fromData(oBlob.towerinfo());
  return true;
}

bool SceneTower::save(BlobUserTower* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  m_stData.toData(pBlob->mutable_towerinfo());
  XDBG << "[无限塔-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

void SceneTower::toData(UserTowerInfo* pInfo)
{
  resetData();
  m_stData.toData(pInfo);
}

DWORD SceneTower::getEverMaxLayer() const
{
  if (m_stData.setEverPassLayers.empty() == true)
    return 0;
  return *m_stData.setEverPassLayers.rbegin();
}

void SceneTower::sendTowerData()
{
  UserTowerInfoCmd cmd;
  toData(cmd.mutable_usertower());
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[无限塔-同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步数据" << cmd.ShortDebugString() << XEND;
}

bool SceneTower::isRewarded(DWORD dwLayer)
{
  resetData();

  auto v = find_if(m_stData.vecPassLayers.begin(), m_stData.vecPassLayers.end(), [dwLayer](const STowerLayerData& r) -> bool{
    return r.dwLayer == dwLayer;
  });
  if (v == m_stData.vecPassLayers.end())
    return false;

  return v->bRewardGeted;
}

bool SceneTower::passLayer(DWORD dwLayer, bool realPass /*= true*/, bool teamCheck /*= true*/)
{
  resetData();
  if (teamCheck && m_pUser->getTeamID() == 0)
    return false;

  DWORD dwPreMaxLayer = m_stData.dwCurMaxLayer;
  m_stData.dwCurMaxLayer = dwLayer;
  if (dwLayer > m_stData.dwOldMaxLayer)
    m_stData.dwOldMaxLayer = dwLayer;
  if (dwLayer > m_stData.dwMaxLayer)
  {
    m_stData.dwMaxLayer = dwLayer;
    m_pUser->getQuest().onTowerPass();
  }
  auto v = find_if(m_stData.vecPassLayers.begin(), m_stData.vecPassLayers.end(), [dwLayer](const STowerLayerData& r) -> bool{
    return r.dwLayer == dwLayer;
  });
  if (v != m_stData.vecPassLayers.end())
  {
    if(realPass == true)
      v->bRewardGeted = realPass;
    return false;
  }

  STowerLayerData stData;
  stData.dwLayer = dwLayer;
  stData.dwTime = xTime::getCurSec();
  stData.bRewardGeted = realPass;
  m_stData.vecPassLayers.push_back(stData);
  if(realPass == true && m_stData.setEverPassLayers.find(dwLayer) == m_stData.setEverPassLayers.end())
  {
    addEverPassLayer(dwLayer);
  }

  sendTowerData();

  XLOG << "[恩德勒斯塔-通关]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "通关" << dwLayer << "层" << "获取奖励:" << realPass << XEND;

  m_pUser->getShare().addNormalData(ESHAREDATATYPE_MAX_TOWER, dwLayer);
  m_pUser->getAchieve().onPassTower();
  m_pUser->getTutorTask().onPassTower(dwLayer, dwPreMaxLayer);
  m_pUser->getGuildChallenge().onPassTower(dwLayer);
  m_pUser->getServant().onFinishEvent(ETRIGGER_TOWER_PASS);
  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_TOWER_PASS);
  m_pUser->getEvent().onPassTower();

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Tower_Complete;
  PlatLogManager::getMe().eventLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eid,
    m_pUser->getUserSceneData().getCharge(), eType, 0, 1);

  PlatLogManager::getMe().TowerLog(thisServer,
    m_pUser->getUserSceneData().getPlatformId(),
    m_pUser->getZoneID(),
    m_pUser->accid,
    m_pUser->id,
    eType,
    eid,
    1,
    dwLayer,
    m_stData.dwOldMaxLayer,
    m_pUser->getTeamID(),
    m_pUser->getLevel());

  if (dwLayer % 10 == 0 && dwLayer != 0)
  {
    if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_TOWER_COUNT, dwLayer))
      m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_TOWER_COUNT, 0, dwLayer, m_pUser->getLevel(), (DWORD)1);
  }
  return true;
}

void SceneTower::resetData()
{
  auto reset = [&]()
  {
    m_stData.dwLastWeekMaxLayer = 0;
    DWORD offlinetime = m_pUser->getUserSceneData().getOfflineTime();
    if (now() - offlinetime < WEEK_T)
    {
      DWORD layernum = m_stData.vecPassLayers.size();
      for (auto &v : m_stData.vecPassLayers)
      {
        if (v.dwLayer > layernum * 2)
          continue;
        if (v.dwLayer > m_stData.dwLastWeekMaxLayer)
         m_stData.dwLastWeekMaxLayer = v.dwLayer;
      }  
      
      DWORD recordlayer = MiscConfig::getMe().getEndlessTowerCFG().dwRecordLayer;
      m_stData.dwLastWeekMaxLayer = recordlayer ? m_stData.dwLastWeekMaxLayer / recordlayer * recordlayer : 0;
      XLOG << "[无限塔], 更新上周通过层数,玩家:" << m_pUser->name << m_pUser->id << "上周层数更新为:" << m_stData.dwLastWeekMaxLayer << XEND;
    } 

    m_stData.reset();
    m_pUser->getVar().setVarValue(EVARTYPE_TOWER, 1);
    m_pUser->updateTeamTower();
    sendTowerData();
    
    XLOG << "[恩德勒斯塔-数据重置]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
  };

  DWORD needResetTimes = ActivityEventManager::getMe().getNeedResetTimes(EAEREWARDMODE_TOWER);

  if (m_pUser->getVar().getVarValue(EVARTYPE_TOWER) == 0)
  {
    reset();

    if (needResetTimes)
      m_pUser->getVar().setVarValue(EVARTYPE_TOWER_RESETTIME, needResetTimes);
    else
      m_pUser->getVar().setVarValue(EVARTYPE_TOWER_RESETTIME, 1);
    return;
  }

  if (needResetTimes)
  {
    DWORD resettimes = m_pUser->getVar().getVarValue(EVARTYPE_TOWER_RESETTIME);
    if (resettimes < needResetTimes)
    {
      reset();
      m_pUser->getVar().setVarValue(EVARTYPE_TOWER_RESETTIME, needResetTimes);
    }
  }
}

bool SceneTower::canEnter(DWORD dwLayer)
{
  if (m_stData.vecPassLayers.empty() == true)
    return dwLayer == 1;

  return dwLayer <= m_stData.dwOldMaxLayer + 1;
}

DWORD SceneTower::calcNoMonsterLayer() const
{
  std::set<SceneUser*> userset = m_pUser->getTeamSceneUser();
  DWORD maxlayer = 0;
  for (auto &s : userset)
  {
    DWORD mylayer = s->getTower().getLastWeekLayer();
    if (mylayer > maxlayer)
      maxlayer = mylayer;
  }

  return maxlayer;
}

void SceneTower::addEverPassLayer(DWORD dwLayer)
{
  if(getEverPassLayer(dwLayer) == true)
    return;

  m_stData.setEverPassLayers.insert(dwLayer);
  STowerLayerData stPassData;   //目前只需要保存层数信息
  stPassData.dwLayer = dwLayer;
  m_stData.vecEverPassLayers.push_back(stPassData);

  XLOG << "[恩德勒斯塔-通关] : 通关记录添加 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "通关" << dwLayer << "层" << XEND;
}

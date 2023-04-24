#include "TowerConfig.h"
#include "xLuaTable.h"
#include "NpcConfig.h"
#include "RewardConfig.h"

// config data
void STowerLayerCFG::toData(TowerLayer* pLayer) const
{
  if (pLayer == nullptr)
    return;

  pLayer->set_layer(dwID);

  for (auto v = vecCurMonster.begin(); v != vecCurMonster.end(); ++v)
    pLayer->add_curmonsterids(v->dwMonsterID);
}

// config
TowerConfig::TowerConfig()
{

}

TowerConfig::~TowerConfig()
{

}

bool TowerConfig::loadConfig()
{
  bool bResult = true;
  if (loadEndLessTowerConfig() == false)
    bResult = false;
  if (loadEndLessMonsterConfig() == false)
    bResult = false;

  return bResult;
}

bool TowerConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapTowerLayerCFG.begin(); m != m_mapTowerLayerCFG.end(); ++m)
  {
    // check sky
    if (m->second.dwSky == 0)
    {
      XERR << "[恩德勒斯塔-配置检查] layer :" << m->first << "sky:" << m->second.dwSky << "是一个不合法的天空球" << XEND;
      bCorrect = false;
    }

    // check reward
    if (RewardConfig::getMe().getRewardCFG(m->second.dwRewardID) == nullptr)
    {
      XERR << "[恩德勒斯塔-配置检查] layer :" << m->first << "Reward :" << m->second.dwRewardID << "未在 Table_Reward.txt 表中找到" << XEND;
      bCorrect = false;
    }

    // check npc
    for (auto v = m->second.vecAllMonster.begin(); v != m->second.vecAllMonster.end(); ++v)
    {
      if (NpcConfig::getMe().getNpcCFG(v->dwMonsterID) == nullptr)
      {
        XERR << "[恩德勒斯塔-配置检查] layer :" << m->first << "monsterid :" << v->dwMonsterID << "未在 Table_Monster.txt 表中找到" << XEND;
        bCorrect = false;
      }
    }
  }

  return bCorrect;
}

bool TowerConfig::initDataFromDB(const TowerInfo& rBlob)
{
  m_setKillMonsterIDs.clear();
  for (int i = 0; i < rBlob.killmonsters_size(); ++i)
    m_setKillMonsterIDs.insert(rBlob.killmonsters(i));

  for (int i = 0; i < rBlob.layers_size(); ++i)
  {
    const TowerLayer &layer = rBlob.layers(i);
    auto m = m_mapTowerLayerCFG.find(layer.layer());
    if (m == m_mapTowerLayerCFG.end())
    {
      XERR << "[恩德勒斯塔-列表读取] layer = " << layer.layer() << " 在Table_EndLessTower.txt表中未找到" << XEND;
      continue;
    }

    for (auto v = m->second.vecAllMonster.begin(); v != m->second.vecAllMonster.end(); ++v)
    {
      if (v->bUnLock) continue;

      if (v->dwUnlockCondition == 0 || m_setKillMonsterIDs.find(v->dwUnlockCondition) != m_setKillMonsterIDs.end())
        v->bUnLock = true;
    }

    m->second.vecCurMonster.clear();
    for (int j = 0; j < layer.curmonsterids_size(); ++j)
    {
      DWORD dwMonsterID = layer.curmonsterids(j);
      auto v = find_if(m->second.vecAllMonster.begin(), m->second.vecAllMonster.end(), [dwMonsterID](const STowerMonsterCFG& r) -> bool{
        return r.dwMonsterID == dwMonsterID;
      });
      if (v == m->second.vecAllMonster.end())
      {
        XERR << "[恩德勒斯塔-列表读取] layer = " << layer.layer() << " curmonster = " << dwMonsterID << " 在Table_EndLessMonster.txt" << XEND;
        continue;
      }

      if (!v->bUnLock)
        continue;

      m->second.vecCurMonster.push_back(*v);
    }
  }

  const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
  //m_dwMaxLayer = rBlob.maxlayer() < rCFG.dwDefaultFloor ? rCFG.dwDefaultFloor : rBlob.maxlayer();
  m_dwMaxLayer = rCFG.dwDefaultFloor;
  //m_dwClearTime = rBlob.cleartime();

  XLOG << "[恩德勒斯塔-列表读取] maxlayer :" << m_dwMaxLayer /*<< " cleartime :" << m_dwClearTime*/ << "layers :" << rBlob.layers_size() << XEND;
  return true;
}

bool TowerConfig::unlockMonster(DWORD dwDeadMonster)
{
  auto s = m_setKillMonsterIDs.find(dwDeadMonster);
  if (s != m_setKillMonsterIDs.end())
    return false;

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwDeadMonster);
  if (pCFG == nullptr)
    return false;

  m_dwMaxLayer += pCFG->dwTowerUnlock;
  m_setKillMonsterIDs.insert(dwDeadMonster);

  XLOG << "[恩德勒斯塔-怪物解锁] monster id = " << dwDeadMonster << " 解锁" << XEND;
  return true;
}

const STowerLayerCFG* TowerConfig::getTowerLayerCFG(DWORD dwLayer) const
{
  auto m = m_mapTowerLayerCFG.find(dwLayer);
  if (m != m_mapTowerLayerCFG.end())
    return &m->second;

  return nullptr;
}

bool TowerConfig::isTower(DWORD dwRaidID) const
{
  return m_setTowerRaidIDs.find(dwRaidID) != m_setTowerRaidIDs.end();
}

void TowerConfig::generateMonster()
{
  for (auto m = m_mapTowerLayerCFG.begin(); m != m_mapTowerLayerCFG.end(); ++m)
  {
    if (m->first > m_dwMaxLayer)
      break;

    STowerLayerCFG* pLayer = &m->second;
    if (pLayer == nullptr)
      continue;

    TVecTowerMonsterCFG vecNormal;
    TVecTowerMonsterCFG vecMini;
    TVecTowerMonsterCFG vecMvp;

    for (auto v = pLayer->vecAllMonster.begin(); v != pLayer->vecAllMonster.end(); ++v)
    {
      if (!v->bUnLock)
        continue;

      const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(v->dwMonsterID);
      if (pNpcCFG == nullptr)
        continue;
      if (pNpcCFG->eNpcType == ENPCTYPE_MONSTER)
        vecNormal.push_back(*v);
      else if (pNpcCFG->eNpcType == ENPCTYPE_MINIBOSS)
        vecMini.push_back(*v);
      else if (pNpcCFG->eNpcType == ENPCTYPE_MVP)
        vecMvp.push_back(*v);
    }

    pLayer->vecCurMonster.clear();

    makeMonster(vecNormal, pLayer->dwNormalCount, pLayer->dwNormalType, pLayer->vecCurMonster);
    makeMonster(vecMini, pLayer->dwMiniCount, static_cast<DWORD>(vecMini.size()), pLayer->vecCurMonster);
    makeMonster(vecMvp, pLayer->dwMvpCount, static_cast<DWORD>(vecMvp.size()), pLayer->vecCurMonster);

//#ifdef _DEBUG
    XDBG << "[恩德勒斯塔-怪物刷新] 第" << pLayer->dwID << "层 需要刷新普通怪" << pLayer->dwNormalCount << "个 mini怪" << pLayer->dwMiniCount << "个 mvp怪" << pLayer->dwMvpCount << "个 时间" << xTime::getCurSec() << XEND;
    std::stringstream sstr;
    sstr << " ";
    for (auto v = pLayer->vecCurMonster.begin(); v != pLayer->vecCurMonster.end(); ++v)
      sstr << v->dwMonsterID << " ";
    XDBG << "[恩德勒斯塔-怪物刷新] 第" << pLayer->dwID << "层 生成怪物 id =" << sstr.str() << XEND;
//#endif
  }

  //m_dwClearTime = xTime::getCurSec() + MiscConfig::getMe().getEndlessTowerCFG().dwUpdateTimeSecDiff;
}

void TowerConfig::updateConfig(const TowerInfo& cmd)
{
  initDataFromDB(cmd);
}

bool TowerConfig::loadEndLessMonsterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EndLessMonster.txt"))
  {
    XERR << "[恩德勒斯塔-加载配置],加载配置Table_EndLessMonster.txt失败" << XEND;
    return false;
  }

  m_setKillMonsterIDs.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EndLessMonster", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    xLuaData& floor = m->second.getMutableData("AppearFloor");
    DWORD dwMin = floor.getTableInt("1");
    DWORD dwMax = floor.getTableInt("2");

    for (DWORD d = dwMin; d <= dwMax; ++d)
    {
      auto o = m_mapTowerLayerCFG.find(d);
      if (o == m_mapTowerLayerCFG.end())
      {
        XERR << "[恩德勒斯塔-加载配置] monsterid : " << m->first << " layer : " << d << " 未在 Table_EndLessTower.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }

      DWORD dwMonsterID = m->first;
      auto monster = find_if(o->second.vecAllMonster.begin(), o->second.vecAllMonster.end(), [dwMonsterID](const STowerMonsterCFG& r) -> bool{
        return r.dwMonsterID == dwMonsterID;
      });
      if (monster != o->second.vecAllMonster.end())
        continue;

      STowerMonsterCFG stCFG;
      stCFG.dwMonsterID = dwMonsterID;
      stCFG.dwUnlockCondition = m->second.getTableInt("UnlockCriteria");
      stCFG.dwUniqueID = m->second.getTableInt("UniqueID");
      stCFG.pairLayer.first = dwMin;
      stCFG.pairLayer.second = dwMax;
      stCFG.bUnLock = stCFG.dwUnlockCondition == 0;
      if (stCFG.bUnLock)
        m_setKillMonsterIDs.insert(stCFG.dwMonsterID);

      o->second.vecAllMonster.push_back(stCFG);
    }
  }

  if (bCorrect)
    XLOG << "[恩德勒斯塔-加载配置] 成功加载配置Table_EndLessMonster.txt" << XEND;
  return bCorrect;
}

bool TowerConfig::loadEndLessTowerConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_EndLessTower.txt"))
  {
    XERR << "[恩德勒斯塔-加载配置],加载配置Table_EndLessTower.txt失败" << XEND;
    return false;
  }

  m_setTowerRaidIDs.clear();
  m_mapTowerLayerCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_EndLessTower", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    // check duplicated
    auto o = m_mapTowerLayerCFG.find(m->first);
    if (o != m_mapTowerLayerCFG.end())
    {
      bCorrect = false;
      continue;
    }

    STowerLayerCFG stCFG;

    stCFG.dwID = m->first;
    stCFG.dwRaidID = m->second.getTableInt("RaidId");
    stCFG.dwSky = m->second.getTableInt("SkyType");
    stCFG.dwNormalType = m->second.getTableInt("NormalType");
    stCFG.dwNormalCount = m->second.getTableInt("NormalNum");
    stCFG.dwMiniCount = m->second.getTableInt("MiniNum");
    stCFG.dwMvpCount = m->second.getTableInt("MVPNum");
    stCFG.dwRewardID = m->second.getTableInt("Reward");
    stCFG.strBgm = m->second.getTableString("BGM");
    xLuaData& buffs = m->second.getMutableData("MonsterAI");
    ENpcType eType = ENPCTYPE_MONSTER;
    auto getais = [&](const string& key, xLuaData& d)
    {
      stCFG.mapNpcExtraAIs[eType].insert(d.getInt());
    };
    eType = ENPCTYPE_MVP;
    buffs.getMutableData("mvp").foreach(getais);
    eType = ENPCTYPE_MINIBOSS;
    buffs.getMutableData("mini").foreach(getais);
    eType = ENPCTYPE_MONSTER;
    buffs.getMutableData("monster").foreach(getais);

    m_setTowerRaidIDs.insert(stCFG.dwRaidID);
    m_mapTowerLayerCFG[stCFG.dwID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[恩德勒斯塔-加载配置] 成功加载配置Table_EndLessTower.txt" << XEND;
  return bCorrect;
}

void TowerConfig::makeMonster(const TVecTowerMonsterCFG& vecAll, DWORD dwNum, DWORD typeNum, TVecTowerMonsterCFG& vecResult)
{
  if (typeNum == 0)
    return;
  if (vecAll.empty() == true)
    return;

  DWORD dwMaxTypeNum = static_cast<DWORD>(vecAll.size());
  if (dwMaxTypeNum <= typeNum)
  {
    for (DWORD d = 0; d < dwNum; ++d)
    {
      DWORD dwIndex = randBetween(0, dwMaxTypeNum - 1);
      vecResult.push_back(vecAll[dwIndex]);
    }
  }
  else
  {
    TVecTowerMonsterCFG vecCopy = vecAll;
    while (vecCopy.size() < typeNum)
    {
      DWORD dwIndex = randBetween(0, static_cast<int>(vecCopy.size()));
      vecResult.push_back(vecCopy[dwIndex]);
      vecCopy.erase(vecCopy.begin() + dwIndex);
    }
    dwMaxTypeNum = static_cast<DWORD>(vecCopy.size());
    for (DWORD d = 0; d < dwNum; ++d)
    {
      DWORD dwIndex = randBetween(0, dwMaxTypeNum - 1);
      vecResult.push_back(vecAll[dwIndex]);
    }
  }
}


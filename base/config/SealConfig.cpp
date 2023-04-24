#include "SealConfig.h"
#include "NpcConfig.h"
#include "xLuaTable.h"
#include "RewardConfig.h"
#include "MapConfig.h"

bool SealCFG::haveMonsterInPoint(DWORD lastValue, DWORD nowValue) const
{
  for (auto &pa : vecRate2Point)
  {
    if (lastValue < pa.first && nowValue >= pa.first)
      return true;
  }
  for (auto &pa : vecSpcRate2Point)
  {
    if (lastValue < pa.first && nowValue >= pa.first)
      return true;
  }
  return false;
}

bool SealCFG::getMonster(DWORD lastValue, DWORD nowValue, DWORD userLevel, TVecSealMonster& outVec) const
{
  TVecDWORD normalVecPoint;
  for (auto &pa : vecRate2Point)
  {
    if (lastValue < pa.first && nowValue >= pa.first)
      normalVecPoint.push_back(pa.second);
  }
  for (auto &v : normalVecPoint)
  {
    if (getMonsterByPoint(v, userLevel, false, outVec) == false)
      return false;
  }

  TVecDWORD specVecPoint;
  for (auto &pa : vecSpcRate2Point)
  {
    if (lastValue < pa.first && nowValue >= pa.first)
      specVecPoint.push_back(pa.second);
  }

  for (auto &v : specVecPoint)
  {
    if (getMonsterByPoint(v, userLevel, true, outVec) == false)
      return false;
  }
  return true;
}

bool SealCFG::getMonsterByPoint(DWORD point, DWORD level, bool isspec, TVecSealMonster& outVec) const
{
  TVecSealMonster toolInVec;
  if (isspec)
  {
    for (auto &st : vecMonster)
    {
      if (st.isspec == false || st.dwAppearLv > level)
        continue;
      toolInVec.push_back(st);
    }
  }
  else
  {
    for (auto &st : vecMonster)
    {
      if (st.isspec || st.dwAppearLv > level)
        continue;
      toolInVec.push_back(st);
    }
  }

  if (findPoint(point, toolInVec, outVec) == false)
    return false;
  return true;
}

bool SealCFG::findPoint(DWORD leftPoint, const TVecSealMonster& toolInVec, TVecSealMonster& outVec) const
{
  DWORD protectCount = 30;
  TVecSealMonster toolVec;
  while(leftPoint != 0 && protectCount > 0)
  {
    toolVec.clear();
    getVecLessThan(leftPoint, toolInVec, toolVec);
    DWORD size = toolVec.size();
    if (size == 0)
      return false;

    DWORD randIndex = randBetween(0, size-1);

    leftPoint -= toolVec[randIndex].dwPoint;
    outVec.push_back(toolVec[randIndex]);

    protectCount --;
  }

  if (leftPoint != 0)
    return false;
  return true;
}

void SealCFG::getVecLessThan(DWORD point, const TVecSealMonster& toolInVec, TVecSealMonster& toolVec) const
{
  toolVec.clear();
  for (auto &v : toolInVec)
  {
    if (v.dwPoint <= point)
      toolVec.push_back(v);
  }
}

SealConfig::SealConfig()
{

}

SealConfig::~SealConfig()
{

}

bool SealConfig::loadConfig()
{
  bool bResult = true;
  if (loadSealMonsterConfig() == false)
    bResult = false;
  if (loadRepairSealConfig() == false)
    bResult = false;

  return bResult;
}

const SealCFG* SealConfig::getSealCFG(DWORD dwID) const
{
  auto m = m_mapSealCFG.find(dwID);
  if (m != m_mapSealCFG.end())
    return &m->second;

  return nullptr;
}

const SealCFG* SealConfig::getSealCFGByQuest(DWORD dwQuestID) const
{
  for (auto m = m_mapSealCFG.begin(); m != m_mapSealCFG.end(); ++m)
  {
    if (m->second.dwPreQuest == dwQuestID)
      return &m->second;
  }

  return nullptr;
}

bool SealConfig::loadRepairSealConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_RepairSeal.txt"))
  {
    XERR << "[SealConfig], 加载配置Table_RepairSeal.txt失败" << XEND;
    return false;
  }

  m_mapSealCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_RepairSeal", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto o = m_mapSealCFG.find(m->first);
    if (o != m_mapSealCFG.end())
    {
      XERR << "[SealConfig::loadRepairSealConfig] id : " << m->first << " duplicated" << XEND;
      bCorrect = false;
      continue;
    }

    SealCFG stCFG;
    stCFG.dwID = m->first;
    stCFG.dwMapID = m->second.getTableInt("MapID");
    stCFG.dwDMapID = m->second.getTableInt("RaidMap");
    stCFG.dwMaxTime = m->second.getTableInt("SealTime");
    stCFG.dwMaxValue = m->second.getTableInt("SealMax");
    stCFG.dwPosRange = m->second.getTableInt("SealRange");
    stCFG.dwPreQuest = m->second.getTableInt("QuestID");
    stCFG.dwSealLevel = m->second.getTableInt("SealLevel");
    stCFG.dwBaseExp = m->second.getTableInt("BaseExp");
    stCFG.dwJobExp = m->second.getTableInt("JobExp");

    auto rewardf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecRewardIDs.push_back(data.getInt());
    };
    m->second.getMutableData("reward").foreach(rewardf);
    // check reward
    for (auto &v : stCFG.vecRewardIDs)
    {
      if (RewardConfig::getMe().getRewardCFG(v) == nullptr)
      {
        XERR << "[Table_Repair], id : " << m->first << ", reward : " << v << " 在reward表中找不到" << XEND;
        bCorrect = false;
      }
    }

    stCFG.dwMaxMonsterNum = m->second.getTableInt("MaxMonsterNum");

    DWORD type = m->second.getTableInt("type");
    if (type <= ESEALTYPE_MIN || type >= ESEALTYPE_MAX)
    {
      XERR << "[SealConfig::loadRepairSealConfig] id = " << m->first << " type = " << type << " error" << XEND;
      bCorrect = false;
      continue;
    }
    stCFG.eType = static_cast<ESealType>(type);

    xLuaData& pos = m->second.getMutableData("SealRange");
    auto posf = [&](const string& str, xLuaData& data)
    {
      xLuaData& poss = data.getMutableData("p");

      SealPos stPos;
      stPos.oPos.x = poss.getTableInt("1");
      stPos.oPos.y = poss.getTableInt("2");
      stPos.oPos.z = poss.getTableInt("3");
      stPos.dwRange = data.getTableInt("r");

      stCFG.vecRefreshPos.push_back(stPos);
    };
    pos.foreach(posf);

    xLuaData& pointdata = m->second.getMutableData("point");
    bool isspec = false;
    auto funpoint = [&](const string &str, xLuaData& data)
    {
      pair<DWORD, DWORD> rate2point;
      rate2point.first = data.getTableInt("1");
      rate2point.second = data.getTableInt("2");
      if (isspec)
        stCFG.vecSpcRate2Point.push_back(rate2point);
      else
        stCFG.vecRate2Point.push_back(rate2point);
    };
    pointdata.foreach(funpoint);
    xLuaData& specdata = m->second.getMutableData("SpecialPoint");
    isspec = true;
    specdata.foreach(funpoint);

    DWORD teamid = m->second.getTableInt("TeamId");
    for (auto &v : m_vecSealMonster)
    {
      if (v.dwTeamID == teamid)
      {
        stCFG.vecMonster.push_back(v);
      }
    }
    if (stCFG.vecMonster.empty())
    {
      XERR << "[RepairSeal], id = " << m->first << ", TeamID = " << teamid << ", 在sealmonster表中无法找到" << XEND;
      bCorrect = false;
      continue;
    }

    m_mapSealCFG[stCFG.dwID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[SealConfig], 成功加载配置Table_RepairSeal.txt" << XEND;
  return bCorrect;
}

bool SealConfig::loadSealMonsterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_SealMonster.txt"))
  {
    XERR << "[SealConfig], 加载配置Table_SealMonster.txt失败" << XEND;
    return false;
  }

  m_vecSealMonster.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_SealMonster", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SealMonster stMonster;
    DWORD dwTeamID = m->second.getTableInt("TeamId");
    if (dwTeamID == 0)
    {
      bCorrect = false;
      XERR << "[SealMonster], id = " << m->first << ", TeamID为0" << XEND;
      continue;
    }
    stMonster.dwTeamID = dwTeamID;

    stMonster.dwID = m->second.getTableInt("MonsterID");
    if (NpcConfig::getMe().getNpcCFG(stMonster.dwID) == nullptr)
    {
      XERR << "[SealConfig] monster id = " << stMonster.dwID << ", 在monster表中找不到" << XEND;
      bCorrect = false;
      continue;
    }
    stMonster.dwAppearLv = m->second.getTableInt("AppearLv");
    stMonster.dwPoint = m->second.getTableInt("MonsterPoint");
    stMonster.isspec = m->second.getTableInt("Special") != 0;

    m_vecSealMonster.push_back(stMonster);
  }

  if (bCorrect)
    XLOG << "[SealConfig] 成功加载Table_SealMonster.txt" << XEND;
  return bCorrect;
}

bool SealConfig::checkConfig() const
{
  bool bResult = true;
  for (auto &m : m_mapSealCFG)
  {
    const SRaidCFG *pRaid = MapConfig::getMe().getRaidCFG(m.second.dwDMapID);
    if (pRaid == nullptr)
    {
      XERR << "[SealConfig], 找不到对应的副本id" << "封印id:" << m.first << XEND;
      bResult = false;
      continue;
    }
    if (pRaid->dwMapID != m.second.dwMapID)
    {
      XERR << "[SealConfig], MapID与RaidMap 不对应" << "封印id:" << m.first <<XEND;
      bResult = false;
      continue;
    }
    if (!(pRaid->eRestrict == ERAIDRESTRICT_TEAM && m.second.eType == ESEALTYPE_NORMAL) && !(pRaid->eRestrict == ERAIDRESTRICT_PRIVATE && m.second.eType == ESEALTYPE_PERSONAL))
    {
      XERR << "[SealConfig -> MapRaid], 副本限制类型配置错误, 封印id:" << m.first << "副本id:" << m.second.dwDMapID << XEND;
      bResult = false;
      continue;
    }
  }
  return bResult;
}


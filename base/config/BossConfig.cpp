#include "BossConfig.h"
#include "NpcConfig.h"

// SBossCFG
DWORD SBossCFG::getMapID() const
{
  if (vecMap.size() <= 0)
  {
    XERR << "[Table_Boss] id :" << dwID << "Map未配置" << XEND;
    return 0;
  }

  if (eType == EBOSSTYPE_WORLD)
  {
    DWORD dwRand = randBetween(0, vecMap.size() - 1);
    return vecMap[dwRand];
  }
  return vecMap[0];
}

DWORD SBossCFG::getReliveTime(DWORD mapid) const
{
  auto it = mapReliveTime.find(mapid);
  if (it == mapReliveTime.end())
  {
    XERR << "[Table_Boss] id :" << dwID << "ReliveTime map:" << mapid << "未配置" << XEND;
    return 120;
  }
  return it->second;
}

const SBossLvCFG* SBossCFG::getLvCFG(DWORD dwLv) const
{
  auto m = mapLvCFG.find(dwLv);
  if (m != mapLvCFG.end())
    return &m->second;
  return nullptr;
}

DWORD SBossCFG::randActID() const
{
  if (vecActIDs.empty() == true)
    return 0;
  DWORD dwRand = randBetween(0, vecActIDs.size() - 1);
  return vecActIDs[dwRand];
}

// BossConfig
BossConfig::BossConfig()
{
}

BossConfig::~BossConfig()
{
}

bool BossConfig::loadConfig()
{
  bool bCorrect = true;

  bCorrect &= loadBossConfig();
  bCorrect &= loadDeadBossConfig();
  bCorrect &= loadBossStepConfig();

  return bCorrect;
}

bool BossConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto &m : m_mapBossCFG)
  {
    DWORD dwID = m.second.oBossDefine.getTableInt("id");
    if (NpcConfig::getMe().getNpcCFG(dwID) == nullptr)
    {
      bCorrect = false;
      XERR << "[Table_Boss] id :" << m.first << "bossid :" << dwID << "未在 Table_Monster.txt 表中找到" << XEND;
    }
  }
  return bCorrect;
}

const SBossCFG* BossConfig::getBossCFG(DWORD dwID) const
{
  auto m = m_mapBossCFG.find(dwID);
  if (m != m_mapBossCFG.end() && m->second.blInit)
    return &m->second;
  return nullptr;
}

const SBossCFG* BossConfig::randSetBossCFG(EBossType eType, const TSetDWORD& setExcludeIDs)
{
  auto m = m_mapTypeBossCFG.find(eType);
  if (m == m_mapTypeBossCFG.end())
    return nullptr;

  struct SRand
  {
    DWORD dwBossID = 0;
    DWORD dwWeight = 0;
  };

  std::vector<SRand> vecItems;
  DWORD dwTotalWeight = 0;
  for (auto &v : m->second)
  {
    const SBossCFG& rCFG = v;
    auto s = setExcludeIDs.find(rCFG.dwID);
    if (s != setExcludeIDs.end())
      continue;

    dwTotalWeight += rCFG.dwSetWeight;

    SRand stRand;
    stRand.dwBossID = rCFG.dwID;
    stRand.dwWeight = dwTotalWeight;
    vecItems.push_back(stRand);
  }

  DWORD dwRand = randBetween(0, dwTotalWeight);
  for (auto &v : vecItems)
  {
    if (dwRand <= v.dwWeight)
      return getBossCFG(v.dwBossID);
  }

  return nullptr;
}

bool BossConfig::loadBossConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Boss.txt"))
  {
    XERR << "[Table_Boss.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapBossCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Boss", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= EBOSSTYPE_MIN || dwType >= EBOSSTYPE_MAX)
    {
      bCorrect = false;
      XERR << "[Table_Boss] id :" << m->first << "type :" << dwType << "不合法" << XEND;
      continue;
    }

    SBossCFG& rCFG = m_mapBossCFG[m->first];

    rCFG.dwID = m->first;
    rCFG.dwDeadID = m->second.getTableInt("MvpID");
    rCFG.dwSetWeight = m->second.getTableInt("SetWeight");
    rCFG.dwRandomTime = m->second.getTableInt("RandomTime");
    rCFG.eType = static_cast<EBossType>(dwType);
    rCFG.strName = m->second.getTableString("NameZh");

    rCFG.vecActIDs.clear();
    m->second.getMutableData("ActID").getIDList(rCFG.vecActIDs);

    rCFG.vecMap.clear();
    auto mapf = [&](const string& key, xLuaData& data)
    {
      rCFG.vecMap.push_back(data.getInt());
    };
    m->second.getMutableData("Map").foreach(mapf);

    rCFG.mapReliveTime.clear();
    auto relivef = [&](const string& key, xLuaData& data)
    {
      rCFG.mapReliveTime[atoi(key.c_str())] = data.getInt();
    };
    m->second.getMutableData("ReliveTime").foreach(relivef);

    rCFG.oBossDefine = m->second.getMutableData("BossSetup");
    rCFG.blInit = true;
  }

  m_mapTypeBossCFG.clear();
  for (auto &m : m_mapBossCFG)
    m_mapTypeBossCFG[m.second.eType].push_back(m.second);

  if (bCorrect)
    XLOG << "[Table_Boss], 成功加载配置Table_Boss.txt" << XEND;
  return bCorrect;
}

bool BossConfig::loadDeadBossConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Deadboss.txt"))
  {
    XERR << "[Table_Deadboss.txt], 加载配置失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Deadboss", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwID = m->second.getTableInt("MonsterID");
    auto item = m_mapBossCFG.find(dwID);
    if (item == m_mapBossCFG.end())
    {
      bCorrect = false;
      XERR << "[Table_Deadboss] id :" << m->first << "MonsterID :" << dwID << "未在 Table_Boss.txt 表中找到" << XEND;
      continue;
    }

    DWORD dwLv = m->second.getTableInt("DeadBoss_lv");

    SBossCFG& rCFG = item->second;
    SBossLvCFG& rLvCFG = rCFG.mapLvCFG[dwLv];

    rLvCFG.dwLv = m->first;
    rLvCFG.setRewardIDs.clear(), m->second.getMutableData("Dead_Reward").getIDList(rLvCFG.setRewardIDs);
    rLvCFG.setBuffIDs.clear(), m->second.getMutableData("Buff").getIDList(rLvCFG.setRewardIDs);
    rLvCFG.setSuperAIs.clear(), m->second.getMutableData("SuperAI").getIDList(rLvCFG.setRewardIDs);
  }

  if (bCorrect)
    XLOG << "[Table_Deadboss], 成功加载配置 Table_Deadboss.txt" << XEND;
  return bCorrect;
}

bool BossConfig::loadBossStepConfig()
{
  bool bCorrect = false;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Boss_step.txt"))
  {
    XERR << "[Table_Boss_step.txt], 加载配置失败" << XEND;
    return false;
  }

  m_mapBossStepCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Boss_step", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    const string& content = m->second.getTableString("Content");
    EBossStep eStep = getBossStep(content);
    if (eStep == EBOSSSTEP_MIN)
    {
      bCorrect = false;
      XERR << "[Table_Boss_step] id :" << m->first << "Content :" << content << "不合法" << XEND;
      continue;
    }

    TVecBossStepCFG& vecCFG = m_mapBossStepCFG[m->second.getTableInt("ActID")];

    SBossStepCFG stCFG;
    stCFG.step = eStep;
    stCFG.data = m->second;
    vecCFG.push_back(stCFG);
  }

  if (bCorrect)
    XLOG << "[Table_Boss_step], 成功加载配置 Table_Boss_step.txt" << XEND;
  return bCorrect;
}

Cmd::EBossStep BossConfig::getBossStep(const string& str)
{
  if (str == "visit")
    return Cmd::EBOSSSTEP_VISIT;
  else if (str == "summon")
    return Cmd::EBOSSSTEP_SUMMON;
  else if (str == "clear")
    return Cmd::EBOSSSTEP_CLEAR;
  else if (str == "boss")
    return Cmd::EBOSSSTEP_BOSS;
  else if (str == "limit")
    return Cmd::EBOSSSTEP_LIMIT;
  else if (str == "dialog")
    return Cmd::EBOSSSTEP_DIALOG;
  else if (str == "status")
    return Cmd::EBOSSSTEP_STATUS;
  else if (str == "wait")
    return Cmd::EBOSSSTEP_WAIT;
  else if (str == "kill")
    return Cmd::EBOSSSTEP_KILL;
  else if (str == "world")
    return Cmd::EBOSSSTEP_WORLD;
  else if (str == "show")
    return Cmd::EBOSSSTEP_SHOW;

  return Cmd::EBOSSSTEP_MIN;
}


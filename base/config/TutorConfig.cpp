#include "TutorConfig.h"
#include "xLuaTable.h"

bool STutorTaskCFG::checkValid(QWORD value) const
{
  switch (eType)
  {
  case ETUTORTASKTYPE_BOARD_QUEST:
  case ETUTORTASKTYPE_REPAIR_SEAL:
  case ETUTORTASKTYPE_PASS_TOWER:
  case ETUTORTASKTYPE_GUILD_RAID_FINISH:
  case ETUTORTASKTYPE_LABORATORY_FINISH:
    return true;
  case ETUTORTASKTYPE_USE_ITEM:
  case ETUTORTASKTYPE_BE_USED_ITEM:
    for (auto& v : setValues)
      if (value == v)
        return true;
    return false;
  case ETUTORTASKTYPE_PASS_TOWER_LAYER:
    return value >= dwFlag;
  default:
    return false;
  }
  return false;
}

TutorConfig::TutorConfig()
{
}

TutorConfig::~TutorConfig()
{
}

bool TutorConfig::loadConfig()
{
  bool bCorrect = true;

  if (loadTask() == false)
    bCorrect = false;
  if (loadGrowReward() == false)
    bCorrect = false;

  return bCorrect;
}

bool TutorConfig::checkConfig()
{
  bool bCorrect = true;

  return bCorrect;
}

ETutorTaskType TutorConfig::getTaskType(const string& type)
{
  if (type == "board_quest")
    return ETUTORTASKTYPE_BOARD_QUEST;
  else if (type == "repair_seal")
    return ETUTORTASKTYPE_REPAIR_SEAL;
  else if (type == "use_item")
    return ETUTORTASKTYPE_USE_ITEM;
  else if (type == "be_used_item")
    return ETUTORTASKTYPE_BE_USED_ITEM;
  else if (type == "pass_tower")
    return ETUTORTASKTYPE_PASS_TOWER;
  else if (type == "guild_raid_finish")
    return ETUTORTASKTYPE_GUILD_RAID_FINISH;
  else if (type == "laboratory_finish")
    return ETUTORTASKTYPE_LABORATORY_FINISH;
  else if (type == "pass_tower_layer")
    return ETUTORTASKTYPE_PASS_TOWER_LAYER;
  return ETUTORTASKTYPE_MIN;
}

bool TutorConfig::loadTask()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_StudentAdventureQuest.txt"))
  {
    XERR << "[导师配置-冒险任务] 加载配置Table_StudentAdventureQuest.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_StudentAdventureQuest", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    STutorTaskCFG cfg;

    cfg.dwID = m->second.getTableInt("id");
    cfg.eType = getTaskType(m->second.getTableString("Type"));
    cfg.dwTotalProgress = m->second.getTableInt("Target");
    cfg.dwTeacherReward = m->second.getTableInt("TeacherReward");
    cfg.dwProficiency = m->second.getTableInt("Familiar");
    cfg.dwResetTime = m->second.getTableInt("ResetTime");
    cfg.strName = m->second.getTableString("Traceinfo");

    auto stdrewardf = [&](const string& str, xLuaData& data)
    {
      cfg.mapStudentReward[atoi(str.c_str())] = data.getInt();
    };
    m->second.getMutableData("StudentReward").foreach(stdrewardf);

    if (cfg.eType <= ETUTORTASKTYPE_MIN || cfg.eType >= ETUTORTASKTYPE_MAX)
    {
      XERR << "[导师配置-冒险任务] id:" << cfg.dwID << "type配置错误" << XEND;
      bCorrect = false;
      continue;
    }
    if (cfg.dwTotalProgress <= 0)
    {
      XERR << "[导师配置-冒险任务] id:" << cfg.dwID << "Target配置错误" << XEND;
      bCorrect = false;
      continue;
    }
    if (cfg.dwResetTime != 1 && cfg.dwResetTime != 7)
    {
      XERR << "[导师配置-冒险任务] id:" << cfg.dwID << "ResetTime配置错误" << XEND;
      bCorrect = false;
      continue;
    }

    auto paramf = [&](const string& str, xLuaData& data)
    {
      if (str == "item")
      {
        auto f = [&](const string& s, xLuaData& d)
        {
          cfg.setValues.insert(d.getInt());
        };
        data.foreach(f);
      }
      else if (str == "layer")
        cfg.dwFlag = data.getInt();
    };
    m->second.getMutableData("Param").foreach(paramf);

    m_mapTutorTaskCFG[cfg.dwID] = cfg;
    m_vecType2TutorTaskCFG[cfg.eType].push_back(cfg);
  }

  if (bCorrect)
    XLOG << "[导师配置-冒险任务] 成功加载配置Table_StudentAdventureQuest.txt" << XEND;
  return bCorrect;
}

bool TutorConfig::loadGrowReward()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_TutorGrowUpReward.txt"))
  {
    XERR << "[导师配置-成长奖励] 加载配置Table_TutorGrowUpReward.txt失败" << XEND;
    return false;
  }

  bool bCorrect = true;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_TutorGrowUpReward", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    STutorGrowRewardCFG cfg;

    DWORD level = m->second.getTableInt("MaxLevel");
    cfg.dwProficiency = m->second.getTableInt("Familiar");
    cfg.dwStudentReward = m->second.getTableInt("StudentReward");
    cfg.dwTeacherReward = m->second.getTableInt("TeacherReward");

    m_mapTutorGrowRewardCFG[level] = cfg;
  }

  if (bCorrect)
    XLOG << "[导师配置-成长奖励] 成功加载配置Table_TutorGrowUpReward.txt" << XEND;
  return bCorrect;
}

const STutorTaskCFG* TutorConfig::getTutorTaskCFG(DWORD dwID)
{
  auto it = m_mapTutorTaskCFG.find(dwID);
  if (it == m_mapTutorTaskCFG.end())
    return nullptr;
  return &it->second;
}

const TVecTutorTaskCFG& TutorConfig::getTutorTaskCFGByType(ETutorTaskType eType)
{
  const static TVecTutorTaskCFG emptyvec;
  auto it = m_vecType2TutorTaskCFG.find(eType);
  if (it == m_vecType2TutorTaskCFG.end())
    return emptyvec;
  return it->second;
}

const STutorGrowRewardCFG* TutorConfig::getTutorGrowRewardCFG(DWORD dwLevel)
{
  auto it = m_mapTutorGrowRewardCFG.find(dwLevel);
  if (it == m_mapTutorGrowRewardCFG.end())
    return nullptr;
  return &it->second;
}

DWORD TutorConfig::getProficiencyByLv(DWORD dwLevel)
{
  DWORD profic = 0;
  for (auto& v : m_mapTutorGrowRewardCFG)
  {
    if (dwLevel < v.first)
      break;
    profic += v.second.dwProficiency;
  }
  return profic;
}

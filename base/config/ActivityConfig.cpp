#include "ActivityConfig.h"
#include "xLuaTable.h"
#include "xTime.h"

DWORD SGlobalActCFG::getDepositId() const
{
  DWORD dwDepositId = 0;
  switch (m_actType)
  {
  case GACTIVITY_CHARGE_EXTRA_REWARD:
  {
    if (m_vecParam.size() == 2)
      dwDepositId = m_vecParam[0];
    break;
  }
  case GACTIVITY_CHARGE_DISCOUNT:
  {
    if (m_vecParam.size() == 2)
      dwDepositId = m_vecParam[1];
    break;
  }
  case GACTIVITY_CHARGE_EXTRA_COUNT:
  {
    if (m_vecParam.size() == 1)
      dwDepositId = m_vecParam[0];
    break;
  }
  default:
    break;
  }

  XDBG << "[全服活动-获取关联充值id] 活动id" << m_dwId << "type" << m_actType << "充值id" << dwDepositId << XEND;
  return dwDepositId;
}

bool SGlobalActCFG::isParamExist(DWORD value) const
{
  for(auto s : m_vecParam)
    if(s == value)
      return true;

  return false;
}

float SGlobalActCFG::getDiscount(DWORD serviceid) const
{
  if (m_vecParam.size() % 2 != 0)
    return 1.0f;
  auto v = find(m_vecParam.begin(), m_vecParam.end(), serviceid);
  if (v == m_vecParam.end())
    return 1.0f;
  if (++v == m_vecParam.end())
    return 1.0f;
  return *v / 10000.0f;
}

// const char LOG_NAME[] = "ActivityConfig";
ActivityConfig::ActivityConfig()
{
}

ActivityConfig::~ActivityConfig()
{
}

bool ActivityConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Activity.txt"))
  {
    XERR << "[表格],加载表格,Table_Activity.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Activity", table);
  m_mapActivityCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");
    auto it = m_mapActivityCFG.find(id);
    if (it == m_mapActivityCFG.end())
    {
      m_mapActivityCFG[id] = SActivityCFG();
      it = m_mapActivityCFG.find(id);
    }
    if (it == m_mapActivityCFG.end())
    {
      XERR << "[ActivityConfig] id = " << id << " can not be found" << XEND;
      bCorrect = false;
      continue;
    }
    SActivityCFG& rCfg = it->second;
    rCfg.m_dwId = id;
    rCfg.m_actType = static_cast<EActivityType>(m->second.getTableInt("Type"));
    rCfg.m_limitType = static_cast<EActLimitType>(m->second.getTableInt("LimitType"));

    //Map
    xLuaData& rData = m->second.getMutableData("Map");
    auto func = [&](const std::string& key, xLuaData& data)
    {
      rCfg.m_vecMapId.push_back(data.getInt());
    };
    rData.foreach(func);
        
    //Condition
    rCfg.m_condition = m->second.getMutableData("Condition");

    rData = m->second.getMutableData("StartTime");

    auto funcStartTime = [&](const std::string& key, xLuaData& data)
    {
      tm stm;
      bzero(&stm, sizeof(stm));
      if (data.has("mday"))
        stm.tm_mday = data.getTableInt("mday");
      else
        stm.tm_mday = -1;

      if (data.has("wday"))
        stm.tm_wday = data.getTableInt("wday"); //[0,6] 0 is sunday
      else
        stm.tm_wday = -1;
      if (stm.tm_wday >= 7)
      {
        stm.tm_wday = 0;
      }

      if (data.has("hour"))
        stm.tm_hour = data.getTableInt("hour");
      else
        stm.tm_hour = -1;
      if (data.has("min"))
        stm.tm_min = data.getTableInt("min");
      else
        stm.tm_min = -1;
      if (data.has("sec"))
        stm.tm_sec = data.getTableInt("sec");
      else
        stm.tm_sec = -1;

      XDBG << "[Table_Activity] key: "<< key << stm.tm_mday << stm.tm_wday << stm.tm_hour << stm.tm_min << stm.tm_sec << XEND;
      rCfg.m_vecStartTime.push_back(stm);
    };
    rData.foreach(funcStartTime);

    //Duration
    rCfg.m_duration = m->second.getTableInt("Duration");

    rCfg.m_dwPath = m->second.getTableInt("Path");

    auto unshowF = [&](const std::string&key, xLuaData &data)
    {
      rCfg.m_vecUnshowMap.push_back(data.getInt());
    };
    m->second.getMutableData("UnshowMap").foreach(unshowF);
  }

  bCorrect = loadStepConfig();

  if (bCorrect)
    XLOG << "[Table_Activity], 成功加载表格Table_Activity.txt" << XEND;
  
  bCorrect = loadGlobalActConfig();

  return bCorrect;
}

bool ActivityConfig::loadStepConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ActivityStep.txt"))
  {
    XERR << "[表格],加载表格,Table_ActivityStep.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ActivityStep", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("ID");
    auto it = m_mapActivityCFG.find(id);   
    if (it == m_mapActivityCFG.end())
    {
      XERR << "[ActivityConfig] id = " << id << " can not be found in Table_Activity.txt" << XEND;
      bCorrect = false;
      continue;
    }

    SActivityCFG& rCfg = it->second;

    SActvityNodeBase nodebase;
    //ItemType 
    std::string itemType = m->second.getTableString("ItemType");
    
    nodebase.content = m->second.getTableString("Content");
    nodebase.oParams = m->second.getMutableData("Params");

    if (m->second.has("ConditionStage"))
    {
      if (m->second.getMutableData("ConditionStage").has("success"))
        nodebase.oParams.setData("ok_stage", m->second.getMutableData("ConditionStage").getTableInt("success"));
      if (m->second.getMutableData("ConditionStage").has("fail"))
        nodebase.oParams.setData("fail_stage", m->second.getMutableData("ConditionStage").getTableInt("fail"));
    }

    if (itemType == "stage")
    {
      nodebase.type = EACTNODETYPE_STAGE;      
      DWORD duration = 0;
      if (rCfg.m_duration == 0)
      {
        duration = 0;       //0 一直持续
      }
      else
      {
        duration = rCfg.m_duration * m->second.getTableInt("Duration") / 100;   //百分比
        if (duration == 0)
          duration = 1;         //1 second
      }
      nodebase.oParams.setData("base_duration", duration);

      if (nodebase.oParams.has("master_stage"))
      {
        if (nodebase.oParams.has("stage_index") == false || nodebase.oParams.getTableInt("master_stage") == nodebase.oParams.getTableInt("stage_index"))
        {
          XERR << "[ActivityConfig] Table_ActivityStep.txt 配置错误, 缺少有效参数stage_index, id:" << m->first << XEND;
          bCorrect = false;
        }
      }
      ActivityStageItem stage;
      stage.stageNode = nodebase;
      rCfg.m_vecStage.push_back(stage);
    }
    else if (itemType == "seq_node")
    {
      nodebase.type = EACTNODETYPE_SEQ;
      nodebase.oParams.setData("base_interval", m->second.getTableInt("Interval"));
      nodebase.oParams.setData("base_limitcount", m->second.getTableInt("Limitcount"));
      if (m->second.has("LimitTime"))
        nodebase.oParams.setData("LimitTime", m->second.getTableInt("LimitTime"));
      auto it = rCfg.m_vecStage.rbegin();
      if (it == rCfg.m_vecStage.rend())
      {
        XERR << "[ActivityConfig] Table_ActivityStep.txt id= " << id << " invalid order" << XEND;
        bCorrect = false;
        continue;
      }
      it->vecNode.push_back(nodebase);
    }
    else if (itemType == "par_node")
    {
      nodebase.type = EACTNODETYPE_PAR;
      nodebase.oParams.setData("base_interval", m->second.getTableInt("Interval"));
      nodebase.oParams.setData("base_limitcount", m->second.getTableInt("Limitcount"));
      if (m->second.has("LimitTime"))
        nodebase.oParams.setData("LimitTime", m->second.getTableInt("LimitTime"));
      auto it = rCfg.m_vecStage.rbegin();
      if (it == rCfg.m_vecStage.rend())
      {
        XERR << "[ActivityConfig] Table_ActivityStep.txt id= " << id << " invalid order" << XEND;
        bCorrect = false;
        continue;
      }
      it->vecNode.push_back(nodebase);
    }
    else 
    {
      XERR << "[ActivityConfig] type=" << itemType << " invalid item type" << XEND;
      bCorrect = false;
      continue;
    }
  }

  if (bCorrect)
    XLOG << "[Table_Activity], 成功加载表格Table_ActivityStep.txt" << XEND;

  XLOG << "[Table_Activity], 成功加载表格Table_ActivityStep.txt, bCorrect"<<bCorrect << XEND;
  return bCorrect;
}

bool ActivityConfig::loadGlobalActConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_GlobalActivity.txt"))
  {
    XERR << "[表格],加载表格,Table_GlobalActivity.txt,失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GlobalActivity", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->first;
    //auto it = m_mapActivityCFG.find(id);
    //if (it != m_mapActivityCFG.end())
    //{
    //  XERR << "[Table_GlobalActivity] id = " << id << "在 Activity表中已经配置" << XEND;
    //  bCorrect = false;
    //  continue;
    //}
    SGlobalActCFG cfg;
    cfg.m_dwId = id;
    DWORD type = m->second.getTableInt("Type");
    if (type <GACTIVITY_MIN || type > GACTIVITY_MAX)
    {
      XERR << "[Table_GlobalActivity] id = " << id << "type 非法" << type << XEND;
      bCorrect = false;
      continue;
    }
    cfg.m_actType = static_cast<GlobalActivityType>(type);
    cfg.m_strName = m->second.getTableString("Name");
    cfg.m_dwLimitCount = m->second.getTableInt("LimitCount");
    DWORD limitType = m->second.getTableInt("LimitType");
    if (limitType >= EGlobalActLimitType_Max)
    {
      XERR << "[Table_GlobalActivity] id = " << id << "limittype 非法" << limitType << XEND;
      bCorrect = false;
      continue;
    }
    cfg.m_eLimitType = static_cast<EGlobalActLimitType>(m->second.getTableInt("LimitType"));
    auto paramF = [&](const std::string& key, xLuaData& data)
    {
      cfg.m_vecParam.push_back(data.getInt());
    };
    m->second.getMutableData("Param").foreach(paramF);

    //参数个数检测
    switch (cfg.m_actType)
    {
    case GACTIVITY_CHARGE_DISCOUNT:
    case GACTIVITY_CHARGE_EXTRA_REWARD:
    case GACTIVITY_AUGURY:
    case GACTIVITY_NOVICE_WELFARE:
      if (cfg.m_vecParam.size() != 2)
      {
        XERR << "[Table_GlobalActivity] id = " << id << type << "参数数量不对" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    case GACTIVITY_GUILD_QUEST:
    case GACTIVITY_GUILD_DONATE:
    case GACTIVITY_GUILD_FUBEN:
    case GACTIVITY_NORMAL_REFINE:
    case GACTIVITY_SAFE_REFINE:
    case GACTIVITY_CHARGE_EXTRA_COUNT:
    {
      if (cfg.m_vecParam.size() != 1)
      {
        XERR << "[Table_GlobalActivity] id = " << id << type << "参数数量不对" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    }
    case GACTIVITY_WEDDING_SERVICE:
      if (cfg.m_vecParam.size() % 2 != 0)
      {
        XERR << "[Table_GlobalActivity] id = " << id << type << "参数数量不对" << XEND;
        bCorrect = false;
        continue;
      }
      break;
    default:
      break;
    }

    m_mapGlobalActCFG[id] = cfg;  
  }

  if (bCorrect)
    XLOG << "[Table_GlobalActivity], 成功加载表格Table_GlobalActivity.txt" << XEND;
  else
    XERR << "[Table_GlobalActivity], 加载表格Table_GlobalActivity.txt, 有错误" << XEND;
  return bCorrect;
}

const SActivityCFG* ActivityConfig::getActivityCFG(DWORD dwID) const
{
  auto m = m_mapActivityCFG.find(dwID);
  if (m != m_mapActivityCFG.end())
    return &m->second;

  return nullptr;
}

const SGlobalActCFG* ActivityConfig::getGlobalActCFG(DWORD dwID) const
{
  auto m = m_mapGlobalActCFG.find(dwID);
  if (m != m_mapGlobalActCFG.end())
    return &m->second;

  return nullptr;
}

bool ActivityConfig::openNpcFunc(DWORD npcid, const TSetDWORD& funcSet)
{
  auto it = m_mapCloseNpcFunctions.find(npcid);
  if (it == m_mapCloseNpcFunctions.end())
  {
    XERR << "[Npc功能-开启], 未找到npcid:" << npcid << "有关闭的功能" << XEND;
    return false;
  }
  XLOG << "[Npc功能-开启], 开启功能:";
  for (auto &s : funcSet)
  {
    if (it->second.find(s) != it->second.end())
    {
      XLOG << s;
      it->second.erase(s);
    }
  }
  XLOG << XEND;

  if (it->second.empty())
  {
    m_mapCloseNpcFunctions.erase(it);
  }

  return true;
}

bool ActivityConfig::closeNpcFunc(DWORD npcid, const TSetDWORD& funcSet)
{
  TSetDWORD& funcs = m_mapCloseNpcFunctions[npcid];
  funcs.insert(funcSet.begin(), funcSet.end());

  XLOG << "[Npc功能-关闭], 关闭功能:";
  for (auto &s : funcSet)
    XLOG << s;
  XLOG << XEND;

  return true;
}

bool ActivityConfig::checkNpcFuncClose(DWORD npcid, DWORD funcid) const
{
  auto it = m_mapCloseNpcFunctions.find(npcid);
  if (it == m_mapCloseNpcFunctions.end())
    return false;
  return it->second.find(funcid) != it->second.end();
}

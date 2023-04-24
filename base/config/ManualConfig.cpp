#include "ManualConfig.h"
#include "RewardConfig.h"
#include "QuestConfig.h"
#include "ItemConfig.h"
#include "xLuaTable.h"
#include "TableManager.h"

// config
ManualConfig::ManualConfig()
{

}

ManualConfig::~ManualConfig()
{
  for (auto &m : m_mapReturnCFG)
    SAFE_DELETE(m.second);
  m_mapReturnCFG.clear();
}

bool ManualConfig::loadConfig()
{
  bool bResult = true;
  if (loadItemType() == false)
    bResult = false;
  if (loadAdventureLevelConfig() == false)
    bResult = false;
  if (loadAdventureAppendConfig() == false)
    bResult = false;
  if (loadCollectionConfig() == false)
    bResult = false;
  if (loadReturnConfig() == false)
    bResult = false;

  return bResult;
}

bool ManualConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto m = m_mapManualQuestCFG.begin(); m != m_mapManualQuestCFG.end(); ++m)
  {
    if (m->second.dwRewardID != 0 && RewardConfig::getMe().getRewardCFG(m->second.dwRewardID) == nullptr)
    {
      XERR << "[冒险手册-追加配置] id :" << m->first << "rewardid : " << m->second.dwRewardID << "未在 Table_Reward.txt 表中找到" << XEND;
      bCorrect = false;
    }
  }

  for (auto m = m_mapManualGroupCFG.begin(); m != m_mapManualGroupCFG.end(); ++m)
  {
    if (m->second.dwQuestID != 0 && QuestConfig::getMe().getQuestCFG(m->second.dwQuestID) == nullptr)
    {
      XERR << "[冒险手册-组配置] id :" << m->first << "questid : " << m->second.dwQuestID << "未在 Table_Quest.txt 表中找到" << XEND;
      bCorrect = false;
    }
    for (auto &s : m->second.setManualIDs)
    {
      if (ItemConfig::getMe().getItemCFG(s) == nullptr)
      {
        XERR << "[Table_Collection] id :" << m->first << "manualid : " << s << "未在 Table_Item.txt 表中找到" << XEND;
        m->second.blInit = bCorrect = false;
      }
    }
    for (auto &s : m->second.setRewardIDs)
    {
      if (RewardConfig::getMe().getRewardCFG(s) == nullptr)
      {
        XERR << "[Table_Collection] id :" << m->first << "rewardid : " << s << "未在 Table_Reward.txt 表中找到" << XEND;
        m->second.blInit = bCorrect = false;
      }
    }
  }

  for (auto &m : m_mapReturnCFG)
  {
    for (auto &v : m.second->vecItems)
    {
      if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_ManualReturn] id :" << m.first << "itemid :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
      }
    }
    for (auto &v : m.second->vecUnlock1Items)
    {
      if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_ManualReturn] id :" << m.first << "itemid :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
      }
    }
    for (auto &v : m.second->vecUnlock2Items)
    {
      if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_ManualReturn] id :" << m.first << "itemid :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
      }
    }
  }
  return bCorrect;
}

EManualType ManualConfig::getManualType(EItemType eType) const
{
  auto m = m_mapManualType.find(eType);
  if (m == m_mapManualType.end())
    return EMANUALTYPE_MIN;

  return m->second;
}

const SManualLvCFG* ManualConfig::getManualLvCFG(DWORD dwLv) const
{
  auto m = m_mapManualLvCFG.find(dwLv);
  if (m != m_mapManualLvCFG.end() && m->second.blInit == true)
    return &m->second;

  return nullptr;
}

const SManualQuestCFG* ManualConfig::getManualQuestCFG(DWORD dwID) const
{
  auto m = m_mapManualQuestCFG.find(dwID);
  if (m != m_mapManualQuestCFG.end() && m->second.blInit == true)
    return &m->second;

  return nullptr;
}

const SManualGroupCFG* ManualConfig::getManualGroupCFG(DWORD dwID) const
{
  auto m = m_mapManualGroupCFG.find(dwID);
  if (m != m_mapManualGroupCFG.end() && m->second.blInit == true)
    return &m->second;
  return nullptr;
}

const SManualReturnCFG* ManualConfig::getManualReturnCFG(DWORD dwID) const
{
  auto m = m_mapReturnCFG.find(dwID);
  if (m != m_mapReturnCFG.end() && m->second->blInit == true)
    return m->second;
  return nullptr;
}

void ManualConfig::collectQuest(EManualType eType, DWORD dwID, TVecManualQuestCFG& vecCFG)
{
  if (eType <= EMANUALTYPE_MIN || eType >= EMANUALTYPE_MAX || EManualType_IsValid(eType) == false)
    return;

   TMapTypeMQuestCFG& mapCFG = m_mapTypeMQuestCFG[eType];
   auto m = mapCFG.find(dwID);
   if (m == mapCFG.end())
     return;

   vecCFG.clear();
   for(auto c = m->second.begin(); c != m->second.end(); ++c)
   {
     if(c->blInit == true)
       vecCFG.push_back(*c);
   }
}

bool ManualConfig::loadItemType()
{
  // itemtypeadventurelog
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ItemTypeAdventureLog.txt"))
  {
    XERR << "[Table_ItemTypeAdventureLog.txt], 加载配置失败" << XEND;
    return false;
  }

  m_mapManualType.clear();
  map<DWORD, EManualType> mapTemp;
  map<DWORD, DWORD> redTmp;
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ItemTypeAdventureLog", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    redTmp[m->first] = m->second.getTableInt("RidTip");

    DWORD dwType = m->second.getTableInt("type");
    if (dwType == 0)
      continue;
    if (dwType <= EMANUALTYPE_MIN || dwType >= EMANUALTYPE_MAX)
      continue;

    auto o = mapTemp.find(m->first);
    if (o != mapTemp.end())
    {
      XERR << "[ItemTypeAdventureLog] id : " << m->first << " duplicated" << XEND;
      return false;
    }

    EManualType eType = static_cast<EManualType>(dwType);
    mapTemp[m->first] = eType;
  }
  if (bCorrect)
    XLOG << "[ItemTypeAdventureLog], 加载配置Table_ItemTypeAdventureLog.txt" << XEND;

  // itemtype
  if (!xLuaTable::getMe().open("Lua/Table/Table_ItemType.txt"))
  {
    XERR << "[Table_ItemType.txt], 加载配置失败" << XEND;
    return false;
  }

  table.clear();
  xLuaTable::getMe().getLuaTable("Table_ItemType", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwAdventure = m->second.getTableInt("AdventureLogGroup");
    if (dwAdventure == 0)
      continue;

    DWORD dwType = m->first;
    if (dwType <= EITEMTYPE_MIN || dwType >= EITEMTYPE_MAX)
      continue;

    auto o = mapTemp.find(dwAdventure);
    if (o == mapTemp.end())
      continue;

    EItemType eItemType = static_cast<EItemType>(dwType);
    m_mapManualType[eItemType] = o->second;
  }

  if (bCorrect)
    XLOG << "[ItemType], 加载配置Table_ItemType.txt" << XEND;
  return bCorrect;
}

bool ManualConfig::loadAdventureLevelConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_AdventureLevel.txt"))
  {
    XERR << "[Table_AdventureLevel.txt], 加载配置失败" << XEND;
    return false;
  }

  for(auto m = m_mapManualLvCFG.begin(); m != m_mapManualLvCFG.end(); ++m)
    m->second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_AdventureLevel", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SManualLvCFG& rCFG = m_mapManualLvCFG[m->first];
    rCFG.dwLevel = m->first;
    rCFG.dwNeedExp = m->second.getTableInt("AdventureExp");
    rCFG.dwSkillPoint = m->second.getTableInt("SkillPoint");

    auto attrf = [&](const string& key, xLuaData& data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_AdventureLevel] id :" << m->first << "attr : " << key.c_str() << "未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }
      EAttrType eType = static_cast<EAttrType>(dwAttr);

      UserAttrSvr& rAttr = rCFG.mapAttrs[eType];
      rAttr.set_type(eType);
      rAttr.set_value(rAttr.value() + data.getFloat());
    };
    m->second.getMutableData("AdventureAttr").foreach(attrf);
    if (!bCorrect)
      continue;

    rCFG.blInit = true;
  }

  map<EAttrType, float> mapLastAttr;
  for (auto &m : m_mapManualLvCFG)
  {
    for (auto &attr : m.second.mapAttrs)
    {
      float& value = mapLastAttr[attr.second.type()];
      attr.second.set_value(attr.second.value() + value);
      value = attr.second.value();
      XDBG << "[Table_AdventureLevel] lv :" << m.first << "attr :" << attr.second.ShortDebugString() << XEND;
    }
  }

  if (bCorrect)
    XLOG << "[Table_AdventureLevel] 成功加载配置Table_AdventureLevel.txt" << XEND;
  return bCorrect;
}

bool ManualConfig::loadAdventureAppendConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_AdventureAppend.txt"))
  {
    XERR << "[Table_AdventureAppend.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto m = m_mapManualQuestCFG.begin(); m != m_mapManualQuestCFG.end(); ++m)
  {
    m->second.blInit = false;
  }

  for (int i = EMANUALTYPE_MIN + 1; i < EMANUALTYPE_MAX; ++i)
  {
    for(auto m = m_mapTypeMQuestCFG[i].begin(); m != m_mapTypeMQuestCFG[i].end(); ++m)
    {
      for(auto c = m->second.begin(); c != m->second.end(); ++c)
        c->blInit = false;
    }
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_AdventureAppend", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= EMANUALTYPE_MIN || dwType >= EMANUALTYPE_MAX || EManualType_IsValid(dwType) == false)
    {
      XERR << "[冒险手册-加载追加] id : " << m->first << " type : " << dwType << " 不是合法的冒险类型" << XEND;
      bCorrect = false;
      continue;
    }

    EManualQuest eType = getManualQuest(m->second.getTableString("Content"));
    if (eType == EMANUALQUEST_MIN)
    {
      XERR << "[冒险手册-加载追加] id : " << m->first << " type : " << dwType << " content : " << m->second.getTableString("Content") << " 不是合法的类型" << XEND;
      bCorrect = false;
      continue;
    }

    TMapTypeMQuestCFG& mapCFG = m_mapTypeMQuestCFG[dwType];

    SManualQuestCFG stCFG;
    stCFG.dwID = m->first;
    stCFG.dwTargetID = m->second.getTableInt("targetID");
    stCFG.dwRewardID = m->second.getTableInt("Reward");
    stCFG.dwBuffID = m->second.getTableInt("BuffID");
    stCFG.eManualType = static_cast<EManualType>(dwType);
    stCFG.eQuestType = eType;
    stCFG.blInit = true;

    xLuaData& pre = m->second.getMutableData("PreID");
    auto pref = [&](const string& str, xLuaData& data)
    {
      stCFG.setPreIDs.insert(data.getInt());
    };
    pre.foreach(pref);

    xLuaData& params = m->second.getMutableData("Params");
    auto paramsf = [&](const string& str, xLuaData& data)
    {
      stCFG.vecParams.push_back(data.getInt());
    };
    params.foreach(paramsf);

    bool blNew = true;
    auto smanual = mapCFG.find(stCFG.dwTargetID);
    if(smanual != mapCFG.end())
    {
      for(auto m = smanual->second.begin(); m != smanual->second.end(); ++m)
        if(m->dwID == stCFG.dwID)
        {
          *m = stCFG;
          blNew = false;
          break;
        }

    }
    if(blNew == true)
      mapCFG[stCFG.dwTargetID].push_back(stCFG);
    m_mapManualQuestCFG[stCFG.dwID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[冒险手册-加载追加] 成功加载配置Table_AdventureAppend.txt" << XEND;
  return bCorrect;
}

bool ManualConfig::loadCollectionConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Collection.txt"))
  {
    XERR << "[Table_Collection.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto &m : m_mapManualGroupCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Collection", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SManualGroupCFG& rCFG = m_mapManualGroupCFG[m->first];

    rCFG.dwGroupID = m->first;
    rCFG.dwQuestID = m->second.getTableInt("QuestID");

    auto itemf = [&](const string& key, xLuaData& data)
    {
      rCFG.setManualIDs.insert(data.getInt());
    };
    m->second.getMutableData("ItemId").foreach(itemf);

    xLuaData& reward = m->second.getMutableData("AdventureReward");
    auto rewardf = [&](const string& key, xLuaData& data)
    {
      rCFG.setRewardIDs.insert(data.getInt());
    };
    reward.foreach(rewardf);

    auto attrf = [&](const string& key, xLuaData& data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_Collection] id :" << m->first << "attr : " << key.c_str() << "未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }
      EAttrType eType = static_cast<EAttrType>(dwAttr);

      UserAttrSvr& rAttr = rCFG.mapAttrs[eType];
      rAttr.set_type(eType);
      rAttr.set_value(rAttr.value() + data.getFloat());
    };
    m->second.getMutableData("RewardProperty").foreach(attrf);

    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_Collection] 成功加载" << XEND;
  return bCorrect;
}

bool ManualConfig::loadReturnConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_ManualReturn.txt"))
  {
    XERR << "[Table_ManualReturn.txt], 加载配置失败" << XEND;
    return false;
  }

  for (auto m : m_mapReturnCFG)
    m.second->blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ManualReturn", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto item = m_mapReturnCFG.find(m->first);
    if (item == m_mapReturnCFG.end())
    {
      SManualReturnCFG* pCFG = new SManualReturnCFG();
      m_mapReturnCFG.insert(std::make_pair(m->first, pCFG));

      item = m_mapReturnCFG.find(m->first);
      if (item == m_mapReturnCFG.end())
      {
        bCorrect = false;
        XERR << "[Table_ManualReturn] id :" << m->first << "创建失败" << XEND;
        continue;
      }
    }

    SManualReturnCFG* pCFG = item->second;

    pCFG->dwID = m->first;

    DWORD dwType = 0;
    auto itemf = [&](const string& key, xLuaData& data)
    {
      DWORD dwID = data.getTableInt("1");
      if (dwID == 0)
        return;

      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      if (dwType == 0)
        combinItemInfo(pCFG->vecItems, oItem);
      else if (dwType == 1)
        combinItemInfo(pCFG->vecUnlock1Items, oItem);
      else if (dwType == 2)
        combinItemInfo(pCFG->vecUnlock2Items, oItem);
    };
    m->second.getMutableData("item").foreach(itemf);
    dwType = 1;
    m->second.getMutableData("unlock_1").foreach(itemf);
    dwType = 2;
    m->second.getMutableData("unlock_2").foreach(itemf);

    pCFG->blInit = true;
  }

  if (bCorrect)
    XLOG << "[Table_ManualReturn] 成功加载" << XEND;
  return bCorrect;
}

EManualQuest ManualConfig::getManualQuest(const string& str)
{
  if (str == "kill")
    return EMANUALQUEST_KILL;
  if (str == "selfie")
    return EMANUALQUEST_PHOTO;
  if (str == "active")
    return EMANUALQUEST_ACTIVE;

  return EMANUALQUEST_MIN;
}


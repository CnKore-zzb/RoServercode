#include "MapConfig.h"
#include "ItemConfig.h"
#include "xLuaTable.h"

// config data
bool SMapCFG::hasFunction(EMapFunction eFunction) const
{
  DWORD dtype = static_cast<DWORD>(eFunction);
  if (dtype <= 0 || dtype >= 32)
    return false;
  dtype --;
  return dwFunction & (1 << dtype);
}

DWORD SMapCFG::getMonsterRatioBuff(DWORD npcid) const
{
  auto s = stMapMonsterRatioCFG.setMonsterIDs.find(npcid);
  if(s != stMapMonsterRatioCFG.setMonsterIDs.end())
    return stMapMonsterRatioCFG.dwBuffID;

  return 0;
}

DWORD SMapCFG::getMonsterRewardRatio(DWORD npcid) const
{
  auto s = stMapMonsterRatioCFG.setMonsterIDs.find(npcid);
  if(s != stMapMonsterRatioCFG.setMonsterIDs.end())
    return stMapMonsterRatioCFG.dwRatio;

  return 0;
}

// config
MapConfig::MapConfig()
{

}

MapConfig::~MapConfig()
{

}

bool MapConfig::loadConfig()
{
  bool bResult = true;
  if (loadMapConfig() == false)
    bResult = false;
  if (loadRaidMapConfig() == false)
    bResult = false;
  if (loadRaidMonsterConfig() == false)
    bResult = false;
  return bResult;
}

bool MapConfig::checkConfig()
{
  bool bCorrect = true;

  for (auto &m : m_mapMapCFG)
  {
    if (getMapCFG(m.second.dwReliveMap) == nullptr && m.first != VIRTUAL_MAP)
    {
      XERR << "[场景配置-配置检查] mapid : " << m.first << " relivemap : " << m.second.dwReliveMap << " 未在 Table_Map.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    for (auto &v : m.second.vecManualReward)
    {
      if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
      {
        XERR << "[场景配置-配置检查] mapid : " << m.first << " item : " << v.id() << " 未在 Table_Item.txt 表中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }
  }

  return bCorrect;
}

const SMapCFG* MapConfig::getMapCFG(DWORD dwMapID) const
{
  auto m = m_mapMapCFG.find(dwMapID);
  if (m != m_mapMapCFG.end())
    return &m->second;

  return nullptr;
}

const SRaidCFG* MapConfig::getRaidCFG(DWORD dwRaidID) const
{
  auto m = m_mapRaidCFG.find(dwRaidID);
  if (m != m_mapRaidCFG.end())
    return &m->second;

  return nullptr;
}

bool MapConfig::loadMapConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Map.txt"))
  {
    XERR << "[场景配置-加载地图] 加载配置 Table_Map.txt 失败" << XEND;
    return false;
  }

  m_setUnopenMapIDs.clear();
  m_mapMapCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Map", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    const string& nameen = m->second.getTableString("NameEn");
    if (nameen.empty() == true)
    {
      m_setUnopenMapIDs.insert(m->first);
      XLOG << "[场景配置-加载地图] id : " << m->first << " NameEn为空 未开放" << XEND;
      continue;
    }

    auto map = m_mapMapCFG.find(m->first);
    if (map != m_mapMapCFG.end())
    {
      XERR << "[场景配置-加载地图] id : " << m->first << " 重复了" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD dwType = m->second.getTableInt("Type");
    if (m->first != VIRTUAL_SCENE_ID)
    {
      if (dwType <= EMAPTYPE_MIN || dwType >= EMAPTYPE_MAX)
      {
        XERR << "[场景配置-加载地图] id : " << m->first << " type : " << dwType << " 不合法的类型" << XEND;
        bCorrect = false;
        continue;
      }
    }

    SMapCFG stCFG;
    stCFG.dwID = m->first;
    stCFG.dwReliveMap = m->second.getMutableData("ReliveMapID").getTableInt("1");
    stCFG.dwReliveBp = m->second.getMutableData("ReliveMapID").getTableInt("2");
    stCFG.dwArea = m->second.getTableInt("MapArea");
    stCFG.dwRange = m->second.getTableInt("Range");
    stCFG.dwFunction = m->second.getTableInt("MapCompositeFunction");
    stCFG.eTransType = static_cast<ETransType>(m->second.getTableInt("MoneyType"));
    stCFG.dwTransMoney = m->second.getTableInt("Money");
    stCFG.swAdventureValue = m->second.getTableInt("AdventureValue");

    stCFG.bStatic = m->second.getTableInt("StaticMap") == 1;
    // stCFG.bPvp = m->second.getTableInt("PVPmap") == 1;
    stCFG.bPvp = m->second.getTableInt("Mode") == 2;
    stCFG.dwStaticGroup = m->second.getTableInt("StaticMapGroup");
    stCFG.bPreview = m->second.getTableInt("SceneAnimation") == 1;
    stCFG.dwPetLimit = m->second.getTableInt("NoCat");


    xLuaData indexRange = m->second.getMutableData("IndexRange");
    stCFG.nIndexRangeMin = indexRange.getTableInt("1");
    stCFG.nIndexRangeMax = indexRange.getTableInt("2");

    stCFG.strMapBgm.clear();
    if (m->second.has("Bgm"))
      stCFG.strMapBgm = m->second.getTableString("Bgm");

    stCFG.eType = static_cast<EMapType>(dwType);

    stCFG.strNameZh = m->second.getTableString("NameZh");
    stCFG.strNameEn = m->second.getTableString("NameEn");

    xLuaData& sky = m->second.getMutableData("SkyType");
    auto skyf = [&](const string& str, xLuaData& data)
    {
      DWORD dwId = data.getInt();
      stCFG.stSkyCFG.push_back(dwId);
    };
    sky.foreach(skyf);

    xLuaData weather = m->second.getMutableData("WeatherType");
    auto weatherf = [&](const string& str, xLuaData& data)
    {
      SWeatherSkyItem stItem;
      stItem.dwID = data.getTableInt("1");
      stItem.dwRate = data.getTableInt("2");
      stItem.dwMaxTime = data.getTableInt("3") * 3600 / 4;// / MiscConfig::getMe().getSystemCFG().dwTimeSpeed;
     
      xLuaData &weatherSky = data.getMutableData("skytype");
      
      DWORD t = weatherSky.getTableInt("1");
      if (t) stItem.vecSky.push_back(t);
      t = weatherSky.getTableInt("2");
      if (t) stItem.vecSky.push_back(t);
      t = weatherSky.getTableInt("3");
      if (t) stItem.vecSky.push_back(t);
      t = weatherSky.getTableInt("4");
      if (t) stItem.vecSky.push_back(t);      
      stCFG.stWeatherCFG.vecItems.push_back(stItem);
    };
    weather.foreach(weatherf);

    xLuaData& monsterratio = m->second.getMutableData("MonsterRatio");
    stCFG.stMapMonsterRatioCFG.dwBuffID = monsterratio.getTableInt("buffid");
    stCFG.stMapMonsterRatioCFG.dwRatio = monsterratio.getTableInt("ratio");
    auto monsterfunc = [&](const string& str, xLuaData& data)
    {
      stCFG.stMapMonsterRatioCFG.setMonsterIDs.insert(data.getInt());
    };
    monsterratio.getMutableData("monster").foreach(monsterfunc);

    // calc rate
    for (auto &v : stCFG.stWeatherCFG.vecItems)
    {
      v.dwRate += stCFG.stWeatherCFG.dwMaxRate;
      stCFG.stWeatherCFG.dwMaxRate = v.dwRate;
    }

    xLuaData& adventure = m->second.getMutableData("AdventureReward");

    xLuaData& buff = adventure.getMutableData("buffid");
    auto bufff = [&](const string& str, xLuaData& data)
    {
      stCFG.setManualBuffIDs.insert(data.getInt());
    };
    buff.foreach(bufff);

    xLuaData& item = adventure.getMutableData("item");
    auto itemf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_MANUAL);
      stCFG.vecManualReward.push_back(oItem);
    };
    item.foreach(itemf);

    //type="activity", id=23
    xLuaData& enterCond = m->second.getMutableData("EnterCond");
    string t = enterCond.getTableString("type");
    if ("activity" == t)
      stCFG.enterCond.type = EENTERCONDTYPE_ACTIVITY;
    else
      stCFG.enterCond.type = EENTERCONDTYPE_NONE;

    stCFG.enterCond.param = enterCond.getTableInt("id");
    XDBG << "[wld] mapid" << m->first <<stCFG.enterCond.type << stCFG.enterCond.param << XEND;

    // insert to list
    m_mapMapCFG[stCFG.dwID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[场景配置-加载地图] 成功加载配置 Table_Map.txt" << XEND;
  return bCorrect;
}

bool MapConfig::loadRaidMapConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_MapRaid.txt"))
  {
    XERR << "[场景配置-加载副本] 加载配置 Table_MapRaid.txt 失败" << XEND;
    return false;
  }

  m_mapRaidCFG.clear();
  m_mapItemImage.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MapRaid", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwMapID = m->second.getTableInt("Map");
    auto map = m_mapMapCFG.find(dwMapID);
    if (map == m_mapMapCFG.end())
    {
      XERR << "[场景配置-加载副本] id : " << m->first << " mapid : " << dwMapID << " 未在 Table_Map.txt 表中找到" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD dwRaidType = m->second.getTableInt("Type");
    if (dwRaidType < ERAIDTYPE_MIN || dwRaidType > ERAIDTYPE_MAX)
    {
      XERR << "[场景配置-加载副本] id : " << m->first << " Type : " << dwRaidType << " 不合法" << XEND;
      bCorrect = false;
      continue;
    }

    DWORD dwRestrict = m->second.getTableInt("Restrict");
    if (dwRestrict <= ERAIDRESTRICT_MIN || dwRestrict >= ERAIDRESTRICT_MAX)
    {
      XERR << "[场景配置-加载副本] id : " << m->first << " Restrict : " << dwRestrict << " 不合法" << XEND;
      bCorrect = false;
      continue;
    }

    SRaidCFG stCFG;

    stCFG.dwRaidID = m->first;
    stCFG.dwMapID = map->second.dwID;
    stCFG.dwRaidLimitTime = m->second.getTableInt("TimeLimit");
    stCFG.dwRaidTimeNoUser = m->second.getTableInt("TimeNone");
    stCFG.dwRaidEndWait = m->second.getTableInt("EndWait");
    stCFG.bShowAllNpc = m->second.getTableInt("ShowAllNpc") != 0;
    stCFG.bQuestFail = m->second.getTableInt("QuestFail") != 0;

    stCFG.eRaidType = static_cast<ERaidType>(dwRaidType);
    stCFG.eRestrict = static_cast<ERaidRestrict>(dwRestrict);

    stCFG.strNameZh = m->second.getTableString("NameZh");
    stCFG.strNameEn = m->second.getTableString("NameEn");

    stCFG.bNoPlayGoMap = m->second.getTableInt("NoPlayGoMap");
    stCFG.dwLeaveImageTime = m->second.getTableInt("LeaveImageTime");

    stCFG.dwLimitLv = m->second.getTableInt("LimitLv");

    m_mapRaidCFG[stCFG.dwRaidID] = stCFG;

    if (stCFG.eRaidType == ERAIDTYPE_ITEMIMAGE)
    {
      m_mapItemImage[stCFG.dwMapID] = stCFG.dwRaidID;
    }
  }

  if (bCorrect)
    XLOG << "[场景配置-加载副本] 成功加载配置 Table_MapRaid.txt" << XEND;
  return bCorrect;
}

DWORD MapConfig::getItemImageRaid(DWORD mapId) const
{
  auto it = m_mapItemImage.find(mapId);
  if (it == m_mapItemImage.end())
    return 0;
  return it->second;
}

bool MapConfig::loadRaidMonsterConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Raid_Monster.txt"))
  {
    XERR << "[配置-加载], Table_Raid_Monster.txt 加载失败" << XEND;
    return false;
  }

  m_mapRaidMonsterCFG.clear();
  m_mapRaidMonsterUids.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Raid_Monster", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD groupid = m->second.getTableInt("GroupID");
    TVecRaidMonster& vecMonster = m_mapRaidMonsterCFG[groupid];
    SRaidMonsterCFG stCFG;
    stCFG.dwGroupID = groupid;
    stCFG.dwWeight = m->second.getTableInt("Weight");
    if (!vecMonster.empty())
      stCFG.dwWeight += vecMonster.back().dwWeight;

    TSetDWORD& setUids = m_mapRaidMonsterUids[groupid];
    auto getids = [&](const string& key, xLuaData& d)
    {
      DWORD uid = d.getTableInt("uid");
      if (d.has("id") == false && d.has("ids") == false)
        return;
      setUids.insert(uid);
      TVecDWORD& vecids = stCFG.mapUniqueID2NpcIDs[uid];
      if (d.has("id"))
      {
        vecids.push_back(d.getTableInt("id"));
      }
      else if (d.has("ids"))
      {
        auto funcids = [&](const string& keyer, xLuaData& der)
        {
          vecids.push_back(der.getInt());
        };
        d.getMutableData("ids").foreach(funcids);
      }
    };
    m->second.getMutableData("UniqueID").foreach(getids);

    vecMonster.push_back(stCFG);
  }
  if (bCorrect)
    XLOG << "[配置-加载], Table_Raid_Monster.txt加载成功" << XEND;

  return bCorrect;
}

void MapConfig::getRaidMonster(DWORD groupid, map<DWORD, DWORD>& outUid2NpcID) const
{
  auto it = m_mapRaidMonsterCFG.find(groupid);
  if (it == m_mapRaidMonsterCFG.end() || it->second.empty())
    return;

  DWORD totalWeight = it->second.back().dwWeight;
  DWORD rand = randBetween(1, totalWeight);
  for (auto &v : it->second)
  {
    if (rand <= v.dwWeight)
    {
      for (auto m : v.mapUniqueID2NpcIDs)
      {
        DWORD index = m.second.size();
        if (index == 0)
          continue;
        index = randBetween(0, index - 1);
        outUid2NpcID[m.first] = m.second[index];
      }
      return;
    }
  }
}

const TSetDWORD& MapConfig::getRaidMonsterUids(DWORD dwGroupID) const
{
  auto it = m_mapRaidMonsterUids.find(dwGroupID);
  if (it == m_mapRaidMonsterUids.end())
  {
    static const TSetDWORD emptySet;
    return emptySet;
  }
  return it->second;
}

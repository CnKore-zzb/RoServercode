#include "GuildConfig.h"
#include "xLuaTable.h"
#include "RoleDataConfig.h"
#include "ItemConfig.h"
#include "MiscConfig.h"

// config data
const SGuildPrayItemCFG* SGuildPrayCFG::getItem(DWORD dwLv) const
{
  if (!blInit)
    return nullptr;

  auto m = mapLvItem.find(dwLv);
  if (m == mapLvItem.end())
    return nullptr;

  return &m->second;
}

void SGuildPrayCFG::toData(GuildPrayCFG* pCFG, DWORD dwLv) const
{
  if (pCFG == nullptr)
    return;

  pCFG->Clear();
  const SGuildPrayItemCFG* pItem = getItem(dwLv);
  if (pItem == nullptr)
    return;

  pCFG->set_type(eType);
  pCFG->set_prayid(dwID);
  pCFG->set_praylv(dwLv);

  for (auto &v : pItem->vecAttrs)
  {
    UserAttr* pAttr = pCFG->add_attrs();
    pAttr->set_type(v.type());
    pAttr->set_value(round(v.value() * TEN_THOUSAND));
  }
  for (auto &v : pItem->vecCosts)
    pCFG->add_costs()->CopyFrom(v);
}

bool SGuildFuncCFG::isOverTime() const
{
  DWORD dwNow = xTime::getCurSec();
  DWORD dwDay = xTime::getWeek(dwNow);

  if (dwNow < dwStartTime || dwNow > dwEndTime)
    return true;

  if (vecPeriod.empty() == false)
  {
    auto v = find(vecPeriod.begin(), vecPeriod.end(), dwDay);
    if (v == vecPeriod.end())
      return true;
  }

  if (vecDayPeriod.empty() == true)
    return false;
  DWORD dwTime = xTime::getHour(dwNow) * 3600 + xTime::getMin(dwNow) * 60;
  auto v = find_if(vecDayPeriod.begin(), vecDayPeriod.end(), [dwTime](const pair<DWORD, DWORD>& p) -> bool{
    return dwTime >= p.first && dwTime <= p.second;
  });

  return v == vecDayPeriod.end();
}

const SGuildDonateItem* SGuildDonateCFG::randDonateItem() const
{
  if (vecItems.empty())
    return nullptr;
  DWORD dwRand = randBetween(0, (DWORD)(vecItems.size() - 1));
  return &(vecItems[dwRand]);
}

void SGuildCFG::setCommonDivisor()
{
  auto commndivisor = [](DWORD number1, DWORD number2)
  {
    DWORD ret = number2;
    while(number1%number2 != 0)
    {
      ret = number1%number2;
      number1 = number2;
      number2 = ret;
    }
    return ret;
  };

  DWORD num1 = commndivisor(dwDonateRefreshInterval1, dwDonateRefreshInterval2);
  DWORD num2 = commndivisor(dwDonateRefreshInterval3, dwDonateRefreshInterval4);
  dwCommonDivisor = commndivisor(num1, num2);
}

// guild quest
void SGuildQuestCFG::toData(GuildQuest* pQuest) const
{
  if (pQuest == nullptr)
    return;
  pQuest->set_questid(dwQuestID);
  pQuest->set_time(dwTime);
}

// config
GuildConfig::GuildConfig()
{

}

GuildConfig::~GuildConfig()
{

}

bool GuildConfig::loadConfig()
{
  bool bResult = true;
  if (loadGuildConfig() == false)
    bResult = false;
  if (loadGuildPray() == false)
    bResult = false;
  if (loadGuildFunc() == false)
    bResult = false;
  if (loadGuildDonate() == false)
    bResult = false;
  if (loadGuildQuestConfig() == false)
    bResult = false;
  if (loadGuildPhotoFrame() == false)
    bResult = false;
  if (loadGuildJob() == false)
    bResult = false;
  if (loadGuildBuilding() == false)
    bResult = false;
  if (loadGuildBuildingMaterial() == false)
    bResult = false;
  if (loadGuildChallenge() == false)
    bResult = false;
  if (loadArtifact() == false)
    bResult = false;
  if (loadGuildTreasure() == false)
    bResult = false;

  return bResult;
}

#include "QuestConfig.h"
#include "NpcConfig.h"
bool GuildConfig::checkConfig()
{
  bool bCorrect = true;

  // check function
  for (auto &m : m_mapGuildFuncCFG)
  {
    for (auto &s : m.second.setQuestIDs)
    {
      if (QuestConfig::getMe().getQuestCFG(s) == nullptr)
      {
        bCorrect = false;
        XERR << "[公会配置-功能检查] id :" << s << "未在 Table_Quest.txt 表中找到" << XEND;
      }
    }
  }

  // check donate cfg
  for (auto &m : m_mapDonateCFG)
  {
    for (auto &v : m.second.vecItems)
    {
      if (ItemConfig::getMe().getItemCFG(v.dwItemID) == nullptr)
      {
        bCorrect = false;
        XERR << "[公会配置-捐赠检查] ID" << m.first << "itemid :" << v.dwItemID << "未在 Table_Item.txt 表中找到" << XEND;
      }
    }
  }

  // check quest
  for (auto &m : m_mapQuestCFG)
  {
    for (auto &v : m.second)
    {
      if (QuestConfig::getMe().getQuestCFG(v.dwQuestID) == nullptr)
      {
        bCorrect = false;
        XERR << "[公会配置-任务检查] id :" << v.dwQuestID << "未在 Table_Quest.txt 表中找到" << XEND;
      }
      if (NpcConfig::getMe().getNpcCFG(v.dwNpcID) == nullptr)
      {
        bCorrect = false;
        XERR << "[公会配置-任务检查] id :" << v.dwQuestID << "npcid :" << v.dwNpcID << "未在 Table_Npc.txt 表中找到" << XEND;
      }
      if (getGuildCFG(v.dwGuildLv) == nullptr)
      {
        bCorrect = false;
        XERR << "[公会配置-任务检查] id :" << v.dwQuestID << "lv :" << v.dwGuildLv << "未在 Table_Guild.txt 表中找到" << XEND;
      }
    }
  }

  return bCorrect;
}

const SGuildCFG* GuildConfig::getGuildCFG(DWORD dwLv) const
{
  auto m = m_mapGuildCFG.find(dwLv);
  if (m != m_mapGuildCFG.end())
    return &m->second;

  return nullptr;
}

const SGuildPrayCFG* GuildConfig::getGuildPrayCFG(DWORD dwPrayID) const
{
  auto m = m_mapGuildPrayCFG.find(dwPrayID);
  if (m != m_mapGuildPrayCFG.end() && m->second.blInit)
    return &m->second;

  return nullptr;
}

const SGuildFuncCFG* GuildConfig::getGuildFuncCFG(DWORD dwID) const
{
  auto m = m_mapGuildFuncCFG.find(dwID);
  if (m != m_mapGuildFuncCFG.end())
    return &m->second;

  return nullptr;
}

const SGuildDonateCFG* GuildConfig::getGuildDonateCFG(DWORD dwID) const
{
  auto m = m_mapDonateCFG.find(dwID);
  if (m != m_mapDonateCFG.end())
    return &m->second;

  return nullptr;
}

const SGuildDonateCFG* GuildConfig::randGuildDonateCFG(DWORD dwLv, EDonateType eType) const
{
  if (eType <= EDONATETYPE_MIN || eType >= EDONATETYPE_MAX)
    return nullptr;
  
  std::vector<std::pair<DWORD, DWORD >> vecWeight;

  DWORD weight = 0;
  for (auto &m : m_mapDonateCFG)
  {
    if (m.second.dwIfNoActive == 1 && dwLv >= m.second.lvrange.first && dwLv <= m.second.lvrange.second && m.second.eType == eType)
    {
      weight += m.second.dwWeight;
      vecWeight.push_back(std::make_pair(weight, m.second.dwID));
    }
  }

  if (vecWeight.empty())
    return nullptr;

  DWORD r = randBetween(1, weight);  //[1, weight]
  
  DWORD id = 0;
  for (auto &v : vecWeight)
  {
    if (r <= v.first)
    {
      id = v.second;
      break;
    }
  }
  auto it = m_mapDonateCFG.find(id);
  if (it == m_mapDonateCFG.end())
    return nullptr;

  return &(it->second);
}

const SGuildQuestCFG* GuildConfig::getGuildQuestCFG(DWORD dwQuestID) const
{
  for (auto &m : m_mapQuestCFG)
  {
    auto s = find_if(m.second.begin(), m.second.end(), [dwQuestID](const SGuildQuestCFG& r) -> bool{
      return r.dwQuestID == dwQuestID;
    });
    if (s != m.second.end())
      return &(*s);
  }
  return nullptr;
}

const SGuildQuestCFG* GuildConfig::randomGuildQuest(DWORD dwGuildLv, DWORD dwActiveMember, const TListGuildQuestCFG& setExclude)
{
  TListGuildQuestCFG listCFG;
  for (auto &m : m_mapQuestCFG)
  {
    DWORD dwType = m.first;
    auto s = find_if(setExclude.begin(), setExclude.end(), [dwType](const SGuildQuestCFG& r) -> bool{
      return r.dwType == dwType;
    });
    if (s != setExclude.end())
      continue;

    for (auto &s : m.second)
    {
      if (dwGuildLv >= s.dwGuildLv && dwActiveMember >= s.dwActiveMember)
        listCFG.push_back(s);
    }
  }

  DWORD dwIndex = 0;
  DWORD dwMax = 40;
  while (dwIndex++ < dwMax)
  {
    SGuildQuestCFG* pCFG = randomStlContainer(listCFG);
    if (pCFG == nullptr)
      continue;
    DWORD dwQuestID = pCFG->dwQuestID;
    auto s = find_if(setExclude.begin(), setExclude.end(), [dwQuestID](const SGuildQuestCFG& r) -> bool{
      return r.dwQuestID == dwQuestID;
    });
    if (s != setExclude.end())
      continue;

    return getGuildQuestCFG(dwQuestID);
  }

  return nullptr;
}

const SGuildJobCFG* GuildConfig::getGuildJobCFG(EGuildJob eJob) const
{
  auto m = m_mapJobCFG.find(eJob);
  if (m != m_mapJobCFG.end())
    return &m->second;
  return nullptr;
}

bool GuildConfig::isSuitableFrame(DWORD dwID, DWORD dwAngleZ) const
{
  EGuildFrameType eType = getAngleFrameType(dwAngleZ);
  for (auto &m : m_mapPhotoFrameCFG)
  {
    auto s = m.second.find(dwID);
    if (s != m.second.end())
      return eType == m.first;
  }

  return false;
}

EGuildFrameType GuildConfig::getPhotoFrameType(DWORD dwID) const
{
  for (auto &m : m_mapPhotoFrameCFG)
  {
    auto s = m.second.find(dwID);
    if (s != m.second.end())
      return m.first;
  }
  return EGUILDFRAMETYPE_MAX;
}

EGuildFrameType GuildConfig::getAngleFrameType(DWORD dwAngleZ) const
{
  if (dwAngleZ <= 45 || (dwAngleZ >= 135 && dwAngleZ <= 235) || (dwAngleZ >= 325 && dwAngleZ <= 360))
    return EGUILDFRAMETYPE_HORIZONTAL;
  return EGUILDFRAMETYPE_VERTICAL;
}

DWORD GuildConfig::getMaxPrayLv() const
{
  DWORD lv = 0;
  for(auto s : m_mapGuildCFG)
  {
    if(s.second.dwMaxPrayLv > lv)
      lv = s.second.dwMaxPrayLv;
  }

  return lv;
}

const SGuildBuildingCFG* GuildConfig::getGuildBuildingCFG(EGuildBuilding type, DWORD lv) const
{
  auto it = m_mapBuildingCFG.find(type);
  if (it == m_mapBuildingCFG.end())
    return nullptr;
  auto m = it->second.find(lv);
  if (m == it->second.end())
    return nullptr;
  return &m->second;
}

bool SGuildFuncBuildingParam::isShowNpc(const GuildBuilding* b) const
{
  if (b == nullptr)
    return false;

  if (b->level() < dwShowLv)
    return false;

  if (bShowWhenBuilding && b->isbuilding())
    return true;
  if (bShowAfterBuilding && b->level() >= 1)
    return true;
  if (!bShowWhenBuilding && !bShowAfterBuilding)
    return true;

  return false;
}

bool SGuildFuncBuildingParam::isDisNpc(const GuildBuilding* b) const
{
  if (b == nullptr)
    return true;

  if (b->level() >= dwDisLv)
    return true;

  if (bDisWhenNotBuilding && b->isbuilding() == false)
    return true;

  return false;
}

EGuildBuilding GuildConfig::getGuildBuildingByShopType(DWORD shoptype) const
{
  auto it = m_mapShopType2Building.find(shoptype);
  if (it == m_mapShopType2Building.end())
    return EGUILDBUILDING_MIN;
  return it->second;
}

const SGuildBuildingMaterialCFG* GuildConfig::getGuildBuildingMaterial(DWORD id) const
{
  auto it = m_mapBuildingMaterialCFG.find(id);
  if (it == m_mapBuildingMaterialCFG.end())
    return nullptr;
  return &it->second;
}

bool SGuildChallengeCFG::checkValid(QWORD value) const
{
  switch (eType)
  {
  case EGUILDCHALLENGE_LOGIN:
  case EGUILDCHALLENGE_GUILD_RAID:
  case EGUILDCHALLENGE_GUILD_QUEST:
  case EGUILDCHALLENGE_GVG:
  case EGUILDCHALLENGE_SEAL:
  case EGUILDCHALLENGE_WANTED_QUEST:
  case EGUILDCHALLENGE_KILL_MVP:
  case EGUILDCHALLENGE_KILL_MINI:
    return true;
  case EGUILDCHALLENGE_ENDLESS_TOWER:
    return oParam.has("layer") ? value >= QWORD(oParam.getTableInt("layer")) : true;
  default:
    return false;
  }
  return false;
}

bool SGuildTreasureCFG::collectMemberReward(DWORD dwTime, TSetDWORD& setIDs) const
{
  if (mapMemberReward.empty() == true)
    return true;
  auto m = mapMemberReward.find(dwTime);
  if (m == mapMemberReward.end())
    return false;
  setIDs.clear();
  setIDs = m->second;
  return true;
}

const SGuildChallengeCFG* GuildConfig::getGuildChallengeCFG(DWORD id) const
{
  auto it = m_mapChallengeCFG.find(id);
  if (it == m_mapChallengeCFG.end())
    return nullptr;
  return &it->second;
}

const TVecGuildChallengeCFG& GuildConfig::getGuildChallengeCFGByType(EGuildChallenge type) const
{
  static TVecGuildChallengeCFG empty;
  auto it = m_mapType2Challenge.find(type);
  if (it == m_mapType2Challenge.end())
    return empty;
  return it->second;
}

const TVecDWORD& GuildConfig::getGuildChallengeIDByGroup(DWORD groupid) const
{
  static TVecDWORD empty;
  auto it = m_mapGroup2ChallengeID.find(groupid);
  if (it == m_mapGroup2ChallengeID.end())
    return empty;
  return it->second;
}

void GuildConfig::randGuildChallenge(TVecDWORD& ids, TVecDWORD& extrarewardids, DWORD guildlv, DWORD count) const
{
  if (count <= 0)
    return;

  static map<DWORD, pair<DWORD, DWORD>> group2lvweight;
  if (group2lvweight.empty())
  {
    for (auto& v : m_mapChallengeCFG)
    {
      auto it = group2lvweight.find(v.second.dwGroupID);
      if (it != group2lvweight.end())
        continue;
      group2lvweight[v.second.dwGroupID] = pair<DWORD, DWORD>(v.second.dwGuildLevel, v.second.dwWeight);
    }
  }

  DWORD total = 0;
  for (auto& v : group2lvweight)
    if (guildlv >= v.second.first)
      total += v.second.second;

  set<DWORD> results;
  while (count--)
  {
    DWORD w = randBetween(1, total), t = 0;
    for (auto& v : group2lvweight)
    {
      if (results.find(v.first) != results.end())
        continue;
      if (v.second.first > guildlv)
        continue;
      t += v.second.second;
      if (w > t)
        continue;
      if (total >= v.second.second)
        total -= v.second.second;
      results.insert(v.first);
      break;
    }
  }

  if (results.empty() == false)
  {
    DWORD r = randBetween(1, results.size());
    DWORD idx = 0;
    for (auto groupid : results)
    {
      idx += 1;
      auto it = m_mapGroup2ChallengeID.find(groupid);
      if (it == m_mapGroup2ChallengeID.end())
        continue;

      if (idx == r) // 有额外奖励的任务
        for (auto id : it->second)
          extrarewardids.push_back(id);
      else
        for (auto id : it->second)
          ids.push_back(id);
    }
  }
}

bool SGuildBuildingMaterialCFG::getRandItem(DWORD& itemid, DWORD& itemcount, EGuildBuilding type, QWORD charid, DWORD materialid) const
{
  if (vecItem.empty())
    return false;

  srand(xTime::getDayStart(now(), 5 * HOUR_T) * 1000 + type * 100 + charid * 10 + materialid);
  DWORD i = randBetween(0, vecItem.size() - 1);
  srand(xTime::getCurUSec());

  itemid = vecItem[i].dwItemID;
  itemcount = vecItem[i].dwItemCount;
  return true;
}

EGuildChallenge GuildConfig::getChallengeType(const string& type)
{
  if (type == "endless_tower")
    return EGUILDCHALLENGE_ENDLESS_TOWER;
  else if (type == "guild_raid")
    return EGUILDCHALLENGE_GUILD_RAID;
  else if (type == "guild_quest")
    return EGUILDCHALLENGE_GUILD_QUEST;
  else if (type == "gvg")
    return EGUILDCHALLENGE_GVG;
  else if (type == "seal")
    return EGUILDCHALLENGE_SEAL;
  else if (type == "wanted_quest")
    return EGUILDCHALLENGE_WANTED_QUEST;
  else if (type == "kill_mvp")
    return EGUILDCHALLENGE_KILL_MVP;
  else if (type == "kill_mini")
    return EGUILDCHALLENGE_KILL_MINI;
  else if (type == "login")
    return EGUILDCHALLENGE_LOGIN;
  return EGUILDCHALLENGE_MIN;
}

const SArtifactCFG* GuildConfig::getArtifactCFG(DWORD id) const
{
  auto it = m_mapID2ArtifactCFG.find(id);
  if (it == m_mapID2ArtifactCFG.end())
    return nullptr;
  return &it->second;
}

const SArtifactCFG* GuildConfig::getArtifactCFGByQuestID(DWORD id) const
{
  auto it = m_mapQuestID2ArtifactID.find(id);
  if (it == m_mapQuestID2ArtifactID.end())
    return nullptr;
  return getArtifactCFG(it->second);
}

const SGuildTreasureCFG* GuildConfig::getTreasureCFG(DWORD id) const
{
  auto m = m_mapTreasureCFG.find(id);
  if (m != m_mapTreasureCFG.end())
    return &m->second;
  return nullptr;
}

bool GuildConfig::loadGuildConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild.txt"))
  {
    XERR << "[公会配置-加载],加载配置Table_Guild.txt失败" << XEND;
    return false;
  }

  m_mapGuildCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild", table);
  TVecDWORD vecDojo;
  vecDojo.reserve(5);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto level = m_mapGuildCFG.find(m->first);
    if (level != m_mapGuildCFG.end())
    {
      XERR << "[公会配置-加载] level : " << m->first << " 重复了" << XEND;
      bCorrect = false;
      continue;
    }

    SGuildCFG stCFG;
    stCFG.dwLevel = m->first;
    stCFG.dwLevelupFund = m->second.getTableInt("ReviewFund");
    stCFG.dwAssetMaxPerDay = m->second.getTableInt("UpperLimit");
    stCFG.dwNeedFund = m->second.getTableInt("LevelupFund");
    stCFG.dwMaxMember = m->second.getTableInt("MemberNum");
    stCFG.dwViceCount = m->second.getTableInt("Management");
    stCFG.dwMaintenance = m->second.getTableInt("maintenanceCharge");
    stCFG.dwMaxPrayLv = m->second.getTableInt("BeliefUL");
    stCFG.dwDonateList = m->second.getTableInt("DonateListLimit");
    stCFG.dwQuestTime = m->second.getTableInt("GuildQuestNumber");
    stCFG.dwChallengeCount = m->second.getTableInt("ChallengeCount");

    xLuaData& item = m->second.getMutableData("DeductItem");
    ItemInfo oItem;
    oItem.set_id(item.getTableInt("1"));
    oItem.set_count(item.getTableInt("2"));
    combinItemInfo(stCFG.vecLevelupItem, TVecItemInfo{oItem});

    xLuaData& donate = m->second.getMutableData("donate_refresh");
    stCFG.dwDonateRefreshInterval1 = donate.getTableInt("1") * 60;
    stCFG.dwDonateRefreshInterval2 = donate.getTableInt("2") * 60;
    stCFG.dwDonateRefreshInterval3 = donate.getTableInt("3") * 60;
    stCFG.dwDonateRefreshInterval4 = donate.getTableInt("4") * 60;

    xLuaData& dojoGroup = m->second.getMutableData("DojoGroup");
    auto dojof = [&](const string& key, xLuaData& data)
    {
      DWORD dwDojoGroup = data.getInt();
      vecDojo.push_back(dwDojoGroup);     
    };
    dojoGroup.foreach(dojof);
    stCFG.vecDojoGroup = vecDojo;
    stCFG.setCommonDivisor();
    m_mapGuildCFG[stCFG.dwLevel] = stCFG;
  }

  if (bCorrect)
    XLOG << "[公会配置-加载] 加载配置Table_Guild.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildPray()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Faith.txt"))
  {
    XERR << "[公会祈祷配置-加载],加载配置Table_Guild_Faith.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapGuildPrayCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_Faith", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwType = m->second.getTableInt("Type");
    if (dwType >= EPRAYTYPE_MAX)
    {
      bCorrect = false;
      XERR << "[Table_Guild_Faith] id :" << m->first << "type :" << dwType << "不合法" << XEND;
      continue;
    }
    if (MiscConfig::getMe().isForbid("Guild_Faith", m->first))
      continue;

    SGuildPrayCFG& rCFG = m_mapGuildPrayCFG[m->first];
    rCFG.dwID = m->first;
    rCFG.eType = static_cast<EPrayType>(dwType);
    rCFG.blInit = true;
  }

  bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Pray.txt"))
  {
    XERR << "[Table_Guild_Pray] 加载失败" << XEND;
    return false;
  }

  table.clear();
  xLuaTable::getMe().getLuaTable("Table_Guild_Pray", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    bool bNoOpen = m->second.getTableInt("NoOpen") == 1;
    if (bNoOpen)
    {
      XERR << "[Table_Guild_Pray] id :" << m->first << "未开放" << XEND;
      continue;
    }

    auto item = m_mapGuildPrayCFG.find(m->first / 10000);
    if (item == m_mapGuildPrayCFG.end())
    {
      bCorrect = false;
      XERR << "[Table_Guild_Pray] id :" << m->first << "未在 Table_Guild_Faith.txt 表中找到对应祈祷" << XEND;
      continue;
    }

    SGuildPrayCFG& rCFG = item->second;
    SGuildPrayItemCFG& rItem = rCFG.mapLvItem[m->first - item->first * 10000];

    auto attrf = [&](const string& key, xLuaData& data)
    {
      DWORD dwAttr = RoleDataConfig::getMe().getIDByName(key.c_str());
      if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
      {
        bCorrect = false;
        XERR << "[Table_Guild_Pray] id :" << m->first << "attr : " << key.c_str() << "未在 Table_RoleData.txt 表中找到" << XEND;
        return;
      }

      UserAttrSvr oAttr;
      oAttr.set_type(static_cast<EAttrType>(dwAttr));
      oAttr.set_value(data.getFloat());
      rItem.vecAttrs.push_back(oAttr);
    };
    m->second.getMutableData("Attr").foreach(attrf);

    auto costf = [&](const string& key, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));

      if (ItemConfig::getMe().getItemCFG(oItem.id()) == nullptr)
      {
        bCorrect = false;
        XERR << "[Table_Guild_Pray] id :" << m->first << "cost :" << oItem.ShortDebugString() << "未在 Table_Item.txt 表中找到" << XEND;
        return;
      }

      combinItemInfo(rItem.vecCosts, oItem);
    };
    m->second.getMutableData("Cost").foreach(costf);
  }

  for (auto &m : m_mapGuildPrayCFG)
  {
    map<EAttrType, float> mapLastAttr;
    for (auto &lv : m.second.mapLvItem)
    {
      for (auto &v : lv.second.vecAttrs)
      {
        float& value = mapLastAttr[v.type()];
        v.set_value(v.value() + value);
        value = v.value();
        XDBG << "[Table_Guild_Pray] id :" << m.first << "lv :" << lv.first << "attr :" << v.ShortDebugString() << XEND;
      }
    }
  }

  if (bCorrect)
  {
    XLOG << "[Table_Guild_Faith] 成功加载" << XEND;
    XLOG << "[Table_Guild_Pray] 成功加载" << XEND;
  }
  return bCorrect;
}

bool GuildConfig::loadGuildFunc()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildFunction.txt"))
  {
    XERR << "[公会祈祷配置-加载],加载配置Table_GuildFunction.txt失败" << XEND;
    return false;
  }

  m_mapGuildFuncCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildFunction", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwUniqueID = m->second.getTableInt("UniqueID");
    if (getGuildFuncCFG(dwUniqueID) != nullptr)
    {
      XERR << "[公会功能配置-加载] uniqueid : " << dwUniqueID << " 重复了" << XEND;
      bCorrect = false;
      continue;
    }

    SGuildFuncCFG stCFG;
    stCFG.dwGuildLv = m->second.getTableInt("GuildLevel");
    stCFG.dwDisGuildLv = m->second.getTableInt("DisGuildLevel");
    stCFG.dwUniqueID = dwUniqueID;
    stCFG.strName = m->second.getTableString("Name");
    auto questidf = [&](const string& key, xLuaData& data)
    {
      stCFG.setQuestIDs.insert(data.getInt());
    };
    m->second.getMutableData("QuestID").foreach(questidf);

    DWORD showfunc = m->second.getTableInt("ShowFunc");
    if (showfunc && showfunc > EGUILDFUNCTION_MIN && showfunc < EGUILDFUNCTION_MAX && EGuildFunction_IsValid(showfunc))
      stCFG.eShowFunc = static_cast<EGuildFunction>(showfunc);
    DWORD disfunc = m->second.getTableInt("DisFunc");
    if (disfunc && disfunc > EGUILDFUNCTION_MIN && disfunc < EGUILDFUNCTION_MAX && EGuildFunction_IsValid(disfunc))
      stCFG.eDisFunc = static_cast<EGuildFunction>(disfunc);

    const string& starttime = m->second.getTableString("AppearTime");
    const string& end = m->second.getTableString("FinishTime");
    if (starttime.empty() == false)
      parseTime(starttime.c_str(), stCFG.dwStartTime);
    if (end.empty() == false)
      parseTime(end.c_str(), stCFG.dwEndTime);

    xLuaData& period = m->second.getMutableData("Period");
    auto periodf = [&](const string& key, xLuaData& data)
    {
      DWORD dwDay = data.getInt() - 1;
      if (dwDay > 6)
      {
        bCorrect = false;
        XERR << "[公会功能配置-加载] period : " << data.getInt() << " 不是一周的天数" << XEND;
        return;
      }
      stCFG.vecPeriod.push_back(data.getInt());
    };
    period.foreach(periodf);

    xLuaData& dayper = m->second.getMutableData("AppearTimeRange");
    auto dayperf = [&](const string& key, xLuaData& data)
    {
      const string& start = data.getTableString("1");
      const string& end = data.getTableString("2");
      DWORD dwStart = 0;
      DWORD dwEnd = 0;

      DWORD dwHour = 0;
      DWORD dwMin = 0;
      sscanf(start.c_str(), "%u:%u", &dwHour, &dwMin);
      dwStart = dwHour * 3600 + dwMin * 60;

      dwHour = 0;
      dwMin = 0;
      sscanf(end.c_str(), "%u:%u", &dwHour, &dwMin);
      dwEnd = dwHour * 3600 + dwMin * 60;

      stCFG.vecDayPeriod.push_back(pair<DWORD, DWORD>(dwStart, dwEnd));
    };
    dayper.foreach(dayperf);

    xLuaData& bp = m->second.getMutableData("BuildingParam");
    if (bp.has("type"))
    {
      DWORD btype = bp.getTableInt("type");
      if (btype > EGUILDBUILDING_MIN && btype < EGUILDBUILDING_MAX && EGuildBuilding_IsValid(btype))
      {
        stCFG.stBuildParam.eType = static_cast<EGuildBuilding>(btype);
        if (bp.has("show"))
        {
          xLuaData& showd = bp.getMutableData("show");
          stCFG.stBuildParam.dwShowLv = showd.getTableInt("lv");
          stCFG.stBuildParam.bShowWhenBuilding = showd.getTableInt("when_building") == 1;
          stCFG.stBuildParam.bShowAfterBuilding = showd.getTableInt("after_building") == 1;
        }
        if (bp.has("dis"))
        {
          xLuaData& disd = bp.getMutableData("dis");
          if (disd.has("lv"))
            stCFG.stBuildParam.dwDisLv = disd.getTableInt("lv");
          stCFG.stBuildParam.bDisWhenNotBuilding = disd.getTableInt("when_not_building") == 1;
        }
        stCFG.stBuildParam.bGearStatus = bp.getTableInt("gear_status") == 1;
        stCFG.stBuildParam.dwBuffWhenLvup = bp.getTableInt("buff_when_lvup");
        stCFG.stBuildParam.dwCreateDelay = bp.getTableInt("create_delay");
      }
    }

    m_mapGuildFuncCFG[stCFG.dwUniqueID] = stCFG;
  }

  if (bCorrect)
    XLOG << "[公会功能配置-加载], 成功加载配置 Table_GuildFunction.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildDonate()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Donate.txt"))
  {
    XERR << "[公会捐赠配置-加载],加载配置Table_Guild_Donate.txt失败" << XEND;
    return false;
  }

  m_mapDonateCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_Donate", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    auto item = m_mapDonateCFG.find(m->first);
    if (item != m_mapDonateCFG.end())
    {
      bCorrect = false;
      XERR << "[公会捐赠配置-加载] id : " << m->first << " 重复了" << XEND;
      continue;
    }

    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= EDONATETYPE_MIN || dwType >= EDONATETYPE_MAX)
    {
      bCorrect = false;
      XERR << "[公会捐赠配置-加载] id : " << m->first << " type : " << dwType << " 不合法" << XEND;
      continue;
    }

    SGuildDonateCFG stCFG;
    stCFG.dwWeight = m->second.getTableInt("Weight");
    stCFG.dwNextID = m->second.getTableInt("nextID");
    stCFG.dwIfNoActive = m->second.getTableInt("IfActive");
    stCFG.dwID = m->first;
    stCFG.eType = static_cast<EDonateType>(dwType);
    stCFG.lvrange.first = m->second.getMutableData("Level").getTableInt("1");
    stCFG.lvrange.second = m->second.getMutableData("Level").getTableInt("2");

    if (stCFG.lvrange.first > stCFG.lvrange.second)
    {
      bCorrect = false;
      XERR << "[公会捐赠配置-加载] id :" << m->first << "Level : min :" << stCFG.lvrange.first << "大于 max : " << stCFG.lvrange.second << XEND;
      continue;
    }

    xLuaData& detail = m->second.getMutableData("conDetail");
    auto detailf = [&](const string& str, xLuaData& data)
    {
      SGuildDonateItem stItem;
      stItem.dwItemID = data.getTableInt("itemid");
      stItem.dwCount = data.getTableInt("num");
      stCFG.vecItems.push_back(stItem);
    };
    detail.foreach(detailf);

    xLuaData& user = m->second.getMutableData("con");
    auto userf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_DONATE);
      stCFG.vecUserReward.push_back(oItem);
    };
    user.foreach(userf);

    xLuaData& asset = m->second.getMutableData("asset");
    auto assetf = [&](const string& str, xLuaData& data)
    {
      ItemInfo oItem;
      oItem.set_id(data.getTableInt("1"));
      oItem.set_count(data.getTableInt("2"));
      oItem.set_source(ESOURCE_DONATE);
      stCFG.vecGuildReward.push_back(oItem);
    };
    asset.foreach(assetf);

    m_mapDonateCFG[stCFG.dwID] = stCFG;
  }


  if (bCorrect)
    XLOG << "[公会功能配置-加载], 成功加载配置 Table_GuildFunction.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildQuestConfig()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Quest.txt"))
  {
    XERR << "[公会配置-任务加载],加载配置Table_Guild_Quest.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapQuestCFG)
  {
    for (auto &v : m.second)
      v.blInit = false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_Quest", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwQuestID = m->first;
    DWORD dwType = m->second.getTableInt("Type");

    TListGuildQuestCFG& listCFG = m_mapQuestCFG[dwType];
    auto l = find_if(listCFG.begin(), listCFG.end(), [dwQuestID](const SGuildQuestCFG& r) -> bool{
      return r.dwQuestID == dwQuestID;
    });
    if (l == listCFG.end())
    {
      SGuildQuestCFG stCFG;
      stCFG.dwQuestID = dwQuestID;
      listCFG.push_back(stCFG);
      l = find_if(listCFG.begin(), listCFG.end(), [dwQuestID](const SGuildQuestCFG& r) -> bool{
        return r.dwQuestID == dwQuestID;
      });
    }
    if (l == listCFG.end())
    {
      bCorrect = false;
      XERR << "[公会配置-任务加载] questid :" << dwQuestID << "未列表中发现" << XEND;
      continue;
    }

    SGuildQuestCFG& rCFG = *l;
    rCFG.dwType = dwType;
    rCFG.dwNpcID = m->second.getTableInt("NpcId");
    rCFG.dwGuildLv = m->second.getTableInt("GuildLv");
    rCFG.dwActiveMember = m->second.getTableInt("ActiveMember");
    rCFG.blInit = true;
  }

  if (bCorrect)
    XLOG << "[公会配置-任务加载], 成功加载配置 Table_Guild_Quest.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildPhotoFrame()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_ScenePhotoFrame.txt"))
  {
    XERR << "[公会相框配置],加载配置Table_ScenePhotoFrame.txt失败" << XEND;
    return false;
  }

  m_mapPhotoFrameCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_ScenePhotoFrame", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD dwDir = m->second.getTableInt("Dir");
    if (dwDir != EGUILDFRAMETYPE_HORIZONTAL && dwDir != EGUILDFRAMETYPE_VERTICAL)
    {
      XERR << "[公会相框配置-加载] id :" << m->first << "dir :" << dwDir << "不合法" << XEND;
      bCorrect = false;
      continue;
    }
    m_mapPhotoFrameCFG[static_cast<EGuildFrameType>(dwDir)].insert(m->first);
  }

  for (auto &m : m_mapPhotoFrameCFG)
  {
    XDBG << "[公会相框配置]" << m.first;
    for (auto &s : m.second)
      XDBG << s;
    XDBG << XEND;
  }

  if (bCorrect)
    XLOG << "[公会相框配置] 成功加载配置 Table_ScenePhotoFrame.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildJob()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildJob.txt"))
  {
    XERR << "[公会职位配置],加载配置Table_GuildJob.txt失败" << XEND;
    return false;
  }

  m_mapJobCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildJob", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    if (m->first <= EGUILDJOB_MIN || m->first >= EGUILDJOB_MAX)
    {
      XERR << "[公会职位配置],加载配置Table_GuildJob.txt失败" << XEND;
      continue;
    }

    EGuildJob eJob = static_cast<EGuildJob>(m->first);

    SGuildJobCFG& rJob = m_mapJobCFG[eJob];
    rJob.dwDefaultAuth = m->second.getTableInt("Authority");
    rJob.dwDefaultEditAuth = m->second.getTableInt("EditAuthority");
    rJob.dwReqLv = m->second.getTableInt("OpenLevel");
    rJob.eJob = eJob;
    rJob.name = m->second.getTableString("Name");
  }

  if (bCorrect)
    XLOG << "[公会职位配置] 成功加载配置 Table_GuildJob.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildBuilding()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildBuilding.txt"))
  {
    XERR << "[公会配置-建筑] 加载配置Table_GuildBuilding.txt失败" << XEND;
    return false;
  }

  m_mapBuildingCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildBuilding", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD type = m->second.getTableInt("Type");
    if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(type) == false)
    {
      XERR << "[公会配置-建筑] id:" << m->second.getTableInt("ID") << "type:" << type << "错误" << XEND;
      bCorrect = false;
      continue;
    }
    DWORD lv = m->second.getTableInt("Level");

    SGuildBuildingCFG& cfg = m_mapBuildingCFG[static_cast<EGuildBuilding>(type)][lv];
    cfg.eType = static_cast<EGuildBuilding>(type);
    cfg.dwLv = lv;
    cfg.strName = m->second.getTableString("Name");
    cfg.dwRewardID = m->second.getTableInt("Reward");
    cfg.dwRewardCycle = m->second.getTableInt("RewardCycle");
    cfg.dwBuildTime = m->second.getTableInt("BuildTime");

    cfg.dwNpcID = m->second.getTableInt("NpcID");

    auto materialf = [&](const string& key, xLuaData& data)
    {
      DWORD id = atoi(key.c_str());
      DWORD count = data.getInt();
      if (id && count)
      {
        auto it = cfg.mapMaterial.find(id);
        if (it == cfg.mapMaterial.end())
          cfg.mapMaterial[id] = 0;
        cfg.mapMaterial[id] += count;
      }
    };
    m->second.getMutableData("Material").foreach(materialf);

    if (m->second.getMutableData("LevelUpCond").has("buildingtype"))
    {
      DWORD lvuptype = m->second.getMutableData("LevelUpCond").getTableInt("buildingtype");
      if (lvuptype <= EGUILDBUILDING_MIN || lvuptype >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(lvuptype) == false)
      {
        XERR << "[公会配置-建筑] id:" << m->second.getTableInt("ID") << "LevelUpCond.buildingtype:" << lvuptype << "错误" << XEND;
        bCorrect = false;
      }
      cfg.eLvupBuildingType = static_cast<EGuildBuilding>(lvuptype);
    }
    cfg.dwLvupBuildingLv = m->second.getMutableData("LevelUpCond").getTableInt("buildinglv");

    // equip
    xLuaData& equip = m->second.getMutableData("UnlockParam").getMutableData("equip");

    DWORD dwType = 0;
    auto typef = [&](const string& key, xLuaData& data)
    {
      DWORD dwEquipType = data.getInt();
      if (dwEquipType <= EEQUIPTYPE_MIN || dwEquipType >= EEQUIPTYPE_MAX || EEquipType_IsValid(dwEquipType) == false)
      {
        bCorrect = false;
        XERR << "[公会配置-建筑] id :" << m->second.getTableInt("ID") << "refine_type :" << dwEquipType << "不合法" << XEND;
        return;
      }
      if (dwType == 1)
        cfg.setRefineType.insert(static_cast<EEquipType>(dwEquipType));
      else if (dwType == 2)
        cfg.setStrengthType.insert(static_cast<EEquipType>(dwEquipType));
    };

    dwType = 1;
    equip.getMutableData("refine_type").foreach(typef);
    cfg.dwMaxRefineLv = equip.getTableInt("refinemaxlv");

    dwType = 2;
    equip.getMutableData("strength_type").foreach(typef);
    cfg.dwMaxStrengthLv = equip.getTableInt("strengthmaxlv");

    cfg.oUnlockParam = m->second.getData("UnlockParam");
    DWORD pos = 0;

    auto unlockF = [&](const string& key, xLuaData& data)
    {
      if (key == "hrefine_part")
      {
        auto posF = [&](const string& key, xLuaData& data)
        {
          auto F = [&](const string& key, xLuaData& data)
          {
            DWORD t = data.getInt();
            cfg.mapHRefinePos[pos].insert(t);
            XDBG << "loadGuildBuilding" << pos << "t" <<t << XEND;
          };
          pos = atoi(key.c_str());
          data.foreach(F);
        };
        data.foreach(posF);
      }
    };
    cfg.oUnlockParam.foreach(unlockF);

    if (cfg.oUnlockParam.has("shopid"))
    {
      auto shopf = [&](const string& key, xLuaData& data)
      {
        cfg.setShopID.insert(data.getInt());
      };
      cfg.oUnlockParam.getMutableData("shopid").foreach(shopf);
    }
    if (cfg.oUnlockParam.has("shoptype"))
    {
      cfg.dwShopType = cfg.oUnlockParam.getTableInt("shoptype");
      m_mapShopType2Building[cfg.dwShopType] = cfg.eType;
    }

    if (cfg.oUnlockParam.has("ownlimit"))
      cfg.dwArtifactMaxCount = cfg.oUnlockParam.getTableInt("ownlimit");
    if (cfg.oUnlockParam.has("singlelimit"))
      cfg.dwArtifactTypeMaxCount = cfg.oUnlockParam.getTableInt("singlelimit");

    cfg.dwStrengthLvAdd = cfg.oUnlockParam.getTableInt("strengthmaxlv_add");
  }

  for (auto& v : m_mapBuildingCFG)
  {
    if (v.second.empty() == false)
      v.second.rbegin()->second.bIsMaxLv = true;
  }

  if (bCorrect)
    XLOG << "[公会配置-建筑] 成功加载配置Table_GuildBuilding.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildBuildingMaterial()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildBuildingMaterial.txt"))
  {
    XERR << "[公会配置-建筑材料] 加载配置Table_GuildBuildingMaterial.txt失败" << XEND;
    return false;
  }

  m_mapBuildingMaterialCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildBuildingMaterial", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SGuildBuildingMaterialCFG& cfg = m_mapBuildingMaterialCFG[m->second.getTableInt("id")];

    cfg.dwID = m->second.getTableInt("id");
    cfg.dwRewardID = m->second.getTableInt("Reward");
    cfg.dwFund = m->second.getTableInt("Fund");

    auto itemf = [&](const string& key, xLuaData& data)
    {
      DWORD itemid = data.getTableInt("1");
      if (ItemConfig::getMe().getItemCFG(itemid) == nullptr)
      {
        XERR << "[公会配置-建筑材料] 材料:" << cfg.dwID << "道具:" << itemid << "在item表中找不到" << XEND;
        return;
      }
      SGuildMaterialItemCFG item;
      item.dwItemID = itemid;
      item.dwItemCount = data.getTableInt("2");
      item.dwWeight = data.getTableInt("3");
      cfg.vecItem.push_back(item);
    };
    m->second.getMutableData("Item").foreach(itemf);
  }

  if (bCorrect)
    XLOG << "[公会配置-建筑材料] 成功加载配置Table_GuildBuildingMaterial.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildChallenge()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_GuildChallenge.txt"))
  {
    XERR << "[公会配置-挑战] 加载配置Table_GuildChallenge.txt失败" << XEND;
    return false;
  }

  m_mapChallengeCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_GuildChallenge", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SGuildChallengeCFG cfg;

    cfg.dwID = m->second.getTableInt("id");
    cfg.dwGroupID = m->second.getTableInt("GroupID");
    cfg.eType = getChallengeType(m->second.getTableString("Type"));
    cfg.oParam = m->second.getData("Param");
    cfg.dwSubProgress = cfg.oParam.has("progress") ? cfg.oParam.getTableInt("progress") : 1;
    cfg.dwTotalProgress = m->second.getTableInt("Target");
    cfg.dwReward = m->second.getTableInt("Reward");
    cfg.dwGuildLevel = m->second.getTableInt("GuildLevel");
    cfg.dwWeight = m->second.getTableInt("Weight");
    cfg.strGroupName = m->second.getTableString("GroupName");

    if (cfg.eType <= EGUILDCHALLENGE_MIN || cfg.eType >= EGUILDCHALLENGE_MAX)
    {
      XERR << "[公会配置-挑战] id:" << cfg.dwID << "type配置错误" << XEND;
      bCorrect = false;
      continue;
    }

    m_mapChallengeCFG[cfg.dwID] = cfg;
    m_mapType2Challenge[cfg.eType].push_back(cfg);
    m_mapGroup2ChallengeID[cfg.dwGroupID].push_back(cfg.dwID);
  }

  if (bCorrect)
    XLOG << "[公会配置-挑战] 成功加载配置Table_GuildChallenge.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadArtifact()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Artifact.txt"))
  {
    XERR << "[公会配置-神器] 加载配置Table_Artifact.txt失败" << XEND;
    return false;
  }

  m_mapID2ArtifactCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Artifact", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SArtifactCFG cfg;

    cfg.dwID = m->second.getTableInt("id");
    cfg.strName = m->second.getTableString("Name");
    cfg.dwType = m->second.getTableInt("Type");
    cfg.dwLevel = m->second.getTableInt("Level");
    cfg.dwDistributeCount = m->second.getTableInt("DistributeCount");
    cfg.dwUnlockMsg = m->second.getTableInt("UnlockMsg");
    cfg.dwNpcUniqueID = m->second.getTableInt("NpcUniqueID");
    cfg.dwNextLevelID = m->second.getTableInt("NextLevelID");
    cfg.dwQuestID = m->second.getTableInt("QuestID");

    DWORD btype = m->second.getTableInt("BuildingType");
    if (btype <= EGUILDBUILDING_MIN || btype >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(btype) == false)
    {
      XERR << "[公会配置-神器]" << "id:" << cfg.dwID << "buildingtype:" << btype << "建筑类型非法" << XEND;
      continue;
    }
    cfg.eBuildingType = static_cast<EGuildBuilding>(btype);

    auto materialf = [&](const string& key, xLuaData& data)
    {
      DWORD type = atoi(key.c_str());
      auto f = [&](const string& key, xLuaData& data)
      {
        DWORD itemid = data.getTableInt("id");
        DWORD itemcount = data.getTableInt("num");
        if (itemid && itemcount)
        {
          if (cfg.mapMaterial[type].find(itemid) == cfg.mapMaterial[type].end())
            cfg.mapMaterial[type][itemid] = itemcount;
          else
            cfg.mapMaterial[type][itemid] += itemcount;
        }
      };
      data.foreach(f);
    };
    m->second.getMutableData("Material").foreach(materialf);

    m_mapID2ArtifactCFG[cfg.dwID] = cfg;
    m_mapQuestID2ArtifactID[cfg.dwQuestID] = cfg.dwID;
  }

  if (bCorrect)
    XLOG << "[公会配置-神器] 成功加载配置Table_Artifact.txt" << XEND;
  return bCorrect;
}

bool GuildConfig::loadGuildTreasure()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Guild_Treasure.txt"))
  {
    XERR << "[Table_Guild_Treasure] 加载配置Table_Guild_Treasure.txt失败" << XEND;
    return false;
  }

  for (auto &m : m_mapTreasureCFG)
    m.second.blInit = false;

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Guild_Treasure", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SGuildTreasureCFG& rCFG = m_mapTreasureCFG[m->first];
    rCFG.dwID = m->first;
    rCFG.dwCityID = m->second.getTableInt("CityID");
    rCFG.dwOrderID = m->second.getTableInt("Order");

    DWORD dwType = m->second.getTableInt("Type");
    if (dwType <= EGUILDTREASURETYPE_MIN || dwType >= EGUILDTREASURETYPE_MAX)
    {
      bCorrect = false;
      XERR << "[Table_Guild_Treasure] id :" << m->first << "Type :" << dwType << "不合法" << XEND;
      continue;
    }
    if (dwType == EGUILDTREASURETYPE_PREVIEW)
      continue;
    rCFG.eType = static_cast<EGuildTreasureType>(dwType);
    rCFG.strName = m->second.getTableString("Name");

    auto grewardf = [&](const string& key, xLuaData& data)
    {
      rCFG.setGuildReward.insert(data.getInt());
    };
    m->second.getMutableData("GuildReward").foreach(grewardf);

    auto mrewardf = [&](const string& key, xLuaData& data)
    {
      TSetDWORD& setIDs = rCFG.mapMemberReward[atoi(key.c_str())];

      auto listf = [&](const string& k, xLuaData& d)
      {
        setIDs.insert(d.getInt());
      };
      data.foreach(listf);
    };
    m->second.getMutableData("GuildMemberReward").foreach(mrewardf);

    rCFG.blInit = true;
  }

  m_vecGuildTreasureCFG.clear();
  for (auto &m : m_mapTreasureCFG)
  {
    if (m.second.blInit && (m.second.eType == EGUILDTREASURETYPE_GUILD_BCOIN || m.second.eType == EGUILDTREASURETYPE_GUILD_ASSET))
      m_vecGuildTreasureCFG.push_back(m.second);
  }

  XLOG << "[Table_Guild_Treasure] 加载配置Table_Guild_Treasure.txt失败" << XEND;
  return bCorrect;
}


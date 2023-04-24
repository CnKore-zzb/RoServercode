#include "MiscConfig.h"
#include "xLuaTable.h"
#include "ConfigManager.h"
#include "xTime.h"
#include "BaseConfig.h"
#include "SysmsgConfig.h"

// guild
const string& SGuildMiscCFG::getAuthName(EAuth eAuth) const
{
  if (eAuth <= EAUTH_MIN || eAuth >= EAUTH_MAX)
    return STRING_EMPTY;
  auto m = mapAuthName.find(eAuth);
  if (m != mapAuthName.end())
    return m->second;
  return STRING_EMPTY;
}

DWORD SGuildMiscCFG::getAssetPrice(DWORD dwTime) const
{
  auto m = mapTreasureAsset.find(dwTime);
  if (m != mapTreasureAsset.end())
    return m->second;
  return DWORD_MAX;
}

DWORD SGuildMiscCFG::getBCoinPrice(DWORD dwTime) const
{
  auto m = mapTreasureBCoin.find(dwTime);
  if (m != mapTreasureBCoin.end())
    return m->second;
  return DWORD_MAX;
}

// scene item config
float SSceneItemCFG::getRange(DWORD dwCount) const
{
  for (auto &v : vecRange)
  {
    if (dwCount >= v.dwMin && dwCount <= v.dwMax)
      return v.fRange;
  }

  return 1.0f;
}

// package
DWORD SPackageCFG::getIndex(EItemType type) const
{
  DWORD index = 0;
  for (auto v = vecSortType.begin(); v != vecSortType.end(); ++v)
  {
    if (*v == type)
      break;
    ++index;
  }

  return index;
}

const TSetDWORD& SPackageCFG::getPackFunc(EPackFunc eFunc) const
{
  auto m = mapPackFunc.find(eFunc);
  if (m != mapPackFunc.end())
    return m->second;

  return setPackFuncDefault;
}

// quest
float SQuestMiscCFG::getWantedParams(DWORD count) const
{
  if (count >= vecWantedParams.size())
    return 1.0f;

  return vecWantedParams[count];
}

DWORD SQuestMiscCFG::getMaxWantedCount(EWantedType eType) const
{
  if (eType >= EWANTEDTYPE_MAX || EWantedType_IsValid(eType) == false)
    return 0;
  if (eType == EWANTEDTYPE_TOTAL)
    return QuestConfig::getMe().getActiveMaxWantedCount() == 0 ? arrMaxWanted[eType] : QuestConfig::getMe().getActiveMaxWantedCount();
  return arrMaxWanted[eType];
}

const SDailyPerDay* SQuestMiscCFG::getDailyCount(DWORD dwMapID) const
{
  auto m = mapDailyPerDay.find(dwMapID);
  if (m != mapDailyPerDay.end())
    return &m->second;
  return nullptr;
}

const string& SQuestMiscCFG::getPrefixion(DWORD dwIndex) const
{
  auto m = mapPrefixion.find(dwIndex);
  if (m != mapPrefixion.end())
    return m->second;
  return STRING_EMPTY;
}

// infinite tower config
const xPos& SEndlessTowerCFG::getPos(DWORD dwFloor) const
{
  static const xPos ePos;
  for (auto &v : vecBosPos)
  {
    if (dwFloor >= v.pairFloor.first && dwFloor <= v.pairFloor.second)
      return v.oPos;
  }

  return ePos;
}

// system
ESysMsgID SSystemCFG::checkNameValid(const string& name, ENameType eType) const
{
  if (eType <= ENAMETYPE_MIN || eType >= ENAMETYPE_MAX)
    return ESYSTEMMSG_ID_GUILD_NAMEEMPTY;

  auto check_forbid = [&](bool bSymbol, bool bWord, bool bName) -> ESysMsgID
  {
    if (bSymbol)
    {
      for (auto &v : vecForbidSymbol)
      {
        if (name.find(v) != string::npos)
          return ESYSTEMMSG_ID_GUILD_NAMEINVALID;
      }
    }
    if (bWord)
    {
      for (auto &v : vecForbidWord)
      {
        if (name.find(v) != string::npos)
          return ESYSTEMMSG_ID_GUILD_NAMEINVALID;
      }
    }
    if (bName)
    {
      for (auto &v : vecForbidName)
      {
        if (name.find(v) != string::npos)
          return ESYSTEMMSG_ID_GUILD_NAMEINVALID;
      }
    }
    return ESYSTEMMSG_ID_MIN;
  };

  DWORD dwCount = getWordCount(name);
  ESysMsgID eMsg = ESYSTEMMSG_ID_MIN;
  switch(eType)
  {
    case ENAMETYPE_MIN:
      break;
    case ENAMETYPE_USER:
      if (dwCount < dwNameMinSize || dwCount > dwNameMaxSize)
        eMsg = ESYSTEMMSG_ID_NAMEEMPTY;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_GUILD:
      if (dwCount < dwGuildNameMinSize || dwCount > dwGuildNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_GUILD_JOB:
      if (dwCount < dwGuildJobMinSize || dwCount > dwGuildJobMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_GUILD_RECRUIT:
      if (dwCount > dwGuildRecruitMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_GUILD_BOARD:
      if (dwCount > dwGuildBoardMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_TEAM:
      if (dwCount < dwTeamNameMinSize || dwCount > dwTeamNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, false, false);
      break;
    case ENAMETYPE_ROOM:
      if (dwCount < dwRoomNameMinSize || dwCount > dwRoomNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(false, true, true);
      break;
    case ENAMETYPE_PET:
      if (dwCount < dwPetNameMinSize || dwCount > dwPetNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_RECORD:
      if (dwCount < dwRecordNameMinSize || dwCount > dwRecordNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(true, true, true);
      break;
    case ENAMETYPE_BOOTH:
      if (dwCount < dwBoothNameMinSize || dwCount > dwBoothNameMaxSize)
        eMsg = ESYSTEMMSG_ID_GUILD_NAMELEN;
      else
        eMsg = check_forbid(false, true, true);
      break;
    case ENAMETYPE_MAX:
      break;
  }

  return eMsg;
}

bool SSystemCFG::isNight(DWORD curTime) const
{
  EGameTimeType etype = getTimeType(curTime);
  return etype == EGAMETIMETYPE_NIGHT;
}

EGameTimeType SSystemCFG::getTimeType(DWORD curTime) const
{
  if (dwGameDaySec == 0)
    return EGAMETIMETYPE_MIN;

  SDWORD swCurTime = curTime;
  SDWORD swDaySec = swCurTime - static_cast<SDWORD>(xTime::getDayStart(swCurTime));
  int hour = ((swDaySec % dwGameDaySec) * dwTimeSpeed) / HOUR_T;
  if (hour >= 4 && hour < 6)          //[4, 6)
    return EGAMETIMETYPE_DAWN;
  if (hour >= 6 && hour < 18)
    return EGAMETIMETYPE_DAYTIME;
  if (hour >= 18 && hour < 20)
    return EGAMETIMETYPE_DUSK;
  if (hour >= 20 || hour < 4)
    return EGAMETIMETYPE_NIGHT;

  return EGAMETIMETYPE_MIN;
}

DWORD SSystemCFG::getSkyTypeIndex(DWORD curTime) const
{
  if (dwGameDaySec == 0)
    return 0;

  SDWORD swCurTime = curTime;
  SDWORD swDaySec = swCurTime - static_cast<SDWORD>(xTime::getDayStart(swCurTime));
  int hour = ((swDaySec % dwGameDaySec) * dwTimeSpeed) / HOUR_T;
  if (hour >= 4 && hour < 6)          //[4, 6)
    return 0;
  if (hour >= 6 && hour < 18)
    return 1;
  if (hour >= 18 && hour < 20)
    return 2;
  if (hour >= 20 || hour < 4)
    return 3;
  return 0;
}

// trap npc
bool STrapNpcCFG::isTrapNpc(DWORD dwID) const
{
  auto v = find_if(vecTrapNpcIDs.begin(), vecTrapNpcIDs.end(), [dwID](const DWORD& d){
      return d == dwID;
      });
  return v != vecTrapNpcIDs.end();
}

// dojo
DWORD SGuildDojoCFG::getBaseLvReq(DWORD dwGroupID) const
{
  auto m = mapBaseLvReq.find(dwGroupID);
  if (m == mapBaseLvReq.end())
    return DWORD_MAX;
  return m->second;
}

// monster
DWORD SMonsterMiscCFG::getDisappearTime(DWORD dwNpcType) const
{
  if (dwNpcType == ENPCTYPE_MONSTER)
    return dwMonsterDisappearTime;
  else if (dwNpcType == ENPCTYPE_MINIBOSS)
    return dwMiniDisappearTime;
  else if (dwNpcType == ENPCTYPE_MVP)
    return dwMvpDisappearTime;
  else if (dwNpcType == ENPCTYPE_NPC)
    return dwNpcDisappearTime;

  return 0;
}

// handnpc
DWORD SHandNpcCFG::getRandomOne(const SHandShowData& stData) const
{
  if (stData.vecValue.size() == 0)
    return 0;
  if ((DWORD)randBetween(0, 100) > stData.dwOdds)
    return 0;
  DWORD index = randBetween(0, stData.vecValue.size() - 1);
  return stData.vecValue[index];
}

// zone
EZoneStatus SZoneMiscCFG::getZoneStatus(DWORD dwRate) const
{
  for (auto &v : vecItems)
  {
    if (dwRate >= v.status.first && dwRate <= v.status.second)
      return v.eStatus;
  }

  return EZONESTATUS_MIN;
}

EZoneState SZoneMiscCFG::getZoneState(DWORD dwRate) const
{
  return dwRate >= dwZoneStateRate ? EZONESTATE_FULL : EZONESTATE_NOFULL;
}

const TVecItemInfo& SZoneMiscCFG::getJumpCost(EZoneStatus eStatus) const
{
  const static TVecItemInfo e;
  for (auto &v : vecItems)
  {
    if (v.eStatus == eStatus)
      return v.vecCost;
  }

  return e;
}

// seal
DWORD SealMiscCFG::getSealLv(DWORD dwMapID) const
{
  auto m = mapMapLv.find(dwMapID);
  return m == mapMapLv.end() ? 65 : m->second;
}

// Item
DWORD SItemMiscCFG::getPackMaxSlot(EPackType eType) const
{
  if (eType == EPACKTYPE_MAIN)
    return dwMainMaxSlot;
  else if (eType == EPACKTYPE_TEMP_MAIN)
    return dwTempMainMaxSlot;
  else if (eType == EPACKTYPE_PERSONAL_STORE)
    return dwPersonalStoreMaxSlot;
  else if (eType == EPACKTYPE_STORE)
    return dwStoreMaxSlot;
  else if (eType == EPACKTYPE_BARROW)
    return dwBarrowMaxSlot;
  else if (eType == EPACKTYPE_QUEST)
    return dwQuestMaxSlot;
  else if (eType == EPACKTYPE_FOOD)
    return dwFoodMaxSlot;
  else if (eType == EPACKTYPE_PET)
    return dwPetMaxSlot;

  return DWORD_MAX;
}

const TVecSkillSlot& SItemMiscCFG::getPackSkillSlot(EPackType eType) const
{
  const static TVecSkillSlot e;
  auto m = mapPackSkillSlot.find(eType);
  if (m != mapPackSkillSlot.end())
    return m->second;
  return e;
}

DWORD SItemMiscCFG::getCardRestoreCost(EQualityType eQuality) const
{
  auto m = mapRestoreCardCost.find(eQuality);
  if (m != mapRestoreCardCost.end())
    return m->second;
  return 0;
}

DWORD SItemMiscCFG::getEnchantRestoreCost(EEnchantType eType) const
{
  auto m = mapRestoreEnchantCost.find(eType);
  if (m != mapRestoreEnchantCost.end())
    return m->second;
  return 0;
}

DWORD SItemMiscCFG::getUpgradeRestoreCost(DWORD dwLv) const
{
  auto m = mapRestoreUpgradeCost.find(dwLv);
  if (m != mapRestoreUpgradeCost.end())
    return m->second;
  return 0;
}

DWORD SItemMiscCFG::getDecomposePrice(DWORD dwID) const
{
  auto m = mapDecomposePrice.find(dwID);
  if (m != mapDecomposePrice.end())
    return m->second;
  return 0;
}

EEquipPos SItemMiscCFG::getValidEquipPos(EEquipPos ePos, EEquipType eType) const
{
  auto m = mapEquipType2Pos.find(eType);
  if (m == mapEquipType2Pos.end())
    return EEQUIPPOS_MIN;

  if (m->first == EEQUIPTYPE_ACCESSORY)
    return (ePos == EEQUIPPOS_ACCESSORY1 || ePos == EEQUIPPOS_ACCESSORY2) ? ePos : EEQUIPPOS_ACCESSORY1;
  return *m->second.begin();
}

bool SItemMiscCFG::isSpecEnchantItem(EEnchantType eEnchantType, EItemType eType) const
{
  auto it = mapSpecEnchantCost.find(eEnchantType);
  if (it != mapSpecEnchantCost.end())
  {
    auto m = it->second.find(eType);
    if (m != it->second.end())
      return true;
  }
  return false;
}

const TVecItemInfo& SItemMiscCFG::getSpecEnchantItem(EEnchantType eEnchantType, EItemType eType) const
{
  auto it = mapSpecEnchantCost.find(eEnchantType);
  if (it != mapSpecEnchantCost.end())
  {
    auto m = it->second.find(eType);
    if (m != it->second.end())
      return m->second;
  }
  static const TVecItemInfo emptyitem;
  return emptyitem;
}

DWORD SSkillMiscCFG::getActionBySkill(DWORD dwID) const
{
  if (pairSkillAction.first == dwID)
    return pairSkillAction.second;
  return 0;
}

// pet
bool SPetMiscCFG::isValidPartner(EProfession eProfession, DWORD dwNpcID) const
{
  if (eProfession >= EPROFESSION_HUNTER && eProfession <= EPROFESSION_RANGER)
    return setArcherPartnerID.find(dwNpcID) != setArcherPartnerID.end();
  else if ((eProfession >= EPROFESSION_MERCHANT && eProfession <= EPROFESSION_MECHANIC) || (eProfession >= EPROFESSION_ALCHEMIST && eProfession <= EPROFESSION_GENETIC))
    return setMerchantPartnerID.find(dwNpcID) != setMerchantPartnerID.end();

  return false;
}

DWORD SPetMiscCFG::getNpcBarrow(DWORD dwEquipID) const
{
  auto m = mapBarrowEquipNpc.find(dwEquipID);
  if (m != mapBarrowEquipNpc.end())
    return m->second;
  return 0;
}

DWORD SPetMiscCFG::randSkill(DWORD dwItemID) const
{
  auto m = mapID2SkillReset.find(dwItemID);
  if (m == mapID2SkillReset.end() || mapID2SkillReset.empty() == true)
    return 0;

  DWORD dwRand = randBetween(0, m->second.rbegin()->second);
  for (auto &item : m->second)
  {
    if (dwRand <= item.second)
    {
      XDBG << "[ServerGame] skillrand :" << dwRand << "weight :" << item.second << XEND;
      return item.first;
    }
  }
  return 0;
}

DWORD SGuildFireCFG::getNextStartTime(DWORD cur) const
{
  if (setWeekStartTime.empty())
    return false;
  DWORD weekstart = xTime::getWeekStart(cur);
  for (auto &s : setWeekStartTime)
  {
    if (s + weekstart + dwLastTime > cur)
      return s + weekstart;
  }

  return (*(setWeekStartTime.begin()) + weekstart + 86400 * 7); // 下周开启
}

void SAutoSkillGroupCFG::getHigherSkill(DWORD skillid, TSetDWORD& skills) const
{
  skillid = skillid / ONE_THOUSAND;
  for (auto &v : vecAutoSkillGroup)
  {
    bool find = false;
    for (auto &v1 : v)
    {
      if (find)
        skills.insert(v1);// 获取同一组的后续技能
      if (skillid == v1)
        find = true;
    }
    find = false;
  }
}

//index  从0开始
DWORD SPoliFireCFG::getBuffId(DWORD index) const
{
  auto it = mapTransBuffId.find(index);
  if (it == mapTransBuffId.end())
    return 0;
  return it->second;
}

DWORD SPoliFireCFG::getRewardId(DWORD lv) const
{
  for (auto it = mapLv2ReardId.begin(); it != mapLv2ReardId.end(); ++it)
  {
    if (lv <= it->first)
    {
      return it->second;
    }
  }
  return 0;
}

const TSetDWORD& SNewRoleCFG::getClassInitBuff(DWORD professionid) const
{
  auto it = mapClassInitBuff.find(professionid);
  if (it == mapClassInitBuff.end())
  {
    static const TSetDWORD emptyset;
    return emptyset;
  }

  return it->second;
}

bool SLotteryMiscCFG::checkLinkage(DWORD type, DWORD year, DWORD month) const
{
  for(auto& v : m_vecLinkage)
  {
    if(type == v.dwType && year == v.dwYear && month == v.dwMonth)
      return v.dwEndTime >= now();
  }
  return true;
}

DWORD SLotteryMiscCFG::randomItemType(bool isTicket) const
{
  auto f = [&](const std::map<DWORD/*itemtype*/, DWORD/*weight*/> &mapWeight,const DWORD &dwTotalWeight)->DWORD {
    DWORD r = randBetween(1, dwTotalWeight);
    DWORD offset = 0;
    for (auto&m : mapWeight)
    {
      if (m.second == 0)
        continue;
      if (r <= m.second + offset)
      {
        return m.first;
      }
      offset += m.second;
    }
    return 0;
  };

  if (isTicket)
    return f(m_mapTicketWeight, m_dwTotalTicketWeight);
  else
    return f(m_mapCoinWeight, m_dwTotalCoinWeight);
}

const TMapGvgTimes2RewardData& SGuildFireCFG::getRewardInfo(EGvgDataType eType) const
{
  switch(eType)
  {
    case EGVGDATA_MIN:
      break;
    case EGVGDATA_PARTINTIME:
      return mapPartinTimeReward;
    case EGVGDATA_KILLMON:
      return mapKillMonsterReward;
    case EGVGDATA_RELIVE:
      return mapReliveUserReward;
    case EGVGDATA_EXPEL:
      return mapExpelReward;
    case EGVGDATA_DAMMETAL:
      return mapDamMetalReward;
    case EGVGDATA_KILLMETAL:
      return mapKillMetalReward;
    case EGVGDATA_KILLUSER:
      break;
    case EGVGDATA_HONOR:
      return mapGetHonorReward;
  }

  static const TMapGvgTimes2RewardData emptydata;
  return emptydata;
}

bool SMvpBattleCFG::checkMatchOk(DWORD time, DWORD teamnum) const
{
  for (auto &v : vecTeamNum2MatchInfo)
  {
    if (teamnum >= v.dwMinTeamNum && (v.dwMinMatchTime <= time && v.dwMaxMatchTime >= time))
      return true;
  }
  return false;
}

bool SMvpBattleCFG::getBossAndMiniNum(DWORD teamnum, pair<DWORD, DWORD>& bossAndMini) const
{
  for (auto &v : vecTeamNum2MatchInfo)
  {
    if (teamnum >= v.dwMinTeamNum)
    {
      bossAndMini.first = v.dwMvpNum;
      bossAndMini.second = v.dwMiniNum;
      return true;
    }
  }
  return false;
}

const SMvpTeamNumMatchCFG* SMvpBattleCFG::getMatchRoomInfo(DWORD teamnum) const
{
  for (auto &v : vecTeamNum2MatchInfo)
  {
    if (teamnum >= v.dwMinTeamNum)
      return &v;
  }
  return nullptr;
}

bool SGuildFireCFG::hasSuperGvg(DWORD starttime) const/*判断某次开启后是否有supergvg*/
{
  DWORD weekday = xTime::getWeek(starttime);
  if (weekday == 0)
    weekday = 7;
  return stSuperGvgCFG.setSuperGvgDays.find(weekday) != stSuperGvgCFG.setSuperGvgDays.end();
}

DWORD SGvgTowerCFG::getSpeed(DWORD guildnum, DWORD usernum) const
{
  float per1 = 0;
  for (auto &v : vecGuildNum2Spd)
  {
    if (guildnum >= v.first)
    {
      per1 = v.second;
      break;
    }
  }
  float per2 = 0;
  for (auto &v : vecUserNum2Spd)
  {
    if (usernum >= v.first)
    {
      per2 = v.second;
      break;
    }
  }
  return (dwBaseValue * per1 * per2);
}

float SSuperGvgCFG::getRewardPer(DWORD rank, DWORD level, bool userOrGuild) const
{
  for (auto &v : vecRewardPerCFG)
  {
    if (v.dwLevel == level)
    {
      auto it = v.mapRank2RewardPer.find(rank);
      if (it != v.mapRank2RewardPer.end())
        return userOrGuild ? it->second.second : it->second.first;
    }
  }
  return 0;
}

DWORD SSuperGvgCFG::getLevelByScore(float aveScore) const
{
  if (aveScore == 0) // 第一次匹配默认最低等级
    return 1;
  for (auto &v : vecRewardPerCFG)
  {
    if (aveScore >= v.fMinScore && aveScore < v.fMaxScore)
      return v.dwLevel;
  }
  return 1;
}

// misc config
MiscConfig::MiscConfig()
{

}

MiscConfig::~MiscConfig()
{

}

bool MiscConfig::loadConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/GameConfig.txt"))
  {
    XERR << "[GameConfig], 加载配置GameConfig.txt失败" << XEND;
    return false;
  }

  xLuaData table;
  xLuaTable::getMe().getLuaData("GameConfig", table);
  for (auto m = table.m_table.begin(); m != table.m_table.end(); ++m)
  {
    if (m->first == "UserRelive")
    {
      m_stReliveCFG.dwReliveHpPercent = m->second.getTableInt("hppercent");

      m_stReliveCFG.vecConsume.clear();
      xLuaData& data = m->second.getMutableData("deathcost2");
      auto dataf = [this](const string& key, xLuaData& data)
      {
        ItemInfo stItem;
        stItem.set_id(data.getTableInt("id"));
        stItem.set_count(data.getTableInt("num"));

        m_stReliveCFG.vecConsume.push_back(stItem);
      };
      data.foreach(dataf);

      m_stReliveCFG.vecBuff.clear();
      auto funbuff = [this](const string& key, xLuaData& data)
      {
        m_stReliveCFG.vecBuff.push_back(data.getInt());
      };
      xLuaData& databuff = m->second.getMutableData("buffID");
      databuff.foreach(funbuff);
    }
    else if (m->first == "SceneDropItem")
    {
      m_stSceneItemCFG.qwPickupInterval = m->second.getTableQWORD("pickUpInterval");
      m_stSceneItemCFG.dwDisappearTime = m->second.getTableInt("disappeartime");
      m_stSceneItemCFG.dwOwnerTime = m->second.getTableInt("privateOwnTime") / ONE_THOUSAND;
      m_stSceneItemCFG.dwDropInterval = m->second.getTableInt("dropInterval");
      m_stSceneItemCFG.dwDropRadius = m->second.getTableInt("pickUpRadius");
      m_stSceneItemCFG.dwPickEffectRange = m->second.getTableInt("pickEffectItemRange");
      m_stSceneItemCFG.fTeamValidRange = m->second.getTableFloat("teamValidRange");

      DWORD dwLastCount = 0;
      m_stSceneItemCFG.vecRange.clear();
      auto range = [&](const string& str, xLuaData& data)
      {
        SDropRange stRange;

        stRange.dwMin = dwLastCount;
        stRange.dwMax = data.getTableInt("1");
        stRange.fRange = data.getTableFloat("2");

        if (stRange.dwMin > stRange.dwMax)
        {
          bCorrect = false;
          XERR << "[GameConfig] SceneDropItem.dropRange min > max值" << XEND;
        }

        m_stSceneItemCFG.vecRange.push_back(stRange);
        dwLastCount = stRange.dwMax;
      };
      m->second.getMutableData("dropRange").foreach(range);
    }
    else if (m->first == "ScenePurifyItem")
    {
      m_stSPurifyCFG.dwItemDisTime = m->second.getTableInt("disappeartime");
      m_stSPurifyCFG.dwItemDropInterval = m->second.getTableInt("dropInterval");
      m_stSPurifyCFG.dwPurifyRange = m->second.getTableInt("purifyrange");
      m_stSPurifyCFG.dwMaxPurify = m->second.getTableInt("maxPurify");
      m_stSPurifyCFG.dwGainInterval = m->second.getTableInt("gainInterval");
      if (m_stSPurifyCFG.dwGainInterval == 0)
      {
        XERR << "[GameConfig], purify恢复间隔为0" << XEND;
        bCorrect = false;
      }
    }
    else if (m->first == "NewRole")
    {
      m_stNewRoleCFG.vecItems.clear();
      xLuaData& item = m->second.getMutableData("item");
      auto index = [this](const string& key, xLuaData& data)
      {
        ItemInfo oItem;
        oItem.set_id(data.getTableInt("id"));
        oItem.set_count(data.getTableInt("num"));
        oItem.set_source(ESOURCE_NORMAL);

        m_stNewRoleCFG.vecItems.push_back(oItem);
      };
      item.foreach(index);

      m_stNewRoleCFG.vecMalePortrait.clear();
      xLuaData& portrait = m->second.getMutableData("portrait");
      xLuaData& male = portrait.getMutableData("male");
      auto malef = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecMalePortrait.push_back(data.getInt());
      };
      male.foreach(malef);
      m_stNewRoleCFG.vecFemalePortrait.clear();
      xLuaData& female = portrait.getMutableData("female");
      auto femalef = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecFemalePortrait.push_back(data.getInt());
      };
      female.foreach(femalef);

      m_stNewRoleCFG.vecFrame.clear();
      xLuaData& frame = m->second.getMutableData("frame");
      auto framef = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecFrame.push_back(data.getInt());
      };
      frame.foreach(framef);

      m_stNewRoleCFG.vecHair.clear();
      xLuaData& hair = m->second.getMutableData("hair");
      auto hairf = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecHair.push_back(data.getInt());
      };
      hair.foreach(hairf);

      m_stNewRoleCFG.vecEye.clear();
      xLuaData& eye = m->second.getMutableData("eye");
      auto eyef = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecEye.push_back(data.getInt());
      };
      eye.foreach(eyef);

      m_stNewRoleCFG.vecAction.clear();
      xLuaData& action = m->second.getMutableData("action");
      auto actionf = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecAction.push_back(data.getInt());
      };
      action.foreach(actionf);

      m_stNewRoleCFG.vecExpression.clear();
      xLuaData& express = m->second.getMutableData("expression");
      auto expressf = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecExpression.push_back(data.getInt());
      };
      express.foreach(expressf);

      m_stNewRoleCFG.vecBuffs.clear();
      xLuaData& buff = m->second.getMutableData("buff");
      auto bufff = [this](const string& key, xLuaData& data)
      {
        DWORD id = data.getInt();
        auto v = find(m_stNewRoleCFG.vecBuffs.begin(), m_stNewRoleCFG.vecBuffs.end(), id);
        if (v != m_stNewRoleCFG.vecBuffs.end())
          return;
        m_stNewRoleCFG.vecBuffs.push_back(data.getInt());
      };
      buff.foreach(bufff);

      m_stNewRoleCFG.vecFirstShortcut.clear();
      xLuaData& shortcut = m->second.getMutableData("shorcutfirst");
      auto shortcutf = [this](const string& key, xLuaData& data)
      {
        m_stNewRoleCFG.vecFirstShortcut.push_back(data.getInt());
      };
      shortcut.foreach(shortcutf);

      xLuaData& skill = m->second.getMutableData("riskskill");
      m_stNewRoleCFG.dwCollectSkill = skill.getTableInt("1");
      m_stNewRoleCFG.dwTransSkill = skill.getTableInt("2");
      //m_stNewRoleCFG.dwRepairSkill = skill.getTableInt("3");
      m_stNewRoleCFG.dwFlashSkill = m->second.getTableInt("flashskill");

      m_stNewRoleCFG.dwPurify = m->second.getTableInt("purify");

      m_stNewRoleCFG.dwMaxShortCut = m->second.getTableInt("maxshortcut");

      m_stNewRoleCFG.pairDefaultHair.first = m->second.getMutableData("default_hair").getTableInt("male");
      m_stNewRoleCFG.pairDefaultHair.second = m->second.getMutableData("default_hair").getTableInt("female");

      m_stNewRoleCFG.pairDefaultEye.first = m->second.getMutableData("default_eye").getTableInt("male");
      m_stNewRoleCFG.pairDefaultEye.second = m->second.getMutableData("default_eye").getTableInt("female");

      DWORD dwDefaultQueryType = m->second.getTableInt("default_querytype");
      if (dwDefaultQueryType <= EQUERYTYPE_MIN || dwDefaultQueryType >= EQUERYTYPE_MAX || EQueryType_IsValid(dwDefaultQueryType) == false)
      {
        bCorrect = false;
        XERR << "[GameConfig] default_querytype :" << dwDefaultQueryType << "不合法" << XEND;
      }
      else
      {
        m_stNewRoleCFG.eDefaultQueryType = static_cast<EQueryType>(m->second.getTableInt("default_querytype"));
      }

      m_stNewRoleCFG.vecActiveMap.clear();
      xLuaData& activemap = m->second.getMutableData("activemap");
      auto activemapf = [this](const string& str, xLuaData& data)
      {
        m_stNewRoleCFG.vecActiveMap.push_back(data.getInt());
      };
      activemap.foreach(activemapf);

      m_stNewRoleCFG.vecMapArea.clear();
      xLuaData& maparea = m->second.getMutableData("maparea");
      auto mapareaf = [this](const string& str, xLuaData& data)
      {
        m_stNewRoleCFG.vecMapArea.push_back(data.getInt());
      };
      maparea.foreach(mapareaf);

      m_stNewRoleCFG.vecManualCard.clear();
      xLuaData& manual = m->second.getMutableData("manual");
      xLuaData& card = manual.getMutableData("card");
      auto cardf = [this](const string& str, xLuaData& data)
      {
        m_stNewRoleCFG.vecManualCard.push_back(data.getInt());
      };
      card.foreach(cardf);

      xLuaData& defaultmanual = m->second.getMutableData("default_manual");

      m_stNewRoleCFG.vecManualItems.clear();
      xLuaData& defaultitems = defaultmanual.getMutableData("item");
      auto itemfunc = [this](const string& str, xLuaData& data)
      {
        m_stNewRoleCFG.vecManualItems.push_back(data.getInt());
      };
      defaultitems.foreach(itemfunc);

      xLuaData& defaultnpcs = defaultmanual.getMutableData("npc");
      auto npcfunc = [this](const string& str, xLuaData& data)
      {
        m_stNewRoleCFG.vecManualNpcs.push_back(data.getInt());
      };
      defaultnpcs.foreach(npcfunc);

      m_stNewRoleCFG.mapClassInitBuff.clear();
      xLuaData& classbuff = m->second.getMutableData("ClassBuff");
      auto getbuf = [this](const string& k, xLuaData& d)
      {
        DWORD profession = atoi(k.c_str());
        TSetDWORD& ids = m_stNewRoleCFG.mapClassInitBuff[profession];
        d.getIDList(ids);
      };
      classbuff.foreach(getbuf);
    }
    else if (m->first == "Team")
    {
      m_stTeamCFG.dwMaxMember = m->second.getTableInt("maxmember");
      m_stTeamCFG.dwMaxInvite = m->second.getTableInt("maxinvite");
      m_stTeamCFG.dwInviteTime = m->second.getTableInt("inviteovertime");
      m_stTeamCFG.dwApplyTime = m->second.getTableInt("applyovertime");
      m_stTeamCFG.dwOverTime = m->second.getTableInt("overtime");
      m_stTeamCFG.sDefaultName = m->second.getTableString("teamname");
      m_stTeamCFG.dwQuickEnterTime = m->second.getTableInt("quickentertime");
      m_stTeamCFG.dwPickupMode = m->second.getTableInt("pickupmode");

      m_stTeamCFG.dwDefaultType = m->second.getTableInt("defaulttype");

      TVecDWORD vecFilLv;
      auto fillvf = [&](const string& key, xLuaData& data)
      {
        vecFilLv.push_back(data.getInt());
      };
      m->second.getMutableData("filtratelevel").foreach(fillvf);
      sort(vecFilLv.begin(), vecFilLv.end());
      if (vecFilLv.empty() == true)
      {
        bCorrect = false;
        XERR << "[GameConfig] Team -> filtratelevel 不包含任何等级" << XEND;
        continue;
      }
      m_stTeamCFG.dwDefaultMinLv = *vecFilLv.begin();
      m_stTeamCFG.dwDefaultMaxLv = vecFilLv.back();

      DWORD dwDefaultAuto = m->second.getTableInt("defaultauto");
      if (dwDefaultAuto >= EAUTOTYPE_MAX || EAutoType_IsValid(dwDefaultAuto) == false)
      {
        bCorrect = false;
        XERR << "[GameConfig] Team -> defaultauto :" << dwDefaultAuto << "不合法" << XEND;
      }
      else
      {
        m_stTeamCFG.eDefaultAuto = static_cast<EAutoType>(dwDefaultAuto);
      }
    }
    /*else if (m->first == "EquipDecompose")
    {
      m_vecEquipDecomposeCFG.clear();
      auto decomposef = [this](const string& key, xLuaData& data)
      {
        SEquipDecomposeCFG stCFG;
        stCFG.eQuality = static_cast<EQualityType>(data.getTableInt("color"));
        ItemInfo oItem;
        oItem.set_id(data.getTableInt("itemid"));
        oItem.set_count(data.getTableInt("num"));
        stCFG.vecProduct.push_back(oItem);

        xLuaData& type = data.getMutableData("type");
        auto typef = [this, &stCFG](const string& key, xLuaData& d)
        {
          stCFG.eType = static_cast<EItemType>(d.getInt());
          m_vecEquipDecomposeCFG.push_back(stCFG);
        };
        type.foreach(typef);
      };
      m->second.foreach(decomposef);
    }*/
    else if (m->first == "StoreMaxCount")
    {
      m_stPackageCFG.dwMaxStoreCount = m->second.getTableInt("max");
    }
    else if (m->first == "CardComposeType")
    {
      m_stCardCFG.vecCardPosition.clear();
      auto cardf = [this](const string& key, xLuaData& data)
      {
        SCardPositionCFG stCFG;
        stCFG.dwPosition = stoi(key);

        xLuaData& type = data.getMutableData("types");
        auto typef = [&stCFG](const string& key, xLuaData& data)
        {
          DWORD type = data.getInt();
          if (type <= EITEMTYPE_MIN || type >= EITEMTYPE_MAX || EItemType_IsValid(type) == false)
            return;

          stCFG.vecItemType.push_back(static_cast<EItemType>(type));
        };
        type.foreach(typef);

        m_stCardCFG.vecCardPosition.push_back(stCFG);
      };
      m->second.foreach(cardf);
    }
    else if (m->first == "backpackSort")
    {
      m_stPackageCFG.vecSortType.clear();
      if (m_stPackageCFG.vecSortType.empty() == true)
        m_stPackageCFG.vecSortType.resize(m->second.m_table.size());

      auto packf = [this](const string& key, xLuaData& data)
      {
        DWORD index = stoi(key) - 1;
        DWORD type = data.getInt();
        if (type <= EITEMTYPE_MIN || type >= EITEMTYPE_MAX || EItemType_IsValid(type) == false)
        {
          XERR << "[MiscConfig] item = backpackSort type = " << type << " invalid!" << XEND;
          return;
        }
        auto v = find(m_stPackageCFG.vecSortType.begin(), m_stPackageCFG.vecSortType.end(), type);
        if (v != m_stPackageCFG.vecSortType.end())
        {
          XERR << "[MiscConfig] item = backpackSort type = " << type << " duplicated!" << XEND;
          return;
        }
        m_stPackageCFG.vecSortType[index] = static_cast<EItemType>(type);
      };
      m->second.foreach(packf);
    }
    else if (m->first == "Quest")
    {
      auto maxf = [&](const string& key, xLuaData& data)
      {
        DWORD dwType = data.getTableInt("1");
        if (dwType >= EWANTEDTYPE_MAX || EWantedType_IsValid(dwType) == false)
        {
          bCorrect = false;
          XERR << "[MiscConfig] Quest : maxwanted : type :" << dwType << "不合法" << XEND;
          return;
        }
        m_stQuestCFG.arrMaxWanted[dwType] = data.getTableInt("2");
      };
      m->second.getMutableData("maxwanted").foreach(maxf);

      m_stQuestCFG.dwResetProtectTime = m->second.getTableInt("resetprotecttime");
      m_stQuestCFG.dwDailyIncrease = m->second.getTableInt("dailycount");
      m_stQuestCFG.dwMaxDailyCount = m->second.getTableInt("maxdailycount");
      m_stQuestCFG.dwMaxMapCount = m->second.getTableInt("maxcount");
      m_stQuestCFG.dwTeamFinishBoardQuestCD = m->second.getTableInt("team_finish_board_quest_cd");

      m_stQuestCFG.vecWantedRefresh.clear();
      xLuaData& refresh = m->second.getMutableData("refresh");
      auto refreshf = [this](const string& key, xLuaData& data)
      {
        const string& str = data.getString();
        DWORD dwHour = 0;
        DWORD dwMin = 0;
        sscanf(str.c_str(), "%u:%u", &dwHour, &dwMin);

        DWORD dwTime = dwHour * HOUR_T + dwMin * MIN_T;
        m_stQuestCFG.vecWantedRefresh.push_back(dwTime);
      };
      refresh.foreach(refreshf);
      if (m_stQuestCFG.vecWantedRefresh.empty())
      {
        bCorrect = false;
        XERR << "[Misc配置-任务] 看板重置时间未配置" << XEND;
      }

      m_stQuestCFG.vecWantedParams.clear();
      auto proportionf = [&](const string& key, xLuaData& data)
      {
        m_stQuestCFG.vecWantedParams.push_back(data.getFloat());
      };
      m->second.getMutableData("proportion").foreach(proportionf);
      std::sort(m_stQuestCFG.vecWantedParams.begin(), m_stQuestCFG.vecWantedParams.end());

      m_stQuestCFG.vecBoardQuickFinishItems.clear();
      auto itemf = [&](const string& key, xLuaData& data)
      {
        SQuickBoardItem stItem;
        stItem.dwItemID = data.getTableInt("1");
        stItem.lvrange.first = data.getTableInt("2");
        stItem.lvrange.second = data.getTableInt("3");
        m_stQuestCFG.vecBoardQuickFinishItems.push_back(stItem);
      };
      m->second.getMutableData("quick_finish_board_quest").foreach(itemf);

      DWORD type = 1;
      auto typef = [&](const string& key, xLuaData& data)
      {
        EQuestType eType = QuestConfig::getMe().getQuestType(data.getString());
        if (eType <= EQUESTTYPE_MIN || eType >= EQUESTTYPE_MAX || EQuestType_IsValid(eType) == false)
        {
          XERR << "[GameConfig -> Quest] type :" << data.getString() << "不合法" << XEND;
          return;
        }

        if (type == 1)
          m_stQuestCFG.setManualMainType.insert(eType);
        else if (type == 2)
          m_stQuestCFG.setManualStoryType.insert(eType);
      };

      type = 1, m->second.getMutableData("manual_main").foreach(typef);
      type = 2, m->second.getMutableData("manual_story").foreach(typef);
    }
    else if (m->first == "EndlessTower")
    {
      m_stEndlessTowerCFG.dwDefaultFloor = m->second.getTableInt("defaultFloor");
      //m_stEndlessTowerCFG.dwUpdateTimeSecDiff = m->second.getTableInt("rewardReset");
      m_stEndlessTowerCFG.dwMaxSkillLv = m->second.getTableInt("maxskilllv");

      m_stEndlessTowerCFG.oRewardBoxDefine = m->second.getMutableData("rewardbox");
      m_stEndlessTowerCFG.fRewardBoxRange = m->second.getTableFloat("rewardbox_range");
      m_stEndlessTowerCFG.dwRewardBoxGetAction = m->second.getTableInt("rewardbox_action_get");
      m_stEndlessTowerCFG.dwRewardBoxUngetAction = m->second.getTableInt("rewardbox_action_unget");
      m_stEndlessTowerCFG.dwBossFloorEffect = m->second.getTableInt("boss_floor_effect");
      m_stEndlessTowerCFG.strRewardBoxEffect = m->second.getTableString("rewardbox_effect");

      xLuaData& mini = m->second.getMutableData("minibossscale");
      m_stEndlessTowerCFG.pairMiniScale.first = mini.getTableInt("1");
      m_stEndlessTowerCFG.pairMiniScale.second = mini.getTableInt("2");

      m_stEndlessTowerCFG.vecBosPos.clear();
      xLuaData& boxpos = m->second.getMutableData("bossboxpos");
      auto boxposf = [&](const string& str, xLuaData& data)
      {
        SEndlessBox stBox;
        stBox.pairFloor.first = data.getTableInt("1");
        stBox.pairFloor.second = data.getTableInt("2");

        xLuaData& pos = data.getMutableData("3");
        stBox.oPos.x = pos.getTableInt("1");
        stBox.oPos.y = pos.getTableInt("2");
        stBox.oPos.z = pos.getTableInt("3");

        m_stEndlessTowerCFG.vecBosPos.push_back(stBox);
      };
      boxpos.foreach(boxposf);

      m_stEndlessTowerCFG.dwLimitUserLv = m->second.getTableInt("limit_user_lv");
      m_stEndlessTowerCFG.dwRecordLayer = m->second.getTableInt("record_layer");
    }
    else if (m->first == "Seal")
    {
      m_stSealCFG.dwSkillID = m->second.getTableInt("skillid");
      m_stSealCFG.dwSealNpcID = m->second.getTableInt("npcid");
      m_stSealCFG.dwSealRefresh = m->second.getTableInt("sealRefresh");
      m_stSealCFG.dwChangePosTime = m->second.getTableInt("changePositionTime");
      m_stSealCFG.dwRewardDispTime = m->second.getTableInt("itemGuardTime");
      m_stSealCFG.dwMaxDaySealNum = m->second.getTableInt("maxSealNum");
      m_stSealCFG.dwSpeed = m->second.getTableInt("barSpeed");
      m_stSealCFG.dwSealNextTime = m->second.getTableInt("sealNexttime");
      m_stSealCFG.dwSealRange = m->second.getTableInt("sealRange");
      m_stSealCFG.dwRewardRange = m->second.getTableInt("rewardRange");
      m_stSealCFG.dwDropDelay = m->second.getTableInt("dropDelay");
      m_stSealCFG.strGMShakeScreen = m->second.getTableString("shakeScreen");
      m_stSealCFG.dwPreActionTime = m->second.getTableInt("preActionTime");
      m_stSealCFG.dwCountDownTime = m->second.getTableInt("countDownTime");
      m_stSealCFG.dwQuickFinishBuff = m->second.getTableInt("quickFinishBuff");
      m_stSealCFG.dwWaitTime = m->second.getTableInt("waitTime");

      m_stSealCFG.vecQuickFinishItems.clear();
      auto quickf = [&](const string& key, xLuaData& data)
      {
        pair<DWORD, DWORD> p;
        p.first = data.getTableInt("1");
        p.second = data.getTableInt("2");
        m_stSealCFG.vecQuickFinishItems.push_back(p);
      };
      m->second.getMutableData("quickfinish_cost").foreach(quickf);
      for (auto &v : m_stSealCFG.vecQuickFinishItems)
        XDBG << "[快速完成封印配置]" << v.first << v.second << XEND;

      m_stSealCFG.mapMapLv.clear();
      xLuaData& npclv = m->second.getMutableData("npclv");
      auto npclvf = [&](const string& key, xLuaData& data)
      {
        DWORD dwMapID = atoi(key.c_str());
        DWORD dwLv = data.getInt();
        auto m = m_stSealCFG.mapMapLv.find(dwMapID);
        if (m != m_stSealCFG.mapMapLv.end())
        {
          bCorrect = false;
          XERR << "[GameConfig] Seal->npclv key :" << key << "重复了" << XEND;
          return;
        }
        m_stSealCFG.mapMapLv[dwMapID] = dwLv;
      };
      npclv.foreach(npclvf);
    }
    else if (m->first == "EquipRefineRate")
    {
      m_vecItem2Rate.clear();
      auto rrate = [this] (const string& key, xLuaData& data)
      {
        pair<DWORD, DWORD> pa;
        pa.first = data.getTableInt("itemid");
        pa.second = data.getTableInt("rate");
        m_vecItem2Rate.push_back(pa);
      };
      m->second.foreach(rrate);
    }
    else if (m->first == "System")
    {
      m_stSystemCFG.dwTimeSpeed = m->second.getTableInt("timespeed");
      if (m_stSystemCFG.dwTimeSpeed)
        m_stSystemCFG.dwGameDaySec = 24 * HOUR_T / m_stSystemCFG.dwTimeSpeed;
      else
        m_stSystemCFG.dwGameDaySec = 0;

      m_stSystemCFG.dwMaxBaseLv = m->second.getTableInt("maxbaselevel");
      m_stSystemCFG.dwMaxAttrPoint = m->second.getTableInt("maxattrpoint");

      m_stSystemCFG.dwMusicBoxNpc = m->second.getTableInt("musicboxnpc");
      m_stSystemCFG.dwMusicStatusPlay = m->second.getTableInt("music_status_play");
      m_stSystemCFG.dwMusicStatusStop = m->second.getTableInt("music_status_stop");
      m_stSystemCFG.dwMusicRange = m->second.getTableInt("musicrange");

      m_stSystemCFG.dwNameMaxSize = m->second.getTableInt("namesize_max");
      m_stSystemCFG.dwNameMinSize = m->second.getTableInt("namesize_min");

      m_stSystemCFG.dwGuildNameMaxSize = m->second.getTableInt("guildname_max");
      m_stSystemCFG.dwGuildNameMinSize = m->second.getTableInt("guildname_min");

      m_stSystemCFG.dwGuildBoardMaxSize = m->second.getTableInt("guildboard_max");
      m_stSystemCFG.dwGuildRecruitMaxSize = m->second.getTableInt("guildrecruit_max");

      m_stSystemCFG.dwGuildJobMinSize = m->second.getTableInt("guildjob_min");
      m_stSystemCFG.dwGuildJobMaxSize = m->second.getTableInt("guildjob_max");

      m_stSystemCFG.dwItemShowTime = m->second.getTableInt("itemshowtime");
      m_stSystemCFG.dwHandRange = m->second.getTableInt("handrange");

      m_stSystemCFG.dwTeamNameMinSize = m->second.getTableInt("teamname_min");
      m_stSystemCFG.dwTeamNameMaxSize = m->second.getTableInt("teamname_max");

      m_stSystemCFG.dwRoomNameMinSize = m->second.getTableInt("roomname_min");
      m_stSystemCFG.dwRoomNameMaxSize = m->second.getTableInt("roomname_max");

      m_stSystemCFG.dwBattleInterval = m->second.getTableInt("battleinterval");

      m_stSystemCFG.dwSkyPassTime = m->second.getTableInt("sky_pass_time");
      if (m_stSystemCFG.dwBattleInterval == 0)
      {
        XERR << "[Config], gameconfig system->battleinterval配置错误" << XEND;
        bCorrect = false;
      }

      m_stSystemCFG.vecBarrageMap.clear();
      xLuaData& barrage = m->second.getMutableData("barragemap");
      auto barragef = [&](const string& str, xLuaData& data)
      {
        m_stSystemCFG.vecBarrageMap.push_back(data.getInt());
      };
      barrage.foreach(barragef);

      m_stSystemCFG.dwGoMapBuff = m->second.getTableInt("gomap_buff");

      m_stSystemCFG.dwMaxMusicBattleTime = m->second.getTableInt("max_music_battle_time");
      m_stSystemCFG.dwMusicCheckTime = m->second.getTableInt("music_check_time");
      m_stSystemCFG.dwMusicContinueTime = m->second.getTableInt("music_continue_time");
      m_stSystemCFG.dwMusicReturnTime = m->second.getTableInt("music_return_time");
      m_stSystemCFG.dwMaxBaseBattleTime = m->second.getTableInt("max_base_battle_time");
      m_stSystemCFG.dwMusicBuff = m->second.getTableInt("music_buff");

      m_stSystemCFG.dwNewCharMapID = m->second.getTableInt("newchar_map_id");
      m_stSystemCFG.dwMainCityMapID = m->second.getTableInt("maincity_map_id");

      DWORD dwPro = m->second.getTableInt("newchar_profession");
      if (dwPro <= EPROFESSION_MIN || dwPro >= EPROFESSION_MAX || EProfession_IsValid(dwPro) == false)
      {
        XERR << "[GameConfig] newchar_profession :" << dwPro << "不合法" << XEND;
        bCorrect = false;
      }
      else
      {
        m_stSystemCFG.eNewCharPro = static_cast<EProfession>(dwPro);
      }

      m_stSystemCFG.dwZoneBossLimitBuff = m->second.getTableInt("zone_boss_limit_buff");
      m_stSystemCFG.dwOpenStoreCost = m->second.getTableInt("warehouseZeny");

      //m_stSystemCFG.dwMailOverTime = m->second.getTableInt("mail_overtime") * DAY_T;
      m_stSystemCFG.dwSysMailOverTime = m->second.getTableInt("sysmail_overtime") * DAY_T;
      m_stSystemCFG.dwChatWorldReqLv = m->second.getTableInt("chat_world_reqlv");
      m_stSystemCFG.bMergeLineValid = m->second.getTableInt("merge_line_switch");
    }
    else if (m->first == "LockEffect")
    {
      m_stBeLockCFG.dwChainAtkLmtTime = m->second.getTableInt("chainAtkLmt_time");
      if (m_stBeLockCFG.dwChainAtkLmtTime == 0)
      {
        XERR << "[GameConfig:LockEffect], chainAtkLmt_time配置错误, 不能为0" << XEND;
        bCorrect = false;
        continue;
      }
      DWORD gap = m->second.getTableInt("interval");
      if (gap < 2)
      {
        XERR << "[GameConfig:LockEffect], 更新时间配置过短" << XEND;
        bCorrect = false;
        continue;
      }
      m_stBeLockCFG.dwRefreshInterval = gap;
      m_stBeLockCFG.dwValidRange = m->second.getTableInt("range");

      m_stBeLockCFG.dwMaxFleeLockNum = m->second.getTableInt("fleemaxnum");
      m_stBeLockCFG.dwMinFleeLockNum = m->second.getTableInt("fleeminnum");
      m_stBeLockCFG.dwAutoHitTime = m->second.getTableInt("autohittime");
      if (m_stBeLockCFG.dwMaxFleeLockNum <= m_stBeLockCFG.dwMinFleeLockNum)
      {
        XERR << "[GameConfig:LockEffect], fleemaxnum，fleeminnum配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      float fleemaxper = m->second.getTableFloat("fleemaxper");
      m_stBeLockCFG.fFleeOneDecPer = fleemaxper / (m_stBeLockCFG.dwMaxFleeLockNum - m_stBeLockCFG.dwMinFleeLockNum);

      m_stBeLockCFG.dwMaxDefLockNum = m->second.getTableInt("defmaxnum");
      m_stBeLockCFG.dwMinDefLockNum = m->second.getTableInt("defminnum");
      if (m_stBeLockCFG.dwMaxDefLockNum <= m_stBeLockCFG.dwMinDefLockNum)
      {
        XERR << "[GameConfig:LockEffect], defmaxnum，defminnum配置错误" << XEND;
        bCorrect = false;
        continue;
      }
      float defmaxper = m->second.getTableFloat("defmaxper");
      m_stBeLockCFG.fDefOneDecPer = defmaxper / (m_stBeLockCFG.dwMaxDefLockNum - m_stBeLockCFG.dwMinDefLockNum);
    }
    else if (m->first == "EffectPath")
    {
      m_stEffectPath.strLeaveSceneEffect = m->second.getMutableData("leavescene").getTableString("effect");
      m_stEffectPath.strLeaveSceneSound = m->second.getMutableData("leavescene").getTableString("sound");
      m_stEffectPath.strEnterSceneEffect = m->second.getMutableData("enterscene").getTableString("effect");
      m_stEffectPath.strEnterSceneSound = m->second.getMutableData("enterscene").getTableString("sound");
      m_stEffectPath.strTeleportEffect = m->second.getMutableData("TeleportSkill").getTableString("effect");
      m_stEffectPath.oImmuneEffect = m->second.getMutableData("BuffImmune");
      m_stEffectPath.oResistEffect = m->second.getMutableData("BuffResist");
    }
    else if (m->first == "Social")
    {
      m_stSocialityCFG.dwMaxApplyCount = m->second.getTableInt("maxapply");
      m_stSocialityCFG.dwMaxFriendCount = m->second.getTableInt("maxfriend");
      m_stSocialityCFG.dwMaxFindCount = m->second.getTableInt("maxfind");
      m_stSocialityCFG.dwMaxNearTeamCount = m->second.getTableInt("maxteam");
      m_stSocialityCFG.dwMaxChatWords = m->second.getTableInt("maxchatwords");
      m_stSocialityCFG.dwBlackOverTime = m->second.getTableInt("BlackListRemoveTime") * DAY_T;
      m_stSocialityCFG.dwMaxBlackCount = m->second.getTableInt("maxblack");
      m_stSocialityCFG.dwMaxChatCount = m->second.getTableInt("maxchat");
    }
    else if (m->first == "Produce")
    {
      m_stProduceCFG.dwPreAction = m->second.getTableInt("preaction");
      m_stProduceCFG.dwPreActionTime = m->second.getTableInt("preactiontime");
    }
    else if (m->first == "EquipRefine")
    {
      m_stRefineCFG.vecSuccessEmoji.clear();
      m_stRefineCFG.vecFailEmoji.clear();
      m_stRefineCFG.dwNpcAction = m->second.getTableInt("npc_action");
      m_stRefineCFG.dwGuildNpcAction = m->second.getTableInt("guild_npc_action");
      DWORD type = 1;
      auto getemoji = [&](const string& key, xLuaData& data)
      {
        if (type == 1)
          m_stRefineCFG.vecSuccessEmoji.push_back(data.getInt());
        else if (type == 2)
          m_stRefineCFG.vecFailEmoji.push_back(data.getInt());
      };
      xLuaData& sdata = m->second.getMutableData("success_emoji");
      xLuaData& fdata = m->second.getMutableData("fail_emoji");
      type = 1;
      sdata.foreach(getemoji);
      type = 2;
      fdata.foreach(getemoji);

      m_stRefineCFG.strSuccessEffect = m->second.getTableString("success_effect");
      m_stRefineCFG.strFailEffect = m->second.getTableString("fail_effect");
      m_stRefineCFG.dwDelayMSec = m->second.getTableInt("delay_time");
      m_stRefineCFG.dwRepairMaxLv = m->second.getTableInt("repair_stuff_max_lv");
    }
    else if (m->first == "EquipStrength")
    {
      m_stStrengthCFG.dwGuildNpcAction = m->second.getTableInt("guild_npc_action");
    }
    else if (m->first == "Laboratory")
    {
      xLuaData& garden = m->second.getMutableData("garden");
      m_stLaboratoryCFG.dwGarden = garden.getTableInt("1");
      m_stLaboratoryCFG.fGarden = garden.getTableFloat("2");

      xLuaData& rob = m->second.getMutableData("rob");
      m_stLaboratoryCFG.dwRob = rob.getTableInt("1");
      m_stLaboratoryCFG.fRob = rob.getTableFloat("2");
    }
    else if (m->first == "ChatRoom")
    {
      m_stChatRoomCFG.dwEnterProtectTime = m->second.getTableInt("enter_protect_time");

      m_stChatRoomCFG.vecBuffIDs.clear();
      xLuaData& buff = m->second.getMutableData("buffid");
      auto bufff = [&](const string& key, xLuaData& data)
      {
        m_stChatRoomCFG.vecBuffIDs.push_back(data.getInt());
      };
      buff.foreach(bufff);
    }
    else if (m->first == "Guild")
    {
      m_stGuildCFG.dwCreateBaseLv = m->second.getTableInt("createbaselv");
      m_stGuildCFG.dwDismissTime = m->second.getTableInt("dismisstime");
      m_stGuildCFG.dwChairOffline = m->second.getTableInt("chairoffline");
      m_stGuildCFG.dwMaxApplyCount = m->second.getTableInt("maxapplycount");
      //m_stGuildCFG.dwFreezePercent = m->second.getTableInt("freezepercent");
      m_stGuildCFG.dwTerritory = m->second.getTableInt("territory");
      m_stGuildCFG.dwPrayAction = m->second.getTableInt("praynpcaction");
      m_stGuildCFG.dwMaxEventCount = m->second.getTableInt("max_event_count");
      m_stGuildCFG.dwEventOverTime = m->second.getTableInt("event_overtime");
      m_stGuildCFG.strPrayEffect = m->second.getTableString("praynpceffect");
      m_stGuildCFG.strDefaultPortrait = m->second.getTableString("defaultportrait");

      m_stGuildCFG.dwAssetItemID = m->second.getTableInt("asset_id");
      m_stGuildCFG.dwAssetGoldID = m->second.getTableInt("asset_gold_id");

      m_stGuildCFG.dwRenameCoolDown = m->second.getTableInt("rename_cooldown");
      m_stGuildCFG.dwRenameItemId = m->second.getTableInt("rename_item");

      m_stGuildCFG.dwIconCount = m->second.getTableInt("icon_uplimit");
      m_stGuildCFG.dwEnterPunishTime = m->second.getTableInt("enterpunishtime");
      m_stGuildCFG.strOfflineChChariManMsg = m->second.getTableString("chairoffline_change_msg");
      m_stGuildCFG.dwRealtimeVoiceLimit = m->second.getTableInt("realtime_voice_limit");

      m_stGuildCFG.dwBuildingCheckLv = m->second.getTableInt("building_check_lv");
      m_stGuildCFG.vecCreateItems.clear();
      xLuaData& data = m->second.getMutableData("createitem");
      auto dataf = [this](const string& key, xLuaData& data)
      {
        ItemInfo stItem;
        stItem.set_id(data.getTableInt("1"));
        stItem.set_count(data.getTableInt("2"));

        m_stGuildCFG.vecCreateItems.push_back(stItem);
      };
      data.foreach(dataf);

      xLuaData& pray = m->second.getMutableData("praydeduction");
      m_stGuildCFG.dwPrayItem = pray.getTableInt("1");
      m_stGuildCFG.dwPrayRob = pray.getTableInt("2");

      m_stGuildCFG.dwCityGiveupCD = m->second.getTableInt("city_giveup_cd");
    }
    else if (m->first == "Exchange")
    {
      m_stTradeCFG.dwExpireTime = m->second.getTableInt("ExchangeHour");
      m_stTradeCFG.fRate = m->second.getTableFloat("ExchangeRate");
      m_stTradeCFG.dwMaxPendingCount = m->second.getTableInt("MaxPendingCount");
      m_stTradeCFG.dwAdjustPendingInterval = m->second.getTableInt("AdjustPendingInterval");
      m_stTradeCFG.dwMaxLogCount = m->second.getTableInt("MaxLog");
      m_stTradeCFG.dwHotTime = m->second.getTableInt("HotTime");
      m_stTradeCFG.fSellCost = m->second.getTableFloat("SellCost");
      m_stTradeCFG.dwMaxBoothfee = m->second.getTableInt("MaxBoothfee");      

      xLuaData& rData = m->second.getMutableData("Cycle_T");
      m_stTradeCFG.dwCycleTBegin = rData.getTableInt("1");
      m_stTradeCFG.dwCycleTEnd = rData.getTableInt("2");
      m_stTradeCFG.dwCycleKT = m->second.getTableInt("Cycle_KT");
      m_stTradeCFG.fMaxPriceUp = m->second.getTableFloat("MaxPriceUp");
      m_stTradeCFG.fNoDealDropRatio = m->second.getTableFloat("NoDealDropRatio");

      m_stTradeCFG.dwUpRate = m->second.getTableInt("UpRate");
      if (m_stTradeCFG.dwUpRate == 0)
        m_stTradeCFG.dwUpRate = 10;
      m_stTradeCFG.dwDownRate = m->second.getTableInt("DownRate");
      if (m_stTradeCFG.dwDownRate == 0)
        m_stTradeCFG.dwDownRate = 100;

      m_stTradeCFG.fGoodRate = m->second.getTableFloat("GoodRate");
      m_stTradeCFG.dwMobPrice = m->second.getTableInt("MobPrice");
      m_stTradeCFG.fInflation = m->second.getTableFloat("Inflation");

      xLuaData& rLvRateData = m->second.getMutableData("LvRate");
      m_stTradeCFG.vecLvRate.clear();
      m_stTradeCFG.vecLvRate.push_back(rLvRateData.getTableInt("1"));
      m_stTradeCFG.vecLvRate.push_back(rLvRateData.getTableInt("2"));
      m_stTradeCFG.vecLvRate.push_back(rLvRateData.getTableInt("3"));
      m_stTradeCFG.vecLvRate.push_back(rLvRateData.getTableInt("4")); 

      xLuaData& rTypeRateData = m->second.getMutableData("TypeRate");
      m_stTradeCFG.vecTypeRate.clear();
      m_stTradeCFG.vecTypeRate.push_back(rTypeRateData.getTableFloat("1"));
      m_stTradeCFG.vecTypeRate.push_back(rTypeRateData.getTableFloat("2"));
      m_stTradeCFG.vecTypeRate.push_back(rTypeRateData.getTableFloat("3"));
      m_stTradeCFG.dwLogTime = m->second.getTableInt("LogTime");
      m_stTradeCFG.dwPageNumber = m->second.getTableInt("PageNumber");
      m_stTradeCFG.dwCantSendTime = m->second.getTableInt("CantSendTime");
      m_stTradeCFG.dwSendMoneyLimit = m->second.getTableInt("SendMoneyLimit");
      m_stTradeCFG.dwWeekRefineRate = m->second.getTableInt("WeekRefineRate");
      m_stTradeCFG.dwMonthRefineRate = m->second.getTableInt("MonthRefineRate");
      m_stTradeCFG.dwSendButtonTime = m->second.getTableInt("SendButtonTime");
      
      xLuaData& rSendMoney = m->second.getMutableData("SendMoney");
      m_stTradeCFG.m_giveBack.clear();
      auto givef = [&](const string& key, xLuaData& data)
      {
        DWORD bgid = atoi(key.c_str());
        if (bgid == 0)
          return;
        SGiveBackground& rGiveBack = m_stTradeCFG.m_giveBack[bgid];
        rGiveBack.dwBuffid = data.getTableInt("buffid");
      };
      rSendMoney.foreach(givef);
    }
    else if(m->first == "Booth")
    {
      m_stSystemCFG.dwBoothNameMinSize = 1;
      m_stSystemCFG.dwBoothNameMaxSize = m->second.getTableInt("name_length_max");

      m_stBoothCFG.dwBasePendingCount = m->second.getTableInt("base_pending_count");
      m_stBoothCFG.dwQuotaExchangeRate = m->second.getTableInt("quota_exchange_rate");
      m_stBoothCFG.dwNameLengthMax = m->second.getTableInt("name_length_max");
      m_stBoothCFG.dwQuotaCostMax = m->second.getTableInt("quota_cost_max");
      m_stBoothCFG.dwPendingCountMax = m->second.getTableInt("max_pending_count");
      m_stBoothCFG.dwScoreExchangeRate = m->second.getTableInt("score_exchange_rate");
      m_stBoothCFG.dwBuffId = m->second.getTableInt("booth_buff");
      m_stBoothCFG.dwMaxSizeOneScene = m->second.getTableInt("max_size_one_scene");
      m_stBoothCFG.dwMaxSizeNine = m->second.getTableInt("max_size_nine");
      m_stBoothCFG.dwUpdateCD = m->second.getTableInt("update_cd");
      m_stBoothCFG.dwSkillId = m->second.getTableInt("skill_id");

      xLuaData& mapIds = m->second.getMutableData("map_ids");
      m_stBoothCFG.vecMaps.clear();
      auto fmap = [this](const string& key, xLuaData& data)
      {
        m_stBoothCFG.vecMaps.push_back(data.getInt());
      };
      mapIds.foreach(fmap);

      xLuaData& signIds = m->second.getMutableData("score");
      m_stBoothCFG.mapSign2Score.clear();
      auto fsign = [this](const string& key, xLuaData& data)
      {
        DWORD dwSign = atoi(key.c_str());
        DWORD dwScore = data.getTableInt("num");
        m_stBoothCFG.mapSign2Score[dwSign] = dwScore;
      };
      signIds.foreach(fsign);
    }
    else if (m->first == "Item")
    {
      m_stItemCFG.dwMaxSellRefineLv = m->second.getTableInt("sell_equip_max_refine_lv");
      m_stItemCFG.dwMainMaxSlot = m->second.getTableInt("main_max_slot");
      m_stItemCFG.dwTempMainMaxSlot = m->second.getTableInt("tempmain_max_slot");
      m_stItemCFG.dwTempMainOvertime = m->second.getTableInt("tempmain_item_overtime") * HOUR_T;
      m_stItemCFG.dwPersonalStoreMaxSlot = m->second.getTableInt("personalstore_max_slot");
      m_stItemCFG.dwStoreMaxSlot = m->second.getTableInt("store_max_slot");
      m_stItemCFG.dwBarrowMaxSlot = m->second.getTableInt("barrow_max_slot");
      m_stItemCFG.dwQuestMaxSlot = m->second.getTableInt("quest_max_slot");
      if (m_stItemCFG.dwQuestMaxSlot == 0)
        m_stItemCFG.dwQuestMaxSlot = PACK_QUEST_DEFAULT_MAXSLOT;
      m_stItemCFG.dwFoodMaxSlot = m->second.getTableInt("food_max_slot");
      if (m_stItemCFG.dwFoodMaxSlot == 0)
        m_stItemCFG.dwFoodMaxSlot = PACK_FOOD_DEFAULT_MAXSLOT;
      m_stItemCFG.dwPetMaxSlot = m->second.getTableInt("pet_max_slot");
      if (m_stItemCFG.dwPetMaxSlot == 0)
        m_stItemCFG.dwPetMaxSlot = PACK_PET_DEFAULT_MAXSLOT;
      m_stItemCFG.dwStoreBaseLvReq = m->second.getTableInt("store_baselv_req");
      m_stItemCFG.dwStoreTakeBaseLvReq = m->second.getTableInt("store_takeout_baselv_req");
      m_stItemCFG.dwWeponcatItem = m->second.getTableInt("weaponcat_itemid");
      m_stItemCFG.dwMaxMaterialRefineLv = m->second.getTableInt("material_max_refine");
      m_stItemCFG.mapPackSkillSlot.clear();
      m_stItemCFG.setPackSkill.clear();
      xLuaData& skillslot = m->second.getMutableData("skill_slot");
      auto skillslotf = [&](const string& key, xLuaData& data)
      {
        EPackType eType = EPACKTYPE_MIN;
        if (key == "main")
          eType = EPACKTYPE_MAIN;
        else if (key == "tempmain")
          eType = EPACKTYPE_TEMP_MAIN;
        else if (key == "pstore")
          eType = EPACKTYPE_PERSONAL_STORE;
        else if (key == "store")
          eType = EPACKTYPE_STORE;
        else
        {
          bCorrect = false;
          XERR << "[GameConfig] Item->skill_slot 包裹类型错误" << XEND;
          return;
        }

        TVecSkillSlot& vecSlot = m_stItemCFG.mapPackSkillSlot[eType];
        auto slotf = [&](const string& key, xLuaData& data)
        {
          SSkillSlot stItem;
          stItem.dwSkillID = data.getTableInt("1");
          stItem.attr = data.getTableString("2");
          stItem.dwAttrValue = data.getTableInt("3");
          stItem.dwSlot = data.getTableInt("4");
          vecSlot.push_back(stItem);
          m_stItemCFG.setPackSkill.insert(stItem.dwSkillID);
        };
        data.foreach(slotf);
      };
      skillslot.foreach(skillslotf);
    }
    else if (m->first == "EquipRecover")
    {
      m_stItemCFG.dwRestoreStrengthCost = m->second.getTableInt("Strength");

      xLuaData& cardcost = m->second.getMutableData("Card");
      m_stItemCFG.mapRestoreCardCost.clear();
      m_stItemCFG.mapRestoreCardCost[EQUALITYTYPE_PURPLE] = cardcost.getTableInt("4");
      m_stItemCFG.mapRestoreCardCost[EQUALITYTYPE_BLUE] = cardcost.getTableInt("3");
      m_stItemCFG.mapRestoreCardCost[EQUALITYTYPE_GREEN] = cardcost.getTableInt("2");
      m_stItemCFG.mapRestoreCardCost[EQUALITYTYPE_WHITE] = cardcost.getTableInt("1");

      m_stItemCFG.mapRestoreEnchantCost.clear();
      m_stItemCFG.mapRestoreEnchantCost[EENCHANTTYPE_PRIMARY] = m->second.getTableInt("Enchant");
      m_stItemCFG.mapRestoreEnchantCost[EENCHANTTYPE_MEDIUM] = m->second.getTableInt("Enchant");
      m_stItemCFG.mapRestoreEnchantCost[EENCHANTTYPE_SENIOR] = m->second.getTableInt("Enchant");

      m_stItemCFG.mapRestoreUpgradeCost.clear();
      auto funcf = [&](const string& key, xLuaData& data)
      {
        DWORD dwIndex = atoi(key.c_str());
        m_stItemCFG.mapRestoreUpgradeCost[dwIndex] = data.getInt();
      };
      m->second.getMutableData("Upgrade").foreach(funcf);
    }
    else if (m->first == "TrapNpcID")
    {
      m_stTrapNpcCFG.vecTrapNpcIDs.clear();
      auto getid = [this](const string& key, xLuaData& data)
      {
        m_stTrapNpcCFG.vecTrapNpcIDs.push_back(data.getInt());
      };
      m->second.foreach(getid);
    }
    else if (m->first == "GuildDojo")
    {
      m_stGuildDojoCFG.dwCountDownTick = m->second.getTableInt("CountdownTick");
      auto dojolv = [&](const string& key, xLuaData& data)
      {
        if (data.getTableInt("DojoGroupId") == 0)
          return;
        m_stGuildDojoCFG.mapBaseLvReq[data.getTableInt("DojoGroupId")] = data.getTableInt("lvreq");
      };
      m->second.getMutableData("Dojo").foreach(dojolv);
    }
    else if (m->first == "Expression_Blink")
    {
      m_stExpressionCFG.dwBlinkNeedSkill = m->second.getTableInt("needskill");
    }
    else if (m->first == "BaseAttrConfig")
    {
      m_stAttrCFG.vecShowAttrs.clear();
      auto attrf = [&](const string& key, xLuaData& data)
      {
        DWORD dwAttr = RoleDataConfig::getMe().getIDByName(data.getString());
        if (dwAttr <= EATTRTYPE_MIN || dwAttr >= EATTRTYPE_MAX || EAttrType_IsValid(dwAttr) == false)
        {
          bCorrect = false;
          XERR << "[MiscConfig] BaseAttrConfig attr : " << data.getString() << " 不合法" << XEND;
        }
        m_stAttrCFG.vecShowAttrs.push_back(static_cast<EAttrType>(dwAttr));
      };
      m->second.foreach(attrf);

      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MAXHP);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MAXHPPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MAXSP);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MAXSPPER);

      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_ATKPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_DEFPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MATKPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MDEFPER);

      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_RESTORESPDPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_SPRESTORESPDPER);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_MOVESPDPER);

      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_STR);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_INT);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_AGI);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_DEX);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_VIT);
      m_stAttrCFG.vecShowAttrs.push_back(EATTRTYPE_LUK);
    }
    else if (m->first == "Delay")
    {
      m_stDelayCFG.dwWantedQuest = m->second.getTableInt("wantedquestmsg");
      m_stDelayCFG.dwExchangeCard = m->second.getTableInt("card_exchange");
      m_stDelayCFG.dwLottery = m->second.getTableInt("lottery");
      m_stDelayCFG.dwGuildTreasure = m->second.getTableInt("guild_treasure");
    }
    else if (m->first == "MonsterBodyDisappear")
    {
      m_stMonsterCFG.dwMonsterDisappearTime = m->second.getTableInt("Monster") / ONE_THOUSAND;
      m_stMonsterCFG.dwMiniDisappearTime = m->second.getTableInt("MINI") / ONE_THOUSAND;
      m_stMonsterCFG.dwMvpDisappearTime = m->second.getTableInt("MVP") / ONE_THOUSAND;
      m_stMonsterCFG.dwNpcDisappearTime = m->second.getTableInt("NPC") / ONE_THOUSAND;
    }
    else if (m->first == "ItemImage")
    {
      m_stItemImageCFG.dwRange = m->second.getTableInt("range");
      m_stItemImageCFG.dwNpcId = m->second.getTableInt("npcid");
      m_stItemImageCFG.dwLoveNpcId = m->second.getTableInt("lovenpcid");
    }
    else if (m->first == "Treasure")
    {
      m_stTreasureCFG.dwMaxGoldTree = m->second.getTableInt("gold_tree_max");
      m_stTreasureCFG.dwGoldTreeRefreshTime = m->second.getTableInt("gold_tree_refreshtime");
      m_stTreasureCFG.dwMaxMagicTree = m->second.getTableInt("magic_tree_max");
      m_stTreasureCFG.dwMagicTreeRefreshTime = m->second.getTableInt("magic_tree_refreshtime");
      m_stTreasureCFG.dwMaxHighTree = m->second.getTableInt("high_tree_max");
      m_stTreasureCFG.dwHighTreeRefreshTime = m->second.getTableInt("high_tree_refreshtime");

      m_stTreasureCFG.dwShakeActionNpc = m->second.getTableInt("shake_action_npc");
      m_stTreasureCFG.dwShakeActionMonster = m->second.getTableInt("shake_action_monster");
      m_stTreasureCFG.dwDisTime = m->second.getTableInt("dis_time");
      m_stTreasureCFG.dwKnownBuffID = m->second.getTableInt("known_buff");
    }
    else if (m->first == "Camera")
    {
      m_stCameraCFG.vecMonster.clear();
      xLuaData& summon = m->second.getMutableData("summon");
      auto func = [&](const string& key, xLuaData& data)
      {
        SCameraMonster sData;
        sData.dwOdds = data.getTableInt("odds");
        sData.dwDistance = data.getTableInt("dis");
        sData.dwID = data.getTableInt("id");
        sData.dwDayMaxCnt = data.getTableInt("day_maxcnt");
        sData.oParams = data;
        m_stCameraCFG.vecMonster.push_back(sData);
      };
      summon.foreach(func);
      m_stCameraCFG.dwInterval = m->second.getTableInt("interval");
    }
    else if (m->first == "Zone")
    {
      xLuaData& free = m->second.getMutableData("free");
      xLuaData& busy = m->second.getMutableData("busy");
      xLuaData& verybusy = m->second.getMutableData("verybusy");

      SZoneItem stItem;
      auto costf = [&](const string& key, xLuaData& data)
      {
        ItemInfo oItem;
        oItem.set_id(data.getTableInt("1"));
        oItem.set_count(data.getTableInt("2"));
        stItem.vecCost.push_back(oItem);
      };

      m_stZoneCFG.vecItems.clear();

      stItem.status.first = free.getTableInt("min");
      stItem.status.second = free.getTableInt("max");
      stItem.vecCost.clear();
      free.getMutableData("cost").foreach(costf);
      stItem.eStatus = EZONESTATUS_FREE;
      m_stZoneCFG.vecItems.push_back(stItem);

      stItem.status.first = busy.getTableInt("min");
      stItem.status.second = busy.getTableInt("max");
      stItem.vecCost.clear();
      busy.getMutableData("cost").foreach(costf);
      stItem.eStatus = EZONESTATUS_BUSY;
      m_stZoneCFG.vecItems.push_back(stItem);

      stItem.status.first = verybusy.getTableInt("min");
      stItem.status.second = verybusy.getTableInt("max");
      stItem.vecCost.clear();
      verybusy.getMutableData("cost").foreach(costf);
      stItem.eStatus = EZONESTATUS_VERYBUSY;
      m_stZoneCFG.vecItems.push_back(stItem);

      m_stZoneCFG.dwZoneStateRate = m->second.getTableInt("state");
      m_stZoneCFG.dwMaxRecent = m->second.getTableInt("maxrecent");
      m_stZoneCFG.dwJumpBaseLv = m->second.getTableInt("basejumplv");

      // user zone exchange
      xLuaData& user_zone = m->second.getMutableData("user_zone_exchange");
      m_stZoneCFG.dwActionQuest = user_zone.getTableInt("actionquest");

      // guild zone exchange
      xLuaData& guild_zone = m->second.getMutableData("guild_zone_exchange");
      m_stZoneCFG.dwGuildZoneTime = guild_zone.getTableInt("cd") * MIN_T;

      xLuaData& guild_cost = guild_zone.getMutableData("cost");
      m_stZoneCFG.vecGuildZoneCost.clear();
      auto guild_costf = [&](const string& key, xLuaData& data)
      {
        ItemInfo oItem;
        oItem.set_id(data.getTableInt("1"));
        oItem.set_count(data.getTableInt("2"));
        m_stZoneCFG.vecGuildZoneCost.push_back(oItem);
      };
      guild_cost.foreach(guild_costf);
    }
    else if (m->first == "QA")
    {
      m_stQACFG.rewardCount = m->second.getTableInt("reward_count");
      xLuaData& rData = m->second.getMutableData("reward_map");

      auto reward_mapf = [&](const string&key, xLuaData& data)
      {
        DWORD level = data.getTableInt("level");
        DWORD rewardid = data.getTableInt("rewardid");
        m_stQACFG.mapReward[level] = rewardid;
      };
      rData.foreach(reward_mapf);
    }
    else if (m->first == "FriendShip")
    {
      xLuaData& rData = m->second.getMutableData("Reward");
      auto reward_mapf = [&](const string&key, xLuaData& data)
      {
        SFriendShipReward reward;
        reward.dwItemId = data.getTableInt("Itemid");
        reward.dwMaxLimitCount = data.getTableInt("MaxLimitCount");
        reward.dwSealRewardCount = data.getTableInt("SealCount");
        reward.dwDojoRewardCount = data.getTableInt("DojoCount");
        reward.dwTowerMiniRewardCount = data.getTableInt("TowerCount_Mini");
        reward.dwTowerMvpRewardCount = data.getTableInt("TowerCount_Mvp");
        reward.dwLaboratoryRewardCount = data.getTableInt("LaboratoryCount");
        reward.dwWantedQuestRewardCount = data.getTableInt("WantedQuestCount");
        reward.dwQuestRewardCount = data.getTableInt("QuestCount");
        reward.dwGuildRaidCount = data.getTableInt("GuildRaid");
        reward.dwPveCardCount = data.getTableInt("PveCard");

        m_stFriendShipCFG.mapReward[key] = reward;
      };
      rData.foreach(reward_mapf);
    }
    else if (m->first == "DepositCard")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        SDepositTypeCFG cfg;
        cfg.type = static_cast<EDepositCardType>(data.getTableInt("type1"));
        cfg.name = data.getTableString("name");
        cfg.duration = data.getTableInt("duration");
        cfg.expandpackage = data.getTableInt("expandpackage");
        xLuaData rData = data.getMutableData("funcs");        
        auto func2 = [&](const string&key, xLuaData& data)
        {
          cfg.vecFuns.push_back(data.getInt());
        };
        rData.foreach(func2);
        m_mapDepositTypeCFG[cfg.type] = cfg;
      };
      m->second.foreach(func);
    }
    else if (m->first == "PVPConfig")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        SPvpCFG cfg;
        DWORD type = atoi(key.c_str());
        cfg.dwPeopleLImit = data.getTableInt("PeopleLimit");
        cfg.dwTeamLimit = data.getTableInt("TeamLimit");
        cfg.raidId = data.getTableInt("Raid");

        cfg.dwDuration = data.getTableInt("Time");
        cfg.dwMaxScore = data.getTableInt("MaxScore");

        xLuaData& rData = data.getMutableData("RaidMap");
        auto func2 = [&](const string&key, xLuaData& data)
        {
          DWORD raidId = data.getTableInt("1");
          DWORD count = data.getTableInt("2");
          cfg.mapRaid[raidId] = count;
        };
        rData.foreach(func2);

        m_mapPvpCFG[type] = cfg;
      };
      m->second.foreach(func);
    }
    else if (m->first == "Authorize")
    {
      m_stAuthorize.dwResetTime = m->second.getTableInt("ResetTime");
      m_stAuthorize.dwInputTimes = m->second.getTableInt("InputTimes");
      m_stAuthorize.dwInterval = m->second.getTableInt("Interval");
      m_stAuthorize.dwRefineLv = m->second.getTableInt("RefineLv");
    }
    else if (m->first == "EquipType")
    {
      auto equiptypef = [&](const string& key, xLuaData& data)
      {
        DWORD dwEquipType = atoi(key.c_str());
        if (dwEquipType <= EEQUIPTYPE_MIN || dwEquipType >= EEQUIPTYPE_MAX || EEquipType_IsValid(dwEquipType) == false)
        {
          bCorrect = false;
          XERR << "[GameConfig] EquipType key :" << dwEquipType << "不合法" << XEND;
          return;
        }

        EEquipType eEquipType = static_cast<EEquipType>(dwEquipType);
        auto posf = [&](const string& key, xLuaData& data)
        {
          DWORD dwEquipPos = data.getInt();
          if (dwEquipPos <= EEQUIPPOS_MIN || dwEquipPos >= EEQUIPPOS_MAX || EEquipPos_IsValid(dwEquipPos) == false)
          {
            bCorrect = false;
            XERR << "[GameConfig] EquipType key :" << dwEquipType << "pos :" << dwEquipPos << "不合法" << XEND;
            return;
          }
          m_stItemCFG.mapEquipType2Pos[eEquipType].insert(static_cast<EEquipPos>(dwEquipPos));
        };
        data.getMutableData("site").foreach(posf);
      };
      m->second.foreach(equiptypef);
    }
    else if (m->first == "MapTrans")
    {
      m_stMapTransCFG.fHandRate = m->second.getTableFloat("HandRate");
    }
    else if (m->first == "Augury")
    {
      m_stAugury.dwItemId = m->second.getTableInt("ItemId");
      m_stAugury.dwMaxRewardCountPerDay = m->second.getTableInt("MaxRewardCountPerDay");
      m_stAugury.dwHandRewardTime = m->second.getTableInt("HandRewardTime");
      m_stAugury.dwHandTipSysId = m->second.getTableInt("HandTipSysId");
      m_stAugury.dwRewardTipSysId = m->second.getTableInt("RewardTipSysId");
      m_stAugury.dwHandTipTime = m->second.getTableInt("HandTipTime");
      m_stAugury.fRange = m->second.getTableFloat("Range");
      m_stAugury.dwMaxRewardCount = m->second.getTableFloat("MaxRewardCount");
      m_stAugury.dwExtraItemId = m->second.getTableFloat("ExtraItemId");      

      parseTime(m->second.getTableString("StartTime"), m_stAugury.dwStartTime);
      parseTime(m->second.getTableString("EndTime"), m_stAugury.dwEndTime);

      auto typeConfig = [&](const string& key, xLuaData& data)
      {
        DWORD type = atoi(key.c_str());
        SAuguryTypeCFG& typeCfg = m_stAugury.m_typeConfig[type];
        typeCfg.dwNpcId = data.getTableInt("npcid");
        typeCfg.strTableName = data.getTableString("tbname");
      };
      m->second.getMutableData("Config").foreach(typeConfig);
    }
    else if (m->first == "GuildRaid")
    {
      m_stGuildRaidCFG.clear();

      auto func = [&](const string& key, xLuaData& data) {
        SGuildRaid& raid = m_stGuildRaidCFG.mapNpcID2GuildRaid[data.getTableInt("NpcID")];
        raid.dwNpcID = data.getTableInt("NpcID");
        raid.dwGuildLevel = data.getTableInt("GuildLevel");
        raid.bSpecial = data.getTableInt("Special");
        raid.dwUserLevel = data.getTableInt("UserLevel");

        auto funclevel = [&](const string& key, xLuaData& data) {
          raid.setLevel.insert(data.getInt());
        };
        data.getMutableData("Level").foreach(funclevel);

        auto funcunlock = [&](const string& key, xLuaData& data) {
          pair<DWORD, DWORD> item;
          item.first = data.getTableInt("1");
          item.second = data.getTableInt("2");
          raid.vecUnlockItem.push_back(item);
        };
        data.getMutableData("UnlockItem").foreach(funcunlock);

        auto funcopen = [&](const string& key, xLuaData& data) {
          pair<DWORD, DWORD> item;
          item.first = data.getTableInt("1");
          item.second = data.getTableInt("2");
          raid.vecOpenItem.push_back(item);
        };
        data.getMutableData("OpenItem").foreach(funcopen);
      };
      m->second.foreach(func);

      // 生成副本组关联的npc和等级
      DWORD group = 0;
      for (auto &it : m_stGuildRaidCFG.mapNpcID2GuildRaid)
      {
        for (auto &s : it.second.setLevel)
        {
          ++ group;
          pair<DWORD, DWORD>& pa = m_stGuildRaidCFG.mapGroup2NpcAndLv[group];
          pa.first = it.first; //npcid
          pa.second = s; //lv
        }
      }
    }
    else if (m->first == "SafeRefineEquipCost")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        DWORD refinelv = atoi(key.c_str());
        SSafeRefineCost cost;
        cost.normal_cost = data.getTableInt("1");
        cost.discount_cost = data.getTableInt("2");
        m_mapSafeRefineCFG.insert(std::make_pair(refinelv, cost));
      };
      m->second.foreach(func);
    }
    else if (m->first == "SafeRefineEquipCostLottery")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        DWORD refinelv = atoi(key.c_str());
        DWORD count = data.getInt();
        m_mapSafeRefineLotteryCFG.insert(std::make_pair(refinelv, count));
      };
      m->second.foreach(func);
    }
    else if (m->first == "Astrolabe")
    {
      auto funccost = [&](const string& key, xLuaData& data) {
        ItemInfo item;
        item.set_id(data.getTableInt("1"));
        item.set_count(data.getTableInt("2"));
        m_stAstrolabeCFG.vecResetCost.push_back(item);
      };
      m->second.getMutableData("ResetCost").foreach(funccost);

      if (m->second.has("ResetRate"))
        m_stAstrolabeCFG.fResetRate = m->second.getTableFloat("ResetRate");
      else {
        XERR << "[GameConfig] Astrolabe ResetRate 未配置" << XEND;
        bCorrect = false;
      }

      auto funccostlimit = [&](const string& key, xLuaData& data) {
        m_stAstrolabeCFG.mapResetCostLimit[data.getTableInt("1")] = data.getTableInt("2");
      };
      m->second.getMutableData("ResetCostLimit").foreach(funccostlimit);

      if (m->second.has("InitStar")) {
        auto func = [&](const string& key, xLuaData& data) {
          DWORD type = atoi(key.c_str());
          if (type <= EASTROLABETYPE_MIN || type >= EASTROLABETYPE_MAX || !EAstrolabeType_IsValid(type)) {
            XERR << "[GameConfig] Astrolabe InitStar 类型:" << type << "非法" << XEND;
            bCorrect = false;
            return;
          }
          pair<DWORD, DWORD> v;
          v.first = data.getInt() / 10000;
          v.second = data.getInt() % 10000;
          m_stAstrolabeCFG.mapInitStar[static_cast<EAstrolabeType>(type)] = v;
        };
        m->second.getMutableData("InitStar").foreach(func);
      } else {
        XERR << "[GameConfig] Astrolabe InitStar 未配置" << XEND;
        bCorrect = false;
      }

      if (m->second.has("Default_TypeBranch")) {
        auto func = [&](const string& key, xLuaData& data) {
          DWORD type = atoi(key.c_str());
          DWORD typebranch = data.getInt();
          if (type <= 0 || typebranch <= 0) {
            XERR << "[GameConfig] Astrolabe Default_TypeBranch配置错误" << type << typebranch << XEND;
            bCorrect = false;
            return;
          }
          m_stAstrolabeCFG.mapDefaultTypeBranch[type] = typebranch;
        };
        m->second.getMutableData("Default_TypeBranch").foreach(func);
      } else {
        XERR << "[GameConfig] Astrolabe Default_TypeBranch 未配置" << XEND;
      }

      if (m->second.has("MenuID"))
        m_stAstrolabeCFG.dwMenuId = m->second.getTableInt("MenuID");
      else
        XERR << "[GameConfig] Astrolabe MenuID 未配置" << XEND;
    }
    else if (m->first == "GetLimitMsg")
    {
      auto func = [&](const string& key, xLuaData& data) {
        m_mapGetLimitMsg[static_cast<EItemGetLimitType>(atoi(key.c_str()))] = data.getInt();
      };
      m->second.foreach(func);
    }
    else if(m->first == "MontchCardActivity")
    {
      m_stMonthCardActivityCfg.dwID = m->second.getTableInt("ActivityID");
      auto func = [&](const string&key, xLuaData& data)
      {
        SCelebrationMCardCFG cfg;
        DWORD type = atoi(key.c_str());
        cfg.dwCostCard = data.getTableInt("CostCard");
        cfg.dwRewardID = data.getTableInt("RewardItemId");
        cfg.dwCount = data.getTableInt("Count");

        m_stMonthCardActivityCfg.m_mapCelebrationMCardCfg[static_cast<ECelebrationLevel>(type)] = cfg;
      };
      m->second.getMutableData("MontchCardReward").foreach(func);
    }
    else if (m->first == "Photo")
    {
      m_stUserPhotoCFG.dwDefaultSize = m->second.getTableInt("DefaultSize");
      auto f = [&](const string& key, xLuaData& data)
        {
          pair<DWORD, DWORD> p;
          p.first = atoi(key.c_str());
          p.second = data.getInt();
          m_stUserPhotoCFG.vecSkillIncreaseSize.push_back(p);
        };
      m->second.getMutableData("SkillIncreaseSize").foreach(f);
    }
    else if (m->first == "PetAdventureMinLimit")
    {
      m_stPetCFG.dwPetAdventureFriendReq = m->second.getTableInt("limit_friendly_lv");
      m_stPetCFG.dwPetAdventureMaxCount = m->second.getTableInt("max_adventure");
    }
    else if (m->first == "Pet")
    {
      m_stSystemCFG.dwPetNameMinSize = m->second.getTableInt("petname_min");
      m_stSystemCFG.dwPetNameMaxSize = m->second.getTableInt("petname_max");

      m_stPetCFG.dwUserPetGiftReqValue = m->second.getTableInt("userpet_gift_reqvalue");
      m_stPetCFG.dwHandReqFriendLv = m->second.getTableInt("Hug_LimitFriendLv");
      m_stPetCFG.dwMaxCatchCartoonTime = m->second.getTableInt("maxCatchCartoonTime");

      m_stPetCFG.setCancelEffectsID.clear();
      auto effectsfunc = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.setCancelEffectsID.insert(data.getInt());
      };
      m->second.getMutableData("cancel_special_effects").foreach(effectsfunc);
    }
    else if (m->first == "PetWorkSpace")
    {
      xLuaData& rUnlock = m->second.getMutableData("pet_work_manual_unlock");
      m_stPetCFG.dwWorkManualFriendLv = rUnlock.getTableInt("friendlv");
      m_stPetCFG.dwWorkManualBaseLv = rUnlock.getTableInt("baselv");
      m_stPetCFG.dwWorkMaxContinueDay = m->second.getTableInt("pet_work_continue_day");
      m_stPetCFG.dwWorkMaxExchange = m->second.getTableInt("pet_work_max_exchange");
      m_stPetCFG.dwWorkMaxWorkCount = m->second.getTableInt("pet_work_max_workcount");

      xLuaData& skill = m->second.getMutableData("pet_work_skill_extra");
      auto skillf = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.mapSkillWork[atoi(key.c_str())] = data.getInt();
      };
      skill.foreach(skillf);
    }
    else if (m->first == "Food")
    {
      m_stFood.dwMaxCookFoodLv = m->second.getTableInt("MaxCookFoodLv");
      m_stFood.dwMaxTasterFoodLv = m->second.getTableInt("MaxTasterFoodLv");
      m_stFood.dwMaxLastCooked = m->second.getTableInt("MaxLastCooked");
      m_stFood.dwDefaultSatiey = m->second.getTableInt("MaxSatiety_Default");
      m_stFood.dwDefaultLimitFood = m->second.getTableInt("MaxLimitFood_Default");
      m_stFood.dwCookFoodExpPerLv = m->second.getTableInt("CookFoodExpPerLv");

      auto funskill = [this](const string& key, xLuaData& data)
      {
        m_stFood.vecEatSkill.push_back(data.getInt());
      };
      xLuaData& eatSkills = m->second.getMutableData("eat_skills");
      eatSkills.foreach(funskill);
    }
    else if (m->first == "Lottery")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        DWORD t = atoi(key.c_str());
        std::pair<DWORD, DWORD> pr;
        pr.first = data.getTableInt("itemid");
        pr.second = data.getTableInt("count");
        m_stLotteryCFG.m_mapTicket[t] = pr;
      };
      xLuaData& rData = m->second.getMutableData("Ticket");
      rData.foreach(func);

      m_stLotteryCFG.dwSendPrice = m->second.getTableInt("SendPrice");
      m_stLotteryCFG.dwDiscountPrice = m->second.getTableInt("DiscountPrice");
      m_stLotteryCFG.dwExpireTime = m->second.getTableInt("SendExpireTime");
      m_stLotteryCFG.dwLoveLeterItemId = m->second.getTableInt("LoveLetterItemId");

      xLuaData& enchantTrans = m->second.getMutableData("TransferCost");
      m_stLotteryCFG.oEnchantTrans.set_id(enchantTrans.getTableInt("itemid"));
      m_stLotteryCFG.oEnchantTrans.set_count(enchantTrans.getTableInt("num"));

      map<DWORD, SLotteryRepair>& mapRepairItem = m_stLotteryCFG.mapRepairItem;
      mapRepairItem.clear();

      auto repairf = [&](const string& key, xLuaData& data)
      {
        DWORD dwItemID = atoi(key.c_str());
        SLotteryRepair& rRepair = mapRepairItem[dwItemID];

        auto funcf = [&](const string& k, xLuaData& d)
        {
          DWORD dwQuality = d.getTableInt("quality");
          if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
          {
            XERR << "[GameConfig] Lottery -> repair_material -> itemid :" << dwItemID << "quality :" << dwQuality << "不合法" << XEND;
            bCorrect = false;
            return;
          }

          rRepair.mapQualityCount[static_cast<EQualityType>(dwQuality)] = d.getTableInt("count");
        };
        data.foreach(funcf);
      };
      m->second.getMutableData("repair_material").foreach(repairf);
    }
    else if (m->first == "Tutor")
    {
      m_stTutorCFG.dwStudentBaseLvReq = m->second.getTableInt("student_baselv_req");
      m_stTutorCFG.dwTutorBaseLvReq = m->second.getTableInt("tutor_baselv_req");

      m_stTutorCFG.dwMaxTutorApply = m->second.getTableInt("max_tutor_apply");
      m_stTutorCFG.dwMaxStudent = m->second.getTableInt("max_student");

      m_stTutorCFG.dwProtectTime = m->second.getTableInt("protecttime") * HOUR_T;
      m_stTutorCFG.dwForbidTime = m->second.getTableInt("forbidtime") * HOUR_T;

      m_stTutorCFG.dwStudentGraduationTime = m->second.getTableInt("student_graduation_time") * DAY_T;
      m_stTutorCFG.dwApplyRefuseProtect = m->second.getTableInt("refuse_same_interval");
      m_stTutorCFG.dwApplyOverTime = m->second.getTableInt("apply_overtime");

      m_stTutorCFG.dwMaxProfrociency = m->second.getTableInt("max_proficiency");
      m_stTutorCFG.dwStudentGraduationReward = m->second.getTableInt("student_graduation_reward");
      m_stTutorCFG.dwTeacherGraduationReward = m->second.getTableInt("teacher_graduation_reward");
      m_stTutorCFG.dwTutorMenuID = m->second.getTableInt("tutor_menuid");
      m_stTutorCFG.dwTeacherTaskMailID = m->second.getTableInt("teacher_task_mailid");
      m_stTutorCFG.dwTeacherGrowMailID = m->second.getTableInt("teacher_grow_mailid");
      m_stTutorCFG.dwTeacherGraduationMailID = m->second.getTableInt("teacher_graduation_mailid");

      m_stTutorCFG.fTutorExtraBattleTimePer = 1.0f * m->second.getTableInt("tutor_vip_battletime_per") / TEN_THOUSAND;
      m_stTutorCFG.dwTutorExtraMaxBattleTime = m->second.getTableInt("tutor_vip_max_battletime");
    }
    else if (m->first == "Auction")
    {
      m_stAuctionMiscCFG.dwDurationPerOrder = m->second.getTableInt("DurationPerOrder");
      m_stAuctionMiscCFG.dwRecordPageCnt = m->second.getTableInt("RecordPerPageMaxCount");
      if (m_stAuctionMiscCFG.dwRecordPageCnt == 0)
      {
        XERR << "[MiscConfig] Auction 拍卖记录每页显示大小为0，没有配置 RecordPerPageMaxCount" <<  XEND;
        bCorrect = false;
        m_stAuctionMiscCFG.dwRecordPageCnt = 10;
      }
      m_stAuctionMiscCFG.dwFlowingWaterMaxCount = m->second.getTableInt("FlowingWaterMaxCount");
      if (m_stAuctionMiscCFG.dwFlowingWaterMaxCount == 0)
      {
        XERR << "[MiscConfig] Auction 拍卖记录每页显示大小为0，没有配置 FlowingWaterMaxCount" << XEND;
        bCorrect = false;
        m_stAuctionMiscCFG.dwFlowingWaterMaxCount = 50;
      }    
      m_stAuctionMiscCFG.fRate = m->second.getTableFloat("Rate");
      m_stAuctionMiscCFG.dwNpcId = m->second.getTableInt("AuctionNpc");
      m_stAuctionMiscCFG.dwReceiveTime = m->second.getTableInt("ReceiveTime");
      m_stAuctionMiscCFG.qwMaxPrice = m->second.getTableInt("MaxPrice");
      m_stAuctionMiscCFG.dwEnchantAttrCount = m->second.getTableInt("EnchantAttrCount");
      m_stAuctionMiscCFG.dwEnchantAttrValuableCount = m->second.getTableInt("EnchantAttrValuableCount");
      m_stAuctionMiscCFG.dwEnchantBuffExtraCount = m->second.getTableInt("EnchantBuffExtraCount");

      if (m->second.has("MaskPrice")) {
        auto func = [&](const string& key, xLuaData& data) {
          DWORD index = atoi(key.c_str());
          DWORD time = data.getInt();
          if (index <= 0 || time <= 0) {
            XERR << "[GameConfig] Auction MaskPrice配置错误" << index << time << XEND;
            bCorrect = false;
            return;
          }
          m_stAuctionMiscCFG.m_mapMaskPrice[index] = time;
        };
        m->second.getMutableData("MaskPrice").foreach(func);
      } else {
        XERR << "[GameConfig] Auciton MaskPrice 未配置" << XEND;
      }

    }
    else if(m->first == "PlayerRename")
    {
      m_stPlayerRenameCFG.dwRenameItemId = m->second.getTableInt("rename_item");
      m_stPlayerRenameCFG.dwRenameCoolDown = m->second.getTableInt("rename_cooldown");
    }
    else if (m->first == "BranchForbid")
    {
      m_stBranchForbidCFG.mapForbids.clear();
      char tb[100] = {};
      auto f = [&](const string& key, xLuaData& data)
      {
        for (auto v = data.m_table.begin(); v != data.m_table.end(); ++v)
          for (auto n = v->second.m_table.begin(); n != v->second.m_table.end(); ++n)
          {
            //Table_n->first  Table_Map
            bzero(tb, sizeof(tb));
            snprintf(tb, sizeof(tb), "Table_%s", v->first.c_str());
            m_stBranchForbidCFG.mapForbids[tb][atoi(n->first.c_str())] = n->second.getInt();
          }
      };
      m->second.foreach(f);
    }
    else if (m->first == "GVGConfig")
    {
      m_stGuildFireCFG.setWeekStartTime.clear();
      m_stGuildFireCFG.stSuperGvgCFG.setSuperGvgDays.clear();
      auto gettime = [&](const string& key, xLuaData& d)
      {
        DWORD day = d.getTableInt("day");
        if (day == 0)
        {
          XERR << "[MiscConfig], 公会战配置时间错误, day=0" << XEND;
          bCorrect = false;
          day = 1;
        }
        DWORD hour = d.getTableInt("hour");
        DWORD min = d.getTableInt("min");
        DWORD weekdistance = (day - 1) * 86400 + hour * 3600 + min * 60;
        m_stGuildFireCFG.setWeekStartTime.insert(weekdistance);
        if (d.getTableInt("super") == 1)
          m_stGuildFireCFG.stSuperGvgCFG.setSuperGvgDays.insert(day);
      };
      xLuaData& starttime = m->second.getMutableData("start_time");
      starttime.foreach(gettime);

      m_stGuildFireCFG.dwLastTime = m->second.getTableInt("last_time");
      if (m_stGuildFireCFG.setWeekStartTime.empty())
      {
        XERR << "[MisConfig], 公会战没有配置开启时间" << XEND;
        bCorrect = false;
      }

      m_stGuildFireCFG.dwDangerTime = m->second.getTableInt("danger_time");
      m_stGuildFireCFG.dwDangerSuccessTime= m->second.getTableInt("danger_success_time");

      /*
      m_stGuildFireCFG.vecSpecReward.clear();
      m_stGuildFireCFG.vecPartInReward.clear();
      m_stGuildFireCFG.vecWinGuildReward.clear();

      auto getrwd = [&](TVecItemInfo& items, xLuaData& data)
      {
        items.clear();
        auto getitem = [&](const string& key, xLuaData& d)
        {
          ItemInfo item;
          item.set_id(d.getTableInt("itemid"));
          item.set_count(d.getTableInt("num"));
          items.push_back(item);
        };
        data.foreach(getitem);
      };
      getrwd(m_stGuildFireCFG.vecPartInReward, m->second.getMutableData("partin_reward"));
      getrwd(m_stGuildFireCFG.vecSpecReward, m->second.getMutableData("spec_reward"));
      getrwd(m_stGuildFireCFG.vecWinGuildReward, m->second.getMutableData("win_guild_reward"));
      */

      m_stGuildFireCFG.dwPartInMailID = m->second.getTableInt("partin_mail");
      m_stGuildFireCFG.dwSpecMailID = m->second.getTableInt("spec_mail");
      m_stGuildFireCFG.dwWinGuildMailID = m->second.getTableInt("win_guild_mail");
      m_stGuildFireCFG.dwExtraMailID = m->second.getTableInt("extra_reward_mail");
      m_stGuildFireCFG.dwExtraMsgID = m->second.getTableInt("extra_reward_msg");

      m_stGuildFireCFG.dwPartInBuffID = m->second.getTableInt("partin_buff");
      auto getmap =[&](const string& k, xLuaData& d)
      {
        m_stGuildFireCFG.setMsgMap.insert(d.getInt());
      };
      m->second.getMutableData("msg_map").foreach(getmap);

      m_stGuildFireCFG.dwExpelSkill = m->second.getTableInt("expel_skill");

      m_stGuildFireCFG.time2ReliveCD.clear();
      xLuaData& diepunish = m->second.getMutableData("die_punish");

      auto getcd = [&](const string& k, xLuaData& d)
      {
        xLuaData& interd = d.getMutableData("interval");
        pair<DWORD, DWORD> pa;
        pa.first = interd.getTableInt("1");
        pa.second = interd.getTableInt("2");

        DWORD cd = d.getTableInt("cd");
        m_stGuildFireCFG.time2ReliveCD.push_back(make_pair(cd, pa));
      };
      diepunish.foreach(getcd);
      m_stGuildFireCFG.dwResetCDTime = m->second.getTableInt("die_punish_reset");
      m_stGuildFireCFG.dwMaxCD = m->second.getTableInt("die_max_cd");
      m_stGuildFireCFG.dwHpRate = m->second.getTableInt("hp_rate");
      m_stGuildFireCFG.dwAttExpelTime = m->second.getTableInt("att_expel_time");
      m_stGuildFireCFG.dwDefExpelTime = m->second.getTableInt("def_expel_time");

      auto gettimesrwd = [&](TMapGvgTimes2RewardData& mapdatas, xLuaData& luadata)
      {
        mapdatas.clear();
        auto rwd = [&](const string& k, xLuaData& d)
        {
          if (d.has("times") == false)
            return;
          SGvgRewardData sdata;
          sdata.dwNeedTimes = d.getTableInt("times");
          sdata.dwItemID = d.getTableInt("itemid");
          sdata.dwItemCnt = d.getTableInt("count");
          mapdatas[sdata.dwNeedTimes] = sdata;
        };
        luadata.foreach(rwd);
      };

      xLuaData& rewarddata = m->second.getMutableData("reward");
      gettimesrwd(m_stGuildFireCFG.mapPartinTimeReward, rewarddata.getMutableData("partin_time"));
      gettimesrwd(m_stGuildFireCFG.mapKillMonsterReward, rewarddata.getMutableData("kill_monster"));
      gettimesrwd(m_stGuildFireCFG.mapReliveUserReward, rewarddata.getMutableData("relive_other"));
      gettimesrwd(m_stGuildFireCFG.mapExpelReward, rewarddata.getMutableData("expel_enemy"));
      gettimesrwd(m_stGuildFireCFG.mapDamMetalReward, rewarddata.getMutableData("dam_metal"));
      gettimesrwd(m_stGuildFireCFG.mapKillMetalReward, rewarddata.getMutableData("kill_metal"));
      gettimesrwd(m_stGuildFireCFG.mapGetHonorReward, rewarddata.getMutableData("get_honor"));

      m_stGuildFireCFG.dwKillUserItem = rewarddata.getMutableData("kill_one_user").getTableInt("itemid");
      m_stGuildFireCFG.dwKillUserItemCnt = rewarddata.getMutableData("kill_one_user").getTableInt("count");

      m_stGuildFireCFG.dwPartInTime = rewarddata.getTableInt("valid_partin_time");
      m_stGuildFireCFG.dwMaxHonorCount = rewarddata.getTableInt("max_honor");

      m_stGuildFireCFG.dwSafeBuffID = m->second.getTableInt("safe_area_buff");
      m_stGuildFireCFG.dwKillUserTeamGetNum = m->second.getTableInt("kill_team_get_num");

      m_stGuildFireCFG.dwMinShowNum = m->second.getTableInt("min_show_num");
      m_stGuildFireCFG.fCriPer = m->second.getTableFloat("show_cri_per");
      m_stGuildFireCFG.fNormalPer = m->second.getTableFloat("show_normal_per");
    }
    else if (m->first == "GvgDroiyan")
    {
      SSuperGvgCFG& cfg = m_stGuildFireCFG.stSuperGvgCFG;
      cfg.dwMetalGodBuff = m->second.getTableInt("MetalGodBuff");
      cfg.dwMetalGodTime = m->second.getTableInt("MetalGodTime");
      cfg.setSouthBossUids.clear();
      cfg.setNorthBossUids.clear();
      cfg.setBossIDs.clear();
      m->second.getMutableData("SouthBossUids").getIDList(cfg.setSouthBossUids);
      m->second.getMutableData("NorthBossUids").getIDList(cfg.setNorthBossUids);
      m->second.getMutableData("BossIDs").getIDList(cfg.setBossIDs);

      cfg.dwSuperGvgBeginTime = m->second.getTableInt("BeginTime");
      cfg.dwNoticeTime = m->second.getTableInt("NoticeTime");
      cfg.dwMinBossInterval = m->second.getTableInt("MinBossInterval");
      cfg.dwMaxBossInterval = m->second.getTableInt("MaxBossInterval");
      cfg.dwBuffEffectInterval = m->second.getTableInt("BuffEffectInterval");
      cfg.dwFirstBossTime = m->second.getTableInt("FirstBossTime");
      cfg.dwWinMetalNum = m->second.getTableInt("WinNeedCrystalNum");
      cfg.dwComposeChipNum = m->second.getTableInt("ComposeChipNum");
      cfg.dwUserRewardMail = m->second.getTableInt("RewardUserMail");
      cfg.dwPartinRewardTime = m->second.getTableInt("TakePartInTime");

      DWORD maxvalue = m->second.getTableInt("RobPlatform_RobValue");

      auto sortmax = [&](vector<pair<DWORD, float>>& vec)
      {
        std::sort(vec.begin(), vec.end(), [&](const pair<DWORD, float>& d1, const pair<DWORD, float>& d2) -> bool
        {
          return d1.first > d2.first;
        });
      };
      cfg.mapTowerCFG.clear();
      auto gettower = [&](const string& k, xLuaData& d)
      {
        DWORD dtype = atoi(k.c_str());
        EGvgTowerType etype = static_cast<EGvgTowerType>(dtype);
        if (EGvgTowerType_IsValid(etype) == false)
          return;
        SGvgTowerCFG& tdata = cfg.mapTowerCFG[etype];
        tdata.eType = etype;
        tdata.dwAllValue = maxvalue;
        tdata.dwBaseValue = d.getTableInt("base_speed");
        tdata.dwShowNpcID = d.getTableInt("show_npc");
        tdata.dwCrystalNum = d.getTableInt("crystal_num");
        tdata.fRange = d.getTableFloat("occupy_range");
        xPos& p = tdata.pos;
        xLuaData& dpos = d.getMutableData("pos");
        p.x = dpos.getTableFloat("1");
        p.y = dpos.getTableFloat("2");
        p.z = dpos.getTableFloat("3");
        tdata.strTowerName = d.getTableString("name");
      };
      m->second.getMutableData("RobPlatform").foreach(gettower);

      auto gnumspd = [&](const string& k1, xLuaData& d1)
      {
        DWORD num = d1.getTableInt("guild_num");
        float spd = d1.getTableFloat("speed_per");
        for (auto &m : cfg.mapTowerCFG)
        {
          m.second.vecGuildNum2Spd.push_back(make_pair(num, spd));
        }
      };
      m->second.getMutableData("GuildNumToSpeed").foreach(gnumspd);

      auto unumspd = [&](const string& k1, xLuaData& d1)
      {
        DWORD num = d1.getTableInt("user_num");
        float spd = d1.getTableFloat("speed_per");
        for (auto &m : cfg.mapTowerCFG)
        {
          m.second.vecUserNum2Spd.push_back(make_pair(num, spd));
        }
      };
      m->second.getMutableData("UserNumToSpeed").foreach(unumspd);
      for (auto &m : cfg.mapTowerCFG)
      {
        sortmax(m.second.vecUserNum2Spd);
        sortmax(m.second.vecGuildNum2Spd);
      }

      cfg.mapCampCFG.clear();
      auto getcamp = [&](const string& k, xLuaData& d)
      {
        DWORD color = atoi(k.c_str());
        if (color == 0)
          return;
        SGvgCampCFG& cdata = cfg.mapCampCFG[color];
        cdata.dwColor = color;
        cdata.dwMetalUniqID = d.getTableInt("metal_uniqueid");
        cdata.dwGearUniqID = d.getTableInt("barrier_uniqueid");
        cdata.dwBpID = d.getTableInt("bp_point");
        cdata.dwNpcShowActionID = d.getTableInt("show_actionid");
        cdata.strColorName = d.getTableString("color_name");
      };
      m->second.getMutableData("CampInfo").foreach(getcamp);

      cfg.setNorthSceneEffectIDs.clear();
      m->second.getMutableData("NorthSceneEffectIDs").getIDList(cfg.setNorthSceneEffectIDs);
      cfg.setSouthSceneEffectIDs.clear();
      m->second.getMutableData("SouthSceneEffectIDs").getIDList(cfg.setSouthSceneEffectIDs);
      cfg.dwRaidID = m->second.getTableInt("RaidID");

      // 奖励
      cfg.vecBaseGuildReward.clear();
      cfg.vecBaseUserReward.clear();
      cfg.vecUserStableReward.clear();
      DWORD type = 1;
      auto getitem = [&](const string&k, xLuaData& d)
      {
        ItemInfo item;
        item.set_id(d.getTableInt("itemid"));
        item.set_count(d.getTableInt("count"));
        if (type == 1)
          cfg.vecBaseGuildReward.push_back(item);
        else if (type == 2)
          cfg.vecBaseUserReward.push_back(item);
        else if (type == 3)
          cfg.vecUserStableReward.push_back(item);
      };
      type = 1;
      m->second.getMutableData("GuildReward").foreach(getitem);
      type =2;
      m->second.getMutableData("UserReward").foreach(getitem);
      type = 3;
      m->second.getMutableData("UserStableReward").foreach(getitem);

      cfg.vecRewardPerCFG.clear();
      auto getrewardper = [&](const string& k, xLuaData& d)
      {
        SSuGvgRewardCFG v;
        v.fMinScore = d.getMutableData("win_rate").getTableInt("1");
        v.fMaxScore = d.getMutableData("win_rate").getTableInt("2");
        v.dwLevel = atoi(k.c_str());
        auto getrank = [&](const string& k1, xLuaData& d1)
        {
          DWORD rank = d1.getTableInt("rank");
          pair<float, float>& pa = v.mapRank2RewardPer[rank];
          pa.first = d1.getTableFloat("guild_reward_per");
          pa.second = d1.getTableFloat("user_reward_per");
        };
        d.getMutableData("rank").foreach(getrank);
        v.strLvName = d.getTableString("LvDesc");
        cfg.vecRewardPerCFG.push_back(v);
      };
      m->second.getMutableData("GvgDroiyanReward").foreach(getrewardper);

      cfg.vecRankScore.clear();
      m->second.getMutableData("RankScore").getIDList(cfg.vecRankScore);
      cfg.dwNpcDefaultActionID = m->second.getTableInt("DefaultShowActionID");
      cfg.dwMetalDieExpelTime = m->second.getTableInt("MetalDieExpelTime");
      cfg.dwExpelSkill = m->second.getTableInt("ExpelSkill");
      cfg.dwMatchToEnterTime = m->second.getTableInt("MatchToEnterTime");
      cfg.dwMatchToFireTime = m->second.getTableInt("MatchToFireTime");
      cfg.dwTowerOpenTime = m->second.getTableInt("TowerOpenTime");
      cfg.dwItemDispTime = m->second.getTableInt("ItemDispTime");
      cfg.dwOneSecJoinUserNum = m->second.getTableInt("OneSecJoinUserNum");
      cfg.vecPreLimitArea.clear();
      auto getarea = [&](const string& key, xLuaData& d)
      {
        pair<xPos, float> pa;
        pa.first.x = d.getMutableData("pos").getTableFloat("1");
        pa.first.y = d.getMutableData("pos").getTableFloat("2");
        pa.first.z = d.getMutableData("pos").getTableFloat("3");
        pa.second = d.getTableFloat("radius");
        cfg.vecPreLimitArea.push_back(pa);
      };
      m->second.getMutableData("PrepareLimitArea").foreach(getarea);
      cfg.dwExtraMailID = m->second.getTableInt("ExtraRewardMail");
      cfg.dwExtraMsgID = m->second.getTableInt("ExtraRewardMsg");
    }
    else if (m->first == "Map_BranchForbid")
    {
      m_stMapBranchForbidCFG.mapNextMapID2Forbid.clear();
      auto getinfo = [&](const string& k, xLuaData& d)
      {
        int nextmapid = atoi(k.c_str());
        DWORD branch = d.getInt();
        m_stMapBranchForbidCFG.mapNextMapID2Forbid[nextmapid] = branch;
      };
      m->second.getMutableData("gvgmap").foreach(getinfo);
    }
    else if (m->first == "SystemForbid")
    {
      DWORD dwLimit = m->second.getTableInt("Limit");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_LIMIT, dwLimit));
      DWORD dwTutor = m->second.getTableInt("Tutor");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_TUTOR, dwTutor));
      DWORD dwGVG = m->second.getTableInt("GVG");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_GVG, dwGVG));
      DWORD dwPeak = m->second.getTableInt("Peak");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_PEAK, dwPeak));
      DWORD dwPray = m->second.getTableInt("GvGPvP_Pray");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_PRAY, dwPray));
      DWORD dwAuction = m->second.getTableInt("Auction");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_AUCTION, dwAuction));
      DWORD dwSend = m->second.getTableInt("Send");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_SEND, dwSend));
      DWORD dwPve = m->second.getTableInt("FashionEquipEnchant");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_PVE, dwPve));
      DWORD dwCareer = m->second.getTableInt("MultiProfession");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_MULTI_CAREER, dwCareer));
      DWORD dwBooth = m->second.getTableInt("Booth");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_BOOTH, dwBooth));
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_DEAD, m->second.getTableInt("Dead")));
      DWORD dwExchangeShop = m->second.getTableInt("ExchangeShop");
      m_mapSystemsForbidCFG.insert(std::make_pair(ESYSTEM_FORBID_EXCHANGESHOP, dwExchangeShop));
    }
    else if (m->first == "AutoSkillGroup")
    {
      m_stAutoSkillGrpCFG.vecAutoSkillGroup.clear();
      auto getgrp = [&](xLuaData& d)
      {
        TVecDWORD vec;
        DWORD id = d.getTableInt("1");
        if (id) vec.push_back(id);
        id = d.getTableInt("2");
        if (id) vec.push_back(id);
        id = d.getTableInt("3");
        if (id) vec.push_back(id);
        id = d.getTableInt("4");
        if (id) vec.push_back(id);
        id = d.getTableInt("5");
        if (id) vec.push_back(id);

        m_stAutoSkillGrpCFG.vecAutoSkillGroup.push_back(vec);
      };
      getgrp(m->second.getMutableData("Monk_Combo"));
    }
    else if (m->first == "PoliFire")
    {   
      m_stPoliFireCFG.mapTransBuffId.clear();
      auto fTransBuff = [&](const string& k, xLuaData& d)
      {
        DWORD index = atoi(k.c_str());
        DWORD buffid = d.getTableInt("buff");
        m_stPoliFireCFG.mapTransBuffId[index] = buffid;
      };
      m->second.getMutableData("trans_buffid").foreach(fTransBuff);

      m_stPoliFireCFG.mapItem2Skill.clear();
      auto getskill = [&](const string& k, xLuaData& d)
      {
        DWORD item = atoi(k.c_str());
        DWORD skill = d.getInt();
        m_stPoliFireCFG.mapItem2Skill[item] = skill;
      };
      m->second.getMutableData("item_skill").foreach(getskill);
      m_stPoliFireCFG.dwActivityMap = m->second.getTableInt("ActivityMap");
      m_stPoliFireCFG.dwTransportNpcID = m->second.getTableInt("TransportNpcID");
      m_stPoliFireCFG.dwGreedyBuff = m->second.getTableInt("GreedyBuff");
      m_stPoliFireCFG.dwPickAppleCDMs = m->second.getTableInt("PickAppleCD");
      m_stPoliFireCFG.dwDropAppleCDMs = m->second.getTableInt("DropApplePickCD");
    }
    else if (m->first == "Being")
    {
      m_stBeingCFG.dwOffOwnerToBattleTime = m->second.getTableInt("leave_owner_time");
      m_stBeingCFG.dwBirthRange = m->second.getTableInt("birth_range");
      m_stBeingCFG.dwAutoSkillMax = m->second.getTableInt("auto_skill_max");

      auto skillorderf = [&](const string& key, xLuaData& data)
      {
        auto f = [&](const string& k, xLuaData& d)
        {
          if (d.getInt() > 0)
            m_stBeingCFG.mapID2SkillOrder[atoi(key.c_str())].push_back(d.getInt());
        };
        data.foreach(f);
      };
      m->second.getMutableData("reset_skill_order").foreach(skillorderf);
    }
    else if(m->first == "LoveLetter")
    {
      m_stLetterCFG.dwConstellation = m->second.getTableInt("StarId");
      m_stLetterCFG.dwChristmas = m->second.getTableInt("ChristmasId");
      m_stLetterCFG.dwChristmasReward = m->second.getTableInt("letter_christmas_reward");
      m_stLetterCFG.dwSpring = m->second.getTableInt("SpringId");
      m_stLetterCFG.dwSpringReward = m->second.getTableInt("letter_spring_reward");
    }
    else if (m->first == "Peak")
    {
      m_stPeakLevelCFG.m_dwSkillBreakPoint = m->second.getTableInt("SkillPointToBreak");

      auto itemfunc = [&](const string& key, xLuaData& data)
      {
        SPeakItem sItem;
        sItem.dwItem = data.getTableInt("item");
        sItem.dwNum = data.getTableInt("num");
        sItem.dwAddLv = data.getTableInt("level");
        m_stPeakLevelCFG.m_vecPeakItem.push_back(sItem);
      };
      m->second.getMutableData("itemaddlevel").foreach(itemfunc);

      auto exspf = [&](const string& key, xLuaData& data)
      {
        m_stPeakLevelCFG.m_mapEvo2PreSkillPoint[atoi(key.c_str())] = data.getInt();
      };
      m->second.getMutableData("Evo2PreSkillPoint").foreach(exspf);
    }
    else if (m->first == "PetAdventureEff")
    {
      m_stPetAdvEffCFG.vecRefineScore.clear();
      xLuaData& refined = m->second.getMutableData("refine");
      refined.getIDList(m_stPetAdvEffCFG.vecRefineScore);
      std::sort(m_stPetAdvEffCFG.vecRefineScore.begin(), m_stPetAdvEffCFG.vecRefineScore.end());

      m_stPetAdvEffCFG.mapType2MaxScoreAndBaseAttr.clear();
      auto getattr = [&](const string& k, xLuaData& d)
      {
        DWORD dwAttr = RoleDataConfig::getMe().getIDByName(k.c_str());
        if (dwAttr == 0)
          return;
        EAttrType etype = static_cast<EAttrType>(dwAttr);
        pair<DWORD, DWORD>& pa = m_stPetAdvEffCFG.mapType2MaxScoreAndBaseAttr[etype];
        pa.first = d.getTableInt("allscore");
        pa.second = d.getTableInt("maxattr");
      };
      m->second.getMutableData("enchant").foreach(getattr);

      m_stPetAdvEffCFG.vecSuperEnchantScore.clear();
      m->second.getMutableData("enchantgroup").getIDList(m_stPetAdvEffCFG.vecSuperEnchantScore);
      std::sort(m_stPetAdvEffCFG.vecSuperEnchantScore.begin(), m_stPetAdvEffCFG.vecSuperEnchantScore.end());

      m_stPetAdvEffCFG.mapItemID2Score.clear();
      auto gettitle = [&](const string& k, xLuaData &d)
      {
        m_stPetAdvEffCFG.mapItemID2Score[atoi(k.c_str())] = d.getInt();
      };
      m->second.getMutableData("title").foreach(gettitle);

      m_stPetAdvEffCFG.mapHeadWearQua2Score.clear();
      auto gethead = [&](const string& k, xLuaData& d)
      {
        EQualityType e = static_cast<EQualityType>(atoi(k.c_str()));
        if (EQualityType_IsValid(e) == false)
          return;
        m_stPetAdvEffCFG.mapHeadWearQua2Score[e] = d.getInt();
      };
      m->second.getMutableData("headwear").foreach(gethead);

      m_stPetAdvEffCFG.mapCardQua2Score.clear();
      auto getcard = [&](const string& k, xLuaData& d)
      {
        EQualityType e = static_cast<EQualityType>(atoi(k.c_str()));
        if (EQualityType_IsValid(e) == false)
          return;
        m_stPetAdvEffCFG.mapCardQua2Score[e] = d.getInt();
      };
      m->second.getMutableData("card").foreach(getcard);

      m_stPetAdvEffCFG.dwRefineMax = m->second.getTableInt("refine_max");
      m_stPetAdvEffCFG.dwEnchantMax = m->second.getTableInt("enchant_max");
      m_stPetAdvEffCFG.dwStarMax = m->second.getTableInt("star_max");
      m_stPetAdvEffCFG.dwTitleMax = m->second.getTableInt("title_max");
      m_stPetAdvEffCFG.dwHeadWearMax = m->second.getTableInt("headwear_max");
      m_stPetAdvEffCFG.dwCardMax = m->second.getTableInt("card_max");
      m_stPetAdvEffCFG.dwEnchantCalcPer = m->second.getTableInt("enchant_calc_per");
    }
    else if (m->first == "FuncForbid")
    {
      m_stFuncForbid.m_setForbid.clear();
      auto func = [this](const string& key, xLuaData& data)
      {
        m_stFuncForbid.m_setForbid.insert(data.getString());
        XLOG << "[配置misc-功能屏蔽] 屏蔽功能：" << data.getString() << XEND;
      };

      stringstream ss;
      ss << BaseConfig::getMe().getBranch();
      XLOG << "[配置misc-功能屏蔽] 加载 分支id" << ss.str() << XEND;
      string strBranchdId = ss.str();
      m->second.getMutableData(strBranchdId.c_str()).foreach(func);
    }
    else if (m->first == "GuildBuilding")
    {
      m_stGuildBuildingCFG.dwMaxSubmitCount = m->second.getTableInt("max_submit_count");
      if (m->second.has("open_cost"))
      {
        m_stGuildBuildingCFG.pairOpenCost.first = m->second.getData("open_cost").getTableInt("1");
        m_stGuildBuildingCFG.pairOpenCost.second = m->second.getData("open_cost").getTableInt("2");
      }
      else
      {
        m_stGuildBuildingCFG.pairOpenCost.first = 0;
        m_stGuildBuildingCFG.pairOpenCost.second = 0;
      }
      m_stGuildBuildingCFG.dwGateNpcID = m->second.getTableInt("gate_npc_id");
      m_stGuildBuildingCFG.dwBuildingLvUpNpcID = m->second.getTableInt("lvup_npc_id");
      m_stGuildBuildingCFG.dwBuildingLvUpMsgID = m->second.getTableInt("lvup_msg_id");
      m_stGuildBuildingCFG.dwBuildingBuildMsgID = m->second.getTableInt("build_msg_id");
      m_stGuildBuildingCFG.dwBuildingLvUpFinishMsgID = m->second.getTableInt("lvup_finish_msg_id");
      m_stGuildBuildingCFG.dwBuildingBuildFinishMsgID = m->second.getTableInt("build_finish_msg_id");
      m_stGuildBuildingCFG.dwOpenGuildLevel = m->second.getTableInt("open_guild_level");
    }
    else if(m->first == "Joy")
    {
      m_stJoyCFG.dwGuess = m->second.getTableInt("joy_guess");
      m_stJoyCFG.dwMischief = m->second.getTableInt("joy_mischief");
      m_stJoyCFG.dwQuestion = m->second.getTableInt("joy_question");
      m_stJoyCFG.dwFood = m->second.getTableInt("joy_food");
      m_stJoyCFG.dwYoyo = m->second.getTableInt("joy_yoyo");
      m_stJoyCFG.dwATF = m->second.getTableInt("joy_atf");
      m_stJoyCFG.dwAugury = m->second.getTableInt("joy_augury");

      auto rewardfunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwJoyValue = data.getTableInt("value");
        DWORD dwRewardID = data.getTableInt("rewardid");
        m_stJoyCFG.mapReward.insert(std::make_pair(dwJoyValue, dwRewardID));
      };
      m->second.getMutableData("reward").foreach(rewardfunc);

      auto foodfunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwFoodID = data.getInt();
        m_stJoyCFG.setFoodID.insert(dwFoodID);
      };
      m->second.getMutableData("food_id").foreach(foodfunc);

      m_stJoyCFG.dwFoodAdd = m->second.getTableInt("food_add");
      m_stJoyCFG.dwAuguryAdd = m->second.getTableInt("augury_add");
      m_stJoyCFG.dwSeatAdd = m->second.getTableInt("seat_add");

      m_stJoyCFG.dwMusicID = m->second.getTableInt("music_id");
      m_stJoyCFG.dwMusicNpcID = m->second.getTableInt("music_npc_id");

      m_stJoyCFG.dwShopType = m->second.getTableInt("shop_type");
      m_stJoyCFG.dwShopID = m->second.getTableInt("shop_id");
    }
    else if (m->first == "Recall")
    {
      m_stRecallCFG.dwNeedOffline = m->second.getTableInt("OfflineTime");
      m_stRecallCFG.dwNeedBaseLv = m->second.getTableInt("BaseLv");
      m_stRecallCFG.dwRecallMailID = m->second.getTableInt("recall_mailid");
      m_stRecallCFG.dwRecallBuffID = m->second.getTableInt("recall_buffid");
      m_stRecallCFG.dwMaxRecallCount = m->second.getTableInt("max_recall_count");
      m_stRecallCFG.dwFirstShareReward = m->second.getTableInt("first_share_reward");
      m_stRecallCFG.dwContractMailID = m->second.getTableInt("contract_mailid");
      m_stRecallCFG.dwContractTime = m->second.getTableInt("ContractTime");

      m_stRecallCFG.dwRewardBuffID = m->second.getTableInt("reward_buffid");

      m_stRecallCFG.vecRewardBuff2Layer.clear();
      xLuaData& bufflayer = m->second.getMutableData("reward_bufflayer");
      auto getbuff = [&](const string& k, xLuaData& d)
      {
        DWORD id = d.getTableInt("id");
        DWORD layer = d.getTableInt("layer");
        m_stRecallCFG.vecRewardBuff2Layer.push_back(make_pair(id, layer));
      };
      bufflayer.foreach(getbuff);
    }
    else if (m->first == "PackageMaterialCheck")
    {
      if (m->second.has("default") == true)
      {
        m_stPackageCFG.setPackFuncDefault.clear();
        auto defaultf = [&](const string& key, xLuaData& data)
        {
          m_stPackageCFG.setPackFuncDefault.insert(data.getInt());
        };
        m->second.getMutableData("default").foreach(defaultf);
      }

      m_stPackageCFG.mapPackFunc.clear();
      auto funcf = [&](const string& str, EPackFunc eFunc)
      {
        TSetDWORD& setPack = m_stPackageCFG.mapPackFunc[eFunc];
        auto packf = [&](const string& key, xLuaData& data)
        {
          setPack.insert(data.getInt());
        };
        m->second.getMutableData(str.c_str()).foreach(packf);
      };

      funcf("produce", EPACKFUNC_PRODUCE);
      funcf("upgrade", EPACKFUNC_UPGRADE);
      funcf("equipexchange", EPACKFUNC_EQUIPEXCHANGE);
      funcf("exchange", EPACKFUNC_EXCHANGE);
      funcf("refine", EPACKFUNC_REFINE);
      funcf("repair", EPACKFUNC_REPAIR);
      funcf("enchant", EPACKFUNC_ENCHANT);
      funcf("guilddonate", EPACKFUNC_GUILDDONATE);
      funcf("restore", EPACKFUNC_RESTORE);
      funcf("shop", EPACKFUNC_SHOP);
      funcf("pet_workspace", EPACKFUNC_PETWORK);
      funcf("quest_randitem", EPACKFUNC_QUEST_RANDITEM);
      funcf("guildBuilding", EPACKFUNC_GUILDBUILDING);
      funcf("exchange_shop", EPACKFUNC_EXCHANGESHOP);
      funcf("equipcompose", EPACKFUNC_EQUIPCOMPOSE);
    }
    else if (m->first == "GuildTreasure")
    {
      xLuaData& bcoin = m->second.getMutableData("LotteryWeekLimit");
      auto bcoinf = [&](const string& key, xLuaData& data)
      {
        m_stGuildCFG.mapTreasureBCoin[atoi(key.c_str())] = data.getInt();
      };
      bcoin.foreach(bcoinf);

      xLuaData& asset = m->second.getMutableData("GuildAssetWeekLimit");
      auto assetf = [&](const string& key, xLuaData& data)
      {
        m_stGuildCFG.mapTreasureAsset[atoi(key.c_str())] = data.getInt();
      };
      asset.foreach(assetf);

      m_stGuildCFG.dwTreasureBroadcastNpc = m->second.getTableInt("BroadcastNpc");
    }
    else if (m->first == "Wedding")
    {
      m_stWeddingMiscCFG.dwEngageRefresh = m->second.getTableInt("EngageRefresh");
      m_stWeddingMiscCFG.dwInviteMaxCount = m->second.getTableInt("InviteMaxCount");
      m_stWeddingMiscCFG.dwEngageInviteOverTime = m->second.getTableInt("EngageInviteOverTime");
      m_stWeddingMiscCFG.dwMaxFramePhotoCount = m->second.getTableInt("MaxFramePhotoCount");
      m_stWeddingMiscCFG.dwPhotoRefreshTime = m->second.getTableInt("FramePhotoRefreshTime");
      m_stWeddingMiscCFG.dwWeddingNpcID = m->second.getTableInt("Cememony_Npc");
      m_stWeddingMiscCFG.dwCourtshipInviteOverTime = m->second.getTableInt("Courtship_InviteOverTime");
      //m_stWeddingMiscCFG.dwDivorceNpc = m->second.getTableInt("Divorce_Npc");
      m_stWeddingMiscCFG.dwDivorceNpcDistance = m->second.getTableInt("Divorce_NpcDistance");
      m_stWeddingMiscCFG.dwDivorceOverTime = m->second.getTableInt("Courtship_InviteOverTime");
      m_stWeddingMiscCFG.dwTicketItemId = m->second.getTableInt("ticket_itemid");
    }
    else if (m->first == "Card")
    {
      m_stCardMiscCFG.dwDecomposePriceID = m->second.getTableInt("decompose_price_id");
      m_stCardMiscCFG.dwDecomposePriceCount = m->second.getTableInt("decompose_price_count");
      m_stCardMiscCFG.dwDecomposeItemID = m->second.getTableInt("decompose_item_id");
      m_stCardMiscCFG.dwExchangeCardMaxDraw = m->second.getTableInt("exchangecard_draw_max");
      auto dbfunc = [&](const string& key, xLuaData& data)
      {
        m_stCardMiscCFG.mapDecomposeBase[atoi(key.c_str())] = data.getInt();
      };
      m_stCardMiscCFG.mapDecomposeBase.clear();
      m->second.getMutableData("decompose_base").foreach(dbfunc);
    }
    else if (m->first == "CardRaid")
    {
      m_stPveCFG.dwLimitLv = m->second.getTableInt("enterlevel");

      m_stPveCFG.dwWeekResetTime = m->second.getTableInt("begin_reset_time");
      m_stPveCFG.dwWeekResetEndTime = m->second.getTableInt("end_reset_time");
      m_stPveCFG.dwRaidCardSuitNum = m->second.getTableInt("raid_card_suit");
      m_stPveCFG.dwAllMonsterCardNum = m->second.getTableInt("monster_card_num");
      m_stPveCFG.dwMaxSameMonsterNum = m->second.getTableInt("max_same_monster");
      m_stPveCFG.dwAllItemAndEnvNum = m->second.getTableInt("item_and_env_num");
      m_stPveCFG.dwMinItemCardNum = m->second.getTableInt("min_item_num");
      m_stPveCFG.dwMinEnvCardNum = m->second.getTableInt("min_env_num");
      m_stPveCFG.dwMaxSameEnvOrItemNum = m->second.getTableInt("max_same_env_item_num");

      m_stPveCFG.dwBossCardNum = m->second.getTableInt("boss_card_num");
      m_stPveCFG.setBossGroups.clear();
      m->second.getMutableData("boss_group_index").getIDList(m_stPveCFG.setBossGroups);

      m_stPveCFG.dwPlayCardInterval = m->second.getTableInt("play_card_interval");
      m_stPveCFG.dwFriendCardDelay = m->second.getTableInt("friend_card_delay");
      m_stPveCFG.dwFinishCardCloseTime = m->second.getTableInt("card_finish_close");
      m_stPveCFG.dwFriendNpcID = m->second.getTableInt("friend_npc_id");
      m_stPveCFG.dwEnemyNpcID = m->second.getTableInt("enemy_npc_id");
      m_stPveCFG.dwSandGlassNpcID = m->second.getTableInt("sandglass_npc_id");
      m_stPveCFG.dwPrepareCardTime = m->second.getTableInt("prepare_card_time");

      xLuaData& posd = m->second.getMutableData("enemy_npc_pos");
      m_stPveCFG.oEnemyNpcDestPos.x = posd.getTableFloat("1");
      m_stPveCFG.oEnemyNpcDestPos.y = posd.getTableFloat("2");
      m_stPveCFG.oEnemyNpcDestPos.z = posd.getTableFloat("3");

      xLuaData& safed = m->second.getMutableData("safe_central_pos");
      m_stPveCFG.oSafeCentralPos.x = safed.getTableFloat("1");
      m_stPveCFG.oSafeCentralPos.y = safed.getTableFloat("2");
      m_stPveCFG.oSafeCentralPos.z = safed.getTableFloat("3");

      m_stPveCFG.setSafeRange.clear();
      m->second.getMutableData("safe_range").getIDList(m_stPveCFG.setSafeRange);
      m_stPveCFG.setPosionBuff.clear();
      m->second.getMutableData("posion_buff").getIDList(m_stPveCFG.setPosionBuff);
      m_stPveCFG.dwSafeShrinkInterval = m->second.getTableInt("safe_shrink_interval");

      m_stPveCFG.vecCardPos.clear();
      auto getcardpos = [&](const string& k, xLuaData& d)
      {
        xPos p;
        p.x = d.getTableFloat("1");
        p.y = d.getTableFloat("2");
        p.z = d.getTableFloat("3");
        m_stPveCFG.vecCardPos.push_back(p);
      };
      m->second.getMutableData("card_pos").foreach(getcardpos);

      m_stPveCFG.dwNormalCardNpcID = m->second.getTableInt("normal_card_npc");
      m_stPveCFG.dwBossCardNpcID = m->second.getTableInt("boss_card_npc");
      m_stPveCFG.dwCardNpcShowTime = m->second.getTableInt("card_npc_show_time");
      m_stPveCFG.dwCardNpcStayTime = m->second.getTableInt("card_npc_stay_time");
      xLuaData& fposd = m->second.getMutableData("friend_card_pos");
      m_stPveCFG.oFriendCardPos.x = fposd.getTableFloat("1");
      m_stPveCFG.oFriendCardPos.y = fposd.getTableFloat("2");
      m_stPveCFG.oFriendCardPos.z = fposd.getTableFloat("3");

      m_stPveCFG.vecEffectPath.clear();
      auto geteffect = [&](const string& k, xLuaData& d)
      {
        m_stPveCFG.vecEffectPath.push_back(d.getString());
      };
      m->second.getMutableData("posion_effect").foreach(geteffect);
      m_stPveCFG.vecSandGlassAction.clear();
      m->second.getMutableData("sandglass_action").getIDList(m_stPveCFG.vecSandGlassAction);
      m_stPveCFG.dwWeekRewardNum = m->second.getTableInt("max_reward_count");
      m_stPveCFG.dwSitActionID = m->second.getTableInt("sit_action");
      m_stPveCFG.dwEnemyNpcDir = m->second.getTableInt("enemy_npc_dir");
      m_stPveCFG.dwItemDispTime = m->second.getTableInt("item_disp_time");
      m_stPveCFG.oPreGotoEffect = m->second.getMutableData("pre_goto_effect");
      m_stPveCFG.oEndGotoEffect = m->second.getMutableData("end_goto_effect");
      m_stPveCFG.dwEnemyCardDir = m->second.getTableInt("enemy_card_dir");
      m_stPveCFG.dwMaxValidDis = m->second.getTableInt("max_valid_dis");
    }
    else if (m->first == "EquipEnchant")
    {
      m_stItemCFG.mapSpecEnchantCost.clear();
      xLuaData& costd = m->second.getMutableData("SpecialCost");
      auto getcost = [&](const string& k, xLuaData&d)
      {
        DWORD id = atoi(k.c_str());
        if (id <= EENCHANTTYPE_MIN || id >= EENCHANTTYPE_MAX)
        {
          XERR << "[GameConfig-EquipEnchant], 附魔类型配置错误" << id << XEND;
          return;
        }

        EEnchantType eType = static_cast<EEnchantType>(id);
        map<EItemType, TVecItemInfo>& costmap = m_stItemCFG.mapSpecEnchantCost[eType];

        auto cost = [&](const string& k1, xLuaData& d1)
        {
          DWORD dtype = atoi(k1.c_str());
          if (EItemType_IsValid(dtype) == false)
            return;
          EItemType e = static_cast<EItemType> (dtype);
          TVecItemInfo& vecItems = costmap[e];
          auto getitem = [&](const string& k2, xLuaData& d2)
          {
            ItemInfo item;
            item.set_id(d2.getTableInt("itemid"));
            item.set_count(d2.getTableInt("num"));
            vecItems.push_back(item);
          };

          d1.foreach(getitem);
        };
        d.foreach(cost);
      };
      costd.foreach(getcost);
    }
    else if (m->first == "MvpBattle")
    {
      m_stMvpBattleCFG.vecTeamNum2MatchInfo.clear();
      auto getmatchtime = [&](const string& k, xLuaData& d)
      {
        SMvpTeamNumMatchCFG stData;

        stData.dwMinTeamNum = d.getTableInt("TeamNumber");
        stData.dwMinMatchTime = d.getTableInt("MinTime");
        stData.dwMaxMatchTime = d.getTableInt("MaxTime");
        stData.dwMvpNum = d.getTableInt("MvpNum");
        stData.dwMiniNum = d.getTableInt("MiniNum");
        stData.dwRaidID = d.getTableInt("RaidID");
        stData.dwMatchPunishTime = d.getTableInt("MatchPunishTime");
        m_stMvpBattleCFG.vecTeamNum2MatchInfo.push_back(stData);
      };
      m->second.getMutableData("ActivityOpenCondition").foreach(getmatchtime);
      std::sort(m_stMvpBattleCFG.vecTeamNum2MatchInfo.begin(), m_stMvpBattleCFG.vecTeamNum2MatchInfo.end(), [&](const SMvpTeamNumMatchCFG& r1, const SMvpTeamNumMatchCFG& r2) -> bool
      {
        return r1.dwMinTeamNum > r2.dwMinTeamNum;
      });

      m_stMvpBattleCFG.dwMaxBossRewardCnt = m->second.getTableInt("MvpRewardTimes");
      m_stMvpBattleCFG.dwMaxMiniRewardCnt = m->second.getTableInt("MiniRewardTimes");
      m_stMvpBattleCFG.dwLimitUserLv = m->second.getTableInt("BaseLevel");
      m_stMvpBattleCFG.dwLimitTeamUserNum = m->second.getTableInt("TeamMumbernumber");
      m_stMvpBattleCFG.dwEndDialogID = m->second.getTableInt("EndDialogID");
      m_stMvpBattleCFG.dwEndWaitTime = m->second.getTableInt("EndWaitTime");
      m_stMvpBattleCFG.dwMaxMatchTime = m->second.getTableInt("MaxMatchTime");
      m_stMvpBattleCFG.dwResetCDTime = m->second.getTableInt("DiePunishResetTime");
      m_stMvpBattleCFG.dwMaxCD = m->second.getTableInt("DieMaxCD");
      m_stMvpBattleCFG.dwExpelTime = m->second.getTableInt("DieExpelTime");

      m_stMvpBattleCFG.time2ReliveCD.clear();
      xLuaData& diepunish = m->second.getMutableData("DiePunish");
      auto getcd = [&](const string& k, xLuaData& d)
      {
        xLuaData& interd = d.getMutableData("interval");
        pair<DWORD, DWORD> pa;
        pa.first = interd.getTableInt("1");
        pa.second = interd.getTableInt("2");

        DWORD cd = d.getTableInt("cd");
        m_stGuildFireCFG.time2ReliveCD.push_back(make_pair(cd, pa));
      };
      diepunish.foreach(getcd);

      //m_stMvpBattleCFG.setValidRaidMap.clear();
      //m->second.getMutableData("ActivityMap").getIDList(m_stMvpBattleCFG.setValidRaidMap);
    }
    else if (m->first == "Servant")
    {
      m_stServantCFG.dwMaxFavorability = m->second.getTableInt("max_favorability");
      m_stServantCFG.fRange = m->second.getTableFloat("pos_range");
      m_stServantCFG.dwStayTime = m->second.getTableInt("stay_time");
      m_stServantCFG.dwStayNum = m->second.getTableInt("stay_num");
      m_stServantCFG.dwStayFavorability = m->second.getTableInt("stay_favorability");
      m_stServantCFG.dwDisappearTime = m->second.getTableInt("disappear_time");
      m_stServantCFG.dwAddFavo = m->second.getTableInt("add_favo");
      m_stServantCFG.dwCemeteryRaid= m->second.getTableInt("cemetery_raid");
      m_stServantCFG.dwSpaceRaid = m->second.getTableInt("space_raid");
      m_stServantCFG.dwTerroristRaid = m->second.getTableInt("terrorist_raid");

      m_stServantCFG.m_mapDescription.clear();
      xLuaData& desc = m->second.getMutableData("description");
      auto getdesc = [&](const string& k, xLuaData& d)
      {
        DWORD dwid = d.getTableInt("id");
        string sname = d.getTableString("name");
        m_stServantCFG.m_mapDescription.emplace(dwid, sname);
      };
      desc.foreach(getdesc);

      m_stServantCFG.m_vecReward.clear();
      xLuaData& reward = m->second.getMutableData("reward");
      auto getreward = [&](const string& k, xLuaData& d)
      {
        DWORD dwValue = d.getTableInt("value");
        DWORD dwRewardID = d.getTableInt("rewardid");
        DWORD size = m_stServantCFG.m_vecReward.size();
        m_stServantCFG.m_mapVar.insert(make_pair(dwValue, 1 << size));
        m_stServantCFG.m_vecReward.emplace_back(make_pair(dwValue, dwRewardID));
      };
      reward.foreach(getreward);

      m_stServantCFG.m_mapGrowthReward.clear();
      xLuaData& growthdata = m->second.getMutableData("growth_reward");
      auto growthfunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwid = atoi(key.c_str());
        map<DWORD, DWORD> mapReward;
        auto rewardfunc = [&](const string& k, xLuaData& d)
        {
          DWORD value = d.getTableInt("value");
          DWORD reward = d.getTableInt("rewardid");
          mapReward.emplace(value, reward);
        };
        data.foreach(rewardfunc);

        m_stServantCFG.m_mapGrowthReward.emplace(dwid, mapReward);
      };
      growthdata.foreach(growthfunc);
    }
    else if (m->first == "RestoreBook")
    {
      m_stSystemCFG.dwRecordNameMinSize = 1;
      m_stSystemCFG.dwRecordNameMaxSize = m->second.getTableInt("record_name_maxlen");

      m_stRecordCFG.dwDefaultSlotNum = m->second.getTableInt("default_solt");
      m_stRecordCFG.dwMonthCardSlotNum = m->second.getTableInt("month_card_solt");
      m_stRecordCFG.dwLoadCD = m->second.getTableInt("load_cd_seconds");
      m_stRecordCFG.dwReocrdNameMaxLen = m->second.getTableInt("record_name_maxlen");
      m_stRecordCFG.dwRestoreBookItemID = m->second.getTableInt("restore_book_itemid");
      m_stRecordCFG.dwOpenNoviceSkillID = m->second.getTableInt("open_novice_skillid");
      m_stRecordCFG.dwMenuID = m->second.getTableInt("menuid");
      m_stRecordCFG.mapBuyTimes2CostIDAndNum.clear();
      auto getcost = [&](const string& k, xLuaData& d)
      {
        DWORD dwBuyTimes = atoi(k.c_str());
        if (dwBuyTimes == 0)
        {
          return;
        }
        pair<DWORD, DWORD>& pa = m_stRecordCFG.mapBuyTimes2CostIDAndNum[dwBuyTimes];
        pa.first = d.getTableInt("1");
        pa.second = d.getTableInt("2");
      };
      m->second.getMutableData("buy_solt_info").foreach(getcost);
      m_stRecordCFG.dwTotalBuyNum = m_stRecordCFG.mapBuyTimes2CostIDAndNum.size();
    }
    else if( m->first == "Profession")
    {
      m_stProfessionMiscCFG.dwGoldCost = m->second.getTableInt("price_gold");
      m_stProfessionMiscCFG.dwZenyCost = m->second.getTableInt("price_zeny");
      m_stProfessionMiscCFG.dwSwitchCost = m->second.getTableInt("switch_zeny");
      m_stProfessionMiscCFG.dwLoadCD = m->second.getTableInt("load_cd");
    }
    else if (m->first == "FoodPackPage")
    {
      auto pagef = [&](const string& key, xLuaData& data)
      {
        auto typef = [&](const string& key, xLuaData& data)
        {
          DWORD dwType = data.getInt();
          if (dwType <= EITEMTYPE_MIN || dwType >= EITEMTYPE_MAX || EItemType_IsValid(dwType) == false)
          {
            bCorrect = false;
            XERR << "[GameConfig] FoodPackPage" << dwType << "不是合法的ItemType" << XEND;
            return;
          }
          m_stFood.setPackTypes.insert(static_cast<EItemType>(dwType));
        };
        data.getMutableData("types").foreach(typef);
      };
      m->second.foreach(pagef);
    }
    else if (m->first == "Shop")
    {
      m_stGameShopCFG.mapLimitItem.clear();
      auto limitfunc = [&](const string& str, xLuaData& data)
      {
        DWORD itemid = atoi(str.c_str());
        DWORD num = data.getInt();
        m_stGameShopCFG.mapLimitItem.emplace(itemid, num);
      };
      m->second.getMutableData("forever_limit_item").foreach(limitfunc);
    }
    else if (m->first == "DressUp")
    {
      m_stDressStageCFG.m_dwStaticMap = m->second.getTableInt("static_map");
      m_stDressStageCFG.m_dwShowTime = m->second.getTableInt("showtime");
      m_stDressStageCFG.m_dwMateRange = m->second.getTableInt("mate_range");
      m_stDressStageCFG.m_dwStageRange = m->second.getTableInt("stage_range");
      m_stDressStageCFG.m_dwUnattackedBuff = m->second.getTableInt("unattack_buff");
      m_stDressStageCFG.m_dwRideWolfSkill = m->second.getTableInt("ridewolf_skill");

      xLuaData& quitpos = m->second.getMutableData("quitpos");
      m_stDressStageCFG.m_pQuitPos.x = quitpos.getTableFloat("1");
      m_stDressStageCFG.m_pQuitPos.y = quitpos.getTableFloat("2");
      m_stDressStageCFG.m_pQuitPos.z = quitpos.getTableFloat("3");

      xLuaData& musicPos = m->second.getMutableData("musicPos");
      m_stDressStageCFG.m_pMusicPos.x = musicPos.getTableFloat("1");
      m_stDressStageCFG.m_pMusicPos.y = musicPos.getTableFloat("2");
      m_stDressStageCFG.m_pMusicPos.z = musicPos.getTableFloat("3");

      m_stDressStageCFG.m_setStageIDs.clear();
      auto stagefunc = [&](const string& key, xLuaData& data)
      {
        m_stDressStageCFG.m_setStageIDs.emplace(data.getInt());
      };
      m->second.getMutableData("stageid").foreach(stagefunc);

      m_stDressStageCFG.m_mapEnterPos.clear();
      auto enterposfunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwID = atoi(key.c_str());
        xPos pos;
        pos.x = data.getTableFloat("1");
        pos.y = data.getTableFloat("2");
        pos.z = data.getTableFloat("3");
        m_stDressStageCFG.m_mapEnterPos.emplace(dwID, pos);
      };
      m->second.getMutableData("enterPos").foreach(enterposfunc);

      m_stDressStageCFG.m_mapDoubleEnterPos.clear();
      auto doubleposfunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwID = atoi(key.c_str());
        xPos pos1;
        xPos pos2;
        xLuaData& pos1data = data.getMutableData("pos1");
        xLuaData& pos2data = data.getMutableData("pos2");
        pos1.x = pos1data.getTableFloat("1");
        pos1.y = pos1data.getTableFloat("2");
        pos1.z = pos1data.getTableFloat("3");
        pos2.x = pos2data.getTableFloat("1");
        pos2.y = pos2data.getTableFloat("2");
        pos2.z = pos2data.getTableFloat("3");
        pair<xPos, xPos> pairPos;
        pairPos.first = pos1;
        pairPos.second = pos2;
        m_stDressStageCFG.m_mapDoubleEnterPos.emplace(dwID, pairPos);
      };
      m->second.getMutableData("doubleEnterPos").foreach(doubleposfunc);

      m_stDressStageCFG.m_mapCost.clear();
      auto costfunc = [&](const string& key, xLuaData& data)
      {
        DWORD id = data.getTableInt("id");
        DWORD num = data.getTableInt("num");
        m_stDressStageCFG.m_mapCost.emplace(id, num);
      };
      m->second.getMutableData("cost").foreach(costfunc);

      m_stDressStageCFG.m_mapVaildEquip.clear();
      auto equipfunc = [&](const string& key, xLuaData& data)
      {
        DWORD stageid = atoi(key.c_str());
        auto it = m_stDressStageCFG.m_setStageIDs.find(stageid);
        if(it == m_stDressStageCFG.m_setStageIDs.end())
          return;

        TMapDressEquip validEquip;
        auto stagefunc = [&](const string& ktype, xLuaData& datastage)
        {
          DWORD dwDataType = atoi(ktype.c_str());
          TSetDWORD setIDs;
          auto idfunc = [&](const string& kequip, xLuaData& dataequip)
          {
            setIDs.emplace(dataequip.getInt());
            if(m_stDressStageCFG.m_mapDefaultEquip[stageid][dwDataType] == 0)
              m_stDressStageCFG.m_mapDefaultEquip[stageid][dwDataType] = dataequip.getInt();
          };
          datastage.foreach(idfunc);
          validEquip.emplace(dwDataType, setIDs);
        };
        data.foreach(stagefunc);

        m_stDressStageCFG.m_mapVaildEquip.emplace(stageid, validEquip);
      };
      m->second.getMutableData("equippos").foreach(equipfunc);
    }
    else if (m->first == "Activity")
    {
      auto petf = [&](const string& key, xLuaData& data)
      {
        DWORD dwID = atoi(key.c_str());
        parseTime(data.getString(), m_stActCFG.mapPetWorkEndTime[dwID]);
      };
      m->second.getMutableData("PetWork").getMutableData("endtime").foreach(petf);
    }
    else if (m->first == "Dead")
    {
      m_stDeadCFG.dwMaxCoin = m->second.getTableInt("max_deadcoin");
      m_stDeadCFG.dwQuestNum = m->second.getTableInt("quest_perday");
      m_stDeadCFG.dwMaxLv = m->second.getTableInt("max_deadlv");
    }
    else if (m->first == "PvpTeamRaid" || m->first == "PvpTeamRaid_Relax")
    {
      auto getPwsData = [&](STeamPwsCFG& cfg, xLuaData& Data)
      {
        cfg.dwCacheMatchNum = Data.getTableInt("APoooCacheSize");
        cfg.dwMaxTeamMatchTime = Data.getTableInt("MaxTeamMatchTime");
        cfg.dwMaxFireMatchTime = Data.getTableInt("MaxFireMatchTime");
        cfg.dwMaxAPoolMatchTime = Data.getTableInt("APoolMaxMatchTime");
        cfg.dwMaxBPoolMatchTime = Data.getTableInt("BPoolMaxMatchTime");
        cfg.dwMaxPrepareTime = Data.getTableInt("MaxPrepareTime");
        cfg.dwWeekMaxCount = Data.getTableInt("WeekMaxCount");
        cfg.dwRequireLv = Data.getTableInt("RequireLv");
        cfg.dwMaxRecordCnt = Data.getTableInt("MaxRankNum");
        cfg.dwLeaveTeamPunishCD = Data.getTableInt("LeavePunishCD");
        cfg.intLeaveTeamPunishScore = Data.getTableInt("LeavePunishScore");
        cfg.dwSeasonBattleTimes = Data.getTableInt("SeasonBattleTimes");
        cfg.dwRewardTime = Data.getTableInt("FinalBeginRewardTime");

        // 匹配池分组
        cfg.vecMatchScoreGroup.clear();
        auto getpoolgrp = [&](const string& key, xLuaData& d)
        {
          DWORD min = d.getTableInt("1");
          DWORD max = d.getTableInt("2");
          cfg.vecMatchScoreGroup.push_back(std::make_pair(min,max));
        };
        Data.getMutableData("MatchScoreGruop").foreach(getpoolgrp);
        auto &grp = cfg.vecMatchScoreGroup;
        std::sort(grp.begin(), grp.end(), [](const pair<DWORD,DWORD>& r1, const pair<DWORD,DWORD>& r2) -> bool{
            return r1.first < r2.first;
            });

        // 段位-积分
        cfg.mapRankInfo.clear();
        auto getrank = [&](const string& key, xLuaData& d)
        {
          DWORD rank = atoi(key.c_str());
          DWORD min = d.getTableInt("min");
          DWORD max = d.getTableInt("max");
          ETeamPwsRank e = static_cast<ETeamPwsRank>(rank);
          SPwsRankCFG& info = cfg.mapRankInfo[e];
          info.dwMinScore = min;
          info.dwMaxScore = max;
          info.eRank = e;

          bool win = true;
          auto gitem = [&](const string& k, xLuaData& d1)
          {
            ItemInfo item;
            item.set_id(d1.getTableInt("itemid"));
            item.set_count(d1.getTableInt("count"));
            if (win)
              info.vecWinItems.push_back(item);
            else
              info.vecLoseItems.push_back(item);
          };
          d.getMutableData("winreward").foreach(gitem);
          win = false;
          d.getMutableData("losereward").foreach(gitem);
        };
        Data.getMutableData("Rank2Score").foreach(getrank);

        // 赛季奖励
        cfg.vecSeasonReward.clear();
        auto getrwd = [&](const string& key, xLuaData& d)
        {
          SPwsSeasonReward s;
          s.dwNeedRank = d.getTableInt("NeedRank");
          s.dwMailID = d.getTableInt("Mail");
          cfg.vecSeasonReward.push_back(s);
        };
        Data.getMutableData("RewardInfo").foreach(getrwd);

        cfg.setRaidIDs.clear();
        Data.getMutableData("RaidIDs").getIDList(cfg.setRaidIDs);

        cfg.dwLastTime = Data.getTableInt("Time");
        cfg.dwCollectSkill = Data.getTableInt("ElementCollectSkillID");
        cfg.dwColleckBuffSkill = Data.getTableInt("BuffCollectSkillID");
        cfg.dwBallInterval = Data.getTableInt("BallValueInterval");
        cfg.dwBallDelayTime = Data.getTableInt("BallDelayValueTime");
        cfg.dwWinScore = Data.getTableInt("WinScore");
        cfg.dwPrepareTime = Data.getTableInt("PrepareTime");
        cfg.dwMagicCD = Data.getTableInt("MagicCD");
        cfg.dwKillScore = Data.getTableInt("KillScore");
        cfg.dwSummonBallTime = Data.getTableInt("SummonBallTime");
        cfg.dwPickBuffScore = Data.getTableInt("PickBuffScore");
        cfg.dwBuffNpcBeginTime = Data.getTableInt("BebinBuffTime");
        cfg.dwBuffNpcInterval = Data.getTableInt("RefreshBuffInterval");
        cfg.dwBuffNpcUniqueID = Data.getTableInt("BuffNpcUniqueID");

        // buffnpc buff效果
        cfg.mapNpc2Buffs.clear();
        auto getbuffeff = [&](const string& key, xLuaData& d)
        {
          DWORD npcid = atoi(key.c_str());
          SPwsPickBuffEffect& st = cfg.mapNpc2Buffs[npcid];
          auto buffeff = [&](const string& k1, xLuaData& d1)
          {
            if (d1.has("selfbuff"))
            {
              DWORD buff = d1.getTableInt("selfbuff");
              st.mapSelfBuff2Time[buff] = d1.getTableInt("time");
            }
            else if (d1.has("teambuff"))
            {
              DWORD buff = d1.getTableInt("teambuff");
              st.mapTeamBuff2Time[buff] = d1.getTableInt("time");
            }
          };
          d.foreach(buffeff);
          st.strName = d.getTableString("name");
        };
        Data.getMutableData("BuffEffect").foreach(getbuffeff);

        // 持球buff
        cfg.setBallBuffs.clear();
        Data.getMutableData("HoldBallBuff").getIDList(cfg.setBallBuffs);
        // 法球unique id
        cfg.vecBallUniqueID.clear();
        Data.getMutableData("BallUniqueID").getIDList(cfg.vecBallUniqueID);
        // 持球分数
        cfg.mapBallNum2Score.clear();
        auto getnumscore = [&](const string& key, xLuaData& d)
        {
          DWORD num = atoi(key.c_str());
          cfg.mapBallNum2Score[num] = d.getInt();
        };
        Data.getMutableData("BallScore").foreach(getnumscore);

        // 魔法组合
        cfg.mapBallCombines.clear();
        auto getmagic = [&](const string& key, xLuaData& d)
        {
          DWORD configid = atoi(key.c_str());
          DWORD e1 = configid / 10;
          DWORD e2 = configid % 10;
          SPwsMagicCombine& mdata = cfg.mapBallCombines[configid];
          mdata.eBall1 = static_cast<EMagicBallType>(e1);
          mdata.eBall2 = static_cast<EMagicBallType>(e2);
          mdata.dwConfigID = configid;
          d.getMutableData("BuffIDs").getIDList(mdata.setBuffIDs);
          mdata.strName = d.getTableString("name");
        };
        Data.getMutableData("ElementCombine").foreach(getmagic);

        // 元素球
        cfg.mapBallData.clear();
        auto getball = [&](const string& key, xLuaData& d)
        {
          DWORD ball = atoi(key.c_str());
          EMagicBallType e = static_cast<EMagicBallType>(ball);
          SPwsMagicBallData& stball = cfg.mapBallData[e];
          stball.eBallType = e;
          stball.dwSummonNpcID = d.getTableInt("npcid");
          stball.dwBuffID = d.getTableInt("buffid");
          stball.strName = d.getTableString("name");
        };
        Data.getMutableData("ElementNpcsID").foreach(getball);

        cfg.mapColorInfo.clear();
        auto getcolor = [&] (const string& key, xLuaData& d)
        {
          DWORD color = atoi(key.c_str());
          SPwsColorInfo& cinfo = cfg.mapColorInfo[color];
          cinfo.strName = d.getTableString("name");
        };
        Data.getMutableData("TeamColorInfo").foreach(getcolor);

        cfg.dwWarnScore = Data.getTableInt("WarnScore");
        cfg.dwRewardMailID = Data.getTableInt("UserRewardMail");

        cfg.vecPreLimitArea.clear();
        auto getarea = [&](const string& key, xLuaData& d)
        {
          pair<xPos, float> pa;
          pa.first.x = d.getMutableData("pos").getTableFloat("1");
          pa.first.y = d.getMutableData("pos").getTableFloat("2");
          pa.first.z = d.getMutableData("pos").getTableFloat("3");
          pa.second = d.getTableFloat("radius");
          cfg.vecPreLimitArea.push_back(pa);
        };
        Data.getMutableData("PrepareLimitArea").foreach(getarea);

        cfg.dwBuffNpcClearTime = Data.getTableInt("BuffNpcClearTime");

        auto getaddscore = [](xLuaData& scored, TVecDWORD& vecadd)
        {
          vecadd.clear();
          scored.getIDList(vecadd);
          std::sort(vecadd.begin(), vecadd.end(), [](DWORD v1, DWORD v2)-> bool { return v1 > v2; });
        };
        getaddscore(Data.getMutableData("AllScore2ExtraAdd"), cfg.vecAllScoreAdd);
        getaddscore(Data.getMutableData("KillScore2ExtraAdd"), cfg.vecKillScoreAdd);
        getaddscore(Data.getMutableData("HealScore2ExtraAdd"), cfg.vecHealSocreAdd);
      };
      if (m->first == "PvpTeamRaid")
        getPwsData(m_stTeamPwsCFG, m->second);
      else if (m->first == "PvpTeamRaid_Relax")
        getPwsData(m_stRelaxTeamPwsCFG, m->second);
    }
    else if (m->first == "RaidDeadBoss")
    {
      //无限塔
      xLuaData& endlessd = m->second.getMutableData("tower");
      m_stEndlessTowerCFG.dwDeadBossSummonTime = endlessd.getTableInt("week_deadboss_count");
      m_stEndlessTowerCFG.dwDeadBossUID = endlessd.getTableInt("deadboss_uid");
      m_stEndlessTowerCFG.dwDeadBossNum = endlessd.getTableInt("deadboss_num");
      m_stEndlessTowerCFG.dwDeadBossLayer = endlessd.getTableInt("deadboss_layer");
      // 公会随机副本
      xLuaData& guildraidd = m->second.getMutableData("guild_raid");
      m_stGuildRaidCFG.dwDeadBossSummonTime = guildraidd.getTableInt("week_deadboss_count");
      m_stGuildRaidCFG.dwDeadBossUID = guildraidd.getTableInt("deadboss_uid");
      m_stGuildRaidCFG.dwDeadBossNum = guildraidd.getTableInt("deadboss_num");
      m_stGuildRaidCFG.dwDeadBossRaidLv = guildraidd.getTableInt("deadboss_raidlv");
      // 卡牌副本
      xLuaData& pvecardd = m->second.getMutableData("pve_card");
      m_stPveCFG.dwDeadBossSummonTime = pvecardd.getTableInt("week_deadboss_count");
      m_stPveCFG.dwDeadBossUID = pvecardd.getTableInt("deadboss_uid");
      m_stPveCFG.dwDeadBossNum = pvecardd.getTableInt("deadboss_num");
      m_stPveCFG.dwDeadBossNpcUID = pvecardd.getTableInt("deadboss_npc_uid");
      m_stPveCFG.dwDeadBossMinDifficulty = pvecardd.getTableInt("deadboss_min_difficulty");
      m_stPveCFG.dwWaitDeadNpcTime = pvecardd.getTableInt("deadboss_npc_time");
      // mvp竞争战
      xLuaData& mvpbattled = m->second.getMutableData("mvp_battle");
      m_stMvpBattleCFG.dwDeadBossKillTime = mvpbattled.getTableInt("week_deadboss_count");
      m_stMvpBattleCFG.dwDeadBossNum = mvpbattled.getTableInt("deadboss_num");
      xLuaData& timed = mvpbattled.getMutableData("deadboss_summon_time");
      m_stMvpBattleCFG.paSummonDeadBossTime.first = timed.getTableInt("1");
      m_stMvpBattleCFG.paSummonDeadBossTime.second = timed.getTableInt("2");
      m_stMvpBattleCFG.setDeadBossUniqIDs.clear();
      mvpbattled.getMutableData("dead_boss_uids").getIDList(m_stMvpBattleCFG.setDeadBossUniqIDs);
    }
  }

  if (m_stNewRoleCFG.dwPurify > m_stSPurifyCFG.dwMaxPurify)
  {
    XERR << "[MiscConfig] NewRole purify大于maxvalue, " << m_stNewRoleCFG.dwPurify << ", " << m_stSPurifyCFG.dwMaxPurify << XEND;
    bCorrect = false;
  }

  if (loadObscenceLanguageConfig() == false)
    bCorrect = false;
  if (loadServerGameConfig() == false)
    bCorrect = false;
  if (loadUnionConfig() == false)
    bCorrect = false;

  if (bCorrect)
    XLOG << "[MiscConfig], 成功加载GameConfig.txt" << XEND;
  return bCorrect;
}

bool MiscConfig::checkConfig()
{
  bool bCorrect = true;

  // item
  for (auto &m : m_stItemCFG.mapPackSkillSlot)
  {
    for (auto &v : m.second)
    {
      if (SkillConfig::getMe().getSkillCFG(v.dwSkillID) == nullptr)
      {
        bCorrect = false;
        XERR << "[GameConfig] Item->skill_slot skillid :" << v.dwSkillID << "未在Table_Skill.txt表中找到" << XEND;
      }
    }
  }
  if (ItemConfig::getMe().getItemCFG(m_stItemCFG.dwKapulaMapItem) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Item->kapula_map_item :" << m_stItemCFG.dwKapulaMapItem << "未在Table_Item.txt表中找到" << XEND;
  }
  if (ItemConfig::getMe().getItemCFG(m_stItemCFG.dwKapulaStoreItem) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Item->kapula_store_item :" << m_stItemCFG.dwKapulaStoreItem << "未在Table_Item.txt表中找到" << XEND;
  }

  // dojo
  for (auto &m : m_stGuildDojoCFG.mapBaseLvReq)
  {
    if (DojoConfig::getMe().hasGroup(m.first) == false)
    {
      bCorrect = false;
      XERR << "[GameConfig] GuildDojo Dojo->DojoGroupId :" << m.first << "未在Table_Guild_Dojo.txt表中找到" << XEND;
    }
  }

  // quest
  const TVecWantedQuestCFG& vecWantedCFG = QuestConfig::getMe().getWantedQuestList();
  for (int i = EWANTEDTYPE_TOTAL; i < EWANTEDTYPE_MAX; ++i)
  {
    DWORD dwCount = m_stQuestCFG.arrMaxWanted[i];
    for (auto &v : vecWantedCFG)
    {
      DWORD dwCurCount = 0;
      for (auto &item : v.vecQuest)
        dwCurCount += item.arrAllItem[i].size();
      if (dwCount > dwCurCount)
      {
        bCorrect = false;
        XERR << "[GameConfig] Quest -> maxwanted type :" << i << "minlv :" << v.lvRange.first << " maxlv :" << v.lvRange.second
          << "需要 :" << dwCount << "个任务, 但是实际只有 :" << dwCurCount << "个任务库" << XEND;
      }
    }
  }

  // new role
  for (auto &v : m_stNewRoleCFG.vecMalePortrait)
  {
    const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(v);
    if (pCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> portrait male :" << v << "未在 Table_HeadImage.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecFemalePortrait)
  {
    const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(v);
    if (pCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> portrait female :" << v << "未在 Table_HeadImage.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecFrame)
  {
    const SPortraitCFG* pCFG = PortraitConfig::getMe().getPortraitCFG(v);
    if (pCFG == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> frame :" << v << "未在 Table_HeadImage.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecHair)
  {
    if (ItemConfig::getMe().getHairCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> hair :" << v << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecEye)
  {
    if (ItemConfig::getMe().getItemCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> eye :" << v << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecAction)
  {
    const SActionAnimBase* pBase = TableManager::getMe().getActionAnimCFG(v);
    if (pBase == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> action :" << v << "未在 Table_ActionAnime.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecExpression)
  {
    const SExpression* pBase = TableManager::getMe().getExpressionCFG(v);
    if (pBase == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> expression :" << v << "未在 Table_Expression.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecItems)
  {
    if (ItemConfig::getMe().getItemCFG(v.id()) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> item :" << v.id() << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecFirstShortcut)
  {
    if (ItemConfig::getMe().getItemCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> shortcutfirst :" << v << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecBuffs)
  {
    if (TableManager::getMe().getBufferCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> buff :" << v << "未在 Table_Buffer.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecActiveMap)
  {
    if (MapConfig::getMe().getMapCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> map :" << v << "未在 Table_Map.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecManualItems)
  {
    if (ItemConfig::getMe().getItemCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> manual -> item:" << v << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }
  for (auto &v : m_stNewRoleCFG.vecManualNpcs)
  {
    if (NpcConfig::getMe().getNpcCFG(v) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] NewRole -> manual -> npc :" << v << "未在 Table_Npc.txt 表中找到" << XEND;
    }
  }

  // system
  if (TableManager::getMe().getMailCFG(m_stSystemCFG.dwDebtMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] System -> debt_mail_id :" << m_stSystemCFG.dwDebtMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }

  // pet
  for (auto &s : m_stPetCFG.setArcherPartnerID)
  {
    if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
    {
      bCorrect = false;
      XERR << "[ServerGame] Pet -> archer_partner :" << s << "未在 Table_Npc.txt 表中找到" << XEND;
    }
  }
  for (auto &s : m_stPetCFG.setMerchantPartnerID)
  {
    if (NpcConfig::getMe().getNpcCFG(s) == nullptr)
    {
      bCorrect = false;
      XERR << "[ServerGame] Pet -> merchant_partner :" << s << "未在 Table_Npc.txt 表中找到" << XEND;
    }
  }
  for (auto &m : m_stPetCFG.mapBarrowEquipNpc)
  {
    if (ItemConfig::getMe().getItemCFG(m.first) == nullptr)
    {
      bCorrect = false;
      XERR << "[ServerGame] Pet -> merchant_barrow equipid :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
    }
    if (NpcConfig::getMe().getNpcCFG(m.second) == nullptr)
    {
      bCorrect = false;
      XERR << "[ServerGame] Pet -> merchant_barrow npc:" << m.second << "未在 Table_Npc.txt 表中找到" << XEND;
    }
  }

  // pet
  for (auto &m : m_stPetCFG.mapID2SkillReset)
  {
    if (ItemConfig::getMe().getItemCFG(m.first) == nullptr)
    {
      bCorrect = false;
      XERR << "[ServerGame] Pet -> pet_skill :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
      continue;
    }
  }

  // manual
  if (TableManager::getMe().getMailCFG(m_stManualCFG.dwHeadReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> head_item_return_mail" << m_stManualCFG.dwHeadReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }
  if (TableManager::getMe().getMe().getMailCFG(m_stManualCFG.dwCardReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> card_item_return_mail" << m_stManualCFG.dwCardReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }
  if (TableManager::getMe().getMailCFG(m_stManualCFG.dwHeadUnlockReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> head_unlock_return_mail" << m_stManualCFG.dwHeadUnlockReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }
  if (TableManager::getMe().getMailCFG(m_stManualCFG.dwCardUnlockReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> card_unlock_return_mail" << m_stManualCFG.dwCardUnlockReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }
  if (TableManager::getMe().getMailCFG(m_stManualCFG.dwLevelReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> level_return_mail" << m_stManualCFG.dwCardUnlockReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }
  if (TableManager::getMe().getMailCFG(m_stManualCFG.dwQualityReturnMailID) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Manual -> quality_return_mail" << m_stManualCFG.dwQualityReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
  }

  // lottery
  if (ItemConfig::getMe().getItemCFG(m_stLotteryCFG.oEnchantTrans.id()) == nullptr)
  {
    bCorrect = false;
    XERR << "[ServerGame] Lottery -> TransferCost" << m_stLotteryCFG.oEnchantTrans.ShortDebugString() << "未在 Table_Item.txt 表中找到" << XEND;
  }
  for (auto &m : m_stLotteryCFG.mapRepairItem)
  {
    if (ItemConfig::getMe().getItemCFG(m.first) == nullptr)
    {
      bCorrect = false;
      XERR << "[GameConfig] Lottery -> repair_material -> itemid :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
    }
  }

  return bCorrect;
}

// equip decompose
/*const SEquipDecomposeCFG* MiscConfig::getEquipDecomposeCFG(EQualityType eQuality, EItemType eType) const
{
  auto v = find_if(m_vecEquipDecomposeCFG.begin(), m_vecEquipDecomposeCFG.end(), [eQuality, eType](const SEquipDecomposeCFG& r) -> bool{
    return r.eQuality == eQuality && r.eType == eType;
  });
  if (v == m_vecEquipDecomposeCFG.end())
    return nullptr;

  return &(*v);
}*/

bool SAuguryCFG::inTime(DWORD curSec) const
{
  if (dwStartTime == 0 && dwEndTime == 0)
    return false;
  if (dwEndTime <= dwStartTime)
    return false;
  if (curSec >= dwStartTime && curSec <= dwEndTime)
    return true;
  return false;
}

// card
bool MiscConfig::canEquip(DWORD position, EItemType eType)
{
  auto v = find_if(m_stCardCFG.vecCardPosition.begin(), m_stCardCFG.vecCardPosition.end(), [position](const SCardPositionCFG& r) -> bool{
    return r.dwPosition == position;
  });
  if (v == m_stCardCFG.vecCardPosition.end())
    return false;

  auto o = find(v->vecItemType.begin(), v->vecItemType.end(), eType);
  return o != v->vecItemType.end();
}

DWORD MiscConfig::getRefineRate(DWORD itemid)
{
  auto v = find_if(m_vecItem2Rate.begin(), m_vecItem2Rate.end(), [itemid](const pair<DWORD, DWORD>& r) -> bool
  {
    return r.first == itemid;
  });

  return v == m_vecItem2Rate.end() ? 0 : v->second;
}

bool MiscConfig::loadObscenceLanguageConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/ObscenceLanguage.txt"))
  {
    XERR << "[屏蔽字配置], 加载配置ObscenseLanguage.txt失败" << XEND;
    return false;
  }

  xLuaData table;
  xLuaTable::getMe().getLuaData("ObscenceLanguage", table);
  for (auto m = table.m_table.begin(); m != table.m_table.end(); ++m)
  {
    if (m->first == "Chat")
    {
      auto chatf = [&](const string& key, xLuaData& data)
      {
        m_stSystemCFG.vecForbidWord.push_back(data.getString());
      };
      m->second.foreach(chatf);
    }
    else if (m->first == "Name")
    {
      auto namef = [&](const string& key, xLuaData& data)
      {
        m_stSystemCFG.vecForbidName.push_back(data.getString());
      };
      m->second.foreach(namef);
    }
    else if (m->first == "NameExclude")
    {
      m_stSystemCFG.vecForbidSymbol.clear();
      auto nameexf = [&](const string& str, xLuaData& data)
      {
        m_stSystemCFG.vecForbidSymbol.push_back(data.getString());
      };
      m->second.foreach(nameexf);
    }
  }

  if (bCorrect)
    XLOG << "[屏蔽字配置] 成功加载ObscenseLanguage.txt配置" << XEND;
  return bCorrect;
}

void MiscConfig::formatMvpInfo(EMvpScoreType eType, TVecString& param, const SMvpKillInfo& info, bool bShowHighest) const
{
  param.push_back("·");
  std::stringstream str;
  str.str("");
  str << m_stMvpScoreCFG.getNameByType(eType);
  switch(eType)
  {
    case EMVPSCORETYPE_DAMAGE:
      str << " ";
      param.push_back(str.str());
      str.str("");
      str << info.m_dwHitDamage;
      param.push_back(str.str());
      if (bShowHighest)
      {
        param.push_back("[" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_HIGHEST) + "]");
      }
      break;
    case EMVPSCORETYPE_HEAL:
      str << " ";
      param.push_back(str.str());
      str.str("");
      str << info.m_dwHealHp;
      param.push_back(str.str());
      if (bShowHighest)
      {
        param.push_back("[" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_HIGHEST) + "]");
      }
      break;
    case EMVPSCORETYPE_RELIVE:
      str << " ";
      param.push_back(str.str());
      str.str("");
      str << info.m_dwReliveOtherTimes;
      param.push_back(str.str());
      param.push_back(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_PERSON));
      if (bShowHighest)
      {
        param.push_back("[" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_HIGHEST) + "]");
      }
      break;
    case EMVPSCORETYPE_BELOCK:
      str << " ";
      param.push_back(str.str());
      str.str("");
      char buf[20];
      sprintf(buf, "%.1f", (float)info.m_qwBeLockTime / 1000.0f);
      str << buf;
      param.push_back(str.str());
      param.push_back(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_SECOND));
      if (bShowHighest)
      {
        param.push_back("[" + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_HIGHEST) + "]");
      }
      break;
    case EMVPSCORETYPE_DEADHIT:
    case EMVPSCORETYPE_FIRSTDAM:
    case EMVPSCORETYPE_TOPDAMAGE:
      param.push_back(str.str());
      param.push_back(" " + SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_REACH));
      break;
    case EMVPSCORETYPE_BREAKSKILL:
      param.push_back(str.str());
      break;
    case EMVPSCORETYPE_MIN:
    case EMVPSCORETYPE_MAX:
      break;
    default:
      break;
  }
  param.push_back("\n");
}

SCelebrationMCardCFG* MiscConfig::getCelebrationMCardCFGbyID(ECelebrationLevel type)
{
  auto it = m_stMonthCardActivityCfg.m_mapCelebrationMCardCfg.find(type);
  if (it == m_stMonthCardActivityCfg.m_mapCelebrationMCardCfg.end())
    return nullptr;

  return &(it->second);
}

bool MiscConfig::loadServerGameConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/ServerGame.lua"))
  {
    XERR << "[ServerGame.lua-加载], 加载失败" << XEND;
    return false;
  }
  xLuaData table;
  xLuaTable::getMe().getLuaData("ServerGame", table);

  for (auto m = table.m_table.begin(); m != table.m_table.end(); ++m)
  {
    if (m->first == "MvpScore")
    {
      m_stMvpScoreCFG.clear();

      m_stMvpScoreCFG.dwValidTime = m->second.getTableInt("validTime");
      m_stMvpScoreCFG.dwDeadNoticeTime = m->second.getTableInt("noticeTime");

      DWORD type = 0;
      auto getnumber = [&](const string& key, xLuaData& d)
      {
        if (type == 1)
          m_stMvpScoreCFG.vecDamScore.push_back(d.getInt());
        else if (type == 2)
          m_stMvpScoreCFG.vecBeLockScore.push_back(d.getInt());
        else if (type == 3)
          m_stMvpScoreCFG.vecHealScore.push_back(d.getInt());
        else if (type == 4)
          m_stMvpScoreCFG.vecRebirthScore.push_back(d.getInt());
        else if (type == 5)
          m_stMvpScoreCFG.vecDeadHitScore.push_back(d.getInt());
      };
      xLuaData& dam = m->second.getMutableData("damageScore");
      type = 1;
      dam.foreach(getnumber);
      xLuaData& lock = m->second.getMutableData("beLockScore");
      type = 2;
      lock.foreach(getnumber);
      xLuaData& heal = m->second.getMutableData("healScore");
      type = 3;
      heal.foreach(getnumber);
      xLuaData& rebirth = m->second.getMutableData("rebirthScore");
      type = 4;
      rebirth.foreach(getnumber);
      xLuaData& deadhit = m->second.getMutableData("deadHitScore");
      type = 5;
      deadhit.foreach(getnumber);

      auto sortmax = [&](TVecDWORD& vec)
      {
        std::sort(vec.begin(), vec.end(), [&](const DWORD& d1, const DWORD& d2) -> bool
        {
          return d1 > d2;
        });
      };
      sortmax(m_stMvpScoreCFG.vecDamScore);
      sortmax(m_stMvpScoreCFG.vecBeLockScore);
      sortmax(m_stMvpScoreCFG.vecHealScore);
      sortmax(m_stMvpScoreCFG.vecRebirthScore);
      sortmax(m_stMvpScoreCFG.vecDeadHitScore);

      m_stMvpScoreCFG.dwFirstHitScore = m->second.getTableInt("firstHitScore");
      m_stMvpScoreCFG.dwDeadHitTime = m->second.getTableInt("deadHitTime");

      xLuaData& namedata = m->second.getMutableData("showNames");
      m_stMvpScoreCFG.mapType2NameRank.clear();

      map<EMvpScoreType, const char*> mapReadCFG;
      mapReadCFG[EMVPSCORETYPE_DAMAGE] = "damage";
      mapReadCFG[EMVPSCORETYPE_BELOCK] = "belock";
      mapReadCFG[EMVPSCORETYPE_HEAL] = "heal";
      mapReadCFG[EMVPSCORETYPE_RELIVE] = "rebirth";
      mapReadCFG[EMVPSCORETYPE_DEADHIT] = "deadhit";
      mapReadCFG[EMVPSCORETYPE_FIRSTDAM] = "firsthit";
      mapReadCFG[EMVPSCORETYPE_BREAKSKILL] = "breakskill";
      mapReadCFG[EMVPSCORETYPE_TOPDAMAGE] = "top1damage";

      auto getcfg = [&](EMvpScoreType eType){
        auto it = mapReadCFG.find(eType);
        if (it == mapReadCFG.end())
          return;
        pair<string, DWORD> &pairName2Rank = m_stMvpScoreCFG.mapType2NameRank[eType];
        pairName2Rank.first = namedata.getMutableData(it->second).getTableString("name");
        pairName2Rank.second = namedata.getMutableData(it->second).getTableInt("show_order");
      };
      for (size_t i = EMVPSCORETYPE_MIN + 1; i < EMVPSCORETYPE_MAX; ++i)
      {
        getcfg((EMvpScoreType)i);
      }

      auto getdec = [&](const string& key, xLuaData& d)
      {
        DWORD per = d.getTableInt("per");
        DWORD dec = d.getTableInt("dec");
        m_stMvpScoreCFG.vecDamDecScore.push_back(std::make_pair(per, dec));
      };
      m->second.getMutableData("damageDecScore").foreach(getdec);
      std::sort(m_stMvpScoreCFG.vecDamDecScore.begin(), m_stMvpScoreCFG.vecDamDecScore.end(), [&](const pair<DWORD, DWORD>& r1, const pair<DWORD, DWORD>& r2) ->bool {
        return r1.first < r2.first;
      });

      m_stMvpScoreCFG.vecRewardRatio.clear();
      xLuaData& ratiod = m->second.getMutableData("mvpRatio");
      float ratio = ratiod.getTableFloat("1");
      if (ratio) m_stMvpScoreCFG.vecRewardRatio.push_back(ratio);
      ratio = ratiod.getTableFloat("2");
      if (ratio) m_stMvpScoreCFG.vecRewardRatio.push_back(ratio);
      ratio = ratiod.getTableFloat("3");
      if (ratio) m_stMvpScoreCFG.vecRewardRatio.push_back(ratio);
      ratio = ratiod.getTableFloat("4");
      if (ratio) m_stMvpScoreCFG.vecRewardRatio.push_back(ratio);
      ratio = ratiod.getTableFloat("5");
      if (ratio) m_stMvpScoreCFG.vecRewardRatio.push_back(ratio);

      m_stMvpScoreCFG.fMinifirstRatio = m->second.getTableFloat("miniFirstDamRatio");
      m_stMvpScoreCFG.fMiniDamMaxRatio = m->second.getTableFloat("miniMaxDamRatio");
    }
    else if (m->first == "OperateReward")
    {
      string expire = m->second.getTableString("expiretime");
      parseTime(expire.c_str(), m_operRewardMiscCfg.expireTime);
    }
    else if (m->first == "Item")
    {
      m_stItemCFG.dwKapulaMapItem = m->second.getTableInt("kapula_map_item");
      m_stItemCFG.dwKapulaStoreItem = m->second.getTableInt("kapula_store_item");
      m_stItemCFG.dwUpgradeRefineLvDec = m->second.getTableInt("equip_upgrade_refinelv_dec");
      m_stItemCFG.dwTempPackOverTimeCount = m->second.getTableInt("temp_pack_overtime_itemcount");
      DWORD dwMode = m->second.getTableInt("item_pickup_mode");
      if (dwMode >= EPICKUPMODE_CLIENT && dwMode <= EPICKUPMODE_SERVER)
      {
        m_stItemCFG.ePickupMode = static_cast<EPickupMode>(dwMode);
        XLOG << "[ServerGame] pickupmode :" << m_stItemCFG.ePickupMode << XEND;
      }
      xLuaData& hint = m->second.getMutableData("extra_hint_item");
      auto hintf = [&](const string& key, xLuaData& data)
      {
        m_stItemCFG.setExtraHint.insert(data.getInt());
      };
      hint.foreach(hintf);
      auto machinef1 = [&](const string& key, xLuaData& data)
      {
        m_stItemCFG.setNoColorMachineMount.insert(data.getInt());
      };
      m->second.getMutableData("no_color_machine_mount").foreach(machinef1);
      auto machinef2 = [&](const string& key, xLuaData& data)
      {
        m_stItemCFG.setMachineMount.insert(data.getInt());
      };
      m->second.getMutableData("machine_mount").foreach(machinef2);
    }
    else if (m->first == "Decompose")
    {
      auto decomposef = [&](const string& key, xLuaData& data)
      {
        m_stItemCFG.mapDecomposePrice[atoi(key.c_str())] = data.getInt();
      };
      m->second.getMutableData("price").foreach(decomposef);
    }
    else if (m->first == "Credit")
    {
      m_stCreditCFG.dwDefaultValue = m->second.getTableInt("default_value");
      m_stCreditCFG.dwMaxLimitValue = m->second.getTableInt("max_value");
      m_stCreditCFG.dwDecLimitValue = m->second.getTableInt("dec_limit_value");
      m_stCreditCFG.dwDayDecValue = m->second.getTableInt("dec_day_value");
      m_stCreditCFG.dwMonsterValue = m->second.getTableInt("monster_value");
      m_stCreditCFG.dwMaxMstValue = m->second.getTableInt("max_monster_value");
      m_stCreditCFG.dwMiniValue = m->second.getTableInt("mini_value");
      m_stCreditCFG.dwMvpValue = m->second.getTableInt("mvp_value");
      m_stCreditCFG.dwWQuestValue = m->second.getTableInt("wantedquest_value");
      m_stCreditCFG.dwWQuestTimes = m->second.getTableInt("wantedquest_times");
      m_stCreditCFG.dwSealValue = m->second.getTableInt("seal_value");
      m_stCreditCFG.dwSealTimes = m->second.getTableInt("seal_times");
      m_stCreditCFG.dwChargeRatio = m->second.getTableInt("charge_ratio");
      m_stCreditCFG.dwBuyRatio = m->second.getTableInt("buy_ratio");
      m_stCreditCFG.dwAddInterval = m->second.getTableInt("add_interval");
      m_stCreditCFG.dwIntervalValue = m->second.getTableInt("interval_value");
      m_stCreditCFG.dwFirstBadValue = m->second.getTableInt("first_bad_value");
      m_stCreditCFG.dwSecondBadValue = m->second.getTableInt("second_bad_value");
      m_stCreditCFG.dwBadInterval = m->second.getTableInt("bad_interval");
      m_stCreditCFG.dwRepeatValue = m->second.getTableInt("repeat_dec_value");
      m_stCreditCFG.dwRepeatInterval = m->second.getTableInt("repeat_interval");
      m_stCreditCFG.iValueLimitPersonal = m->second.getTableInt("value_limit_personal");
      m_stCreditCFG.iValueForbid = m->second.getTableInt("value_forbid");
      m_stCreditCFG.dwTimeForbid = m->second.getTableInt("time_forbid");
      m_stCreditCFG.dwChatSaveCharCount = m->second.getTableInt("chat_save_char_count");
      m_stCreditCFG.dwChatSaveMaxCount = m->second.getTableInt("chat_save_max_count");
      m_stCreditCFG.dwChatCalcMaxCount = m->second.getTableInt("chat_calc_max_count");
      if (m_stCreditCFG.dwChatCalcMaxCount <= 0)
        m_stCreditCFG.dwChatCalcMaxCount = 50;

      auto getchannel = [&](const string& key, xLuaData& d)
      {
        m_stCreditCFG.setChannels.insert(d.getInt());
      };
      m_stCreditCFG.setChannels.clear();
      m->second.getMutableData("check_channel").foreach(getchannel);
      m_stCreditCFG.bPunish = m->second.getTableInt("switch_punish");
      m_stCreditCFG.iBlack = m->second.getTableInt("black_value");
      auto getstrs = [&](const string& key, xLuaData& d)
      {
        m_stCreditCFG.setBadStrs.insert(d.getString());
      };
      m_stCreditCFG.setBadStrs.clear();
      m->second.getMutableData("bad_strs").foreach(getstrs);
    }
    else if (m->first == "MoneyCat")
    {
      m_stMoneyCatCFG.dwMaxMoney = m->second.getTableInt("max_money");
      auto getmoney = [&](const string& key, xLuaData& d)
      {
        DWORD step = d.getTableInt("step");
        pair<DWORD, DWORD>& pa = m_stMoneyCatCFG.mapStep2Money[step];
        pa.first = d.getTableInt("min_money");
        pa.second = d.getTableInt("max_money");
      };
      m_stMoneyCatCFG.mapStep2Money.clear();
      m->second.getMutableData("rand_money").foreach(getmoney);
    }
    else if (m->first == "RefineChangeRate")
    {
      m_stRefineCFG.dwBeginChangeRateLv = m->second.getTableInt("begin_change_lv");
      m_stRefineCFG.fLastSuccessDecRate = m->second.getTableFloat("last_success_dec");
      m_stRefineCFG.fLastFailAddRate = m->second.getTableFloat("last_fail_add");
      m_stRefineCFG.fRepairPerRate = m->second.getTableFloat("repair_per_add");
    }
    else if (m->first == "System")
    {
      m_stSystemCFG.bValidPosCheck = m->second.getTableInt("valid_pos_check") == 1;
      m_stSystemCFG.fValidPosRadius = m->second.getTableFloat("valid_pos_radius");
      m_stSystemCFG.dwMaxExtraAddBattleTime = m->second.getTableInt("max_extra_add_battletime");
      m_stSystemCFG.dwMaxWeaponPetNum = m->second.getTableInt("max_mercenary_cat_num");
      m_stSystemCFG.dwDebtMailID = m->second.getTableInt("debt_mail_id");
      m_stSystemCFG.dwSellLimitWarning = m->second.getTableInt("sell_limit_warning");
      if (m_stSystemCFG.dwSellLimitWarning == 0)
        m_stSystemCFG.dwSellLimitWarning = 1000000;
      m_stSystemCFG.dwNpcFadeoutTime = m->second.getTableInt("npc_fade_out_time");
      m_stSystemCFG.dwKillMonsterPeriod = m->second.getTableInt("kill_monster_period");
      m_stSystemCFG.dwShopRandomCnt = m->second.getTableInt("shop_random_cnt");
      m_stSystemCFG.dwOpenTowerLayer = m->second.getTableInt("open_tower_layer");
      if (m->second.has("max_seat_dis"))
        m_stSystemCFG.fMaxSeatDis = m->second.getTableFloat("max_seat_dis");
      m_stSystemCFG.m_mapMapID2MaxScopeNum.clear();
      if (m->second.has("map_max_scope_num"))
      {
        auto getnum = [&](const string& k, xLuaData& d)
        {
          DWORD mapid = atoi(k.c_str());
          DWORD num = d.getInt();
          m_stSystemCFG.m_mapMapID2MaxScopeNum[mapid] = num;
        };
        m->second.getMutableData("map_max_scope_num").foreach(getnum);
      }
      m_stSystemCFG.m_mapMapID2MaxScopeFriendNum.clear();
      if (m->second.has("map_max_scope_friend_num"))
      {
        auto getnum = [&](const string& k, xLuaData& d)
        {
          DWORD mapid = atoi(k.c_str());
          DWORD num = d.getInt();
          m_stSystemCFG.m_mapMapID2MaxScopeFriendNum[mapid] = num;
        };
        m->second.getMutableData("map_max_scope_friend_num").foreach(getnum);
      }
      m_stSystemCFG.dwStatKillNum = m->second.getTableInt("monster_kill_statnum");
    }
    else if (m->first == "ItemCheckDel")
    {
      auto getdata = [&](const string& key, xLuaData& d)
      {
        DWORD id = d.getTableInt("id");
        SItemCheckDel& cfg = m_stItemCheckDelCFG.mapItem2DelInfo[id];
        const string& stype = d.getTableString("type");
        if (stype == "Level")
          cfg.eType = EITEMCHECKDEL_LEVEL;
        cfg.param1 = d.getTableInt("param");
      };
      m->second.foreach(getdata);
    }
    else if (m->first == "Skill")
    {
      xLuaData& manual = m->second.getMutableData("manual_action_skill");
      m_stSkillCFG.pairSkillAction.first = manual.getTableInt("1");
      m_stSkillCFG.pairSkillAction.second = manual.getTableInt("2");
      auto skillargf = [&](const string& key, xLuaData& data)
      {
        ESkillType stype = SkillConfig::getMe().getSkillType(key);
        if (stype > ESKILLTYPE_MIN && stype < ESKILLTYPE_MAX)
        {
          auto bufff = [&](const string& k, xLuaData& d)
          {
            if (d.getInt())
              m_stSkillCFG.mapSkillType2CFG[stype].setBuff.insert(d.getInt());
          };
          data.getMutableData("buff").foreach(bufff);
        }
      };
      m_stSkillCFG.mapSkillType2CFG.clear();
      m->second.getMutableData("skillarg").foreach(skillargf);
      auto ensemblef = [&](const string& key, xLuaData& data)
      {
        DWORD tb = atoi(key.c_str());
        if (!tb)
          return;
        auto skillf = [&](const string& k, xLuaData& d)
        {
          if (d.getInt())
            m_stSkillCFG.mapTypeBranch2SkillID[tb].insert(d.getInt());
        };
        data.foreach(skillf);
      };
      m_stSkillCFG.mapTypeBranch2SkillID.clear();
      m->second.getMutableData("ensemble_skill").foreach(ensemblef);
    }
    else if (m->first == "Courier")
    {
      m_stTradeCFG.dwMaleDialog = m->second.getTableInt("male_dialog");
      m_stTradeCFG.dwFemaleDialog = m->second.getTableInt("female_dialog");
      m_stTradeCFG.dwDialogInterval = m->second.getTableInt("dialog_interval");
      m_stTradeCFG.dwSignDialog = m->second.getTableInt("Sign_dialog");
      m_stTradeCFG.dwRefuseDialog = m->second.getTableInt("Refuse_diaglog");
      m_stTradeCFG.dwFollowTime = m->second.getTableInt("Follow_time");
    }
    else if (m->first == "Pet")
    {
      m_stPetCFG.setArcherPartnerID.clear();
      auto archerf = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.setArcherPartnerID.insert(data.getInt());
      };
      m->second.getMutableData("archer_partner").foreach(archerf);
      m_stPetCFG.setMerchantPartnerID.clear();
      auto merchantf = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.setMerchantPartnerID.insert(data.getInt());
      };
      m->second.getMutableData("merchant_partner").foreach(merchantf);
      auto barrowf = [&](const string& key, xLuaData& data)
      {
        DWORD dwEquipID = atoi(key.c_str());
        m_stPetCFG.mapBarrowEquipNpc[dwEquipID] = data.getInt();
      };
      m->second.getMutableData("merchant_barrow").foreach(barrowf);

      m_stPetCFG.dwUserPetTimeTick = m->second.getTableInt("userpet_tick");
      m_stPetCFG.dwUserPetHapplyGift = m->second.getTableInt("userpet_happly_gift");
      m_stPetCFG.dwUserPetHapplyFriend = m->second.getTableInt("userpet_happly_friend");
      m_stPetCFG.dwUserPetExciteGift = m->second.getTableInt("userpet_excite_gift");
      m_stPetCFG.dwUserPetExciteFriend = m->second.getTableInt("userpet_excite_friend");
      m_stPetCFG.dwUserPetHappinessGift = m->second.getTableInt("userpet_happiness_gift");
      m_stPetCFG.dwUserPetHappinessFriend = m->second.getTableInt("userpet_happiness_friend");

      m_stPetCFG.dwUserPetTouchTime = m->second.getTableInt("userpet_touch_time");
      m_stPetCFG.dwUserPetTouchFriend = m->second.getTableInt("userpet_touch_friend");
      m_stPetCFG.dwUserPetTouchPerDay = m->second.getTableInt("userpet_touch_perday");
      m_stPetCFG.dwUserPetTouchTick = m->second.getTableInt("userpet_touch_tick");
      m_stPetCFG.dwUserPetTouchEffectID = m->second.getTableInt("userpet_touch_effectid");

      m_stPetCFG.dwUserPetFeedTime = m->second.getTableInt("userpet_feed_time");
      m_stPetCFG.dwUserPetFeedFriend = m->second.getTableInt("userpet_feed_friend");
      m_stPetCFG.dwUserPetFeedPerDay = m->second.getTableInt("userpet_feed_perday");
      m_stPetCFG.dwUserPetFeedTick = m->second.getTableInt("userpet_feed_tick");
      m_stPetCFG.dwUserPetFeedEffectID = m->second.getTableInt("userpet_feed_effectid");

      m_stPetCFG.dwUserPetGiftTime = m->second.getTableInt("userpet_gift_time");
      m_stPetCFG.dwUserPetGiftPerDay = m->second.getTableInt("userpet_gift_perday");
      m_stPetCFG.dwUserPetGiftFriend = m->second.getTableInt("userpet_gift_friend");
      m_stPetCFG.dwUserPetGiftMaxValue = m->second.getTableInt("userpet_gift_maxvalue");
      m_stPetCFG.dwUserPetGiftMonthCard = m->second.getTableInt("userpet_gift_monthcard");
      m_stPetCFG.dwUserPetGiftMonthCardItem = m->second.getTableInt("userpet_gift_monthcard_item");
      if (m_stPetCFG.dwUserPetGiftMonthCard == 0)
        m_stPetCFG.dwUserPetGiftMonthCard = 1;

      m_stPetCFG.dwTransformEggBuff = m->second.getTableInt("transformEggBuff");
      //m_stPetCFG.dwMaxCatchCartoonTime = m->second.getTableInt("maxCatchCartoonTime");
      m_stPetCFG.dwMaxCatchTime = m->second.getTableInt("maxCatchTime");
      m_stPetCFG.dwMaxCatchDistance = m->second.getTableInt("maxCatchPetDistance");
      m_stPetCFG.dwNoOperationNoticeTime = m->second.getTableInt("noOperationNoticeTime");
      m_stPetCFG.dwCatchSkill = m->second.getTableInt("catchSkill");
      m_stPetCFG.dwCatchSkillPlayTime = m->second.getTableInt("catchSkillPlayTime");
      m_stPetCFG.dwTransformTime = m->second.getTableInt("transformTime");
      m_stPetCFG.dwEatFoodSkill = m->second.getTableInt("eatFoodSkill");
      m_stPetCFG.dwOverUserLv = m->second.getTableInt("overUserLevel");
      m_stPetCFG.dwBirthRange = m->second.getTableInt("birthRange");
      m_stPetCFG.dwCatchLineID = m->second.getTableInt("catchLineID");
      m_stPetCFG.dwPetReliveTime = m->second.getTableInt("reliveTime");
      m_stPetCFG.dwHatchFadeInTime = m->second.getTableInt("hatchFadeIn");
      m_stPetCFG.dwHatchFadeOutTime = m->second.getTableInt("hatchFadeOut");

      xLuaData& adventure_area = m->second.getMutableData("adventure_default_area");
      auto adventure_areaf = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.setDefaultAdventureArea.insert(data.getInt());
      };
      adventure_area.foreach(adventure_areaf);

      xLuaData& reset_skill = m->second.getMutableData("reset_skill");
      auto reset_skillf = [&](const string& key, xLuaData& data)
      {
        map<DWORD, DWORD>& mapSkill = m_stPetCFG.mapID2SkillReset[atoi(key.c_str())];
        auto dataf = [&](const string& key, xLuaData& d)
        {
          mapSkill[atoi(key.c_str())] = d.getInt();
        };
        data.foreach(dataf);
      };
      reset_skill.foreach(reset_skillf);

      for (auto &m : m_stPetCFG.mapID2SkillReset)
      {
        DWORD dwLast = 0;
        for (auto &item : m.second)
        {
          item.second += dwLast;
          dwLast = item.second;
          XDBG << "[ServerGame] itemid :" << m.first << "skillid :" << item.first << "rate :" << item.second << XEND;
        }
      }
      m_stPetCFG.mapOldEquip2NewUseItem.clear();
      auto transitem = [&](const string& key, xLuaData& data)
      {
        m_stPetCFG.mapOldEquip2NewUseItem[data.getTableInt("1")] = data.getTableInt("2");
      };
      m->second.getMutableData("equip_to_useitem").foreach(transitem);
    }
    else if (m->first == "MercenaryCat")
    {
      m_stWeaponPetCFG.dwHelpTeamerDis = m->second.getTableInt("help_Teamer_dis");
      m_stWeaponPetCFG.dwOffOwnerToBattleTime = m->second.getTableInt("leave_owner_time");
      m_stWeaponPetCFG.dwOffTeamerToBattleTime = m->second.getTableInt("leave_teamer_time");
    }
    else if(m->first == "Arena")
    {
       m_stArenaCFG.dwBeKilledScore = m->second.getTableInt("be_killed_score");
       m_stArenaCFG.dwKillScore = m->second.getTableInt("kill_score");
       m_stArenaCFG.dwHelpScore = m->second.getTableInt("help_kill_score");
       m_stArenaCFG.dwComboScore = m->second.getTableInt("combo_kill_score");
       m_stArenaCFG.dwBraverScore = m->second.getTableInt("braver_kill_score");
       m_stArenaCFG.dwSaviorScore = m->second.getTableInt("savior_kill_score");
       m_stArenaCFG.dwHelpTime = m->second.getTableInt("help_kill_time");
       m_stArenaCFG.dwComboTime = m->second.getTableInt("combo_kill_time");
       m_stArenaCFG.dwBraverHP = m->second.getTableInt("braver_hp_percent");
       m_stArenaCFG.dwSaviorHP = m->second.getTableInt("savior_hp_percent");
       m_stArenaCFG.dwOriginScore = m->second.getTableInt("origin_score");

       m_stArenaCFG.dwMetalNpcID = m->second.getTableInt("metal_npcid");
    }
    else if (m->first == "GuildRaid")
    {
      m_stGuildRaidCFG.dwBossMapNum = m->second.getTableInt("boss_num");
      /*xLuaData& npcd = m->second.getMutableData("npc_index");
      auto getnpcindex = [&](const string& key, xLuaData& data)
      {
        DWORD npcid = data.getTableInt("npcid");
        DWORD index = data.getTableInt("index");
        if (npcid == 0 || index == 0)
        {
          bCorrect = false;
          XERR << "[ServerGame-配置错误], GuildRaid, npcid与index不可为0" << XEND;
          return;
        }
        m_stGuildRaidCFG.mapNpcID2Index[npcid] = index;
      };
      npcd.foreach(getnpcindex);
      */

      DWORD mindepth = m->second.getMutableData("map_depth").getTableInt("min");
      DWORD maxdepth = m->second.getMutableData("map_depth").getTableInt("max");
      if (mindepth == 0|| maxdepth == 0 || maxdepth < mindepth)
      {
        XERR << "[ServerGame-配置错误],GuildRaid, map_depth配置错误" << XEND;
        bCorrect = false;
      }
      m_stGuildRaidCFG.pairMapDepth.first = mindepth;
      m_stGuildRaidCFG.pairMapDepth.second = maxdepth;

      DWORD mapminnum = m->second.getMutableData("map_num").getTableInt("min");
      DWORD mapmaxnum = m->second.getMutableData("map_num").getTableInt("max");
      if (mapminnum == 0 || mapmaxnum == 0 || mapmaxnum < mapminnum)
      {
        XERR << "[ServerGame-配置错误],GuildRaid, map_num配置错误" << XEND;
        bCorrect = false;
      }
      m_stGuildRaidCFG.pairMapNumRange.first = mapminnum;
      m_stGuildRaidCFG.pairMapNumRange.second = mapmaxnum;

      DWORD unsteady_time = m->second.getTableInt("unsteady_time");
      if (unsteady_time == 0)
      {
        XERR << "[ServerGame-配置错误],GuildRaid, unsteady_time配置错误" << XEND;
        bCorrect = false;
      }
      m_stGuildRaidCFG.unsteady_time = unsteady_time;

      m_stGuildRaidCFG.fRewardRatio = m->second.getTableFloat("reward_ratio");
      if (m_stGuildRaidCFG.fRewardRatio == 0)
      {
        XERR << "[ServerGame-配置错误], GUildRaid, reward_ratio为0";
        bCorrect = false;
      }
    }
    else if (m->first == "Guild")
    {
      m_stGuildCFG.dwQuestClearTime = m->second.getTableInt("npc_clear_time");
      m_stGuildCFG.dwQuestDefaultCount = m->second.getTableInt("default_npc_count");
      m_stGuildCFG.dwQuestProtectTime = m->second.getTableInt("quest_protect_time");
      m_stGuildCFG.dwMaintenanceProtectTime = m->second.getTableInt("maintenance_protect_lv");
      if (m_stGuildCFG.dwMaintenanceProtectTime == 0)
        m_stGuildCFG.dwMaintenanceProtectTime = 1;

      xLuaData& icon = m->second.getMutableData("newicon_ntf_lv");
      m_stGuildCFG.pairIconNtfLv.first = icon.getTableInt("1");
      m_stGuildCFG.pairIconNtfLv.second = icon.getTableInt("2");

      m_stGuildCFG.dwMaxPhotoPerMember = m->second.getTableInt("photo_max_permember");
      m_stGuildCFG.dwPhotoRefreshTime = m->second.getTableInt("photo_refresh_time");
      m_stGuildCFG.dwPhotoMemberActiveDay = m->second.getTableInt("photo_member_active_day");
      m_stGuildCFG.dwMaxFramePhotoCount = m->second.getTableInt("photo_max_frame_photo");

      xLuaData& auth = m->second.getMutableData("auth_name");
      auto authf = [&](const string& key, xLuaData& data)
      {
        DWORD dwAuth = atoi(key.c_str());
        if (dwAuth <= EAUTH_MIN || dwAuth >= EAUTH_MAX)
        {
          XERR << "[ServerGame-Guild] auth_name key :" << dwAuth << "不是合法的权限值" << XEND;
          return;
        }
        m_stGuildCFG.mapAuthName[static_cast<EAuth>(dwAuth)] = data.getString();
      };
      auth.foreach(authf);

      auto framef = [&](const string& key, xLuaData& data)
      {
        m_stGuildCFG.setFrameIDs.insert(data.getInt());
      };
      m->second.getMutableData("photo_frame").foreach(framef);
    }
    else if(m->first == "ExtraReward")
    {
      auto func = [&](const string&key, xLuaData& data)
      {
        SExtraRewardCFG cfg;
        cfg.etype = static_cast<ETaskExtraRewardType>(data.getTableInt("id"));
        cfg.normalReward.dwFinishTimes = data.getMutableData("normal").getTableInt("count");
        cfg.normalReward.dwItemID = data.getMutableData("normal").getTableInt("item");
        cfg.normalReward.dwItemNum = data.getMutableData("normal").getTableInt("itemnum");
        cfg.dwLevel = data.getMutableData("lv_spec").getTableInt("lv");
        if(cfg.dwLevel != 0)
        {
          cfg.addReward.dwFinishTimes = data.getMutableData("lv_spec").getTableInt("count");
          cfg.addReward.dwItemID = data.getMutableData("lv_spec").getTableInt("item");
          cfg.addReward.dwItemNum = data.getMutableData("lv_spec").getTableInt("itemnum");
        }
        m_mapExtraRewardCFG[cfg.etype] = cfg;
      };
      m->second.foreach(func);
    }
    else if (m->first == "StatusProtect")
    {
      m_stBuffStatusCFG.mapBuffStatusData.clear();
      auto getstatus = [&](const string& key, xLuaData& data)
      {
        DWORD status = data.getTableInt("status");
        SBuffStatusData &cfg = m_stBuffStatusCFG.mapBuffStatusData[status];
        cfg.dwStatus = status;
        cfg.dwPeriod = data.getTableInt("period");
        cfg.dwMaxTime = data.getTableInt("maxtime");
      };
      m->second.foreach(getstatus);
    }
    else if (m->first == "PvpCommon")
    {
      m_stPvpCommonCFG.dwKillCoin = m->second.getTableInt("KillCoin");
      m_stPvpCommonCFG.dwHelpKillCoin = m->second.getTableInt("HelpKillCoin");
      m_stPvpCommonCFG.dwDesertWinCoin = m->second.getTableInt("DesertWinCoin");
      m_stPvpCommonCFG.dwGlamWinCoin = m->second.getTableInt("GlamWinCoin");
      m_stPvpCommonCFG.dwDayMaxCoin = m->second.getTableInt("DayMaxCoin");
      m_stPvpCommonCFG.dwWeekMaxCoin = m->second.getTableInt("WeekMaxCoin");

      m_stPvpCommonCFG.dwHpRate = m->second.getTableInt("HpRate");
      m_stPvpCommonCFG.dwDesertWinScore = m->second.getTableInt("DesertWinScore");
      m_stPvpCommonCFG.dwExtraRewardCoinNum = m->second.getMutableData("ExtraReward").getTableInt("coinnum");
      m_stPvpCommonCFG.paExtraItem2Num.first = m->second.getMutableData("ExtraReward").getTableInt("itemid");
      m_stPvpCommonCFG.paExtraItem2Num.second = m->second.getMutableData("ExtraReward").getTableInt("num");
      m_stPvpCommonCFG.dwHealRate = m->second.getTableInt("HealRate");
    }
    else if (m->first == "Achieve")
    {
      m_stAchieveCFG.dwHandTimeLimit = m->second.getTableInt("hand_time_limit");
      m_stAchieveCFG.dwMvpTimeLimit = m->second.getTableInt("mvp_time_limit");
      m_stAchieveCFG.dwDeadTimeLimit = m->second.getTableInt("dead_time_limit");
      m_stAchieveCFG.dwItemUseTimeLimit = m->second.getTableInt("item_use_interval_limit");
      auto excludef = [&](const string& key, xLuaData& data)
      {
        m_stAchieveCFG.setCollectExclude.insert(data.getInt());
      };
      m->second.getMutableData("collection_exclude").foreach(excludef);
    }
    else if(m->first == "ChatChannel")
    {
      m_stChatChannelCFG.dwRound = m->second.getTableInt("echat_channel_round");
      m_stChatChannelCFG.dwTeam = m->second.getTableInt("echat_channel_team");
      m_stChatChannelCFG.dwGuild = m->second.getTableInt("echat_channel_guild");
      m_stChatChannelCFG.dwFriend = m->second.getTableInt("echat_channel_friend");
      m_stChatChannelCFG.dwWorld = m->second.getTableInt("echat_channel_world");
      m_stChatChannelCFG.dwMap = m->second.getTableInt("echat_channel_map");
      m_stChatChannelCFG.dwSys = m->second.getTableInt("echat_channel_sys");
      m_stChatChannelCFG.dwRoom = m->second.getTableInt("echat_channel_room");
      m_stChatChannelCFG.dwBarrage = m->second.getTableInt("echat_channel_barrage");
      m_stChatChannelCFG.dwChat = m->second.getTableInt("echat_channel_chat");
    }
    else if (m->first == "Food")
    {
      m_stFood.dwSkillId = m->second.getTableInt("SkillId");
      m_stFood.dwCookerKnife = m->second.getTableInt("CookerKnife");
      m_stFood.dwCookerHat = m->second.getTableInt("CookerHat");
      m_stFood.dwDarkFood = m->second.getTableInt("DarkFood");
      // m_stFood.dwEatSkillId = m->second.getTableInt("EatSkillId");
      m_stFood.fSatietyRate = m->second.getTableFloat("SatietyRate");
      m_stFood.dwCookMeterialExpPerLv = m->second.getTableInt("CookMeterialExpPerLv");
      m_stFood.dwMaxCookMeterialLv = m->second.getTableInt("MaxCookMeterialLv");
      m_stFood.dwTaserFoodExpPerLv = m->second.getTableInt("TaserFoodExpPerLv");
      m_stFood.dwFoodNpcDuration = m->second.getTableInt("FoodNpcDuration");
      m_stFood.dwMaxPutFoodCount = m->second.getTableInt("MaxPutFoodCount");
      m_stFood.dwMaxPutMatCount = m->second.getTableInt("MaxPutMatCount");
      m_stFood.dwChristmasCake = m->second.getTableInt("ChristmasCake");
    }
    else if (m->first == "TimerTableList")
    {
      m_stTimerTableCFG.vecTables.clear();
      auto mf = [this](const string& key, xLuaData& data)
      {
        m_stTimerTableCFG.vecTables.push_back(data.getTableString("key"));
      };
      m->second.foreach(mf);
    }
    else if (m->first == "TradeBlack")
    {
      m_stTradeBlack.m_setAccid.clear();
      auto func = [this](const string& key, xLuaData& data)
      {        
        m_stTradeBlack.m_setAccid.insert(data.getInt());
        XLOG << "[交易-黑名单] 加载 accid" << data.getInt() << XEND;
      };
      stringstream ss;
      ss << BaseConfig::getMe().getBranch();
      XLOG << "[交易-黑名单] 加载 分支id" << ss.str() << XEND;
      string strBranchdId = ss.str();
      m->second.getMutableData(strBranchdId.c_str()).foreach(func);
    }
    else if (m->first == "QuestTableList")
    {
      m_stQuestTableCFG.vecTables.clear();
      auto mf = [this](const string& key, xLuaData& data)
      {
        m_stQuestTableCFG.vecTables.push_back(data.getTableString("key"));
      };
      m->second.foreach(mf);
    }
    else if (m->first == "Lottery")
    {
      m_stLotteryCFG.dwPrice = m->second.getTableInt("Price");
      m_stLotteryCFG.dwDiscount = m->second.getTableInt("Discount");
      if (m_stLotteryCFG.dwDiscount > 100)
        m_stLotteryCFG.dwDiscount = 100;

      m_stLotteryCFG.m_mapBatch.clear();
      auto batchF = [&](const string&key, xLuaData& data)
      {
        DWORD t = atoi(key.c_str());
        m_stLotteryCFG.m_mapBatch[t] = data.getInt();
      };
      m->second.getMutableData("Batch").foreach(batchF);

      m_stLotteryCFG.m_mapCoinWeight.clear();
      m_stLotteryCFG.m_dwTotalCoinWeight = 0;
      auto coinWeightF = [&](const string&key, xLuaData& data)
      {
        DWORD t = atoi(key.c_str());
        m_stLotteryCFG.m_mapCoinWeight[t] = data.getInt();
        m_stLotteryCFG.m_dwTotalCoinWeight += data.getInt();
      };
      m->second.getMutableData("CoinWeight").foreach(coinWeightF);
      
      m_stLotteryCFG.m_mapTicketWeight.clear();
      m_stLotteryCFG.m_dwTotalTicketWeight = 0;
      auto ticketWeightF = [&](const string&key, xLuaData& data)
      {
        DWORD t = atoi(key.c_str());
        m_stLotteryCFG.m_mapTicketWeight[t] = data.getInt();
        m_stLotteryCFG.m_dwTotalTicketWeight += data.getInt();
      };
      m->second.getMutableData("TicketWeight").foreach(ticketWeightF);

      m_stLotteryCFG.m_dwDayBuyLotteryGiveCnt = m->second.getTableInt("DayBuyLotteryGiveCnt");
      m_stLotteryCFG.m_dwBasePriceRateMin = m->second.getTableInt("BasePriceRateMin");
      m_stLotteryCFG.m_dwBasePriceRateMax = m->second.getTableInt("BasePriceRateMax");
      m_stLotteryCFG.m_dwSaveRateMax = m->second.getTableInt("SaveRateMax");
      
      m_stLotteryCFG.m_mapDayMaxCnt.clear();
      auto dayMaxCntF = [&](const string&key, xLuaData& data)
      {
        DWORD t = atoi(key.c_str());
        m_stLotteryCFG.m_mapDayMaxCnt[t] = data.getInt();
      };
      m->second.getMutableData("DayMaxCount").foreach(dayMaxCntF);

      auto linkage = [&](const string& key, xLuaData& d)
      {
        SLotteryLinkage linkage;
        linkage.dwType = d.getTableInt("type");
        linkage.dwYear = d.getTableInt("year");
        linkage.dwMonth = d.getTableInt("month");
        std::string strEndTime = d.getTableString("end_time");
        parseTime(strEndTime.c_str(), linkage.dwEndTime);
        m_stLotteryCFG.m_vecLinkage.push_back(linkage);
      };
      m_stLotteryCFG.m_vecLinkage.clear();
      m->second.getMutableData("Linkage").foreach(linkage);
    }
    else if (m->first == "Auction")
    {    
      xLuaData& rData = m->second.getMutableData("StartTime");
      auto funcStartTime = [&](const std::string& key, xLuaData& data)
      {
        tm stm;
        bzero(&stm, sizeof(stm));
        if (data.has("wday"))
          stm.tm_wday = data.getTableInt("wday"); //stm.tm_wday[0,6] 0 is sunday，输入1-7
        else
        {
          XERR << "[MiscConfig] Auction 没有配置拍卖时间 " << XEND;
          bCorrect = false;
          return;
        }
        if (data.has("hour"))
          stm.tm_hour = data.getTableInt("hour");
        else
          return;
        if (data.has("min"))
          stm.tm_min = data.getTableInt("min");
        
        XDBG << "[MiscConfig] Auction 拍卖开始时间 key: " << key <<"周几" << stm.tm_wday <<"小时" <<stm.tm_hour <<"分钟"<<stm.tm_min << XEND;
        m_stAuctionMiscCFG.m_mapBeginTime[stm.tm_wday] = stm;
      };
      rData.foreach(funcStartTime);
      
      m_stAuctionMiscCFG.dwVerifySignupDuration = m->second.getTableInt("VerifySignupDuration");
      m_stAuctionMiscCFG.dwVerifyTakeDuration = m->second.getTableInt("VerifyTakeDuration");
      m_stAuctionMiscCFG.dwMaxAuctionItemCount = m->second.getTableInt("MaxAuctionItemCount");
      m_stAuctionMiscCFG.dwMinAuctionItemCount = m->second.getTableInt("MinAuctionItemCount");     
      m_stAuctionMiscCFG.dwDurationPerOrder = m->second.getTableInt("DurationPerOrder");
      m_stAuctionMiscCFG.dwNextOrderDuration = m->second.getTableInt("NextOrderDuration");
      m_stAuctionMiscCFG.dwTradePriceDiscount = m->second.getTableInt("TradePriceDiscount");
      m_stAuctionMiscCFG.dwStartDialogTime = m->second.getTableInt("AuctionStartDialogTime");
      m_stAuctionMiscCFG.dwPublicityDuration = m->second.getTableInt("PublicityDuration");
      
    }
    else if (m->first == "Shop")
    {
      m_stShopCFG.mapInfos.clear();

      auto f = [&](const string& str, xLuaData& data)
      {
        auto& cfg = m_stShopCFG.mapInfos[atoi(str.c_str())];
        auto func = [&](const string& str, xLuaData& data)
        {
          DWORD shopid = atoi(str.c_str());
          cfg[shopid].dwCount = data.getTableInt("count");
          cfg[shopid].bDuplicate = data.getTableInt("duplicate");
        };
        data.foreach(func);
      };
      m->second.getMutableData("randbylv").foreach(f);
      
      m_stShopCFG.mapWeekShopRand.clear();
      m_stShopCFG.setWeekShopRand.clear();
      auto fWeekRand = [&](const string& str, xLuaData& data)
      {
        DWORD groupId = atoi(str.c_str());
        auto& rCfg = m_stShopCFG.mapWeekShopRand[groupId];
        rCfg.dwGroupId = groupId;        
        rCfg.dwWeight = data.getTableInt("weight");
        
        auto func2 = [&](const string& str, xLuaData& data)
        {
          DWORD shopid = data.getInt();
          rCfg.vecShopId.push_back(shopid);
          m_stShopCFG.setWeekShopRand.insert(shopid);
        };
        data.getMutableData("shopid").foreach(func2);
      };

      m->second.getMutableData("weekrand").foreach(fWeekRand);
    }
    else if(m->first == "ExchangeShop")
    {
      m_stExchangeShopCFG.dwMinDelayTime = m->second.getTableInt("MinDelayStartTime") * 60;
      m_stExchangeShopCFG.dwMaxDelayTime = m->second.getTableInt("MaxDelayStartTime") * 60;
      m_stExchangeShopCFG.dwBaseWorldLevelExpBuff = m->second.getTableInt("BaseWorldLevelExpBuffID");
      m_stExchangeShopCFG.dwJobWorldLevelExpBuff = m->second.getTableInt("JobWorldLevelExpBuffID");

      m_stExchangeShopCFG.setOpenPros.clear();
      auto prof = [&](const string& key, xLuaData& data)
      {
        DWORD dwPro = data.getInt();
        if (dwPro <= EPROFESSION_MIN || dwPro >= EPROFESSION_MAX || EProfession_IsValid(dwPro) == false)
        {
          XERR << "[ServerGame] ExchangeShop -> open_pros" << dwPro << "不合法" << XEND;
          return;
        }
        m_stExchangeShopCFG.setOpenPros.insert(static_cast<EProfession>(dwPro));
      };
      m->second.getMutableData("noopen_pros").foreach(prof);
    }
    else if (m->first == "ToyDoll")
    {
      DWORD type = 1;
      auto func = [&](const string& str, xLuaData& data)
      {
        DWORD begin = data.getTableInt("1");
        DWORD end = data.getTableInt("2");
        for (DWORD i = begin; i <= end; ++i)
        {
          if (type == 1)
            m_stHandNpcCFG.vecBody.push_back(i);
          else if (type == 2)
            m_stHandNpcCFG.vecHead.push_back(i);
          else if (type == 3)
            m_stHandNpcCFG.vecHair.push_back(i);
          else if (type == 4)
            m_stHandNpcCFG.vecHairColor.push_back(i);
          else if (type == 5)
            m_stHandNpcCFG.vecEye.push_back(i);
        }
      };
      m_stHandNpcCFG.clear();
      xLuaData& body = m->second.getMutableData("Body");
      type = 1;
      body.foreach(func);
      xLuaData& head = m->second.getMutableData("Head");
      type = 2;
      head.foreach(func);
      xLuaData& hair = m->second.getMutableData("Hair");
      type = 3;
      hair.foreach(func);
      xLuaData& haircolor = m->second.getMutableData("HairColor");
      type = 4;
      haircolor.foreach(func);
      xLuaData& eye = m->second.getMutableData("Eye");
      type = 5;
      eye.foreach(func);
      if (m_stHandNpcCFG.vecBody.empty() || m_stHandNpcCFG.vecHead.empty() ||
          m_stHandNpcCFG.vecHair.empty() || m_stHandNpcCFG.vecHairColor.empty() ||
          m_stHandNpcCFG.vecEye.empty())
      {
        XERR << "[MiscConfig] 艾娃配置, 随机列表为空" << XEND;
        bCorrect = false;
      }

      m_stHandNpcCFG.dwContinueTime = m->second.getTableInt("time");

      auto funcValue = [&](xLuaData& luadata, SHandShowData& stData)
      {
        stData.dwOdds = luadata.getTableInt("odds");
        stData.vecValue.clear();
        auto getValue = [&](const string& key, xLuaData& data)
        {
          stData.vecValue.push_back(data.getInt());
        };
        luadata.getMutableData("value").foreach(getValue);
      };
      funcValue(m->second.getMutableData("birth_emoji"), m_stHandNpcCFG.stBirthEmoji);
      funcValue(m->second.getMutableData("birth_dialog"), m_stHandNpcCFG.stBirthDialog);
      funcValue(m->second.getMutableData("disp_emoji"), m_stHandNpcCFG.stDispEmoji);
      funcValue(m->second.getMutableData("disp_dialog"), m_stHandNpcCFG.stDispDialog);
      funcValue(m->second.getMutableData("normal_emoji"), m_stHandNpcCFG.stNormalEmoji);
      funcValue(m->second.getMutableData("normal_dialog"), m_stHandNpcCFG.stNormalDialog);
      funcValue(m->second.getMutableData("attack_emoji"), m_stHandNpcCFG.stAttackEmoji);
      funcValue(m->second.getMutableData("attack_dialog"), m_stHandNpcCFG.stAttackDialog);

      m_stHandNpcCFG.dwEmojiInterval = m->second.getTableInt("emoji_interval");
      m_stHandNpcCFG.dwDialogInterval = m->second.getTableInt("dialog_interval");
    }
    else if (m->first == "Quota")
    {
      m_stQuotaCFG.dwItemId = m->second.getTableInt("ItemId");
      m_stQuotaCFG.dwDefaultExpireDay = m->second.getTableInt("DefautExpireDay");
      m_stQuotaCFG.dwLogCountPerPage = m->second.getTableInt("LogCountPerPage");
      m_stQuotaCFG.dwMaxSaveLogCount = m->second.getTableInt("MaxSaveLogCount");
      m_stQuotaCFG.dwDetailCountPerPage = m->second.getTableInt("DetailCountPerPage");
      m_stQuotaCFG.dwDetailExpireDay = m->second.getTableInt("DetailExpireDay");
      m_stQuotaCFG.dwDetailExpireDayDel = m->second.getTableInt("DetailExpireDayDel");
    }
    else if (m->first == "PoliFire")
    {
      m_stPoliFireCFG.fDropRange = m->second.getTableFloat("drop_range");
      m_stPoliFireCFG.dwItemDispTime = m->second.getTableInt("item_disp_time");
      m_stPoliFireCFG.dwDefaultSkillID = m->second.getTableInt("default_skill");
      /*m_stPoliFireCFG.mapItem2Skill.clear();
      auto getitemskill = [&](const string& k, xLuaData& d)
      {
        DWORD itemid = d.getTableInt("1");
        DWORD skillid = d.getTableInt("2");
        m_stPoliFireCFG.mapItem2Skill[itemid] = skillid;
      };
      m->second.getMutableData("item_skill").foreach(getitemskill);
      */

      m_stPoliFireCFG.dwMaxSkillPos = m->second.getTableInt("max_skill_pos");
      m_stPoliFireCFG.dwAppleItemID = m->second.getTableInt("gold_apple_id");
      m_stPoliFireCFG.dwGhostPoliBuff = m->second.getTableInt("ghost_poli_buff");
      m_stPoliFireCFG.dwRecoverAppleNum = m->second.getTableInt("recover_num");
      m_stPoliFireCFG.dwDefualtScore = m->second.getTableInt("default_score");
      m_stPoliFireCFG.dwShowBuff = m->second.getTableInt("show_buff");

      m_stPoliFireCFG.dwGodBuffid = m->second.getTableInt("god_buffid");
      m_stPoliFireCFG.dwGodDuration = m->second.getTableInt("god_duration");
      m_stPoliFireCFG.dwAppleLimitCount = m->second.getTableInt("apple_limit_count");
      m_stPoliFireCFG.dwSocreItemId = m->second.getTableInt("score_itemid");
      m_stPoliFireCFG.dwPreCloseMsgTime = m->second.getTableInt("pre_close_msgtime");

      auto fLvReward = [&](const string& k, xLuaData& d)
      {
        m_stPoliFireCFG.mapLv2ReardId[d.getTableInt("1")] = d.getTableInt("2");  
      };
      m->second.getMutableData("level_reward").foreach(fLvReward);
      
      auto fMaskBuff = [&](const string& k, xLuaData& d)
      {
        m_stPoliFireCFG.setMaskBuffId.insert(d.getInt());       
      };
      m->second.getMutableData("mask_buff").foreach(fMaskBuff);
    }
    else if (m->first == "Manual")
    {
      m_stManualCFG.dwHeadReturnMailID = m->second.getTableInt("head_item_return_mail");
      m_stManualCFG.dwCardReturnMailID = m->second.getTableInt("card_item_return_mail");
      m_stManualCFG.dwHeadUnlockReturnMailID = m->second.getTableInt("head_unlock_return_mail");
      m_stManualCFG.dwCardUnlockReturnMailID = m->second.getTableInt("card_unlock_return_mail");
      m_stManualCFG.dwLevelReturnMailID = m->second.getTableInt("level_return_mail");
      m_stManualCFG.dwQualityReturnMailID = m->second.getTableInt("quality_return_mail");
      m_stManualCFG.dwSkillReturnMailID = m->second.getTableInt("skill_return_mail");
      m_stManualCFG.dwUnsolvedPhotoOvertime = m->second.getTableInt("unsolvedphoto_overtime");

      auto levelf = [&](const string& key, xLuaData& data)
      {
        m_stManualCFG.mapLevelReturn[atoi(key.c_str())] = data.getInt();
      };
      m->second.getMutableData("level_return").foreach(levelf);
      for (auto &m : m_stManualCFG.mapLevelReturn)
        XDBG << "[ServerGame.Manual] level :" << m.first << m.second << XEND;

      auto qualityf = [&](const string& key, xLuaData& data)
      {
        DWORD dwQuality = atoi(key.c_str());
        if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
        {
          bCorrect = false;
          XERR << "[ServerGame->Manual] quality :" << dwQuality << "不合法" << XEND;
          return;
        }

        EQualityType eQuality = static_cast<EQualityType>(dwQuality);
        map<DWORD, ItemInfo>& mapCFG = m_stManualCFG.mapQualityReturn[eQuality];

        auto numf = [&](const string& key, xLuaData& d)
        {
          ItemInfo& rItem = mapCFG[atoi(key.c_str())];
          rItem.set_id(d.getTableInt("1"));
          rItem.set_count(d.getTableInt("2"));
        };
        data.foreach(numf);
      };
      m->second.getMutableData("quality_return").foreach(qualityf);
      for (auto &m : m_stManualCFG.mapQualityReturn)
      {
        XDBG << "[ServerGame.Manual] quality :" << m.first;
        for (auto &item : m.second)
          XDBG << "num :" << item.first << "count :" << item.second.ShortDebugString();
        XDBG << XEND;
      }

      auto quality_namef = [&](const string& key, xLuaData& data)
      {
        EQualityType eQuality = static_cast<EQualityType>(atoi(key.c_str()));
        m_stManualCFG.mapQualityName[eQuality] = data.getString();
      };
      m->second.getMutableData("quality_name").foreach(quality_namef);
    }
    else if (m->first == "Quest")
    {
      m_stQuestCFG.mapDailyPerDay.clear();
      auto dailyf = [&](const string& key, xLuaData& data)
      {
        SDailyPerDay& rDay = m_stQuestCFG.mapDailyPerDay[atoi(key.c_str())];
        rDay.dwSubmitCount = data.getTableInt("1");
        rDay.dwAcceptCount = data.getTableInt("2");
      };
      m->second.getMutableData("dailyrand_per_day").foreach(dailyf);

      m_stQuestCFG.mapPrefixion.clear();
      auto prefixionf = [&](const string& key, xLuaData& data)
      {
        m_stQuestCFG.mapPrefixion[atoi(key.c_str())] = data.getString();
      };
      m->second.getMutableData("prefixion").foreach(prefixionf);
    }
    else if (m->first == "GuildWelfare")
    {
      m_stGuildWelfareCFG.dwOverdueTime = m->second.getTableInt("overdue_time") <= 0 ? 7 * DAY_T : m->second.getTableInt("overdue_time");
    }
    else if (m->first == "GuildChallenge")
    {
      m_stGuildChallengeCFG.dwExtraReward = m->second.getTableInt("extra_reward");
    }
    else if (m->first == "Boss")
    {
      m_stBossCFG.dwRefreshBaseTimes = m->second.getTableInt("refresh_base_times");
      m_stBossCFG.dwDeadBossOpenNtf = m->second.getTableInt("deadboss_open_ntf_dialog");
      m_stBossCFG.dwDeadSetRate = m->second.getTableInt("dead_set_rate");

      auto timef = [&](const string& key, xLuaData& data)
      {
        const string& str = data.getString();
        DWORD dwHour = 0;
        DWORD dwMin = 0;
        sscanf(str.c_str(), "%u:%u", &dwHour, &dwMin);
        m_stBossCFG.setRefreshTimes.insert(dwHour * HOUR_T + dwMin * MIN_T);
      };
      m->second.getMutableData("refresh_time").foreach(timef);
    }
    else if(m->first == "FunctionSwitch")
    {
      m_stFunctionSwitchCfg.dwFreeFreyja = m->second.getTableInt("FreeFreyja");
      m_stFunctionSwitchCfg.dwFreePackage = m->second.getTableInt("FreePackage");
      m_stFunctionSwitchCfg.dwFreeFreyjaTeam = m->second.getTableInt("FreeFreyja_Team");
    }
    else if (m->first == "Artifact")
    {
      m_stArtifactCFG.dwRetrieveCD = m->second.getTableInt("retrieve_cd");
      auto buildingf = [&](const string& key, xLuaData& data)
      {
        DWORD btype = m->second.getTableInt("BuildingType");
        if (btype <= EGUILDBUILDING_MIN || btype >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(btype) == false)
          return;
        SArtifactBuildingMiscCFG& cfg = m_stArtifactCFG.mapBuilding2CFG[static_cast<EGuildBuilding>(btype)];
        cfg.dwProduceNpcID = data.getTableInt("produce_npc_id");
        cfg.dwProduceMsgID = data.getTableInt("produce_msg_id");
      };
      m->second.getMutableData("building").foreach(buildingf);
    }
    else if (m->first == "Deposit")
    {     
      m_stSDepositCFG.m_vecDiscountVersionCard.clear();
      m_stSDepositCFG.m_vecDiscountVersionCard.reserve(4);
      auto f = [&](const string& key, xLuaData& data)
      {
        std::pair<DWORD, DWORD> pr;
        pr.first = data.getTableInt("fromid");
        pr.second = data.getTableInt("toid");
        if (!pr.first)
          return;
        m_stSDepositCFG.m_vecDiscountVersionCard.push_back(pr);
      };
      m->second.getMutableData("DiscountVersionCard").foreach(f);     
    }
    else if (m->first == "Wedding")
    {
      m_stWeddingMiscCFG.dwMaxReserveDay = m->second.getTableInt("max_reserve_day");
      m_stWeddingMiscCFG.dwMaxTicketReserveDay = m->second.getTableInt("max_ticket_reserve_day");
      m_stWeddingMiscCFG.dwDivorceBuffid = m->second.getTableInt("DivorceBuffid");
      m_stWeddingMiscCFG.dwWeddingManualId = m->second.getTableInt("wedding_manual_itemid");
      m_stWeddingMiscCFG.dwInvitationMailID = m->second.getTableInt("invitation_mail_id");
      m_stWeddingMiscCFG.dwInvitationItemID = m->second.getTableInt("invitation_item_id");
      m_stWeddingMiscCFG.dwDelManualMailID = m->second.getTableInt("del_manual_mail_id");
      m_stWeddingMiscCFG.dwDelInvitationMailID = m->second.getTableInt("del_invitation_mail_id");
      m_stWeddingMiscCFG.dwShowQuestID = m->second.getTableInt("wedding_quest_show");
      m_stWeddingMiscCFG.dwWeddingMsgID = m->second.getTableInt("wedding_msg_id");
      m_stWeddingMiscCFG.dwDefaultRingID = m->second.getTableInt("default_ring_id");
      m_stWeddingMiscCFG.dwTopPackageID = m->second.getTableInt("top_package_id");
      m_stWeddingMiscCFG.dwRingShopType = m->second.getTableInt("ring_shop_type");
      m_stWeddingMiscCFG.dwRingShopID = m->second.getTableInt("ring_shop_id");

      m_stWeddingMiscCFG.vecPreQuestion.clear();
      m->second.getMutableData("pre_question").getIDList(m_stWeddingMiscCFG.vecPreQuestion);
      m_stWeddingMiscCFG.vecFirstStageQuestion.clear();
      m->second.getMutableData("first_stage_question").getIDList(m_stWeddingMiscCFG.vecFirstStageQuestion);
      m_stWeddingMiscCFG.vecSecondStageQuestion.clear();
      m->second.getMutableData("second_stage_question").getIDList(m_stWeddingMiscCFG.vecSecondStageQuestion);

      m_stWeddingMiscCFG.setWeddingMsgTime.clear();
      m->second.getMutableData("wedding_msg_time").getIDList(m_stWeddingMiscCFG.setWeddingMsgTime);
      m_stWeddingMiscCFG.dwDressLetterId = m->second.getTableInt("wedding_dress_letter_id");
      m_stWeddingMiscCFG.dwRingItemID = m->second.getTableInt("wedding_ring_id");
      m_stWeddingMiscCFG.dwWeddingCertificate = m->second.getTableInt("wedding_certificate_id");
      m_stWeddingMiscCFG.dwRollerCoasterMapId = m->second.getTableInt("roller_coaster_mapid");
      m_stWeddingMiscCFG.dwDivorceNpc = m->second.getTableInt("divore_npc");
      m_stWeddingMiscCFG.dwDivorceRollerCoasterMapId = m->second.getTableInt("divorce_roller_coaster_mapid");
      m_stWeddingMiscCFG.dwForceDivorceQuestId = m->second.getTableInt("force_divorce_questid");
      m_stWeddingMiscCFG.dwDivorceCarrierId = m->second.getTableInt("divorce_carrierid");
      m_stWeddingMiscCFG.dwWeddingRaidID = m->second.getTableInt("wedding_raid_id");
      m_stWeddingMiscCFG.dwWeddingTeamBuff = m->second.getTableInt("wedding_team_buff");
      m_stWeddingMiscCFG.dwWeddingCarrierID = m->second.getTableInt("wedding_carrier_id");
      m_stWeddingMiscCFG.dwWeddingCarrierLine = m->second.getTableInt("wedding_carrier_line");

      m_stWeddingMiscCFG.setMarrySkills.clear();
      m->second.getMutableData("marry_skill").getIDList(m_stWeddingMiscCFG.setMarrySkills);

      m_stWeddingMiscCFG.bGenderCheck = m->second.getTableInt("wedding_gender_check") == 1;
    }
    else if (m->first == "KFCActivity")
    {
      m_stKFCActivityCFG.dwRewardID = m->second.getTableInt("reward");
      parseTime(m->second.getTableString("start_time"), m_stKFCActivityCFG.dwStartTime);
      parseTime(m->second.getTableString("end_time"), m_stKFCActivityCFG.dwEndTime);
    }
    else if (m->first == "Card")
    {
      DWORD rate = 0;
      auto drfunc = [&](const string& key, xLuaData& data)
      {
        rate += atoi(key.c_str());
        m_stCardMiscCFG.mapDecomposeRate[rate] = data.getInt();
      };
      m_stCardMiscCFG.mapDecomposeRate.clear();
      m->second.getMutableData("decompose_rate").foreach(drfunc);
    }
    else if (m->first == "Buff")
    {
      m_stBuffMiscCFG.setNineSyncBuffs.clear();
      m->second.getMutableData("nine_buff").getIDList(m_stBuffMiscCFG.setNineSyncBuffs);
    }
    else if (m->first == "Var")
    {
      m_stVarMiscCFG.dwOffset = m->second.getTableInt("offset");
      auto offsetf = [&](const string& key, xLuaData& data)
      {
        DWORD var = atoi(key.c_str());
        if (var <= EVARTYPE_MIN || var >= EVARTYPE_MAX || EVarType_IsValid(var) == false)
        {
          XLOG << "[MiscConfig] var:" << var << "非法" << XEND;
          return;
        }
        m_stVarMiscCFG.mapVar2Offset[static_cast<EVarType>(var)] = data.getInt();
      };
      m->second.getMutableData("var2offset").foreach(offsetf);
      auto accoffsetf = [&](const string& key, xLuaData& data)
      {
        DWORD var = atoi(key.c_str());
        if (var <= EACCVARTYPE_MIN || var >= EACCVARTYPE_MAX || EAccVarType_IsValid(var) == false)
        {
          XLOG << "[MiscConfig] accvar:" << var << "非法" << XEND;
          return;
        }
        m_stVarMiscCFG.mapAccVar2Offset[static_cast<EAccVarType>(var)] = data.getInt();
      };
      m->second.getMutableData("accvar2offset").foreach(accoffsetf);
    }
    else if (m->first == "CapraActivity")
    {
      m_stCapraActivityCFG.mapId = m->second.getTableInt("MapId");

      m_stCapraActivityCFG.m_vecAddQuests.clear();
      m->second.getMutableData("addquests").getIDList(m_stCapraActivityCFG.m_vecAddQuests);
      m_stCapraActivityCFG.m_vecDelQuests.clear();
      m->second.getMutableData("delquests").getIDList(m_stCapraActivityCFG.m_vecDelQuests);
      m_stCapraActivityCFG.m_vecDelBuffs.clear();
      m->second.getMutableData("delbuffs").getIDList(m_stCapraActivityCFG.m_vecDelBuffs);
    }
    else if (m->first == "Altman")
    {
      DWORD dwRaid = m->second.getTableInt("raid_id");
      if(dwRaid != 0)
        mapType2RaidID.emplace(ERAIDTYPE_ALTMAN, dwRaid);

      m_stAltmanCFG.dwDefaultSkillID = m->second.getTableInt("default_skill");
      m_stAltmanCFG.dwMaxSkillPos = m->second.getTableInt("max_skill_pos");
      m_stAltmanCFG.dwRewardID = m->second.getTableInt("rewardid");
      m_stAltmanCFG.dwClearNpcTime = m->second.getTableInt("clear_npc_time");
      m_stAltmanCFG.dwFashionBuff = m->second.getTableInt("fashion_buff");
      m_stAltmanCFG.oRewardBoxDefine = m->second.getMutableData("rewardbox");

      m_stAltmanCFG.setEnterBuffs.clear();
      auto bufffunc = [&](const string& key, xLuaData& data)
      {
        m_stAltmanCFG.setEnterBuffs.emplace(data.getInt());
      };
      m->second.getMutableData("trans_buff").foreach(bufffunc);

      m_stAltmanCFG.setManualHeads.clear();
      auto headfunc = [&](const string& key, xLuaData& data)
      {
        m_stAltmanCFG.setManualHeads.emplace(data.getInt());
      };
      m->second.getMutableData("head").foreach(headfunc);

      m_stAltmanCFG.setFashionEquips.clear();
      auto fashionfunc = [&](const string& key, xLuaData& data)
      {
        m_stAltmanCFG.setFashionEquips.emplace(data.getInt());
      };
      m->second.getMutableData("fashion_equip").foreach(fashionfunc);

      m_stAltmanCFG.vecHeadReward.clear();
      auto headrewardfunc = [&](const string& key, xLuaData& data)
      {
        DWORD num = data.getTableInt("num");
        DWORD rewardid = data.getTableInt("rewardid");
        m_stAltmanCFG.vecHeadReward.emplace_back(make_pair(num, rewardid));
      };
      m->second.getMutableData("extra_head_reward").foreach(headrewardfunc);

      m_stAltmanCFG.vecKillReward.clear();
      auto killrewardfunc = [&](const string& key, xLuaData& data)
      {
        DWORD num = data.getTableInt("num");
        DWORD rewardid = data.getTableInt("rewardid");
        m_stAltmanCFG.vecKillReward.emplace_back(make_pair(num, rewardid));
      };
      m->second.getMutableData("kill_reward").foreach(killrewardfunc);

      m_stAltmanCFG.mapItem2Skill.clear();
      auto getskill = [&](const string& k, xLuaData& d)
      {
        DWORD item = atoi(k.c_str());
        DWORD skill = d.getInt();
        m_stAltmanCFG.mapItem2Skill.emplace(item, skill);
      };
      m->second.getMutableData("item_skill").foreach(getskill);

      m_stAltmanCFG.mapKill2Title.clear();
      auto titlefunc = [&](const string& key, xLuaData& data)
      {
        DWORD dwKill = data.getTableInt("killcount");
        const string& stitle = data.getTableString("title");
        m_stAltmanCFG.mapKill2Title.emplace(dwKill, stitle);
      };
      m->second.getMutableData("kill_rank_desc").foreach(titlefunc);
    }
  }

  if (bCorrect)
    XLOG << "[MiscConfig], 成功加载ServerGame.lua" << XEND;

  return bCorrect;
}

const bool MiscConfig::isSystemForbid(ESystemForbidType eType)
{
  auto s = m_mapSystemsForbidCFG.find(eType);
  if(s != m_mapSystemsForbidCFG.end())
  {
    DWORD dwBranch = BaseConfig::getMe().getInnerBranch();
    if(dwBranch == BRANCH_DEBUG)
      dwBranch = static_cast<DWORD>(EBRANCH_TYPE_DEBUG);
    else if(dwBranch == BRANCH_TF)
      dwBranch = static_cast<DWORD>(EBRANCH_TYPE_TF);
    else if(dwBranch == BRANCH_PUBLISH)
      dwBranch = static_cast<DWORD>(EBRANCH_TYPE_RELEASE);

    return s->second & dwBranch;
  }

  return false;
}

const DWORD MiscConfig::getJoyLimitByType(EJoyActivityType eType)
{
  DWORD dwLimit = 0;
  switch (eType)
  {
    case JOY_ACTIVITY_GUESS:
      dwLimit = m_stJoyCFG.dwGuess;
      break;
    case JOY_ACTIVITY_MISCHIEF:
      dwLimit = m_stJoyCFG.dwMischief;
      break;
    case JOY_ACTIVITY_QUESTION:
      dwLimit = m_stJoyCFG.dwQuestion;
      break;
    case JOY_ACTIVITY_FOOD:
      dwLimit = m_stJoyCFG.dwFood;
      break;
    case JOY_ACTIVITY_YOYO:
      dwLimit = m_stJoyCFG.dwYoyo;
      break;
    case JOY_ACTIVITY_ATF:
      dwLimit = m_stJoyCFG.dwATF;
      break;
    case JOY_ACTIVITY_AUGURY:
      dwLimit = m_stJoyCFG.dwAugury;
      break;
    default:
      break;
  }

  return dwLimit;
}

const DWORD MiscConfig::getJoyReward(DWORD oldjoy, DWORD newjoy)
{
  for(auto s : m_stJoyCFG.mapReward)
  {
    if(oldjoy < s.first && s.first <= newjoy)
      return s.second;
  }

  return 0;
}

const DWORD MiscConfig::getFoodValue(DWORD foodid)
{
  auto s = m_stJoyCFG.setFoodID.find(foodid);
  if(s != m_stJoyCFG.setFoodID.end())
    return m_stJoyCFG.dwFoodAdd;

  return 0;
}
const DWORD MiscConfig::getAddLvByItem(DWORD item, DWORD num) const
{
  for(auto s : m_stPeakLevelCFG.m_vecPeakItem)
  {
    if(s.dwItem == item && s.dwNum == num)
      return s.dwAddLv;
  }

  return 0;
}

bool MiscConfig::loadUnionConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/UnionConfig.txt"))
  {
    XERR << "[海外替换配置], 加载配置UnionConfig.txt失败" << XEND;
    return false;
  }

  xLuaData table;
  xLuaTable::getMe().getLuaData("UnionConfig", table);
  for (auto m = table.m_table.begin(); m != table.m_table.end(); ++m)
  {
    if (m->first == "System")
    {
      m_stSystemCFG.dwNameMaxSize = m->second.getTableInt("namesize_max");
      m_stSystemCFG.dwNameMinSize = m->second.getTableInt("namesize_min");
    }
    else if (m->first == "Tutor")
    {
      m_stTutorCFG.bGrowRewardPatchSwitch = m->second.getTableInt("grow_reward_patch_switch") == 1;
    }
  }

  if (bCorrect)
    XLOG << "[海外替换配置] 成功加载UnionConfig.txt配置" << XEND;
  return bCorrect;
}


DWORD MiscConfig::getRaidByType(ERaidType eType) const
{
  auto it = mapType2RaidID.find(eType);
  if(it != mapType2RaidID.end())
    return it->second;

  return 0;
}

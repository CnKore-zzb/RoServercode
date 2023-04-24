#include "ServantConfig.h"
#include "xLuaTable.h"
#include "MiscConfig.h"


ServantConfig::ServantConfig()
{

}

ServantConfig::~ServantConfig()
{

}

bool ServantConfig::loadConfig()
{
  bool bRecommend = loadRecommendConfig();
  bool bGrowth = loadGrowthConfig();

  return bRecommend && bGrowth;
}

bool ServantConfig::loadRecommendConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Recommend.txt"))
  {
    XERR << "[仆人配置-加载今日推荐] 加载配置 Table_Recommend.txt 失败" << XEND;
    return false;
  }

  m_mapServantCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Recommend", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");

    auto iter = m_mapServantCFG.find(id);
    if(iter != m_mapServantCFG.end())
    {
      XERR << "[仆人配置-加载今日推荐] 加载配置 Table_Recommend.txt 重复ID: " << id << XEND;
      continue;
    }

    SServantItemCFG stCFG;
    stCFG.dwID = id;
    stCFG.dwReward = m->second.getTableInt("Reward");
    stCFG.dwFavorability = m->second.getTableInt("Favorability");
    ECycleType eType = static_cast<ECycleType>(m->second.getTableInt("Recycle"));
    if (eType == ECYCLE_FOREVER_GUIDE)
      eType = ECYCLE_FOREVER;
    stCFG.eType = eType;
    stCFG.bIsAcc = m->second.getTableInt("IsAcc");

    ETriggerType eReco = getTriggerCondition(m->second.getTableString("Appear"));
    stCFG.stAppearCondition.eTrigger = eReco;
    auto getParam = [&](const string& key, xLuaData& data)
    {
      stCFG.stAppearCondition.vecParams.emplace_back(data.getInt());
    };
    m->second.getMutableData("Appear_Params").foreach(getParam);

    ETriggerType eFinish = getTriggerCondition(m->second.getTableString("Finish"));
    stCFG.stFinishCondition.eTrigger = eFinish;
    auto getFinishParam = [&](const string& key_1, xLuaData& data_1)
    {
      stCFG.stFinishCondition.vecParams.emplace_back(data_1.getInt());
    };
    m->second.getMutableData("Finish_Params").foreach(getFinishParam);

    m_mapServantCFG.emplace(id, stCFG);
  }

  XLOG << "[仆人配置] 加载今日推荐Table_Recommend.txt 成功, 大小: " << m_mapServantCFG.size() << XEND;
  return true;
}

bool ServantConfig::loadGrowthConfig()
{
  if (!xLuaTable::getMe().open("Lua/Table/Table_Growth.txt"))
  {
    XERR << "[仆人配置-加载成长计划] 加载配置 Table_Growth.txt 失败" << XEND;
    return false;
  }

  m_mapGrowthCFG.clear();
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Growth", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    DWORD id = m->second.getTableInt("id");

    auto iter = m_mapGrowthCFG.find(id);
    if(iter != m_mapGrowthCFG.end())
    {
      XERR << "[仆人配置-加载成长计划] 加载配置 Table_Growth.txt 重复ID: " << id << XEND;
      continue;
    }

    SGrowthItemCFG stCFG;
    stCFG.dwID = id;
    stCFG.dwGroupID = id / SERVANT_GROWTH_ID_PARAM;
    stCFG.dwReward = m->second.getTableInt("Reward");
    stCFG.dwGrowth = m->second.getTableInt("Growth");
    stCFG.dwType = m->second.getTableInt("type");

    ETriggerType eGrowth1 = getTriggerCondition(m->second.getTableString("Group_Appear1"));
    stCFG.stGroupAppearCond1.eTrigger = eGrowth1;
    auto getGroup1Param = [&](const string& key, xLuaData& data)
    {
      stCFG.stGroupAppearCond1.vecParams.emplace_back(data.getInt());
    };
    m->second.getMutableData("Group_Params1").foreach(getGroup1Param);

    ETriggerType eGrowth2 = getTriggerCondition(m->second.getTableString("Group_Appear2"));
    stCFG.stGroupAppearCond2.eTrigger = eGrowth2;
    auto getGroup2Param = [&](const string& key_1, xLuaData& data_1)
    {
      stCFG.stGroupAppearCond2.vecParams.emplace_back(data_1.getInt());
    };
    m->second.getMutableData("Group_Params2").foreach(getGroup2Param);

    ETriggerType eReco = getTriggerCondition(m->second.getTableString("Appear"));
    stCFG.stGrowthAppearCond.eTrigger = eReco;
    auto getParam = [&](const string& key, xLuaData& data)
    {
      stCFG.stGrowthAppearCond.vecParams.emplace_back(data.getInt());
    };
    m->second.getMutableData("Appear_Params").foreach(getParam);

    ETriggerType eFinish = getTriggerCondition(m->second.getTableString("Finish"));
    stCFG.stGrowthFinishCond.eTrigger = eFinish;
    auto getFinishParam = [&](const string& key_1, xLuaData& data_1)
    {
      stCFG.stGrowthFinishCond.vecParams.emplace_back(data_1.getInt());
    };
    m->second.getMutableData("Finish_Params").foreach(getFinishParam);
    if(eFinish == ETRIGGER_MIN)
      XLOG << "[仆人配置] 完成类型没找到 " << "id: " << id << "Finish: " << m->second.getTableString("Finish") << XEND;

    m_mapGrowthCFG.emplace(id, stCFG);
  }

  XLOG << "[仆人配置] 加载成长计划Table_Growth.txt 成功, 大小: " << m_mapGrowthCFG.size() << XEND;
  return true;
}

const SServantItemCFG* ServantConfig::getServantCFG(DWORD id)
{
  auto m = m_mapServantCFG.find(id);
  if(m != m_mapServantCFG.end())
    return &(m->second);

  return nullptr;
}

ETriggerType ServantConfig::getRecoTrigger(DWORD id)
{
  auto m = m_mapServantCFG.find(id);
  if(m != m_mapServantCFG.end())
    return m->second.stAppearCondition.eTrigger;

  return ETRIGGER_MIN;
}

ETriggerType ServantConfig::getFinishTrigger(DWORD id)
{
  auto m = m_mapServantCFG.find(id);
  if(m != m_mapServantCFG.end())
    return m->second.stFinishCondition.eTrigger;

  return ETRIGGER_MIN;
}

ETriggerType ServantConfig::getTriggerCondition(const string& str)
{
  if (str == "polly_fight")
    return ETRIGGER_POLLY_FIGHT;
  else if (str == "cat_invasion")
    return ETRIGGER_CAT_INVASION;
  else if (str == "capra")
    return ETRIGGER_CAPRA;
  else if (str == "guild_fuben")
    return ETRIGGER_GUILD_FUBEN;
  else if (str == "endlesstower")
    return ETRIGGER_TOWER_PASS;
  else if (str == "gvg")
    return ETRIGGER_GVG;
  else if (str == "item_get")
    return ETRIGGER_ITEM_GET;
  else if (str == "levelup")
    return ETRIGGER_LEVELUP;
  else if (str == "joblevelup")
    return ETRIGGER_JOBLEVELUP;
  else if (str == "maxjoblevel")
    return ETRIGGER_MAXJOBLEVEL;
  else if (str == "menu")
    return ETRIGGER_MENU;
  else if (str == "pet_adventure")
    return ETRIGGER_PET_ADVENTURE;
  else if (str == "questfinish")
    return ETRIGGER_QUEST_SUBMIT;
  else if (str == "own_student")
    return ETRIGGER_OWN_STUDENT;
  else if (str == "tutorskill")
    return ETRIGGER_TUTOR_SKILL;
  else if (str == "augury")
    return ETRIGGER_AUGURY;
  else if (str == "wanted_quest_day")
    return ETRIGGER_WANTED_QUEST_DAY;
  else if (str == "daily_monster")
    return ETRIGGER_DAILYMONSTER;
  else if (str == "repair_seal")
    return ETRIGGER_REPAIR_SEAL;
  else if (str == "laboratory")
    return ETRIGGER_LABORATORY;
  else if (str == "fighttime")
    return ETRIGGER_BATTLE_TIME;
  else if (str == "join_guild")
    return ETRIGGER_JOIN_GUILD;
  else if (str == "listen_music")
    return ETRIGGER_LISTEN_MUSIC;
  else if (str == "play_music")
    return ETRIGGER_PLAY_MUSIC;
  else if (str == "pvp_kill")
    return ETRIGGER_PVP_KILL;
  else if (str == "photo_scenery")
    return ETRIGGER_PHOTO_SCENERY;
  else if (str == "dojo")
    return ETRIGGER_DOJO;
  else if (str == "guild_pray")
    return ETRIGGER_GUILD_PRAY;
  else if (str == "guild_donate")
    return ETRIGGER_GUILD_DONATE;
  else if (str == "guild_building")
    return ETRIGGER_GUILD_BUILDING;
  else if (str == "enchant_primary")
    return ETRIGGER_ENCHANT_PRIMARY;
  else if (str == "enchant_middle")
    return ETRIGGER_ENCHANT_MIDDLE;
  else if (str == "enchant_advanced")
    return ETRIGGER_ENCHANT_ADVANCED;
  else if (str == "cook_food")
    return ETRIGGER_COOKFOOD;
  else if (str == "own_tutor")
    return ETRIGGER_OWN_TUTOR;
  else if (str == "hair")
    return ETRIGGER_HAIR;
  else if (str == "eye")
    return ETRIGGER_EYE;
  else if (str == "title")
    return ETRIGGER_TITLE;
  else if (str == "recommend")
    return ETRIGGER_RECOMMEND;
  else if (str == "petwork")
    return ETRIGGER_PETWORK;
  else if (str == "eat_food")
    return ETRIGGER_EAT_FOOD;
  else if (str == "mapid")
    return ETRIGGER_MAPID;
  else if (str == "lottery")
    return ETRIGGER_LOTTERY;
  else if (str == "guild_build_donate")
    return ETRIGGER_GUILD_BUILD_DONATE;
  else if (str == "time_interval")
    return ETRIGGER_TIME_INTERVAL;
  else if (str == "world_freyja")
    return ETRIGGER_WORLD_FREYJA;
  else if (str == "card_reset")
    return ETRIGGER_CARD_RESET;
  else if (str == "card_customize")
    return ETRIGGER_CARD_CUSTOMIZE;
  else if (str == "produce_head")
    return ETRIGGER_PRODUCE_HEAD;
  else if (str == "enchant_head")
    return ETRIGGER_ENCHANT_HEAD;
  else if(str == "mercenary_cat")
    return ETRIGGER_MERCENARY_CAT;
  else if(str == "change_profession")
    return ETRIGGER_CHANGE_PROFESSION;
  else if (str == "kill_mini")
    return ETRIGGER_KILL_MINI;
  else if (str == "kill_mvp")
    return ETRIGGER_KILL_MVP;
  else if (str == "pve_card")
    return ETRIGGER_PVE_CARD;
  else if (str == "kill_star_npc")
    return ETRIGGER_KILL_STAR_NPC;
  else if (str == "mvp_battle")
    return ETRIGGER_MVP_BATTLE;
  else if (str == "unlock_catnum")
    return ETRIGGER_UNLOCK_CATNUM;

  else if (str == "last_growth")
    return ETRIGGER_LAST_GROWTH;
  else if (str == "quick_wanted_quest")
    return ETRIGGER_QUICK_WANTED_QUEST;
  else if (str == "equip_strength")
    return ETRIGGER_EQUIP_STRENGTH;
  else if (str == "pet_capture")
    return ETRIGGER_PET_CAPTURE;
  else if (str == "accept_quest")
    return ETRIGGER_ACCEPT_QUEST;
  else if (str == "exchange_head_drawing")
    return ETRIGGER_EXCHANGE_HEAD_DRAWING;
  else if (str == "quick_buy_drawing")
    return ETRIGGER_QUICK_BUY_DRAWING;
  else if (str == "head_compose")
    return ETRIGGER_HEAD_COMPOSE;
  else if (str == "equip_refine")
    return ETRIGGER_EQUIP_REFINE;
  else if (str == "safe_refine")
    return ETRIGGER_SAFE_REFINE;
  else if (str == "equip_compose")
    return ETRIGGER_EQUIP_COMPOSE;
  else if (str == "equip_compose_weapon")
    return ETRIGGER_EQUIP_COMPOSE_WEAPON;
  else if (str == "equip_compose_armour")
    return ETRIGGER_EQUIP_COMPOSE_ARMOUR;
  else if (str == "enter_pvp")
    return ETRIGGER_ENTER_PVP;
  else if (str == "card_mosaic")
    return ETRIGGER_CARD_MOSAIC;
  else if (str == "use_catapult")
    return ETRIGGER_USE_CATAPULT;
  else if (str == "extract_cat_litter")
    return ETRIGGER_EXTRACT_CAT_LITTER;
  else if (str == "vending_machine")
    return ETRIGGER_VENDING_MACHINE;
  else if (str == "equip_upgrade")
    return ETRIGGER_EQUIP_UPGRADE;
  else if (str == "guild_special_pray")
    return ETRIGGER_GUILD_SPECIAL_PRAY;
  else if (str == "guild_head_refine")
    return ETRIGGER_GUILD_HEAD_REFINE;
  else if (str == "guild_equip_firm")
    return ETRIGGER_GUILD_EQUIP_FIRM;
  else if (str == "castle_raid")
    return ETRIGGER_CASTLE_RAID;
  else if (str == "equip_decompose")
    return ETRIGGER_EQUIP_DECOMPOSE;
  else if (str == "equip_restore")
    return ETRIGGER_EQUIP_RESTORE;
  else if (str == "graduate")
    return ETRIGGER_GRADUATE;
  else if (str == "pve_card")
    return ETRIGGER_PVE_CARD;
  else if (str == "mvp_battle_kill")
    return ETRIGGER_MVP_BATTLE_KILL;
  else if (str == "record_save")
    return ETRIGGER_MVP_RECORD_SAVE;
  else if (str == "record_load")
    return ETRIGGER_MVP_RECORD_LOAD;
  else if (str == "profession_exchange")
    return ETRIGGER_PROFESSION_EXCHANGE;
  else if (str == "buy_profession")
    return ETRIGGER_BUY_PROFESSION;
  else if (str == "pve_card_simple")
    return ETRIGGER_PVE_CARD_SIMPLE;
  else if (str == "pve_card_middle")
    return ETRIGGER_PVE_CARD_MIDDLE;
  else if (str == "flame_raid")
    return ETRIGGER_FLAME_RAID;
  else if (str == "own_pet")
    return ETRIGGER_OWN_PET;
  else if (str == "pet_friend_full")
    return ETRIGGER_PET_FRIEND_FULL;
  else if (str == "pet_adventure_reward")
    return ETRIGGER_PET_ADVENTURE_REWARD;
  else if (str == "friendship_moracoin")
    return ETRIGGER_FRIENDSHIP_MORACOIN;
  else if (str == "start_petwork")
    return ETRIGGER_START_PETWORK;
  else if (str == "petwork_space")
    return ETRIGGER_PETWORK_SPACE;
  else if (str == "equip_exchange")
    return ETRIGGER_EQUIP_EXCHANGE;
  else if (str == "gold_medal")
    return ETRIGGER_GOLD_MEDAL;
  else if (str == "nightmare_raid")
    return ETRIGGER_NIGHTMARE_RAID;
  else if (str == "terrorist_raid")
    return ETRIGGER_TERRORIST_RAID;
  else if (str == "guild_artifact")
    return ETRIGGER_GUILD_ARTIFACT;
  else if (str == "guild_level")
    return ETRIGGER_GUILD_LEVEL;
  else if (str == "space_break_skill")
    return ETRIGGER_SPACE_BREAK_SKILL;
  else if (str == "lun_shard")
    return ETRIGGER_LUN_SHARD;
  else if (str == "lun_shard_special")
    return ETRIGGER_LUN_SHARD_SPECIAL;
  else if (str == "gvg_defense")
    return ETRIGGER_GVG_DEFENSE;
  else if (str == "guild_quest_accept")
    return ETRIGGER_GUILD_QUEST_ACCEPT;
  else if (str == "guild_quest_submit")
    return ETRIGGER_GUILD_QUEST_SUBMIT;
  else if (str == "trade_material")
    return ETRIGGER_TRADE_MATERIAL;
  else if (str == "manual_unlock")
    return ETRIGGER_MANUAL_UNLOCK;
  else if (str == "big_lottery")
    return ETRIGGER_BIG_LOTTERY;

  return ETRIGGER_MIN;
}

bool ServantConfig::checkInTimeInterval(DWORD curTime, DWORD startWeekDay, DWORD startHour, DWORD endWeekDay, DWORD endHour)
{
  DWORD dwWeekDay = xTime::getWeek(curTime);
  DWORD dwHour = xTime::getHour(curTime);
  if(dwWeekDay == 0)
    dwWeekDay = 7;
  if(dwWeekDay < startWeekDay)
    return false;
  else if(dwWeekDay == startWeekDay && dwHour < startHour)
    return false;
  else if(dwWeekDay > endWeekDay)
    return false;
  else if(dwWeekDay == endWeekDay && dwHour >= endHour)
    return false;
  return true;
}

const SGrowthItemCFG* ServantConfig::getGrowthCFG(DWORD id)
{
  auto m = m_mapGrowthCFG.find(id);
  if(m != m_mapGrowthCFG.end())
    return &(m->second);

  return nullptr;
}

ETriggerType ServantConfig::getGrowthAppearTrigger(DWORD id)
{
  auto m = m_mapGrowthCFG.find(id);
  if(m != m_mapGrowthCFG.end())
    return m->second.stGrowthAppearCond.eTrigger;

  return ETRIGGER_MIN;
}

ETriggerType ServantConfig::getGrowthFinishTrigger(DWORD id)
{
  auto m = m_mapGrowthCFG.find(id);
  if(m != m_mapGrowthCFG.end())
    return m->second.stGrowthFinishCond.eTrigger;

  return ETRIGGER_MIN;
}

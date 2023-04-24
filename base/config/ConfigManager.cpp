#include "ConfigManager.h"
#include "BaseConfig.h"
#include "XoclientConfig.h"
#include "DepositConfig.h"
#include "OperateRewardConfig.h"
#include "ValentineConfig.h"
#include "WeaponPetConfig.h"
#include "GuildRaidConfig.h"
#include "DateLandConfig.h"
#include "PetConfig.h"
#include "RecipeConfig.h"
#include "FoodConfig.h"
#include "TutorConfig.h"
#include "BeingConfig.h"
#include "HighRefineConfig.h"
#include "WeddingConfig.h"
#include "PveCardConfig.h"
#include "ServantConfig.h"
#include "SysmsgConfig.h"
#include "DeathTransferMapConfig.h"
#include "DeadConfig.h"
#include "BossConfig.h"

// 初始化顺序需要严格按照枚举ConfigType顺序, 逻辑中需要使用ConfigType取值
ConfigEnum ConfigEnums[] =
{
  {ConfigType::roledata,      SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | TRADE_LOAD | SOCIAL_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::misc,          RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | TRADE_LOAD | SOCIAL_LOAD | TEAM_LOAD | GUILD_LOAD | MATCH_LOAD | AUCTION_LOAD | WEDDING_LOAD, nullptr},
  {ConfigType::item,          RELOAD | SCENE_LOAD | SESSION_LOAD | TRADE_LOAD | SOCIAL_LOAD | RECORD_LOAD | TEAM_LOAD | GUILD_LOAD | AUCTION_LOAD | STAT_LOAD| MATCH_LOAD | WEDDING_LOAD, nullptr},
  {ConfigType::reward,        RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::superai,       RELOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::npc,           RELOAD | SCENE_LOAD | SESSION_LOAD | TRADE_LOAD | SOCIAL_LOAD | RECORD_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::role,          SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD, nullptr},   // 修正好友老账号body,读物user配置表,以后开新服可以关闭这个选项
  {ConfigType::attributepoint, RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD, nullptr},   // 修正好友老账号body,读物user配置表,以后开新服可以关闭这个选项
  {ConfigType::baselevel,     RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD, nullptr},   // 修正好友老账号body,读物user配置表,以后开新服可以关闭这个选项
  {ConfigType::joblevel,      RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD, nullptr},   // 修正好友老账号body,读物user配置表,以后开新服可以关闭这个选项
  {ConfigType::compose,       RELOAD | SCENE_LOAD | SESSION_LOAD | SOCIAL_LOAD | TRADE_LOAD | RECORD_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::menu,          RELOAD | RECORD_LOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::portrait,      RELOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::quest,         RELOAD | RECORD_LOAD | SCENE_LOAD | SESSION_LOAD | TEAM_LOAD | GUILD_LOAD,   nullptr},
  {ConfigType::skill,         RELOAD | SCENE_LOAD | RECORD_LOAD | SESSION_LOAD | SOCIAL_LOAD | TEAM_LOAD | TRADE_LOAD |GUILD_LOAD, nullptr},
  {ConfigType::feature,       RELOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::seal,          SCENE_LOAD | SESSION_LOAD,               nullptr},
  {ConfigType::tower,         SCENE_LOAD | SESSION_LOAD | TEAM_LOAD,               nullptr},
  {ConfigType::manual,        RELOAD | RECORD_LOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::shop,          RELOAD | SCENE_LOAD | MATCH_LOAD | WEDDING_LOAD | GUILD_LOAD,                              nullptr},
  {ConfigType::tip,           SCENE_LOAD,                              nullptr},
  {ConfigType::buffer,        RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::music,         SCENE_LOAD,                              nullptr},
  {ConfigType::scene,         SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD | MATCH_LOAD, nullptr},
  {ConfigType::raid,          RELOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::lab,           SCENE_LOAD,                              nullptr},
  {ConfigType::boss,          RELOAD | SCENE_LOAD | SESSION_LOAD,               nullptr},
  {ConfigType::haircolor,     SCENE_LOAD | RECORD_LOAD | SESSION_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::mail,          SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | TRADE_LOAD | SOCIAL_LOAD | TEAM_LOAD | GUILD_LOAD | WEDDING_LOAD | MATCH_LOAD, nullptr},
  {ConfigType::monsteremoji,  SCENE_LOAD,                              nullptr},
  {ConfigType::interlocution, SCENE_LOAD,                              nullptr},
  {ConfigType::bus,           SCENE_LOAD,                              nullptr},
  {ConfigType::scenery,       RELOAD | SCENE_LOAD | RECORD_LOAD | GUILD_LOAD | DATA_LOAD,                          nullptr},
  {ConfigType::dialog,        RELOAD | SCENE_LOAD,                              nullptr},
  {ConfigType::actionanim,    SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::expression,    RELOAD | SCENE_LOAD | SESSION_LOAD | RECORD_LOAD | SOCIAL_LOAD | TRADE_LOAD | TEAM_LOAD | GUILD_LOAD, nullptr},
  {ConfigType::teamgoal,      RELOAD | TEAM_LOAD,                               nullptr},
  {ConfigType::guild,         RELOAD | RECORD_LOAD | SCENE_LOAD | SESSION_LOAD | GUILD_LOAD | DATA_LOAD,                 nullptr},
  {ConfigType::dojo,          RELOAD | SCENE_LOAD | SESSION_LOAD | TEAM_LOAD | RECORD_LOAD | GUILD_LOAD | SOCIAL_LOAD | TRADE_LOAD, nullptr },
  {ConfigType::npcfun,        RELOAD | SCENE_LOAD | SESSION_LOAD,               nullptr },
  {ConfigType::avatar,        RELOAD | SCENE_LOAD,                              nullptr },
  {ConfigType::shadercolor,   RELOAD | SCENE_LOAD,                              nullptr },
  {ConfigType::tradetype,     RELOAD | SCENE_LOAD | SESSION_LOAD | TRADE_LOAD | RECORD_LOAD,  nullptr },
  {ConfigType::treasure,      SCENE_LOAD,                                       nullptr },
  {ConfigType::activity,      RELOAD | SCENE_LOAD | SESSION_LOAD  ,                      nullptr },
  {ConfigType::xoclient,      RELOAD | SCENE_LOAD,                              nullptr },
  {ConfigType::timer,         RELOAD | SESSION_LOAD,                            nullptr },
  {ConfigType::operatereward, RELOAD | SESSION_LOAD,                              nullptr },
  {ConfigType::speffect,      RELOAD | SCENE_LOAD,                              nullptr },
  {ConfigType::deposit,       RELOAD | SCENE_LOAD | SESSION_LOAD,               nullptr },
  {ConfigType::loveletter,    SCENE_LOAD,               nullptr },  
  {ConfigType::weaponpet,     RELOAD | SCENE_LOAD | TEAM_LOAD,               nullptr },
  {ConfigType::valentine,     RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::guildraid,     RELOAD | GUILD_LOAD | RECORD_LOAD | SCENE_LOAD | SESSION_LOAD,               nullptr },
  {ConfigType::achieve,       RELOAD | RECORD_LOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::dateland,      RELOAD | SCENE_LOAD | SESSION_LOAD , nullptr },
  {ConfigType::astrolabe,     RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::pet,           RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::food,          RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::recipe,        RELOAD | RECORD_LOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::cookerlevel,   RELOAD | RECORD_LOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::tasterlevel,   RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::effect,        RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::soundeffect,   RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::actshortcutpower, RELOAD | SESSION_LOAD, nullptr},
  {ConfigType::body,          RELOAD | SCENE_LOAD, nullptr},
  {ConfigType::tutor,         RELOAD | SCENE_LOAD,                nullptr },
  {ConfigType::action,        RELOAD | SCENE_LOAD,                nullptr },
  {ConfigType::being,         RELOAD | SCENE_LOAD,                nullptr },
  {ConfigType::highrefine,    RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::wedding,       RELOAD | SCENE_LOAD | SESSION_LOAD | WEDDING_LOAD ,               nullptr },
  {ConfigType::question,      RELOAD | SCENE_LOAD,                nullptr },
  {ConfigType::pvecard,       RELOAD | SCENE_LOAD | TEAM_LOAD | SESSION_LOAD,               nullptr },
  {ConfigType::servant,       RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::dead,          RELOAD | SCENE_LOAD ,               nullptr },
  {ConfigType::cloth,         RELOAD | SCENE_LOAD,               nullptr },
  {ConfigType::sysmsg,        RELOAD | SCENE_LOAD | SESSION_LOAD | TEAM_LOAD | RECORD_LOAD | GUILD_LOAD | SOCIAL_LOAD | TRADE_LOAD, nullptr },
  {ConfigType::transfer,      RELOAD | SCENE_LOAD,               nullptr },
  {ConfigType::exchangeshop,  RELOAD | SCENE_LOAD,               nullptr },
  {ConfigType::exchangeworth, RELOAD | SCENE_LOAD,               nullptr },
};

ConfigManager::ConfigManager()
{
  init();
}

ConfigManager::~ConfigManager()
{

}

void ConfigManager::init()
{
  m_list.clear();

  m_list["Table_RoleData.txt"].push_back(ConfigType::roledata);

  m_list["Table_Item.txt"].push_back(ConfigType::item);
  m_list["Table_UseItem.txt"].push_back(ConfigType::item);
  m_list["Table_Equip.txt"].push_back(ConfigType::item);
  m_list["Table_EquipLottery.txt"].push_back(ConfigType::item);
  m_list["Table_Card.txt"].push_back(ConfigType::item);
  m_list["Table_Mount.txt"].push_back(ConfigType::item);
  m_list["Table_EquipMaster.txt"].push_back(ConfigType::item);
  m_list["Table_EquipSuit.txt"].push_back(ConfigType::item);
  m_list["Table_EquipRefine.txt"].push_back(ConfigType::item);
  m_list["Table_EquipFashion.txt"].push_back(ConfigType::item);
  m_list["Table_ItemOrigin.txt"].push_back(ConfigType::item);
  m_list["Table_ItemType.txt"].push_back(ConfigType::item);
  m_list["Table_EquipEnchant.txt"].push_back(ConfigType::item);
  m_list["Table_Exchange.txt"].push_back(ConfigType::item);
  m_list["Table_Appellation.txt"].push_back(ConfigType::item);
  m_list["Table_Lottery.txt"].push_back(ConfigType::item);
  m_list["Table_LotteryGive.txt"].push_back(ConfigType::item);
  m_list["Table_HeadEffect.txt"].push_back(ConfigType::item);

  m_list["GameConfig.txt"].push_back(ConfigType::misc);

  m_list["Table_Reward.txt"].push_back(ConfigType::reward);

  m_list["Table_SuperAI.txt"].push_back(ConfigType::superai);

  m_list["Table_Npc.txt"].push_back(ConfigType::npc);
  m_list["Table_Monster.txt"].push_back(ConfigType::npc);
  m_list["Table_NpcTalk.txt"].push_back(ConfigType::npc);
  m_list["Table_Character.txt"].push_back(ConfigType::npc);
  m_list["Table_MonsterSkill.txt"].push_back(ConfigType::npc);
  m_list["Table_MonsterEvolution.txt"].push_back(ConfigType::npc);

  m_list["Table_Class.txt"].push_back(ConfigType::role);
  m_list["Table_Profess.txt"].push_back(ConfigType::role);
  m_list["Table_AttributePoint.txt"].push_back(ConfigType::attributepoint);
  m_list["Table_BaseLevel.txt"].push_back(ConfigType::baselevel);
  m_list["Table_JobLevel.txt"].push_back(ConfigType::joblevel);

  m_list["Table_Compose.txt"].push_back(ConfigType::compose);

  m_list["Table_Menu.txt"].push_back(ConfigType::menu);

  m_list["Table_HeadImage.txt"].push_back(ConfigType::portrait);

  m_list["Table_Quest.txt"].push_back(ConfigType::quest);
  m_list["Table_WantedQuest.txt"].push_back(ConfigType::quest);
  m_list["Table_Dialog.txt"].push_back(ConfigType::quest);
  m_list["Table_ExpPool.txt"].push_back(ConfigType::quest);

  m_list["Table_Skill.txt"].push_back(ConfigType::skill);

  m_list["NpcFeatures.txt"].push_back(ConfigType::feature);

  m_list["Table_RepairSeal.txt"].push_back(ConfigType::seal);
  m_list["Table_SealMonster.txt"].push_back(ConfigType::seal);

  m_list["Table_EndLessTower.txt"].push_back(ConfigType::tower);
  m_list["Table_EndLessMonster.txt"].push_back(ConfigType::tower);

  m_list["Table_Achievement.txt"].push_back(ConfigType::manual);
  m_list["Table_ItemTypeAdventureLog.txt"].push_back(ConfigType::manual);
  m_list["Table_ItemType.txt"].push_back(ConfigType::manual);
  m_list["Table_AdventureLevel.txt"].push_back(ConfigType::manual);
  m_list["Table_AdventureAppend.txt"].push_back(ConfigType::manual);

  m_list["Table_Shop.txt"].push_back(ConfigType::shop);

  m_list["Table_RedTip.txt"].push_back(ConfigType::tip);

  m_list["Table_Buffer.txt"].push_back(ConfigType::buffer);

  m_list["Table_MusicBox.txt"].push_back(ConfigType::music);

  m_list["Table_Map.txt"].push_back(ConfigType::scene);
  m_list["Table_MapRaid.txt"].push_back(ConfigType::scene);

  m_list["Table_Raid.txt"].push_back(ConfigType::raid);

  m_list["Table_Laboratory.txt"].push_back(ConfigType::lab);

  m_list["Table_Boss.txt"].push_back(ConfigType::boss);
  m_list["Table_HairColor.txt"].push_back(ConfigType::haircolor);
  m_list["Table_Mail.txt"].push_back(ConfigType::mail);
  m_list["Table_MonsterEmoji.txt"].push_back(ConfigType::monsteremoji);
  m_list["Table_xo_server.txt"].push_back(ConfigType::interlocution);
  m_list["Table_Bus.txt"].push_back(ConfigType::bus);
  m_list["Table_Viewspot.txt"].push_back(ConfigType::scenery);
  m_list["Table_Dialog.txt"].push_back(ConfigType::dialog);
  m_list["Table_ActionAnime.txt"].push_back(ConfigType::actionanim);
  m_list["Table_Expression.txt"].push_back(ConfigType::expression);
  m_list["Table_TeamGoals.txt"].push_back(ConfigType::teamgoal);
  m_list["Table_Guild.txt"].push_back(ConfigType::guild);
  m_list["Table_Guild_Faith.txt"].push_back(ConfigType::guild);
  m_list["Table_GuildFunction.txt"].push_back(ConfigType::guild);
  m_list["Table_GuildBuilding.txt"].push_back(ConfigType::guild);
  m_list["Table_GuildBuildingMaterial.txt"].push_back(ConfigType::guild);
  m_list["Table_Artifact.txt"].push_back(ConfigType::guild);
  m_list["Table_Guild_Dojo.txt"].push_back(ConfigType::dojo);
  m_list["Table_NpcFunction.txt"].push_back(ConfigType::npcfun);
  m_list["Table_Avatar.txt"].push_back(ConfigType::avatar);
  m_list["Table_ShaderColor.txt"].push_back(ConfigType::shadercolor);
  m_list["Table_Activity.txt"].push_back(ConfigType::activity);
  m_list["Table_xo.txt"].push_back(ConfigType::xoclient);  
  m_list["Table_Timer.txt"].push_back(ConfigType::timer);  
  m_list["Table_OperateReward.txt"].push_back(ConfigType::operatereward);
  m_list["Table_SpEffect.txt"].push_back(ConfigType::speffect);
  m_list["Table_Deposit.txt"].push_back(ConfigType::deposit);
  m_list["Table_MonthCard.txt"].push_back(ConfigType::deposit);
  m_list["Table_DepositFunction.txt"].push_back(ConfigType::deposit);
  m_list["Table_Valentine.txt"].push_back(ConfigType::valentine);
  m_list["Table_Achievement.txt"].push_back(ConfigType::achieve);
  m_list["Table_MercenaryCat.txt"].push_back(ConfigType::weaponpet);
  m_list["Table_GuildPVE_Map.txt"].push_back(ConfigType::guildraid);
  m_list["Table_DateLand.txt"].push_back(ConfigType::dateland);
  m_list["Table_Astrolabe.txt"].push_back(ConfigType::astrolabe);
  m_list["Table_Pet.txt"].push_back(ConfigType::pet);
  m_list["Table_Food.txt"].push_back(ConfigType::food);
  m_list["Table_Recipe.txt"].push_back(ConfigType::recipe);
  m_list["Table_Effect.txt"].push_back(ConfigType::effect);
  m_list["Table_SoundEffect.txt"].push_back(ConfigType::soundeffect);
  m_list["Table_ActivityShortcutPower.txt"].push_back(ConfigType::actshortcutpower);
  m_list["Table_StudentAdventureQuest.txt"].push_back(ConfigType::tutor);
  m_list["Table_TutorGrowUpReward.txt"].push_back(ConfigType::tutor);
  m_list["Table_Action.txt"].push_back(ConfigType::action);
  m_list["Table_Being.txt"].push_back(ConfigType::being);
  m_list["Table_BeingBaseLevel.txt"].push_back(ConfigType::being);
  m_list["Table_HighRefineMatCompose.txt"].push_back(ConfigType::highrefine);
  m_list["Table_HighRefine.txt"].push_back(ConfigType::highrefine);
  m_list["Table_Wedding.txt"].push_back(ConfigType::wedding);
  m_list["Table_WeddingService.txt"].push_back(ConfigType::wedding);
  m_list["Talbe_PveCard.txt"].push_back(ConfigType::pvecard);
  m_list["Table_Recommend.txt"].push_back(ConfigType::servant);
  m_list["Table_Growth.txt"].push_back(ConfigType::servant);
  m_list["Table_Sysmsg.txt"].push_back(ConfigType::sysmsg);
  m_list["Table_DeathTransferMap.txt"].push_back(ConfigType::transfer);
  m_list["Table_ExchangeShop.txt"].push_back(ConfigType::exchangeshop);
  m_list["Table_ExchangeWorth.txt"].push_back(ConfigType::exchangeworth);

  m_list["Table_DeadLevel.txt"].push_back(ConfigType::dead);
}

bool ConfigManager::getConfig(ConfigType type, ConfigEnum &item)
{
  DWORD len = sizeof(ConfigEnums) / sizeof(ConfigEnum);
  for (DWORD i = 0; i < len; ++i)
  {
    ConfigEnum &it = ConfigEnums[i];
    if (it.type == type)
    {
      item = it;
      return true;
    }
  }
  return false;
}

bool ConfigManager::loadConfigbyType(LOADTYPE type)
{
  XLOG << "[开始加载配置] type:" << type << XEND;
  bool bResult = true;
  DWORD len = sizeof(ConfigEnums) / sizeof(ConfigEnum);
  // load config
  for (DWORD i = 0; i < len; ++i)
  {
    ConfigEnum &item = ConfigEnums[i];
    if (item.isTypeLoad(type))
    {
      XLOG << "[开始加载配置] type:" << type <<"config type:"<<item.type<< XEND;
      if (loadConfig(item) == false)
        bResult = false;
    }
  }
  // check config
  for (DWORD i = 0; i < len; ++i)
  {
    ConfigEnum &item = ConfigEnums[i];
    if (item.isTypeLoad(type))
    {
      if (checkConfig(item) == false)
        bResult = false;
    }
  }
  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

bool ConfigManager::loadConfig(const ConfigEnum& item)
{
  bool bResult = false;
  switch (item.type)
  {
    case ConfigType::max:
      break;
    case ConfigType::roledata:
      RoleDataConfig::getMe().delMe();
      bResult = RoleDataConfig::getMe().loadConfig();
      break;
    case ConfigType::item:
      ItemConfig::getMe().delMe();
      bResult = ItemConfig::getMe().loadConfig();
      break;
    case ConfigType::misc:
      MiscConfig::getMe().delMe();
      bResult = MiscConfig::getMe().loadConfig();
      break;
    case ConfigType::reward:
      // 有会话发送来的交换reward,只能清空配置列表重加载
      bResult = RewardConfig::getMe().loadConfig();
      break;
    case ConfigType::superai:
      SuperAIConfig::getMe().delMe();
      bResult = SuperAIConfig::getMe().loadConfig();
      break;
    case ConfigType::npc:
      NpcConfig::getMe().delMe();
      bResult = NpcConfig::getMe().loadConfig();
      break;
    case ConfigType::role:
      RoleConfig::getMe().delMe();
      bResult = RoleConfig::getMe().loadConfig();
      break;
    case ConfigType::attributepoint:
      AttributePointConfig::getMe().delMe();
      bResult = AttributePointConfig::getMe().loadConfig();
      break;
    case ConfigType::baselevel:
      BaseLevelConfig::getMe().delMe();
      bResult = BaseLevelConfig::getMe().loadConfig();
      break;
    case ConfigType::joblevel:
      JobLevelConfig::getMe().delMe();
      bResult = JobLevelConfig::getMe().loadConfig();
      break;
    case ConfigType::compose:
      ComposeConfig::getMe().delMe();
      bResult = ComposeConfig::getMe().loadConfig();
      break;
    case ConfigType::menu:
      MenuConfig::getMe().delMe();
      bResult = MenuConfig::getMe().loadConfig();
      break;
    case ConfigType::portrait:
      PortraitConfig::getMe().delMe();
      bResult = PortraitConfig::getMe().loadConfig();
      break;
    case ConfigType::quest:
      QuestConfig::getMe().delMe();
      bResult = QuestConfig::getMe().loadConfig();
      break;
    case ConfigType::skill:
      SkillConfig::getMe().delMe();
      bResult = SkillConfig::getMe().loadConfig();
      break;
    case ConfigType::feature:
      FeatureConfig::getMe().delMe();
      bResult = FeatureConfig::getMe().loadConfig();
      break;
    case ConfigType::seal:
      SealConfig::getMe().delMe();
      bResult = SealConfig::getMe().loadConfig();
      break;
    case ConfigType::tower:
      // 有会话发送来的交换monster,只能清空配置列表重加载
      bResult = TowerConfig::getMe().loadConfig();
      break;
    case ConfigType::manual:
      ManualConfig::getMe().delMe();
      bResult = ManualConfig::getMe().loadConfig();
      break;
    case ConfigType::shop:
      ShopConfig::getMe().delMe();
      bResult = ShopConfig::getMe().loadConfig();
      break;
    case ConfigType::tip:
      TipConfig::getMe().delMe();
      bResult = TipConfig::getMe().loadConfig();
      break;
    case ConfigType::boss:
      bResult = BossConfig::getMe().loadConfig();
      break;
    case ConfigType::buffer:
    case ConfigType::music:
    case ConfigType::haircolor:
    case ConfigType::mail:
    case ConfigType::monsteremoji:
    case ConfigType::interlocution:
    case ConfigType::bus:
    case ConfigType::scenery:
    case ConfigType::dialog:
    case ConfigType::actionanim:
    case ConfigType::expression:
    case ConfigType::teamgoal:
    case ConfigType::shadercolor:
    case ConfigType::tradetype:
    case ConfigType::speffect:
    case ConfigType::npcfun:
    case ConfigType::timer:
    case ConfigType::loveletter:
    case ConfigType::cookerlevel:
    case ConfigType::tasterlevel:
    case ConfigType::effect:
    case ConfigType::soundeffect:
    case ConfigType::actshortcutpower:
    case ConfigType::body:
    case ConfigType::question:
    case ConfigType::cloth:
    case ConfigType::exchangeshop:
    case ConfigType::exchangeworth:
      bResult = TableManager::getMe().loadConfig(item.type);
      break;
    case ConfigType::scene:
      bResult = MapConfig::getMe().loadConfig();
      break;
    case ConfigType::raid:
      bResult = true;
      break;
    case ConfigType::lab:
      bResult = true;
      break;
    case ConfigType::guild:
      GuildConfig::getMe().delMe();
      bResult = GuildConfig::getMe().loadConfig();
      break;
    case ConfigType::dojo:
      DojoConfig::getMe().delMe();
      bResult = DojoConfig::getMe().loadConfig();
      break;
    case ConfigType::avatar:
      AvatarConfig::getMe().delMe();
      bResult = AvatarConfig::getMe().loadConfig();
      break;
    case ConfigType::treasure:
      TreasureConfig::getMe().delMe();
      bResult = TreasureConfig::getMe().loadConfig();
      break;
    case ConfigType::activity:
      ActivityConfig::getMe().delMe();
      bResult = ActivityConfig::getMe().loadConfig();
      break;
    case ConfigType::xoclient:
      XoclientConfig::getMe().delMe();
      bResult = XoclientConfig::getMe().loadConfig();
      break;
    case ConfigType::operatereward:
      OperateRewardConfig::getMe().delMe();
      bResult = OperateRewardConfig::getMe().loadConfig();
      break;
    case ConfigType::deposit:
      DepositConfig::getMe().delMe();
      bResult = DepositConfig::getMe().loadConfig();
      break;
    case ConfigType::valentine:
      ValentineConfig::getMe().delMe();
      bResult = ValentineConfig::getMe().loadConfig();
      break;
    case ConfigType::achieve:
      AchieveConfig::getMe().delMe();
      bResult = AchieveConfig::getMe().loadConfig();
      break;
    case ConfigType::weaponpet:
      WeaponPetConfig::getMe().delMe();
      bResult = WeaponPetConfig::getMe().loadConfig();
      break;
    case ConfigType::guildraid:
      GuildRaidConfig::getMe().delMe();
      GuildRaidConfig::getMe().loadConfig();
      break;
    case ConfigType::dateland:
      DateLandConfig::getMe().delMe();
      DateLandConfig::getMe().loadConfig();
      break;
    case ConfigType::astrolabe:
      AstrolabeConfig::getMe().delMe();
      bResult = AstrolabeConfig::getMe().loadConfig();
      break;
    case ConfigType::pet:
      //PetConfig::getMe().delMe();
      bResult = PetConfig::getMe().loadConfig();
      break;
    case ConfigType::food:
      FoodConfig::getMe().delMe();
      bResult = FoodConfig::getMe().loadConfig();
      break;
    case ConfigType::recipe:
      RecipeConfig::getMe().delMe();
      bResult = RecipeConfig::getMe().loadConfig();
      break;
    case ConfigType::tutor:
      TutorConfig::getMe().delMe();
      bResult = TutorConfig::getMe().loadConfig();
      break;
    case ConfigType::action:
      ActionConfig::getMe().delMe();
      bResult = ActionConfig::getMe().loadConfig();
      break;
    case ConfigType::being:
      BeingConfig::getMe().delMe();
      bResult = BeingConfig::getMe().loadConfig();
      break;
    case ConfigType::highrefine:
      HighRefineConfig::getMe().delMe();
      bResult = HighRefineConfig::getMe().loadConfig();
      break;
    case ConfigType::wedding:
      WeddingConfig::getMe().delMe();
      bResult = WeddingConfig::getMe().loadConfig();
      break;
    case ConfigType::pvecard:
      PveCardConfig::getMe().delMe();
      bResult = PveCardConfig::getMe().loadConfig();
      break;
    case ConfigType::servant:
      ServantConfig::getMe().delMe();
      bResult = ServantConfig::getMe().loadConfig();
      break;
    case ConfigType::sysmsg:
      SysmsgConfig::getMe().delMe();
      bResult = SysmsgConfig::getMe().loadConfig();
      break;
    case ConfigType::transfer:
      DeathTransferConfig::getMe().delMe();
      bResult = DeathTransferConfig::getMe().loadConfig();
      break;
    case ConfigType::dead:
      bResult = DeadConfig::getMe().loadConfig();
      break;
  }

  XLOG << "[配置管理-加载配置] type : " << item.type << " result : " << bResult << XEND;
  return bResult;
}

bool ConfigManager::checkConfig(const ConfigEnum& item)
{
  bool bResult = true;
  switch (item.type)
  {
    case ConfigType::max:
      break;
    case ConfigType::roledata:
      //bResult = RoleDataConfig::getMe().checkConfig();
      break;
    case ConfigType::item:
      bResult = ItemConfig::getMe().checkConfig();
      break;
    case ConfigType::misc:
      bResult = MiscConfig::getMe().checkConfig();
      break;
    case ConfigType::reward:
      bResult = RewardConfig::getMe().checkConfig();
      break;
    case ConfigType::superai:
      //bResult = SuperAIConfig::getMe().checkConfig();
      break;
    case ConfigType::npc:
      bResult = NpcConfig::getMe().checkConfig();
      break;
    case ConfigType::role:
    case ConfigType::attributepoint:
    case ConfigType::baselevel:
    case ConfigType::joblevel:
      break;
    case ConfigType::compose:
      //bResult = ComposeConfig::getMe().checkConfig();
      break;
    case ConfigType::menu:
      bResult = MenuConfig::getMe().checkConfig();
      break;
    case ConfigType::portrait:
      //bResult = PortraitConfig::getMe().checkConfig();
      break;
    case ConfigType::quest:
      bResult = QuestConfig::getMe().checkConfig();
      break;
    case ConfigType::skill:
      //bResult = SkillConfig::getMe().checkConfig();
      break;
    case ConfigType::feature:
      //bResult = FeatureConfig::getMe().checkConfig();
      break;
    case ConfigType::seal:
      bResult = SealConfig::getMe().checkConfig();
      break;
    case ConfigType::tower:
      bResult = TowerConfig::getMe().checkConfig();
      break;
    case ConfigType::manual:
      bResult = ManualConfig::getMe().checkConfig();
      break;
    case ConfigType::shop:
      bResult = ShopConfig::getMe().checkConfig();
      break;
    case ConfigType::tip:
      bResult = TipConfig::getMe().checkConfig();
      break;
    case ConfigType::buffer:
    case ConfigType::music:
      bResult = TableManager::getMe().checkConfig(item.type);
      break;
    case ConfigType::scene:
      bResult = MapConfig::getMe().checkConfig();
      break;
    case ConfigType::dojo:
      bResult = DojoConfig::getMe().checkConfig();
      break;
    case ConfigType::boss:
      bResult = BossConfig::getMe().checkConfig();
      break;
    case ConfigType::raid:
    case ConfigType::lab:
    case ConfigType::haircolor:
    case ConfigType::mail:
    case ConfigType::monsteremoji:
    case ConfigType::interlocution:
    case ConfigType::bus:
    case ConfigType::scenery:
    case ConfigType::dialog:
    case ConfigType::actionanim:
    case ConfigType::expression:
    case ConfigType::teamgoal:
    case ConfigType::npcfun:
    case ConfigType::shadercolor:
    case ConfigType::tradetype:
    case ConfigType::xoclient:
    case ConfigType::treasure:
    case ConfigType::activity:
    case ConfigType::timer:
    case ConfigType::operatereward:
    case ConfigType::speffect:
    case ConfigType::loveletter:
    case ConfigType::cookerlevel:
    case ConfigType::tasterlevel:
    case ConfigType::effect:
    case ConfigType::soundeffect:
    case ConfigType::actshortcutpower:
    case ConfigType::body:
    case ConfigType::question:
    case ConfigType::cloth:
    case ConfigType::exchangeshop:
    case ConfigType::exchangeworth:
      TableManager::getMe().checkConfig(item.type);
      break;
    case ConfigType::guild:
      bResult = GuildConfig::getMe().checkConfig();
      break;
    case ConfigType::avatar:
      bResult = AvatarConfig::getMe().checkConfig();
      break;
    case ConfigType::deposit:
      bResult = DepositConfig::getMe().checkConfig();
      break;
    case ConfigType::valentine:
      bResult = ValentineConfig::getMe().checkConfig();
      break;
    case ConfigType::achieve:
      bResult = AchieveConfig::getMe().checkConfig();
      break;
    case ConfigType::weaponpet:
      break;
    case ConfigType::guildraid:
      break;
    case ConfigType::dateland:
      bResult = DateLandConfig::getMe().checkConfig();
      break;
    case ConfigType::astrolabe:
      bResult = AstrolabeConfig::getMe().checkConfig();
      break;
    case ConfigType::pet:
      bResult = PetConfig::getMe().checkConfig();
      break;
    case ConfigType::food:
      bResult = FoodConfig::getMe().checkConfig();
      break;
    case ConfigType::recipe:
      bResult = RecipeConfig::getMe().checkConfig();
      break;
    case ConfigType::tutor:
      bResult = TutorConfig::getMe().checkConfig();
      break;
    case ConfigType::action:
      bResult = ActionConfig::getMe().checkConfig();
      break;
    // case ConfigType::cookerlevel:
    //   bResult = CookerLevelCofnig::getMe().checkConfig();
    //   break;
    // case ConfigType::tasterlevel:
    //   bResult = TasterLevelCofnig::getMe().checkConfig();
    //   break;
    case ConfigType::being:
      bResult = BeingConfig::getMe().checkConfig();
      break;
    case ConfigType::highrefine:
      bResult = HighRefineConfig::getMe().checkConfig();
      break;
    case ConfigType::wedding:
      bResult = WeddingConfig::getMe().checkConfig();
      break;
    case ConfigType::pvecard:
      bResult = PveCardConfig::getMe().checkConfig();
      break;
    case ConfigType::servant:
      //bResult = ServantConfig::getMe().checkConfig();
      break;
    case ConfigType::sysmsg:
      bResult = PveCardConfig::getMe().checkConfig();
      break;
    case ConfigType::transfer:
      bResult = DeathTransferConfig::getMe().checkConfig();
      break;
    case ConfigType::dead:
      bResult = DeadConfig::getMe().checkConfig();
      break;
  }

  if (item.loadfunc != nullptr)
    bResult = item.loadfunc();
  XLOG << "[配置管理-检查配置] type : " << item.type << " result : " << bResult << XEND;
  return bResult;
}

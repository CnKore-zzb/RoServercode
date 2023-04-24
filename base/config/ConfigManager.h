/**
 * @file ConfigManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-11
 */

#pragma once

#include "xSingleton.h"
#include "RoleDataConfig.h"
#include "ItemConfig.h"
#include "MiscConfig.h"
#include "RewardConfig.h"
#include "SuperAIConfig.h"
#include "UserConfig.h"
#include "ComposeConfig.h"
#include "MenuConfig.h"
#include "PortraitConfig.h"
#include "NpcConfig.h"
#include "QuestConfig.h"
#include "SkillConfig.h"
#include "FeatureConfig.h"
#include "SealConfig.h"
#include "TowerConfig.h"
#include "ManualConfig.h"
#include "ShopConfig.h"
#include "TipConfig.h"
#include "TableManager.h"
#include "GuildConfig.h"
#include "MapConfig.h"
#include "DojoConfig.h"
#include "AvatarConfig.h"
#include "TreasureConfig.h"
#include "ActivityConfig.h"
#include "AchieveConfig.h"
#include "AstrolabeConfig.h"
#include "ActionConfig.h"

enum LOADTYPE
{
  RELOAD = 1 << 0,
  SCENE_LOAD = 1 << 1,
  SESSION_LOAD = 1 << 2,
  RECORD_LOAD = 1 << 3,
  TRADE_LOAD = 1 << 4,
  SOCIAL_LOAD = 1 << 5,
  TEAM_LOAD = 1 << 6,
  GUILD_LOAD = 1 << 7,
  MATCH_LOAD = 1 << 8,
  AUCTION_LOAD = 1 << 9,
  STAT_LOAD = 1 << 10,
  WEDDING_LOAD = 1 << 11,
  DATA_LOAD = 1 << 12,
};

struct ConfigEnum
{
  ConfigType type;
  DWORD bits;

  std::function<bool()> loadfunc;

  bool isReload() { return bits & RELOAD; }
  // SceneConfig 无效 默认加载
  bool isSceneLoad() { return bits & SCENE_LOAD; }
  // SessionConfig 无效 默认加载
  bool isSessionLoad() { return bits & SESSION_LOAD; }
  // RecordConfig 无效 默认加载
  bool isRecordLoad() { return bits & RECORD_LOAD; }
  // GuildConfig 无效 默认加载
  bool isGuildLoad() { return bits & GUILD_LOAD; }
  // RecordConfig 无效 默认加载
  bool isTypeLoad(LOADTYPE type) { return bits & type; }
};

typedef std::vector<ConfigType> TVecConfigType;

// ConfigManager
class ConfigManager : public xSingleton<ConfigManager>
{
  friend class xSingleton<ConfigManager>;
  private:
    ConfigManager();
  public:
    virtual ~ConfigManager();

  private:
    void init();

  public:
    bool loadSceneConfig() { return loadConfigbyType(SCENE_LOAD); }
    bool loadSessionConfig() { return loadConfigbyType(SESSION_LOAD); }
    bool loadRecordConfig() { return loadConfigbyType(RECORD_LOAD); }
    bool loadTradeConfig() { return loadConfigbyType(TRADE_LOAD); }
    bool loadSocialConfig() { return loadConfigbyType(SOCIAL_LOAD); }
    bool loadTeamConfig() { return loadConfigbyType(TEAM_LOAD); }
    bool loadGuildConfig() { return loadConfigbyType(GUILD_LOAD); }
    bool loadMatchConfig() { return loadConfigbyType(MATCH_LOAD);  }
    bool loadAuctionConfig() { return loadConfigbyType(AUCTION_LOAD); }
    bool loadStatConfig() { return loadConfigbyType(STAT_LOAD); }
    bool loadWeddingConfig() { return loadConfigbyType(WEDDING_LOAD); }
    bool loadDataConfig() { return loadConfigbyType(DATA_LOAD); }
  public:
    bool getConfig(ConfigType type, ConfigEnum &item);
    bool loadConfigbyType(LOADTYPE type);
    bool loadConfig(const ConfigEnum& item);
    bool checkConfig(const ConfigEnum& item);
  public:
    void getType(std::string name, TVecConfigType &vec)
    {
      auto it = m_list.find(name);
      if (it != m_list.end())
        vec = it->second;
    }
  public:
    std::map<std::string, TVecConfigType> m_list;
};


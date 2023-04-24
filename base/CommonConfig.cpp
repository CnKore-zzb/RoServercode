#include "CommonConfig.h"
#include "xLog.h"
extern "C"
{
#include "md5/md5.h"
#include "crypto/hmac.h"
}

DWORD CommonConfig::m_dwClientCmdMax = 0;
DWORD CommonConfig::m_dwClientCmdSec = 0;
DWORD CommonConfig::m_dwActiveUserMax = 0;
DWORD CommonConfig::m_dwOnlineUserMax = 0;
std::map<DWORD, ActiveOnline> CommonConfig::m_oActiveOnlineList;
DWORD CommonConfig::m_dwOnceOpenZoneNum = 3;
DWORD CommonConfig::m_dwNewZoneProtectiveTime = 86400;
DWORD CommonConfig::m_dwMaxScopeNum = 50;
DWORD CommonConfig::m_dwMaxFriendScopeNum = 0;
DWORD CommonConfig::m_dwMailSendMinTick = 60;
DWORD CommonConfig::m_dwMailSendMaxTick = 120;
DWORD CommonConfig::m_dwMailSendMaxCount = 10;
DWORD CommonConfig::m_dwSkillDelayMs = 1000;
float CommonConfig::m_fSkillSyncDis = 1.0;
float CommonConfig::m_fEnemySpedErrDis = 1;
DWORD CommonConfig::m_dwSkillQueueDelayMs = 500;
DWORD CommonConfig::m_dwSkillDisQueueMax = 3;
DWORD CommonConfig::m_dwSkillQueueCntTime = 600;
std::map<DWORD, DWORD> CommonConfig::m_mapDelCharTime;
std::map<DWORD, TSetDWORD> CommonConfig::m_mapIgnoreCmd;
DWORD CommonConfig::m_dwClientCmdLimitMax = 20;
DWORD CommonConfig::m_dwClientCmdLimitSec = 1;
std::map<DWORD, DWORD> CommonConfig::m_mapClientCmdLimit;
DWORD CommonConfig::m_dwSocialLoadTime = 0;
DWORD CommonConfig::m_dwGuildFrameOvertime = 0;
bool CommonConfig::m_bGuildOptOpen = false;
DWORD CommonConfig::m_dwPwdFailTimes = 5;
DWORD CommonConfig::m_dwPwdIntervalTime = 300;
DWORD CommonConfig::m_dwDelCharCD = 20160 * 60;
DWORD CommonConfig::m_dwSkillBreakErr = 200;
float CommonConfig::m_fSkillMoveDisErr = 0.2;
std::string CommonConfig::m_strUpyunAuthValue;
std::string CommonConfig::m_strUpyunPwd;
std::set<QWORD> CommonConfig::m_setTFGMWhiteList;
DWORD CommonConfig::m_dwVerifyServerPlatID = 5;
DWORD CommonConfig::m_dwChatFilterListSize = 65534;
DWORD CommonConfig::m_dwChatFilterTimeout = 1;
std::string CommonConfig::m_strLoginSign;
std::string CommonConfig::m_strSafeDeviceSign;
std::string CommonConfig::m_strAuthorizeSign;
bool CommonConfig::m_bJumpZoneCheckLevel = true;
DWORD CommonConfig::m_dwFluentLogCount = 10;
DWORD CommonConfig::m_dwFluentRetryCount = 4;
DWORD CommonConfig::m_dwGuildGMFlushTick = 300;
DWORD CommonConfig::m_dwLuaAlterCD = 300;
DWORD CommonConfig::m_dwOneSecMaxKickUserNum = 50;
DWORD CommonConfig::m_dwBuffExecPrintTime = 1000;
DWORD CommonConfig::m_dwSkillExecPrintTime = 3000;
ELanguageType CommonConfig::m_eDefaultLanguage = ELANGUAGE_English;
bool CommonConfig::m_bLua = true;
bool CommonConfig::m_bOpenRollback = false;
DWORD CommonConfig::SCENE_USER_GROUP_NUM = 1;
string CommonConfig::m_strSuperGvgSceneName;
DWORD CommonConfig::m_dwSuperGvgRefreshInterval = 2;
bool CommonConfig::m_bPackSyncNew = true;

CommonConfig::CommonConfig()
{

}

CommonConfig::~CommonConfig()
{

}

bool CommonConfig::loadConfig()
{
  if (!xLuaTable::getMe().open("CommonConfig.lua"))
  {
    XERR << "[CommonConfig], 加载配置CommonConfig.lua失败" << XEND;
    return false;
  }
  XLOG << "[CommonConfig] 加载配置CommonConfig.lua" << XEND;

  m_oData.clear();
  xLuaTable::getMe().getLuaData("CommonConfig", m_oData);

  XLOG << "[CommonConfig], load"<<m_oData.getTableString("test")<<XEND;

  if (m_oData.has("open_trade_server"))
  {
    m_bOpenTradeServer = m_oData.getTableInt("open_trade_server");
  }
  else
  {
    m_bOpenTradeServer = true;
  }

  if (m_oData.has("gchar_load_db_need"))
  {
    m_bGCharLoadDBNeed = m_oData.getTableInt("gchar_load_db_need") == 1;
  }
  else
  {
    m_bGCharLoadDBNeed = true;
  }

  if (m_oData.has("client_cmd_max"))
  {
    m_dwClientCmdMax = m_oData.getTableInt("client_cmd_max");
  }
  if (m_oData.has("client_cmd_sec"))
  {
    m_dwClientCmdSec = m_oData.getTableInt("client_cmd_sec");
  }
  XLOG << "[通用配置]" << "客户端消息频率" << m_dwClientCmdMax << m_dwClientCmdSec << XEND;

  // 开服配置
  if (m_oData.has("ActiveOnline"))
  {
    m_dwActiveUserMax = 0;
    m_dwOnlineUserMax = 0;
    const xLuaData &data = m_oData.getData("ActiveOnline");
    for (auto &it : data.m_table)
    {
      DWORD l = atoi(it.first.c_str());
      m_oActiveOnlineList[l * MIN_T].dwOnline = it.second.getTableInt("pcu");
      m_oActiveOnlineList[l * MIN_T].dwActive = it.second.getTableInt("dau");
    }
    for (auto &it : m_oActiveOnlineList)
    {
      XLOG << "[开服配置]" << "开服时长" << it.first << "最大在线" << it.second.dwOnline << "最大活跃" << it.second.dwActive << XEND;
      if (it.second.dwOnline > m_dwOnlineUserMax)
        m_dwOnlineUserMax = it.second.dwOnline;
      if (it.second.dwActive > m_dwActiveUserMax)
        m_dwActiveUserMax = it.second.dwActive;
    }
  }
  XLOG << "[开服配置]" << "创建角色最大活跃" << m_dwActiveUserMax << "最大在线" << m_dwOnlineUserMax << XEND;
  if (m_oData.has("once_open_zone_num"))
  {
    m_dwOnceOpenZoneNum = m_oData.getTableInt("once_open_zone_num");
  }
  XLOG << "[开服配置]" << "一次新开区数量" << m_dwOnceOpenZoneNum << XEND;
  if (m_oData.has("new_zone_protective_time"))
  {
    m_dwNewZoneProtectiveTime = m_oData.getTableInt("new_zone_protective_time") * MIN_T;
  }
  XLOG << "[开服配置]" << "新区保护时间" << m_dwNewZoneProtectiveTime << XEND;
  // 开服配置

  if (m_oData.has("scope_max_user_num"))
  {
    m_dwMaxScopeNum = m_oData.getTableInt("scope_max_user_num");
    if (!m_dwMaxScopeNum) m_dwMaxScopeNum = 50;
  }
  if (m_oData.has("scope_max_friend_num"))
  {
    m_dwMaxFriendScopeNum = m_oData.getTableInt("scope_max_friend_num");
  }
  XLOG << "[通用配置]" << "地图同屏限制" << m_dwMaxScopeNum << m_dwMaxFriendScopeNum << XEND;

  if (m_oData.has("mail_send_min_tick"))
    m_dwMailSendMinTick = m_oData.getTableInt("mail_send_min_tick");
  if (m_oData.has("mail_send_max_tick"))
    m_dwMailSendMaxTick = m_oData.getTableInt("mail_send_max_tick");
  if (m_oData.has("mail_send_max_count"))
    m_dwMailSendMaxCount = m_oData.getTableInt("mail_send_max_count");
  if (m_oData.has("skill_delay_ms"))
    m_dwSkillDelayMs = m_oData.getTableInt("skill_delay_ms");
  if (m_oData.has("skill_sync_dis"))
    m_fSkillSyncDis = m_oData.getTableFloat("skill_sync_dis");
  if (m_oData.has("skill_queue_delay_ms"))
    m_dwSkillQueueDelayMs = m_oData.getTableInt("skill_queue_delay_ms");
  if (m_oData.has("skill_dis_queue_max"))
    m_dwSkillDisQueueMax = m_oData.getTableInt("skill_dis_queue_max");
  if (m_oData.has("enemy_sped_err_dis"))
    m_fEnemySpedErrDis = m_oData.getTableFloat("enemy_sped_err_dis");
  if (m_oData.has("skill_queue_cnt_time"))
    m_dwSkillQueueCntTime = m_oData.getTableInt("skill_queue_cnt_time");
  if (m_oData.has("skill_break_err"))
    m_dwSkillBreakErr = m_oData.getTableInt("skill_break_err");
  if (m_oData.has("skill_move_dis_err"))
    m_fSkillMoveDisErr = m_oData.getTableFloat("skill_move_dis_err");

  if (m_oData.has("pwd_max_fail_times"))
    m_dwPwdFailTimes = m_oData.getTableInt("pwd_max_fail_times");
  if (m_oData.has("pwd_interval_time"))
    m_dwPwdIntervalTime = m_oData.getTableInt("pwd_interval_time");
  if (m_oData.has("DelCharTime"))
  {
    m_mapDelCharTime.clear();
    const xLuaData& data = m_oData.getData("DelCharTime");
    for (auto &it : data.m_table)
      m_mapDelCharTime[atoi(it.first.c_str())] = it.second.getInt();
    for (auto &m : m_mapDelCharTime)
      XDBG << m.first << m.second << XEND;
  }
  if (m_oData.has("DelCharCD"))
    m_dwDelCharCD = m_oData.getTableInt("DelCharCD") * 60;

  if (m_oData.has("IgnoreCmdCount"))
  {
    m_mapIgnoreCmd.clear();
    const xLuaData& data = m_oData.getData("IgnoreCmdCount");
    for (auto &m : data.m_table)
    {
      TSetDWORD& setParams = m_mapIgnoreCmd[atoi(m.first.c_str())];
      for (auto &param : m.second.m_table)
        setParams.insert(param.second.getInt());
    }
#ifdef _DEBUG
    for (auto &m : m_mapIgnoreCmd)
    {
      for (auto &s : m.second)
        XDBG << "IgnoreCmdCount" << m.first << s << XEND;
    }
#endif
  }

  // 客户端消息频率控制
  if (m_oData.has("ClientCmdLimitMax"))
    m_dwClientCmdLimitMax = m_oData.getTableInt("ClientCmdLimitMax");
  if (m_oData.has("ClientCmdLimitSec"))
    m_dwClientCmdLimitSec = m_oData.getTableInt("ClientCmdLimitSec");
  XLOG << "[通用配置]" << "客户端消息频率控制" << "最大消息数" << m_dwClientCmdLimitMax << "时间间隔" << m_dwClientCmdLimitSec <<XEND;

  if (m_oData.has("ClientCmdLimits"))
  {
    const xLuaData& data = m_oData.getData("ClientCmdLimits");
    for (auto &cmd : data.m_table)
    {
      for (auto &param : cmd.second.m_table)
      {
        m_mapClientCmdLimit[((DWORD)(atoi(cmd.first.c_str()))<<16) + (DWORD)(atoi(param.first.c_str()))] = param.second.getInt();
        XLOG << "[通用配置]" << "客户端消息频率控制" << "cmd" << cmd.first << "param" << param.first << "最大消息数" << param.second.getInt() <<XEND;
      }
    }
  }

  if (m_oData.has("social_load_time"))
  {
    m_dwSocialLoadTime = m_oData.getTableInt("social_load_time");
    if (m_dwSocialLoadTime == 0)
      m_dwSocialLoadTime = 100;
  }

  if (m_oData.has("guild_frame_overtime"))
  {
    m_dwGuildFrameOvertime = m_oData.getTableInt("guild_frame_overtime");
    if (m_dwGuildFrameOvertime == 0)
      m_dwGuildFrameOvertime = 1000;
  }

  if (m_oData.has("guild_opt_open"))
  {
    m_bGuildOptOpen = m_oData.getTableInt("guild_opt_open") == 1;
  }

  if (m_oData.has("pvp_llh"))
  {
    m_bPvpLLH = m_oData.getTableInt("pvp_llh");
  }
  else
  {
    m_bPvpLLH = true;
  }

  if (m_oData.has("pvp_smzl"))
  {
    m_bPvpSMZL = m_oData.getTableInt("pvp_smzl");
  }
  else
  {
    m_bPvpSMZL = true;
  }

  if (m_oData.has("pvp_hljs"))
  {
    m_bPvpHLJS = m_oData.getTableInt("pvp_hljs");
  }
  else
  {
    m_bPvpHLJS = true;
  }
  
  if (m_oData.has("fluent_log_count"))
    m_dwFluentLogCount = m_oData.getTableInt("fluent_log_count");
  if (m_oData.has("fluent_retry_count"))
    m_dwFluentRetryCount = m_oData.getTableInt("fluent_retry_count");

  if (m_oData.has("guild_gm_flush_tick"))
    m_dwGuildGMFlushTick = m_oData.getTableInt("guild_gm_flush_tick");


  if (m_oData.has("upyun_authvalue"))
  {
    if (base64Encode(m_oData.getTableString("upyun_authvalue"), &m_strUpyunAuthValue) == false)
      XERR << "[通用配置]" << "upyun auth value转码失败" << XEND;
  }
  else
  {
    XERR << "[通用配置]" << "upyun auth value未配置" << XEND;
  }

  if (m_oData.has("upyun_password"))
  {
    char pwd_md5[1024];
    bzero(pwd_md5, sizeof(pwd_md5));
    std::string pwd = m_oData.getTableString("upyun_password");
    upyun_md5(pwd.c_str(), pwd.size(), pwd_md5);
    m_strUpyunPwd = pwd_md5;
  }
  else
  {
    XERR << "[通用配置]" << "upyun password未配置" << XEND;
  }
  
  if (m_oData.has("login_sign"))
  {
    m_strLoginSign = m_oData.getTableString("login_sign");
  }
  else
    m_strLoginSign = "rtyuio@#$%^&";

  if (m_oData.has("safedevice_sign"))
  {
    m_strSafeDeviceSign = m_oData.getTableString("safedevice_sign");
  }
  else
    m_strSafeDeviceSign = "bmfdsa@#$%^&";
  if (m_oData.has("authorize_sign"))
    m_strAuthorizeSign = m_oData.getTableString("authorize_sign");
  else
    m_strAuthorizeSign = "wdlmfdsa@#$%^&";

  m_setTFGMWhiteList.clear();
  if (m_oData.has("TFGMWhiteList"))
  {
    auto f = [&](const string& key, xLuaData& data)
    {
      m_setTFGMWhiteList.insert(data.getQWORD());
    };
    m_oData.getMutableData("TFGMWhiteList").foreach(f);
  }

  if (m_oData.has("verify_server_plat_id"))
    m_dwVerifyServerPlatID = m_oData.getTableInt("verify_server_plat_id");

  if(m_oData.has("chat_filter_list_size"))
  {
    m_dwChatFilterListSize = m_oData.getTableInt("chat_filter_list_size");
  }
  else
  {
    XERR << "[通用配置]" << "chat_filter_list_size 未配置, 使用默认配置：65535" << XEND;
  }

  if(m_oData.has("chat_filter_timeout"))
  {
    m_dwChatFilterTimeout = m_oData.getTableInt("chat_filter_timeout");
  }
  else
  {
    XERR << "[通用配置]" << "chat_filter_timeout 未配置, 使用默认配置：1" << XEND;
  }

  if (m_oData.has("default_language"))
  {
    m_eDefaultLanguage = static_cast<ELanguageType>(m_oData.getTableInt("default_language"));
  }

  if (m_oData.has("jumpzone_check_level"))
    m_bJumpZoneCheckLevel = m_oData.getTableInt("jumpzone_check_level") != 0;

  if (m_oData.has("lua_alter_cd"))
    m_dwLuaAlterCD = m_oData.getTableInt("lua_alter_cd");

  if (m_oData.has("onesec_max_kickuser_num"))
    m_dwOneSecMaxKickUserNum = m_oData.getTableInt("onesec_max_kickuser_num");

  if (m_oData.has("buff_exec_print_time"))
    m_dwBuffExecPrintTime = m_oData.getTableInt("buff_exec_print_time");
    
  if (m_oData.has("skill_exec_print_time"))
    m_dwSkillExecPrintTime = m_oData.getTableInt("skill_exec_print_time");

  if (m_oData.has("attr_lua_use"))
    m_bLua = m_oData.getTableInt("attr_lua_use") == 1;

  if (m_oData.has("open_rollback"))
    m_bOpenRollback = (m_oData.getTableInt("open_rollback") == 1);

  if (m_oData.has("scene_user_group_num"))
    SCENE_USER_GROUP_NUM = m_oData.getTableInt("scene_user_group_num");

  if (!SCENE_USER_GROUP_NUM) SCENE_USER_GROUP_NUM = 1;

  if (m_oData.has("pack_sync_new"))
    m_bPackSyncNew = m_oData.getTableInt("pack_sync_new") == 1;

  XLOG << "[CommonConfig]" << "玩家分组数:" << SCENE_USER_GROUP_NUM << XEND;

  if (m_oData.has("supergvg_scene"))
    m_strSuperGvgSceneName = m_oData.getTableString("supergvg_scene");
  if (m_oData.has("supergvg_refresh_time"))
    m_dwSuperGvgRefreshInterval = m_oData.getTableInt("supergvg_refresh_time");

  loadLanguageConfig();
  return true;
}

bool CommonConfig::upyun_form_str(std::string savekey, std::string &out_policy, std::string &out_signature, std::string type)
{
  DWORD timestamp = now() + 4800;

  std::stringstream policystream;
  policystream.str("");
  policystream << "{\"bucket\":\"ro-xdcdn\",\"expiration\":"<< timestamp << ",\"save-key\":\"" << savekey << "." << type << "\",\"content-lenth-range\":\"0,1024000\"}";
  base64Encode(policystream.str(), &out_policy);

  std::stringstream signstream;
  signstream.str("");
  signstream << "POST&/ro-xdcdn&" << out_policy;

  char sha1[HMAC_SHA1_LEN];
  bzero(sha1, sizeof(sha1));
  hmac_sha1(m_strUpyunPwd.c_str(), m_strUpyunPwd.size(), signstream.str().c_str(), signstream.str().size(), sha1);

  char base64[HMAC_SHA1_LEN << 1];
  bzero(base64, sizeof(base64));
  base64Encode((unsigned char*)sha1, base64, HMAC_SHA1_LEN);
  out_signature = base64;

  XLOG << "[upyun]," << out_policy << XEND;
  XLOG << "[upyun]," << out_signature << XEND;

  return true;
}

bool CommonConfig::loadLanguageConfig()
{
  // todo xde by CHEN JIAN
  static std::map<std::string, DWORD> list = {
          {"ChineseTraditional", ELANGUAGE_CHINESE_TRADITIONAL},
          {"Korean", ELANGUAGE_Korean}
  };

  for (auto &pair : list)
  {
    auto name = pair.first;
    auto type = pair.second;

    if (!xLuaTable::getMe().open(("lang/" + name + ".txt").c_str()))
    {
      XERR << "[CommonConfig], 加载配置" + name + ".txt失败，可能不存在" << XEND;
      continue;
    }

    xLuaData table;
    xLuaTable::getMe().getLuaData("Language", table);

    for (auto &m : table.m_table)
    {
      if (m.first == name)
      {
        auto &dict = m_oLanguageList[type];

        auto func = [&](const string&key, xLuaData& data)
        {
            string origin = data.getTableString("origin");
            string target = data.getTableString("target");

            if (!target.empty())
              dict.emplace(origin, target);
        };

        m.second.foreach(func);
        XLOG << "[" + name + ".txt] Chinese配置加载完成, 加载数目: " << dict.size() << XEND;

        break;
      }
    }
  }

  return true;
}

string CommonConfig::getTranslateString(const string& content, DWORD language)
{
  // todo xde by CHEN JIAN
  auto lang = m_oLanguageList.find(language);
  if (lang == m_oLanguageList.end())
    return content;

  auto dict = lang->second.find(content);
  if (dict == lang->second.end())
    return content;

  return dict->second;
}

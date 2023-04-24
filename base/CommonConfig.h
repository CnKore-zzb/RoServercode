#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

struct ActiveOnline
{
  DWORD dwActive = 0;
  DWORD dwOnline = 0;
};

enum ELanguageType
{
  ELANGUAGE_Afrikaans = 0,
  ELANGUAGE_Arabic = 1,
  ELANGUAGE_Basque = 2,
  ELANGUAGE_Belarusian = 3,
  ELANGUAGE_Bulgarian = 4,
  ELANGUAGE_Catalan = 5,
  ELANGUAGE_Chinese = 6,
  ELANGUAGE_Czech = 7,
  ELANGUAGE_Danish = 8,
  ELANGUAGE_Dutch = 9,
  ELANGUAGE_English = 10,
  ELANGUAGE_Estonian = 11,
  ELANGUAGE_Faroese = 12,
  ELANGUAGE_Finnish = 13,
  ELANGUAGE_French = 14,
  ELANGUAGE_German = 15,
  ELANGUAGE_Greek = 16,
  ELANGUAGE_Hebrew = 17,
  ELANGUAGE_Hungarian = 18,
  ELANGUAGE_Icelandic = 19,
  ELANGUAGE_Indonesian = 20,
  ELANGUAGE_Italian = 21,
  ELANGUAGE_Japanese = 22,
  ELANGUAGE_Korean = 23,
  ELANGUAGE_Latvian = 24,
  ELANGUAGE_Lithuanian = 25,
  ELANGUAGE_Norwegian = 26,
  ELANGUAGE_Polish = 27,
  ELANGUAGE_Portuguese = 28,
  ELANGUAGE_Romanian = 29,
  ELANGUAGE_Russian = 30,
  ELANGUAGE_SerboCroatian = 31,
  ELANGUAGE_Slovak = 32,
  ELANGUAGE_Slovenian = 33,
  ELANGUAGE_Spanish = 34,
  ELANGUAGE_Swedish = 35,
  ELANGUAGE_Thai = 36,
  ELANGUAGE_Turkish = 37,
  ELANGUAGE_Ukrainian = 38,
  ELANGUAGE_Vietnamese = 39,
  ELANGUAGE_CHINESE_SIMPLIFIED = 40,
  ELANGUAGE_CHINESE_TRADITIONAL = 41,
  ELANGUAGE_Unknown = 42,
};

class CommonConfig : public xSingleton<CommonConfig>
{
  public:
    CommonConfig();
    ~CommonConfig();
    bool loadConfig();
    bool loadLanguageConfig();

    bool IsTradeServerOpen() { return m_bOpenTradeServer; }
    bool IsGCharLoadDBNeed() const { return m_bGCharLoadDBNeed; }
  public:
    xLuaData m_oData;

  private:
    bool m_bOpenTradeServer = true;
    bool m_bGCharLoadDBNeed = true;

  private:
    std::map<DWORD, std::map<string, string>> m_oLanguageList;
  public:
    string getTranslateString(const string& content, DWORD language);

  public:
    // 消息频率检测
    //
    // 最大消息数量
    static DWORD m_dwClientCmdMax;
    // 检测间隔
    static DWORD m_dwClientCmdSec;
    // 创角最大活跃
    static DWORD m_dwActiveUserMax;
    // 创角最大在线
    static DWORD m_dwOnlineUserMax;
    // 活跃在线配置
    static std::map<DWORD, ActiveOnline> m_oActiveOnlineList;
    // 一次新开区数量
    static DWORD m_dwOnceOpenZoneNum;
    // 新开区保护时间
    static DWORD m_dwNewZoneProtectiveTime;
    // 地图人数限制
    static DWORD m_dwMaxScopeNum;
    static DWORD m_dwMaxFriendScopeNum;

    // 系统邮件在线发送间隔
    static DWORD m_dwMailSendMinTick;
    static DWORD m_dwMailSendMaxTick;
    static DWORD m_dwMailSendMaxCount;

    // 技能动作延迟
    static DWORD m_dwSkillDelayMs;
    // 技能前位置同步容错距离
    static float m_fSkillSyncDis;
    // 攻击目标时, 若目标在移动, 移动速度对技能释放距离容错误差影响
    static float m_fEnemySpedErrDis;
    // 技能队列延迟时间
    static DWORD m_dwSkillQueueDelayMs;
    // 技能位置错误队列保存长度
    static DWORD m_dwSkillDisQueueMax;
    // 技能队列超时统计时间(s)
    static DWORD m_dwSkillQueueCntTime;
    // 技能打断容错时间(ms)
    static DWORD m_dwSkillBreakErr;
    // 技能位移容错误差
    static float m_fSkillMoveDisErr;
    // 账号删除时间
    static std::map<DWORD, DWORD> m_mapDelCharTime;
    // 账号删除功能cd, 单位秒
    static DWORD m_dwDelCharCD;
    // 忽略计数的协议
    static std::map<DWORD, TSetDWORD> m_mapIgnoreCmd;

    // 消息发送频率控制
    // 所有类型消息最大数量
    static DWORD m_dwClientCmdLimitMax;
    // 检测间隔
    static DWORD m_dwClientCmdLimitSec;
    // 特定消息号对应最大数量
    static std::map<DWORD, DWORD> m_mapClientCmdLimit;

    // 社交加载帧处理时间
    static DWORD m_dwSocialLoadTime;
    // 公会帧处理超时时间
    static DWORD m_dwGuildFrameOvertime;
    // 公会优化开关
    static bool m_bGuildOptOpen;
    
    //安全密码
    static DWORD m_dwPwdFailTimes;
    static DWORD m_dwPwdIntervalTime;

    //斗技场溜溜猴开放
    bool m_bPvpLLH = true;
    //斗技场沙漠之狼
    bool m_bPvpSMZL = true;
    //斗技场华丽金属
    bool m_bPvpHLJS = true;

    // upyun auth value
    static string m_strUpyunAuthValue;
    // upyun form api
    static string m_strUpyunPwd;
    static bool upyun_form_str(std::string savekey, std::string &out_policy, std::string &out_signature, std::string type = "jpg");

    // tf分支gm指令白名单
    static std::set<QWORD> m_setTFGMWhiteList;

    // 送审服 platid
    static DWORD m_dwVerifyServerPlatID;

    // 聊天过滤
    static DWORD m_dwChatFilterListSize;
    static DWORD m_dwChatFilterTimeout;

    static string m_strLoginSign;
    static string m_strSafeDeviceSign;
    static string m_strAuthorizeSign;
    
    // 切线相关
    static bool m_bJumpZoneCheckLevel;

    static DWORD m_dwFluentLogCount;
    static DWORD m_dwFluentRetryCount;

    // 公会GM指令
    static DWORD m_dwGuildGMFlushTick;

    // lua报错推送间隔
    static DWORD m_dwLuaAlterCD;

    // 副本每秒中踢人上限
    static DWORD m_dwOneSecMaxKickUserNum;

    // buff 执行时长打印
    static DWORD m_dwBuffExecPrintTime;

    // 技能执行时长打印
    static DWORD m_dwSkillExecPrintTime;
    
    // 默认语言
    static ELanguageType m_eDefaultLanguage;
    
    // 属性计算开关
    static bool m_bLua;

    // gvg决战专用场景
    static string m_strSuperGvgSceneName;

    // gvg决战属性刷新时间间隔
    static DWORD m_dwSuperGvgRefreshInterval;

    static DWORD getDelCharTime(DWORD dwLv)
    {
      for (auto &m : m_mapDelCharTime)
      {
        if (dwLv <= m.first)
          return m.second;
      }
      return DWORD_MAX;
    }

    static bool isIngoreCmd(DWORD dwCmd, DWORD dwParam)
    {
      auto m = m_mapIgnoreCmd.find(dwCmd);
      if (m != m_mapIgnoreCmd.end())
        return m->second.find(dwParam) != m->second.end();
      return false;
    }

    static DWORD getCmdLimit(DWORD cmd, DWORD param)
    {
      auto limit = m_mapClientCmdLimit.find((cmd<<16)+param);
      if (limit != m_mapClientCmdLimit.end())
        return limit->second;
      return m_dwClientCmdLimitMax;
    }

    static bool isInTFGMWhiteList(QWORD accid)
    {
      return m_setTFGMWhiteList.find(accid) != m_setTFGMWhiteList.end();
    }

    // 备份数据开关
    static bool m_bOpenRollback;

    // 玩家分组
    static DWORD SCENE_USER_GROUP_NUM;

    // 包裹同步
    static bool m_bPackSyncNew;
};

#pragma once

#include "xNoncopyable.h"
#include "Var.h"
#include "GameStruct.h"
#include "xTime.h"
#include "GuildConfig.h"
#include "SocialCmd.pb.h"
#include "Dojo.pb.h"
#include "GuildBuilding.h"
#include "GuildWelfare.h"
#include "GuildChallenge.h"
#include "GuildQuest.h"
#include "GuildArtifact.h"
#include "GuildTreasure.h"
#include "GuildShop.h"
#include "GuildGvg.h"

const DWORD AUTH_VERSION = 7;

class Guild;
class GuildMisc : private xNoncopyable
{
  public:
    GuildMisc(Guild *pGuild);
    virtual ~GuildMisc();

  public:
    bool toMiscString(string& str);

  public:
    void init();
  private:
    DWORD m_dwInitStatus = GUILD_BLOB_INIT_NULL;

    // misc str
  public:
    void setBlobMiscString(const char* str, DWORD len);
  private:
    string m_oBlobMisc;

  private:
    bool fromMiscString(const string& str);

  public:
    const string& getJobName(DWORD e)
    {
      if (e <= EGUILDJOB_MIN || e >= EGUILDJOB_MAX) return STRING_EMPTY;
      return m_arrJob[e].name();
    }
    void setJobName(DWORD e, const string& name)
    {
      if (e <= EGUILDJOB_MIN || e >= EGUILDJOB_MAX) return;
      m_arrJob[e].set_name(name);
      updateJob(static_cast<EGuildJob>(e));
    }

    bool modifyAuth(bool bAdd, EModify eModify, EGuildJob eJob, EAuth eAuth);
    bool hasAuth(EGuildJob eJob, EAuth eAuth);
    bool hasEditAuth(EGuildJob eJob, EAuth eAuth);

    void setAuth(EGuildJob eJob, DWORD dwAuth) { m_arrJob[eJob].set_auth(dwAuth); }
    DWORD getAuth(EGuildJob eJob) const { return m_arrJob[eJob].auth(); }

    void setEditAuth(EGuildJob eJob, DWORD dwAuth) { m_arrJob[eJob].set_editauth(dwAuth); }

    void updateJob(EGuildJob eJob);
    const GuildJob& getJob(EGuildJob eJob) const;

    DWORD getRenameTime() { return m_dwRenameTime; }
    void setRenameTime(DWORD time) { m_dwRenameTime = time; }
  private:
    void auth_version();
  private:
    GuildJob m_arrJob[EGUILDJOB_MAX];
    DWORD m_dwAuthVersion = 0;

  public:
    //Variable& getVar() { return m_oVar; }
    DWORD getVarValue(EVarType eType);
    void setVarValue(EVarType eType, DWORD dwValue);
  private:
    DWORD getVarTemp(EVarType eType);
    void setVarTemp(EVarType eType, DWORD dwValue);
  private:
    Variable m_oVar;
    DWORD m_arrVarValue[EVARTYPE_MAX];
    map<EVarType, EGuildData> m_mapVarGuild;

  public:
    DWORD getDonateTime1() { refreshDonateTime(xTime::getCurSec()); return m_dwDonateTime1; }
    DWORD getDonateTime2() { refreshDonateTime(xTime::getCurSec()); return m_dwDonateTime2; }
    DWORD getDonateTime3() { refreshDonateTime(xTime::getCurSec()); return m_dwDonateTime3; }
    DWORD getDonateTime4() { refreshDonateTime(xTime::getCurSec()); return m_dwDonateTime4; }

    void refreshDonateTime(DWORD curSec);
  private:
    DWORD m_dwDonateTime1 = 0;
    DWORD m_dwDonateTime2 = 0;
    DWORD m_dwDonateTime3 = 0;
    DWORD m_dwDonateTime4 = 0;

  public:
    const TListGuildQuestCFG& getQuestList() const { return m_listGuildQuest; }
    void refreshQuest(DWORD curSec);
  private:
    TListGuildQuestCFG m_listGuildQuest;
    DWORD m_dwNextQuestTime = 0;
    DWORD m_dwRenameTime = 0;

    DWORD m_dwQuestTick = 0;

  public:
    /*获取公共信息 目前只有留言*/
    void getPublicInfo(const SocialUser& rUser, DojoPublicInfoCmd& rev);
    /*增加留言*/
    void addMsg(const SocialUser& rUser, DojoAddMsg& rev);
  private:
    std::map<DWORD/*dojo id*/, std::list<Cmd::DojoMsg>> m_mapDojoMsg;
    // 公会城池
  public:
    DWORD getCityID() const { return m_dwCityID; }
    void setCityID(DWORD dwCityID);

    DWORD getCityGiveupTime() const { return m_dwGiveupTime; }
    void setCityGiveupTime(DWORD dwTime);

    void updateGiveupCity(DWORD curTime);
  private:
    DWORD m_dwGiveupTime = 0;
    DWORD m_dwCityID = 0;
  private:
    Guild *m_pGuild = nullptr;
    // 公会建筑
  public:
    GuildBuildingMgr& getBuilding() { return m_oBuilding; }
  private:
    GuildBuildingMgr m_oBuilding;
    // 公会功能
  public:
    void openFunction(EGuildFunction eFunction);
    bool isFunctionOpen(EGuildFunction eFunction) { return (m_qwOpenFunction & (QWORD(1) << (DWORD(eFunction) - 1))) != 0; }
    QWORD getOpenFunction() { return m_qwOpenFunction; }
  private:
    QWORD m_qwOpenFunction = 0;
  public:
    GuildWelfareMgr& getWelfare() { return m_oWelfare; }
  private:
    GuildWelfareMgr m_oWelfare;
  public:
    GuildChallengeMgr& getChallenge() { return m_oChallenge; }
  private:
    GuildChallengeMgr m_oChallenge;
    // 公会任务
  public:
    GuildQuestMgr& getQuest() { return m_oQuest; }
  private:
    GuildQuestMgr m_oQuest;
  public:
    GuildArtifactMgr& getArtifact() { return m_oArtifact; }
  private:
    GuildArtifactMgr m_oArtifact;
  // gvg
  public:
    GuildGvg& getGvg() { return m_oGvg; }
  private:
    GuildGvg m_oGvg;
    // 公会宝箱
  public:
    GTreasure& getTreasure() { return m_oTreasure; }
  private:
    GTreasure m_oTreasure;
  public:
    GuildShopMgr& getShop() { return m_oShop; }
  private:
    GuildShopMgr m_oShop;
};


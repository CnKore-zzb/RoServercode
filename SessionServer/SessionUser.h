#pragma once

#include <bitset>
#include "xEntry.h"
#include "UserData.h"
#include "SessionCmd.pb.h"
#include "ProtoCommon.pb.h"
#include "CarrierCmd.pb.h"
#include "SocialCmd.pb.h"
#include "SessionSociality.pb.h"
#include "Mail.h"
#include "TradeLog.h"
#include "SessionServer.h"
#include "GCharManager.h"
#include "OperateRewardConfig.h"
#include "TeamCmd.pb.h"
#include "GTeam.h"
#include "Authorize.h"
#include "GGuild.h"
#include "Auction.h"
#include "GSocial.h"
#include "ChatManager_SE.h"
#include "WeddingSCmd.pb.h"
#include "MatchSCmd.pb.h"

using namespace Cmd;
using std::bitset;

class SessionScene;
class ServerTask;

enum class GateServerUserState
{
  create,
  req_login,
  run,
};

enum class SceneServerUserState
{
  create,
  scene_load_data,
  run,
};

// 各个进程的状态
struct ServerStateController
{
  public:
    GateServerUserState m_oGateServerState = GateServerUserState::create;
    SceneServerUserState m_oSceneServerState = SceneServerUserState::create;
};

class RewardManager
{
  private:
    RewardManager() {}
    ~RewardManager() {}

  public:
    static bool roll(DWORD id, SessionUser* pUser, TVecItemInfo& vecItemInfo, ESource source, float fRatio = 1.0f, DWORD dwMapID = 0);
};

struct SSessionUserData
{
  // 组队排位赛玩家积分
  DWORD dwTeamPwsScore = 0;

  // 玩家个人数据副本(场景服更新)
  std::map<EVarType, DWORD> m_mapVarValues;
};

// session user
class SessionUser : public xEntry, public xObjectPool<SessionUser>
{
  public:
    SessionUser(QWORD uid, const char *uName, QWORD accid, ServerTask* net, DWORD platformId, const string& deviceid);
    virtual ~SessionUser();
    /*************************************************************//**
     *                        登录
     ****************************************************************/
  public:
    void userOnline();
    void userOffline();
    USER_STATE getUserState() const { return user_state; }
    void setUserState(USER_STATE state) { user_state=state; }
    DWORD getIP() const { return ip; }
    void setIP(DWORD _ip) { ip=_ip; }
    inline DWORD getZoneID() const { return thisServer->getZoneID(); }
    void setPhone(const string& phone) { m_strPhone = phone; }
    void setSafeDevice(bool safeDevice);
    void setMaxBaseLv(DWORD maxbaselv) { m_dwMaxBaseLv = maxbaselv; }
    //void initDataFromScene(const SessionUserData &data);
    void timer(DWORD curTime);
  public:
    QWORD accid;
    USER_STATE user_state;
    DWORD ip;
    DWORD m_platformId;
    string m_strDeviceID;
    string m_strPhone;
    bool m_bSafeDevice = false;

  public:
    void setGateServerUserState(GateServerUserState s) { m_oServerStateController.m_oGateServerState = s; }
    void setSceneServerUserState(SceneServerUserState s) { m_oServerStateController.m_oSceneServerState = s; }
  private:
    ServerStateController m_oServerStateController;
    /*************************************************************//**
     *                          网关
     ****************************************************************/
  public:
    void set_gate_task(ServerTask *gate_task) { gate_task_ = gate_task; }
    ServerTask* gate_task() { return gate_task_; }
    bool sendCmdToMe(const void* cmd,WORD len);
    const char* getGateServerName()
    {
      if (gate_task_) return gate_task_->getName();
      return "";
    }

    bool doSessionUserCmd(const BYTE* buf, WORD len);
  private:
    bool doSessionUserGuildCmd(const BYTE* buf, WORD len);
    bool doSessionUserTeamCmd(const BYTE* buf, WORD len);
    bool doSessionUserShopCmd(const BYTE* buf, WORD len);
    bool doSessionUserTowerCmd(const BYTE* buf, WORD len);
    bool doSessionUserMailCmd(const BYTE* buf, WORD len);
    bool doSessionUserSocialCmd(const BYTE* buf, WORD len);
    bool doSessionUserDojoCmd(const BYTE* buf, WORD len);
    bool doSessionUserChatCmd(const BYTE* buf, WORD len);
    bool doCarrierCmd(const BYTE* buf, WORD len);
    bool doSocialCmd(const BYTE* buf, WORD len);
    bool doTradeCmd(const BYTE* buf, WORD len);
    bool doMatchCmd(const BYTE* buf, WORD len);
    bool doAuctionCmd(const BYTE* buf, WORD len);
    bool sendUserCmdToTradeServer(const BYTE* buf, WORD len);
    bool sendUserCmdToAuctionServer(const BYTE* buf, WORD len);
    bool sendUserCmdToWeddingServer(const BYTE* buf, WORD len);
    bool sendUserCmdToRecordServer(const BYTE* buf, WORD len);
    bool sendUserCmdToMatchServer(const BYTE* buf, WORD len);
    bool doSessionAuthorizeCmd(const BYTE* buf, WORD len);
    bool doUserEventCmd(const BYTE* buf, WORD len);
    bool doUserWeddingCmd(const BYTE* buf, WORD len);    
    bool doUserItemCmd(const BYTE* buf, WORD len);    
    bool doPveCardCmd(const BYTE* buf, WORD len);
    bool doTeamRaidCmd(const BYTE* buf, WORD len);
  public:
    bool sendUserCmdToSocialServer(const BYTE* buf, WORD len, ECmdType eType);
    bool sendUserCmdToTeamServer(const BYTE* buf, WORD len, ECmdType eType);
    bool sendUserCmdToGuildServer(const BYTE* buf, WORD len, ECmdType eType);
  public:
    void addOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i);
    void delOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i);
  private:
    ServerTask *gate_task_;
    /*************************************************************//**
     *                          场景 
     ****************************************************************/
  public:
    void setScene(SessionScene* s);
    SessionScene* getScene() const { return myScene; }
    bool sendCmdToScene(const void* cmd,WORD len);
    void changeScene(Cmd::ChangeSceneSessionCmd &cmd);
    QWORD getSceneID() const { return m_qwSceneID; }
    void onEnterScene();
    // 发送命令到SceneUser
  public:
    bool sendCmdToSceneUser(const void *cmd, WORD len);
  private:
    SessionScene* myScene;
    QWORD m_qwSceneID = 0;
    // 数据
  public:
    void toData(UserInfo* pInfo);
    void toData(SocialUser* pUser);

    GCharReader* getGCharData() { return &m_oGCharData; }

    DWORD getBaseLevel() const { return m_oGCharData.getBaseLevel(); }
    DWORD getJobLevel() const { return m_oGCharData.getJobLevel(); }
    DWORD getMapID() const { return m_oGCharData.getMapID(); }
    DWORD getDMapID() const { return m_dwRaidID; }
    DWORD getCreateTime() const { return m_dwCreateTime; }
    QWORD getFollowerID() const { return m_qwFollowID; }
    QWORD getHandID() const { return m_qwHandID; }
    EProfession getProfession() const { return m_oGCharData.getProfession(); }
    EGender getGender() const { return m_oGCharData.getGender(); }
    EQueryType getQueryType() const { return static_cast<EQueryType>(m_oGCharData.getQueryType()); }
    bool hasMonthCard() const { return m_bHasMonthCard; }
    DWORD getMaxBaseLv() const { return m_dwMaxBaseLv; }

    void setAttr(EAttrType eType, float value);
    float getAttr(EAttrType eType) const;
  private:
    GCharReader m_oGCharData;
    DWORD m_dwRaidID = 0;
    QWORD m_qwFollowID = 0;
    QWORD m_qwHandID = 0;
    QWORD m_qwZenyDebt = 0;
    DWORD m_dwCreateTime = 0;
    TVecAttrSvrs m_vecAttrs;
    bool m_bHasMonthCard = false;
    DWORD m_dwMaxBaseLv = 0;
  public:
    DWORD m_dwReloginTime = 0;
    // 数据同步
  public:
    void onDataChanged(const UserDataSync& sync);
    static void fetchChangedData(const UserDataSync& sync, UserInfoSyncSocialCmd& social, UserInfoSyncSocialCmd& team, UserInfoSyncSocialCmd& guild);
    // 邮件
  public:
    Mail& getMail() { return m_oMail; }
  private:
    Mail m_oMail;
    // 交易所记录
  public:
    TradeLog& getTradeLog() { return m_oTradeLog; }
  private:
    TradeLog m_oTradeLog;

    // 载具
  public:
    void setCarrierID(DWORD dwID) { m_dwCarrierID = dwID; }
    DWORD getCarrierID() const { return m_dwCarrierID; }
  private:
    DWORD m_dwCarrierID = 0;
    // 公会
  public:
    GGuild& getGuild() { return m_oGuild; }
    void updateUserInfo(const GuildUserInfoSyncGuildCmd& cmd);
  private:
    GGuild m_oGuild;
    bool m_bPrayFirst = true;
    // 社交
  public:
    GSocial& getSocial() { return m_oSocial; }
  private:
    GSocial m_oSocial;
    // 队伍
  public:
    GTeam& getTeam() { return m_oGTeam; }
    QWORD getTeamID() const { return m_oGTeam.getTeamID(); }

    void onEnterTeam(QWORD teamid);
    void onLeaveTeam(QWORD teamid);

    bool getTeamOnline() const { return m_bTeamOnline; }
    void setTeamOnline(bool b) { m_bTeamOnline = b; }
  private:
    bool m_bTeamOnline = true;
    GTeam m_oGTeam;
  public:
    EOperateState queryOperateState(EOperateType type,SOperateRewardCFG** pCfg);
    void queryOperateState(OperateQuerySocialCmd& rev);
    void takeOperateReward(OperateTakeSocialCmd& rev);
    //bool useGiftCode(std::string code);
  //安全密码
  private:
    Authorize m_oAuthorize;
  public:
    Authorize& getAuthorize() { return m_oAuthorize; }
  public:
    void sendChargeCnt();
    bool setDepositCount(DWORD depositid, DWORD count);
    bool resetDepositVirgin(DWORD depositid, DWORD tag);
    bool setDepositVirgin(DWORD depositid, DWORD tag);
    bool setDepositLimit(DWORD depositid, DWORD count);
    DWORD getDepositLimit(DWORD depositid);
    void updateDepositLimit();
    //DWORD getChargeCnt(DWORD id, DWORD key = 0);
    //void addChargeCnt(DWORD id, DWORD key = 0);
  private:
    std::map<DWORD/*dataid*/,DWORD/*cnt*/ > m_mapChargeCnt;
    //std::map<DWORD/*dataid*/, std::pair<DWORD/*key*/, DWORD/*cnt*/ >> m_mapChargeCnt;

    DWORD m_dwChargeYM = 0; // year * 100 + month;
    std::map<DWORD, DWORD> m_mapDepositLimit;
    DWORD m_dwDepositLimitYM = 0; // year * 100 + month
  public:
    Auction& getAuction() { return m_oAuction; }
  private:
    Auction m_oAuction;
  public:
    bool isInPollyScene();
  public:
    void processLoveLetter(const LoveLetterUse& cmd);
    void sendActivityCnt();
    DWORD getActivityCnt(DWORD dwActivityId);
    void addActivityCnt(DWORD dwActivityId);
    void sendVersionVard2Client();
  private:
    std::map<DWORD/*activity id*/, DWORD/*count*/> m_mapActivityCnt;
  public:
    void useItemCode(UseItemCodeSessionCmd& cmd);
    void getUsedItemCode(ReqUsedItemCodeSessionCmd& cmd);
    void setWeddingInfo(const Cmd::WeddingInfo& info);
    bool sendWeddingInfo2Scene();
    //获取订婚对象的charid
    QWORD getReserveParnter();
    //获取结婚对象的charid
    QWORD getWeddingParnter();
    void updateParnterName(const string& name);
  private:
    Cmd::WeddingInfo m_oWeddingInfo;
  private:
    DWORD m_dwOperateReward = 0;
  public:
    void setOperateReward(DWORD value) { m_dwOperateReward = value; }
    bool checkOperateReward(EOperateType type);
  public:
    void setLanguage(ELanguageType language) { m_eLanguage = language; }
    ELanguageType getLanguage() { return m_eLanguage; }
  private:
    ELanguageType m_eLanguage = ELANGUAGE_CHINESE_SIMPLIFIED;

  public:
    void updateVar(const SyncUserVarSessionCmd& cmd);
    DWORD getVarValue(EVarType eType) const;
    void setMatchScore(const SyncUserScoreMatchSCmd& cmd);
    DWORD getMatchScore(EPvpType eType) const;
  private:
    SSessionUserData m_stSessionUserData;
};


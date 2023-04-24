#pragma once

#include "SceneServer.h"
#include "xSceneEntryDynamic.h"
#include "Package.h"
#include "UserSceneData.h"
#include "Quest.h"
#include "Pet.h"
#include "Stage.h"
#include "Var.h"
#include "UserQuestNpc.h"
#include "Portrait.h"
#include "Hair.h"
#include "Carrier.h"
#include "UserEvent.h"
#include "UserTeamRaid.h"
#include "UserGear.h"
#include "SceneFighter.h"
#include "LuaManager.h"
#include "SceneTip.h"
#include "UserMessage.h"
#include "ChatRoom.h"
#include "Shortcut.h"
#include "SceneTower.h"
#include "Freyja.h"
#include "Seal.h"
#include "Interlocution.h"
#include "Manual.h"
#include "UserLaboratory.h"
#include "UserScenery.h"
#include "UserHands.h"
#include "Title.h"
#include "Transform.h"
#include "SceneDojo.h"
#include "ConfigManager.h"
#include "UserHandNpc.h"
#include "UserItemMusic.h"
#include "UserCamera.h"
#include "SocialCmd.pb.h"
#include "SessionSociality.pb.h"
#include "UserZone.h"
#include "TeamSealManager.h"
#include "SessionCmd.pb.h"
#include "Deposit.h"
#include "RewardConfig.h"
#include "UserChat.h"
#include "WeaponPet.h"
#include "UserStat.h"
#include "GCharManager.h"
#include "UserGingerBread.h"
#include "UserTicket.h"
#include "GTeam.h"
#include "GGuild.h"
#include "Share.h"
#include "Achieve.h"
#include "Authorize.pb.h"
#include "Astrolabe.h"
#include "UserPhoto.h"
#include "UserMap.h"
#include "UserPet.h"
#include "PetAdventure.h"
#include "SceneFood.h"
#include "Lottery.h"
#include "GSocial.h"
#include "TutorTask.h"
#include "Action.h"
#include "Eye.h"
#include "ActivityCmd.pb.h"
#include "UserBeing.h"
#include "HighRefine.h"
#include "GuildChallenge.h"
#include "UserGvg.h"
#include "UserProposal.h"
#include "UserWedding.h"
#include "Servant.h"
#include "Profession.h"
#include "Booth.h"
#include "DressUp.h"
#include "Transfer.h"
#include "UserElementElf.h"
#include "UserMatchData.h"
#include "CheatTag.h"

using std::map;
using namespace Cmd;
using std::string;

class ServerTask;
class SceneUserData;
class PetWork;
class SceneShop;
class Menu;
class UserRecords;
class ExchangeShop;
enum class GoMapType;

#define MAX_RANDOM_FOR_SKILL 20
const DWORD DATA_FLUSH_INTERVAL = 300;

// tmp data
struct TmpData
{
  DWORD m_dwHead = 0;
  DWORD m_dwBack = 0;
  DWORD m_dwMount = 0;
  EProfession m_eProfession = EPROFESSION_MIN;
  DWORD m_dwBody = 0;
};
struct SChatParams
{
  std::list<string> m_oListChatMsgs;
  DWORD m_dwInvalidCount = 0;
  DWORD m_dwInvalidTime = 0;
  DWORD m_dwMaxSize = 5;

  void putMsg(const char* msg) {
    m_oListChatMsgs.push_back(msg);
    if (m_oListChatMsgs.size() > m_dwMaxSize)
      m_oListChatMsgs.pop_front();
  }
  void popMsg() { m_oListChatMsgs.pop_front(); }

  bool find(const char* msg)
  {
    for (auto &s : m_oListChatMsgs)
    {
      if (s.find(msg) != std::string::npos)
        return true;
    }
    return false;
  }

  void resetSize(DWORD size) { m_dwMaxSize = size; }

  void setInvalidTime(DWORD time) { m_dwInvalidTime = time; }
  void setInvalidCount(DWORD count) { m_dwInvalidCount = count; }
  void addInvalidCount() { m_dwInvalidCount ++; }

  DWORD getInvalidCount() { return m_dwInvalidCount; }
  DWORD getInvalidTime() { return m_dwInvalidTime; }
};

// SceneUser
class SceneUser : public xSceneEntryDynamic
{
  friend class xSceneEntryIndex;
  public:
    SceneUser(QWORD qwID, const char* name, QWORD uAccID, ServerTask* net);
    virtual ~SceneUser();

    virtual SCENE_ENTRY_TYPE getEntryType() const { return SCENE_ENTRY_USER; }

    void toData(SocialUser* pUser);

    // 基本数据
  public:
    inline DWORD getZoneID() const { return thisServer->getZoneID(); }
    BYTE getPrivilege() const;
    virtual QWORD getTempID() const { return id; }
    string getCharIDString();
    string getAccIDString();

    DWORD m_dwZoneID = 0;
    QWORD accid = 0;

    // 基本状态
  public:
    USER_STATE getUserState() const { return user_state; }
    void setUserState(USER_STATE state) {user_state = state;}

    bool m_bTowerHint = false;
  private:
    USER_STATE user_state = USER_STATE_NONE;

    // 定时器
  public:
    virtual void onOneSecTimeUp(QWORD curMSec);
    virtual void onFiveSecTimeUp(QWORD curMSec);
    virtual void onTenSecTimeUp(QWORD curMSec);
    virtual void onOneMinTimeUp(QWORD curMSec);
    virtual void onTenMinTimeUp(QWORD curMSec);
    virtual void onDailyRefresh(QWORD curMSec);
    virtual void refreshMe(QWORD curMSec);

    // 消息收发
  public:
    bool doUserCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUser2Cmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserItemCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserSkillCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserQuestCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserMapCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserPetCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserFuBenCmd(const Cmd::UserCmd* cmd, WORD len);
    bool doSceneUserCarrierCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserTipCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserChatRoomCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserTowerCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserInterCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserManualCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserSealCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSessionUserShopCmd(const Cmd::UserCmd* buf, WORD len);
    bool doUserEventCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserTrade(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserDojoCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSocialCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserAuguryCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSessionUserTeamCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserAchieveCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserAstrolabeCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserPhotoCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserFoodCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserTutorCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserBeingCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSessionAuthorizeCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSessionUserGuildCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneWeddingCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserPveCardCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserTeamRaidCmd(const Cmd::UserCmd* buf, WORD len);
    bool doSceneUserBossCmd(const Cmd::UserCmd* buf, WORD len);
  public:
    const char* getGateServerName()
    {
      if (gatetask) return gatetask->getName();
      return "";
    }
    ServerTask* getGateTask()
    {
      return gatetask;
    }
  private:
    ServerTask* gatetask = nullptr;

    // 网关索引
  public:
    void addOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i, DWORD param=0);
    void delOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i);
    void addTwoLevelIndex(TWO_LEVEL_INDEX_TYPE indexT, DWORD i, DWORD i2);
    void delTwoLevelIndex(TWO_LEVEL_INDEX_TYPE indexT, DWORD i, DWORD i2);

     // 登录
  public:
    bool initChar(SceneUserData *pData);
    bool initNewChar();
    void enterServer();
    void sendSessionUserData();
    void sendGateUserData();
    void sendMainUserData();
    bool getInitChar() const { return initCharSuccess; }

    void appendData(const Cmd::UserDataRecordCmd& rCmd);
    const string& getData() const { return m_userdata; }

    string& getLogSign() { return m_logSign; }
  private:
    bool initCharSuccess = false;
    std::string m_logSign;   //登录登出 日志签名
    std::string m_userdata;

    // 退出
  public:
    void unReg(UnregType type);
    void unRegBeforeSave();
    void unRegAfterSave();
    void onUserOffline();

    // 同步和存储
  private:
    void fetchChangeData(UserSyncCmd& sync, UserNineSyncCmd& nine, UserDataSync& session);
  public:
    void refreshDataToRecord(UnregType eType);

    void saveDataNow();

  public:
    void updateData(UnregType eType, EUserSyncType eSync = EUSERSYNCTYPE_SYNC);
    // 场景
  public:
    virtual bool enterScene(Scene* scene);
    virtual void leaveScene();
    virtual void sendCmdToMe(const void* cmd, DWORD len);
    virtual void sendMeToNine();
    virtual void sendCmdToScope(const void* cmd, DWORD len);

    void sendNineNpcToMe();
    void sendNineToMe();
    void delMeToNine();
    void delNineToMe();
    void delMeToUser(SceneUser* user);
    void sendMeToUser(SceneUser* user);
    void fillMapUserData(MapUser* data, bool bSendSelf = false);
    void fillMapBoothData(MapUser* data);
    void notifyChangeScene();

    void gomap(DWORD mapid, GoMapType type, const xPos& pos = xPos(0, 0, 0));
    void goTo(xPos pos, bool isGoMap=false, bool noCheckScene=false);
  private:
    void gomap(Scene *s, const xPos& newpos);
  public:
    void setVisitNpc(QWORD qwNpcID) { m_qwVisitNpc = qwNpcID; }
    QWORD getVisitNpc() const { return m_qwVisitNpc; }
    SceneNpc* getVisitNpcObj();
  private:
    QWORD m_qwVisitNpc = 0;
    bool m_blInScene = false;
    bool m_bMarkInScene = false; // 锁变量, 仅用于enterScene/leaveScene , 避免enterScene时调用leaveScene
  public:
    void setVisitUser(QWORD qwUserID) { m_qwVisitUser = qwUserID; }
    QWORD getVisitUser() const { return m_qwVisitUser; }
  private:
    QWORD m_qwVisitUser = 0;
    // 攻击相关
  protected:
    virtual bool beAttack(QWORD damage, xSceneEntryDynamic* attacker);
    // 事件
  public:
    UserEvent& getEvent() { return m_event; }
  private:
    //此接口可能经常变动，因此用pimpl形式
    UserEvent m_event;

  protected:
    //virtual const BaseSkill* getFixedSkill(const UINT skillId) const;
    virtual bool canUseSkill(const BaseSkill* skill);

    // 属性
  public:
    virtual bool initAttr();
    virtual SQWORD changeHp(SQWORD hp, xSceneEntryDynamic* entry, bool bshare = false, bool bforce = false);

    DWORD getSp() const { return getAttr(EATTRTYPE_SP); }
    void setSp(DWORD sp);

    virtual void setStatus(ECreatureStatus eStatus);
    virtual bool isAlive() { return getStatus() != ECREATURESTATUS_DEAD; }
  private:
    void onHealMe(xSceneEntryDynamic* entry, DWORD healhp);/*仅处理逻辑, 不改变实际血量*/
  public:
    void onSkillHealMe(xSceneEntryDynamic* entry, DWORD healhp);/*仅处理逻辑, 不改变实际血量*/
    // 包裹
  public:
    Package& getPackage() { return m_oPackage; }
    bool useItem(const Cmd::ItemUse& cmd);
    bool canUseItem(DWORD itemid, QWORD targetid, bool isSkillCost = false);
  private:
    Package m_oPackage;
    // 数据
  public:
    UserSceneData& getUserSceneData() { return m_oUserSceneData; }

    bool addAttrPoint(EUserDataType eType, DWORD point);
    bool resetAttrPoint();

    // level
    void setLevel(DWORD lv) { m_oUserSceneData.setRolelv(lv); }
    virtual DWORD getLevel() const { return m_oUserSceneData.getRolelv(); }
    DWORD getMaxBaseLv() { return m_oUserSceneData.getMaxBaseLv(); } // 账号下所有未删除角色的最大等级

    // profession exchange
    bool exchangeProfession(EProfession profession);
    EProfession getProfession() { return m_oUserSceneData.getProfession(); }

    // patch
    bool hasPatchLoad(DWORD dwVersion) { return m_oUserSceneData.hasPatchVersion(dwVersion); }
    bool addPatchLoad(DWORD dwVersion) { return m_oUserSceneData.addPatchVersion(dwVersion); }

    // acc patch
    bool hasAccPatchLoad(QWORD qwVersion) { return m_oUserSceneData.hasAccPatchVersion(qwVersion); }
    bool addAccPatchLoad(QWORD qwVersion) { return m_oUserSceneData.addAccPatchVersion(qwVersion); }

    bool battlePointNeedCalc() { return m_bitset.test(EUSERDATATYPE_BATTLEPOINT) == true; }

    // profession
    QWORD getMainCharId() const { return m_oUserSceneData.getMainCharId(); }
    void setMainCharId(QWORD qwCharId) { m_oUserSceneData.setMainCharId(qwCharId); }

    ////换装
    //void setClothColor(DWORD value) { m_oUserSceneData.setClothColor(value); }

    // change name
    bool changeName(const string& newName);
    void rename(const string& newName);

    EError levelupDead();
    DWORD getTempDeadGet() { DWORD dwValue = m_dwTempDeadGet; m_dwTempDeadGet = 0; return dwValue; }
  public:
    void resetRecordSave() { m_dwRecordSave = 0; }
  private:
    UserSceneData m_oUserSceneData;

    DWORD m_dwRecordSave = 0;
    DWORD m_dwTempDeadGet = 0;
    // 升级
  public:
    void addBaseExp(QWORD exp, ESource eSource, int* pIsFull = nullptr);
    void addJobExp(QWORD exp, ESource eSource, int* pIsFull = nullptr);
    void dropBaseExp(ESource eSource);
  private:
    void baseLevelup(DWORD oldLv, DWORD newLv);
    void jobLevelup(DWORD oldLv, DWORD newLv);
    // 货币
  public:
    bool checkMoney(EMoneyType eType, QWORD value);
    bool addMoney(EMoneyType eType, QWORD value, ESource eSource, bool itemShow = false);
    bool subMoney(EMoneyType eType, QWORD value, ESource eSource);
    //chargeMoney 充值金额 单位分
    bool addCharge(DWORD chargeMoney);
    // 净化值
  public:
    bool checkPurify(DWORD value) const { return m_oUserSceneData.getPurify() >= value; }
    bool addPurify(DWORD value, bool upperLimit = true);
    bool subPurify(DWORD value);
    DWORD getPurify() const { return m_oUserSceneData.getPurify(); }
    // 改名
    void setRenameTime(DWORD time) { m_oUserSceneData.setRenameTime(time); }
    bool checkRenameTime() { return m_oUserSceneData.getRenameTime() + MiscConfig::getMe().getPlayerRenameCFG().dwRenameCoolDown < xTime::getCurSec(); }

    // 任务
  public:
    Quest& getQuest() { return m_oQuest; }
    UserQuestNpc& getQuestNpc() { return m_oQuestNpc; }
  private:
    Quest m_oQuest;
    UserQuestNpc m_oQuestNpc;
    // 宠物
  public:
    Pet& getPet() { return m_oPet; }
  private:
    Pet m_oPet;
  public:
    UserStage m_stage;
    // var
  public:
    Variable& getVar() { return m_oVar; }
    void updateVar();
  private:
    Variable m_oVar;
    // 合成
  public:
    bool checkCompose(DWORD id, EPackMethod eMethod, GlobalActivityType eEvent, ECheckMethod eCheckMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    string doCompose(DWORD id, EPackMethod eMethod, GlobalActivityType eEvent,
        bool bShow = true, bool bInform = true, bool bForceShow = true, ECheckMethod eCheckMethod = ECHECKMETHOD_NONORMALEQUIP, EPackFunc eFunc = EPACKFUNC_MIN);
    // 功能开启
  public:
    Menu& getMenu();
  private:
    Menu* m_pMenu = nullptr;
    // 队伍
  public:
    QWORD getTeamID() const { return m_oGTeam.getTeamID(); }
    QWORD getTeamLeaderID() const { return m_oGTeam.getLeaderID(); }
    GTeam& getTeam() { return m_oGTeam; }

    virtual bool isMyTeamMember(QWORD id);
    void startCalcTeamLen();
    void stopCalcTeamLen();

    void syncMyPosToTeam(bool bInit = false);
    void syncMemberPosToMe();
    void syncPosAtonce() { m_bSyncPosAtonce = true; }

    void updateTeamTower();

    DWORD getTeamMemberCount(DWORD dwMapID, DWORD dwRaidID);
    const TeamMemberInfo* getTeamMember(QWORD qwCharID) const { return m_oGTeam.getTeamMember(qwCharID); }
    //获取同一个场景里队友，包括了自己
    std::set<SceneUser*> getTeamSceneUser();
    std::set<SceneUser*> getTeamRewardUsers();
    const string& getTeamerName(QWORD id);
    void onTeamChange(const GTeam& oldTeam, const GTeam& nowTeam, bool bOnline = false);

    void updateDataToTeam();
    void updateDataToUser(QWORD qwTargetID);
    void setMark(EMemberData eData) { m_TeamBitset.set(eData); }
  private:
    void fetchTeamData(MemberDataUpdate& cmd);
  private:
    GTeam m_oGTeam;
  private:
    DWORD m_teamTimePoint = 0;    //组队时间点。
    DWORD getTeamTimeLen();
    bool m_bSyncPosAtonce = false;
    bitset<EMEMBERDATA_MAX> m_TeamBitset;
    // 头像
  public:
    Portrait& getPortrait() { return m_oPortrait; }
  private:
    Portrait m_oPortrait;
    // 载具
  public:
    Carrier m_oCarrier;
    // 组队副本
  public:
    UserTeamRaid m_oTeamRaid;
  public:
    void gotoUserMap(QWORD charid);
    //void purifyRaidBoss(QWORD charid);
    // 个人副本
  public:
    void setFubenBeAttackCount(DWORD dwCount) { m_dwFubenBeAttackCount = dwCount; }
    DWORD getFubenBeAttackCount() const { return m_dwFubenBeAttackCount; }
  private:
    DWORD m_dwFubenBeAttackCount = 0;
    // 商城
  public:
    SceneShop& getSceneShop();
  private:
    SceneShop* m_pSceneShop = nullptr;
    //头发
  public:
    Hair& getHairInfo() { return m_oHair; }
  private:
    Hair m_oHair;
  public:
    UserGear m_oGear;
    // 角色
  public:
    SceneFighter* getCurFighter() const { return m_pCurFighter; }
    SceneFighter* getFighter() const { return m_pCurFighter; }
    SceneFighter* getFighter(EProfession eProfession) const;
    SceneFighter* getFighter(DWORD dwType, DWORD dwBranch) const;

    const SRoleBaseCFG* getRoleBaseCFG() const { return m_pCurFighter == nullptr ? nullptr : m_pCurFighter->getRoleCFG(); }
    DWORD getFighterTypeCount(EProfession eProfession) const;

    DWORD getJobLv() const;

    bool addSkill(DWORD id, DWORD sourceid, ESource eSource = ESOURCE_EQUIP);
    bool removeSkill(DWORD id, DWORD sourceid, ESource eSource = ESOURCE_EQUIP);
    void showSkill();

    bool load(const BlobData& oBlob);
    bool save(BlobData& oBlob);

    void sendFighterInfo();

    void addMaxSkillPos();
    void addAutoSkillPos();
    void validMonthCardSkillPos();
    void addExtendSkillPos();
    void decSkillPos();
    void decAutoSkillPos();
    void decExtendSkillPos();
    void invalidMonthCardSkillPos();

    void reloadConfig(ConfigType type);
  private:
    void clearFighter();
  private:
    TVecSceneFighter m_vecFighters;
    SceneFighter* m_pCurFighter = nullptr;
    DWORD m_dwTotalAttrPoint = 0;
    DWORD m_dwMaxSkillPos = 0;
    DWORD m_dwAutoSkillPos = 0;
    DWORD m_dwExtendSkillPos = 0;
    DWORD m_dwMaxJobLv = 0;  //JOb等级上限
    DWORD m_dwMaxCurJobLv = 0; //当前所有职业的最大Job等级
  public:
    DWORD getTotalPoint(){ return m_dwTotalAttrPoint; }
    DWORD getMaxSkillPos(){ return m_dwMaxSkillPos; }
    DWORD getAutoSkillPos(){ return m_dwAutoSkillPos; }
    DWORD getExtendSkillPos(){ return m_dwExtendSkillPos; }
    DWORD getMaxJobLv(){ return m_dwMaxJobLv; }
    void setMaxJobLv(DWORD dwMaxJobLv){ if(m_dwMaxJobLv < dwMaxJobLv) m_dwMaxJobLv = dwMaxJobLv; }
    DWORD getMaxCurJobLv(){ return m_dwMaxCurJobLv; }
    void setMaxCurJobLv(DWORD dwJobLv){ if(m_dwMaxCurJobLv < dwJobLv) m_dwMaxCurJobLv = dwJobLv; }

    // 红点提醒
  public:
    SceneGameTip& getTip(){ return m_oTip;}
  private:
    SceneGameTip m_oTip;
    // 预设聊天消息
  public:
    UserMessage& getMsg(){return m_oMsg;}
  private:
    UserMessage m_oMsg;
   // 聊天室
  public:
    void setChatRoomID(DWORD roomid);
    DWORD getChatRoomID() const { return m_dwChatRoomID; }
    ChatRoom* getChatRoom();
    bool hasChatRoom() { return m_dwChatRoomID != 0; }
  private:
    DWORD m_dwChatRoomID = 0;
    // 天气和时间
  public:
    void setOwnWeather(DWORD dwWeather, bool bPrivate = false);
    DWORD getOwnWeather() const { return m_dwOwnWeather; }
    bool isOwnWeather() const { return m_bPrivateWeather; }

    void setOwnSky(DWORD dwSky, bool bPrivate = false, DWORD sec = 0);
    DWORD getOwnSky() const { return m_dwOwnSky; }
    bool isOwnSky() const { return m_bPrivateSky; }

    void setOwnTime(DWORD dWDestTime, bool bPrivate = false);
    void syncOwnTime(DWORD curTime, bool bAtOnce = false);
    DWORD getOwnTime() { return xTime::getCurSec() + m_sdwGameTimeOffset; }
    bool isOwnTime() const { return m_bPrivateTime; }
  private:
    DWORD m_dwOwnWeather = 0;
    bool m_bPrivateWeather = false;

    DWORD m_dwOwnSky = 0;
    bool m_bPrivateSky = false;

    SDWORD m_sdwGameTimeOffset = 0;
    bool m_bPrivateTime = false;
    // 快捷键
  public:
    Shortcut& getShortcut() { return m_oShortcut; }
  private:
    Shortcut m_oShortcut;

    // 玩家刷塔数据
  public:
    SceneTower& getTower() { return m_oTower; }
  private:
    SceneTower m_oTower;

    // 女神像
  public:
    Freyja& getFreyja() { return m_oFreyja; }
  private:
    Freyja m_oFreyja;
    // 封印
  public:
    Seal& getSeal() { return m_oSeal; }
  private:
    Seal m_oSeal;
    // 问答
  public:
    Interlocution& getInterlocution() { return m_oInter; }
  private:
    Interlocution m_oInter;
    // 冒险手册
  public:
    Manual& getManual() { return m_oManual; }
  private:
    Manual m_oManual;
    // 种族
  public:
    virtual DWORD getRaceType() { return static_cast<DWORD> (ERACE_HUMAN); }
    // 研究所
  public:
    UserLaboratory& getLaboratory() { return m_oLaboratory; }
  private:
    UserLaboratory m_oLaboratory;
    // 计数器
  //public:
  //  CounterManager m_oCounter;
    // 景点
  public:
    UserScenery& getScenery() { return m_oScenery; }
  private:
    UserScenery m_oScenery;
    // 技能,属性相关
  public:
    DWORD getValidNumLockMe();
    void addLockMe(QWORD id) { m_setLockMeIDs.insert(id); XDBG << "id" << id << "开始锁定我了" << XEND; }
    void delLockMe(QWORD id) { m_setLockMeIDs.erase(id); XDBG << "id" << id << "不锁定我了" << XEND; }
    void lockMeCheckEmoji(const char *t);
    const TSetQWORD& getLockMeList() const { return m_setLockMeIDs; }
    void addAttackMe(QWORD guid, DWORD time);
    DWORD getAttackMeSize() { return m_vecTime2Attacker.size(); }
    bool isAttackedMe(QWORD qwID);
  private:
    void watchLockEffect(DWORD curTime);
    TSetQWORD m_setLockMeIDs;
    DWORD m_dwLastLockMeNum = 0;
    DWORD m_dwLockTimeTick = 0;

    vector<pair<DWORD, QWORD>> m_vecTime2Attacker;
    // 人物待机,被持续攻击5s, 自动反击攻击者
  public:
    void addHitMe(QWORD id, DWORD damage);
    void clearHitMe() { m_mapEnemy2HitTime.clear(); }
  private:
    map<QWORD, DWORD> m_mapEnemy2HitTime;
    // lua 函数调用
  public:
    virtual DWORD getWeaponType();
    virtual DWORD getSkillLv(DWORD skillGroupID);
    virtual DWORD getNormalSkill() const { return m_pCurFighter ? m_pCurFighter->getSkill().getNormalSkill() : 0; }
    virtual DWORD getArrowID();// { return m_oPackage.getArrowTypeID(); }
    //virtual DWORD getItemUseCnt(DWORD itemid) { return m_oPackage.getVarUseCnt(itemid); }
    virtual bool isBuffLayerEnough(EBuffType eType) { return m_oBuff.isLayerEnough(eType); }
    void getAllSkillSpec(SSpecSkillInfo& info) { m_oBuff.getAllSkillSpec(info); }// 获取对所有技能的影响
    DWORD getEquipedItemNum(DWORD itemid);
    DWORD getMainPackageItemNum(DWORD itemid);
    DWORD getEquipID(DWORD pos);
    DWORD getEquipRefineLv(DWORD pos);
    DWORD getEquipCardNum(DWORD pos, DWORD cardid);
    xSceneEntryDynamic* getBaseObject() { return (xSceneEntryDynamic*)this; }
    DWORD getRuneSpecNum(DWORD specID);
    DWORD getAppleNum();
    bool isEquipForceOff(DWORD pos);
    DWORD getMapTeamPros(); // 获取同地图队伍中职业数量
    bool hasMonthCard();
    bool isBattleTired();
    bool isSkillEnable(DWORD skillid);
    virtual bool inGuildZone();
    DWORD getEquipedWeapon();
    bool addBuff(DWORD buffid, QWORD targetid);
    virtual bool inSuperGvg();

    // 动作状态, 最近表情
  public:
    virtual void setAction(DWORD id);// { m_dwActionID = id; }
    virtual DWORD getAction() const { return m_dwActionID; }
    void setExpression(DWORD id) { m_dwExpressionID = id; m_dwLastExpressionTime = xTime::getCurSec(); }
    DWORD getExpression() { return m_dwExpressionID; }
    DWORD getExpressionTime() const { return m_dwLastExpressionTime; }
    void playDynamicExpression(EAvatarExpression eExpression);
  private:
    DWORD m_dwActionID = 0;
    DWORD m_dwExpressionID = 0;
    DWORD m_dwLastExpressionTime = 0;
  public:
    DWORD m_dwBarrageTime = 0;

  public:
    bool m_blShowSkill = false;
    bool m_blCheckTeamImage = false;

  public:
    // for team
    void addShowNpc(DWORD id, bool bShare = false);
    void delShowNpc(DWORD id); //{ m_oUserSceneData.delShowNpc(id); }
    const TSetDWORD& getShowNpcs() const { return m_oUserSceneData.getShowNpcs(); }
    // for personal
    void addSeeNpc(DWORD id);
    void delSeeNpc(DWORD id);
    void addHideNpc(DWORD id);
    void delHideNpc(DWORD id);
  private:
    void beforeSendNine();
  public:
    UserHands m_oHands;

    TmpData m_oTmpData;
    // 点唱机
  public:
    void setMusicData(DWORD dwMusicID, DWORD dwStartTime, bool bDemand, xPos pos = xPos(0, 0, 0), bool bLoop = false);
    void setBrowse(bool bBrowse) { m_bBrowse = bBrowse; }
    bool getBrowse() const { return m_bBrowse; }
    /*是否在音乐盒附近*/
    bool isInMusicNpcRange();
  private:
    DWORD m_dwMusicID = 0;
    DWORD m_dwStartTime = 0;
    bool m_bDemand = false;
    bool m_bBrowse = false;
    xPos m_oMusicPos;
    bool m_bLoop = false;
  private:
    DWORD m_dwListenBeginTime = 0;
    DWORD m_dwMusicTimeTick = 0;
    DWORD m_dwMusicTime = 0;
  private:
    // 点唱机恢复战斗时间
    void checkMusicRecover(DWORD curTime, bool atonce = false);

    // 战斗时间统计
  public:
    DWORD getBattleTime() const { return m_oUserSceneData.getBattleTime();}
    void addBattleTime(DWORD timelen, bool bExtra = true);
    //void addSkillTimePoint(DWORD curtime) { if (m_dwBeginBattleT == 0) m_dwBeginBattleT = curtime; else m_dwEndBattleT = curtime; }
    void addSkillTimePoint(DWORD curTime) { m_dwLastBattleT = curTime; }
    void addTeamSkillTimePoint(DWORD curtime);
    void markHitFieldMonster() { m_bHasHitFieldMonster = true; }
  private:
    /*
    DWORD m_dwBeginBattleT = 0;
    DWORD m_dwEndBattleT = 0;

    DWORD m_dwLastBattleT = 0;
    */
    DWORD m_dwLastBattleT = 0;
    DWORD m_dwNextCalcBattleTime = 0;
    bool m_bHasHitFieldMonster = false;

    //防沉迷刷新
  public:
    void antiAddictRefresh(bool clear =false);
    /*
    @brief：获取沉迷时间
    @return：秒为单位
    */
    DWORD getAddictTime();
    /*
    @brief：是否需要沉迷衰减
    */
    bool checkAddict(ENpcType npcType);
    void addcitTips(bool isLogin);
  private:
    DWORD getAddictTipsTime()const { return m_oUserSceneData.getAddictTipsTime(); }
    void setAddictTipsTime(DWORD timelen) { m_oUserSceneData.setAddictTipsTime(timelen); }

    //称号
  public:
    Title& getTitle() { return m_oTitle; }
  private:
    Title m_oTitle;
    // 变身魔物
  public:
    Transform& getTransform() { return m_oTransform; }
  private:
    Transform m_oTransform;
    // 公会
  public:
    bool hasGuild() const { return m_oGuild.id() != 0; }
    void sendGuildInfoToNine();
    void sendGuildCity();
    GGuild& getGuild() { return m_oGuild; }
    DWORD getGuildPrayTimes();
  private:
    GGuild m_oGuild;
    //DOJO
  public:
    SceneDojo& getDojo() { return m_oDojo; }
  private:
    SceneDojo m_oDojo;

    // 随机数, lua调用
  public:
    virtual DWORD getRandomNum();
    virtual void setRandIndex(DWORD index);
    void enterSceneSendRandom() { createRandom(true, true); sendRandom(true, true); }
  private:
    void updateRandom();
    void createRandom(bool AGroup, bool BGroup);
    void sendRandom(bool AGroup, bool BGroup);

    bool m_bUpdateAGroup = false;
    bool m_bUpdateBGroup = false;
    DWORD m_dwRandIndex = 0;
    DWORD m_dwClientLastIndex = 0;
    DWORD m_dwRandIndexErrCnt = 0;
    TVecDWORD m_vecRandomNums;
    // 复活
  public:
    void addReliveMeUser(QWORD userid) { m_setReliveMeUsers.insert(userid); }
    void delReliveMeUser(QWORD userid) { m_setReliveMeUsers.erase(userid); }
    TSetQWORD& getReliveMeUsers() { return m_setReliveMeUsers; }
    void markKillByMonster() { m_bMarkKillByMonster = true; }
    virtual bool isReliveByOther() { return !m_setReliveMeUsers.empty(); }

    bool canClientRelive(EReliveType eType) const;
    void relive(EReliveType eType, SceneUser* pReliver = nullptr);
    void checkReliveWhenLoginout();
    void setKillerName(const string& killerName) { m_strKillerName = killerName; setDataMark(EUSERDATATYPE_KILLERNAME); }
  private:
    TSetQWORD m_setReliveMeUsers;
    bool m_bMarkKillByMonster = false;
    DWORD m_dwDieDropExp = 0;
    string m_strKillerName;
  public:
    bool bOpenMoveTrack = false;

  public:
    bool addReBattleTime(DWORD timelen); // music recover battle time
  public:
    DWORD m_dwEnterNanMenTime = 0;
  public:
    const BaseSkill* getLearnedSkillByID(DWORD skillid, bool bJustNormal = false); // 获得学过的技能对象

   // hand npc
  private:
    UserHandNpc m_oHandNpc;
  public:
    UserHandNpc& getHandNpc() { return m_oHandNpc; }

   // fuben step
  public:
    void setClientFubenID(DWORD stepid) { m_dwFubenStepID = stepid; }
    DWORD getClientFubenID() { return m_dwFubenStepID; }
  private:
    DWORD m_dwFubenStepID = 0;

    // itemmusic
  private:
    UserItemMusic m_oItemMusic;
  public:
    UserItemMusic& getItemMusic() { return m_oItemMusic; }

    // camera
  private:
    UserCamera m_oCamera;
  public:
    UserCamera& getUserCamera() { return m_oCamera; }

    // zone
  private:
    UserZone m_oZone;
  public:
    UserZone& getUserZone() { return m_oZone; }
    bool jumpZone(QWORD qwNpcID, DWORD dwLine);

    void setTmpJumpZone(DWORD dwZoneID) { m_dwTmpJumpZone = dwZoneID; }
    DWORD getTmpJumpZone() const { return m_dwTmpJumpZone; }
  private:
    DWORD m_dwTmpJumpZone = 0;
  public:
    SceneTeamSeal* getTeamSeal() { return TeamSealManager::getMe().getTeamSealByID(getTeamID()); }
  public:
    void sendItemImage(bool broadcast);

    // 社交
  public:
    GSocial& getSocial() { return m_oSocial; }
    bool addRelationPreCheck(QWORD qwCharID, ESocialRelation eRelation);
    QWORD getTutorCharID() { return m_oSocial.getTutorCharID(); }
    void onSocialChange();
  private:
    GSocial m_oSocial;

    //help reward
  public:
    void getHelpReward(const std::set<SceneUser*>& helpSet, EHelpType eType, bool bSelf = false, DWORD specFriendCnt = 0, DWORD specGuildCnt = 0); // bSelf 表示自己属于有效完成的玩家

  private:
    void addExpLog(DWORD expId, DWORD  decCount, DWORD after, ESource source);
    void reduceExpLog(DWORD expId, DWORD  decCount, DWORD after, ESource source);
  private:
    //void setGChar();

  public:
    Deposit& getDeposit() { return m_oDeposit; }
    DWORD getDepositAddictTime() { return m_oDeposit.getAddcitTime(0); }
    DWORD getOneDayBattleTime();
  private:
    Deposit m_oDeposit;

    // scene item
  public:
    bool canPickup() const { return m_qwLastPickupTime + MiscConfig::getMe().getSceneItemCFG().dwDropInterval < xTime::getCurMSec(); }
    void setPickupTime() { m_qwLastPickupTime = xTime::getCurMSec(); }
  private:
    QWORD m_qwLastPickupTime = 0;

    // be attacked by mvp or its monsters
  private:
    map<QWORD, DWORD> m_mapMvpID2Damage;
    QWORD m_qwKillMeMvpID = 0;
    // replace music
  public:
    void replaceBgmToMe(const string& bgm);
  public:
    bool isInBattleTimeMap();

  public:
    //获取防沉迷衰减率
    float getAddictRatio();
    //获取组队防沉迷衰减率
    float getTeamAddictRatio();
  public:
    void setLastChat(DWORD time, const string& str) { oPairTime2Msg.first = time; oPairTime2Msg.second = str; }
    const std::pair<DWORD, string>& getLastChat() { return oPairTime2Msg; }
    void setBadChatValue(DWORD value) { m_dwLastBadChatValue = value; }
    DWORD getLastBadValue() const { return m_dwLastBadChatValue; }
    void setBadChatTime(DWORD time) { m_dwLastBadChatTime = time; }
    DWORD getLastBadTime() const { return m_dwLastBadChatTime; }
  private:
    std::pair<DWORD, string> oPairTime2Msg;
    DWORD m_dwLastBadChatValue = 0;
    DWORD m_dwLastBadChatTime = 0;
  public:
    void getExtraReward(EExtraRewardType eType);
    DWORD getDoubleReward(EDoubleRewardType eType, TVecItemInfo& vecReward);
  private:
    SChatParams m_oChatParam;
  public:
    SChatParams& getChatParam() { return m_oChatParam; }
  public:
    void playChargeNpc(const Cmd::ChargePlayUserCmd& rev);
    void playChargeNpcTick(DWORD curSec);
  private:
    std::list<DWORD/*data id*/> m_chargeList;
    bool m_bPlayCharge = false;
    DWORD m_dwNextPlayTime = 0; 

  public:
    void setPhone(const string& phone);
    bool hasPhone() { return !m_strPhone.empty(); }
  private:
    string m_strPhone;
    // 聊天信息
  public:
    UserChat& getUserChat() { return m_oUserChat; }
  private:
    UserChat m_oUserChat;
  public:
    UserStat m_oUserStat;
  public:
    EBattleStatus getBattleStatus();
    DWORD getTotalBattleTime() { return m_oUserSceneData.getTotalBattleTime(); }
    void sendBattleStatusToMe();

  public:
    bool checkSeat(DWORD seatId);
    void seatUp();
    void sendSeatCmdToNine(DWORD seatId, bool isSeatDown);
    bool checkYoYoSeat(QWORD npcguid);
    bool checkSeatTime();
  private:
    DWORD m_seatId = 0;
  public:
    WeaponPet& getWeaponPet() { return m_oWeaponPet; }
  private:
    WeaponPet m_oWeaponPet;
  private:
    DWORD m_dwDieTime = 0;
    DWORD m_dwLastDieTime = 0;
    DWORD m_dwReliveCD = 0;

    QWORD m_qwReliverID = 0;
    DWORD m_dwTime2Relive = 0;
    bool m_bSuperGvgExpel = false;
  public:
    DWORD getDieTime() { return m_dwDieTime; }
    DWORD getReliveCD() { return m_dwReliveCD; }
    void setWaitRelive(QWORD reliverid, DWORD time) { m_qwReliverID = reliverid; m_dwTime2Relive = time; }
    bool inReliveStatus() { return getStatus() == ECREATURESTATUS_DEAD && m_dwTime2Relive >= now() && m_dwTime2Relive >= m_dwDieTime; }
  private:
    DWORD m_dwSkillQueueCntTick = 0;
  public:
    UserGingerBread m_oGingerBread;     
  public:
    UserTicket& getTicket() { return m_oTicket; }
  private:
    UserTicket m_oTicket;
  public:
    Share& getShare() { return m_oShare; }

    void refreshTeamTime(DWORD dwTime);
  private:
    Share m_oShare;
  public:
    Achieve& getAchieve() { return m_oAchieve; }

    void addBeUsedItem(DWORD dwItem);
    DWORD getBeUsedItemCount(DWORD dwItem);
  private:
    Achieve m_oAchieve;
  public:
    void refreshTradeInfo() { m_bMarkRefreshTradeInfo = true; }
    void refreshTradeQuota();
    void refreshBoothInfo() { m_bMarkRefreshBoothInfo = true; }
  private:
    void refreshPendingCount();
    void refrshTradeBackMoneyPer();
    void refreshBoothCount();
  private:
    bool m_bMarkRefreshTradeInfo = false;
    bool m_bMarkRefreshBoothInfo = false;
    DWORD m_dwTradePendingCount = 0;
    DWORD m_dwTradeBackMoneyPer = 0; // per * 1000
  public:
    void enterGuildRaid(QWORD npcguid);
    void sendGuildGateDataToMe(DWORD npcid = 0);
    DWORD getGuildGateGearStatus(SceneNpc* pNpc);
    void openGuildRaidGate(QWORD npcguid);
    void unlockGuildRaidGate(QWORD npcguid, DWORD level);
    void unlockGuildRaidGate(SceneNpc* pNpc, DWORD lv, bool sync = true);
    void syncGuildRaidData(DWORD guildlv);
  public:
    void getTaskExtraReward(ETaskExtraRewardType type, DWORD times);
    //安全密码
  private:
    bool m_blIgnorepwd = false;
  public:
    void setIgnorePwd(bool ignore) { m_blIgnorepwd = ignore; }
    bool checkPwd(EUnlockType type, DWORD param = 0);
  public:
    DWORD getSceneTeamCnt();
    void addKillCount(DWORD monsterid, float fcnt);
    float getKillCount(DWORD monsterid);
  private:
    map<DWORD, float> m_mapMonsterKillNum;
    DWORD m_dwKillCountPeriod = 0;;
  public:
    DWORD getProfessionType() { return getRoleBaseCFG() == nullptr ? 0 : getRoleBaseCFG()->dwType; }
    DWORD getEvo();
    DWORD getProfesTypeBranch() { return getRoleBaseCFG() == nullptr ? 0 : getRoleBaseCFG()->dwTypeBranch; }
    Astrolabes& getAstrolabes() { return m_oAstrolabes; }
  private:
    Astrolabes m_oAstrolabes;
  public:
    DWORD getAstrolabePoint(DWORD specID) { return getAstrolabes().getEffectCnt(specID); }
  public:
    void redisPatch();
  public:
    void getCelebrationID(TVecDWORD& vec);
    bool getCelebrationReward(DWORD level);
    bool getActivityReward();
    void showMonthCardErrorLog();
  public:
    bool isWantedQuestLeader();

  private:
    map<DWORD,LoveLetterData> m_mapLoveLetter;
  public:
    void addLoveLetter(DWORD letterID, string sendUserName, string bg, DWORD configID, string content);
    bool addLoveLetterItem(DWORD letterID);
  private:
    UserPhoto m_oPhoto;
  public:
    UserPhoto& getPhoto() { return m_oPhoto; }
    void sendUpyunUrl();
    void sendUpyunAuthorization();
    bool getAlbumName(EAlbumType type, string& name);

  private:
    UserMap m_oUserMap;
  public:
    UserMap& getUserMap() { return m_oUserMap; }
  public:
    void sendTransformPreData();
  public:
    UserPet& getUserPet() { return m_oUserPet; }
    void getAllFriendNpcs(std::list<SceneNpc*>& list); // 宠物, 猫, 生命体
  private:
    UserPet m_oUserPet;
    // 宠物冒险
  public:
    PetAdventure& getPetAdventure() { return m_oPetAdventure; }
  private:
    PetAdventure m_oPetAdventure;
  public:
    SceneFood& getSceneFood() { return m_oSceneFood; }
    void soundEffect(DWORD dwId);
    DWORD getCookerLv() { return m_oSceneFood.getCookerLv(); }
    DWORD getTasterLv() { return m_oSceneFood.getTasterLv(); }
  private:
    SceneFood m_oSceneFood;
  public:
    Lottery& getLottery() { return m_oLottery; }
  private:
    Lottery m_oLottery;

  public:
    void handleFollower(const Cmd::InviteFollowUserCmd cmd);
  public:
    bool getPracticeReward();

  public:
    TutorTask& getTutorTask() { return m_oTutorTask; }
  private:
    TutorTask m_oTutorTask;
    // 行为
  public:
    Action& getUserAction() { return m_oAction; }
  private:
    Action m_oAction;
  public:
    void stopSendInactiveLog();
    // 美瞳
  public:
    Eye& getEye() { return m_oEye; }
  private:
    Eye m_oEye;
  private:
    DWORD m_dwLanguage = ELANGUAGE_CHINESE_SIMPLIFIED;  //  简体中文
  public:
    DWORD getLanguage() { return m_dwLanguage; }
    void setLanguage(DWORD value) { m_dwLanguage = value; }
  public:
    bool isInPollyScene();
    DWORD m_dwEnterSceneTime = 0;
  public:
    UserBeing& getUserBeing() { return m_oUserBeing; }
    bool isBeingPresent(DWORD beingid) { return m_oUserBeing.isBeingPresent(beingid); }
    QWORD getBeingGUID() { return m_oUserBeing.getCurBeingGUID(); }
  private:
    UserBeing m_oUserBeing;
  public:
    void processLoveLetter(const Cmd::LoveLetterSessionCmd cmd);
    void processLoveLetterSend(const LoveLetterSendSessionCmd& cmd);
  private:

    void addEquipAttrAction();
  public:
    DWORD getPeakEffect();
  public:
    HighRefine& getHighRefine() { return m_oHighRefine; }
  private:
    HighRefine m_oHighRefine;
  public:
    GuildChallengeMgr& getGuildChallenge() { return m_oGuildChallenge; }
  private:
    GuildChallengeMgr m_oGuildChallenge;
  public:
    void sendOpenBuildingGateMsg();
  public:
    void updateArtifact();
    void updateArtifact(const ArtifactUpdateGuildSCmd& cmd);
  public:
    void setRealAuthorized(bool b) { m_bRealAuthorized = b; }
    bool isRealAuthorized();
  private:
    bool m_bRealAuthorized = false;
    // 周年庆-好友回归
  public:
    void sendRecallReward();
    void getFirstShareReward();
  private:
    void checkRecallBuff();
  public:
    bool m_blThreadLoad = false;
    UserGvg& getUserGvg() { return m_oUserGvg; }
  private:
    UserGvg m_oUserGvg;

  public:
    UserProposal m_oProposal;
  public:
    UserProposal& getProposal() {return m_oProposal;}
  public:
    UserWedding& getUserWedding() { return m_oUserWedding; }
    void checkWeddingBuff();
  private:
    UserWedding m_oUserWedding;
  public:
    void notifyInviteeWeddingStart();
    // 宠物打工
  public:
    PetWork& getPetWork();
  private:
    PetWork* m_pPetWork = nullptr;
  public :
    bool addOperateRewardVar(DWORD eType);
    void SyncOperateRewardToSession();
    void patch_OperateReward();
    // 变性
  public:
    EError changeGender();
  public:
    bool checkOtherRelation(QWORD tarid, ESocialRelation eType);
  private:
    QWORD m_qwTwinsID = 0;
    DWORD m_dwTwinsActionID = 0;
    bool m_bSponsor = false;
    bool m_bTiwnsClose = false;
    DWORD m_dwRequestTime = 0;
  public:
    QWORD getTwinsID() { return m_qwTwinsID; }
    void setTwinsID(QWORD dwid) { m_qwTwinsID = dwid; }
    DWORD getTwinsActionID() { return m_dwTwinsActionID; }
    void setTwinsActionID(DWORD actid) { m_dwTwinsActionID = actid; }
    bool getTwinsSponsor() const { return m_bSponsor; }
    void setTwinsSponsor(bool sponsor) { m_bSponsor = sponsor; }
    bool getTwinsClose() const { return m_bTiwnsClose; }
    void setTwinsClose(bool bClose) { m_bTiwnsClose = bClose; }
    DWORD getRequestTime() const { return m_dwRequestTime; }
    void setRequestTime(DWORD dwTime) { m_dwRequestTime = dwTime; }
    void handleTwinsAction(const TwinsActionUserCmd cmd);
    void notifyTwinsAction(QWORD userid, DWORD actionid, bool sponsor);
    void changeTwinsStatus(QWORD userid);
    void onTwinsMove();
  public:
    //判断当前是否是骑乘(龙)状态
    bool isRide(DWORD dwId);
    //判断当前的PartnerID是否是指定的ID
    bool isPartnerID(DWORD dwId);
    // 仆人管理
  public:
    Servant& getServant() { return m_oServant; }
  private:
    Servant m_oServant;
    //玩家存档位
  public:
    UserRecords& getUserRecords();
  private:
    UserRecords* m_pUserRecord = nullptr;

  // 多职业
  public:
    Profession m_oProfession;
    DWORD getBranch() { return nullptr == m_pCurFighter ? 0 : m_pCurFighter->getBranch(); }
  public:
    bool buyProfession(DWORD dwBranch);
    bool changeProfession(DWORD dwBranch);
    bool checkProfessionBuy();
    void syncProfessionData(Cmd::EProfressionDataType type);
    void setBranch();
    bool checkMapForChangeProfession();
    bool fixBranch(std::vector<std::pair<DWORD, DWORD>>& vecProfessions);
    void reqFixBranch();
    bool isBuy();
  private:
    bool m_bEquipedAltmanFashion = false;
  public:
    bool isEquipedAltmanFashion();
    bool getAltmanFashion() const { return m_bEquipedAltmanFashion; }
    void onAltmanFashionEquip();
    void onAltmanEnd();
    void altmanCheck();

  public:
    float getNormalDrop(float in) { return m_oDeposit.getNormalDrop(in); }

    // 摆摊
  public:
    Booth m_oBooth;
  public:
    std::set<SceneUser*> getTeamSceneUserInPvpGvg();
  public:
    void leaveDressSatgeOnUnreg();
    void breakAllFollowers();
    // 舞台换装
  public:
    DressUp& getDressUp() { return m_oDressUp; }
  private:
    DressUp m_oDressUp;

  // 属性刷新限制,决战地图1秒刷新一次
  private:
    DWORD m_dwLastRefreshTimeTick = 0;
  // 死亡国度传送阵
  public:
    Transfer& getTransfer() { return m_oTransfer; }
    bool canUseWingOfFly() { return m_oTransfer.canUseWingOfFly(getMapID()); }
    bool canTeamUseWingOfFly();
  private:
    Transfer m_oTransfer;

    //兑换商店
  public:
    ExchangeShop& getExchangeShop();
  private:
    ExchangeShop* m_pExchangeShop = nullptr;
    //世界等级
  public:
    void checkWorldLevelBuff();
  // 贤者召唤元素
  public:
    UserElementElf& getUserElementElf() { return m_oUserElementElf; }
    DWORD getCurElementElfID() { return m_oUserElementElf.getCurElementID(); }
  private:
    UserElementElf m_oUserElementElf;
  public:
    xSceneEntryDynamic* getEnsemblePartner();

  public:
    void updateEnsembleSkill();
    const string& getEnsembleSkill() { return m_strEnsembleSkill; }
  private:
    void setEnsembleSkill(const string& str);
    string m_strEnsembleSkill;

    // 组队排位赛数据
  public:
    UserMatchData& getMatchData() { return m_oMatchData; }
  private:
    UserMatchData m_oMatchData;

  public:
    // 模型展示
    void toModelShowData(QueryUserInfo* pInfo);
    void toPortraitData(UserPortraitData* pData);
    void sendVarToSession(EVarType eType);
  private:
    CheatTag m_oCheatTag;
  public:
    CheatTag& getCheatTag(){ return m_oCheatTag; };
};


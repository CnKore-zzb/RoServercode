/**
 * @file UserSceneData.h
 * @brief user data 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-04-22
 */

#pragma once

#include <string>
#include <vector>
#include "RecordCmd.pb.h"
#include "ProtoCommon.pb.h"
#include "SceneUser2.pb.h"
#include "UserEvent.pb.h"
#include "xPos.h"
#include "RewardConfig.h"
#include "MiscConfig.h"
#include <bitset>
#include <list>

using std::string;
using std::vector;
using std::pair;
using std::set;
using std::list;
using namespace Cmd;

// gm effect data
struct SGmEffect
{
  DWORD mapid = 0;
  DWORD index = 0;
  string gmeffect;
};

// item trace
struct STraceItem
{
  DWORD dwItemID = 0;
  DWORD dwMonsterID = 0;

  STraceItem() {}
  STraceItem(DWORD itemid, DWORD monsterid) : dwItemID(itemid), dwMonsterID(monsterid) {}
};
typedef vector<STraceItem> TVecTraceItem;

// option
struct SUserOption
{
  EQueryType eType = EQUERYTYPE_MIN;
  DWORD m_dwNormalSkillOption = 0;
  DWORD m_dwFashionHide = 0;
  std::bitset<64> m_btSet;
  map<ESkillOption, DWORD> m_mapSkillOpt;
  EQueryType eWeddingType = EQUERYTYPE_MIN;

};

// recent zone
struct SRecentZoneInfo
{
  EJumpZone eType = EJUMPZONE_MIN;
  DWORD dwZoneID = 0;

  SRecentZoneInfo(EJumpZone eZone, DWORD zoneID) : eType(eZone), dwZoneID(zoneID) {}
};
typedef vector<SRecentZoneInfo> TVecRecentZoneInfo;

// 活动模板额外奖励
struct SAERewardData
{
  EAERewardMode eMode = EAEREWARDMODE_MIN;

  // char
  DWORD dwDayCount = 0;
  DWORD dwMulDayCount = 0;

  // acc
  QWORD qwAccLimitCharid = 0;
  QWORD qwMulAccLimitCharid = 0;

  void fromCharData(const AERewardItem& data) {
    eMode = data.mode();
    dwDayCount = data.daycount();
    dwMulDayCount = data.multipledaycount();
  }
  void fromAccData(const AERewardItem& data) {
    eMode = data.mode();
    qwAccLimitCharid = data.acclimitcharid();
    qwMulAccLimitCharid = data.multipleacclimitcharid();
  }
  void toData(AERewardItem* data) {
    if (data)
    {
      data->set_mode(eMode);
      data->set_daycount(dwDayCount);
      data->set_acclimitcharid(qwAccLimitCharid);
      data->set_multipledaycount(dwMulDayCount);
      data->set_multipleacclimitcharid(qwMulAccLimitCharid);
    }
  }

  void resetAcc() {
    qwAccLimitCharid = 0;
    qwMulAccLimitCharid = 0;
  }
  void resetChar() {
    dwDayCount = 0;
    dwMulDayCount = 0;
  }
};

// 活动模板

typedef std::map<QWORD/*id*/, ActivityEventCnt> TMapId2ActivityEvent;
typedef std::map<EActivityEventType, TMapId2ActivityEvent> TMapType2ActivityEvent;

struct SActivityEventData
{
  map<EAERewardMode, SAERewardData> mapReward;

  TMapType2ActivityEvent mapAccEventCnt;
  TMapType2ActivityEvent mapCharEventCnt;
  
  void fromData(const BlobActivityEvent& accdata, const BlobActivityEvent& chardata) {
    for (int i = 0; i < accdata.rewarditems_size(); ++i)
      mapReward[accdata.rewarditems(i).mode()].fromAccData(accdata.rewarditems(i));
    for (int i = 0; i < chardata.rewarditems_size(); ++i)
      mapReward[chardata.rewarditems(i).mode()].fromCharData(chardata.rewarditems(i));

    //acc
    for (int i = 0; i < accdata.eventcnt_size(); ++i)
    {
      const ActivityEventCnt& rCnt = accdata.eventcnt(i);
      TMapId2ActivityEvent& rId2Cnt = mapAccEventCnt[rCnt.type()];
      rId2Cnt[rCnt.id()] = rCnt;
    }

    //char
    for (int i = 0; i < chardata.eventcnt_size(); ++i)
    {
      const ActivityEventCnt& rCnt = chardata.eventcnt(i);
      TMapId2ActivityEvent& rId2Cnt = mapCharEventCnt[rCnt.type()];
      rId2Cnt[rCnt.id()] = rCnt;
    }
  }

  void toData(BlobActivityEvent* accdata, BlobActivityEvent* chardata) {
    if (accdata)
    {
      for (auto& v : mapReward)
        v.second.toData(accdata->add_rewarditems());
    }
    if (chardata)
    {
      for (auto& v : mapReward)
        v.second.toData(chardata->add_rewarditems());
    }

    auto f = [&](TMapType2ActivityEvent& rMapEventCnt, BlobActivityEvent* data)
    {
      for (auto&m : rMapEventCnt)
      {
        for (auto&m2 : m.second)
        {
          data->add_eventcnt()->CopyFrom(m2.second);
        }
      }
    };
    if (accdata)
      f(mapAccEventCnt, accdata);
    if (chardata)
      f(mapCharEventCnt, chardata);
  }

  void toActivityEventUserDataNtf(ActivityEventUserDataNtf* data, EAERewardMode mode) {
    if (data == nullptr)
      return;
    if (mode == EAEREWARDMODE_MIN)
    {
      for (auto& v : mapReward)
        v.second.toData(data->add_rewarditems());
    }
    else
    {
      auto it = mapReward.find(mode);
      if (it != mapReward.end())
        it->second.toData(data->add_rewarditems());
    }   
  }

  void resetChar() {
    for (auto& v : mapReward)
      v.second.resetChar();
  }
  void resetAcc() {
    for (auto& v : mapReward)
      v.second.resetAcc();
  }
};

// credit
struct SCreditData
{
  int iCurCredit = 0;
  DWORD dwMonsterValue = 0;
  DWORD dwSavedTime = 0;
  DWORD dwForbidRecoverTime = 0;

  void toData(BlobNewCredit* pData) {
    if (pData == nullptr)
      return;
    pData->set_credit(iCurCredit);
    pData->set_monster_value(dwMonsterValue);
    pData->set_savedtime(dwSavedTime);
    pData->set_forbidtime(dwForbidRecoverTime);
  }
  void fromData(const BlobNewCredit& oData) {
    iCurCredit = oData.credit();
    dwMonsterValue = oData.monster_value();
    dwSavedTime = oData.savedtime();
    dwForbidRecoverTime = oData.forbidtime();
  }
};

// acc user
struct SUserAccData
{
  DWORD dwAuguryRewardTime = 0;   //运营活动,占卜奖励领取时间

  DWORD dwMaxBaseLv = 0; // 每日5点刷新的最大角色等级
  DWORD dwMaxBaseLvResetTime = 0;
  DWORD dwCurMaxBaseLv = 0; // 在登录时由proxyserver同步过来的最大等级

  TSetDWORD setShowNpcs;

  SActivityEventData stActivityEvent;

  list<PhotoMd5> listPhotoMd5;

  std::map<EJoyActivityType, DWORD> m_mapJoy;

  TSetQWORD setPatchVersion;

  void toData(BlobAccUser* pData) {
    if (pData == nullptr)
      return;
    pData->set_auguryreward(dwAuguryRewardTime);
    pData->set_maxbaselv(dwMaxBaseLv);
    pData->set_maxbaselv_resettime(dwMaxBaseLvResetTime);

    pData->clear_md5s();
    for (auto &l : listPhotoMd5)
      pData->add_md5s()->CopyFrom(l);
    for(auto s : m_mapJoy)
    {
      BlobJoyData* pJoyData = pData->add_accjoy();
      if(pJoyData != nullptr)
      {
        pJoyData->set_etype(s.first);
        pJoyData->set_joyvalue(s.second);
      }
    }

    for (auto &s : setPatchVersion)
      pData->add_patchversion(s);
  }
  void fromData(const BlobAccUser& oData) {
    dwAuguryRewardTime = oData.auguryreward();
    dwMaxBaseLv = oData.maxbaselv();
    dwMaxBaseLvResetTime = oData.maxbaselv_resettime();

    listPhotoMd5.clear();
    for (int i = 0; i < oData.md5s_size(); ++i)
      listPhotoMd5.push_back(oData.md5s(i));
    for(int i = 0; i < oData.accjoy_size(); ++i)
    {
      const BlobJoyData pdata = oData.accjoy(i);
      EJoyActivityType eType = pdata.etype();
      DWORD dwJoy = pdata.joyvalue();
      m_mapJoy.insert(std::make_pair(eType, dwJoy));
    }

    setPatchVersion.clear();
    for (int i = 0; i < oData.patchversion_size(); ++i)
      setPatchVersion.insert(oData.patchversion(i));
  }
  bool addJoyValue(EJoyActivityType etype, DWORD value)
  {
    DWORD typelimit = MiscConfig::getMe().getJoyLimitByType(etype);
    auto s = m_mapJoy.find(etype);
    if(s != m_mapJoy.end())
    {
      if(s->second >= typelimit)
        return false;
      s->second += value;
      if(s->second > typelimit)
        s->second = typelimit;
    }
    else
    {
      if(value > typelimit)
        value = typelimit;
      m_mapJoy.insert(std::make_pair(etype, value));
    }

    return true;
  }
  void setCurMaxBaseLv(DWORD maxbaselv) { dwCurMaxBaseLv = maxbaselv; }
};

// activity data
struct SActivityData
{
  string name;

  TVecDWORD vecParams;
};
typedef std::map<string, SActivityData> TMapName2ActivityData;

// 公会随机副本
struct SGuildRaidData
{
  DWORD dwNpcId = 0;
  EGuildGateState eState = EGUILDGATESTATE_LOCK;
  TSetDWORD setKilledBoss;
};
typedef std::map<DWORD, SGuildRaidData> TMapNpcID2GuildRaidData;

// blob data
struct SUserBlobData
{
  // user data
  xPos oPos;

  DWORD dwClothColor = 0;
  DWORD dwPurify = 0;
  DWORD dwSaveMap = 0;
  DWORD dwLastMap = 0;
  DWORD dwLastRealMap = 0;

  TSetDWORD setShowNpcs;
  TSetDWORD setMapArea;
  TSetDWORD setPatchVersion;

  // first action
  DWORD dwFirstAction = 0;

  // show
  TSetDWORD setActions;
  TSetDWORD setExpressions;

  // known map
  DWORD dwKnownMapVer = 0;
  TSetDWORD setKnownMaps;

  // gm effect
  vector<SGmEffect> gms;

  // trace
  TVecTraceItem vecTraceItem;

  // teamtimelen
  DWORD dwTeamTimeLen = 0;

  // followerid
  QWORD qwFollowerID = 0;

  DWORD dwLevelUpTime = 0;

  // recent zone
  TVecRecentZoneInfo vecRecentZones;

  //
  DWORD dwLastSMapid = 0;
  xPos lastSPos;

  //heal count
  DWORD dwHealCount = 0;

  // credit
  SCreditData stCredit;

  // activity data
  TMapName2ActivityData mapActivityData;

  DWORD dwTotalBattleTime = 0;
  // black
  //TSetQWORD setBlackIDs;
  TSetDWORD setSeeNpcs;
  TSetDWORD setHideNpcs;

  // zeny
  QWORD qwZenyMax = 0;
  QWORD qwZenyDebt = 0;

  // guild raid
  TMapNpcID2GuildRaidData mapGuildRaidData;
  DWORD m_dwGuildRaidVersion = 0;

  // 传送之阵保存地图
  DWORD dwTransMap = 0;
  xPos oTransPos;

  // pvp coin
  DWORD dwPvpCoin = 0;

  // 贡献
  /*DWORD dwCon = 0;
  bool bConInit = false;*/

  QWORD qwChargeZeny = 0;
  QWORD qwChargeLottery = 0;

  //20180706 该字段修改为统计当天总共获得的zeny
  QWORD qwDailyNormalZeny = 0;
  //20180706 该字段修改为统计当天通过充值购买、B格猫金币购买和打开礼包3841-10,000,000 Zeny 后获得的zeny数量
  QWORD qwDailyChargeZeny = 0;

  DWORD dwLotteryCoin = 0;
  // 公会荣耀
  DWORD dwGuildHonor = 0;

  // 改名时间戳
  DWORD dwRenameTime = 0;

  QWORD qwSaveIndex = 0;

  SActivityEventData stActivityEvent;
  bool bDivorceRollerCoaster = false;

  // 导师从学生获得的战斗时长
  DWORD dwTutorbattletime = 0;
  // 使用的部分
  DWORD dwUsedtutorbattletime = 0;
  //上次离线时间、离线时job等级base等级。用于世界等级统计
  DWORD dwLastOfflineTime = 0;
  DWORD dwLastBaseLv = 0;
  DWORD dwLastJobLv = 0;
  DWORD dwDeadCoin = 0;
  DWORD dwDeadLv = 0;
  DWORD dwDeadExp = 0;
};

// online data
struct SUserOnlineData
{
  DWORD dwDir = 0;
  bool bBlink = false;
  bool bDebtNtf = false;

  TSetDWORD setPackSkill;

  SUserOnlineData() {}
};

class SceneUser;
class UserSceneData
{
  public:
    UserSceneData(SceneUser* pUser);
    ~UserSceneData();

  private:
    void setOnlineMapID(DWORD mapID);
  public:
    void setOnlinePos(const xPos& rPos) { m_stBlobData.oPos = rPos; }
    const xPos& getOnlinePos() const { return m_stBlobData.oPos; }
    DWORD getOnlineMapID() const { return m_oBaseData.mapid(); }
    void setOnlineMapPos(DWORD mapID, const xPos &pos);
    DWORD getRaidid() const;
  public:
    // data
    bool fromAccData(const UserAccData& rData);
    bool fromBaseData(const UserBaseData& rData);
    bool toBaseData(UserBaseData& rData);
    //多职业外观预览
    bool toPreviewAppearanceData(ProfessionData& rData);

    bool load(const BlobAccData& oAccBlob, const BlobData& oBlob);
    bool save(BlobAccData& oAccBlob, BlobData& oBlob);

    bool loadActivity(const BlobActivityEvent& acc_data, const BlobActivityEvent& char_data);
    bool saveActivity(BlobActivityEvent* acc_data, BlobActivityEvent* char_data);

    bool loadCredit(const BlobNewCredit& oCredit);
    bool saveCredit(BlobNewCredit* pCredit);

    bool loadAccUser(const BlobAccUser& rData);
    bool saveAccUser(BlobAccUser* pData);

    bool loadBoss(const BlobBoss& rData);
    bool saveBoss(BlobBoss* pData);

    // base data
    void setGender(EGender gender);
    EGender getGender() const { return m_oBaseData.gender(); }
    void setRolelv(DWORD value);
    DWORD getRolelv() const { return m_oBaseData.rolelv(); }
    void setRoleexp(QWORD value);
    QWORD getRoleexp() const { return m_oBaseData.roleexp(); }
    void setPurify(DWORD value);
    DWORD getPurify() const { return m_stBlobData.dwPurify; }
    void setOnlineTime();
    void setOfflineTime(DWORD time);
    void setProfession(EProfession eProfession);
    EProfession getProfession() const;
    EProfession getDestProfession() const { return m_oBaseData.destprofession(); }

    void setMaxPro();

    void setCharge(DWORD value);
    DWORD getCharge() const { return m_oBaseData.charge(); }
    void setDiamond(DWORD value);
    DWORD getDiamond() const { return m_oBaseData.diamond(); }
    void setSilver(QWORD value);
    QWORD getSilver() const { return m_oBaseData.silver(); }
    void setGold(DWORD value);
    DWORD getGold() const { return m_oBaseData.gold(); }
    void setGarden(DWORD value);
    DWORD getGarden() const { return m_oBaseData.garden(); }
    void setFriendShip(DWORD value);
    DWORD getFriendShip();

    void addZenyDebt(QWORD qwMax, QWORD qwDebt);
    QWORD getZenyMax() const { return m_stBlobData.qwZenyMax; }
    QWORD getZenyDebt() const { return m_stBlobData.qwZenyDebt; }

    DWORD getOnlineTime() const { return m_oBaseData.onlinetime(); }
    DWORD getOfflineTime() const { return m_oBaseData.offlinetime(); }
    DWORD getAddict() const { return m_oBaseData.addict(); }
    DWORD getPlatformId() const { return m_oBaseData.platformid(); }

    void setCreateTime(DWORD dwTime);
    DWORD getCreateTime() const { return m_oBaseData.createtime(); }
    bool isNew() { return ((getOnlineTime() - getCreateTime()) < 24*60*60 ? true:false); };

    void setZoneID(DWORD zoneid, EJumpZone eMethod = EJUMPZONE_MIN);
    DWORD getZoneID() const { return m_oBaseData.zoneid(); }
    void setDestZoneID(DWORD zoneid);
    DWORD getDestZoneID() const { return m_oBaseData.destzoneid(); }
    void setOriginalZoneID(DWORD zoneid);
    DWORD getOriginalZoneID() const { return m_oBaseData.originalzoneid(); }
    void setBattleTime(DWORD timelen);
    DWORD getBattleTime() const { return m_oBaseData.battletime(); }
    void setReBattleTime(DWORD timelen);
    DWORD getReBattleTime() const { return m_oBaseData.rebattletime(); }
    void setUsedBattleTime(DWORD timelen);
    DWORD getUsedBattleTime() const { return m_oBaseData.usedbattletime(); }
    void setTotalBattleTime(DWORD timelen);
    DWORD getTotalBattleTime() const { return m_stBlobData.dwTotalBattleTime; }
    void setTutorBattleTime(DWORD timelen);
    DWORD getTutorBattleTime() const { return m_stBlobData.dwTutorbattletime; }
    void setUsedTutorBattleTime(DWORD timelen);
    DWORD getUsedTutorBattleTime() const { return m_stBlobData.dwUsedtutorbattletime; }
    bool haveEnoughBattleTime() const { return m_oBaseData.battletime() < m_stBlobData.dwTotalBattleTime|| m_oBaseData.usedbattletime() < m_oBaseData.rebattletime() || m_stBlobData.dwUsedtutorbattletime < m_stBlobData.dwTutorbattletime; }

    void setAddictTipsTime(DWORD timelen);
    DWORD getAddictTipsTime() const { return m_oBaseData.addicttipstime(); }
    QWORD getSaveIndex()const { return m_stBlobData.qwSaveIndex; }

    DWORD getGagTime() const { return m_oBaseData.gagtime(); }
    void setGagTime(DWORD dwTime, string reason = "");
    void setNologinTime(DWORD dwTime, string reason = "");
    DWORD getTeamTimeLen() const { return m_stBlobData.dwTeamTimeLen; }
    void setTeamTimeLen(DWORD timeLen);
    void addHealCount();
    void setNormalSkillOption(DWORD option);
    DWORD getNormalSkillOption() const { return m_stOptionData.m_dwNormalSkillOption; }
    void setOption(EOptionType type, DWORD flag);
    DWORD getOption(EOptionType type);
    QWORD getOptionSet();
    void setSkillOpt(ESkillOption opt, DWORD value);
    const map<ESkillOption, DWORD>& getSkillOpt() { return m_stOptionData.m_mapSkillOpt; }
    DWORD getSkillOptValue(ESkillOption opt);

    // show data
    void setClothColor(DWORD value, bool bRealUse = true);
    DWORD getClothColor(bool bReal = false) const;

    // body color 默认设为1, 客户端需要, 15-12-07
    //DWORD getClothColor() const {return m_stBlobData.dwClothColor != 0 ? m_stBlobData.dwClothColor : 1; }

    void addGMCommand(DWORD index, string gm);
    void delGMCommand(DWORD index);
    void getGMCommand(vector<string>& gms) const;

    void addShowNpc(DWORD id, bool bShare = false);
    void delShowNpc(DWORD id);
    const TSetDWORD& getShowNpcs() const { return m_setShowNpc; }

    void addSeeNpcs(DWORD id);
    void delSeeNpcs(DWORD id);
    void addHideNpcs(DWORD id);
    void delHideNpcs(DWORD id);
    bool haveSeeNpcs(DWORD id) const { return m_stBlobData.setSeeNpcs.find(id) != m_stBlobData.setSeeNpcs.end(); }
    bool haveHideNpcs(DWORD id) const { return m_stBlobData.setHideNpcs.find(id) != m_stBlobData.setHideNpcs.end(); }

    void addAction(DWORD id);
    void delAction(DWORD id);
    bool haveAction(DWORD id);

    void addExpression(DWORD id);
    bool haveExpression(DWORD id);
    DWORD getExpressionCount() const { return m_stBlobData.setExpressions.size(); }

    bool addFirstActionDone(EFirstActionType eType, bool ntfClient = true);
    void addClientFirstAction(DWORD dwAction);
    void sendFirstAction();

    void updateTraceItem(const UpdateTraceList& rCmd);

    DWORD getBody(bool bGetReal = false);
    DWORD getRealBody();
    DWORD getBodyScale(bool bGetReal = false);
    DWORD getLefthand(bool bGetReal = false);
    DWORD getRighthand(bool bGetReal = false);

    DWORD getHead(bool bGetReal = false);
    DWORD getBack(bool bGetReal = false);
    DWORD getFace(bool bGetReal = false);
    DWORD getTail(bool bGetReal = false);
    DWORD getMount(bool bGetReal = false);
    DWORD getMouth(bool bGetReal = false);
    DWORD getShowHair() const { return m_oBaseData.hair(); }
    DWORD getShowHairColor() const { return m_oBaseData.haircolor(); }

    void setBlink(bool bBlink);
    bool getBlink() const { return m_stOnlineData.bBlink; }

    void insertPackSkill(DWORD dwSkillID) { m_stOnlineData.setPackSkill.insert(dwSkillID); }
    void erasePackSkill(DWORD dwSkillID) { m_stOnlineData.setPackSkill.erase(dwSkillID); }
    bool isPackSkillLearned(DWORD dwSkillID) const { return m_stOnlineData.setPackSkill.find(dwSkillID) != m_stOnlineData.setPackSkill.end(); }

    // blob data
    bool isNewMap(DWORD mapid);
    bool addNewMap(DWORD mapid);

    void setSaveMap(DWORD mapid);
    DWORD getSaveMap() const { return m_stBlobData.dwSaveMap; }

    void setFollowerID(QWORD qwID, EFollowType eType = EFOLLOWTYPE_MIN);
    void setFollowerIDNoCheck(QWORD qwID, EFollowType eType = EFOLLOWTYPE_MIN);
    QWORD getFollowerID() const { return m_stBlobData.qwFollowerID; }

    void setLastMapID(DWORD dwLastMapID, DWORD dwRealMapID);
    DWORD getLastMapID() const { return m_stBlobData.dwLastMap; }
    DWORD getLastRealMapID() const { return m_stBlobData.dwLastRealMap; }

    DWORD getDeadCoin() const { return m_stBlobData.dwDeadCoin; }
    void setDeadCoin(DWORD dwCount);

    DWORD getDeadLv() const { return m_stBlobData.dwDeadLv; }
    void setDeadLv(DWORD dwLv);

    DWORD getDeadExp() const { return m_stBlobData.dwDeadExp; }
    void setDeadExp(DWORD dwExp);

    friend class UserMap;
  private:
    void setLastSMapIdPos(DWORD mapId, xPos pos) { m_stBlobData.dwLastSMapid = mapId; m_stBlobData.lastSPos = pos; }
    DWORD getLastSMapId() const { return m_stBlobData.dwLastSMapid; }
    const xPos& getLastSPos() const { return m_stBlobData.lastSPos; }

  public:
    void setLevelUpTime(DWORD dwTime);
    DWORD getLevelUpTime() const { return m_stBlobData.dwLevelUpTime; }
    void setDir(DWORD dwDir);
    DWORD getDir() const { return m_stOnlineData.dwDir; }

    void setQueryType(EQueryType eType);
    DWORD getQueryType() const { return m_stOptionData.eType; }
    void setQueryWeddingType(EQueryType eType);
    DWORD getQueryWeddingType() const { return m_stOptionData.eWeddingType; }

    void setFashionHide(DWORD data);
    DWORD getFashionHide() const { return m_stOptionData.m_dwFashionHide; }
    bool isFashionHide(EFashionHideType type) const;

    bool hasPatchVersion(DWORD dwVersion) { return m_stBlobData.setPatchVersion.find(dwVersion) != m_stBlobData.setPatchVersion.end(); }
    bool addPatchVersion(DWORD dwVersion);

    bool hasAccPatchVersion(QWORD qwVersion) { return m_stAccData.setPatchVersion.find(qwVersion) != m_stAccData.setPatchVersion.end(); }
    bool addAccPatchVersion(QWORD qwVersion);

    DWORD getShaderColor();

    void addMapArea(DWORD dwMapID);
    void sendMapAreaList();
    void sendAllActions();
    void sendTraceItemList();

    const TVecRecentZoneInfo& getRecentZones() const { return m_stBlobData.vecRecentZones; }

    void addCredit(DWORD value);
    void decCredit(DWORD value);
    void setCredit(int value);
    void addMonsterCredit(DWORD value);
    int getCredit() const { return m_stBlobData.stCredit.iCurCredit; }

    void setCreditSavedTime(DWORD time) { m_stBlobData.stCredit.dwSavedTime = time; }
    DWORD getCreditSavedTime() const { return m_stBlobData.stCredit.dwSavedTime; }

    void setCreditForbidRCTime(DWORD time) { m_stBlobData.stCredit.dwForbidRecoverTime = time; }
    DWORD getCreditForbidRCTime() const { return m_stBlobData.stCredit.dwForbidRecoverTime; }

    DWORD getAuguryRewardTime() const { return m_stAccData.dwAuguryRewardTime; }
    void setAuguryRewardTime();

    QWORD getMainCharId() const { return m_oBaseData.maincharid(); }
    void setMainCharId(QWORD qwCharId) { m_oBaseData.set_maincharid(qwCharId); }

    DWORD getJoyByType(EJoyActivityType etype);
    bool addJoyValue(EJoyActivityType etype, DWORD value);
    DWORD getTotalJoy();

    DWORD getExtraRewardTimes(EExtraRewardType eType);
    void setExtraRewardTimes(EExtraRewardType eType, DWORD times);

    TMapNpcID2GuildRaidData& getGuildRaids();
    void resetGuildRaidData();
    void newGuildRaidData(const SGuildRaid& rCfg);
    bool canEnterGuildRaid(DWORD gatenpcid);
    bool hasGotGRaidReward(DWORD gatenpcid, DWORD mapindex);
    void markGotGRaidReward(DWORD gatenpcid, DWORD mapindex);
    void clearGuildRaidData();
    DWORD getGRaidKilledBossCnt(DWORD gatenpcid);

    void setTransMap(DWORD mapid, const xPos& pos) { m_stBlobData.dwTransMap = mapid; m_stBlobData.oTransPos = pos; }
    DWORD getTransMap() const { return m_stBlobData.dwTransMap; }
    const xPos& getTransPos() const { return m_stBlobData.oTransPos; }
    /*bool addBlackID(QWORD qwCharID);
    bool removeBlackID(QWORD qwCharID);
    bool checkBlack(QWORD qwCharID) const;*/

    bool addQuestNtf(DWORD dwQuestID);
    bool isQuestNtf(DWORD dwQuestID);
    bool hasQuestNtf() const { return m_oBaseData.questmapntf_size() != 0; }
    void clearQuestNtf() { m_oBaseData.clear_questmapntf(); }
    void setPvpCoin(DWORD coinnum);
    DWORD getPvpCoin() const { return m_stBlobData.dwPvpCoin; }

    /*void setCon(DWORD value);
    DWORD getCon() const { return m_stBlobData.dwCon; }
    void setConInit() { m_stBlobData.bConInit = true; }
    bool isConInit() const { return m_stBlobData.bConInit; }*/

    void setChargeLottery(QWORD v) { m_stBlobData.qwChargeLottery = v; }
    QWORD getChargeLottery() { return m_stBlobData.qwChargeLottery; }
    void setChargeZeny(QWORD v) { m_stBlobData.qwChargeZeny = v; }
    QWORD getChargeZeny() { return m_stBlobData.qwChargeZeny; }

    void setDailyNormalZeny(QWORD v) { m_stBlobData.qwDailyNormalZeny = v; }
    QWORD getDailyNormalZeny() { return m_stBlobData.qwDailyNormalZeny; }
    void setDailyChargeZeny(QWORD v) { m_stBlobData.qwDailyChargeZeny = v; }
    QWORD getDailyChargeZeny() { return m_stBlobData.qwDailyChargeZeny; }
    
    void setLotteryCoin(DWORD newNum);
    DWORD getLotteryCoin() const { return m_stBlobData.dwLotteryCoin; }

    DWORD getMaxBaseLv();
    void updateMaxBaseLv(DWORD oldLv);

    void setRenameTime(DWORD time) { m_stBlobData.dwRenameTime = time; }
    DWORD getRenameTime() { return m_stBlobData.dwRenameTime; }

    void setGuildHonor(DWORD num);
    DWORD getGuildHonor() const { return m_stBlobData.dwGuildHonor; }

    void notifyActivityEventToClient(EAERewardMode mode = EAEREWARDMODE_MIN);
    void resetAEReward(bool ntf = true);
    DWORD getAERewardDayCount(EAERewardMode mode);
    void addAERewardDayCount(EAERewardMode mode, DWORD cnt);
    QWORD getAERewardAccLimitCharID(EAERewardMode mode);
    void setAERewardAccLimitCharID(EAERewardMode mode, QWORD charid);
    DWORD getAERewardMulDayCount(EAERewardMode mode);
    void addAERewardMulDayCount(EAERewardMode mode, DWORD cnt);
    QWORD getAERewardMulAccLimitCharID(EAERewardMode mode);
    void setAERewardMulAccLimitCharID(EAERewardMode mode, QWORD charid);

    const list<PhotoMd5>& getPhotoMd5List() const { return m_stAccData.listPhotoMd5; }
    void queryPhotoMd5List();
    bool addPhotoMd5(const PhotoMd5& md5);
    bool removePhotoMd5(const PhotoMd5& md5);
    bool getDivorceRollerCoaster() { return m_stBlobData.bDivorceRollerCoaster; }
    void setDivorceRollerCoaster(bool b);

    DWORD getActivityEventCnt(EActivityEventType eventType, QWORD id, EUserType userType);
    void setActivityEventCnt(EActivityEventType eventType, QWORD id, EUserType userType, DWORD count);

    void setCurMaxBaseLv(DWORD maxbaselv) { m_stAccData.setCurMaxBaseLv(maxbaselv); }
    DWORD getLastOfflineTime() { return m_stBlobData.dwLastOfflineTime; }
    void setLastOfflineTime(DWORD dwTime) { m_stBlobData.dwLastOfflineTime = dwTime; }
    DWORD getLastBaseLv() { return m_stBlobData.dwLastBaseLv; }
    void setLastBaseLv(DWORD dwBaseLv) { m_stBlobData.dwLastBaseLv = dwBaseLv; }
    DWORD getLastJobLv() { return m_stBlobData.dwLastJobLv; }
    void setLastJobLv(DWORD dwJobLv) { m_stBlobData.dwLastJobLv = dwJobLv; }
    void notifyDeadBoss();
  private:
    void initDefaultData();
    //void resetAccountStatus();

    void refreshDebt();
    void refreshPhotoMd5();
  private:
    SceneUser* m_pUser = nullptr;

    UserAccData m_oAccData;
    UserBaseData m_oBaseData;
    BlobBoss m_oBoss;

    SUserBlobData m_stBlobData;
    SUserAccData m_stAccData;
    SUserOnlineData m_stOnlineData;
    SUserOption m_stOptionData;

    TSetDWORD m_setShowNpc;

    bool m_bRaid = false;
    std::pair<DWORD, DWORD> m_pairClothColor;
};


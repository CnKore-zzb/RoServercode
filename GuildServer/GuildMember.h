#pragma once

#include <bitset>
#include "xDefine.h"
#include "xEntry.h"
#include "GuildCmd.pb.h"
#include "SocialCmd.pb.h"
#include "GCharManager.h"
#include "GuildConfig.h"
#include "Var.h"
#include "CommonConfig.h"
#include "GuildBuilding.h"

using std::string;
using std::vector;
using std::set;
using std::bitset;
using std::list;
using namespace Cmd;

class Guild;

// guild member
typedef vector<GuildMemberPray> TVecMemberPray;
typedef vector<DonateItem> TVecMemberDonateItem;
typedef list<GuildPhoto> TListMemberPhoto;
typedef map<EGuildBuilding, UserGuildBuilding> TMapUserGuildBuilding;

// GMember
class GMember
{
  friend class Guild;
  friend class GuildManager;
  public:
    GMember(Guild* guild, const SocialUser& rUser, EGuildJob eJob);
    virtual ~GMember();

    void fromData(const GuildMember& rData);
    void toData(GuildMember* pData, bool bClient = false);

    void fromData(const GuildApply& rData);
    void toData(GuildApply* pData);

    bool fromPrayData(const string& str);
    bool toPrayData(string& str);

    bool fromDonateData(const string& str);
    bool toDonateData(string& str);

    bool fromVarData(const string& str);
    bool toVarData(string& str);

    bool fromBuildingData(const string& str);
    bool toBuildingData(string& str);

    void toScenePrayData(GuildUserInfoSyncGuildCmd& cmd);
    void toData(GuildSMember* pMember);

    void updateData(const UserInfo& rInfo, bool bInit = false);

    QWORD getAccid() const { return m_oGCharData.getAccID(); }
    QWORD getCharID() const { return m_oGCharData.getCharID(); }

    void setZoneID(DWORD dwZoneID)
    {
      if (m_oGCharData.getZoneID() != dwZoneID)
      {
        m_oGCharData.setZoneID(dwZoneID); 
        m_bitset.set(EGUILDMEMBERDATA_ZONEID);
        updateData();
      }
    }
    DWORD getZoneID() const { return m_oGCharData.getZoneID(); }

    DWORD getBaseLevel() const { return m_oGCharData.getBaseLevel(); }
    DWORD getPortrait() const { return m_oGCharData.getPortrait(); }
    DWORD getFrame() const { return m_oGCharData.getFrame(); }
    DWORD getHair() const { return m_oGCharData.getHair(); }
    DWORD getHairColor() const { return m_oGCharData.getHairColor(); }
    DWORD getBody() const { return m_oGCharData.getBody(); }
    DWORD getHead() const { return m_oGCharData.getHead(); }
    DWORD getFace() const { return m_oGCharData.getFace(); }
    DWORD getMouth() const { return m_oGCharData.getMouth(); }
    DWORD getEye() const { return m_oGCharData.getEye(); }

    DWORD getWeekContribution();

    DWORD getContribution() const { return m_dwCon; }
    void setContribution(DWORD c, bool bNtf = true, bool bAddRecord = true);

    DWORD getWeekAsset();
    DWORD getAsset() const { return m_dwAsset; }
    void setAsset(DWORD a);

    DWORD getTotalContribution() const { return m_dwTotalCon; }
    void setTotalContribution(DWORD c) { m_dwTotalCon = c; m_bitset.set(EGUILDMEMBERDATA_TOTALCONTRIBUTION); updateData(); }

    DWORD getEnterTime() const { return m_dwEnterTime; }

    DWORD getOnlineTime() const { return m_oGCharData.getOnlineTime(); }
    void setOnlineTime(DWORD dwTime) { m_oGCharData.setOnlineTime(dwTime); m_bitset.set(EGUILDMEMBERDATA_ONLINETIME); updateData(); }
    DWORD getOfflineTime() const { return m_oGCharData.getOfflineTime(); }
    void setOfflineTime(DWORD dwTime) { m_oGCharData.setOfflineTime(dwTime); m_bitset.set(EGUILDMEMBERDATA_OFFLINETIME); updateData(); }
    void resetOfflineTime(DWORD dwTime) { m_oGCharData.setOfflineTime(dwTime); }

    EGender getGender() const { return m_oGCharData.getGender(); }
    void setGender(EGender eGender) { m_oGCharData.setGender(eGender); updateData(); }

    EProfession getProfession() const { return m_oGCharData.getProfession(); }

    EGuildJob getJob() const { return m_eJob; }
    void setJob(EGuildJob eJob);

    void setName(const string& name) { m_oGCharData.setName(name); m_bitset.set(EGUILDMEMBERDATA_NAME); updateData(); }
    const char* getName() const { return m_oGCharData.getName(); }

    QWORD getGuildID() const { return m_oGCharData.getGuildID(); }

    bool sendCmdToMe(const void* buf, WORD len);
    bool sendCmdToZone(const void* buf, WORD len);

    DWORD getPrayLv(DWORD dwPrayID) const;
    void setPrayLv(DWORD dwPrayID, DWORD dwLv);
    bool hasPray() const { return m_vecPrays.empty() == false; }

    DWORD getGiftPoint() const { return m_dwGiftPoint; }
    void setGiftPoint(DWORD dwPoint);

    bool getLevelupEffect() const { return m_bLevelupEffect; }
    void setLevelupEffect(bool bEffect);

    DonateItem* getDonateItem(DWORD dwConfigID, DWORD dwTime);
    void collectDonateList(DonateListGuildCmd& cmd);
    //void collectDonateStep(TSetDWORD& setStep);
    bool addDonateItem(DWORD dwConfigID, DWORD dwTime, DWORD dwItemID, DWORD dwItemCount, DWORD donateCount = 0);
    void refreshDonate();

    void setDonateFrame(bool bDonate) { m_bDonate = bDonate; }
    bool getDonateFrame() const { return m_bDonate; }

    bool isOnline() const { return m_oGCharData.getOfflineTime() == 0; }

    void setExchangeZone(bool b) { m_bExchangeZone = b; }
    bool getExchangeZone() const { return m_bExchangeZone; }

    void updateData(bool bNoCache = false);
    void fetchData(const bitset<EGUILDMEMBERDATA_MAX>& bit, GuildMemberDataUpdateGuildCmd& cmd, GuildMemberDataUpdateGuildSCmd& scmd, bool& bNeedSave);

    void setAuthorize(bool ignore) { m_ignorePwd = ignore; }
    bool getAuthorize() { return m_ignorePwd; }

    bool updateDonateItem(DWORD nextid, EDonateType eType, DWORD time, DWORD configid);
    DWORD getDonateItemCount();
    void removeDonateItem(UpdateDonateItemGuildCmd& cmd);
    void refreshDonateInOrder();
    DWORD getAssetByID(DWORD dwConfigID);
    void resetDonateTime(DWORD time);
    void cleatDonateItem() { m_vecDonateItems.clear(); }

    DWORD getMedalByID(DWORD dwConfigID);

    void setFrameStatus(bool bFrame, bool bOffline = false);
    bool getFrameStatus() const { return CommonConfig::m_bGuildOptOpen ? m_bFrameStatus : true; }

    void setGuildMark(EGuildData eData) { m_guildbitset.set(eData); }
    void addPackUpdate(const string& guid) { m_setPackUpdate.insert(guid); }

    void addMemberUpdate(QWORD qwCharID) { m_setMemberUpdate.insert(qwCharID); }
    void setMemberMark(QWORD qwCharID, EGuildMemberData eData) { m_mapMemberBitset[qwCharID].set(eData); }

    void addApplyUpdate(QWORD qwCharID) { m_setApplyUpdate.insert(qwCharID); }
    void setApplyMark(QWORD qwCharID, EGuildMemberData eData) { m_mapApplyBitset[qwCharID].set(eData); }

    void setRedTip(bool bRed, bool sync = true);
    void redTipMessage(ERedSys eRedSys, bool isAdd = true);

    bool hasRedTip() { return m_bHasRedTip; }

    void redTipMessage();
    bool isRedTipSet(ERedSys type) { return m_qwRedTip & (QWORD(1) << DWORD(type)); }
    void setRedTip(ERedSys type, bool add);
    void syncRedTipToScene(ERedSys type);

    bool canSubmitMaterial(EGuildBuilding type, DWORD count);
    void addSubmitCount(EGuildBuilding type, DWORD count);
    void resetSubmitCount(bool force = false);
    void resetSubmitCountTotal(EGuildBuilding type);
    DWORD getSubmitCount(EGuildBuilding type);
    void sendSubmitCountToMe(EGuildBuilding type);
    const UserGuildBuilding* getBuildingData(EGuildBuilding type);

    void clearVarWhenRemovedFromGuild();
    void clearBuildingDataWhenExitGuild();
    void buildingLevelUpEffect(const set<EGuildBuilding>& types, bool add);
    bool canGetBuildingWelfare(EGuildBuilding type);
    void setBuildingNextWelfareTime(EGuildBuilding type, DWORD time);
    void setBuildingEffect();
    void resetBuildingEffect();

    DWORD getLastExitTime() { return m_dwLastExitTime; } // 上一次退出公会的时间
    void setLastExitTime(DWORD time);

    // 退出公会的那一周不参与挑战计数, 也不可领取挑战奖励
    bool canChallenge() { return m_dwLastExitTime <= 0 || xTime::getWeekStart(m_dwLastExitTime, 5 * HOUR_T) != xTime::getWeekStart(now(), 5 * HOUR_T); }
    bool canGetChallengeWelfare(DWORD createtime) { return canChallenge() && (m_dwLastExitTime <= 0 || xTime::getWeekStart(m_dwLastExitTime, 5 * HOUR_T) < xTime::getWeekStart(createtime, 5 * HOUR_T)); }
    void setChallengeFinished(DWORD index);
    bool isChallengeFinished(DWORD index) { return (m_qwChallenge & QWORD(1) << index) != 0; }
    void resetChallenge();
    bool hasWelfare(EGuildWelfare type, QWORD id = 0);

    void addTotalBCoin(DWORD dwCount);
    DWORD getTotalBCoin() const { return m_dwTotalBCoin; }
    DWORD getWeekBCoin();

    bool openRealtimeVoice(bool open);
    bool isRealtimeVoiceOpen() { return m_bRealtimeVoice; }
    void sendRealtimeVoiceID();
    //void patch_checkLvOverflow();
  private:
    void resetWeek();
    void refreshDonate(DWORD& dwTime, DWORD curTime, EDonateType eType);
  private:
    GCharReader m_oGCharData;

    DWORD m_dwWeekCon = 0;
    DWORD m_dwCon = 0;
    DWORD m_dwTotalCon = 0;
    DWORD m_dwEnterTime = 0;
    DWORD m_dwGiftPoint = 0;
    DWORD m_dwWeekAsset = 0;
    DWORD m_dwAsset = 0;
    DWORD m_dwExchangeZone = 0;
    DWORD m_dwDonateTime1 = 0;
    DWORD m_dwDonateTime2 = 0;
    DWORD m_dwDonateTime3 = 0;
    DWORD m_dwDonateTime4 = 0;
    DWORD m_dwTotalBCoin = 0;
    DWORD m_dwWeekBCoin = 0;

    EGuildJob m_eJob = EGUILDJOB_MIN;

    bool m_bLevelupEffect = false;
    bool m_bExchangeZone = false;
    bool m_bDonate = false;
    bool m_ignorePwd = true;
    bool m_bFrameStatus = false;

    TVecMemberPray m_vecPrays;
    TVecMemberDonateItem m_vecDonateItems;

    Variable m_oVar;

    Guild* m_pGuild = nullptr;
    bitset<EGUILDMEMBERDATA_MAX> m_bitset;
    bitset<EGUILDDATA_MAX> m_guildbitset;

    TSetString m_setPackUpdate;

    TSetQWORD m_setMemberUpdate;
    map<QWORD, bitset<EGUILDMEMBERDATA_MAX>> m_mapMemberBitset;

    TSetQWORD m_setApplyUpdate;
    map<QWORD, bitset<EGUILDMEMBERDATA_MAX>> m_mapApplyBitset;

    bool m_bHasRedTip = false;

    TMapUserGuildBuilding m_mapGuildBuilding;
    QWORD m_qwChallenge = 0;
    DWORD m_dwLastExitTime = 0;
    QWORD m_qwRedTip = 0;
    bool m_bBuildingEffect = false;
    bool m_bRealtimeVoice = false;
};

typedef vector<GMember*> TVecGuildMember;

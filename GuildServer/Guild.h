/**
 * @file Guild.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-12
 */

#pragma once

#include <bitset>
#include "xDefine.h"
#include "xEntry.h"
#include "GuildConfig.h"
#include "GuildCmd.pb.h"
#include "MsgManager.h"
#include "SocialCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "TableManager.h"
#include "GuildMisc.h"
#include "GuildEvent.h"
#include "GuildPack.h"
#include "GuildMember.h"
#include "GuildPhoto.h"

using std::string;
using std::vector;
using std::set;
using std::bitset;
using namespace Cmd;

struct SGuildBase;

class xRecord;
class Guild;

#ifdef _DEBUG
const DWORD GUILD_RECORD_TICK_TIME = 10;
#else
const DWORD GUILD_RECORD_TICK_TIME = 600;
#endif

// guild blob
enum EGuildBlob
{
  EGUILDBLOB_MIN = 0,
  EGUILDBLOB_MISC = 1,
  EGUILDBLOB_PACK = 2,
  EGUILDBLOB_EVENT = 3,
  EGUILDBLOB_MAX = 4,
};

// guild
enum EApplyMethod
{
  EAPPLYMETHOD_MIN = 0,
  EAPPLYMETHOD_TIME,
};
enum EExchangeMethod
{
  EEXCHANGEMETHOD_MIN = 0,
  EEXCHANGEMETHOD_FIND,
};

class Guild : public xEntry
{
  friend class GuildManager;
  public:
    Guild(QWORD guid, DWORD dwZoneID, const string& name);
    virtual ~Guild();

    QWORD getGUID() const { return m_qwGuildID; }
    void setGUID(QWORD guid) { m_qwGuildID = guid; set_id(guid); }
    void setZoneID(DWORD dwZoneID) { m_dwZoneID = dwZoneID; setMark(EGUILDDATA_ZONEID); updateData(true); }
    DWORD getZoneID() const { return m_dwZoneID; }
    void setMark(EGuildData eType) { if (EGuildData_IsValid(eType) == false) return; m_bitset.set(eType); m_recordbitset.set(eType); }

    bool toMemberString(string& str);
    bool toApplyString(string& str);

    void toSummaryData(GuildSummaryData* pData);
    void toData(GuildData* pData, bool bChairman = true, GMember* member = nullptr);
    void toData(GuildInfo* pInfo, const GuildJob& rJob = GuildJob());
    void toData(GuildArtifactQuest* pQuest);
    bool toRecord(xRecord& record);

    void reload(ConfigType type);
    void dismiss();

    void setGuildCFG(const SGuildCFG* pCFG) { m_pGuildCFG = pCFG; }
  public:
    const SGuildCFG* getGuildCFG() const { return m_pGuildCFG; }

    void broadcastMsg(DWORD dwMsgID, MsgParams oParams = MsgParams(), EMessageType eType = EMESSAGETYPE_FRAME, DWORD dwDelay = 0);
    void broadcastNpcMsg(DWORD dwNpcID, DWORD dwMsgID, MsgParams oParams = MsgParams());
    void broadcastCmd(const void* buf, WORD len, bool bLeader = false);

    const TVecGuildMember& getMemberList() const { return m_vecMembers; }
    TVecGuildMember& getAllMemberList() { return m_vecMembers; }
    DWORD getActiveMemberCount() const;
    DWORD getApplyCount() const { return static_cast<DWORD>(m_vecApplys.size()); }
  private:
    DWORD getMemberCount() const { return static_cast<DWORD>(m_vecMembers.size()); }
    DWORD getMaxViceCount() const;
    DWORD getViceCount() const;

    bool hasOnlineMember() const;

    bool addMember(const SocialUser& rUser, EGuildJob eJob, bool bBack);
    bool removeMember(QWORD qwCharID);
  public:
    GMember* getMember(QWORD qwCharID, QWORD qwAccID = 0);
    GMember* getChairman();
  private:
    bool addApply(const UserInfo& rUser);
    bool removeApply(QWORD qwCharID, EApplyMethod eMethod = EAPPLYMETHOD_MIN);
    bool sendApplyList(QWORD qwCharID);
  public:
    GMember* getApply(QWORD qwCharID);

  private:
    bool addInvite(QWORD qwCharID);
    bool removeInvite(QWORD qwCharID);
    GMember* getInvite(QWORD qwCharID);

    bool exchangeChairman(QWORD qwOldChair, QWORD qwNewChair, EExchangeMethod eMethod = EEXCHANGEMETHOD_MIN);
  public:
    bool levelup(QWORD qwCharID, DWORD addlevel = 1);
  private:
    void maintenance(DWORD curTime);
  public:
    void syncInfoToScene(const GMember& rMember, bool bCreate = false);
    void broadcastGuildInfoToScene();
  public:
    void setLevel(DWORD dwLevel) { m_dwLevel = dwLevel; setMark(EGUILDDATA_LEVEL); updateData(); }
    DWORD getLevel() const { return m_dwLevel; }

    DWORD getQuestTime() const { return m_dwQuestTime; }

    bool addAsset(DWORD dwCount, bool bNoCheck = false, ESource eSource = ESOURCE_MAX);
    bool subAsset(DWORD dwCount, ESource eSource);
    DWORD getAsset() const { return m_dwAsset; }

    void setDismissTime(DWORD dwDismiss);
    DWORD getDismissTime() const { return m_dwDismissTime; }
    DWORD getDismissLastTime();

    void setZoneTime(DWORD dwZone) { m_dwZoneTime = dwZone; setMark(EGUILDDATA_ZONETIME); updateData(); }
    DWORD getZoneTime() const { return m_dwZoneTime; }

    void setNextZone(DWORD dwZone) { m_dwNextZone = dwZone; setMark(EGUILDDATA_NEXTZONE); updateData(); }
    DWORD getNextZone() const { return m_dwNextZone; }

    void setRenameTime(DWORD time) { m_oMisc.setRenameTime(time); }
    DWORD getRenameTime() { return m_oMisc.getRenameTime(); }
  public:
    void clearNextZone();
    const string& getName() const { return m_strName; }
  private:
    void setBoard(QWORD qwCharID, DWORD dwZoneID, const string& board);
    const string& getBoard() const { return m_strBoard; }

    void setRecruit(const string& recruit) { m_strRecruit = recruit; setMark(EGUILDDATA_RECRUITINFO); updateData(); }
    const string& getRecruit() const { return m_strRecruit; }

    void setPortrait(const string& portrait) { m_strPortrait = portrait; setMark(EGUILDDATA_PORTRAIT); updateData(); }
  public:
    const string& getPortrait() const { return m_strPortrait; }
  private:
    void setJobName(EGuildJob eJob, const string& name);
  public:
    const string& getJobName(EGuildJob eJob);
    void refreshDonate(DWORD& dwTime, DWORD curTime, EDonateType eType, QWORD qwUserId = 0);

  private:
    void timer(DWORD curTime);
  private:
    void updateMember();
    void updateApply();
  public:
    void fetchMember(const TSetQWORD& setIDs, GuildMemberUpdateGuildCmd& cmd, GuildMemberUpdateGuildSCmd& scmd);
    void fetchApply(const TSetQWORD& setIDs, GuildMemberUpdateGuildCmd& cmd, GuildMemberUpdateGuildSCmd& scmd);
  public:
    void updateData(bool bNoCache = false);
    void fetchData(const bitset<EGUILDDATA_MAX>& bit, GuildDataUpdateGuildCmd& cmd, GuildDataUpdateGuildSCmd& scmd, CityDataUpdateGuildSCmd& ccmd);
  private:
    void updateRecord(DWORD curTime);
    void checkChairman(DWORD curTime);
    void updateVar(DWORD curTime);
    void updateExchangeZone(DWORD curTime);
  private:
    QWORD m_qwGuildID = 0;

    DWORD m_dwZoneID = 0;
    DWORD m_dwLevel = 0;

    DWORD m_dwQuestTime = 0;
    DWORD m_dwAsset = 0;
    DWORD m_dwDismissTime = 0;
    DWORD m_dwCreateTime = 0;
    DWORD m_dwNextZone = 0;
    DWORD m_dwZoneTime = 0;

    DWORD m_dwVarTick = 0;
    DWORD m_dwRecordTick = 0;
    DWORD m_dwChairTick = 0;

    string m_strName;
    string m_strBoard;
    string m_strRecruit;
    string m_strPortrait;

    TVecGuildMember m_vecMembers;
    TVecGuildMember m_vecApplys;
    TVecGuildMember m_vecInvites;

    bitset<EGUILDDATA_MAX> m_bitset;
    bitset<EGUILDDATA_MAX> m_recordbitset;

    TSetQWORD m_setMemberUpdate;
    TSetQWORD m_setApplyUpdate;

    const SGuildCFG* m_pGuildCFG = nullptr;
    // 包裹
  public:
    bool toBlobPackString(string& str) { return m_oPack.toBlobPackString(str); }
    void setBlobPackString(const char* str, DWORD len) { return m_oPack.setBlobPackString(str, len); }
    GuildPack& getPack() { m_oPack.init(); return m_oPack; }
  private:
    GuildPack m_oPack;

    // 工会事件
  public:
    bool toBlobEventString(string& str) { return m_oEvent.toBlobEventString(str); }
    void setBlobEventString(const char* str, DWORD len) { return m_oEvent.setBlobEventString(str, len); }
    GuildEventM& getEvent() { m_oEvent.init(); return m_oEvent; }
  private:
    GuildEventM m_oEvent;

    // 杂项
  public:
    bool toBlobMiscString(string& str) { return m_oMisc.toMiscString(str); }
    void setBlobMiscString(const char* str, DWORD len) { return m_oMisc.setBlobMiscString(str, len); }
    GuildMisc& getMisc() { m_oMisc.init(); return m_oMisc; }
  private:
    GuildMisc m_oMisc;

    // 公会照片
  public:
    bool toBlobPhotoString(string& str) { return m_oPhotoMgr.toBlobPhotoString(str); }
    void setBlobPhotoString(const char* str, DWORD len) { m_oPhotoMgr.setBlobPhotoString(str, len); }
    GuildPhotoMgr& getPhoto() { m_oPhotoMgr.init(); return m_oPhotoMgr; }
  private:
    GuildPhotoMgr m_oPhotoMgr;

  public:
    void processRedTip(bool addMember);

    void rename(const string& str);

  public:
    const string& getRealtimeVoiceID();
    bool canAddRealtimeVoice();
    bool openRealtimeVoice(QWORD operatorid, QWORD memberid, bool open);
    DWORD getOpenedRealtimeVoiceCount();
  private:
    string m_strRealtimeVoiceID;
};

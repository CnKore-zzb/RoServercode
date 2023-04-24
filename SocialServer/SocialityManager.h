/**
 * @file SocialityManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-02-22
 */

#pragma once

#include "UserSociality.h"

typedef map<QWORD, UserSociality> TMapSocialData;
typedef map<QWORD, SocialUser> TMapLoadList;

// social manager
class SocialityManager : public xSingleton<SocialityManager>
{
  friend class xSingleton<SocialityManager>;
  private:
    SocialityManager();
  public:
    virtual ~SocialityManager();

  public:
    void final();
    void delChar(QWORD qwCharID);

    void onUserOnline(const SocialUser& rUser);
    void onUserOffline(const SocialUser& rUser);

    void updateUserInfo(const UserInfoSyncSocialCmd& cmd);

    bool frameStatus(QWORD qwGUID, bool bFrame);

    bool addRelation(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bCheck = true);
    bool removeRelation(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bCheck = true);
    bool checkRelation(QWORD qwCharID, QWORD qwDestCharID, ESocialRelation eRelation);

    bool updateSocialData(QWORD qwGUID, ESocialData eType, DWORD dwValue, const string& strName);
    bool updateSocialData(QWORD qwGUID, const TSetQWORD& setUpdateIDs, ESocialData eType, DWORD dwValue, const string& strName);
    bool findUser(QWORD id, const string& keyword);

    bool recallFriend(DWORD zoneid, QWORD charid, QWORD targetid);

    void setRecallActivityOpen(bool bOpen);
    bool getRecallActivityOpen() { return m_bRecallOpen; }

    bool doUserCmd(const SocialUser& rUser, const BYTE* buf, WORD len);

    UserSociality* getUserSociality(QWORD qwCharID);

    void addLoad(const SocialUser& rUser) { m_mapLoadList[rUser.charid()].CopyFrom(rUser); }
    void removeLoad(QWORD qwCharID) { m_mapLoadList.erase(qwCharID); }

    void addRequest(const SocialUser& rUser) { m_mapReqList[rUser.charid()].CopyFrom(rUser); }
    void removeRequest(QWORD qwCharID) { m_mapReqList.erase(qwCharID); }

    void timer(DWORD dwCurTime);
    void loadtimer(DWORD curTime);

    void SyncRedTip(QWORD dwid, QWORD charid, ERedSys redsys, bool opt);
  private:
    bool addTutor(QWORD qwGUID, QWORD qwDestGUID, ESocialRelation eRelation, bool bConfim = true);
  private:
    void processLoad();
    void processReq();

    bool isLoad(QWORD qwCharID) { return m_mapLoadList.find(qwCharID) != m_mapLoadList.end(); }
    bool isReq(QWORD qwCharID) { return m_mapReqList.find(qwCharID) != m_mapReqList.end(); }

    bool loadSocialData(QWORD qwGUID);
  private:
    TMapSocialData m_mapSocialData;

    TMapLoadList m_mapLoadList;
    TMapLoadList m_mapReqList;

    bool m_bRecallOpen = false;
};


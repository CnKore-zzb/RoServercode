/**
 * @file UserSociality.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-02-22
 */

#pragma once

#include "Sociality.h"

typedef map<QWORD, Sociality> TMapSociality;
typedef vector<Sociality*> TVecSociality;
typedef map<ESocialRelation, TVecSociality> TMapRelationList;

enum ESRMethod
{
  ESRMETHOD_MIN = 0,
  ESRMETHOD_TIME_MIN = 1,
  ESRMETHOD_ALL = 2,
  ESRMETHOD_CONTRACT_OVERTIME = 3,
  ESRMETHOD_MAX = 4,
};
class UserSociality
{
  friend class SocialityManager;
  public:
    UserSociality();
    ~UserSociality();

    bool loadRecall();

    void queryRecallList();
    void querySocialNtfList();

    void setZoneID(DWORD dwZoneID) { m_oGCharData.setZoneID(dwZoneID); }
    QWORD getZoneID() const { return m_oGCharData.getZoneID(); }

    void setGUID(QWORD qwGUID) { m_oGCharData.setCharID(qwGUID); }
    QWORD getGUID() const { return m_oGCharData.getCharID(); }

    void setOfflineTime(DWORD t) { m_oGCharData.setOfflineTime(t); }
    QWORD getOfflineTime() const { return m_oGCharData.getOfflineTime(); }

    void setBaseLevel(DWORD lv) { m_oGCharData.setBaseLevel(lv); }
    DWORD getBaseLevel() const { return m_oGCharData.getBaseLevel(); }

    void setName(const string& name) { m_oGCharData.setName(name); }
    const string getName() const { return m_oGCharData.getName(); }

    GCharReader& getGCharData() { return m_oGCharData; }

    bool syncSocialList();
    bool querySocialList();

    bool addRelation(QWORD qwGUID, ESocialRelation eRelation, bool bCheck = true);
    bool removeRelation(QWORD qwGUID, ESocialRelation eRelation, bool bCheck = true);
    bool removeRelation(ESocialRelation eRelation, ESRMethod eMethod);

    //bool addFocus(QWORD qwGUID);
    //bool removeFocus(QWORD qwGUID);

    Sociality* getSociality(QWORD qwGUID);//, bool bFoucsInclude = true);
    DWORD getSocialityCount(ESocialRelation eRelation) const;

    void collectID(TSetQWORD& setID, ESocialRelation eRelation = ESOCIALRELATION_MIN);
    void addUpdateID(QWORD qwID) { m_setUpdateIDs.insert(qwID); }
    void addSaveID(QWORD qwID) { m_setSaveIDs.insert(qwID); }
    void clearUpdateIDs() { m_setUpdateIDs.clear(); }

    bool getFrame() const { return m_bFrameOpen; }
    void setFrame(bool b) { m_bFrameOpen = b; if (m_bFrameOpen) reqUpdate(); }
    void clearUpdate();

    void update();
    void save(DWORD curTime, bool bRecallOpen);
    void updateRedisAndSync();

    const TMapSociality& getSocialityList() const { return m_mapSocial; }
    //const TMapSociality& getForusList() const { return m_mapFocus; }

    bool getAuthorize() { return m_ignorepwd; }
    void setAuthorize(bool ignore) { m_ignorepwd = ignore; }

    bool isRecall(QWORD qwCharID) const { return m_setRecallIDs.find(qwCharID) != m_setRecallIDs.end(); }
    bool isBeRecall(QWORD qwCharID) const { return m_setBeRecallIDs.find(qwCharID) != m_setBeRecallIDs.end(); }
    void addRecall(QWORD qwCharID) { if (isRecall(qwCharID) == false) {m_setRecallIDs.insert(qwCharID); m_setRecallUpdateIDs.insert(qwCharID);} }

    void setTutorMatch(bool b) { m_bTutorMatch = b; }
  private:
    bool canBeRecall(Sociality& rSocial);
    bool addSociality(const Sociality& rSocial);
  public:
    bool delSociality(QWORD qwCharID);
  private:
    bool createSocialData(QWORD qwGUID, Sociality& rData);
    bool reqUpdate();
    void refreshBlackStatus();

    void checkRecallList();
    void updateRecall();
  private:
    GCharReader m_oGCharData;

    TMapSociality m_mapSocial;
    //TMapSociality m_mapFocus;

    TSetQWORD m_setUpdateIDs;
    TSetQWORD m_setSaveIDs;

    TSetQWORD m_setRecallIDs;
    TSetQWORD m_setBeRecallIDs;

    TSetQWORD m_setRecallUpdateIDs;
    TSetQWORD m_setBeRecallUpdateIDs;

    DWORD m_dwTickTime = 0;

    bool m_bFrameOpen = false;
    bool m_ignorepwd = true;
    bool m_bTutorMatch = false;
};


/**
 * @file RecordUserManager.h
 * @brief User Record Manager
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-04-23
 */

#pragma once

#include "RecordUser.h"

class RecordUserManager : public xSingleton<RecordUserManager>
{
  friend class xSingleton<RecordUserManager>;
  private:
    RecordUserManager();
  public:
    virtual ~RecordUserManager();

    void final();

    RecordUser* getData(USER_ID id, USER_ID accid = 0);
    RecordUser* getDataNoQuery(USER_ID id, USER_ID accid = 0);

    bool delData(QWORD accid, USER_ID charid);
    void timetick(DWORD curTime, bool bTimeOut = false);
    void delChar(USER_ID id,USER_ID accid);

    void patch();
    void patch1();
  private:
    bool loadData(USER_ID id, USER_ID accid);
  private:
    TMapRecordUser m_mapID2User;
};

/*// user data
struct SRecordUserData
{
  UserBaseData oBase;
  RedisUserData oRedis;

  string oBlobTemp;

  string oBlob;
  string oCredit;
  string oAccQuest;
  string oStore;

  UnregType eType;
  bitset<EUSERDATATYPE_MAX> mark;

  DWORD timeTick = 0;
  DWORD m_dwSequence = 0;

  SRecordUserData() : eType(UnregType::Null), timeTick(0) {}

  bool saveData(bool redis = true);
  bool saveRedis();
  bool updateRedis(EUserDataType eType);
  bool updateData(const UserDataRecordCmd& rCmd);

  bool loadStore();
  bool saveCommonData();
};
typedef std::map<QWORD, SRecordUserData> TMapRecordUserData;*/

// RecordUserManager
    //TMapRecordUserData m_mapID2UserData;
    //SRecordUserData* getData(USER_ID id, USER_ID accid = 0, UnregType eType = UnregType::ChangeScene);
    //SRecordUserData* getDataNoQuery(USER_ID accid);

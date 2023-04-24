/**
 * @file RecordUser.h
 * @brief User Record
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-11-13
 */

#pragma once

#include <bitset>
#include "xSingleton.h"
#include "RecordCmd.pb.h"
#include "UserData.h"
#include "xEntry.h"

using std::bitset;
using std::string;
using namespace Cmd;

const DWORD USER_SAVE_TICK = 300;

// record user
struct SExchangeUser
{
  UserBaseData oBase;
  BlobData oData;
};
typedef std::map<QWORD, SExchangeUser> TMapExchangeUser;

enum class ROLLBACK_TYPE
{
  accbase   = 1,
  charbase  = 2,
};

class RecordUser : public xEntry
{
  public:
    RecordUser(QWORD qwAccID, QWORD qwCharID);
    virtual ~RecordUser();

    void toData(RecordUserData& rData);
    //void toData(BlobAccData& rData);

    QWORD accid() const { return m_oBase.accid(); }
    QWORD charid() const { return m_oBase.charid(); }
    DWORD onlinetime() const { return m_oBase.onlinetime(); }
    DWORD mapid() const { return m_oBase.mapid(); }
    EProfession profession() const { return m_oBase.profession(); }
    const string& name() const { return m_oBase.name(); }
    UserBaseData& base() { return m_oBase; }

    DWORD offlinetime() const { return m_oBase.offlinetime(); }
    void setOfflineTime(DWORD dwTime) { m_oBase.set_offlinetime(dwTime); }

    DWORD timetick() const { return m_dwTimeTick; }
    void setTimeTick(DWORD dwTime) { m_dwTimeTick = dwTime; }

    bool loadData();
    //bool loadStore();
    bool saveData(bool isOffline=false);
    bool saveRedis(EUserDataType eType = EUSERDATATYPE_MIN);

    bool updateData(const UserDataRecordCmd& rCmd);
    bool exchangeData();
    bool exchangeFoodQuest();
    bool exchangeAccData();
    bool achieve_patch_1();
    bool exchangeStore();
  private:
    bool loadCharData();
    bool loadAccData();

    bool saveCharData(bool isOffline=false);
    bool saveAccData(bool isOffline=false);
    void addRollback(QWORD accid, QWORD charid, DWORD timestamp, ROLLBACK_TYPE datatype, std::string &data);

    bool exchangeScenery(const TMapExchangeUser& mapUser, BlobScenery* pAccScenery);
    bool exchangeManual(const TMapExchangeUser& mapUser, BlobManual* pAccManual, BlobUnsolvedPhoto* pAccPhoto);
    bool exchangeFood(const TMapExchangeUser& mapUser, BlobFood* pAccFood, BlobManual* pAccManual);
    bool exchangeQuest(const TMapExchangeUser& mapUser, BlobQuest* pAccQuest);
    bool exchangeTitle(const TMapExchangeUser& mapUser, BlobTitle* pAccTitle, BlobQuest* pAccQuest);
    bool exchangeAchieve(const TMapExchangeUser& mapUser, BlobAchieve* pAccAchieve, BlobManual* pAccManual);
    bool exchangeMenu(const TMapExchangeUser& mapUser, BlobMenu* pAccMenu);
    bool exchangePortrait(const TMapExchangeUser& mapUser, BlobPortrait* pAccPortrait);

    bool updataCharPrimaryId();
  private:
    UserAccData m_oAcc;
    UserBaseData m_oBase;
    RedisUserData m_oRedis;
    BlobAccQuest m_oOldQuest;

    string m_oTransData;
    string m_oCharData;
    string m_oAccData;
    //string m_oStore;

    QWORD m_qwDeleteCharID = 0;

    DWORD m_dwTimeTick = 0;
    DWORD m_dwSequence = 0;
};
typedef std::map<QWORD, RecordUser*> TMapRecordUser;


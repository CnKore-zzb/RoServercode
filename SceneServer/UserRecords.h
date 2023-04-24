#pragma once
#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"
#include "RecordCmd.pb.h"
#include "SceneUser2.pb.h"
#include <map>

using namespace Cmd;
using std::map;

class SceneUser;
typedef map<DWORD, SlotInfo> TMapType2SlotInfo;
typedef map<DWORD, ProfessionData> TMapType2RecordUserInfo;
typedef map<QWORD, UserAstrolMaterialData> TMapType2AstrolMaterialInfo; //星盘材料

class UserRecords : private xNoncopyable
{
  public:
    UserRecords(SceneUser* pUser);
    ~UserRecords();

    bool loadAcc(const BlobRecordInfo& rData);
    bool saveAcc(BlobRecordInfo* pBlob);
    void initSlotInfo();
    void refreshSlotInfo();
    void sendRecordsData();

    void userSaveRecord(SaveRecordUserCmd& cmd);
    void userLoadRecord(LoadRecordUserCmd& cmd, bool multi_role = false);
    void userBuySlot(BuyRecordSlotUserCmd& cmd);
    void userChangeRecordName(ChangeRecordNameUserCmd& cmd);
    void userDeleteRecord(DeleteRecordUserCmd& cmd);

    void onProfesChange();
    void onJobLvChange(DWORD dwJobLv);
    void onCharNameChange(std::string strCharName);
    void onDelChar(QWORD qwDelCharid);
    void onEquipExchange(const std::string& guidOld, const std::string& guidNew);
    void onEnterSceneEnd();

    void setCardExpireTime(DWORD dwTime);

  private:
    bool isInMonthCard() const;
    bool isInValidMap() const;
    bool isTwoExchangePro() const;
    bool isInLoadCD() const;
    bool checkSlotStatusOn(DWORD dwSlotID);
    bool checkHasRecord(DWORD dwSlotID);
    bool checkSlotCanBuy(DWORD dwSlotID);
    bool checkRecordName(std::string strRecordName, std::string strOldName = "");
    bool checkHasMenuOpen();
    SlotInfo* getSlotInfo(DWORD dwSlotID);
    ProfessionData* getRecordUserInfo(DWORD dwSlotID);
    void clearMultiRoleData();

    void collectCurRecordData(ProfessionData& record);
    void loadRecordData(DWORD dwSlotID);

    void collectTotalAstrolMaterialData(UserAstrolMaterialData* pAstrolData);

  private:
    TMapType2SlotInfo m_mapType2Slot;
    TMapType2RecordUserInfo m_mapType2Record;
    TMapType2AstrolMaterialInfo m_mapType2AstrolMaterial;
    DWORD m_dwCurBuyTimes;
    DWORD m_dwLastLoadTime;
    DWORD m_dwDestSlotID;
    DWORD m_dwDestCharID;
    DWORD m_dwDestMapID;
    DWORD m_dwCardExpireTime;
    SceneUser* m_pUser = nullptr;
};

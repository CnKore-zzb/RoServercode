/**
 * @file PetWork.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-03-07
 */

#pragma once

#include "xDefine.h"
#include "xNoncopyable.h"
#include "RecordCmd.pb.h"

struct SPetWorkCond;
struct SPetWorkCFG;
class SceneUser;

enum EWorkRewardMethod
{
  EWORKREWARDMETHOD_PACK = 1,
  EWORKREWARDMETHOD_MAIL = 2,
};

// work space
struct SWorkSpace
{
  DWORD dwID = 0;
  DWORD dwStartTime = 0;
  DWORD dwLastRewardTime = 0;

  bool bUnlock = false;

  const SPetWorkCFG* pCFG = nullptr;

  Cmd::EWorkState eState = Cmd::EWORKSTATE_MIN;
  std::map<std::string, Cmd::ItemData> mapEggs;
  TVecDWORD vecCounts;
  TVecDWORD vecLastCounts;

  void fromData(const Cmd::WorkSpace& rData);
  void toData(Cmd::WorkSpace* pData);
  void toClient(Cmd::WorkSpace* pData);
};
typedef std::map<DWORD, SWorkSpace> TMapWorkSpace;

struct SWorkPetExtra
{
  std::string guid;
  DWORD dwLastSpaceID = 0;
  DWORD dwCount = 0;

  void fromData(const Cmd::WorkPetExtra& rData);
  void toData(Cmd::WorkPetExtra* pData);
};
typedef std::map<std::string, SWorkPetExtra> TMapWorkPetExtra;
typedef std::map<DWORD, DWORD> TMapWorkLastReward;

// pet work
class PetWork : private xNoncopyable
{
  public:
    PetWork(SceneUser* pUser);
    virtual ~PetWork();

    bool load(const Cmd::BlobPetWork& rData);
    bool save(Cmd::BlobPetWork* pData);
    void reload();

    void queryWorkManual();
    void queryWorkData();
    void refreshWorkSpace(bool bNtf = true);
    void setCardExpireTime(DWORD dwTime) { if (dwTime > m_dwCardExpireTime) m_dwCardExpireTime = dwTime; }
    DWORD getCardExpireTime() { return m_dwCardExpireTime; }
    bool isInMonthCard() const;

    EError unlockManual(bool bCheck = true);
    EError startWork(const Cmd::StartWorkPetCmd& cmd);
    EError stopWork(const Cmd::StopWorkPetCmd& cmd, EWorkRewardMethod eMethod = EWORKREWARDMETHOD_PACK);
    EError getWorkReward(const Cmd::GetPetWorkRewardPetCmd& cmd, EWorkRewardMethod eMethod = EWORKREWARDMETHOD_PACK);

    void testFrequency();
    SWorkSpace* getSpaceData(DWORD dwID);
    DWORD getMaxWorkCount();
  private:
    bool checkEnable(const SPetWorkCond& rCond, SceneUser* pUser);
    bool calcRewardCount(const SWorkSpace& rSpace, DWORD dwNow, DWORD& rCount, DWORD& rFrequency) const;
    void refreshCapra(SWorkSpace& rSpace);
    void refreshActivity(SWorkSpace& rSpace, bool bStop = true);
    void refreshLastCount(SWorkSpace& rSpace, DWORD dwNow, DWORD dwEnableCount);

    DWORD getCurWorkCount();
    DWORD getDestTime(const SWorkSpace& rSpace, DWORD dwNow) const;
    DWORD getCFGDestTime(const SWorkSpace& rSpace) const;
    const SWorkPetExtra* getPetExchange(const std::string& guid);

    void addPetExchangeCount(const std::string& guid, const SWorkSpace& rSpace);
    void update();
    void resetPetExtra();
  private:
    SceneUser* m_pUser = nullptr;

    Cmd::WorkManual m_oManual;
    TMapWorkSpace m_mapWorkSpace;
    TMapWorkPetExtra m_mapPetExtra;
    TMapWorkLastReward m_mapLastReward;

    TSetDWORD m_setUpdateIDs;

    bool m_bQueryData = false;
    DWORD m_dwCardExpireTime = 0;
};


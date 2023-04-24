/**
 * @file Servant.h
 * @brief 
 * @author liangyongqiang, liangyongqiang@xindong.com
 * @version v1
 * @date 2018-02-28
 */

#pragma once

#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "RecordCmd.pb.h"
#include "ServantConfig.h"

using std::vector;
using std::string;
using std::set;

using namespace Cmd;

namespace Cmd
{
  class BlobServant;
};

const static DWORD kDayRefreshTime = 5;

enum EStayStatus
{
  ESTAY_STATUS_MIN = 0,
  ESTAY_STATUS_COUNT = 1 << 0,
  ESTAY_STATUS_COUNT_REWARD = 1 << 1,
  ESTAY_STATUS_OVER = 1 << 2,
};

class SceneUser;
class SceneNpc;

class Servant
{
  public:
    Servant(SceneUser* pUser);
    ~Servant();

    bool load(const BlobServant& oAccServant, const BlobServant& oCharServant);
    bool save(BlobServant* pAccBlob, BlobServant* pCharBlob);

    void initDefaultRecommendData();
    void resetRecommendData(RecommendItemInfo* pItem);
    void resetRecommendByType(ECycleType eType);
    bool setRecommendStatus(RecommendItemInfo* pItem, ERecommendStatus eStatus, bool bCheck = true);
    void refreshRcommendInfo(DWORD dwid);
    RecommendItemInfo* getRecommendItem(DWORD dwid);
    ERecommendStatus getRecommendStatus(DWORD dwID);

    void timer(DWORD curTime);
    void processService(EServantService eType);
    bool setServant(bool replace, DWORD servantid);
    void showServant(bool show);
    void sendAllRecommendInfo();
    void getServantReward(bool bFavorability, DWORD dwID);
    bool addFavoribility(DWORD dwValue);
    void sendFavorabilityStatus();
    DWORD getServantID() const { return m_dwServantID; }
    bool canGetReward() const;
    bool canCountTime() const;
    void sendStayFavoStatus(DWORD status) const;

    bool checkReset();
    bool checkAppearCondition(DWORD dwid);
    bool checkFinishCondition(DWORD dwid, RecommendItemInfo* pItem);
    bool onAppearEvent(ETriggerType eType);
    bool onFinishEvent(ETriggerType eType, DWORD dwParam = 1);

    void onUserAttack(const SceneNpc* pNpc);
    void onKillNpc(const SceneNpc* pNpc);
    void onUserMove(const xPos& pos);
    void onUserMoveTo(const xPos& pos);
    void onUserGoTo(const xPos& dest);

    bool checkMenu(DWORD dwid);
    bool checkQuest(DWORD dwid, bool isAppearEvent, RecommendItemInfo* pItem = nullptr);
    void checkActivityRealOpen();
    void checkForeverRecommend();


    //女仆提升功能
    void initDefaultGrowthData();
    void sendAllGrowthInfo();
    void sendGrowthInfo(DWORD groupid);
    void sendGrowthStatus(DWORD dwGroupID);
    bool checkGrowthGroupAppearCond(DWORD dwid);
    bool checkGrowthGroupFinishCond(DWORD dwGroupID);
    bool checkGrowthAppearCond(DWORD dwid);
    bool checkGrowthFinishCond(DWORD dwid, GrowthItemInfo* pItem);
    bool onGrowthAppearEvent(ETriggerType eType);
    bool onGrowthFinishEvent(ETriggerType eType, DWORD dwParam = 1);
    void getGrowthReward(DWORD dwid, DWORD dwvalue);
    bool addGrowthValue(DWORD dwGroup, DWORD dwValue);
    bool setGrowthStatus(GrowthItemInfo* pItem, EGrowthStatus eStatus, bool bCheck = true);
    void refreshGrowthInfo(DWORD dwid, bool addUnlock);
    GrowthItemInfo* getGrowthItem(DWORD dwid);
    void resetGrowthData(GrowthItemInfo* pItem);
    bool canAddGrowthRedTip();
    bool openNewGrowth(DWORD groupid);
    bool checkExistGroup(DWORD groupid);
    bool checkExistCurGroup(DWORD groupid);
    bool checkGrowthQuest(DWORD dwid, bool isAppearEvent, GrowthItemInfo* pItem, ETriggerType type);
    bool checkGrowthMenu(DWORD dwid, bool isAppearEvent);
    void checkNewGrowthGroup();
    bool isCountTriggerType(ETriggerType type);
    EGrowthStatus getGrowthStatus(DWORD dwid);
    void finishGrowthGroup(DWORD dwGroupID);
    DWORD getItemCount(DWORD dwItemid);
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwServantID = 0;
    QWORD m_qwServantNpcID = 0;
    TSetDWORD m_setOwnServant;
    DWORD m_dwCountDownStatus = 0;
    DWORD m_dwCountDown = 0;

    std::map<DWORD, RecommendItemInfo> m_mapRecommendItems;
    std::map<DWORD, GrowthItemInfo> m_mapGrowthItems;
    std::map<DWORD, GrowthValueInfo> m_mapGrowthValues;
    std::map<EGrowthType, DWORD> m_mapCurGrowthGroup;
    TSetDWORD m_setUnlockedGrowth;
};

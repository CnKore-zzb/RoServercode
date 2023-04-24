#pragma once

#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "ActivityEvent.pb.h"
#include "xTime.h"
#include "SceneUser.h"
#include <map>
#include <set>
#include "GuildSCmd.pb.h"
#include "SessionShop.pb.h"

using namespace Cmd;
using std::map;
using std::set;

typedef  map<QWORD, ActivityEventInfo> TMapId2ActivityEventInfo;
enum EGBuildingSubmitInc
{
  EGBUILDINGSUBMITINC_SUBMIT = 1,
  EGBUILDINGSUBMITINC_REWARD = 2,
};

class ActivityEventManager : public xSingleton<ActivityEventManager>
{
  friend class xSingleton<ActivityEventManager>;
private:
  ActivityEventManager() {}
public:
  virtual ~ActivityEventManager() {}

  void updateEvent(const ActivityEventNtfSessionCmd& cmd);

  // 具体活动处理函数
  bool isTransferToMapFree(EGoToGearType type, DWORD mapid);
  bool isStoreFree();
  bool getReward(SceneUser* user, EAERewardMode mode, DWORD count, TVecItemInfo& items, DWORD& times);
  void onNpcGuidChange(QWORD oldguid, QWORD id);
  DWORD getExtraTimes(EAERewardMode eMode);
  DWORD getNeedResetTimes(EAERewardMode eMode);/*返回到当前时间本周应该重置的次数*/
  DWORD getNextResetTime(EAERewardMode eMode);
  
  //获取扭蛋打折折扣活动
  ActivityEventInfo* getLotteryDiscount(SceneUser* user, ELotteryType lotteryType, ECoinType coinType, DWORD dwYearMonth, DWORD count);
  void sendActivityEventCount(SceneUser* pUser, EActivityEventType type);
  DWORD getGuildBuildingSubmitInc(EGuildBuilding type, DWORD level, EGBuildingSubmitInc inc);
  bool canBuyShopItem(DWORD shopid);
  DWORD getShopItemOrder(DWORD shopid);

private:
  bool isEventStart(ActivityEventInfo& event) { DWORD cur = now(); return event.begintime() <= cur && event.endtime() > cur; }
  bool isEventStop(QWORD id, TMapId2ActivityEventInfo& rMapEvent);
  bool canGetRewardExtra(EAERewardMode mode, AERewardExtraInfo* info, SceneUser* user, DWORD finishcount);
  bool canGetRewardMutiple(EAERewardMode mode, AERewardMultipleInfo* info, SceneUser* user, DWORD finishcount);
  void bindCharExtra(EAERewardMode mode, AERewardExtraInfo* info, SceneUser* user);
  void bindCharMulti(EAERewardMode mode, AERewardMultipleInfo* info, SceneUser* user);
  void summonNpc();
  void summonLotteryNpc();
  bool isOnlyOne(EActivityEventType type);
  DWORD getLotteryNpcId(Cmd::ELotteryType lotteryType);
  void updateShopCache();
  const ShopItem* getShopItem(DWORD shopid);

  map<EActivityEventType, TMapId2ActivityEventInfo> m_mapType2Event;
  map<QWORD, set<QWORD>> m_mapID2NpcID;
  map<QWORD, set<QWORD>> m_mapID2LotteryNpcID;
  map<DWORD, map<QWORD, ShopItem>> m_mapID2ShopItem;
};

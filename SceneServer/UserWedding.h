#pragma once
#include "xDefine.h"
#include "WeddingSCmd.pb.h"
#include "WeddingCCmd.pb.h"
#include "WeddingConfig.h"

class SceneUser;

class UserWedding
{
public:
  UserWedding(SceneUser*pUser);
  ~UserWedding();   

  //从婚礼同步婚礼数据
  void updateWeddingInfo(const Cmd::SyncWeddingInfoSCmd& rev);
  void weddingEvent(const Cmd::WeddingEventMsgCCmd& rev);
  //订婚成功
  void onReserved(const Cmd::WeddingEventMsgCCmd& rev);
  //放弃订婚成功
  void onGiveupReserve(const Cmd::WeddingEventMsgCCmd& rev);
  //结婚成功
  void onMarried(const Cmd::WeddingEventMsgCCmd& rev);
  //离婚成功
  void onDivorce(const Cmd::WeddingEventMsgCCmd& rev);

  //判断与对方是否婚姻关系
  bool checkMarryRelation(QWORD charid) { return charid && charid == getWeddingParnter(); }

  //获取婚姻状态
  Cmd::EMARITAL getMaritalState();
  bool isMarried() { return getMaritalState() == EMARITAL_MARRIED; }
  //获取订婚对象的charid
  QWORD getReserveParnter();
  //获取结婚对象的charid
  QWORD getWeddingParnter();

  //判断是否可以订婚
  bool checkCanReserve(QWORD qwOtherId);
  //是否有婚礼券
  bool hasTicket();
  //是否有指定的婚礼服务计划
  bool hasPlan(EWeddingPlanType type);
  
  //请求预定婚礼
  bool reqReserve(Cmd::ReserveWeddingDateCCmd& rev);
  //被邀请人回复
  void reserveInviteeReply(const Cmd::ReplyReserveWeddingDateCCmd& rev);
  //主邀请人收到回复
  void reserveInviterReply(const Cmd::ReplyReserveWeddingDateCCmd& rev, QWORD qwCharid2);
  void sendReseveResultCmd2Wedding(DWORD dwDate, DWORD dwCondigId, QWORD qwCharid1, QWORD qwCharid2, bool bSuccess, DWORD dwZoneid, DWORD ticket=0, DWORD money=0);
  Cmd::WeddingInfo& getWeddingInfo() { return m_oWeddingInfo; }
  bool enterRollterCoaster();
  bool divorceRollerCoasterInvite(Cmd::DivorceRollerCoasterInviteCCmd& rev);
  bool divorceRollerCoasterReply(Cmd::DivorceRollerCoasterReplyCCmd& rev);
  bool processDivorceRollerCoasterReply(SceneUser* pInviter, EReply reply);
  bool isLockInvite(DWORD curSec);
  void lockInvite(DWORD curSec);
  void unlockInvite();
  //请求离婚
  bool reqDivorce(Cmd::ReqDivorceCCmd& rev);
  //协议离婚确认
  bool reqTogetherDivorce(Cmd::ReqDivorceCCmd& rev);
  void setTogetherDivorce() { m_bTogetherDivorce = true; }
  bool getTogetherDivorce() {return m_bTogetherDivorce;}
  //删除冒险手册
  void delManualItem(DWORD itemId);
  void delInvitation(DWORD itemId, QWORD qwWeddingId);

  // 召回标记
  void setMissInfo(const ParnterInfo& rInfo) { m_oParnterInfo.CopyFrom(rInfo); }
  const ParnterInfo& getMissInfo() const { return m_oParnterInfo; }

  DWORD reserveTime() const { return m_dwReserveTime; }
private:
  void checkDivorceDelItem();
private:
  SceneUser* m_pUser = nullptr;
  Cmd::WeddingInfo m_oWeddingInfo;
  DWORD m_dwDivorceRollterCoasterReplyTime = 0;
  bool m_bTogetherDivorce = false;      //内存数据

  ParnterInfo m_oParnterInfo;
  DWORD m_dwReserveTime = 0;
};

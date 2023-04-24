#include "ChatManager_SE.h"
#include "PlatLogManager.h"
#include "StatisticsDefine.h"
#include "UserCmd.h"
#include "SessionServer.h"
#include "RedisManager.h"
#include "RecordCmd.pb.h"
#include "SocialCmd.pb.h"
#include "MiscConfig.h"
#include "RoleDataConfig.h"
#include "ItemConfig.h"
#include "UserConfig.h"
//#include "SocialServer.h"
//#include "Guild.h"
//#include "GuildManager.h"
#include "GMTools.pb.h"
#include "TeamCmd.pb.h"
#include "SessionUser.h"
#include "SessionUserManager.h"
#include "CommonConfig.h"
#include "GuildSCmd.pb.h"
#include "SessionThread.h"

// offline msg
OfflineMsgBase::OfflineMsgBase()
{
  m_dwTime = xTime::getCurSec();
}

OfflineMsgBase::~OfflineMsgBase()
{
}

bool OfflineMsgBase::fromData(const xRecord& rRecord)
{
  m_qwId = rRecord.get<QWORD>("id");
  //m_type = static_cast<EOfflineMsg>(rRecord.get<DWORD>("type"));
  m_qwSenderID = rRecord.get<QWORD>("senderid");
  m_qwTargetID = rRecord.get<QWORD>("targetid");
  m_dwTime = rRecord.get<DWORD>("time");
  return true;
}

bool OfflineMsgBase::toData(xRecord& rRecord)
{
  if (m_qwId)
    rRecord.put("id", m_qwId);
  rRecord.put("type", getType());
  rRecord.put("targetid", m_qwTargetID);
  rRecord.put("senderid", m_qwSenderID);
  rRecord.put("time", m_dwTime);
  return true;
}

// offline msg - user
OfflineMsgUser::OfflineMsgUser() : OfflineMsgBase()
{

}

OfflineMsgUser::~OfflineMsgUser()
{

}

bool OfflineMsgUser::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_oCmd.ParseFromString(data);
}

bool OfflineMsgUser::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_oCmd.SerializeToString(&data) == false)
    return false;
  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgUser::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;
  if (pUser->getSocial().checkRelation(m_oCmd.id(), ESOCIALRELATION_BLACK) == true || pUser->getSocial().checkRelation(m_oCmd.id(), ESOCIALRELATION_BLACK_FOREVER) == true)
    return true;
  PROTOBUF(m_oCmd, send, len);
  return pUser->sendCmdToMe(send, len);
}

// offline msg - sys2
OfflineMsgSys2::OfflineMsgSys2() : OfflineMsgBase()
{

}

OfflineMsgSys2::~OfflineMsgSys2()
{

}

bool OfflineMsgSys2::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_oCmd.ParseFromString(data);
}

bool OfflineMsgSys2::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_oCmd.SerializeToString(&data) == false)
    return false;
  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgSys2::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;
  PROTOBUF(m_oCmd, send, len);
  return pUser->sendCmdToMe(send, len);
}


// offline msg trade
OfflineMsgTrade::OfflineMsgTrade() : OfflineMsgBase()
{

}

OfflineMsgTrade::~OfflineMsgTrade()
{

}

bool OfflineMsgTrade::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  m_dwItemId = rRecord.get<DWORD>("itemid");
  m_dwPrice = rRecord.get<DWORD>("price");
  m_dwCount = rRecord.get<DWORD>("count");
  m_dwGiveMoney = rRecord.get<DWORD>("givemoney");
  m_eMoneyType = static_cast<EMoneyType>(rRecord.get<DWORD>("moneytype"));
  return true;
}

bool OfflineMsgTrade::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  rRecord.put("itemid", m_dwItemId);
  rRecord.put("price", m_dwPrice);
  rRecord.put("count", m_dwCount);
  rRecord.put("givemoney", m_dwGiveMoney);
  rRecord.put("moneytype", m_eMoneyType);
  return true;
}

bool OfflineMsgTrade::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;
  const SExchangeItemCFG *pBase = ItemConfig::getMe().getExchangeItemCFG(m_dwItemId);
  if (pBase == nullptr)
    return false;

  //AddMoneyRecordTradeCmd cmd;
  //cmd.set_charid(pUser->id);
  //cmd.set_money_type(m_eMoneyType);
  //cmd.set_total_money(m_dwGiveMoney);
  //cmd.set_itemid(m_dwItemId);
  //cmd.set_count(m_dwCount);
  //PROTOBUF(cmd, send, len);
  //if (pUser->sendCmdToAllScene(send, len) == false)
  //  return false;

  SysMsg cmd;
  cmd.set_id(10250);
  cmd.set_type(EMESSAGETYPE_FRAME);
  cmd.set_act(EMESSAGEACT_ADD);

  MsgParams oParams;
  oParams.addNumber(m_dwCount);
  oParams.addString(pBase->strName);
  oParams.addNumber(m_dwPrice * m_dwCount - m_dwGiveMoney);
  oParams.addNumber(m_dwGiveMoney);
  oParams.toData(cmd);
  MsgManager::sendMsg(pUser->id, 10250, oParams);

  XINF << "[离线消息-交易] 玩家上线发送交易离线消息，id:" << m_qwId << "itemid:" << m_dwItemId << "count:" << m_dwCount << "moneytype:" << m_eMoneyType << "givemoney:" << m_dwGiveMoney << XEND;
  return true;
}

// offline msg sys
OfflineMsgSys::OfflineMsgSys() : OfflineMsgBase()
{

}

OfflineMsgSys::~OfflineMsgSys()
{

}

bool OfflineMsgSys::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);
  m_dwMsgID = rRecord.get<DWORD>("itemid");
  m_str = rRecord.getString("sendername");
  return true;
}

bool OfflineMsgSys::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);
  rRecord.put("itemid", m_dwMsgID);
  rRecord.putString("sendername", m_str);
  return true;
}

bool OfflineMsgSys::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;
  MsgManager::sendMsg(pUser->id, m_dwMsgID, MsgParams(m_str));
  return true;
}

// offline gm msg
OfflineMsgGM::OfflineMsgGM() : OfflineMsgBase()
{

}

bool OfflineMsgGM::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);
  m_strGmcmd = rRecord.getString("gmcmd");
  return true;
}

bool OfflineMsgGM::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);
  rRecord.putString("gmcmd", m_strGmcmd);
  return true;
}

bool OfflineMsgGM::send(SessionUser* pUser)
{
  ExecGMCmd cmd;
  cmd.set_act(exec_gm_cmd_player);
  cmd.set_data(m_strGmcmd);

  PROTOBUF(cmd, send, len);
  Cmd::ForwardToUserSceneSocialCmd message;
  message.set_charid(m_qwTargetID);
  message.set_data(send, len);
  message.set_len(len);
  PROTOBUF(message, send2, len2);
  thisServer->doSocialCmd((const BYTE*)send2, len2);

  XINF << "[离线消息-GM] 玩家上线发送离线GM消息：targetid:" << m_qwTargetID << "id:" << m_qwId << "msg:" << m_strGmcmd << XEND;
  return true;
}

bool OfflineMsgAddItem::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_data.ParseFromString(data);
}

bool OfflineMsgAddItem::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_data.SerializeToString(&data) == false)
    return false;

  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgAddItem::send(SessionUser* pUser)
{
  if(nullptr == pUser)
    return false;

  AddOfflineItemSessionCmd cmd;
  cmd.set_charid(m_qwTargetID);
  cmd.mutable_data()->CopyFrom(m_data);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToScene(send, len);

  return true;
}

// relation msg
OfflineMsgRelation::OfflineMsgRelation() : OfflineMsgBase()
{

}

bool OfflineMsgRelation::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  m_dwRelation = rRecord.get<DWORD>("itemid");
  m_eType = static_cast<EOfflineMsg>(rRecord.get<DWORD>("type"));
  return true;
}

bool OfflineMsgRelation::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);
  rRecord.put("itemid", m_dwRelation);
  return true;
}

bool OfflineMsgRelation::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;

  /*if (m_eType == EOFFLINEMSG_REMOVE_FOCUS)
  {
    RemoveFocusSocialCmd cmd;
    pUser->toData(cmd.mutable_user());
    cmd.set_destid(m_qwSenderID);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToServer(send, len, "SocialServer");
    return true;
  }*/

  static const set<ESocialRelation> setRelation = set<ESocialRelation>{
      ESOCIALRELATION_FRIEND,
      ESOCIALRELATION_MERRY,
      ESOCIALRELATION_CHAT,
      ESOCIALRELATION_TEAM,
      ESOCIALRELATION_APPLY,
      ESOCIALRELATION_BLACK,
      ESOCIALRELATION_BLACK_FOREVER,
      ESOCIALRELATION_TUTOR,
      ESOCIALRELATION_TUTOR_APPLY,
      ESOCIALRELATION_STUDENT,
      ESOCIALRELATION_STUDENT_APPLY,
      ESOCIALRELATION_STUDENT_RECENT,
      ESOCIALRELATION_TUTOR_PUNISH,
      ESOCIALRELATION_TUTOR_CLASSMATE,
      ESOCIALRELATION_RECALL,
      ESOCIALRELATION_BERECALL,
  };
  for (auto &s : setRelation)
  {
    if ((m_dwRelation & s) == 0)
      continue;

    /*bool bHas = pUser->getSocial().checkRelation(m_qwSenderID, s);
    if (bHas && m_eType == EOFFLINEMSG_ADD_RELATION)
      continue;
    if (!bHas && m_eType == EOFFLINEMSG_REMOVE_RELATION)
      continue;*/

    if (m_eType == EOFFLINEMSG_ADD_RELATION)
    {
      AddRelationSocialCmd cmd;
      pUser->toData(cmd.mutable_user());
      cmd.set_destid(m_qwSenderID);
      cmd.set_relation(s);
      cmd.set_check(false);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");

      if(s == ESOCIALRELATION_APPLY)
      {
        SyncRedTipSocialCmd cmd;
        cmd.set_dwid(m_qwSenderID);
        cmd.set_charid(pUser->id);
        cmd.set_red(EREDSYS_SOCIAL_FRIEND_APPLY);
        cmd.set_add(true);

        PROTOBUF(cmd, send, len);
        pUser->sendCmdToScene(send, len);
      }
    }
    else if (m_eType == EOFFLINEMSG_REMOVE_RELATION)
    {
      RemoveRelationSocialCmd cmd;
      pUser->toData(cmd.mutable_user());
      cmd.set_destid(m_qwSenderID);
      cmd.set_relation(s);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");
    }
  }

  return true;
}

// tutor reward
OfflineMsgTutorReward::OfflineMsgTutorReward() : OfflineMsgBase()
{
}

bool OfflineMsgTutorReward::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string rewarddata;
  rewarddata.assign((const char*)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  OfflineTutorReward reward;
  if (reward.ParseFromString(rewarddata) == false)
    return false;

  for (int i = 0; i < reward.teacherrewards_size(); ++i)
    addReward(reward.teacherrewards(i));
  return true;
}

bool OfflineMsgTutorReward::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  OfflineTutorReward reward;
  for (auto& v : m_mapRewards)
  {
    TutorReward* p = reward.add_teacherrewards();
    if (p != nullptr)
      p->CopyFrom(v.second);
  }

  string data;
  if (reward.SerializeToString(&data) == false)
    return false;
  rRecord.putBin("chat", (unsigned char*)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgTutorReward::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;

  SendTutorRewardSocialCmd cmd;
  cmd.set_charid(pUser->id);
  for (auto& v : m_mapRewards)
  {
    TutorReward* p = cmd.add_rewards();
    if (p)
      p->CopyFrom(v.second);

    if (v.second.reward_size() > 0 || v.second.item_size() > 0)
    {
      XLOG << "[导师奖励-同步到玩家]" << pUser->accid << pUser->id << pUser->name << "学生:" << v.second.charid() << v.second.name() << "奖励:";
      for (int i = 0; i < v.second.reward_size(); ++i)
        XLOG << v.second.reward(i);
      XLOG << "任务id:";
      for (int i = 0; i < v.second.item_size(); ++i)
        XLOG << v.second.item(i).taskid();
      XLOG << XEND;
    }
    else
    {
      XERR << "[导师奖励-同步到玩家]" << pUser->accid << pUser->id << pUser->name << "学生:" << v.second.charid() << v.second.name() << "没有奖励";
    }
  }

  if (m_mapRewards.empty() == false)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToScene(send, len);
    XLOG << "[导师奖励-同步到玩家]" << pUser->accid << pUser->id << pUser->name << "成功" << XEND;
  }
  else
  {
    XLOG << "[导师奖励-同步到玩家]" << pUser->accid << pUser->id << pUser->name << "无奖励" << XEND;
  }
  return true;
}

void OfflineMsgTutorReward::addReward(const TutorReward& reward)
{
  auto it = m_mapRewards.find(reward.charid());
  if (it == m_mapRewards.end())
  {
    m_mapRewards[reward.charid()] = reward;
  }
  else
  {
    it->second.set_name(reward.name());
    for (int i = 0; i < reward.reward_size(); ++i)
      it->second.add_reward(reward.reward(i));
    for (int i = 0; i < reward.item_size(); ++i)
    {
      TutorRewardItem* p = it->second.add_item();
      if (p)
      {
        p->set_taskid(reward.item(i).taskid());
        p->set_time(reward.item(i).time());
      }
    }
  }
}

bool OfflineMsgTutorReward::saveToDB()
{
  if (m_mapRewards.empty())
    return true;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[导师奖励-离线数据保存] 获取数据库失败" << m_qwTargetID << XEND;
    return false;
  }

  SessionThreadData *pData = SessionThread::getMe().create(m_qwTargetID, SessionThreadAction_OfflineMsgTutorReward);
  if (!pData)
  {
    XERR << "[导师奖励-离线数据保存] 创建SessionThreadData失败" << m_qwTargetID << XEND;
    return false;
  }
  xRecord *pRecord = pData->createRecord(pField);
  if (!pRecord)
  {
    XERR << "[导师奖励-离线数据保存] 创建Record失败" << m_qwTargetID << XEND;
    return false;
  }
  toData(*pRecord);
  SessionThread::getMe().add(pData);

  return true;
}

OfflineMsgUserAddItem::OfflineMsgUserAddItem() : OfflineMsgBase()
{
}

bool OfflineMsgUserAddItem::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_oData.ParseFromString(data);
}

bool OfflineMsgUserAddItem::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_oData.SerializeToString(&data) == false)
    return false;

  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgUserAddItem::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;

  UserAddItemSocialCmd cmd;
  cmd.mutable_user()->set_charid(pUser->id);
  for (int i = 0; i < m_oData.items_size(); ++i)
  {
    ItemInfo* p = cmd.add_items();
    if (p)
      p->CopyFrom(m_oData.items(i));
  }

  switch (m_oData.type())
  {
  case EUSERADDITEMTYPE_GUILD_DONATE:
    cmd.set_doublereward(static_cast<DWORD>(EDOUBLESOURCE_GUILD_DONATE));
    break;
  default:
    break;
  }

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToScene(send, len);
  return true;
}


// offline msg - wedding
OfflineMsgWedding::OfflineMsgWedding() : OfflineMsgBase()
{

}

OfflineMsgWedding::~OfflineMsgWedding()
{

}

bool OfflineMsgWedding::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_oCmd.ParseFromString(data);
}

bool OfflineMsgWedding::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_oCmd.SerializeToString(&data) == false)
    return false;
  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgWedding::send(SessionUser* pUser)
{
  if (pUser == nullptr)
    return false;
  PROTOBUF(m_oCmd, send, len);
  //发送消息到场景
  return pUser->sendCmdToSceneUser(send, len);
}

OfflineMsgQuota::OfflineMsgQuota() : OfflineMsgBase()
{
}

OfflineMsgQuota::~OfflineMsgQuota()
{
}

bool OfflineMsgQuota::fromData(const xRecord& rRecord)
{
  OfflineMsgBase::fromData(rRecord);

  string data;
  data.assign((const char *)rRecord.getBin("chat"), rRecord.getBinSize("chat"));

  return m_oData.ParseFromString(data);
}

bool OfflineMsgQuota::toData(xRecord& rRecord)
{
  OfflineMsgBase::toData(rRecord);

  string data;
  if (m_oData.SerializeToString(&data) == false)
    return false;

  rRecord.putBin("chat", (unsigned char *)(data.c_str()), data.size());
  return true;
}

bool OfflineMsgQuota::send(SessionUser* pUser)
{
  if (!pUser)
    return false;

  UserQuotaOperSessionCmd cmd;
  cmd.set_charid(m_qwTargetID);
  cmd.set_quota(m_oData.quota());
  cmd.set_oper(m_oData.oper());
  cmd.set_type(m_oData.type());

  PROTOBUF(cmd, send, len);
  return pUser->sendCmdToSceneUser(send, len);
}

// chat manager
ChatManager_SE::ChatManager_SE()
{

}

ChatManager_SE::~ChatManager_SE()
{
  final();
}

void ChatManager_SE::onUserOnline(SessionUser* pUser)
{
  TVecDWORD vecType; //需要上线加载的离线数据类型
  vecType.push_back((DWORD)EOFFLINEMSG_USER);
  vecType.push_back((DWORD)EOFFLINEMSG_TRADE);
  vecType.push_back((DWORD)EOFFLINEMSG_GM);
  vecType.push_back((DWORD)EOFFLINEMSG_SYS);
  vecType.push_back((DWORD)EOFFLINEMSG_SYS2);
  vecType.push_back((DWORD)EOFFLINEMSG_TUTOR_REWARD);
  vecType.push_back((DWORD)EOFFLINEMSG_USER_ADD_ITEM);
  vecType.push_back((DWORD)EOFFLINEMSG_ADD_ITEM);
  vecType.push_back((DWORD)EOFFLINEMSG_WEDDING);
  vecType.push_back((DWORD)EOFFLINEMSG_USER_QUOTA);

  preLoadDb(pUser->id, vecType, OFFLINE_MSG_LOAG_TYPE_ONE);
}

void ChatManager_SE::final()
{
}

OfflineMsgBase* ChatManager_SE::createOfflineMsg(EOfflineMsg eType)
{
  OfflineMsgBase* pData = nullptr;
  switch (eType)
  {
    case EOFFLINEMSG_MIN:
      break;
    case EOFFLINEMSG_USER:
      pData = NEW OfflineMsgUser();
      break;
    case EOFFLINEMSG_TRADE:
      pData = NEW OfflineMsgTrade();
      break;
    case EOFFLINEMSG_GM:
      pData = NEW OfflineMsgGM();
      break;
    case EOFFLINEMSG_SYS:
      pData = NEW OfflineMsgSys();
      break;
    case EOFFLINEMSG_SYS2:
      pData = NEW OfflineMsgSys2();
      break;
    case EOFFLINEMSG_ADD_RELATION:
    case EOFFLINEMSG_REMOVE_RELATION:
    //case EOFFLINEMSG_REMOVE_FOCUS:
      pData = NEW OfflineMsgRelation();
      break;
    case EOFFLINEMSG_ADD_ITEM:
      pData = NEW OfflineMsgAddItem();
      break;
    case EOFFLINEMSG_TUTOR_REWARD:
      pData = NEW OfflineMsgTutorReward();
      break;
    case EOFFLINEMSG_USER_ADD_ITEM:
      pData = NEW OfflineMsgUserAddItem();
      break;
    case EOFFLINEMSG_WEDDING:
      pData = NEW OfflineMsgWedding();
      break;
    case EOFFLINEMSG_USER_QUOTA:
      pData = NEW OfflineMsgQuota();
      break;
    case EOFFLINEMSG_MAX:
      break;
  }
  return pData;
}

bool ChatManager_SE::preLoadDb(QWORD targetId, TVecDWORD& typeVec, DWORD type)
{
  if (typeVec.empty())
  {
    XERR << "[离线消息-加载],没有类型可以加载" << targetId << XEND;
    return false;
  }
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[离线消息-加载],无法获得数据库:" << REGION_DB << XEND;
    return false;
  }
  SessionThreadData *pData = SessionThread::getMe().create(targetId, SessionThreadAction_OfflineMsg_Load);
  if (!pData)
  {
    XERR << "[离线消息-加载] 创建SessionThreadData失败" << XEND;
    return false;
  }
  xRecord *pRecord = pData->createRecord(pField);
  if (!pRecord)
  {
    XERR << "[离线消息-加载] 创建Record失败" << targetId << XEND;
    return false;
  }
  DWORD index = 0;
  std::stringstream ssCondition;
  ssCondition.str("");
  for(auto &v : typeVec)
  {
    if(0 != index) ssCondition << " or ";
    ssCondition << "type = " << v;
    index++;
  }

  char where[1024] = { 0 };
  snprintf(where, sizeof(where), "targetid = %llu and (%s)", targetId, ssCondition.str().c_str());
  pData->m_strWhere = where;
  pData->m_dwMsgLoadType = type;
  SessionThread::getMe().add(pData);

  return true;
}

bool ChatManager_SE::loadDb(QWORD targetId, TVecOfflineMsg& resVec, xRecordSet &set)
{
  if (set.empty()) return true;

  DWORD ret = set.size();
  std::map<QWORD/*itemid <<32+price*/, OfflineMsgTrade*> mapTradeMsg;
  OfflineMsgBase* firstTutorMsg = nullptr;
  for (DWORD i = 0; i < ret; ++i)
  {
    DWORD dwType = set[i].get<DWORD>("type");
    if (dwType <= EOFFLINEMSG_MIN || EOfflineMsg_IsValid(dwType) == false)
    {
      XERR << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "不合法,无法加载" << XEND;
      continue;
    }

    if (dwType == EOFFLINEMSG_TRADE)
    {
      QWORD key = set[i].get<DWORD>("itemid");//msg.itemid();
      key = (key << 31) + set[i].get<DWORD>("price");//msg.price();
      OfflineMsgTrade* pTradeMsg = nullptr;
      auto it = mapTradeMsg.find(key);
      if (it == mapTradeMsg.end())
      {
        OfflineMsgBase* pBase = createOfflineMsg(static_cast<EOfflineMsg>(dwType));
        if (pBase == nullptr || pBase->fromData(set[i]) == false)
        {
          XERR << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "创建失败或者数据错误" << XEND;
          if (pBase)
            SAFE_DELETE(pBase);
          continue;
        }
        pTradeMsg = dynamic_cast<OfflineMsgTrade*>(pBase);
        if (pTradeMsg == nullptr)
        {
          XERR << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "不是交易所离线消息" << XEND;
          SAFE_DELETE(pBase);
          continue;
        }
        XDBG << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "加载成功" << XEND;
        //OfflineMsgTrade* pMsg = NEW OfflineMsgTrade();
        //pMsg->fromData(msg);
        if (pBase)
          mapTradeMsg.insert(std::make_pair(key, pTradeMsg));
      }
      else
      {
        pTradeMsg = it->second;
        if (pTradeMsg)
        {
          pTradeMsg->addCount(set[i].get<DWORD>("count"));//msg.count());
          pTradeMsg->addGiveMoney(set[i].get<DWORD>("givemoney"));//msg.givemoney());
        }
      }
      if (pTradeMsg)
      {
        pTradeMsg->insertId(set[i].get<QWORD>("id"));
      }
    }
    else if (dwType == EOFFLINEMSG_TUTOR_REWARD)
    {
      OfflineMsgBase* pBase = createOfflineMsg(static_cast<EOfflineMsg>(dwType));
      if (pBase == nullptr)
        continue;
      pBase->setId(set[i].get<QWORD>("id")); // 设置id用于删除
      resVec.push_back(pBase);
      if (firstTutorMsg == nullptr)
        firstTutorMsg = pBase;
      QWORD preid = firstTutorMsg->getId();
      if (firstTutorMsg->fromData(set[i]) == false) // 使用第一个msg一次性发送所有数据
        XERR << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "导师数据错误" << XEND;
      firstTutorMsg->setId(preid);
    }
    else
    {
      OfflineMsgBase* pBase = createOfflineMsg(static_cast<EOfflineMsg>(dwType));
      if (pBase == nullptr || pBase->fromData(set[i]) == false)
      {
        XERR << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "创建失败或者数据错误" << XEND;
        if (pBase)
          SAFE_DELETE(pBase);
        continue;
      }
      resVec.push_back(pBase);
      XDBG << "[离线消息-加载] charid :" << targetId << "id :" << set[i].get<QWORD>("id") << "type :" << dwType << "加载成功" << XEND;
    }
  }

  for (auto it = mapTradeMsg.begin(); it != mapTradeMsg.end(); ++it)
    resVec.push_back(it->second);

  return true;
}

bool ChatManager_SE::reLoadDbByType(SessionUser* pUser, EOfflineMsg eType)
{
  TVecDWORD vecType;
  vecType.push_back((DWORD)eType);

  if (!preLoadDb(pUser->id, vecType, OFFLINE_MSG_LOAG_TYPE_THREE))
    return false;

  return true;
}

void ChatManager_SE::insertDb(OfflineMsgBase* pMsgBase)
{
  if (!pMsgBase)
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[离线消息-保存] charid :" << pMsgBase->getTargetID() << "id :" << pMsgBase->getId() << "type :" << pMsgBase->getType() << "获取offmsg数据表失败" << XEND;
    return;
  }
  SessionThreadData *pData = SessionThread::getMe().create(pMsgBase->getTargetID(), SessionThreadAction_OfflineMsg_Insert);
  if (!pData)
  {
    XERR << "[离线消息-保存] 创建SessionThreadData失败" << pMsgBase->getTargetID() << XEND;
    return;
  }
  xRecord *pRecord = pData->createRecord(pField);
  if (!pRecord)
  {
    XERR << "[离线消息-保存] 创建Record失败" << pMsgBase->getTargetID() << XEND;
    return;
  }

  pMsgBase->toData(*pRecord);
  SessionThread::getMe().add(pData);

  XLOG << "[离线消息-保存] charid :" << pMsgBase->getTargetID() << "id :" << pMsgBase->getId() << "type :" << pMsgBase->getType() << "保存成功" << XEND;
}

void ChatManager_SE::delDb(const TSetQWORD&/*id*/ setId)
{
  if (setId.empty())
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[离线消息-删除] 获取数据表offmsg失败" << XEND;
    return;
  }

  SessionThreadData *pData = SessionThread::getMe().create(0, SessionThreadAction_OfflineMsg_Delete);
  if (!pData)
  {
    XERR << "[离线消息-删除] 创建SessionThreadData失败" << XEND;
    return;
  }
  xRecord *pRecord = pData->createRecord(pField);
  if (!pRecord)
  {
    XERR << "[离线消息-保存] 创建Record失败" << XEND;
    return;
  }

  DWORD index = 0;
  std::stringstream ssCondition;
  ssCondition.str("");
  ssCondition << "id in (";
  for (auto &v : setId)
  {
    if (0 != index) ssCondition << ",";
    ssCondition << v;
    index++;
  }
  ssCondition << ")";

  pData->m_strWhere = ssCondition.str();
  SessionThread::getMe().add(pData);

  //setId.clear();
}

//离线聊天暂时不处理了
void ChatManager_SE::chatLog(QWORD sID, QWORD sAccID, DWORD sPlat, DWORD sZone, DWORD sLv, QWORD rID, QWORD rAccID, const string rname, const ChatRetCmd& cmd)
{
    if (cmd.voicetime() == 0)
    {
      PlatLogManager::getMe().chatLog(thisServer,
        sPlat,
        sZone,
        sAccID,
        cmd.name(),
        sID,
        rAccID,
        rname,
        rID,
        cmd.channel(),
        0,
        cmd.str(),
        sLv
      );
    }
    else
    {
      PlatLogManager::getMe().chatLog(thisServer,
        sPlat,
        sZone,
        sAccID,
        cmd.name(),
        sID,
        rAccID,
        rname,
        rID,
        cmd.channel(),
        0,
        "",
        sLv,
        cmd.voicetime()
      );
    }

}

void ChatManager_SE::addOfflineMsg(QWORD qwCharID, const ItemData& data)
{
  OfflineMsgAddItem* pMsg = NEW OfflineMsgAddItem();
  if(nullptr == pMsg)
    return;

  pMsg->setTargetID(qwCharID);
  pMsg->init(data);
  insertDb(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD qwCharID, const ChatRetCmd& cmd)
{
  OfflineMsgUser* pMsg = NEW OfflineMsgUser();
  if (pMsg == nullptr)
    return;

  pMsg->setSenderID(qwCharID);
  pMsg->setTargetID(cmd.targetid());
  pMsg->init(cmd);
  insertDb(pMsg);
}

//在线直接发送
void ChatManager_SE::addOfflineMsg(QWORD qwCharID, const SysMsg& cmd)
{
  OfflineMsgSys2* pMsg = NEW OfflineMsgSys2();
  if (pMsg == nullptr)
    return;
  pMsg->setSenderID(qwCharID);
  pMsg->setTargetID(qwCharID);
  pMsg->init(cmd);

  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwCharID);
  if (pUser)
  {
    if (pMsg->send(pUser))
    {
      SAFE_DELETE(pMsg);
      return;
    }
  }
  insertDb(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD destUser, DWORD itemId, DWORD price, DWORD count, DWORD giveMoney, EMoneyType moneyType)
{
  OfflineMsgTrade* pMsg = NEW OfflineMsgTrade();
  if (pMsg == nullptr)
    return;

  pMsg->setTargetID(destUser);
  pMsg->setItemID(itemId);
  pMsg->setPrice(price);
  pMsg->addCount(count);
  pMsg->addGiveMoney(giveMoney);
  pMsg->setMoneyType(moneyType);

  XLOG << "[离线消息-交易] 添加离线消息 targetid:" << destUser << ",itemid:" << itemId << ", price:" << price <<
    ",count:" << count << ",givemoney:" << giveMoney << ", moneytype:" << moneyType << XEND;

  insertDb(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, DWORD dwMsgID, const string& str /*= ""*/)
{
  OfflineMsgSys* pMsg = NEW OfflineMsgSys();
  if (pMsg == nullptr)
    return;

  pMsg->setTargetID(qwTargetID);
  pMsg->setMsgID(dwMsgID);
  pMsg->setStr(str);

  insertDb(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, QWORD qwDestID, EOfflineMsg eType, DWORD dwRelation)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[离线消息-保存] charid :" << qwTargetID << "id :" << qwDestID << "type :" << eType << "获取offmsg数据表失败" << XEND;
    return;
  }

  stringstream sstr;
  sstr << "targetid = " << qwTargetID << " and senderid = " << qwDestID << " and type = " << eType;

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[离线消息-保存] charid :" << qwTargetID << "id :" << qwDestID << "type :" << eType << "查询offmsg数据表失败" << XEND;
    return;
  }

  OfflineMsgRelation oMsg;
  oMsg.setTargetID(qwTargetID);
  oMsg.setSenderID(qwDestID);
  oMsg.setType(eType);
  if (ret == 0)
  {
    oMsg.setRelation(dwRelation);
    xRecord record(pField);
    oMsg.toData(record);
    QWORD retcode = thisServer->getDBConnPool().exeInsert(record);
    if (retcode == QWORD_MAX)
    {
      XERR << "[离线消息-保存] charid :" << oMsg.getTargetID() << "id :" << oMsg.getId() << "type :" << oMsg.getType() << "插入数据库失败" << XEND;
      return;
    }
    XLOG << "[离线消息-保存] charid :" << oMsg.getTargetID() << "id :" << oMsg.getId() << "type :" << oMsg.getType() << "保存成功" << XEND;
  }
  else
  {
    DWORD dwValue = set[0].get<DWORD>("itemid");
    if (oMsg.getType() == EOFFLINEMSG_ADD_RELATION)
      dwValue |= dwRelation;
    else if (oMsg.getType() == EOFFLINEMSG_REMOVE_RELATION)
      dwValue &= (~dwRelation);
    oMsg.setRelation(dwValue);
    xRecord record(pField);
    oMsg.toData(record);
    QWORD retcode = thisServer->getDBConnPool().exeInsert(record, true, true);
    if (retcode == QWORD_MAX)
    {
      XERR << "[离线消息-保存] charid :" << oMsg.getTargetID() << "id :" << oMsg.getId() << "type :" << oMsg.getType() << "更新数据库失败" << XEND;
      return;
    }
    XLOG << "[离线消息-保存] charid :" << oMsg.getTargetID() << "id :" << oMsg.getId() << "type :" << oMsg.getType() << "更新成功" << XEND;
  }

  GCharReader oReader(thisServer->getRegionID(), qwTargetID);
  oReader.getByTutor();

  GCharWriter oWriter(thisServer->getRegionID(), qwTargetID);
  const TMapSocial& mapList = oReader.getSocial();
  for (auto &m : mapList)
    oWriter.updateRelation(m.first, m.second);
  oWriter.addRelation(qwDestID, dwRelation);
  oWriter.setSocial();
  oWriter.save();
}

void ChatManager_SE::addOfflineGmMsg(QWORD qwTargetID,const std::string& gmcmd)
{
  OfflineMsgGM* pMsg = NEW OfflineMsgGM();
  if (pMsg == nullptr)
    return;
  pMsg->setTargetID(qwTargetID);
  pMsg->setGM(gmcmd);
  insertDb(pMsg);
  XLOG << "[离线消息-GM] 添加离线GM消息：charid:" << qwTargetID << "," << gmcmd << XEND;
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, const TutorReward& reward)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "offmsg");
  if (pField == nullptr)
  {
    XERR << "[添加离线导师奖励],无法获得数据库:" << REGION_DB << XEND;
    return;
  }

  char where[128] = { 0 };
  snprintf(where, sizeof(where), "targetid = %llu and senderid = %llu and type = %d", qwTargetID, static_cast<QWORD>(reward.charid()), EOFFLINEMSG_TUTOR_REWARD);
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[添加离线导师奖励]" << qwTargetID << "查询奖励出错" << XEND;
    ret = 0;
  }

  OfflineMsgTutorReward* pMsg = dynamic_cast<OfflineMsgTutorReward*>(createOfflineMsg(EOFFLINEMSG_TUTOR_REWARD));
  if (pMsg == nullptr)
    return;

  pMsg->setTargetID(qwTargetID);
  pMsg->setSenderID(reward.charid());
  for (DWORD i = 0; i < ret; ++i)
    pMsg->fromData(set[i]);
  pMsg->addReward(reward);
  pMsg->saveToDB();

  SAFE_DELETE(pMsg);

  XLOG << "[添加离线导师奖励]" << qwTargetID << "添加成功" << XEND;
  return;
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, const OffMsgUserAddItem& data)
{
  OfflineMsgUserAddItem* pMsg = NEW OfflineMsgUserAddItem();
  if (pMsg == nullptr)
    return;

  pMsg->setTargetID(qwTargetID);
  pMsg->init(data);
  insertDb(pMsg);
  SAFE_DELETE(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, const WeddingEventMsgCCmd& cmd)
{
  OfflineMsgWedding* pMsg = new OfflineMsgWedding();
  if (pMsg == nullptr)
    return;

  pMsg->setTargetID(qwTargetID);
  pMsg->init(cmd);

  //在线直接处理掉
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwTargetID);
  if (pUser)
  {
    pMsg->send(pUser);
  }
  else
  {
    insertDb(pMsg);
  }

  SAFE_DELETE(pMsg);
}

void ChatManager_SE::addOfflineMsg(QWORD qwTargetID, const OffMsgUserQuotaData& cmd)
{
  OfflineMsgQuota* pMsg = NEW OfflineMsgQuota();
  if(!pMsg)
    return;

  pMsg->setTargetID(qwTargetID);
  pMsg->init(cmd);

  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwTargetID);
  if(pUser)
    pMsg->send(pUser);
  else
    insertDb(pMsg);

  SAFE_DELETE(pMsg);
}

bool ChatManager_SE::queryZoneStatus(DWORD dwNow, xNetProcessor* pTask /*= nullptr*/)
{
  if (dwNow > m_dwZoneTime || m_oZoneCmd.infos_size() == 0)
  {
    m_oZoneCmd.Clear();

    xField* pField = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "zone");
    if (pField == nullptr)
    {
      XERR << "[聊天管理-区信息查询] 无法获取" << RO_DATABASE_NAME << "zone数据表" << XEND;
      return false;
    }

    m_oZoneCmd.Clear();
    const SZoneMiscCFG& rCFG = MiscConfig::getMe().getZoneCFG();

    xRecordSet set;
    pField->setValid("zoneid, active, online, category");
    stringstream sstr;
    sstr << "status = 2" << " and regionid = " << thisServer->getRegionID();
    QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
    if (ret == QWORD_MAX)
    {
      XERR << "[聊天管理-区信息查询] 查询失败 ret :" << ret << XEND;
      return false;
    }

    xLuaData oMaxLvData;
    for (QWORD q = 0; q < ret; ++q)
    {
      if (!CommonConfig::m_dwOnlineUserMax || !CommonConfig::m_dwActiveUserMax)
      {
        XERR << "[聊天管理-区信息查询] CommonConfig onlineuser and activeuser 未配置" << XEND;
        break;
      }
      DWORD dwCategory = set[q].get<DWORD>("category");
      if (dwCategory)
        continue;

      ZoneInfo* pInfo = m_oZoneCmd.add_infos();
      if (pInfo == nullptr)
      {
        XERR << "[聊天管理-区信息查询] 创建 ZoneInfo 失败" << XEND;
        continue;
      }

      DWORD dwZoneID = set[q].get<DWORD>("zoneid");
      sstr.str("");
      sstr << dwZoneID;
      oMaxLvData.setData(sstr.str(), "");

      pInfo->set_zoneid(dwZoneID);
      pInfo->set_status(rCFG.getZoneStatus(100.0f * set[q].get<QWORD>("online") / CommonConfig::m_dwOnlineUserMax));
      pInfo->set_state(rCFG.getZoneState(100.0f * set[q].get<QWORD>("active") / CommonConfig::m_dwActiveUserMax));
    }

    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_MAX_BASELV, "data");
    if (RedisManager::getMe().getHash(key, oMaxLvData) == true)
    {
      for (int i = 0; i < m_oZoneCmd.infos_size(); ++i)
      {
        const ZoneInfo& rInfo = m_oZoneCmd.infos(i);

        stringstream szone;
        szone << rInfo.zoneid();

        DWORD dwMaxLv = oMaxLvData.getTableInt(szone.str());

        // process old data
        if (dwMaxLv == 0)
        {
          string oldkey = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_MAX_BASELV, rInfo.zoneid());
          if (RedisManager::getMe().getData(oldkey, dwMaxLv) == true)
          {
            xLuaData oData;
            oData.setData(szone.str(), dwMaxLv);
            if (RedisManager::getMe().setHash(key, oData) == true)
            {
              RedisManager::getMe().delData(oldkey);
              oMaxLvData.setData(szone.str(), dwMaxLv);
              XLOG << "[聊天管理-区信息查询] 处理原数据 maxlv :" << dwMaxLv << XEND;
            }
          }
          else
          {
            XERR << "[聊天管理-区信息查询] 处理原数据失败" << XEND;
          }
        }

        m_oZoneCmd.mutable_infos(i)->set_maxbaselv(dwMaxLv);
      }
    }
    else
    {
      XERR << "[聊天管理-区信息查询] 查询最大等级失败" << XEND;
    }

    m_dwZoneTime = dwNow + QUERY_ZONE_TICK;
    XLOG << "[聊天管理-区信息查询] 在" << dwNow << "查询信息 :" << m_oZoneCmd.ShortDebugString() << XEND;
  }

  PROTOBUF(m_oZoneCmd, send, len);
  if (pTask != nullptr)
    pTask->sendCmd(send, len);
  else
    thisServer->sendCmdToAllScene(send, len);
  return true;
}

bool ChatManager_SE::doChatCmd(SessionUser* pUser, const BYTE* buf, WORD len)
{
  if (pUser == nullptr || buf == nullptr || len == 0)
    return false;

  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case CHATPARAM_QUERYITEMDATA:   // 在SceneUser::doChatCmd处理
    case CHATPARAM_BARRAGE:         // 在SceneUser::doChatCmd处理
    case CHATPARAM_BARRAGEMSG:      // 在SceneUser::doChatCmd处理
    case CHATPARAM_CHAT:            // 在SceneUser::doChatCmd处理
      break;
    case CHATPARAM_QUERYUSERINFO:
      {
        PARSE_CMD_PROTOBUF(QueryUserInfoChatCmd, rev);
        GCharReader* pTargetGChar = nullptr;
        GCharReader gChar(thisServer->getRegionID(), rev.charid());
        SessionUser* pTarget = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pTarget != nullptr)
        {
          pTargetGChar = pTarget->getGCharData();
        }
        else
        {
          if (!gChar.getByQuery())
          {
            MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_QUERY_CLOSE);
            XERR << "[玩家-聊天消息]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "查询 charid :" << rev.charid() << "失败,未存在该玩家" << XEND;
            break;
          }
          pTargetGChar = &gChar;
        }

        do
        {
          QWORD qwPartnerID = pUser->getReserveParnter();
          if (rev.charid() != qwPartnerID)
          {
            if (pTarget != nullptr)
            {
              GSocial& rSocial = pTarget->getSocial();
              if (rSocial.checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || rSocial.checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true)
              {
                rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
                break;
              }
              if (pTargetGChar->getQueryType() == EQUERYTYPE_FRIEND && rSocial.checkRelation(pUser->id, ESOCIALRELATION_FRIEND) == false)
              {
                rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
                break;
              }
            }
            else
            {
              if (pTargetGChar->checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || pTargetGChar->checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) != 0)
              {
                rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
                break;
              }
              if (pTargetGChar->getQueryType() == EQUERYTYPE_FRIEND && pTargetGChar->checkRelation(pUser->id, ESOCIALRELATION_FRIEND) == false)
              {
                rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
                break;
              }
            }

            if (pTargetGChar->getQueryType() == EQUERYTYPE_CLOSE)
            {
              rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
              break;
            }
          }

          string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_USERINFO, rev.charid());
          if (RedisManager::getMe().getProtoData(key, rev.mutable_info()) == true)
            break;

          xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
          if (field == nullptr)
          {
            rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
            break;
          }

          field->setValid("charid, accid, mapid, rolelv, offlinetime, profession, gender, name, body, hair, haircolor, data");
          char where[256] = {0};
          snprintf(where, sizeof(where), "charid=%llu", static_cast<QWORD>(rev.charid()));

          xRecordSet set;
          QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char*)where, nullptr);
          if (ret == QWORD_MAX || ret == 0)
          {
            rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
            break;
          }

          string data;
          data.assign((const char *)set[0].getBin("data"), set[0].getBinSize("data"));
          if (uncompress(data, data) == false)
          {
            rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
            break;
          }

          BlobData oUserData;
          if (oUserData.ParseFromString(data) == false)
          {
            rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
            break;
          }

          QueryUserInfo* pInfo = rev.mutable_info();
          if (pInfo == nullptr)
          {
            rev.set_msgid(ESYSTEMMSG_ID_QUERY_CLOSE);
            break;
          }
          pInfo->set_charid(set[0].get<QWORD>("charid"));
          pInfo->set_name(set[0].getString("name"));

          pInfo->set_guildid(pTargetGChar->getGuildID());
          pInfo->set_guildname(pTargetGChar->getGuildName());

          EGender eGender = static_cast<EGender>(set[0].get<DWORD>("gender"));
          EProfession eProfession = static_cast<EProfession>(set[0].get<DWORD>("profession"));
          add_data(pInfo->add_datas(), EUSERDATATYPE_SEX, eGender);
          add_data(pInfo->add_datas(), EUSERDATATYPE_PROFESSION, eProfession);
          add_data(pInfo->add_datas(), EUSERDATATYPE_ROLELEVEL, set[0].get<DWORD>("rolelv"));
          add_data(pInfo->add_datas(), EUSERDATATYPE_HAIR, set[0].get<DWORD>("hair"));
          add_data(pInfo->add_datas(), EUSERDATATYPE_HAIRCOLOR, set[0].get<DWORD>("haircolor"));
          add_data(pInfo->add_datas(), EUSERDATATYPE_BODY, set[0].get<DWORD>("body"));

          const BlobEye& rEye = oUserData.eye();
          DWORD dwEyeID = rEye.curid();
          if (dwEyeID == 0)
          {
            const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(eProfession);
            if (pCFG != nullptr)
              dwEyeID = eGender == EGENDER_MALE ? pCFG->maleEye : pCFG->femaleEye;
            else
              XERR << "[玩家-信息查询]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                << "查询" << rev.charid() << "职业" << eProfession << "未在 Table_Class.txt 表中找到,无法设置默认美瞳" << XEND;
          }
          add_data(pInfo->add_datas(), EUSERDATATYPE_EYE, dwEyeID);
          add_data(pInfo->add_datas(), EUSERDATATYPE_CLOTHCOLOR, oUserData.user().clothcolor());

          pInfo->clear_attrs();
          const vector<EAttrType>& vecAttrs = MiscConfig::getMe().getAttrCFG().vecShowAttrs;
          for (int i = 0; i < oUserData.attr().datas_size(); ++i)
          {
            const UserAttrSvr& rAttr = oUserData.attr().datas(i);
            auto o = find(vecAttrs.begin(), vecAttrs.end(), rAttr.type());
            if (o == vecAttrs.end())
              continue;
            UserAttr* pAttr = pInfo->add_attrs();
            if (pAttr == nullptr)
              continue;
            const RoleData* pData = RoleDataConfig::getMe().getRoleData(rAttr.type());
            if (pData != nullptr && pData->bPercent)
              pAttr->set_value(rAttr.value() * FLOAT_TO_DWORD);
            else
              pAttr->set_value(rAttr.value());
            pAttr->set_type(rAttr.type());
          }

          pInfo->clear_equip();
          pInfo->clear_fashion();
          map<EUserDataType, DWORD> mapEquipShow;
          const BlobPack& pack = oUserData.pack();
          for (int i = 0; i < pack.datas_size(); ++i)
          {
            const PackageData& rData = pack.datas(i);
            if (rData.type() == EPACKTYPE_EQUIP || rData.type() == EPACKTYPE_FASHIONEQUIP)
            {
              for (int j = 0; j < rData.items_size(); ++j)
              {
                const ItemData& rItem = rData.items(j);
                if (rData.type() == EPACKTYPE_EQUIP)
                  pInfo->add_equip()->CopyFrom(rItem);
                else if (rData.type() == EPACKTYPE_FASHIONEQUIP)
                  pInfo->add_fashion()->CopyFrom(rItem);

                if (rItem.base().index() == EEQUIPPOS_HEAD)
                  mapEquipShow[EUSERDATATYPE_HEAD] = rItem.base().id();
                if (rItem.base().index() == EEQUIPPOS_BACK)
                  mapEquipShow[EUSERDATATYPE_BACK] = rItem.base().id();
                if (rItem.base().index() == EEQUIPPOS_FACE)
                  mapEquipShow[EUSERDATATYPE_FACE] = rItem.base().id();
                if (rItem.base().index() == EEQUIPPOS_TAIL)
                  mapEquipShow[EUSERDATATYPE_TAIL] = rItem.base().id();
                if (rItem.base().index() == EEQUIPPOS_MOUNT)
                  mapEquipShow[EUSERDATATYPE_MOUNT] = rItem.base().id();
                if (rItem.base().index() == EEQUIPPOS_MOUTH)
                  mapEquipShow[EUSERDATATYPE_MOUTH] = rItem.base().id();
              }
            }
          }
          for (auto &m : mapEquipShow)
            add_data(pInfo->add_datas(), m.first, m.second);

          pInfo->clear_highrefine();
          const BlobHighRefine& rHighRefine = oUserData.highrefine();
          for (int i = 0; i < rHighRefine.datas_size(); ++i)
          {
            pInfo->add_highrefine()->CopyFrom(rHighRefine.datas(i));
          }

          //process wedding 
          {
            if (pTargetGChar->getQueryWeddingType() == EQUERYTYPE_WEDDING_ALL ||
              (pTargetGChar->getQueryWeddingType() == EQUERYTYPE_WEDDING_FRIEND && pTargetGChar->checkRelation(pUser->id, ESOCIALRELATION_FRIEND)))
            {
              pInfo->set_partner(pTargetGChar->getWeddingPartner());
            }
          }

          RedisManager::getMe().setProtoData(key, pInfo, 300);
        } while(0);

        PROTOBUF(rev, send, len2);
        pUser->sendCmdToMe(send, len2);
        XDBG << "[玩家信息-查询]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "查询" << rev.charid() << "结果为" << rev.ShortDebugString() << XEND;
      }
      break;
    case CHATPARAM_QUERY_VOICE:
      break;
    case CHATPARAM_PLAYEXPRESSION:
      pUser->sendUserCmdToTeamServer(buf, len, ECMDTYPE_CHAT);
      break;
    case CHATPARAM_QUERY_REALTIME_VOICE_ID:
      {
        PARSE_CMD_PROTOBUF(QueryRealtimeVoiceIDCmd, rev);
        switch (rev.channel())
        {
          case ECHAT_CHANNEL_TEAM:
            {
              pUser->sendUserCmdToTeamServer(buf, len, ECMDTYPE_CHAT);
            }
            break;
          case ECHAT_CHANNEL_GUILD:
            {
              pUser->sendUserCmdToGuildServer(buf, len, ECMDTYPE_CHAT);
            }
            break;
          default:
            {
              XERR << "[实时语音-查询房间号]" << pUser->accid << pUser->id << pUser->name << "频道:" << rev.channel() << "不支持该频道" << XEND;
            }
            return true;
        }
      }
      break;
    default:
      return false;
  }

  return true;
}

void ChatManager_SE::timer(DWORD curTime)
{
  queryZoneStatus(curTime);
}

void ChatManager_SE::sendAndDelMsg(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
  auto it = m_mapCharID2Msgs.find(pUser->id);
  if (it == m_mapCharID2Msgs.end())
    return;
  TSetQWORD delIds;
  for (auto pMsg : it->second)
  {
    if (pMsg == nullptr)
      continue;
    if (pMsg->send(pUser))
      delIds.insert(pMsg->getId());
    SAFE_DELETE(pMsg);
  }
  delDb(delIds);
  it->second.clear();
}

void ChatManager_SE::eraseMapCharID2Msgs(QWORD charid)
{
  m_mapCharID2Msgs[charid].clear();
}

void ChatManager_SE::addMapCharID2Msgs(QWORD charid, TVecOfflineMsg &vec)
{
  for (auto& pMsg : vec)
  {
    if (pMsg == nullptr)
      continue;
    m_mapCharID2Msgs[charid].push_back(pMsg);
  }
}

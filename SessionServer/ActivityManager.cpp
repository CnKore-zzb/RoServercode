#include "ActivityManager.h"
#include "RedisManager.h"
#include "SessionServer.h"
#include "TableManager.h"

ActivityManager::ActivityManager()
{
}

ActivityManager::~ActivityManager()
{
}

bool ActivityManager::load()
{
  updateActivity();
  XLOG << "[活动面板-加载] 加载成功" << XEND;
  return true;
}

void ActivityManager::updateActivity()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ACTIVITY_INFO, "data");
  OperActivityNtfSocialCmd cmd;
  if (RedisManager::getMe().getProtoData(key, &cmd) == false)
  {
    XERR << "[活动面板-更新缓存] key:" << key << "redis读取失败" << XEND;
    return;
  }
  m_oActivity.CopyFrom(cmd);
  updateShortcutPower();
  XLOG << "[活动面板-更新缓存] 更新成功" << XEND;

  notifyAllUser();
}

void ActivityManager::updateShortcutPower()
{
  for (int i = 0; i < m_oActivity.activity_size(); ++i)
  {
    OperActivity* pAct = m_oActivity.mutable_activity(i);
    if (pAct == nullptr)
      continue;
    for (int j = 0; j < pAct->sub_activity_size(); ++j)
    {
      OperSubActivity* pSAct = pAct->mutable_sub_activity(j);
      if (pSAct == nullptr)
        continue;
      if (pSAct->pathid() != 0)
      {
        const SActShortcutPower* pCfg = TableManager::getMe().getActShortcutPowerCFG(pSAct->pathid());
        if (pCfg == nullptr)
        {
          XERR << "[活动面板-更新寻路] id:" << pSAct->pathid() << "配置找不到" << XEND;
          continue;
        }
        pSAct->set_pathtype(pCfg->getType());
        pSAct->set_pathevent(pCfg->getEvent());
      }
    }
  }
}

void ActivityManager::notifyUser(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
  PROTOBUF(m_oActivity, send, len);
  pUser->sendCmdToMe(send, len);
  XLOG << "[活动面板-推送玩家]" << pUser->accid << pUser->id << pUser->name << " 成功" << XEND;
  XDBG << "[活动面板-推送玩家]" << pUser->accid << pUser->id << pUser->name << m_oActivity.ShortDebugString() << " 成功" << XEND;
}

void ActivityManager::notifyAllUser()
{
  PROTOBUF(m_oActivity, send, len);
  MsgManager::sendWorldCmd(send, len);
  XLOG << "[活动面板-推送全部玩家] 成功" << XEND;
  XDBG << "[活动面板-推送全部玩家] 成功" << m_oActivity.ShortDebugString() << XEND;
}

bool ActivityEventManager::load()
{
  updateEvent();
  XLOG << "[活动模板-加载] 加载成功" << XEND;
  return true;
}

void ActivityEventManager::updateEvent()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ACTIVITY_EVENT, "data");
  ActivityEventNtfSessionCmd cmd;
  if (RedisManager::getMe().getProtoData(key, &cmd) == false)
  {
    XERR << "[活动模板-更新缓存] key:" << key << "redis读取失败" << XEND;
    return;
  }

  m_oAEToScene.CopyFrom(cmd);
  XLOG << "[活动模板-更新缓存] 更新成功" <<"msg"<<m_oAEToScene.ShortDebugString() << XEND;

  // 更新需要同步前端的数据
  m_oAEToClient.Clear();
  for (int i = 0; i < m_oAEToScene.infos_size(); ++i)
  {
    switch (m_oAEToScene.infos(i).type())
    {
    case EACTIVITYEVENTTYPE_FREE_TRANSFER:
    {
      if (m_oAEToScene.infos(i).has_freetransferinfo())
      {
        ActivityEvent* event = m_oAEToClient.add_events();
        if (event)
        {
          event->set_id(m_oAEToScene.infos(i).id());
          event->set_type(m_oAEToScene.infos(i).type());
          event->set_begintime(m_oAEToScene.infos(i).begintime());
          event->set_endtime(m_oAEToScene.infos(i).endtime());
          event->mutable_freetransfer()->CopyFrom(m_oAEToScene.infos(i).freetransferinfo());
        }
      }
      break;
    }
    case EACTIVITYEVENTTYPE_REWARD:
    {
      if (m_oAEToScene.infos(i).has_rewardinfo())
      {
        ActivityEvent* event = m_oAEToClient.add_events();
        AERewardInfo* info = event ? event->add_reward() : nullptr;
        if (info)
        {
          event->set_id(m_oAEToScene.infos(i).id());
          event->set_type(m_oAEToScene.infos(i).type());
          event->set_begintime(m_oAEToScene.infos(i).begintime());
          event->set_endtime(m_oAEToScene.infos(i).endtime());
          info->CopyFrom(m_oAEToScene.infos(i).rewardinfo());
        }
      }
      break;
    }
    case EACTIVITYEVENTTYPE_SUMMON:
    case EACTIVITYEVENTTYPE_LOTTERY_NPC:
      break;
    case EACTIVITYEVENTTYPE_RESETTIME:
    {
      ActivityEvent* event = m_oAEToClient.add_events();
      AERewardInfo* info = event ? event->add_reward() : nullptr;
      if (info)
      {
        event->set_id(m_oAEToScene.infos(i).id());
        event->set_type(m_oAEToScene.infos(i).type());
        event->set_begintime(m_oAEToScene.infos(i).begintime());
        event->set_endtime(m_oAEToScene.infos(i).endtime());
        info->CopyFrom(m_oAEToScene.infos(i).resetinfo());
      }
    }
    break;
    case EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT:
    {
      ActivityEvent* event = m_oAEToClient.add_events();
      AELotteryDiscount* info = event ? event->mutable_lotterydiscount(): nullptr;
      if (info)
      {
        event->set_id(m_oAEToScene.infos(i).id());
        event->set_type(m_oAEToScene.infos(i).type());
        event->set_begintime(m_oAEToScene.infos(i).begintime());
        event->set_endtime(m_oAEToScene.infos(i).endtime());
        info->CopyFrom(m_oAEToScene.infos(i).lotterydiscount());
      }
    }
    break;
    case EACTIVITYEVENTTYPE_LOTTERY_BANNER:
    {
      ActivityEvent* event = m_oAEToClient.add_events();
      AELotteryBanner* info = event ? event->mutable_lotterybanner() : nullptr;
      if (info)
      {
        event->set_id(m_oAEToScene.infos(i).id());
        event->set_type(m_oAEToScene.infos(i).type());
        event->set_begintime(m_oAEToScene.infos(i).begintime());
        event->set_endtime(m_oAEToScene.infos(i).endtime());
        info->CopyFrom(m_oAEToScene.infos(i).lotterybanner());
      }
    }
    break;
    case EACTIVITYEVENTTYPE_GUILD_BUILDING_SUBMIT:
    {
      if (m_oAEToScene.infos(i).has_gbuildingsubmitinfo())
      {
        ActivityEvent* event = m_oAEToClient.add_events();
        if (event)
        {
          event->set_type(m_oAEToScene.infos(i).type());
          event->set_begintime(m_oAEToScene.infos(i).begintime());
          event->set_endtime(m_oAEToScene.infos(i).endtime());
          event->mutable_gbuildingsubmit()->CopyFrom(m_oAEToScene.infos(i).gbuildingsubmitinfo());
        }
      }
      break;
    }
    default:
      continue;
    }
  }

  notifyAllUser();
  notifyAllScene();
}

void ActivityEventManager::notifyUser(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
  PROTOBUF(m_oAEToClient, send, len);
  pUser->sendCmdToMe(send, len);
  XLOG << "[活动模板-推送玩家]" << pUser->accid << pUser->id << pUser->name << " 成功" << XEND;
  XDBG << "[活动模板-推送玩家]" << pUser->accid << pUser->id << pUser->name << " 成功" <<"msg"<<m_oAEToClient.ShortDebugString() << XEND;
}

void ActivityEventManager::notifyAllUser()
{
  PROTOBUF(m_oAEToClient, send, len);
  MsgManager::sendWorldCmd(send, len);
  XLOG << "[活动模板-推送全部玩家] 成功" << XEND;
  XDBG << "[活动模板-推送全部玩家] 成功" << "msg" << m_oAEToClient.ShortDebugString() << XEND;
}

void ActivityEventManager::notifyScene(ServerTask* task)
{
  if (task == nullptr)
    return;
  PROTOBUF(m_oAEToScene, send, len);
  task->sendCmd(send, len);
  XLOG << "[活动模板-推送场景]" << task->getName() << "成功" << XEND;
  XDBG << "[活动模板-推送场景]" << task->getName() << "成功" <<"msg" <<m_oAEToScene.ShortDebugString() << XEND;
}

void ActivityEventManager::notifyAllScene()
{
  PROTOBUF(m_oAEToScene, send, len);
  thisServer->broadcastScene((unsigned char *)send, len);
  XLOG << "[活动模板-推送场景] 成功" << XEND;
  XDBG << "[活动模板-推送场景] 成功" << "msg" << m_oAEToScene.ShortDebugString() << XEND;
}

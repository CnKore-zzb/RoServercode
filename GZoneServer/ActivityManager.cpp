#include "ActivityManager.h"
#include "xDBFields.h"
#include "SessionCmd.pb.h"
#include "RedisManager.h"
#include "GZoneServer.h"
#include "BaseConfig.h"

void SActivityInfo::toOperActivity(OperActivity* pData)
{
  if (pData == nullptr)
    return;
  pData->set_id(dwId);
  pData->set_name(strName);
  pData->set_iconurl(strIconUrl);
  pData->set_begintime(dwBeginTime);
  pData->set_endtime(dwEndTime);
  pData->set_url(strUrl);
  pData->set_countdown(bCountdown);
  pData->mutable_data()->CopyFrom(oData);
}

void SSubActivityInfo::toOperSubActivity(OperSubActivity* pData)
{
  if (pData == nullptr)
    return;
  pData->set_id(dwId);
  pData->set_groupid(dwGroupId);
  pData->set_name(strName);
  pData->set_begintime(dwBeginTime);
  pData->set_endtime(dwEndTime);
  pData->set_pathid(dwPath);
  pData->set_url(strUrl);
  pData->set_pic_url(strPicUrl);
  pData->mutable_data()->CopyFrom(oData);
}

ActivityManager::ActivityManager()
{
}

ActivityManager::~ActivityManager()
{
}

bool ActivityManager::load()
{
  bool update = false;
  DWORD cur = now();

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "activity");
  if (field == nullptr)
  {
    XERR << "[活动面板-加载] activity数据库表获取失败" << XEND;
    return false;
  }
  field->setValid("id,name,iconurl,begintime,endtime,url,countdown,data,md5");
  stringstream sstr;
  sstr << "endtime>" << cur;

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str(), NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[活动面板-加载] activity表查询失败" << XEND;
    return false;
  }
  for (DWORD i = 0; i < ret; ++i)
  {
    SActivityInfo ai;
    ai.dwId = set[i].get<DWORD>("id");
    ai.strName = set[i].getString("name");
    ai.strIconUrl = set[i].getString("iconurl");
    ai.dwBeginTime = set[i].get<DWORD>("begintime");
    ai.dwEndTime = set[i].get<DWORD>("endtime");
    ai.strUrl = set[i].getString("url");
    ai.bCountdown = set[i].get<DWORD>("countdown") != 0;
    ai.strMd5 = set[i].getString("md5");

    string data;
    data.assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));
    if (ai.oData.ParseFromString(data) == false)
    {
      XERR << "[活动面板-加载] id:" << ai.dwId << "活动data数据解析出错" << XEND;
      continue;
    }

    auto it = m_mapAct.find(ai.dwId);
    if (it != m_mapAct.end() && it->second == ai)
    {
      it->second.updated = true;
      continue;
    }

    m_mapAct[ai.dwId] = ai;
    update = true;
  }
  for (auto it = m_mapAct.begin(); it != m_mapAct.end();)
  {
    if (it->second.dwEndTime <= cur || it->second.updated == false)
    {
      it = m_mapAct.erase(it);
      update = true;
      continue;
    }
    it->second.updated = false;
    ++it;
  }

  field = thisServer->getDBConnPool().getField(REGION_DB, "activity_detail");
  if (field == nullptr)
  {
    XERR << "[活动面板-加载] activity_detail数据库表获取失败" << XEND;
    return false;
  }
  field->setValid("id,groupid,name,begintime,endtime,path,url,pic_url,data,md5");
  sstr.str("");
  sstr << "endtime>" << cur << " order by groupid,`order`,begintime,id";

  set.clear();
  ret = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str(), NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[活动面板-加载] activity_tail表查询失败" << XEND;
    return false;
  }
  m_vecSubActId.clear();
  for (DWORD i = 0; i < ret; ++i)
  {
    SSubActivityInfo sai;
    sai.dwId = set[i].get<DWORD>("id");
    sai.dwGroupId = set[i].get<DWORD>("groupid");
    sai.strName = set[i].getString("name");
    sai.dwBeginTime = set[i].get<DWORD>("begintime");
    sai.dwEndTime = set[i].get<DWORD>("endtime");
    sai.dwPath = set[i].get<DWORD>("path");
    sai.strUrl = set[i].getString("url");
    sai.strPicUrl = set[i].getString("pic_url");
    sai.strMd5 = set[i].getString("md5");

    string data;
    data.assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));
    if (sai.oData.ParseFromString(data) == false)
    {
      XERR << "[活动面板-加载] id:" << sai.dwId << "子活动data数据解析出错" << XEND;
      continue;
    }

    m_vecSubActId.push_back(sai.dwId);

    auto it = m_mapSubAct.find(sai.dwId);
    if (it != m_mapSubAct.end() && it->second == sai)
    {
      it->second.updated = true;
      continue;
    }

    m_mapSubAct[sai.dwId] = sai;
    update = true;
  }
  for (auto it = m_mapSubAct.begin(); it != m_mapSubAct.end();)
  {
    if (it->second.dwEndTime <= cur || it->second.updated == false)
    {
      it = m_mapSubAct.erase(it);
      update = true;
      continue;
    }
    it->second.updated = false;
    ++it;
  }

  if (update)
  {
    XLOG << "[活动面板-加载] 加载成功,数据变化,更新redis" << XEND;
    updateRedis();
    notifySession();
  }
  else
  {
    XLOG << "[活动面板-加载] 加载成功,数据未变化" << XEND;
  }

  return true;
}

void ActivityManager::updateRedis()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ACTIVITY_INFO, "data");

  map<DWORD, OperActivity*> id2act;
  OperActivityNtfSocialCmd cmd;
  for (auto& v : m_mapAct)
  {
    OperActivity* p = cmd.add_activity();
    if (p == nullptr)
      continue;
    v.second.toOperActivity(p);
    id2act[v.first] = p;
  }
  for (auto& id : m_vecSubActId)
  {
    auto it = m_mapSubAct.find(id);
    if (it == m_mapSubAct.end())
      continue;
    auto itact = id2act.find(it->second.dwGroupId);
    if (itact == id2act.end())
      continue;
    it->second.toOperSubActivity(itact->second->add_sub_activity());
  }

  if (RedisManager::getMe().setProtoData(key, &cmd) == false)
  {
    XERR << "[活动面板-更新redis] key:" << key << "更新失败" << XEND;
    return;
  }
  XLOG << "[活动面板-更新redis] key:" << key << "更新成功" << XEND;
}

void ActivityManager::notifySession()
{
  UpdateOperActivitySessionCmd cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllZone(send, len);
  XLOG << "[活动面板-通知SessionServer] 成功" << XEND;
}

bool ActivityEventManager::load()
{
  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "activity_event");
  if (field == nullptr)
  {
    XERR << "[活动模板-加载] activity_event数据库表获取失败" << XEND;
    return false;
  }

  DWORD cur = now();
  stringstream sstr;
  string timeprefix;

  if (BaseConfig::getMe().getInnerBranch() == BRANCH_PUBLISH)
  {
    field->setValid("id,release_begintime,release_endtime,`type`,`data`,md5");
    sstr << "release_endtime>" << cur;
    timeprefix = "release_";
  }
  else
  {
    field->setValid("id,tf_begintime,tf_endtime,`type`,`data`,md5");
    sstr << "tf_endtime>" << cur;
    timeprefix = "tf_";
  }

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, sstr.str().c_str(), NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[活动模板-加载] activity_event表查询失败" << XEND;
    return false;
  }

  bool update = false;
  for (DWORD i = 0; i < ret; ++i)
  {
    SActivityEventInfo aei;
    aei.dwID = set[i].get<DWORD>("id");
    aei.strMd5 = set[i].getString("md5");

    aei.oInfo.set_id(set[i].get<DWORD>("id"));
    aei.oInfo.set_begintime(set[i].get<DWORD>(timeprefix + "begintime"));
    aei.oInfo.set_endtime(set[i].get<DWORD>(timeprefix + "endtime"));

    DWORD type = set[i].get<DWORD>("type");
    if (type <= EACTIVITYEVENTTYPE_MIN || type >= EACTIVITYEVENTTYPE_MAX || EActivityEventType_IsValid(type) == false)
    {
      XERR << "[活动模板-加载] id:" << aei.dwID << "type非法" << XEND;
      continue;
    }
    aei.oInfo.set_type(static_cast<EActivityEventType>(type));

    string info;
    info.assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));

    switch (aei.oInfo.type())
    {
    case EACTIVITYEVENTTYPE_FREE_TRANSFER:
      if (aei.oInfo.mutable_freetransferinfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    case EACTIVITYEVENTTYPE_SUMMON:
      if (aei.oInfo.mutable_summoninfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    case EACTIVITYEVENTTYPE_REWARD:
      if (aei.oInfo.mutable_rewardinfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    case EACTIVITYEVENTTYPE_RESETTIME:
      if (aei.oInfo.mutable_resetinfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    case EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT:
    {
      if (aei.oInfo.mutable_lotterydiscount()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
    }
    break;
    case EACTIVITYEVENTTYPE_GUILD_BUILDING_SUBMIT:
      if (aei.oInfo.mutable_gbuildingsubmitinfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    case EACTIVITYEVENTTYPE_LOTTERY_NPC:
    {
      if (aei.oInfo.mutable_lotterynpc()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
    }
    break;
    case EACTIVITYEVENTTYPE_LOTTERY_BANNER:
    {
      if (aei.oInfo.mutable_lotterybanner()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
    }
    break;
    case EACTIVITYEVENTTYPE_SHOP:
      if (aei.oInfo.mutable_shopinfo()->ParseFromString(info) == false)
      {
        XERR << "[活动模板-加载] id:" << aei.dwID << "活动数据解析出错" << XEND;
        continue;
      }
      break;
    default:
      XERR << "[活动模板-加载] id:" << aei.dwID << "type非法" << XEND;
      continue;
    }

    auto it = m_mapEvent.find(aei.dwID);
    if (it != m_mapEvent.end() && it->second == aei)
    {
      it->second.updated = true;
      continue;
    }

    m_mapEvent[aei.dwID] = aei;
    update = true;
  }

  for (auto it = m_mapEvent.begin(); it != m_mapEvent.end();)
  {
    if (it->second.updated == false)
    {
      it = m_mapEvent.erase(it);
      update = true;
      continue;
    }
    it->second.updated = false;
    ++it;
  }

  if (update)
  {
    XLOG << "[活动模板-加载] 加载成功,数据变化,更新redis" << XEND;
    updateRedis();
    notifySession();
  }
  else
  {
    XLOG << "[活动模板-加载] 加载成功,数据未变化" << XEND;
  }

  return true;
}

void ActivityEventManager::updateRedis()
{
  ActivityEventNtfSessionCmd cmd;

  for (auto& v : m_mapEvent)
  {
    ActivityEventInfo* p = cmd.add_infos();
    if (p == nullptr)
      continue;
    v.second.toActivityEventInfo(p);
  }

  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ACTIVITY_EVENT, "data");
  if (RedisManager::getMe().setProtoData(key, &cmd) == false)
  {
    XERR << "[活动模板-更新redis] key:" << key << "更新失败" << XEND;
    return;
  }
  XLOG << "[活动模板-更新redis] key:" << key << "更新成功" <<"msg"<< cmd.ShortDebugString() << XEND;
}

void ActivityEventManager::notifySession()
{
  UpdateActivityEventSessionCmd cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllZone(send, len);
  XLOG << "[活动模板-通知SessionServer] 成功" << XEND;
}

#include "SessionActivityMgr.h"
#include "xDBFields.h"
#include "SessionServer.h"
#include "RecordCmd.pb.h"
#include "SessionUser.h"
#include "SessionUserManager.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "ActivityCmd.pb.h"
#include "LuaManager.h"
#include "MsgManager.h"
#include "ActivityConfig.h"
#include "MatchSCmd.pb.h"
#include "TimerM.h"

void ActivityItem::packProto(StartActCmd& cmd)
{
  cmd.Clear();
  cmd.set_id(m_id);
  cmd.set_mapid(m_mapId);
  cmd.set_starttime(m_startTime);
  cmd.set_endtime(m_endTime);
  if (m_dwPath)
    cmd.set_path(m_dwPath);
  
  for (auto &v : m_vecUnshowMap)
  {
    cmd.add_unshowmap(v);
  }
}


SessionActivityMgr::SessionActivityMgr()
{
}

SessionActivityMgr::~SessionActivityMgr()
{
}

void SessionActivityMgr::init()
{
  if (m_bInit)
    return;
  m_mapTimers.clear();
  m_listRetry.clear();
  auto func = [this](DWORD id, SActivityCFG& rCfg) {
    string type = rCfg.m_condition.getTableString("type");
    if (type == "time")
    {
      if (rCfg.m_vecStartTime.empty())
        return;
      if (rCfg.m_duration)
      {
        xTimer2 timer(rCfg.m_vecStartTime, rCfg.m_duration);
        //m_mapTimers[id] = timer;
        m_mapTimers.insert(std::make_pair(id, timer));
      }
      else
      {
        xTimer2 timer(rCfg.m_vecStartTime);
        //m_mapTimers[id] = timer;
        m_mapTimers.insert(std::make_pair(id, timer));
      }
    }
    else if (type == "init")
    {
      //start activity
      for (auto &v : rCfg.m_vecMapId)
      {
        ActivityItem* ptr = startActivity(static_cast<EActivityType>(id), v, true);
        if (!ptr)
        {
          m_listRetry.push_back(std::make_pair(id, v));
        }
      }
    }
  };
  ActivityConfig::getMe().foreach(func);
  m_bInit = true;
}

void SessionActivityMgr::reload()
{
  m_mapTimers.clear();
  m_listRetry.clear();
  auto func = [this](DWORD id, SActivityCFG& rCfg) {
    string type = rCfg.m_condition.getTableString("type");
    if (type == "time")
    {
      if (rCfg.m_vecStartTime.empty())
        return;
      xTimer2 timer(rCfg.m_vecStartTime);
      m_mapTimers.insert(std::make_pair(id, timer));
    }
  };
  ActivityConfig::getMe().foreach(func);
  XLOG << "[活动] 配置 重加载成功" << XEND;
}

void SessionActivityMgr::testAndSet(ActivityTestAndSetSessionCmd rev)
{
  XLOG << "[活动-发起] " << rev.id() << rev.mapid() << rev.charid() << XEND;

  SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(rev.mapid());
  if (!pScene)
  {
    XERR << "[活动-发起]，失败 " << rev.id() << rev.mapid() << rev.charid() <<"找不到对应mapid 的scene"<< XEND;
    return;
  }
 
  ActivityItem* pActivityItem = startActivity(rev.id(), rev.mapid(), false, rev.charid());
  if (pActivityItem)
  {
    rev.set_starttime(pActivityItem->m_startTime);
    rev.set_uid(pActivityItem->m_uid);
    rev.set_ret(1);
  }else 
  {
    rev.set_ret(0);
  }
  PROTOBUF(rev, send, len);
  pScene->sendCmd(send, len);
}

void SessionActivityMgr::stop(const ActivityStopSessionCmd& rev)
{
  ActivityItem item ;
  auto it = m_mapActvity.find(rev.mapid());
  if (it != m_mapActvity.end())
  {
    for (auto&v : it->second)
    {
      if (v.m_uid == rev.uid())
      {
        v.m_bStop = true;
        item = v;
      }
    }
  }
  
  if (needWorldNtf(item.m_type))
  {
    StopActCmd cmd;
    cmd.set_id(rev.id());
    PROTOBUF(cmd, send, len);
    MsgManager::sendWorldCmd(send, len);
  }
  
  if (item.m_type == EACTIVITYTYPE_POLLY)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(false);
    cmd.set_etype(EPVPTYPE_POLLY);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
  }
  else if (item.m_type == EACTIVITYTYPE_MVPBATTLE)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(false);
    cmd.set_etype(EPVPTYPE_MVP);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);

    // 通知场景关闭副本
    thisServer->sendCmdToAllScene(send, len);
  }
  else if (item.m_type == EACTIVITYTYPE_TEAMPWS)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(false);
    cmd.set_etype(EPVPTYPE_TEAMPWS);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
  }

  XLOG << "[活动-结束] 提前结束 uid:" << item.m_uid << "id:" << item.m_id << "mapid:" << item.m_mapId << "starttime:" << item.m_startTime << "endtime:" << item.m_endTime << XEND;
}

//bool SessionActivityMgr::sendActStatusScene(EActivityType eType, DWORD dwMapID, bool start)
//{
//  SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(dwMapID);
//  if (pScene == nullptr)
//    return false;
//  ActivityStatusSessionCmd cmd;
//  cmd.set_type(eType);
//  cmd.set_mapid(dwMapID);
//  cmd.set_start(start);
//  PROTOBUF(cmd, send, len);
//  return pScene->sendCmd(send, len);
//}

bool SessionActivityMgr::sendActStatusUser(SessionUser* pUser, ActivityItem& actItem)
{
  if (pUser == nullptr)
    return false;

  if (needWorldNtf(actItem.m_type))
  {
    StartActCmd cmd;
    actItem.packProto(cmd);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }

  return true;
}

void SessionActivityMgr::timer(DWORD curSec)
{
  if (thisServer->getServerState() != ServerState::run)
    return;
  checkStartActivity(curSec);
  checkStopActivity(curSec);

  //retry  
  for (auto it = m_listRetry.begin(); it != m_listRetry.end();)
  {
    ActivityItem* ptr = startActivity(it->first, it->second, true);
    if (ptr)
    {
      it = m_listRetry.erase(it);
    }
    else
    {
      it++;
    }
  }
}

DWORD SessionActivityMgr::calcActivityCount(EActivityType type, DWORD mapId)
{
  auto it = m_mapActvity.find(mapId);
  if (it == m_mapActvity.end())
    return 0;

  DWORD curSec = now();
  DWORD count = 0;
  for (auto subIt = it->second.begin(); subIt != it->second.end(); ++subIt)
  {
    if (subIt->m_type != type)
      continue;
    if (subIt->m_endTime > curSec)
      count++;
  }
  return count;
}

void SessionActivityMgr::checkStartActivity(DWORD curSec)
{
  for (auto it = m_mapTimers.begin(); it != m_mapTimers.end(); ++it)
  {
    if (it->second.timeUp(curSec, nullptr))
    {
      const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(it->first);
      if (!pCfg)
        continue;
      if (pCfg->m_limitType == EACTLIMITTYPE_RANDOMMAP)
      {
        DWORD mapId = pCfg->randomMap();
        if (mapId)
          startActivity(it->first, mapId, true);
      }
      else
      {
        for (auto &v : pCfg->m_vecMapId)
          startActivity(it->first, v, true);
      }
    }
  }
}

void SessionActivityMgr::checkStopActivity(DWORD curSec)
{
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end();)
    {
      if (subIt->m_bStop ||  (subIt->m_endTime <= curSec && subIt->m_startTime > subIt->m_endTime ))      //del expire   subIt->m_startTime == subIt->m_endTime 持续进行的活动 
      {
        XDBG << "[活动-删除过期] " << "uid:" << subIt->m_uid << "id:" << subIt->m_type << "starttime:" << subIt->m_startTime << "endtime:" << subIt->m_endTime<<"bstop"<< subIt->m_bStop << XEND;
        subIt = it->second.erase(subIt);
        continue;
      }
      ++subIt;
    }
  }
}

ActivityItem* SessionActivityMgr::startActivity(DWORD id, DWORD mapId, bool bNotify, QWORD charId/* = 0*/)
{ 
  if (!checkCanStart(id, charId, mapId))
    return nullptr;

  SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(mapId);
  if (!pScene)
  {
    XERR << "[活动-发起] 失败,找不到对应mapid 的场景" << id << mapId << XEND;
    return nullptr;
  }
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(id);
  if (pCfg == nullptr)
  {
    XERR << "[活动-发起] 失败" << id << mapId << "找不到对应id 的策划表" << XEND;
    return nullptr;
  }

  auto &lis = m_mapActvity[mapId];
  ActivityItem item;
  item.m_id = id;
  item.m_type = pCfg->m_actType;
  item.m_uid = generateIndex();
  item.m_mapId = mapId;
  item.m_startTime = now();
  item.m_endTime = item.m_startTime + pCfg->m_duration;
  item.m_dwPath = pCfg->m_dwPath;
  item.m_vecUnshowMap = pCfg->m_vecUnshowMap;

  lis.push_back(item);
  
  if (bNotify)
  {
    ActivityTestAndSetSessionCmd cmd;
    cmd.set_id(item.m_id);
    cmd.set_uid(item.m_uid);
    cmd.set_mapid(mapId);
    cmd.set_starttime(item.m_startTime);
    cmd.set_ret(1);
    PROTOBUF(cmd, send, len);
    pScene->sendCmd(send, len);
  }
  
  if (needWorldNtf(item.m_type))   
  {
    StartActCmd cmd;
    item.packProto(cmd);
    PROTOBUF(cmd, send, len);
    MsgManager::sendWorldCmd(send, len);    
  }
  
  if (item.m_type == EACTIVITYTYPE_POLLY)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(true);
    cmd.set_etype(EPVPTYPE_POLLY);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
  }
  else if (item.m_type == EACTIVITYTYPE_MVPBATTLE)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(true);
    cmd.set_etype(EPVPTYPE_MVP);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
  }
  else if (item.m_type == EACTIVITYTYPE_TEAMPWS)
  {
    ActivityMatchSCmd cmd;
    cmd.set_open(true);
    cmd.set_etype(EPVPTYPE_TEAMPWS);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
  }

  XLOG << "[活动-开始] uid:" <<item.m_uid <<"id"<<item.m_id <<"acttype:"<<item.m_type<<"mapid:"<< item.m_mapId << "starttime:" << item.m_startTime << "endtime:" << item.m_endTime << XEND;
  return &lis.back();
}

QWORD SessionActivityMgr::generateIndex()
{
  DWORD curSec = now();
  m_index++;
  QWORD index = m_index << 32;
  index += curSec;  
  return index;
}

void SessionActivityMgr::onUserOnline(SessionUser* pUser)
{
  for (auto& v : m_mapActvity)
  {
    for (auto& subV : v.second)
    {
      sendActStatusUser(pUser, subV);
    }
  }
  for(auto& s : m_mapGlobalActivity)
    notifyGlobalAct(&s.second, pUser, true);
}

bool SessionActivityMgr::checkCanStart(DWORD id, QWORD charId, DWORD mapId)
{
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(id);
  if (pCfg == nullptr)
    return false;

  auto msgFunc = [](QWORD charId, DWORD msgId) {
    if (charId)
      MsgManager::sendMsg(charId, msgId);
  };

  EActivityType type = pCfg->m_actType;
  
  if (type == EACTIVITYTYPE_BCAT)
  {
    DWORD count = calcActivityCount(EACTIVITYTYPE_BCAT, mapId);
    if (count > 0)
    {
      XERR << "[活动-发起]，失败 " << type << mapId << charId << "对应地图已存在逼格猫入侵活动" << XEND;
      return false;
    }
    else
      return true;
  }
  
  if (type == EACTIVITYTYPE_CRAZYGHOST)
  {
    DWORD count = calcActivityCount(EACTIVITYTYPE_BCAT, mapId);
    if (count > 0)
    {
      msgFunc(charId, 879);
      XERR << "[活动-发起]，失败 " << type << mapId << charId << "对应地图已存在逼格猫入侵活动" << XEND;
      return false;
    }

    count = calcActivityCount(EACTIVITYTYPE_CRAZYGHOST, mapId);
    if (count > 0)
    {
      msgFunc(charId, 880);
      XERR << "[活动-发起]，失败 " << type << mapId << charId << "对应地图已存在幽灵入侵活动" << XEND;
      return false;
    }

    if (!checkMaintainTime(id))
    {
      msgFunc(charId, 889);
      XERR << "[活动-发起]，失败 " << type << mapId << charId << "邻近维护时间" << XEND;
      return false;
    }
  }
  
  return true;
}

bool SessionActivityMgr::checkMaintainTime(DWORD id)
{
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(id);
  if (pCfg == nullptr)
  {
    XERR << "[活动-发起] 失败" << id << "找不到对应id 的策划表" << XEND;
    return false;
  }
  DWORD startTime = now();
  DWORD endTime = startTime + pCfg->m_duration + 60;
  DWORD mainStartTime = 0;


  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "region");
  if (field)
  {
    field->setValid("maintainstart");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid=%u", thisServer->getRegionID());

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX != ret && ret)
    {
      std::string msStr = set[0].getString("maintainstart");
      if (msStr != "")
      {
        parseTime(msStr.c_str(), mainStartTime);
      }
    }
  }
  if (mainStartTime == 0)
    return true;

  if (endTime >= mainStartTime && startTime <= mainStartTime)
    return false;

  return true;
}

GlobalActivityItem* SessionActivityMgr::getGlobalActivity(DWORD id)
{
  auto it = m_mapGlobalActivity.find(id);
  if (it == m_mapGlobalActivity.end())
    return nullptr;
  return &(it->second);
}

bool SessionActivityMgr::addGlobalActivity(DWORD id, DWORD timerId)
{
  GlobalActivityItem* pGlobalAct = getGlobalActivity(id);
  if (pGlobalAct)
  {
    XLOG << "[全服活动-添加] 重新开启" << id << "已经开启" << XEND;
    return true;    //只重启场景服使用
  }  
  
  const SGlobalActCFG* pCfg = ActivityConfig::getMe().getGlobalActCFG(id);
  if (!pCfg)
  {
    XERR << "[全服活动-添加] 失败，找不到配置， id" << id <<"timerId"<< timerId << XEND;
    return false;
  }       

  pGlobalAct = &(m_mapGlobalActivity[id]);
  pGlobalAct->m_dwId = id;
  pGlobalAct->m_eType = pCfg->m_actType;
  pGlobalAct->m_dwTimerId = timerId;
  

  DWORD dwDepositId = pCfg->getDepositId();
  if (dwDepositId)
  {
    TSetDWORD& rSet = m_mapCharge2GlobalAct[dwDepositId];
    rSet.insert(id);
  }

  StartActCmd cmd;
  cmd.set_id(id);
  PROTOBUF(cmd, send, len);
  MsgManager::sendWorldCmd(send, len);

  notifyGlobalAct(pGlobalAct, nullptr, true);

  XLOG << "[全服活动-Session] 开启成功" << "添加" << id <<"timerid"<<timerId<<pCfg->m_strName <<"充值id"<< dwDepositId << XEND;
  return true;
}

bool SessionActivityMgr::delGlobalActivity(DWORD id)
{
  GlobalActivityItem* pGlobalAct = getGlobalActivity(id);
  if (!pGlobalAct)
  {
    XERR << "[全服活动-结束] " << id << "找不到活动" << XEND;
    return false;    
  }
    
  if(true)//needWorldNtf(pCfg->m_actType))
  {
    StopActCmd cmd;
    cmd.set_id(id);
    PROTOBUF(cmd, send, len);
    MsgManager::sendWorldCmd(send, len);
  }
  notifyGlobalAct(pGlobalAct, nullptr, false);
  XLOG << "[全服活动-Session] 结束成功" << "删除" << id <<pGlobalAct->m_dwTimerId << XEND;
  
  const SGlobalActCFG* pCfg = ActivityConfig::getMe().getGlobalActCFG(id);
  if (pCfg)
  {
    DWORD dwDepositId = pCfg->getDepositId();
    if (dwDepositId)
    {
      TSetDWORD& rSet = m_mapCharge2GlobalAct[dwDepositId];
      rSet.erase(pGlobalAct->m_dwId);
    }
  }
  //放到最后，删除后不要再使用pGlobalAct了。
  m_mapGlobalActivity.erase(id);
  return true;
}

bool SessionActivityMgr::isOpen(DWORD id)
{
  GlobalActivityItem* pGlobal = getGlobalActivity(id);
  if (pGlobal)
    return true;
  return false;
}

void SessionActivityMgr::notifyGlobalAct(GlobalActivityItem* pGlobalAct, SessionUser* pUser, bool startcmd)
{
  if (!pGlobalAct)
    return;
  const SGlobalActCFG* pCfg = ActivityConfig::getMe().getGlobalActCFG(pGlobalAct->m_dwId);
  if (!pCfg)
    return;

  if (startcmd)
  {
    GlobalActivityStartSessionCmd cmd;
    cmd.set_id(pGlobalAct->m_dwId);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToServer(send, len, "SocialServer");
  }
  else
  {
    GlobalActivityStopSessionCmd cmd;
    cmd.set_id(pGlobalAct->m_dwId);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToServer(send, len, "SocialServer");
  }

  StartGlobalActCmd scmd;
  scmd.set_id(pGlobalAct->m_dwId);  
  if (GlobalActivityType_IsValid(pGlobalAct->m_eType))
  {
    scmd.set_type(pGlobalAct->m_eType);
  }

  if(startcmd == true)
  {
    scmd.set_count(pCfg->m_dwLimitCount);
    for (auto &v : pCfg->m_vecParam)
    {
      scmd.add_params(v);
    }
    scmd.set_open(true);
    if (pGlobalAct->m_dwTimerId)
    {
      DWORD dwStartTime = 0;
      DWORD dwEndTime = 0;
      TimerM::getMe().getStartEndTime(pGlobalAct->m_dwTimerId, dwStartTime, dwEndTime);
      scmd.set_starttime(dwStartTime);

      //充值类活动，让客户端提前10分钟结束
      if (scmd.type() == GACTIVITY_CHARGE_DISCOUNT || scmd.type() == GACTIVITY_CHARGE_EXTRA_COUNT
        || scmd.type() == GACTIVITY_CHARGE_EXTRA_REWARD)
      {
        if (dwEndTime >= 10*60)
          dwEndTime -= 10 * 60;
      }
      
      scmd.set_endtime(dwEndTime);
    }
  }
  else
    scmd.set_open(false);

  PROTOBUF(scmd, ssend, slen);
  if(pUser == nullptr)
    MsgManager::sendWorldCmd(ssend, slen);
  else
    pUser->sendCmdToMe(ssend, slen);
}

void SessionActivityMgr::registRegion(ClientType type)
{
  if (type != ClientType::match_server) 
    return;
  
  for (auto it = m_mapActvity.begin(); it != m_mapActvity.end(); ++it)
  {
    for (auto subIt = it->second.begin(); subIt != it->second.end();)
    {
      if (subIt->m_type == EACTIVITYTYPE_POLLY)
      {
        ActivityMatchSCmd cmd;
        cmd.set_open(true);
        cmd.set_etype(EPVPTYPE_POLLY);
        cmd.set_server_restart(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::match_server, send, len);
      }
      else if (subIt->m_type == EACTIVITYTYPE_MVPBATTLE)
      {
        ActivityMatchSCmd cmd;
        cmd.set_open(true);
        cmd.set_etype(EPVPTYPE_MVP);
        cmd.set_server_restart(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::match_server, send, len);
      }
      else if (subIt->m_type == EACTIVITYTYPE_TEAMPWS)
      {
        ActivityMatchSCmd cmd;
        cmd.set_open(true);
        cmd.set_etype(EPVPTYPE_TEAMPWS);
        cmd.set_server_restart(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::match_server, send, len);
      }
      ++subIt;
    }
  }
}

void SessionActivityMgr::getActivityEffect(const SGlobalActCFG* pCFG,DWORD&dwRate)
{ 
  dwRate = 1;
  if (!pCFG)
    return;
  
  switch (pCFG->m_actType)
  {
  case GACTIVITY_CHARGE_EXTRA_REWARD:
    dwRate = pCFG->getParam(1);
  default:
    break;
  }
}

bool SessionActivityMgr::getUserActivityCnt(const SGlobalActCFG* pCfg, QWORD qwAccid, QWORD qwCharId, DWORD dwStartTime, DWORD&dwCount)
{
  if (!pCfg)
    return false;
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwCharId);
  if (pUser)
  {
    dwCount = pUser->getActivityCnt(pCfg->m_dwId);
    return true;
  }
  return getUserActivityCntFromDb(pCfg, qwAccid, qwCharId, dwStartTime, dwCount);
}

bool SessionActivityMgr::getUserActivityCntFromDb(const SGlobalActCFG* pCfg, QWORD qwAccid, QWORD qwCharId, DWORD dwStartTime, DWORD&dwCount)
{
  if (!pCfg)
    return false;

  DWORD dwActivityId = pCfg->m_dwId;
  dwCount = 0;
  xField field(REGION_DB, DB_GLOBALACT_RECORD);
  field.m_list["total_count"] = MYSQL_TYPE_NEWDECIMAL;

  char totalCountSql[128];
  bzero(totalCountSql, sizeof(totalCountSql));
  if (pCfg->m_eLimitType == EGlobalActLimitType_Acc)
  { //账号次数
    snprintf(totalCountSql, sizeof(totalCountSql), "select count(*) as total_count from %s.%s where id=%d and accid=%llu and time>=%d", field.m_strDatabase.c_str(), field.m_strTable.c_str(), dwActivityId, qwAccid, dwStartTime);
  }
  else if (pCfg->m_eLimitType == EGlobalActLimitType_Char)
  { //角色次数
    snprintf(totalCountSql, sizeof(totalCountSql), "select count(*) as total_count from %s.%s where id=%d and charid=%llu and time>=%d", field.m_strDatabase.c_str(), field.m_strTable.c_str(), dwActivityId, qwCharId, dwStartTime);
  }
  
  xRecordSet tmpSet;
  std::string sql(totalCountSql);
  QWORD ret = thisServer->getDBConnPool().exeRawSelect(&field, tmpSet, sql);
  if ((QWORD)-1 == ret)
  {
    XERR << "[活动参加次数-查询] 查询数据库出错， accid" <<qwAccid << "charid:" << qwCharId << "ret:" << ret << XEND;
    return false;
  }
  if (ret == 1)
  {
    dwCount = tmpSet[0].get<DWORD>("total_count");
  }
  XLOG << "[活动参加次数-查询] accid" << qwAccid << "charid:" << qwCharId << "活动id" << dwActivityId <<"开始时间"<<dwStartTime <<"次数" <<dwCount << XEND;
  return true;
}

bool SessionActivityMgr::getUserAllActivityCnt(QWORD qwAccid, QWORD qwCharId, std::map<DWORD, DWORD>&mapCnt)
{
  for (auto&m : m_mapGlobalActivity)
  {
    //has cnt limit 
    if (m.second.m_eType != GACTIVITY_CHARGE_EXTRA_REWARD && m.second.m_eType != GACTIVITY_CHARGE_DISCOUNT
      && m.second.m_eType != GACTIVITY_CHARGE_EXTRA_COUNT)
      continue;
    
    if (m.second.m_dwTimerId == 0)
      continue;
    DWORD dwStartTime = 0;
    DWORD dwEndTime = 0;
    TimerM::getMe().getStartEndTime(m.second.m_dwTimerId, dwStartTime, dwEndTime);
    DWORD dwCnt = 0;
    const SGlobalActCFG*pCfg = ActivityConfig::getMe().getGlobalActCFG(m.second.m_dwId);
    if (!pCfg)
      continue;
    if (getUserActivityCntFromDb(pCfg, qwAccid, qwCharId, dwStartTime, dwCnt) == false)
      continue;
    mapCnt[m.second.m_dwId] = dwCnt;
  }
  return true;
}

bool SessionActivityMgr::addUserActivityCnt(DWORD dwActivityId, QWORD qwAccid, QWORD qwCharId)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, DB_GLOBALACT_RECORD);
  if (!field)
    return false;

  xRecord record(field);
  record.put("id", dwActivityId);
  record.put("accid", qwAccid);
  record.put("charid", qwCharId);
  record.put("time", now());
 
  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
  if (ret == QWORD_MAX)
  {
    return false;
  }
  
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwCharId);
  if (pUser)
  {
    pUser->addActivityCnt(dwActivityId);
  }
  XLOG << "[活动参加次数-插入] accid" << qwAccid << "charid:" << qwCharId << "活动id" << dwActivityId << XEND;
  return true;
}

bool SessionActivityMgr::checkUserGlobalActCnt(GlobalActivityType actType, const SDeposit* pCFG, QWORD qwAccid, QWORD qwCharid, const SGlobalActCFG** pGlobalActCfg)
{
  if (!pCFG) return false;

  auto it = m_mapCharge2GlobalAct.find(pCFG->id);
  if (it == m_mapCharge2GlobalAct.end())
    return false;
  if (it->second.empty())
    return false;
  
  for (auto&v : it->second)
  {
    GlobalActivityItem* pGlobalAct = getGlobalActivity(v);
    if (!pGlobalAct)
      continue;
    if (pGlobalAct->m_eType != actType)
      continue;

    if (pGlobalAct->m_dwTimerId == 0)
      continue;
    *pGlobalActCfg = ActivityConfig::getMe().getGlobalActCFG(pGlobalAct->m_dwId);
    if (!(*pGlobalActCfg))
      continue;

    DWORD dwStartTime = 0;
    DWORD dwEndTime = 0;
    TimerM::getMe().getStartEndTime(pGlobalAct->m_dwTimerId, dwStartTime, dwEndTime);
    DWORD dwCnt = 0;
    if (getUserActivityCnt((*pGlobalActCfg), qwAccid, qwCharid, dwStartTime, dwCnt) == false)
      continue;
    if (dwCnt < (*pGlobalActCfg)->m_dwLimitCount)
    {
      return true;
    }     
  }
  (*pGlobalActCfg) = nullptr;
  return false;
}

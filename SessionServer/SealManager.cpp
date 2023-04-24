#include "SealManager.h"
#include "SessionUser.h"
#include "SceneSeal.pb.h"
#include "RecordCmd.pb.h"
#include "SessionUserManager.h"
#include "MsgManager.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "RedisManager.h"
#include "SealConfig.h"
#include "SessionTeam.pb.h"

extern SessionServer *thisServer;

void SSealData2::loadTeamInfo(const SetTeamSeal& data)
{
  if (data.teamid())
    qwTeamID = data.teamid();
  if (data.leaderid())
    qwLeaderID = data.leaderid();
  for (int i = 0; i < data.teamers_size(); ++i)
  {
    setTeamerIDs.insert(data.teamers(i));
  }
}

void SSealData2::fromData(const TeamSealData& data)
{
  dwConfigID = data.seal();
  xSealPos = data.pos();
  qwTeamID = data.teamid();
  qwLastUserOnlineTime = data.lastonlinetime();
  if (qwLastUserOnlineTime == 0)
    qwLastUserOnlineTime = now();
  bHaveSendScene = false;
}

void SSealData2::toData(TeamSealData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_teamid(qwTeamID);
  pData->set_seal(dwConfigID);
  pData->mutable_pos()->CopyFrom(xSealPos);
  pData->set_lastonlinetime(qwLastUserOnlineTime);
}

void SSealData2::sendSealInfoToUser(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
  if (dwConfigID == 0)
    return;
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwConfigID);
  if (!pCFG)
    return;
  SealAcceptCmd message;
  message.set_seal(dwConfigID);
  message.mutable_pos()->CopyFrom(xSealPos);
  PROTOBUF(message, send2, lend2);
  pUser->sendCmdToMe(send2, lend2);

  if (dwBeginTime != 0)
  {
    QuerySealTimer cmd;
    cmd.set_teamid(qwTeamID);
    cmd.set_userid(pUser->id);
    PROTOBUF(cmd, send, len);
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(dwSealingMapID);
    if (pScene)
      pScene->sendCmd(send, len);
  }
}

void SSealData2::delSealToScene()
{
  if (dwConfigID == 0)
    return;
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwConfigID);
  if (!pCFG)
    return;
  SetTeamSeal cmd;
  cmd.set_teamid(qwTeamID);
  cmd.set_sealid(dwConfigID);
  cmd.set_estatus(ESETSEALSTATUS_INVALID);
  PROTOBUF(cmd, send, len);
  SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
  if (pScene)
    pScene->sendCmd(send, len);
}

void SSealData2::update(DWORD curTime)
{
  if (dwNextCreateTime != 0 && curTime >= dwNextCreateTime)
  {
    const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwConfigID);
    if (pCFG)
    {
      SetTeamSeal cmd;
      cmd.set_teamid(qwTeamID);
      cmd.set_sealid(dwConfigID);
      cmd.set_estatus(ESETSEALSTATUS_CREATE);
      PROTOBUF(cmd, send, len);
      SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
      if (pScene)
        pScene->sendCmd(send, len);
    }
    for (auto &q : setTeamerIDs)
    {
      MsgManager::sendMsg(q, 1610);
    }
    dwNextCreateTime = 0;
  }
  if (bLeaderNoSeal && dwBeginTime == 0)
  {
    const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwConfigID);
    if (pCFG)
    {
      SetTeamSeal cmd;
      cmd.set_teamid(qwTeamID);
      cmd.set_sealid(dwConfigID);
      cmd.set_estatus(ESETSEALSTATUS_ABANDON);
      PROTOBUF(cmd, send, len);
      SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
      if (pScene)
        pScene->sendCmd(send, len);
    }
    SealAcceptCmd message;
    message.set_seal(dwConfigID);
    message.set_abandon(true);
    PROTOBUF(message, send2, lend2);
    for (auto &q : setTeamerIDs)
    {
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(q);
      if (!pUser)
        continue;
      pUser->sendCmdToMe(send2, lend2);
      DWORD msgid = (q == qwLeaderID) ? 1617 : 1616;
      MsgManager::sendMsg(q, msgid);
    }
    clear();
    bSaveUpdate = true;
  }

  if (bSaveUpdate)
  {
    saveData();
    bSaveUpdate = false;
  }
}

void SSealData2::saveData()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "seal");
  if (pField == nullptr)
  {
    XERR << "[封印-保存], 获取数据库失败" << XEND;
    return;
  }

  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "zoneid=%u and teamid=%llu", thisServer->getZoneID(), qwTeamID);
  QWORD ret;
  if (bNeedClear)
  {
    ret = thisServer->getDBConnPool().exeDelete(pField, where);
  }
  else
  {
    string str;
    TeamSealData rData;
    toData(&rData);
    if (rData.SerializeToString(&str) == false)
    {
      XERR << "[封印-保存], 序列化失败" << XEND;
    }

    xRecord record(pField);
    record.put("zoneid", thisServer->getZoneID());
    record.put("teamid", qwTeamID);
    record.putBin("data", (unsigned char *)str.c_str(), str.size());

    ret = thisServer->getDBConnPool().exeReplace(record);
  }
  if (ret == QWORD_MAX)
  {
    XERR << "[封印-保存], 保存失败" << XEND;
    return;
  }
  XLOG << "[封印-保存], 保存成功" << XEND;
}


SealManager::SealManager()
{

}

SealManager::~SealManager()
{

}

bool SealManager::loadDataFromDB()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "seal");
  if (pField == nullptr)
  {
    XERR << "[封印-加载], 读取数据库失败" << XEND;
    return false;
  }
  char where[1024];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "zoneid = %u", thisServer->getZoneID());

  xRecordSet rSet;
  QWORD num = thisServer->getDBConnPool().exeSelect(pField, rSet, where, nullptr);
  if (num == QWORD_MAX)
  {
    XERR << "[封印-加载], 读取数据库失败" << XEND;
    return false;
  }
  for (QWORD i = 0; i < num; ++i)
  {
    string data;
    data.assign((const char *)rSet[i].getBin("data"), rSet[i].getBinSize("data"));
    TeamSealData blob;
    if (blob.ParsePartialFromString(data) == false)
    {
      XERR << "[封印-加载], 序列化失败" << XEND;
      return false;
    }
    SSealData2& sdata = m_mapTeam2SealData[rSet[i].get<QWORD>("teamid")];
    sdata.fromData(blob);
  }
  XLOG << "[封印-加载], 加载成功" << XEND;
  return true;
}

void SealManager::setSeal(SetTeamSeal& scmd)
{
  auto it = m_mapTeam2SealData.find(scmd.teamid());
  if (it == m_mapTeam2SealData.end())
  {
    m_mapTeam2SealData[scmd.teamid()] = SSealData2();
    it = m_mapTeam2SealData.find(scmd.teamid());
    if (it == m_mapTeam2SealData.end())
      return;
    it->second.loadTeamInfo(scmd);
  }

  switch(scmd.estatus())
  {
    case ESETSEALSTATUS_CREATE:
      {
        if (it->second.dwBeginTime != 0)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(it->second.qwLeaderID);
          if (pUser)
            MsgManager::sendMsg(pUser->id, 1608);
          break;
        }
        it->second.dwConfigID = scmd.sealid();
        it->second.dwBeginTime = 0;

        PROTOBUF(scmd, send, len);
        thisServer->sendCmdToAllScene(send, len);

        it->second.bSaveUpdate = true;
        //m_bSealUpdate = true;
      }
      break;
    case ESETSEALSTATUS_ABANDON:
      {
        if (it->second.dwConfigID == 0)
          break;
        if (it->second.dwBeginTime != 0)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(it->second.qwLeaderID);
          if (pUser)
            MsgManager::sendMsg(pUser->id, 1608);
          break;
        }

        PROTOBUF(scmd, send, len);
        SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(scmd.mapid());
        if (pScene)
          pScene->sendCmd(send, len);

        SealAcceptCmd message;
        message.set_seal(it->second.dwConfigID);
        message.set_abandon(true);
        PROTOBUF(message, send2, lend2);
        for (auto &q : it->second.setTeamerIDs)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(q);
          if (pUser)
            pUser->sendCmdToMe(send2, lend2);
        }

        it->second.clear();
        it->second.bSaveUpdate = true;
        //m_bSealUpdate = true;
      }
      break;
    case ESETSEALSTATUS_BEGIN:
      if (it->second.dwConfigID == 0)
        break;
      it->second.dwBeginTime = now();
      it->second.dwSealingMapID = scmd.mapid();
      break;
    case ESETSEALSTATUS_FINISH:
      if (it->second.dwBeginTime == 0)
        break;
      it->second.dwBeginTime = 0;
      if (it->second.bLeaderNoSeal)
        break;

      it->second.dwNextCreateTime = now() + MiscConfig::getMe().getSealCFG().dwSealNextTime;
      break;
    case ESETSEALSTATUS_FAIL:
      {
        if (it->second.dwBeginTime == 0)
          break;
        it->second.dwBeginTime = 0;

        if (it->second.bLeaderNoSeal)
          break;

        SetTeamSeal cmd;
        cmd.set_teamid(it->second.qwTeamID);
        cmd.set_sealid(it->second.dwConfigID);
        cmd.set_estatus(ESETSEALSTATUS_CREATE);
        cmd.set_mapid(scmd.mapid());
        cmd.mutable_pos()->CopyFrom(it->second.xSealPos);
        PROTOBUF(cmd, send, len);
        SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(scmd.mapid());
        if (pScene)
          pScene->sendCmd(send, len);
      }
      break;
    case ESETSEALSTATUS_SETPOS:
      {
        if (it->second.dwConfigID == 0)
          break;
        it->second.xSealPos = scmd.pos();
        it->second.bSaveUpdate = true;
        //m_bSealUpdate = true;
      }
      break;
    case ESETSEALSTATUS_INVALID:
      {
        it->second.bLeaderNoSeal = true;
      }
      break;
    default:
      break;
  }
}

void SealManager::addMember(QWORD teamid, QWORD userid)
{
  auto it = m_mapTeam2SealData.find(teamid);
  if (it == m_mapTeam2SealData.end())
    return;
  if (it->second.setTeamerIDs.find(userid) != it->second.setTeamerIDs.end())
    return;
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr)
    return;

  it->second.setTeamerIDs.insert(userid);
  it->second.sendSealInfoToUser(pUser);

  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(it->second.dwConfigID);
  if (pCFG)
  {
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
    if (pScene)
    {
      ChangeTeamSessionCmd cmd;
      cmd.set_join(true);
      cmd.set_teamid(teamid);
      cmd.set_userid(userid);
      PROTOBUF(cmd, send, len);
      pScene->sendCmd(send, len);
    }
  }
}

void SealManager::removeMember(QWORD teamid, QWORD userid)
{
  auto it = m_mapTeam2SealData.find(teamid);
  if (it == m_mapTeam2SealData.end())
    return;
  if (it->second.setTeamerIDs.find(userid) == it->second.setTeamerIDs.end())
    return;
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(userid);
  if (pUser && it->second.dwConfigID != 0)
  {
    SealAcceptCmd cmd;
    cmd.set_abandon(true);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
  it->second.setTeamerIDs.erase(userid);

  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(it->second.dwConfigID);
  if (pCFG)
  {
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
    if (pScene)
    {
      ChangeTeamSessionCmd cmd;
      cmd.set_join(false);
      cmd.set_teamid(teamid);
      cmd.set_userid(userid);
      PROTOBUF(cmd, send, len);
      pScene->sendCmd(send, len);
    }
  }

  if (it->second.setTeamerIDs.empty())
    m_mapTeam2SealData.erase(it);
}

void SealManager::changeLeader(QWORD teamid, QWORD newleader)
{
  auto it = m_mapTeam2SealData.find(teamid);
  if (it == m_mapTeam2SealData.end() || it->second.qwLeaderID == newleader)
    return;
  it->second.qwLeaderID = newleader;

  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SEAL, newleader);
  BlobSeal pCmd;
  RedisManager::getMe().getProtoData(key, &pCmd);
  if (it->second.dwConfigID != 0)
  {
    it->second.bLeaderNoSeal = true;
    for (int i = 0; i < pCmd.openseals_size(); ++i)
    {
      if (pCmd.openseals(i) == it->second.dwConfigID)
      {
        it->second.bLeaderNoSeal = false;
        break;
      }
    }
  }
  if (it->second.bLeaderNoSeal == false)
  {
    // 队长移交给非本线玩家, 删除本线封印
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(newleader);
    if (pUser == nullptr)
    {
      it->second.bLeaderNoSeal = true;
    }
  }
}

void SealManager::timer(DWORD curTime)
{
  QWORD maxtime =  MiscConfig::getMe().getTeamCFG().dwOverTime;

  for (auto m = m_mapTeam2SealData.begin(); m != m_mapTeam2SealData.end(); )
  {
    m->second.update(curTime);

    if (m->second.bNeedClear)
    {
      m->second.delSealToScene();
      m = m_mapTeam2SealData.erase(m);
      continue;
    }
    if (m->second.qwLastUserOnlineTime != 0 && curTime >= m->second.qwLastUserOnlineTime + maxtime)
    {
      m->second.delSealToScene();
      m = m_mapTeam2SealData.erase(m);
      continue;
    }
    ++m;
  }
}

void SealManager::onUserOnline(SessionUser* pUser)
{
  if (pUser == nullptr)
    return;
   auto it = m_mapTeam2SealData.find(pUser->getTeamID());
   if (it == m_mapTeam2SealData.end())
     return;
   // session 重启
   if (it->second.bHaveSendScene == false)
   {
     it->second.bHaveSendScene = true;
     const GTeam& rTeam = pUser->getTeam();
     it->second.qwLeaderID = rTeam.getLeaderID();
     for (auto &m : rTeam.getTeamMemberList())
     {
      it->second.setTeamerIDs.insert(m.second.charid());
     }
     const SealCFG* pCFG = SealConfig::getMe().getSealCFG(it->second.dwConfigID);
     if (pCFG == nullptr)
     {
       XLOG << "[封印], 删除无效封印, 队伍:" << it->first << "封印:" << it->second.dwConfigID << XEND;
       m_mapTeam2SealData.erase(it);
       return;
     }
     SetTeamSeal cmd;
     cmd.set_teamid(it->second.qwTeamID);
     cmd.set_sealid(it->second.dwConfigID);
     cmd.set_estatus(ESETSEALSTATUS_CREATE);
     cmd.mutable_pos()->CopyFrom(it->second.xSealPos);
     for (auto &s : it->second.setTeamerIDs)
     {
      cmd.add_teamers(s);
     }
     PROTOBUF(cmd, send, len);
     SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(pCFG->dwMapID);
     if (pScene)
       pScene->sendCmd(send, len);
   }

   it->second.sendSealInfoToUser(pUser);
   it->second.qwLastUserOnlineTime = 0;
}

void SealManager::onUserOffline(SessionUser* pUser)
{
  auto it = m_mapTeam2SealData.find(pUser->getTeamID());
  if (it == m_mapTeam2SealData.end())
    return;
  bool haveUser = false;
  for (auto &s : it->second.setTeamerIDs)
  {
    if (pUser->id == s)
      continue;
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(s);
    if (pUser != nullptr)
    {
      haveUser = true;
      break;
    }
  }

  if (!haveUser)
    it->second.qwLastUserOnlineTime = now();
}


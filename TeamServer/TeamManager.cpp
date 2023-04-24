#include "TeamManager.h"
#include "xCommand.h"
#include "MiscConfig.h"
#include "xTime.h"
#include "TeamServer.h"
#include "SceneUser2.pb.h"
#include "MapConfig.h"
#include "PlatLogManager.h"
#include "GCharManager.h"
#include "UserCmd.h"
#include "TableManager.h"
#include "MsgManager.h"
#include "TeamCmd.pb.h"
#include "TeamDataThread.h"
#include "MatchSCmd.pb.h"
#include "PveCard.pb.h"
#include "TeamRaidCmd.pb.h"
#include "TeamIDManager.h"

// guild manager
TeamManager::TeamManager()
{

}

TeamManager::~TeamManager()
{

}

bool TeamManager::addTeam(Team *team)
{
  if (!team) return false;
  return xEntryManager::addEntry(team);
}

void TeamManager::removeTeam(Team *team)
{
  if (!team) return;

  xEntryManager::removeEntry(team);
}

bool TeamManager::loadAllDataFromDB()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "team");
  if (pField == nullptr)
    return false;

  char where[128];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "zoneid = %u", thisServer->getZoneID());

  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, where);
  if (QWORD_MAX == retNum)
    return false;

  for (QWORD q = 0; q < retNum; ++q)
  {
    QWORD guid = set[q].get<QWORD>("id");
    const string& name = set[q].getString("name");
    DWORD type = set[q].get<DWORD>("teamtype");

    if (type == 0)
    {
      TeamIDManager::getMe().putTeamID(guid);
      continue;
    }

    const TeamGoalBase* pCFG = TableManager::getMe().getTeamCFG(type);
    if (pCFG == nullptr)
    {
      XERR << "[队伍管理-加载]" << guid << name << "type :" << type << "未在 Table_TeamGoal.txt 中找到" << XEND;
      continue;
    }

    Team* pTeam = NEW Team(guid, set[q].get<DWORD>("zoneid"));
    if (pTeam == nullptr)
    {
      XERR << "[队伍管理-加载]" << guid << name << "创建失败" << XEND;
      continue;
    }
    if (!addTeam(pTeam))
    {
      XERR << "[队伍管理-加载]" << guid << name << "添加到管理器失败" << XEND;
      SAFE_DELETE(pTeam);
      continue;
    }

    if (pTeam->fromData(set[q]) == false)
    {
      XERR << "[队伍管理-加载]" << guid << name << "数据加载失败" << XEND;
      continue;
    }
    pTeam->m_pTeamCFG = pCFG;

    m_mapType2Team[pCFG->id].insert(pTeam->getGUID());

    DWORD dwNow = xTime::getCurSec();
    for (auto &v : pTeam->m_mapMembers)
    {
      if (!v.second) continue;
      if (v.second->isOnline() == true)
        v.second->resetOfflineTime(dwNow);
      m_mapUserID2Team[v.second->getGUID()] = pTeam->getGUID();
    }
    for (auto &v : pTeam->m_mapApplys)
    {
      if (!v.second) continue;
      if (v.second->isOnline() == true)
        v.second->resetOfflineTime(dwNow);
      addApplyTeam(v.second->getGUID(), pTeam);
    }

    XLOG << "[队伍管理-加载]" << pTeam->getGUID() << pTeam->getName() << "成功加载" << XEND;
  }

  return true;
}

void TeamManager::final()
{
  DWORD dwNow = xTime::getCurSec();
  for (auto m : xEntryID::ets_)
  {
    Team *t = (Team *)m.second;
    if (t)
    {
      //t->m_bitset.set();
      //t->updateData(dwNow);
      t->updateRecord(dwNow + TEAM_RECORD_TIME_MAX);
    }
  }
}

void TeamManager::reload()
{
  for (auto &m : m_mapType2Team)
  {
    const TeamGoalBase* pCFG = TableManager::getMe().getTeamCFG(m.first);
    for (auto &s : m.second)
    {
      Team *pTeam = getTeamByID(s);
      if (pTeam)
      {
        pTeam->setTeamType(pCFG);
      }
    }
  }
}

void TeamManager::delChar(QWORD qwCharID)
{
  clearInviteMember(qwCharID);
  XLOG << "[队伍管理-删号] charid :" << qwCharID << "删除申请成功" << XEND;

  auto quick = m_mapQuickEnterUser.find(qwCharID);
  if (quick != m_mapQuickEnterUser.end())
  {
    m_mapQuickEnterUser.erase(quick);
    XLOG << "[队伍管理-删号] charid :" << qwCharID << "删除快速加入成功" << XEND;
  }

  auto m = m_mapUserID2Team.find(qwCharID);
  if (m != m_mapUserID2Team.end())
  {
    SocialUser oUser;
    oUser.set_charid(qwCharID);
    if (exitTeam(oUser) == true)
      XLOG << "[队伍管理-删号] charid :" << qwCharID << "退出队伍成功" << XEND;
  }

  auto apply = m_mapUserApplyTeam.find(qwCharID);
  if (apply != m_mapUserApplyTeam.end())
  {
    for (auto &s : apply->second)
    {
      Team *pTeam = getTeamByID(s);
      if (pTeam)
      {
        pTeam->removeApply(qwCharID);
      }
    }
    XLOG << "[队伍管理-删号] charid :" << qwCharID << "删除申请成功" << XEND;
  }

  auto info = m_mapInviteInfo.find(qwCharID);
  if (info != m_mapInviteInfo.end())
  {
    m_mapInviteInfo.erase(info);
    XLOG << "[队伍管理-删号] charid :" << qwCharID << "删除邀请信息成功" << XEND;
  }
}

void TeamManager::onUserOnline(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return;

  m_setOnlineUser.insert(rUser.charid());

  removeLeaderChangeCache(rUser.charid());

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam != nullptr)
  {
    TMember* pMember = pTeam->getMember(rUser.charid());
    if (pMember == nullptr)
      return;

    auto m = m_mapQuickEnterUser.find(rUser.charid());
    if (m != m_mapQuickEnterUser.end())
      m_mapQuickEnterUser.erase(m);

    // update team over time
    pTeam->setOverTime(0);

    // set leader if no other online member
    pMember->resetOfflineTime(0);
    
    // update online state
    pMember->setOfflineTime(0);
    pMember->setZoneID(rUser.zoneid());
    pMember->setMapID(rUser.mapid());

    // inform client
    EnterTeam cmd;
    pTeam->toClientData(cmd.mutable_data(), pMember->isLeader());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
       

    // inform scene
    TeamDataSyncTeamCmd synccmd;
    synccmd.set_charid(rUser.charid());
    pTeam->toData(synccmd);
    synccmd.set_online(true);
    PROTOBUF(synccmd, send1, len1);
    thisServer->sendCmdToSession(send1, len1);

    // remove temp leader if leader online
    resetTempLeader(pTeam);

    if(pTeam->getRedTip() == true && pMember->isLeader() == true)
    {
      pTeam->setRedTip(false);
      syncRedTip(pTeam->getGUID(), rUser.charid(), true);
    }

    // check leader
    pTeam->checkLeaderWhenOnline(rUser.charid());

    // broadcast msg
    pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_MEMBER_ONLINE, MsgParams(pMember->getName()));

    // check invalid cat
    //pTeam->checkInvalidCat(rUser.charid());

    pTeam->queryMemberQuestList(rUser.charid());
    pTeam->queryHelpQuestList(rUser.charid());

    pMember->sendRealtimeVoiceID();

    XDBG << "[队伍管理-成员上线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "队伍" << pTeam->getGUID() << pTeam->getName() << XEND;
  }
  else
  {
    auto m = m_mapQuickEnterUser.find(rUser.charid());
    if (m != m_mapQuickEnterUser.end())
    {
      QuickEnter cmd;
      cmd.set_time(m->second.dwEndTime);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
    }
    TeamDataSyncTeamCmd cmd;
    cmd.set_charid(rUser.charid());
    cmd.set_online(true);
    PROTOBUF(cmd, send1, len1);
    thisServer->sendCmdToSession(send1, len1);
  }
}

void TeamManager::onUserOffline(const SocialUser& rUser)
{
  if (rUser.charid() == 0)
    return;

  m_setOnlineUser.erase(rUser.charid());

  clearInviteMember(rUser.charid());

  auto m = m_mapQuickEnterUser.find(rUser.charid());
  if (m != m_mapQuickEnterUser.end())
    m_mapQuickEnterUser.erase(m);

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam != nullptr)
  {
    // reset member and exchange leader if leader
    TMember* pMember = pTeam->getMember(rUser.charid());
    if (pMember == nullptr)
      return;

    if (pMember->isLeader() == true)
      addLeaderChangeCache(rUser);
    else
      pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_MEMBER_OFFLINE, MsgParams(pMember->getName()));

    pMember->setOfflineTime(xTime::getCurSec());

    // update team over time
    if (pTeam->getOnlineMemberCount() == 0)
      pTeam->setOverTime(xTime::getCurSec());

    // dojo
    pTeam->cancelSponsor(rUser);

    // notify mem offline
    TeamDataSyncTeamCmd synccmd;
    pTeam->toData(synccmd);
    const TMapTeamMember& memlist = pTeam->getTeamMembers();
    for (auto &t : memlist)
    {
      if (t.first == rUser.charid())
        continue;
      if (t.second && t.second->isOnline())
      {
        synccmd.set_charid(t.first);
        PROTOBUF(synccmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
    }

    // 成员下线, 取消匹配
    if (pTeam->isInMatchTeamPws())
    {
      pTeam->broadcastMsg(25903);
      pTeam->cancelMatchPws();
    }

    XDBG << "[队伍管理-成员下线]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "队伍" << pTeam->getGUID() << pTeam->getName() << XEND;
  }
}

void TeamManager::updateUserInfo(const UserInfoSyncSocialCmd& cmd)
{
  Team* pTeam = getTeamByUserID(cmd.info().user().charid());
  if (pTeam != nullptr)
  {
    TMember* pMember = pTeam->getMember(cmd.info().user().charid());
    if (pMember != nullptr)
    {
      pMember->updateData(cmd.info());

      /*// update cat
      const TMapTeamMember& mapMember = pTeam->getTeamMembers();
      for (auto &m : mapMember)
      {
        if (m.second->getCatID() != 0 && m.second->getGUID() / 10000 == pMember->getGUID())
          m.second->updateData(cmd.info());
      }*/
    }
  }

  auto m = m_mapUserApplyTeam.find(cmd.info().user().charid());
  if (m != m_mapUserApplyTeam.end())
  {
    for (auto &s : m->second)
    {
      Team *pTeam = getTeamByID(s);
      if (pTeam)
      {
        TMember* pMember = pTeam->getApply(cmd.info().user().charid());
        if (pMember != nullptr)
        {
          pMember->updateData(cmd.info());
        }
      }
    }
  }

  auto n = m_mapInviteInfo.find(cmd.info().user().charid());
  if (n != m_mapInviteInfo.end())
  {
    for (int i = 0; i < cmd.info().datas_size(); ++i)
    {
      const UserData& rData = cmd.info().datas(i);
      for (int j = 0; j < n->second.datas_size(); ++j)
      {
        UserData* pData = n->second.mutable_datas(j);
        if (pData != nullptr && pData->type() == rData.type())
        {
          pData->CopyFrom(rData);
          break;
        }
      }
    }
    for (int i = 0; i < cmd.info().attrs_size(); ++i)
    {
      const UserAttr& rAttr = cmd.info().attrs(i);
      for (int j = 0; j < n->second.attrs_size(); ++j)
      {
        UserAttr* pAttr = n->second.mutable_attrs(j);
        if (pAttr != nullptr && pAttr->type() == rAttr.type())
        {
          pAttr->CopyFrom(rAttr);
          break;
        }
      }
    }
  }
}

void TeamManager::checkQuickEnter(Team* pTeam)
{
  const STeamCFG& rCFG = MiscConfig::getMe().getTeamCFG();
  if (pTeam == nullptr || pTeam->getAuto() == false || pTeam->getMemberCount() >= rCFG.dwMaxMember || pTeam->getOnlineMemberCount() == 0)
    return;

  const UserInfo* pInfo = nullptr;
  auto m = m_mapQuickEnterUser.begin();
  for (; m != m_mapQuickEnterUser.end(); ++m)
  {
    pInfo = getInviteInfo(m->first);
    if (pInfo != nullptr)
      break;
  }
  if (pInfo == nullptr)
    return;

  TMember* pLeader = pTeam->getLeader();
  if (pLeader == nullptr || pInfo->user().zoneid() != pLeader->getZoneID())
    return;
  if (pInfo->user().baselv() < pTeam->getMinReqLv())
    return;

  if (pTeam->getMemberCount() == 0 || pTeam->getMemberCount() >= rCFG.dwMaxMember)
  {
    pTeam->removeCat(ECATREMOVEMETHOD_EARLY);
    if (pTeam->getMemberCount() == 0 || pTeam->getMemberCount() >= rCFG.dwMaxMember)
      return;
  }

  if (checkTeam(m->second.pBase, pTeam) == false)
    return;

  TMember *pMember = NEW TMember(pTeam, *pInfo, ETEAMJOB_MEMBER);
  if (addMember(pTeam, pMember) == false)
  {
    SAFE_DELETE(pMember);
    return;
  }

  m_mapQuickEnterUser.erase(m);

  const SocialUser &oUser = pInfo->user();
  XLOG << "[队伍管理-快速加入]" << oUser.accid() << oUser.accid() << oUser.profession() << oUser.name() << "快速加入了" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
}

bool TeamManager::addInviteMember(const UserInfo& rInvite, QWORD qwBeInviteID)
{
  TVecTeamInvite& vecTeamInvites = m_mapUserID2InviteTeam[qwBeInviteID];
  auto v = find_if(vecTeamInvites.begin(), vecTeamInvites.end(), [&](const TPairTeamInvite& r) -> bool{
    return r.first == rInvite.user().charid();
  });
  if (v != vecTeamInvites.end())
    return false;

  TPairTeamInvite p;
  p.first = rInvite.user().charid();
  p.second = xTime::getCurSec();
  vecTeamInvites.push_back(p);

  if (isTeamMember(rInvite.user().charid()) == false)
    m_mapInviteInfo[rInvite.user().charid()].CopyFrom(rInvite);
  return true;
}

bool TeamManager::clearInviteMember(QWORD userid)
{
  auto m = m_mapUserID2InviteTeam.find(userid);
  if (m == m_mapUserID2InviteTeam.end())
    return false;

  m_mapUserID2InviteTeam.erase(m);
  auto n = m_mapInviteInfo.find(userid);
  if (n != m_mapInviteInfo.end())
    m_mapInviteInfo.erase(n);

  //XLOG << "[队伍管理] 邀请成员数量 :" << m_mapUserID2InviteTeam.size() << "个" << XEND;
  return true;
}

DWORD TeamManager::getInviteMemberCount(QWORD userid)
{
  auto m = m_mapUserID2InviteTeam.find(userid);
  if (m == m_mapUserID2InviteTeam.end())
    return 0;

  return static_cast<DWORD>(m->second.size());
}

TPairTeamInvite* TeamManager::getInvite(QWORD inviteid, QWORD userid)
{
  auto m = m_mapUserID2InviteTeam.find(userid);
  if (m == m_mapUserID2InviteTeam.end())
    return nullptr;

  TVecTeamInvite& vecTeamInvites = m_mapUserID2InviteTeam[userid];
  auto v = find_if(vecTeamInvites.begin(), vecTeamInvites.end(), [inviteid](const TPairTeamInvite& r) -> bool{
    return r.first == inviteid;
  });
  if (v == vecTeamInvites.end())
    return nullptr;

  return &(*v);
}

Team* TeamManager::getTeamByID(QWORD id)
{
  return (Team *)getEntryByID(id);
}

Team* TeamManager::getTeamByUserID(QWORD userid)
{
  auto m = m_mapUserID2Team.find(userid);
  if (m == m_mapUserID2Team.end())
    return nullptr;

  return getTeamByID(m->second);
}

Team* TeamManager::createOneTeam(const string& name, const UserInfo& rLeader, DWORD dwType, DWORD dwMinLv, DWORD dwMaxLv, EAutoType eAutoType)
{
  if (rLeader.user().charid() == 0 || isTeamMember(rLeader.user().charid()) == true)
    return nullptr;

  const SocialUser& rUser = rLeader.user();
  const STeamCFG& rTeamCFG = MiscConfig::getMe().getTeamCFG();
  const TeamGoalBase* pTeamCFG = TableManager::getMe().getTeamCFG(dwType);
  if (pTeamCFG == nullptr || pTeamCFG->getTableInt("SetShow") == 0)
  {
    pTeamCFG = TableManager::getMe().getTeamCFG(rTeamCFG.dwDefaultType);
    if (pTeamCFG == nullptr)
      return nullptr;
    XERR << "[队伍管理-创建队伍]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "创建" << name << "目标" << dwType << "不合法,修正为默认" << XEND;
  }

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  ESysMsgID eMsg = rCFG.checkNameValid(name, ENAMETYPE_TEAM);
  if (eMsg != ESYSTEMMSG_ID_MIN)
  {
    XERR << "[队伍管理-创建队伍]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "创建" << name << "失败,含有屏蔽字" << XEND;
    return nullptr;
  }

  QWORD qwTeamID = TeamIDManager::getMe().getTeamID();
  if (!qwTeamID) return nullptr;

  Team* pTeam = NEW Team(qwTeamID, rLeader.user().zoneid());
  if (pTeam == nullptr)
  {
    TeamIDManager::getMe().putTeamID(qwTeamID);
    return nullptr;
  }

 // pTeam->setDataMark(ETEAMDATA_ZONEID);
  pTeam->setName(name);
  pTeam->setTeamType(pTeamCFG);
  pTeam->setMinReqLv(dwMinLv);
  pTeam->setMaxReqLv(dwMaxLv);
  pTeam->setAuto(eAutoType);
  pTeam->setOverTime(0);
  pTeam->setPickupMode(MiscConfig::getMe().getTeamCFG().dwPickupMode);

  if (!addTeam(pTeam))
  {
    TeamIDManager::getMe().putTeamID(qwTeamID);
    SAFE_DELETE(pTeam);
    return nullptr;
  }

  // add leader
  TMember *pLeader = NEW TMember(pTeam, rLeader, ETEAMJOB_LEADER);
  if (!pTeam->addMember(pLeader))
  {
    TeamIDManager::getMe().putTeamID(qwTeamID);
    removeTeam(pTeam);
    SAFE_DELETE(pTeam);
    SAFE_DELETE(pLeader);
    return nullptr;
  }

  // inform client
  thisServer->sendMsg(pLeader->getZoneID(), pLeader->getGUID(), ESYSTEMMSG_ID_TEAM_CREATE_SUCCESS, MsgParams(pTeam->getName()));

  // insert to list
  m_mapUserID2Team[rLeader.user().charid()] = pTeam->getGUID();
  m_mapType2Team[dwType].insert(pTeam->getGUID());

  checkQuickEnter(pTeam);

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Create;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rLeader.user().zoneid(),
    rLeader.user().accid(),
    rLeader.user().charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().TeamLog(thisServer,
    rLeader.user().accid(),
    rLeader.user().charid(),
    eType,
    eid,
    pTeam->getGUID(),
    0
  );
  return pTeam;
}

void TeamManager::timer(DWORD curTime)
{
  // check invalid team
  for (auto m = ets_.begin(); m != ets_.end();)
  {
    Team *pTeam = (Team *)(m->second);
    if (!pTeam)
    {
      m = ets_.erase(m);
      continue;
    }
    if (pTeam->getMemberCount() == 0 || (pTeam->getOverTime() != 0 && curTime - pTeam->getOverTime() >= MiscConfig::getMe().getTeamCFG().dwOverTime))
    {
      // remove from db
      ThTeamData *pData = new ThTeamData(ThTeamDataAction_Delete);
      pData->m_qwTeamID = pTeam->getGUID();
      ThTeamDataThread::getMe().add(pData);

      // remove from list
      m = ets_.erase(m);
      for (auto o = m_mapUserID2Team.begin(); o != m_mapUserID2Team.end();)
      {
        if (o->second == pTeam->getGUID())
        {
          pTeam->removeMember(o->first);
          o = m_mapUserID2Team.erase(o);
          continue;
        }

        ++o;
      }

      auto t = m_mapType2Team.find(pTeam->getTeamType()->id);
      if (t != m_mapType2Team.end())
        t->second.erase(pTeam->getGUID());

      removeApplyTeam(pTeam);

      //teamTempIDManager->putUniqueID(pTeam->getGUID());
      XLOG << "[队伍管理-计时]" << pTeam->getGUID() << pTeam->getName() << "超时被删除" << XEND;

      SAFE_DELETE(pTeam);
      continue;
    }

    pTeam->timer(curTime);
    ++m;
  }

  // process invite
  const STeamCFG& stCFG = MiscConfig::getMe().getTeamCFG();
  for (auto m = m_mapUserID2InviteTeam.begin(); m != m_mapUserID2InviteTeam.end(); ++m)
  {
    TVecTeamInvite& vecInvites = m->second;
    for (auto v = vecInvites.begin(); v != vecInvites.end();)
    {
      if (curTime > v->second + stCFG.dwInviteTime + 3)
      {
        v = vecInvites.erase(v);
        continue;
      }

      ++v;
    }
  }

  // process quick enter
  updateQuickUser(curTime);
  // process offline leader change
  updateLeaderChangeCache(curTime);

  ThTeamData *data = ThTeamDataThread::getMe().get();
  while (data)
  {
    ThTeamDataThread::getMe().pop();
    doThTeamData(data);
    SAFE_DELETE(data);

    data = ThTeamDataThread::getMe().get();
  }
}

void TeamManager::doThTeamData(ThTeamData *pData)
{
  if (!pData) return;

  switch (pData->m_oAction)
  {
    case ThTeamDataAction_Update:
      {
      }
      break;
    case ThTeamDataAction_CreateTeamID:
      {
        for (auto it : pData->m_oCreateTeamIDList)
        {
          TeamIDManager::getMe().putTeamID(it);
        }
      }
      break;
    default:
      break;
  }
}

bool TeamManager::doTeamCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  /*
  auto it = m_oQueryTimeList.find(rUser.charid());
  if (it != m_oQueryTimeList.end())
  {
    auto iter = it->second.find(cmd->param);
    if (iter != it->second.end())
    {
      if (iter->second > now())
        return true;
      iter->second = now() + 30;
    }
    else
    {
      it->second[cmd->param] = now() + 30;
    }
  }
  else
  {
    m_oQueryTimeList[rUser.charid()][cmd->param] = now() + 30;
  }
  */


  switch (cmd->param)
  {
    case TEAMPARAM_TEAMLIST:
      {
        PARSE_CMD_PROTOBUF(TeamList, rev);
        queryTeamList(rUser, rev.type(), rev.page(), rev.lv());
      }
      break;
    case TEAMPARAM_PROCESSAPPLY:
      {
        PARSE_CMD_PROTOBUF(ProcessTeamApply, rev);
        processApplyTeam(rev.type(), rUser, rev.userguid());
      }
      break;
    case TEAMPARAM_KICKMEMBER:
      {
        PARSE_CMD_PROTOBUF(KickMember, rev);
        if (rev.catid() == 0)
          kickMember(rUser, rev.userid());
        else
          kickCat(rUser, rev.userid(), rev.catid());
      }
      break;
    case TEAMPARAM_EXCHANGELEADER:
      {
        PARSE_CMD_PROTOBUF(ExchangeLeader, rev);
        exchangeLeader(rUser, rev.userid());
      }
      break;
    case TEAMPARAM_EXITTEAM:
      {
        PARSE_CMD_PROTOBUF(ExitTeam, rev);
        exitTeam(rUser);
      }
      break;
    case TEAMPARAM_LOCKTARGET:
      {
        PARSE_CMD_PROTOBUF(LockTarget, rev);
        lockTarget(rUser, rev.targetid());
      }
      break;
    case TEAMPARAM_SUMMON:
      {
        PARSE_CMD_PROTOBUF(TeamSummon, rev);
        summon(rUser, rev.raidid());
      }
      break;
    case TEAMPARAM_CLEARAPPLYLIST:
      {
        //PARSE_CMD_PROTOBUF(ClearApplyList, rev);
        clearApplyList(rUser);
      }
      break;
    case TEAMPARAM_SETOPTION:
      {
        PARSE_CMD_PROTOBUF(SetTeamOption, rev);
        setTeamOption(rUser, rev);
      }
      break;

    case TEAMPARAM_QUERYUSERTEAMINFO:
      {
        PARSE_CMD_PROTOBUF(QueryUserTeamInfoTeamCmd, rev);
        queryUserTeamInfo(rUser, rev.charid());
      }
      break;
    case TEAMPARAM_SETMEMBEROPTION:
      {
        PARSE_CMD_PROTOBUF(SetMemberOptionTeamCmd, rev);
        setMemberOption(rUser, rev);
      }
      break;
    case TEAMPARAM_ACCEPTHELPWANTED:
      {
        PARSE_CMD_PROTOBUF(AcceptHelpWantedTeamCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam != nullptr)
        {
          pTeam->acceptHelpQuest(rUser.charid(), rev.questid(), rev.isabandon());
        }
      }
      break;
    case TEAMPARAM_QUERYHELPWANTED:
      {
        /*
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam != nullptr)
          pTeam->queryHelpQuestList(rUser.charid());
          */
      }
      break;
    case TEAMPARAM_QUERYWANTEDQUEST:
      {
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam != nullptr)
          pTeam->queryMemberQuestList(rUser.charid());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool TeamManager::doDojoCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {   
    case EDOJOPARAM_SPONSOR:
      {
        PARSE_CMD_PROTOBUF(DojoSponsorCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          return false;

        if (rev.is_cancel())
        {
          return pTeam->cancelSponsor(rUser);
        }
        return pTeam->sponsorDojo(rUser, rev);
        break;
      }
      break;
    case EDOJOPARAM_REPLY:
      {
        PARSE_CMD_PROTOBUF(DojoReplyCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          return false;
        return pTeam->procInviteDojo(rUser, rev);
      }
      break;
    case EDOJOPARAM_QUERYSTATE:
      {
        PARSE_CMD_PROTOBUF(DojoQueryStateCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        GCharReader pGChar(thisServer->getRegionID(), rUser.charid());
        if (!pTeam || !pGChar.get())
        {
          rev.set_state(DOJOSTATE_ERROR);
        }
        else
        {
          pTeam->getDojoInfo(pGChar.getGuildID(), rev);
        }
        PROTOBUF(rev, send, len);
        XDBG << "[道场-查询状态] 返回，" << rev.ShortDebugString() << XEND;
        thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
        return true;
      }
      break;
    case EDOJOPARAM_ENTERDOJO:
      {
        PARSE_CMD_PROTOBUF(EnterDojo, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
        {
          return false;
        }
        return pTeam->createDojoScene(rUser, rev);     //TODO
      }
      break; 
    default:
      return false;
  }
  return true;
}

bool TeamManager::doTowerCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case ETOWERPARAM_TEAMTOWERSUMMARY:
      {
        if (rUser.baselv() < MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv)
          break;
        PARSE_CMD_PROTOBUF(TeamTowerSummaryCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        rev.mutable_teamtower()->set_teamid(pTeam->getGUID());
        rev.mutable_teamtower()->set_layer(pTeam->getCurTowerLayer());
        rev.mutable_teamtower()->mutable_leadertower()->CopyFrom(pTeam->getLeaderTowerInfo());
        PROTOBUF(rev, send, len);
        thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
      }
      break;
    case ETOWERPARAM_INVITE:
      {
        PARSE_CMD_PROTOBUF(TeamTowerInviteCmd, rev);

        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        TMember* pLeader = pTeam->getLeader();
        if (pLeader == nullptr || pLeader->isOnline() == false)
        {
          pLeader = pTeam->getTempLeader();
          if (pLeader == nullptr || pLeader->isOnline() == false)
            break;
          if (pLeader->getGUID() != rUser.charid())
            break;
        }
        if (rev.iscancel() == true)
        {
          pTeam->setTowerInviteOpen(false);
          break;
        }

        if (pTeam->getTowerInviteOpen() == true)
          break;

        DWORD limitlv = MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv;
        const TMapTeamMember& mapMembers = pTeam->getTeamMembers();
        for (auto &v : mapMembers)
        {
          if (!v.second) continue;
          if (v.second->getBaseLv() < limitlv)
            continue;
          if (v.second->getGUID() != pLeader->getGUID())
            v.second->sendCmdToMe(buf, len);
        }
        pTeam->setTowerInviteOpen(true);
      }
      break;
    case ETOWERPARAM_REPLY:
      {
        PARSE_CMD_PROTOBUF(TeamTowerReplyCmd, rev);

        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        if (pTeam->getTowerInviteOpen() == false && rev.ereply() == ETOWERREPLY_AGREE)
        {
          thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 1308);
          break;
        }

        TMember* pMember = pTeam->getMember(rUser.charid());
        if (pMember != nullptr && rev.ereply() == ETOWERREPLY_AGREE)
          pTeam->addTowerEnterMember(rUser.charid());

        TMember* pLeader = pTeam->getLeader();
        if (pLeader == nullptr || pLeader->isOnline() == false)
        {
          pLeader = pTeam->getTempLeader();
          if (pLeader == nullptr || pLeader->isOnline() == false)
            break;
        }
        pLeader->sendCmdToMe(buf, len);
      }
      break;
    case ETOWERPARAM_ENTERTOWER:
      {
        PARSE_CMD_PROTOBUF(EnterTower, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        pTeam->createTowerScene(rUser, rev);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool TeamManager::queryTeamList(const SocialUser& rUser, DWORD dwType, DWORD dwPage, DWORD dwLv)
{
  if (rUser.charid() == 0)
    return false;

  const TeamGoalBase* pTeamCFG = TableManager::getMe().getTeamCFG(dwType);
  if (pTeamCFG == nullptr)
    return false;

  set<Team*> setTeam;
  ETeamFilter eFilter = pTeamCFG->getTeamFilter();
  if (eFilter == ETEAMFILTER_MIN)
  {
    //DWORD dwCount = 0;
    for (auto m : xEntryID::ets_)
    {
      //if (dwCount++ >= 40)
        //break;
      Team *pTeam = (Team *)m.second;
      TMember* pLeader = pTeam->getLeader();
      TMember* pTempLeader = pTeam->getTempLeader();
      bool bLeader = pLeader != nullptr && pLeader->isOnline() == true && pLeader->getZoneID() == rUser.zoneid() && pLeader->getMapID() == rUser.mapid();
      bool bTempLeader = pTempLeader != nullptr && pTempLeader->isOnline() == true && pTempLeader->getZoneID() == rUser.zoneid() && pTempLeader->getMapID() == rUser.mapid();
      if (bLeader || bTempLeader)
        setTeam.insert(pTeam);
      if (setTeam.size() >= 40)
        break;
    }
  }
  else if (eFilter == ETEAMFILTER_MAP || eFilter == ETEAMFILTER_MAPRANGE)
  {
    auto m = m_mapType2Team.find(pTeamCFG->getTableInt("type"));
    if (m != m_mapType2Team.end())
    {
      TSetDWORD setTeamIDs;
      if (m->second.empty() == false)
      {
        for (int i = 0; i < 40; ++i)
          setTeamIDs.insert(*randomStlContainer(m->second));
      }
      for (auto &s : setTeamIDs)
      {
        Team *pTeam = getTeamByID(s);
        if (pTeam)
        {
          if (pTeam->getZoneID() == rUser.zoneid())
            setTeam.insert(pTeam);
        }
      }
    }
  }
  else if (eFilter == ETEAMFILTER_SEAL || eFilter == ETEAMFILTER_DOJO || eFilter == ETEAMFILTER_TOWER || eFilter == ETEAMFILTER_LABORATORY)
  {
    TVecDWORD vecFilters;
    pTeamCFG->collectFilterValue(vecFilters);
    for (auto v = vecFilters.begin(); v != vecFilters.end(); ++v)
    {
      auto m = m_mapType2Team.find(*v);
      if (m != m_mapType2Team.end())
      {
        if (eFilter == ETEAMFILTER_DOJO)
        {
          GCharReader pGChar(thisServer->getRegionID(), rUser.charid());
          if (pGChar.get())
          {
            TSetDWORD setTeamIDs;
            if (m->second.empty() == false)
            {
              for (int i = 0; i < 40; ++i)
                setTeamIDs.insert(*randomStlContainer(m->second));
            }
            for (auto &s : setTeamIDs)
            {
              Team *pTeam = getTeamByID(s);
              if (pTeam)
              {
                if (pTeam->getZoneID() != rUser.zoneid())
                  continue;
                TMember* pLeader = pTeam->getLeader();
                if (pLeader == nullptr)
                  continue;
                GCharReader p(thisServer->getRegionID(), pLeader->getGUID());
                if (!p.get() || p.getGuildID() != pGChar.getGuildID())
                  continue;
                setTeam.insert(pTeam);
              }
            }
          }
        }
        else
        {
          for (auto s = m->second.begin(); s != m->second.end(); ++s)
          {
            Team *pTeam = getTeamByID(*s);
            if (pTeam)
            {
              if (pTeam->getZoneID() == rUser.zoneid())
                setTeam.insert(pTeam);
            }
          }
        }
      }
    }
  }
  else if (eFilter == ETEAMFILTER_CHAT)
  {
    DWORD dwCount = 0;
    for (auto m : xEntryID::ets_)
    {
      if (dwCount++ >= 40)
        break;
      Team *pTeam = (Team *)m.second;
      if (rUser.zoneid() == pTeam->getZoneID())
        setTeam.insert(pTeam);
    }
  }

  TeamList cmd;
  cmd.set_type(dwType);
  cmd.set_page(dwPage);

  if (dwPage > 0)
    --dwPage;

  DWORD dwCount = 0;
  DWORD dwAdd = 0;
  DWORD dwStart = dwPage * ETEAMGLOBAL_LISTCOUNT_PERPAGE;
  for (auto s = setTeam.begin(); s != setTeam.end(); ++s)
  {
    Team* pTeam = *s;
    if (pTeam == nullptr)
      continue;
    if (pTeam->getMinReqLv() < dwLv)
      continue;
    if (checkTeam(pTeamCFG, pTeam) == false)
      continue;

    if (++dwCount < dwStart)
      continue;
    if (++dwAdd >= ETEAMGLOBAL_LISTCOUNT_PERPAGE)
      break;

    TeamData* pData = cmd.add_list();
    if (pData == nullptr)
      continue;

    pTeam->toSummaryData(pData);
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  XLOG << "[队伍管理-列表]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "查询获得了" << cmd.list_size() << "个队伍" << XEND;
  return true;
}

bool TeamManager::inviteMember(const UserInfo& rInvite, SocialUser rBeInvite)
{
  const SocialUser& rUser = rInvite.user();
  if (rInvite.user().charid() == 0 || rBeInvite.charid() == 0)
    return false;

  // 双方都有组队,提示334
  if (isTeamMember(rInvite.user().charid()) == true && isTeamMember(rBeInvite.charid()) == true)
  {
    thisServer->sendMsg(rInvite.user().zoneid(), rInvite.user().charid(), 334);
    return false;
  }

  // check zoneid
  if (rBeInvite.zoneid() == 0)
  {
    GCharReader pGChar(thisServer->getRegionID(), rBeInvite.charid());
    if (pGChar.get())
    {
      rBeInvite.set_zoneid(pGChar.getZoneID());
      rBeInvite.set_name(pGChar.getName());
    }
  }
  if (rBeInvite.zoneid() == 0 || rBeInvite.name().empty() == true)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 421);
    return false;
  }

  // check count
  DWORD count = getInviteMemberCount(rBeInvite.charid());
  if (count >= MiscConfig::getMe().getTeamCFG().dwMaxInvite)
    return false;

  // check is invited
  TPairTeamInvite* pPair = getInvite(rInvite.user().charid(), rBeInvite.charid());
  if (pPair != nullptr)
  {
    thisServer->sendMsg(rInvite.user().zoneid(), rInvite.user().charid(), 307);
    return false;
  }

  // step 1
  Team* pTeam = getTeamByUserID(rInvite.user().charid());
  if (pTeam != nullptr)
  {
    TMember *pMember = pTeam->getMember(rInvite.user().charid());
    if (nullptr == pMember)
      return false;

    if (pMember->isLeader() == false)
    {
      thisServer->sendMsg(rInvite.user().zoneid(), rInvite.user().charid(), 300, MsgParams(pTeam->getName()));
      return false;
    }

    if (nullptr != pTeam->getApply(rBeInvite.charid()))
    {
      if (processApplyTeam(ETEAMAPPLYTYPE_AGREE, rInvite.user(), rBeInvite.charid()) == true)
      {
        XLOG << "[队伍管理-邀请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
          << "邀请" << rBeInvite.accid() << rBeInvite.charid() << rBeInvite.profession() << rBeInvite.name() << "已申请,直接进入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
        return false;
      }
      thisServer->sendMsg(rInvite.user().zoneid(), rInvite.user().charid(), 307, MsgParams(pTeam->getName()));
      return false;
    }

    InviteMember cmd;
    cmd.set_userguid(rInvite.user().charid());
    cmd.set_teamname(pTeam->getName());
    cmd.set_username(rInvite.user().name());

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(rBeInvite.zoneid(), rBeInvite.charid(), send, len);

    thisServer->sendMsg(rInvite.user().zoneid(), rInvite.user().charid(), 325, MsgParams(rBeInvite.name()));

    // add invite info
    addInviteMember(rInvite, rBeInvite.charid());

    XLOG << "[队伍管理-邀请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << "邀请" << rBeInvite.accid() << rBeInvite.charid() << rBeInvite.profession() << rBeInvite.name() << "进入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
    return true;
  }

  // step 2
  pTeam = getTeamByUserID(rBeInvite.charid());
  if (pTeam != nullptr)
  {
    XLOG << "[队伍管理-邀请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "申请进入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
    return applyTeam(rInvite, pTeam->getGUID());
  }

  // step 3
  InviteMember cmd;
  cmd.set_userguid(rUser.charid());
  cmd.set_username(rUser.name());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rBeInvite.zoneid(), rBeInvite.charid(), send, len);

  //发送提示给自己, msg id = 325
  thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 325, MsgParams(rBeInvite.name()));

  // add invite info
  addInviteMember(rInvite, rBeInvite.charid());

  XLOG << "[队伍管理-邀请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
    << "邀请" << rBeInvite.accid() << rBeInvite.charid() << rBeInvite.profession() << rBeInvite.name() << "组队" << XEND;
  return true;
}

bool TeamManager::processInviteMember(ETeamInviteType eType, const UserInfo& rUser, QWORD qwLeaderID)
{
  const SocialUser& user = rUser.user();
  if (user.charid() == 0 || isTeamMember(user.charid()) == true)
    return false;

  // update invite time to three protect time
  TPairTeamInvite* pPair = getInvite(qwLeaderID, user.charid());
  if (pPair == nullptr)
    return false;
  pPair->second = xTime::getCurSec() - MiscConfig::getMe().getTeamCFG().dwInviteTime;

  Team* pTeam = getTeamByUserID(qwLeaderID);
  TMember* pLeader = nullptr;
  if (pTeam != nullptr)
    pLeader = pTeam->getLeader();
  const UserInfo* pLeaderInfo = getInviteInfo(qwLeaderID);

  // disagree invite
  if (eType != ETEAMINVITETYPE_AGREE)
  {
    if (pLeaderInfo != nullptr)
      thisServer->sendMsg(pLeaderInfo->user().zoneid(), pLeaderInfo->user().charid(), 309, MsgParams(user.name()));
    else if (pLeader != nullptr)
      thisServer->sendMsg(pLeader->getZoneID(), pLeader->getGUID(), 309, MsgParams(user.name()));
    XLOG << "[队伍管理-邀请答复]" << user.accid() << user.charid() << user.profession() << user.name() << "拒绝了" << qwLeaderID << "的邀请" << XEND;
    return false;
  }

  // agree invite and create team if not exist
  if (pTeam == nullptr)
  {
    if (pLeaderInfo == nullptr)
      return false;
    std::string sTeamName = pLeaderInfo->user().name();
    sTeamName += MiscConfig::getMe().getTeamCFG().sDefaultName;
    const STeamCFG& rCFG = MiscConfig::getMe().getTeamCFG();
    pTeam = createOneTeam(sTeamName, *pLeaderInfo, rCFG.dwDefaultType, rCFG.dwDefaultMinLv, rCFG.dwDefaultMaxLv, rCFG.eDefaultAuto);
    if (pTeam == nullptr)
      return false;
    pLeader = pTeam->getLeader();
  }
  if (pLeader == nullptr)
    return false;
  if (pTeam == nullptr)
    return false;

  // remove cat if member full
  if (pTeam->getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
    pTeam->removeCat(ECATREMOVEMETHOD_EARLY);

  // check team max member
  if (pTeam->getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
  {
    thisServer->sendMsg(pLeader->getZoneID(), pLeader->getGUID(), 331);
    return false;
  }

  // add member
  TMember *pMember = NEW TMember(pTeam, rUser, ETEAMJOB_MEMBER);
  if (addMember(pTeam, pMember) == false)
  {
    SAFE_DELETE(pMember);
    return false;
  }

  XLOG << "[队伍管理-邀请答复]" << user.accid() << user.charid() << user.profession() << user.name()
    << "接受" << qwLeaderID << "邀请进入了" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
  return true;
}

bool TeamManager::applyTeam(const UserInfo& rApply, QWORD teamid)
{
  const SocialUser& rUser = rApply.user();
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == true)
    return false;

  Team* pTeam = getTeamByID(teamid);
  if (pTeam == nullptr)
    return false;

  if (pTeam->isMatchCreate() && pTeam->isMatchNoAddMember())
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 25929);
    return false;
  }

  if (pTeam->getMinReqLv() > rUser.baselv())
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 305, MsgParams(pTeam->getName()));
    return false;
  }

  bool bNtf = true;
  if (pTeam->getMemberCount(false) >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
  {
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 302);
    bNtf = false;
  }

  // check auto accept
  if (pTeam->canAutoEnter(rApply.user()) == true)
  {
    TMember *pMember = NEW TMember(pTeam, rApply, ETEAMJOB_MEMBER);
    if (addMember(pTeam, pMember))
    {
      XLOG << "[队伍管理-申请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
        << "申请加入" << pTeam->getGUID() << pTeam->getName() << "队伍,队伍设置为自动批转并并成功进入" << XEND;
      return true;
    }
    else
    {
      SAFE_DELETE(pMember);
    }
  }

  // apply to list
  TMember *pApply = NEW TMember(pTeam, rApply, ETEAMJOB_APPLY);
  if (pTeam->addApply(pApply) == false)
  {
    SAFE_DELETE(pApply);
    return false;
  }

  TMember* pLeaderMember = pTeam->getLeader();
  if (nullptr != pLeaderMember && pLeaderMember->isOnline() == true)
    thisServer->sendMsg(pLeaderMember->getZoneID(), pLeaderMember->getGUID(), 322, MsgParams(rUser.name()));
  else
    pLeaderMember = pTeam->getTempLeader();

  if (nullptr != pLeaderMember && pLeaderMember->isOnline() == true)
    syncRedTip(teamid, pLeaderMember->getGUID(), true);
  else
    pTeam->setRedTip(true);

  addApplyTeam(rUser.charid(), pTeam);

  // 发送提示给自己
  if (bNtf)
    thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 335);
  XLOG << "[队伍管理-申请]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "申请加入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Apply;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rApply.user().zoneid(),
    rApply.user().accid(),
    rApply.user().charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().TeamLog(thisServer,
    rApply.user().accid(),
    rApply.user().charid(),
    eType,
    eid,
    pTeam->getGUID(),
    rApply.user().charid()
  );

  return true;
}

bool TeamManager::applyTeamForce(const UserInfo& rApply, QWORD teamid)
{
  const SocialUser& rUser = rApply.user();
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == true)
    return false;

  Team* pTeam = getTeamByID(teamid);
  if (pTeam == nullptr)
    return false;

  DWORD maxcnt = MiscConfig::getMe().getTeamCFG().dwMaxMember;
  if (pTeam->getMemberCount() >= maxcnt)
  {
    pTeam->removeCat(ECATREMOVEMETHOD_EARLY);
    if (pTeam->getMemberCount() >= maxcnt)
    {
      XERR << "[队伍管理-pvp自动申请] 失败，队伍人数超标" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
        << "申请加入" << pTeam->getGUID() << pTeam->getName()<<"count"<< pTeam->getMemberCount(false) << XEND;
      return false;
    }
  }

  TMember *pMember = NEW TMember(pTeam, rApply, ETEAMJOB_MEMBER);
  if (pMember == nullptr)
  {
    XERR << "[队伍管理-pvp自动申请] 失败,创建失败" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << "申请加入" << pTeam->getGUID() << pTeam->getName() <<XEND;
    return false;
  }
  pMember->getGCharReader().getByPvpTeam();
  if (addMember(pTeam, pMember, true))
  {
    XLOG << "[队伍管理-pvp自动申请] 成功" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << "申请加入" << pTeam->getGUID() << pTeam->getName() <<XEND;
    return true;
  }
  else
  {
    SAFE_DELETE(pMember);
    XERR << "[队伍管理-pvp自动申请] 失败" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
      << "申请加入" << pTeam->getGUID() << pTeam->getName() << XEND;
  }
  return true;
}


bool TeamManager::processApplyTeam(ETeamApplyType eType, const SocialUser& rLeader, QWORD id)
{
  if (rLeader.charid() == 0)
    return false;

  Team* pTeam = TeamManager::getMe().getTeamByUserID(rLeader.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pMember = pTeam->getMember(rLeader.charid());
  if (pMember == nullptr || pMember->isLeader() == false)
    return false;

  TMember* pApply = pTeam->getApply(id);
  if (pApply == nullptr)
    return false;

  if (isTeamMember(id) == true)
  {
    removeApplyTeam(id, pTeam);
    thisServer->sendMsg(rLeader.zoneid(), rLeader.charid(), 336, MsgParams(pApply->getName()));
    return pTeam->removeApply(id);
  }

  if (eType == ETEAMAPPLYTYPE_DISAGREE)
  {
    thisServer->sendMsg(pApply->getZoneID(), pApply->getGUID(), 303);

    XLOG << "[队伍管理-申请答复]" << rLeader.accid() << rLeader.charid() << rLeader.profession() << rLeader.name()
      << "拒绝了" << id << pApply->getName() << "加入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;
  }
  else if (eType == ETEAMAPPLYTYPE_AGREE)
  {
    // remove cat if member full
    if (pTeam->getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
      pTeam->removeCat(ECATREMOVEMETHOD_EARLY);

    if (pTeam->getMemberCount() >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
    {
      thisServer->sendMsg(rLeader.zoneid(), rLeader.charid(), 331);
      return false;
    }

    if (addMember(pTeam, pApply) == false)
      return false;

    removeApplyTeam(id, pTeam);
    thisServer->sendMsg(rLeader.zoneid(), id, 312, MsgParams(pTeam->getName()));
    XLOG << "[队伍管理-申请答复]" << rLeader.accid() << rLeader.charid() << rLeader.profession() << rLeader.name()
      << "同意了" << id << pApply->getName() << "加入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;

    return true;
  }
  else
  {
    return false;
  }

  removeApplyTeam(id, pTeam);
  return pTeam->removeApply(id);
}

bool TeamManager::kickMember(const SocialUser& rLeader, QWORD userid)
{
  if (rLeader.charid() == 0 || isTeamMember(rLeader.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rLeader.charid());
  if (pTeam == nullptr)
    return false;

  // 组队排位赛不可踢出
  if (pTeam->isMatchCreate() && pTeam->getMatchPvpType() == EPVPTYPE_TEAMPWS)
  {
    XERR << "[队伍管理-剔除], 队伍处于排位赛中, 不可踢人, 队伍:" << pTeam->getGUID() << "队长:" << rLeader.charid() << "队员:" << userid << XEND;
    return false;
  }

  TMember* pLeaderMember = pTeam->getMember(rLeader.charid());
  if (pLeaderMember == nullptr || pLeaderMember->isLeader() == false)
    return false;

  TMember* pMember = pTeam->getMember(userid);
  if (pMember == nullptr)
    return false;

  DWORD dwZoneID = pMember->getZoneID();
  bool bIsOnline = pMember->isOnline();
  bool bLeader = pMember->isLeader();
  string name = pMember->getName();
  if (removeMember(userid) == false)
    return false;

  if (bIsOnline)
  {
    ExitTeam cmd;
    cmd.set_teamid(pTeam->getGUID());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(dwZoneID, userid, send, len);
    thisServer->sendMsg(dwZoneID, userid, 314);
  }

  /*if (bLeader)
  {
    pLeaderMember->setJob(ETEAMJOB_LEADER);
    pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_TEMPLEADER_FIRE, MsgParams(pLeaderMember->getName()));
  }*/

  if (thisServer->getZoneCategory() || pTeam->isMatchCreate())
  {
    //kick old team
    Cmd::KickTeamMatchSCmd cmd;
    cmd.set_teamid(pTeam->getGUID());
    cmd.set_charid(userid);
    cmd.set_roomid(pTeam->getPvpRoomId());
    PROTOBUF(cmd, send, len);
    bool ret = thisServer->forwardCmdToMatch(send, len);
    XLOG << "[斗技场-踢出队伍] 通知外面的队伍踢出玩家 teamid " <<pTeam->getGUID()<<"roomid"<<pTeam->getPvpRoomId()<< "charid" << userid << "发送ret" << ret << XEND;
  }

  checkQuickEnter(pTeam);
  XLOG << "[队伍管理-剔除]" << rLeader.accid() << rLeader.charid() << rLeader.profession() << rLeader.name()
    << "把" << userid << name << "踢出了" << pTeam->getGUID() << pTeam->getName() << "队伍" << (bLeader ? "成为了队长" : "") << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Kick;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rLeader.zoneid(),
    rLeader.accid(),
    rLeader.charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().TeamLog(thisServer,
    rLeader.accid(),
    rLeader.charid(),
    eType,
    eid,
    pTeam->getGUID(),
    userid);

  return true;
}

bool TeamManager::kickCat(const SocialUser& rLeader, QWORD userid, DWORD catid)
{
  if (rLeader.charid() == 0 || isTeamMember(rLeader.charid()) == false || catid == 0)
    return false;

  Team* pTeam = getTeamByUserID(rLeader.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pMember = pTeam->getMember(rLeader.charid());
  TMember* pTarget = pTeam->getMember(userid);
  if (pMember == nullptr || pTarget == nullptr)
    return false;
  if (pTarget->getCatID() != catid)
    return false;
  if (pMember->isLeader() == false && pTarget->getCatOwnerID() != userid / ONE_THOUSAND)
    return false;

  string name = pMember->getName();
  if (pTeam->removeCat(userid, catid, true) == false)
    return false;

  checkQuickEnter(pTeam);
  XLOG << "[队伍管理-剔除]" << rLeader.accid() << rLeader.charid() << rLeader.profession() << rLeader.name()
    << "把" << userid << name << "踢出了" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Kick;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rLeader.zoneid(),
    rLeader.accid(),
    rLeader.charid(),
    eid,
    0,  //charge
    eType, 0, 1);
  PlatLogManager::getMe().TeamLog(thisServer,
    rLeader.accid(),
    rLeader.charid(),
    eType,
    eid,
    pTeam->getGUID(),
    userid);

  return true;

}

bool TeamManager::exchangeLeader(const SocialUser& rLeader, QWORD userid)
{
  if (rLeader.charid() == 0 || isTeamMember(rLeader.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rLeader.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pLeaderMember = pTeam->getMember(rLeader.charid());
  if (pLeaderMember == nullptr || pLeaderMember->isOnline() == false)
    return false;

  TMember* pMember = pTeam->getMember(userid);
  if (pMember == nullptr)
    return false;
  if (pMember->getCatID() != 0)
    return false;

  if(pLeaderMember->getJob() == ETEAMJOB_TEMPLEADER && pMember->isOnline() == false)
  {
    thisServer->sendMsg(rLeader.zoneid(), rLeader.charid(), 353);
    return false;
  }

  if (pTeam->setLeader(pMember->getGUID(), pLeaderMember->getJob()) == false)
    return false;

  thisServer->sendMsg(rLeader.zoneid(), rLeader.charid(), 319, MsgParams(pMember->getName()));
  thisServer->sendMsg(pMember->getZoneID(), pMember->getGUID(), 320, MsgParams(pLeaderMember->getName()));

  XLOG << "[队伍管理-队长交换]" << rLeader.accid() << rLeader.charid() << rLeader.profession() << rLeader.name() << "把队长移交给了" << userid << pMember->getName() << XEND;
  return true;
}

bool TeamManager::exitTeam(const SocialUser& rUser)
{
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == false)
    return false;

  Team *pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;
  if (removeMember(rUser.charid()) == false)
    return false;

  ExitTeam cmd;
  cmd.set_teamid(pTeam->getGUID());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);

  //发送离队提示 msg id = 313
  thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 313);
  //发送离队通知 msg id = 318
  pTeam->broadcastMsg(318, MsgParams(rUser.name()));

  if (thisServer->getZoneCategory() || pTeam->isMatchCreate())
  {
    //kick old team
    Cmd::KickTeamMatchSCmd cmd;
    cmd.set_teamid(pTeam->getGUID());
    cmd.set_charid(rUser.charid());
    cmd.set_roomid(pTeam->getPvpRoomId());
    PROTOBUF(cmd, send, len);
    bool ret = thisServer->forwardCmdToMatch(send, len);
    XLOG << "[斗技场-踢出队伍] 通知外面的队伍踢出玩家 teamid " <<pTeam->getGUID()<<"roomid"<<pTeam->getPvpRoomId()<< "charid" << rUser.charid() << "发送ret" << ret << XEND;
  }

  XLOG << "[队伍管理-退出]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "退出了" << pTeam->getGUID() << "队伍" << XEND;

  //log
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Exit;
  PlatLogManager::getMe().eventLog(thisServer,
    0,
    rUser.zoneid(),
    rUser.accid(),
    rUser.charid(),
    eid,
    0,  //charge
    eType, 0, 1);

  PlatLogManager::getMe().TeamLog(thisServer,
    rUser.accid(),
    rUser.charid(),
    eType,
    eid,
    pTeam->getGUID(),
    rUser.charid()
   );

  // check last team member
  if (pTeam->getMemberCount(false) <= 1)
  {
    const TMapTeamMember& mapMembers = pTeam->getTeamMembers();
    TMember* p = nullptr;
    for (auto &m : mapMembers)
    {
      if (m.second->getCatID() == 0)
      {
        p = m.second;
        break;
      }
    }
    if (p)
    {
      SocialUser oLast;
      oLast.set_accid(p->getAccid());
      oLast.set_charid(p->getGUID());
      oLast.set_zoneid(p->getZoneID());
      oLast.set_profession(p->getProfession());
      oLast.set_name(p->getName());

      //log
      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eType = EventType_Team_Dismiss;
      PlatLogManager::getMe().eventLog(thisServer,
          0,
          p->getZoneID(),
          p->getAccid(),
          p->getGUID(),
          eid,
          0,  //charge
          eType, 0, 1);

      PlatLogManager::getMe().TeamLog(thisServer,
          p->getAccid(),
          p->getGUID(),
          eType,
          eid,
          pTeam->getGUID(),
          0
          );

      if (exitTeam(oLast) == true)
      {
        XLOG << "[队伍管理-退出] 由于" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "退出了" << pTeam->getGUID() << "队伍,队伍只有一个人"
          << oLast.accid() << oLast.charid() << oLast.profession() << oLast.name() << "一起退出了组队" << XEND;
      }
      else
      {
        XERR << "[队伍管理-退出]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "退出了" << pTeam->getGUID() << "队伍,队伍只有一个人"
          << oLast.accid() << oLast.charid() << oLast.profession() << oLast.name() << "退出组队失败" << XEND;
      }
    }
  }
  else
  {
    checkQuickEnter(pTeam);
  }
  return true;
}

bool TeamManager::lockTarget(const SocialUser& rUser, QWORD targetID)
{
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pMember = pTeam->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  pMember->setTarget(targetID);
  // to prevent delay and update data immeditely
  pTeam->timer(xTime::getCurSec());
  return true;
}

bool TeamManager::summon(const SocialUser& rUser, DWORD raidid)
{
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;

  return pTeam->summon(rUser.charid(), raidid);
}

bool TeamManager::clearApplyList(const SocialUser& rUser)
{
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pLeader = pTeam->getLeader();
  if (pLeader == nullptr)
    return false;

  pTeam->clearApply();
  return true;
}

bool TeamManager::quickEnter(const UserInfo& rUser, DWORD dwType, bool bSet)
{
  if (rUser.user().charid() == 0 || isTeamMember(rUser.user().charid()) == true)
    return false;

  DWORD dwTime = 0;
  auto m = m_mapQuickEnterUser.find(rUser.user().charid());
  if (bSet)
  {
    if (m != m_mapQuickEnterUser.end())
      return false;

    const TeamGoalBase* pTeamCFG = TableManager::getMe().getTeamCFG(dwType);
    if (pTeamCFG == nullptr)
      return false;

    const STeamCFG& rCFG = MiscConfig::getMe().getTeamCFG();

    DWORD dwDeltaLv = DWORD_MAX;
    Team* pTeam = nullptr;
    for (auto m : xEntryID::ets_)
    {
      Team *p = (Team *)m.second;
      if (p->getAuto() == false || p->getMemberCount() >= rCFG.dwMaxMember || p->getOnlineMemberCount() == 0)
        continue;
      if (rUser.user().baselv() < p->getMinReqLv())
        continue;
      if (checkTeam(pTeamCFG, p) == false)
        continue;
      TMember* pLeader = p->getLeader();
      if (pLeader == nullptr || pLeader->getZoneID() != rUser.user().zoneid())
        continue;

      DWORD dwTmp = rUser.user().baselv() > pLeader->getBaseLv() ? rUser.user().baselv() - pLeader->getBaseLv() : pLeader->getBaseLv() - rUser.user().baselv();
      if (dwTmp < dwDeltaLv)
      {
        dwDeltaLv = dwTmp;
        pTeam = p;
      }
    }
    if (pTeam != nullptr)
    {
      TMember *pMember = NEW TMember(pTeam, rUser, ETEAMJOB_MEMBER);
      if (addMember(pTeam, pMember) == true)
      {
        XLOG << "[队伍管理-快速加入申请]" << rUser.user().accid() << rUser.user().charid() << rUser.user().profession() << rUser.user().name()
          << "成功加入" << pTeam->getGUID() << pTeam->getName() << XEND;
        return true;
      }
      else
      {
        SAFE_DELETE(pMember);
      }
    }

    SQuickUser& rQuickUser = m_mapQuickEnterUser[rUser.user().charid()];
    rQuickUser.pBase = pTeamCFG;
    dwTime = rQuickUser.dwEndTime = xTime::getCurSec() + MiscConfig::getMe().getTeamCFG().dwQuickEnterTime;
    m_mapInviteInfo[rUser.user().charid()].CopyFrom(rUser);
  }
  else
  {
    if (m == m_mapQuickEnterUser.end())
      return false;

    m_mapQuickEnterUser.erase(m);
  }

  QuickEnter cmd;
  cmd.set_type(dwType);
  cmd.set_time(dwTime);
  cmd.set_set(bSet);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.user().zoneid(), rUser.user().charid(), send, len);
  XLOG << "[队伍管理-快速加入申请]" << rUser.user().accid() << rUser.user().charid() << rUser.user().profession() << rUser.user().name()
    << (bSet ? "开启" : "关闭") <<"了 type :" << dwType << "快速加入" << XEND;
  return true;
}

bool TeamManager::setTeamOption(const SocialUser& rUser, const SetTeamOption& cmd)
{
  if (rUser.charid() == 0 || isTeamMember(rUser.charid()) == false)
    return false;

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;
  TMember* pLeader = pTeam->getMember(rUser.charid());
  if (pLeader == nullptr || pLeader->isLeader() == false || pLeader->isOnline() == false)
    return false;

  if (cmd.name().empty() == false && cmd.name() != pTeam->getName())
  {
    const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
    ESysMsgID eMsg = rCFG.checkNameValid(cmd.name(), ENAMETYPE_TEAM);
    if (eMsg != ESYSTEMMSG_ID_MIN)
    {
      thisServer->sendMsg(rUser.zoneid(), rUser.charid(), eMsg);
    }
    else
    {
      std::string oldName = pTeam->getName();
      pTeam->setName(cmd.name());

      //log change team name
      QWORD eid = xTime::getCurUSec();
      EVENT_TYPE eType = EventType_Change_TeamName;
      PlatLogManager::getMe().eventLog(thisServer,
        0,
        rUser.zoneid(),
        rUser.accid(),
        rUser.charid(),
        eid,
        0,  //charge
        eType, 0, 1);
      PlatLogManager::getMe().changeLog(thisServer,
        0,
        rUser.zoneid(),
        rUser.accid(),
        rUser.charid(),
        eType,
        eid,
        EChange_TeamName,
        oldName,
        cmd.name(),
        pTeam->getGUID());
    }
  }
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    DWORD dwValue = static_cast<DWORD>(cmd.items(i).value());
    switch (cmd.items(i).type())
    {
      case ETEAMDATA_MIN:
        break;
      case ETEAMDATA_TYPE:
        {
          const TeamGoalBase* pTeamCFG = TableManager::getMe().getTeamCFG(dwValue);
          if (pTeamCFG != nullptr && pTeamCFG->getTableInt("SetShow") != 0)
          {
            auto m = m_mapType2Team.find(pTeam->getTeamType()->getTableInt("id"));
            if (m != m_mapType2Team.end())
              m->second.erase(pTeam->getGUID());

            DWORD dwOldGlobal = 0;
            const TeamGoalBase* pOld = pTeam->getTeamType();
            if (pOld)
              dwOldGlobal = pOld->id;

            pTeam->setTeamType(pTeamCFG);
            m_mapType2Team[pTeam->getTeamType()->getTableInt("id")].insert(pTeam->getGUID());

            //log change team global
            //plat log
            QWORD eid = xTime::getCurUSec();
            EVENT_TYPE eType = EventType_Change_TeamGlobal;
            PlatLogManager::getMe().eventLog(thisServer,
              0,
              rUser.zoneid(),
              rUser.accid(),
              rUser.charid(),
              eid,
              0,  //charge
              eType, 0, 1);

            PlatLogManager::getMe().changeLog(thisServer,
              0,
              rUser.zoneid(),
              rUser.accid(),
              rUser.charid(),
              eType,
              eid,
              EChange_TeamGlobal,
              dwOldGlobal,
              dwValue,
              pTeam->getGUID());
            XLOG << "[队伍管理-选项设置]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "设置队伍目标为" << dwValue << XEND;
            checkQuickEnter(pTeam);
          }
        }
        break;
      case ETEAMDATA_MINLV:
        pTeam->setMinReqLv(dwValue);
        XLOG << "[队伍管理-选项设置]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "设置队伍最小等级限制" << dwValue << "级" << XEND;
        break;
      case ETEAMDATA_MAXLV:
        pTeam->setMaxReqLv(dwValue);
        XLOG << "[队伍管理-选项设置]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "设置队伍最大等级限制" << dwValue << "级" << XEND;
        break;
      case ETEAMDATA_OVERTIME:
        break;
      case ETEAMDATA_AUTOACCEPT:
        {
          if (dwValue >= EAUTOTYPE_MAX)
            return false;
          thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_TEAM_AUTOTYPE_CHANGE);
          pTeam->setAuto(static_cast<EAutoType>(dwValue));
          XLOG << "[队伍管理-选项设置]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "设置队伍自动选项 :" << dwValue << XEND;
          if (pTeam->getAuto() != EAUTOTYPE_CLOSE && pTeam->getAuto() != EAUTOTYPE_MAX)
          {
            TMapTeamMember mapApply = pTeam->getApplyList();
            auto v = mapApply.begin();
            auto tmp = v;
            for ( ; v != mapApply.end(); )
            {
              tmp = v++;

              if (!tmp->second) continue;

              if (tmp->second->isOnline() == false)
                continue;

              SocialUser user;
              user.set_accid(tmp->second->getAccid());
              user.set_charid(tmp->second->getGUID());
              user.set_baselv(tmp->second->getBaseLv());
              if (pTeam->canAutoEnter(user) == false)
                continue;

              if (addMember(pTeam, tmp->second) == false)
                continue;

              tmp->second->setJob(ETEAMJOB_MEMBER);

              mapApply.erase(tmp);
            }

            checkQuickEnter(pTeam);
          }
        }
        break;
      case ETEAMDATA_MEMBERCOUNT:
        break;
      case ETEAMDATA_PICKUP_MODE:
        {
          pTeam->setPickupMode(dwValue);
          XLOG << "[队伍管理-选项设置]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "设置拾取方式" << dwValue << XEND;
          DWORD msgId = dwValue == 0 ? 337 : 338;
          pTeam->broadcastMsg(msgId);
          break;
        }
      case ETEAMDATA_MEMBER:
      case ETEAMDATA_HELPWANTED:
      case ETEAMDATA_MAX:
        break;
    }
  }

  return true;
}

bool TeamManager::queryUserTeamInfo(const SocialUser& rUser, QWORD qwCharID)
{
  if (rUser.charid() == 0 || qwCharID == 0)
    return false;

  QueryUserTeamInfoTeamCmd cmd;
  Team* pTeam = getTeamByUserID(qwCharID);
  cmd.set_charid(qwCharID);
  cmd.set_teamid(pTeam == nullptr ? 0 : pTeam->getGUID());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToMe(rUser.zoneid(), rUser.charid(), send, len);
  return true;
}

bool TeamManager::setMemberOption(const SocialUser& rUser, const SetMemberOptionTeamCmd& cmd)
{
  if (rUser.charid() == 0)
    return false;

  Team* pTeam = getTeamByUserID(rUser.charid());
  if (pTeam == nullptr)
    return false;

  TMember* pMember = pTeam->getMember(rUser.charid());
  if (pMember == nullptr)
    return false;

  if (pMember->getAutoFollow() != cmd.autofollow())
  {
    pMember->setAutoFollow(cmd.autofollow());
    XDBG << "[队伍管理-设置选项]" << pMember->getAccid() << pMember->getGUID() << pMember->getProfession() << pMember->getName() << "设置 autofollow :" << pMember->getAutoFollow() << XEND;
  }

  return true;
}

bool TeamManager::addMember(Team* pTeam, TMember *pMember, bool force /*=false*/)
{
  if (!pMember) return false;
  if (pTeam == nullptr || isTeamMember(pMember->getGUID()) == true)
    return false;

  DWORD count = pTeam->getMemberCount();
  if (count >= MiscConfig::getMe().getTeamCFG().dwMaxMember)
    return false;

  if (!force && pTeam->isMatchCreate() && pTeam->isMatchNoAddMember())
    return false;

  clearInviteMember(pMember->getGUID());
  if (pTeam->addMember(pMember) == false)
  {
    XERR << "[队伍管理-添加队员]" << pMember->getAccid() << pMember->getGUID() << pMember->getProfession() << pMember->getName() << "加入" << pTeam->getGUID() << pTeam->getName() << "队伍失败" << XEND;
    return false;
  };
  removeApplyTeam(pMember->getGUID(), pTeam);
  m_mapUserID2Team[pMember->getGUID()] = pTeam->getGUID();
  XLOG << "[队伍管理-添加队员]" << pMember->getAccid() << pMember->getGUID() << pMember->getProfession() << pMember->getName()
    << "成功加入" << pTeam->getGUID() << pTeam->getName() << "队伍" << XEND;

  for (auto &t : pTeam->getTeamMembers())
  {
    if (!t.second) continue;

    if (pMember->getGUID() == t.second->getGUID())
      continue;
    if (pMember->getCatID() != 0 || t.second->getCatID() != 0)
      continue;

    if (pMember->checkRelation(t.second->getGUID(), ESOCIALRELATION_TEAM) == false)
    {
      AddRelationTeamCmd cmd;
      cmd.set_charid(pMember->getGUID());
      cmd.set_targetid(t.second->getGUID());
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");
    }
    if (t.second->checkRelation(pMember->getGUID(), ESOCIALRELATION_TEAM) == false)
    {
      AddRelationTeamCmd cmd;
      cmd.set_charid(t.second->getGUID());
      cmd.set_targetid(pMember->getGUID());
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");
    }
  }

  // platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Team_Join;
  PlatLogManager::getMe().eventLog(thisServer, 0, pMember->getZoneID(), pMember->getAccid(), pMember->getGUID(), eid, 0, eType, 0, 1);
  PlatLogManager::getMe().TeamLog(thisServer, pMember->getAccid(), pMember->getGUID(), eType, eid, pTeam->getGUID(), pMember->getGUID());

  return true;
}

bool TeamManager::removeMember(QWORD id)
{
  auto m = m_mapUserID2Team.find(id);
  if (m == m_mapUserID2Team.end())
    return false;

  Team *pTeam = getTeamByID(m->second);
  if (pTeam == nullptr)
    return false;
  if (pTeam->removeMember(id) == false)
    return false;

  syncRedTip(m->second, id, false);

  m_mapUserID2Team.erase(m);
  pTeam->callCatEnter();
  return true;
}

bool TeamManager::addApplyTeam(QWORD qwCharID, Team* pTeam)
{
  if (qwCharID == 0 || pTeam == nullptr)
    return false;
  m_mapUserApplyTeam[qwCharID].insert(pTeam->getGUID());
  return true;
}

bool TeamManager::removeApplyTeam(QWORD qwCharID, Team* pTeam)
{
  if (qwCharID == 0 || pTeam == nullptr)
    return false;

  auto m = m_mapUserApplyTeam.find(qwCharID);
  if (m == m_mapUserApplyTeam.end())
    return false;

  m->second.erase(pTeam->getGUID());
  if (m->second.empty() == true)
    m_mapUserApplyTeam.erase(m);
  return true;
}

bool TeamManager::removeApplyTeam(Team* pTeam)
{
  if (pTeam == nullptr)
    return false;

  for (auto m = m_mapUserApplyTeam.begin(); m != m_mapUserApplyTeam.end();)
  {
    m->second.erase(pTeam->getGUID());
    if (m->second.empty() == true)
      m = m_mapUserApplyTeam.erase(m);
    else
      ++m;
  }
  return true;
}

bool TeamManager::checkTeam(const TeamGoalBase* pTeamCFG, Team* pTeam)
{
  if (pTeam == nullptr || pTeamCFG == nullptr)
    return false;

  TMember* pLeader = pTeam->getLeader();
  if (pLeader == nullptr || pLeader->isOnline() == false)
  {
    pLeader = pTeam->getTempLeader();
    if (pLeader == nullptr || pLeader->isOnline() == false)
      return false;
  }

  ETeamFilter eFilter = pTeamCFG->getTeamFilter();
  TVecDWORD vecFilters;
  if (eFilter == ETEAMFILTER_MIN)
  {
    return true;
  }
  else if (eFilter == ETEAMFILTER_MAP)
  {
    pTeamCFG->collectFilterValue(vecFilters);
    return std::find(vecFilters.begin(), vecFilters.end(), pLeader->getMapID()) != vecFilters.end();
  }
  else if (eFilter == ETEAMFILTER_TOWER)
  {
    return true;
  }
  else if (eFilter == ETEAMFILTER_LABORATORY)
  {
    const TeamGoalBase* pSelfCFG = pTeam->getTeamType();
    if (pSelfCFG == nullptr)
      return false;
    pTeamCFG->collectFilterValue(vecFilters);
    if (vecFilters.empty() == true)
      return false;
    DWORD dwType = pSelfCFG->getTableInt("type");
    return dwType == vecFilters[0];
  }
  else if (eFilter == ETEAMFILTER_MAPRANGE)
  {
    pTeamCFG->collectFilterValue(vecFilters);
    auto v = find_if(vecFilters.begin(), vecFilters.end(), [pLeader](DWORD d) -> bool{
      const SMapCFG* pBase = MapConfig::getMe().getMapCFG(pLeader->getMapID());
      if (pBase == nullptr)
        return false;
      return d == pBase->dwRange;
    });
    return v != vecFilters.end();
  }
  else if (eFilter == ETEAMFILTER_SEAL || eFilter == ETEAMFILTER_DOJO || eFilter == ETEAMFILTER_CHAT)
  {
    return true;
  }

  return false;
}

void TeamManager::updateQuickUser(DWORD curSec)
{
  // collect valid members
  TSetQWORD setMembers;
  for (auto m = m_mapQuickEnterUser.begin(); m != m_mapQuickEnterUser.end();)
  {
    if (m->second.dwEndTime < curSec)
    {
      setMembers.insert(m->first);
      m = m_mapQuickEnterUser.erase(m);
      continue;
    }

    ++m;
  }

  // create team if has member
  if (setMembers.empty() == false)
  {
    // collect leader
    const UserInfo* pLeader = nullptr;
    for (auto s = setMembers.begin(); s != setMembers.end();)
    {
      pLeader = getInviteInfo(*setMembers.begin());
      if (pLeader == nullptr)
      {
        s = setMembers.erase(s);
        continue;
      }

      break;
    }

    // create team
    if (pLeader != nullptr)
    {
      const STeamCFG& rCFG = MiscConfig::getMe().getTeamCFG();
      string name = pLeader->user().name() + rCFG.sDefaultName;
      Team* pTeam = TeamManager::getMe().createOneTeam(name, *pLeader, rCFG.dwDefaultType, rCFG.dwDefaultMinLv, rCFG.dwDefaultMaxLv, rCFG.eDefaultAuto);
      if (pTeam != nullptr)
      {
        const SocialUser& rUser = pLeader->user();
        XLOG << "[队伍管理-快速加入]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
          << rCFG.dwQuickEnterTime << "秒后未找到符合的队伍,自动创建队伍" << pTeam->getGUID() << pTeam->getName() << XEND;

        for (auto &s : setMembers)
        {
          if (s == pLeader->user().charid())
            continue;
          const UserInfo* pInfo = getInviteInfo(s);
          if (pInfo == nullptr)
            continue;

          TMember *pMember = NEW TMember(pTeam, *pInfo, ETEAMJOB_MEMBER);
          if (addMember(pTeam, pMember) == false)
          {
            SAFE_DELETE(pMember);
            XERR << "[队伍管理-快速加入]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
              << rCFG.dwQuickEnterTime << "秒后未找到符合的队伍,自动加入队伍" << pTeam->getGUID() << pTeam->getName() << "失败" << XEND;
            continue;
          }

          XLOG << "[队伍管理-快速加入]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << rCFG.dwQuickEnterTime << "秒后未找到符合的队伍,自动加入队伍" << pTeam->getGUID() << pTeam->getName() << XEND;
        }
      }
    }
  }
}

const UserInfo* TeamManager::getInviteInfo(QWORD qwCharID) const
{
  auto m = m_mapInviteInfo.find(qwCharID);
  if (m != m_mapInviteInfo.end())
    return &m->second;
  return nullptr;
}

bool TeamManager::addLeaderChangeCache(const SocialUser& rUser)
{
  auto v = find_if(m_vecLeaderChangeCache.begin(), m_vecLeaderChangeCache.end(), [&](const pair<SocialUser, DWORD>& r) -> bool{
    return r.first.charid() == rUser.charid();
  });
  if (v != m_vecLeaderChangeCache.end())
    v->second = xTime::getCurSec();
  else
    m_vecLeaderChangeCache.push_back(pair<SocialUser, DWORD>(rUser, xTime::getCurSec()));

  XDBG << "[队伍管理-添加队长切换]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "进入队长缓存队列 total :" << m_vecLeaderChangeCache.size() << XEND;
  return true;
}

bool TeamManager::removeLeaderChangeCache(QWORD qwCharID)
{
  auto v = find_if(m_vecLeaderChangeCache.begin(), m_vecLeaderChangeCache.end(), [&](const pair<SocialUser, DWORD>& r) -> bool{
    return r.first.charid() == qwCharID;
  });
  if (v != m_vecLeaderChangeCache.end())
  {
    m_vecLeaderChangeCache.erase(v);
    XDBG << "[队伍管理-移除队长切换]" << qwCharID << "从队长缓存队列移除 total :" << m_vecLeaderChangeCache.size() << XEND;
    return true;
  }
  return false;
}

void TeamManager::updateLeaderChangeCache(DWORD curTime)
{
  for (auto v = m_vecLeaderChangeCache.begin(); v != m_vecLeaderChangeCache.end();)
  {
    if (v->second + 10 < curTime)
    {
      Team* pTeam = getTeamByUserID(v->first.charid());
      if (pTeam == nullptr)
      {
        v = m_vecLeaderChangeCache.erase(v);
        continue;
      }

      TMember* pLeader = pTeam->getLeader();
      if (pLeader != nullptr && pLeader->getGUID() == v->first.charid())
        pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_MEMBER_OFFLINE, MsgParams(v->first.name()));

      resetTempLeader(pTeam);
      v = m_vecLeaderChangeCache.erase(v);
      XDBG << "[队伍管理-队长离线缓存] 数量 :" << m_vecLeaderChangeCache.size() << XEND;
      continue;
    }

    ++v;
  }
}

void TeamManager::resetTempLeader(Team* pTeam)
{
  if (pTeam == nullptr)
    return;

  TMember* pLeader = pTeam->getLeader();
  if (pLeader == nullptr)
    return;

  if (pLeader->isOnline() == true)
  {
    for (auto v = pTeam->m_mapMembers.begin(); v != pTeam->m_mapMembers.end(); ++v)
    {
      if (!v->second) continue;
      if (v->second->getJob() == ETEAMJOB_TEMPLEADER)
      {
        v->second->setJob(ETEAMJOB_MEMBER);
        pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_TEMPLEADER_FIRE, MsgParams(pLeader->getName()));
        pLeader->setJob(ETEAMJOB_LEADER);
        XDBG << "[队伍管理-临时队长] 队长" << pLeader->getAccid() << pLeader->getGUID() << pLeader->getProfession() << pLeader->getName()
          << "上线了" << v->second->getAccid() << v->second->getGUID() << v->second->getProfession() << v->second->getName() << "被撤销临时队长的职责" << XEND;
      }
    }
    return;
  }

  for (auto v : pTeam->m_mapMembers)
  {
    if (v.second)
    {
      if (v.second->isOnline() == true && v.second->getJob() == ETEAMJOB_TEMPLEADER)
      {
        return;
      }
    }
  }

  std::multimap<DWORD, TMember*> list;
  for (auto &v : pTeam->m_mapMembers)
  {
    if (v.second)
    {
      list.insert(std::make_pair(v.second->getCreateTime(), v.second));
    }
  }

  string name;
  for (auto &v : list)
  {
    if (v.second)
    {
      if (v.second->getJob() == ETEAMJOB_TEMPLEADER)
      {
        v.second->setJob(ETEAMJOB_MEMBER);
        name = v.second->getName();
        XDBG << "[队伍管理-临时队长]" << v.second->getAccid() << v.second->getGUID() << v.second->getProfession() << v.second->getName() << "不在线,被撤销临时队长的职责" << XEND;
        break;
      }
    }
  }

  for (auto &v : list)
  {
    if (!v.second) continue;
    if (v.second->isOnline() == true)
    {
      v.second->setJob(ETEAMJOB_TEMPLEADER);
      v.second->syncBeLeader();
      pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_TEMPLEADER_DONE, MsgParams(v.second->getName()));
      if (name.empty() == false)
        pTeam->broadcastMsg(ESYSTEMMSG_ID_TEAM_TEMPLEADER_EXCHANGE, MsgParams(name, v.second->getName()));
      pTeam->setApplyUpdate();
      XDBG << "[队伍管理-临时队长]" << v.second->getAccid() << v.second->getGUID() << v.second->getProfession() << v.second->getName() << "成为了临时队长" << XEND;
      break;
    }
  }
}

//通知客户端红点变化
void TeamManager::syncRedTip(QWORD teamid, QWORD userid, bool add)
{
  SyncRedTipSocialCmd cmd;
  cmd.set_dwid(teamid);
  cmd.set_charid(userid);
  cmd.set_red(EREDSYS_TEAMAPPLY);
  cmd.set_add(add);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

bool TeamManager::doPveCardCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;
  if (MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_PVE))
    return false;

  switch(cmd->param)
  {
    case EPVE_INVITE_TEAM_CMD:
      {
        if (MiscConfig::getMe().getMiscPveCFG().canEnter(now()) == false)
        {
          thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 117);
          break;
        }
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        TMember* pLeader = pTeam->getLeader();
        if (pLeader == nullptr || pLeader->isOnline() == false)
        {
          pLeader = pTeam->getTempLeader();
          if (pLeader == nullptr || pLeader->isOnline() == false)
            break;
        }
        if (pLeader->getGUID() != rUser.charid())
          break;

        PARSE_CMD_PROTOBUF(InvitePveCardCmd, rev);
        if (rev.iscancel() == true)
        {
          if (pTeam->isPveFighting() == false)
            pTeam->cancelEnterPve();
          break;
        }

        DWORD limitlv = MiscConfig::getMe().getMiscPveCFG().dwLimitLv;
        const TMapTeamMember& mapMembers = pTeam->getTeamMembers();
        for (auto &v : mapMembers)
        {
          if (!v.second) continue;
          if (v.second->getBaseLv() < limitlv)
            continue;
          if (v.second->getGUID() != pLeader->getGUID())
            v.second->sendCmdToMe(buf, len);
        }
        pTeam->setPveInviteOpen(true);
      }
      break;
    case EPVE_REPLY_TEAM_CMD:
      {
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        PARSE_CMD_PROTOBUF(ReplyPveCardCmd, rev);
        if (pTeam->isPveInviteOpen() == false && rev.agree())
        {
          //thisServer->sendMsg
          break;
        }

        TMember* pMember = pTeam->getMember(rUser.charid());
        if (pMember != nullptr && rev.agree())
          pTeam->addPveEnterMember(rUser.charid());

        TMember* pLeader = pTeam->getLeader();
        if (pLeader == nullptr || pLeader->isOnline() == false)
        {
          pLeader = pTeam->getTempLeader();
          if (pLeader == nullptr || pLeader->isOnline() == false)
            break;
        }
        rev.set_charid(rUser.charid());
        PROTOBUF(rev, retbuf, retlen);
        pLeader->sendCmdToMe(retbuf, retlen);
      }
      break;
    case EPVE_ENTER_RAID_CMD:
      {
        if (MiscConfig::getMe().getMiscPveCFG().canEnter(now()) == false)
        {
          thisServer->sendMsg(rUser.zoneid(), rUser.charid(), 117);
          break;
        }
        PARSE_CMD_PROTOBUF(EnterPveCardCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;

        TMember* pLeader = pTeam->getLeader();
        if (pLeader == nullptr || pLeader->isOnline() == false)
        {
          pLeader = pTeam->getTempLeader();
          if (pLeader == nullptr || pLeader->isOnline() == false)
            break;
        }
        if (pLeader->getGUID() != rUser.charid()) //只能队长开启进入
          break;

        pTeam->createPveCardRaid(rUser, rev.configid());
      }
      break;
    default:
      break;
  }
  return true;
}

bool TeamManager::doTeamRaidCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch(cmd->param)
  {
    case TEAMRAIDPARAM_INVITE:
      {
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        TMember* pLeader = pTeam->getOnlineLeader();
        if(pLeader == nullptr || pLeader->getGUID() != rUser.charid())
            break;

        PARSE_CMD_PROTOBUF(TeamRaidInviteCmd, rev);
        auto it = pTeam->getTeamRaidData(rev.raid_type());
        if(it == nullptr)
        {
          STeamRaid stData;
          stData.eType = rev.raid_type();
          stData.bInviteOn = true;
          pTeam->addTeamRaidData(rev.raid_type(), stData);
        }
        else
          it->clear();

        const TMapTeamMember& mapMembers = pTeam->getTeamMembers();
        for (auto &v : mapMembers)
        {
          if (!v.second) continue;
          if (v.second->getGUID() != pLeader->getGUID())
            v.second->sendCmdToMe(buf, len);
        }
        pTeam->setTeamRaidInviteOpen(rev.raid_type());
     }
      break;
    case TEAMRAIDPARAM_REPLY:
      {
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;
        PARSE_CMD_PROTOBUF(TeamRaidReplyCmd, rev);
        if (pTeam->getTeamRaidInviteOpen(rev.raid_type()) == false || rev.reply() == false)
        {
          //thisServer->sendMsg
          break;
        }

        TMember* pMember = pTeam->getMember(rUser.charid());
        if (pMember != nullptr && rev.reply())
        {
          if(pTeam->addTeamRaidEnterMember(rev.raid_type(), rUser.charid()) == false)
            break;
        }

        TMember* pLeader = pTeam->getOnlineLeader();
        if(pLeader != nullptr)
        {
          rev.set_charid(rUser.charid());
          PROTOBUF(rev, retbuf, retlen);
          pLeader->sendCmdToMe(retbuf, retlen);
        }
      }
      break;
    case TEAMRAIDPARAM_ENTER:
      {
        PARSE_CMD_PROTOBUF(TeamRaidEnterCmd, rev);
        Team* pTeam = getTeamByUserID(rUser.charid());
        if (pTeam == nullptr)
          break;

        pTeam->createTeamRaid(rUser, rev);
      }
      break;
    default:
      break;
  }
  return true;
}

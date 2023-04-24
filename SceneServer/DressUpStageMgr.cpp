#include "DressUpStageMgr.h"
#include "MiscConfig.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneManager.h"
#include "SceneNpcManager.h"

DressUpStageMgr::DressUpStageMgr()
{
}

DressUpStageMgr::~DressUpStageMgr()
{
}

void DressUpStageMgr::init()
{
  m_setStages.clear();

  for(auto &it : MiscConfig::getMe().getDressStageCFG().m_setStageIDs)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(it);
    if(pCFG == nullptr)
    {
      XERR << "[换装舞台] 配置" << "舞台ID " << it << "未在Npc.txt中找到" << XEND;
      continue;
    }
    m_setStages.emplace(it);
  }

  DWORD dwMapID = MiscConfig::getMe().getDressStageCFG().m_dwStaticMap;
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if(pScene == nullptr)
    return;

  xSceneEntrySet set;
  pScene->getAllEntryList(SCENE_ENTRY_NPC, set);

  for(auto iter = m_setStages.begin(); iter != m_setStages.end(); ++iter)
  {
    for (auto it=set.begin(); it!=set.end(); ++it)
    {
      SceneNpc *npc = (SceneNpc *)(*it);
      if (npc->getNpcID() == *iter)
      {
        initAppearance(npc);
        break;
      }
    }
    vector<UserStageInfo> info;
    m_mapWaitUsers.emplace(*iter, info);
  }
}

void DressUpStageMgr::initAppearance(SceneNpc* pNpc)
{
  if(pNpc == nullptr)
    return;

  DWORD stageid = pNpc->getNpcID();
  auto itEquipType = MiscConfig::getMe().getDressStageCFG().m_mapDefaultEquip.find(stageid);
  if(itEquipType == MiscConfig::getMe().getDressStageCFG().m_mapDefaultEquip.end())
    return;

  auto itEquip = itEquipType->second.begin();
  for(; itEquip != itEquipType->second.end(); ++itEquip)
  {
    changeStageAppearance(nullptr, stageid, static_cast<EUserDataType>(itEquip->first), itEquip->second, pNpc);
  }
}

void DressUpStageMgr::timer(DWORD curTime)
{
  if(m_setStages.empty() == true)
    init();

  for(auto it = m_mapWaitUsers.begin(); it != m_mapWaitUsers.end(); ++it)
    checkLineUp(it->first);
}

SceneNpc* DressUpStageMgr::getStageNpc(DWORD stageid)
{
  DWORD dwMapID = MiscConfig::getMe().getDressStageCFG().m_dwStaticMap;
  Scene* pScene = SceneManager::getMe().getSceneByID(dwMapID);
  if(pScene == nullptr)
    return nullptr;

  SceneNpc* pNpc = nullptr;
  xSceneEntrySet set;
  pScene->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto it=set.begin(); it!=set.end(); ++it)
  {
    SceneNpc *npc = (SceneNpc *)(*it);
    if (npc->getNpcID() == stageid)
    {
      pNpc = npc;
      break;
    }
  }

  return pNpc;
}

void DressUpStageMgr::sendStageInfo(SceneUser* pUser, DWORD stageid)
{
  if(pUser == nullptr)
    return;

  QueryStageUserCmd cmd;
  for(auto it = m_mapWaitUsers.begin(); it != m_mapWaitUsers.end(); ++it)
  {
    if(stageid == 0 || it->first == stageid)
    {
      StageInfo* pInfo = cmd.add_info();
      if(pInfo == nullptr)
        continue;
      DWORD curTime = xTime::getCurSec();
      pInfo->set_stageid(it->first);
      DWORD count = it->second.size();
      DWORD waitTime = count * MiscConfig::getMe().getDressStageCFG().m_dwShowTime;
      auto iter = m_mapDisplayStage.find(it->first);
      if(iter != m_mapDisplayStage.end() && iter->second.dwTime >= curTime)
        waitTime += iter->second.dwTime - curTime;
      pInfo->set_usernum(count);
      pInfo->set_waittime(waitTime);
      pInfo->set_status(pUser->getDressUp().getDressUpStatus());
    }
  }

  if(cmd.info_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
#ifdef _DEBUG
    XLOG <<  "[换装舞台-消息], 玩家" << pUser->name << pUser->id << "size: " << cmd.info_size() << "消息内容: " << cmd.ShortDebugString() << XEND;
#endif
  }
}

void DressUpStageMgr::refreshStageInfo(DWORD stageid)
{
  auto it = m_mapWaitUsers.find(stageid);
  if(it == m_mapWaitUsers.end() || it->second.empty() == true)
    return;

  DWORD dwShowLeft = 0;
  DWORD curTime = xTime::getCurSec();
  auto itDisplay = m_mapDisplayStage.find(stageid);
  if(itDisplay != m_mapDisplayStage.end() && itDisplay->second.dwTime >= curTime)
    dwShowLeft = itDisplay->second.dwTime - curTime;

  auto iter = it->second.begin();
  for(DWORD i = 0; i < it->second.size(); ++i)
  {
    for(auto &s : iter->setUsers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(s);
      if(pUser == nullptr)
        continue;
      QueryStageUserCmd cmd;
      StageInfo* pInfo = cmd.add_info();
      if(pInfo == nullptr)
        continue;
      pInfo->set_stageid(stageid);
      pInfo->set_usernum(i);
      DWORD dwTime = MiscConfig::getMe().getDressStageCFG().m_dwShowTime * i + dwShowLeft;
      pInfo->set_waittime(dwTime);
      pInfo->set_status(pUser->getDressUp().getDressUpStatus());
      PROTOBUF(cmd, send, len);
      pUser->sendCmdToMe(send, len);
    }
  }
}

void DressUpStageMgr::changeStageAppearance(SceneUser* pUser, DWORD stageid, EUserDataType eType, DWORD value, SceneNpc* pNpc)
{
  if(pNpc == nullptr)
  {
    if(pUser == nullptr || pUser->getScene() == nullptr || pUser->getDressUp().getDressUpStatus() == EDRESSUP_MIN)
      return;

    pNpc = getStageNpc(stageid);
  }

  if(pNpc == nullptr)
    return;

  if(MiscConfig::getMe().getDressStageCFG().canEquip(stageid, eType, value) == false)
    return;

  if(pUser && pUser->getDressUp().getDressUpStatus() == EDRESSUP_WAIT)
  {
    changeMyStage(pUser, stageid, eType, value, pNpc);
    return;
  }

  switch(eType)
  {
    case EUSERDATATYPE_BACK:
      {
        if(pNpc->define.getBack() == value)
          return;
        pNpc->define.setBack(value);
        pNpc->setDataMark(EUSERDATATYPE_BACK);
        pNpc->refreshDataAtonce();
      }
      break;
    case EUSERDATATYPE_HAIR:
      {
        if(pNpc->define.getHair() == value)
          return;
        pNpc->define.setHair(value);
        pNpc->setDataMark(EUSERDATATYPE_HAIR);
        pNpc->refreshDataAtonce();
      }
      break;
    case EUSERDATATYPE_LEFTHAND:
      {
        if(pNpc->define.getLeftHand() == value)
          return;
        pNpc->define.setLeftHand(value);
        pNpc->setDataMark(EUSERDATATYPE_LEFTHAND);
        pNpc->refreshDataAtonce();
      }
      break;
    case EUSERDATATYPE_RIGHTHAND:
      {
        if(pNpc->define.getRightHand() == value)
          return;
        pNpc->define.setRightHand(value);
        pNpc->setDataMark(EUSERDATATYPE_RIGHTHAND);
        pNpc->refreshDataAtonce();
      }
      break;
    case EUSERDATATYPE_HEAD:
      {
        if(pNpc->define.getHead() == value)
          return;
        pNpc->define.setHead(value);
        pNpc->setDataMark(EUSERDATATYPE_HEAD);
        pNpc->refreshDataAtonce();
      }
      break;
    default:
      return;
  }

  if(pUser != nullptr)
  {
    UserStageInfo* pInfo = getUserStageInfo(pUser);
    if(pInfo != nullptr)
      pInfo->mapAttr[eType] = value;

    XLOG <<  "[换装舞台], 舞台换装" << pUser->accid << pUser->id << pUser->name << "stage: " << stageid << eType << value << XEND;
  }
  else
    XLOG <<  "[换装舞台], 舞台换装" << "初始化 "<< "stage: " << stageid << eType << value << pNpc->define.getDir() << XEND;
}

void DressUpStageMgr::requestAddLine(SceneUser* pUser, DWORD stageid, DWORD mode)
{
  if(pUser == nullptr || pUser->getDressUp().getDressUpStatus() != 0)
    return;

  if(pUser->m_oBooth.hasOpen())
  {
    MsgManager::sendMsg(pUser->id, 25534);
    return;
  }

  MainPackage* pPackage = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pPackage == nullptr)
    return;

  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();
  SceneNpc* pNpc = pUser->getVisitNpcObj();
  if(pNpc == nullptr || getDistance(pUser->getPos(), pNpc->getPos()) > rCFG.m_dwMateRange)
  {
    MsgManager::sendMsg(pUser->id, 25531);
    return;
  }
  for(auto &it : rCFG.m_mapCost)
  {
    if (pPackage->checkItemCount(it.first, it.second) == false)
    {
      MsgManager::sendMsg(pUser->id, 25527);
      return;
    }
  }

  SceneUser* pMate = nullptr;
  if(mode != 0)
  {
    std::set<SceneUser*> teamSceneUsers = pUser->getTeamSceneUser();
    if(teamSceneUsers.size() != 2 || pUser->getTeamLeaderID() != pUser->id)
    {
      MsgManager::sendMsg(pUser->id, 25526);
      return;
    }
    for(auto &s : teamSceneUsers)
    {
      if(s == nullptr || s->id == pUser->id)
        continue;
      if(s->getDressUp().getDressUpStatus() != 0)
      {
        MsgManager::sendMsg(pUser->id, 25530);
        return;
      }
      if(s->m_oBooth.hasOpen())
      {
        MsgManager::sendMsg(pUser->id, 25534);
        return;
      }
      if(getDistance(pUser->getPos(), s->getPos()) > rCFG.m_dwMateRange)
      {
        MsgManager::sendMsg(pUser->id, 25529);
        return;
      }

      pMate = s;
      break;
    }
  }

  for(auto &it : rCFG.m_mapCost)
    pPackage->reduceItem(it.first, ESOURCE_DRESSUP_STAGE, it.second);

  addLineUp(pUser, pMate, stageid);
}

void DressUpStageMgr::addLineUp(SceneUser* pUser, SceneUser* pMate, DWORD stageid)
{
  if(pUser == nullptr)
    return;

  pUser->getDressUp().m_dwStageID = stageid;

  UserStageInfo uInfo;
  uInfo.setUsers.insert(pUser->id);
  if(pMate != nullptr)
  {
    pMate->getDressUp().m_dwStageID = stageid;
    uInfo.setUsers.insert(pMate->id);
  }

  auto itEquipType = MiscConfig::getMe().getDressStageCFG().m_mapDefaultEquip.find(stageid);
  if(itEquipType != MiscConfig::getMe().getDressStageCFG().m_mapDefaultEquip.end())
  {
    auto itEquip = itEquipType->second.begin();
    for(; itEquip != itEquipType->second.end(); ++itEquip)
      uInfo.mapAttr.emplace(static_cast<EUserDataType>(itEquip->first), itEquip->second);
  }

  auto it = m_mapDisplayStage.find(stageid);
  if(it == m_mapDisplayStage.end() && m_mapWaitUsers[stageid].empty())
  {
    showUsers(stageid, &uInfo);
  }
  else
  {
    addToWaitUser(stageid, &uInfo);
  }
}

bool DressUpStageMgr::showUsers(DWORD stageid, UserStageInfo* pInfo)
{
  bool bSuccess = false;
  QueryStageUserCmd cmd;
  cmd.set_stageid(stageid);
  StageInfo* pStageInfo = cmd.add_info();
  if(pStageInfo == nullptr)
    return false;
  pStageInfo->set_stageid(stageid);
  pStageInfo->set_waittime(MiscConfig::getMe().getDressStageCFG().m_dwShowTime);
  pStageInfo->set_status(EDRESSUP_SHOW);

  pInfo->dwTime = xTime::getCurSec() + MiscConfig::getMe().getDressStageCFG().m_dwShowTime;
  for(auto it = pInfo->setUsers.begin(); it != pInfo->setUsers.end(); ++it)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(*it);
    if(pUser != nullptr)
    {
      pUser->getDressUp().setDressUpStatus(EDRESSUP_SHOW, pInfo, cmd);
      bSuccess = true;
    }
  }

  if(bSuccess == true)
    m_mapDisplayStage[stageid] = *pInfo;

  return bSuccess;
}

bool DressUpStageMgr::addToWaitUser(DWORD stageid, UserStageInfo* pInfo)
{
  bool bSuccess = false;
  QueryStageUserCmd cmd;
  cmd.set_stageid(stageid);
  StageInfo* pStageInfo = cmd.add_info();
  if(pStageInfo == nullptr)
    return false;
  DWORD dwNum = m_mapWaitUsers[stageid].size();
  pStageInfo->set_stageid(stageid);
  pStageInfo->set_usernum(dwNum);
  DWORD dwTime = MiscConfig::getMe().getDressStageCFG().m_dwShowTime * dwNum;
  auto itShow = m_mapDisplayStage.find(stageid);
  if(itShow != m_mapDisplayStage.end() && itShow->second.dwTime > xTime::getCurSec())
    dwTime += itShow->second.dwTime - xTime::getCurSec();
  pStageInfo->set_waittime(dwTime);
  pStageInfo->set_status(EDRESSUP_WAIT);

  for(auto it = pInfo->setUsers.begin(); it != pInfo->setUsers.end(); ++it)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(*it);
    if(pUser != nullptr)
    {
      pUser->getDressUp().setDressUpStatus(EDRESSUP_WAIT, pInfo, cmd);
      bSuccess = true;
    }
  }

  if(bSuccess == true)
    m_mapWaitUsers[stageid].emplace_back(*pInfo);

  return bSuccess;
}

void DressUpStageMgr::checkLineUp(DWORD stageid)
{
  DWORD curTime = xTime::getCurSec();
  auto it = m_mapDisplayStage.find(stageid);
  if(it != m_mapDisplayStage.end() && it->second.dwTime <= curTime)
  {
    if(m_mapWaitUsers[stageid].empty() == true)
    {
      it->second.dwTime = curTime + MiscConfig::getMe().getDressStageCFG().m_dwShowTime;
      QueryStageUserCmd cmd;
      cmd.set_stageid(stageid);
      StageInfo* pInfo = cmd.add_info();
      if(pInfo != nullptr)
      {
        pInfo->set_stageid(stageid);
        pInfo->set_waittime(MiscConfig::getMe().getDressStageCFG().m_dwShowTime);
        pInfo->set_status(EDRESSUP_SHOW);
        PROTOBUF(cmd, send, len);
        for(auto &s : it->second.setUsers)
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(s);
          if(pUser != nullptr)
            pUser->sendCmdToMe(send, len);
        }
      }
    }
    else
    {
      UserStageInfo info = it->second;
      m_mapDisplayStage.erase(it);
      addToWaitUser(stageid, &info);
    }
    XLOG << "[换装舞台-展示] 结束" << "舞台 " << stageid << XEND;
  }

  if(m_mapDisplayStage.find(stageid) == m_mapDisplayStage.end() && m_mapWaitUsers[stageid].empty() == false)
  {
    while(true)
    {
      auto it = m_mapWaitUsers[stageid].begin();
      if(it == m_mapWaitUsers[stageid].end())
        break;

      UserStageInfo info = m_mapWaitUsers[stageid][0];
      bool bSuccess = showUsers(stageid, &info);
      m_mapWaitUsers[stageid].erase(it);
      if(bSuccess == true)
      {
        XLOG << "[换装舞台-展示] 开始" << "舞台 " << stageid << XEND;
        break;
      }
    }
  }
}

UserStageInfo* DressUpStageMgr::getUserStageInfo(SceneUser* pUser)
{
  if(pUser == nullptr || pUser->getDressUp().m_dwStageID == 0)
    return nullptr;

  auto it = m_mapWaitUsers.find(pUser->getDressUp().m_dwStageID);
  if(it != m_mapWaitUsers.end())
  {
    auto iter = it->second.begin();
    for(; iter != it->second.end(); ++iter)
    {
      auto itUser = iter->setUsers.find(pUser->id);
      if(itUser != iter->setUsers.end())
        return &(*iter);
    }
  }

  auto itShow = m_mapDisplayStage.find(pUser->getDressUp().m_dwStageID);
  if(itShow != m_mapDisplayStage.end())
  {
    auto iter = itShow->second.setUsers.find(pUser->id);
    if(iter != itShow->second.setUsers.end())
      return &(itShow->second);
  }

  return nullptr;
}

void DressUpStageMgr::leaveDressStage(SceneUser* pUser)
{
  if(pUser == nullptr)
    return;

  DWORD stageid = pUser->getDressUp().m_dwStageID;
  if(stageid == 0)
    return;

  DWORD status = pUser->getDressUp().getDressUpStatus();
  if(status == EDRESSUP_WAIT)
  {
    auto it = m_mapWaitUsers.find(pUser->getDressUp().m_dwStageID);
    if(it != m_mapWaitUsers.end())
    {
      auto iter = it->second.begin();
      for(; iter != it->second.end(); ++iter)
      {
        auto itUser = iter->setUsers.find(pUser->id);
        if(itUser != iter->setUsers.end())
        {
          iter->setUsers.erase(itUser);
          if(iter->setUsers.empty() == true)
          {
            it->second.erase(iter);
            refreshStageInfo(stageid);
          }
          else    //多人
          {
            auto s = iter->setUsers.begin();
            onMatesLeave(pUser, *s);
          }
          break;
        }
      }
    }
  }
  else if(status == EDRESSUP_SHOW)
  {
    auto it = m_mapDisplayStage.find(stageid);
    if(it != m_mapDisplayStage.end())
    {
      it->second.setUsers.erase(pUser->id);
      if(it->second.setUsers.empty() == true)
      {
        m_mapDisplayStage.erase(it);
        refreshStageInfo(stageid);
        SceneNpc* pNpc = getStageNpc(stageid);
        if(pNpc != nullptr)
          initAppearance(pNpc);
      }
    }
  }

  QueryStageUserCmd cmd;
  pUser->getDressUp().setDressUpStatus(EDRESSUP_MIN, nullptr, cmd);
  XLOG << "[换装舞台-退出]" << pUser->accid << pUser->id << pUser->name << stageid << "status " << status << XEND;
}

void DressUpStageMgr::onMatesLeave(SceneUser* pUser, QWORD userid)
{
  SceneUser* pMate = SceneUserManager::getMe().getUserByID(userid);
  if(pUser == nullptr || pMate == nullptr)
    return;

  DressUpStageUserCmd cmd;
  cmd.add_userid(0);
  PROTOBUF(cmd, send, len);
  pMate->sendCmdToMe(send, len);
  XLOG << "[换装舞台-队友] - 离开" << pUser->accid << pUser->id << pUser->name << XEND;
}

void DressUpStageMgr::changeMyStage(SceneUser* pUser, DWORD stageid, EUserDataType eType, DWORD value, SceneNpc* pNpc)
{
  if(pUser == nullptr || pNpc == nullptr)
    return;

  UserStageInfo* pInfo = getUserStageInfo(pUser);
  if(pInfo == nullptr)
    return;

  if(pInfo->setUsers.size() > 1)
  {
    if(pUser->getTeamLeaderID() != pUser->id)
      return;
  }

  pInfo->mapAttr[eType] = value;

  DressUpStageUserCmd cmd;
  cmd.set_stageid(pUser->getDressUp().m_dwStageID);
  StageUserDataType* pData = cmd.add_datas();
  if(pData != nullptr)
  {
    pData->set_type(eType);
    pData->set_value(value);
  }
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  for(auto &s : pInfo->setUsers)
  {
    if(s != pUser->id)
    {
      SceneUser* pMate = SceneUserManager::getMe().getUserByID(s);
      if(pMate != nullptr)
        pMate->sendCmdToMe(send, len);
    }
  }

#ifdef _DEBUG
  XLOG << "[换装舞台-舞台换装] - 自己" << pUser->accid << pUser->id << pUser->name << stageid << eType << value << XEND;
#endif
}

bool DressUpStageMgr::isInStageRange(xSceneEntryDynamic* pEntry, xPos pos, DWORD npcid, Scene* pScene) const
{
  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();
  if(pEntry != nullptr && (pEntry->getScene() == nullptr || pEntry->getScene()->getMapID() != rCFG.m_dwStaticMap))
    return false;

  if(pScene && pScene->getMapID() != rCFG.m_dwStaticMap)
    return false;

  if(npcid)
  {
    auto it = rCFG.m_setStageIDs.find(npcid);
    if(it != rCFG.m_setStageIDs.end())
      return false;
  }

  for(auto &s : rCFG.m_mapEnterPos)
  {
    if(getDistance(pos, s.second) <= rCFG.m_dwStageRange)
      return true;
  }

  return false;
}

bool DressUpStageMgr::checkStageDistance(const xPos& pos, DWORD distance, Scene* pScene)
{
  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();

  if(pScene && pScene->getMapID() != rCFG.m_dwStaticMap)
    return false;

  if(distance == 0)
    distance = rCFG.m_dwStageRange;

  for(auto &s : rCFG.m_mapEnterPos)
  {
    if(getDistance(pos, s.second) <= distance)
      return true;
  }

  return false;
}

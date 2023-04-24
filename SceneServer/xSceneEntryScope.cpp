
#include "SceneUser.h"

xScope::xScope(SceneUser *user) : m_pUser(user)
{
}

xScope::~xScope()
{
}

void xScope::addToWaitList(SceneUser *pUser)
{
  if (isInOK(pUser)) return;
  if (pUser == m_pUser) return;

  m_oWaitList.insert(pUser);
}

bool xScope::isInOK(SceneUser *pUser)
{
  if (!pUser) return false;

  return (m_oOKList.find(pUser) != m_oOKList.end());
}

bool xScope::addToOK(SceneUser *pUser, bool isFriend)
{
  if (!pUser) return false;
  if (has(pUser))
  {
    if (isFriend)
    {
      m_setFriendList.insert(pUser->id);
    }
    return false;
  }

  m_oWaitList.erase(pUser);
  m_oOKList.insert(pUser);

  if (isFriend)
  {
    m_setFriendList.insert(pUser->id);
  }
  else
  {
    if (getSizeExceptFriend() >= MAX_SCOPE_USER_NUM)
    {
      m_blFull = true;
    }
  }

  return true;
}

void xScope::addToOKList(SceneUser *pUser, bool isFriend)
{
  if (!addToOK(pUser, isFriend)) return;

  Cmd::AddMapUser cmd;
  pUser->fillMapUserData(cmd.add_users());
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void xScope::remove(SceneUser *pUser, bool from)
{
  if (!pUser) return;

  m_oWaitList.erase(pUser);
  {
    auto it = m_oOKList.find(pUser);
    if (it != m_oOKList.end())
    {
      m_oOKList.erase(pUser);
      m_setFriendList.erase(pUser->id);
      if (from)
      {
        m_setDeleteList.insert(pUser->id);
      }
    }
  }
}

void xScope::refresh(xSceneEntryScope *pScope)
{
  if (!pScope) return;

  QWORD qwGuildID = m_pUser->getGuild().id();
  QWORD qwTeamID = m_pUser->getTeamID();
  std::set<SceneUser *> setTeam;
  std::multimap<DWORD, SceneUser *> setGuild;
  if (!m_oWaitList.empty())
  {
    std::multimap<DWORD, SceneUser *> list;
    for (auto &it : m_oWaitList)
    {
      xScope *pUserScope = pScope->get(it->id);
      if (!pUserScope) continue;

      if (qwGuildID && it->getGuild().id() == qwGuildID)
      {
        setGuild.insert(std::make_pair(pUserScope->getFriendSize(), it));
        continue;
      }
      if (qwTeamID && it->getTeamID() == qwTeamID)
      {
        setTeam.insert(it);
        continue;
      }
      list.insert(std::make_pair(pUserScope->getSizeExceptFriend(), it));
    }
    m_oWaitList.clear();

    // 添加非友方
    for (auto &it : list)
    {
      if (m_oAddList.size() + this->getSizeExceptFriend() >= MAX_SCOPE_USER_NUM)
        break;

      m_oAddList.insert(it.second);
#ifdef _LX_DEBUG
  //    XLOG << "[视野-添加-敌方]" << m_pUser->id << it.second->id << XEND;
#endif
    }

    // 添加队友
    for (auto &it : setTeam)
    {
      m_oAddList.insert(it);
#ifdef _LX_DEBUG
   //   XLOG << "[视野-添加-组队]" << m_pUser->id << it->id << XEND;
#endif
    }

    // 添加公会
    DWORD dwAddGuildNum = 0;
    DWORD dwFriendSize = getFriendSize();
    for (auto &it : setGuild)
    {
      if (MAX_SCOPE_FRIEND_NUM)
      {
        if (dwFriendSize + dwAddGuildNum >= MAX_SCOPE_FRIEND_NUM)
        {
          break;
        }
        ++dwAddGuildNum;
      }
      m_oAddList.insert(it.second);
#ifdef _LX_DEBUG
    //  XLOG << "[视野-添加-公会]" << m_pUser->id << it.second->id << XEND;
#endif
    }
  }

  if (!m_oAddList.empty())
  {
    bool isHideScene = m_pUser->getScene() && m_pUser->getScene()->isHideUser();
    bool isHide = m_pUser->getAttr(EATTRTYPE_HIDE);

    Cmd::AddMapUser addMeCmd;
    m_pUser->fillMapUserData(addMeCmd.add_users());
    PROTOBUF(addMeCmd, addMeSend, addMeLen);

    SceneUserSet addMeUserSet;

    Cmd::AddMapUser cmd;
    for (auto it : m_oAddList)
    {
      if (!it) continue;
      xScope *pUserScope = pScope->get(it->id);
      if (!pUserScope) continue;

      bool isFriend = (setTeam.find(it) != setTeam.end()) || (qwGuildID && it->getGuild().id() == qwGuildID);

      pUserScope->addToOK(m_pUser, isFriend);
      addToOK(it, isFriend);

      // 隐身的地图
      if (isHideScene)
      {
        if (it->getAttr(EATTRTYPE_HIDE))
        {
          // 对方隐身 如果是队友 则同步给我 否则不同步
          if (it->canSeeHideBy(m_pUser))
          {
            it->fillMapUserData(cmd.add_users());
          }
        }
        else
        {
          it->fillMapUserData(cmd.add_users());
        }
        if (isHide)
        {
          // 我隐身 同步给我的队友
          if (m_pUser->canSeeHideBy(it))
          {
            addMeUserSet.insert(it);
          }
        }
        else
        {
          addMeUserSet.insert(it);
        }
      }
      else
      {
        it->fillMapUserData(cmd.add_users());

        addMeUserSet.insert(it);
      }

      if (cmd.users_size() >= MAX_SEND_USER_NUM)
      {
        PROTOBUF(cmd, send, len);
        m_pUser->sendCmdToMe(send, len);

        cmd.Clear();
      }
    }

    if (cmd.users_size())
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }

    if (!addMeUserSet.empty())
    {
      xSceneEntryScope::sendCmdToSceneUsers(addMeUserSet, addMeSend, addMeLen);
    }

    m_oAddList.clear();
  }

  if (!m_setDeleteList.empty())
  {
    Cmd::DeleteEntryUserCmd delMeCmd;
    delMeCmd.add_list(m_pUser->id);
    PROTOBUF(delMeCmd, delMeSend, delMeSize);

    Cmd::DeleteEntryUserCmd cmd;
    for (auto &it : m_setDeleteList)
    {
      cmd.add_list(it);
      thisServer->sendCmdToMe(it, delMeSend, delMeSize);
    }
    PROTOBUF(cmd, send, size);
    m_pUser->sendCmdToMe(send, size);
    m_setDeleteList.clear();

    if (m_blFull)
    {
      if (this->getSizeExceptFriend() < MAX_SCOPE_USER_NUM/2)
      {
        refreshNineUserToMe();
        m_blFull = false;
      }
    }
  }
}

void xScope::destroy(xSceneEntryScope *pScope)
{
  if (!pScope) return;
  if (m_oOKList.empty()) return;

  SceneUserSet tmplist(m_oOKList);
  for (auto &it : tmplist)
  {
    xScope *pToScope = pScope->get(it->id);
    if (pToScope)
    {
      pToScope->remove(m_pUser, false);
    }
    this->remove(it, true);
  }
  this->refresh(pScope);
}

void xScope::sendCmdToAll(const void* cmd, DWORD len)
{
  if (!cmd || !len) return;

  SceneUserSet set = m_oOKList;
  set.insert(m_pUser);
  xSceneEntryScope::sendCmdToSceneUsers(set, cmd, len);
}

void xScope::refreshNineUserToMe()
{
  if (!m_pUser || !m_pUser->getScene()) return;

  xSceneEntrySet set;
  m_pUser->getScene()->getEntryListInNine(SCENE_ENTRY_USER, m_pUser->getPos(), set);

  if (!set.empty())
  {
    for (auto &it : set)
    {
      SceneUser *user = (SceneUser *)(it);
      m_pUser->getScene()->addScope(m_pUser, user);
    }
    m_pUser->getScene()->refreshScope(m_pUser);
  }
}

void xScope::sendMeToScope()
{
  Cmd::AddMapUser addMeCmd;
  m_pUser->fillMapUserData(addMeCmd.add_users());
  PROTOBUF(addMeCmd, addMeSend, addMeLen);
  sendCmdToAll(addMeSend, addMeLen);
}

void xScope::hide()
{
  SceneUserSet set;

  Cmd::DeleteEntryUserCmd delMeCmd;
  delMeCmd.add_list(m_pUser->id);
  PROTOBUF(delMeCmd, delMeSend, delMeSize);
  for (auto &it : m_oOKList)
  {
    if (m_pUser->canSeeHideBy(it) == false)
    {
      set.insert(it);
    }
  }
  xSceneEntryScope::sendCmdToSceneUsers(set, delMeSend, delMeSize);
}

/************************************************/
/************************************************/
/************************************************/
/************************************************/

xSceneEntryScope::xSceneEntryScope()
{
}

xSceneEntryScope::~xSceneEntryScope()
{
}

bool xSceneEntryScope::add(SceneUser *fromUser, SceneUser *toUser)
{
  if (!fromUser || !toUser) return false;

  xScope *pFromScope = get(fromUser);
  if (pFromScope)
  {
    pFromScope->addToWaitList(toUser);
  }

#ifdef _LX_DEBUG
//  XLOG << "[视野-添加]" << fromUser->id << toUser->id << XEND;
#endif

  return true;
}

bool xSceneEntryScope::remove(SceneUser *fromUser, SceneUser *toUser)
{
  if (!fromUser || !toUser) return false;

  xScope *pFromScope = get(fromUser->id);
  if (pFromScope)
  {
    pFromScope->remove(toUser, true);
  }
  xScope *pToScope = get(toUser->id);
  if (pToScope)
  {
    pToScope->remove(fromUser, false);
  }

#ifdef _LX_DEBUG
 // XLOG << "[视野-移除]" << fromUser->id << toUser->id << XEND;
#endif

  return true;
}

xScope* xSceneEntryScope::get(QWORD id)
{
  if (!id) return nullptr;

  auto it = m_oScopeList.find(id);
  if (it != m_oScopeList.end())
  {
    return it->second;
  }
  return nullptr;
}

xScope* xSceneEntryScope::get(SceneUser *pUser)
{
  if (!pUser) return nullptr;

  auto it = m_oScopeList.find(pUser->id);
  if (it != m_oScopeList.end())
  {
    return it->second;
  }

  xScope *pScope = NEW xScope(pUser);

  if (pUser->getScene())
  {
    DWORD dwNum = MiscConfig::getMe().getSystemCFG().getMapScopeNumByMap(pUser->getScene()->getMapID());
    if (!dwNum)
    {
      dwNum = CommonConfig::m_dwMaxScopeNum;
    }
    DWORD dwFriendNum = MiscConfig::getMe().getSystemCFG().getMapScopeFriendNumByMap(pUser->getScene()->getMapID());
    if (dwFriendNum == (DWORD)-1)
    {
      dwFriendNum = CommonConfig::m_dwMaxFriendScopeNum;
    }
    pScope->setMaxScopeNum(dwNum, dwFriendNum);
  }
  m_oScopeList[pUser->id] = pScope;

  Cmd::AddMapUser cmd;
  pUser->fillMapUserData(cmd.add_users(), true);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  return pScope;
}

bool xSceneEntryScope::inScope(SceneUser *fromUser, SceneUser *toUser)
{
  if (!fromUser || !toUser) return false;
  if (fromUser == toUser) return true;

  xScope *pScope = get(fromUser->id);
  if (pScope)
  {
    return pScope->has(toUser);
  }

  return false;
}

void xSceneEntryScope::destroy(SceneUser *pUser)
{
  if (!pUser) return;

  auto it = m_oScopeList.find(pUser->id);
  if (it != m_oScopeList.end())
  {
    xScope *pScope = it->second;
    m_oScopeList.erase(it);
    if (pScope)
    {
      pScope->destroy(this);
      SAFE_DELETE(pScope);
    }
  }
}

bool xSceneEntryScope::getSceneUserSet(SceneUser *pUser, SceneUserSet& set)
{
  if (!pUser) return false;

  xScope *pScope = get(pUser->id);
  if (pScope)
  {
    set = pScope->getSceneUserSet();
    return true;
  }
  return false;
}

void xSceneEntryScope::sendCmdToScope(SceneUser *pUser, const void *cmd, DWORD len)
{
  if (!pUser) return;

  xScope *pScope = get(pUser->id);
  if (pScope)
  {
    pScope->sendCmdToAll(cmd, len);
  }
}

void xSceneEntryScope::sendCmdToSceneUsers(SceneUserSet &set, const void* cmd, unsigned short len)
{
  if (!cmd || !len) return;

  typedef std::map<ServerTask *, Cmd::ForwardToGateUserCmd> ForwardToGateUserMap;
  ForwardToGateUserMap list;

  auto add_user = [&](ForwardToGateUserMap &list, SceneUser *pUser)
  {
    if (!pUser || !pUser->getGateTask()) return;

    list[pUser->getGateTask()].add_accids(pUser->accid);
  };

  for (auto &it : set)
  {
    add_user(list, it);
  }

  for (auto &it : list)
  {
    if (nullptr == it.first) continue;
    if (!it.second.accids_size()) continue;

    it.second.set_data((const char *)cmd, len);

    PROTOBUF(it.second, send, dwSendLen);
    it.first->sendCmd(send, dwSendLen);
#ifdef _LX_DEBUG
  //  XLOG << "[视野-广播]" << it.first->getName() << it.second.accids_size() << XEND;
#endif
  }
}

void xSceneEntryScope::sendUserToScope(SceneUser *pUser)
{
  if (!pUser) return;
  xScope *pScope = get(pUser->id);
  if (pScope)
  {
    pScope->sendMeToScope();
  }
}

void xSceneEntryScope::sendUserToUser(SceneUser *pFromUser, SceneUser *pToUser)
{
  if (!pFromUser || !pToUser) return;

  xScope *pScope = get(pFromUser->id);
  if (pScope)
  {
    if (pScope->has(pToUser))
    {
      {
        Cmd::AddMapUser addFromCmd;
        pFromUser->fillMapUserData(addFromCmd.add_users());
        PROTOBUF(addFromCmd, addFromSend, addFromLen);
        pToUser->sendCmdToMe(addFromSend, addFromLen);
      }

      {
        Cmd::AddMapUser addToCmd;
        pToUser->fillMapUserData(addToCmd.add_users());
        PROTOBUF(addToCmd, addToSend, addToLen);
        pFromUser->sendCmdToMe(addToSend, addToLen);
      }
    }
    else
    {
      if (pFromUser->check2PosInNine(pToUser))
      {
        add(pFromUser, pToUser);
        pScope->refresh(this);
      }
    }
  }
}

void xSceneEntryScope::hideMeToScope(SceneUser *pUser)
{
  if (!pUser) return;

  xScope *pScope = get(pUser->id);
  if (pScope)
  {
    pScope->hide();
  }
}

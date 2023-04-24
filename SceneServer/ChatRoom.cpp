#include "ChatRoom.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "xDefine.h"
#include "xTime.h"

RoomMember::RoomMember(QWORD userid, EChatRoomJob eRoomJob)
{
  //m_qwUserID = userid;
  //m_eRoomJob = eRoomJob;
  //m_eRoleJob = EPROFESSION_MIN;
  m_oData.set_id(userid);
  m_oData.set_job(eRoomJob);
}

SceneUser* RoomMember::getUser()
{
  return SceneUserManager::getMe().getUserByID(m_oData.id());
}

bool RoomMember::init(SceneUser *pUser)
{
  if (nullptr == pUser)
    return false;

  m_oData.set_id(pUser->id);
  m_oData.set_level(pUser->getUserSceneData().getRolelv());
  m_oData.set_body(pUser->getUserSceneData().getBody());
  m_oData.set_hair(pUser->getHairInfo().getCurHair());
  m_oData.set_haircolor(pUser->getHairInfo().getCurHairColor());
  m_oData.set_portrait(pUser->getPortrait().getCurPortrait());
  m_oData.set_eye(pUser->getEye().getCurID());
  m_oData.set_gender(pUser->getUserSceneData().getGender());
  m_oData.set_rolejob(pUser->getProfession());
  m_oData.set_blink(pUser->getUserSceneData().getBlink());
  m_oData.set_name(pUser->name);
  m_oData.set_guildname(pUser->getGuild().name());

  return true;
}

bool RoomMember::toData(Cmd::ChatRoomMember& rMember)
{
  rMember.CopyFrom(m_oData);
  /*rMember.set_id(m_qwUserID);
  rMember.set_job(m_eRoomJob);
  rMember.set_name(m_sName);
  rMember.set_frame(m_dwFrameID);
  rMember.set_portrait(m_dwPortaritID);
  rMember.set_rolejob(m_eRoleJob);*/

  return true;
}

ChatRoom::ChatRoom(DWORD roomid, QWORD ownerid, const string &name,const string &pswd, DWORD maxnum)
{
  m_stBaseData.dwRoomID = roomid;
  m_stBaseData.qwOwnerID = ownerid;
  m_stBaseData.sName = name;
  m_stBaseData.sPswd = pswd;
  if (m_stBaseData.sPswd.length() < 1)
    m_stBaseData.eRoomType = ECHATROOMTYPE_PUBLIC;
  else
    m_stBaseData.eRoomType = ECHATROOMTYPE_PRIVATE;
  m_stBaseData.dwMaxNum = maxnum;
  m_bNineSync = false;
}

void ChatRoom::timer(DWORD curSec)
{
  updateMember(curSec);
  updateKick(curSec);
  updateNineSync(curSec);
}

bool ChatRoom::updateMember(DWORD curTimeSec)
{
  if (m_vecUpdate.size() == 0)
    return true;
  RoomMemberUpdate cmd;
  for (auto v = m_vecUpdate.begin(); v != m_vecUpdate.end(); ++v)
  {
    RoomMember* pMember = getMember(*v);
    if (nullptr == pMember)
    {
      cmd.add_deletes(*v);
    }
    else
    {
       Cmd::ChatRoomMember* pUpdateMember = cmd.add_updates();
       if (nullptr == pUpdateMember)
         continue;
       pMember->toData(*pUpdateMember);
    }
  }

  if (cmd.updates_size() > 0 || cmd.deletes_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    sendCmdToMember(send, len);
  }
  m_vecUpdate.clear();
  return true;
}

void ChatRoom::updateKick(DWORD curTimeSec)
{
  const SChatRoomMiscCFG& rCFG = MiscConfig::getMe().getChatRoomCFG();
  for (auto v = m_mapKick.begin(); v != m_mapKick.end();)
  {
    if ((curTimeSec - v->second) >= rCFG.dwEnterProtectTime)
      v = m_mapKick.erase(v);
    else
      ++v;
  }
}

void ChatRoom::setNineSync(bool bSync)
{
  m_bNineSync = bSync;
}

void ChatRoom::updateNineSync(DWORD curTimeSec)
{
  if (!m_bNineSync)
    return;

  Cmd::ChatRoomDataSync cmd;
  cmd.set_esync(ECHATROOMSYNC_UPDATE);
  Cmd::ChatRoomSummary *pSummary = cmd.mutable_data();
  if (nullptr == pSummary)
    return;

  toSummary(*pSummary);
  PROTOBUF(cmd, send, len);

  RoomMember *pOwner = getOwner();
  if (nullptr == pOwner)
  {
    XERR << "Room has no owner, roomId=" << m_stBaseData.dwRoomID << XEND;
    return;
  }
  SceneUser *pUser = pOwner->getUser();
  if (nullptr != pUser)
  {
    pUser->sendCmdToNine(send, len, GateIndexFilter(pUser->id));
    pUser->sendCmdToMe(send, len);
  }

  //sendCmdToMember(send, len);
  m_bNineSync = false;
}

bool ChatRoom::toSummary(Cmd::ChatRoomSummary &rSummary)
{
  rSummary.set_name(m_stBaseData.sName);
  rSummary.set_roomid(m_stBaseData.dwRoomID);
  rSummary.set_ownerid(m_stBaseData.qwOwnerID);
  rSummary.set_roomtype(m_stBaseData.eRoomType);
  rSummary.set_maxnum(m_stBaseData.dwMaxNum);
  rSummary.set_curnum(m_lsMember.size());
  rSummary.set_pswd(m_stBaseData.sPswd);
  return true;
}

bool ChatRoom::toClient(Cmd::ChatRoomData &rRoomData)
{
  rRoomData.set_name(m_stBaseData.sName);
  rRoomData.set_roomid(m_stBaseData.dwRoomID);
  rRoomData.set_maxnum(m_stBaseData.dwMaxNum);
  rRoomData.set_ownerid(m_stBaseData.qwOwnerID);
  rRoomData.set_roomtype(m_stBaseData.eRoomType);
  rRoomData.set_pswd(m_stBaseData.sPswd);
  for (auto v = m_lsMember.begin(); v != m_lsMember.end(); ++v)
  {
    ChatRoomMember *pMember = rRoomData.add_members();
    if (nullptr == pMember)
      continue;
    (*v)->toData(*pMember);
  }

  return true;
}

bool ChatRoom::addMember(RoomMember* memberPtr, bool bUpdate)
{
  if (nullptr == memberPtr)
    return false;

  if (isMember(memberPtr->getUserID()))
    return false;

  if (m_stBaseData.dwMaxNum <= m_lsMember.size())
    return false;

  m_lsMember.push_back(memberPtr);
  if (bUpdate)
    m_vecUpdate.push_back(memberPtr->getUserID());
  return true;
}

bool ChatRoom::removeMember(QWORD userid)
{
  auto it = find_if(m_lsMember.begin(), m_lsMember.end(), [userid](RoomMember* ptr){ return ptr->getUserID()==userid;} );
  if (it != m_lsMember.end())
  {
#ifdef _TYH_DEBUG
    XLOG << "[聊天室]removeMember, userid =" << (*it)->getUserID() << XEND;
#endif
    SAFE_DELETE(*it);
    m_lsMember.erase(it);
    m_vecUpdate.push_back(userid);
    //if (0 == m_lsMember.size())
    //  m_stBaseData.qwOwnerID = 0;

  }
  return true;
}

bool ChatRoom::setOwner(QWORD ownerid)
{
  auto it = find_if(m_lsMember.begin(), m_lsMember.end(), [ownerid](RoomMember* ptr){ return ptr->getUserID()==ownerid;} );
  if (it == m_lsMember.end())
    return false;

  RoomMember *pMember = getOwner();
  if (nullptr != pMember)
  {
    pMember->setJob(ECHATROOM_MEMBER);
    m_vecUpdate.push_back(pMember->getUserID());
  }

  (*it)->setJob(ECHATROOM_OWNER);
  RoomMember *pOwner = (*it);

  m_lsMember.erase(it);
  m_lsMember.push_front(pOwner);

  m_stBaseData.qwOwnerID = ownerid;
  m_vecUpdate.push_back(ownerid);

  return true;
}

RoomMember* ChatRoom::getOwner()
{
  for (auto m = m_lsMember.begin(); m != m_lsMember.end(); ++m)
  {
    if (ECHATROOM_OWNER == (*m)->getJob())
      return *m;
  }
  return nullptr;
}

RoomMember* ChatRoom::getFirstMember()
{
  if (m_lsMember.size() == 0)
    return nullptr;
  RoomMember* pMember  = m_lsMember.front();
  return pMember;
}

bool ChatRoom::isInKick(QWORD userid)
{
  auto it = m_mapKick.find(userid);
  if (it != m_mapKick.end())
    return true;

  return false;
}

RoomMember* ChatRoom::getMember(QWORD userid)
{
  auto it = find_if(m_lsMember.begin(), m_lsMember.end(), [userid](RoomMember* ptr)
  {
    if (ptr == nullptr || ptr->getUser() == nullptr)
      return false;
    return ptr->getUserID()==userid;
  });
  if (it == m_lsMember.end())
    return nullptr;
  return *it;
}

bool ChatRoom::isMember(QWORD userid)
{
  auto it = find_if(m_lsMember.begin(), m_lsMember.end(), [userid](RoomMember* ptr)
  {
    if (ptr == nullptr || ptr->getUser() == nullptr)
      return false;
    return ptr->getUserID()==userid;
  });
  if (it == m_lsMember.end())
    return false;

  return true;
}

bool ChatRoom::sendCmdToMember(const void* cmd, DWORD len)
{
  if (nullptr == cmd || len < 1)
    return false;

  for (auto v = m_lsMember.begin(); v != m_lsMember.end(); ++v)
  {
    SceneUser *pUser = (*v)->getUser();
    if (nullptr != pUser)
      pUser->sendCmdToMe(cmd, len);
  }
#ifdef _TYH_DEBUG
  XLOG << "[聊天室消息],sendCmdToMember, member num=" << m_lsMember.size() << XEND;
#endif
  return true;
}

bool ChatRoom::addKickMember(QWORD userid)
{
  QWORD curUSec = xTime::getCurUSec();
  DWORD curSec = curUSec / ONE_MILLION;
  if (!isInKick(userid))
    m_mapKick.insert(make_pair(userid, curSec));

  return true;
}

bool ChatRoom::init(DWORD roomid, QWORD ownerid,const string &name, const string &pswd, DWORD maxnum)
{
  m_stBaseData.dwRoomID = roomid;
  m_stBaseData.qwOwnerID = ownerid;
  m_stBaseData.sName = name;
  m_stBaseData.sPswd = pswd;
  if (m_stBaseData.sPswd.length() < 1)
    m_stBaseData.eRoomType = ECHATROOMTYPE_PUBLIC;
  else
    m_stBaseData.eRoomType = ECHATROOMTYPE_PRIVATE;
  m_stBaseData.dwMaxNum = maxnum;
  m_bNineSync = false;
  return true;
}

#include "ChatRoomManager.h"
#include "SceneUser.h"
#include "xTime.h"
#include "xDefine.h"
#include "SceneUserManager.h"
#include "GuidManager.h"

ChatRoomManager::ChatRoomManager()
{

}

ChatRoomManager::~ChatRoomManager()
{

}

bool ChatRoomManager::addRoom(ChatRoom *p)
{
  if (!p) return false;

  DWORD roomid = p->getRoomID();
  if (!roomid) return false;

  if (m_mapRoom.find(roomid) != m_mapRoom.end()) return false;

  m_mapRoom[roomid] = p;

  return true;
}

bool ChatRoomManager::delRoom(ChatRoom *p)
{
  if (!p) return false;

  DWORD roomid = p->getRoomID();
  if (!roomid) return false;

  m_mapRoom.erase(roomid);
  SAFE_DELETE(p);

  return true;
}

void ChatRoomManager::timer(DWORD curSec)
{
  FUN_TIMECHECK_30();
  checkValidRoom();
  for (auto r = m_mapRoom.begin(); r != m_mapRoom.end(); ++r)
    r->second->timer(curSec);
}

void ChatRoomManager::checkValidRoom()
{
  for (auto r = m_mapRoom.begin(); r != m_mapRoom.end();)
  {
    if (0 == r->second->curMemberNum())
    {
      XLOG << "[聊天室],删除聊天室," << r->second->getRoomID() << XEND;
      //release unique id
      //s_pTemRoomIDManager->putUniqueID(r->second->getRoomID());

      SAFE_DELETE(r->second);
      r = m_mapRoom.erase(r);
    }
    else
      ++r;
  }
}

ChatRoom* ChatRoomManager::createRoom(SceneUser *pUser, const string &name, const string & pswd, DWORD maxnum)
{
  if (nullptr == pUser)
    return nullptr;
  if (pUser->hasChatRoom())
    return nullptr;

  //if (pUser->m_oHands.has() == true && pUser->m_oHands.isMaster() == false)
    //return nullptr;

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  ESysMsgID eMsg = rCFG.checkNameValid(name, ENAMETYPE_ROOM);
  if (eMsg != ESYSTEMMSG_ID_MIN)
  {
    XERR << "[聊天室-创建]" << pUser->accid << pUser->id << pUser->name << "名字:" << name << "不合法" << XEND;
    return nullptr;
  }

  DWORD dwGuid = GuidManager::getMe().getNextChatRoomID();

  ChatRoom* pRoom = NEW ChatRoom(dwGuid, pUser->id, name, pswd, maxnum);
  if (nullptr == pRoom)
    return nullptr;

  if (!addRoom(pRoom))
  {
    SAFE_DELETE(pRoom);
    return nullptr;
  }

  RoomMember* pMember = NEW RoomMember(pUser->id, ECHATROOM_OWNER);
  if (nullptr == pMember)
    return pRoom;

  pMember->init(pUser);

  if (!pRoom->addMember(pMember, false))
  {
    SAFE_DELETE(pMember);
    return pRoom;
  }
  pUser->setChatRoomID(dwGuid);

  XLOG << "[聊天室-创建]" << pUser->accid << pUser->id << pUser->name << "创建成功:" << pRoom->getRoomName() << "(" << pRoom->getRoomID() << ")" << XEND;

  Cmd::EnterChatRoom cmd;
  Cmd::ChatRoomData* pRoomData = cmd.mutable_data();
  if (nullptr == pRoomData)
    return pRoom;

  pRoom->toClient(*pRoomData);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  //nine sync
  pRoom->setNineSync(true);
  return pRoom;
}

/*bool ChatRoomManager::removeRoom(DWORD roomid)
{
  auto it = m_mapRoom.find(roomid);
  if (it == m_mapRoom.end())
    return false;

  m_mapRoom.erase(it);
  return true;
}*/

bool ChatRoomManager::joinRoom(SceneUser *pUser, DWORD roomid, string pswd)
{
  if (nullptr == pUser)
    return false;

  ChatRoom* pRoom = getRoom(roomid);
  if (nullptr == pRoom)
    return false;
  if (pRoom->isMember(pUser->id))
    return false;

  if (pRoom->isInKick(pUser->id))
    return false;

  //if (pUser->m_oHands.has() == true)
    //return false;

  if (ECHATROOMTYPE_PRIVATE == pRoom->getRoomType() && pRoom->getPswd() != pswd )
    return false;

  RoomMember *pMember = NEW RoomMember(pUser->id, ECHATROOM_MEMBER);
  if (nullptr == pMember)
    return false;
  pMember->init(pUser);

  if (!pRoom->addMember(pMember))
  {
    SAFE_DELETE(pMember);
    return false;
  }
  pUser->setChatRoomID(pRoom->getRoomID());
  Cmd::EnterChatRoom cmd;
  Cmd::ChatRoomData* pRoomData = cmd.mutable_data();
  if (nullptr == pRoomData)
    return false;

  pRoom->toClient(*pRoomData);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  //nine sync
  pRoom->setNineSync(true);
  XLOG << "[聊天室]" << pUser->accid << pUser->id << pUser->name << "加入聊天室" << pRoom->getRoomID() << pRoom->getRoomName() << XEND;
  ChatRoomTip cmdTip;
  cmdTip.set_tip(ECHATROOMTIP_JOIN);
  cmdTip.set_userid(pUser->id);
  cmdTip.set_name(pUser->name);
  PROTOBUF(cmdTip, send2, len2);
  pRoom->sendCmdToMember(send2, len2);

  return true;
}

ChatRoom* ChatRoomManager::getRoom(DWORD roomid)
{
  auto it = m_mapRoom.find(roomid);
  if (it == m_mapRoom.end())
    return nullptr;

  return it->second;
}

bool ChatRoomManager::broadcastRoomCmd(DWORD roomid, const void *cmd, DWORD len)
{
  if (nullptr == cmd || len < 1)
    return false;
  ChatRoom *pRoom = getRoom(roomid);
  if (nullptr == pRoom)
    return false;
  pRoom->sendCmdToMember(cmd, len);
  return true;
}

bool ChatRoomManager::exitRoom(SceneUser *pUser, DWORD roomid)
{
  if (nullptr == pUser)
    return false;

  ChatRoom *pRoom = getRoom(roomid);
  if (nullptr == pRoom)
    return false;
  RoomMember *pMember = pRoom->getMember(pUser->id);

  if (nullptr == pMember)
    return false;

  XLOG << "[聊天室-退出]" << pUser->accid << pUser->id << pUser->name << "RoomID:" << roomid << XEND;


  if(ECHATROOM_OWNER != pMember->getJob())
  {
    pRoom->removeMember(pUser->id);
  }
  else
  {
    pRoom->removeMember(pUser->id);

    ChatRoomDataSync cmd;
    cmd.set_esync(ECHATROOMSYNC_REMOVE);
    ChatRoomSummary *pSummmary = cmd.mutable_data();
    pRoom->toSummary(*pSummmary);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToNine(send, len);

    RoomMember *pOwner = pRoom->getFirstMember();
    if (nullptr != pOwner)
    {
      pRoom->setOwner(pOwner->getUserID());
      // tip client

      ChatRoomTip cmdTip;
      cmdTip.set_tip(ECHATROOMTIP_OWNERCHANGE);
      cmdTip.set_userid(pOwner->getUserID());
      SceneUser* pUserOwner = pOwner->getUser();
      if (nullptr != pUserOwner)
      {
        cmdTip.set_name(pUserOwner->name);
        PROTOBUF(cmdTip, send2, len2);
        pRoom->sendCmdToMember(send2, len2);
      }
    }

  }
  pUser->setChatRoomID(0);

  //nitify client
  ExitChatRoom exitcmd;
  exitcmd.set_roomid(pRoom->getRoomID());
  exitcmd.set_userid(pUser->id);
  PROTOBUF(exitcmd, send1, len1);
  pUser->sendCmdToMe(send1, len1);
  //nine sync
  //pRoom->setNineSync(true);

  if (pRoom->curMemberNum() == 0)
  {
     ChatRoomDataSync cmd;
     cmd.set_esync(ECHATROOMSYNC_REMOVE);
     ChatRoomSummary *pSummmary = cmd.mutable_data();
     pRoom->toSummary(*pSummmary);
     PROTOBUF(cmd, send, len);
     pUser->sendCmdToNine(send, len);
  }
  else
  {
    pRoom->setNineSync(true);
    ChatRoomTip cmdTip;
    cmdTip.set_tip(ECHATROOMTIP_EXIT);
    cmdTip.set_userid(pUser->id);
    cmdTip.set_name(pUser->name);
    PROTOBUF(cmdTip, send2, len2);
    pRoom->sendCmdToMember(send2, len2);
  }

  return true;
}

bool ChatRoomManager::kickMember(SceneUser* pUser, DWORD roomid, QWORD memberid)
{
  ChatRoom *pRoom = getRoom(roomid);
  if (nullptr == pRoom)
    return false;

  RoomMember *pMember = pRoom->getMember(pUser->id);
  if (nullptr == pMember)
    return false;
  if (ECHATROOM_OWNER != pMember->getJob())
    return false;

  if (pRoom->removeMember(memberid))
  {
    pRoom->addKickMember(memberid);
    SceneUser *pMemberUser = SceneUserManager::getMe().getUserByID(memberid);
    if (pMemberUser)
    {
      pMemberUser->setChatRoomID(0);
      //send cmd to client
      Cmd::KickChatMember cmd;
      cmd.set_roomid(pRoom->getRoomID());
      cmd.set_memberid(memberid);
      PROTOBUF(cmd, send, len);

      pMemberUser->sendCmdToMe(send, len);
    }

    ChatRoomTip cmdTip;
    cmdTip.set_tip(ECHATROOMTIP_KICK);
    cmdTip.set_userid(memberid);
    if (pMemberUser)
    {
      cmdTip.set_name(pMemberUser->name);
    }
    PROTOBUF(cmdTip, send2, len2);
    pRoom->sendCmdToMember(send2, len2);
  }

  //nine sync
  pRoom->setNineSync(true);
  return true;
}

bool ChatRoomManager::changeOwner(SceneUser* pUser, QWORD ownerid)
{
  if (nullptr == pUser || nullptr == pUser->getChatRoom())
    return false;
  ChatRoom *pUserRoom = pUser->getChatRoom();
  if (nullptr == pUserRoom)
    return false;

  if (nullptr == getRoom(pUserRoom->getRoomID()))
    return false;

  RoomMember *pUserMember = pUserRoom->getMember(pUser->id);

  if (nullptr == pUserMember)
    return false;
  if (ECHATROOM_OWNER != pUserMember->getJob())
    return false;

  //pUserMember->setJob(ECHATROOM_MEMBER);
  pUserRoom->setOwner(ownerid);

  //nine sync
  Cmd::ChatRoomDataSync cmdremove;
  cmdremove.set_esync(ECHATROOMSYNC_REMOVE);
  ChatRoomSummary *pSummmary = cmdremove.mutable_data();
  pUserRoom->toSummary(*pSummmary);
  pSummmary->set_ownerid(pUser->id);
  PROTOBUF(cmdremove, send, len);
  pUser->sendCmdToNine(send, len);

  pUserRoom->setNineSync(true);

  RoomMember *pOwner = pUserRoom->getOwner();
  if (nullptr != pOwner)
  {
    SceneUser* pOwnerUser = pOwner->getUser();
    if (nullptr == pOwnerUser)
      return false;
    ChatRoomTip cmdTip;
    cmdTip.set_tip(ECHATROOMTIP_OWNERCHANGE);
    cmdTip.set_userid(pOwnerUser->id);
    cmdTip.set_name(pOwnerUser->name);
    PROTOBUF(cmdTip, send2, len2);
    pUserRoom->sendCmdToMember(send2, len2);
  }
  return true;
}

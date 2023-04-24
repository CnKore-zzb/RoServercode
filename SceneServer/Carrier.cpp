#include "Carrier.h"
#include "CarrierCmd.pb.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneMap.pb.h"
#include "MiscConfig.h"
#include "MsgManager.h"
#include "SceneServer.h"
#include "GMCommandRuler.h"
#include "TableStruct.h"
#include "TableManager.h"
#include "ChatRoomManager.h"
#include "StatisticsDefine.h"
#include "DScene.h"
#include "SceneWeddingMgr.h"

Carrier::Carrier(SceneUser *user):m_pUser(user)
{
}

Carrier::~Carrier()
{
}

bool Carrier::create(DWORD carrierID, DWORD line, DWORD dwNeedAct, DWORD invite)
{
  if (!carrierID) return false;
  if (!canJoin(m_pUser)) return false;
  if (has()) return false;

  const BusBase *base = TableManager::getMe().getBusCFG(carrierID);
  if (base == nullptr)
    return false;

  if (base->getType() == 1) // 单人载具
    invite = 0;

  // bool iswedding = base->isWedding();// 婚礼双方可乘坐
  //if (iswedding && SceneWeddingMgr::getMe().isCurWeddingUser(m_pUser->id) == false)
  //  return false;

  // 上载具前下坐骑
  m_pUser->getPackage().equip(EEQUIPOPER_OFFPOS, EEQUIPPOS_MOUNT, "");
  if (!invite)
    m_pUser->m_oHands.breakup();

  m_oData.reset();
  setCarrierID(carrierID);
  m_oData.m_qwMasterID = m_pUser->id;
  m_oData.m_dwLine = line;
  m_oData.m_dwNeedAnimation = dwNeedAct;
  // 保留玩家的开始位置 中途离开时使用
  m_oData.m_oStartPos = m_pUser->getPos();

  m_dwIndex = 1;
  m_dwProgress = 0;

  sendMeToNine();

  // 等待列表
  Cmd::CarrierWaitListUserCmd waitMessage;
  waitMessage.set_masterid(m_oData.m_qwMasterID);

  std::stringstream stream;
  stream.str("");

  if (invite)
  {
    // 邀请组队玩家
    const TMapGTeamMember& mapMember = m_pUser->getTeam().getTeamMemberList();
    for (auto &m : mapMember)
    {
      if (m.second.catid() != 0)
        continue;
      SceneUser* pTarget = SceneUserManager::getMe().getUserByID(m.second.charid());
      if (pTarget != nullptr &&
          (pTarget->getSocial().checkRelation(m_pUser->id, ESOCIALRELATION_BLACK) == true || pTarget->getSocial().checkRelation(m_pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true))
      {
        XERR << "[玩家-玩家2消息]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
          << "邀请" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "失败,黑名单" << XEND;
        continue;
      }

      if (base->isWedding())
      {
        if (m_pUser->id != m.second.charid() && SceneWeddingMgr::getMe().isCurWeddingUser(m.second.charid()) == false && m_pUser->getUserWedding().checkMarryRelation(m.second.charid()) == false)
          continue;
      }
      m_oInvites.insert(m.second.charid());

      // 改成可以邀请不同地图的玩家
      Cmd::CarrierMember *member = waitMessage.add_members();
      member->set_id(m.second.charid());
      member->set_name(m.second.name());
      stream << m.second.name() << ",";
    }
  }
  PROTOBUF(waitMessage, waitSend, waitLen);

  // 邀请玩家，等待回复
  Cmd::InviteCarrierUserCmd message;
  message.set_masterid(m_oData.m_qwMasterID);
  message.set_mastername(m_pUser->name);
  message.set_carrierid(m_oData.m_dwCarrierID);
  PROTOBUF(message, send, len);

  for (auto it : m_oInvites)
  {
    if (m_oData.m_dwNeedAnimation && (it!=m_oData.m_qwMasterID))
    {
      // 发送邀请给玩家场景，等待回复
      thisServer->forwardCmdToSceneUser(it, send, len);
    }
    // 发送等待列表
    thisServer->sendCmdToMe(it, waitSend, waitLen);
  }
  XLOG << "[载具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "创建载具:" << m_oData.m_dwCarrierID << "乘坐人:" << stream.str() << XEND;
  return true;
}

bool Carrier::checkMasterCanBeJoin(QWORD charid)
{
  // 被加入的不是主驾驶
  if (!m_oData.m_dwCarrierID || !isMaster(m_pUser->id)
      || charid==m_pUser->id)// || m_oInvites.find(charid)==m_oInvites.end())
  {
    MsgManager::sendMsg(charid, 403);
    return false;
  }
  // 已经出发 不能假如
  if (m_dwProgress)
  {
    MsgManager::sendMsg(charid, 404);
    return false;
  }
  DWORD max = MiscConfig::getMe().getTeamCFG().dwMaxMember;
  if (m_members.size()+1 >= max)
  {
    MsgManager::sendMsg(charid, 404);
    return false;
  }

  const BusBase *base = TableManager::getMe().getBusCFG(m_oData.m_dwCarrierID);
  if (base == nullptr)
    return false;
  //if (base->isWedding() && SceneWeddingMgr::getMe().isCurWeddingUser(charid) == false)
  //  return false;

  return true;
}

// 主驾驶 拉玩家进载具
bool Carrier::join(QWORD charid)
{
  if (!checkMasterCanBeJoin(charid))
    return false;

  Cmd::CatchUserJoinCarrierUserCmd message;
  message.set_masterid(m_pUser->id);
  message.set_charid(charid);
  message.set_mapid(m_pUser->getScene()->id);
  PROTOBUF(message, send, len);

  thisServer->forwardCmdToSceneUser(charid, send, len);

  return true;
}

void Carrier::onUserEnter()
{
  if (m_qwJoinMasterID)
  {
    SceneUser *master = SceneUserManager::getMe().getUserByID(m_qwJoinMasterID);
    if (master)
    {
      master->m_oCarrier.join(m_pUser);
    }
    m_qwJoinMasterID = 0;
  }
}

bool Carrier::checkUserCanJoinMe(SceneUser *user)
{
  if (!user) return false;
  if (!user->getScene() || !user->isAlive())
  {
    MsgManager::sendMsg(user->id, 404);
    return false;
  }
  // 已经有载具了
  if (user->m_oCarrier.m_oData.m_dwCarrierID)
  {
    MsgManager::sendMsg(user->id, 404);
    return false;
  }
  // 不在同一个地图
  if (user->getScene() != m_pUser->getScene())
  {
    MsgManager::sendMsg(user->id, 402);
    return false;
  }
  return true;
}

bool Carrier::join(SceneUser *user, bool notify)
{
  if (!user) return false;

  if (!checkMasterCanBeJoin(user->id)) return false;
  if (!checkUserCanJoinMe(user)) return false;

  user->getPackage().equip(EEQUIPOPER_OFFPOS, EEQUIPPOS_MOUNT, "");

  user->m_oCarrier.m_oData.reset();
  user->m_oCarrier.setCarrierID(m_oData.m_dwCarrierID);
  user->m_oCarrier.m_oData = m_oData;
  user->m_oCarrier.m_dwProgress = 0;

  DWORD max = MiscConfig::getMe().getTeamCFG().dwMaxMember;
  for (DWORD i=2; i<=max; ++i)
  {
    if (m_members.find(i)==m_members.end())
    {
      m_members[i] = user;
      user->m_oCarrier.m_dwIndex = i;
      break;
    }
  }

  user->setScenePos(m_pUser->getPos());
  if (notify)
    sendMeToNine();

  if (user->getChatRoom() != nullptr)
    ChatRoomManager::getMe().exitRoom(user, user->getChatRoom()->getRoomID());
  if(user->m_oBooth.hasOpen())
    user->m_oBooth.onCarrier();

  XLOG << "[载具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "加入载具:" << m_oData.m_dwCarrierID << XEND;
  return true;
}

void Carrier::offline()
{
  if (m_qwJoinMasterID) return;
  if (!m_oData.m_dwCarrierID)   // 还没有登上载具
  {
    if (!m_oBeInvites.empty())
    {
      // 通知所有邀请的人拒绝
      for (auto it : m_oBeInvites)
      {
        Cmd::JoinCarrierUserCmd message;
        message.set_masterid(it);
        message.set_mastername(m_pUser->name);
        message.set_agree(false);
        PROTOBUF(message, send, len);
        m_pUser->doUserCmd((const Cmd::UserCmd *)send, len);
      }
      m_oBeInvites.clear();
    }
    return;
  }
}

void Carrier::leave()
{
  if (!m_oData.m_dwCarrierID)
  {
    offline();
    return;
  }

  Cmd::LeaveCarrierUserCmd message;
  message.set_masterid(m_oData.m_qwMasterID);

  if (isMaster(m_pUser->id))  // 驾驶者
  {
    if (m_dwProgress)   // 如果出发了  转移驾驶者
    {
      SceneUser *pMaster = NULL;
      for (auto it : m_members)
      {
        if (!pMaster)
        {
          pMaster = it.second;
          pMaster->m_oCarrier.clear();
          pMaster->m_oCarrier.create(m_oData.m_dwCarrierID, m_oData.m_dwLine);
          pMaster->m_oCarrier.m_oInvites = m_oInvites;
          pMaster->m_oCarrier.m_oInvites.erase(m_pUser->id);
          message.set_newmasterid(it.second->id);
        }
        else
        {
          it.second->m_oCarrier.clear();
          pMaster->m_oCarrier.join(it.second, false);
        }
      }
      m_members.clear();
      m_oInvites.clear();
    }
    else    // 没出发全部离开
    {
      SysMsg cmd;
      cmd.set_id(403);
      cmd.set_type(EMESSAGETYPE_FRAME);
      for (auto it : m_oInvites)
      {
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToMe(it, send, len);
      }

      Cmd::ReachCarrierUserCmd message;
      ScenePos *p = message.mutable_pos();
      p->set_x(m_oData.m_oStartPos.getX());
      p->set_y(m_oData.m_oStartPos.getY());
      p->set_z(m_oData.m_oStartPos.getZ());
      reach(message);

      return;
    }
  }
  else      // 被邀请者
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByID(m_oData.m_qwMasterID);
    if (pUser)
    {
      pUser->m_oCarrier.m_members.erase(m_dwIndex);
      pUser->m_oCarrier.m_oInvites.erase(pUser->id);
    }
  }

  m_pUser->setScenePos(m_oData.m_oStartPos);

  ScenePos *sp = message.mutable_pos();
  sp->set_x(m_pUser->getPos().getX());
  sp->set_y(m_pUser->getPos().getY());
  sp->set_z(m_pUser->getPos().getZ());

  if (m_pUser->getScene())
  {
    message.set_charid(m_pUser->id);
    PROTOBUF(message, send, len);
    m_pUser->getScene()->sendCmdToNine(m_pUser->getPos(), send, len);
  }
  // send();
  clear();
}

void Carrier::reach(Cmd::ReachCarrierUserCmd &message)
{
  if (!m_oData.m_dwCarrierID) return;

  xPos dest;
  if (message.has_pos())
  {
    dest.set(message.pos().x(), message.pos().y(), message.pos().z());
  }
  else
  {
    const BusBase *base = TableManager::getMe().getBusCFG(m_oData.m_dwCarrierID);
    if (base && base->getPos(m_oData.m_dwLine, dest))
    {
      ScenePos *p = message.mutable_pos();
      p->set_x(dest.getX());
      p->set_y(dest.getY());
      p->set_z(dest.getZ());
    }
  }

  message.set_masterid(m_oData.m_qwMasterID);

  DWORD dwCarrierID = m_oData.m_dwCarrierID;
  SceneUser *pMaster = SceneUserManager::getMe().getUserByID(m_oData.m_qwMasterID);
  if (pMaster)
  {
    TSetTmpUser setUser;
    setUser.insert(pMaster);
    for (auto it : pMaster->m_oCarrier.m_members)
      setUser.insert(it.second);

    if (pMaster->getScene() != nullptr)
    {
      if (dynamic_cast<FerrisWheelScene*>(pMaster->getScene()) != nullptr)
        pMaster->getAchieve().onFerrisWheel();
      else if (dynamic_cast<DivorceRollerCoasterScene*>(pMaster->getScene()) != nullptr)    //离婚过山车
        pMaster->getUserSceneData().setDivorceRollerCoaster(true);
      else
      {
        pMaster->getAchieve().onCarrier(dwCarrierID);
        pMaster->getQuest().onCarrier(setUser, dwCarrierID);
      }
    }
    for (auto it : pMaster->m_oCarrier.m_members)
    {
      it.second->m_oCarrier.clear();
      it.second->setScenePos(dest);
      if (it.second->getScene() != nullptr)
      {
        if (dynamic_cast<FerrisWheelScene*>(it.second->getScene()) != nullptr)
          it.second->getAchieve().onFerrisWheel();
        else if (dynamic_cast<DivorceRollerCoasterScene*>(pMaster->getScene()) != nullptr)    //离婚过山车
          it.second->getUserSceneData().setDivorceRollerCoaster(true);
        else
        {
          it.second->getAchieve().onCarrier(dwCarrierID);
          it.second->getQuest().onCarrier(setUser, dwCarrierID);
        }
      }
    }

    pMaster->m_oCarrier.clear();
    pMaster->setScenePos(dest);

    pMaster->m_oCarrier.m_members.clear();
  }

  if (m_pUser->getScene())
  {
    const xPos& rPos = m_pUser->getPos();
    PROTOBUF(message, send, len);
    m_pUser->getScene()->sendCmdToNine(rPos, send, len);
    XLOG << "[载具-终点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "到达终点 pos :" << rPos.x << rPos.y << rPos.z << "mapid :" << m_pUser->getScene()->id << XEND;
  }

  // 下载具后有短时间无敌状态, 被怪打死, 前端动作播放出现问题
  m_pUser->m_oBuff.add(101, m_pUser);
  m_pUser->getUserPet().onUserOffCarrier();
}

bool Carrier::move(Cmd::CarrierMoveUserCmd &message)
{
  if (!m_oData.m_dwCarrierID || !isMaster(m_pUser->id)) return false;
  if (message.progress() <= m_dwProgress) return false;

  xPos dest;
  dest.set(message.pos().x(), message.pos().y(), message.pos().z());

  message.set_masterid(m_oData.m_qwMasterID);
  m_dwProgress = message.progress();

  m_pUser->setScenePos(dest);
  for (auto it : m_members)
  {
    it.second->setScenePos(dest);
  }
  return true;
}

bool Carrier::canJoin(SceneUser *user)
{
  if (!user) return false;
  if (!user->getScene() || !user->isAlive()) return false;

  return true;
}

bool Carrier::isMaster(QWORD id)
{
  return id==m_oData.m_qwMasterID;
}

// 通知9屏载具数据
void Carrier::sendMeToNine()
{
  if (!m_pUser->getScene()) return;

  Cmd::CarrierInfoUserCmd message;
  message.set_carrierid(m_oData.m_dwCarrierID);
  message.set_masterid(m_oData.m_qwMasterID);
  message.set_needanimation(m_oData.m_dwNeedAnimation);
  for (auto it : m_members)
  {
    Cmd::CarrierMember *p = message.add_members();
    p->set_id(it.second->id);
    p->set_index(it.first);
  }
  PROTOBUF(message, send, len);
  m_pUser->getScene()->sendCmdToNine(m_pUser->getPos(), send, len);
}

void Carrier::clear()
{
  setCarrierID(0);
  m_oData.reset();
  m_dwIndex = 0;
  m_dwProgress = 0;
  m_oInvites.clear();
  m_oBeInvites.clear();
}

void Carrier::start()
{
  if (!m_pUser->getScene()) return;
  if (!isMaster(m_pUser->id)) return;

  Cmd::CarrierStartUserCmd message;
  message.set_masterid(m_oData.m_qwMasterID);
  message.set_line(m_oData.m_dwLine);
  PROTOBUF(message, send, len);
  m_pUser->getScene()->sendCmdToNine(m_pUser->getPos(), send, len);

  if (m_oData.m_dwAssembleID)
  {
    changeAssemble(m_oData.m_dwAssembleID);
  }

  m_oData.m_oStartPos = m_pUser->getPos();
  for (auto it : m_members)
  {
    it.second->m_oCarrier.m_oData.m_oStartPos = m_pUser->getPos();
    SceneUser* pUser = it.second;
    if (pUser)
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CARIIER_COUNT, m_oData.m_dwCarrierID, 0, 0, (DWORD)1);
  }
  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CARIIER_COUNT, m_oData.m_dwCarrierID, 0, 0, (DWORD)1);
  m_dwProgress = 1;

  if (dynamic_cast<FerrisWheelScene*>(m_pUser->getScene()) == nullptr)
  {
    const TMapGTeamMember& mapMember = m_pUser->getTeam().getTeamMemberList();
    if (mapMember.size() == 2)
    {
      SceneUser* pUser1 = nullptr;
      SceneUser* pUser2 = nullptr;
      DWORD dwIndex = 0;
      for (auto &m : mapMember)
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(m.first);
        if (pUser == nullptr)
          continue;

        if (dwIndex == 0)
          pUser1 = pUser;
        if (dwIndex == 1)
          pUser2 = pUser;

        ++dwIndex;
      }
      if (pUser1 != nullptr && pUser2 != nullptr)
      {
        pUser1->getShare().addCalcData(ESHAREDATATYPE_MOST_CARRIER, pUser2->id, 1);
        pUser2->getShare().addCalcData(ESHAREDATATYPE_MOST_CARRIER, pUser1->id, 1);

        pUser1->getAchieve().onTravel();
        pUser2->getAchieve().onTravel();
      }
    }
  }

  XLOG << "[载具]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "开始载具:" << m_oData.m_dwCarrierID << XEND;
}

void Carrier::save(Cmd::BlobCarrier *data)
{
  using namespace Cmd;
  data->Clear();

  if (m_qwJoinMasterID)
    data->set_joinmaster(m_qwJoinMasterID);
  if (m_dwBuyAssembleID)
    data->set_assemble(m_dwBuyAssembleID);
  XDBG << "[载具-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
}

void Carrier::load(const Cmd::BlobCarrier &data)
{
  using namespace Cmd;
  int version = data.version();

  if (data.has_joinmaster())
  {
    m_qwJoinMasterID = data.joinmaster();
  }

  if (data.has_assemble())
  {
    m_dwBuyAssembleID = data.assemble();
  }

  if (version >= 2)
  {
  }
}

void Carrier::sendCmdAll(const void* data, unsigned short len)
{
  for (auto it : m_oInvites)
  {
    thisServer->sendCmdToMe(it, data, len);
  }
  m_pUser->sendCmdToMe(data, len);
}

bool Carrier::changeAssemble(DWORD dwAssembleID)
{
  if (m_pUser->id == m_oData.m_qwMasterID)
  {
    for (auto it : m_members)
    {
      it.second->m_oCarrier.m_oData.m_dwAssembleID = dwAssembleID;
    }
  }
  m_oData.m_dwAssembleID = dwAssembleID;

  // 9屏同步
  ChangeCarrierUserCmd cmd;
  cmd.set_masterid(m_oData.m_qwMasterID);
  cmd.set_carrierid(dwAssembleID);
  PROTOBUF(cmd, send, len);
  m_pUser->getScene()->sendCmdToNine(m_pUser->getPos(), send, len);
  XLOG << "[更换载具],载具id:" << m_oData.m_dwCarrierID << "masterid:" << m_oData.m_qwMasterID << XEND;
  return true;
}

void Carrier::setCarrierID(DWORD dwID)
{
  m_oData.m_dwCarrierID = dwID;
  m_pUser->setDataMark(EUSERDATATYPE_CARRIER);

  if (m_oData.m_dwCarrierID)
  {
    if (m_pUser->m_oHands.has() == false)
    m_pUser->getUserSceneData().setFollowerID(0);
    // ChatMessage::sendSysMsg(pMember, 330, EMESSAGETYPE_FRAME, SysMsgParams());
  }
}

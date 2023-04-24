#include "UserHands.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneMap.pb.h"
#include "RecordCmd.pb.h"
#include "SceneServer.h"
#include "ChatRoomManager.h"
#include "MsgManager.h"
#include "PlatLogManager.h"
#include "SessionCmd.pb.h"
#include "AuguryMgr.h"
#include "MiscConfig.h"

UserHands::UserHands(SceneUser *user) :m_pUser(user), m_oTimer(40)
{
}

UserHands::~UserHands()
{
}

void UserHands::save(Cmd::BlobHands *data)
{
  data->Clear();
  data->set_ismaster(m_bMaster);
  data->set_otherid(m_qwOtherID);
  if (m_qwForceJoinUserID)
  {
    data->set_forcejoinid(m_qwForceJoinUserID);
  }

  data->set_last_handname(m_strLastName);
  data->set_handtimelen(m_dwHandTimeLen);
  data->set_nextrewardtime(m_dwRewardTimeLen);
  data->set_nexttiptime(m_dwOneMinTipsTime);
}

void UserHands::load(const Cmd::BlobHands &data)
{
  m_bMaster = data.ismaster();
  m_qwOtherID = data.otherid();
  m_qwForceJoinUserID = data.forcejoinid();

  m_strLastName = data.last_handname();
  m_dwHandTimeLen = data.handtimelen();
  m_dwRewardTimeLen = data.nextrewardtime();
  m_dwOneMinTipsTime = data.nexttiptime();
}

/*
bool UserHands::join(SceneUser *user)
{
  if (!user) return true;
  if (!user->getScene() || !m_pUser->getScene()) return false;
  if (user->getScene() != m_pUser->getScene()) return false;
  if (user->getHandNpc().haveHandNpc() || m_pUser->getHandNpc().haveHandNpc())
    return false;

  if (has())
  {
    if (user->m_oHands.has())
    {
      return true;
    }
    else
    {
      user->m_oHands.clear();
      return true;
    }
  }
  else
  {
    if (user->m_oHands.has())
    {
      clear();
      return true;
    }
  }

  if (m_qwOtherID == user->id && user->m_oHands.m_qwOtherID == m_pUser->id)
  {
  }
  else
  {
    if (m_qwOtherID == user->id)
    {
      m_qwOtherID = 0;
      return true;
    }
    if (user->m_oHands.m_qwOtherID == m_pUser->id)
    {
      user->m_oHands.m_qwOtherID = 0;
      return true;
    }
  }

  if (m_dwIsMaster && user->m_oHands.m_dwIsMaster)
  {
    m_dwIsMaster = 0;
    user->m_oHands.m_dwIsMaster = 0;
  }
  else if (!m_dwIsMaster && !user->m_oHands.m_dwIsMaster)
  {
    m_dwIsMaster = 1;
  }

  user->goTo(m_pUser->getPos());

  m_qwOtherID = user->id;
  m_pOther = user;
  m_pUser->getPackage().equip(EEQUIPOPER_OFFPOS, EEQUIPPOS_MOUNT, "");

  user->m_oHands.m_qwOtherID = m_pUser->id;
  user->m_oHands.m_pOther = m_pUser;
  user->getPackage().equip(EEQUIPOPER_OFFPOS, EEQUIPPOS_MOUNT, "");

  m_pUser->getUserSceneData().setFollowerIDNoCheck(0);
  user->getUserSceneData().setFollowerIDNoCheck(0);

  //牵手破隐
  m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  m_pOther->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);

  //牵手打断变身
  m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);
  m_pOther->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);

  if (m_pUser->getItemMusic().hasMusicItem())
    m_pUser->getItemMusic().sendMusicToMe(true, true);

  if (m_pOther->getItemMusic().hasMusicItem())
    m_pOther->getItemMusic().sendMusicToMe(true, true);


  // 退出聊天室
  if (m_pUser->getChatRoom() != nullptr)
    ChatRoomManager::getMe().exitRoom(m_pUser, m_pUser->getChatRoom()->getRoomID());
  if (m_pOther->getChatRoom() != nullptr)
    ChatRoomManager::getMe().exitRoom(m_pOther, m_pOther->getChatRoom()->getRoomID());

  send();

  return true;
}
*/

void UserHands::breakup(bool isOffLine /*=false*/)
{
  if (!m_pUser) return;
  if (!has()) return;

  QWORD otherid = m_qwOtherID;
  bool ismaster = m_bMaster;

  clear();

  // self
  if (!ismaster)
  {
    m_pUser->setDataMark(EUSERDATATYPE_HANDID);
    m_pUser->refreshDataAtonce();
    m_pUser->getUserSceneData().setFollowerIDNoCheck(0);// if has() 回调 breakup()

    //clear 
  }
  else
  {
    BeFollowUserCmd cmd;
    cmd.set_etype(EFOLLOWTYPE_BREAK);
    cmd.set_userid(otherid);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  if (m_pUser->getItemMusic().hasMusicItem())
    m_pUser->getItemMusic().sendMusicToMe(true, false);

  // other
  SceneUser* m_pOther = SceneUserManager::getMe().getUserByID(otherid);
  if (m_pOther)
  {
    m_pOther->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
    if (m_pOther->getItemMusic().hasMusicItem())
      m_pOther->getItemMusic().sendMusicToMe(true, false);

    m_pOther->m_oHands.breakByOther(isOffLine);

    m_pUser->m_oHands.stopCalcHandTime(m_pOther->m_oHands.isMaster() == true ? otherid : 0);
    m_pOther->m_oHands.stopCalcHandTime(m_pUser->m_oHands.isMaster() == true ? m_pUser->id : 0);
  }
  else
  {
    BreakHandSessionCmd cmd;
    cmd.set_userid(otherid);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  // 用于下线保存
  if (isOffLine)
  {
    m_bMaster = ismaster;
    m_qwForceJoinUserID = otherid;
  }

  AuguryMgr::getMe().quit(m_pUser);

  m_pUser->m_oBuff.onHandChange();
}

void UserHands::breakByOther(bool isOffLine)
{
  if (!m_pUser) return;
  if (!has())
    return;

  m_pUser->m_oBuff.delBuffByType(EBUFFTYPE_HIDE);
  if (m_pUser->getItemMusic().hasMusicItem())
    m_pUser->getItemMusic().sendMusicToMe(true, false);

  QWORD otherid = m_qwOtherID;
  bool ismaster = m_bMaster;
  clear();

  if (ismaster)
  {
    BeFollowUserCmd cmd;
    cmd.set_etype(EFOLLOWTYPE_BREAK);
    cmd.set_userid(otherid);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  else
  {
    m_pUser->setDataMark(EUSERDATATYPE_HANDID);
    m_pUser->refreshDataAtonce();
    m_pUser->getUserSceneData().setFollowerIDNoCheck(0); // if has() 回调 breakup()
  }

  if (isOffLine)
    m_qwForceJoinUserID = otherid;
  AuguryMgr::getMe().quit(m_pUser);

  m_pUser->m_oBuff.onHandChange();
}

void UserHands::move(xPos p)
{
  if (!m_bMaster || !has() || m_bWaitBuild) return;

  SceneUser* m_pOther = getOther();
  if (m_pOther)
  {
    m_pOther->setScenePos(p);
  }
}

void UserHands::setMaster(QWORD id)
{
  m_qwOtherID = id; m_qwForceJoinUserID = 0; m_bMaster = 0;
  
  XLOG << "[牵手] 被牵人" << m_pUser->id << "设置主牵人" << m_qwOtherID <<XEND;
  if (m_pUser->getScene())
    m_pUser->getShare().onHand(m_qwOtherID, m_pUser->getScene()->getMapID());
  startCalcHandTime();
  m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_HAND, id, 1);
}

void UserHands::setFollower(QWORD id) 
{ 
  m_qwOtherID = id; 
  m_bMaster = 1; 
  m_qwForceJoinUserID = 0; 
  if (m_pUser->getScene())
    m_pUser->getShare().onHand(m_qwOtherID, m_pUser->getScene()->getMapID());
  m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_HAND, id, 1);
}


QWORD UserHands::getMasterID()
{
  if (!has())
    return 0;
  return m_bMaster ? m_pUser->id : m_qwOtherID;
}

QWORD UserHands::getFollowerID()
{
  if (!has())
    return 0;
  return m_bMaster ? m_qwOtherID : m_pUser->id;
}

void UserHands::leaveScene()
{
  if (m_qwOtherID == 0)
    return;

  if (!m_bWaitBuild)
    send(false);

  QWORD otherid = 0;

  SceneUser* m_pOther = getOther();
  if (m_pOther)
  {
    m_pOther->m_oHands.stopCalcHandTime(isMaster() ? m_pUser->id : 0);
    otherid = m_pOther->m_oHands.isMaster() ? m_pOther->id : 0;
  }
  stopCalcHandTime(otherid);

  m_pUser->m_oHands.setWaitStatus();

  SceneUser* user = SceneUserManager::getMe().getUserByID(m_qwOtherID);
  if (user == nullptr || user->getScene() != m_pUser->getScene())
    return;
  user->m_oHands.setWaitStatus();

  AuguryMgr::getMe().quit(m_pUser);
}

void UserHands::userOnline()
{
  //if (has() && !m_bWaitBuild)
}

/*
   //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Hand;
    PlatLogManager::getMe().eventLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eid,
      0,
      eType, 0, 1);

    PlatLogManager::getMe().SocialLog(thisServer,
      m_pUser->getUserSceneData().getPlatformId(),
      m_pUser->getZoneID(),
      m_pUser->accid,
      m_pUser->id,
      eType,
      eid,
      ESocial_Hand,
      0,
      m_qwForceJoinUserID,
      0, 0);

    m_qwForceJoinUserID = 0;
  }
  else if (m_qwOtherID && !m_pOther)
  {
    if (!checkInTeam(m_qwOtherID))
    {
      clear();
      return;
    }
    SceneUser *pUser = SceneUserManager::getMe().getUserByID(m_qwOtherID);
    if (pUser && pUser->m_oHands.join(m_pUser))
    {
    }
  }
}
*/

void UserHands::enterScene()
{
  // if has hands, recover
  if (m_qwOtherID)
  {
    SceneUser* user = SceneUserManager::getMe().getUserByID(m_qwOtherID);
    if (user == nullptr || user->getScene() == nullptr || user->getScene() != m_pUser->getScene())
    {
      return;
    }
    if (isMaster())
      user->m_oHands.startCalcHandTime();
    else
      m_pUser->m_oHands.startCalcHandTime();
  }


  if (m_qwForceJoinUserID == 0)
    return;
  // 对方下线, 切换地图
  SceneUser* user = SceneUserManager::getMe().getUserByID(m_qwForceJoinUserID);
  if (user == nullptr || user->getScene() == nullptr || user->getScene()->id != m_pUser->getUserSceneData().getOnlineMapID())
  {
    m_qwForceJoinUserID = 0;
    return;
  }
  m_qwForceJoinUserID = 0;

  // overtime
  /*if (now() > m_pUser->getUserSceneData().getOfflineTime() + 60)
  {
    return;
  }
  */
  // 不再同队
  if (user->isMyTeamMember(m_pUser->id) == false)
  {
    return;
  }
  // 另一方重新建立了牵手
  if (user->m_oHands.has())
  {
    return;
  }
  // 另一方是被牵人且在跟随状态中
  if (m_bMaster && user->getUserSceneData().getFollowerID() != 0)
  {
    return;
  }

  // 重新建立牵手
  if (m_bMaster)
  {
    user->getUserSceneData().setFollowerID(m_pUser->id, EFOLLOWTYPE_HAND);
  }
  else
  {
    m_pUser->getUserSceneData().setFollowerID(user->id, EFOLLOWTYPE_HAND);
  }
}

bool UserHands::canBeAttack()
{
  return true;
}

bool UserHands::checkInTeam(QWORD charid)
{
  if (!charid) return false;
  if (m_pUser->getTeamID() != 0)
  {
    const GTeam& rTeam = m_pUser->getTeam();
    for (auto &m : rTeam.getTeamMemberList())
    {
      const TeamMemberInfo& rMember = m.second;
      if (m_pUser->id == rMember.charid())
        continue;

      if (rMember.charid() == charid)
        return true;
    }
  }
  return false;
}

QWORD UserHands::getHandFollowID()
{
  if (has() == true)
    return (!m_bMaster && !m_bWaitBuild) ? m_qwOtherID : 0;
  else if (m_pUser->getTwinsSponsor() == false && m_pUser->getRequestTime() > xTime::getCurSec())
    return m_pUser->getTwinsID();

  return 0;
}

void UserHands::changeHandStatus(bool bBuild, QWORD otherid)
{
  if (bBuild && isMaster())
    return;
  if (m_qwOtherID != otherid)
    return;

  if (bBuild)
  {
    if (otherid != m_qwOtherID)
      return;
    SceneUser* user = SceneUserManager::getMe().getUserByID(otherid);
    if (user == nullptr || user->getScene() != m_pUser->getScene())
      return;
    if (getDistance(m_pUser->getPos(), user->getPos()) > MiscConfig::getMe().getSystemCFG().dwHandRange)
      return;

    if (!isInWait() || !user->m_oHands.isInWait())
      return;

    setBuildStaus();
    user->m_oHands.setBuildStaus();

    m_pUser->setDataMark(EUSERDATATYPE_HANDID);
    m_pUser->refreshDataAtonce();
  }
  else
  {
    setWaitStatus();
    if (!isMaster())
    {
      m_pUser->setDataMark(EUSERDATATYPE_HANDID);
      m_pUser->refreshDataAtonce();
    }
    SceneUser* user = SceneUserManager::getMe().getUserByID(otherid);
    if (user == nullptr || user->getScene() != m_pUser->getScene())
      return;
    user->m_oHands.setWaitStatus();
  }
  send(bBuild);

  m_pUser->m_oBuff.onHandChange();
  SceneUser* m_pOther = getOther();
  if (m_pOther)
    m_pOther->m_oBuff.onHandChange();
}


void UserHands::userOffline()
{
  if (!has())
    return;
  breakup(true);
}

void UserHands::send(bool bBuild)
{
  if (!m_qwOtherID)
    return;

  HandStatusUserCmd cmd;
  cmd.set_build(bBuild);
  cmd.set_masterid(getMasterID());
  cmd.set_followid(getFollowerID());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
}

void UserHands::timeTick(DWORD curSec)
{
  if (m_oTimer.timeUp(curSec))
  {
    if (isMaster())
      return;

    if (isInWait())
      return;
    if (m_pUser == nullptr)// || m_pOther == nullptr)
      return;

    /*if (MiscConfig::getMe().getAuguryCfg().inTime(curSec))
    {
      calcHandTime(curSec);
      if (m_dwHandTimeLen > m_dwOneMinTipsTime)
      {
        //tips
        MsgParams param1;
        param1.addString(m_pOther->name);
        param1.addNumber(m_dwHandTimeLen / 60);
        MsgManager::sendMsg(m_pUser->id, MiscConfig::getMe().getAuguryCfg().dwHandTipSysId, param1);

        MsgParams param2;
        param2.addString(m_pUser->name);
        param2.addNumber(m_dwHandTimeLen / 60);
        MsgManager::sendMsg(m_pOther->id, MiscConfig::getMe().getAuguryCfg().dwHandTipSysId, param2);

        m_dwOneMinTipsTime = m_dwHandTimeLen + MiscConfig::getMe().getAuguryCfg().dwHandTipTime;

        XLOG << "[牵手-奖励-提示] " << m_pUser->id << m_pUser->name << "other" << m_pOther->name << m_dwHandTimeLen << "下次提示时间" << m_dwOneMinTipsTime << XEND;
      }

      if (m_dwHandTimeLen >= MiscConfig::getMe().getAuguryCfg().dwHandRewardTime && m_dwHandTimeLen > m_dwRewardTimeLen)
      {
        //reward
        DWORD takeCount = m_pUser->getVar().getVarValue(EVARTYPE_AUGURY_REWARD);
        if (takeCount < MiscConfig::getMe().getAuguryCfg().dwMaxRewardCountPerDay)
        {
          MsgParams param1;
          param1.addString(m_pOther->name);
          param1.addNumber(m_dwHandTimeLen / 60);
          MsgManager::sendMsg(m_pUser->id, MiscConfig::getMe().getAuguryCfg().dwRewardTipSysId, param1);

          ItemInfo itemInfo;
          itemInfo.set_id(MiscConfig::getMe().getAuguryCfg().dwItemId);
          itemInfo.set_count(1);
          m_pUser->getPackage().addItemAvailable(itemInfo);

          m_pUser->getVar().setVarValue(EVARTYPE_AUGURY_REWARD, takeCount + 1);
        }
        else
        {
          XLOG << "[牵手-奖励] 被牵人已达最大领取次数" << m_pUser->id << m_pUser->name << "已领取次数" << takeCount << "other" << m_pOther->name << m_dwHandTimeLen << "下次奖励时间" << m_dwRewardTimeLen << XEND;
        }

        takeCount = m_pOther->getVar().getVarValue(EVARTYPE_AUGURY_REWARD);
        if (takeCount < MiscConfig::getMe().getAuguryCfg().dwMaxRewardCountPerDay)
        {
          MsgParams param2;
          param2.addString(m_pUser->name);
          param2.addNumber(m_dwHandTimeLen / 60);
          MsgManager::sendMsg(m_pOther->id, MiscConfig::getMe().getAuguryCfg().dwRewardTipSysId, param2);

          ItemInfo itemInfo;
          itemInfo.set_id(MiscConfig::getMe().getAuguryCfg().dwItemId);
          itemInfo.set_count(1);
          m_pOther->getPackage().addItemAvailable(itemInfo);

          m_pOther->getVar().setVarValue(EVARTYPE_AUGURY_REWARD, takeCount + 1);
        }
        else
        {
          XLOG << "[牵手-奖励] 主牵人已达最大领取次数" << m_pUser->id << m_pUser->name << "已领取次数" << takeCount << "other" << m_pOther->name << m_dwHandTimeLen << "下次奖励时间" << m_dwRewardTimeLen << XEND;
        }

        m_dwRewardTimeLen = m_dwHandTimeLen + MiscConfig::getMe().getAuguryCfg().dwHandRewardTime;
        XLOG << "[牵手-奖励-发放奖励提示] " << m_pUser->id << m_pUser->name << "other" << m_pOther->name << m_dwHandTimeLen << "下次奖励时间" << m_dwRewardTimeLen << XEND;
      }
    }*/
  }
}

void UserHands::startCalcHandTime()
{
  if (isMaster())
    return;

  /*DWORD curSec = now();
  if (!MiscConfig::getMe().getAuguryCfg().inTime(curSec))
  {
    XDBG << "[牵手-奖励] 超过时间，不在计时 " << m_strLastName << "最近牵手时间点" << m_dwLastHandTime << "累计时长" << m_dwHandTimeLen << m_dwHandTimeLen / 60 << XEND;
    return;
  }*/
  
  if (bCalcHandTimeing)
  {
    XDBG << "[牵手-奖励-已经在计时] " << m_strLastName << "最近牵手时间点" << m_dwLastHandTime << "累计时长" << m_dwHandTimeLen << m_dwHandTimeLen / 60 << XEND;
    return;
  }

  SceneUser* pMaster = getOther();
  if (pMaster == nullptr)
  {
    XERR << "[牵手-奖励] 被牵人" << m_pUser->id << "找不到主牵人" << m_qwOtherID << XEND;
    return;
  }
  if (m_pUser->getScene() == nullptr || m_pUser->getScene() != pMaster->getScene())
    return;

  /*if (m_strLastName.empty())
  {
    m_strLastName = pMaster->name;

    //主牵人的上次牵手人是我
    if (pMaster->m_oHands.m_strLastName == m_pUser->name && m_dwHandTimeLen < pMaster->m_oHands.m_dwHandTimeLen)
    {
      //clone
      m_dwHandTimeLen = pMaster->m_oHands.m_dwHandTimeLen;
      m_dwRewardTimeLen = pMaster->m_oHands.m_dwRewardTimeLen;
      m_dwOneMinTipsTime = pMaster->m_oHands.m_dwOneMinTipsTime;
      XLOG << "[牵手-时长] 交换主牵人，被牵人" << m_pUser->id << "主牵人" << pMaster->id << "拷贝牵手时长" << m_dwHandTimeLen << "奖励时间" << m_dwRewardTimeLen << XEND;
    }
    else
    {
      m_dwHandTimeLen = 0;
      m_dwRewardTimeLen = MiscConfig::getMe().getAuguryCfg().dwHandRewardTime;
      m_dwOneMinTipsTime = MiscConfig::getMe().getAuguryCfg().dwHandTipTime;
    }
  }
  else
  {
    if (m_strLastName != pMaster->name)
    {
      XLOG << "[牵手-奖励] 更换牵手人清空牵手时长，上次" << m_strLastName << "现在" << pMaster->name << XEND;
      m_strLastName = pMaster->name;
      m_dwHandTimeLen = 0;
      m_dwRewardTimeLen = MiscConfig::getMe().getAuguryCfg().dwHandRewardTime;
      m_dwOneMinTipsTime = MiscConfig::getMe().getAuguryCfg().dwHandTipTime;
    }
    else if (!pMaster->m_oHands.m_strLastName.empty() && pMaster->m_oHands.m_strLastName != m_pUser->name)
    {
      XLOG << "[牵手-奖励] 主牵人抛弃了我,清空牵手时长，上次" << m_strLastName << "现在" << pMaster->name << XEND;
      m_strLastName = pMaster->name;
      m_dwHandTimeLen = 0;
      m_dwRewardTimeLen = MiscConfig::getMe().getAuguryCfg().dwHandRewardTime;
      m_dwOneMinTipsTime = MiscConfig::getMe().getAuguryCfg().dwHandTipTime;
    }
    else if (pMaster->m_oHands.m_strLastName == m_pUser->name && m_dwHandTimeLen < pMaster->m_oHands.m_dwHandTimeLen)
    {
      //clone
      m_dwHandTimeLen = pMaster->m_oHands.m_dwHandTimeLen;
      m_dwRewardTimeLen = pMaster->m_oHands.m_dwRewardTimeLen;
      m_dwOneMinTipsTime = pMaster->m_oHands.m_dwOneMinTipsTime;
      XLOG << "[牵手-时长] 交换主牵人，被牵人" << m_pUser->id << "主牵人" << pMaster->id << "拷贝牵手时长" << m_dwHandTimeLen << "奖励时间" << m_dwRewardTimeLen << XEND;
    }
  }
  pMaster->m_oHands.m_strLastName = m_pUser->name;*/

  m_dwLastHandTime = now();
  bCalcHandTimeing = true;
  XLOG << "[牵手-奖励-开始计时] " << m_pUser->id << "主牵人" << pMaster->id << m_strLastName << "最近牵手时间点" << m_dwLastHandTime << "累计时长" << m_dwHandTimeLen << m_dwHandTimeLen / 60 << XEND;
}

void UserHands::stopCalcHandTime(QWORD qwOtherID /*= 0*/)
{
  if (isMaster())
    return;
  if (!bCalcHandTimeing)
    return;

  if (m_dwLastHandTime)
  {
    DWORD dwDelta = now() - m_dwLastHandTime;
    m_dwHandTimeLen += dwDelta;//now() - m_dwLastHandTime;
    m_dwLastHandTime = 0;
    if (qwOtherID != 0)
    {
      SceneUser* pMaster = SceneUserManager::getMe().getUserByID(qwOtherID);
      if (pMaster != nullptr)
      {
        pMaster->getShare().addCalcData(ESHAREDATATYPE_MOST_HANDTIME, m_pUser->id, dwDelta);
        //if (dwDelta > 180)
          pMaster->getShare().addCalcData(ESHAREDATATYPE_MOST_HAND_TIMECOUNT, m_pUser->id, dwDelta);
        pMaster->getAchieve().onHandTime(dwDelta / MIN_T);
        pMaster->getQuest().onHand(m_pUser->id, m_dwHandTimeLen);
      }
    }
    m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_HANDTIME, qwOtherID, dwDelta);
    //if (dwDelta > 180)
      m_pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_HAND_TIMECOUNT, qwOtherID, dwDelta);
    m_pUser->getAchieve().onHandTime(dwDelta / MIN_T);
    m_pUser->getQuest().onHand(qwOtherID, m_dwHandTimeLen);
    XLOG << "[牵手-奖励-停止计时] " << m_pUser->id << m_strLastName << "最近牵手时间点" << m_dwLastHandTime << "累计时长" << m_dwHandTimeLen << m_dwHandTimeLen / 60 << "分" << XEND;
  }
  bCalcHandTimeing = false;
}

void UserHands::calcHandTime(DWORD curSec)
{
  if (isMaster())
    return;
  if (!bCalcHandTimeing)
    return;
  m_dwHandTimeLen += curSec - m_dwLastHandTime;
  m_dwLastHandTime = curSec;
  XLOG << "[牵手-奖励] 计时中。。。" << m_pUser->id << m_strLastName << "最近牵手时间点" << m_dwLastHandTime << "累计时长" << m_dwHandTimeLen << m_dwHandTimeLen / 60 << "分" << XEND;
}

SceneUser* UserHands::getOther()
{
  if (m_qwOtherID == 0)
    return nullptr;
  SceneUser* user = SceneUserManager::getMe().getUserByID(m_qwOtherID);
  if (user != nullptr && user->getScene() == m_pUser->getScene())
    return user;
  return nullptr;
}


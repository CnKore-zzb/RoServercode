#include "Booth.h"
#include "SceneUser.h"
#include "SceneBoothManager.h"
#include "MatchSCmd.pb.h"

Booth::Booth(SceneUser* pUser) : m_pUser(pUser)
{
  m_strName = "";
  m_eSign = Cmd::EBOOTHSIGN_WHITE;
  m_bOpen = false;
  m_dwUpdateTime = 0;
  m_dwScore = 0;
}

Booth::~Booth()
{
}

bool Booth::req(const Cmd::BoothReqUserCmd& cmd)
{
  if(!m_pUser)
    return false;

  if(MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_BOOTH))
    return false;

  XLOG << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << cmd.ShortDebugString() << XEND;
  // 检测职业
  /*if(Cmd::EPROFESSION_MERCHANT != RoleConfig::getMe().getBaseProfession(m_pUser->getProfession()))
  {
    XERR << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "oper:" << cmd.oper() << "职业不符合,疑似玩家作弊" << XEND;
    return false;
  }*/

  // 检测技能
  if(m_pUser->getSkillLv( MiscConfig::getMe().getBoothCFG().dwSkillId) <= 0)
  {
    XERR << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "摆摊开启失败，没有对应的技能! 疑似玩家作弊" << XEND;
    return false;
  }


  if(Cmd::EBOOTHOPER_OPEN == cmd.oper())
  {
    if(!open(cmd.name()))
    {
      XERR << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "oper:" << cmd.oper() << "name:" << cmd.name() << "open failed!" << XEND;
      return false;
    }
  }
  else if(Cmd::EBOOTHOPER_UPDATE == cmd.oper())
  {
    if(!update(cmd.name()))
    {
      XERR << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "oper:" << cmd.oper() << "name:" << cmd.name() << "update failed!" << XEND;
      return false;
    }
  }
  else if(Cmd::EBOOTHOPER_CLOSE == cmd.oper())
  {
    if(!close())
    {
      XERR << "[摆摊-请求]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "oper:" << cmd.oper() << "name:" << cmd.name() << "close failed!" << XEND;
      return false;
    }
  }
  else
    return false;

  sendCmdToNine(cmd.oper());
  sendCmdToMatchServer(cmd.oper());
  return true;
}

bool Booth::open(const string& name)
{
  if(!m_pUser)
    return false;

  if(hasOpen())
  {
    XERR << "[摆摊-open]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "摆摊开启失败，已处于开启状态!" << XEND;
    return false;
  }

  // 检测名字
  if (ESYSTEMMSG_ID_MIN != MiscConfig::getMe().getSystemCFG().checkNameValid(name, ENAMETYPE_BOOTH))
  {
    XERR << "[摆摊-open]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "名字检测失败! name:" << name << XEND;
    return false;
  }

  // 检测地图
  if(!MiscConfig::getMe().getBoothCFG().checkMap(m_pUser->getMapID()))
  {
    XERR << "[摆摊-open]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "地图检测失败! map:" << m_pUser->getMapID() << XEND;
    return false;
  }

  // 添加buff
  m_pUser->m_oBuff.add(MiscConfig::getMe().getBoothCFG().dwBuffId, m_pUser);
  m_strName = name;
  m_eSign = static_cast<Cmd::EBoothSign>(MiscConfig::getMe().getBoothCFG().getSign(m_dwScore));
  m_bOpen = true;
  m_pUser->getEvent().onBoothOpen();

  Cmd::BoothOpenTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_open(1);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionTrade(m_pUser->id, m_pUser->name, send, len);

  // 同步状态至redis（交易所服用）
  GCharWriter gChar(thisServer->getRegionID(), m_pUser->id);
  gChar.setBoothOpenStatus(1);
  gChar.save();
  return true;
}

bool Booth::update(const std::string& name)
{
  if(!m_pUser)
    return false;

  if(!hasOpen())
  {
    XERR << "[摆摊-update]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "摆摊更新失败，未处于开启状态!" << XEND;
    return false;
  }

  DWORD curTime = now();
  if(curTime <= m_dwUpdateTime)
  {
    MsgManager::sendMsg(m_pUser->id, 25709);
    return false;
  }

  // 检测名字
  if (ESYSTEMMSG_ID_MIN != MiscConfig::getMe().getSystemCFG().checkNameValid(name, ENAMETYPE_BOOTH))
  {
    XERR << "[摆摊-update]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "名字检测失败! name:" << name << XEND;
    return false;
  }

  m_dwUpdateTime = curTime + MiscConfig::getMe().getBoothCFG().dwUpdateCD;
  m_strName = name;
  return true;
}

bool Booth::close()
{
  if(!m_pUser)
    return false;

  if(!hasOpen())
  {
    XERR << "[摆摊-close]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "摆摊关闭失败，未处于开启状态!" << XEND;
    return false;
  }

  m_strName = "";

  // 移除buff
  m_pUser->m_oBuff.del(MiscConfig::getMe().getBoothCFG().dwBuffId, m_pUser);
  m_pUser->m_oBuff.update(xTime::getCurMSec());
  m_bOpen = false;

  BoothOpenTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_open(0);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionTrade(m_pUser->id, m_pUser->name, send, len);

  // 同步状态至redis（交易所服用）
  GCharWriter gChar(thisServer->getRegionID(), m_pUser->id);
  gChar.setBoothOpenStatus(0);
  gChar.save();

  return true;
}

void Booth::onRelive(Cmd::EReliveType eType)
{
  if(!m_pUser)
    return;

  if(!hasOpen())
    return;

  if(ERELIVETYPE_SKILL != eType)
  {
    XLOG << "[摆摊-复活]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "非技能复活，关闭摆摊状态!" << XEND;
    close();
    sendCmdToMe(Cmd::EBOOTHOPER_CLOSE);
    sendCmdToNine(Cmd::EBOOTHOPER_CLOSE);
    sendCmdToMatchServer(Cmd::EBOOTHOPER_CLOSE);
  }
}

void Booth::onLogin()
{
  m_bOpen = false;
}

void Booth::onLeaveScene()
{
  if(!m_pUser)
    return;

  if(!hasOpen())
    return;

  XLOG << "[摆摊-离开场景]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "离开场景，关闭摆摊状态!" << XEND;
  close();
  sendCmdToMe(Cmd::EBOOTHOPER_CLOSE);
  sendCmdToMatchServer(Cmd::EBOOTHOPER_CLOSE);
}

void Booth::onCarrier()
{
  if(!m_pUser)
    return;

  if(!hasOpen())
    return;

  XLOG << "[摆摊-加入载具]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "加入载具，关闭摆摊状态!" << XEND;
  close();
  sendCmdToMe(Cmd::EBOOTHOPER_CLOSE);
  sendCmdToNine(Cmd::EBOOTHOPER_CLOSE);
  sendCmdToMatchServer(Cmd::EBOOTHOPER_CLOSE);
}

bool Booth::load(const Cmd::BlobBooth& oData)
{
  m_dwScore = oData.score();
  return true;
}

bool Booth::save(Cmd::BlobBooth* pData)
{
  if(!pData)
    return false;

  pData->set_score(m_dwScore);
  return true;
}

bool Booth::toBoothData(Cmd::BoothInfo* pData)
{
  if(!pData)
    return false;

  pData->set_name(m_strName);
  pData->set_sign(m_eSign);
  return true;
}

DWORD Booth::getSize()
{
  if(!m_pUser)
    return 0;

  DWORD size = LuaManager::getMe().call<DWORD>("calcBoothMaxPendingCout", (xSceneEntryDynamic*)(m_pUser));
  if(size > MiscConfig::getMe().getBoothCFG().dwPendingCountMax)
    size = MiscConfig::getMe().getBoothCFG().dwPendingCountMax;

  return size;
}

void Booth::calcTax(QWORD tax)
{
  if(!tax || !m_pUser)
    return;

  XLOG << "[摆摊-积分]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "税收增加! tax:" << tax << XEND;
  addScore(MiscConfig::getMe().getBoothCFG().getScoreByRate(tax));
  m_pUser->setDataMark(EUSERDATATYPE_BOOTH_SCORE);
  m_pUser->refreshDataAtonce();
}

void Booth::addScore(DWORD score)
{
  if(!score)
    return;

  m_dwScore += score;

  MsgParams params;
  params.addNumber(score);
  MsgManager::sendMsg(m_pUser->id, 25706, params);
  XLOG << "[摆摊-积分]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "积分增加! score:" << score << " total_score:" << m_dwScore << XEND;
}

void Booth::sendCmdToMe(Cmd::EBoothOper oper)
{
  if(!m_pUser)
    return;

  Cmd::BoothReqUserCmd cmd;
  cmd.set_oper(oper);
  cmd.set_name(m_strName);
  cmd.set_success(true);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[摆摊-同步自己]" << m_pUser->accid << m_pUser->id << m_pUser->name << "oper:" << oper << "name:" << m_strName << "同步摆摊信息至九屏" << XEND;
}

void Booth::sendCmdToNine(Cmd::EBoothOper oper)
{
  if(!m_pUser)
    return;

  Cmd::BoothInfoSyncUserCmd cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_oper(oper);
  cmd.mutable_info()->set_name(m_strName);
  cmd.mutable_info()->set_sign(getSign());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
  XLOG << "[摆摊-九屏]" << m_pUser->accid << m_pUser->id << m_pUser->name << "oper:" << oper << "name:" << m_strName << "同步摆摊信息至九屏" << XEND;
}

void Booth::sendCmdToMatchServer(Cmd::EBoothOper oper)
{
  if(!m_pUser)
    return;

  Cmd::UserBoothReqMatchSCmd cmd;
  cmd.set_oper(oper);
  cmd.set_sceneid(m_pUser->getMapID());
  cmd.set_zoneid(thisServer->getZoneID());

  Cmd::MapUser mapUser;
  m_pUser->fillMapBoothData(&mapUser);
  cmd.mutable_user()->CopyFrom(mapUser);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[摆摊-同步]" << m_pUser->accid << m_pUser->id << m_pUser->name << "oper:" << oper << "发送请求至matchserver" << XEND;
}


#include "SessionUser.h"
#include "SessionUserManager.h"
#include "ServerTask.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "GatewayCmd.h"
#include "RedisManager.h"
#include "SceneTrade.pb.h"
#include "LuaManager.h"
#include "SessionSociality.pb.h"
#include "SocialCmd.pb.h"
#include "UserActive.h"
#include "SealManager.h"
#include "SessionActivityMgr.h"
#include "SocialCmd.pb.h"
#include "ChatManager_SE.h"
#include "MatchSCmd.pb.h"
#include "Authorize.pb.h"
#include "ActivityManager.h"
#include "AuctionCCmd.pb.h"
#include "AuctionSCmd.pb.h"
#include "DepositConfig.h"
#include "WeddingSCmd.pb.h"
#include "GMCommandManager.h"
#include "WorldLevelManager.h"
#include "Boss.h"

// reward manager
bool RewardManager::roll(DWORD id, SessionUser* pUser, TVecItemInfo& vecItemInfo, ESource source, float fRatio /* = 1.0f*/, DWORD dwMapID /* = 0 */)
{
  RewardEntry oEntry;
  if (pUser != nullptr)
  {
    oEntry.set_pro(pUser->getProfession());
    oEntry.set_baselv(pUser->getBaseLevel());
    oEntry.set_joblv(pUser->getJobLevel());
  }
  return RewardConfig::getMe().roll(id, oEntry, vecItemInfo, source, fRatio, dwMapID);
}

// session user
SessionUser::SessionUser(QWORD uid, const char *uName, QWORD accID, ServerTask* net, DWORD platformId, const string& deviceid) :
  m_platformId(platformId),
  m_strDeviceID(deviceid),
  m_oGCharData(thisServer->getRegionID(), uid),
  m_oMail(this),
  m_oTradeLog(this),
  m_oAuthorize(this),
  m_oAuction(this)
{
  set_id(uid);
  set_name(uName);
  set_tempid(accID);
  this->accid = accID;
  gate_task_ = net;
  user_state = USER_STATE_NONE;
  ip = 0;
  myScene = NULL;
  m_oGTeam.setCharID(uid);

  m_oGCharData.getByTeam();
}

SessionUser::~SessionUser()
{

}

//one sec tick
void SessionUser::timer(DWORD curTime)
{
  m_oMail.timer(curTime);
  m_oTradeLog.timer(curTime);
}

void SessionUser::userOnline()
{
  if (getUserState() != USER_STATE_CHANGE_SCENE)
  {
    Cmd::OnlineStatusSocialCmd message;
    toData(message.mutable_user());
    message.set_online(1);
    PROTOBUF(message, send, len);

    thisServer->sendCmdToServer(send, len, "TeamServer");
    thisServer->sendCmdToServer(send, len, "SocialServer");
    thisServer->sendCmd(ClientType::global_server, send, len);
    thisServer->sendCmd(ClientType::trade_server, send, len);
    thisServer->sendCmd(ClientType::guild_server, send, len);
    thisServer->sendCmd(ClientType::match_server, send, len);
    thisServer->sendCmd(ClientType::auction_server, send, len);
    thisServer->sendCmd(ClientType::wedding_server, send, len);

    UserActive::getMe().add(accid);
    m_oMail.onUserOnline();
    m_oTradeLog.onUserOnline();
    SessionActivityMgr::getMe().onUserOnline(this);
    SessionUserManager::getMe().sendActivityProgress(this);
    ChatManager_SE::getMe().onUserOnline(this);
    ActivityManager::getMe().onUserOnline(this);
    ActivityEventManager::getMe().onUserOnline(this);
    WorldLevelManager::getMe().onUserOnline(this);

    if (QuestConfig::getMe().getActiveWanted() == true)
    {
      QueryWantedInfoQuestCmd cmd;
      cmd.set_maxcount(QuestConfig::getMe().getActiveMaxWantedCount());
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
      XLOG << "[玩家-登录]" << accid << id << getProfession() << name << "同步了看板信息" << cmd.ShortDebugString() << XEND;
    }

    if (m_qwZenyDebt > 0)
      MsgManager::sendMsg(id, ESYSTEMMSG_ID_DEBT_LOGIN, MsgParams(m_qwZenyDebt));

    m_oAuction.onUserOnline();
    sendActivityCnt();
    sendVersionVard2Client();

    XLOG << "[玩家-登录]" << accid << id << getProfession() << name
      << "注册成功,加入" << (getScene() ? getScene()->name : "未知地图") << "当前在线人数:" << SessionUserManager::getMe().size() << XEND;
  }
  else
  {
    GuildUserInfoSyncGuildCmd usercmd;
    usercmd.set_charid(id);
    if (m_oGuild.toData(usercmd.mutable_info()) == true)
    {
      PROTOBUF(usercmd, usersend, userlen);
      sendCmdToScene(usersend, userlen);
    }

    GuildArtifactQuestGuildSCmd quest;
    quest.set_charid(id);
    if (m_oGuild.toData(quest.mutable_quest()) == true)
    {
      PROTOBUF(quest, questsend, questlen);
      sendCmdToScene(questsend, questlen);
    }

    GuildInfoSyncGuildSCmd cmd;
    cmd.set_charid(id);
    if (m_oGuild.toData(cmd.mutable_info()) == true)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToScene(send, len);
    }

    TeamDataSyncTeamCmd teamcmd;
    teamcmd.set_charid(id);
    teamcmd.set_online(true);
    if (m_oGTeam.toData(teamcmd.mutable_info()) == true)
    {
      PROTOBUF(teamcmd, teamsend, teamlen);
      sendCmdToScene(teamsend, teamlen);
    }

    SyncSocialListSocialCmd socialcmd;
    if (m_oSocial.toData(socialcmd) == true)
    {
      PROTOBUF(socialcmd, socialsend, sociallen);
      sendCmdToScene(socialsend, sociallen);
    }

    sendWeddingInfo2Scene();

    XLOG << "[玩家-切场景]" << accid << id << getProfession() << name
      << "注册成功,加入" << (getScene() ? getScene()->name : "未知地图") << "当前在线人数:" << SessionUserManager::getMe().size() << XEND;
  }

  getAuthorize().syncInfoToScene();
  setUserState(USER_STATE_RUN);
  setSceneServerUserState(SceneServerUserState::run);
}

void SessionUser::userOffline()
{
  UserActive::getMe().setActiveTime(accid, now());
  UserActive::getMe().update();

  Cmd::OnlineStatusSocialCmd message;
  toData(message.mutable_user());
  message.set_online(0);
  PROTOBUF(message, send, len);
  thisServer->sendCmd(ClientType::global_server, send, len);
  thisServer->sendCmdToServer(send, len, "TeamServer");
  thisServer->sendCmdToServer(send, len, "SocialServer");
  thisServer->sendCmd(ClientType::guild_server, send, len);
  thisServer->sendCmd(ClientType::trade_server, send, len);
  thisServer->sendCmd(ClientType::match_server, send, len);
  thisServer->sendCmd(ClientType::auction_server, send, len);
  thisServer->sendCmd(ClientType::wedding_server, send, len);

  SealManager::getMe().onUserOffline(this);
  SessionUserManager::getMe().removeCharIDFromSysCache(id);
  if (m_oGTeam.getTeamID() != 0)
    delOneLevelIndex(ONE_LEVEL_INDEX_TYPE_TEAM, m_oGTeam.getTeamID());
  AuctionMgr::getMe().closePanel(this);
}

bool SessionUser::sendCmdToMe(const void* cmd,WORD len)
{
  if (!gate_task_)
    return false;

  BUFFER_CMD_SIZE(send, ForwardToUserGatewayCmd, sizeof(ForwardToUserGatewayCmd)+len);
  send->accid = accid;
  send->len = len;
  bcopy(cmd,send->data,len);
  gate_task_->sendCmd(send, sizeof(ForwardToUserGatewayCmd)+len);

  return true;
}

void SessionUser::addOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i)
{
  if (gate_task_)
  {
    AddOneLevelIndexGatewayCmd send;
    send.indexT = indexT;
    send.i = i;
    send.accid = accid;
    gate_task_->sendCmd(&send, sizeof(send));
  }
}

void SessionUser::delOneLevelIndex(ONE_LEVEL_INDEX_TYPE indexT, QWORD i)
{
  if (gate_task_)
  {
    DelOneLevelIndexGatewayCmd send;
    send.indexT = indexT;
    send.i = i;
    send.accid = accid;
    gate_task_->sendCmd(&send, sizeof(send));
  }
}

void SessionUser::setScene(SessionScene* s)
{
  if (!s) return;

  if (s->isDScene())
  {
    myScene = SessionSceneManager::getMe().mapRedirect(s);
  }
  else
  {
    myScene = s;
  }

  m_qwSceneID = s->id;
}

bool SessionUser::sendCmdToScene(const void* cmd,WORD len)
{
  if (myScene)
    return myScene->sendCmd(cmd,len);
  else
    return false;
}

bool SessionUser::sendCmdToSceneUser(const void *cmd, WORD len)
{
  if (!myScene)
    return false;
  SessionSceneUserCmd sceneCmd;
  sceneCmd.set_userid(id);
  sceneCmd.set_cmddata(cmd, len);
  PROTOBUF(sceneCmd, send, len2);
  return myScene->sendCmd(send, len2);
  return true;
}

void SessionUser::toData(UserInfo* pInfo)
{
  if (pInfo == nullptr)
    return;

  toData(pInfo->mutable_user());

  add_data(pInfo->add_datas(), EUSERDATATYPE_ROLELEVEL, m_oGCharData.getBaseLevel());
  add_data(pInfo->add_datas(), EUSERDATATYPE_MAPID, m_oGCharData.getMapID());
  add_data(pInfo->add_datas(), EUSERDATATYPE_ONLINETIME, m_oGCharData.getOnlineTime());
  add_data(pInfo->add_datas(), EUSERDATATYPE_RAIDID, m_dwRaidID);
  add_data(pInfo->add_datas(), EUSERDATATYPE_PORTRAIT, m_oGCharData.getPortrait());
  add_data(pInfo->add_datas(), EUSERDATATYPE_BODY, m_oGCharData.getBody());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HEAD, m_oGCharData.getHead());
  add_data(pInfo->add_datas(), EUSERDATATYPE_FACE, m_oGCharData.getFace());
  add_data(pInfo->add_datas(), EUSERDATATYPE_BACK, m_oGCharData.getBack());
  add_data(pInfo->add_datas(), EUSERDATATYPE_TAIL, m_oGCharData.getTail());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HAIR, m_oGCharData.getHair());
  add_data(pInfo->add_datas(), EUSERDATATYPE_EYE, m_oGCharData.getEye());
  add_data(pInfo->add_datas(), EUSERDATATYPE_HAIRCOLOR, m_oGCharData.getHairColor());
  add_data(pInfo->add_datas(), EUSERDATATYPE_CLOTHCOLOR, m_oGCharData.getClothColor());
  add_data(pInfo->add_datas(), EUSERDATATYPE_LEFTHAND, m_oGCharData.getLeftHand());
  add_data(pInfo->add_datas(), EUSERDATATYPE_RIGHTHAND, m_oGCharData.getRightHand());
  add_data(pInfo->add_datas(), EUSERDATATYPE_BLINK, m_oGCharData.getBlink());
  add_data(pInfo->add_datas(), EUSERDATATYPE_MOUTH, m_oGCharData.getMouth());

  add_data(pInfo->add_datas(), EUSERDATATYPE_PROFESSION, m_oGCharData.getProfession());
  add_data(pInfo->add_datas(), EUSERDATATYPE_SEX, m_oGCharData.getGender());
  add_data(pInfo->add_datas(), EUSERDATATYPE_NAME, 0, m_oGCharData.getName());

  add_attr(pInfo->add_attrs(), EATTRTYPE_HP, getAttr(EATTRTYPE_HP));
  add_attr(pInfo->add_attrs(), EATTRTYPE_SP, getAttr(EATTRTYPE_SP));
  add_attr(pInfo->add_attrs(), EATTRTYPE_MAXHP, getAttr(EATTRTYPE_MAXHP));
  add_attr(pInfo->add_attrs(), EATTRTYPE_MAXSP, getAttr(EATTRTYPE_MAXSP));
}

void SessionUser::toData(SocialUser* pUser)
{
  if (pUser == nullptr)
    return;
  pUser->set_accid(accid);
  pUser->set_charid(id);
  pUser->set_zoneid(thisServer->getZoneID());
  pUser->set_mapid(getMapID());
  pUser->set_baselv(getBaseLevel());
  pUser->set_profession(getProfession());
  pUser->set_name(name);
}

// client<->gatesever<->tradeserver
bool SessionUser::sendUserCmdToTradeServer(const BYTE* buf, WORD len)
{
  Cmd::SessionForwardUsercmdTrade cmd;
  cmd.set_charid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  thisServer->sendCmd(ClientType::trade_server, (BYTE*)send, len2);
  return true;
}

// client<->gatesever<->matchserver
bool SessionUser::sendUserCmdToMatchServer(const BYTE* buf, WORD len)
{
  Cmd::SessionForwardCCmdMatch cmd;
  cmd.set_charid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  return thisServer->sendCmd(ClientType::match_server, (BYTE*)send, len2);
}

bool SessionUser::sendUserCmdToAuctionServer(const BYTE* buf, WORD len)
{
  Cmd::ForwardCCmd2Auction cmd;
  cmd.set_charid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_name(name);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  return thisServer->sendCmd(ClientType::auction_server, (BYTE*)send, len2);
}

bool SessionUser::sendUserCmdToWeddingServer(const BYTE* buf, WORD len)
{
  Cmd::ForwardC2WeddingSCmd cmd;
  cmd.set_charid(id);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_name(name);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  return thisServer->sendCmd(ClientType::wedding_server, (BYTE*)send, len2);
}

// client<->gatesever<->recordserver
bool SessionUser::sendUserCmdToRecordServer(const BYTE* buf, WORD len)
{
  Cmd::ForwardUserCmdToRecordCmd cmd;
  cmd.set_charid(id);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  thisServer->sendCmdToRecord(send, len2);
  return true;
}

// client<->gatesever<->socialserver
bool SessionUser::sendUserCmdToSocialServer(const BYTE* buf, WORD len, ECmdType eType)
{
  Cmd::SessionForwardSocialCmd cmd;

  toData(cmd.mutable_user());

  cmd.set_type(eType);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  switch (eType)
  {
    case ECMDTYPE_TEAM:
    case ECMDTYPE_DOJO:
    case ECMDTYPE_TOWER:
    case ECMDTYPE_TEAMRAID:
      return thisServer->sendCmdToServer(send, len2, "TeamServer");
    case ECMDTYPE_GUILD:
      return thisServer->sendCmd(ClientType::guild_server, (BYTE*)send, len2);
    case ECMDTYPE_CHAT:
      return thisServer->sendCmd(ClientType::global_server, (BYTE*)send, len2);
    case ECMDTYPE_DOJO_GUILD:
      return thisServer->sendCmd(ClientType::guild_server, (BYTE*)send, len2);
    default:
      return thisServer->sendCmdToServer(send, len2, "SocialServer");
  }

  return false;
}

bool SessionUser::sendUserCmdToTeamServer(const BYTE* buf, WORD len, ECmdType eType)
{
  Cmd::SessionForwardSocialCmd cmd;

  toData(cmd.mutable_user());

  cmd.set_type(eType);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  return thisServer->sendCmdToServer(send, len2, "TeamServer");
}

bool SessionUser::sendUserCmdToGuildServer(const BYTE* buf, WORD len, ECmdType eType)
{
  Cmd::SessionForwardSocialCmd cmd;

  toData(cmd.mutable_user());

  cmd.set_type(eType);
  cmd.set_data(buf, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len2);

  return thisServer->sendCmd(ClientType::guild_server, send, len2);
}

void SessionUser::changeScene(Cmd::ChangeSceneSessionCmd &cmd)
{
  if (!myScene)
  {
    XERR << "[跨服切换场景]" << id << name << "没有找到玩家所在场景,dest:" << cmd.mapid() << XEND;
    return;
  }
  if (USER_STATE_RUN != getUserState())
  {
    XERR << "[跨服切换场景]" << id << name << "玩家状态异常,dest:" << cmd.mapid() << "status:" << getUserState() << XEND;
    return;
  }

  DWORD dwMapId = cmd.mapid();

  SessionScene *targetscene = NULL;
  if (cmd.has_mapid())
  {
    targetscene = SessionSceneManager::instance()->getSceneByID(cmd.mapid());
    if (targetscene == nullptr)
    {
      const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
      targetscene = SessionSceneManager::instance()->getSceneByID(rCFG.dwNewCharMapID);
      dwMapId = rCFG.dwNewCharMapID;
    }
  }

  if (targetscene && targetscene->canEnter(this))
  {
    Cmd::ChangeSceneResultSessionCmd message;
    message.set_charid(id);
    message.set_mapid(dwMapId);
    ScenePos *p = message.mutable_pos();
    p->CopyFrom(cmd.pos());
    PROTOBUF(message, send, len);
    myScene->sendCmd(send, len);

    setUserState(USER_STATE_CHANGE_SCENE);
    XLOG << "[跨服切换场景]" << accid << id << name << "通知场景注销:(" << myScene->id << myScene->name << ")->(" << cmd.mapid() << ")" << XEND;

    //同步自己位置给队友,否则跟随的队友先进入场景会乱跑
    if (m_oGTeam.getTeamID() != 0)
    {
      MemberPosUpdate posCmd;
      posCmd.set_id(id);
      ScenePos *pos = posCmd.mutable_pos();
      pos->CopyFrom(cmd.pos());
      PROTOBUF(posCmd, send1, len1);
      for (auto &m : m_oGTeam.getTeamMemberList())
      {
        SessionUser* pTarget = SessionUserManager::getMe().getUserByID(m.first);
        if (pTarget == nullptr || m.first == id)
          continue;
        pTarget->sendCmdToMe(send1, len1);
      }
    }
  }
  else
  {
    XERR << "[跨服切换场景]" << accid << id << name << "切换失败,目标场景不能进入:(" << myScene->id << myScene->name << ")->(" << cmd.mapid() << ")" << XEND;
  }
}

void SessionUser::onEnterScene()
{
  BossList::getMe().onEnterScene(this);
}

void SessionUser::setAttr(EAttrType eType, float value)
{
  auto v = find_if(m_vecAttrs.begin(), m_vecAttrs.end(), [eType](const UserAttrSvr& r) -> bool{
    return r.type() == eType;
  });
  if (v != m_vecAttrs.end())
  {
    v->set_value(value);
  }
  else
  {
    UserAttrSvr oAttr;
    oAttr.set_type(eType);
    oAttr.set_value(value);

    m_vecAttrs.push_back(oAttr);
  }
}

float SessionUser::getAttr(EAttrType eType) const
{
  auto v = find_if(m_vecAttrs.begin(), m_vecAttrs.end(), [eType](const UserAttrSvr& r) -> bool{
    return r.type() == eType;
  });
  if (v != m_vecAttrs.end())
      return v->value();

  return 0.0f;
}

void SessionUser::onDataChanged(const UserDataSync& sync)
{
  DWORD dwOldBaseLv = m_oGCharData.getBaseLevel();
  // datas
  for (int i = 0; i < sync.datas_size(); ++i)
  {
    const UserData& rData = sync.datas(i);
    switch (rData.type())
    {
      case EUSERDATATYPE_MAPID:
        m_oGCharData.setMapID(rData.value());
        break;
      case EUSERDATATYPE_RAIDID:
        m_dwRaidID = rData.value();
        break;
      case EUSERDATATYPE_SEX:
        m_oGCharData.setGender(static_cast<EGender>(rData.value()));
        break;
      case EUSERDATATYPE_JOBLEVEL:
        m_oGCharData.setJobLevel(rData.value());
        break;
      case EUSERDATATYPE_ROLELEVEL:
        m_oGCharData.setBaseLevel(rData.value());
        break;
      case EUSERDATATYPE_ONLINETIME:
        m_oGCharData.setOnlineTime(rData.value());
        break;
      case EUSERDATATYPE_PORTRAIT:
        m_oGCharData.setPortrait(rData.value());
        break;
      case EUSERDATATYPE_BODY:
        m_oGCharData.setBody(rData.value());
        break;
      case EUSERDATATYPE_HEAD:
        m_oGCharData.setHead(rData.value());
        break;
      case EUSERDATATYPE_FACE:
        m_oGCharData.setFace(rData.value());
        break;
      case EUSERDATATYPE_BACK:
        m_oGCharData.setBack(rData.value());
        break;
      case EUSERDATATYPE_TAIL:
        m_oGCharData.setTail(rData.value());
        break;
      case EUSERDATATYPE_EYE:
        m_oGCharData.setEye(rData.value());
        break;
      case EUSERDATATYPE_MOUTH:
        m_oGCharData.setMouth(rData.value());
        break;
      case EUSERDATATYPE_HAIR:
        m_oGCharData.setHair(rData.value());
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        m_oGCharData.setHairColor(rData.value());
        break;
      case EUSERDATATYPE_CLOTHCOLOR:
        m_oGCharData.setClothColor(rData.value());
        break;
      case EUSERDATATYPE_LEFTHAND:
        m_oGCharData.setLeftHand(rData.value());
        break;
      case EUSERDATATYPE_RIGHTHAND:
        m_oGCharData.setRightHand(rData.value());
        break;
      /*case EUSERDATATYPE_FRAME:
        m_stUserData.frame = rData.value();
        add_data(cmd.mutable_info()->add_datas(), rData.type(), rData.value());
        break;*/
      case EUSERDATATYPE_PROFESSION:
        m_oGCharData.setProfession(static_cast<EProfession>(rData.value()));
        break;
      case EUSERDATATYPE_FOLLOWID:
        m_qwFollowID = rData.value();
        break;
      case EUSERDATATYPE_HANDID:
        m_qwHandID = rData.value();
        break;
      case EUSERDATATYPE_CARRIER:
        m_dwCarrierID = rData.value();
        break;
      case EUSERDATATYPE_MANUAL_LV:
        m_oGCharData.setManualLv(rData.value());
        break;
      /*case EUSERDATATYPE_MANUAL_EXP:
        m_oGCharData.setManualExp(rData.value());
        add_data(cmd.mutable_info()->add_datas(), rData.type(), rData.value());
        break;*/
      case EUSERDATATYPE_CUR_TITLE:
        m_oGCharData.setTitleID(rData.value());
        break;
      case EUSERDATATYPE_CREATETIME:
        m_dwCreateTime = rData.value();
        break;
      case EUSERDATATYPE_QUERYTYPE:
        m_oGCharData.setQueryType(rData.value());
        break;
      case EUSERDATATYPE_QUERYWEDDINGTYPE:
        m_oGCharData.setQueryWeddingType(rData.value());
        break;
      case EUSERDATATYPE_BLINK:
        m_oGCharData.setBlink(rData.value());
        break;
      case EUSERDATATYPE_NAME:
        m_oGCharData.setName(rData.data().c_str());
        set_name(rData.data().c_str());
        break;
      case EUSERDATATYPE_ZENY_DEBT:
        m_qwZenyDebt = rData.value();
        break;
      case EUSERDATATYPE_MONTHCARD:
        m_bHasMonthCard = rData.value() != 0 ? true : false;
        break;
      case EUSERDATATYPE_TUTOR_ENABLE:
        m_oGCharData.setTutor(rData.value());
        break;
      default:
        XDBG << "[会话玩家-数据同步]" << accid << id << getProfession() << name << "收到数据type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << "未处理" << XEND;
        break;
    }

    XDBG << "[会话玩家-数据同步]" << accid << id << getProfession() << name << "收到数据type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << XEND;
  }

  // tutor match
  const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
  if (dwOldBaseLv < rCFG.dwTutorBaseLvReq && m_oGCharData.getBaseLevel() >= rCFG.dwTutorBaseLvReq)
  {
    const string& key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_MATCH, id);
    DWORD dwMatch = 0;
    RedisManager::getMe().getData<DWORD>(key, dwMatch);
    if (dwMatch != 0)
    {
      LeaveRoomCCmd cmd;
      cmd.set_type(EPVPTYPE_TUTOR);
      PROTOBUF(cmd, send, len);
      doMatchCmd((const BYTE*)send, len);
      XLOG << "[导师匹配-等级]" << accid << id << getProfession() << name << "baselv" << dwOldBaseLv << "->" << m_oGCharData.getBaseLevel() << "终止匹配" << XEND;
    }
  }
}

void SessionUser::fetchChangedData(const UserDataSync& sync, UserInfoSyncSocialCmd& social, UserInfoSyncSocialCmd& team, UserInfoSyncSocialCmd& guild)
{
  social.mutable_info()->mutable_user()->set_charid(sync.id());
  team.mutable_info()->mutable_user()->set_charid(sync.id());
  guild.mutable_info()->mutable_user()->set_charid(sync.id());

  for (int i = 0; i < sync.datas_size(); ++i)
  {
    const UserData& rData = sync.datas(i);
    switch (rData.type())
    {
      case EUSERDATATYPE_SEX:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_MAPID:
      case EUSERDATATYPE_RAIDID:
      case EUSERDATATYPE_GUILDRAIDINDEX:
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_ROLELEVEL:
      case EUSERDATATYPE_PORTRAIT:
      case EUSERDATATYPE_BODY:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_HEAD:
      case EUSERDATATYPE_FACE:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_BACK:
      case EUSERDATATYPE_TAIL:
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_EYE:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_MOUTH:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_HAIR:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_CLOTHCOLOR:
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      /*case EUSERDATATYPE_LEFTHAND:
      case EUSERDATATYPE_RIGHTHAND:
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;*/
      case EUSERDATATYPE_PROFESSION:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(guild.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_HANDID:
      case EUSERDATATYPE_CARRIER:
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_MANUAL_LV:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_CUR_TITLE:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_BLINK:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        add_data(team.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_NAME:
        add_data(social.mutable_info()->add_datas(), rData.type(), 0, rData.data());
        add_data(team.mutable_info()->add_datas(), rData.type(), 0, rData.data());
        add_data(guild.mutable_info()->add_datas(), rData.type(), 0, rData.data());
        break;
      case EUSERDATATYPE_TUTOR_ENABLE:
      case EUSERDATATYPE_TUTOR_PROFIC:
        add_data(social.mutable_info()->add_datas(), rData.type(), rData.value());
        break;
      case EUSERDATATYPE_ENSEMBLESKILL:
        add_data(team.mutable_info()->add_datas(), rData.type(), 0, rData.data());
        break;
      default:
        XDBG << "[会话玩家-数据收集]" << sync.id() << "数据type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << "未处理" << XEND;
        break;
    }

    XDBG << "[会话玩家-数据收集]" << sync.id() << "数据type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << XEND;
  }
}

/*void SessionUser::syncDataToTeam(const UserDataSync& cmd)
{
  MemberDataUpdate tcmd;
  tcmd.set_id(id);
  for (int i = 0; i < cmd.attrs_size(); ++i)
  {
    const UserAttr& rAttr = cmd.attrs(i);
    MemberData* pData = tcmd.add_members();
    pData->set_value(rAttr.value());

    switch (rAttr.type())
    {
      case EATTRTYPE_HP:
        pData->set_type(EMEMBERDATA_HP);
        break;
      case EATTRTYPE_MAXHP:
        pData->set_type(EMEMBERDATA_MAXHP);
        break;
      case EATTRTYPE_SP:
        pData->set_type(EMEMBERDATA_SP);
        break;
      case EATTRTYPE_MAXSP:
        pData->set_type(EMEMBERDATA_MAXSP);
        break;
      default:
        XERR << "[玩家-队伍属性]" << accid << id << getProfession() << name << "同步了属性" << rAttr.ShortDebugString() << "未处理" << XEND;
        break;
    }
    XDBG << "[玩家-队伍属性]" << accid << id << getProfession() << name << "同步了属性" << rAttr.ShortDebugString() << XEND;
  }
}*/

void SessionUser::updateUserInfo(const GuildUserInfoSyncGuildCmd& cmd)
{
  const GuildUserInfo& rOld = m_oGuild.getGuildUserInfo();

  map<DWORD, DWORD> mapPrayLv;
  for (int i = 0; i < cmd.info().prays_size(); ++i)
  {
    const GuildMemberPray& rPray = cmd.info().prays(i);
    mapPrayLv[rPray.pray()] = rPray.lv();
  }

  GuildPrayNtfGuildCmd ncmd;
  if (m_bPrayFirst)
  {
    const TMapGuildPrayCFG& mapCFG = GuildConfig::getMe().getGuildPrayList();
    for (auto &m : mapCFG)
    {
      GuildMemberPray* pPray = ncmd.add_prays();

      auto pray = mapPrayLv.find(m.first);
      if (pray != mapPrayLv.end())
      {
        pPray->set_pray(pray->first);
        pPray->set_lv(pray->second);

        m.second.toData(pPray->mutable_cur(), pray->second);
        m.second.toData(pPray->mutable_next(), pray->second + 1);
      }
      else
      {
        m.second.toData(pPray->mutable_next(), 1);
      }
    }
    m_bPrayFirst = false;
  }
  else
  {
    for (int i = 0; i < cmd.info().prays_size(); ++i)
    {
      const GuildMemberPray& rNewPray = cmd.info().prays(i);

      bool bNtf = true;
      for (int i = 0; i < rOld.prays_size(); ++i)
      {
        const GuildMemberPray& rOldPray = rOld.prays(i);
        if (rNewPray.pray() == rOldPray.pray() && rNewPray.lv() == rOldPray.lv())
        {
          bNtf = false;
          break;
        }
      }
      if (!bNtf)
        continue;

      const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(rNewPray.pray());
      if (pCFG == nullptr)
      {
        XDBG << "[玩家-公会祈祷]" << accid << id << getProfession() << name << "同步" << rNewPray.pray() << "失败,未在Table_Guild_Faith.txt 表中找到" << XEND;
        continue;
      }

      GuildMemberPray* pPray = ncmd.add_prays();
      pPray->CopyFrom(rNewPray);

      pCFG->toData(pPray->mutable_cur(), pPray->lv());
      pCFG->toData(pPray->mutable_next(), pPray->lv() + 1);
    }
  }

  if (ncmd.prays_size() > 0)
  {
    PROTOBUF(ncmd, send, len);
    sendCmdToMe(send, len);
    XDBG << "[玩家-公会祈祷]" << accid << id << getProfession() << name << "同步" << ncmd.ShortDebugString() << XEND;
  }

  m_oGuild.updateUserInfo(cmd);
}

void SessionUser::onEnterTeam(QWORD teamid)
{
  SealManager::getMe().addMember(teamid, id);
  addOneLevelIndex(ONE_LEVEL_INDEX_TYPE_TEAM, teamid);

  // use this cmd to team data update
  const TeamMemberInfo* pMember = m_oGTeam.getTeamMember(id);
  if (pMember != nullptr)
  {
    TeamMemberUpdateTeamCmd cmd;
    cmd.add_updates()->CopyFrom(*pMember);
    for (auto &m : m_oGTeam.getTeamMemberList())
    {
      SessionUser* pTarget = SessionUserManager::getMe().getUserByID(m.first);
      if (pTarget == nullptr)
        continue;
      cmd.set_charid(m.first);
      PROTOBUF(cmd, send, len);
      pTarget->sendCmdToScene(send, len);
    }
  }
}

void SessionUser::onLeaveTeam(QWORD teamid)
{
  SealManager::getMe().removeMember(teamid, id);
  delOneLevelIndex(ONE_LEVEL_INDEX_TYPE_TEAM, teamid);
}

EOperateState SessionUser::queryOperateState(EOperateType type,SOperateRewardCFG** pCfg)
{
  DWORD curSec = now();

  DWORD expireTime = MiscConfig::getMe().getOperateRewardMiscCFG().expireTime;
  if ((type == EOperateType_Charge ||   //充返
    type == EOperateType_Autumn ||      //求测枫叶
    type == EOperateType_Summer ||      //夏测
    type == EOperateType_CodeBW ||      //百万集结
    type == EOperateType_CodeMX)        //冒险集结补给包
    && curSec > expireTime)
    return EOperateState_Expire;

  if (checkOperateReward(type) == true)
    return EOperateState_Toke;
  if (type == EOperateType_RedBag) //每个人都可以领取红包
    return EOperateState_CanTake;

  //手机号绑定
  if (type == EOperateType_Phone)
  {
    if (m_strPhone.empty())
      return EOperateState_None;
    else 
      return EOperateState_CanTake;
  }
  
  if (type == EOperateType_Summer || type == EOperateType_Autumn || type == EOperateType_Charge)
  {
    xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "plat_account");
    if (field == nullptr)
      return EOperateState_None;

    char where[32] = { 0 };
    snprintf(where, sizeof(where), "accid=%llu", accid);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
    if (ret != 1)
    {
      return EOperateState_None;
    }
    string uid = set[0].getString("uid");
    *pCfg = OperateRewardConfig::getMe().getOperateRewardCFG(uid, type);
    if (*pCfg == nullptr)
      return EOperateState_None;
  }

  auto checkCode = [&](EOperateType type) {
    DWORD batchId = 0;
    if (type == EOperateType_CodeBW)
      batchId = 80001;
    else if (type == EOperateType_CodeMX)
      batchId = 65005;
    else
      return false;

    char where[32] = { 0 };
    snprintf(where, sizeof(where), "accid=%llu and batchid=%u", accid, batchId);

    QWORD ret = thisServer->getDBConnPool().checkExist(RO_DATABASE_NAME, "gift_code_use", where);
    XLOG << "[运营-活动-code] 查询状态" << id <<accid << "ret"<<ret << XEND;
    if (ret == QWORD_MAX)
    {
      return false;
    }
    if (ret > 0)
      return false;
    return true;
  };
  
  if (type == EOperateType_CodeBW || type == EOperateType_CodeMX)
  {//礼包码
    if (checkCode(type) == false)
      return EOperateState_Toke;
  }
  return EOperateState_CanTake;
}

void SessionUser::queryOperateState(OperateQuerySocialCmd& rev)
{
  if(rev.type() == EOperateType_MonthCard)
  {
    DWORD activityid = MiscConfig::getMe().getMonthCardActivityCFG().dwID;
    bool isOpen = SessionActivityMgr::getMe().isOpen(activityid);
    if(isOpen)
    {
      PROTOBUF(rev, send, len);
      sendCmdToSceneUser(send, len);
    }
    else
    {
      rev.set_state(EOperateState_None);
      PROTOBUF(rev, send, len);
      sendCmdToMe(send, len);
      XLOG << "[运营-活动] 查询状态" << id << "ret" << rev.ShortDebugString() << XEND;
    }
    return;
  }

  SOperateRewardCFG* pCfg = nullptr;
  EOperateState state = queryOperateState(rev.type(), &pCfg);
  rev.set_state(state);
  if (state == EOperateState_CanTake && pCfg &&  (pCfg->dwZeny || pCfg->dwMonthcardCount))
  {
    rev.set_param1(pCfg->dwZeny);
    rev.set_param2(MiscConfig::getMe().getOperateRewardMiscCFG().expireTime);
    rev.set_param3(pCfg->dwMonthcardCount);
  }
  PROTOBUF(rev, send, len);
  sendCmdToMe(send, len);
  XLOG << "[运营-活动] 查询状态" << id << "ret" << rev.ShortDebugString() << XEND;
}

void SessionUser::takeOperateReward(OperateTakeSocialCmd& rev)
{
  if(rev.type() == EOperateType_MonthCard)
  {
    DWORD activityid = MiscConfig::getMe().getMonthCardActivityCFG().dwID;
    bool isOpen = SessionActivityMgr::getMe().isOpen(activityid);
    if(isOpen)
    {
      PROTOBUF(rev, send, len);
      sendCmdToSceneUser(send, len);
    }
    else
    {
      rev.set_state(EOperateState_None);
      PROTOBUF(rev, send, len);
      sendCmdToMe(send, len);
      XERR << "[运营-活动] 领取奖励 失败，活动未开启 " << rev.type() << name << accid << id << XEND;
    }
    return;
  }
  SOperateRewardCFG* pCfg = nullptr;
  EOperateState state = queryOperateState(rev.type(), &pCfg);
  rev.set_state(state);
  if (state == EOperateState_CanTake)
  {
    //take
    TVecItemInfo vecItemInfo;
    
    if (rev.type() == EOperateType_CodeBW || rev.type() == EOperateType_CodeMX)
    {
      ItemInfo info;
      if (rev.type() == EOperateType_CodeBW)
        info.set_id(3651);
      else if (rev.type() == EOperateType_CodeMX)
        info.set_id(3652);
      info.set_count(1);
      info.set_source(ESOURCE_OPERATE);
      vecItemInfo.push_back(info);
    }
    else if (rev.type() == EOperateType_RedBag)
    {
      ItemInfo info1;
      info1.set_id(3635);
      info1.set_count(1);
      info1.set_source(ESOURCE_OPERATE);
      vecItemInfo.push_back(info1);

      ItemInfo info2;
      info2.set_id(500510);
      info2.set_count(1);
      info2.set_source(ESOURCE_OPERATE);
      vecItemInfo.push_back(info2);
    }
    else if (rev.type() == EOperateType_Phone)
    {
      DWORD rewardId = 1012;
      if (!RewardManager::roll(rewardId, this, vecItemInfo, ESOURCE_OPERATE))
      {
        XERR << "[运营-活动] 领取奖励 失败，roll 失败" << rev.type() << accid << id << rev.type() << "rewardid" << rewardId << XEND;
      }
      else
      {
        XINF << "[运营-活动] 成功领取reward奖励" << rev.type() << accid << id << rev.type() << "rewardid" << rewardId << XEND;
      }
    }
    else
    {
      if (pCfg)
      {
        if (pCfg->dwRewardId)
        {
          if (!RewardManager::roll(pCfg->dwRewardId, this, vecItemInfo, ESOURCE_OPERATE))
          {
            XERR << "[运营-活动] 领取奖励 失败，roll 失败" << rev.type() << accid << id << rev.type() << "rewardid" << pCfg->dwRewardId << XEND;
          }
          else
          {
            XINF << "[运营-活动] 成功领取reward奖励" << rev.type() << accid << id << rev.type() << "rewardid" << pCfg->dwRewardId << XEND;
          }
        }

        if (pCfg->dwZeny)
        {
          ItemInfo info;
          info.set_id(100);
          info.set_count(pCfg->dwZeny);
          info.set_source(ESOURCE_OPERATE);
          vecItemInfo.push_back(info);
          XINF << "[运营-活动] 成功领取 zeny 奖励" << rev.type() << accid << id << rev.type() << "zeny" << pCfg->dwZeny << XEND;
        }
        if (pCfg->dwMonthcardCount)
        {
          ItemInfo info;
          info.set_id(800101);
          info.set_count(pCfg->dwMonthcardCount);
          info.set_source(ESOURCE_OPERATE);
          vecItemInfo.push_back(info);
          XINF << "[运营-活动] 成功领取 月卡 奖励" << rev.type() << accid << id << rev.type() << "monthcard id" << 800101 << pCfg->dwMonthcardCount << XEND;
        }
      }
    }

    UserAddItemSocialCmd cmd;
    cmd.mutable_user()->set_charid(id);
    cmd.set_operatereward(rev.type());
    for (auto &v : vecItemInfo)
    {
      cmd.add_items()->CopyFrom(v);
    }
    PROTOBUF(cmd, send, len);
    sendCmdToScene(send, len);
  }
  PROTOBUF(rev, send, len);
  sendCmdToMe(send, len);
  XLOG << "[运营-活动] 领取" << id << "ret" << rev.ShortDebugString() << XEND;
}

void SessionUser::setSafeDevice(bool safeDevice)
{
  m_bSafeDevice = safeDevice;
  XLOG << "[安全设备-设置] charid" << id << "m_bSafeDevice" << m_bSafeDevice << XEND;
}

/*bool SessionUser::useGiftCode(std::string code)
{
}*/

void SessionUser::sendChargeCnt()
{
  sendVersionVard2Client(); // 同步ep特典配置

  DWORD curSec = now() - 5 * 3600;    //5点才算
  DWORD ym = xTime::getYear(curSec) * 100 + xTime::getMonth(curSec);
  if (ym != m_dwChargeYM)
    m_mapChargeCnt.clear();
  
  if (m_mapChargeCnt.empty())
  {
    m_dwChargeYM = ym;
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charge_accid");
    if (field == nullptr)
      return ;

    char where[512];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "accid=%lld and ym=%d", accid, m_dwChargeYM);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
    if (QWORD_MAX == ret)
    {
      return;
    }
    else if (1 == ret)
    {
      auto f = [&](string columnName, DWORD dwDataId)
      {
        if (!field->has(columnName))
          return;
        m_mapChargeCnt[dwDataId] = set[0].get<DWORD>(columnName);
      };      
      DepositConfig::getMe().loadDb(f);
    }
    
    //load ep特典
    {
      xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charge_epcard");
      if (field == nullptr)
        return;

      char where[512];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "accid=%lld", accid);
      xRecordSet set;
      QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
      if (QWORD_MAX == ret)
      {
        return;
      }
      for (DWORD i = 0; i < ret; i++)
      {
        m_mapChargeCnt[set[i].get<DWORD>("dataid")] = set[i].get<DWORD>("epcard");
      }
    }
  }

  updateDepositLimit();
  QueryChargeCnt cmd;
  TSetDWORD except;
  for (auto &m : m_mapChargeCnt)
  {
    ChargeCntInfo* pInfo = cmd.add_info();
    if (pInfo)
    {
      DWORD fromId;
      DWORD toId;
      //数据库里记录的是fromid，发给前端的时toid
      if (MiscConfig::getMe().getSDepositMiscCFG().getDiscountPair(m.first, fromId, toId))
      {
        pInfo->set_dataid(toId);
      }
      else
        pInfo->set_dataid(m.first);
      pInfo->set_count(m.second);

      auto it = m_mapDepositLimit.find(pInfo->dataid());
      if (it != m_mapDepositLimit.end())
      {
        except.insert(it->first);
        pInfo->set_limit(it->second);
      }
    }
  }
  for (auto& v : m_mapDepositLimit)
  {
    if (except.find(v.first) != except.end())
      continue;
    ChargeCntInfo* pInfo = cmd.add_info();
    if (pInfo)
    {
      pInfo->set_dataid(v.first);
      pInfo->set_count(0);
      pInfo->set_limit(v.second);
    }
  }
  
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  XLOG << "[充值-次数查询] charid"<<id <<"accid"<<accid <<"ym"<<m_dwChargeYM <<"msg"<<cmd.ShortDebugString() << XEND;
  m_mapChargeCnt.clear();
}

//DWORD SessionUser::getChargeCnt(DWORD id, DWORD key /*=0*/)
//{
//  auto it = m_mapChargeCnt.find(id);
//  if (it == m_mapChargeCnt.end())
//    return 0;
//  if (it->second.first != key)
//  {
//    m_mapChargeCnt.erase(id);
//    return 0;
//  }
//  return it->second.second;
//}
//
//void SessionUser::addChargeCnt(DWORD id, DWORD key /*=0*/)
//{
//  DWORD count = getChargeCnt(id, key);
//  count++;
//  std::pair<DWORD, DWORD>& rPr = m_mapChargeCnt[id];
//  rPr.first = key;
//  rPr.second = count;
//}

bool SessionUser::isInPollyScene()
{
  XDBG << "[波利乱斗-功能屏蔽] charid" << id << getMapID() << XEND;

  if (9004 == getMapID())
    return true;  
  return false;
}

void SessionUser::processLoveLetter(const LoveLetterUse& cmd)
{
  QWORD targetid = cmd.targets();
  SessionUser* pTarget = SessionUserManager::getMe().getUserByID(targetid);
  if (pTarget == nullptr)
    return;

  LoveLetterSessionCmd scmd;
  scmd.set_charid(id);
  scmd.set_itemguid(cmd.itemguid());
  scmd.set_targets(cmd.targets());
  scmd.set_content(cmd.content());
  scmd.set_type(cmd.type());

  PROTOBUF(scmd, send, len);
  getScene()->sendCmd(send, len);
}

void SessionUser::sendActivityCnt()
{ 
  QueryActivityCnt cmd;
  
  SessionActivityMgr::getMe().getUserAllActivityCnt(accid, id, m_mapActivityCnt);
  if (m_mapActivityCnt.empty())
    return;

  for (auto&m : m_mapActivityCnt)
  {
    ActivityCntItem*pItem = cmd.add_info();
    if (!pItem)
      continue;
    pItem->set_activityid(m.first);
    pItem->set_count(m.second);
  }

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  XLOG << "[全局活动-次数查询] charid" << id << "accid" << accid << "msg" << cmd.ShortDebugString() << XEND;
}

DWORD SessionUser::getActivityCnt(DWORD dwActivityId)
{
  auto it = m_mapActivityCnt.find(dwActivityId);
  if (it == m_mapActivityCnt.end())
    return 0;
  return it->second;
}

void SessionUser::addActivityCnt(DWORD dwActivityId)
{
  m_mapActivityCnt[dwActivityId]++;
  DWORD count = m_mapActivityCnt[dwActivityId];
  //ntf client
  UpdateActivityCnt ntf;
  ActivityCntItem*pItem = ntf.mutable_info();
  if (pItem)
  {
    pItem->set_activityid(dwActivityId);
    pItem->set_count(count);
  }
  PROTOBUF(ntf, send, len);
  sendCmdToMe(send, len);
  XLOG << "[全局活动-次数更新] 发送给客户端，charid" << id << "accid" << accid << dwActivityId <<count << XEND;
}

// 查询一个未绑定的礼包码, 将其和玩家角色绑定
void SessionUser::useItemCode(UseItemCodeSessionCmd& cmd)
{
  if (!getScene())
    return;

  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "item_code");
  if (!pField)
    return;

  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "state=0 and `type`=%d", cmd.type());
  char extra[64];
  bzero(extra, sizeof(extra));
  snprintf(extra, sizeof(extra), "limit 1");

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, where, extra);
  if (1 != ret)
  {
    XERR << "[道具-礼包码] 查询数据库出错,或者礼包码不足了 ret:" << ret <<"charid" <<accid << id <<"guid"<<cmd.guid()<<"itemid"<<cmd.itemid() << XEND;
    return;
  }
  
  cmd.set_code(set[0].getString("code"));

  //update
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "code=\"%s\" and state=0 and `type`=%d", cmd.code().c_str(), cmd.type());
  xRecord record(pField);
  record.put("state", 1);
  record.put("charid", cmd.charid());
  record.putString("guid", cmd.guid());
  record.put("itemid", cmd.itemid());
  record.put("usetime", now());
  ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (1 != ret)
  {
    XERR << "[道具-礼包码] 更新礼包码错误 ret:" << ret << "charid" << accid << id << "guid" << cmd.guid() << "itemid" << cmd.itemid() << XEND;
    return;
  }
  
  PROTOBUF(cmd, send, len);
  getScene()->sendCmd(send, len);
  XLOG << "[道具-礼包码] 成功获得礼包码:" << "charid" << accid << id << "guid" << cmd.guid() << "itemid" << cmd.itemid() <<"code"<<cmd.code() << XEND;
}

void SessionUser::getUsedItemCode(ReqUsedItemCodeSessionCmd& cmd)
{
  if (!getScene())
    return;

  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "item_code");
  if (!pField)
    return;

  char where[64];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "state=1 and charid=%llu and `type`=%d", id, cmd.type());

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, where, NULL);
  if (QWORD_MAX == ret)
  {
    XERR << "[道具-礼包码] 查询数据库出错 ret:" << ret << "charid" << accid << id << XEND;
    return;
  }

  if (set.empty())
    return;

  for (DWORD i = 0; i < set.size(); ++i)
  {
    cmd.add_guid(set[i].getString("guid"));
  }
  
  PROTOBUF(cmd, send, len);
  getScene()->sendCmd(send, len);
  XLOG << "[道具-礼包码] 通知scene更新兑换记录 " << "charid" << accid << id << "size" << set.size() << XEND;
}

void SessionUser::sendVersionVard2Client()
{
  NtfVersionCardInfo cmd; 
  for (auto&v : MiscConfig::getMe().getSDepositMiscCFG().m_vecDiscountVersionCard)
  {
    const SDeposit* pCfg = DepositConfig::getMe().getSDeposit(v.first);
    if (!pCfg)
      continue;
    VersionCardInfo* pInfo = cmd.add_info();
    if (!pInfo)
      continue;
    pInfo->set_version(pCfg->cardVersion);
    pInfo->set_id1(v.first);
    pInfo->set_id2(v.second);
  } 
  
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
  XDBG << "[充值-ep特典折扣] 推送给前端"<<id <<name <<"msg"<<cmd.ShortDebugString() << XEND;
}

bool SessionUser::sendWeddingInfo2Scene()
{
  SyncWeddingInfoSCmd cmd;
  cmd.set_charid(id);
  cmd.mutable_weddinginfo()->CopyFrom(m_oWeddingInfo);
  PROTOBUF(cmd, send, len);
  sendCmdToScene(send, len); 
  XDBG <<"[婚礼-同步到场景]" <<this->id <<this->name <<"msg"<<cmd.ShortDebugString() << XEND;
  m_oMail.processWeddingMsg();
  return true;
}
void SessionUser::setWeddingInfo(const Cmd::WeddingInfo& info)
{
  m_oWeddingInfo.CopyFrom(info);
  if (m_oWeddingInfo.status() == EWeddingStatus_Married)
  {
    if (info.manual().name1() == name)
      m_oGCharData.setWeddingPartner(info.manual().name2());
    else
      m_oGCharData.setWeddingPartner(info.manual().name1());
  }
  else
  {
    m_oGCharData.setWeddingPartner("");
  }
}

//获取订婚对象的charid
QWORD SessionUser::getReserveParnter()
{
  if (m_oWeddingInfo.status() != EWeddingStatus_Reserve)
    return 0;
  return m_oWeddingInfo.charid1() == id ? m_oWeddingInfo.charid2() : m_oWeddingInfo.charid1();
}

//获取结婚对象的charid
QWORD SessionUser::getWeddingParnter()
{
  if (m_oWeddingInfo.status() != EWeddingStatus_Married)
    return 0;
  return m_oWeddingInfo.charid1() == id ? m_oWeddingInfo.charid2() : m_oWeddingInfo.charid1();
}

void SessionUser::updateParnterName(const string& name)
{
  if (m_oWeddingInfo.charid1() == id)
    m_oWeddingInfo.mutable_manual()->set_name2(name);
  else if (m_oWeddingInfo.charid2() == id)
    m_oWeddingInfo.mutable_manual()->set_name1(name);
  else
    return;

  m_oGCharData.setWeddingPartner(name);
  XLOG << "[婚礼-更名]" << accid << id << getProfession() << name << "更新配偶姓名" << name << "成功" << XEND;
}

bool SessionUser::checkOperateReward(EOperateType type)
{
  DWORD value = 1 << type;
  return m_dwOperateReward & value;
}

bool SessionUser::setDepositCount(DWORD depositid, DWORD count)
{
  const SDeposit* cfg = DepositConfig::getMe().getSDeposit(depositid);
  if (cfg == nullptr)
  {
    XERR << "[充值-设置充值次数]" << accid << id << name << "DepositID:" << depositid << "count:" << count << "配置找不到" << XEND;
    return false;
  }

  DWORD curSec = now() - 5 * 3600, ym = xTime::getYear(curSec) * 100 + xTime::getMonth(curSec);
  if (GMCommandManager::getMe().updateBuyLimitDb(accid, id, cfg, ym, count) == false)
  {
    XERR << "[充值-设置充值次数]" << accid << id << name << "DepositID:" << depositid << "count:" << count << "设置失败" << XEND;
    return false;
  }

  XLOG << "[充值-设置充值次数]" << accid << id << name << "DepositID:" << depositid << "count:" << count << "设置成功" << XEND;
  return true;
}

bool SessionUser::resetDepositVirgin(DWORD depositid, DWORD tag)
{
  if (tag == 0)
  {
    const SDeposit* cfg = DepositConfig::getMe().getSDeposit(depositid);
    if (cfg == nullptr)
    {
      XERR << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "配置找不到" << XEND;
      return false;
    }
    if (cfg->virginTag == 0)
    {
      XERR << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "virginTag为0" << XEND;
      return false;
    }
    tag = cfg->virginTag;
  }

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "charge_virgin");
  if (field == nullptr)
  {
    XERR << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "数据表找不到" << XEND;
    return false;
  }

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "accid=%llu", accid);
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "查询出错" << XEND;
    return false;
  }
  else if (ret == 0)
  {
    XLOG << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "状态已为空" << XEND;
    return true;
  }

  TVecDWORD tags;
  numTok(set[0].getString("tag"), ",", tags);
  auto it = std::find(tags.begin(), tags.end(), tag);
  if(it == tags.end())
  {
    XLOG << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "已被重置" << XEND;
    return true;
  }
  tags.erase(it);

  stringstream ss;
  ss.str("");
  for(auto tagit = tags.begin(); tagit != tags.end(); ++tagit)
  {
    if (tagit != tags.begin()) ss << ",";
    ss << *tagit;
  }

  xRecord record(field);
  record.putString("tag", ss.str());

  ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if(ret == QWORD_MAX || ret == 0)
  {
    XERR << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "ret:" << ret << "重置失败" << XEND;
    return false;
  }

  XLOG << "[充值-重置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "重置成功" << XEND;
  return true;
}

bool SessionUser::setDepositLimit(DWORD depositid, DWORD count)
{
  const SDeposit* cfg = DepositConfig::getMe().getSDeposit(depositid);
  if (cfg == nullptr)
  {
    XERR << "[充值-设置充值上限]" << accid << id << name << "DepositID:" << depositid << "上限:" << count << "配置找不到" << XEND;
    return false;
  }

  if (getDepositLimit(depositid) == count)
  {
    XLOG << "[充值-设置充值上限]" << accid << id << name << "DepositID:" << depositid << "上限:" << count << "已设置" << XEND;
    return true;
  }

  DWORD curSec = now() - 5 * 3600, year = xTime::getYear(curSec), month = xTime::getMonth(curSec);
  DWORD expire = xTime::getDayStart(year + month / 12,  month % 12  + 1, 1) - curSec;

  xLuaData data;
  stringstream ss;
  for (auto& v : m_mapDepositLimit)
  {
    ss.str("");
    ss << v.first;
    data.setData(ss.str(), v.second, true);
  }
  ss.str("");
  ss << depositid;
  data.setData(ss.str(), count, true);

  const string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_DEPOSIT_LIMIT, id, m_dwDepositLimitYM);
  if (RedisManager::getMe().setHash(key, data) == false)
  {
    XERR << "[充值-设置充值上限]" << accid << id << name << "DepositID:" << depositid << "上限:" << count << "key:" << key << "redis设置失败" << XEND;
    return false;
  }
  m_mapDepositLimit[depositid] = count;

  if (expire && RedisManager::getMe().setExpire(key, expire) == false)
    XERR << "[充值-设置充值上限]" << accid << id << name << "key:" << key << "expire:" << expire << "redis设置过期时间失败" << XEND;

  XLOG << "[充值-设置充值上限]" << accid << id << name << "DepositID:" << depositid << "上限:" << count << "年月:" << m_dwDepositLimitYM << "设置成功" << XEND;
  return true;
}

DWORD SessionUser::getDepositLimit(DWORD depositid)
{
  updateDepositLimit();
  auto it = m_mapDepositLimit.find(depositid);
  if (it == m_mapDepositLimit.end())
    return 0;
  return it->second;
}

// 更新缓存的数据, 清除过期数据
void SessionUser::updateDepositLimit()
{
  DWORD curSec = now() - 5 * 3600;
  DWORD ym = xTime::getYear(curSec) * 100 + xTime::getMonth(curSec);

  if (m_dwDepositLimitYM == 0)
  {
    xLuaData data;
    if (GMCommandManager::getMe().getDepositLimit(id, ym, data) == false)
    {
      XERR << "[充值-更新充值上限]" << accid << id << "年月:" << ym << "查询redis失败" << XEND;
      return;
    }
    data.foreach([&](const string& k, xLuaData& d) {
        m_mapDepositLimit[atoi(k.c_str())] = d.getInt();
      });
    m_dwDepositLimitYM = ym;
  }

  if (m_dwDepositLimitYM == ym)
    return;

  m_mapDepositLimit.clear();
  return;
}

bool SessionUser::setDepositVirgin(DWORD depositid, DWORD tag)
{
  if (tag == 0)
  {
    const SDeposit* cfg = DepositConfig::getMe().getSDeposit(depositid);
    if (cfg == nullptr)
    {
      XERR << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "配置找不到" << XEND;
      return false;
    }
    if (cfg->virginTag == 0)
    {
      XERR << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "virginTag为0" << XEND;
      return false;
    }
    tag = cfg->virginTag;
  }

  xField* field = thisServer->getDBConnPool().getField(REGION_DB, "charge_virgin");
  if (field == nullptr)
  {
    XERR << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "数据表找不到" << XEND;
    return false;
  }

  char where[64] = { 0 };
  snprintf(where, sizeof(where), "accid=%llu", accid);
  xRecordSet set;
  TVecDWORD tags;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (ret == QWORD_MAX)
  {
    XERR << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "查询出错" << XEND;
    return false;
  }
  else if (ret == 0)
  {
    tags.push_back(tag);
  }
  else
  {
    numTok(set[0].getString("tag"), ",", tags);
    auto it = std::find(tags.begin(), tags.end(), tag);
    if(it != tags.end())
    {
      XLOG << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "已被设置" << XEND;
      return true;
    }
    tags.push_back(tag);
  }

  stringstream ss;
  ss.str("");
  for(auto tagit = tags.begin(); tagit != tags.end(); ++tagit)
  {
    if (tagit != tags.begin()) ss << ",";
    ss << *tagit;
  }

  xRecord record(field);
  record.put("accid", accid);
  record.putString("tag", ss.str());

  ret = thisServer->getDBConnPool().exeInsert(record, false, true);
  if(ret == QWORD_MAX || ret == 0)
  {
    XERR << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "ret:" << ret << "设置失败" << XEND;
    return false;
  }

  XLOG << "[充值-设置首充]" << accid << id << name << "DepositID:" << depositid << "tag:" << tag << "设置成功" << XEND;
  return true;
}

void SessionUser::updateVar(const SyncUserVarSessionCmd& cmd)
{
  for (int i = 0; i < cmd.vars_size(); ++i)
    m_stSessionUserData.m_mapVarValues[cmd.vars(i).type()] = cmd.vars(i).value();
}

DWORD SessionUser::getVarValue(EVarType eType) const
{
  auto it = m_stSessionUserData.m_mapVarValues.find(eType);
  return it != m_stSessionUserData.m_mapVarValues.end() ? it->second : 0;
}

void SessionUser::setMatchScore(const SyncUserScoreMatchSCmd& cmd)
{
  if (cmd.etype() == EPVPTYPE_TEAMPWS)
  {
    m_stSessionUserData.dwTeamPwsScore = cmd.score();
  }
}

DWORD SessionUser::getMatchScore(EPvpType eType) const
{
  if (eType == EPVPTYPE_TEAMPWS)
    return m_stSessionUserData.dwTeamPwsScore;
  return 0;
}


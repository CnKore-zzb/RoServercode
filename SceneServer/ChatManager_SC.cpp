#include "ChatManager_SC.h"
#include "SceneUser.h"
#include "MsgManager.h"
#include "RedisManager.h"
#include "GuidManager.h"
#include "StatisticsDefine.h"
#include "PlatLogManager.h"
#include "SceneUserManager.h"
#include "LuaManager.h"

ChatManager_SC::ChatManager_SC()
{

}

ChatManager_SC::~ChatManager_SC()
{

}

void ChatManager_SC::onLeaveScene(SceneUser* pUser)
{
  BarrageChatCmd cmd;
  cmd.set_opt(EBARRAGE_CLOSE);
  setBarrageUser(pUser, cmd);
}

bool ChatManager_SC::queryVoiceData(SceneUser* pUser, DWORD dwVoiceID) const
{
  if (pUser == nullptr)
    return false;

  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_CHAT_VOICE, dwVoiceID);
  QueryVoiceUserCmd cmd;
  xTime frameTimer;
  if (RedisManager::getMe().getProtoData(key, &cmd) == false)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_ITEMSHOW_OVERTIME);
    return false;
  }
  XDBG << "[聊天管理-语音]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "加载语音 id : " << dwVoiceID << "size :" << cmd.voice().size() << "耗时" << frameTimer.uElapse() << "微秒" << XEND;

  const string& data = cmd.voice();

  QueryVoiceUserCmd retcmd;
  retcmd.set_voiceid(cmd.voiceid());
  retcmd.set_msgover(false);
  retcmd.set_msgid(dwVoiceID);

  const int MAX_BUFFER = 20000;
  size_t t = data.size();
  DWORD dwIndex = 0;
  while (t > 0)
  {
    if (t > MAX_BUFFER)
    {
      cmd.set_voice(data.c_str() + dwIndex * MAX_BUFFER, MAX_BUFFER);
      t -= MAX_BUFFER;
    }
    else
    {
      cmd.set_voice(data.c_str() + dwIndex * MAX_BUFFER, t);
      cmd.set_msgover(true);
      t = 0;
    }
    XDBG << "[聊天管理-语音]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "index :" << dwIndex << "size :" << cmd.voice().size() << XEND;

    ++dwIndex;

    PROTOBUF(cmd, send, len2);
    pUser->sendCmdToMe(send, len2);
    cmd.clear_voice();
  }

  return true;
}

bool ChatManager_SC::processChatCmd(SceneUser* pUser, const ChatCmd& cmd)
{
  if (pUser == nullptr)
  {
    XERR << "[聊天管理-协议处理] 未发现玩家" << XEND;
    return false;
  }

  //check config
  bool ignoreGagTime = checkIgnoreGagTime(cmd.channel());
  // check gag
  DWORD dwNow = xTime::getCurSec();
  if (dwNow < pUser->getUserSceneData().getGagTime() && !ignoreGagTime)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_USER_GAG, MsgParams((pUser->getUserSceneData().getGagTime() - dwNow) / MIN_T));
    XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "处于禁言状态" << XEND;
    return false;
  }

  const SCreditCFG& creCFG = MiscConfig::getMe().getCreditCFG();
  // check bad string , affect user's credit
  if (creCFG.setChannels.find(cmd.channel()) != creCFG.setChannels.end())
  {
    if (creCFG.hasBadStrs(cmd.str()))
    {
      DWORD badvalue = creCFG.dwFirstBadValue;
      if (pUser->getLastBadTime() + creCFG.dwBadInterval > dwNow)
        badvalue = creCFG.dwSecondBadValue + pUser->getLastBadValue();

      pUser->setBadChatValue(badvalue);
      pUser->setBadChatTime(dwNow);
      pUser->getUserSceneData().decCredit(badvalue);
      pUser->getUserSceneData().setCreditSavedTime(0);
      XLOG << "[玩家-聊天], 聊天内容含有非法字符, 扣除信用度, 玩家" << pUser->name << pUser->id << "内容:" << cmd.str() << XEND;
    }
  }

  /*const std::pair<DWORD, string>& lastmsg = pUser->getLastChat();
    if (lastmsg.first + creCFG.dwRepeatInterval > dwNow)
    {
    if (lastmsg.second == cmd.str())
    {
    pUser->getUserSceneData().decCredit(creCFG.dwRepeatValue);
    pUser->getUserSceneData().setCreditSavedTime(0);
    XLOG << "[玩家-聊天], 发送重复聊天内容, 扣除信用度, 玩家" << pUser->name << pUser->id << "内容:" << cmd.str() << XEND;
    }
    }
    */

  int repeatMinus = LuaManager::getMe().call<int> ("CheckInvalidChat", (DWORD)cmd.channel(), cmd.str(), pUser);
  if (repeatMinus < 0)
  {
    pUser->getUserSceneData().decCredit(-repeatMinus);
    pUser->getUserSceneData().setCreditSavedTime(0);
    XLOG << "[玩家-聊天], 发送重复聊天内容, 扣除信用度:" << -repeatMinus << "玩家" << pUser->name << pUser->id << "内容:" << cmd.str() << XEND;
  }

  pUser->setLastChat(dwNow, cmd.str());

  // pre check
  switch (cmd.channel())
  {
    case ECHAT_CHANNEL_FRIEND:
      {
        if (pUser->getSocial().checkRelation(cmd.desid(), ESOCIALRELATION_BLACK) == true || pUser->getSocial().checkRelation(cmd.desid(), ESOCIALRELATION_BLACK_FOREVER) == true)
        {
          MsgManager::sendMsg(pUser->id, 462);
          XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送私聊失败, 目标玩家" << cmd.desid() << "在黑名单中" << XEND;
          return false;
        }
        /*
           if (pUser->checkRelation(ESOCIALRELATION_CHAT, cmd.desid()) == false)
           {
           XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送私聊失败, 与目标玩家" << cmd.desid() << "无聊天关系" << XEND;
           return false;
           }
           */
      }
      break;
    case ECHAT_CHANNEL_TEAM:
      if (pUser->getTeamID() == 0)
      {
        XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送组队聊天失败,当前没有队伍" << XEND;
        return false;
      }
      break;
    case ECHAT_CHANNEL_ROOM:
      if (pUser->hasChatRoom() == false)
      {
        XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送聊天室聊天失败,当前没有聊天室" << XEND;
        return false;
      }
      break;
    case ECHAT_CHANNEL_WORLD:
      if (MiscConfig::getMe().getSystemCFG().dwChatWorldReqLv > pUser->getLevel())
        return false;
      break;
    default:
      break;
  }

  // check cache
  TMapChatCache& mapCache = m_mapUserChatCache[pUser->id];
  if (cmd.msgid() != 0)
  {
    auto m = mapCache.find(cmd.msgid());
    if (m == mapCache.end())
    {
      XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "语音缓存失败 msgid :" << cmd.msgid() << "不合法" << XEND;
      return false;
    }

    m->second.append(cmd.voice().c_str(), cmd.voice().size());
    XDBG << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "语音缓存成功 msgid :" << cmd.msgid() << "size :" << cmd.voice().size() << XEND;

    if (cmd.msgover() == false)
      return true;

    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_CHAT_VOICE, cmd.msgid());

    QueryVoiceUserCmd voicecmd;
    voicecmd.set_voiceid(cmd.msgid());
    voicecmd.set_voice(m->second);
    mapCache.erase(m);

    xTime frameTimer;
    RedisManager::getMe().setProtoData(key, &voicecmd, HOUR_T);
    XDBG << "[聊天管理-语音]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "存储语音 id :" << cmd.msgid() << "size :" << voicecmd.voice().size() << "耗时" << frameTimer.uElapse() << "微秒" << XEND;
  }

  // init ret cmd
  ChatSocialCmd ret;
  ret.set_accid(pUser->accid);
  ret.set_platformid(pUser->getUserSceneData().getPlatformId());
  ret.set_to_global(true);
  //ret.set_voice(cmd.voice());
  //ret.set_msgid(cmd.msgid());
  //ret.set_msgover(cmd.msgover());

  if(!chatRetFill(pUser, *(ret.mutable_ret()), cmd))
    return false;

  bool bPunish = creCFG.bPunish && pUser->getUserSceneData().getCredit() <= 0 && !ignoreGagTime;

  // process chat
  switch (cmd.channel())
  {
    case ECHAT_CHANNEL_ROUND:
      {
        bool needReturn;
        bool returnval = roundChat(pUser, ret, false, needReturn, bPunish);
        if(needReturn)
          return returnval;
      }
      break;
    case ECHAT_CHANNEL_TEAM:
    case ECHAT_CHANNEL_GUILD:
      {
        PROTOBUF(ret, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      break;
    case ECHAT_CHANNEL_MAP:
      {
        if (pUser->getScene() == nullptr)
          return false;

        PROTOBUF(ret.ret(), send, len);
        MsgManager::sendMapCmd(pUser->getScene()->getMapID(), send, len);
      }
      break;
    case ECHAT_CHANNEL_ROOM:
      {
        PROTOBUF(ret.ret(), send, len);
        pUser->getChatRoom()->sendCmdToMember(send, len);
      }
      break;
    case ECHAT_CHANNEL_FRIEND:
      {
        PROTOBUF(ret.ret(), ssend, slen);
        pUser->sendCmdToMe(ssend, slen);
        pUser->getUserChat().onChat(ret.ret());

        if (bPunish)
        {
          XLOG << "[聊天管理], 玩家当前信用度小于0, 不能发送私聊, 玩家" << pUser->name << pUser->id << XEND;
          return false;
        }

        pUser->getUserChat().setFriendChatID(cmd.desid());
        SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(cmd.desid());
        if (pTargetUser != nullptr)
        {
          if ((pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true))
          {
            XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送私聊失败,我在charid :" << cmd.desid() << "在黑名单中" << XEND;
            break;
          }
          PROTOBUF(ret, retsend, retlen);
          doSocialCmd((const BYTE*)retsend, retlen);
          break;
        }

        PROTOBUF(ret, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      break;
    case ECHAT_CHANNEL_WORLD:
      {
        PROTOBUF(ret.ret(), send, len);
        if (bPunish)
        {
          XLOG << "[聊天管理], 玩家当前信用度小于0, 不能发送世界聊天, 玩家" << pUser->name << pUser->id << XEND;
          pUser->sendCmdToMe(send, len);
          return false;
        }
        MsgManager::sendWorldCmd(send, len);
      }
      break;
    case ECHAT_CHANNEL_MIN:
    case ECHAT_CHANNEL_MAX:
    case ECHAT_CHANNEL_SYS:
    case ECHAT_CHANNEL_BARRAGE: // 在本cpp函数处理
    case ECHAT_CHANNEL_CHAT:
      return false;
  }

  if (ret.ret().voicetime() && !pUser->isRealAuthorized())
  {
    XLOG << "[聊天管理], 尚未实名认证不能发送语音聊天, 玩家" << pUser->name << pUser->id << XEND;
    return true;
  }
  pUser->getAchieve().onChat(cmd.channel());
  pUser->getUserPet().playAction(EPETACTION_OWNER_CHAT);
  pUser->getQuest().onChatSystem(cmd.channel(), cmd.desid(), cmd.str());
  chatLog(ret.ret().id(), pUser->accid, pUser->getUserSceneData().getPlatformId(), pUser->getZoneID(), ret.ret().baselevel(), cmd.desid(), 0, "", ret.ret());

  // collect show item
  TVecString vecString;
  auto collect = [&](const string& str)
  {
    // {il=ssss;"ddddd"}
    string::size_type index = 0;
    while (true)
    {
      string::size_type begin = str.find_first_of("{", index);
      string::size_type end = str.find_first_of("}", index);
      if (begin == string::npos || end == string::npos || end < begin)
        break;

      index = end + 1;
      string temp = str.substr(begin, end - begin);

      begin = temp.find("=");
      end = temp.find(";");
      if (begin != string::npos && end != string::npos && end > begin)
      {
        vecString.push_back(temp.substr(begin + 1, end - begin - 1));
        string tmp = temp.substr(begin + 1, end - begin - 1);
      }
    }
  };
  collect(cmd.str());
  for (auto &v : vecString)
  {
    ItemBase* pBase = pUser->getPackage().getItem(v);
    if (pBase == nullptr || pBase->getCFG() == nullptr)
      continue;

    const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
    ItemData oData;
    pBase->toItemData(&oData);
    RedisManager::getMe().setProtoData(RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ITEMSHOW, pBase->getGUID()), &oData, rCFG.dwItemShowTime);
    XDBG << "[聊天-物品展示]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "展示了 guid:" << v << "name:" << pBase->getCFG()->strNameZh << "typeid:" << pBase->getTypeID() << "的物品" << XEND;
  }
  return true;
}

bool ChatManager_SC::skillChat(SceneUser* pUser, DWORD sysmsgid)
{
  // init ret cmd
  ChatCmd cmd;
  cmd.set_str("");
  cmd.set_channel(ECHAT_CHANNEL_ROUND);

  ChatSocialCmd ret;
  ret.set_accid(pUser->accid);
  ret.set_platformid(pUser->getUserSceneData().getPlatformId());
  ret.set_to_global(true);

  if(!chatRetFill(pUser, *(ret.mutable_ret()), cmd))
    return false;

  bool needReturn;
  roundChat(pUser, ret, true, needReturn, false, sysmsgid);
  return true;
}

bool ChatManager_SC::doChatCmd(SceneUser* pUser, const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (pUser == nullptr || cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case CHATPARAM_QUERYITEMDATA:
      {
        PARSE_CMD_PROTOBUF(QueryItemData, rev);

        ItemData oData;
        if (RedisManager::getMe().getProtoData(RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ITEMSHOW, rev.guid()), &oData) == false)
        {
          MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_ITEMSHOW_OVERTIME);
          break;
        }

        rev.mutable_data()->CopyFrom(oData);
        PROTOBUF(rev, send, len);
        pUser->sendCmdToMe(send, len);
      }
      break;
    case CHATPARAM_QUERY_VOICE:
      {
        PARSE_CMD_PROTOBUF(QueryVoiceUserCmd, rev);
        queryVoiceData(pUser, rev.voiceid());
      }
      break;
    case CHATPARAM_BARRAGE:
      {
        PARSE_CMD_PROTOBUF(BarrageChatCmd, rev);
        setBarrageUser(pUser, rev);
        StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_DANMU_COUNT, 0, 0, pUser->getLevel(), (DWORD)1);
      }
      break;
    case CHATPARAM_BARRAGEMSG:
      {
        PARSE_CMD_PROTOBUF(BarrageMsgChatCmd, rev);
        sendBarrageMsg(pUser, rev);
      }
      break;
    case CHATPARAM_CHAT:
      {
        PARSE_CMD_PROTOBUF(ChatCmd, rev);
        processChatCmd(pUser, rev);
      }
      break;
    case CHATPARAM_GET_VOICEID:
      getVoiceID(pUser);
      break;
    case CHATPARAM_CHAT_SELF:
      {
        ChatRetCmd cmd;
        PARSE_CMD_PROTOBUF(ChatSelfNtf, rev);
        if(!chatRetFill(pUser, cmd, rev.chat()))
          return false;
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool ChatManager_SC::doSocialCmd(const BYTE* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case SOCIALPARAM_CHAT_MSG:
      {
        PARSE_CMD_PROTOBUF(ChatSocialCmd, rev);
        const ChatRetCmd& ret = rev.ret();
        if (ret.channel() == ECHAT_CHANNEL_FRIEND)
        {
          SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(ret.targetid());
          if (pTargetUser == nullptr)
            break;
          PROTOBUF(ret, send, len);
          pTargetUser->sendCmdToMe(send, len);
          pTargetUser->getUserChat().onChat(ret);
        }
      }
      break;
    default:
      return false;
  }

  return true;
}

bool ChatManager_SC::setBarrageUser(SceneUser* pUser, const BarrageChatCmd& cmd)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  if (cmd.opt() == EBARRAGE_OPEN)
    pUser->addOneLevelIndex(ONE_LEVEL_INDEX_TYPE_BARRAGE, pUser->getScene()->id * BARRAGE_INDEX);
  else if (cmd.opt() == EBARRAGE_CLOSE)
    pUser->delOneLevelIndex(ONE_LEVEL_INDEX_TYPE_BARRAGE, pUser->getScene()->id * BARRAGE_INDEX);
  else
    return false;

  return true;
}

bool ChatManager_SC::sendBarrageMsg(SceneUser* pUser, const BarrageMsgChatCmd& cmd)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  GTeam& rTeam = pUser->getTeam();
  if (rTeam.getTeamID() != 0)
  {
    const TMapGTeamMember& mapMember = rTeam.getTeamMemberList();
    if (mapMember.size() == 2)
    {
      SceneUser* pUser1 = nullptr;
      SceneUser* pUser2 = nullptr;
      DWORD dwIndex = 0;
      for (auto &m : mapMember)
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(m.first);
        if (pUser != nullptr)
        {
          if (dwIndex == 0)
            pUser1 = pUser;
          else if (dwIndex == 1)
            pUser2 = pUser;
        }
        ++dwIndex;
      }
      if (pUser1 != nullptr && pUser2 != nullptr)
      {
        pUser1->getShare().addCalcData(ESHAREDATATYPE_MOST_BARRAGEMSG, pUser2->id, 1);
        pUser2->getShare().addCalcData(ESHAREDATATYPE_MOST_BARRAGEMSG, pUser1->id, 1);

        pUser1->getAchieve().onTravel();
        pUser2->getAchieve().onTravel();
      }
    }
  }

  pUser->getQuest().onChat(cmd.str());

  PROTOBUF(cmd, send, len);
  thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_BARRAGE, pUser->getScene()->id * BARRAGE_INDEX, send, len);
  return true;
}

bool ChatManager_SC::getVoiceID(SceneUser* pUser)
{
  if (pUser == nullptr)
  {
    XERR << "[聊天管理-语音id] 获取id失败 未发现玩家" << XEND;
    return false;
  }

  GetVoiceIDChatCmd cmd;

  TMapChatCache& mapCache = m_mapUserChatCache[pUser->id];
  if (mapCache.empty() == true)
  {
    // 临时处理:使用char低16位和当前时间低16拼接一个id
    DWORD dwNow = xTime::getCurSec();
    DWORD dwChar = static_cast<DWORD>(pUser->id & 0xFFFF);
    DWORD dwTime = dwNow & 0xFFFF;
    DWORD dwVoiceID = ((dwChar << 16) | dwTime);
    //DWORD dwVoiceID = GuidManager::getMe().getNextVoiceID() * 10 + thisServer->getZoneID() % 10;
    mapCache.insert(make_pair(dwVoiceID, ""));
    if (mapCache.find(dwVoiceID) == mapCache.end())
    {
      XERR << "[聊天管理-语音id]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "获取id失败" << XEND;
      return false;
    }
  }

  cmd.set_id(mapCache.begin()->first);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XDBG << "[聊天管理-语音id]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "成功获取id :" << cmd.id() << XEND;
  return true;
}

EZoneStatus ChatManager_SC::getZoneStatus(DWORD dwZoneID) const
{
  for (int i = 0; i < m_oZoneCmd.infos_size(); ++i)
  {
    if (m_oZoneCmd.infos(i).zoneid() == dwZoneID)
      return m_oZoneCmd.infos(i).status();
  }

  return EZONESTATUS_MIN;
}

DWORD ChatManager_SC::getZoneMaxBaseLv(DWORD dwZoneID) const
{
  for (int i = 0; i < m_oZoneCmd.infos_size(); ++i)
  {
    if (m_oZoneCmd.infos(i).zoneid() == dwZoneID)
      return m_oZoneCmd.infos(i).maxbaselv();
  }

  return DWORD_MAX;
}

void ChatManager_SC::setZoneInfo(const QueryZoneStatusSessionCmd& cmd)
{
  m_oZoneCmd.CopyFrom(cmd);
  XDBG << "[聊天管理-区线信息] 收到同步 :" << cmd.ShortDebugString() << XEND;
}

void ChatManager_SC::toData(QueryZoneStatusUserCmd& cmd, SceneUser* pUser)
{
  if (pUser == nullptr)
    return;

  for (int i = 0; i < m_oZoneCmd.infos_size(); ++i)
  {
    ZoneInfo* pInfo = cmd.add_infos();
    if (pInfo != nullptr)
    {
      pInfo->CopyFrom(m_oZoneCmd.infos(i));
      pInfo->set_zoneid(getClientZoneID(pInfo->zoneid()));
    }
  }

  const TVecRecentZoneInfo& vecInfos = pUser->getUserSceneData().getRecentZones();
  for (auto &v : vecInfos)
  {
    RecentZoneInfo* pInfo = cmd.add_recents();
    if (pInfo == nullptr)
      continue;
    pInfo->set_type(v.eType);
    pInfo->set_zoneid(v.dwZoneID);
  }
}

//离线聊天暂时不处理了
void ChatManager_SC::chatLog(QWORD sID, QWORD sAccID, DWORD sPlat, DWORD sZone, DWORD sLv, QWORD rID, QWORD rAccID, const string rname, const ChatRetCmd& cmd)
{
  if (cmd.voicetime() == 0)
  {
    PlatLogManager::getMe().chatLog(thisServer,
        sPlat,
        sZone,
        sAccID,
        cmd.name(),
        sID,
        rAccID,
        rname,
        rID,
        cmd.channel(),
        0,
        cmd.str(),
        sLv
        );
  }
  else
  {
    PlatLogManager::getMe().chatLog(thisServer,
        sPlat,
        sZone,
        sAccID,
        cmd.name(),
        sID,
        rAccID,
        rname,
        rID,
        cmd.channel(),
        0,
        "",
        sLv,
        cmd.voicetime()
        );
  }

  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CHAT_COUNT, cmd.channel(), 0, cmd.baselevel(), (DWORD)1);
}

bool ChatManager_SC::checkIgnoreGagTime(EGameChatChannel eType)
{
  bool ret = false;
  if(EGameChatChannel_IsValid(static_cast<DWORD>(eType)) == false)
    return ret;

  //check config
  const SChatChannelCFG sCFG = MiscConfig::getMe().getChatChannelCFG();
  switch (eType)
  {
    case ECHAT_CHANNEL_ROUND:
      {
        if(sCFG.dwRound != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_TEAM:
      {
        if(sCFG.dwTeam != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_GUILD:
      {
        if(sCFG.dwGuild != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_FRIEND:
      {
        if(sCFG.dwFriend != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_WORLD:
      {
        if(sCFG.dwWorld != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_MAP:
      {
        if(sCFG.dwMap != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_SYS:
      {
        if(sCFG.dwSys != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_ROOM:
      {
        if(sCFG.dwRoom != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_BARRAGE:
      {
        if(sCFG.dwBarrage != 0)
          ret = true;
      }
      break;
    case ECHAT_CHANNEL_CHAT:
      {
        if(sCFG.dwChat != 0)
          ret = true;
      }
      break;
    default:
      break;
  }

  return ret;
}

bool ChatManager_SC::chatRetFill(SceneUser* pUser, ChatRetCmd& ret, const ChatCmd& cmd)
{
  if(!pUser) return false;

  ret.set_id(pUser->id);
  ret.set_portrait(pUser->getPortrait().getCurPortrait());
  ret.set_frame(pUser->getPortrait().getCurFrame());
  ret.set_baselevel(pUser->getLevel());
  ret.set_voiceid(cmd.msgid());
  ret.set_voicetime(cmd.voicetime());
  ret.set_hair(pUser->getHairInfo().getRealHair());
  ret.set_haircolor(pUser->getHairInfo().getRealHairColor());
  ret.set_body(pUser->getUserSceneData().getRealBody());
  ret.set_appellation(pUser->getTitle().getCurTitle(ETITLE_TYPE_MANNUAL));
  ret.set_channel(cmd.channel());
  ret.set_rolejob(pUser->getProfession());
  ret.set_gender(pUser->getUserSceneData().getGender());
  ret.set_blink(pUser->getUserSceneData().getBlink());
  ret.set_head(pUser->getUserSceneData().getHead(true));
  ret.set_face(pUser->getUserSceneData().getFace(true));
  ret.set_mouth(pUser->getUserSceneData().getMouth(true));
  ret.set_eye(pUser->getEye().getCurID(true));
  ret.set_str(cmd.str());
  ret.set_name(pUser->name);
  if(0 != pUser->getGuild().id())
    ret.set_guildname(pUser->getGuild().name());
  if(ECHAT_CHANNEL_FRIEND == cmd.channel())
    ret.set_targetid(cmd.desid());

  return true;
}

void ChatManager_SC::sendChatMsgToNine(SceneUser* pUser, const char* str, EGameChatChannel channel)
{
  if (pUser == nullptr || str == nullptr)
    return;
  ChatCmd cmd;
  cmd.set_str(str);
  cmd.set_channel(channel);

  ChatRetCmd ret;
  if (chatRetFill(pUser, ret, cmd) == false)
    return;

  PROTOBUF(ret, send, len);

  pUser->sendCmdToNine(send, len);
}

void ChatManager_SC::sendChatMsgToMe(SceneUser* pUser, const char* str, EGameChatChannel channel)
{
  if (pUser == nullptr || str == nullptr)
    return;
  ChatCmd cmd;
  cmd.set_str(str);
  cmd.set_channel(channel);

  ChatRetCmd ret;
  if (chatRetFill(pUser, ret, cmd) == false)
    return;

  PROTOBUF(ret, send, len);

  pUser->sendCmdToMe(send, len);
}


bool ChatManager_SC::roundChat(SceneUser* pUser, ChatSocialCmd& ret, bool bSkillChat,bool& needReturn, bool bPunish/* = false*/, DWORD sysmsgid/* = 0 若是发出的是系统的msg,则不为0*/)
{
  needReturn = false;
  if (pUser->getScene() == nullptr)
  {
    needReturn = true;
    return false;
  }

  if(sysmsgid != 0)
  {
    ret.mutable_ret()->set_sysmsgid(sysmsgid);
  }

  PROTOBUF(ret.ret(), send, len);

  // 在释放技能时发出的消息不需要判断
  if (bPunish && !bSkillChat)
  {
    pUser->sendCmdToMe(send, len);
    XLOG << "[聊天管理], 玩家当前信用度小于0, 不能发送附近聊天, 玩家" << pUser->name << pUser->id << XEND;
    needReturn = true;
    return true;
  }
  xSceneEntrySet set;
  pUser->getScene()->getEntryListInNine(SCENE_ENTRY_USER, pUser->getPos(), set);
  for (auto &s : set)
  {
    SceneUser* pDestUser = dynamic_cast<SceneUser*>(s);
    if (pDestUser != nullptr)
      pDestUser->sendCmdToMe(send, len);
  }
  return true;
}

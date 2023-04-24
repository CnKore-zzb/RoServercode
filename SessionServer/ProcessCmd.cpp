#include "SessionServer.h"
#include "RegCmd.h"
#include "SessionUserManager.h"
#include "SessionUser.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "SessionWeddingMgr.h"
//#include "SessionSceneCmd.h"
#include "xDefine.h"
#include "xNetProcessor.h"
#include "GatewayCmd.h"
#include "LoginUserCmd.pb.h"
#include "LogCmd.pb.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "MsgManager.h"
//#include "ChatManager.h"
#include "RedisManager.h"
#include "GMCommandManager.h"
#include "ClientCmd.h"
#include "SessionSociality.pb.h"
#include "SocialCmd.pb.h"
#include "TeamCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "ChatManager_SE.h"
#include "SealManager.h"
#include "MatchSCmd.pb.h"
#include "RecordCmd.pb.h"
#include "AuctionSCmd.pb.h"
#include "GuildGmMgr.h"
#include "WeddingSCmd.pb.h"
#include "PveCardConfig.h"
#include "SessionGvg.h"
#include "SessionActivityMgr.h"
#include "SysmsgConfig.h"
#include "GlobalManager.h"
#include "Boss.h"

bool SessionServer::doRegCmd(xNetProcessor *np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;
  RegCmd *cmd = (RegCmd *)buf; 

  switch (cmd->param)
  {
    case CHANGE_ZONE_CHAR_REGCMD:
      {
        ChangeZoneCharRegCmd *rev = (ChangeZoneCharRegCmd *)buf;

        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser)
        {
          Cmd::SelectCharClientUserCmd send;
          send.charid = pUser->id;
          pUser->sendCmdToMe(&send, sizeof(send));

          XLOG << "[跨区] " << pUser->accid << pUser->id << pUser->name << " 跨区成功" << XEND;
        }

        return true;
      }
      break;
    case CREATE_OK_CHAR_REGCMD:
      {
        CreateOKCharRegCmd *rev = (CreateOKCharRegCmd *)buf;

        ServerTask *net = getConnectedServer("GateServer", rev->gatename);
        if (!net) return true;

        net->sendCmd(rev, sizeof(*rev));

        return true;
      }
      break;
    case RET_DELETE_CHAR_REGCMD:
      {
        RetDeleteCharRegCmd* rev = (RetDeleteCharRegCmd*)cmd;
       // delChar(((RetDeleteCharRegCmd *) cmd)->id);
        sendCmdToServer(cmd, len, "GateServer");
        //del success
        if (rev->ret)
        {
          sendCmd(ClientType::trade_server, buf, len);
          sendCmd(ClientType::wedding_server, buf, len);
        }
        return true;
      }
      break;
    case DELETE_CHAR_REGCMD:
      {
        DelCharRegCmd *rev = (DelCharRegCmd *)cmd;
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev->id);
        if (pUser) return true;

        sendCmdToServer(cmd, len, "RecordServer");

        UserDelSocialCmd delcmd;
        delcmd.set_charid(rev->id);
        PROTOBUF(delcmd, delsend, dellen);
        thisServer->sendCmdToServer(delsend, dellen, "SocialServer");
        thisServer->sendCmdToServer(delsend, dellen, "TeamServer");
        sendCmd(ClientType::guild_server, delsend, dellen);

        return true;
      }
      break;
    case LOGIN_REGCMD:
      {
        LoginRegCmd *rev = (LoginRegCmd *)cmd;

        ServerTask *task = getConnectedServer("GateServer", rev->gateName);
        if (!task) return true;

        SessionUser *pUser = SessionUserManager::getMe().getUserByAccID(rev->accid);
        if (pUser)
        {
          if (strncmp(pUser->m_strDeviceID.c_str(), rev->deviceid, MAX_NAMESIZE) != 0)
          {
            RegErrUserCmd cmd;
            cmd.set_ret(REG_ERR_DUPLICATE_LOGIN);
            PROTOBUF(cmd, send, len);
            pUser->sendCmdToMe(send, len);
          }

          Cmd::ReconnectClientUserCmd send;
          thisServer->sendCmdToLoginUser(rev->accid, &send, sizeof(send), task);

          DWORD dwNow = xTime::getCurSec();
          if (pUser->getUserState() != USER_STATE_QUIT || pUser->m_dwReloginTime <= dwNow)
          {
            LoginOutRegCmd send;
            send.id = pUser->id;
            send.accid = pUser->accid;
            strncpy(send.gateName, pUser->getGateServerName(), MAX_NAMESIZE);
            thisServer->doRegCmd(np, (BYTE *)&send, sizeof(send));
          }

          XLOG << "[登录]" << pUser->accid << pUser->id << pUser->name << "已在线,等待重连" << XEND;

          return true;
        }
        pUser = NEW SessionUser(rev->id, rev->name, rev->accid, task, rev->platformID, rev->deviceid);
        if (!pUser)
        {
          loginErr(rev->accid, Cmd::REG_ERR_SESSION_CREATE_FAILD, task);
          XERR << "[登录]" << rev->accid << rev->id << rev->name << "创建SessionUser失败" << XEND;
          return true;
        }
        if (!SessionUserManager::getMe().addUser(pUser))
        {
          loginErr(rev->accid, Cmd::REG_ERR_SESSION_CREATE_FAILD, task);
          XERR << "[登录]" << pUser->accid << pUser->id << pUser->name << "SessionUser添加到管理器失败" << XEND;
          SAFE_DELETE(pUser);
          return true;
        }
        pUser->setUserState(USER_STATE_LOGIN);
        pUser->setGateServerUserState(GateServerUserState::req_login);
        pUser->setIP(rev->ip);
        pUser->setPhone(rev->phone);
        pUser->setSafeDevice(rev->safeDevice);
        pUser->getAuthorize().setAuthorize(rev->password, rev->ignorepwd, rev->resettime, false);
        pUser->getAuthorize().setRealAuthorize(rev->realAuthorized);
        pUser->setMaxBaseLv(rev->maxbaselv);
        pUser->setLanguage(static_cast<ELanguageType>(rev->language));

        GCharReader gchar(thisServer->getRegionID(), rev->id);
        DWORD dwMapID = gchar.fetchMapID();

        //通知场景可以读数据了
        SessionScene *scene = SessionSceneManager::getMe().getSceneByID(dwMapID);
        if (!scene || !scene->canEnter(pUser))
        {
          scene = SessionSceneManager::getMe().getSceneByID(SessionSceneManager::getMe().getMapIDByDMap(dwMapID));
          if (scene == nullptr)
          {
            const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
            dwMapID = rCFG.dwNewCharMapID;
            scene = SessionSceneManager::getMe().getSceneByID(rCFG.dwNewCharMapID);
            XLOG << "[登录]" << pUser->accid << pUser->id << pUser->name << "找不到目标地图:" << dwMapID << ",设置上线地图:" << rCFG.dwNewCharMapID << XEND;
          }
        }

        if (scene)
        {
          pUser->setScene(scene);

          /*Cmd::Session::NotifyLoginSceneSessionCmd send;
          send.id = rev->id;
          strncpy(send.name, rev->name, MAX_NAMESIZE);
          send.accID = rev->accid;
          strncpy(send.gateName, task->getName(), MAX_NAMESIZE);
          send.mapid = scene->id;
          scene->sendCmd(&send, sizeof(send));*/

          NotifyLoginSessionCmd cmd;
          cmd.set_id(rev->id);
          cmd.set_name(rev->name);
          cmd.set_accid(rev->accid);
          cmd.set_gatename(task->getName());
          cmd.set_mapid(scene->id);
          cmd.set_phone(rev->phone);
          cmd.set_ignorepwd(rev->ignorepwd);
          cmd.set_language(rev->language);
          cmd.set_realauthorized(rev->realAuthorized);
          cmd.set_maxbaselv(rev->maxbaselv);
          PROTOBUF(cmd, send, len);
          scene->sendCmd(send, len);

          pUser->setSceneServerUserState(SceneServerUserState::scene_load_data);

          GateUserOnlineGatewayCmd gateSend;
          gateSend.accid = rev->accid;
          task->sendCmd(&gateSend, sizeof(gateSend));

          XLOG << "[登录]" << "成功" << pUser->accid << pUser->id << pUser->name << "通知场景" << scene->name << "读数据,gatetask:" << task->getName() <<
            ",设备id:" << rev->deviceid << "在线:" << (DWORD)SessionUserManager::getMe().size() <<"实名认证"<<rev->realAuthorized << XEND;
        }
        else
        {
          loginErr(rev->accid, Cmd::REG_ERR_NOTIFY_SCENE, task);
          XERR << "[登录]" << pUser->accid << pUser->id << pUser->name << "未找到对应场景,MapID:" << dwMapID << XEND;
          SessionUserManager::getMe().delUser(pUser);
          return true;
        }
        return true;
      }
      break;
    case LOGIN_OUT_SCENE_REGCMD:
      {
        LoginOutSceneRegCmd *rev = (LoginOutSceneRegCmd *)cmd;

        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser)
        {
          XLOG << "[注销]" << pUser->accid << pUser->id << pUser->name << "退出游戏" << XEND;
          SessionUserManager::getMe().loginOutGate(pUser->accid, pUser->gate_task());
          SessionUserManager::getMe().onUserQuit(pUser);
        }
        else
        {
          XERR << "[注销]" << rev->charid << "管理器中找不到玩家" << XEND;
        }

        return true;
      }
      break;
    case LOGIN_OUT_REGCMD:
      {
        LoginOutRegCmd *rev = (LoginOutRegCmd *)cmd;

        processLoginOutRegCmd(rev);

        return true;
      }
      break;
    case RET_CREATE_CHAR_REGCMD:
      {
        //RetCreateCharRegCmd *rev = (RetCreateCharRegCmd *)cmd;

        //登陆
        //RetCreateCharRegCmd *rev = (RetCreateCharRegCmd *)cmd;
        //if (!rev->ret || rev->ip)
        sendCmdToServer(buf, len, "GateServer");
        return true;
      }
      break;
    case CREATE_CHAR_REGCMD:
      {
        sendCmdToServer(buf, len, "RecordServer");
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

void SessionServer::processLoginOutRegCmd(LoginOutRegCmd *rev, Cmd::RegErrRet eErr /*= REG_ERR_DUPLICATE_LOGIN*/)
{
  if (!rev) return;

  LoginOutSceneRegCmd send;
  send.accid = rev->accid;
  send.charid = rev->id;
  send.bDelete = rev->deletechar;

  SessionUser *pUser = SessionUserManager::getMe().getUserByAccID(rev->accid);
  if (pUser)
  {
    pUser->setUserState(USER_STATE_QUIT);
    pUser->m_dwReloginTime = now() + 60;

    if (pUser->getScene())
    {
      pUser->getScene()->sendCmd(&send, sizeof(send));
      XLOG << "[注销]" << pUser->accid << pUser->id << pUser->name << "通知场景退出" << pUser->getScene()->id << pUser->getScene()->name << XEND;
    }
    else
    {
      sendCmdToRecord(&send, sizeof(send));
      SessionUserManager::getMe().loginOutGate(pUser->accid, pUser->gate_task());
    }

    RegErrUserCmd cmd;
    cmd.set_ret(eErr);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
  else
  {
    sendCmdToRecord(&send, sizeof(send));

    ServerTask *gt = getConnectedServer("GateServer", rev->gateName);
    SessionUserManager::getMe().loginOutGate(rev->accid, gt);
  }
}

bool SessionServer::doTradeCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    //转发到client
    case Cmd::WORLD_MSG:
      {
        PARSE_CMD_PROTOBUF(Cmd::WorldMsgCmd, rev);
        MsgManager::sendWorldCmd((void *)rev.data().c_str(), rev.len());
        return true;
      }
    case Cmd::SESSION_TO_ME_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::SessionToMeRecordTrade, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser && pUser->getScene())
        {
          pUser->sendCmdToMe((void*)(rev.data().c_str()), rev.len());
        }
        else
        {
          //不需要通知record log
          XINF << "[交易] [转发SESSION_TO_ME_RECORDTRADE消息给玩家,找不到玩家]" << rev.charid() << XEND;
        }
        return true;
      }
      break;
      //scene -> trade
    case Cmd::SESSION_FORWARD_SCENECMD_TRADE:
      {
        XDBG << "[交易] [收到scene返回的协议转发给trade]" << XEND;
        thisServer->sendCmd(ClientType::trade_server, (BYTE*)buf, len);
        return true;
      }
      break;
      //转发到scene
    case Cmd::REDUCE_MONEY_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::ReduceMoneyRecordTradeCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(buf, len);
          XINF << "[交易] [转发REDUCE_MONEY_RECORDTRADE消息给场景 ]" << rev.charid() << rev.ShortDebugString() << XEND;
        }
        else
        {
          XINF << "[交易] [转发REDUCE_MONEY_RECORDTRADE消息给场景,找不到玩家]" << rev.charid() << XEND;
          //要通知record
          rev.set_ret(Cmd::ETRADE_RET_CODE_NOT_ONLINE);
          PROTOBUF(rev, send, len);
          thisServer->sendCmdToRecord(send, len);
        }
        return true;
      }
      break;
    case Cmd::ADD_ITEM_RECORDTRADE:
      {
        ////除了下架的 不再给物品 只是提示
        PARSE_CMD_PROTOBUF(Cmd::AddItemRecordTradeCmd, rev);
        if (rev.addtype() == EADDITEMTYP_RETURN)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser && pUser->getScene() && pUser->getScene()->sendCmd(buf, len))
          {
            XINF << "[交易] [转发ADD_ITEM_RECORDTRADE消息给场景 ]" << rev.charid() << rev.ShortDebugString() << XEND;
          }
          else
          {
            if (MailManager::getMe().addOfflineTradeItem(rev))
              XINF << "[交易] 离线加道具 成功 charid:" << rev.charid() << rev.ShortDebugString() << XEND;
            else
              XERR << "[交易] 离线加道具 失败 charid:" << rev.charid() << rev.ShortDebugString() << XEND;
          }
        }
        return true;
      }
      break;
    case Cmd::ADD_MONEY_RECORDTRADE:
      {
        //不再给钱 只是提示
        PARSE_CMD_PROTOBUF(Cmd::AddMoneyRecordTradeCmd, rev);

        //SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        //bool bSetMail = false;

        /*  if (pUser && pUser->getScene())
            {
            if (rev.total_money())
            {
            if (pUser->getScene()->sendCmd(buf, len))
            {
            XINF << "[交易] [购买，给卖家加钱，转发到scene]" << rev.ShortDebugString() << XEND;
            bSetMail = false;
            }
            else
            {
            bSetMail = true;
            }
            }
            }
            else*/
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          //bSetMail = true;
          if (rev.type() == EOperType_PublicityBuyFail)
          {
            const SExchangeItemCFG* pCfg = ItemConfig::getMe().getExchangeItemCFG(rev.itemid());
            if (pCfg)
            {
              //您在公示期期间抢购[63cd4e]%s[-]失败，返还[63cd4e]%s[-]Zeny。
              DWORD consume = rev.money2();
              DWORD retMoney = rev.total_money();
              if (consume == 0)
              {
                MsgParams params;
                params.addString(pCfg->strName);
                params.addNumber(retMoney);
                MsgManager::sendMsgMust(rev.charid(), 10405, params);
              }
              else
              {
                //您在公示期抢购中成功购买[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，返还[63cd4e]%s[-]Zeny。
                MsgParams params;
                params.addNumber(rev.count());
                params.addString(pCfg->strName);
                params.addNumber(rev.money2());
                params.addNumber(retMoney);
                MsgManager::sendMsgMust(rev.charid(), 10401, params);
              }
            }
          }

          if (rev.type() == EOperType_NormalSell || rev.type() == EOperType_PublicitySellSuccess)
          {
            // msg
            AddOfflineMsgSocialCmd cmd;
            cmd.mutable_msg()->set_type(EOFFLINEMSG_TRADE);
            cmd.mutable_msg()->set_targetid(rev.charid());
            cmd.mutable_msg()->set_itemid(rev.itemid());
            cmd.mutable_msg()->set_price(rev.price());
            cmd.mutable_msg()->set_count(rev.count());
            cmd.mutable_msg()->set_moneytype(static_cast<EMoneyType>(rev.money_type()));
            cmd.mutable_msg()->set_givemoney(rev.total_money());
            PROTOBUF(cmd, send, len);
            thisServer->doSocialCmd((const BYTE*)send, len);
          }
        }

        //if (bSetMail)
        //{     
        //  // mail
        //  if (MailManager::getMe().addOfflineTradeMoney(rev))
        //    XINF << "[交易] [购买，给卖家加钱, 卖家不在线]" << rev.ShortDebugString() << XEND;
        //  else
        //    XERR << "[交易] [购买，给卖家加钱, 卖家不在线]" << rev.ShortDebugString() << XEND;
        //}
        return true;
      }
      break;
      //卖 转发到scene
    case Cmd::REDUCE_ITEM_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::ReduceItemRecordTrade, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser && pUser->getScene())
        {
          XINF << "[交易] [出售，卖家扣装备，转发到scene]" << rev.ShortDebugString().c_str() << XEND;
          pUser->getScene()->sendCmd(buf, len);
        }
        else
        {
          XINF << "[交易] [出售，卖家扣装备，卖家不在线" << rev.ShortDebugString() << XEND;
          //要通知
          rev.set_ret(Cmd::ETRADE_RET_CODE_NOT_ONLINE);
          PROTOBUF(rev, send, len);
          thisServer->sendCmdToRecord(send, len);
        }
        return true;
      }
      break;
    case Cmd::UPDATE_TRADELOG_RECORDTRADE:
      {
        PARSE_CMD_PROTOBUF(Cmd::UpdateTradeLogCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          pUser->getTradeLog().updateTradeLog(rev);
        }
        return true;
      }
      break;
    case Cmd::GIVE_CHECK_MONEY_RECORDTRADE:
      {
        //scene -> session
        PARSE_CMD_PROTOBUF(Cmd::GiveCheckMoneySceneTradeCmd, rev);

        if (rev.fromtrade())
        {
          //来自交易所返回
          PARSE_CMD_PROTOBUF(Cmd::GiveCheckMoneySceneTradeCmd, rev);
          XLOG << "[交易-赠送者] 收到交易所服额度计算返回" << rev.charid() << XEND;
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser == nullptr)
          {
            XERR << "[交易-赠送者] 收到交易所服额度计算返回,玩家不在线" << rev.charid() << XEND;
            return false;
          }
          return pUser->getTradeLog().giveTradeRes(rev);
        }
        else
        { 
          //来自场景返回
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser == nullptr)
          {
            XERR << "[交易-赠送-从场景扣除赠送返回] 找不玩家" << rev.charid() << "friend" << rev.friendid() << XEND;
            return false;
          }

          return pUser->getTradeLog().giveSceneRes(rev);
        }
      }
      break;
    case Cmd::ADD_GIVE_ITEM_RECORDTRADE: //scene -> session
      {
        PARSE_CMD_PROTOBUF(AddGiveItemSceneTradeCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XLOG << "[交易-领取赠送] 场景返回" << rev.charid() << "玩家不在线" << rev.ShortDebugString() << XEND;
          return false;
        }

        pUser->getTradeLog().acceptGiveSceneRes(rev);

        XLOG << "[交易-领取赠送] 场景返回" << rev.charid() << "领取成功" << rev.ShortDebugString() << XEND;
        return true;
      }
      break;
    case Cmd::RECEIVE_GIVE_RECORDTRADE: //session->global -> session
      {
        PARSE_CMD_PROTOBUF(ReceiveGiveSceneTradeCmd, rev);
        XLOG << "[交易-赠送接收者] 收到好友的赠送" << rev.charid() << "id" << rev.id() << XEND;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[交易-赠送接收者] 收到好友的赠送,玩家不在线" << rev.charid() << "id" << rev.id() << XEND;
          return false;
        }
        bool ret = pUser->getTradeLog().receiveGive(rev.id());
        XLOG << "[交易-赠送接收者] 收到好友的赠送 处理"<<(ret?"成功":"失败") << rev.charid() << "赠送id" << rev.id() << XEND;
        return ret;
      }
      break;
    case Cmd::NTF_GIVE_STATUS_RECORDTRADE: //session->global -> session
      {
        PARSE_CMD_PROTOBUF(NtfGiveStatusSceneTradeCmd, rev);
        XLOG << "[交易-赠送者] 收到接收者的反馈" << rev.charid() << "id" << rev.id() <<"status"<<rev.status() << XEND;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[交易-赠送者] 收到接收者的反馈,玩家不在线" << rev.charid() << "id" << rev.id() << XEND;
          return false;
        }
        bool ret = pUser->getTradeLog().ntfGiveRes(rev);
        XLOG << "[交易-赠送者] 收到接收者的反馈 处理" << (ret ? "成功" : "失败") << rev.charid() << "赠送id" << rev.id() << XEND;
        return ret;
      }
      break;  
    case Cmd::SYNC_GIVE_ITEM_RECORDTRADE: //scene -> session ->scene
      {
        PARSE_CMD_PROTOBUF(SyncGiveItemSceneTradeCmd, rev);
        XLOG << "[交易-赠送接收者] 收到列表请求" << rev.charid() << XEND;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[交易-赠送接收者] 收到列表请求,玩家不在线" << rev.charid() << XEND;
          return false;
        }
        pUser->getTradeLog().syncToScene(rev);
        pUser->getMail().syncGingerToScene();
        return true;
      }
      break;
    case Cmd::TRADE_PRICE_QUERY_RECORDTRADE:
      {
        return thisServer->sendCmd(ClientType::auction_server, buf, len);
      }
      break;
    default:
      return false;
  }
  return true;
}

bool SessionServer::doMatchCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
  case Cmd::MATCHSPARAM_SESSION_FORWARD_CCMD_MATCH:       //matchserver ->client
  {
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardCCmdMatch, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      XINF << "[斗技场-转发匹配服消息给客户端]" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
      return pUser->sendCmdToMe(rev.data().c_str(), rev.len());
    }
    else
    {
      XERR << "[斗技场-转发匹配服消息给客户端],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
  }
  break;
  case Cmd::MATCHSPARAM_SESSION_FORWARD_MATCH_SCENE:       //matchserver ->sceneserver
  {
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardMatchScene, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser && pUser->getScene())
    {
      bool ret = pUser->getScene()->sendCmd(rev.data().c_str(), rev.len());
      XINF << "[斗技场-转发匹配服消息给场景]" << rev.charid() <<"ret" << ret<< rev.ShortDebugString().c_str() << XEND;
      return ret;
    }
    else
    {
      XERR << "[斗技场-转发匹配服消息给场景],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
  }
  break;
  case Cmd::MATCHSPARAM_SESSION_FORWARD_MATCH_TEAM:       //matchserver ->teamserver
  {
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardMatchTeam, rev);
    
    //转发到TeamServer
    bool ret = thisServer->sendCmdToServer(rev.data().c_str(), rev.len(), "TeamServer");
    XDBG << "[斗技场-消息转发] MatchServer->TeamServer, ret" << ret << XEND;
    return ret;
  }
  break;
  case Cmd::MATCHSPARAM_ENTER_PVP_MAP:
  {
    PARSE_CMD_PROTOBUF(Cmd::EnterPvpMapSCmdMatch, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser && pUser->getScene())
    {
      XINF << "[斗技场-通知场景跳线进入]"<< rev.charid() << rev.ShortDebugString().c_str() << XEND;
      return pUser->getScene()->sendCmd(buf, len);
    }
    else
    {
      XERR << "[交易] [出售，卖家扣装备，卖家不在线" << rev.ShortDebugString() << XEND;
    }
    return true;
  }
  case Cmd::MATCHSPARAM_NTF_JOIN_ROOM:
  {
    //转发到TeamServer
    
    thisServer->sendCmdToServer(buf, len, "TeamServer");
    return true;
  }
  break;
  case Cmd::MATCHSPARAM_NTF_LEAVE_ROOM:
  {
    //转发到TeamServer

    thisServer->sendCmdToServer(buf, len, "TeamServer");
    return true;
  }
  break;
  case Cmd::MATCHSPARAM_SESSION_FORWARD_TEAM_MATCH:
  {
    //转发TeamServer -> MatchServer
    PARSE_CMD_PROTOBUF(Cmd::SessionForwardTeamMatch, rev);
    bool ret = thisServer->sendCmd(ClientType::match_server, rev.data().c_str(), rev.len());
    XDBG << "[斗技场-消息转发] TeamServer->MatchServer, ret"<<ret << rev.ShortDebugString()<< XEND;
    return true;
  }
  break;
  //case Cmd::MATCHSPARAM_UPDATE_PVP_TEAM_DATA:
  //{
  //  //转发到MatchServer    
  //  thisServer->sendCmd(ClientType::match_server, buf, len);
  //  XDBG << "[斗技场-参赛队伍信息同步到匹配服]" << XEND;
  //  return true;
  //}
  //break;
  case Cmd::MATCHSPARAM_LEAVE_PVP_MAP:
  {
    //scene转发到MatchServer    
    thisServer->sendCmd(ClientType::match_server, buf, len);
    XLOG << "[斗技场-玩家离开斗技场场景]" << XEND;
    return true;
  }
  case Cmd::MATCHSPARAM_SYNC_RAIDSCENE:
  {
    PARSE_CMD_PROTOBUF(Cmd::SyncRaidSceneMatchSCmd, rev);
    
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(rev.sceneid());
    if (pScene)
    {
      rev.set_roomid(pScene->m_oRaidData.m_qwRoomId);
      if (rev.open() == false)
      {
        SessionSceneManager::getMe().clearOnePvpRoom(pScene);
      }   
      PROTOBUF(rev, send2, len2);
      bool ret = thisServer->sendCmd(ClientType::match_server, send2, len2);
      XLOG << "[斗技场-状态同步到匹配服] sceneid"<<rev.sceneid()<<"roomid"<< pScene->m_oRaidData.m_qwRoomId<<"open"<<rev.open()<<"count"<<rev.count() <<"ret"<<ret << XEND;
    }    
    return true;
  }
  case Cmd::MATCHSPARAM_SYNC_ROOMINFO:
  {
    PARSE_CMD_PROTOBUF(SyncRoomSceneMatchSCmd, rev);
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(rev.sceneid());
    if (pScene)
    {
      pScene->sendCmd(buf, len);
      XLOG << "[斗技场-房间信息同步场景], sceneid:" << rev.sceneid() << "roomid:" << pScene->m_oRaidData.m_qwRoomId << XEND;
    }
    return true;
  }
  case Cmd::MATCHSPARAM_USER_BOOTH_NTF:
  {
    PARSE_CMD_PROTOBUF(UserBoothNTFMatchSCmd, rev);
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(rev.sceneid());
    if (pScene)
    {
      pScene->sendCmd(buf, len);
      XLOG << "[斗技场-同步摆摊信息], sceneid:" << rev.sceneid() << "charid:" << rev.user().guid() << XEND;
    }
    return true;
  }
  case Cmd::MATCHSPARAM_KICK_USER:
  case Cmd::MATCHSPARAM_RESET_PVP:
  case Cmd::MATCHSPARAM_SWITCH_PVP:
  case Cmd::MATCHSPARAM_QUERY_SOLD_CNT:
  case Cmd::MATCHSPARAM_CHECK_CAN_BUY:
  case Cmd::MATCHSPARAM_ADD_BUY_CNT:
  case Cmd::MATCHSPARAM_SUPERGVG_JOIN: //guild->match
  case Cmd::MATCHSPARAM_CLEAR_MVPCD:
  case Cmd::MATCHSPARAM_USER_BOOTH_REQ:
  case Cmd::MATCHSPARAM_SCENE_GM_TEST:
  case Cmd::MATCHSPARAM_UPDATE_SCORE:
  {
    // scene -> match
    thisServer->sendCmd(ClientType::match_server, buf, len);
    return true;
  }
  break;
  case MATCHSPARAM_TUTOR_OPT:
  {
    PARSE_CMD_PROTOBUF(TutorOptMatchSCmd, rev);
    if (rev.ret() == false)
      thisServer->sendCmdToServer(buf, len, "SocialServer");
    else
      thisServer->sendCmd(ClientType::match_server, buf, len);
  }
  break;
  case Cmd::MATCHSPARAM_SYNC_SCORE:
  {
    PARSE_CMD_PROTOBUF(SyncUserScoreMatchSCmd, rev);
    SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      pUser->setMatchScore(rev);
      pUser->sendCmdToScene(buf, len);
    }
    return true;
  }
  break;
  default:
    return false;
  }
  return false;
}

bool SessionServer::doSocialCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  using namespace Cmd;

  switch (cmd->param)
  {
    case SOCIALPARAM_FORWARD_TO_USER_SCENE:
      {
        PARSE_CMD_PROTOBUF(ForwardToUserSceneSocialCmd, rev);
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(rev.data().c_str(), rev.len());
        }
      }
      break;
    case SOCIALPARAM_FORWARD_TO_SCENE_USER:
      {
        PARSE_CMD_PROTOBUF(ForwardToSceneUserSocialCmd, rev);
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser && pUser->getScene())
        {
          pUser->sendCmdToSceneUser(rev.data().c_str(), rev.len());
        }
        return true;
      }
      break;
    case SOCIALPARAM_FORWARD_TO_USER:
      {
        PARSE_CMD_PROTOBUF(ForwardToUserSocialCmd, rev);
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          pUser->sendCmdToMe(rev.data().c_str(), rev.len());
        }
      }
      break;
    case SOCIALPARAM_FORWARD_TO_SESSION_USER:
      {
        PARSE_CMD_PROTOBUF(ForwardToSessionUserSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
          pUser->doSessionUserCmd((const BYTE*)rev.data().c_str(), rev.len());
      }
      break;
    case SOCIALPARAM_CHAT_WORLDMSG:
      {
        PARSE_CMD_PROTOBUF(ChatWorldMsgSocialCmd, rev);
        PROTOBUF(rev.msg(), send, len);
        MsgManager::sendWorldCmd(send, len);
      }
      return true;
    case SOCIALPARAM_CHAT_MSG:
      {
        PARSE_CMD_PROTOBUF(ChatSocialCmd, rev);
        if (rev.to_global() == false)
        {
          SessionUser* pTargetUser = SessionUserManager::getMe().getUserByID(rev.ret().targetid());
          if (pTargetUser != nullptr)
            pTargetUser->sendCmdToScene(buf, len);
          return true;
        }

        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.ret().id());
        if (pUser == nullptr)
          return true;

        if (rev.ret().channel() == ECHAT_CHANNEL_TEAM)
        {
          PROTOBUF(rev.ret(), send, len);
          ForwardAllServerTeamCmd message;
          message.set_charid(pUser->id);
          message.set_data(send, len);
          message.set_len(len);
          PROTOBUF(message, fsend, flen);
          thisServer->sendCmdToServer(fsend, flen, "TeamServer");
        }
        else if (rev.ret().channel() == ECHAT_CHANNEL_GUILD)
        {
          PROTOBUF(rev.ret(), send, len);
          ChatSyncGuildSCmd message;
          message.set_charid(pUser->id);
          message.set_data(send, len);
          message.set_len(len);
          PROTOBUF(message, ssend, slen);
          thisServer->sendCmd(ClientType::guild_server, (BYTE *)ssend, slen);
        }
        else if (rev.ret().channel() == ECHAT_CHANNEL_FRIEND)
        {
          SessionUser* pTargetUser = SessionUserManager::getMe().getUserByID(rev.ret().targetid());
          if (pTargetUser != nullptr)
          {
            if (pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true)
            {
              XERR << "[私聊], 玩家" << pTargetUser->name << pTargetUser->id << "已将" << pUser->name << pUser->id << "拉黑" << XEND;
              break;
            }
            pTargetUser->sendCmdToScene(buf, len);
            break;
          }
          else
          {
            GCharReader gChar(thisServer->getRegionID(), rev.ret().targetid());
            if (!gChar.get())
            {
              XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送私聊失败" << rev.ret().targetid() << "不存在" << XEND;
              break;
            }
            if (gChar.checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true || gChar.checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true)
            {
              XERR << "[聊天管理-协议处理]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送私聊失败" << rev.ret().targetid() << "无聊天或黑名单关系" << XEND;
              break;
            }

            if (rev.to_global())
              thisServer->sendCmd(ClientType::global_server, buf, len);
          }
        }
      }
      return true;
    case SOCIALPARAM_CREATEGUILD:
      {
        PARSE_CMD_PROTOBUF(CreateGuildSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.user().user().charid());
        if (pUser != nullptr)
          pUser->toData(rev.mutable_user());
        else
          rev.set_msgid(10);
        PROTOBUF(rev, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);
      }
      break;
    case SOCIALPARAM_TOWER_SYNC_LEADERINFO:
    case SOCIALPARAM_TOWER_SYNC_LAYER:
      {
        thisServer->sendCmdToServer(buf, len, "TeamServer");
      }
      break;
    case SOCIALPARAM_GUILDDONATE:
    case SOCIALPARAM_GUILD_LEVELUP:
      thisServer->sendCmd(ClientType::guild_server, buf, len);
      break;
    case SOCIALPARAM_SYNC_REDTIP:
      {
        PARSE_CMD_PROTOBUF(SyncRedTipSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          PROTOBUF(rev, send, len);
          if((rev.red() == EREDSYS_GUILD_APPLY || rev.red() == EREDSYS_GUILD_ICON /*|| rev.red() == EREDSYS_GUILD_CHALLENGE_ADD*/ || rev.red() == EREDSYS_GUILD_CHALLENGE_REWARD) && rev.dwid() == 0)
            thisServer->sendCmd(ClientType::guild_server, send, len);
          //else if(rev.red() == EREDSYS_SOCIAL_FRIEND_APPLY || rev.red() == EREDSYS_TEAMAPPLY)
          else
            pUser->sendCmdToScene(send, len);
        }
      }
      break;
    case SOCIALPARAM_TOWER_SYNC_SCENEINFO:
      thisServer->sendCmdToServer(buf, len, "TeamServer");
      break;
    case SOCIALPARAM_DOJO_CREATE:
      {
        PARSE_CMD_PROTOBUF(DojoCreateSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          return false;

        const SDojoItemCfg* pCfg = DojoConfig::getMe().getDojoItemCfg(rev.dojoid());
        if (pCfg == nullptr)
        {
          return false;
        }
        const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(pCfg->dwRapid);
        if (pBase == nullptr || pBase->eRestrict != ERAIDRESTRICT_GUILD_TEAM)
          return false;
        CreateRaidMapSessionCmd cmd;
        RaidMapData* pData = cmd.mutable_data();
        pData->set_raidid(pCfg->dwRapid);
        pData->set_charid(pUser->id);
        pData->set_dojoid(rev.dojoid());
        pData->set_teamid(rev.teamid());
        pData->set_restrict(static_cast<DWORD>(pBase->eRestrict));
        pData->mutable_guildinfo()->set_id(rev.guildid());
        SessionSceneManager::getMe().createRaidMap(cmd);
        return true;
      }
    case SOCIALPARAM_TOWER_SCENE_CREATE:
      {
        PARSE_CMD_PROTOBUF(TowerSceneCreateSocialCmd, rev);
        SessionUser* pLeader = SessionUserManager::getMe().getUserByID(rev.user().charid());
        if (pLeader == nullptr)
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_state(EDOJOSTATE_CLOSE);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "TeamServer");
          break;
        }
        pLeader->sendCmdToScene(buf, len);

        /*const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(rev.layer());
        if (pCFG == nullptr)
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_state(EDOJOSTATE_CLOSE);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "TeamServer");
          break;
        }
        const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(pCFG->dwRaidID);
        if (pRaidCFG == nullptr)
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_state(EDOJOSTATE_CLOSE);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "TeamServer");
          break;
        }

        CreateRaidMapSessionCmd cmd;
        RaidMapData* pData = cmd.mutable_data();
        pData->set_raidid(pCFG->dwRaidID);
        pData->set_charid(rev.user().charid());
        pData->set_teamid(rev.teamid());
        pData->set_restrict(static_cast<DWORD>(pRaidCFG->eRestrict));
        pData->set_layer(rev.layer());
        SessionSceneManager::getMe().createRaidMap(cmd);*/
      }
      break;
    case SOCIALPARAM_CREATE_PVECARDRAID:
      {
        PARSE_CMD_PROTOBUF(CardSceneCreateSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.userid());
        if (pUser == nullptr)
        {
          // 队长下线了
          CardSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_open(false);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "TeamServer");
          break;
        }
        const SPveRaidCFG* pPveRaidCFG = PveCardConfig::getMe().getPveRaidCFGByID(rev.configid());
        if (pPveRaidCFG == nullptr)
          return false;

        const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(pPveRaidCFG->dwRaidID);
        if (pBase == nullptr)
          return false;
        CreateRaidMapSessionCmd cmd;
        RaidMapData* pData = cmd.mutable_data();
        pData->set_raidid( pPveRaidCFG->dwRaidID);
        pData->set_charid(pUser->id);
        pData->set_teamid(rev.teamid());
        pData->set_restrict(static_cast<DWORD>(pBase->eRestrict));
        pData->set_roomid(rev.configid());//PveRaid配置ID
        SessionSceneManager::getMe().createRaidMap(cmd);
      }
      break;
    case SOCIALPARAM_CREATE_TEAMRAID:
      {
        PARSE_CMD_PROTOBUF(TeamRaidSceneCreateSocialCmd, rev);
        SessionUser* pLeader = SessionUserManager::getMe().getUserByID(rev.user().charid());
        bool bActivityOpen = true;
        if(rev.raid_type() == ERAIDTYPE_ALTMAN)
        {
          bActivityOpen = SessionActivityMgr::getMe().isOpen(GACTIVITY_ALTMAN);
        }
        if (pLeader == nullptr || bActivityOpen == false)
        {
          TeamRaidSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_open(false);
          cmd.set_raid_type(rev.raid_type());
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "TeamServer");
          return false;
        }

        DWORD dwRaidID = MiscConfig::getMe().getRaidByType(static_cast<ERaidType>(rev.raid_type()));
        const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(dwRaidID);
        if (pRaidCFG == nullptr)
        {
          XERR << "[队伍副本]" << pLeader->accid << pLeader->id << pLeader->getProfession() << pLeader->name
            << "创建主队副本" << rev.raid_type() << "失败,未找到副本配置" << dwRaidID << XEND;
          return false;
        }

        CreateRaidMapSessionCmd cmd;
        RaidMapData* pData = cmd.mutable_data();
        pData->set_raidid( pRaidCFG->dwRaidID);
        pData->set_charid(pLeader->id);
        pData->set_teamid(rev.teamid());
        pData->set_restrict(static_cast<DWORD>(pRaidCFG->eRestrict));
        SessionSceneManager::getMe().createRaidMap(cmd);
        XLOG << "[队伍副本]" << pLeader->accid << pLeader->id << pLeader->getProfession() << pLeader->name
          << "创建队伍副本" << rev.raid_type() << "raidid: " << dwRaidID << XEND;
      }
      break;
    case SOCIALPARAM_GO_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(GoTeamRaidSocialCmd, rev);
        if (rev.raidzoneid())
        {
          SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser)
          {
            pUser->sendCmdToScene(buf, len);
          }
        }
        else
        {
          sendCmdToServer(buf, len, "TeamServer");
        }
        return true;
      }
      break;
    case SOCIALPARAM_TEAM_SEAL_FIHISH:
      {
        sendCmdToServer(buf, len, "TeamServer");
      }
      break;
    case SOCIALPARAM_SOCIAL_DATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialDataUpdateSocialCmd, rev);
        if (rev.to_global() == false)
          sendCmdToServer(buf, len, "SocialServer");
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;
    case SOCIALPARAM_SOCIAL_ADDRELATION:
      {
        PARSE_CMD_PROTOBUF(AddRelationSocialCmd, rev);
        if (rev.to_global() == false)
          sendCmdToServer(buf, len, "SocialServer");
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;
    case SOCIALPARAM_SOCIAL_REMOVERELATION:
      {
        PARSE_CMD_PROTOBUF(RemoveRelationSocialCmd, rev);
        if (rev.to_global() == false)
          sendCmdToServer(buf, len, "SocialServer");
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;
    /*case SOCIALPARAM_SOCIAL_REMOVEFOCUS:
      {
        PARSE_CMD_PROTOBUF(RemoveFocusSocialCmd, rev);
        if (rev.to_global() == false)
          sendCmdToServer(buf, len, "SocialServer");
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;*/
    case SOCIALPARAM_SOCIAL_REMOVESOCIAL:
      {
        PARSE_CMD_PROTOBUF(RemoveSocialitySocialCmd, rev);
        if (rev.to_global() == false)
          sendCmdToServer(buf, len, "SocialServer");
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;
    case SOCIALPARAM_SOCIAL_SYNC_LIST:
      {
        PARSE_CMD_PROTOBUF(SyncSocialListSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[社交消息-社交列表同步] charid :" << rev.charid() << "玩家不存在" << XEND;
          break;
        }
        pUser->getSocial().initSocial(rev);
        pUser->sendCmdToScene(buf, len);

        ChatManager_SE::getMe().sendAndDelMsg(pUser);
      }
      break;
    case SOCIALPARAM_SOCIAL_UPDATE_RELATIONTIME:
      thisServer->sendCmdToServer(buf, len, "SocialServer");
      break;
    case SOCIALPARAM_SOCIAL_LIST_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialListUpdateSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[社交消息-社交列表更新] charid :" << rev.charid() << "玩家不存在" << XEND;
          break;
        }

        GSocial& rSocial = pUser->getSocial();

        TSetQWORD setBlackIDs;
        rSocial.collectRelation(ESOCIALRELATION_BLACK, setBlackIDs);
        rSocial.collectRelation(ESOCIALRELATION_BLACK_FOREVER, setBlackIDs);

        pUser->getSocial().updateSocial(rev);

        TSetQWORD setNewBlackIDs;
        rSocial.collectRelation(ESOCIALRELATION_BLACK, setNewBlackIDs);
        rSocial.collectRelation(ESOCIALRELATION_BLACK_FOREVER, setNewBlackIDs);

        if (setBlackIDs.size() != setNewBlackIDs.size())
        {
          TutorBlackUpdateMatchSCmd scmd;
          scmd.set_charid(pUser->id);
          for (auto &s : setNewBlackIDs)
            scmd.add_blackids(s);
          PROTOBUF(scmd, ssend, slen);
          thisServer->sendCmd(ClientType::match_server, ssend, slen);
        }

        pUser->sendCmdToScene(buf, len);
        thisServer->sendCmdToServer(buf, len, "TeamServer");
      }
      break;
    case SOCIALPARAM_ADD_OFFLINEMSG:
      {
        PARSE_CMD_PROTOBUF(AddOfflineMsgSocialCmd, rev);
        if (rev.msg().type() == EOFFLINEMSG_TRADE)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().itemid(), rev.msg().price(), rev.msg().count(), rev.msg().givemoney(), rev.msg().moneytype());
        else if (rev.msg().type() == EOFFLINEMSG_SYS)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().id(), rev.msg().sysstr());
        else if (rev.msg().type() == EOFFLINEMSG_SYS2)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().syscmd());
        else if (rev.msg().type() == EOFFLINEMSG_GM)
          ChatManager_SE::getMe().addOfflineGmMsg(rev.msg().targetid(), rev.msg().gmcmd());
        else if (rev.msg().type() == EOFFLINEMSG_USER)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().chat());
        else if(rev.msg().type() == EOFFLINEMSG_ADD_RELATION)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().senderid(), rev.msg().type(), static_cast<ESocialRelation>(rev.msg().itemid()));
        else if (rev.msg().type() == EOFFLINEMSG_ADD_ITEM)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().itemdata());
        else if (rev.msg().type() == EOFFLINEMSG_TUTOR_REWARD)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().tutorreward());
        else if (rev.msg().type() == EOFFLINEMSG_ADD_RELATION || rev.msg().type() == EOFFLINEMSG_REMOVE_RELATION)// || rev.msg().type() == EOFFLINEMSG_REMOVE_FOCUS)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().senderid(), rev.msg().type(), static_cast<ESocialRelation>(rev.msg().itemid()));
        else if (rev.msg().type() == EOFFLINEMSG_USER_ADD_ITEM)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().useradditem());
        else if (rev.msg().type() == EOFFLINEMSG_WEDDING)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().weddingmsg());
        else if (rev.msg().type() == EOFFLINEMSG_USER_QUOTA)
          ChatManager_SE::getMe().addOfflineMsg(rev.msg().targetid(), rev.msg().quotadata());

      }
      break;
    case SOCIALPARAM_TEAM_QUEST_UPDATE:
      {
        thisServer->sendCmdToServer(buf, len, "TeamServer");
      }
      break;
    case SOCIALPARAM_GLOBAL_FORWARD_CMD:
      {
        PARSE_CMD_PROTOBUF(GlobalForwardCmdSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
          if (rev.dir() == EDir_ToScene)
            pUser->sendCmdToScene(rev.data().c_str(), rev.len());
          else
            pUser->sendCmdToMe(rev.data().c_str(), rev.len());
        else
          thisServer->sendCmd(ClientType::global_server, buf, len);
      }
      break;
    case SOCIALPARAM_SYNC_TUTOR_REWARD:
      {
        PARSE_CMD_PROTOBUF(SyncTutorRewardSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          pUser->sendCmdToScene(buf, len);
        }
        else
        {
          if (rev.searchuser())
            thisServer->sendCmd(ClientType::global_server, buf, len);
          else if (rev.has_reward())
            ChatManager_SE::getMe().addOfflineMsg(rev.charid(), rev.reward());
        }
      }
      break;
    case SOCIALPARAM_MODIFY_DEPOSIT:
      {
        PARSE_CMD_PROTOBUF(ModifyDepositSocialCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[社交消息-修改充值数据]" << "玩家:" << rev.charid() << "找不到" << XEND;
          return false;
        }
        xLuaData param;
        if (param.fromJsonString(rev.command()) == false)
        {
          XERR << "[社交消息-修改充值数据]" << pUser->accid << pUser->id << pUser->name << "命令:" << rev.command() << "命令解析失败" << XEND;
          return false;
        }
        const string& act = param.getTableString("action");
        if (act == "reset")
          pUser->resetDepositVirgin(param.getTableInt("id"), param.getTableInt("tag"));
        else if (act == "set")
          pUser->setDepositCount(param.getTableInt("id"), param.getTableInt("count"));
        else if (act == "limit")
          pUser->setDepositLimit(param.getTableInt("id"), param.getTableInt("count"));
        else if (act == "close")
          pUser->setDepositVirgin(param.getTableInt("id"), param.getTableInt("tag"));
        else
          XERR << "[社交消息-修改充值数据]" << pUser->accid << pUser->id << pUser->name << "命令:" << rev.command() << "action非法" << XEND;
      }
      break;
    default:
      return false;
      break;
  }

  return true;
}

bool SessionServer::doGmToolsCmd(xNetProcessor* np, const BYTE* buf, WORD len)
{
  if (!np || !buf || !len)
    return false;

  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case EXEC_GM_CMD:
      {
        PARSE_CMD_PROTOBUF(ExecGMCmd, message);
        const TVecString& vecResult = GMCommandManager::getMe().processGMCommand(message);

        RetExecGMCmd cmd;
        cmd.set_ret(vecResult[EJSONRET_MESSAGE]);
        cmd.set_data(vecResult[EJSONRET_DATA]);
        cmd.set_conid(message.conid());
        PROTOBUF(cmd, send, len);
        np->sendCmd(send, len);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SessionServer::doLogCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case Cmd::QUERY_CHAT_LOG_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::QueryChatLogCmd, rev);
        XINF << "[聊天记录-查询] 协议打印：" << rev.ShortDebugString() << XEND;
        break;
      }
    default:
      break;
  }
  return true;
}

bool SessionServer::doClientCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case RECONNECT_CLIENT_USER_CMD:
      {
        ReconnectClientUserCmd *rev = (ReconnectClientUserCmd *)cmd;
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser)
        {
          pUser->sendCmdToMe(buf, len);
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool SessionServer::doTeamCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case SERVERTEAMPARAM_UPDATE_GUILD:
      sendCmdToServer(buf, len, "TeamServer");
      break;
    case SERVERTEAMPARAM_TEAMDATA_SYNC:
      {
        PARSE_CMD_PROTOBUF(TeamDataSyncTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          QWORD qwOldTeamID = pUser->getTeam().getTeamID();
          pUser->getTeam().updateTeam(rev);
          pUser->sendCmdToScene(buf, len);

          if (qwOldTeamID == 0 && pUser->getTeam().getTeamID() != 0)
            pUser->onEnterTeam(pUser->getTeamID());
          else if (qwOldTeamID != 0 && pUser->getTeam().getTeamID() == 0)
            pUser->onLeaveTeam(qwOldTeamID);
          if (pUser->getTeamOnline())
          {
            pUser->setTeamOnline(false);
            SealManager::getMe().onUserOnline(pUser);
          }
          else
          {
            if (qwOldTeamID == 0 && pUser->getTeamID() != 0)
              SealManager::getMe().addMember(pUser->getTeamID(), pUser->id);
          }
          SessionScene *pScene = SessionSceneManager::getMe().getSceneByID(pUser->getSceneID());
          if (pScene)
          {
            if (!pScene->canEnter(pUser))
            {
              Cmd::GoBackSessionCmd message;
              message.set_charid(pUser->id);
              PROTOBUF(message, send, len);
              pUser->getScene()->sendCmd(send, len);
            }
          }
        }
      }
      break;
    case SERVERTEAMPARAM_TEAMDATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(TeamDataUpdateTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          pUser->getTeam().updateTeamData(rev);
          pUser->sendCmdToScene(buf, len);
        }
      }
      break;
    case SERVERTEAMPARAM_TEAMMEMBER_UPDATE:
      {
        PARSE_CMD_PROTOBUF(TeamMemberUpdateTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          TSetQWORD oldmembers;
          for (auto &m : pUser->getTeam().getTeamMemberList())
            oldmembers.insert(m.first);

          pUser->getTeam().updateMember(rev);
          pUser->sendCmdToScene(buf, len);

          TSetQWORD newmembers;
          for (auto &m : pUser->getTeam().getTeamMemberList())
          {
            if (oldmembers.find(m.first) != oldmembers.end())
            {
              oldmembers.erase(m.first);
              continue;
            }
            newmembers.insert(m.first);
          }
          for (auto &s : oldmembers)
            SealManager::getMe().removeMember(pUser->getTeamID(), s);
          for (auto &s : newmembers)
            SealManager::getMe().addMember(pUser->getTeamID(), s);
        }
      }
      break;
    case SERVERTEAMPARAM_MEMBERDATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(MemberDataUpdateTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          pUser->getTeam().updateMemberData(rev);
          pUser->sendCmdToScene(buf, len);
        }
      }
      break;
    case SERVERTEAMPARAM_BROADCAST_CMD:
      {
        PARSE_CMD_PROTOBUF(BroadcastCmdTeamCmd, rev);
        thisServer->broadcastOneLevelIndexCmd(static_cast<ONE_LEVEL_INDEX_TYPE>(rev.type()), rev.id(), rev.data().c_str(), rev.len());
      }
      break;
    case SERVERTEAMPARAM_CAT_ENTERTEAM:
    case SERVERTEAMPARAM_CAT_ENTEROWNTEAM:
      thisServer->sendCmdToServer(buf, len, "TeamServer");
      break;
    case SERVERTEAMPARAM_CAT_EXITTEAM:
      {
        PARSE_CMD_PROTOBUF(CatExitTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case SERVERTEAMPARAM_CAT_FIRE:
      sendCmdToServer(buf, len, "TeamServer");
      break;
    case SERVERTEAMPARAM_CAT_CALL:
      {
        PARSE_CMD_PROTOBUF(CatCallTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case SERVERTEAMPARAM_BE_LEADER:
      {
        PARSE_CMD_PROTOBUF(BeLeaderTeamCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->sendCmdToScene(buf, len);
      }
      break;
    default:
      return false;
  }
  return true;
}

bool SessionServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case GUILDSPARAM_LOAD_LUA:
      sendCmd(ClientType::guild_server, buf, len);
      break;
    case GUILDSPARAM_GUILD_SYNC_USERINFO:
      {
        PARSE_CMD_PROTOBUF(GuildUserInfoSyncGuildCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;

        pUser->updateUserInfo(rev);
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(GuildInfoSyncGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateGuild(rev);

        GCharReader* pGChar = pUser->getGCharData();
        if (pGChar != nullptr && pUser->getGuild().name() != pGChar->getGuildName())
        {
          pGChar->setGuildID(pUser->getGuild().id());
          pGChar->setGuildName(pUser->getGuild().name());

          UserGuildInfoSyncGuildSCmd cmd;
          cmd.set_charid(pUser->id);
          cmd.set_guildid(pUser->getGuild().id());
          cmd.set_guildname(pUser->getGuild().name());
          cmd.set_guildportrait(pUser->getGuild().portrait());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "SocialServer");
        }

        {
          UpdateGuildServerTeamCmd cmd;
          cmd.set_charid(pUser->id);
          cmd.set_guildid(pUser->getGuild().id());
          cmd.set_guildname(pUser->getGuild().name());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
        }
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_UPDATE_GUILDDATA:
      {
        PARSE_CMD_PROTOBUF(GuildDataUpdateGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateGuildData(rev);
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_UPDATE_CITYDATA:
      {
        PARSE_CMD_PROTOBUF(CityDataUpdateGuildSCmd, rev);
        SessionGvg::getMe().updateCityInfoFromGuild(rev);
        thisServer->sendCmdToAllScene(buf, len);
      }
      break;
    case GUILDSPARAM_UPDATE_MEMBER:
      {
        PARSE_CMD_PROTOBUF(GuildMemberUpdateGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateMember(rev);
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_UPDATE_MEMBERDATA:
      {
        PARSE_CMD_PROTOBUF(GuildMemberDataUpdateGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateMemberData(rev);
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_UPDATE_QUEST:
      {
        PARSE_CMD_PROTOBUF(GuildQuestUpdateGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateQuest(rev);
        pUser->sendCmdToScene(buf, len);
      }
      break;
    case GUILDSPARAM_REFRESH_TERRITORY:
      {
        PARSE_CMD_PROTOBUF(RefreshGuildTerritoryGuildSCmd, rev);
        SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.info().id());
        if (pScene == nullptr)
          break;
        rev.set_sceneid(pScene->id);
        PROTOBUF(rev, send, len);
        pScene->sendCmd(send, len);
      }
      break;
    case GUILDSPARAM_FRAME_UPDATE:
      thisServer->sendCmd(ClientType::guild_server, buf, len);
      break;
    case GUILDSPARAM_QUERY_PHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryPhotoListGuildSCmd, rev);
        if (rev.result() == true)
        {
          SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
          if (pScene != nullptr)
          {
            rev.set_sceneid(pScene->id);
            PROTOBUF(rev, nsend, nlen);
            pScene->sendCmd(nsend, nlen);
          }
        }
        else
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
        }
      }
      break;
    case GUILDSPARAM_QUERY_USERPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryUserPhotoListGuildSCmd, rev);
        if (rev.result() == true)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.user().charid());
          if (pUser != nullptr)
            pUser->sendCmdToScene(buf, len);
        }
        else
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
        }
      }
      break;
    case GUILDSPARAM_QUERY_SHOWPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryShowPhotoGuildSCmd, rev);
        if (rev.action() == EPHOTOACTION_LOAD_FROM_RECORD)
          thisServer->sendCmdToData(buf, len);
        else if (rev.action() == EPHOTOACTION_LOAD_FROM_SCENE)
        {
          if (rev.members_size() <= 0)
          {
            XERR << "[公会消息-加载照片]" << rev.ShortDebugString() << "未处理,没有成员" << XEND;
            break;
          }
          SessionUser* pUser = SessionUserManager::getMe().getUserByAccID(rev.members(0));
          if (pUser == nullptr)
          {
            XERR << "[公会消息-加载照片]" << rev.ShortDebugString() << "未处理,成员" << rev.members(0) << "不在线" << XEND;
            break;
          }
          pUser->sendCmdToScene(buf, len);
        }
        else if (rev.action() == EPHOTOACTION_UPDATE_FROM_RECORD || rev.action() == EPHOTOACTION_UPDATE_FROM_SCENE)
          thisServer->sendCmd(ClientType::guild_server, buf, len);
        else
          XERR << "[公会消息]" << rev.ShortDebugString() << "未处理" << XEND;
      }
      break;
    case GUILDSPARAM_PHOTO_UPDATE:
      {
        PARSE_CMD_PROTOBUF(PhotoUpdateGuildSCmd, rev);
        if (rev.to_guild() == true)
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
        }
        else
        {
          SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
          if (pScene != nullptr)
          {
            rev.set_sceneid(pScene->id);
            PROTOBUF(rev, nsend, nlen);
            pScene->sendCmd(nsend, nlen);
          }
        }

        return true;
      }
      break;
    case GUILDSPARAM_JOB_UPDATE:
      {
        PARSE_CMD_PROTOBUF(JobUpdateGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateJob(rev);
        pUser->sendCmdToScene(buf, len);

        return true;
      }
      break;
    case GUILDSPARAM_GUILD_MUSIC_DELETE:
      {
        PARSE_CMD_PROTOBUF(GuildMusicDeleteGuildSCmd, rev);
        GuildMusicDeleteRecordCmd cmd;
        cmd.set_guildid(rev.guildid());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToData(send, len);

        return true;
      }
      break;
    case GUILDSPARAM_RENAME_NTF:
      {
        thisServer->sendCmd(ClientType::guild_server, buf, len);

        return true;
      }
      break;
    case GUILDSPARAM_GUILD_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(GuildCityActionGuildSCmd, rev);
        if (rev.action() == EGUILDCITYACTION_TO_GUILD_UPDATE || rev.action() == EGUILDCITYACTION_TO_GUILD_SAVE || rev.action() == EGUILDCITYACTION_TO_GUILD_RESET)
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
          XDBG << "[公会城池-行为]" << rev.ShortDebugString() << "发送至GuildServer成功" << XEND;
        }
        else if (rev.action() == EGUILDCITYACTION_TO_RECORD_SAVE || rev.action() == EGUILDCITYACTION_GUILD_QUERY)
        {
          thisServer->sendCmdToData(buf, len);
          XDBG << "[公会城池-行为]" << rev.ShortDebugString() << "发送至DataServer成功" << XEND;
        }
        else if (rev.action() == EGUILDCITYACTION_TO_SCENE_UPDATE)
        {
          SessionGvg::getMe().updateCityInfoFromGuild(rev);
          if (rev.scenename().empty() == false)
          {
            ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
            if (net == nullptr)
            {
              XERR << "[公会城池-行为]" << rev.ShortDebugString() << "发送至" << rev.scenename() << "失败" << XEND;
              break;
            }
            net->sendCmd(buf, len);
            XDBG << "[公会城池-行为]" << rev.ShortDebugString() << "发送至" << rev.scenename() << "成功" << XEND;
          }
          else
          {
            thisServer->sendCmdToAllScene(buf, len);
            XDBG << "[公会城池-行为]" << rev.ShortDebugString() << "发送至SceneServer成功" << XEND;
          }
        }
        else
        {
          XERR << "[公会城池-行为]" << rev.ShortDebugString() << "失败,无法处理该action" << XEND;
        }
      }
      break;
    case GUILDSPARAM_UPDATE_CITYSTATE:
      {
        PARSE_CMD_PROTOBUF(UpdateCityStateGuildSCmd, rev);
        SessionGvg::getMe().updateCityState(rev);
      }
      break;
    case GUILDSPARAM_OPEN_GVG:
      {
        PARSE_CMD_PROTOBUF(GvgOpenToServerGuildSCmd, rev);
        SessionGvg::getMe().setFireStatus(rev.fire());
      }
      break;
    case GUILDSPARAM_GVG_RESULT:
      {
        thisServer->sendCmdToData(buf, len);
        XDBG << "[公会战结果统计], 发送到DataServer成功" << XEND;
      }
      break;
    case GUILDSPARAM_SEND_MAIL:
      {
        PARSE_CMD_PROTOBUF(SendMailGuildSCmd, message);
        SendMail cmd;
        cmd.mutable_data()->CopyFrom(message.data());
        bool ret = MailManager::getMe().sendMail(cmd);
        XLOG << "[邮件] guild邮件消息 处理结果"<< ret <<"msg" << message.ShortDebugString() << XEND;
        return ret;
      }
      break;
    case GUILDSPARAM_GVG_REWARD:
    case GUILDSPARAM_GUILDPRAY:
    case GUILDSPARAM_GVG_USER_PARTIN:
    case GUILDSPARAM_JOINSUPERGVG_REQ:
    case GUILDSPARAM_SUPERGVG_END:
    case GUILDSPARAM_GUILD_MAIL:
    case GUILDSPARAM_GUILD_MSG:
      thisServer->sendCmd(ClientType::guild_server, buf, len);
      break;
    case GUILDSPARAM_SUBMIT_MATERIAL:
    case GUILDSPARAM_SHOP_BUY_ITEM:
      thisServer->sendCmd(ClientType::guild_server, buf, len);
      break;
    case GUILDSPARAM_BUILDING_UPDATE:
      {
        PARSE_CMD_PROTOBUF(BuildingUpdateGuildSCmd, rev);
        if (rev.guildid())
        {
          SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
          if (pScene)
          {
            rev.set_sceneid(pScene->id);
            PROTOBUF(rev, send, len);
            pScene->sendCmd(send, len);
          }
        }
        if (rev.charid())
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser)
          {
            pUser->sendCmdToScene(buf, len);
          }
        }
      }
      break;
    case GUILDSPARAM_QUERY_GUILD_INFO:
      {
        PARSE_CMD_PROTOBUF(QueryGuildInfoGuildSCmd, rev);
        if (rev.result() == true)
        {
          SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
          if (pScene != nullptr)
          {
            rev.set_sceneid(pScene->id);
            PROTOBUF(rev, nsend, nlen);
            pScene->sendCmd(nsend, nlen);
          }
        }
        else
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
        }
      }
      break;
    case GUILDSPARAM_SEND_WELFARE:
      {
        PARSE_CMD_PROTOBUF(SendWelfareGuildSCmd, rev);
        bool send = false;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          send = pUser->sendCmdToScene(buf, len);
        }
        if (send == false)
        {
          XERR << "[公会-福利]" << rev.ShortDebugString() << "获取玩家或者发送消息到场景失败,将奖励以邮件方式发送给玩家" << XEND;
          TVecItemInfo items;
          for (int i = 0; i < rev.items_size(); ++i)
          {
            TVecItemInfo rollitem;
            if (RewardManager::roll(rev.items(i).rewardid(), nullptr, rollitem, rev.items(i).source()) == false)
            {
              XERR << "[公会-福利]" << rev.charid() << "奖励:" << rev.items(i).rewardid() << "来源:" << rev.items(i).source() << "获取失败" << XEND;
              continue;
            }
            combinItemInfo(items, rollitem);
          }
          if (items.empty() == false)
          {
            XLOG << "[公会-福利]" << rev.charid() << "保存到邮件,奖励:";
            for (auto& v : items)
              XLOG << v.id() << v.count() << v.source();
            if (MailManager::getMe().sendMail(rev.charid(), 0, "", SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_MAIL), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_COMPENSATE), EMAILTYPE_NORMAL, 0, items, TVecItemData{}, EMAILATTACHTYPE_ITEM))
              XLOG << "成功" << XEND;
            else
              XLOG << "失败" << XEND;
          }
          else
          {
            XERR << "[公会-福利]" << rev.charid() << "奖励为空:";
            for (int i = 0; i < rev.items_size(); ++i)
              XERR << rev.items(i).rewardid() << rev.items(i).source();
            XERR << XEND;
          }
        }
      }
      break;
    case GUILDSPARAM_CHALLENGE_PROGRESS:
      {
        thisServer->sendCmd(ClientType::guild_server, buf, len);
      }
      break;
    case GUILDSPARAM_GM_COMMAND:
      {
        PARSE_CMD_PROTOBUF(GMCommandGuildSCmd, rev);
        GuildGmMgr::getMe().add(rev);
      }
      break;
    case GUILDSPARAM_GM_RESPOND:
      {
        PARSE_CMD_PROTOBUF(GMCommandRespondGuildSCmd, rev);
        GuildGmMgr::getMe().respond(rev);
      }
      break;
    case GUILDSPARAM_BUILDINGEFFECT:
      {
        thisServer->sendCmd(ClientType::guild_server, buf, len);
      }
      break;
    case GUILDSPARAM_ARTIFACT_UPDATE:
      {
        PARSE_CMD_PROTOBUF(ArtifactUpdateGuildSCmd, rev);
        if (rev.charid())
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
          if (pUser)
          {
            pUser->getGuild().updateArtifact(rev);
            pUser->sendCmdToScene(buf, len);
          }
        }
        if (rev.guildid())
        {
          SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
          if (pScene)
          {
            rev.set_sceneid(pScene->id);
            PROTOBUF(rev, send, len);
            pScene->sendCmd(send, len);
          }
        }
      }
      break;
    case GUILDSPARAM_QUEST_ARTIFACT:
      {
        PARSE_CMD_PROTOBUF(GuildArtifactQuestGuildSCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          pUser->getGuild().updateGQuest(rev);
          pUser->sendCmdToScene(buf, len);
        }
      }
      break;
    case GUILDSPARAM_TREASURE_QUERY:
      {
        PARSE_CMD_PROTOBUF(QueryTreasureGuildSCmd, rev);
        if (rev.result() == false)
        {
          thisServer->sendCmd(ClientType::guild_server, buf, len);
          break;
        }

        SessionScene* pScene = SessionSceneManager::getMe().getSceneByGuildID(rev.guildid());
        if (pScene != nullptr)
        {
          rev.set_sceneid(pScene->id);
          PROTOBUF(rev, send, len);
          pScene->sendCmd(send, len);
        }
      }
      break;
    case GUILDSPARAM_TREASURE_RESULTNTF:
      thisServer->sendCmd(ClientType::guild_server, buf, len);
      break;
    default:
      return false;
  }
  return true;
}

bool SessionServer::doAuctionCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
  case Cmd::AUCTIONSPARAM_FORWARD_CCMD2AUCTION:       //auctionserver ->client
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardCCmd2Auction, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      XINF << "[拍卖行-转发拍卖服消息给客户端]" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
      return pUser->sendCmdToMe(rev.data().c_str(), rev.len());
    }
    else
    {
      XERR << "[拍卖行-转发拍卖服消息给客户端],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
    return true;
  }
  break;
  case Cmd::AUCTIONSPARAM_FORWARD_AUCTION2SCMD:       //auctionserver ->sceneserver
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardAuction2SCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser && pUser->getScene())
    {
      bool ret = pUser->getScene()->sendCmd(rev.data().c_str(), rev.len());
      XINF << "[拍卖行-转发拍卖服消息给场景]" << rev.charid() << "ret" << ret << rev.ShortDebugString().c_str() << XEND;
    }
    else
    {
      XERR << "[拍卖行-转发拍卖服消息给场景],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
    return true;
  }
  break;  
  case Cmd::AUCTIONSPARAM_FORWARD_SCMD2AUCTION:
  {
    //转发sceneServer -> AuctionServer
    PARSE_CMD_PROTOBUF(Cmd::ForwardSCmd2Auction, rev);
    bool ret = thisServer->sendCmd(ClientType::auction_server, buf, len);
    XDBG << "[拍卖行-消息转发] SceneServer->AuctionServer, ret" << ret <<rev.charid() << rev.name() << rev.ShortDebugString() << XEND;
    return true;
  }
  break;  
  case Cmd::AUCTIONSPARAM_WORLD_CMD:
  {
    PARSE_CMD_PROTOBUF(Cmd::WorldCmdSCmd, rev);
    
    MsgManager::sendWorldCmd(rev.data().c_str(), rev.len());
    XDBG << "[拍卖行-世界广播消息] msg" << rev.ShortDebugString() << XEND;
    return true;
  }
  break;
  case Cmd::AUCTIONSPARAM_BROADCASE_MSG_BYSESSION:
  {
    PARSE_CMD_PROTOBUF(Cmd::BroadcastMsgBySessionAuctionSCmd, rev);
    AuctionMgr::getMe().sendCmd2OpenUser(rev.data().c_str(), rev.len());
    XDBG << "[拍卖行-发送消息给打开界面的玩家] " << XEND;
    return true;
  }
  break;
  default:
    return false;
  }
  return true;
}

bool SessionServer::doWeddingCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
  case Cmd::WEDDINGSPARAM_FORWARD_WEDDING2C:       //weddingserver ->client
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardWedding2CSCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      XINF << "[婚礼-转发婚礼服消息给客户端]" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
      
      //auto f = [&](const BYTE* buf, WORD len)
      //{
      //  if (!buf || !len) return false;
      //  xCommand *cmd = (xCommand *)buf;

      //  switch (cmd->param)
      //  {
      //  case Cmd::WEDDINGCPARAM_NTF_RESERVE_WEDDINGDATE:
      //  {
      //    PARSE_CMD_PROTOBUF(Cmd::NtfReserveWeddingDateCCmd, rev);
      //    //todo test 同意
      //    ReplyReserveWeddingDateCCmd replyCmd;
      //    replyCmd.set_date(rev.date());
      //    replyCmd.set_configid(rev.configid());
      //    replyCmd.set_charid1(rev.charid1());
      //    replyCmd.set_reply(EReply_Agree);
      //    replyCmd.set_time(rev.time());
      //    replyCmd.set_use_ticket(rev.use_ticket());
      //    replyCmd.set_zoneid(rev.zoneid());
      //    replyCmd.set_sign(rev.sign());
      //    PROTOBUF(replyCmd, send, len);
      //    pUser->sendCmdToSceneUser(send, len);
      //  }
      //  }
      //  return true;
      //};

      //f((const BYTE*)rev.data().c_str(), rev.len());
        
      return pUser->sendCmdToMe(rev.data().c_str(), rev.len());
    }
    else
    {
      XERR << "[婚礼-转发婚礼服消息给客户端],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_FORWARD_WEDDING2S:       //weddingserver ->sceneserver
  {
    PARSE_CMD_PROTOBUF(Cmd::ForwardWedding2SSCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser && pUser->getScene())
    {
      bool ret = pUser->getScene()->sendCmd(rev.data().c_str(), rev.len());
      XINF << "[婚礼-转发婚礼服消息给场景]" << rev.charid() << "ret" << ret << rev.ShortDebugString().c_str() << XEND;
    }
    else
    {
      XERR << "[婚礼-转发婚礼服消息给场景],出错，找不到玩家" << rev.charid() << rev.ShortDebugString().c_str() << XEND;
    }
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_FORWARD_S2WEDDING:
  {
    //转发sceneServer -> weddingserver
    PARSE_CMD_PROTOBUF(Cmd::ForwardS2WeddingSCmd, rev);
    bool ret = thisServer->sendCmd(ClientType::wedding_server, buf, len);
    XDBG << "[婚礼-消息转发] SceneServer->WeddingServer, ret" << ret << rev.charid() << rev.name() << rev.ShortDebugString() << XEND;
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_FORWARD_C2WEDDING:
  {
    //转发sceneServer user cmd-> weddingserver
    PARSE_CMD_PROTOBUF(Cmd::ForwardC2WeddingSCmd, rev);
    bool ret = thisServer->sendCmd(ClientType::wedding_server, buf, len);
    XDBG << "[婚礼-消息转发-玩家消息] SceneServer->WeddingServer, ret" << ret << rev.charid() << rev.name() << rev.ShortDebugString() << XEND;
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_START_WEDDING:       
  {
    PARSE_CMD_PROTOBUF(Cmd::StartWeddingSCmd, rev);
    SessionWeddingMgr::getMe().startWedding(rev);
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_STOP_WEDDING:       
  {
    PARSE_CMD_PROTOBUF(Cmd::StopWeddingSCmd, rev);
    SessionWeddingMgr::getMe().stopWedding(rev);
    return true;
  }
  break;  
  case Cmd::WEDDINGSPARAM_BUY_SERVICE:
  {
    PARSE_CMD_PROTOBUF(Cmd::BuyServiceWeddingSCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      pUser->sendCmdToMe(buf, len);
    }
    else
    {
      rev.set_success(false);
      PROTOBUF(rev, send, len2);
      thisServer->sendCmd(ClientType::wedding_server, send, len2);
    }
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_SYNC_WEDDINGINFO:
  {
    PARSE_CMD_PROTOBUF(Cmd::SyncWeddingInfoSCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      pUser->setWeddingInfo(rev.weddinginfo());
      pUser->sendWeddingInfo2Scene();
    }
    return true;
  }
  case Cmd::WEDDINGSPARAM_UPDATE_MANUAL:
  case Cmd::WEDDINGSPARAM_MARRY_SUCCESS:
  {
    thisServer->sendCmdToAllScene(buf, len);
    return true;
  }
  case Cmd::WEDDINGSPARAM_CHECK_WEDDING_RESERVE:
  {
    PARSE_CMD_PROTOBUF(Cmd::CheckWeddingReserverSCmd, rev);
    SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
      pUser->getMail().getAttach(rev.mailid(), false, true, rev.result());
    return true;
  }
  case WEDDINGSPARAM_MISSYOU_INVITE:
  {
    PARSE_CMD_PROTOBUF(MissyouInviteWedSCmd, rev);
    SessionUser* pTarget = SessionUserManager::getMe().getUserByID(rev.charid());
    if (pTarget != nullptr)
    {
      pTarget->sendCmdToScene(buf, len);
      XLOG << "[婚姻-好想你]" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "被配偶邀请回到身边,在该线被找到,成功发送至SceneServer处理" << XEND;
    }
    else
    {
      thisServer->sendCmd(ClientType::global_server, buf, len);
      XLOG << "[婚姻-好想你]" << rev.charid() << "被配偶邀请回到身边,在该线未找到,成功发送至GlobalServer处理" << XEND;
    }
  }
  return true;
  default:
    return false;
  }
  return true;
}

bool SessionServer::doBossSCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
    case BOSSSPARAM_DEADBOSS_OPEN:
      {
        PARSE_CMD_PROTOBUF(DeadBossOpenBossSCmd, rev);
        SessionUserManager::getMe().updateGlobalBoss(rev.info());
      }
      break;
    case BOSSSPARAM_DEADBOSS_SYNC:
      {
        PARSE_CMD_PROTOBUF(DeadBossOpenSyncBossSCmd, rev);
        if (GlobalManager::getMe().updateGlobalBoss(rev.info()) == false)
          break;
        BossList::getMe().onBossOpen();
        SessionUserManager::getMe().syncGlobalBossScene(nullptr);
      }
      break;
    case BOSSSPARAM_BOSS_DIE:
      {
        PARSE_CMD_PROTOBUF(BossDieBossSCmd, message);
        BossList::getMe().onBossDie(message.npcid(), message.killer().c_str(), message.killid(), message.mapid(), message.reset());
      }
      break;
    case BOSSSPARAM_WORLD_NTF:
      {
        PARSE_CMD_PROTOBUF(WorldBossNtfBossSCmd, rev);
        BossList::getMe().updateWorldBoss(rev);
      }
      break;
    case BOSSSPARAM_BOSS_SET:
      {
        BossList::getMe().callSetFunc();
      }
      break;
    default:
      return false;
  }

  return true;
}


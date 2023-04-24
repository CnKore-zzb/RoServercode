#include "SessionServer.h"
#include "SessionUser.h"
#include "SessionUserManager.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "MsgManager.h"
#include "xNetProcessor.h"
#include "RegCmd.h"
#include "SessionCmd.pb.h"
#include "TableManager.h"
#include "MiscConfig.h"
#include "SessionScene.h"
#include "TowerManager.h"
#include "config/ConfigManager.h"
#include "MailManager.h"
#include "LuaManager.h"
#include "GuidManager.h"
#include "PlatLogManager.h"
#include "FerrisWheelManager.h"
#include "SessionActivityMgr.h"
#include "StatisticsDefine.h"
#include "SealManager.h"
#include "RecordCmd.pb.h"
#include "SystemCmd.pb.h"
#include "ChatManager_SE.h"
#include "TeamCmd.pb.h"
#include "DateLandConfig.h"
#include "ActivityManager.h"
#include "WorldLevelManager.h"
#include "Boss.h"

extern xLog srvLog;

void RegMapErr(DWORD mapid,std::string mapname,ServerTask* net,std::string log)
{
  XERR << "[场景地图注册]" << mapid << mapname << net->getName() << "注册失败" << log << XEND;
  //Cmd::Session::SessionRegMapFailCmd send;
  //send.mapID = mapid;
  RegMapFailSessionCmd cmd;
  cmd.set_mapid(mapid);
  PROTOBUF(cmd, send, len);
  net->sendCmd(send, len);
}

/*bool SessionServer::doSessionSceneCmd(const BYTE* buf, WORD len)
{
  using namespace Cmd;
  using namespace Cmd::Session;
  Session::SceneSessionCmd* cmd = (SceneSessionCmd *)buf;

  switch(cmd->param)
  {
    case FORWARD_CMD_TO_USER_SCENESERVER_SCENESESSION_CMD:
      {
        ForwardCmdToUserSceneServerSessionCmd *rev = (ForwardCmdToUserSceneServerSessionCmd *)cmd;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(buf, len);
        }
        return true;
      }
      break;
    case FORWARD_USER_SCENE_CMD_SCENESESSION_CMD:
      {
        ForwardUserSceneCmdSceneSessionCmd *rev = (ForwardUserSceneCmdSceneSessionCmd *)cmd;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(buf, len);
        }
        return true;
      }
      break;
    case FORWARD_USER_CMD_SCENESESSION_CMD:
      {
        ForwardUserCmdSceneSessionCmd *rev = (ForwardUserCmdSceneSessionCmd *)cmd;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (pUser)
        {
          pUser->sendCmdToMe(rev->data, rev->len);
        }

        return true;
      }
      break;
    case CHANGE_SCENE_SCENESESSION_CMD:
      {
        ChangeSceneCmd *rev = (ChangeSceneCmd *)cmd;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev->userID);
        SessionScene *scene = SessionSceneManager::instance()->getSceneByID(rev->mapID);
        if (pUser)
        {
          if (scene && pUser->gate_task() && USER_STATE_CHANGE_SCENE==pUser->getUserState())
          {
            XLOG("[跨服切换场景],%llu,%s,新场景登录,dest:%s(%u),sour:%s(%u)",
                pUser->id, pUser->name, scene->name, scene->id,
                pUser->getScene()?pUser->getScene()->name:"", pUser->getScene()?pUser->getScene()->id:0);
            pUser->setScene(scene);
            Cmd::Session::NotifyLoginSceneSessionCmd send;
            send.id = rev->userID;
            strncpy(send.name, pUser->name, MAX_NAMESIZE);
            send.accID = pUser->accid;
            strncpy(send.gateName, pUser->gate_task()->getName(), MAX_NAMESIZE);
            send.isChangeScene = 1;
            scene->sendCmd(&send, sizeof(send));
          }
          else
          {
            XERR("[跨服切换场景],%llu,%llu,%s,切换场景失败:%u,sour:%u", pUser->accid, pUser->id, pUser->name, rev->mapID,pUser->getScene()?pUser->getScene()->id:0);
            if (pUser->gate_task())
            {
              
                 LoginErrRegCmd send;
                 send.accid = pUser->accid;
                 send.zoneID = pUser->getZoneID();
                 send.ret = ERR_NOTIFY_SCENE;
                 pUser->gate_task()->sendCmd(&send, sizeof(send));
                 
            }
            pUser->userOffline();
            SessionUserManager::getMe().onUserQuit(pUser);
          }
        }
        return true;
      }
      break;
    case SCENE_REG_SCENESESSION_CMD:
      {
        SceneRegCmd* rev = (SceneRegCmd*)cmd;
        ServerTask *net = getConnectedServer("SceneServer", rev->sceneName);
        if (!net) return true;
        SessionScene *scene = SessionSceneManager::getMe().getSceneByID(rev->id);
        if (scene)
        {
          RegMapErr(rev->id,rev->name,net,"重复注册");
          return true;
        }
        //				if(rev->sID)
        //          scene = NEW SessionDyanScene(rev->id, rev->name, net,rev->sID);
        //          else
        scene = NEW SessionScene(rev->id, rev->name, net);
        if (!scene)
        {
          RegMapErr(rev->id,rev->name,net,"new失败");
          //					if(rev->sID)
          //            {
          //            SessionSceneManager::getMe().putDMapIndex(rev->sID,(rev->id&REAL_MAP_ID_MASK));
          //            SessionSceneManager::getMe().setCreateDMap(false,config?config->type:0,config?config->condition:0,user,rev->septid);
          //            }
          return true;
        }
        if (!SessionSceneManager::getMe().addScene(scene))
        {
          RegMapErr(rev->id,rev->name,net,"添加管理器失败");
          SAFE_DELETE(scene);
          return true;
        }
        if (!scene->init())
        {
          RegMapErr(rev->id,rev->name,net,"初始化失败");
          SessionSceneManager::getMe().delScene(scene);
          SAFE_DELETE(scene);
          return true;
        }
        SessionRegMapOKCmd send;
        send.mapID = scene->id;
        net->sendCmd(&send, sizeof(send));
        XLOG("[场景地图注册],%s(%u),注册成功 %s.", scene->name, scene->id, net->getName());

        BossList::getMe().onSceneOpen(scene);
        return true;
      }
      break;
    case USER_DATA_SCENESESSIONCMD:
      {
        UserDataSceneSessionCmd *rev = (UserDataSceneSessionCmd*)cmd;
        SessionUser *user = SessionUserManager::getMe().getUserByID(rev->data.id);
        if(!user)
        {
          ErrSetUserDataSceneSessionCmd send;
          send.id = rev->data.id;
          sendCmdToServer(&send, sizeof(send), "SceneServer", rev->sceneName);
          XERR("[登录],%u,0,不在管理器中", rev->data.id);
          return true;
        }

        if (user->getUserState() != USER_STATE_CHANGE_SCENE)
        {
          //user->initDataFromScene(rev->data);
          user->userOnline();
        }

        user->setUserState(USER_STATE_RUN);

        XLOG("[登录],%llu,%llu,%s,注册成功,加入%s,当前在线人数:%u", user->accid, user->id, user->name,
            user->getScene()? user->getScene()->name : "未知地图", SessionUserManager::getMe().size());
        return true;
      }
      break;
    case  FORWARD_USER_SESSION_CMD_SCENESESSION_CMD:
      {
        ForwardUserSessionCmdSceneSessionCmd *rev = (ForwardUserSessionCmdSceneSessionCmd *)buf;
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(rev->charid);
        if (nullptr != pUser)
        {
          if (!pUser->doSessionUserCmd(rev->data, rev->len))
          {
            XERR("[消息],%llu,%s,处理错误 %u,%u", pUser->id, pUser->name, ((xCommand*)rev->data)->cmd, ((xCommand*)rev->data)->param);
            return false;
          }
        }
      }
      break;
    default:
      break;
  }
  return false;
}*/

bool SessionServer::doErrorUserCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case REG_ERR_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(RegErrUserCmd, message);

        SessionUser *pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (!pUser)
        {
          XERR << "[登录]," << message.charid() << ",0,登录失败消息,ret:" << message.ret() << XEND;
          return true;
        }

        switch (message.ret())
        {
          case REG_ERR_ENTER_SCENE:
          case REG_ERR_ACC_FORBID:
          case REG_ERR_FORBID_REG:
            {
              LoginOutRegCmd send;
              send.id = pUser->id;
              send.accid = pUser->accid;
              strncpy(send.gateName, pUser->getGateServerName(), MAX_NAMESIZE);
              processLoginOutRegCmd(&send, message.ret());
            }
            break;
          case REG_ERR_GET_USER_DATA:
          case REG_ERR_FIND_GATE ... REG_ERR_REQ_DATA_FROM_RECORD:
            {
              if (pUser->getUserState() != USER_STATE_RUN)
              {
                XERR << "[登录]," << pUser->accid << "," << pUser->id << "," << pUser->name << ",登录失败 error:" << message.ret() << XEND;
                pUser->userOffline();
                if (pUser->gate_task())
                {
                  loginErr(pUser->accid, message.ret(), pUser->gate_task());
                }
                SessionUserManager::getMe().delUser(pUser);
              }
            }
          default:
            break;
        }

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool SessionServer::doSessionCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case LOAD_LUA_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(LoadLuaSessionCmd, message);
        bool bNtf = false;
        bool bRecordNtf = false;
        bool bSocialNtf = false;
        bool bTeamNtf = false;
        bool bOneZone = message.serverid() == thisServer->getZoneID();    //公共服之加载一次

        if (message.has_lua() == true)
        {
          LuaManager::getMe().reload();
          bNtf = bRecordNtf = bSocialNtf = bTeamNtf = true;
        }
        if (message.has_log() == true)
        {
          srvLog.reload();
          bNtf = bRecordNtf = bSocialNtf = bTeamNtf = true;
        }
        if (message.has_table())
        {
          string str = message.table();
          TVecConfigType vec;
          ConfigManager::getMe().getType(str, vec);
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isSessionLoad())
                ConfigManager::getMe().loadConfig(cfg);
              if (cfg.isSceneLoad())
                bNtf = true;
              if (cfg.isTypeLoad(RECORD_LOAD))
                bRecordNtf = true;
              if (cfg.isTypeLoad(SOCIAL_LOAD))
                bSocialNtf = true;
              if (cfg.isTypeLoad(TEAM_LOAD))
                bTeamNtf = true;
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isSessionLoad())
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }

        // send to scene
        if (bNtf)
        {
          thisServer->sendCmdToAllScene(buf, len);
        }

        // send to record
        if (bRecordNtf)
        {
          LoadLuaSceneRecordCmd cmd;
          cmd.set_lua(message.lua());
          cmd.set_table(message.table());
          cmd.set_log(message.log());
          PROTOBUF(cmd, send1, len1);
          thisServer->sendCmdToRecord(send1, len1);
        }
        // send to team
        if (bTeamNtf)
        {
          LoadLuaTeamCmd cmd;
          cmd.set_lua(message.lua());
          cmd.set_table(message.table());
          cmd.set_log(message.log());
          PROTOBUF(cmd, send1, len1);
          thisServer->sendCmdToServer(send1, len1, "TeamServer");
        }
        // send to social
        if (bSocialNtf)
        {
          LoadLuaTeamCmd cmd;
          cmd.set_lua(message.lua());
          cmd.set_table(message.table());
          cmd.set_log(message.log());
          PROTOBUF(cmd, send1, len1);
          thisServer->sendCmdToServer(send1, len1, "SocialServer");
        }

        if (message.load_type() != EComLoadType_None)
        {
            Cmd::CommonReloadSystemCmd cmd2;
            cmd2.set_type(message.load_type());
            PROTOBUF(cmd2, send2, len2);
            thisServer->sendCmdToGate(send2, len2);
            thisServer->sendCmdToRecord(send2, len2);
            thisServer->sendCmdToSuper(send2, len2);
            thisServer->sendCmdToAllScene(send2, len2);
            thisServer->sendCmdToServer(send2, len2, "TeamServer");
            thisServer->sendCmdToServer(send2, len2, "SocialServer");
            thisServer->commonReload(cmd2);

            if (bOneZone)
            {
              thisServer->sendCmd(ClientType::trade_server, send2, len2);
              thisServer->sendCmd(ClientType::stat_server, send2, len2);
              thisServer->sendCmd(ClientType::match_server, send2, len2);
              thisServer->sendCmd(ClientType::global_server, send2, len2);
              thisServer->sendCmd(ClientType::guild_server, send2, len2);
              thisServer->sendCmd(ClientType::gzone_server, send2, len2);
              thisServer->sendCmd(ClientType::match_server, send2, len2);
              thisServer->sendCmd(ClientType::auction_server, send2, len2);
            }
        }
        
        if (bOneZone)
        {
          PROTOBUF(message, send, len);
          thisServer->sendCmd(ClientType::auction_server, send, len);
          thisServer->sendCmd(ClientType::guild_server, send, len);
          thisServer->sendCmd(ClientType::match_server, send, len);
          thisServer->sendCmd(ClientType::wedding_server, send, len);
        }

        XINF << "[策划表-重加载] zoneid:" << thisServer->getZoneID() <<"source serverid"<<message.serverid()<< "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << "allzone:" << message.allzone() << XEND;
        return true;
      }
      break;
    case EXEC_GM_CMD_SESSIONCMD:
    {
      PARSE_CMD_PROTOBUF(ExecGMCmdSessionCmd, message);

      thisServer->sendCmdToAllScene(buf, len);
      XLOG << "[全线-GM] 发送scene处理" << message.gmcmd() << XEND;
      return true;
    }
    break;

    case GOTO_USER_MAP_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(GoToUserMapSessionCmd, message);

        SessionUser *pUser = SessionUserManager::getMe().getUserByID(message.targetuserid());
        if (pUser && pUser->getScene())
        {
          SessionUser *pMe = SessionUserManager::getMe().getUserByID(message.gotouserid());
          if (pMe)
          {
            ChangeSceneSessionCmd cmd;
            cmd.set_mapid(pUser->getScene()->id);
            pMe->changeScene(cmd);
          }
        }

        return true;
      }
      break;
    case DELETE_DMAP_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(DeleteDMapSessionCmd, message);

        SessionScene *scene = SessionSceneManager::getMe().getSceneByID(message.mapid());
        if (scene)
        {
          XLOG << "[动态地图],关闭," << scene->id << "," << scene->name << XEND;
          SessionSceneManager::getMe().delScene(scene);
        }

        return true;
      }
      break;
    case CREATE_RAIDMAP_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(CreateRaidMapSessionCmd, message);

        SessionSceneManager::getMe().createRaidMap(message);

        return true;
      }
      break;
    case CHANGE_SCENE_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(ChangeSceneSessionCmd, message);

        for (int i=0; i<message.charid_size(); ++i)
        {
          SessionUser *pUser = SessionUserManager::getMe().getUserByID(message.charid(i));
          if (pUser)
          {
            pUser->changeScene(message);
          }
          else
          {
            XERR << "[跨服切换场景] session 找不到玩家 charid" << message.charid(i) << XEND;
          }
        }

        return true;
      }
      break;
    case CMDPARAM_USER_DATA_SYNC:
      {
        PARSE_CMD_PROTOBUF(UserDataSync, message);
        USER_STATE eState = USER_STATE_NONE;
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.id());
        if (pUser != nullptr)
        {
          pUser->onDataChanged(message);
          eState = pUser->getUserState();
        }

        UserInfoSyncSocialCmd social;
        UserInfoSyncSocialCmd team;
        UserInfoSyncSocialCmd guild;
        SessionUser::fetchChangedData(message, social, team, guild);

        // sync social info
        if (eState != USER_STATE_LOGIN && social.info().datas_size() > 0)
        {
          PROTOBUF(social, send, len);
          thisServer->sendCmdToServer(send, len, "SocialServer");
        }

        // sync team info
        if (team.info().datas_size() > 0)
        {
          PROTOBUF(team, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
        }

        // sync guild info
        if (guild.info().datas_size() > 0)
        {
          PROTOBUF(guild, send, len);
          thisServer->sendCmd(ClientType::guild_server, send, len);
        }
        return true;
      }
      break;
    case MAP_REG_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(MapRegSessionCmd, message);
        ServerTask *task = getConnectedServer("SceneServer", message.scenename());
        if (!task) return true;
        SessionScene *scene = SessionSceneManager::getMe().getSceneByID(message.mapid());
        if (scene)
        {
          RegMapErr(message.mapid(), message.mapname(), task, "重复注册");
          return true;
        }
        //scene = NEW SessionScene(message.mapid(), message.mapname().c_str(), task);
        scene = SessionSceneManager::getMe().createScene(message.mapid(), message.mapname().c_str(), task);
        if (scene == nullptr)
        {
          RegMapErr(message.mapid(), message.mapname(), task, "创建失败");
          return true;
        }

        if (!SessionSceneManager::getMe().addScene(scene))
        {
          RegMapErr(message.mapid(), message.mapname(), task, "添加管理器失败");
          SAFE_DELETE(scene);
          return true;
        }
        if (!scene->init())
        {
          RegMapErr(message.mapid(), message.mapname(), task, "初始化失败");
          SessionSceneManager::getMe().delScene(scene);
          return true;
        }
        if (message.has_data())
        {
          const RaidMapData& data = message.data();
          if (data.restrict())
          {
            scene->m_oRaidData.m_dwRaidID = data.raidid();
            scene->m_oRaidData.m_qwTeamID = data.teamid();
            scene->m_oRaidData.m_qwCharID = data.charid();
            scene->m_oRaidData.m_qwGuildID = data.guildinfo().id();
            scene->m_oRaidData.m_oRestrict = (ERaidRestrict)data.restrict();
            scene->m_oRaidData.m_dwLayer = data.layer();
            scene->m_oRaidData.m_qwRoomId = data.roomid();
            scene->m_oRaidData.m_dwGuildRaidIndex = data.guildraidindex();
            int size = data.memberlist_size();
            for (int i=0; i<size; ++i)
            {
              scene->m_oRaidData.m_oMembers.insert(data.memberlist(i));
            }
            SessionSceneManager::getMe().onRaidSceneOpen(scene);
          }
        }

        //Cmd::Session::SessionRegMapOKCmd send;
        //send.mapID = scene->id;
        RegMapOKSessionCmd cmd;
        cmd.set_mapid(scene->id);
        PROTOBUF(cmd, send, len);
        task->sendCmd(send, len);
        XLOG << "[场景地图注册]," << scene->name << "(" << scene->id << "),注册成功 " << task->getName() << "." << XEND;

        BossList::getMe().onSceneOpen(scene);
        onSceneOpen(scene);

        return true;
      }
      break;
    case TOWER_MONSTERKILL:
      {
        PARSE_CMD_PROTOBUF(TowerMonsterKill, message);
        DWORD dwOldMaxLayer = TowerConfig::getMe().getMaxLayer();
        if (TowerConfig::getMe().unlockMonster(message.monsterid()) == true)
        {
          if (dwOldMaxLayer != TowerConfig::getMe().getMaxLayer())
            TowerManager::getMe().refreshMonsterGM();
        }
      }
      return true;
    case CMDPARAM_SEND_MAIL:
      {
        PARSE_CMD_PROTOBUF(SendMail, message);

        const MailData& rData = message.data();
        // process mail
        if (rData.type() == EMAILTYPE_SYSTEM)
        {
          MailManager::getMe().addSysMail(rData.sysid(), rData);
          auto client = [&](xEntry* e) -> bool
          {
            if (e == nullptr)
              return false;
            SessionUserManager::getMe().addSysMailCache(rData.sysid(), e->id);
            return true;
          };
          SessionUserManager::getMe().foreach(client);
        }
        else if (rData.type() == EMAILTYPE_NORMAL || rData.type() == EMAILTYPE_LOTTERY_GIVE|| rData.type() == EMAILTYPE_WEDDINGMSG || rData.type() == EMAILTYPE_WEDDINGINVITATION)
        {
          SessionUser* pUser = SessionUserManager::getMe().getUserByID(rData.receiveid());
          if (pUser != nullptr)
            pUser->getMail().addMail(rData, false);
        }       
        XLOG << "[邮件-发送处理] senderid :" << rData.senderid() << "receiveid :" << rData.receiveid() << "type :" << rData.type() << "sender :" << rData.sender()
          << "title :" << rData.title() << "msg :" << rData.msg() << "成功" << XEND;
      }
      return true;
    case CMDPARAM_SEND_MAIL_FROM_SCENE:
    {
      PARSE_CMD_PROTOBUF(SendMailFromScene, message);
      SendMail cmd;
      cmd.mutable_data()->CopyFrom(message.data());
      bool ret = MailManager::getMe().sendMail(cmd);
      XLOG << "[邮件] scene邮件消息 处理结果"<< ret <<"msg" << message.ShortDebugString() << XEND;
      return ret;
    }
    break;
    case SESSIONPARAM_GET_MAILATTACH:
      {
        PARSE_CMD_PROTOBUF(GetMailAttachSessionCmd, rev);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[邮件-获取附件]" << rev.charid() << "获取附件后不在线" << "msgid :" << rev.msgid() << XEND;
          return true;
        }
        pUser->getMail().processGetAttach(rev);
      }
      return true;

    case SESSIONPARAM_GET_TRADELOG:
    {
      PARSE_CMD_PROTOBUF(GetTradeLogSessionCmd, rev);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[交易-场景领取返回]" << rev.charid() << "领取后不在线" << rev.ShortDebugString() << XEND;
        return true;
      }
      return pUser->getTradeLog().takeSceneRes(rev);
    }
    case CMDPARAM_FOLLOWERIDCHECK:
      {
        PARSE_CMD_PROTOBUF(FollowerIDCheck, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.userid());
        if (pUser == nullptr)
        {
          XERR << "[玩家-跟随检查] charid :" << message.userid() << "检查失败,玩家未在线" << XEND;
          return true;
        }
        SessionScene* pScene = pUser->getScene();
        if (pScene == nullptr)
        {
          XERR << "[玩家-跟随检查]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "检查跟随" << message.ShortDebugString() << "失败,玩家未在正确的场景中" << XEND;
          return true;
        }

        SessionUser* pFollower = SessionUserManager::getMe().getUserByID(message.followid());
        QWORD qwResultFollowerID  = message.followid();

        do
        {
          if (pFollower == nullptr)
          {
            qwResultFollowerID = 0;
            break;
          }

          if (pFollower->getCarrierID() != 0)
          {
            MsgManager::sendMsg(pUser->id, 411);
            qwResultFollowerID = 0;
            break;
          }

          const SRaidCFG* pCFG = MapConfig::getMe().getRaidCFG(pFollower->getDMapID());
          if (pCFG != nullptr && (pCFG->eRestrict == ERAIDRESTRICT_GUILD || pCFG->eRestrict == ERAIDRESTRICT_GUILD_TEAM))
          {
            if (pUser->getGuild().id() == 0 || pFollower->getGuild().id() == 0)
            {
              MsgManager::sendMsg(pUser->id, 2638);
              qwResultFollowerID = 0;
              break;
            }
            if (pUser->getGuild().id() != pFollower->getGuild().id())
            { 
              MsgManager::sendMsg(pUser->id, 2638);              
              qwResultFollowerID = 0;
              break;
            }
          }
        }while (0);

        FollowerIDCheck cmd;
        cmd.set_userid(message.userid());
        cmd.set_followid(qwResultFollowerID);
        cmd.set_etype(message.etype());
        PROTOBUF(cmd, send, len);
        pScene->sendCmd(send, len);
        //thisServer->sendCmdToScene(send, len);
      }
      return true;
    /*case CHAT_MSG:
      {
        PARSE_CMD_PROTOBUF(ChatMsgSession, message);
        ChatManager::getMe().sendMessage(message);
      }
      return true;*/
    case CMDPARAM_SET_SEAL:
      {
        PARSE_CMD_PROTOBUF(SetTeamSeal, message);
        SealManager::getMe().setSeal(message);
        /*Team* pTeam = TeamManager::getMe().getTeamByID(message.teamid());
        if (pTeam == nullptr)
          return false;
        pTeam->setSeal(message);*/
      }
      return true;
    case DEL_SCENE_IMAGE:
      {
        thisServer->sendCmdToAllScene(buf, len);
      }
      return true;
    case SESSIONPARAM_BREAK_HAND:
      {
        PARSE_CMD_PROTOBUF(BreakHandSessionCmd, message);
        SessionUser* user = SessionUserManager::getMe().getUserByID(message.userid());
        if (user && user->getScene())
        {
          user->getScene()->sendCmd(buf, len);
        }
      }
      return true;
    case CMDPARAM_SET_GLOBL_DAILY:
      {
        thisServer->sendCmdToAllScene(buf, len);
      }
      return true;
    case CMDPARAM_REFRESH_QUEST:
      QuestConfig::getMe().randomWantedQuest(true);
      return true;
    case CHANGE_SCENE_RESULT_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(ChangeSceneResultSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser != nullptr && pUser->getUserState() == USER_STATE_CHANGE_SCENE)
          pUser->setUserState(USER_STATE_RUN);
      }
      return true;
    case SESSIONPARAM_USERLOGIN_NTF:
      {
        PARSE_CMD_PROTOBUF(UserLoginNtfSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
        {
          ErrSetUserDataSessionCmd cmd;
          cmd.set_id(message.charid());
          //sendCmdToServer(&send, sizeof(send), "SceneServer", message.servername().c_str());//rev->sceneName);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "SceneServer", message.servername().c_str());
          XERR << "[登录]" << message.charid() << "不在管理器中" << XEND;
          return true;
        }

          pUser->userOnline();
        }
      return true;
    case CMDPARAM_REFRESH_TOWER:
      TowerManager::getMe().timer(0);
      return true;
    case SESSIONPARAM_CHANGESCENE:
      {
        PARSE_CMD_PROTOBUF(ChangeSceneSingleSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        SessionScene *scene = SessionSceneManager::instance()->getSceneByID(message.mapid());
        if (pUser)
        {
          if (scene && pUser->gate_task() && USER_STATE_CHANGE_SCENE==pUser->getUserState())
          {
            XLOG << "[跨服切换场景]," << pUser->id << "," << pUser->name << ",新场景登录,dest:" << scene->name << "(" << scene->id << "),sour:"
              << (pUser->getScene()?pUser->getScene()->name:"") << "(" << (pUser->getScene()?pUser->getScene()->id:0) << XEND;

            {
              //platlog scene->dscene  进入动态副本日志
              if (scene && scene->isDScene())
              {
                SessionScene* pOldScene = pUser->getScene();
                DWORD dwNewMapid = scene->m_oRaidData.m_dwRaidID;
                DWORD dwOldMapid = 0;
                if (pOldScene)
                  dwOldMapid = pOldScene->id;

                QWORD eid = xTime::getCurUSec();
                EVENT_TYPE eType = EventType_Change_Move;
                ECHANGE_TYPE eChangeType = EChange_Move;
                if (pUser->getFollowerID())
                {
                  eType = EventType_Change_Follow;
                  eChangeType = EChange_Follow;
                }
                PlatLogManager::getMe().eventLog(thisServer,
                  pUser->m_platformId,
                  pUser->getZoneID(),
                  pUser->accid,
                  pUser->id,
                  eid,
                  0,  /*charge */
                  eType, 0, 1);
                PlatLogManager::getMe().changeLog(thisServer,
                  pUser->m_platformId,
                  pUser->getZoneID(),
                  pUser->accid,
                  pUser->id,
                  eType,
                  eid,
                  eChangeType,
                  dwOldMapid,
                  dwNewMapid,
                  0);
              }
            }
            pUser->setScene(scene);

            NotifyLoginSessionCmd cmd;
            cmd.set_id(message.charid());
            cmd.set_accid(pUser->accid);
            cmd.set_name(pUser->name);
            cmd.set_gatename(pUser->gate_task()->getName());
            cmd.set_ischangescene(true);
            cmd.set_ignorepwd(pUser->getAuthorize().getConfirmed());
            cmd.set_realauthorized(pUser->getAuthorize().getRealAutorize());
            cmd.set_maxbaselv(pUser->getMaxBaseLv());
            PROTOBUF(cmd, send, len);
            scene->sendCmd(send, len);
          }
          else
          {
            XERR << "[跨服切换场景]," << pUser->accid << "," << pUser->id << "," << pUser->name << ",切换场景失败:" << message.mapid() << ",sour:" << (pUser->getScene()?pUser->getScene()->id:0) << XEND;
            if (pUser->gate_task())
            {
            }
            pUser->userOffline();
            SessionUserManager::getMe().onUserQuit(pUser);
          }
        }
        return true;
      }
      break;
    case SESSIONPARAM_FORWARDUSER:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->sendCmdToMe(message.data().c_str(), message.data().size());

        return true;
      }
      break;
    case SESSIONPARAM_FORWARDUSERSESSION://  FORWARD_USER_SESSION_CMD_SCENESESSION_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSessionSessionCmd, message);
        SessionUser *pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (nullptr != pUser)
        {
          if (!pUser->doSessionUserCmd((const BYTE*)message.data().c_str(), message.data().size()))
          {
            XERR << "[消息]," << pUser->id << "," << pUser->name << ",处理错误 " << message.cmd() << "," << message.param() << XEND;
            return false;
          }
        }
      }
      break;
    case SESSIONPARAM_FORWARDUSERSCENESVR:// FORWARD_CMD_TO_USER_SCENESERVER_SCENESESSION_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSceneSvrSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(buf, len);
        }
        return true;
      }
      break;
    case SESSIONPARAM_FORWARDUSERSCENE://  FORWARD_USER_SCENE_CMD_SCENESESSION_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSceneSessionCmd, message);
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser && pUser->getScene())
        {
          pUser->getScene()->sendCmd(buf, len);
        }
        return true;
      }
      break;
    case SESSIONPARAM_ENTERGUILD:
      {
        PARSE_CMD_PROTOBUF(EnterGuildTerritorySessionCmd, message);
      }
      return true;
    case SESSIONPARAM_ITEMIMAGE:
      {
        PARSE_CMD_PROTOBUF(IteamImageSessionCmd, message);    
        /*Team* pTeam = TeamManager::getMe().getTeamByID(message.teamid());
        if (pTeam)
        {
          pTeam->setImageCreator(message.charid());
        }*/
      }
      return true;
    case SESSIONPARAM_FERRIS_INVITE:
      {
        PARSE_CMD_PROTOBUF(FerrisInviteSessionCmd, message);
        if (message.msgid() != 0)
        {
          if (message.msgid() != 10)
          {
            if (message.msgid() == ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR)
            {
              const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(message.id());
              if (pCFG != nullptr)
                MsgManager::sendMsg(message.charid(), message.msgid(), MsgParams(pCFG->sName));
            }
            else
              MsgManager::sendMsg(message.charid(), message.msgid());
          }
          return true;
        }

        SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
          return true;

        FerrisWheelManager::getMe().invite(message.id(), pUser, message.targetid());
      }
      return true;
    case SESSIONPARAM_FERRIS_READYENTER:
      {
        PARSE_CMD_PROTOBUF(EnterFerrisReadySessionCmd, message);
        FerrisWheelManager::getMe().ready(message);
      }
      return true;
    case SESSIONPARAM_ACTIVITY_TESTANDSET:
    {
      PARSE_CMD_PROTOBUF(ActivityTestAndSetSessionCmd, message);
      SessionActivityMgr::getMe().testAndSet(message);
      return true;
    }
    
    case SESSIONPARAM_ACTIVITY_STOP:
    {
      PARSE_CMD_PROTOBUF(ActivityStopSessionCmd, message);
      SessionActivityMgr::getMe().stop(message);
      return true;
    }
    case SESSIONPARAM_FORWARD_REGION:
    {
      PARSE_CMD_PROTOBUF(ForwardRegionSessionCmd, message);
      thisServer->sendCmd(static_cast<ClientType>(message.region_type()), (const BYTE*)(message.data().c_str()), message.len());
      return true;
    }
    case SESSIONPARAM_QUEST_RAID_CLOSE:
    {
      PARSE_CMD_PROTOBUF(QuestRaidCloseSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.userid());
      if (pUser && pUser->getScene())
      {
        pUser->getScene()->sendCmd(buf, len);
      }
      return true;
    }
    case SESSIONPARAM_GUILDRAID_CLOSE:
    {
      PARSE_CMD_PROTOBUF(GuildRaidCloseSessionCmd, message);
      SessionSceneManager::getMe().closeGuildRaidGroup(message.guildid(), message.teamid(), message.curmapindex());
      return true;
    }
    case SESSIONPARAM_DELETE_AUTHORIZE:
    {
      PARSE_CMD_PROTOBUF(DeletePwdSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->getAuthorize().deletepwd();
      }
      return true;
    }
    case SESSIONPARAM_WANTED_QUEST_FINISH:
    {
      PARSE_CMD_PROTOBUF(WantedQuestFinishCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.leaderid());
      if (pUser && pUser->getScene())
      {
        pUser->getScene()->sendCmd(buf, len);
      }
      return true;
    }
    case SESSIONPARAM_UPDATE_OPERACTIVITY:
    {
      ActivityManager::getMe().updateActivity();
      return true;
    }
    case SESSIONPARAM_UPDATE_ACTIVITYEVENT:
    {
      ActivityEventManager::getMe().updateEvent();
      return true;
    }
    case SESSIONPARAM_LOVELETTER_SEND:
    {
      PARSE_CMD_PROTOBUF(LoveLetterSendSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->getScene()->sendCmd(buf, len);
      }
      return true;
    }
    case SESSIONPARAM_USE_ITEMCODE:
    {
      PARSE_CMD_PROTOBUF(UseItemCodeSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->useItemCode(message);
      }
      return true;
    }
    break;
    case SESSIONPARAM_REQ_USED_ITEMCODE:
    {
      PARSE_CMD_PROTOBUF(ReqUsedItemCodeSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->getUsedItemCode(message);
      }
      return true;
    }
    break;
    case SESSIONPARAM_OPERATE_REWARD:
    {
      PARSE_CMD_PROTOBUF(SyncOperateRewardSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser)
      {
        pUser->setOperateReward(message.var());
      }
      return true;
    }
    break;
    case SESSIONPARAM_NOTIFY_ACTIVITY:
    {
      thisServer->sendCmdToAllScene(buf, len);
      return true;
    }
    break;
    case SESSIONPARAM_GIVE_REWARD:
    {
      PARSE_CMD_PROTOBUF(GiveRewardSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->getScene()->sendCmd(buf, len);
      }
      return true;
    }
    break;
    case SESSIONPARAM_WANTED_QUEST_SET_CD:
    {
      PARSE_CMD_PROTOBUF(WantedQuestSetCDSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser && pUser->getScene())
      {
        pUser->getScene()->sendCmd(buf, len);
      }
      return true;
    }
    break;
    case SESSIONPARAM_SYNC_WORLD_LEVEL:
    {
      PARSE_CMD_PROTOBUF(SyncWorldLevelSessionCmd, message);
      WorldLevelManager::getMe().processSyncWorldLevelCmd(message);
      return true;
    }
    break;
    case SESSIONPARAM_USER_ENTERSCENE:
    {
      PARSE_CMD_PROTOBUF(UserEnterSceneSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser != nullptr)
        pUser->onEnterScene();
    }
    break;
    case SESSIONPARAM_USER_VAR_SYNC:
    {
      PARSE_CMD_PROTOBUF(SyncUserVarSessionCmd, message);
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(message.charid());
      if (pUser)
        pUser->updateVar(message);
    }
    break;
    default:
      return false;
  }

  return false;
}

bool SessionServer::onSceneOpen(SessionScene *scene)
{
  if (nullptr == scene)
    return false;

  // 如果是无限塔
  /*if (SessionTower::getMe().isTower(scene->m_oRaidData.m_dwRaidID))
    {
    DWORD dwTeamLayer = SessionTower::getMe().getTeamRegChallengeLayer(scene->m_oRaidData.m_dwTeamID);
    if (dwTeamLayer > 0)
    {
  //通知副本场景进程，队伍开始挑战dwTeamLayer层
  Cmd::TowerChallenge cmd;
  cmd.set_teamid(scene->m_oRaidData.m_dwTeamID);
  cmd.set_layer(dwTeamLayer);
  PROTOBUF(cmd, send, len);
  scene->sendCmd(send, len);
  }
  }*/

  return true;
}

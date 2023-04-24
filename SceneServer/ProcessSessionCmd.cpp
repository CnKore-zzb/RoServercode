#include "SceneServer.h"
#include "SceneUserManager.h"
#include "SceneUser.h"
#include "xNetProcessor.h"
#include "SceneManager.h"
#include "SceneNpcManager.h"
#include "Scene.h"
#include "FuBen.h"
#include "SceneTower.h"
#include "UserCmd.h"
#include "config/ConfigManager.h"
#include "SceneWeddingMgr.h"
//#include "SceneConfig.h"
#include "json/json.h"
#include "GMCommandRuler.h"
#include "DScene.h"
#include "SceneTrade.pb.h"
#include "MsgManager.h"
#include "StatisticsDefine.h"
#include "ChatManager_SC.h"
#include "CarrierCmd.pb.h"
#include "MsgManager.h"
#include "ActivityManager.h"
#include "SocialCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "TeamSealManager.h"
#include "MailManager.h"
#include "RedisManager.h"
#include "SceneNpc.h"
#include "MiscConfig.h"
#include "MatchSCmd.pb.h"
#include "DateLandConfig.h"
#include "AuctionSCmd.pb.h"
#include "GuildCityManager.h"
#include "ActivityEventManager.h"
#include "ActivityEvent.pb.h"
#include "GuildConfig.h"
#include "SceneShop.h"
//#include "ItemSCmd.pb.h"
#include "SysmsgConfig.h"
#include "SceneBoothManager.h"
#include "BossSCmd.pb.h"
#include "BossConfig.h"
#include "BossMgr.h"
#include "Menu.h"

extern xLog srvLog;

bool SceneServer::doSessionCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case LOAD_LUA_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(LoadLuaSessionCmd, message);

        if (message.has_lua() == true)
          LuaManager::getMe().reload();
        if (message.has_log() == true)
          srvLog.reload();

        if (message.has_table() == true)
        {
          TVecConfigType vec;
          ConfigManager::getMe().getType(message.table(), vec);
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (cfg.isReload() && cfg.isSceneLoad())
                ConfigManager::getMe().loadConfig(cfg);
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (cfg.isReload() && cfg.isSceneLoad())
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }

        return true;
      }
      break;
    case EXEC_GM_CMD_SESSIONCMD:
    {
      PARSE_CMD_PROTOBUF(ExecGMCmdSessionCmd, message);
      bool ret = GMCommandRuler::getMe().execute(message.gmcmd());
      XLOG << "[全线-GM] 收到session 消息" << message.gmcmd() << "执行结果" << ret << XEND;
      return true;
    }
    break;
    case DELETE_DMAP_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(DeleteDMapSessionCmd, message);

        Scene *scene = SceneManager::getMe().getSceneByID(message.mapid());
        if (scene)
        {
          XLOG << "[动态地图],设置关闭" << scene->id << scene->name << XEND;
          scene->setState(xScene::SCENE_STATE_CLOSE);
        }

        return true;
      }
      break;
    case CREATE_RAIDMAP_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(CreateRaidMapSessionCmd, message);

        SceneManager::getMe().createDScene(message);

        return true;
      }
      break;
    case CHANGE_SCENE_RESULT_SESSIONCMD:
      {
        PARSE_CMD_PROTOBUF(ChangeSceneResultSessionCmd, message);

        SceneUser *pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr || (pUser->getScene() != nullptr && pUser->getScene()->id == message.mapid()))
        {
          sendCmdToSession(buf, len);
          return true;
        }

        xPos p;
        p.set(message.pos().x(), message.pos().y(), message.pos().z());
        pUser->getUserSceneData().setOnlineMapPos(message.mapid(), p);

        //pUser->m_oHands.onChangeScene(message.mapid(), p);

        XLOG << "[离线]" << pUser->id << pUser->name << "跨服切换场景" << message.mapid() << "pos:" << message.pos().x() << message.pos().y() << message.pos().z() << XEND;
        SceneUserManager::getMe().onUserQuit(pUser, UnregType::ChangeScene);
        return true;
      }
      break;
    case TOWERINFO_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SceneTowerUpdate, message);
        TowerConfig::getMe().updateConfig(message.info());
        XLOG << "[TowerConfig::initDataFromDB] servername =" << thisServer->getServerName() << "maxlayer =" << message.info().maxlayer() << XEND;
        return true;
      }
      break;
    case CMDPARAM_SCENEUSERCMD:
      {
        PARSE_CMD_PROTOBUF(SessionSceneUserCmd, message);
        const string &byteData = message.cmddata();
        QWORD userid = message.userid();
        SceneUser *pUser = SceneUserManager::getMe().getUserByID(userid);
        if (nullptr != pUser)
        {
          Cmd::UserCmd* pcmd = (Cmd::UserCmd*)byteData.c_str();
          pUser->doUserCmd(pcmd, byteData.size());
          return true;
        }
      }
      break;
    case SESSIONPARAM_GET_MAILATTACH:
      {
        PARSE_CMD_PROTOBUF(GetMailAttachSessionCmd, rev);
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
          if (pUser == nullptr)
          {
            rev.set_msgid(10);
            XERR << "[邮件-获取附件]" << rev.charid() << "获取失败,玩家不在线" << XEND;
            break;
          }

          TVecItemData vecData;
          DWORD dwChargeMoney = 0;
          QWORD qwQuota = 0;
          for (int i = 0; i < rev.items_size(); ++i)
          {
            const ItemInfo& rItem = rev.items(i);

            ItemData oData;
            oData.mutable_base()->CopyFrom(rItem);
            combineItemData(vecData, oData);

            if (rItem.chargemoney())
              dwChargeMoney += rItem.chargemoney();
            if (rItem.quota())
              qwQuota = rItem.quota();
          }

          TVecItemData vecItemData;
          ItemData lotteryLetter;
          for (int i = 0; i < rev.itemdatas_size(); ++i)
          {
            ItemData* pData = rev.mutable_itemdatas(i);
            if (pUser->getPackage().checkAndFillItemData(*pData) == false)
            {
              rev.set_msgid(10);
              XERR << "[邮件-获取附件]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "填充" << pData->ShortDebugString() << "信息失败" << XEND;
              break;
            }
            combineItemData(vecData, rev.itemdatas(i));
            
            if (rev.opt() == EGetMailOpt_LotteryGive && pData->base().id() == MiscConfig::getMe().getLotteryCFG().dwLoveLeterItemId)
            {
              lotteryLetter = *pData;
            }
          }
          if (pUser->getPackage().addItem(vecData, EPACKMETHOD_NOCHECK) == false)
          {
            rev.set_msgid(10);
            MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PACK_FULL);
            XERR << "[邮件-获取附件]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "添加" << vecData << "到包裹失败" << XEND;
            break;
          }
         
          pUser->addCharge(dwChargeMoney);
          
          if (qwQuota == 0 && dwChargeMoney != 0)
            qwQuota = (QWORD)dwChargeMoney * 10000;
         
          pUser->getDeposit().addQuota(qwQuota, EQuotaType_G_Charge);
          
          //领取扭蛋赠送
          if (rev.opt() == EGetMailOpt_LotteryGive || rev.opt() == EGetMailOpt_LotteryGive_Auto)
          {
            //删除姜饼人
            pUser->m_oGingerBread.delOne(EGiveType_Lottery, rev.mailid(), true, false);
            //发送消息
            
            //打开最近添加的贺卡
            if (rev.opt() == EGetMailOpt_LotteryGive)
            {
              pUser->m_oGingerBread.sendLotterLetter2Client(lotteryLetter);
            }     
            else
            {
              //您有一份【x年x月扭蛋盒】因长时间未领取，已自动加入背包；
              const SLotteryGiveCFG* pCfg = ItemConfig::getMe().getLotteryBoxYearMonth(vecData);
              if (pCfg)
              {
                MsgParams param;
                param.addNumber(pCfg->dwYear);
                param.addNumber(pCfg->dwMonth);
                MsgManager::sendMsg(pUser->id, 25310, param);
              }
            }
          }
          XLOG << "[邮件-获取附件]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name <<"充值金额" << dwChargeMoney << "额度" << qwQuota << "获取成功" <<rev.ShortDebugString() << XEND;
        } while (0);

        PROTOBUF(rev, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      break;
    case SESSIONPARAM_GET_TRADELOG:
    {
      PARSE_CMD_PROTOBUF(GetTradeLogSessionCmd, rev);
      do
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          rev.set_success(false);
          XERR << "[交易-领取]" << rev.charid() << "领取失败，玩家不在线"<<rev.ShortDebugString() << XEND;
          break;
        }

        TVecItemInfo vecItem;
        if (rev.has_item())
          vecItem.push_back(rev.item());

        pUser->getPackage().addItem(vecItem, EPACKMETHOD_NOCHECK);
        rev.set_success(true);   

        TVecItemData vecItemData;
        
        if (rev.has_itemdata() && rev.itemdata().has_previewenchant())
          rev.mutable_itemdata()->clear_previewenchant();

        if (rev.has_itemdata())
          vecItemData.push_back(rev.itemdata());
        for (auto &v : vecItemData)
          pUser->getPackage().checkAndFillItemData(v);
        pUser->getPackage().addItem(vecItemData, EPACKMETHOD_NOCHECK);
        if (rev.logtype() == EOperType_NormalSell || rev.logtype() == EOperType_PublicitySellSuccess)
        {
          //出售获得钱
          pUser->getShare().addNormalData(ESHAREDATATYPE_S_TRADEGAIN, rev.item().count());
          pUser->getShare().onTradeSell(rev.sell_item_id(), rev.sell_count()*rev.sell_price(), rev.refine_lv());
          pUser->getAchieve().onTradeSell(rev.sell_count() * rev.sell_price());
          pUser->getAchieve().onTradeRecord();

          if(Cmd::ETRADETYPE_BOOTH == rev.trade_type())
            pUser->m_oBooth.calcTax(rev.tax());
        }
        if (rev.logtype() == EOperType_NoramlBuy || rev.logtype() == EOperType_PublicityBuySuccess)
        {
          pUser->getAchieve().onItemAdd(rev.item());
          pUser->getAchieve().onTradeRecord();
        }

        if(Cmd::ETRADETYPE_BOOTH == rev.trade_type())
        {
          if(Cmd::EOperType_PublicityBuyFail == rev.logtype())
            pUser->getDeposit().unlockQuota(rev.quota(), Cmd::EQuotaType_U_Booth);
          else if(Cmd::EOperType_PublicityBuySuccess == rev.logtype())
            pUser->getDeposit().unlockQuota(rev.quota(), Cmd::EQuotaType_U_Booth, true);
        }

        if (rev.logtype() == EOperType_AutoOffTheShelf && rev.ret_cost())
        {
          Cmd::ItemInfo itemInfo;
          itemInfo.set_id(100);
          itemInfo.set_count(rev.ret_cost());
          itemInfo.set_source(ESOURCE_TRADE);
          pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE);
        }

        XLOG << "[交易-领取]" << rev.charid() << "领取成功" << rev.ShortDebugString() << XEND;
      } while (0);

      PROTOBUF(rev, send, len);
      thisServer->sendCmdToSession(send, len);
    }
    break;
    case CMDPARAM_FOLLOWERIDCHECK:
      {
        PARSE_CMD_PROTOBUF(FollowerIDCheck, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.userid());
        if (pUser != nullptr)
        {
          if(pUser->getUserSceneData().getFollowerID() != 0 && message.followid() != 0)
            pUser->getUserSceneData().setFollowerIDNoCheck(0);    //跟随qwID前,先断掉之前的跟随
          pUser->getUserSceneData().setFollowerIDNoCheck(message.followid());
        }

        return true;
      }
      break;
    case CMDPARAM_EVENT:
      {
        PARSE_CMD_PROTOBUF(Event, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.guid());
        if (pUser != nullptr)
        {
          if (message.type() == EEVENTTYPE_SOCIALAPPLY_ADD)
            pUser->getTip().addRedTip(EREDSYS_SOCIAL_FRIEND_APPLY);
        }
        return true;
      }
      break;
    case CMDPARAM_SEALTIMER:
      {
        PARSE_CMD_PROTOBUF(QuerySealTimer, message);
        SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(message.teamid());
        if (pTeamSeal == nullptr)
          return true;
        pTeamSeal->sendSealInfo(message.userid());
        return true;
      }
      break;
    case CMDPARAM_SET_SEAL:
      {
        PARSE_CMD_PROTOBUF(SetTeamSeal, message);

        SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(message.teamid());
        if (pTeamSeal == nullptr)
        {
          if (message.estatus() == ESETSEALSTATUS_CREATE)
            pTeamSeal = TeamSealManager::getMe().createOneTeamSeal(message.teamid());
          if (pTeamSeal)
          {
            for (int i = 0; i < message.teamers_size(); ++i)
            {
              pTeamSeal->addMember(message.teamers(i));
            }
          }
        }
        if (pTeamSeal == nullptr)
          break;

        if (message.estatus() == ESETSEALSTATUS_CREATE)
        {
          xPos pos;
          pos.set(message.pos().x(), message.pos().y(), message.pos().z());
          pTeamSeal->clearSealData(message.sealid());
          pTeamSeal->openSeal(message.sealid(), nullptr, pos);
        }
        else if (message.estatus() == ESETSEALSTATUS_ABANDON)
        {
          pTeamSeal->clearSealData(0);
        }
        else if (message.estatus() == ESETSEALSTATUS_INVALID)
        {
          pTeamSeal->setDelStatus();
        }
      }
      return true;
    case SESSIONPARAM_CHANGE_TEAM:
      {
        PARSE_CMD_PROTOBUF(ChangeTeamSessionCmd, message);
        SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(message.teamid());
        if (pTeamSeal == nullptr)
          break;
        if (message.join())
          pTeamSeal->addMember(message.userid());
        else
          pTeamSeal->removeMember(message.userid());
      }
      break;
    case DEL_SCENE_IMAGE:
      {
        PARSE_CMD_PROTOBUF(DelSceneImage, message);
        DWORD mapid = message.realscene();
        Scene* pScene = SceneManager::getMe().getSceneByID(mapid);
        if (pScene)
        {
          pScene->m_oImages.del(message.guid(), message.raid());
        }

        if (message.etype() == ESCENEIMAGE_SEAL)
        {
          SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(message.guid());
          if (pTeamSeal == nullptr)
            break;
          pTeamSeal->onOverRaidSeal(message.realscene());
        }
      }
      break;
    case CMDPARAM_SET_GLOBL_DAILY:
      {
        PARSE_CMD_PROTOBUF(SetGlobalDaily, message);
        QuestManager::getMe().setDailyExtra(message.value());
      }
      return true;
    case SESSIONPARAM_NOTIFY_LOGIN:
      {
        PARSE_CMD_PROTOBUF(NotifyLoginSessionCmd, message);
        SceneUser *pUser = SceneUserManager::getMe().getUserByID(message.id());
        if (pUser)
        {
          logErr(pUser, pUser->id, pUser->name, Cmd::REG_ERR_RELOGIN_SCENE, true);
          return true;
        }
        pUser = SceneUserManager::getMe().getLoginUserByID(message.id());
        if (pUser)
        {
          SceneUserManager::getMe().delLoginUser(pUser);
          XERR << "[登录]" << pUser->accid << pUser->id << pUser->name << "out重复登陆" << XEND;

          logErr(pUser, pUser->id, pUser->name, Cmd::REG_ERR_RELOGIN_SCENE, false);

          // pUser 已被删除  后面不要再用
        }

        ServerTask *net = getConnectedServer("GateServer", message.gatename().c_str());
        if (!net)
        {
          logErr(NULL, message.id(), message.name().c_str(), REG_ERR_FIND_GATE, false);
          return true;
        }
        SceneUser* user = NEW SceneUser(message.id(), message.name().c_str(), message.accid(), net);
        if (!user)
        {
          logErr(NULL, message.id(), message.name().c_str(), Cmd::REG_ERR_SCENE_CREATE_FAILD,false);
          return true;
        }
        user->setPhone(message.phone());
        user->setIgnorePwd(message.ignorepwd());

        if (!SceneUserManager::getMe().addLoginUser(user))
        {
          logErr(user, message.id(), message.name().c_str(), Cmd::REG_ERR_RELOGIN_SCENE,false);
          return true;
        }
        if (message.ischangescene())
          user->setUserState(USER_STATE_CHANGE_SCENE);
        else
          user->setUserState(USER_STATE_LOGIN);
        user->getUserMap().setLoginMapID(message.mapid());
        user->setLanguage(message.language());
        user->setRealAuthorized(message.realauthorized());
        user->getUserSceneData().setCurMaxBaseLv(message.maxbaselv());

        NotifyLoginRecordCmd cmd;
        cmd.set_id(message.id());
        cmd.set_accid(message.accid());
        cmd.set_scenename(thisServer->getServerName());

        PROTOBUF(cmd, send, len);
        if (!sendCmdToRecord(send, len))
        {
          SceneUserManager::getMe().delLoginUser(user);
          logErr(user, message.id(), message.name().c_str(), REG_ERR_REQ_DATA_FROM_RECORD, false);
          return true;
        }
        else
        {
          XLOG << "[登录]" << user->accid << user->id << user->name << "通知玩家登录" << (message.ischangescene() ? "" : "非") << "跨场景,zoneid:" << user->getZoneID() <<"实名认证"<<user->isRealAuthorized() << XEND;
        }
      }
      return true;
    case SESSIONPARAM_ERR_SET_USERDATA:
      {
        PARSE_CMD_PROTOBUF(ErrSetUserDataSessionCmd, message);
        SceneUser* sUser = SceneUserManager::getMe().getUserByID(message.id());
        if(sUser)
        {
          XLOG << "[登录]" << sUser->accid << sUser->id << sUser->name << "set data failed,delete." << XEND;
          SceneUserManager::getMe().onUserQuit(sUser, UnregType::Normal);
        }
        else
        {
          XERR << "[登录]" << message.id() << "0,0,session set data failed." << XEND;
        }
        return true;
      }
      break;
    case SESSIONPARAM_REGMAPFAIL:
      {
        PARSE_CMD_PROTOBUF(RegMapFailSessionCmd, message);
        Scene* scene = SceneManager::getMe().getSceneByID(message.mapid());
        if (scene)
          SceneManager::getMe().delScene(scene);
        return true;
      }
      break;
    case SESSIONPARAM_REGMAPOK:
      {
        PARSE_CMD_PROTOBUF(RegMapOKSessionCmd, message);
        Scene* scene = SceneManager::getMe().getSceneByID(message.mapid());
        if(scene)
        {
          scene->regSucceed();
          XLOG << "[加载地图]加载地图" << scene->name << "(" << scene->id << ")成功,会话注册成功" << XEND;
        }
        return true;
      }
      break;
    case SESSIONPARAM_FORWARDUSERSCENE:// FORWARD_USER_SCENE_CMD_SCENESESSION_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSceneSessionCmd, message);
        SceneUser *pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->doUserCmd((const Cmd::UserCmd *)message.data().c_str(), message.data().size());
        return true;
      }
      break;
    case SESSIONPARAM_FORWARDUSERSCENESVR://FORWARD_CMD_TO_USER_SCENESERVER_SCENESESSION_CMD:
      {
        PARSE_CMD_PROTOBUF(ForwardUserSceneSvrSessionCmd, message);
        return doSessionCmd((const BYTE*)message.data().c_str(), message.data().size());
      }
      break;
    /*case SESSIONPARAM_SYNC_DOJO:
      {
        PARSE_CMD_PROTOBUF(SyncDojoSessionCmd, message);
        SceneTeam* pTeam = SceneTeamManager::getMe().getTeamByID(message.teamguid());
        if (pTeam == nullptr)
          return true;
        pTeam->syncDojoData(message.guildid(), message.sponsorid(), message.dojoid(), message.isopen(), message.del());
        return true;
      }
      break;*/
    //case SESSIONPARAM_CHARGE:
    //  {
    //    PARSE_CMD_PROTOBUF(ChargeSessionCmd, message);
    //    XLOG << "[充值] 收到charid :" << message.charid() << "进行 charge :" << message.charge() << message.orderid() << message.itemid() << message.count() << "充值, 开始处理" << XEND;

    //    SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
    //    if (pUser != nullptr)
    //    {
    //      pUser->addCharge(&message);
    //    }
    //  }
    //  return true;
    case SESSIONPARAM_GM_GAG:
      {
        PARSE_CMD_PROTOBUF(GagSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser != nullptr)
        {
          pUser->getUserSceneData().setGagTime(message.time(), message.reason());
          XLOG << "[GM控制-禁言]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "被设置了禁言时间" << message.time()<<"原因"<<message.reason() << XEND;
        }
      }
      return true;
    case SESSIONPARAM_GM_LOCK:
      {
        PARSE_CMD_PROTOBUF(LockSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser != nullptr)
        {
          if (!message.account())
            pUser->getUserSceneData().setNologinTime(message.time(), message.reason());
          XLOG << "[GM控制-封号]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "被设置了封号时间" << message.time()<<"原因" << message.reason() <<"是否是账号封号" <<message.account() << XEND;

          KickUserErrorCmd cmd;
          cmd.set_accid(pUser->accid);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToGate(send, len);
        }
      }
      return true;
    case SESSIONPARAM_FERRIS_INVITE:
      {
        PARSE_CMD_PROTOBUF(FerrisInviteSessionCmd, message);
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
          if (pUser == nullptr)
          {
            message.set_msgid(ESYSTEMMSG_ID_USER_OFFLINE);
            break;
          }
          const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(message.id());
          if (pCFG == nullptr)
          {
            XERR << "[约会圣地]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "约会id:" << message.id() << "配置找不到" << XEND;
            message.set_msgid(10);
            break;
          }
          MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
          if (pMainPack == nullptr)
          {
            message.set_msgid(ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR);
            break;
          }
          if (pMainPack->checkItemCount(pCFG->dwTicketItem) == false)
          {
            message.set_msgid(ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR);
            break;
          }
        } while(0);

        PROTOBUF(message, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      return true;
    case SESSIONPARAM_ACTIVITY_TESTANDSET:   //活动查询返回
    {
      PARSE_CMD_PROTOBUF(ActivityTestAndSetSessionCmd, rev);        
      ActivityManager::getMe().startActivity(rev);      
      return true;
    }

    case SESSIONPARAM_FERRIS_READYENTER:
      {
        PARSE_CMD_PROTOBUF(EnterFerrisReadySessionCmd, message);
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
          if (pUser == nullptr)
          {
            message.set_msgid(ESYSTEMMSG_ID_USER_OFFLINE);
            break;
          }

          const SDateLandCFG* pCFG = DateLandConfig::getMe().getDateLandCFG(message.id());
          if (pCFG == nullptr)
          {
            XERR << "[约会圣地]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "约会id:" << message.id() << "配置找不到" << XEND;
            message.set_msgid(10);
            break;
          }
          MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
          if (pMainPack == nullptr)
          {
            message.set_msgid(ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR);
            break;
          }
          if (pMainPack->checkItemCount(pCFG->dwTicketItem) == false)
          {
            message.set_msgid(ESYSTEMMSG_ID_FERRIS_INVITE_ITEM_ERROR);
            break;
          }
          pMainPack->reduceItem(pCFG->dwTicketItem, ESOURCE_FERRISWHEEL);

          pUser->getUserSceneData().setFollowerIDNoCheck(0);
        }while(0);

        PROTOBUF(message, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      return true;
    case SESSIONPARAM_ACTIVITY_STATUS:
      {
        //PARSE_CMD_PROTOBUF(ActivityStatusSessionCmd, message);
        //LuaManager::getMe().call<void>("activitystatus", message.type(), message.mapid(), message.start());
      }
      return true;
    case SESSIONPARAM_BREAK_HAND:
      {
        PARSE_CMD_PROTOBUF(BreakHandSessionCmd, message);
        SceneUser* user = SceneUserManager::getMe().getUserByID(message.userid());
        if (user)
          user->m_oHands.breakByOther();
      }
      return true;
    case SESSIONPARAM_WANTED_INFO_SYNC:
      {
        PARSE_CMD_PROTOBUF(WantedInfoSyncSessionCmd, message);
        QuestConfig::getMe().randomWantedQuest(true);
        //QuestConfig::getMe().setWantedActive(message.active(), message.maxcount());
      }
      return true;
    case SESSIONPARAM_ZONE_QUERYSTATUS:
      {
        PARSE_CMD_PROTOBUF(QueryZoneStatusSessionCmd, message);
        ChatManager_SC::getMe().setZoneInfo(message);
      }
      return true;
    case SESSIONPARAM_QUEST_RAID_CLOSE:
      {
        PARSE_CMD_PROTOBUF(QuestRaidCloseSessionCmd, message);
        SceneUser* user = SceneUserManager::getMe().getUserByID(message.userid());
        if (user)
          user->getQuest().onPassRaid(message.raidid(), false);
      }
      return true;
    case SESSIONPARAM_SYNC_AUTHORIZE:
      {
        PARSE_CMD_PROTOBUF(AuthorizeInfoSessionCmd, message);
        SceneUser* user = SceneUserManager::getMe().getUserByID(message.charid());
        if (user)
        {
          user->setIgnorePwd(message.ignorepwd());
          XLOG << "[安全密码-同步场景] 成功" << user->accid << user->id << user->getProfession() << user->name
            << "ignorepwd" << message.ignorepwd() << XEND;
        }
      }
      return true;
    case SESSIONPARAM_GUILDRAID_CLOSE:
      {
        PARSE_CMD_PROTOBUF(GuildRaidCloseSessionCmd, message);
        Scene* pScene = SceneManager::getMe().getSceneByID(message.mapid());
        if (pScene)
        {
          GuildRaidScene* pGScene = dynamic_cast<GuildRaidScene*> (pScene);
          if (pGScene)
          {
            pGScene->setCloseTime(now());
            pGScene->setCloseTime(now(), true);
          }
        }
      }
      return true;
    case SESSIONPARAM_GO_BACK:
      {
        PARSE_CMD_PROTOBUF(GoBackSessionCmd, message);
        SceneUser *pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
        {
          pUser->getUserMap().gotoLastStaticMap();
        }
        return true;
      }
      break;
    case SESSIONPARAM_WANTED_QUEST_FINISH:
      {
        PARSE_CMD_PROTOBUF(WantedQuestFinishCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.leaderid());
        if (pUser)
          pUser->getAchieve().onHelpQuest();
      }
      return true;
    case SESSIONPARAM_ADD_OFFLINE_ITEM:
      {
        PARSE_CMD_PROTOBUF(AddOfflineItemSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if(nullptr == pUser)
        {
          XERR << "[玩家-物品]" << message.charid() << "玩家不在线，离线添加" << message.ShortDebugString() << "失败" << XEND;
          return false;
        }
        BasePackage* pPack = nullptr;
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(message.data().base().id());
        if (pCFG != nullptr && ItemConfig::getMe().isQuest(pCFG->eItemType) == true)
          pPack = pUser->getPackage().getPackage(EPACKTYPE_QUEST);
        else
          pPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);

        if(nullptr == pPack)
        {
          XERR << "[玩家-物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "无法获取主背包，离线添加" << message.ShortDebugString() << "失败" << XEND;
          return false;
        }

        if(ItemManager::getMe().isEquip(message.data().base().type()))
        {
          ItemBase* pBase = ItemManager::getMe().createItem(message.data().base());
          if(nullptr == pBase)
          {
            XERR << "[玩家-物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "无法创建物品，离线添加" << message.data().base().ShortDebugString() << "失败" << XEND;
            return false;
          }
          pBase->fromItemData(message.data());
          pPack->addItemObj(pBase, true, ESOURCE_PUBLIC_OFFSTORE);
        }
        else
        {
          ItemData oData;
          oData.CopyFrom(message.data());
          pPack->addItem(oData, EPACKMETHOD_NOCHECK);
        }

        XLOG << "[玩家- 物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "离线物品添加" << message.data().ShortDebugString() << "成功" << XEND;
        return true;
      }
      break;
    case SESSIONPARAM_SYNC_SHOP:
    {
      PARSE_CMD_PROTOBUF(SyncShopSessionCmd, message);
      ShopConfig::getMe().setWeekRand(message.item());
      XLOG << "[SHOP] 刷新，groupid" << message.item() << "成功" << XEND;
      return true;
    }
    break;
    case SESSIONPARAM_ACTIVITYEVENT_NTF:
      {
        PARSE_CMD_PROTOBUF(ActivityEventNtfSessionCmd, cmd);
        ActivityEventManager::getMe().updateEvent(cmd);
      }
      break;
    case SESSIONPARAM_LOVELETTER_USE:
      {
        PARSE_CMD_PROTOBUF(LoveLetterSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->processLoveLetter(message);
      }
      break;
    case SESSIONPARAM_LOVELETTER_SEND:
      {
        PARSE_CMD_PROTOBUF(LoveLetterSendSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->processLoveLetterSend(message);
      }
      break;
    case SESSIONPARAM_USE_ITEMCODE:
      {
        PARSE_CMD_PROTOBUF(UseItemCodeSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->getPackage().useItemCodeSessionRes(message);
        return true;
      }
      break;
    case SESSIONPARAM_REQ_USED_ITEMCODE:
      {
        PARSE_CMD_PROTOBUF(ReqUsedItemCodeSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->getPackage().reqUsedItemCodeSessionRes(message); 
        return true;
      }
      break;
    case SESSIONPARAM_REQ_LOTTERY_GIVE:
      {
        PARSE_CMD_PROTOBUF(ReqLotteryGiveSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->getLottery().lotteryGive(message.iteminfo());
        return true;
      }
      break;
    case SESSIONPARAM_NOTIFY_ACTIVITY:
      {
        PARSE_CMD_PROTOBUF(NotifyActivitySessionCmd, message);
        if(message.open() == true)
          ActivityManager::getMe().addActivity(message.actid());
        else
          ActivityManager::getMe().delActivity(message.actid());
        return true;
      }
      break;
    case SESSIONPARAM_GIVE_REWARD:
      {
        PARSE_CMD_PROTOBUF(GiveRewardSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
          return false;
        if(message.rewardid())
          pUser->getPackage().rollReward(message.rewardid());
        if(message.buffid())
          pUser->m_oBuff.add(message.buffid(), pUser);
        return true;
      }
      break;
    case SESSIONPARAM_WANTED_QUEST_SET_CD:
      {
        PARSE_CMD_PROTOBUF(WantedQuestSetCDSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->getQuest().setTeamFinishBoardQuestTime(message.time());
        return true;
      }
      break;
    case SESSIONPARAM_USER_QUOTA_OPER:
      {
        PARSE_CMD_PROTOBUF(UserQuotaOperSessionCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (!pUser)
        {
          XERR << "[玩家-额度]" << message.charid() << "玩家不在线，离线添加" << message.ShortDebugString() << "失败" << XEND;
          return false;
        }

        if(EUSERQUOTAOPER_UNLOCK == message.oper())
          pUser->getDeposit().unlockQuota(message.quota(), message.type());
        else if(EUSERQUOTAOPER_UNLOCK_SUB == message.oper())
          pUser->getDeposit().unlockQuota(message.quota(), message.type(), true);

        XLOG << "[玩家-额度]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "离线操作额度成功" << message.ShortDebugString() << XEND;
        return true;
      }
      break;
    case SESSIONPARAM_SYNC_WORLD_LEVEL:
      {
        PARSE_CMD_PROTOBUF(SyncWorldLevelSessionCmd, message);
        SceneUserManager::getMe().setBaseWorldLevel(message.base_worldlevel());
        SceneUserManager::getMe().setJobWorldLevel(message.job_worldlevel());
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
          pUser->checkWorldLevelBuff();
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool SceneServer::doGmToolsCmd(xNetProcessor* np, const BYTE* buf, WORD len)
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

        Json::Reader reader;
        Json::Value root;
        if (reader.parse(message.data().c_str(), root) == false)
        {
          XERR << "[GM-执行] json解析出错" << "msg" << message.data() << XEND;
          break;
        }

        QWORD qwCharID = 0;
        string command;
        string saveDb;
        try
        {
          qwCharID = atoll(root["player_id"].asString().c_str());
          command = root["command"].asString();
          saveDb = root["savedb"].asString();
        }
        catch(...)
        {
          XERR << "[GM-执行] json解析出错2" << "msg" << message.data() << XEND;
          break;
        }

        if (message.act() == exec_gm_cmd_player)
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(qwCharID);
          if (pUser == nullptr)
          {
            XERR << "[GM-执行] 找不到玩家" <<"charid"<<qwCharID<< "command" << command << XEND;
            break;
          }

          if (GMCommandRuler::getMe().execute(pUser, command) == false)
          {
            XERR << "[GM-执行] 执行失败" << "charid" << qwCharID << "command" << command << XEND;
            break;
          }

          XLOG << "[GM-执行] 执行成功" << "charid" << qwCharID << "command" << command<<"savedb"<< saveDb << XEND;

          if (saveDb == "true")
          {
            pUser->refreshDataToRecord(UnregType::GMSave);
          }

          RetExecGMCmd cmd;
          cmd.set_ret("\"result\":\"ok\"");
          cmd.set_conid(message.conid());
          PROTOBUF(cmd, send, len);
          np->sendCmd(send, len);
        }
      }
      break;

    case SESSION_GM_CMD:
      {
        PARSE_CMD_PROTOBUF(SessionGMCmd, message);

        // -- 指定玩家
        if (message.charid())
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
          if (pUser == nullptr)
          {
            XERR << "[Session-GM]" << "找不到玩家" << message.charid() << XEND;
            break;
          }
          xLuaData gmdata;
          if (gmdata.fromJsonString(message.data()) == true)
          {
            if (GMCommandRuler::getMe().execute(pUser, gmdata) == false)
            {
              XERR << "[Session-GM]" << "gm指令执行失败" << message.charid() << message.data() << XEND;
              break;
            }
          }
          else
          {
            if (GMCommandRuler::getMe().execute(pUser, message.data()) == false)
            {
              XERR << "[Session-GM]" << "gm指令执行失败" << message.charid() << message.data() << XEND;
              break;
            }
          }
        }
        // -- 指定地图
        else if(message.mapid())
        {
          Scene* pScene = SceneManager::getMe().getSceneByID(message.mapid());
          if (pScene == nullptr)
          {
            XERR << "[Session-GM]" << "gm指令执行失败, 找不到场景" << message.data() << XEND;
            break;
          }
          xLuaData gmdata;
          if (gmdata.fromJsonString(message.data()) == false || GMCommandRuler::getMe().scene_execute(pScene, gmdata) == false)
          {
            XERR << "[Session-GM]" << "gm指令执行失败" << message.charid() << message.data() << XEND;
            break;
          }
        }
        // -- 所有地图
        else
        {
          xLuaData gmdata;
          if (gmdata.fromJsonString(message.data()) == false || GMCommandRuler::getMe().scene_execute(nullptr, gmdata) == false)
          {
            XERR << "[Session-GM]" << "gm指令执行失败" << message.charid() << message.data() << XEND;
            break;
          }
        }
      }
      break;
  }

  return true;
}

bool SceneServer::forwardCmdToSessionTrade(QWORD charid, const std::string& name, const void* data, unsigned short len)
{
  SessionForwardScenecmdTrade cmd;
  cmd.set_charid(charid);
  cmd.set_name(name);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_data(data, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

bool SceneServer::doTradeCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  XLOG << "[交易] [收到来自Session 的消息] cmd param" << cmd->param << XEND;

  switch (cmd->param)
  {
  case REDUCE_MONEY_RECORDTRADE:  //买家，扣钱
  {
    PARSE_CMD_PROTOBUF(ReduceMoneyRecordTradeCmd, rev);
    Cmd::ETRADE_RET_CODE ret = ETRADE_RET_CODE_SUCCESS;
    std::string name;
    do
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == NULL)
      {
        XERR << "[交易-购买] 交易失败，找不到玩家" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_CANNOT_FIND_USER_IN_SCENE;
        break;
      }
      name = pUser->getName();

      //检查密码
      if(pUser->checkPwd(EUNLOCKTYPE_TRADE) == false)
      {
        XERR << "[交易-购买] 交易失败，密码不正确" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_FAIL;
        break;
      }
      if (!pUser->isRealAuthorized())
      {
        XERR << "[交易-购买] 交易失败，没有实名认证" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_FAIL;
        break;
      }

      //检查挂单费
      if (!pUser->checkMoney((EMoneyType)rev.money_type(), rev.total_money()))
      {
        XINF << "[交易-购买] 交易失败，金币不足" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH;
        MsgManager::sendMsg(pUser->id, 10154);
        break;
      }
      // 检查扣除额度
      if(rev.quota())
      {
        if(!pUser->getDeposit().checkQuota(rev.quota()))
        {
          XINF << "[交易-购买] 交易失败，额度不足" << rev.ShortDebugString() << XEND;
          ret = Cmd::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH;
          MsgManager::sendMsg(pUser->id, 25703);
          break;
        }
      }
      // 检查锁定额度
      if(rev.lock_quota())
      {
        if(!pUser->getDeposit().checkQuota(rev.lock_quota()))
        {
          XINF << "[交易-购买] 交易失败，额度不足" << rev.ShortDebugString() << XEND;
          ret = Cmd::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH;
          MsgManager::sendMsg(pUser->id, 25703);
          break;
        }
      }
      //扣钱
      ESource eSource = ESOURCE_TRADE;
      if (rev.item_info().has_publicity_id() && rev.item_info().publicity_id())
      {
        eSource = ESOURCE_TRADE_PUBLICITY;
      }

      if (!pUser->subMoney((EMoneyType)rev.money_type(), rev.total_money(), eSource))
      {
        XERR << "[交易-购买] 交易失败，扣钱失败" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH;
        MsgManager::sendMsg(pUser->id, 10154);
        break;
      }
      // 扣额度
      if(rev.quota())
      {
        if(!pUser->getDeposit().subQuota(rev.quota(), EQuotaType_C_Booth))
        {
          XERR << "[交易-购买] 交易失败，扣额度失败" << rev.ShortDebugString() << XEND;
          ret = Cmd::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH;
          MsgManager::sendMsg(pUser->id, 25703);
          break;
        }
      }
      // 锁定额度
      if(rev.lock_quota())
      {
        if(!pUser->getDeposit().lockQuota(rev.lock_quota(), EQuotaType_L_Booth))
        {
          XERR << "[交易-购买] 交易失败，锁定额度失败" << rev.ShortDebugString() << XEND;
          ret = Cmd::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH;
          MsgManager::sendMsg(pUser->id, 25703);
          break;
        }
      }
      pUser->getAchieve().onTradeBuy(rev.total_money());
      XINF << "[交易-购买] 扣钱成功" << rev.ShortDebugString() << XEND;

      // 交易所购买统计
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_TRADE_BUY, rev.item_info().itemid(), 0, pUser->getLevel(), rev.item_info().count());
      pUser->getShare().addNormalData(ESHAREDATATYPE_S_TRADECOST, rev.total_money());
      pUser->getShare().onTradeBuy(rev.item_info().itemid(), rev.item_info().count()*rev.item_info().price(), rev.item_info().refine_lv());

    } while (0);

    rev.set_ret(ret);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToSessionTrade(rev.charid(), name, send, len);
    return true;
  }
  break;
  case REDUCE_ITEM_RECORDTRADE:   //卖家，扣钱，扣装备
  {
    PARSE_CMD_PROTOBUF(ReduceItemRecordTrade, rev);
    Cmd::ETRADE_RET_CODE ret = ETRADE_RET_CODE_SUCCESS;
    std::string name;
    do
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == NULL)
      {
        ret = Cmd::ETRADE_RET_CODE_CANNOT_FIND_USER_IN_SCENE;
        XERR << "[交易-出售] 找不到玩家" << rev.ShortDebugString() << XEND;
        break;
      }
      name = pUser->getName();

      //检查密码
      if(!rev.is_resell() && pUser->checkPwd(EUNLOCKTYPE_TRADE) == false)
      {
        XERR << "[交易-出售] 交易失败，密码不正确" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_FAIL;
        break;
      }
      if (!rev.is_resell() && !pUser->isRealAuthorized())
      {
        XERR << "[交易-出售] 交易失败，没有实名认证" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_FAIL;
        break;
      }

      //检查挂单费
      if (!pUser->checkMoney(EMONEYTYPE_SILVER, rev.boothfee()))
      {
        ret = Cmd::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH;
        XERR << "[交易-出售] 银币不足" << rev.ShortDebugString() << XEND;
        MsgManager::sendMsg(pUser->id, 10109);
        break;
      }
      ESource eSource = ESOURCE_TRADE;
      if (rev.item_info().has_key() && !rev.item_info().key().empty())
      {
        eSource = ESOURCE_TRADE_PUBLICITY;
      }
      if (!rev.is_resell())
      {
        //扣除背包里的物品 预处理
        if (!pUser->getPackage().tradePreReduceItem(rev.mutable_item_info(), eSource))
        {
          ret = Cmd::ETRADE_RET_CODE_CANNOT_CANNOT_SELL; //需要详细的原因
          XERR << "[交易-出售] 物品不能出售" << rev.ShortDebugString() << XEND;
          break;
        }
      }

      //check money 上一步可能会消耗钱，这里再次判断
      if (!pUser->checkMoney(EMONEYTYPE_SILVER, rev.boothfee()))
      {
        ret = Cmd::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH;
        XERR << "[交易-出售] 银币不足" << rev.ShortDebugString() << XEND;
        MsgManager::sendMsg(pUser->id, 10109);
        break;
      }

      if(rev.quota_lock() || rev.quota_unlock())
      {
        // 检测并锁定额度
        if(!pUser->getDeposit().resell(rev.quota_unlock(), rev.quota_lock()))
        {
          ret = Cmd::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH;
          XERR << "[交易-出售] 额度不足" << rev.ShortDebugString() << XEND;
          MsgManager::sendMsg(pUser->id, 25703);
          break;
        }
      }

      //扣装备
      if (!rev.is_resell())
      {
        //扣除背包里的物品
        if (!pUser->getPackage().tradeReduceItem(rev.mutable_item_info(), eSource))
        {
          ret = Cmd::ETRADE_RET_CODE_CANNOT_CANNOT_SELL; //需要详细的原因
          XERR << "[交易-出售] 物品不能出售" << rev.ShortDebugString() << XEND;
          break;
        }
      }

      //扣钱     
      pUser->subMoney(EMONEYTYPE_SILVER, rev.boothfee(), eSource); //

      XINF << "[交易-出售] 成功" << rev.ShortDebugString() << XEND;

      //出售统计
      {
        StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_TRADE_SELL, rev.item_info().itemid(), 0, pUser->getLevel(), rev.item_info().count());
      }
    } while (0);
    rev.set_ret(ret);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToSessionTrade(rev.charid(), name, send, len);
    return true;
  }
  break;
  case ADD_ITEM_RECORDTRADE:    //加装备，或者撤单
  {
    PARSE_CMD_PROTOBUF(AddItemRecordTradeCmd, rev);
    XINF << "[交易-加装备] 消息：" << rev.ShortDebugString() << XEND;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == NULL)
    {
      XINF << "[交易-加装备] 给玩家加道具，找不到玩家，添加到邮件" << rev.ShortDebugString() << XEND;
      return MailManager::getMe().addOfflineTradeItem(rev);
    }

    if (!pUser->getPackage().tradeAddItem(rev.item_info()))
      return false;

    // 解锁额度
    if(rev.total_quota())
      pUser->getDeposit().unlockQuota(rev.total_quota(), EQuotaType_U_Booth);
        
    return true;
  }
  break;
  case ADD_MONEY_RECORDTRADE:   //卖家，加钱
  {
    PARSE_CMD_PROTOBUF(AddMoneyRecordTradeCmd, rev);

    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == NULL)
    {    
      XINF << "[交易-购买] 卖家加钱，找不到卖家，添加到邮件" << rev.ShortDebugString() << XEND;
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

        ForwardRegionSessionCmd cmd3;
        cmd3.set_region_type(DWORD(ClientType::global_server));
        cmd3.set_data(send, len);
        cmd3.set_len(len);
        PROTOBUF(cmd3, send2, len2);
        thisServer->sendCmdToSession(send2, len2);
      }

      return MailManager::getMe().addOfflineTradeMoney(rev);
    }
    XINF << "[交易-购买] 卖家加钱" << rev.ShortDebugString() << XEND;
    ESource eSource = ESOURCE_TRADE;
    if (rev.type() == EOperType_PublicitySellSuccess)
    {
      eSource = ESOURCE_TRADE_PUBLICITY;
    }
    else if (rev.type() == EOperType_PublicityBuyFail)
    {
      eSource = ESOURCE_TRADE_PUBLICITY_FAILRET;
    }

    return pUser->addMoney(static_cast<EMoneyType>(rev.money_type()), rev.total_money(), eSource);
  }
  break;
  case GIVE_CHECK_MONEY_RECORDTRADE:    //赠送
  {
    PARSE_CMD_PROTOBUF(GiveCheckMoneySceneTradeCmd, rev);    
    bool ret = false;
    do 
    {
      //reduce money and lock quota
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == NULL)
      {
        XERR << "[交易-赠送] 检测额度以及扣除zeny 失败，玩家不在线" << rev.charid() << "logtype" << rev.type() << "id" << rev.id() << "friendid" << rev.friendid() << "quota" << rev.quota() << "fee" << rev.fee() << XEND;
        ret = false;
        break;
      }

      //检查密码
      if(pUser->checkPwd(EUNLOCKTYPE_TRADE_GIFT) == false)
      {
        XERR << "[交易-赠送] 交易失败，密码不正确" << rev.ShortDebugString() << XEND;
        ret = Cmd::ETRADE_RET_CODE_FAIL;
        break;
      }

      if (pUser->getDeposit().checkQuota(rev.quota()) == false)
      {
        //send msg
        MsgManager::sendMsg(pUser->id, 25003);
        ret = false;
        XERR << "[交易-赠送] 检测额度以及扣除zeny,失败，额度不足" << rev.charid() << "logtype" << rev.type() << "id" << rev.id() << "friendid" << rev.friendid() << "quota" << rev.quota() << "fee" << rev.fee() << XEND;
        break;
      }
      
      if (pUser->checkMoney(EMONEYTYPE_SILVER, rev.fee()) == false)
      {
        //send msg
        MsgManager::sendMsg(pUser->id, 1);
        ret = false;
        XERR << "[交易-赠送] 检测额度以及扣除zeny,失败，zeny不足"<< rev.charid() << "logtype" << rev.type() << "id" << rev.id() << "friendid" << rev.friendid() << "quota" << rev.quota() << "fee" << rev.fee() << XEND;
        break;
      }
      pUser->subMoney(EMONEYTYPE_SILVER, (SQWORD)rev.fee(), ESOURCE_GIVE);
      pUser->getDeposit().lockQuota(rev.quota(), Cmd::EQuotaType_L_Give_Trade);
      ret = true;
    } while (0);
    
    rev.set_ret(ret);
    PROTOBUF(rev, send, len);
    if (sendCmdToSession(send, len) == false)
    {
      XLOG << "[交易-赠送] 检测额度以及扣除zeny发送返回给session,网络发送失败"  << rev.charid() << "logtype" << rev.type() << "id" << rev.id() << "friendid" << rev.friendid() << "quota" << rev.quota() << "fee" << rev.fee() << XEND;
      return false;
    }
    if (ret)
      XLOG << "[交易-赠送] 检测额度以及扣除zeny发送返回给session成功" << rev.charid() << "logtype" << rev.type() << "id" << rev.id() << "friendid" << rev.friendid() << "quota" << rev.quota() << "fee" << rev.fee() << XEND;
    return true;
  }
  break;
  case SYNC_GIVE_ITEM_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(SyncGiveItemSceneTradeCmd, rev);
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[交易-赠送] 同步赠送列表失败，玩家不在线" << rev.charid() << XEND;
        return false;
      }
      XLOG << "[交易-赠送] 同步赠送列表" << rev.charid() << XEND;
      pUser->m_oGingerBread.syncFromSession(rev);
      return true;
    }
    break;
  case ADD_GIVE_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(AddGiveSceneTradeCmd, rev);
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[交易-赠送接收者] ADD_GIVE_RECORDTRADE失败，玩家不在线" << rev.charid() << rev.ShortDebugString() << XEND;
        return false;
      }
      return pUser->m_oGingerBread.addOne(EGiveType_Trade ,rev.iteminfo().id(), rev.iteminfo().expiretime());
    }
    break;
  case DEL_GIVE_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(DelGiveSceneTradeCmd, rev);
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[交易-赠送接收者] DEL_GIVE_RECORDTRADE失败，玩家不在线" << rev.charid() << rev.ShortDebugString() << XEND;
        return false;
      }
      return pUser->m_oGingerBread.delOne(EGiveType_Trade, rev.id(), false, false);
    }
    break;
  case ADD_GIVE_ITEM_RECORDTRADE:     //赠送签收
    {
      PARSE_CMD_PROTOBUF(AddGiveItemSceneTradeCmd, rev);      
      do 
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          rev.set_ret(false);
          XERR << "[交易-赠送接收者] 增加物品失败，玩家不在线" << rev.charid() << rev.ShortDebugString() << XEND;
          break;
        }

        string key = pUser->getTicket().getKeyByParam(ETicketType_AddGiveItem, rev.id());
        ETicketRet ticketRet = pUser->getTicket().getTicketRet(key);
        if (ticketRet == ETickRet_Ok)
        {
          rev.set_ret(true);
          XERR << "[交易-赠送接收者] 增加物品失败，已经加过了" << rev.charid()<<"key"<< key << rev.ShortDebugString() << XEND;
          break;
        }

        // 删除预附魔
        if(rev.itemdata().has_previewenchant())
        {
          rev.mutable_itemdata()->clear_previewenchant();
        }

        if (rev.itemdata().has_base())
        {
          ItemData itemData = rev.itemdata();
          itemData.mutable_base()->set_source(ESOURCE_GIVE);
          if (pUser->getPackage().addItem(itemData, EPACKMETHOD_AVAILABLE) == false)
          {
            rev.set_ret(false);
            XERR << "[交易-赠送接收者] 增加物品失败，加道具失败" << rev.charid() << rev.ShortDebugString() << XEND;
            break;
          }
        }
        else
        {
          ItemInfo itemInfo;
          itemInfo.set_id(rev.itemid());
          itemInfo.set_count(rev.count());
          itemInfo.set_source(ESOURCE_GIVE); 
          if (pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE) == false)
          {
            rev.set_ret(false);
            XERR << "[交易-赠送接收者] 增加物品失败，加带属性道具失败" << rev.charid() << rev.ShortDebugString() << XEND;
            break;
          }
        }
        rev.set_ret(true);
        pUser->getTicket().setTicketRet(ETicketType_AddGiveItem,key, ETickRet_Ok);
        
        //
        const SGiveBackground* pCfg = MiscConfig::getMe().getTradeCFG().getGiveBackGround(rev.background());
        if (pCfg && pCfg->dwBuffid)
        {
          pUser->m_oBuff.add(pCfg->dwBuffid);
          XDBG << "[[交易-赠送接收者] 增加物品成功-签收添加buff] " << rev.charid() << "background" << rev.background() << "buffid" << pCfg->dwBuffid << XEND;
        }

        XLOG << "[交易-赠送接收者] 增加物品成功" << rev.charid() << "key" << key << rev.ShortDebugString() << XEND;
        pUser->m_oGingerBread.delOne(EGiveType_Trade, rev.id(), true, false);

      } while (0);

      PROTOBUF(rev, send, len);
      if (thisServer->sendCmdToSession(send, len) == false)
      {
        XERR << "[交易-赠送接收者] 增加物品成功,发送到session失败" <<rev.charid() << rev.ShortDebugString() << XEND;
        return false;
      }
      return true;
    }
    break;
  case UNLOCK_QUOTA_RECORDTRADE:
    {
      PARSE_CMD_PROTOBUF(UnlockQuotaSceneTradeCmd, rev);
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if(pUser == nullptr)
      {
        // 添加离线消息
        AddOfflineMsgSocialCmd cmd;
        OfflineMsg* pMsg = cmd.mutable_msg();
        if (!pMsg)
        {
          XERR << "[交易-赠送者] 解锁额度，玩家不在线, 添加离线消息失败，获取offlinemsg失败" << rev.charid() << rev.ShortDebugString() << XEND;
          return false;
        }

        pMsg->set_type(EOFFLINEMSG_USER_QUOTA);
        pMsg->set_targetid(rev.charid());

        OffMsgUserQuotaData* pData = pMsg->mutable_quotadata();
        if(!pData)
        {
          XERR << "[交易-赠送者] 解锁额度，玩家不在线, 添加离线消息失败，获取OffMsgUserQuotaData失败" << rev.charid() << rev.ShortDebugString() << XEND;
          return false;
        }

        pData->set_quota(rev.quota());
        pData->set_oper(EUSERQUOTAOPER_UNLOCK);
        pData->set_type(EQuotaType_U_Give_Trade);

        PROTOBUF(cmd, send2, len2);
        thisServer->sendCmdToSession(send2, len2);
        XLOG << "[交易-赠送者] 解锁额度，玩家不在线, 添加离线消息成功" << rev.charid() << rev.ShortDebugString() << XEND;
        return true;
      }
      pUser->getDeposit().unlockQuota(rev.quota(), EQuotaType_U_Give_Trade);
      XLOG << "[交易-赠送者] 解锁额度成功" << rev.charid() << rev.ShortDebugString() << XEND;
      return true;
    }
    break;
  case REDUCE_QUOTA_RECORDTRADE:
  {
    PARSE_CMD_PROTOBUF(ReduceQuotaSceneTradeCmd, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
    {
      // 添加离线消息
      AddOfflineMsgSocialCmd cmd;
      OfflineMsg* pMsg = cmd.mutable_msg();
      if (!pMsg)
      {
        XERR << "[交易-赠送者] 扣除锁定额度，玩家不在线, 添加离线消息失败，获取offlinemsg失败" << rev.charid() << rev.ShortDebugString() << XEND;
        return false;
      }

      pMsg->set_type(EOFFLINEMSG_USER_QUOTA);
      pMsg->set_targetid(rev.charid());

      OffMsgUserQuotaData* pData = pMsg->mutable_quotadata();
      if(!pData)
      {
        XERR << "[交易-赠送者] 扣除锁定额度，玩家不在线, 添加离线消息失败，获取OffMsgUserQuotaData失败" << rev.charid() << rev.ShortDebugString() << XEND;
        return false;
      }

      pData->set_quota(rev.quota());
      pData->set_oper(EUSERQUOTAOPER_UNLOCK_SUB);
      pData->set_type(EQuotaType_U_Give_Trade);

      PROTOBUF(cmd, send2, len2);
      thisServer->sendCmdToSession(send2, len2);
      XLOG << "[交易-赠送者] 扣除锁定额度，玩家不在线, 添加离线消息成功" << rev.charid() << rev.ShortDebugString() << XEND;
      return true;
    }
    pUser->getDeposit().unlockQuota(rev.quota(), EQuotaType_U_Give_Trade, true);

    //您的好友[63cd4e]%s[-]已经成功签收了你的礼物！扣除[63cd4e]%s[-]额度，剩余[63cd4e]%s[-]额度。
    MsgParams param;
    param.addString(rev.receivername());
    param.addNumber(rev.quota());
    param.addNumber(pUser->getDeposit().getQuota());
    MsgManager::sendMsg(pUser->id, 25000, param);

    XLOG << "[交易-赠送者] 扣除锁定额度成功" << rev.charid() << rev.ShortDebugString() << XEND;
    PROTOBUF(rev, send, len);
    if (thisServer->sendCmdToSession(send, len) == false)
    {
      XERR<< "[交易-赠送者] 扣除额度成功,发送到session失败" << rev.charid() << rev.ShortDebugString() << XEND;
      return false;
    }
    return true;
  }
  break;
  }
  return false;
}

bool SceneServer::doMatchCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  XLOG << "[斗技场] [收到来自Session 的消息] cmd param" << cmd->param << XEND;

  switch (cmd->param)
  {
  case MATCHSPARAM_ENTER_PVP_MAP: 
  {
    PARSE_CMD_PROTOBUF(EnterPvpMapSCmdMatch, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
    {
      XERR<< "[斗技场-玩家传送到副本], 找不到玩家了" << rev.charid() << "destzoneid" << rev.dest_zoneid() << "mapid" << rev.raidid()<<"roomid"<<rev.room_guid() << XEND;
      return false;
    }

    pUser->getUserZone().setRoomId(rev.room_guid());
    if (rev.colorindex())
    {
      pUser->getUserZone().setColorIndex(rev.colorindex());
    }
    pUser->getUserZone().gomap(rev.dest_zoneid(), rev.raidid(), GoMapType::Null);    
    XLOG << "[斗技场-玩家传送到副本]" << rev.charid() << "destzoneid" << rev.dest_zoneid() << "mapid" << rev.raidid()<< "roomid" << rev.room_guid()<<"colorindex"<<rev.colorindex() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_SYNC_ROOMINFO:
  {
    PARSE_CMD_PROTOBUF(SyncRoomSceneMatchSCmd, rev);
    Scene* pScene = SceneManager::getMe().getSceneByID(rev.sceneid());
    if (pScene)
    {
      MatchScene* pBScene = dynamic_cast<MatchScene*>(pScene);
      if (pBScene)
      {
        pBScene->onReceiveRoomInfo(rev);
      }
      XLOG << "[斗技场-房间信息同步], 收到房间信息, 副本:" << pScene->id << "info:" << rev.ShortDebugString() << XEND;
    }
    else
    {
      XERR << "[斗技场-房间信息同步], 找不到对应副本:" << rev.sceneid() << XEND;
    }
    return true;
  }
  break;
  case MATCHSPARAM_USER_BOOTH_NTF:
  {
    PARSE_CMD_PROTOBUF(UserBoothNTFMatchSCmd, rev);

    XLOG << "[斗技场-摊位同步], 收到摆摊信息, info:" << rev.ShortDebugString() << XEND;

    if(EBOOTHOPER_OPEN == rev.oper())
    {
      // 检测是否同一个zone
      if(rev.zoneid() == thisServer->getZoneID())
        return true;
      // 检测同步摊位数量
      if(MiscConfig::getMe().getBoothCFG().getMaxSizeOneScene() <= SceneBoothManager::getMe().getCountBySceneId(rev.sceneid()))
        return true;

      Scene* pScene = SceneManager::getMe().getSceneByID(rev.sceneid());
      if(!pScene)
      {
        XERR << "[斗技场-摊位同步]获取场景失败, 地图:" << rev.sceneid() << "info:" << rev.ShortDebugString() << XEND;
        return true;
      }

      SceneBooth* booth = SceneBoothManager::getMe().createSceneBooth(pScene, rev.zoneid(), rev.user());
      if(!booth)
      {
        XERR << "[斗技场-摊位同步]创建摊位失败, 地图:" << pScene->id << "info:" << rev.ShortDebugString() << XEND;
        return true;
      }
    }
    else if(EBOOTHOPER_UPDATE == rev.oper())
    {
      SceneBooth* booth = SceneBoothManager::getMe().getSceneBooth(rev.user().guid());
      if(!booth)
      {
        XERR << "[斗技场-摊位同步]更新摊位失败, info:" << rev.ShortDebugString() << XEND;
        return true;
      }

      booth->update(rev.user().info().name());
    }
    else if(EBOOTHOPER_CLOSE == rev.oper())
    {
      if(!SceneBoothManager::getMe().delBooth(rev.user().guid()))
      {
        XERR << "[斗技场-摊位同步]关闭摊位失败, info:" << rev.ShortDebugString() << XEND;
        return true;
      }
    }
  }
  break;
  case MATCHSPARAM_SYNC_TEAM_INFO:
  {
    PARSE_CMD_PROTOBUF(SyncTeamInfoMatchSCmd, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
    {
      XERR << "[斗技场-同步队伍信息], 找不到玩家了" << rev.charid() << "teamid" << rev.teamid() << "index" << rev.index() << XEND;
      return false;
    }
    /*pUser->setPvpColor(rev.index());*/
    XLOG << "[斗技场-同步队伍信息]" << rev.charid() << "teamid" << rev.teamid() << "index" << rev.index() << XEND;
    return true;
  }
  break;
  case MATCHSPARAM_CHECK_CAN_BUY:
  {
    PARSE_CMD_PROTOBUF(CheckCanBuyMatchSCmd, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
      return false;
    bool bRet = false;
    if (rev.success() == false)
    {
      bRet = false;
    }
    else
    {
      bRet = pUser->getSceneShop().buyItem(rev.id(), rev.count(), rev.price(), rev.price2(), true);
    }
    if (bRet == false)
    {
      BuyShopItem cmd;
      cmd.set_id(rev.id());
      cmd.set_count(rev.count());
      cmd.set_price(rev.price());
      cmd.set_price2(rev.price2());
      cmd.set_success(false);
      PROTOBUF(cmd, send, len);
      pUser->sendCmdToMe(send, len);
    }
    return true;
  }
  break;
  case MATCHSPARAM_POLLY_ACTIVITY:
  {
    PARSE_CMD_PROTOBUF(ActivityMatchSCmd, rev);
    if (rev.open() == false)
    {
      std::set<Scene*> allScenes;
      SceneManager::getMe().getAllSceneByType(SCENE_TYPE_MVPBATTLE, allScenes);
      DWORD cur = now();
      for (auto &s : allScenes)
      {
        MvpBattleScene* pMScene = dynamic_cast<MvpBattleScene*> (s);
        if (pMScene)
          pMScene->setCloseTime(cur);
      }
      XLOG << "[Mvp-竞争战], 活动结束, 关闭场景副本" << XEND;
    }
    return true;
  }
  break;
  case MATCHSPARAM_SYNC_SCORE:
  {
    PARSE_CMD_PROTOBUF(SyncUserScoreMatchSCmd, rev);
    SceneUser* user = SceneUserManager::getMe().getUserByID(rev.charid());
    if (!user)
      return false;
    if (rev.etype() == EPVPTYPE_TEAMPWS)
    {
      user->getMatchData().setPwsSeason(rev.season());
      user->getMatchData().setPwsScore(rev.score());
      user->getMenu().refreshNewMenu(EMENUCOND_TEAMPWS);
    }
    return true;
  }
  break;
  default:
    break;
  }
  return false;
}

bool SceneServer::doAuctionCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  XLOG << "[拍卖行] [收到来自Session 的消息] cmd param" << cmd->param << XEND;

  switch (cmd->param)
  {
  case AUCTIONSPARAM_SIGNUP_ITEM:
  {
    PARSE_CMD_PROTOBUF(SignUpItemSCmd, rev);
    bool ret = false;
    SceneUser* pUser = nullptr;
    string name;
    do 
    {
      pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[拍卖行-报名上架] 扣除物品, 找不到玩家了" << rev.charid() << "上架物品" << rev.iteminfo().itemid() << XEND;
        ret = false;
        break;
      }
      if(pUser->checkPwd(EUNLOCKTYPE_AUCTION_SELL) == false)
      {
        XERR << "[拍卖行-报名上架] 安全密码验证失败" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << rev.iteminfo().itemid() << XEND;
        ret = false;
        break;
      }
      if (!pUser->isRealAuthorized())
      {
        XERR << "[拍卖行-报名上架] 没有实名认证" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << rev.iteminfo().itemid() << XEND;
        ret = false;
        break;
      }
      DWORD itemId = rev.iteminfo().itemid();
      name = pUser->name;
      
      MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
      if (pMainPack == nullptr)
      {
        ret = false;
        break;
      }

      if (pMainPack->checkItemCount(itemId) == false)
      {
        XERR << "[拍卖行-报名上架] 扣除物品失败，找不到物品 charid"<< rev.charid() <<"物品"<< itemId << XEND;
        ret = false;
        break;
      }

      if(!rev.iteminfo().auction())
      {
        ret = false;
        break;
      }
      else if(1 == rev.iteminfo().auction())
      {
        pMainPack->reduceItem(itemId, ESOURCE_AUCTION_SIGNUP);
      }
      else if(2 == rev.iteminfo().auction())
      {
        if(rev.guid().empty())
        {
          ret = false;
          break;
        }

        // 检测装备上架条件
        DWORD dwFmPoint = 0;
        DWORD dwFmBuff = 0;
        if(!pMainPack->checkSignUp(rev.guid(), dwFmPoint, dwFmBuff))
        {
          XERR << "[拍卖行-报名上架] 检测上架失败"<< rev.ShortDebugString() << XEND;
          ret = false;
          break;
        }

        ItemEquip* pItemEquip = dynamic_cast<ItemEquip*>(pMainPack->getItem(rev.guid()));
        if(!pItemEquip)
        {
          XERR << "[拍卖行-报名上架] 获取ItemEquip失败"<< rev.ShortDebugString() << XEND;
          ret = false;
          break;
        }

        ItemData* pData = rev.mutable_itemdata();
        if(!pData)
        {
          XERR << "[拍卖行-报名上架] 获取ItemData失败"<< rev.ShortDebugString() << XEND;
          ret = false;
          break;
        }

        pItemEquip->toItemData(pData);
        rev.set_fm_buff(dwFmBuff);
        rev.set_fm_point(dwFmPoint);

        pMainPack->reduceItem(rev.guid(), ESOURCE_AUCTION_SIGNUP);
      }

      XLOG << "[拍卖行-报名上架] 扣除物品成功,charid" << rev.charid() << "上架物品" << itemId << XEND;
      ret = true;
    } while (0);
    rev.set_ret(ret);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToAuction(rev.charid(), name, send, len);
    if (ret&& pUser)
    {
      pUser->getTicket().addTicketCmd(ETicketCmdType_Auction, send, len, rev.orderid());
    }

    return true;
  }
  break;
  case AUCTIONSPARAM_OFFER_PRICE:
  {
    PARSE_CMD_PROTOBUF(OfferPriceSCmd, rev);
    bool ret = false;
    string name;
    SceneUser* pUser = nullptr;
    do 
    {
      pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        XERR << "[拍卖行-竞拍出价], 找不到玩家了" << rev.charid() << "场次" << rev.batchid() << "itemid" << rev.itemid() << "扣钱" << rev.reduce_money() << "总出价" << rev.total_price() << XEND;
        ret = false;
        break;
      }
      if(pUser->checkPwd(EUNLOCKTYPE_AUCTION_BUY) == false)
      {
        XERR << "[拍卖行-报名上架] 安全密码验证失败" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << rev.itemid() << XEND;
        ret = false;
        break;
      }
      if (!pUser->isRealAuthorized())
      {
        XERR << "[拍卖行-报名上架] 没有实名认证" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << rev.itemid() << XEND;
        ret = false;
        break;
      }
      name = pUser->name;
      if (pUser->checkMoney(EMONEYTYPE_LOTTERY, rev.reduce_money()) == false)
      {
        MsgManager::sendMsg(pUser->id, 9513);        
        XERR << "[拍卖行-竞拍出价], b格猫金币不够" << rev.charid() << "场次" << rev.batchid() << "itemid" << rev.itemid() << "扣钱" << rev.reduce_money() << "总出价" << rev.total_price() << XEND;
        ret = false;
        break;
      }      
      pUser->subMoney(EMONEYTYPE_LOTTERY, rev.reduce_money(), ESOURCE_AUCTION_OFFERPRICE);
      ret = true;
      XLOG << "[拍卖行-竞拍出价], 成功扣除zeny" << rev.charid() << "场次" << rev.batchid() << "itemid" << rev.itemid() << "扣钱" << rev.reduce_money() << "总出价" << rev.total_price() << XEND;
    } while (0);
    rev.set_ret(ret);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToAuction(rev.charid(), name, send, len);
   /* if (ret&& pUser)
    {
      pUser->getTicket().addTicketCmd(ETicketCmdType_Auction, send, len, rev.orderid());
    }*/
  }
  break;
  case AUCTIONSPARAM_TAKE_RECORD:
  {
    PARSE_CMD_PROTOBUF(Cmd::TakeRecordSCmd, rev);
    string name;
    do 
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
      if (pUser == nullptr)
      {
        rev.set_ret(false);
        XERR << "[拍卖行-加道具], 找不到玩家了" << rev.charid() << "type" << rev.type() <<"id"<< rev.id() << "zeny" << rev.zeny() << "itemid" << rev.item().id() << XEND;
        break;
      }
      name = pUser->name;
      
      //去重
      string key = pUser->getTicket().getKeyByParam(ETicketType_AuctionTake, rev.type(), rev.id());
      ETicketRet ticketRet = pUser->getTicket().getTicketRet(key);
      if (ticketRet == ETickRet_Ok)
      {
        rev.set_ret(true);
        XLOG << "[拍卖行-加道具] 增加物品失败，已经加过了" << rev.charid() << "key" << key << rev.ShortDebugString() << XEND;
        break;
      }     

      if (pUser->getPackage().auctionAddItem(rev))
      {
        rev.set_ret(true);
        pUser->getTicket().setTicketRet(ETicketType_AuctionTake, key, ETickRet_Ok);
      }
      else
        rev.set_ret(false);
    } while (0);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToAuction(rev.charid(), name, send, len);
    return true;
  }
  break;
  case AUCTIONSPARAM_OFFER_PRICE_DEL_ORDER:
  {
    PARSE_CMD_PROTOBUF(Cmd::OfferPriceDelOrderSCmd, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
    {
      return true;
    }
    pUser->getTicket().delTicektCmd(ETicketCmdType_Auction, rev.orderid());
  }
  break;
  default:
    break;
  }
  return false;
}

bool SceneServer::doSocialCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case SOCIALPARAM_GO_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(GoTeamRaidSocialCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser)
        {
          if (message.raidid() && message.raidzoneid())
          {
            pUser->getUserZone().setRaidZoneID(message.raidid(), message.raidzoneid());
            pUser->gomap(message.raidid(), (GoMapType)message.gomaptype());
          }
        }
        return true;
      }
      break;
    case SOCIALPARAM_USER_ADD_ITEM:
      {
        PARSE_CMD_PROTOBUF(UserAddItemSocialCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.user().charid());
        ESource eType = ESOURCE_MIN;
        TVecItemInfo vecItems;
        for (int i = 0; i < message.items_size(); ++i)
        {
          vecItems.push_back(message.items(i));
          eType = message.items(i).source();
        }

        bool isOpen = false;
        if(eType == ESOURCE_DONATE && message.doublereward() != 0)
        {
          DWORD activityid = 0;
          DWORD times = 1;
          /*const SGlobalActivityCFG* pCFG = MiscConfig::getMe().getGlobalActivityCFG(GACTIVITY_GUILD_DONATE);
          if(pCFG != nullptr)
          {
            activityid = pCFG->activityid;
            times = pCFG->times;
          }*/
          const SGlobalActCFG*pCFG = ActivityConfig::getMe().getGlobalActCFG(GACTIVITY_GUILD_DONATE);
          if (pCFG)
          {
            activityid = pCFG->m_dwId;
            times = pCFG->getParam(0);;
          }
          isOpen = ActivityManager::getMe().isOpen(activityid);
          if(isOpen)
          {
            for (auto &item : vecItems)
            {
              if(item.id() == EMONEYTYPE_GUILDASSET)
                continue;
              DWORD count = item.count() * times;
              item.set_count(count);
            }
            XLOG << "[包裹-Reward奖励获得] 双倍奖励,工会捐赠" << message.user().charid() << message.user().name() << "type" << message.doublereward() << XEND;
          }
        }

        bool isActivityEventOpen = false;
        if (pUser != nullptr)
        {
          if (eType == ESOURCE_DONATE && message.doublereward() != 0)
          {
            DWORD donatecnt = pUser->getVar().getVarValue(EVARTYPE_GUILD_DONATE_DAY) + 1;
            pUser->getVar().setVarValue(EVARTYPE_GUILD_DONATE_DAY, donatecnt);

            // 额外奖励/多倍奖励
            DWORD times = 1;
            TVecItemInfo extrareward;
            if (ActivityEventManager::getMe().getReward(pUser, EAEREWARDMODE_GUILD_DONATE, donatecnt, extrareward, times))
            {
              isActivityEventOpen = true;
              if (isOpen == false && times > 1)
              {
                for (auto& item : vecItems)
                {
                  if(item.id() == EMONEYTYPE_GUILDASSET)
                    continue;
                  item.set_count(item.count() * times);
                }
                XLOG << "[添加道具-公会捐赠]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动奖励翻倍:" << times << XEND;
              }
              if (extrareward.empty() == false)
              {
                XLOG << "[添加道具-公会捐赠]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动额外奖励:";
                for (auto& v : extrareward)
                  XLOG << v.id() << v.count();
                XLOG << XEND;
                combinItemInfo(vecItems, extrareward);
              }
            }
          }
          if(eType == ESOURCE_DONATE)
          {
            float zenyratio = pUser->m_oBuff.getExtraZenyRatio(EEXTRAREWARD_GUILD_DONATE);
            XLOG << "[回归-Buff], 公会捐赠, 额外zeny倍率:" << zenyratio << "玩家:" << pUser->name << pUser->id << XEND;
            DWORD extracontri = 0;
            if (zenyratio)
            {
              for (auto &v : vecItems)
              {
                if (v.id() == ITEM_CONTRIBUTE)
                  extracontri += v.count() * zenyratio;
                if (v.id() == ITEM_ZENY)
                  v.set_count((1+zenyratio) * v.count());
              }
              if (extracontri)
              {
                pUser->addMoney(EMONEYTYPE_CONTRIBUTE, extracontri, ESOURCE_DONATE);
                MsgManager::sendMsg(pUser->id, 3637, MsgParams(extracontri));
              }
            }
            pUser->m_oBuff.onFinishEvent(EEXTRAREWARD_GUILD_DONATE);
            pUser->getServant().onFinishEvent(ETRIGGER_GUILD_DONATE);
            pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_DONATE);
          }

          pUser->getPackage().addItem(vecItems, EPACKMETHOD_AVAILABLE);
          XLOG << "[转发-加道具]直接加 charid:"<<pUser->id <<pUser->name <<message.ShortDebugString() << XEND;

          if(eType == ESOURCE_DONATE)
            pUser->getExtraReward(EEXTRAREWARD_GUILD_DONATE);
          else if(eType == ESOURCE_OPERATE)
          {
            pUser->addOperateRewardVar(message.operatereward());
          }
        }
        else
        {
          if (isActivityEventOpen == false)
          {

            if (MailManager::getMe().sendMail(message.user().charid(), 0, "", SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_MAIL), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_COMPENSATE), EMAILTYPE_NORMAL, 0, vecItems, TVecItemData{}, EMAILATTACHTYPE_ITEM))
              XINF << "[加道具] 存到离线邮件成功 charid:" << message.user().charid() << message.ShortDebugString() << XEND;
            else
              XERR << "[加道具] 存到离线邮件失败 charid:" << message.user().charid() << message.ShortDebugString() << XEND;

          }
          else
          {
            // 将奖励保存至离线数据
            AddOfflineMsgSocialCmd cmd;
            OfflineMsg* pMsg = cmd.mutable_msg();
            if (pMsg)
            {
              pMsg->set_type(EOFFLINEMSG_USER_ADD_ITEM);
              pMsg->set_targetid(message.user().charid());
              pMsg->mutable_useradditem()->set_type(EUSERADDITEMTYPE_GUILD_DONATE);
              for (int i = 0; i < message.items_size(); ++i)
              {
                ItemInfo* p = pMsg->mutable_useradditem()->add_items();
                if (p)
                  p->CopyFrom(message.items(i));
              }
              PROTOBUF(cmd, send, len2);
              thisServer->sendCmdToSession(send, len2);
            }
          }
        }
      }
      return true;
    case SOCIALPARAM_CHAT_MSG:
      ChatManager_SC::getMe().doSocialCmd(buf, len);
      return true;
    case SOCIALPARAM_CREATEGUILD:
      {
        PARSE_CMD_PROTOBUF(CreateGuildSocialCmd, message);
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.user().user().charid());
          if (pUser == nullptr)
          {
            message.set_msgid(10);
            break;
          }

          const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();

          // check level
          if (pUser->getUserSceneData().getRolelv() < rCFG.dwCreateBaseLv)
          {
            message.set_msgid(10);
            break;
          }

          // check item
          MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
          if (pMainPack == nullptr || pMainPack->checkItemCount(rCFG.vecCreateItems) == false)
          {
            message.set_msgid(10);
            break;
          }

          pMainPack->reduceItem(rCFG.vecCreateItems, ESOURCE_GUILDCREATE);
        } while(0);

        PROTOBUF(message, send, len);
        sendCmdToSession(send, len);
      }
      return true;
    case SOCIALPARAM_GUILDDONATE:
      {
        PARSE_CMD_PROTOBUF(GuildDonateSocialCmd, message);

        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.user().charid());
        if (pUser != nullptr)
        {
          do
          {
            /*BasePackage* pPackage = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
            if (pPackage == nullptr)
            {
              message.set_msgid(10);
              break;
            }

            if (pPackage->checkItemCount(message.item().itemid(), message.item().itemcount()) == false)
            {
              message.set_msgid(10);
              break;
            }
            pPackage->reduceItem(message.item().itemid(), ESOURCE_DONATE, message.item().itemcount());*/

            Package& rPackage = pUser->getPackage();
            ItemInfo oItem;
            oItem.set_id(message.item().itemid());
            oItem.set_count(message.item().itemcount());

            if (rPackage.checkItemCount(oItem, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_GUILDDONATE) == false)
            {
              message.set_msgid(10);
              break;
            }
            rPackage.reduceItem(oItem, ESOURCE_DONATE, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_GUILDDONATE);
            pUser->stopSendInactiveLog();
          } while(0);

          PROTOBUF(message, send1, len1);
          thisServer->sendCmdToSession(send1, len1);
          XLOG << "[公会捐赠]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "进行公会捐赠, 结果 msgid :" << message.msgid() << XEND;
        }
      }
      return true;
    case SOCIALPARAM_GUILD_EXCHANGEZONE:
      {
        PARSE_CMD_PROTOBUF(GuildExchangeZoneSocialCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.user().charid());
        if (pUser != nullptr)
          pUser->getUserSceneData().setZoneID(message.zoneid(), EJUMPZONE_GUILD);
      }
      return true;
    case SOCIALPARAM_TOWER_SCENE_CREATE:
      {
        PARSE_CMD_PROTOBUF(TowerSceneCreateSocialCmd, rev);
        SceneUser* pLeader = SceneUserManager::getMe().getUserByID(rev.user().charid());
        if (pLeader == nullptr)
          return true;

        pLeader->getTower().resetData();

        bool bSuccess = false;
        const STowerLayerCFG* pCFG = nullptr;
        const SRaidCFG* pRaidCFG = nullptr;
        do
        {
          //最高层判断放在TeamServer
          //if (pLeader->getTower().canEnter(rev.layer()) == false)
          //{
          //  XERR << "[无限塔]" << pLeader->accid << pLeader->id << pLeader->getProfession() << pLeader->name << "创建无限塔副本" << rev.layer() << "失败,超过最高挑战层数" << XEND;
          //  break;
          //}
          pCFG = TowerConfig::getMe().getTowerLayerCFG(rev.layer());
          if (pCFG == nullptr)
          {
            XERR << "[无限塔]" << pLeader->accid << pLeader->id << pLeader->getProfession() << pLeader->name << "创建无限塔副本" << rev.layer() << "失败,未找到该层配置" << XEND;
            break;
          }
          pRaidCFG = MapConfig::getMe().getRaidCFG(pCFG->dwRaidID);
          if (pRaidCFG == nullptr)
          {
            XERR << "[无限塔]" << pLeader->accid << pLeader->id << pLeader->getProfession() << pLeader->name
              << "创建无限塔副本" << rev.layer() << "失败,未找到该层副本" << pCFG->dwRaidID << "配置" << XEND;
            break;
          }

          bSuccess = true;
        } while(0);

        if (bSuccess)
        {
          CreateDMapParams params;
          params.dwRaidID = pCFG->dwRaidID;
          params.qwCharID = pLeader->id;
          params.eRestrict = pRaidCFG->eRestrict;
          params.dwLayer = rev.layer();
          params.m_dwNoMonsterLayer = pLeader->getTower().calcNoMonsterLayer();
          SceneManager::getMe().createDScene(params);
        }
        else
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(rev.teamid());
          cmd.set_state(EDOJOSTATE_CLOSE);
          PROTOBUF(cmd, send, len);
          sendCmdToServer(send, len, "SessionServer");
        }
      }
      return true;
    case SOCIALPARAM_SOCIAL_SYNC_LIST:
      {
        PARSE_CMD_PROTOBUF(SyncSocialListSocialCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[社交消息-社交列表同步] charid :" << rev.charid() << "玩家不存在" << XEND;
          return true;
        }
        pUser->getSocial().initSocial(rev);
        pUser->getAchieve().onTutorGuide();
        pUser->getAchieve().onAddFriend();
        pUser->getTutorTask().onLevelUp();
        pUser->getTutorTask().onUpdateSocial();
        pUser->onSocialChange();
      }
      return true;
    case SOCIALPARAM_SOCIAL_LIST_UPDATE:
      {
        PARSE_CMD_PROTOBUF(SocialListUpdateSocialCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[社交消息-社交列表更新] charid :" << rev.charid() << "玩家不存在" << XEND;
          return true;
        }
        QWORD qwOldTutorID = pUser->getSocial().getTutorCharID();
        pUser->getSocial().updateSocial(rev);
        QWORD qwNewTutorID = pUser->getSocial().getTutorCharID();

        pUser->getAchieve().onTutorGuide();
        pUser->getAchieve().onAddFriend();
        pUser->getTutorTask().onUpdateSocial();

        pUser->onSocialChange();

        if (qwOldTutorID != qwNewTutorID)
          pUser->getTutorTask().onTutorChanged(qwOldTutorID, qwNewTutorID);
      }
      return true;
    case SOCIALPARAM_SYNC_REDTIP:
      {
        PARSE_CMD_PROTOBUF(SyncRedTipSocialCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[红点同步-失败] charid :" << rev.charid() << "玩家不存在" << XEND;
          return true;
        }
        if(rev.add() == true)
          pUser->getTip().addRedTip(rev.red());
        else
          pUser->getTip().removeRedTip(rev.red());
      }
      return true;
    case SOCIALPARAM_SEND_TUTOR_REWARD:
      {
        // 登录同步导师奖励
        PARSE_CMD_PROTOBUF(SendTutorRewardSocialCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          for (int i = 0; i < rev.rewards_size(); ++i)
            pUser->getTutorTask().addTutorReward(rev.rewards(i));
          pUser->getTutorTask().sendTutorRewardMail();
        }
        else
        {
          XERR << "[导师-登录同步导师奖励] charid:" << rev.charid() << "玩家找不到";
          for (int i = 0; i < rev.rewards_size(); ++i)
          {
            XERR << "学生:" << rev.rewards(i).charid() << "奖励:";
            for (int j = 0; j < rev.rewards(i).reward_size(); ++j)
              XERR << rev.rewards(i).reward(j);
            XERR << "任务:";
            for (int j = 0; j < rev.rewards(i).item_size(); ++j)
              XERR << rev.rewards(i).item(j).taskid() << rev.rewards(i).item(j).time();
          }
          XERR << XEND;
        }
      }
      return true;
    case SOCIALPARAM_SYNC_TUTOR_REWARD:
      {
        PARSE_CMD_PROTOBUF(SyncTutorRewardSocialCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          if (rev.has_reward())
            pUser->getTutorTask().addTutorReward(rev.reward());
          else if (rev.redpointtip())
            pUser->getTutorTask().addRedPointTip(rev.redpointtip());
        }
        else
        {
          // 找不到玩家, 发送到session找/存离线数据
          if (rev.has_reward())
          {
            rev.set_searchuser(false);
            PROTOBUF(rev, send, len1);
            thisServer->sendCmdToSession(send, len1);
          }
        }
      }
      return true;
    default:
      return false;
  }

  return false;
}

bool SceneServer::doTeamCmd(BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case SERVERTEAMPARAM_TEAMDATA_SYNC:
      {
        PARSE_CMD_PROTOBUF(TeamDataSyncTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          GTeam stOldTeam = pUser->getTeam();
          pUser->getTeam().updateTeam(rev);
          pUser->onTeamChange(stOldTeam, pUser->getTeam(), rev.online());
        }
      }
      break;
    case SERVERTEAMPARAM_TEAMDATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(TeamDataUpdateTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
          pUser->getTeam().updateTeamData(rev);
      }
      break;
    case SERVERTEAMPARAM_TEAMMEMBER_UPDATE:
      {
        PARSE_CMD_PROTOBUF(TeamMemberUpdateTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;

        GTeam stOldTeam = pUser->getTeam();
        pUser->getTeam().updateMember(rev);
        pUser->onTeamChange(stOldTeam, pUser->getTeam());

        for (int i = 0; i < rev.updates_size(); ++i)
        {
          const TeamMemberInfo& rMember = rev.updates(i);
          if (rMember.catid() == 0)
            pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_TEAM, rMember.charid(), 1);     //成为队友
        }

        TSetQWORD setUpdateIDs;
        for (int i = 0; i < rev.updates_size(); ++i)
          setUpdateIDs.insert(rev.updates(i).charid());

        // update data to team
        for (auto &s : setUpdateIDs)
        {
          SceneUser* pMember = SceneUserManager::getMe().getUserByID(s);
          if (pMember == nullptr)
            continue;

          DScene* pDScene = dynamic_cast<DScene*>(pMember->getScene());
          if (pDScene != nullptr)
            pMember->setMark(EMEMBERDATA_MAPID);

          pMember->setMark(EMEMBERDATA_HP);
          pMember->setMark(EMEMBERDATA_MAXHP);
          pMember->setMark(EMEMBERDATA_SP);
          pMember->setMark(EMEMBERDATA_MAXSP);
          pMember->setMark(EMEMBERDATA_GUILDRAIDINDEX);
          pMember->updateDataToTeam();

          const TListPetData& list = pMember->getWeaponPet().getList();
          for (auto &l : list)
          {
            if (pMember->getTeamMember(l.getTempID(pMember->id)) != nullptr)
            {
              pMember->getWeaponPet().setMark(l.dwID, EMEMBERDATA_HP);
              pMember->getWeaponPet().setMark(l.dwID, EMEMBERDATA_MAXHP);
              pMember->getWeaponPet().setMark(l.dwID, EMEMBERDATA_RELIVETIME);
              pMember->getWeaponPet().updateDataToTeam();
            }
          }
        }

        // update team to me
        const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
        for (auto &m : mapMember)
        {
          if (setUpdateIDs.find(m.first) != setUpdateIDs.end())
            continue;

          SceneUser* pTarget = SceneUserManager::getMe().getUserByID(m.first);
          if (pTarget == nullptr)
            continue;

          DScene* pDScene = dynamic_cast<DScene*>(pTarget->getScene());
          if (pDScene != nullptr)
            pTarget->setMark(EMEMBERDATA_MAPID);

          for (auto &s : setUpdateIDs)
          {
            pTarget->setMark(EMEMBERDATA_HP);
            pTarget->setMark(EMEMBERDATA_MAXHP);
            pTarget->setMark(EMEMBERDATA_SP);
            pTarget->setMark(EMEMBERDATA_MAXSP);
            pTarget->setMark(EMEMBERDATA_GUILDRAIDINDEX);
            pTarget->updateDataToUser(s);

            const TListPetData& list = pTarget->getWeaponPet().getList();
            for (auto &l : list)
            {
              if (pTarget->getTeamMember(l.getTempID(pTarget->id)) != nullptr)
              {
                pTarget->getWeaponPet().setMark(l.dwID, EMEMBERDATA_HP);
                pTarget->getWeaponPet().setMark(l.dwID, EMEMBERDATA_MAXHP);
                pTarget->getWeaponPet().setMark(l.dwID, EMEMBERDATA_RELIVETIME);
                pTarget->getWeaponPet().updateDataToUser(s);
              }
            }
          }
        }

        // update cat
        for (int i = 0; i < rev.updates_size(); ++i)
        {
          const TeamMemberInfo& rInfo = rev.updates(i);
          if (rInfo.catid() == 0)
            continue;
          const SWeaponPetData* pData = pUser->getWeaponPet().getData(rInfo.catid());
          if (pData == nullptr)
          {
            XERR << "[玩家-组队消息] catid :" << rInfo.catid() << "加入" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "队伍数据同步失败,未找到对应猫" << XEND;
            continue;
          }
          if (rInfo.charid() != pData->getTempID(pUser->id))
            continue;
          if (pUser->getWeaponPet().enable(rInfo.catid()) == false)
          {
            XERR << "[玩家-组队消息] catid :" << rInfo.catid() << "加入" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "队伍数据同步失败,出场失败" << XEND;
            continue;
          }
          pUser->getWeaponPet().setMark(rInfo.catid(), EMEMBERDATA_HP);
          pUser->getWeaponPet().setMark(rInfo.catid(), EMEMBERDATA_MAXHP);
          pUser->getWeaponPet().setMark(rInfo.catid(), EMEMBERDATA_RELIVETIME);
          pUser->getWeaponPet().updateDataToTeam();
        }
      }
      break;
    case SERVERTEAMPARAM_MEMBERDATA_UPDATE:
      {
        PARSE_CMD_PROTOBUF(MemberDataUpdateTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
          pUser->getTeam().updateMemberData(rev);
      }
      break;
    case SERVERTEAMPARAM_CAT_EXITTEAM:
      {
        PARSE_CMD_PROTOBUF(CatExitTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          if (rev.enterfail()) // 队伍变化重新入队失败
            pUser->getWeaponPet().onEnterTeamFail(rev.catid());
          else
            pUser->getWeaponPet().disable(rev.catid());
        }
      }
      break;
    case SERVERTEAMPARAM_CAT_CALL:
      {
        PARSE_CMD_PROTOBUF(CatCallTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          const TListPetData& list = pUser->getWeaponPet().getList();
          CatEnterTeamCmd cmd;
          cmd.set_charid(pUser->id);
          for (auto &l : list)
          {
            if (l.bActive)
            {
              MemberCat* pCat = cmd.add_cats();
              l.toData(pCat, pUser);
            }
          }
          if (cmd.cats_size() > 0)
          {
            PROTOBUF(cmd, send, len);
            thisServer->sendCmdToSession(send, len);
          }
        }
      }
      break;
    case SERVERTEAMPARAM_BE_LEADER:
      {
        PARSE_CMD_PROTOBUF(BeLeaderTeamCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
          pUser->updateTeamTower();
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case GUILDSPARAM_GUILD_SYNC_USERINFO:
      {
        PARSE_CMD_PROTOBUF(GuildUserInfoSyncGuildCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
        {
          XERR << "[玩家-公会]" << message.charid() << "同步个人数据" << message.ShortDebugString() << "失败,未找到玩家" << XEND;
          return true;
        }
        DWORD dwOldCon = pUser->getGuild().contribute();
        pUser->getGuild().updateUserInfo(message);

        if (dwOldCon != pUser->getGuild().contribute())
          pUser->setDataMark(EUSERDATATYPE_CONTRIBUTE);

        UserAttribute* pAttribute = dynamic_cast<UserAttribute*>(pUser->getAttribute());
        if (pAttribute != nullptr)
        {
          pAttribute->setPrayLevelup(message.levelup());
          pAttribute->setCollectMark(ECOLLECTTYPE_GUILD);
        }
        if (message.levelup())
          pUser->getAchieve().onPrayLvUp();

        pUser->getServant().onFinishEvent(ETRIGGER_GUILD_PRAY);
        pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_PRAY);
        pUser->refreshDataAtonce();
        /*// 更新贡献
        switch (message.optcontype())
        {
        case GUILDOPTCONTYPE_LOGIN: // 登录同步, 第一次同步以guild为准, 其他以少的为准(正常情况下两个相等)
          if (pUser->getUserSceneData().isConInit() == false)
          {
            pUser->getUserSceneData().setConInit();
            pUser->getUserSceneData().setCon(message.info().contribute());
            XLOG << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                 << "登录同步,贡献:" << pUser->getUserSceneData().getCon() << XEND;
          }
          else
          {
            if (pUser->getUserSceneData().getCon() != message.info().contribute())
            {
              XERR << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                   << "type:" << message.optcontype() << "公会:" << message.info().contribute() << "玩家:" << pUser->getUserSceneData().getCon() << XEND;
              if (pUser->getUserSceneData().getCon() < message.info().contribute())
              {
                GuildSubContributeGuildSCmd cmd;
                pUser->toData(cmd.mutable_user());
                cmd.set_con(message.info().contribute() - pUser->getUserSceneData().getCon());
                PROTOBUF(cmd, send, len);
                thisServer->sendCmdToSession(send, len);
                XLOG << "[玩家-同步贡献到guild]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                     << message.info().contribute() << pUser->getUserSceneData().getCon() << XEND;
              }
              else
              {
                XLOG << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                     << "登录贡献不一致,旧贡献:" << pUser->getUserSceneData().getCon() << "新贡献:" << message.info().contribute() << XEND;
                pUser->getUserSceneData().setCon(message.info().contribute());
              }
            }
          }
          break;
        case GUILDOPTCONTYPE_ADD: // 增加同步, 以guild为准
          if (pUser->getUserSceneData().getCon() >= message.info().contribute())
            XERR << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                 << "type:" << message.optcontype() << "公会:" << message.info().contribute() << "玩家:" << pUser->getUserSceneData().getCon() << XEND;
          XLOG << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
               << "贡献增加,旧贡献:" << pUser->getUserSceneData().getCon() << "新贡献:" << message.info().contribute() << XEND;
          pUser->getUserSceneData().setCon(message.info().contribute());
          break;
        case GUILDOPTCONTYPE_SUB: // 扣除同步, 一般应该与同步数据一致, 否则以少的为准(少的时候一般是公会祈祷扣除)
          pUser->UnlockSubContribute();
          if (pUser->getUserSceneData().getCon() >= message.info().contribute())
          {
            XLOG << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                 << "贡献扣除,旧贡献:" << pUser->getUserSceneData().getCon() << "新贡献:" << message.info().contribute() << XEND;
            pUser->getUserSceneData().setCon(message.info().contribute());
          }
          else
            XERR << "[玩家-同步贡献]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
                 << "type:" << message.optcontype() << "公会:" << message.info().contribute() << "玩家:" << pUser->getUserSceneData().getCon() << XEND;
          break;
        default:
          break;
        }*/
      }
      return true;
    case GUILDSPARAM_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(GuildInfoSyncGuildSCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateGuild(message);
        pUser->sendGuildInfoToNine();
        pUser->getQuest().resetGuildQuest();
        pUser->getQuest().resetArtifactQuest();

        if (pUser->getGuild().id() != 0)
          pUser->getEvent().onEnterGuild(pUser->getGuild().id(), pUser->getGuild().create());

        if (message.info().create() == true)
        {
          pUser->getServant().onAppearEvent(ETRIGGER_JOIN_GUILD);
          pUser->getServant().onFinishEvent(ETRIGGER_JOIN_GUILD);
          pUser->getServant().onGrowthAppearEvent(ETRIGGER_JOIN_GUILD);
          pUser->getServant().onGrowthFinishEvent(ETRIGGER_JOIN_GUILD);
        }

        UserAttribute* pAttribute = dynamic_cast<UserAttribute*>(pUser->getAttribute());
        if (pAttribute != nullptr)
        {
          pAttribute->setPrayLevelup(false);
          pAttribute->setCollectMark(ECOLLECTTYPE_GUILD);
        }
        pUser->refreshDataAtonce();

        if (pUser->getScene() && pUser->getScene()->getSceneType() == SCENE_TYPE_GUILD_FIRE)
        {
          pUser->getScene()->addKickList(pUser);
        }
        pUser->updateArtifact();

        do
        {
          if (message.info().id() != 0)
            break;

          pUser->getUserSceneData().clearGuildRaidData();
          pUser->getVar().setVarValue(EVARTYPE_GUILD_RAID_BAN, 1);

          DScene* pDScene = dynamic_cast<DScene*>(pUser->getScene());
          if (pDScene == nullptr)
            break;

          if (pDScene->getRaidRestrict() == ERAIDRESTRICT_GUILD || pDScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_TEAM)
          {
            pDScene->addKickList(pUser);
          }
          else if (pDScene->getRaidRestrict() == ERAIDRESTRICT_GUILD_RANDOM_RAID)
          {
            pDScene->addKickList(pUser);
          }
          else if (pDScene->isSuperGvg())
          {
            pDScene->addKickList(pUser);
          }
        } while (0);

        if (message.info().id() == 0)
        {
          // 退会清除公会挑战红点
          //pUser->getTip().removeRedTip(EREDSYS_GUILD_CHALLENGE_ADD);
          pUser->getTip().removeRedTip(EREDSYS_GUILD_CHALLENGE_REWARD);
        }
      }
      break;
    case GUILDSPARAM_UPDATE_GUILDDATA:
      {
        PARSE_CMD_PROTOBUF(GuildDataUpdateGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateGuildData(rev);
        for (int i = 0; i < rev.updates_size(); ++i)
        {
          if (rev.updates(i).type() == EGUILDDATA_PORTRAIT)
          {
            pUser->sendGuildInfoToNine();
            break;
          }
          else if(rev.updates(i).type() == EGUILDDATA_LEVEL)
            pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_LEVEL);
        }
      }
      break;
    case GUILDSPARAM_UPDATE_CITYDATA:
      {
        PARSE_CMD_PROTOBUF(CityDataUpdateGuildSCmd, rev);
        GuildCityManager::getMe().updateCityInfoFromGuild(rev);
      }
      break;
    case GUILDSPARAM_UPDATE_MEMBER:
      {
        PARSE_CMD_PROTOBUF(GuildMemberUpdateGuildSCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateMember(message);
      }
      break;
    case GUILDSPARAM_UPDATE_MEMBERDATA:
      {
        PARSE_CMD_PROTOBUF(GuildMemberDataUpdateGuildSCmd, message);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateMemberData(message);
        for (int i = 0; i < message.updates_size(); ++i)
        {
          if (message.updates(i).type() == EGUILDMEMBERDATA_JOB)
          {
            pUser->sendGuildInfoToNine();
            break;
          }
          else if (message.updates(i).type() == EGUILDMEMBERDATA_BUILDINGEFFECT)
          {
            if (message.destid() == pUser->id)
              pUser->sendOpenBuildingGateMsg();
          }
        }
      }
      break;
    case GUILDSPARAM_UPDATE_QUEST:
      {
        PARSE_CMD_PROTOBUF(GuildQuestUpdateGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateQuest(rev);
        pUser->getQuest().resetGuildQuest();
        pUser->getQuest().sendGuildQuestList();
      }
      break;
    case GUILDSPARAM_JOB_UPDATE:
      {
        PARSE_CMD_PROTOBUF(JobUpdateGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          break;
        pUser->getGuild().updateJob(rev);
        pUser->getQuest().resetArtifactQuest();
        const GuildSMember* pMember = pUser->getGuild().getMember(rev.charid());
        if (pMember != nullptr && pMember->job() == rev.job().job())
          pUser->sendGuildInfoToNine();
      }
      break;
    case GUILDSPARAM_ENTER_TERRITORY:
      {
        PARSE_CMD_PROTOBUF(EnterGuildTerritoryGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          return true;

        const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
        const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(rCFG.dwTerritory);
        if (pBase == nullptr || pBase->eRestrict != ERAIDRESTRICT_GUILD)
            return false;

        if (pUser->canGoGuild())
          pUser->m_oSkillProcessor.useTransportSkill(pUser, ETRANSPORTTYPE_GUILD);
        //pUser->getUserZone().gomap(pUser->getGuildInfo().zoneid(), rCFG.dwTerritory, GoMapType::Null);
      }
      return true;
    case GUILDSPARAM_REFRESH_TERRITORY:
      {
        PARSE_CMD_PROTOBUF(RefreshGuildTerritoryGuildSCmd, message);
        GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(message.sceneid()));
        if (pScene != nullptr)
        {
          pScene->setGuildInfo(message.info());
          pScene->refreshNpc();
          pScene->syncGuildRaidGate();
        }
      }
      return true;
    case GUILDSPARAM_QUERY_PHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryPhotoListGuildSCmd, message);
        GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(message.sceneid()));
        if (pScene != nullptr)
          pScene->queryPhotoFromGuild(message);
      }
      return true;
    case GUILDSPARAM_QUERY_USERPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryUserPhotoListGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.user().charid());
        if (pUser != nullptr)
        {
          pUser->getPhoto().updateUploadPhoto(rev);
          pUser->getPhoto().sendUserPhotoList();
        }
      }
      break;
    case GUILDSPARAM_PHOTO_UPDATE:
      {
        PARSE_CMD_PROTOBUF(PhotoUpdateGuildSCmd, message);
        GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(message.sceneid()));
        if (pScene != nullptr)
          pScene->updatePhoto(message);
      }
      return true;
    case GUILDSPARAM_RENAME_NTF:
      {
        PARSE_CMD_PROTOBUF(RenameNTFGuildCmd, rev);
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.user().charid());
          if(!pUser)
          {
            rev.set_result(false);
            XERR << "[公会-改名] 失败，玩家不在场景"<< rev.ShortDebugString() << XEND;
            break;
          }

          const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
          MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
          if (!pMainPack || !pMainPack->checkItemCount(rCFG.dwRenameItemId, 1))
          {
            rev.set_result(false);
            XERR << "[公会-改名] 失败，没有改名道具（客户端检测，有作弊嫌疑）" << rev.ShortDebugString() << XEND;
            break;
          }
          pMainPack->reduceItem(rCFG.dwRenameItemId, ESOURCE_GUILD_RENAME, 1);
        }while(0);

        PROTOBUF(rev, send1, len1);
        sendCmdToSession(send1, len1);
      }
      break;
    case GUILDSPARAM_GUILD_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(GuildCityActionGuildSCmd, message);
        if (message.action() == EGUILDCITYACTION_TO_SCENE_UPDATE)
          GuildCityManager::getMe().updateCityInfoFromGuild(message);
        else
          XERR << "[公会城池-行为]" << message.ShortDebugString() << "失败,无法处理该action" << XEND;
      }
      return true;
    case GUILDSPARAM_QUERY_SHOWPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryShowPhotoGuildSCmd, rev);
        if (rev.action() == EPHOTOACTION_LOAD_FROM_SCENE)
        {
          if (rev.members_size() <= 0)
          {
            XERR << "[公会消息-加载照片]" << rev.ShortDebugString() << "未处理,没有成员" << XEND;
            break;
          }
          SceneUser* pUser = SceneUserManager::getMe().getUserByAccID(rev.members(0));
          if (pUser == nullptr)
          {
            XERR << "[公会消息-加载照片]" << rev.ShortDebugString() << "未处理,成员" << rev.members(0) << "不在线" << XEND;
            break;
          }
          pUser->getScenery().collectPhoto(rev);
        }
        else
        {
          XERR << "[公会消息-加载照片]" << rev.ShortDebugString() << "未处理" << XEND;
        }
      }
      break;
    case GUILDSPARAM_GUILDPRAY:
      {
        PARSE_CMD_PROTOBUF(GuildPrayGuildSCmd, message);

        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.user().charid());
          if (pUser == nullptr)
          {
            XERR << "[公会祈祷]" << message.user().ShortDebugString() << "进行" << message.prayid() << message.praylv() << "祈祷失败,玩家不在线" << XEND;
            message.set_msgid(10);
            break;
          }

          BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
          if (pMainPack == nullptr)
          {
            XERR << "[公会祈祷]" << message.user().ShortDebugString() << "进行" << message.prayid() << message.praylv() << "祈祷失败,未找到" << EPACKTYPE_MAIN << XEND;
            message.set_msgid(10);
            break;
          }

          const SGuildPrayCFG* pCFG = GuildConfig::getMe().getGuildPrayCFG(message.prayid());
          if (pCFG == nullptr)
          {
            XERR << "[公会祈祷]" << message.user().ShortDebugString() << "进行" << message.prayid() << message.praylv() << "祈祷失败,未在 Table_Guild_Faith.txt 表中找到" << XEND;
            message.set_msgid(10);
            break;
          }

          ENpcFunction eFunc = pCFG->eType == EPRAYTYPE_GODDESS ? ENPCFUNCTION_GUILD_PRAY : ENPCFUNCTION_GUILD_GVGPRAY;
          SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(message.npcid());
          if (pNpc == nullptr || pNpc->getCFG() == nullptr || pNpc->getCFG()->stNpcFunc.hasFunction(eFunc) == false)
          {
            XERR << "[公会祈祷]" << message.user().ShortDebugString() << "进行" << message.prayid() << message.praylv() << "祈祷失败,未在正确的npc旁边" << XEND;
            message.set_msgid(10);
            break;
          }

          TVecItemInfo vecCosts;
          DWORD dwZeny = message.needmon();
          DWORD dwRealCount = 0;

          DWORD costPrayCardNum = 0;
          if (pCFG->eType != EPRAYTYPE_GODDESS)
          {
            const SGuildPrayItemCFG* pItemCFG = pCFG->getItem(message.praylv());
            if (pItemCFG == nullptr)
            {
              message.set_msgid(10);
              break;
            }

            if (pItemCFG->vecCosts.empty() == false)
              vecCosts.insert(vecCosts.end(), pItemCFG->vecCosts.begin(), pItemCFG->vecCosts.end());

            for (auto &v : vecCosts)
            {
              if (v.id() == ITEM_PRAY_ATKCARD || v.id() == ITEM_PRAY_DEFCARD || v.id() == ITEM_PRAY_ELEMCARD)
                costPrayCardNum += v.count();
            }
          }

          const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
          if (message.needmon() >= rCFG.dwPrayRob)
          {
            DWORD dwCount = message.needmon() / rCFG.dwPrayRob;
            DWORD dwItem = pMainPack->getItemCount(rCFG.dwPrayItem);

            dwRealCount = dwItem >= dwCount ? dwCount : dwItem;
            dwZeny = message.needmon() - dwRealCount * rCFG.dwPrayRob;
            if (dwRealCount != 0)
            {
              ItemInfo oItem;
              oItem.set_id(rCFG.dwPrayItem);
              oItem.set_count(dwRealCount);
              combinItemInfo(vecCosts, oItem);
            }
          }
          if (vecCosts.empty() == false && pMainPack->checkItemCount(vecCosts) == false)
          {
            message.set_msgid(ESYSTEMMSG_ID_ZENY_NO_ENOUGH);
            break;
          }
          if (pUser->checkMoney(EMONEYTYPE_SILVER, dwZeny) == false)
          {
            message.set_msgid(ESYSTEMMSG_ID_ZENY_NO_ENOUGH);
            break;
          }

          pUser->subMoney(EMONEYTYPE_SILVER, dwZeny, ESOURCE_GUILDPRAY);
          if (vecCosts.empty() == false)
            pMainPack->reduceItem(vecCosts, ESOURCE_GUILDPRAY);

          if (costPrayCardNum)
            pUser->getAchieve().onCostPrayItem(costPrayCardNum);

          message.set_needmon(dwZeny);
          message.set_prayitem(dwRealCount);

          UserActionNtf cmd;
          cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
          cmd.set_charid(pNpc->id);
          cmd.set_value(rCFG.dwPrayAction);

          xLuaData data;
          data.setData("effect", rCFG.strPrayEffect);
          data.setData("npcid", pNpc->getNpcID());
          GMCommandRuler::effect(pUser, data);

          PROTOBUF(cmd, send, len);
          pNpc->sendCmdToNine(send, len);

          if (message.msgid() == 0)
            pUser->stopSendInactiveLog();

          if (pCFG->eType > EPRAYTYPE_GODDESS && pCFG->eType <= EPRAYTYPE_GVG_ELE)
            pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_SPECIAL_PRAY);
        } while(0);

        PROTOBUF(message, send, len);
        sendCmdToSession(send, len);
      }
      return true;
    case GUILDSPARAM_SUBMIT_MATERIAL:
      {
        PARSE_CMD_PROTOBUF(SubmitMaterialGuildSCmd, rev);
        bool success = true;
        do
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
          if (pUser == nullptr)
          {
            success = false;
            break;
          }

          TVecItemInfo items;
          map<DWORD, DWORD> rewards, submitcounts;
          DWORD submitcount = rev.submitcount();
          for (int i = 0; i < rev.materials_size(); ++i)
          {
            const SGuildBuildingMaterialCFG* cfg = GuildConfig::getMe().getGuildBuildingMaterial(rev.materials(i).id());
            if (cfg == nullptr)
            {
              success = false;
              XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.materials(i).id() << "配置找不到" << XEND;
              break;
            }

            DWORD itemid = 0, itemcount = 0;
            if (cfg->getRandItem(itemid, itemcount, rev.building(), pUser->id, rev.materials(i).id()) == false)
            {
              success = false;
              XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.materials(i).id() << "获取随机道具失败" << XEND;
              break;
            }

            submitcount += 1;
            itemcount = LuaManager::getMe().call<DWORD>("calcGuildBuildingMaterialItemCount", itemcount, submitcount);
            if (itemcount <= 0)
            {
              success = false;
              XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.materials(i).id() << "提交次数:" << submitcount << "扣除道具数量计算错误" << XEND;
              break;
            }

            ItemInfo item;
            item.set_id(itemid);
            item.set_count(itemcount);
            combinItemInfo(items, item);

            if (rewards.find(cfg->dwRewardID) == rewards.end())
              rewards[cfg->dwRewardID] = 0;
            rewards[cfg->dwRewardID] += 1;
          }
          if (success == false)
            break;

          if (pUser->getPackage().reduceItem(items, ESOURCE_GUILD_SUBMIT_MATERIAL, ECHECKMETHOD_NONORMALEQUIP, EPACKFUNC_GUILDBUILDING) == false)
          {
            success = false;
            XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.ShortDebugString() << "道具不足" << XEND;
            break;
          }

          /*BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
          if (pMainPack == nullptr || pMainPack->checkItemCount(items) == false)
          {
            success = false;
            XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.ShortDebugString() << "道具不足" << XEND;
            break;
          }

          pMainPack->reduceItem(items, ESOURCE_GUILD_SUBMIT_MATERIAL);*/
          XLOG << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "提交材料:";
          for (auto& v : items)
          {
            XLOG << v.id() << v.count();
            const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(v.id());
            if (cfg)
              MsgManager::sendMsg(pUser->id, 3716, MsgParams{cfg->strNameZh, v.count()});
          }
          XLOG << XEND;

          TVecItemInfo allrewards;
          DWORD rewardinc = ActivityEventManager::getMe().getGuildBuildingSubmitInc(rev.building(), rev.curlevel(), EGBUILDINGSUBMITINC_REWARD);
          for (auto& v : rewards)
            for (DWORD i = 0; i < v.second; ++i)
            {
              TVecItemInfo items;
              if (RewardManager::roll(v.first, pUser, items, ESOURCE_GUILD_SUBMIT_MATERIAL) == false)
              {
                XERR << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "材料:" << rev.ShortDebugString() << "奖励:" << v.first << "发奖失败" << XEND;
                continue;
              }

              if(rewardinc > 0)
              {
                for (auto &item : items)
                  item.set_count(item.count() * (1.0 + rewardinc / 100.0));
              }
              combinItemInfo(allrewards, items);
            }

          pUser->getPackage().addItem(allrewards, EPACKMETHOD_AVAILABLE);
          pUser->getServant().onFinishEvent(ETRIGGER_GUILD_BUILD_DONATE);
          XLOG << "[公会消息-提交材料]" << pUser->accid << pUser->id << pUser->name << "发放奖励:";
          for (auto& v : rewards)
            XLOG << v.first << v.second;
          XLOG << "奖励提升:" << rewardinc << XEND;
        } while (0);

        rev.set_success(success);
        rev.set_submitinc(ActivityEventManager::getMe().getGuildBuildingSubmitInc(rev.building(), rev.curlevel(), EGBUILDINGSUBMITINC_SUBMIT));
        PROTOBUF(rev, send1, len1);
        thisServer->sendCmdToSession(send1, len1);
      }
      break;
    case GUILDSPARAM_BUILDING_UPDATE:
      {
        PARSE_CMD_PROTOBUF(BuildingUpdateGuildSCmd, message);
        if (message.sceneid())
        {
          GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(message.sceneid()));
          if (pScene != nullptr)
          {
            pScene->getGuild().updateBuilding(message);
            pScene->refreshNpc();
            set<EGuildBuilding> types;
            for (int i = 0; i < message.updates_size(); ++i)
              types.insert(message.updates(i).type());
            pScene->syncBuildingDataToUser(nullptr, types);
            pScene->notifyServantEvent();
          }
        }
        if (message.charid())
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(message.charid());
          if (pUser)
          {
            pUser->getGuild().updateBuilding(message);
          }
        }
      }
      break;
    case GUILDSPARAM_QUERY_GUILD_INFO:
      {
        PARSE_CMD_PROTOBUF(QueryGuildInfoGuildSCmd, message);
        GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(message.sceneid()));
        if (pScene != nullptr)
          pScene->updateGuildInfo(message);
      }
      return true;
    case GUILDSPARAM_SEND_WELFARE:
      {
        PARSE_CMD_PROTOBUF(SendWelfareGuildSCmd, rev);

        // collect reward
        vector<pair<DWORD, ESource>> fails, succs;
        vector<TVecItemInfo> vvecItems;
        vector<MsgParams> vecParams;

        for (int i = 0; i < rev.items_size(); ++i)
        {
          const GuildWelfareItem& rItem = rev.items(i);

          TVecItemInfo vecSingle;

          if (rItem.source() == ESOURCE_GUILD_TREASURE)
          {
            const SRewardCFG* pCFG = RewardConfig::getMe().getRewardCFG(rItem.rewardid());
            if (pCFG != nullptr)
            {
              if (rItem.index() < pCFG->vecItems.size())
              {
                const SRewardItem& rRewardItem = pCFG->vecItems[rItem.index()];
                for (auto &single : rRewardItem.vecItems)
                  combinItemInfo(vecSingle, single.oItem);
                for (auto &v : vecSingle)
                {
                  DWORD dwReal = randBetween(v.count() * 0.5f, v.count() * 1.5f);
                  XLOG << "[公会-福利] 公会宝箱福利" << v.ShortDebugString() << "浮动上下50%,最终结果为";
                  v.set_count(dwReal);
                  XLOG << v.ShortDebugString() << XEND;
                }
              }
              else
              {
                XERR << "[公会-福利] 公会宝箱福利 index" << rItem.index() << "大于reward奖励上限" << pCFG->vecItems.size() << XEND;
              }
            }
          }
          else
          {
            if (RewardManager::roll(rItem.rewardid(), nullptr, vecSingle, rItem.source()) == false)
              fails.push_back(pair<DWORD, ESource>(rItem.rewardid(), rItem.source()));
            else
              succs.push_back(pair<DWORD, ESource>(rItem.rewardid(), rItem.source()));
          }

          vvecItems.push_back(vecSingle);

          const SGuildTreasureCFG* pTreasureCFG = GuildConfig::getMe().getTreasureCFG(rItem.sourceid());
          if (pTreasureCFG != nullptr)
          {
            TreasureResultNtfGuildSCmd cmd;
            cmd.set_charid(rev.charid());

            TreasureResult* pResult = cmd.mutable_result();
            pResult->set_eventguid(rItem.eventguid());

            for (auto &item : vecSingle)
            {
              const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(item.id());
              if (pCFG != nullptr)
              {
                MsgParams oParams;
                oParams.addString(rItem.ownername());
                oParams.addString(pTreasureCFG->strName);
                oParams.addString(pCFG->strNameZh);
                oParams.addNumber(item.count());
                vecParams.push_back(oParams);

                TreasureItem* pItem = pResult->add_items();
                pItem->set_charid(rev.charid());
                pItem->add_datas()->mutable_base()->CopyFrom(item);
              }
            }

            if (pResult->items_size() > 0)
            {
              PROTOBUF(cmd, send, len);
              thisServer->sendCmdToSession(send, len);
              XLOG << "[公会-福利] 宝箱福利通知" << cmd.ShortDebugString() << "成功通知公会服" << XEND;
            }
          }
        }
        if (succs.empty() == false)
        {
          XLOG << "[公会-福利] 奖励:" << rev.charid();
          for (auto& v : succs)
            XLOG << v.first << v.second;
          XLOG << "发放成功" << XEND;
        }
        if (fails.empty() == false)
        {
          XERR << "[公会-福利] 奖励:" << rev.charid();
          for (auto& v : fails)
            XERR << v.first << v.second;
          XERR << "发放失败" << XEND;
        }

        // send reward online-addpack offline-mail
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser)
        {
          bool bSuccess = true;
          for (auto &v : vvecItems)
          {
            if (pUser->getPackage().addItem(v, EPACKMETHOD_AVAILABLE, false, false, false) == false)
            {
              XERR << "[公会-福利]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "添加包裹失败,奖励包含" << v << XEND;
              bSuccess = false;
              continue;
            }
          }
          if (bSuccess)
          {
            for (auto &v : vecParams)
              MsgManager::sendMsg(pUser->id, 4025, v);
          }
        }
        else
        {
          TVecItemInfo vecTotalItem;
          for (auto &v : vvecItems)
            combinItemInfo(vecTotalItem, v);
          if (vecTotalItem.empty())
          {
            XERR << "[公会-福利]" << rev.charid() << "奖励为空:";
            for (int i = 0; i < rev.items_size(); ++i)
              XERR << rev.items(i).rewardid() << rev.items(i).source();
            XERR << XEND;
            return true;
          }
          XLOG << "[公会-福利]" << rev.charid() << "保存到邮件,奖励:" << vecTotalItem;
          if (MailManager::getMe().sendMail(rev.charid(), 0, "", SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_MAIL), SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_OFFLINE_COMPENSATE), EMAILTYPE_NORMAL, 0, vecTotalItem, TVecItemData{}, EMAILATTACHTYPE_ITEM))
            XLOG << "成功" << XEND;
          else
            XLOG << "失败" << XEND;
        }
      }
      return true;
    case GUILDSPARAM_ARTIFACT_UPDATE:
      {
        PARSE_CMD_PROTOBUF(ArtifactUpdateGuildSCmd, rev);
        if (rev.charid())
        {
          SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
          if (pUser)
          {
            pUser->getGuild().updateArtifact(rev);
            pUser->updateArtifact(rev);
            pUser->getServant().onGrowthFinishEvent(ETRIGGER_GUILD_ARTIFACT);
          }
        }
        if (rev.sceneid())
        {
          GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(rev.sceneid()));
          if (pScene)
          {
            pScene->getGuild().updateArtifact(rev);
            pScene->refreshArtifactNpc();
          }
        }
      }
      return true;
    case GUILDSPARAM_QUEST_ARTIFACT:
      {
        PARSE_CMD_PROTOBUF(GuildArtifactQuestGuildSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
          return true;
        pUser->getGuild().updateGQuest(rev);
        pUser->getQuest().resetArtifactQuest();
      }
      return true;
    case GUILDSPARAM_TREASURE_QUERY:
      {
        PARSE_CMD_PROTOBUF(QueryTreasureGuildSCmd, rev);
        GuildScene* pScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(rev.sceneid()));
        if (pScene != nullptr)
          pScene->queryTreasureFromGuild(rev);
      }
      return true;
    default:
      return false;
  }

  return true;
}

//通过session转发到auctionserver
bool SceneServer::forwardCmdToAuction(QWORD charid, const std::string& name, const void* data, unsigned short len)
{
  ForwardSCmd2Auction cmd;
  cmd.set_charid(charid);
  cmd.set_name(name);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_data(data, len);
  cmd.set_len(len);
  PROTOBUF(cmd, send, len1);
  sendCmdToSession(send, len1);
  return true;
}

bool SceneServer::doWeddingCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  xCommand *cmd = (xCommand *)buf;

  switch (cmd->param)
  {
  case Cmd::WEDDINGSPARAM_START_WEDDING:
  {
    PARSE_CMD_PROTOBUF(Cmd::StartWeddingSCmd, rev);
    SceneWeddingMgr::getMe().startWedding(rev);
    return true;
  }
  break;
  case Cmd::WEDDINGSPARAM_STOP_WEDDING:       
  {
    PARSE_CMD_PROTOBUF(Cmd::StopWeddingSCmd, rev);
    SceneWeddingMgr::getMe().stopWedding(rev);
    return true;
  }
  break;
  case WEDDINGSPARAM_SYNC_WEDDINGINFO:
  {
    PARSE_CMD_PROTOBUF(Cmd::SyncWeddingInfoSCmd, rev);
    SceneUser*pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      pUser->getUserWedding().updateWeddingInfo(rev);
    }
  }
  break;
  case WEDDINGSPARAM_UPDATE_MANUAL:
  {
    PARSE_CMD_PROTOBUF(UpdateWeddingManualSCmd, rev);
    SceneWeddingMgr::getMe().updateManual(rev.weddingid(), rev.manual());
  }
  break;
  case WEDDINGSPARAM_MARRY_SUCCESS:
  {
    PARSE_CMD_PROTOBUF(MarrySuccessSCmd, rev);
    SceneWeddingMgr::getMe().onMarrySuccess(rev.weddingid());
  }
  break;
  case WEDDINGSPARAM_BUY_SERVICE:
  {
    PARSE_CMD_PROTOBUF(Cmd::BuyServiceWeddingSCmd, rev);
    bool ret = false;
    SceneUser*pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser)
    {
      TVecItemInfo items;
      for (int i = 0; i < rev.items_size(); ++i)
        combinItemInfo(items, rev.items(i));
      bool bOpen = ActivityManager::getMe().isOpen(static_cast<DWORD>(GACTIVITY_WEDDING_SERVICE));
      if (bOpen)
      {
        do
        {
          const SGlobalActCFG* pCFG = ActivityConfig::getMe().getGlobalActCFG(GACTIVITY_WEDDING_SERVICE);
          if (pCFG == nullptr)
          {
            XERR << "[结婚-购买服务扣道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动打折,折扣失败,id :" << GACTIVITY_WEDDING_SERVICE << "未在 Table_GlobalActivity.txt 表中找到" << XEND;
            break;
          }
          float fDisCount = pCFG->getDiscount(rev.serviceid());
          for (auto &v : items)
          {
            DWORD dwPrice = v.count();
            v.set_count(round(v.count() * fDisCount));
            XLOG << "[结婚-购买服务扣道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "活动打折,折扣" << fDisCount << "道具" << v.id() << "价格" << dwPrice << "->" << v.count() << XEND;
          }
        }while (0);
      }
      BasePackage* pack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
      if (pack && pack->checkItemCount(items))
      {
        pack->reduceItem(items, rev.source());
        ret = true;
        XLOG << "[结婚-购买服务扣道具]" << pUser->accid << pUser->id << pUser->name << "道具:";
        for (auto& item : items)
          XLOG << item.id() << item.count();
        XLOG << "source:" << rev.source() << "扣除成功" << XEND;
      }
      else
      {
        XERR << "[结婚-购买服务扣道具]" << pUser->accid << pUser->id << pUser->name << "道具不足,扣除失败" << XEND;
      }
    }
    else
    {
      XERR << "[结婚-购买服务扣道具]" << rev.charid() << "玩家未找到,扣除失败" << XEND;
    }
    rev.set_success(ret);
    PROTOBUF(rev, send, len2);
    thisServer->sendSCmdToWeddingServer(rev.charid(), "", send, len2);
    return true;
  }
  break;
  case WEDDINGSPARAM_MISSYOU_INVITE:
  {
    PARSE_CMD_PROTOBUF(MissyouInviteWedSCmd, rev);
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
    if (pUser == nullptr)
    {
      XERR << "[婚姻-好想你]" << rev.charid() << "被配偶邀请回到身边,失败了,自己不在线了" << XEND;
      break;
    }
    pUser->getUserWedding().setMissInfo(rev.info());

    MissyouInviteWedCCmd cmd;
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
    XLOG << "[婚姻-好想你]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "被配偶邀请回到身边,成功收到邀请" << XEND;
  }
  break;
  default:
    return false;
  }
  return true;
}

/*bool SceneServer::doItemSCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case ITEMSPARAM_PACKAGEUPDATE:
      {
        PARSE_CMD_PROTOBUF(PackageUpdateItemSCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[" << "\b" << EPACKTYPE_STORE << "\b-更新]" << rev.accid() << rev.charid() << "收到回馈,玩家不在线" << XEND;
          break;
        }
        StorePackage* pStorePack = dynamic_cast<StorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_STORE));
        if (pStorePack == nullptr)
        {
          XERR << "[" << "\b" << EPACKTYPE_STORE << "\b-更新]" << rev.accid() << rev.charid() << "收到回馈,未找到" << EPACKTYPE_STORE << XEND;
          break;
        }
        pStorePack->doFunc(rev.sessionid(), static_cast<EError>(rev.error()));
        pUser->getPackage().setStoreTick(0);
      }
      break;
    default:
      return false;
  }

  return true;
}*/

bool SceneServer::doBossSCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case BOSSSPARAM_DEADBOSS_SYNC:
      {
        PARSE_CMD_PROTOBUF(DeadBossOpenSyncBossSCmd, rev);
        SceneUserManager::getMe().updateGlobalBoss(rev.info());
      }
      break;
    case BOSSSPARAM_BOSS_SUMMON:
      {
        PARSE_CMD_PROTOBUF(SummonBossBossSCmd, message);
        BossMgr::getMe().onSummonBoss(message);
      }
      break;
    default:
      return false;
  }

  return true;
}


#include "GuildServer.h"
#include "SocialCmd.pb.h"
#include "MiscConfig.h"
#include "StatisticsDefine.h"
#include "MailManager.h"
#include "PlatLogManager.h"
//#include "Team.h"
//#include "TeamManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "UserCmd.h"
#include "TeamCmd.pb.h"
#include "GuildSCmd.pb.h"
#include "GCharManager.h"
#include "LuaManager.h"
#include "ConfigManager.h"
#include "MailManager.h"
#include "GuildIconManager.h"
#include "GMCommandMgr.h"

extern xLog srvLog;

bool GuildServer::doSocialCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case SOCIALPARAM_CREATEGUILD:
      {
        PARSE_CMD_PROTOBUF(CreateGuildSocialCmd, message);
        if (message.msgid() == 0)
        {
          if (GuildManager::getMe().createGuild(message.user(), message.name()) == false)
            GuildManager::getMe().removeName(message.name());
        }
        else
        {
          GuildManager::getMe().removeName(message.name());
        }
      }
      return true;
    case SOCIALPARAM_GUILDDONATE:
      {
        PARSE_CMD_PROTOBUF(GuildDonateSocialCmd, message);

        const SocialUser& rUser = message.user();
        if (message.msgid() != 0)
        {
          XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "捐赠后, configid:" << message.item().configid() << "失败, 结果为msgid:" << message.msgid() << XEND;
          return true;
        }

        const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(message.item().configid());
        if (pCFG == nullptr)
        {
          XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "捐赠后, configid:" << message.item().configid() << "配置未找到, 结果为msgid:" << message.msgid() << XEND;
          return true;
        }

        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
        if (pGuild == nullptr)
        {
          XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "捐赠后, configid:" << message.item().configid() << "未在公会内, 结果为msgid:" << message.msgid() << XEND;
          return true;
        }
        GMember* pMember = pGuild->getMember(rUser.charid());
        if (pMember == nullptr)
        {
          XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "捐赠后, configid:" << message.item().configid() << "未在公会内, 结果为msgid:" << message.msgid() << XEND;
          return true;
        }
        DonateItem* pItem = pMember->getDonateItem(message.item().configid(), message.item().time());
        if (pItem == nullptr)
        {
          XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "捐赠后, configid:" << message.item().configid() << "未在列表中找到" << XEND;
          return true;
        }
        pItem->set_count(pItem->count() + 1);
        pGuild->setMark(EGUILDDATA_MEMBER);

        if (pMember->isOnline() == true && pMember->getDonateFrame() == true)
        {
          UpdateDonateItemGuildCmd cmd;
          cmd.mutable_item()->CopyFrom(*pItem);
          cmd.mutable_del()->CopyFrom(*pItem);
          PROTOBUF(cmd, send, len);
          pMember->sendCmdToMe(send, len);
        }

        UserAddItemSocialCmd usercmd;
        usercmd.mutable_user()->CopyFrom(rUser);
        for (auto &v : pCFG->vecUserReward)
        {
          ItemInfo* pItem = usercmd.add_items();
          if (pItem == nullptr)
          {
            XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "捐赠后, configid:" << message.item().configid()
              << "创建 itemid :" << v.id() << "protobuf 失败" << XEND;
            continue;
          }
          pItem->CopyFrom(v);
          pItem->set_source(ESOURCE_DONATE);
        }
        usercmd.set_doublereward(static_cast<DWORD>(EDOUBLESOURCE_GUILD_DONATE));
        PROTOBUF(usercmd, send, len);
        thisServer->sendCmdToScene(rUser.zoneid(), rUser.charid(), send, len);
        thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_DONATEITEM, MsgParams(message.item().itemid(), message.item().itemcount()));
        XLOG << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "捐赠成功" << XEND;

        pMember->updateDonateItem(pCFG->dwNextID, pCFG->eType, pItem->time(), message.item().configid());
        if (pCFG->eType == EDONATETYPE_NORMAL)
          pGuild->getEvent().addEvent(EGUILDEVENT_DONATE, TVecString{rUser.name()});

        /*bool bSuccess = pGuild->getPack().checkAddItem(pCFG->vecGuildReward);
        if (bSuccess)
          pGuild->getPack().addItemToPack(pCFG->vecGuildReward);*/
        bool bSuccess = pGuild->getPack().addItem(pCFG->vecGuildReward) == ESYSTEMMSG_ID_MIN;
        for (auto &v : pCFG->vecGuildReward)
        {
          if (bSuccess)
          {
            XLOG << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "公会获得奖励" << v.id() << v.count() << XEND;
            thisServer->sendMsg(rUser.zoneid(), rUser.charid(), ESYSTEMMSG_ID_GUILD_PACK_ITEM, MsgParams(v.id(), v.count()));
          }
          else
          {
            XERR << "[公会捐赠]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "公会获得奖励" << v.id() << v.count() << "失败" << XEND;
          }
        }

        //log
        /*QWORD eid = xTime::getCurUSec();
          EVENT_TYPE eType = EventType_GuildMoney;
          PlatLogManager::getMe().eventLog(thisServer,
          pUser->m_platformId,
          pUser->getZoneID(),
          pUser->accid,
          pUser->id,
          eid,
          0,  // charge
          eType, 0, 1);
          PlatLogManager::getMe().SocialLog(thisServer,
          pUser->m_platformId,
          pUser->getZoneID(),
          pUser->accid,
          pUser->id,
          eType,
          eid,
          ESocial_GuildMoney,
          pGuild->getGUID(),
          0,
          pCFG->dwCon,
          pGuild->getAsset());*/
      }
      return true;
    case SOCIALPARAM_GUILD_APPLY:
      {
        PARSE_CMD_PROTOBUF(GuildApplySocialCmd, rev);
        GuildManager::getMe().applyGuild(rev.user(), rev.guildid());
      }
      return true;
    case SOCIALPARAM_GUILD_PROCESSINVITE:
      {
        PARSE_CMD_PROTOBUF(GuildProcessInviteSocialCmd, rev);
        GuildManager::getMe().processInviteMember(rev.user(), rev.guildid(), rev.action());
      }
      return true;
    case SOCIALPARAM_GM_MOVE_GUILD_ZONE:
      {
        PARSE_CMD_PROTOBUF(MoveGuildZoneSocialCmd, rev);
        GuildManager::getMe().moveGuildZone(rev.orizone(), rev.newzone());
      }
      return true;
    case SOCIALPARAM_GUILD_LEVELUP:
      {
        PARSE_CMD_PROTOBUF(GuildLevelUpSocialCmd, rev);
        Guild* pGuild = nullptr;
        if(rev.guildid())
        {
          pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        }

        if(pGuild == nullptr && !rev.guildname().empty())
        {
          pGuild = GuildManager::getMe().getGuildByGuildName(rev.guildname());
        }

        if(pGuild == nullptr && rev.charid())
        {
          pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        }

        if(pGuild != nullptr)
        {
          pGuild->levelup(rev.charid(),rev.addlevel());
          XLOG << "[公会-等级] charid:"<<rev.charid()<<"guildid"<<pGuild->getGUID()<<"addlevel"<<rev.addlevel() << XEND;
        }
      }
      return true;
    /*case SOCIALPARAM_DEL_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(DelTeamRaidSocialCmd, rev);
        Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam)
        {
          pTeam->delRaidZoneID(rev.raidid());
        }
        return true;
      }
      break;
    case SOCIALPARAM_GO_TEAM_RAID:
      {
        PARSE_CMD_PROTOBUF(GoTeamRaidSocialCmd, rev);
        Team *pTeam = TeamManager::getMe().getTeamByID(rev.teamid());
        if (pTeam)
        {
          DWORD zoneid = pTeam->getRaidZoneID(rev.raidid());
          if (zoneid)
          {
            rev.set_raidzoneid(zoneid);
            PROTOBUF(rev, send, len2);
            sendCmdToZone(rev.myzoneid(), send, len2);
            return true;
          }
          else
          {
            pTeam->setRaidZoneID(rev.raidid(), rev.myzoneid());
            rev.set_raidzoneid(rev.myzoneid());
            PROTOBUF(rev, send, len2);
            sendCmdToZone(rev.myzoneid(), send, len2);
          }
        }
        return true;
      }
      break;*/
    case SOCIALPARAM_SESSION_FORWARD_SOCIAL_CMD:
      {
        PARSE_CMD_PROTOBUF(SessionForwardSocialCmd, rev);
        /*if (rev.type() == ECMDTYPE_TEAM)
          TeamManager::getMe().doTeamCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_DOJO)
          TeamManager::getMe().doDojoCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_TOWER)
          TeamManager::getMe().doTowerCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());*/
        if (rev.type() == ECMDTYPE_GUILD)
          GuildManager::getMe().doGuildCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        else if (rev.type() == ECMDTYPE_CHAT)
        {
          doChatCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        }
        else if (rev.type() == ECMDTYPE_DOJO_GUILD)
        {
          GuildManager::getMe().doUserDojoCmd(rev.user(), (const BYTE*)rev.data().c_str(), rev.len());
        }
      }
      return true;
    case SOCIALPARAM_ONLINESTATUS:
      {
        PARSE_CMD_PROTOBUF(OnlineStatusSocialCmd, rev);
        if (rev.online())
        {
          GuildManager::getMe().onUserOnline(rev.user());
        }
        else
        {
          GuildManager::getMe().onUserOffline(rev.user());
        }
      }
      return true;
    case SOCIALPARAM_USER_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(UserInfoSyncSocialCmd, rev);
        GuildManager::getMe().updateUserInfo(rev);
      }
      return true;
    case SOCIALPARAM_USER_DEL_CHAR:
      {
        PARSE_CMD_PROTOBUF(UserDelSocialCmd, rev);
        GuildManager::getMe().delChar(rev.charid());
      }
      return true;
    case SOCIALPARAM_AUTHORIZE_SYNC_INFO:
      {
        PARSE_CMD_PROTOBUF(AuthorizeInfoSyncSocialCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild != nullptr)
        {
          GMember* pMember = pGuild->getMember(rev.charid());
          if (pMember != nullptr)
            pMember->setAuthorize(rev.ignorepwd());
        }
      }
      return true;
    case SOCIALPARAM_SYNC_REDTIP:
      {
        PARSE_CMD_PROTOBUF(SyncRedTipSocialCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild != nullptr)
        {
          if(EREDSYS_GUILD_ICON == rev.red())
          {
            GuildIconManager::getMe().setRead(pGuild->getGUID());
            break;
          }

          GMember* pMember = pGuild->getMember(rev.charid());
          if (pMember != nullptr)
          {
            pMember->setRedTip(rev.add(), false);

            if (/*rev.red() == EREDSYS_GUILD_CHALLENGE_ADD ||*/ rev.red() == EREDSYS_GUILD_CHALLENGE_REWARD)
              pMember->setRedTip(rev.red(), rev.add());
          }
        }
      }
      return true;
    default:
      return false;
  }

  return false;
}

bool GuildServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (!buf || !len) return false;
  using namespace Cmd;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->param)
  {
    case GUILDSPARAM_CHAT_SYNC:
      {
        PARSE_CMD_PROTOBUF(ChatSyncGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild != nullptr)
          pGuild->broadcastCmd((const void*)rev.data().c_str(), rev.len());
      }
      break;
   /* case GUILDSPARAM_LOAD_LUA:
      {
        PARSE_CMD_PROTOBUF(LoadLuaGuildSCmd, message);
        if (message.has_lua() == true)
          LuaManager::getMe().reload();
        if (message.has_log() == true)
          srvLog.reload();
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
              if (cfg.isGuildLoad())
                ConfigManager::getMe().loadConfig(cfg);
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (!cfg.isReload())
                continue;
              if (cfg.isGuildLoad())
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }
      }
      break;*/
    case GUILDSPARAM_QUERY_PHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryPhotoListGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-照片墙]" << rev.guildid() << "请求照片失败,没有该公会" << XEND;
          break;
        }
        pGuild->getPhoto().queryPhotoList();
      }
      break;
    case GUILDSPARAM_QUERY_USERPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryUserPhotoListGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-照片墙]" << rev.guildid() << "请求照片失败,没有该公会" << XEND;
          break;
        }
        pGuild->getPhoto().queryUserPhotoList(rev.user());
      }
      break;
    case GUILDSPARAM_QUERY_SHOWPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryShowPhotoGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-照片墙]" << rev.guildid() << "请求离线照片失败,没有该公会" << XEND;
          break;
        }
        if (rev.action() == EPHOTOACTION_UPDATE_FROM_RECORD || rev.action() == EPHOTOACTION_UPDATE_FROM_SCENE)
        {
          for (int i = 0; i < rev.results_size(); ++i)
          {
            const PhotoFrame& rFrame = rev.results(i);
            FrameUpdateGuildSCmd cmd;
            cmd.set_guildid(rev.guildid());
            cmd.set_frameid(rFrame.frameid());
            for (int j = 0; j < rFrame.photo_size(); ++j)
            {
              cmd.mutable_update()->CopyFrom(rFrame.photo(j));
              pGuild->getPhoto().updateFrame(cmd);
            }
          }
          if (rev.action() == EPHOTOACTION_UPDATE_FROM_RECORD)
            pGuild->getPhoto().queryPhotoList();
        }
        else
          XERR << "[公会消息]" << rev.ShortDebugString() << "未处理" << XEND;
      }
      break;
    case GUILDSPARAM_FRAME_UPDATE:
      {
        PARSE_CMD_PROTOBUF(FrameUpdateGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-照片墙]" << rev.guildid() << "照片操作失败,没有该公会" << XEND;
          break;
        }
        pGuild->getPhoto().updateFrame(rev);
      }
      break;
    case GUILDSPARAM_PHOTO_UPDATE:
      {
        PARSE_CMD_PROTOBUF(PhotoUpdateGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-照片墙]" << rev.guildid() << "照片更新失败,没有该公会" << XEND;
          break;
        }
        pGuild->getPhoto().updatePhoto(rev);
      }
      break;
    case GUILDSPARAM_RENAME_NTF:
      {
        PARSE_CMD_PROTOBUF(RenameNTFGuildCmd, rev);
        if(!rev.result())
          GuildManager::getMe().removeName(rev.newname());
        else
          GuildManager::getMe().rename(rev.user(), rev.newname());
      }
      break;
    case GUILDSPARAM_GUILD_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(GuildCityActionGuildSCmd, rev);
        if (rev.infos_size() <= 0)
        {
          XERR << "[公会城池-数据]" << rev.ShortDebugString() << "失败,无更新信息" << XEND;
          break;
        }

        if (rev.action() == EGUILDCITYACTION_TO_GUILD_UPDATE)
        {
          GuildCityActionGuildSCmd newcmd;
          newcmd.CopyFrom(rev);
          newcmd.clear_infos();
          for (int i = 0; i < rev.infos_size(); ++i)
          {
            GuildCityInfo* pInfo = rev.mutable_infos(i);

            QWORD qwGuildID = GuildManager::getMe().getCityOwner(rev.zoneid(), pInfo->flag());
            if (qwGuildID != 0)
            {
              Guild* pOldGuild = GuildManager::getMe().getGuildByID(qwGuildID);
              if (pOldGuild != nullptr)
              {
                GuildMisc& rMisc = pOldGuild->getMisc();
                XLOG << "[公会城池-数据] 更新 action :" << rev.action() << "公会" << pOldGuild->getGUID() << pOldGuild->getName() << "清空城池" << XEND;

                rMisc.setCityID(0);
                rMisc.setCityGiveupTime(0);

                pInfo->clear_portrait();
                pInfo->clear_name();
                pInfo->clear_membercount();
                pInfo->clear_lv();
              }
            }

            Guild* pGuild = GuildManager::getMe().getGuildByID(pInfo->id());
            if (pGuild != nullptr)
            {
              GuildMisc& rMisc = pGuild->getMisc();

              pGuild->getMisc().setCityID(pInfo->flag());
              pGuild->getMisc().setCityGiveupTime(0);

              pInfo->set_portrait(pGuild->getPortrait());
              pInfo->set_name(pGuild->getName());
              pInfo->set_membercount(pGuild->getMemberList().size());
              pInfo->set_lv(pGuild->getLevel());

              GMember* pMem = pGuild->getChairman();
              if (pMem)
                pInfo->set_leadername(pMem->getName());

              XLOG << "[公会城池-数据] 更新 action :" << rev.action() << "公会" << pGuild->getGUID() << pGuild->getName() << "设置城池" << rMisc.getCityID() << XEND;
            }

            GuildManager::getMe().setCityOwner(rev.zoneid(), pInfo->flag(), pInfo->id());
            newcmd.add_infos()->CopyFrom(*pInfo);
          }

          newcmd.set_action(EGUILDCITYACTION_TO_SCENE_UPDATE);
          PROTOBUF(newcmd, send, len);
          bool bSuccess = thisServer->sendCmdToZone(rev.zoneid(), send, len);
          XLOG << "[公会城池-数据] 更新 action :" << rev.action() << "数据" << newcmd.ShortDebugString() << "发送至SessionServer" << (bSuccess ? "成功" : "失败") << XEND;
        }
        else if (rev.action() == EGUILDCITYACTION_TO_GUILD_SAVE)
        {
          /*
          GuildCityActionGuildSCmd newcmd;
          newcmd.CopyFrom(rev);
          newcmd.clear_infos();
          for (int i = 0; i < rev.infos_size(); ++i)
          {
            const GuildCityInfo& rInfo = rev.infos(i);
            Guild* pGuild = GuildManager::getMe().getGuildByID(rInfo.id());
            if (pGuild == nullptr)
            {
              XERR << "[公会城池-数据] 更新 action :" << rev.action() << "数据 :" << rInfo.ShortDebugString() << "失败,无该公会" << XEND;
              continue;
            }
            if (pGuild->getZoneID() != rev.zoneid())
            {
              XERR << "[公会城池-数据] 更新 action :" << rev.action() << "数据 :" << rInfo.ShortDebugString() << "失败,公会区ID不对" << XEND;
              continue;
            }
            newcmd.add_infos()->CopyFrom(rInfo);

            if (pGuild->getMisc().getCityGiveupTime() != 0)
              newcmd.set_status(EGUILDCITYSTATUS_GIVEUP);
            else if (pGuild->getMisc().getCityID() == 0)
              newcmd.set_status(EGUILDCITYSTATUS_OCCUPY);
            else
              newcmd.set_status(EGUILDCITYSTATUS_NONE);
          }

          if (newcmd.infos_size() <= 0)
          {
            XERR << "[公会城池-数据] 更新数据" << newcmd.ShortDebugString() << "失败,无更新数据" << XEND;
            break;
          }

          newcmd.set_action(EGUILDCITYACTION_TO_RECORD_SAVE);
          PROTOBUF(newcmd, send, len);
          bool bSuccess = thisServer->sendCmdToZone(rev.zoneid(), send, len);
          XLOG << "[公会城池-数据] 更新数据" << newcmd.ShortDebugString() << "发送至SessionServer" << (bSuccess ? "成功" : "失败") << XEND;
          */
        }
        else if (rev.action() == EGUILDCITYACTION_TO_GUILD_RESET)
        {
          for (int i = 0; i < rev.infos_size(); ++i)
          {
            GuildCityInfo* pInfo = rev.mutable_infos(i);
            Guild* pGuild = GuildManager::getMe().getGuildByID(pInfo->id());
            if (pGuild != nullptr)
            {
              GuildMisc& rMisc = pGuild->getMisc();
              XLOG << "[公会城池-数据] 更新 action :" << rev.action() << "公会" << pGuild->getGUID() << pGuild->getName() << "清空城池" << XEND;

              rMisc.setCityID(0);
              rMisc.setCityGiveupTime(0);

              pInfo->set_id(0);
              pInfo->clear_portrait();
              pInfo->clear_name();
              pInfo->clear_membercount();
              pInfo->clear_lv();
            }
            GuildManager::getMe().setCityOwner(rev.zoneid(), pInfo->flag(), pInfo->id());
          }

          rev.set_action(EGUILDCITYACTION_TO_RECORD_SAVE);
          PROTOBUF(rev, send, len);
          bool bSuccess = thisServer->sendCmdToZone(rev.zoneid(), send, len);
          XLOG << "[公会城池-数据] 更新数据" << rev.ShortDebugString() << "发送至SessionServer" << (bSuccess ? "成功" : "失败") << XEND;
        }
      }
      break;
    case GUILDSPARAM_GVG_REWARD:
      {
        PARSE_CMD_PROTOBUF(GVGRewardGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会战-奖励], 找不到公会, 公会ID:" << rev.guildid() << XEND;
          return false;
        }
        const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
        const TVecGuildMember& mems = pGuild->getMemberList();
        for (auto &v : mems)
        {
          MailManager::getMe().sendMail(v->getCharID(), firecfg.dwWinGuildMailID);
          XLOG << "[公会战-奖励], 下发同公会奖励:" << firecfg.dwWinGuildMailID << "玩家:" << v->getCharID() << "公会:" << rev.guildid() << XEND;
        }
        const TSetQWORD& users = pGuild->getMisc().getGvg().getGvgPartInUsers();
        for (auto &s : users)
        {
          if (pGuild->getMember(s) == nullptr)
            continue;
          MailManager::getMe().sendMail(s, firecfg.dwSpecMailID);
          XLOG << "[公会战-奖励], 占据并参与奖, 公会:" << rev.guildid() << "玩家:" << s << "邮件:" << firecfg.dwWinGuildMailID << "发送成功" << XEND;
        }
      }
      break;
    case GUILDSPARAM_GVG_USER_PARTIN:
      {
        PARSE_CMD_PROTOBUF(GvgUserPartInGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会战-玩家参与], 找不到公会, 公会ID:" << rev.guildid() << "玩家:" << rev.charid() << XEND;
          return false;
        }

        pGuild->getMisc().getGvg().addGvgPartinUser(rev.charid());
      }
      break;
    case GUILDSPARAM_JOINSUPERGVG_REQ:
      {
        PARSE_CMD_PROTOBUF(JoinSuperGvgGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会战-决战报名], 找不到公会:" << rev.guildid() << XEND;
          return false;
        }
        pGuild->getMisc().getGvg().joinSuperGvg(rev.supergvgtime());
      }
      break;
    case GUILDSPARAM_SUPERGVG_END:
      {
        PARSE_CMD_PROTOBUF(EndSuperGvgGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会战-决战结束], 找不到公会:" << rev.guildid() << XEND;
          return false;
        }
        pGuild->getMisc().getGvg().finishSuperGvg(rev.rank());
      };
      break;
    case GUILDSPARAM_GUILDPRAY:
      {
        PARSE_CMD_PROTOBUF(GuildPrayGuildSCmd, message);
        if (message.msgid() != 0)
        {
          thisServer->sendMsg(message.user().zoneid(), message.user().charid(), message.msgid());
          return true;
        }

        const SocialUser& rUser = message.user();
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(message.user().charid());
        if (pGuild == nullptr)
        {
          XERR << "[公会祈祷-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "成功祈祷后异常退出了公会,可返还消耗资金 rob :" << message.needmon() << " 信仰之证 :" << message.prayitem() << "个" << XEND;
          return true;
        }
        GMember* pMember = pGuild->getMember(rUser.charid());
        if (pMember == nullptr)
        {
          XERR << "[公会祈祷-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "成功祈祷后异常退出了公会,可返还消耗资金 rob :" << message.needmon() << "信仰之证 :" << message.prayitem() << "个" << XEND;
          return true;
        }
        if (pMember->getContribution() < message.needcon())
        {
          XERR << "[公会祈祷-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name()
            << "成功祈祷异常贡献不足,可返还消耗资金 rob :" << message.needmon() << "信仰之证 :" << message.prayitem() << "个" << XEND;
          return true;
        }
        DWORD dwTotalContri = pMember->getContribution() - message.needcon();
        pMember->setContribution(dwTotalContri);

        PlatLogManager::getMe().outcomeMoneyLog(thisServer,
            0,
            pMember->getZoneID(),
            pMember->getAccid(),
            pMember->getCharID(),
            xTime::getCurUSec(),
            0, EMONEYTYPE_CONTRIBUTE, message.needcon(), dwTotalContri,
            ESOURCE_GUILDPRAY, 0, 0);

        DWORD dwPrayID = message.prayid();
        pMember->setPrayLv(dwPrayID, message.praylv());
        XLOG << "[公会祈祷-升级]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "job :" << pMember->getJob() <<
          "进行了 pray :" << dwPrayID << "祈祷, oldlv :" << pMember->getPrayLv(dwPrayID) - 1 << "-> newlv :" << pMember->getPrayLv(dwPrayID) << XEND;

        GuildManager::getMe().syncPrayToScene(rUser, GUILDOPTCONTYPE_SUB, true);

        //公会贡献消耗统计
        StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_CONSUME_COUNT, 145, ESOURCE_GUILDPRAY, rUser.baselv(), message.needcon());
        //platlog
          QWORD eid = xTime::getCurUSec();
          EVENT_TYPE eType = EventType_GuildPray;
          PlatLogManager::getMe().eventLog(thisServer,
          0,
          0,
          rUser.accid(),
          rUser.charid(),
          eid,
          0,  // charge
          eType, 0, 1);

          PlatLogManager::getMe().GuildPrayLog(thisServer,
          0,
          0,
          rUser.accid(),
          rUser.charid(),
          eType,
          eid,
          dwPrayID,
          pMember->getPrayLv(dwPrayID),
          message.prayitem(),
          message.needmon(),
          message.needcon()
          );
      }
      return true;
    case GUILDSPARAM_GUILD_ICON_STATE:
      {
        PARSE_CMD_PROTOBUF(GuildIconStateGuildSCmd, rev);
        for (int i = 0; i < rev.ids_size(); ++i)
        {
          QWORD qwGuildId = rev.ids(i)/1000;
          DWORD dwIndex = rev.ids(i)%1000;
          Guild* pGuild = GuildManager::getMe().getGuildByID(qwGuildId);
          if (!pGuild)
          {
            XERR << "[公会图标-状态变更], 找不到公会, 公会ID:" << qwGuildId << XEND;
            continue;
          }

          GuildIconManager::getMe().setState(qwGuildId, dwIndex, rev.state());

          GMember* pMember = pGuild->getChairman();
          if(!pMember)
          {
            XERR << "[公会图标-状态变更], 找不到公会会长, 公会ID:" << qwGuildId << XEND;
            continue;
          }

          pMember->redTipMessage(EREDSYS_GUILD_ICON);
          XLOG << "[公会图标-状态变更] 状态变更" << rev.ShortDebugString() << XEND;
        }
      }
      break;
    case GUILDSPARAM_SUBMIT_MATERIAL:
      {
        PARSE_CMD_PROTOBUF(SubmitMaterialGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild == nullptr)
          return false;

        if (rev.success() == false)
        {
          XERR << "[公会-提交建筑材料]" << pGuild->getGUID() << pGuild->getName() << "材料:" << rev.ShortDebugString() << "场景返回失败" << XEND;
          return false;
        }

        if (pGuild->getMisc().getBuilding().getSubmitCounter() != rev.counter())
          return false;
        pGuild->getMisc().getBuilding().unlockSubmit();

        GMember* pMember = pGuild->getMember(rev.charid());
        if (pMember == nullptr)
          return false;

        TMapMaterial material;
        for (int i = 0; i < rev.materials_size(); ++i)
        {
          auto it = material.find(rev.materials(i).id());
          if (it == material.end())
            material[rev.materials(i).id()] = rev.materials(i).count();
          else
            material[rev.materials(i).id()] += rev.materials(i).count();
        }

        if (pGuild->getMisc().getBuilding().submitMaterial(rev.building(), material, pMember, rev.submitinc()) == false)
        {
          XERR << "[公会-提交建筑材料]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getCharID() << "材料:" << rev.ShortDebugString() << "提交失败" << XEND;
          return false;
        }

        XLOG << "[公会-提交建筑材料]" << pGuild->getGUID() << pGuild->getName() << "玩家:" << pMember->getCharID() << "材料:" << rev.ShortDebugString() << "提交成功" << XEND;
      }
      break;
    case GUILDSPARAM_QUERY_GUILD_INFO:
      {
        PARSE_CMD_PROTOBUF(QueryGuildInfoGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-查询公会信息]" << rev.guildid() << "失败,没有该公会" << XEND;
          break;
        }
        pGuild->toData(rev.mutable_info());
        rev.set_result(true);
        PROTOBUF(rev, send, len1);
        thisServer->sendCmdToZone(pGuild->getZoneID(), send, len1);
      }
      break;
    case GUILDSPARAM_CHALLENGE_PROGRESS:
      {
        PARSE_CMD_PROTOBUF(ChallengeProgressGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会挑战-进度更新]" << rev.guildid() << rev.ShortDebugString() << "公会找不到" << XEND;
          break;
        }
        GMember* pMember = pGuild->getMember(rev.charid());
        if (pMember == nullptr)
        {
          XERR << "[公会挑战-进度更新]" << rev.guildid() << rev.charid() << rev.ShortDebugString() << "成员找不到" << XEND;
          break;
        }
        vector<GuildChallengeItem*> items;
        for (int i = 0; i < rev.items_size(); ++i)
          items.push_back(rev.mutable_items(i));
        pGuild->getMisc().getChallenge().updateProgress(pMember, items);
      }
      break;
    case GUILDSPARAM_GM_COMMAND:
      {
        PARSE_CMD_PROTOBUF(GMCommandGuildSCmd, rev);
        bool bResult = GMCommandMgr::getMe().exec(rev);

        GMCommandRespondGuildSCmd cmd;
        cmd.mutable_info()->CopyFrom(rev.info());
        if (cmd.info().guildid() == 0)
        {
          Guild* pGuild = GuildManager::getMe().getGuildByUserID(cmd.info().charid());
          if (pGuild != nullptr)
            cmd.mutable_info()->set_guildid(pGuild->getGUID());
        }
        PROTOBUF(cmd, send, len);
        if (thisServer->sendCmdToZone(cmd.info().zoneid(), send, len) == false)
          XERR << "[公会GM-回应] 执行指令" << rev.ShortDebugString() << (bResult ? "成功" : "失败") << "响应" << cmd.ShortDebugString() << "失败" << XEND;
        else
          XLOG << "[公会GM-回应] 执行指令" << rev.ShortDebugString() << (bResult ? "成功" : "失败") << "响应" << cmd.ShortDebugString() << "成功" << XEND;
      }
      break;
    case GUILDSPARAM_BUILDINGEFFECT:
      {
        PARSE_CMD_PROTOBUF(BuildingEffectGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild == nullptr)
          return false;

        GMember* pMember = pGuild->getMember(rev.charid());
        if (pMember == nullptr)
          return false;

        pMember->resetBuildingEffect();
      }
      break;
    case GUILDSPARAM_TREASURE_QUERY:
      {
        PARSE_CMD_PROTOBUF(QueryTreasureGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
          break;
        pGuild->getMisc().getTreasure().queryTreasureToScene();
      }
      break;
    case GUILDSPARAM_TREASURE_RESULTNTF:
      {
        PARSE_CMD_PROTOBUF(TreasureResultNtfGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-福利] 宝箱福利通知" << rev.ShortDebugString() << "失败,charid :" << rev.charid() << "不是公会成员" << XEND;
          break;
        }
        pGuild->getMisc().getTreasure().addResult(rev);
      }
      break;
    case GUILDSPARAM_SHOP_BUY_ITEM:
      {
        PARSE_CMD_PROTOBUF(ShopBuyItemGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByUserID(rev.charid());
        if (pGuild == nullptr)
          break;
        GMember* member = pGuild->getMember(rev.charid());
        if (member == nullptr)
          break;
        bool needreset = false;
        if (member->getFrameStatus() == false)
        {
          member->setFrameStatus(true);
          needreset = true;
        }
        pGuild->getMisc().getShop().buyItem(member, rev.id(), rev.count());
        if (needreset) member->setFrameStatus(false);
      }
      break;
    case GUILDSPARAM_UPDATE_CITY:
      {
        PARSE_CMD_PROTOBUF(UpdateCityGuildSCmd, rev);
        const SGuildCityCFG* pCFG = GuildRaidConfig::getMe().getGuildCityCFG(rev.cityid());
        if (pCFG == nullptr)
        {
          XERR << "[公会-城池] guildid :" << rev.guildid() << (rev.add() == true ? "占领" : "失去") << rev.cityid() << "更新失败,未在 Table_Guild_StrongHold.txt 表中找到" << XEND;
          break;
        }
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-城池] guildid :" << rev.guildid() << (rev.add() == true ? "占领" : "失去") << pCFG->dwCityID << pCFG->strName << "更新失败,未在 Table_Guild_StrongHold.txt 表中找到" << XEND;
          break;
        }

        DWORD dwNow = now();
        stringstream sstr;
        GuildEventM& rEvent = pGuild->getEvent();
        TVecString vecParams;

        sstr.str("");
        sstr << xTime::getYear(dwNow);
        vecParams.push_back(sstr.str());

        sstr.str("");
        sstr << xTime::getMonth(dwNow);
        vecParams.push_back(sstr.str());

        sstr.str("");
        sstr << xTime::getDay(dwNow);
        vecParams.push_back(sstr.str());

        vecParams.push_back(pCFG->strName);
        rEvent.addEvent(rev.add() == true ? EGUILDEVENT_GVG_GET : EGUILDEVENT_GVG_LOST, vecParams);
      }
      break;
    case GUILDSPARAM_GUILD_MAIL:
      {
        PARSE_CMD_PROTOBUF(GuildBrocastMailGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-邮件], 找不到公会:" << rev.guildid() << "邮件:" << rev.mailid() << XEND;
          break;
        }

        const TVecGuildMember& mems = pGuild->getMemberList();
        if (rev.items_size())
        {
          TVecItemInfo vecItem;
          for (int i = 0; i < rev.items_size(); ++i)
            vecItem.push_back(rev.items(i));

          for (auto &v : mems)
          {
            MailManager::getMe().sendMail(v->getCharID(), rev.mailid(), vecItem);
            XLOG << "[公会-邮件], 邮件:" << rev.mailid() << "玩家:" << v->getCharID() << "公会:" << rev.guildid() << XEND;
          }
        }
        else
        {
          for (auto &v : mems)
          {
            MailManager::getMe().sendMail(v->getCharID(), rev.mailid());
            XLOG << "[公会-邮件], 邮件:" << rev.mailid() << "玩家:" << v->getCharID() << "公会:" << rev.guildid() << XEND;
          }
        }
        return true;
      }
      break;
    case GUILDSPARAM_GUILD_MSG:
      {
        PARSE_CMD_PROTOBUF(GuildBrocastMsgGuildSCmd, rev);
        Guild* pGuild = GuildManager::getMe().getGuildByID(rev.guildid());
        if (pGuild == nullptr)
        {
          XERR << "[公会-Msg], 找不到公会:" << rev.guildid() << "msg:" << rev.msgid() << XEND;
          break;
        }
        MsgParams param;
        for (int i = 0; i < rev.params_size(); ++i)
        {
          const MsgParam& p = rev.params(i);

          if (!p.param().empty())
            param.addString(p.param());
          else if (p.subparams_size())
          {
            param.addSubParam();
            for (int j = 0; j < p.subparams_size(); ++j)
              param.addSubString(p.subparams(j));
          }
        }
        pGuild->broadcastMsg(rev.msgid(), param);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool GuildServer::doSessionCmd(BYTE* buf, WORD len)
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
    {
      LuaManager::getMe().reload();
    }
    if (message.has_log() == true)
    {
      srvLog.reload();
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
          if (cfg.isGuildLoad())
            ConfigManager::getMe().loadConfig(cfg);
        }
      }
      for (auto &it : vec)
      {
        ConfigEnum cfg;
        if (ConfigManager::getMe().getConfig(it, cfg))
        {
          if (!cfg.isReload())
            continue;
          if (cfg.isGuildLoad())
            ConfigManager::getMe().checkConfig(cfg);
        }
      }
    }

    XINF << "[策划表-重加载] zoneid:" << thisServer->getZoneID() << "source serverid" << message.serverid() << "lua:" << message.lua() << "table:" << message.table() << "log:" << message.log() << "allzone:" << message.allzone() << XEND;
    return true;
  }
  default:
    return false;
  }

  return false;
}

bool GuildServer::doChatCmd(const SocialUser& rUser, const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;

  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
  case CHATPARAM_QUERY_REALTIME_VOICE_ID:
  {
    Guild* pGuild = GuildManager::getMe().getGuildByUserID(rUser.charid());
    if (pGuild == nullptr)
    {
      XERR << "[聊天管理-查询实时语音房间号]" << rUser.charid() << "没有公会,查询失败" << XEND;
      return true;
    }
    GMember* pMember = pGuild->getMember(rUser.charid());
    if (pMember == nullptr)
    {
      XERR << "[聊天管理-查询实时语音房间号]" << pGuild->getGUID() << rUser.charid() << "成员找不到" << XEND;
      return true;
    }
    pMember->sendRealtimeVoiceID();
    return true;
  }
  default:
    return false;
  }

  return false;
}


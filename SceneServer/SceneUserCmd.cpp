#include "SceneUser.h"
#include "SceneManager.h"
#include "xCmd.pb.h"
#include "SceneUser.pb.h"
#include "SceneUser2.pb.h"
#include "SceneMap.pb.h"
#include "CarrierCmd.pb.h"
#include "SceneItem.pb.h"
#include "FuBenCmd.pb.h"
#include "ScenePet.pb.h"
#include "MsgManager.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "SceneItemManager.h"
#include "UserEvent.h"
#include "GMCommandRuler.h"
#include "SkillManager.h"
#include "SkillItem.h"
#include "DScene.h"
#include "SceneServer.h"
#include "SceneTip.h"
#include "ChatRoomManager.h"
#include "ChatManager_SC.h"
#include "InfiniteTower.pb.h"
#include "SceneTower.h"
#include "MiscConfig.h"
#include "UserConfig.h"
#include "MusicBoxManager.h"
#include "SessionCmd.pb.h"
#include "SealConfig.h"
#include "SceneTrade.pb.h"
#include "UserEvent.pb.h"
#include "Dojo.pb.h"
#include "DojoConfig.h"
#include "RedisManager.h"
#include "StatisticsDefine.h"
#include "ActivityManager.h"
#include "XoclientConfig.h"
#include "DepositConfig.h"
#include "NpcConfig.h"
#include "SceneAugury.pb.h"
#include "AuguryMgr.h"
#include "MiscConfig.h"
#include "BaseConfig.h"
#include "SessionSociality.pb.h"
#include "CommonConfig.h"
#include "GuildMusicBoxManager.h"
#include "GuildRaidConfig.h"
#include "GuildCityManager.h"
#include "ActivityEventManager.h"
#include "SceneActManager.h"
#include "SceneWeddingMgr.h"
#include "PetWork.h"
#include "PveCard.pb.h"
#include "PveCardConfig.h"
#include "SceneShop.h"
#include "Menu.h"
#include "UserRecords.h"
#include "TeamRaidCmd.pb.h"
#include "SceneBoothManager.h"
#include "DressUpStageMgr.h"
#include "ExchangeShop.h"
#include "BossMgr.h"
extern "C"
{
#include "xlib/md5/md5.h"
}

bool SceneUser::doUserCmd(const Cmd::UserCmd* cmd,WORD len)
{
  if (getUserState() != USER_STATE_RUN)
  {
    XERR << "[消息]" << accid << id << getProfession() << name << "尚未完成登录就收到消息" << cmd->cmd << cmd->param << XEND;
    return true;
  }

  switch(cmd->cmd)
  {
    case SCENE_USER_PROTOCMD:
      return doSceneUserCmd(cmd, len);
    case SCENE_USER2_PROTOCMD:
      return doSceneUser2Cmd(cmd, len);
    case SCENE_USER_ITEM_PROTOCMD:
      return doSceneUserItemCmd(cmd, len);
    case SCENE_USER_SKILL_PROTOCMD:
      return doSceneUserSkillCmd(cmd, len);
    case SCENE_USER_QUEST_PROTOCMD:
      return doSceneUserQuestCmd(cmd, len);
    case SCENE_USER_MAP_PROTOCMD:
      return doSceneUserMapCmd(cmd, len);
    case SCENE_USER_PET_PROTOCMD:
      return doSceneUserPetCmd(cmd, len);
    case FUBEN_PROTOCMD:
      return doSceneUserFuBenCmd(cmd, len);
    case SCENE_USER_CARRIER_PROTOCMD:
      return doSceneUserCarrierCmd(cmd, len);
    case SCENE_USER_TIP_PROTOCMD:
      return doSceneUserTipCmd(cmd, len);
    case SCENE_USER_CHATROOM_PROTOCMD:
      return doSceneUserChatRoomCmd(cmd, len);
    case INFINITE_TOWER_PROTOCMD:
      return doSceneUserTowerCmd(cmd, len);
    case SCENE_USER_INTER_PROTOCMD:
      return doSceneUserInterCmd(cmd, len);
    case SCENE_USER_MANUAL_PROTOCMD:
      return doSceneUserManualCmd(cmd, len);
    case SCENE_USER_SEAL_PROTOCMD:
      return doSceneUserSealCmd(cmd, len);
    case SESSION_USER_SHOP_PROTOCMD:
      return doSessionUserShopCmd(cmd, len);
    case SCENE_USER_TRADE_PROTOCMD:
      return doSceneUserTrade(cmd, len);
    case USER_EVENT_PROTOCMD:
      return doUserEventCmd(cmd, len);
    case DOJO_PROTOCMD:
      return doSceneUserDojoCmd(cmd, len);
    case CHAT_PROTOCMD:
      return ChatManager_SC::getMe().doChatCmd(this, (const BYTE*)cmd, len);
    case SESSION_USER_SOCIALITY_PROTOCMD:
      return doSocialCmd(cmd, len);
    case SCENE_USER_AUGURY_PROTOCMD:
      return doSceneUserAuguryCmd(cmd, len);
    case SESSION_USER_TEAM_PROTOCMD:
      return doSessionUserTeamCmd(cmd, len);
    case SCENE_USER_ACHIEVE_PROTOCMD:
      return doSceneUserAchieveCmd(cmd, len);
    case SCENE_USER_ASTROLABE_PROTOCMD:
      return doSceneUserAstrolabeCmd(cmd, len);
    case SCENE_USER_PHOTO_PROTOCMD:
      return doSceneUserPhotoCmd(cmd, len);
    case SCENE_USER_FOOD_PROTOCMD:
      return doSceneUserFoodCmd(cmd, len);
    case SCENE_USER_TUTOR_PROTOCMD:
      return doSceneUserTutorCmd(cmd, len);
    case SCENE_USER_BEING_PROTOCMD:
      return doSceneUserBeingCmd(cmd, len);
    case SESSION_USER_AUTHORIZE_PROTOCMD:
      return doSessionAuthorizeCmd(cmd, len);
    case SESSION_USER_GUILD_PROTOCMD:
      return doSessionUserGuildCmd(cmd, len);
    case WEDDINGC_PROTOCMD:
      return doSceneWeddingCmd(cmd, len);
    case PVE_CARD_PROTOCMD:
      return doSceneUserPveCardCmd(cmd, len);
    case TEAM_RAID_PROTOCMD:
      return doSceneUserTeamRaidCmd(cmd, len);
    case SCENE_BOSS_PROTOCMD:
      return doSceneUserBossCmd(cmd, len);
    default:
      break;
  }

  return false;
}

bool SceneUser::doSceneUserCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf) return false;
  switch (buf->param)
  {
    case GOTO_EXIT_POS_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(GoToExitPosUserCmd, rev);
#ifndef _ALL_SUPER_GM
        return true;
#endif
        if (!getScene()) return true;
        if (rev.mapid() && getScene()->getMapID() != rev.mapid())
        {
          this->m_blShowSkill = true;
          gomap(rev.mapid(), GoMapType::GM);
          return true;
        }
        if (!this->m_oTmpData.m_eProfession)
        {
          static std::vector<EProfession> pVec = { EPROFESSION_WARRIOR, EPROFESSION_KNIGHT, EPROFESSION_LORDKNIGHT, EPROFESSION_RUNEKNIGHT, EPROFESSION_MAGICIAN, EPROFESSION_WIZARD, EPROFESSION_HIGHWIZARD, EPROFESSION_WARLOCK, EPROFESSION_THIEF, EPROFESSION_ASSASSIN, EPROFESSION_ASSASSINCROSS, EPROFESSION_GUILLOTINECROSS, EPROFESSION_ACOLYTE, EPROFESSION_PRIEST, EPROFESSION_HIGHPRIEST, EPROFESSION_ARCHBISHOP };
          if (!this->m_blShowSkill)
          {
            random_shuffle(pVec.begin(), pVec.end());
            this->m_oTmpData.m_dwHead = randBetween(45001, 45056);
            this->m_oTmpData.m_dwBack = randBetween(47002, 47012);

            this->m_oTmpData.m_dwMount = randBetween(25001, 25007);
            this->setDataMark(EUSERDATATYPE_MOUNT);

            this->m_oTmpData.m_eProfession = *(pVec.begin());

            const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(this->m_oTmpData.m_eProfession);
            if (pCFG)
            {
              if (m_oUserSceneData.getGender() == EGENDER_MALE)
                this->m_oTmpData.m_dwBody = pCFG->maleBody;
              else if (m_oUserSceneData.getGender() == EGENDER_FEMALE)
                this->m_oTmpData.m_dwBody = pCFG->femaleBody;
              this->setDataMark(EUSERDATATYPE_BODY);
            }
          }
        }
        this->m_blShowSkill = true;
        this->showSkill();
        xPos p;
        if (getScene()->getRandTargetPos(getPos(), 5, p))
        {
          m_oMove.setFinalPoint(p);
        }
        /*
        {
          SceneObject *pObject = getScene()->getSceneObject();
          if (pObject)
          {
            std::vector<ExitPoint> vec;
            for (auto it : pObject->m_oExitPoints)
            {
              if (it.second.m_dwNextMapID < 100)
              {
                vec.push_back(it.second);
              }
            }
            if (!vec.empty())
            {
              random_shuffle(vec.begin(), vec.end());
              for (auto &it : vec)
              {
                if (m_oMove.setFinalPoint(it.m_oPos))
                {
                  m_oMove.setExit(it.m_dwExitID, it.m_oPos);
                  m_oMove.heartbeat();
                  return true;
                }
              }
            }
          }
        }
        */
        return true;
      }
      break;
    case GOTO_RANDOM_POS_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(GoToRandomPosUserCmd, rev);
#ifndef _ALL_SUPER_GM
        return true;
#endif
        if (!getScene()) return true;
        xPos pos;
        pos.set(rev.pos().x(), rev.pos().y(), rev.pos().z());
        gomap(rev.mapid(), GoMapType::GM, pos);
        if (!this->m_oTmpData.m_eProfession)
        {
          static std::vector<EProfession> pVec = { EPROFESSION_MERCHANT,  EPROFESSION_BLACKSMITH, EPROFESSION_WHITESMITH, EPROFESSION_MECHANIC, EPROFESSION_ALCHEMIST, EPROFESSION_CREATOR, EPROFESSION_GENETIC };
          if (!this->m_blShowSkill)
          {
            random_shuffle(pVec.begin(), pVec.end());
            this->m_oTmpData.m_dwHead = randBetween(45001, 45056);
            this->m_oTmpData.m_dwBack = randBetween(47002, 47012);

            this->m_oTmpData.m_dwMount = randBetween(25003, 25020);
            this->setDataMark(EUSERDATATYPE_MOUNT);

            this->m_oTmpData.m_eProfession = *(pVec.begin());

            const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(this->m_oTmpData.m_eProfession);
            if (pCFG)
            {
              if (m_oUserSceneData.getGender() == EGENDER_MALE)
                this->m_oTmpData.m_dwBody = pCFG->maleBody;
              else if (m_oUserSceneData.getGender() == EGENDER_FEMALE)
                this->m_oTmpData.m_dwBody = pCFG->femaleBody;
              this->setDataMark(EUSERDATATYPE_BODY);
            }

            this->addSkill(278001, SKILL_SOURCEID, ESOURCE_NORMAL);
          }
        }
        return true;
      }
      break;
    case RECONNECTION_POS_USER_CMD:
      {
        /*
        if (!getScene())
        {
          XLOG << "[重连重置坐标]" << id << name << "未进入场景" << XEND;
          return true;
        }

        PARSE_CMD_PROTOBUF(ReconnectionPosUserCmd, rev);

        xPos p;
        p.set(rev.pos().x(), rev.pos().y(), rev.pos().z());
        XLOG << "[重连重置坐标]" << id << name << "设置坐标" << p.x << p.y << p.z << XEND;
        goTo(p);
        */
        XERR << "[无效协议], 非正常客户端发送, 玩家" << name << id << XEND;
        return true;
      }
      break;
    case RELIVE_USER_CMD:
      {
        if (isAlive()) return true;
        PARSE_CMD_PROTOBUF(ReliveUserCmd, rev);

        //relive(rev.type());

        return true;
      }
      break;
    case USERPARAM_GMCOMMAND:
      {
        PARSE_CMD_PROTOBUF(UserGMCommand, rev);

        if (BaseConfig::getMe().getBranch() == BRANCH_DEBUG || (BaseConfig::getMe().getBranch() == BRANCH_TF && CommonConfig::isInTFGMWhiteList(this->accid) == true))
        {
          bool bSuccess = GMCommandRuler::getMe().execute(this, rev.command());
          stringstream sstr;
          sstr << "执行 " << rev.command() << (bSuccess ? " 成功" : " 失败");
          MsgManager::sendMsg(id, 10, MsgParams(sstr.str()));
        }
        return true;
      }
      break;
    case USERPARAM_PROFESSIONEXCHANGE:
      {
        PARSE_CMD_PROTOBUF(UserProfessionExchange, rev);
        exchangeProfession(rev.profession());
        return true;
      }
      break;
    case SKILL_BROADCAST_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(SkillBroadcastUserCmd, message);

        clearHitMe();

        //XDBG << "[技能消息], 收到前端技能消息, 玩家:" << name << id << "技能:" << message.skillid() << "number:" << message.data().number() << "时间:" << xTime::getCurMSec() << XEND;
        switch (message.data().number())
        {
          // special skill can break by client
          case ESKILLNUMBER_BREAK:
            {
              /*if (m_oSkillProcessor.getRunner().getBreak() == true)
              {
                m_oSkillProcessor.breakSkill(0, true);
                return true;
              }*/
              if (message.skillid() == 20002001 || 20000001 == message.skillid() || 20006001 == message.skillid() || 20001001 == message.skillid())
              {
                m_oSkillProcessor.breakSkill(id, true);
                return true;
              }
              else
              {
                // 超出释放距离，可被打断
                const SkillBroadcastUserCmd& olddata = m_oSkillProcessor.getCurCmd();
                const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(olddata.skillid());
                //打断定点技能
                if (pSkillCFG != nullptr && !pSkillCFG->isOutRangeBreak() && isCanMoveChant())
                {
                  xPos oPos;
                  oPos.set(olddata.data().pos().x(), olddata.data().pos().y(), olddata.data().pos().z());
                  if (getDistance(oPos, this->getPos()) > pSkillCFG->getLaunchRange(this) * 1.2)
                  {
                    m_oSkillProcessor.breakSkill(id, true);
                    return true;
                  }
                }
                //打断锁定目标技能
                if (pSkillCFG != nullptr && pSkillCFG->isOutRangeBreak() && olddata.data().hitedtargets_size() >= 1)
                {
                  xSceneEntryDynamic* enemy = xSceneEntryDynamic::getEntryByID(olddata.data().hitedtargets(0).charid());
                  if (!enemy) return true;
                  if (getDistance(enemy->getPos(), this->getPos()) > pSkillCFG->getLaunchRange(this) * 1.2)
                  {
                    m_oSkillProcessor.breakSkill(id, true);
                    return true;
                  }
                  if (enemy->isAlive() == false)
                  {
                    m_oSkillProcessor.breakSkill(id, true);
                    return true;
                  }
                  if (enemy->getAttr(EATTRTYPE_HIDE) == 1)
                  {
                    m_oSkillProcessor.breakSkill(id, true);
                    return true;
                  }
                }
              }
              return true;
            }
            break;
          case ESKILLNUMBER_CHANT:
          case ESKILLNUMBER_MOVECHANT:
            {
              if (!isCanMoveChant() && message.data().number() == ESKILLNUMBER_MOVECHANT)
              {
                return true;
              }

              if (message.data().hitedtargets_size() == 1)
              {
                SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(message.data().hitedtargets(0).charid());
                if (pNpc != nullptr && pNpc->m_ai.isBeChantAttack() == true)
                {
                  pNpc->m_ai.setCurLockID(id);
                  pNpc->m_ai.changeState(ENPCSTATE_ATTACK);
                }
              }
            }
            break;
          default:
            break;

        }

        message.set_charid(id);
        m_oSkillProcessor.setActiveSkill(message);

        return true;
      }
      break;
    case CHANGE_SCENE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ChangeSceneUserCmd, message);

        Scene *pScene = SceneManager::getMe().getSceneByID(m_oUserSceneData.getOnlineMapID());
        if (pScene == nullptr)
        {
          DWORD dwMapID = m_oUserSceneData.getLastMapID();
          pScene = SceneManager::getMe().getSceneByID(dwMapID);
          if (pScene == nullptr)
            dwMapID = MiscConfig::getMe().getSystemCFG().dwNewCharMapID;
          gomap(dwMapID, GoMapType::GoCity);
          MsgManager::sendMsg(id, 10, MsgParams("副本提前关闭,跳转失败"));
          XDBG << "[切换地图异常], 玩家:" << name << id << "目标地图已关闭:" << dwMapID << XEND;
          return true;
        }

        if (pScene->isDScene() == false)
        {
          if (message.mapid() != pScene->getMapID())
          {
            XLOG << "[切换地图重叠], 玩家:" << name << id << "到达了之前的地图:" << message.mapid() << "忽略该消息, 等待客户端进入最新地图" << pScene->getMapID() << XEND;
            return true;
          }
        }

        if (m_blInScene)
        {
          if (m_oUserSceneData.getOnlineMapID()==pScene->id)
          {
            this->goTo(m_oUserSceneData.getOnlinePos());
            this->delMeToNine();
            this->delNineToMe();
            this->sendMeToNine();
            this->sendNineToMe();

            if (TowerConfig::getMe().isTower(pScene->getMapID()) == true)
            {
              TowerScene* pTScene = dynamic_cast<TowerScene*> (pScene);
              if (pTScene)
                pTScene->setChangeScene(false);
            }
          }
          Cmd::MapCmdEnd message;
          PROTOBUF(message, send, len);
          this->sendCmdToMe(send, len);
          XDBG << "[切换地图异常], 玩家已在地图上:" << name << id << "地图:" << pScene->id << XEND;
          return true;
        }
        if (!this->enterScene(pScene))
        {
          XERR << "[切换地图异常], 进入场景失败:" << name << id << "地图:" << pScene->id << XEND;
        }

        return true;
      }
      break;
    case CHANGE_BODY_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ChangeBodyUserCmd, message);

        XLOG << "无效协议ChangeBodyUserCmd, 已删除" << name << id << XEND;
        return true;

        if (message.male() <= EGENDER_MIN || message.male() >= EGENDER_MAX)
          return true;

        if (message.male())
          m_oUserSceneData.setGender(static_cast<EGender>(message.male()));
        //if (message.body())
        //  m_oUserSceneData.setBody(message.body());
        //if (message.hair())
          //m_oUserSceneData.setHair(message.hair());
        //if (message.righthand())
        //  m_oUserSceneData.setRighthand(message.righthand());
        //if (message.accessory())
        //  m_oUserSceneData.setAccessory(message.accessory());
        //if (message.wing())
        //  m_oUserSceneData.setWing(message.wing());

        if (getScene())
        {
          message.set_charid(id);
          message.set_male(m_oUserSceneData.getGender());
          message.set_body(m_oUserSceneData.getBody());
          //message.set_hair(m_oUserSceneData.getHair());
          //message.set_righthand(m_oUserSceneData.getRighthand());
          //message.set_accessory(m_oUserSceneData.getAccessory());
          //message.set_wing(m_oUserSceneData.getWing());
          PROTOBUF(message, send, len);
          getScene()->sendCmdToNine(getPos(), send, len);
        }
        return true;
      }
      break;
    case REQ_MOVE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ReqMoveUserCmd, message);

        // 摆摊状态 无法移动
        if(m_oBooth.hasOpen())
          return true;

        xPos p;
        p.set(message.target().x(), message.target().y(), message.target().z());
        if (!getScene())
        {
       //   DEBUG_MSG2USER(this, "无法移动，未进入场景");
          XDBG << "[无法移动]" << id << name << "未进入场景,目标点:(" << p.x << p.y << p.z << ")" << XEND;
          //log("无法移动", LOG_DBG, "未进入场景,目标点:(%f,%f,%f)", p.x, p.y, p.z);
          return true;
        }
        xPos p2 = p;
        if (!getScene()->getValidPos(p2))
        {
          MsgManager::sendMsg(id, 10, MsgParams("目标点不合法"));
          XDBG << "[无法移动]" << id << name << "目标点:(" << p.x << p.y << p.z << ")" << XEND;
          //log("无法移动", LOG_DBG, "目标点:(%f,%f,%f)", p.x, p.y, p.z);
   //       return true;
        }

        if (getScene()->isSuperGvg())
        {
          SuperGvgScene* pGScene = dynamic_cast<SuperGvgScene*>(getScene());
          if (pGScene && pGScene->isInPrepare())
          {
            if (MiscConfig::getMe().getSuperGvgCFG().isOutLimitArea(getPos()))
            {
              XLOG << "[玩家-移动], 决战准备阶段, 不可移动到墙外, 强制拉回, 玩家:" << name << id << "位置:" << getPos().x << getPos().y << getPos().z << XEND;
              xPos bornpos;
              pGScene->getBornPos(this, bornpos);
              goTo(bornpos, false, true);
              return true;
            }
          }
        }
        if (getScene()->getSceneType() == SCENE_TYPE_TEAMPWS)
        {
          TeamPwsScene* pTScene = dynamic_cast<TeamPwsScene*>(getScene());
          if (pTScene && pTScene->isInPrepare())
          {
            if (MiscConfig::getMe().getTeamPwsCFG().isOutLimitArea(getPos()))
            {
              XLOG << "[玩家-移动], 组队排位赛准备阶段, 不可移动到墙外, 强制拉回, 玩家:" << name << id << "位置:" << getPos().x << getPos().y << getPos().z << XEND;
              xPos bornpos;
              pTScene->getBornPos(this, bornpos);
              goTo(bornpos);
              return true;
            }
          }
        }

        if (m_oMove.setFinalPoint(p2))
        {
        //  log("客户端移动", LOG_DBG, "当前坐标:(%f,%f,%f),目标点:(%f,%f,%f),校正点:(%f,%f,%f)",
         //     getPos().x, getPos().y, getPos().z, p.x, p.y, p.z, p2.x, p2.y, p2.z);

          if (getStatus() == ECREATURESTATUS_FAKEDEAD)
            setStatus(ECREATURESTATUS_LIVE);
          seatUp();
          m_oSceneFood.breakCooking();
          m_oSceneFood.breakEating();
          clearHitMe();
          m_oMove.heartbeat(xTime::getCurUSec() / ONE_THOUSAND);
          m_oBuff.onClientMove();
          m_oWeaponPet.onUserMove(p2);
          m_oUserPet.onUserMove(p2);
          m_oUserBeing.onUserMove(p2);
          m_oUserElementElf.onUserMove(p2);
          m_oServant.onUserMove(p2);
          onTwinsMove();

          std::list<SceneNpc *> npclist;
          m_oFollower.get(npclist);

          // 包含牵手玩家的跟随者
          if (m_oHands.isInWait() == false && m_oHands.has())
          {
            SceneUser* m_pOther = m_oHands.getOther();
            if (m_pOther != nullptr)
            {
              m_pOther->m_oFollower.get(npclist);
              m_pOther->getWeaponPet().onUserMove(p2);
              m_pOther->getUserPet().onUserMove(p2);
              m_pOther->getUserBeing().onUserMove(p2);
              m_pOther->getUserElementElf().onUserMove(p2);
              m_pOther->getServant().onUserMove(p2);
            }
          }
          if (!npclist.empty())
          {
            xPos dest = p2;
            for (auto it : npclist)
            {
              if (it)
              {
                std::list<xPos> list;
                // getScene()->getRandPos(p, 1.0f, dest);
                if (getScene()->findingPath(dest, it->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
                {
                  XDBG << "[Follower]" << accid<<id<<getProfession()<<name << "当前坐标:(" << it->getPos().x<<it->getPos().y<<it->getPos().z << "),目标点:(" << p.x<<p.y<<p.z << "),校正点:(" << p2.x<<p2.y<<p2.z << ")" << XEND;
                  //log("[Follower] 寻路失败", LOG_DBG, "%llu, %llu, %u, %s, 当前坐标:(%f,%f,%f),目标点:(%f,%f,%f),校正点:(%f,%f,%f)",
                  //  accid, id, getProfession(), name, it->getPos().x, it->getPos().y, it->getPos().z, p.x, p.y, p.z, p2.x, p2.y, p2.z);
                  continue;
                }
                if (list.size() > 1)
                {
                  list.pop_front();
                  dest = list.front();
                  it->m_oMove.setStraightPoint(dest);
                  it->m_oMove.action(xTime::getCurMSec());
                }
              }
            }
          }

          //setStatus(ECREATURESTATUS_LIVE);
        }
        else
        {
          if (!xServer::isOuter() && isAlive())
          {
            MsgManager::sendMsg(id, 10, MsgParams("无法移动,寻路失败"));
          }
          XDBG << "[客户端移动失败]" << "当前坐标:(" << getPos().x<<getPos().y<<getPos().z << "),目标点:(" << p.x<<p.y<<p.z << "),校正点:(" << p2.x<<p2.y<<p2.z << ")" << XEND;
          //log("客户端移动失败", LOG_DBG, "当前坐标:(%f,%f,%f),目标点:(%f,%f,%f),校正点:(%f,%f,%f)",
          //    getPos().x, getPos().y, getPos().z, p.x, p.y, p.z, p2.x, p2.y, p2.z);
        }

        return true;
      }
      break;
    case MAP_OBJECT_DATA:
      {
        //return true;
        PARSE_CMD_PROTOBUF(MapObjectData, rev);
        QWORD guid = rev.guid();

        xSceneEntryDynamic* pEntry = SceneUserManager::getMe().getUserByID(guid);
        if (pEntry == nullptr)
          pEntry = SceneNpcManager::getMe().getNpcByTempID(guid);
        if(pEntry != nullptr)
        {
          SceneUser* pUser = dynamic_cast<SceneUser*> (pEntry);
          if(pUser != nullptr)
          {
            setVisitUser(pUser->id);
            setVisitNpc(0);
          }
          else
          {
            SceneNpc *pNpc = dynamic_cast<SceneNpc*> (pEntry);
            if (pNpc != nullptr)
            {
              setVisitNpc(pNpc->id);
              setVisitUser(0);
            }
          }
        }
        return true;

        if (pEntry != nullptr)
        {
          // add attr
          UserAttr* pAttr = rev.add_attrs();
          if (pAttr != nullptr)
          {
            pAttr->set_type(EATTRTYPE_HP);
            pAttr->set_value(pEntry->getAttr(EATTRTYPE_HP));
          }
          pAttr = rev.add_attrs();
          if (pAttr != nullptr)
          {
            pAttr->set_type(EATTRTYPE_MAXHP);
            pAttr->set_value(pEntry->getAttr(EATTRTYPE_MAXHP));
          }
        }

        // send cmd
        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case 10:
      {
        //PARSE_CMD_PROTOBUF(UserTest, rev);
      }
      break;
    case USER_FACE_CMD:
      {
        //PARSE_CMD_PROTOBUF(UserFaceCmd, message);
        sendCmdToNine(buf, len);
      }
      break;
  }

  return false;
}

bool SceneUser::doSceneUser2Cmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    case USER2PARAM_UPLOAD_OK_SCENERY:
      {
        PARSE_CMD_PROTOBUF(UploadOkSceneryUserCmd, rev);

        getScenery().upload(rev);

        return true;
      }
      break;
    case USER2PARAM_INVITE_JOIN_HANDS:
      {
        PARSE_CMD_PROTOBUF(InviteJoinHandsUserCmd, rev);

        //if (m_oHands.has()) return true;

        if (getTransform().isInTransform())
        {
          MsgManager::sendMsg(this->id, 830);
          return true;
        }
        if (isHandEnable() == false)
        {
          MsgManager::sendMsg(this->id, 556);
          return true;
        }
        SceneUser* pTarget = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pTarget == nullptr || pTarget->isAlive() == false || pTarget->getScene() != getScene())
        {
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_HAND_UNENABLE);
          return true;
        }

        // 自己处于摆摊状态
        if(m_oBooth.hasOpen())
        {
          XERR << "[玩家-牵手]" << accid << id << getProfession() << name << "邀请牵手失败，摆摊状态下不能发起邀请，疑似客户端作弊" << XEND;
          return true;
        }

        // 目标处于摆摊状态
        if(pTarget->m_oBooth.hasOpen())
        {
          MsgManager::sendMsg(id, 25711);
          return true;
        }

        if (pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK) == true || pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK_FOREVER) == true)
        {
          XERR << "[玩家-玩笑2消息]" << accid << id << getProfession() << name
            << "邀请" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "牵手失败,黑名单" << XEND;
          return true;
        }

        if (getHandNpc().haveHandNpc())
          getHandNpc().delHandNpc();
        if (getWeaponPet().haveHandCat())
          getWeaponPet().breakHand();
        if (m_oUserPet.handPet())
          m_oUserPet.breakHand();
        /*if (pTarget->m_oHands.has())
        {
          return true;
        }
        */

        /*
        bool bResult = getDistance(getPos(), pTarget->getPos()) <= MiscConfig::getMe().getSystemCFG().dwHandRange;
        if (!bResult)
        {
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_HAND_UNENABLE);
          return true;
        }
        */
        if (m_oHands.checkInTeam(rev.charid()))
        {
          rev.set_masterid(this->id);
          rev.set_mastername(this->name);
          rev.set_time(now() + 300);

          char sign[1024];
          bzero(sign, sizeof(sign));
          std::stringstream ss;
          ss << rev.charid() << "_" << rev.masterid() << "_" << rev.time() << "_" << "#$%^&";
          upyun_md5(ss.str().c_str(), ss.str().size(), sign);

          rev.set_sign(sign);

          PROTOBUF(rev, send, len);

          thisServer->sendCmdToMe(rev.charid(), send, len);
        }

        return true;
      }
      break;
    case USER2PARAM_JOIN_HANDS:
      {
        // 被邀请者返回
        PARSE_CMD_PROTOBUF(JoinHandsUserCmd, rev);

        //if (m_oHands.has()) return true;
        if (getHandNpc().haveHandNpc()) return true;
        if (rev.time() < now()) return true;
        if (m_oHands.has()) m_oHands.breakup();
        if (m_oWeaponPet.haveHandCat())
          m_oWeaponPet.breakHand();
        if (m_oUserPet.handPet())
          m_oUserPet.breakHand();

        SceneUser* pMaster = SceneUserManager::getMe().getUserByID(rev.masterid());
        if (pMaster == nullptr || pMaster->isAlive() == false || isAlive() == false || pMaster->getScene() != getScene())
        {
          MsgManager::sendMsg(id, ESYSTEMMSG_ID_HAND_UNENABLE);
          return true;
        }

        // 自己处于摆摊状态
        if(m_oBooth.hasOpen())
        {
          XERR << "[玩家-牵手]" << accid << id << getProfession() << name << "接受邀请失败，摆摊状态下不能接受邀请" << XEND;
          return true;
        }

        // 目标处于摆摊状态
        if(pMaster->m_oBooth.hasOpen())
        {
          MsgManager::sendMsg(id, 25711);
          return true;
        }

        char sign[1024];
        bzero(sign, sizeof(sign));
        std::stringstream ss;
        ss << this->id << "_" << rev.masterid() << "_" << rev.time() << "_" << "#$%^&";
        upyun_md5(ss.str().c_str(), ss.str().size(), sign);

        if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
          return true;

        // 主牵人已跟他人建立牵手关系, 后来者优先
        if (pMaster->m_oHands.has())
          pMaster->m_oHands.breakup();
        if (pMaster->m_oWeaponPet.haveHandCat())
          pMaster->m_oWeaponPet.breakHand();
        if (pMaster->getHandNpc().haveHandNpc())
          pMaster->getHandNpc().delHandNpc();
        if (pMaster->getUserPet().handPet())
          pMaster->getUserPet().breakHand();

        if (m_oHands.checkInTeam(rev.masterid()))
        {
          m_oUserSceneData.setFollowerID(rev.masterid(), EFOLLOWTYPE_HAND);
          //m_oHands.m_qwForceJoinUserID = rev.masterid();
          //m_oHands.enterScene();
          /*
          Cmd::InviteHandsSessionCmd message;
          message.set_charid(this->id);
          message.set_otherid(rev.charid());
          PROTOBUF(message, send, len);
          thisServer->forwardCmdToUserScene(this->id, send, len);
          */
        }

        /*if (hasChatRoom())
        {
          ChatRoomManager::getMe().exitRoom(this, getChatRoomID());
        }
        */
        return true;
      }
      break;
    case USER2PARAM_BREAK_UP_HANDS:
      {
        PARSE_CMD_PROTOBUF(BreakUpHandsUserCmd, rev);

        m_oHands.breakup();

        return true;
      }
      break;
    case USER2PARAM_HANDSTATUS:
      {
        PARSE_CMD_PROTOBUF(HandStatusUserCmd, rev);
        if (rev.type() == 0)
          m_oHands.changeHandStatus(rev.build(), rev.masterid());
        else if (rev.type() == 1 && rev.build() == true)
          changeTwinsStatus(rev.masterid());
        else if (rev.type() == 1 && rev.build() == false)
          onTwinsMove();
        return true;
      }
      break;
    case USER2PARAM_DOWNLOAD_SCENERY_PHOTO:
      {
        sendUpyunUrl();
        return true;
      }
      break;
    case USER2PARAM_UPLOAD_SCENERY_PHOTO:
      {
        PARSE_CMD_PROTOBUF(UploadSceneryPhotoUserCmd, rev);

        if (!rev.has_sceneryid())
        {
          XERR << "[玩家-upyun上传路径]" << accid << id << getProfession() << name << "请求路径失败,未包含 sceneryid" << XEND;
          return true;
        }

        std::string photodir;
        if (getAlbumName(rev.type(), photodir) == false)
        {
          XERR << "[玩家-upyun上传路径]" << accid << id << getProfession() << name << "请求路径失败,获取 type :" << rev.type() << "失败" << XEND;
          return true;
        }

        std::stringstream stream;
        stream.str("");
        if (thisServer->isOuter())
          stream << "/game/" << photodir << thisServer->getRegionID();
        else
          stream << "/debug/" << photodir << thisServer->getRegionID();

        if (rev.type() == EALBUMTYPE_SCENERY)
          stream << "/acc/" << this->accid;
        else if (rev.type() == EALBUMTYPE_PHOTO)
          stream << "/user/" << this->id;
        else if (rev.type() == EALBUMTYPE_WEDDING)
        {
          DWORD dwID = getUserWedding().getWeddingInfo().id();
          if (dwID == 0)
          {
            XERR << "[玩家-upyun上传路径]" << accid << id << getProfession() << name << "请求路径失败type :" << rev.type() << "为设置weddingid" << XEND;
            return true;
          }
          stream << "/" << this->id << "/" << dwID;
        }
        else
        {
          XERR << "[玩家-upyun上传路径]" << accid << id << getProfession() << name << "请求路径失败,未知 type :" << rev.type() << XEND;
          return true;
        }

        stream << "/" << rev.sceneryid();
        XDBG << "[玩家-upyun上传路径]" << accid << id << getProfession() << name << "路径" << stream.str() << XEND;

        std::string policy,signature;
        CommonConfig::upyun_form_str(stream.str(), policy, signature);

        UploadSceneryPhotoUserCmd message;
        message.set_policy(policy);
        message.set_signature(signature);
        message.set_sceneryid(rev.sceneryid());
        message.set_type(rev.type());

        PROTOBUF(message, send, len);
        sendCmdToMe(send, len);

        return true;
      }
      break;
    case USER2PARAM_GOMAP_FOLLOW:
      {
        PARSE_CMD_PROTOBUF(GoMapFollowUserCmd, rev);

        if (getTeamID() == 0 || getScene() == nullptr)
          return true;

        const TeamMemberInfo* pMember = getTeamMember(rev.charid());
        if (pMember == nullptr)
          return true;

        if (rev.mapid() != pMember->mapid())
        {
          XERR << "[队员地图数据错误], 队员:" << pMember->name() << "server地图:" << pMember->mapid() << "client地图:" << rev.mapid() << XEND;
        }

        if(getDressUp().getDressUpStatus() != 0)
          return true;

        if (MapConfig::getMe().isRaidMap(pMember->mapid()) == false && MapConfig::getMe().isRaidMap(getScene()->getMapID()) == false)
        {
          //rev.set_mapid(pMember->mapid());
          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
          return true;
        }
        //自己在公会领地 跟随的人在静态地图
        const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(getScene()->getMapID());
        if (pRaidCFG && pRaidCFG->eRestrict == ERAIDRESTRICT_GUILD && MapConfig::getMe().isRaidMap(pMember->mapid()) == false)
        {
          DWORD mapid = getUserMap().getLastStaticMapID();
          const xPos& pos = getUserMap().getLastStaticMapPos();
          gomap(mapid, GoMapType::Follow, pos);
          //rev.set_mapid(pMember->mapid());
          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
          return true;
        }

        const SRaidCFG* base = MapConfig::getMe().getRaidCFG(pMember->raidid());
        if (base != nullptr)
        {
          switch (base->eRestrict)
          {
            case ERAIDRESTRICT_MIN:
            case ERAIDRESTRICT_MAX:
              return true;
            case ERAIDRESTRICT_PRIVATE:
            case ERAIDRESTRICT_SYSTEM:
            case ERAIDRESTRICT_PVP_ROOM:
              {
                getUserSceneData().setFollowerID(0);
                MsgManager::sendMsg(id, 329);
              }
              return true;
            case ERAIDRESTRICT_GUILD:
              {
                if (getGuild().id() == 0)
                  return true;

                m_oZone.gomap(m_oGuild.zoneid(), pMember->raidid(), GoMapType::Follow);
              }
              return true;
            case ERAIDRESTRICT_GUILD_RANDOM_RAID:
              return true;
            case ERAIDRESTRICT_WEDDING:
              return true;
            case ERAIDRESTRICT_TEAM:
            case ERAIDRESTRICT_GUILD_TEAM:
            case ERAIDRESTRICT_USER_TEAM:
            case ERAIDRESTRICT_GUILD_FIRE:
              break;
            case ERAIDRESTRICT_HONEYMOON:
              if (getQuest().isAccept(QUEST_WEDDING_HONEYMOON) == false)
              {
                MsgManager::sendMsg(id, 9654);
                return false;
              }
              break;
          }
        }
        if (base && base->eRaidType == ERAIDTYPE_TOWER)
        {
          if (getLevel() < MiscConfig::getMe().getEndlessTowerCFG().dwLimitUserLv)
          {
            MsgManager::sendMsg(id, 1315);
            break;
          }
        }
        else if (base && base->eRaidType == ERAIDTYPE_PVECARD)
        {
          // pve 副本不可跟随进入
          return true;
        }
        gomap(pMember->mapid(), GoMapType::Follow);
      }
      return true;
    case USER2PARAM_GOMAP_QUEST:
      {
#ifndef _ALL_SUPER_GM
        return true;
#endif

        /*PARSE_CMD_PROTOBUF(GoMapQuestUserCmd, rev);

        if (getQuest().isSubmit(rev.questid())) return true;

        TPtrQuestItem pItem = getQuest().getQuest(rev.questid());
        if (pItem == nullptr)
          return true;

        TPtrBaseStep pStep = pItem->getStepCFG();
        if (pStep == nullptr)
          return true;

        if (getScene() != nullptr)
        {
          if (getScene()->isDScene() == true || pStep->getMapID() != getScene()->getMapID())
            m_oUserSceneData.setFollowerID(0);
          if (EQUESTSTEP_MOVE == pStep->getStepType())
          {
            MoveStep *p = (MoveStep *)(&(*pStep));
            if (p->m_dwPathMapID)
            {
              if (getScene()->id == p->m_dwPathMapID || getScene()->id == p->getMapID())
              {
              }
              else
              {
                const SceneBase* pBase = SceneManager::getMe().getDataByID(p->m_dwPathMapID);
                if (pBase)
                {
                  const SceneObject *pObject = pBase->getSceneObject(0);
                  if (pObject != nullptr)
                  {
                    const xPos* pBorn = pObject->getBornPoint(1);
                    if (pBorn != nullptr)
                      gomap(p->m_dwPathMapID, GoMapType::Quest, *pBorn); //p->m_oPathPos);
                  }
                }
              }
              return true;
            }
          }
        }

        if (pStep->getMapID())
          gomap(pStep->getMapID(), GoMapType::Quest);*/

        return true;
      }
      break;
    case USER2PARAM_SCENERY:
      {
        PARSE_CMD_PROTOBUF(SceneryUserCmd, rev);

        if (rev.scenerys().size())
        {
          getScenery().photo(rev.scenerys(0));
        }
        return true;
      }
      break;
    case USER2PARAM_GOTO_LABORATORY:
      {
        PARSE_CMD_PROTOBUF(GotoLaboratoryUserCmd, rev);
        if (getMenu().isOpen(EMENUID_LABORATORY) == false)
          return true;

        const SNpcFunction* pCFG = TableManager::getMe().getNpcFuncCFG(rev.funid());
        if (pCFG == nullptr)
          return false;
        DWORD raid = pCFG->getRaid();
        gomap(raid, GoMapType::Laboratory);
        return true;
      }
      break;
    case USER2PARAM_EXIT_POS:
      {
        PARSE_CMD_PROTOBUF(ExitPosUserCmd, rev);

        if (!getScene()) return true;

        if (getScene()->getMapID()!=rev.mapid()) return true;

        //if (m_oHands.has() && !m_oHands.IsMaster) return true;

        xPos p;
        p.set(rev.pos().x(), rev.pos().y(), rev.pos().z());
        m_oMove.setExit(rev.exitid(), p);
        m_oMove.checkExit();

        return true;
      }
      break;
    /*case USER2PARAM_GOCITY:
      {
        PARSE_CMD_PROTOBUF(GoCity, rev);

        gomap(NEW_CHAR_MAP_ID, GoMapType::GoCity);
        return true;
      }
      break;*/
    case USER2PARAM_ENTER_CAPRA_ACTIVITY:
    {
      DWORD mapid = MiscConfig::getMe().getCapraActivityCFG().mapId;
      XLOG << "[活动-卡普拉] 玩家尝试进入卡普拉活动地图，charid" << id << "name" << name << "mapid" << mapid << XEND;
      gomap(mapid, GoMapType::GoCity);
      return true;
    }
    case USER2PARAM_ACTION:
      {
        PARSE_CMD_PROTOBUF(UserActionNtf, rev);
        if (m_oUserSceneData.haveAction(rev.value()) == false && m_oUserSceneData.haveExpression(rev.value()) == false)
        {
          XERR << "[动作表情], 玩家:" << id << name << "播放动作表情:" << rev.value() << "未解锁" << XEND;
          return false;
        }
        rev.set_charid(id);
        m_event.onAction(rev.type(), rev.value());
        PROTOBUF(rev, send, len);
        sendCmdToNine(send, len, id);

        return true;
      }
      break;
    case USER2PARAM_RELIVE:
      {
        PARSE_CMD_PROTOBUF(Relive, rev);
        if (canClientRelive(rev.type()) == false)
          return true;

        relive(rev.type());
        return true;
      }
      break;
    case USER2PARAM_SERVERTIME:
      {
        PARSE_CMD_PROTOBUF(ServerTime, rev);
        rev.set_time(xTime::getCurMSec());

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        XLOG << "ServerTime:" << rev.time() << "userID:" << this->name << XEND;
        return true;
      }
      break;
    case USER2PARAM_USEPORTRAIT:
      {
        PARSE_CMD_PROTOBUF(UsePortrait, rev);
        m_oPortrait.usePortrait(rev.id());
        return true;
      }
      break;
    case USER2PARAM_USEFRAME:
      {
        PARSE_CMD_PROTOBUF(UseFrame, rev);
        m_oPortrait.useFrame(rev.id());
        return true;
      }
      break;
    case USER2PARAM_ADDATTRPOINT:
      {
        PARSE_CMD_PROTOBUF(AddAttrPoint, rev);
        if (rev.type() == POINTTYPE_ADD)
        {
          if (rev.strpoint() != 0)
            addAttrPoint(EUSERDATATYPE_STRPOINT, rev.strpoint());
          if (rev.intpoint() != 0)
            addAttrPoint(EUSERDATATYPE_INTPOINT, rev.intpoint());
          if (rev.agipoint() != 0)
            addAttrPoint(EUSERDATATYPE_AGIPOINT, rev.agipoint());
          if (rev.dexpoint() != 0)
            addAttrPoint(EUSERDATATYPE_DEXPOINT, rev.dexpoint());
          if (rev.vitpoint() != 0)
            addAttrPoint(EUSERDATATYPE_VITPOINT, rev.vitpoint());
          if (rev.lukpoint() != 0)
            addAttrPoint(EUSERDATATYPE_LUKPOINT, rev.lukpoint());
        }
        else if (rev.type() == POINTTYPE_RESET)
          resetAttrPoint();
        return true;
      }
      break;
    case USER2PARAM_QUERYSHOPGOTITEM:
      {
        getSceneShop().queryGotItem();
        return true;
      }
      break;
    case USER2PARAM_FOLLOWTRANSFER:
      {
        /*
        PARSE_CMD_PROTOBUF(FollowTransferCmd, rev);
        if (id == rev.targetid())
          return true;

        SceneUser *pUser = SceneUserManager::getMe().getUserByID(rev.targetid());
        if (nullptr != pUser && m_oTeamInfo.teamid() != 0)
        {
          float dist = ::getDistance(getPos(), pUser->getPos());
          if (dist < 10.0f)
            return true;

          if (getTeamMember(pUser->id) != nullptr && m_oUserSceneData.getOnlineMapID() == pUser->getUserSceneData().getOnlineMapID())
            gomap(m_oUserSceneData.getOnlineMapID(), GoMapType::Follow, pUser->getPos());
        }
        */
        return true;
      }
      break;
    case USER2PARAM_PRESETCHATMSG:
      {
        PARSE_CMD_PROTOBUF(PresetMsgCmd, rev);
        m_oMsg.resetMsgs(rev);
        return true;
      }
      break;
    case USER2PARAM_STATECHANGE:
      {
        PARSE_CMD_PROTOBUF(StateChange, rev);
        if (getStatus() != ECREATURESTATUS_DEAD)
        {
          if (rev.status() == ECREATURESTATUS_PHOTO)
          {
            setStatus(rev.status());
            m_event.onCameraChange();
          }
          else if (rev.status() == ECREATURESTATUS_MIN)
          {
            setStatus(ECREATURESTATUS_LIVE);
          }

          if (rev.status() != getStatus())
          {
            if (rev.status() == ECREATURESTATUS_PHOTO || rev.status() == ECREATURESTATUS_SELF_PHOTO)
              m_event.onOpenCamera();
          }
        }
        return true;
      }
      break;
    case USER2PARAM_PHOTO:
      {
        PARSE_CMD_PROTOBUF(Photo, rev);
        if (getStatus() == ECREATURESTATUS_PHOTO)
        {
          PROTOBUF(rev, send, len);
          sendCmdToNine(send, len, id);
        }
        return true;
      }
      break;
    case USER2PARAM_CAMERAFOCUS:
      {
        PARSE_CMD_PROTOBUF(CameraFocus, rev);
        if (getStatus() == ECREATURESTATUS_PHOTO)
        {
          TVecQWORD vecids;
          for (int i = 0; i < rev.targets_size(); ++i)
            vecids.push_back(rev.targets(i));
          m_event.onFocusNpc(vecids);
        }
      }
      return true;
    case USER2PARAM_QUERYSHORTCUT:
      {
        m_oShortcut.sendAllShortcut();
        return true;
      }
      break;
    case USER2PARAM_PUTSHORTCUT:
      {
        PARSE_CMD_PROTOBUF(PutShortcut, rev);
        m_oShortcut.putShortcut(rev.item());
        return true;
      }
      break;
    case USER2PARAM_GOTO_GEAR:
      {
        PARSE_CMD_PROTOBUF(GoToGearUserCmd, rev);
        m_oFreyja.goToGear(rev);
        return true;
      }
      break;
    case USER2PARAM_TRANSFER:
      {
        PARSE_CMD_PROTOBUF(UseDeathTransferCmd, rev);
        m_oTransfer.goTransfer(rev);
        return true;
      }
    case USER2PARAM_FOLLOWER:
      {
        PARSE_CMD_PROTOBUF(FollowerUser, rev);
        m_oUserSceneData.setFollowerID(rev.userid());
      }
      return true;
    case USER2PARAM_INVITEFOLLOW:
      {
        PARSE_CMD_PROTOBUF(InviteFollowUserCmd, rev);
        handleFollower(rev);
      }
      return true;
    case USER2PARAM_MUSIC_LIST:
      {
        PARSE_CMD_PROTOBUF(QueryMusicList, rev);
        GuildScene* pGuildScene = dynamic_cast<GuildScene*>(getScene());
        if (pGuildScene != nullptr)
          GuildMusicBoxManager::getMe().queryMusicList(this, rev.npcid());
        else
          MusicBoxManager::getMe().queryMusicList(this, rev.npcid());
      }
      return true;
    case USER2PARAM_MUSIC_DEMAND:
      {
        PARSE_CMD_PROTOBUF(DemandMusic, rev);
        GuildScene* pGuildScene = dynamic_cast<GuildScene*>(getScene());
        if (pGuildScene != nullptr)
          GuildMusicBoxManager::getMe().demandMusic(this, rev.npcid(), rev.musicid());
        else
          MusicBoxManager::getMe().demandMusic(this, rev.npcid(), rev.musicid());
      }
      return true;
    case USER2PARAM_MUSIC_CLOSE:
      {
        setBrowse(false);
      }
      return true;
    case USER2PARAM_UPDATE_TRACE_LIST:
      {
        PARSE_CMD_PROTOBUF(UpdateTraceList, rev);
        m_oUserSceneData.updateTraceItem(rev);
      }
      return true;
    case USER2PARAM_SET_DIRECTION:
      {
        PARSE_CMD_PROTOBUF(SetDirection, rev);
        m_oUserSceneData.setDir(rev.dir());
      }
      return true;
    case USER2PARAM_BATTLE_TIMELEN_USER_CMD:
      {
        sendBattleStatusToMe();
        return true;
      }
      break;
    case USER2PARAM_SETOPTION:
      {
        PARSE_CMD_PROTOBUF(SetOptionUserCmd, rev);
        m_oUserSceneData.setQueryType(rev.type());
        m_oUserSceneData.setFashionHide(rev.fashionhide());
        m_oUserSceneData.setQueryWeddingType(rev.wedding_type());

        return true;
      }
      break;
    case USER2PARAM_SET_NORMALSKILL_OPTION:
    {
      PARSE_CMD_PROTOBUF(SetNormalSkillOptionUserCmd, rev);
      m_oUserSceneData.setNormalSkillOption(rev.flag());
      return true;
    }
    break;
    case USER2PARAM_NEW_SET_OPTION:
    {
      PARSE_CMD_PROTOBUF(NewSetOptionUserCmd, rev);
      m_oUserSceneData.setOption(rev.type(), rev.flag());
      return true;
    }
    break;
    case USER2PARAM_QUERYUSERINFO:
      {
        PARSE_CMD_PROTOBUF(QueryUserInfoUserCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser != nullptr)
        {
          rev.set_teamid(pUser->getTeamID());
          rev.set_blink(pUser->getUserSceneData().getBlink());

          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
        }
      }
      return true;
    case USER2PARAM_SHAKETREE:
      {
        PARSE_CMD_PROTOBUF(ShakeTreeUserCmd, rev);
        Scene* pScene = getScene();
        if (pScene != nullptr)
          pScene->getSceneTreasure().shakeTree(this, rev.npcid());
      }
      return true;

    case USER2PARAM_QUERY_ZONESTATUS:
      {
        PARSE_CMD_PROTOBUF(QueryZoneStatusUserCmd, rev);
        ChatManager_SC::getMe().toData(rev, this);
        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      return true;
    case USER2PARAM_JUMP_ZONE:
      {
        PARSE_CMD_PROTOBUF(JumpZoneUserCmd, rev);
        jumpZone(rev.npcid(), rev.zoneid());
      }
      return true;
    case USER2PARAM_CHANGENAME:
      {
        PARSE_CMD_PROTOBUF(ChangeNameUserCmd, rev);
        changeName(rev.name());
      }
      return true;
    case USER2PARAM_CHARGEPLAY:
      {
        // record in team
        PARSE_CMD_PROTOBUF(ChargePlayUserCmd, rev);
        playChargeNpc(rev);
      }
      return true;
    case USER2PARAM_REQUIRENPCFUNC:
      {
        // 送审服 屏蔽该功能
        if (thisServer->getPlatformID() == CommonConfig::m_dwVerifyServerPlatID)
          return true;

        PARSE_CMD_PROTOBUF(RequireNpcFuncUserCmd, rev);
        const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rev.npcid());
        if (pCFG && pCFG->stNpcFunc.mapRequireFuncs.empty() == false)
        {
          rev.clear_functions();
          for (auto &m : pCFG->stNpcFunc.mapRequireFuncs)
          {
            rev.add_functions(m.second);

            /*NpcFunc* pFunc = rev.add_functions();
            if (!pFunc)
              continue;
            pFunc->set_id(m.first);
            for (auto&v : m.second)
            {
              pFunc->add_param(v);
            }        */    
          }
          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
        }
      }
      return true;
    case USER2PARAM_CHECK_SEAT:
    {
      PARSE_CMD_PROTOBUF(CheckSeatUserCmd, rev);
      if (rev.success() == true)
      {
        bool ret = checkSeat(rev.seatid());
        rev.set_success(ret);
        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      else
        seatUp();

      return true;
    }
    case USER2PARAM_YOYO_SEAT:
    {
      PARSE_CMD_PROTOBUF(YoyoSeatUserCmd, rev);
      checkYoYoSeat(rev.guid());
      return true;
    }
    break;
    case USER2PARAM_TRANSFORM_PREDATA:
    {
      sendTransformPreData();
      return true;
    }
    break;
    case USER2PARAM_UPYUN_AUTHORIZATION:
      {
        sendUpyunAuthorization();
      }
      return true;
    case USER2PARAM_USER_RENAME:
      {
        PARSE_CMD_PROTOBUF(UserRenameCmd, rev);

        DWORD itemId = MiscConfig::getMe().getPlayerRenameCFG().dwRenameItemId;
        MainPackage* pPackage = dynamic_cast<MainPackage*>(m_oPackage.getPackage(EPACKTYPE_MAIN));
        if(!pPackage || !pPackage->checkItemCount(itemId, 1))
          return false;
        if(!checkRenameTime())
        {
          UserRenameCmd cmd;
          cmd.set_name(rev.name());
          cmd.set_code(ERENAME_CD);

          PROTOBUF(cmd, send, len);
          sendCmdToMe(send, len);
          return false;
        }

        // 通知recordserver
        UserRenameQueryRecordCmd cmd;
        cmd.set_charid(id);
        cmd.set_accid(accid);
        cmd.set_oldname(name);
        cmd.set_newname(rev.name());
        cmd.set_scenename(thisServer->getServerName());

        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToRecord(send, len);
        return true;
      }
      break;
    case USER2PARAM_BUY_ZENY:
    {
      PARSE_CMD_PROTOBUF(BuyZenyCmd, rev);
      rev.set_ret(false);
      do 
      {
        if (rev.bcoin() == 0)
          break;        
        if (!subMoney(EMONEYTYPE_LOTTERY, rev.bcoin(), ESOURCE_SHOP))
        {
          XERR << "[B格猫金币购买zeny] B格猫金币数量不够"<<id <<name <<rev.bcoin() << XEND;
          break;
        }
        QWORD qwZeny = rev.bcoin() * 10000;
        addMoney(EMONEYTYPE_SILVER, qwZeny, ESOURCE_SHOP, true);
        rev.set_zeny(qwZeny);
        rev.set_ret(true);
        XLOG << "[B格猫金币购买zeny] 兑换成功" << id << name << "花费猫币" << rev.bcoin() << "获得zeny" << qwZeny << XEND;
      } while (0);

      PROTOBUF(rev, send, len);
      sendCmdToMe(send, len);
      return true;
    }
    break;
    case USER2PARAM_CALL_TEAMER:
      break;
    case USER2PARAM_CALL_TEAMER_JOIN:
      {
        PARSE_CMD_PROTOBUF(CallTeamerReplyUserCmd, rev);
        if (rev.time() < now())
          return true;

        char sign[1024];
        bzero(sign, sizeof(sign));
        std::stringstream ss;
        ss << rev.masterid() << "@" << rev.time() << "@" << rev.mapid() << "@" << rev.pos().x() << rev.pos().y() << rev.pos().z() <<"_" << "#$%^&";

        upyun_md5(ss.str().c_str(), ss.str().size(), sign);
        if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
          return true;

        Cmd::ChangeSceneSessionCmd message;
        message.set_mapid(rev.mapid());
        message.add_charid(this->id);
        message.mutable_pos()->CopyFrom(rev.pos());
        PROTOBUF(message, send, len);
        thisServer->sendCmdToSession(send, len);

        return true;
      }
      break;
    case USER2PARAM_MARRIAGE_PROPOSAL_REPLY:
      {
        // 被邀请者返回
        PARSE_CMD_PROTOBUF(MarriageProposalReplyCmd, rev);

        if (rev.time() < now()) return true;
          getProposal().doReplyCmd(rev);
      }
      break;
    case USER2PARAM_UPLOAD_WEDDING_PHOTO:
      {
        PARSE_CMD_PROTOBUF(UploadWeddingPhotoUserCmd, rev);
        BasePackage* pack = nullptr;
        ItemWedding* item = dynamic_cast<ItemWedding*>(getPackage().getItem(rev.itemguid(), &pack));
        if (item == nullptr)
          return true;
        if (item->uploadPhoto(rev.index(), rev.time()) == false)
        {
          XERR << "[结婚证-上传照片]" << accid << id << name << "item:" << item->getGUID() << "index:" << rev.index() << "上传失败" << XEND;
          return true;
        }
        if (pack)
        {
          pack->setUpdateIDs(rev.itemguid());
          getPackage().timer(now());
          sendCmdToMe(buf, len);
        }
        XLOG << "[结婚证-上传照片]" << accid << id << name << "item:" << item->getGUID() << "index:" << rev.index() << "上传成功" << XEND;
        return true;
      }
      break;    
    case USER2PARAM_KFC_SHARE:
      {
        PARSE_CMD_PROTOBUF(KFCShareUserCmd, rev);
        if (getUserSceneData().addFirstActionDone(EFIRSTACTION_KFC_SHARE, false))
        {
          DWORD rewardid = MiscConfig::getMe().getKFCActivityCFG().getReward();
          if (rewardid)
          {
            if (!getPackage().rollReward(rewardid))
            {
              XERR << "[KFC活动分享]" << accid << id << name << "奖励:" << rewardid << "获取奖励失败" << XEND;
              return true;
            }
            XLOG << "[KFC活动分享]" << accid << id << name << "奖励:" << rewardid << "第一次分享成功" << XEND;
          }
        }
      }
      return true;
    case USER2PARAM_CHECK_RELATION:
      {
        PARSE_CMD_PROTOBUF(CheckRelationUserCmd, rev);
        bool ret = checkOtherRelation(rev.charid(), rev.etype());
        if (ret == true)
        {
          CheckRelationUserCmd cmd;
          cmd.set_charid(rev.charid());
          cmd.set_etype(rev.etype());
          cmd.set_ret(ret);
          PROTOBUF(cmd, send, len);
          sendCmdToMe(send, len);
        }
        else
        {
          xSceneEntryDynamic* pTarget = xSceneEntryDynamic::getEntryByID(rev.charid());
          if (pTarget == nullptr || pTarget->getEntryType() != SCENE_ENTRY_USER)
            MsgManager::sendMsg(id, 711);
          else
            MsgManager::sendMsg(id, 25401);
        }
        return true;
      }
      break;
    case USER2PARAM_TWINS_ACTION :
      {
        PARSE_CMD_PROTOBUF(TwinsActionUserCmd, rev);
        handleTwinsAction(rev);
        return true;
      }
      break;
    case USER2PARAM_SERVANT_SHOW:
      {
        PARSE_CMD_PROTOBUF(ShowServantUserCmd, rev);
        m_oServant.showServant(rev.show());
        return true;
      }
      break;
    case USER2PARAM_SERVANT_REPLACE:
      {
        PARSE_CMD_PROTOBUF(ReplaceServantUserCmd, rev);
        m_oServant.setServant(rev.replace(), rev.servant());
        return true;
      }
      break;
    case USER2PARAM_PROFESSION_QUERY:
      {
        PARSE_CMD_PROTOBUF(ProfessionQueryUserCmd, rev);
        m_oProfession.queryBranchs(rev);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;
    case USER2PARAM_PROFESSION_BUY:
      {
        PARSE_CMD_PROTOBUF(ProfessionBuyUserCmd, rev);

        if (!getMenu().isOpen(EMENUID_PROFESSION))
          return false;

        if(!buyProfession(rev.branch()))
          rev.set_success(false);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;
    case USER2PARAM_PROFESSION_CHANGE:
      {
        PARSE_CMD_PROTOBUF(ProfessionChangeUserCmd, rev);

        if(!changeProfession(rev.branch()))
          rev.set_success(false);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;
    case USER2PARAM_SAVE_RECORD:
      {
        PARSE_CMD_PROTOBUF(SaveRecordUserCmd, rev);

        getUserRecords().userSaveRecord(rev);

        return true;
      }
      break;
    case USER2PARAM_LOAD_RECORD:
      {
        PARSE_CMD_PROTOBUF(LoadRecordUserCmd, rev);

        getUserRecords().userLoadRecord(rev);

        return true;
      }
      break;
    case USER2PARAM_CHANGE_RECORD_NAME:
      {
        PARSE_CMD_PROTOBUF(ChangeRecordNameUserCmd, rev);

        getUserRecords().userChangeRecordName(rev);

        return true;
      }
      break;
    case USER2PARAM_SERVANT_SERVICE:
      {
        PARSE_CMD_PROTOBUF(ServantService, rev);
        m_oServant.processService(rev.type());

        return true;
      }
      break;
    case USER2PARAM_BUY_RECORD_SLOT:
      {
        PARSE_CMD_PROTOBUF(BuyRecordSlotUserCmd, rev);

        getUserRecords().userBuySlot(rev);

        return true;
      }
      break;
    case USER2PARAM_SERVANT_RECEIVE:
      {
        PARSE_CMD_PROTOBUF(ReceiveServantUserCmd, rev);
        m_oServant.getServantReward(rev.favorability(), rev.dwid());

        return true;
      }
      break;
    case USER2PARAM_DELETE_RECORD:
      {
        PARSE_CMD_PROTOBUF(DeleteRecordUserCmd, rev);

        getUserRecords().userDeleteRecord(rev);

        return true;
      }
      break;
    case USER2PARAM_INVITE_WITH_ME:
      {
        PARSE_CMD_PROTOBUF(InviteWithMeUserCmd, rev);
        if(rev.reply() == false)
          return true;
        if (rev.time() < now()) return true;

        char sign[1024];
        bzero(sign, sizeof(sign));
        std::stringstream ss;
        ss << rev.sendid() << "_" << this->id << "_" << rev.time() << "_" << "#$%^&";
        upyun_md5(ss.str().c_str(), ss.str().size(), sign);

        if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
          return true;
        SceneUser* pTarget = SceneUserManager::getMe().getUserByID(rev.sendid());
        if(pTarget == nullptr || pTarget->getScene() == nullptr || getScene() == nullptr)
          return true;

        if(isAlive() == false)
        {
          MsgManager::sendMsg(id, 25525);
          return true;
        }

        if(getTeamID() != pTarget->getTeamID())
        {
          MsgManager::sendMsg(id, 25525);
          return true;
        }

        if(getScene()->isPVPScene() == false && getScene()->isGvg() == false)
        {
          MsgManager::sendMsg(id, 25525);
          MsgManager::sendMsg(pTarget->id, 25524);
          return true;
        }

        DWORD mapid = pTarget->getScene()->getMapID();
        xPos pos = pTarget->getPos();
        if(mapid == getScene()->getMapID())
        {
          goTo(pos, false);

          // 播放特效音效
          const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();

          xLuaData data;
          data.setData("type", "effect");
          data.setData("effect", configPath.strEnterSceneEffect);
          data.setData("posbind", 1);
          GMCommandRuler::getMe().execute(this, data);

          xLuaData sound;
          sound.setData("type", "sound_effect");
          sound.setData("se", configPath.strEnterSceneSound);
          sound.setData("sync", 1);
          GMCommandRuler::getMe().execute(this, sound);
        }
        else
          gomap(mapid, GoMapType::GM, pos);
        return true;
      }
      break;
    case USER2PARAM_QUERY_ALTMAN_KILL:
      {
        DWORD dwKill = getVar().getVarValue(EVARTYPE_ALTMAN_KILL);
        const string title = MiscConfig::getMe().getAltmanCFG().getTitleByKill(dwKill);
        if (title != STRING_EMPTY)
          MsgManager::sendMsg(id, 5203, MsgParams(title));
        else
          MsgManager::sendMsg(id, 5204);

        return true;
      }
      break;
    case USER2PARAM_USER_BOOTH_REQ:
      {
        PARSE_CMD_PROTOBUF(BoothReqUserCmd, rev);

        if(!m_oBooth.req(rev))
          rev.set_success(false);
        else
          rev.set_success(true);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;
    case USER2PARAM_QUERY_STAGE:
      {
        PARSE_CMD_PROTOBUF(QueryStageUserCmd, rev);
        DressUpStageMgr::getMe().sendStageInfo(this, rev.stageid());
        return true;
      }
      break;
    case USER2PARAM_DRESSUP_LINEUP:
      {
        PARSE_CMD_PROTOBUF(DressUpLineUpUserCmd, rev);
        if(rev.enter())
          DressUpStageMgr::getMe().requestAddLine(this, rev.stageid(), rev.mode());
        else
          DressUpStageMgr::getMe().leaveDressStage(this);
        return true;
      }
      break;
    case USER2PARAM_DRESSUP_MODEL:
      {
        PARSE_CMD_PROTOBUF(DressUpModelUserCmd, rev);
        DressUpStageMgr::getMe().changeStageAppearance(this, rev.stageid(), rev.type(), rev.value(), nullptr);
        return true;
      }
      break;
    case USER2PARAM_DRESSUP_HEAD:
      {
        PARSE_CMD_PROTOBUF(DressUpHeadUserCmd, rev);
        if(rev.type() == EUSERDATATYPE_CLOTHCOLOR)
        {
          if(rev.puton() == true)
            m_oUserSceneData.setClothColor(rev.value(), false);
          else
            m_oUserSceneData.setClothColor(0, false);
        }
        else if(rev.type() == EUSERDATATYPE_HAIR)
        {
          if(rev.puton() == true)
            m_oHair.useHair(rev.value(), false);
          else
            m_oHair.useHair(0, false);
        }
        else if(rev.type() == EUSERDATATYPE_HAIRCOLOR)
        {
          if(rev.puton() == true)
            m_oHair.useColor(rev.value(), false);
          else
            m_oHair.useColor(0, false);
        }
        else if(rev.type() == EUSERDATATYPE_EYE)
        {
          if(rev.puton() == true)
            m_oEye.useEye(rev.value(), false);
          else
            m_oEye.useEye(0, false);
        }

        return true;
      }
      break;
    case USER2PARAM_SERVANT_RECEIVE_GROWTH:
      {
        PARSE_CMD_PROTOBUF(ReceiveGrowthServantUserCmd, rev);
        m_oServant.getGrowthReward(rev.dwid(), rev.dwvalue());
        return true;
      }
      break;
    case USER2PARAM_SERVANT_GROWTH_OPEN:
      {
        PARSE_CMD_PROTOBUF(GrowthOpenServantUserCmd, rev);
        m_oServant.openNewGrowth(rev.groupid());
        return true;
      }
      break;
    case USER2PARAM_GOTO_FUNCMAP:
      {
        PARSE_CMD_PROTOBUF(GoToFunctionMapUserCmd, rev);
        switch(rev.etype())
        {
          case EFUNCMAPTYPE_POLLY:
            {
              const SPoliFireCFG& rCFG = MiscConfig::getMe().getPoliFireCFG();
              DWORD npcid = rCFG.dwTransportNpcID;
              DWORD mapid = rCFG.dwActivityMap;
              if (!npcid || !mapid || !getScene())
                return true;
              xSceneEntrySet npcset;
              getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, getPos(), 10, npcset);
              bool checkNpc = false;
              for (auto &s : npcset)
              {
                SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
                if (npc && npc->getNpcID() == npcid)
                {
                  checkNpc = true;
                  break;
                }
              }
              if (!checkNpc)
              {
                XERR << "[波利乱斗-传送], 传送失败, 玩家距离npc太远, 玩家:" << name << id << "地图:" << getScene()->getMapID() << XEND;
                return true;
              }
              gomap(mapid, GoMapType::GM);
              XDBG << "[波利乱斗-传送], 传送成功, 玩家" << name << id << "传送至地图:" << mapid << XEND;
              return true;
            }
            break;
          default:
            break;
        }
      };
      break;
    case USER2PARAM_CHEAT_TAG:
      {
        PARSE_CMD_PROTOBUF(CheatTagUserCmd, rev);
        m_oCheatTag.recCheatTag(rev);
        return true;
      }
    default:
      break;
  }

  return false;
}

bool SceneUser::doSceneUserItemCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    case ITEMPARAM_PACKAGEITEM:
      {
        PARSE_CMD_PROTOBUF(PackageItem, rev);
        if (!CommonConfig::m_bPackSyncNew)
        {
          if (m_oPackage.toClient(rev.type(), rev) == false)
          {
            XERR << "[包裹-数据]" << accid << id << getProfession() << name << "请求了 type :" << rev.type() << "数据失败,异常" << XEND;
            return true;
          }
          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
          XDBG << "[包裹-数据]" << accid << id << getProfession() << name << "请求了 type :" << rev.type() << "数据" << rev.data_size() << "个道具 最大上限 :" << rev.maxslot() << XEND;
        }
        else
        {
          getPackage().sendPackData(rev.type());
        }
        return true;
      }
      break;
      //case ITEMPARAM_PACKAGEUPDATE:
      //  {
      //    PARSE_CMD_PROTOBUF(PackageItem, rev);
      //  }
      //  break;
    case ITEMPARAM_ITEMUSE:
      {
        PARSE_CMD_PROTOBUF(ItemUse, rev);
        useItem(rev);
        return true;
      }
      break;
    case ITEMPARAM_PACKAGESORT:
      {
        PARSE_CMD_PROTOBUF(PackageSort, rev);
        BasePackage* pPack = m_oPackage.getPackage(rev.type());
        if (pPack != NULL)
          pPack->packageSort(this);
      }
      break;
    case ITEMPARAM_EQUIP:
      {
        PARSE_CMD_PROTOBUF(Equip, rev);
        m_oPackage.equip(rev.oper(), rev.pos(), rev.guid(), rev.transfer(), true, rev.count());
      }
      break;
    case ITEMPARAM_SELLITEM:
      {
        PARSE_CMD_PROTOBUF(SellItem, rev);
        m_oPackage.sellItem(rev);
      }
      break;
    case ITEMPARAM_EQUIPSTRENGTH:
      {
        PARSE_CMD_PROTOBUF(EquipStrength, rev);
        m_oPackage.strength(rev.type(), rev.guid(), rev.destcount());
      }
      break;
    case ITEMPARAM_PRODUCE:
      {
        PARSE_CMD_PROTOBUF(Produce, rev);
        for (DWORD i = 0; i < rev.count(); ++i)
          if (m_oPackage.produce(rev.composeid(), rev.npcid(), rev.type(), rev.qucikproduce()) == false)
            break;
      }
      break;
    case ITEMPARAM_REFINE:
      {
        PARSE_CMD_PROTOBUF(EquipRefine, rev);
        //m_oPackage.refine(rev.guid(), rev.composeid(), rev.npcid(), rev.saferefine());
        m_oPackage.refine(rev);
      }
      break;
    case ITEMPARAM_DECOMPOSE:
      {
        PARSE_CMD_PROTOBUF(EquipDecompose, rev);
        m_oPackage.Decompose(rev.guid());
      }
      break;
    case ITEMPARAM_QUERYDECOMPOSERESULT:
      {
        PARSE_CMD_PROTOBUF(QueryDecomposeResultItemCmd, rev);
        m_oPackage.queryDecomposeResult(rev.guid());
      }
      break;
    case ITEMPARAM_QUERYEQUIPDATA:
      {
        PARSE_CMD_PROTOBUF(QueryEquipData, rev);
        m_oPackage.queryEquipData(rev.guid());
      }
      break;
    case ITEMPARAM_BROWSEPACK:
      {
        PARSE_CMD_PROTOBUF(BrowsePackage, rev);
        m_oPackage.browsePackage(rev.type());
      }
      break;
    case ITEMPARAM_EQUIPCARD:
      {
        PARSE_CMD_PROTOBUF(EquipCard, rev);
        m_oPackage.equipcard(rev.oper(), rev.cardguid(), rev.equipguid(), rev.pos());
      }
      break;
    case ITEMPARAM_REPAIR:
      {
        PARSE_CMD_PROTOBUF(EquipRepair, rev);
        m_oPackage.repair(rev.targetguid(), rev.stuffguid());
      }
      break;
    case ITEMPARAM_HINTNTF:
      {
        PARSE_CMD_PROTOBUF(HintNtf, rev);
        m_oPackage.saveHint(rev.itemid());
      }
      break;
    case ITEMPARAM_ENCHANT:
      {
        PARSE_CMD_PROTOBUF(EnchantEquip, rev);
        m_oPackage.enchant(rev.type(), rev.guid());
      }
      break;
    case ITEMPARAM_PROCESSENCHANT:
      {
        PARSE_CMD_PROTOBUF(ProcessEnchantItemCmd, rev);
        m_oPackage.processEnchant(rev.save(), rev.itemid());
      }
      break;
    case ITEMPARAM_EQUIPEXCHANGE:
      {
        PARSE_CMD_PROTOBUF(EquipExchangeItemCmd, rev);
        m_oPackage.setOperItem(rev.guid());
        if (rev.type() == EEXCHANGETYPE_LEVELUP)
          m_oPackage.upgrade(rev.guid());
        else if (rev.type() == EEXCHANGETYPE_EXCHANGE)
          m_oPackage.exchange(rev.guid());
        m_oPackage.setOperItem("");
      }
      break;
    case ITEMPARAM_ONOFFSTORE:
      {
        PARSE_CMD_PROTOBUF(OnOffStoreItemCmd, rev);
        m_oPackage.setStoreStatus(rev.open());
      }
      break;
    case ITEMPARAM_RESTOREEQUIP:
      {
        PARSE_CMD_PROTOBUF(RestoreEquipItemCmd, rev);
        m_oPackage.restore(rev);
      }
      break;
    case ITEMPARAM_USECOUNT:
      {
        PARSE_CMD_PROTOBUF(UseCountItemCmd, rev);
        m_oPackage.sendVarUseCnt(rev.itemid());
      }
      break;
    case ITEMPARAM_EXCHANGECARD:
      {
        PARSE_CMD_PROTOBUF(ExchangeCardItemCmd, rev);
        m_oPackage.exchangeCard(rev);
      }
      break;
    case ITEMPARAM_GETCOUNT:
      {
        PARSE_CMD_PROTOBUF(GetCountItemCmd, rev);
        m_oPackage.sendVarGetCnt(rev.itemid(), rev.source());
      }
      break;
    case ITEMPARAM_SAVE_LOVE_LETTER:
      {
        PARSE_CMD_PROTOBUF(SaveLoveLetterCmd, rev);
        addLoveLetterItem(rev.dwid());
      }
      break;
    case ITEMPARAM_QUERY_LOTTERYINFO:
      {
        PARSE_CMD_PROTOBUF(QueryLotteryInfo, rev);     
        getLottery().sendLotteryCfg(rev.type());
        ActivityEventManager::getMe().sendActivityEventCount(this, EACTIVITYEVENTTYPE_LOTTERY_DISCOUNT);
      }
      break;
    case ITEMPARAM_LOTTERY:
      {
        PARSE_CMD_PROTOBUF(LotteryCmd, rev);
        m_oLottery.lottery(rev);
      }
      break;
    case ITEMPARAM_LOTTERY_RECOVERY:
      {
        PARSE_CMD_PROTOBUF(LotteryRecoveryCmd, rev);
        m_oLottery.lotteryRecovery(rev);
      }
      break;
    case ITEMPARAM_REQ_QUOTA_LOG:
      {
        PARSE_CMD_PROTOBUF(ReqQuotaLogCmd, rev);
        m_oDeposit.reqQuotaLog(rev);
      }
      break;
    case ITEMPARAM_REQ_QUOTA_DETAIL:
      {
        PARSE_CMD_PROTOBUF(ReqQuotaDetailCmd, rev);
        m_oDeposit.reqQuotaDetail(rev);
      }
      break;
    case ITEMPARAM_HIGHREFINE_MATCOMPOSE:
      {
        PARSE_CMD_PROTOBUF(HighRefineMatComposeCmd, rev);
        m_oHighRefine.matCompose(rev);
      }
      break;
    case ITEMPARAM_HIGHREFINE:
      {
        PARSE_CMD_PROTOBUF(HighRefineCmd, rev);
        m_oHighRefine.highRefine(rev);
      }
      break;
    case ITEMPARAM_USE_CODE_ITEM:
      {
        PARSE_CMD_PROTOBUF(UseCodItemCmd, rev);
        m_oPackage.useItemCode(rev);
      }
      break;
    case ITEMPARAM_ADD_JOBLEVEL:
      {
        PARSE_CMD_PROTOBUF(AddJobLevelItemCmd, rev);
        m_pCurFighter->reqAddMaxJobCmd(rev);
      }
      break;
    case ITEMPARAM_GIVE_WEDDING_DRESS:
    {
      PARSE_CMD_PROTOBUF(GiveWeddingDressCmd, rev);
      m_oPackage.giveWeddingDress(rev);
    }
    break;
    case ITEMPARAM_QUICK_STOREITEM:
    {
      PARSE_CMD_PROTOBUF(QuickStoreItemCmd, rev);
      m_oPackage.quickStoreItem(rev);
    }
    break;
    case ITEMPARAM_QUICK_SELLITEM:
    {
      PARSE_CMD_PROTOBUF(QuickSellItemCmd, rev);
      m_oPackage.quickSellItem(rev);
    }
    break;
    case ITEMPARAM_ENCHANT_TRANS:
      {
        PARSE_CMD_PROTOBUF(EnchantTransItemCmd, rev);
        m_oPackage.enchantTrans(rev);
      }
      break;
    case ITEMPARAM_QUERY_LOTTERYHEAD:
      {
        const TSetDWORD& setIDs = ItemConfig::getMe().getAllLotteryHead();
        QueryLotteryHeadItemCmd cmd;
        for (auto &s : setIDs)
          cmd.add_ids(s);
        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case ITEMPARAM_LOTTERY_RATE_QUERY:
    {
      PARSE_CMD_PROTOBUF(LotteryRateQueryCmd, rev);

      if(ELotteryType_Card == rev.type())
      {
        ItemConfig::getMe().fillLotteryCardRate(rev);
      }
      else if(ELotteryType_Magic == rev.type())
      {
        MiscConfig::getMe().getLotteryCFG().fillLotteryMagicRate(rev);
      }

      PROTOBUF(rev, send, len);
      sendCmdToMe(send, len);
    }
    break;
    case ITEMPARAM_EQUIPCOMPOSE:
    {
      PARSE_CMD_PROTOBUF(EquipComposeItemCmd, rev);
      EError eError = getPackage().equipCompose(rev);

      if (eError != EERROR_SUCCESS)
        rev.set_retmsg(26111);
      PROTOBUF(rev, send, len);
      sendCmdToMe(send, len);
    }
    break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserSkillCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf || !len) return false;

  switch (buf->param)
  {
    /*case SKILLPARAM_SKILLITEM:
      {
        PARSE_CMD_PROTOBUF(ReqSkillData, rev);
        m_pCurFighter->getSkill().toClient(rev);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);

        m_pCurFighter->getSkill().sendSpecSkillInfo();
        return true;
      }
      break;*/
    //case SKILLPARAM_SKILLUPDATE:
    //  break;
    //case SKILLPARAM_ACTIVESKILL:
    //  {
    //    PARSE_CMD_PROTOBUF(ActiveSkill, rev);
    //    m_oSkillMgr.activeSkill(rev.skillid);
    //  }
    //  break;
    case SKILLPARAM_LEVELUPSKILL:
      {
        PARSE_CMD_PROTOBUF(LevelupSkill, rev);
        m_pCurFighter->getSkill().levelupSkill(rev);
        //m_pCurFighter->getSkill().levelupSkill(rev.type(), rev.skillid());
        return true;
      }
      break;
    case SKILLPARAM_EQUIPSKILL:
      {
        PARSE_CMD_PROTOBUF(EquipSkill, rev);
        if (rev.beingid() != 0)
          m_oUserBeing.equipSkill(rev);
        else
          m_pCurFighter->getSkill().equipSkill(rev);
        return true;
      }
      break;
    case SKILLPARAM_RESETSKILL:
      break;
    /*case SKILLPARAM_SKILLVALIDPOS:
      {
        PARSE_CMD_PROTOBUF(SkillValidPos, rev);
        m_pCurFighter->getSkill().toClient(rev);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;*/
    case SKILLPARAM_SELECT_RUNE:
      {
        PARSE_CMD_PROTOBUF(SelectRuneSkillCmd, rev);
        if (rev.beingid() == 0)
        {
          m_pCurFighter->getSkill().switchRune(rev.skillid(), rev.selectswitch());
          m_pCurFighter->getSkill().selectRuneSpecID(rev.skillid() / ONE_THOUSAND, rev.runespecid());
        }
        else
        {
          m_oUserBeing.switchRune(rev.beingid(), rev.skillid(), rev.selectswitch());
          m_oUserBeing.selectRuneSpecID(rev.beingid(), rev.skillid() / ONE_THOUSAND, rev.runespecid());
        }
        return true;
      }
      break;
    case SKILLPARAM_TRGGER_SKILLNPC:
      {
        PARSE_CMD_PROTOBUF(TriggerSkillNpcSkillCmd, rev);
        SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(rev.npcguid());
        SkillNpc* pSkillNpc = dynamic_cast<SkillNpc*>(npc);
        if (pSkillNpc)
          pSkillNpc->onTriggered(this);

        if (rev.etype() == ETRIGTSKILL_BTRANS) // 通知前端, 解除移动屏蔽
          sendCmdToMe(buf, len);

        return true;
      }
      break;
    case SKILLPARAM_SKILLOPTIONS:
      {
        PARSE_CMD_PROTOBUF(SkillOptionSkillCmd, rev);
        m_oUserSceneData.setSkillOpt(rev.set_opt().opt(), rev.set_opt().value());
        return true;
      }
      break;
    case SKILLPARAM_SYNC_DEST_POS:
      {
        PARSE_CMD_PROTOBUF(SyncDestPosSkillCmd, rev);
        m_oSkillProcessor.checkSkillMove(rev.skillid(), rev.pos());
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool SceneUser::doSceneUserQuestCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    /*case QUESTPARAM_NPCQUESTITEM:
      {
        PARSE_CMD_PROTOBUF(GetNpcQuestItem, rev);
        m_oQuest.collectNpcQuestItem(rev);

        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      break;*/
    case QUESTPARAM_VISIT_NPC:
      {
        //if (m_oHands.isFollower()) return true;
        PARSE_CMD_PROTOBUF(VisitNpcUserCmd, rev);
        SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(rev.npctempid());
        if (npc)
        {
          m_event.onVisitNpc(npc);
          setVisitNpc(npc->id);
        }
        return true;
      }
      break;
    case QUESTPARAM_QUESTLIST:
      {
        PARSE_CMD_PROTOBUF(QuestList, rev);
        if (rev.type() == EQUESTLIST_CANACCEPT)
          m_oQuest.sendWantedQuestList(rev.id());
        return true;
      }
      break;
    case QUESTPARAM_QUESTUPDATE:
      {
        return true;
      }
      break;
    case QUESTPARAM_QUESTACTION:
      {
        PARSE_CMD_PROTOBUF(QuestAction, rev);
        // 看板队长使用道具快速完成看板任务后, 需要等待队友一同完成看板任务(一段cd时间)
        // cd结束前队长不可继续使用快速完成道具, 或接取完成看板任务
        if (rev.action() == EQUESTACTION_ACCEPT || rev.action() == EQUESTACTION_SUBMIT || rev.action() == EQUESTACTION_QUICK_SUBMIT_BOARD || rev.action() == EQUESTACTION_QUICK_SUBMIT_BOARD_TEAM)
        {
          const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(rev.questid());
          if (pCFG && pCFG->eType == EQUESTTYPE_WANTED && m_oQuest.isTeamFinishBoardQuestCD())
          {
            MsgManager::sendMsg(id, 25441);
            return true;
          }
        }
        m_oQuest.questAction(rev.action(), rev.questid());
        return true;
      }
      break;
    case QUESTPARAM_RUNQUESTSTEP:
      {
        PARSE_CMD_PROTOBUF(RunQuestStep, rev);
        //if (m_oHands.isFollower()) return true;
        //if (scene && scene->isDScene())
        //  ((QuestScene *)scene)->m_fuben.check("visit", rev.starid());
        //else
        //XLOG("client id = %u, group=%u, step=%u",rev.questid(), rev.subgroup(),rev.step());

        // special accept quest here to client show quest mark
        if (m_oQuest.acceptGuildQuest(rev.questid()) == false)
          return true;
        if (m_oQuest.checkStep(rev.questid(), rev.step()) == false)
          return true;
        m_oQuest.runStep(rev.questid(), rev.subgroup(), rev.step());
        return true;
      }
      break;
    case QUESTPARAM_QUESTTRACE:
      {
        PARSE_CMD_PROTOBUF(QuestTrace, rev);
        m_oQuest.setTrace(rev.questid(), rev.trace());
        return true;
      }
      break;
    case QUESTPARAM_QUESTRAIDCMD:
      {
        PARSE_CMD_PROTOBUF(QuestRaidCmd, rev);
        m_oQuest.onRaidCmd(rev.questid());
        return true;
      }
      break;
    case QUESTPARAM_QUERYOTHERDATA:
      {
        PARSE_CMD_PROTOBUF(QueryOtherData, rev);
        if (rev.type() == EOTHERDATA_DAILY)
          m_oQuest.queryOtherData(rev.type());
        else if (rev.type() == EOTHERDATA_CAT)
          m_oWeaponPet.queryCatPrice(rev);
        return true;
      }
      break;
    case QUESTPARAM_HELP_ACCEPT_INVITE:
      {
        if (!isWantedQuestLeader())
        {
          XERR << "[组队看板], 非队长或者没有学习冒险技能, 不可帮助接取看板任务, 玩家:" << name << this->id << XEND;
          return true;
        }
        PARSE_CMD_PROTOBUF(InviteHelpAcceptQuestCmd, rev);
        // 自己接取
        m_oQuest.questAction(EQUESTACTION_ACCEPT, rev.questid());
        // 邀请队友
        InviteAcceptQuestCmd cmd;
        cmd.set_leaderid(this->id);
        cmd.set_questid(rev.questid());
        cmd.set_leadername(name);

        DWORD time = now() + 300;
        cmd.set_time(time);
        char sign[1024];
        bzero(sign, sizeof(sign));
        std::stringstream ss;
        ss << this->id << "@" << rev.questid() << "@" << time << "_" << "#$%^&";
        upyun_md5(ss.str().c_str(), ss.str().size(), sign);
        cmd.set_sign(sign);
        PROTOBUF(cmd, send, len);
        for (auto &m : m_oGTeam.getTeamMemberList())
        {
          if (m.first == this->id)
            continue;
          thisServer->forwardCmdToSceneUser(m.first, send, len);
        }
        return true;
      };
      break;
    case QUESTPARAM_INVITE_ACCEPT_QUEST:
      {
        if (getTeamID() == 0)
          return true;
        PARSE_CMD_PROTOBUF(InviteAcceptQuestCmd, rev);
        if (rev.time() < now())
          return true;

        char sign[1024];
        bzero(sign, sizeof(sign));
        std::stringstream ss;
        ss << rev.leaderid() << "@" << rev.questid() << "@" << rev.time() << "_" << "#$%^&";
        upyun_md5(ss.str().c_str(), ss.str().size(), sign);
        if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
          return true;

        // 未跟随队长不可接取
        if (m_oUserSceneData.getFollowerID() == 0 || m_oUserSceneData.getFollowerID() != rev.leaderid())
          return true;

        if (rev.isquickfinish()) // 快速完成看板
        {
          ItemInfo oItem;
          if (m_oQuest.canQuickFinishBoard(rev.questid(), oItem) == false) // 不可以快速完成看板则不做处理
            return true;
          m_oQuest.setQuickFinishBoardID(rev.questid());
          HelpQuickFinishBoardQuestCmd ntf;
          ntf.set_questid(rev.questid());
          ntf.set_leadername(rev.leadername());
          PROTOBUF(ntf, send1, len1);
          sendCmdToMe(send1, len1);

          // 设置队长的操作cd
          WantedQuestSetCDSessionCmd cmd;
          cmd.set_charid(rev.leaderid());
          cmd.set_time(now());
          PROTOBUF(cmd, send2, len2);
          SceneUser* leader = SceneUserManager::getMe().getUserByID(rev.leaderid());
          if (leader)
            thisServer->doSessionCmd((const BYTE*)send2, len2);
          else
            thisServer->sendCmdToSession(send2, len2);
          return true;
        }

        if (rev.issubmit() == false) // 接取任务
        {
          pair<DWORD, DWORD> curwanted;
          if (m_oQuest.getWantedQuest(curwanted)) // 玩家当前已接取看板任务
          {
            if (curwanted.first == rev.questid()) // 任务已接取
              return true;

            const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(curwanted.first);
            if (pCFG == nullptr || pCFG->eType != EQUESTTYPE_WANTED)
              return true;
            bool replace = true;
            for (DWORD i = 0; i < pCFG->vecSteps.size(); ++i)
              if (pCFG->vecSteps[i]->getStepType() == EQUESTSTEP_REWRADHELP && curwanted.second > i)
              {
                replace = false;
                break;
              }
            if (replace == false)
            {
              // 当前已接看板任务不是队长接取任务, 并且任务包含RewardHelp步骤, 且该步骤已完成, 则不可取消该任务, 而是直接帮玩家自动提交
              bool ret = m_oQuest.finishBoardQuest(curwanted.first, true); // 自动提交任务
              if (ret == true)
                m_oQuest.questAction(EQUESTACTION_ACCEPT, rev.questid(), true); // 自动接取
              return true;
            }
            else
            {
              // 当前已接看板任务不是队长接取任务, 询问玩家是否替换
              InviteHelpAcceptQuestCmd cmd;
              cmd.set_leaderid(rev.leaderid());
              cmd.set_questid(rev.questid());
              cmd.set_time(rev.time());
              cmd.set_leadername(rev.leadername());
              cmd.set_issubmit(rev.issubmit());

              bzero(sign, sizeof(sign));
              ss.str("");
              ss << rev.leaderid() << "_" << rev.questid() << "_" << rev.time() << "_" << "#$%^&" << rev.issubmit();
              upyun_md5(ss.str().c_str(), ss.str().size(), sign);
              cmd.set_sign(sign);

              PROTOBUF(cmd, send, len);
              sendCmdToMe(send, len);
              return true;
            }
          }
          else // 玩家当前未接取任何看板任务
          {
            m_oQuest.questAction(EQUESTACTION_ACCEPT, rev.questid(), true); // 自动接取
            return true;
          }
        }
        else // 交付任务
        {
          // 不询问玩家, 直接交付, 且不论玩家当前看板任务是否是队长接取任务, 都尝试交付
          bool ret = m_oQuest.finishBoardQuest(rev.questid(), false);
          if (ret == false)
          {
            pair<DWORD, DWORD> curwanted;
            if (m_oQuest.getWantedQuest(curwanted))
              ret = m_oQuest.finishBoardQuest(curwanted.first, false);
          }
          if (ret == true)
          {
            WantedQuestFinishCmd cmd;
            cmd.set_leaderid(rev.leaderid());
            cmd.set_teammateid(this->id);
            PROTOBUF(cmd, send, len);

            SceneUser* pLeader = SceneUserManager::getMe().getUserByID(rev.leaderid());
            if (pLeader != nullptr)
              thisServer->doSessionCmd((const BYTE*)send, len);
            else
              thisServer->sendCmdToSession(send, len);
          }
          return true;
        }

        return true;
      }
      break;
    case QUESTPARAM_HELP_ACCEPT_AGREE:
      {
        PARSE_CMD_PROTOBUF(ReplyHelpAccelpQuestCmd, rev);
        if (rev.time() < now())
          return true;
        std::stringstream ss;
        char sign[1024];
        bzero(sign, sizeof(sign));
        ss << rev.leaderid() << "_" << rev.questid() << "_" << rev.time() << "_" << "#$%^&" << rev.issubmit();
        upyun_md5(ss.str().c_str(), ss.str().size(), sign);
        if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
          return true;

        if (!rev.agree())
          return true;

        pair<DWORD, DWORD> curwanted;
        if (m_oQuest.getWantedQuest(curwanted)) // 当前已接看板任务
        {
          if (curwanted.first == rev.questid())
            return true;
          // 已接任务不是队长接取的任务
          m_oQuest.abandonGroup(curwanted.first);
        }
        m_oQuest.questAction(EQUESTACTION_ACCEPT, rev.questid(), true);

        return true;
      };
      break;
    case QUESTPARAM_QUERY_WORLD_QUEST:
      m_oQuest.queryWorldQuest();
      break;
    case QUESTPARAM_QUESTGROUP_TRACE:
      {
        PARSE_CMD_PROTOBUF(QuestGroupTraceQuestCmd, rev);
        m_oQuest.setGroupTrace(rev.id(), rev.trace());
      }
      break;
    case QUESTPARAM_QUERY_MANUAL:
      {
        PARSE_CMD_PROTOBUF(QueryManualQuestCmd, rev);
        m_oQuest.queryManualData(rev.version());
      }
      break;
    case QUESTPARAM_OPEN_PUZZLE:
      {
        PARSE_CMD_PROTOBUF(OpenPuzzleQuestCmd, rev);
        m_oQuest.openPuzzle(rev.version(), rev.id());
      }
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserMapCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    case MAPPARAM_PICKUPITEM:
      {
        if (MiscConfig::getMe().getItemCFG().ePickupMode == EPICKUPMODE_SERVER)
        {
          //XERR << "[玩家-捡取]" << accid << id << getProfession() << name << "捡取失败,当前为服务器捡取模式" << XEND;
          return true;
        }

        PARSE_CMD_PROTOBUF(PickupItem, rev);
        SceneItemManager::getMe().pickupSceneItem(this, rev.itemguid());
      }
      break;
    case MAPPARAM_GO_CITYGATE:
      {
        PARSE_CMD_PROTOBUF(GoCityGateMapCmd, rev);
        const SGuildCityCFG* pCFG = GuildRaidConfig::getMe().getGuildCityCFG(rev.flag());
        if (pCFG == nullptr)
        {
          XERR << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "失败,未在 Table_Guild_StrongHold.txt 表中找到" << XEND;
          break;
        }

        const SceneBase* pNowBase = getScene() ? getScene()->getSceneBase() : nullptr;
        if (pNowBase)
        {
          const SFlag* pFlag = pNowBase->getFlag(rev.flag());
          if (pFlag == nullptr || pFlag->isValidPos(getPos()) == false)
          {
            XERR << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "失败,未找到" << pCFG->dwTeleMapID << "地图配置" << XEND;
            break;
          }
        }

        const SceneBase* pBase = SceneManager::getMe().getDataByID(pCFG->dwTeleMapID);
        if (pBase == nullptr)
        {
          XERR << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "失败,未找到" << pCFG->dwTeleMapID << "地图配置" << XEND;
          break;
        }
        const SceneObject* pObj = pBase->getSceneObject(0);
        if (pObj == nullptr)
        {
          XERR << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "失败,未找到" << pCFG->dwTeleMapID << "地图obj" << XEND;
          break;
        }
        const xPos* pPos = pObj->getBornPoint(pCFG->dwTeleBpID);
        if (pPos == nullptr)
        {
          XERR << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "失败,未找到地图" << pCFG->dwTeleMapID << pCFG->dwTeleBpID << "出生点" << XEND;
          break;
        }
        gomap(pCFG->dwTeleMapID, GoMapType::GM, *pPos);
        XLOG << "[玩家-城池传送]" << accid << id << getProfession() << name << "传送至" << rev.flag() << "成功,地图" << pCFG->dwTeleMapID << pCFG->dwTeleBpID << XEND;
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserCarrierCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    case MAPPARAM_START_FERRISWHEEL:
      {
        /*PARSE_CMD_PROTOBUF(StartFerrisWheelUserCmd, rev);

        getUserSceneData().setFollowerID(0);

        CreateDMapParams params;
        params.dwRaidID = 20003;
        params.eRestrict = ERAIDRESTRICT_SYSTEM;//RaidMapRestrict::System;
        params.vecMembers.push_back(id);
        params.vecMembers.push_back(rev.charid());
        SceneManager::getMe().createDScene(params);*/
        /*std::list<QWORD> list;
        list.push_back(id);
        list.push_back(rev.charid());
        thisServer->createRaid(20003, list);*/

        return true;
      }
      break;
    /*case MAPPARAM_FERRISWHEEL:
      {
        PARSE_CMD_PROTOBUF(FerrisWheelUserCmd, rev);

        if (rev.ret() == 0)
        {
          char sign[1024];
          bzero(sign, sizeof(sign));
          std::stringstream ss;
          ss << rev.charid() << rev.time() << "_" << "#$%^&";
          upyun_md5(ss.str().c_str(), ss.str().size(), sign);

          if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
            return true;

          getUserSceneData().setFollowerID(0);

          Cmd::StartFerrisWheelUserCmd message;
          message.set_masterid(rev.charid());
          message.set_charid(id);
          PROTOBUF(message, send, len);
          thisServer->forwardCmdToSceneUser(rev.charid(), send, len);
        }
        else
        {
          MsgManager::sendMsg(rev.charid(), rev.ret());
        }

        return true;
      }
      break;*/
    case MAPPARAM_REACH_CARRIER:
      {
        PARSE_CMD_PROTOBUF(ReachCarrierUserCmd, rev);

        rev.clear_pos();
        m_oCarrier.reach(rev);

        return true;
      }
      break;
    case MAPPARAM_LEAVE_CARRIER:
      {
        m_oCarrier.leave();

        return true;
      }
      break;
    case MAPPARAM_INVITE_CARRIER:
      {
        PARSE_CMD_PROTOBUF(InviteCarrierUserCmd, rev);

        SceneUser* pTarget = SceneUserManager::getMe().getUserByID(rev.masterid());
        if (pTarget != nullptr &&
            (pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK) == true || pTarget->getSocial().checkRelation(id, ESOCIALRELATION_BLACK_FOREVER) == true))
        {
          XERR << "[玩家-载具消息]" << accid << id << getProfession() << name << "邀请 charid :" << rev.masterid() << "失败,黑名单" << XEND;
          break;
        }

        if (m_oHands.getOtherID() == rev.masterid())
        {
          Cmd::RetJoinCarrierUserCmd cmd;
          cmd.set_agree(true);
          cmd.set_membername(name);
          cmd.set_memberid(id);
          cmd.set_masterid(rev.masterid());
          PROTOBUF(cmd, send1, len1);
          thisServer->forwardCmdToSceneUser(rev.masterid(), send1, len1);
        }
        else
        {
          Cmd::JoinCarrierUserCmd message;
          message.set_masterid(rev.masterid());
          message.set_mastername(rev.mastername());
          message.set_carrierid(rev.carrierid());
          PROTOBUF(message, send, len);
          sendCmdToMe(send, len);
        }

        m_oCarrier.m_oBeInvites.insert(rev.masterid());
        /*
        if (m_oCarrier.has())
        {
          message.set_agree(false);
          PROTOBUF(message, send, len);
          thisServer->forwardCmdToSceneUser(rev.masterid(), send, len);
          return true;
        }
        else
        {
          m_oCarrier.m_oBeInvites.insert(rev.masterid());

          PROTOBUF(message, send, len);
          sendCmdToMe(send, len);
        }
        */

        return true;
      }
      break;
    case MAPPARAM_JOIN_CARRIER:
      {
        // 队员返回是否接受
        PARSE_CMD_PROTOBUF(JoinCarrierUserCmd, rev);

        Cmd::RetJoinCarrierUserCmd message;
        message.set_agree(rev.agree());
        message.set_membername(name);
        message.set_memberid(id);
        message.set_masterid(rev.masterid());
        PROTOBUF(message, send, len);
        thisServer->forwardCmdToSceneUser(rev.masterid(), send, len);

        //m_oHands.breakup();
        if (rev.agree() && m_oHands.has() && m_oHands.getOtherID() != rev.masterid())
        {
          message.set_agree(rev.agree());
          message.set_membername(getTeamerName(m_oHands.getOtherID()));
          message.set_memberid(m_oHands.getOtherID());
          message.set_masterid(rev.masterid());
          PROTOBUF(message, send, len);
          thisServer->forwardCmdToSceneUser(rev.masterid(), send, len);
        }
        return true;
      }
      break;
    case MAPPARAM_RET_JOIN_CARRIER:
      {
        // master 处理
        PARSE_CMD_PROTOBUF(RetJoinCarrierUserCmd, rev);

        if (rev.agree())   // 同意
        {
          if (!m_oCarrier.join(rev.memberid()))
          {
            rev.set_agree(false);
            m_oCarrier.m_oInvites.erase(rev.memberid());
          }
        }
        else  // 拒绝
        {
          m_oCarrier.m_oInvites.erase(rev.memberid());
        }

        PROTOBUF(rev, send, len);
        m_oCarrier.sendCmdAll(send, len);

        return true;
      }
      break;
    case MAPPARAM_CREATE_CARRIER:
      {
        return true;
        PARSE_CMD_PROTOBUF(CreateCarrierUserCmd, rev);

       m_oCarrier.create(rev.carrierid(), rev.line());

        return true;
      }
      break;
    case MAPPARAM_CARRIER_MOVE:
      {
        if (!getScene()) return true;
        PARSE_CMD_PROTOBUF(CarrierMoveUserCmd, rev);
        if (m_oCarrier.move(rev))
        {
          getScene()->sendCmdToNine(getPos(), buf, len);
        }

        return true;
      }
      break;
    case MAPPARAM_CARRIER_START:
      {
        // 防止不是队长的角色创建载具后直接起飞
        if (m_oCarrier.m_oData.m_dwCarrierID==BUS_FERRISWHEEL) return true;
        if (m_oCarrier.m_oData.m_dwCarrierID==BUS_DIVORCEROLLER && dynamic_cast<DivorceRollerCoasterScene*>(getScene()) != nullptr) return true;
        m_oCarrier.start();
        return true;
      }
      break;
    case MAPPARAM_CATCH_USER_JOIN_CARRIER:
      {
        PARSE_CMD_PROTOBUF(CatchUserJoinCarrierUserCmd, rev);

        SceneUser *master = SceneUserManager::getMe().getUserByID(rev.masterid());
        if (master && master->getScene() && master->getScene()==getScene())
        {
          master->m_oCarrier.join(this);
        }
        else
        {
          m_oCarrier.m_qwJoinMasterID = rev.masterid();
          gomap(rev.mapid(), GoMapType::Carrier);
        }

        return true;
      }
      break;
    default:
      break;
  }

  return true;
}

bool SceneUser::doSceneUserPetCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf || len == 0)
    return false;

  switch (buf->param)
  {
    case PETPARAM_FIRE_CAT:
      {
        PARSE_CMD_PROTOBUF(FireCatPetCmd, rev);
        m_oWeaponPet.del(rev.catid());
        XLOG << "[雇佣猫], 玩家解雇猫, 玩家:" << name << id << "猫id:" << rev.catid() << XEND;
      }
      break;
    case PETPARAM_HIRE_CAT:
      {
        PARSE_CMD_PROTOBUF(HireCatPetCmd, rev);
        m_oWeaponPet.add(rev.catid(), rev.etype());
        XLOG << "[雇佣猫], 玩家重新雇佣猫, 玩家:" << name << id << "猫id:" << rev.catid() << XEND;
      }
      break;
    case PETPARAM_HATCH_EGG:
      {
        PARSE_CMD_PROTOBUF(EggHatchPetCmd, rev);
        m_oPackage.hatchEgg(rev);
      }
      break;
    case PETPARAM_RESTORE_EGG:
      {
        PARSE_CMD_PROTOBUF(EggRestorePetCmd, rev);
        m_oPackage.restoreEgg(rev);
      }
      break;
    case PETPARAM_CATCH_PET:
      {
        PARSE_CMD_PROTOBUF(CatchPetPetCmd, rev);
        QWORD npcid = m_oUserPet.getCatchPetID();
        if (rev.npcguid() != 0 && rev.npcguid() != npcid)
        {
          XERR << "[宠物-捕捉], npcid不合法, 玩家:" << name << id << "id:" << rev.npcguid() << XEND;
          return true;
        }
        CatchPetNpc* npc = dynamic_cast<CatchPetNpc*> (SceneNpcManager::getMe().getNpcByTempID(npcid));
        if (npc == nullptr)
          return true;
        if (rev.isstop() == false)
          npc->catchMe();
        else
          npc->onStopCatch();
      }
      break;
    case PETPARAM_CATCH_GIFT:
      {
        PARSE_CMD_PROTOBUF(CatchPetGiftPetCmd, rev);
        m_oUserPet.giveGiftCatchNpc(rev.npcguid());
      }
      break;
    case PETPARAM_GET_GIFT:
      {
        PARSE_CMD_PROTOBUF(GetGiftPetCmd, rev);
        m_oUserPet.getGift(rev.petid());
      }
      break;
    case PETPARAM_EQUIP_OPER:
      {
        PARSE_CMD_PROTOBUF(EquipOperPetCmd, rev);
        m_oUserPet.equip(rev);
      }
      break;
    case PETPARAM_ADVENTURE_QUERYLIST:
      {
        PARSE_CMD_PROTOBUF(QueryPetAdventureListPetCmd, rev);
        m_oPetAdventure.sendAdventureList();
      }
      break;
    case PETPARAM_ADVENTURE_START:
      {
        PARSE_CMD_PROTOBUF(StartAdventurePetCmd, rev);
        m_oPetAdventure.startAdventure(rev);
      }
      break;
    case PETPARAM_ADVENTURE_GETREWARD:
      {
        PARSE_CMD_PROTOBUF(GetAdventureRewardPetCmd, rev);
        m_oPetAdventure.getAdventureReward(rev);
      }
      break;
    case PETPARAM_ADVENTURE_QUERYBATTLEPET:
      {
        m_oUserPet.sendBattlePets();
      }
      break;
    case PETPARAM_INVITE_HAND:
      {
        PARSE_CMD_PROTOBUF(HandPetPetCmd, rev);
        if (rev.breakup())
          m_oUserPet.breakHand();
        else
          m_oUserPet.inviteHand(rev.petguid());
      }
      break;
    case PETPARAM_GIVE_GIFT:
      {
        PARSE_CMD_PROTOBUF(GiveGiftPetCmd, rev);
        m_oUserPet.sendGift(rev.petid(), rev.itemguid());
      }
      break;
    case PETPARAM_RESET_SKILL:
      {
        PARSE_CMD_PROTOBUF(ResetSkillPetCmd, rev);
        m_oUserPet.resetSkill(rev);
      }
      break;
    case PETPARAM_CHANGE_NAME:
      {
        PARSE_CMD_PROTOBUF(ChangeNamePetCmd, rev);
        m_oUserPet.changeName(rev.petid(), rev.name());
      }
      break;
    case PETPARAM_SWITCH_SKILL:
      {
        PARSE_CMD_PROTOBUF(SwitchSkillPetCmd, rev);
        m_oUserPet.switchSkill(rev.petid(), rev.open());
      }
      break;
    case PETPARAM_WORK_UNLOCKMANUAL:
      {
        getPetWork().unlockManual();
      }
      break;
    case PETPARAM_WORK_QUERYWORKDATA:
      {
        getPetWork().queryWorkData();
      }
      break;
    case PETPARAM_WORK_STARTWORK:
      {
        PARSE_CMD_PROTOBUF(StartWorkPetCmd, rev);
        getPetWork().startWork(rev);
      }
      break;
    case PETPARAM_WORK_STOPWORK:
      {
        PARSE_CMD_PROTOBUF(StopWorkPetCmd, rev);
        getPetWork().stopWork(rev);
      }
      break;
    case PETPARAM_WORK_GETREWARD:
      {
        PARSE_CMD_PROTOBUF(GetPetWorkRewardPetCmd, rev);

        PetWork& rWork = getPetWork();
        SWorkSpace* pSpace = rWork.getSpaceData(rev.id());
        if (pSpace == nullptr || pSpace->pCFG == nullptr)
        {
          XERR << "[宠物打工-领取奖励]" << accid << id << getProfession() << name << "领取场所" << rev.id() << "的奖励失败,该场所非法" << XEND;
          break;
        }

        const SPetWorkCFG* pCFG = pSpace->pCFG;
        if (pCFG->dwActID != 0 && ActivityManager::getMe().isOpen(pCFG->dwActID) == false)
        {
          XERR << "[宠物打工-领取奖励]" << accid << id << getProfession() << name << "领取场所" << rev.id() << "的奖励失败,该场所活动" << pCFG->dwActID << "未开启" << XEND;
          break;
        }

        rWork.getWorkReward(rev);
        if (!pSpace->bUnlock)
        {
          StopWorkPetCmd cmd;
          cmd.set_id(rev.id());
          rWork.stopWork(cmd);
        }
      }
      break;
    case PETPARAM_COMPOSE:
      {
        PARSE_CMD_PROTOBUF(ComposePetCmd, rev);
        m_oUserPet.compose(rev);
      }
      break;
    case PETPARAM_CHANGE_WEAR:
      {
        PARSE_CMD_PROTOBUF(ChangeWearPetCmd, rev);
        m_oUserPet.changeWear(rev);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserFuBenCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    /*
    case WORLD_STAGE_USER_CMD:
      {
        m_stage.send();

        return true;
      }
      break;
    case SUB_STAGE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::StageStepUserCmd, rev);
        m_stage.send(rev.stageid());

        return true;
      }
      break;
    case START_STAGE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::StartStageUserCmd, rev);
        m_stage.start(rev.stageid(), rev.stepid(), rev.type());
        return true;
      }
      break;
    case GET_REWARD_STAGE_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(Cmd::GetRewardStageUserCmd, rev);
        m_stage.getReward(rev.stageid(), rev.starid());
        return true;
      }
      break;
      */
    case FUBEN_STEP_SYNC:
      {
        PARSE_CMD_PROTOBUF(Cmd::FubenStepSyncCmd, rev);
        setClientFubenID(rev.id());
        DScene* pDScene = dynamic_cast<DScene*> (getScene());
        if (pDScene)
        {
          pDScene->getFuben().check(rev.id());
        }
        return true;
      }
      break;
    case GUILD_RAID_GATE_OPT:
      {
        PARSE_CMD_PROTOBUF(Cmd::GuildGateOptCmd, cmd);
        switch (cmd.opt()) {
        case EGUILDGATEOPT_UNLOCK:
          unlockGuildRaidGate(cmd.gatenpcid(), cmd.uplocklevel());
          break;
        case EGUILDGATEOPT_OPEN:
          openGuildRaidGate(cmd.gatenpcid());
          break;
        case EGUILDGATEOPT_ENTER:
          enterGuildRaid(cmd.gatenpcid());
          break;
        default:
          return false;
        }
      }
      break;
    case GUILD_FIRE_STATUS:
      {
        PARSE_CMD_PROTOBUF(Cmd::GuildFireStatusFubenCmd, cmd);
        cmd.set_open(GuildCityManager::getMe().isCityInFire(cmd.cityid()));
        cmd.set_starttime(GuildCityManager::getMe().getFireStartTime());
        const SGuildCityCFG* pCity = GuildRaidConfig::getMe().getGuildCityCFG(cmd.cityid());
        if (pCity)
          cmd.set_cityopen(pCity->bOpen);

        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
        return true;
      }
      break;
    case SUPERGVG_QUERY_TOWERINFO:
      {
        SuperGvgScene* pScene = dynamic_cast<SuperGvgScene*>(getScene());
        if (pScene == nullptr)
          break;
        PARSE_CMD_PROTOBUF(QueryGvgTowerInfoFubenCmd, cmd);
        pScene->queryTowerInfo(this, cmd.etype(), cmd.open());
      }
      break;
    case SUPERGVG_QUERY_USER_DATA:
      {
        SuperGvgScene* pScene = dynamic_cast<SuperGvgScene*>(getScene());
        if (pScene == nullptr)
          break;
        pScene->queryUserData(this);
      }
      break;
    case TEAMPWS_SELECT_MAGIC:
      {
        TeamPwsScene* pScene = dynamic_cast<TeamPwsScene*>(getScene());
        if (pScene == nullptr)
          break;
        PARSE_CMD_PROTOBUF(SelectTeamPwsMagicFubenCmd, cmd);
        pScene->selectMagic(this, cmd.magicid());
      }
      break;
    case QUERY_RAID_TEAMPWS_USERINFO:
      {
        TeamPwsScene* pScene = dynamic_cast<TeamPwsScene*>(getScene());
        if (pScene == nullptr)
          break;
        pScene->queryDetailInfo(this);
      }
      break;
    case INVITE_SUMMON_DEADBOSS:
      {
        DScene* pDScene = dynamic_cast<DScene*> (getScene());
        if (pDScene == nullptr)
          break;
        pDScene->inviteSummonDeadBoss(this);
      }
      break;
    case REPLY_SUMMON_DEADBOSS:
      {
        PARSE_CMD_PROTOBUF(ReplySummonBossFubenCmd, rev);
        DScene* pDScene = dynamic_cast<DScene*> (getScene());
        if (pDScene == nullptr)
          break;
        pDScene->replySummonDeadBoss(this, rev.agree());
      }
      break;
    default:
      break;
  }

  return true;
}

bool SceneUser::doSceneUserTipCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (nullptr == buf)
    return false;

  switch (buf->param)
  {
    case TIPPARAM_BROWSE:
      {
        PARSE_CMD_PROTOBUF(BrowseRedTipCmd, rev);
        m_oTip.browseTip(rev.red(), rev.tipid());
      }
      break;
    case TIPPARAM_ADDREDTIP:
      {
        PARSE_CMD_PROTOBUF(AddRedTip, rev);
        m_oTip.addRedTip(rev.red(), rev.tipid());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserChatRoomCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (isInPollyScene())
    return true;

  if (!buf || !len) return false;

  switch (buf->param)
  {
    case ECHATROOMPARAM_CREATE:
      {
        PARSE_CMD_PROTOBUF(CreateChatRoom, rev);
        ChatRoomManager::getMe().createRoom(this, rev.roomname(), rev.pswd(), rev.maxnum());

        return true;
      }
      break;
    case ECHATROOMPARAM_JOIN:
      {
        PARSE_CMD_PROTOBUF(JoinChatRoom, rev);
        ChatRoomManager::getMe().joinRoom(this, rev.roomid(), rev.pswd());

        return true;
      }
      break;
    case ECHATROOMPARAM_EXIT:
      {
        PARSE_CMD_PROTOBUF(ExitChatRoom, rev);
        ChatRoomManager::getMe().exitRoom(this, rev.roomid());

        return true;
      }
      break;
    case ECHATROOMPARAM_KICKMEMBER:
      {
        PARSE_CMD_PROTOBUF(KickChatMember, rev);
        ChatRoomManager::getMe().kickMember(this, rev.roomid(), rev.memberid());

        return true;
      }
      break;
    case ECHATROOMPARAM_CHANGEOWNER:
      {
        PARSE_CMD_PROTOBUF(ExchangeRoomOwner, rev);
        ChatRoomManager::getMe().changeOwner(this, rev.userid());

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

bool SceneUser::doSceneUserTowerCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch(buf->param)
  {
    case ETOWERPARAM_ENTERTOWER:
      {
        PARSE_CMD_PROTOBUF(EnterTower, rev);
        const STowerLayerCFG* pLayerCFG = TowerConfig::getMe().getTowerLayerCFG(rev.layer());
        if (pLayerCFG == nullptr)
        {
          XERR << "[玩家-无限塔]" << accid << id << getProfession() << name << "收到社交服无限塔进入请求" << rev.layer() << "未找到相关配置"<< XEND;
          break;
        }

        m_oZone.gomap(rev.zoneid(), pLayerCFG->dwRaidID, GoMapType::GM);
        XLOG << "[玩家-无限塔]" << accid << id << getProfession() << name << "收到社交服无限塔进入请求" << rev.layer() << "层,准备进入"<< XEND;
      }
      break;
    case ETOWERPARAM_TOWERINFO:
      {
        TowerInfoCmd cmd;
        cmd.set_maxlayer(TowerConfig::getMe().getMaxLayer());
        cmd.set_refreshtime(xTime::getWeekStart(xTime::getCurSec(), 5 * 3600) + 86400 * 7 + 5 * 3600);
        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserInterCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch (buf->param)
  {
    case INTERPARAM_ANSWERINTER:
      {
        PARSE_CMD_PROTOBUF(Answer, rev);
        if (XoclientConfig::getMe().isActivityQuestion(rev.interid()))
        {
          SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rev.npcid());
          if (pNpc == nullptr)
          {
            XERR << "[活动-问答] 找不到相应id 的npc，charid" << id << "npcid" << rev.npcid() << XEND;
            return false;
          }
          ActivityBase* pAct = ActivityManager::getMe().getActivityByUid(pNpc->getActivityUid());
          if (!pAct)
          {
            XERR << "[活动-问答] 找不到相应id 的活动，charid" << id << "npcid" << rev.npcid() << "activity uid" << pNpc->getActivityUid() << XEND;
            return false;
          }
          ActivityQuest* pActQuest = dynamic_cast<ActivityQuest*>(pAct);
          if (!pActQuest)
          {
            XERR << "[活动-问答] 找不到相应的活动类型非法，charid" << id << "npcid" << rev.npcid() << "activity uid" << pNpc->getActivityUid() << XEND;
            return false;
          }
          return pActQuest->answer(this, rev);
        }
        
        m_oInter.answer(rev.guid(), rev.answer());
      }
      return true;
    case INTERPARAM_QUERYINTER:
      {
        PARSE_CMD_PROTOBUF(Query, rev);
        
        SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rev.npcid());
        if (pNpc == nullptr)
        {
          XERR << "[活动-问答] 找不到相应id 的npc，charid" << id << "npcid" << rev.npcid() << XEND;
          return false;
        }
        ActivityBase* pAct = ActivityManager::getMe().getActivityByUid(pNpc->getActivityUid());
        if (!pAct)
        {
          XERR << "[活动-问答] 找不到相应id 的活动，charid" << id << "npcid" << rev.npcid() << "activity uid" << pNpc->getActivityUid() << XEND;
          return false;
        }
        ActivityQuest* pActQuest = dynamic_cast<ActivityQuest*>(pAct);
        if (!pActQuest)
        {
          XERR << "[活动-问答] 找不到相应的活动类型非法，charid" << id << "npcid" << rev.npcid() << "activity uid" << pNpc->getActivityUid() << XEND;
          return false;
        }
        return pActQuest->query(this, rev);
      }
    return true;
  }

  return true;
}

bool SceneUser::doSceneUserManualCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch (buf->param)
  {
    case MANUALPARAM_QUERYDATA:
      {
        PARSE_CMD_PROTOBUF(QueryManualData, rev);
        m_oManual.queryManualData(rev.type());
      }
      break;
    /*case MANUALPARAM_GETREWARD:
      {
        PARSE_CMD_PROTOBUF(GetAchieveReward, rev);
        m_oManual.getAchieveReward(rev.id());
      }
      break;*/
    case MANUALPARAM_UNLOCK:
      {
        PARSE_CMD_PROTOBUF(Unlock, rev);
        m_oManual.unlock(rev.type(), rev.id());
      }
      break;
    case MANUALPARAM_GETQUESTREWARD:
      {
        PARSE_CMD_PROTOBUF(GetQuestReward, rev);
        m_oManual.getQuestReward(rev.appendid());
      }
      break;
    case MANUALPARAM_STOREITEM:
      {
        PARSE_CMD_PROTOBUF(StoreManualCmd, rev);
        m_oManual.storeItem(rev.type(), rev.guid());
      }
      break;
    case MANUALPARAM_GETITEM:
      {
        PARSE_CMD_PROTOBUF(GetManualCmd, rev);
        m_oManual.getItem(rev.type(), rev.itemid());
      }
      break;
    case MANUALPARAM_GROUPACTION:
      {
        PARSE_CMD_PROTOBUF(GroupActionManualCmd, rev);
        m_oManual.groupAction(rev.action(), rev.group_id());
      }
      break;
    case MANUALPARAM_UPDATE_UNSOLVED_PHOTO:
      {
        PARSE_CMD_PROTOBUF(UpdateSolvedPhotoManualCmd, rev);
        m_oManual.unsolvedPhoto(rev);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserSealCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch (buf->param)
  {
    case SEALPARAM_QUERYSEAL:
      {
        //m_oSeal.sendAllSealInfo();
      }
      break;
    case SEALPARAM_SEALTIMER:
      {
        if (getTeamSeal())
        {
          getTeamSeal()->sendSealInfo(this->id);
        }
      }
      break;
    case SEALPARAM_USERLEAVE:
      {
        /*
        if (m_pTeam != nullptr)
        {
          m_pTeam->getSeal().onClientSendLeave(this);
        }
        */
      }
      break;
    case SEALPARAM_BEGINSEAL:
      {
        PARSE_CMD_PROTOBUF(BeginSeal, rev);
        getSeal().beginSeal(rev.sealid(), rev.etype());
      }
      break;
    case SEALPARAM_QUERYLIST:
      {
        SealQueryList cmd;
        const TSetDWORD& vec = getSeal().getOpenSeals();
        for (auto &v : vec)
        {
          cmd.add_configid(v);
        }
        cmd.set_maxtimes(MiscConfig::getMe().getSealCFG().dwMaxDaySealNum);
        cmd.set_donetimes(getVar().getVarValue(EVARTYPE_SEAL));
        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case SEALPARAM_ACCEPTSEAL:
      {
      // record in team
        PARSE_CMD_PROTOBUF(SealAcceptCmd, rev);
        if (getTeamID() == 0 || getTeamLeaderID() != id)
        {
          MsgManager::sendMsg(id, 1611);
          return false;
        }

        const SealCFG* pCFG = SealConfig::getMe().getSealCFG(rev.seal());
        if (pCFG == nullptr)
          return false;
        SetTeamSeal cmd;
        cmd.set_teamid(getTeamID());
        cmd.set_sealid(rev.seal());
        cmd.set_mapid(pCFG->dwMapID);
        cmd.set_leaderid(id);
        const GTeam& rTeam = getTeam();
        for (auto &m : rTeam.getTeamMemberList())
        {
          if (m.second.catid() != 0)
            continue;
          cmd.add_teamers(m.second.charid());
        }
        if (rev.abandon())
        {
          cmd.set_estatus(ESETSEALSTATUS_ABANDON);
        }
        else
        {
          const TSetDWORD& sset = getSeal().getOpenSeals();
          if (sset.find(rev.seal()) == sset.end())
            return false;

          // check leader
          cmd.set_estatus(ESETSEALSTATUS_CREATE);
        }
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
      break;
    default:
      break;
  }
  return true;
}

bool SceneUser::doSessionUserShopCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch (buf->param)
  {
    case SHOPPARAM_BUYITEM:
      {
        PARSE_CMD_PROTOBUF(BuyShopItem, rev);
        bool ret = getSceneShop().buyItem(rev.id(), rev.count(), rev.price(), rev.price2());
        if (ret == false)
        {
          rev.set_success(ret);
          PROTOBUF(rev, send, len);
          sendCmdToMe(send, len);
        }
      }
      break;
    case SHOPPARAM_QUERY_SHOP_CONFIG:
      {
        PARSE_CMD_PROTOBUF(QueryShopConfigCmd, rev);
        getSceneShop().queryConfigByType(rev);
      }
      break;
    case SHOPPARAM_QUICKBUY_SHOP_CONFIG:
    {
      PARSE_CMD_PROTOBUF(QueryQuickBuyConfigCmd, rev);
      getSceneShop().queryQuickBuy(rev);
    }
    break;
    case SHOPPARAM_EXCHANGEITEM_CMD:
    {
      PARSE_CMD_PROTOBUF(ExchangeShopItemCmd, rev);
      getExchangeShop().exchange(rev);
    }
    break;
  }

  return true;
}

bool SceneUser::doSceneUserTrade(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
  case CHECK_PACKAGE_SIZE_TRADE_CMD:
  {
    PARSE_CMD_PROTOBUF(CheckPackageSizeTradeCmd, rev);
    bool ret = false;
    do 
    {
      BasePackage* pPkg = getPackage().getPackage(EPACKTYPE_MAIN);
      if (!pPkg)
        break;
      
      TVecItemInfo itemInfos;
      for (int i = 0; i < rev.items_size(); ++i)
      {
        itemInfos.push_back(rev.items(i));
      }
      
      if (!pPkg->checkAddItem(itemInfos, EPACKMETHOD_CHECK_WITHPILE))
      {
        MsgManager::sendMsg(id, 989);
        XERR << "[交易所-快速购买] 检查背包" << accid << id << getProfession() << name << "背包空间不足" << XEND;
        break;
      }      
      ret = true;
    } while (0);
    rev.set_ret(ret);

    PROTOBUF(rev, buf, len);
    sendCmdToMe(buf, len);
    XLOG << "[交易所-快速购买] 检查背包" << accid << id << getProfession() << name << "背包空间足够:" << ret << XEND;
    return true;
  }
  break;
  }
  return true;
}

bool SceneUser::doUserEventCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (!buf)
    return false;

  switch (buf->param)
  {
    case USER_EVENT_FIRST_ACTION:
      {
        PARSE_CMD_PROTOBUF(FirstActionUserEvent, rev);
        getUserSceneData().addClientFirstAction(rev.firstaction());
      }
      break;   
    case USER_EVENT_DEL_TRANSFORM:
      {
        if (getScene() && getScene()->isPollyScene())
          break;
        if (getScene() && getScene()->isAltmanScene())
          break;
        m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);
        Attribute* pAttr = getAttribute();
        if (pAttr != nullptr)
          pAttr->setMark();
      }
      break;
    case USER_EVENT_NPC_FUNCTION:
      {
        PARSE_CMD_PROTOBUF(TrigNpcFuncUserEvent, rev);
        m_event.onTrigNpcFunction(rev.npcguid(), rev.funcid());
      }
      break;
    case USER_EVENT_SYSTEM_STRING:
      {
        PARSE_CMD_PROTOBUF(SystemStringUserEvent, rev);
        switch(rev.etype())
        {
          case ESYSTEMSTRING_MIN:
            break;
          case ESYSTEMSTRING_MEMO:
            if (getScene() && getScene()->isDScene() == false)
            {
              m_oUserSceneData.setTransMap(getScene()->getMapID(), getPos());
              MsgManager::sendMsg(id, 610);
            }
            else
            {
              MsgManager::sendMsg(id, 611);
            }
            break;
        }
      }
      break;
    case USER_EVENT_HAND_CAT:
      {
        PARSE_CMD_PROTOBUF(HandCatUserEvent, rev);
        if (rev.breakup())
          m_oWeaponPet.breakHand(rev.catguid());
        else
          m_oWeaponPet.inviteHand(rev.catguid());
      }
      break;
    case USER_EVENT_CHANGE_TITLE:
      {
        PARSE_CMD_PROTOBUF(ChangeTitle, rev);
        m_oTitle.changeCurTitle(rev);
      }
      break;
    case USER_EVENT_GET_RECALL_SHARE_REWARD:
      getFirstShareReward();
      break;
    case USER_EVENT_QUERY_RESETTIME:
      {
        PARSE_CMD_PROTOBUF(QueryResetTimeEventCmd, rev);
        DWORD time = ActivityEventManager::getMe().getNextResetTime(rev.etype());
        rev.set_resettime(time);
        PROTOBUF(rev, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case USER_EVENT_INOUT_ACT:
      {
        PARSE_CMD_PROTOBUF(InOutActEventCmd, rev);
        SceneActBase* pAct = SceneActManager::getMe().getSceneAct(rev.actid());
        if (pAct == nullptr)
          return true;
        SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*>(pAct);
        if (pAct == nullptr)
          return true;
        if (rev.inout() == true)
        {
          if (getXZDistance(pActEvent->getPos(), getPos()) < 1.5f * pActEvent->getRange())
            pActEvent->onUserIn(this);
        }
        else
        {
          if (getXZDistance(pActEvent->getPos(), getPos()) > 0.5f * pActEvent->getRange())
            pActEvent->onUserOut(this);
        }
      }
      break;
    case USER_EVENT_MAIL:
      {
        PARSE_CMD_PROTOBUF(UserEventMailCmd, rev);
        m_event.onReceiveEventMail(rev);
      }
      break;
    case USER_EVENT_LEVELUP_DEAD:
      levelupDead();
      break;
    default:
      return false;
  }
  return true;
}

bool SceneUser::doSceneUserDojoCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch (buf->param)
  {
  case EDOJOPARAM_DOJO_PRIVATE_INFO:
  {
    PARSE_CMD_PROTOBUF(DojoPrivateInfoCmd, rev);
    XLOG << "[道场-私人数据] charid:" << id << "msg:" << rev.ShortDebugString() << XEND;
    m_oDojo.getPrivateInfo(rev);
  }
  break;
  case EDOJOPARAM_ENTERDOJO: //进入道场副本
  {
    PARSE_CMD_PROTOBUF(EnterDojo, rev);
    XLOG << "[道场-进入道场] charid:" << id << "msg:" << rev.ShortDebugString() << XEND;
    const SDojoItemCfg* pCfg = DojoConfig::getMe().getDojoItemCfg(rev.dojoid());
    if (pCfg == nullptr)
    {
      return false;
    }
    {
      getUserZone().gomap(rev.zoneid(), pCfg->dwRapid, GoMapType::GM);
    }
    return true;
  }
  break;
  case EDOJOPARAM_SPONSOR:   //发起道场
  {    
    if (getTeamID())
    {
      PARSE_CMD_PROTOBUF(DojoSponsorCmd, rev);
      XLOG << "[道场-发起] charid:" << id << "msg:" << rev.ShortDebugString() << XEND;
      const SDojoItemCfg* pDojoCfg = DojoConfig::getMe().getDojoItemCfg(rev.dojoid());
      if (pDojoCfg == nullptr)
      {
        XERR << "[道场-发起] 失败 accid:" << accid << "id:" << id << "dojoid:" << rev.dojoid() << "找不到道场的配置表" << XEND;
        return false;
      }
      if (getDojo().isOpen(pDojoCfg->dwGroupId) == false)
      {
        //道场组尚未开放
        XERR << "[道场-发起] 失败 accid:" << accid << "id:" << id << "dojoid:" << rev.dojoid() << "guildlevel:" << getGuild().lv() << "功能尚未开放" << XEND;
        return false;
      }
      if (!hasGuild())
      {
        XERR << "[道场-发起] 失败 accid:" << accid << "id:" << id << "dojoid:" << rev.dojoid() << "玩家没有公会" << XEND;
        return false;
      }
      rev.set_sponsorid(id);
      rev.set_sponsorname(name);

      PROTOBUF(rev, send, len);
      thisServer->forwardCmdToSessionUser(id, send, len);
      return true;
    }
    return false;
  }
  break;
  case EDOJOPARAM_ADD_MSG:      //道场留言
  {
    PARSE_CMD_PROTOBUF(DojoAddMsg, rev);
    XLOG << "[道场-留言] charid:" << id << "dojoid:" << rev.dojoid() << "msg:" << rev.dojomsg().conent() << XEND;
    bool isPassed = m_oDojo.isPassed(rev.dojoid());

    Cmd::DojoMsg* pMsg = rev.mutable_dojomsg();
    if (!pMsg)
    {
      return false;
    }
    DWORD dwCount = getWordCount(pMsg->conent());
    if (dwCount > 20)
      return false;

    pMsg->set_iscompleted(isPassed);
    pMsg->set_charid(id);
    pMsg->set_name(name);
    PROTOBUF(rev, send, len);
    thisServer->forwardCmdToSessionUser(id, send, len);
  }
  break;
  default:
    break;
  }

  return true;
}

bool SceneUser::doSceneUserAuguryCmd(const Cmd::UserCmd* buf, WORD len)
{
  static DWORD COST_ITEM_COUNT = 1;
  switch (buf->param)
  {
  case AUGURYPARAM_INVITE:
  {
    PARSE_CMD_PROTOBUF(AuguryInvite, rev);

    //check hand     
    QWORD otherId = m_oHands.getOtherID();
    if (otherId == 0)
    {
      XERR << "[占卜-发起邀请] 两人不是牵手状态" << id << otherId << XEND;
      return false;
    }

    SceneUser* pOtherUser = SceneUserManager::getMe().getUserByID(otherId);
    if (pOtherUser == nullptr)
    {
      MsgManager::sendMsg(id, 941);
      XERR << "[占卜-发起邀请] 牵手人不在地图" << id << otherId << XEND;
      return false;
    }
   
    Augury* pAugury = AuguryMgr::getMe().getAugury(pOtherUser);
    if (pAugury)
    {
      XERR << "[占卜-发起邀请] 另一个人已经建立了邀请" << id << "otherid" << otherId<<"uid"<<pAugury->getUid() << XEND;
      MsgManager::sendMsg(id, 932);
      return false;
    }

    //check pos
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rev.npcguid());
    if (pNpc == nullptr)
    {
      MsgManager::sendMsg(id, 941);
      XERR << "[占卜-发起邀请] 地图上找出到npc" << id << otherId <<"npcguid"<<rev.npcguid() << XEND;
      return false;
    }
    
    //check npcid

    if (pOtherUser->getScene() == nullptr || getScene() == nullptr || pOtherUser->getScene() != getScene())
    {
      MsgManager::sendMsg(id, 941);
      XERR << "[占卜-发起邀请] 被邀请人不在场景" << id << otherId << "npcguid" << rev.npcguid() << XEND;
      return false;
    }

    if (!checkDistance(pOtherUser->getPos(), pNpc->getPos(), MiscConfig::getMe().getAuguryCfg().fRange))
    {
      MsgManager::sendMsg(id, 941);
      XERR << "[占卜-发起邀请] 被邀请人距离不在npc 附近" << id << otherId << "npcguid" << rev.npcguid() << XEND;
      return false;
    }
    
    //老的占卜
    MainPackage* pPackage = nullptr;
    bool bIsExtra = false;
    if (rev.type() == EAUGURYTYPE_VALENTINE)
    {
      //check item
      DWORD itemId = MiscConfig::getMe().getAuguryCfg().dwItemId;
      pPackage = dynamic_cast<MainPackage*>(m_oPackage.getPackage(EPACKTYPE_MAIN));
      if (pPackage == nullptr)
        return false;
      if (pPackage->checkItemCount(itemId, COST_ITEM_COUNT) == false)
      {
        MsgManager::sendMsg(id, 933);
        XERR << "[占卜-发起邀请] 道具数量不够" << id << "itemid" << itemId << XEND;
        return false;
      }
      //check item
      pPackage = dynamic_cast<MainPackage*>(pOtherUser->getPackage().getPackage(EPACKTYPE_MAIN));
      if (pPackage == nullptr)
        return false;
      if (pPackage->checkItemCount(itemId, COST_ITEM_COUNT) == false)
      {
        MsgManager::sendMsg(id, 933);
        XERR << "[占卜-发起邀请] 对方道具不够，道具数量不够" << id << "itemid" << itemId << "otherid" << otherId << XEND;
        return false;
      }
    }
    else
    {
      if (!AuguryMgr::getMe().useExtraCount(this, true, true))
      {
        if (!AuguryMgr::getMe().useFreeCount(this, true))
          MsgManager::sendMsg(id, 937);
      }
      else
        bIsExtra = true;
    }
    rev.set_inviterid(id);
    rev.set_invitername(name);
    rev.set_isextra(bIsExtra);
    PROTOBUF(rev, send, len);
    
    pOtherUser->sendCmdToMe(send, len);
    XLOG << "[占卜-发起邀请] 发起人" << id << "占卜类型" << rev.type() <<"npcguid"<<rev.npcguid()<< "msg" << rev.inviterid() <<rev.invitername() << XEND;
  }
  break;
  case AUGURYPARAM_INVITE_REPLY: 
  {
    PARSE_CMD_PROTOBUF(AuguryInviteReply, rev);
    XLOG << "[占卜-邀请返回] charid" << id << "占卜类型" << rev.augurytype() << "inviter" <<rev.inviterid() <<"回复类型"<<rev.type() <<"额外次数" << rev.isextra() << XEND;

    SceneUser* pInviter = SceneUserManager::getMe().getUserByID(rev.inviterid());
    if (pInviter == nullptr)
    {
      XERR << "[占卜-邀请返回] 找不到邀请人， charid" << id << "占卜类型" << rev.augurytype() << "inviter" << rev.inviterid() << "type" << rev.type() << XEND;
      return false;
    }

    //拒绝
    if (rev.type() == EReplyType_Refuse)
    {
      pInviter->sendCmdToMe(buf, len);
      sendCmdToMe(buf, len);
      return true;;
    }

    Augury *pAugury = AuguryMgr::getMe().getAugury(pInviter);
    if (pAugury)
    {
      XERR << "[占卜-邀请返回] 邀请人已经进入房间了， 被邀请人" << id << "邀请人" << rev.inviterid() << "占卜类型" << rev.augurytype() << "type" << rev.type() << XEND;
      return false;
    }
    pAugury = AuguryMgr::getMe().getAugury(this);
    if (pAugury)
    {
      XERR << "[占卜-邀请返回] 被邀请人已经进入房间了， 被邀请人" << id << "邀请人" << rev.inviterid() << "占卜类型" << rev.augurytype() << "type" << rev.type() << XEND;
      return false;
    }

    //check hand
    if (pInviter->m_oHands.getOtherID() != id)
    {
      MsgManager::sendMsg(pInviter->id, 941);
      XERR << "[占卜-邀请返回] 已经不是牵手状态， charid" << id << "占卜类型" << rev.augurytype() << "inviter" << rev.inviterid() << "type" << rev.type() << XEND;
      return false;
    }    

    //check pos
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rev.npcguid());
    if (pNpc == nullptr)
    {
      MsgManager::sendMsg(id, 941);
      MsgManager::sendMsg(pInviter->id, 941);
      XERR << "[占卜-邀请返回] 地图上找出到npc" << id << "占卜类型" << rev.augurytype() << "npcguid" << rev.npcguid() << XEND;
      return false;
    }

    if (pInviter->getScene() == nullptr || getScene() == nullptr || pInviter->getScene() != getScene())
    {
      MsgManager::sendMsg(id, 941);
      MsgManager::sendMsg(pInviter->id, 941);
      XERR << "[占卜-邀请返回] 被邀请人不在场景" << id << "占卜类型" << rev.augurytype() <<"邀请人"<< rev.inviterid() << "npcguid" << rev.npcguid() << XEND;
      return false;
    }

    if (!checkDistance(getPos(), pNpc->getPos(), MiscConfig::getMe().getAuguryCfg().fRange))
    {
      MsgManager::sendMsg(id, 941);
      MsgManager::sendMsg(pInviter->id, 941);
      XERR << "[占卜-邀请返回] 被邀请人距离不在npc 附近" << id << "占卜类型" << rev.augurytype() << "邀请人" << rev.inviterid() << "npcguid" << rev.npcguid() << XEND;
      return false;
    }

    if (!checkDistance(pInviter->getPos(), pNpc->getPos(), MiscConfig::getMe().getAuguryCfg().fRange))
    {
      MsgManager::sendMsg(id, 941);
      MsgManager::sendMsg(pInviter->id, 941);
      XERR << "[占卜-邀请返回] 邀请人距离不在npc 附近" << id << "占卜类型" << rev.augurytype() << "邀请人" << rev.inviterid() << "npcguid" << rev.npcguid() << XEND;
      return false;
    }
    
    MainPackage* pPackage = nullptr;
    MainPackage* pMyPackage = nullptr;
    if (rev.augurytype() == EAUGURYTYPE_VALENTINE)
    {
      //check inviter item
      pPackage = dynamic_cast<MainPackage*>(pInviter->getPackage().getPackage(EPACKTYPE_MAIN));
      DWORD itemId = MiscConfig::getMe().getAuguryCfg().dwItemId;
      if (pPackage == nullptr)
        return false;
      if (pPackage->checkItemCount(itemId, COST_ITEM_COUNT) == false)
      {
        MsgManager::sendMsg(id, 933);
        MsgManager::sendMsg(pInviter->id, 933);
        XERR << "[占卜-邀请返回] 发起人道具数量不够" << id << "占卜类型" << rev.augurytype() << "发起人" << pInviter->id << "itemid" << itemId << XEND;
        return false;
      }
      //check my item
      pMyPackage = dynamic_cast<MainPackage*>(getPackage().getPackage(EPACKTYPE_MAIN));
      if (pMyPackage == nullptr)
        return false;
      if (pMyPackage->checkItemCount(itemId, COST_ITEM_COUNT) == false)
      {
        MsgManager::sendMsg(id, 933);
        MsgManager::sendMsg(pInviter->id, 933);
        XERR << "[占卜-邀请返回] 被邀请人道具数量不够" << id << "占卜类型" << rev.augurytype() << "发起人" << pInviter->id << "itemid" << itemId << XEND;
        return false;
      }
    }

    pInviter->sendCmdToMe(buf, len);
    sendCmdToMe(buf, len);

    //create room
    pAugury = AuguryMgr::getMe().create(pInviter, rev.augurytype());
    if (pAugury == nullptr)
    {
      XERR << "[占卜-邀请返回] 创建房间失败" << id << "占卜类型" << rev.augurytype() << "发起人" << pInviter->id << XEND;
      return false;
    }
    if (!AuguryMgr::getMe().enter(this, pAugury->getUid()))
    {
      XERR << "[占卜-邀请返回] 加入房间失败" << id << "占卜类型" << rev.augurytype() << "发起人" << pInviter->id << "房间id" << pAugury->getUid() << XEND;
      AuguryMgr::getMe().quit(pInviter);
      return false;
    }
    if (!pAugury->start())
    {
      XERR << "[占卜-邀请返回] 房间启动失败" << id << "占卜类型" << rev.augurytype() << "发起人" << pInviter->id << "房间id"<<pAugury->getUid() << XEND;
      AuguryMgr::getMe().quit(pInviter);
      return false;
    }

    if (rev.augurytype() == EAUGURYTYPE_VALENTINE)
    {
      // reduce item
      if (pPackage)
        pPackage->reduceItem(MiscConfig::getMe().getAuguryCfg().dwItemId, ESOURCE_USEITEM, COST_ITEM_COUNT);
      if (pMyPackage)
        pMyPackage->reduceItem(MiscConfig::getMe().getAuguryCfg().dwItemId, ESOURCE_USEITEM, COST_ITEM_COUNT);
    }
    else
    {
      //邀请者使用额外次数
      if (rev.isextra())
      {
        if (!AuguryMgr::getMe().useExtraCount(this, false, true))
        {
          if (!AuguryMgr::getMe().useFreeCount(this, true))
          {
            MsgManager::sendMsg(id, 937);
          }
        }               
      }
      else
      {
        if (!AuguryMgr::getMe().useFreeCount(this, true))
        {
          MsgManager::sendMsg(id, 937);
        }
      }         
    }
    return true;
  }
  break;
  case AUGURYPARAM_CHAT:   
  {
    PARSE_CMD_PROTOBUF(AuguryChat, rev);   
    Augury* pAugury = AuguryMgr::getMe().getAugury(this);
    if (!pAugury)
      return false;

    pAugury->chat(id, rev);

    PROTOBUF(rev, send, len);
    //sendCmdToMe(send, len);
    return true;
  }

  break;
  case AUGURYPARAM_ANSWER:     
  {
    PARSE_CMD_PROTOBUF(AuguryAnswer, rev);
    
    Augury* pAugury = AuguryMgr::getMe().getAugury(this);
    if (!pAugury)
      return false;
    rev.set_answerid(id);
    pAugury->answer(this, rev);
    return true;
  }
  break;
  case AUGURYPARAM_QUIT:
  {
    PARSE_CMD_PROTOBUF(AuguryQuit, rev);
    AuguryMgr::getMe().quit(this);    
    return true;
  }
  break;
  default:
    break;
  }

  return true;
}

bool SceneUser::doSocialCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param)
  {
    case SOCIALITYPARAM_OPERATE_QUERY:
      {
        PARSE_CMD_PROTOBUF(OperateQuerySocialCmd, rev);
        OperateQuerySocialCmd cmd;
        cmd.set_type(rev.type());
        DWORD activityid = MiscConfig::getMe().getMonthCardActivityCFG().dwID;
        bool isOpen = ActivityManager::getMe().isOpen(activityid);
        if(isOpen)
        {
          TVecDWORD vec;
          getCelebrationID(vec);
          if(vec.empty() == true)
            cmd.set_state(EOperateState_None);
          else
          {
            for(auto &v : vec)
            {
              cmd.add_param4(v);
            }
            cmd.set_state(EOperateState_CanTake);
          }
        }
        else
        {
          cmd.set_state(EOperateState_None);
        }

        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case SOCIALITYPARAM_OPERATE_TAKE:
      {
        PARSE_CMD_PROTOBUF(OperateTakeSocialCmd, rev);
        getCelebrationReward(rev.subkey());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSessionUserTeamCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param)
  {
    case TEAMPARAM_QUERYMEMBERCAT:
      {
        MemberCatUpdateTeam cmd;
        m_oWeaponPet.toData(cmd);
        PROTOBUF(cmd, send, len);
        m_oWeaponPet.broadcastCmdToTeam(send, len);
      }
      break;
    case TEAMPARAM_INVITEMEMBER:
      {
        PARSE_CMD_PROTOBUF(InviteMember, rev);
        if (rev.catid() == 0)
          break;

        const SWeaponPetData* pData = m_oWeaponPet.getData(rev.catid());
        if (pData == nullptr)
          break;
        if (m_oWeaponPet.checkCanActive(rev.catid()) == false)
        {
          InviteCatFailUserEvent cmd;
          PROTOBUF(cmd, send, len);
          sendCmdToMe(send, len);
          MsgManager::sendMsg(id, 5003);
          break;
        }

        if (getTeamID() != 0)
        {
          CatEnterTeamCmd cmd;
          cmd.set_charid(id);
          pData->toData(cmd.add_cats(), this);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
        else
        {
          if (m_oWeaponPet.enable(rev.catid()) == false)
            break;
          TeamMemberUpdate cmd;
          pData->toData(cmd.add_updates(), this);
          PROTOBUF(cmd, send, len);
          sendCmdToMe(send, len);
        }
      }
      break;
    case TEAMPARAM_KICKMEMBER:
      {
        PARSE_CMD_PROTOBUF(KickMember, rev);
        if (getTeamID() != 0)
          break;
        if (m_oWeaponPet.disable(rev.catid()) == false)
          break;
        TeamMemberUpdate cmd;
        cmd.add_deletes(rev.userid());
        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserAchieveCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param)
  {
    case ACHIEVEPARAM_QUERY_USERRESUME:
      m_oAchieve.queryUserResume();
      break;
    case ACHIEVEPARAM_REWARD_GET:
      {
        PARSE_CMD_PROTOBUF(RewardGetAchCmd, rev);
        m_oAchieve.getReward(rev.id());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserAstrolabeCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param) {
  case ASTROLABEPARAM_ACTIVATE_STAR:
  {
    PARSE_CMD_PROTOBUF(AstrolabeActivateStarCmd, rev);
    m_oAstrolabes.handleActivateStar(rev);
    break;
  }
  case ASTROLABEPARAM_RESET:
  {
    PARSE_CMD_PROTOBUF(AstrolabeResetCmd, rev);
    m_oAstrolabes.handleReset(rev);
    break;
  }
  default:
    return false;
  }

  return true;
}

bool SceneUser::doSceneUserPhotoCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param) {
    case PHOTOPARAM_PHOTO_OPT:
      {
        PARSE_CMD_PROTOBUF(PhotoOptCmd, rev);
        m_oPhoto.handlePhotoOpt(rev);
        break;
      }
    case PHOTOPARAM_FRAME_ACTION:
      {
        PARSE_CMD_PROTOBUF(FrameActionPhotoCmd, rev);
        /*GuildScene* pScene = dynamic_cast<GuildScene*>(getScene());
        if (pScene == nullptr)
        {
          XERR << "[公会领地-照片操作]" << accid << id << getProfession() << name << "相片操作" << rev.ShortDebugString() << "失败,未在公会领地" << XEND;
          break;
        }*/
        if (getScene() == nullptr)
          break;
        getScene()->frameAction(this, rev);
        break;
      }
    case PHOTOPARAM_QUERY_FRAMEPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryFramePhotoListPhotoCmd, rev);
        /*GuildScene* pScene = dynamic_cast<GuildScene*>(getScene());
        if (pScene == nullptr)
        {
          XERR << "[公会领地-相框请求]" << accid << id << getProfession() << name << "请求相框" << rev.frameid() << "失败,未在公会领地" << XEND;
          break;
        }*/
        if (getScene() == nullptr)
          break;
        getScene()->queryFrame(this, rev.frameid());
        break;
      }
    case PHOTOPARAM_QUERY_USERPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryUserPhotoListPhotoCmd, rev);
        GuildScene* pScene = dynamic_cast<GuildScene*>(getScene());
        if (pScene != nullptr)
        {
          pScene->sendSelfPhoto(this);
          break;
        }
        m_oPhoto.queryUserPhotoList();
        break;
      }
      break;
    case PHOTOPARAM_ADD_MD5:
      {
        PARSE_CMD_PROTOBUF(AddMd5PhotoCmd, rev);
        m_oUserSceneData.addPhotoMd5(rev.md5());
      }
      break;
    case PHOTOPARAM_REMOVE_MD5:
      {
        PARSE_CMD_PROTOBUF(RemoveMd5PhotoCmd, rev);
        m_oUserSceneData.removePhotoMd5(rev.md5());
      }
      break;
    default:
      return false;
  }

  return true;
}

bool SceneUser::doSceneUserFoodCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param) {
  case FOODPARAM_QUERY_FOOD_DATA:
  {
    m_oSceneFood.sendFoodData();
    break;
  }
  case FOODPARAM_PREPARECOOK:
  {
    PARSE_CMD_PROTOBUF(PrepareCook, rev);
    m_oSceneFood.prepareCook(rev);
    break;
  }
  case FOODPARAM_SELECT_COOKTYPE:
  {
    PARSE_CMD_PROTOBUF(SelectCookType, rev);
    m_oSceneFood.selectCookType(rev.cooktype());
    break;
  }
  case FOODPARAM_START_COOK:
  {
    PARSE_CMD_PROTOBUF(StartCook, rev);
    m_oSceneFood.startCook(rev);
    break;
  }
  case FOODPARAM_PUT_FOOD:
  {
    PARSE_CMD_PROTOBUF(PutFood, rev);
    m_oSceneFood.putFood(rev);
    break;
  }
  case FOODPARAM_EDIT_FOOD_POWER:
  {
    PARSE_CMD_PROTOBUF(EditFoodPower, rev);
    m_oSceneFood.editFoodPower(rev);
    break;
  }
  case FOODPARAM_QUERY_FOOD_NPC_INFO:
  {
    PARSE_CMD_PROTOBUF(QueryFoodNpcInfo, rev);
    m_oSceneFood.queryFoodNpcInfo(rev);
    break;
  }
  case FOODPARAM_START_EAT:
  {
    PARSE_CMD_PROTOBUF(StartEat, rev);
    m_oSceneFood.startEat(rev);
    break;
  }
  case FOODPARAM_CLICK_FOOD_MANUAL_DATA:
  {
    PARSE_CMD_PROTOBUF(ClickFoodManualData, rev);
    m_oSceneFood.clickManual(rev.type(), rev.itemid());
    break;
  }
  default:
    return false;
  }

  return true;
}

bool SceneUser::doSceneUserTutorCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param) {
  case TUTORPARAM_TASK_QUERY:
  {
    PARSE_CMD_PROTOBUF(TutorTaskQueryCmd, rev);
    m_oTutorTask.queryStudentTask(rev);
    break;
  }
  case TUTORPARAM_TASK_TEACHER_REWARD:
  {
    PARSE_CMD_PROTOBUF(TutorTaskTeacherRewardCmd, rev);
    if (m_oTutorTask.getTutorReward(rev.charid(), rev.taskid()))
      sendCmdToMe(buf, len);
    break;
  }
  case TUTORPARAM_GET_GROW_REWARD:
  {
    PARSE_CMD_PROTOBUF(TutorGetGrowRewardCmd, rev);
    if (m_oTutorTask.getGrowReward())
      sendCmdToMe(buf, len);
    break;
  }
  default:
    return false;
  }

  return true;
}

bool SceneUser::doSceneUserBeingCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param)
  {
  case BEINGPARAM_SKILL_QUERY:
  {
    m_oUserBeing.sendSkillData(true);
    break;
  }
  case BEINGPARAM_SKILL_LEVELUP:
  {
    PARSE_CMD_PROTOBUF(BeingSkillLevelUp, cmd);
    m_oUserBeing.skillLevelUp(cmd);
    break;
  }
  case BEINGPARAM_BEING_INFO_QUERY:
  {
    PARSE_CMD_PROTOBUF(BeingInfoQuery, cmd);
    m_oUserBeing.sendBeingInfo();
    break;
  }
  case BEINGPARAM_BEING_SWITCH_STATE:
  {
    PARSE_CMD_PROTOBUF(BeingSwitchState, cmd);
    m_oUserBeing.handleBeingSwitchState(cmd);
    break;
  }
  case BEINGPARAM_BEING_OFF:
  {
    PARSE_CMD_PROTOBUF(BeingOffCmd, cmd);
    m_oUserBeing.handleBeingOffCmd(cmd);
    break;
  }
  case BEINGPARAM_BEING_CHANGE_BODY:
  {
    PARSE_CMD_PROTOBUF(ChangeBodyBeingCmd, cmd);
    m_oUserBeing.changeBody(cmd.beingid(), cmd.body());
  }
  break;
  default:
    return false;
  }

  return true;
}

bool SceneUser::doSessionAuthorizeCmd(const Cmd::UserCmd* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
  case SYNC_REAL_AUTHORIZE_TO_SESSION:
  {
    PARSE_CMD_PROTOBUF(SyncRealAuthorizeToSession, rev);
    setRealAuthorized(rev.authorized());
    XLOG << "[实名认证] accid" << accid << "charid" << id << "name" << name << m_bRealAuthorized << XEND;
  }
  break;
  }
  return true;
}

bool SceneUser::doSessionUserGuildCmd(const Cmd::UserCmd* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case GUILDPARAM_TREASURE_ACTION:
      {
        PARSE_CMD_PROTOBUF(TreasureActionGuildCmd, rev);
        GuildScene* pScene = dynamic_cast<GuildScene*>(getScene());
        if (pScene != nullptr)
          pScene->treasureAction(this, rev);
      }
      break;
    default:
      return false;
  }

  return true;
}


bool SceneUser::doSceneWeddingCmd(const Cmd::UserCmd* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
  case WEDDINGCPARAM_CHECK_CAN_RESERVE:
  {
    PARSE_CMD_PROTOBUF(CheckCanReserveCCmd, rev);
    bool ret = m_oUserWedding.checkCanReserve(rev.charid2());
    rev.set_success(ret);
    PROTOBUF(rev, send, len);
    sendCmdToMe(send, len);
    return true;
  }
  break;
  case WEDDINGCPARAM_REQ_WEDDINGDATE_LIST:
  {
    PARSE_CMD_PROTOBUF(ReqWeddingDateListCCmd, rev);
    if (m_oUserWedding.hasTicket())
      rev.set_use_ticket(true);
    else
      rev.set_use_ticket(false);
    PROTOBUF(rev, send, len);
    thisServer->sendUserCmdToWeddingServer(id, name, send, len);
    return true;
  }
  break;
  case WEDDINGCPARAM_RESERVE_WEDDINGDATE:
  {
    PARSE_CMD_PROTOBUF(ReserveWeddingDateCCmd, rev);
    m_oUserWedding.reqReserve(rev);
    return true;
  }
  break;
  case WEDDINGCPARAM_REPLY_RESERVE_WEDDINGDATE:
  {
    PARSE_CMD_PROTOBUF(ReplyReserveWeddingDateCCmd, rev);
    m_oUserWedding.reserveInviteeReply(rev);
  
    return true;   
  }
  break;
  case WEDDINGCPARAM_WEDDING_EVENT_MSG:
  {
    PARSE_CMD_PROTOBUF(WeddingEventMsgCCmd, rev);
    m_oUserWedding.weddingEvent(rev);
    return true;
  }
  break;
  case WEDDINGCPARAM_INVITE_WEDDING:
  {
    WeddingScene* pScene = dynamic_cast<WeddingScene*> (getScene());
    if (pScene == nullptr)
      return true;
    if (m_oHands.has() == false || m_oHands.isMaster() == false || m_oHands.isInWait())
      return true;

    SceneUser* pOther = m_oHands.getOther();
    if (pOther == nullptr)
      return true;
    // check wedding user
    const WeddingInfo& info = pScene->getWeddingInfo();
    if ((this->id != info.charid1() && this->id != info.charid2()) || (pOther->id != info.charid1() && pOther->id != info.charid2()))
      return true;

    PARSE_CMD_PROTOBUF(InviteBeginWeddingCCmd, rev);
    rev.set_masterid(this->id);
    rev.set_name(this->name);
    rev.set_tocharid(pOther->id);
    PROTOBUF(rev, send, len);

    pOther->sendCmdToMe(send, len);
    return true;
  }
  break;
  case WEDDINGCPARAM_REPLY_WEDDING:
  {
    PARSE_CMD_PROTOBUF(ReplyBeginWeddingCCmd, rev);

    WeddingScene* pScene = dynamic_cast<WeddingScene*> (getScene());
    if (pScene == nullptr)
      return true;
    const WeddingInfo& info = pScene->getWeddingInfo();
    SceneUser* pMaster = SceneUserManager::getMe().getUserByID(rev.masterid());
    if (pMaster == nullptr || pMaster->getScene() != getScene())
      return true;
    if ((this->id != info.charid1() && this->id != info.charid2()) || (pMaster->id != info.charid1() && pMaster->id != info.charid2()) || (pMaster->id == this->id))
      return true;
    pScene->onInviteOk(pMaster->id, this->id);
    return true;
  }
  break;
  case WEDDINGCPARAM_GOTO_WEDDINGPOS:
  {
    WeddingScene* pScene = dynamic_cast<WeddingScene*> (getScene());
    if (pScene == nullptr)
      return true;
    pScene->onUserInPos(this);
  }
  break;
  case WEDDINGCPARAM_ANSWER:
  {
    WeddingScene* pScene = dynamic_cast<WeddingScene*> (getScene());
    if (pScene == nullptr)
      return true;
    PARSE_CMD_PROTOBUF(AnswerWeddingCCmd, rev);
    pScene->onReceiveAnswer(this, rev.questionid(), rev.answer());
  }
  break;
  case WEDDINGCPARAM_QUESTION_SWITCH:
  {
    WeddingScene* pScene = dynamic_cast<WeddingScene*> (getScene());
    if (pScene == nullptr)
      return true;
    PARSE_CMD_PROTOBUF(WeddingSwitchQuestionCCmd, rev);
    if (rev.onoff() == false)
      pScene->onUserQuitQuestion(this);
  }
  break;
  case WEDDINGCPARAM_ENTER_ROLLER_COASTER:
  {
    getUserWedding().enterRollterCoaster();
  }
  break;
  case WEDDINGCPARAM_DIVORCE_ROLLER_COASTER_INVITE:
  {
    PARSE_CMD_PROTOBUF(DivorceRollerCoasterInviteCCmd, rev);
    return m_oUserWedding.divorceRollerCoasterInvite(rev);
  }
  break;
  case WEDDINGCPARAM_DIVORCE_ROLLER_COASTER_REPLY:
  {
    PARSE_CMD_PROTOBUF(DivorceRollerCoasterReplyCCmd, rev);
    return m_oUserWedding.divorceRollerCoasterReply(rev);
  }
  break;
  case WEDDINGCPARAM_REQ_DIVORCE:
  {
    PARSE_CMD_PROTOBUF(ReqDivorceCCmd, rev);
    return m_oUserWedding.reqDivorce(rev);
  }
  break;
  case WEDDINGCPARAM_ENTER_WEDDINGMAP:
  {
    const WeddingInfo& wedinfo = SceneWeddingMgr::getMe().getWeddingInfo();
    if (wedinfo.id() == 0)
      return true;
    if (wedinfo.charid1() != this->id && wedinfo.charid2() != this->id)
    {
      if (getPackage().hasWeddingManual(wedinfo.id()) == false)
        return true;
    }
    CreateDMapParams param;
    param.qwCharID = this->id;
    param.dwRaidID = MiscConfig::getMe().getWeddingMiscCFG().dwWeddingRaidID;
    param.m_qwRoomId = wedinfo.id();
    SceneManager::getMe().createDScene(param);
    XLOG << "[婚礼-进入], 玩家进入婚礼副本,玩家:" << this->name << this->id << "婚礼id:" << wedinfo.id() << XEND;
  }
  break;
  case WEDDINGCPARAM_MISSYOU_INVITE:
  {
    UserWedding& rWedding = getUserWedding();
    QWORD qwParnterID = rWedding.getWeddingParnter();
    if (qwParnterID == 0)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "邀请配偶回到身边失败,未有配偶" << XEND;
      break;
    }
    Scene* pScene = getScene();
    if (pScene == nullptr)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "邀请配偶回到身边失败,自己不在场景中" << XEND;
      break;
    }
    DScene* pDScene = dynamic_cast<DScene*>(pScene);
    if (pDScene != nullptr)
    {
      MsgManager::sendMsg(id, 9649);
      break;
    }

    SceneUser* pParnter = SceneUserManager::getMe().getUserByID(qwParnterID);
    if (pParnter == nullptr)
    {
      GCharReader oChar(thisServer->getRegionID(), qwParnterID);
      if (oChar.getByWedding() == false)
      {
        XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "邀请配偶回到身边失败,获取配偶redis数据失败" << XEND;
        break;
      }
      if (oChar.getOnlineTime() < oChar.getOfflineTime())
      {
        MsgManager::sendMsg(id, 9653);
        break;
      }
    }

    SceneFighter* pFighter = getFighter(EPROFESSION_NOVICE);
    if (pFighter == nullptr)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "邀请配偶回到身边失败,未有初心者武将" << XEND;
      break;
    }
    if (pFighter->getSkill().getSkillLv(SKILL_WEDDING_MISSYOU) == 0)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "邀请配偶回到身边失败,未学习" << SKILL_WEDDING_MISSYOU << "技能" << XEND;
      break;
    }

    xLuaData oData;
    oData.setData("id", SKILL_WEDDING_MISSYOU);
    GMCommandRuler::useskill(this, oData);
  }
  break;
  case WEDDINGCPARAM_MISSYOU_REPLY:
  {
    if(getDressUp().getDressUpStatus() != 0)
      return false;

    PARSE_CMD_PROTOBUF(MisccyouReplyWedCCmd, rev);
    if (rev.agree() == false)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "拒绝了配偶回到身边邀请" << XEND;
      return false;
    }

    const ParnterInfo& rInfo = getUserWedding().getMissInfo();
    if (rInfo.x() == 0 || rInfo.y() == 0 || rInfo.z() == 0)
    {
      XERR << "[婚姻-好想你]" << accid << id << getProfession() << name << "答应配偶回到身边邀请,失败了,配偶未发起过邀请" << XEND;
      return false;
    }

    xLuaData oData;
    oData.setData("mapid", rInfo.mapid());
    oData.setData("zoneid", thisServer->getZoneID() == rInfo.zoneid() ? 0 : rInfo.zoneid());
    oData.setData("x", rInfo.x());
    oData.setData("y", rInfo.y());
    oData.setData("z", rInfo.z());

    if (GMCommandRuler::gocity(this, oData))
    {
      if (rInfo.zoneid() != thisServer->getZoneID())
        m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff);
    }

    XLOG << "[婚姻-好想你]" << accid << id << getProfession() << name << "答应配偶回到身边邀请,成功回到配偶身边" << XEND;
  }
  break;
  case WEDDINGCPARAM_CARRIER:
  {
    if (SceneWeddingMgr::getMe().isCurWeddingUser(this->id) == false)
      return true;
    if (SceneWeddingMgr::getMe().hasServiceID(EWEDDINGPLANTYPE_CRUISE) == false)
    {
      MsgManager::sendMsg(this->id, 9651);
      return true;
    }
    DWORD carrierid = MiscConfig::getMe().getWeddingMiscCFG().dwWeddingCarrierID;
    DWORD carrierline = MiscConfig::getMe().getWeddingMiscCFG().dwWeddingCarrierLine;
    m_oCarrier.create(carrierid, carrierline);
    XLOG << "[婚礼—载具], 玩家发起载具, 玩家:" << name << id << "载具:" << carrierid << XEND;
  }
  break;
  }
  return true;
}


bool SceneUser::doSceneUserPveCardCmd(const Cmd::UserCmd* buf, WORD len)
{
  const Cmd::UserCmd* cmd = (const Cmd::UserCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch(cmd->param)
  {
    case EPVE_QUERY_ALL_CARD_CMD:
      {
        /*PveCardScene* pCScene = dynamic_cast<PveCardScene*> (getScene());
        if (pCScene == nullptr)
          return false;
        pCScene->queryAllCardInfo(this);*/
        QueryCardInfoCmd message;
        PveCardConfig::getMe().formatCardInfo(message);
        PROTOBUF(message, send, len);
        sendCmdToMe(send, len);
      }
      break;
    case EPVE_SELECT_CARD_CMD:
      {
        PveCardScene* pCScene = dynamic_cast<PveCardScene*> (getScene());
        if (pCScene == nullptr)
          return false;
        if (getTeamLeaderID() != this->id)
          return false;
        PARSE_CMD_PROTOBUF(SelectPveCardCmd, rev);
        pCScene->selectCard(this, rev.index());
      }
      break;
    case EPVE_BEGIN_FIRE_CMD:
      {
        PveCardScene* pCScene = dynamic_cast<PveCardScene*> (getScene());
        if (pCScene == nullptr)
          return false;
        if (getTeamLeaderID() != this->id)
          return false;
        pCScene->beginPlayCard(this);
      }
      break;
    case EPVE_ENTER_RAID_CMD: // teamserver -> sceneserver, not client -> scene
      {
        PARSE_CMD_PROTOBUF(EnterPveCardCmd, rev);
        const SPveRaidCFG* pPveRaidCFG = PveCardConfig::getMe().getPveRaidCFGByID(rev.configid());
        if (pPveRaidCFG == nullptr)
        {
          XERR << "[Pve-卡牌副本], id错误:" << rev.configid() << "玩家:" << name << id << XEND;
          break;
        }
        gomap(pPveRaidCFG->dwRaidID, GoMapType::GM);
        return true;
      }
      break;
    default:
      break;
  }
  return true;
}

bool SceneUser::doSceneUserTeamRaidCmd(const Cmd::UserCmd* buf, WORD len)
{
  switch(buf->param)
  {
    case TEAMRAIDPARAM_ENTER:
      {
        PARSE_CMD_PROTOBUF(TeamRaidEnterCmd, rev);
        DWORD dwRaidID = MiscConfig::getMe().getRaidByType(static_cast<ERaidType>(rev.raid_type()));
        const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(dwRaidID);
        if(pRaidCFG == nullptr)
        {
          XERR << "[玩家-队伍副本]" << accid << id << getProfession() << name << "收到社交服队伍副本进入请求" << dwRaidID << "未找到相关配置"<< XEND;
          break;
        }
        m_oZone.gomap(rev.zoneid(), dwRaidID, GoMapType::GM);
        XLOG << "[玩家-队伍副本]" << accid << id << getProfession() << name << "收到社交服队伍副本进入请求" << dwRaidID  << "type: " << rev.raid_type()
          << "zoneid: " << rev.zoneid() << " 准备进入"<< XEND;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool SceneUser::doSceneUserBossCmd(const Cmd::UserCmd* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;

  switch (buf->param)
  {
    case BOSS_STEP_SYNC:
      {
        Scene* pScene = getScene();
        if (pScene == nullptr)
        {
          XERR << "[Boss消息-接受]" << accid << id << getProfession() << name << "处理消息" << BOSS_STEP_SYNC << "失败,玩家未包含正确的场景" << XEND;
          break;
        }
        PARSE_CMD_PROTOBUF(StepSyncBossCmd, rev);
        BossMgr::getMe().runStep(pScene->getMapID(), this);
      }
      break;
    default:
      return false;
  }

  return true;
}


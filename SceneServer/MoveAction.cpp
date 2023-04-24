#include "MoveAction.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "UserEvent.h"
#include "SceneManager.h"
#include "SceneNpcManager.h"
#include "DetourCommon.h"
#include "MsgManager.h"
#include "BossCmd.pb.h"
#include "GMCommandRuler.h"
#include <string>
#include "DScene.h"
#include "CommonConfig.h"
#include "GuildRaidConfig.h"
#include "GuildCityManager.h"
#include "BaseConfig.h"
#include "SceneWeddingMgr.h"

MoveAction::MoveAction(xSceneEntryDynamic* entry):
  m_pEntry(entry),
  m_dwExitID(0)
{
}

MoveAction::~MoveAction()
{
}

void MoveAction::move(xPos pos)
{
  if (!m_pEntry->getScene() || m_pEntry->isAlive() == false)
    return;

  float speed = m_pEntry->getMoveSpeed();
  if (speed <= 0) return;

  float distance = getDistance(pos, m_pEntry->getPos());
  speed = MOVE_SPEED_BASE * distance / speed;

  m_pEntry->setPos(pos);

  if (m_pEntry->getOldPosI()!=m_pEntry->getPosI())
  {
    m_pEntry->getScene()->changeScreen(m_pEntry);
  }

  switch (m_pEntry->getEntryType())
  {
    case SCENE_ENTRY_USER:
      {
 //       XLOG("[实际移动],%llu,%s,移动,(%f,%f,%f),speed:%u,dis:%f,cur:%llu,next:%llu",
   //         m_pEntry->id, m_pEntry->name, pos.x, pos.y, pos.z, speed, distance, xTime::getCurUSec() / 1000, getNextActionTime());
        ((SceneUser *)m_pEntry)->getEvent().onMove(distance);
        checkExit();
        if (((SceneUser *)m_pEntry)->bOpenMoveTrack)
        {
          NpcDefine def;
          def.setID(10001);
          def.setPos(pos);
          SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, m_pEntry->getScene());
          if (npc)
          {
            npc->setClearTime(now() + 2);
          }
        }
        //speed *= 0.9f;
#ifdef _LX_DEBUG
        /*
        NpcDefine def;
        def.setID(10001);
        def.setPos(pos);
        SceneNpc *npc = SceneNpcManager::getMe().createOneNpc(m_pEntry->getScene(), def);
        if (npc)
        {
          npc->m_dwSetClearTime = now() + 2;
        }
        Cmd::EffectUserCmd cmd;
        Cmd::ScenePos *spos = cmd.mutable_pos();
        spos->set_x(pos.getX());
        spos->set_y(pos.getY());
        spos->set_z(pos.getZ());
        cmd.set_effect("Skill/DoubleStrafe");
        PROTOBUF(cmd, send, len);
        m_pEntry->getScene()->sendCmdToNine(pos, send, len);
        */
#endif
      }
      break;
    case SCENE_ENTRY_NPC:
      {
        m_pEntry->setAction(0);
        /*if (((SceneNpc *)m_pEntry)->define.getID() == 10020)
        {
          NpcDefine def;
          def.setID(1361);
          def.setPos(pos);
          SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, m_pEntry->getScene());
          if (npc)
          {
            npc->setClearTime(now() + 3);
          }
        }
        */
#ifdef _LX_DEBUG
//        if (m_pEntry->getScene()->id != 2) break;
 //       if (((SceneNpc *)m_pEntry)->m_ai.m_blAttackState)
        /*
        if (((SceneNpc *)m_pEntry)->define.m_oVar.m_qwFollowerID)
        {
          XLOG("[NPC移动],%llu,%s,移动,%f,%f,%f,speed:%f,dis:%f,cur:%llu,next:%llu",
              m_pEntry->id, m_pEntry->name, pos.x, pos.y, pos.z, speed, distance, xTime::getCurUSec() / 1000, getNextActionTime());

          NpcDefine def;
          def.id = ((SceneNpc *)m_pEntry)->base->dwID;
          def.m_oPos = pos;
          SceneNpc *npc = SceneNpcManager::getMe().createOneNpc(m_pEntry->getScene(), def);
          if (npc)
          {
            npc->m_dwSetClearTime = now() + 2;
          }
        }
        */
#endif
      }
      break;
    default:
      break;
  }
  addActionTime((DWORD)speed);
}

void MoveAction::addActionTime(DWORD speed)
{
  QWORD cur = xTime::getCurUSec() / 1000;
  if (getNextActionTime())
    addNextActionTime(speed);
  else
    setNextActionTime(cur + speed);
  if (getNextActionTime() + 2000 < cur)
    setNextActionTime(cur + speed);
}

void MoveAction::addBeHitDelayTime(DWORD speed)
{
  QWORD cur = xTime::getCurMSec();
  if (m_oPairBegin2EndHitDelay.first < cur && m_oPairBegin2EndHitDelay.second > cur)
  {
    DWORD delta = m_oPairBegin2EndHitDelay.second - cur;
    if (speed > delta)
    {
      addActionTime(speed - delta);
    }
    else
      return;
  }
  else
  {
    addActionTime(speed);
  }
  m_oPairBegin2EndHitDelay.first = cur;
  m_oPairBegin2EndHitDelay.second = cur + speed;
}

void MoveAction::heartbeat(QWORD curMSec)
{
  QWORD lasttime = QWORD_MAX;
  while(lasttime != m_qwNextActionTime && checkActionTime(curMSec))
  {
    lasttime = m_qwNextActionTime;
    if (!action(curMSec))
      break;
  }
}

bool MoveAction::action(QWORD curMSec)
{
  if (empty())
  {
    setNextActionTime(curMSec);
    return false;
  }
  if (!m_pEntry->getScene()) return false;

  if (m_pEntry->isAttrCanMove() == false || m_pEntry->getMoveSpeed() <= 0)
    return false;

  if (m_fixpath.empty())
  {
    xPos p = m_path.front();
    m_path.pop_front();

    setStraightPoint(p);
  }

  while (!m_fixpath.empty())
  {
    xPos fixp = m_fixpath.front();
    m_fixpath.pop_front();

    if (fixp==m_pEntry->getPos()) continue;

    move(fixp);

    if (m_fixpath.empty())
    {
      m_pEntry->m_oSkillProcessor.timer(curMSec);
    }
    break;
  }
  return true;
}

bool MoveAction::setStraightPoint(xPos p)
{
  float speed = m_pEntry->getMoveSpeed();
  if (speed <= 0)
  {
    XLOG << "[客户端移动失败], 当前移动速度为0" << XEND;
    return false;
  }
  if (m_pEntry->isAttrCanMove() == false)
    return false;

  if (!m_pEntry->getScene())
  {
    XLOG << "[客户端移动失败],没有场景,目标点:(" << p.x << p.y << p.z << ")" << XEND;
    return false;
  }
//  m_path.clear();
  m_fixpath.clear();

  /*
#ifdef _LX_DEBUG
  if (m_pEntry->getScene()->findingPath(m_pEntry->getPos(), p, m_fixpath, TOOLMODE_PATHFIND_FOLLOW))
  {
    m_dest = p;
    sendPos(m_dest);
    return true;
  }
  else
  {
    XLOG << "[客户端移动失败],寻路失败,目标点:(" << p.x << p.y << p.z << ")" << XEND;
    m_dest = m_pEntry->getPos();
    sendPos(m_dest);
    return false;
  }
  return true;
#endif
*/

  xPos start = m_pEntry->getPos();
  xPos end = p;
  xPos fix;

  float dx = end.x - start.x;
  float dy = end.y - start.y;
  float dz = end.z - start.z;
  float dist = getDistance(start, end);
  if (dist <= 1.0f)
  {
    m_fixpath.push_back(end);
  }
  else
  {
    for (int i=1; i<=(int)dist; ++i)
    {
      fix.x = dx * i / dist + start.x;
      fix.z = dz * i / dist + start.z;
      fix.y = dy * i / dist + start.y;//end.y;
      m_fixpath.push_back(fix);
    }
    m_fixpath.push_back(end);
  }

  m_dest = p;
  sendPos(m_dest);

  return true;

}

xPos MoveAction::getStraightPoint()
{
  if (empty()) return xPos(0,0,0);

  if (!m_fixpath.empty())
  {
    return m_fixpath.back();
  }
  else
  {
    return m_path.front();
  }
}

bool MoveAction::setFinalPoint(xPos p)
{
  if (!m_pEntry->getScene() || m_pEntry->isAlive() == false) return false;

  // 中Buff等导致不能移动, 重置位置后,结束buff前, 不处理前端移动消息(移动队列非空, 导致技能判断非法, ex:狂暴之怒)
  if (m_pEntry->isAttrCanMove() == false || m_pEntry->getMoveSpeed() <= 0)
    return true;

  m_path.clear();
  m_fixpath.clear();
  if (m_pEntry->getScene()->findingPath(m_pEntry->getPos(), p, m_path))
  {
    m_lastDestFinal = m_destFinal;
    m_destFinal = p;
    return true;
  }
  else
  {
    m_lastDestFinal = m_destFinal;
    m_destFinal = m_pEntry->getPos();
    sendPos(m_destFinal);
  }
  return false;
}

void MoveAction::clearPath()
{
  m_path.clear();
  m_fixpath.clear();

  m_dest = m_pEntry->getPos();
  sendPos(m_dest);

  m_lastDestFinal = m_destFinal;
  m_destFinal = m_pEntry->getPos();
}

void MoveAction::sendPos(xPos p)
{
  if (m_pEntry == nullptr || m_pEntry->getScene() == nullptr)
    return;

  RetMoveUserCmd cmd;
  cmd.set_charid(m_pEntry->id);
  ScenePos *pScenePos = cmd.mutable_pos();
  pScenePos->set_x(p.getX());
  pScenePos->set_y(p.getY());
  pScenePos->set_z(p.getZ());
  PROTOBUF(cmd, send, len);
  m_pEntry->sendCmdToNine(send, len);
}

void MoveAction::checkExit()
{
  if (m_dwExitID)
  {
    if (SCENE_ENTRY_USER!=m_pEntry->getEntryType()) return;
    SceneUser *pUser = (SceneUser *)m_pEntry;
    if (!checkDistance(m_oExitPos, m_pEntry->getPos(), 10.0f)) return;

    Scene *pScene = m_pEntry->getScene();
    if (!pScene) return;

    const SceneObject *pObject = pScene->getSceneObject();
    if (!pObject) return;
    const ExitPoint* pPoint = pObject->getExitPoint(m_dwExitID);
    if (pPoint == nullptr)
    {
      if (!pScene->m_oImages.isImageScene())
      {
        stringstream sstr;
        sstr << "没有ep " << m_dwExitID;
        MsgManager::sendMsg(pUser->id, 10, MsgParams(sstr.str()));
      }
      m_dwExitID = 0;
      m_oExitPos.clear();
      return;
    }
    if (!pPoint->isVisible(pUser))
    {
      stringstream sstr;
      sstr << "ep " << m_dwExitID << " 不可见";
      MsgManager::sendMsg(pUser->id, 10, MsgParams(sstr.str()));
      m_dwExitID = 0;
      m_oExitPos.clear();
      return;
    }
    if (pPoint->m_dwQuestID)
    {
      TPtrQuestItem pItem = pUser->getQuest().getQuest(pPoint->m_dwQuestID);
      if (pItem != nullptr)
      {
        if (pUser->getQuest().abandonGroup(pPoint->m_dwQuestID) == true)
          pUser->getQuest().acceptQuest(pPoint->m_dwQuestID);
      }
      else
      {
        pUser->getQuest().acceptQuest(pPoint->m_dwQuestID);
      }
      m_dwExitID = 0;
      m_oExitPos.clear();
      return;
    }

    bool bNormalExit = true;
    if (TowerConfig::getMe().isTower(pScene->getMapID()) == true)
    {
      bNormalExit = false;
      DScene* pQuestScene = dynamic_cast<DScene*>(pScene);
      if (pQuestScene != nullptr)
      {
        xLuaData data;
        TowerKillAllPhase oPhase(data);
        if (oPhase.exec(pQuestScene) == false)
          MsgManager::sendMsg(pUser->id, 1307);
      }
    }
    else if (pScene->getSceneType() == SCENE_TYPE_GUILD_RAID && pPoint->m_intNextMapID != 0)
    {
      bNormalExit = false;
      GuildRaidScene* pGScene = dynamic_cast<GuildRaidScene*> (pScene);
      if (pGScene)
        pGScene->checkExit(pPoint);
    }

    if (bNormalExit)
    {
      if (pPoint->check(m_oExitPos))
      {
        doExit(*pPoint);
        m_dwExitID = 0;
        m_oExitPos.clear();
      }
    }

    m_dwExitID = 0;
  }
}

void MoveAction::doExit(const ExitPoint &point)
{
  if (SCENE_ENTRY_USER!=m_pEntry->getEntryType()) return;
  SceneUser *pUser = (SceneUser *)m_pEntry;

  Scene *pScene = m_pEntry->getScene();
  if (!pScene) return;

  // buff限制
  if (pUser->m_oBuff.haveBuffType(EBUFFTYPE_NOMAPEXIT))
    return;

  // 地图传送分支屏蔽
  if (MiscConfig::getMe().getBranchForbid().isForbid(point.m_intNextMapID))
  {
    DWORD branch = BaseConfig::getMe().getBranch();
    DWORD usebranch = 0;
    switch(branch)
    {
      case BRANCH_DEBUG:
        usebranch = 1;
        break;
      case BRANCH_TF:
        usebranch = 4;
        break;
      case BRANCH_PUBLISH:
        usebranch = 8;
        break;
    }
    if (MiscConfig::getMe().getBranchForbid().isForbid(point.m_intNextMapID, usebranch))
    {
      XERR << "[地图-分支屏蔽], 当前分支:" << branch << "不可传送:" << point.m_intNextMapID << XEND;
      return;
    }
  }

  DScene* pDScene = dynamic_cast<DScene*>(pScene);
  if (pDScene && pDScene->getRaidType() == ERAIDTYPE_GUILD)
  {
    // 从公户领地进入据点
    if (point.m_intNextMapID == -2)
    {
      const GuildCityInfo* pCityInfo = GuildCityManager::getMe().getCityInfoByGuild(pUser->getGuild().id());
      if (pCityInfo == nullptr)
      {
        // send msg
        MsgManager::sendMsg(pUser->id, 2208);
        return;
      }
      const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFG(pCityInfo->flag());
      if (pCityCFG == nullptr)
      {
        XERR << "[公会据点], 找不到据点配置, 据点" << pCityInfo->flag() << XEND;
        return;
      }
      for (auto &s : pCityCFG->setGroupRaids)
      {
        const SceneBase *nextbase = SceneManager::getMe().getDataByID(s);
        if (nextbase)
        {
          const SceneObject *pNextObject = nextbase->getSceneObject(s);
          if (pNextObject != nullptr)
          {
            const SBornPoint* pPos = pNextObject->getSBornPoint(point.m_dwNextBornPoint);
            if (pPos != nullptr)
            {
              CreateDMapParams param;
              param.qwCharID = pUser->id;
              param.m_dwGuildRaidIndex = pCityCFG->dwCityID;
              param.dwRaidID = s;
              param.m_oEnterPos = pPos->oPos;
              if (pPos->dwRange)
                nextbase->getRandPos(pPos->oPos, pPos->dwRange, param.m_oEnterPos);
              SceneManager::getMe().createDScene(param);
              return;
            }
          }
        }
      }
      return;
    }
    DWORD mapid = pUser->getUserMap().getLastStaticMapID();
    const xPos& pos= pUser->getUserMap().getLastStaticMapPos();
    pUser->gomap(mapid, GoMapType::ExitPoint, pos);
    return;
  }

  if (pScene->getSceneType() == SCENE_TYPE_GUILD_FIRE)
  {
    GuildFireScene* pGFScene = dynamic_cast<GuildFireScene*> (pScene);
    if (pGFScene == nullptr)
      return;
    DWORD cityid = pGFScene->getCityID();
    if (MapConfig::getMe().isRaidMap(point.m_intNextMapID) == true)
    {
      // 仅防守方可以传送的ep
      const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFG(cityid);
      if (pCityCFG && pCityCFG->isDefLimitEp(pScene->getMapID(), point.m_dwExitID))
      {
        if (pGFScene->getDefenseGuildID() == 0 || pGFScene->getDefenseGuildID() != pUser->getGuild().id())
          return;
      }

      CreateDMapParams param;
      const SceneBase *nextbase = SceneManager::getMe().getDataByID(point.m_intNextMapID);
      if (nextbase)
      {
        const SceneObject *pNextObject = nextbase->getSceneObject(point.m_intNextMapID);
        if (pNextObject != nullptr)
        {
          const SBornPoint* pPos = pNextObject->getSBornPoint(point.m_dwNextBornPoint);
          if (pPos)
          {
            param.m_oEnterPos = pPos->oPos;
            if (pPos->dwRange)
              nextbase->getRandPos(pPos->oPos, pPos->dwRange, param.m_oEnterPos);
          }
        }
      }
      param.qwCharID = pUser->id;
      param.m_dwGuildRaidIndex = cityid;
      param.dwRaidID = point.m_intNextMapID;
      SceneManager::getMe().createDScene(param);
      return;
    }
  }

  if (point.m_intNextMapID > 0)
  {
    if (MapConfig::getMe().isRaidMap(point.m_intNextMapID) == true)
    {
      CreateDMapParams param;
      param.qwCharID = pUser->id;
      param.dwRaidID = point.m_intNextMapID;
      const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(point.m_intNextMapID);
      if (pBase)
      {
        switch (pBase->eRestrict)
        {
          case ERAIDRESTRICT_GUILD_FIRE:
            {
              const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFGByRaid(point.m_intNextMapID);
              if (pCityCFG == nullptr)
                return;
              if (GuildCityManager::getMe().isInFire() == false || GuildCityManager::getMe().isCityInFire(pCityCFG->dwCityID) == false)
              {
                const GuildCityInfo* pCityInfo = GuildCityManager::getMe().getCityInfo(pCityCFG->dwCityID);
                if (pCityInfo == nullptr || pCityInfo->id() != pUser->getGuild().id() || pCityInfo->id() == 0)
                {
                  // 攻方不可进, msg提示
                  return;
                }
              }
              param.m_dwGuildRaidIndex = pCityCFG->dwCityID;
            }
            break;
          case ERAIDRESTRICT_WEDDING:
            {
              const WeddingInfo& wedinfo = SceneWeddingMgr::getMe().getWeddingInfo();
              if (wedinfo.id() == 0)
                return;
              if (wedinfo.charid1() != pUser->id && wedinfo.charid2() != pUser->id)
              {
                if (pUser->getPackage().hasWeddingManual(wedinfo.id()) == false)
                  return;
              }
              param.m_qwRoomId = wedinfo.id();
            }
            break;
          default:
            break;
        }
      }

      SceneManager::getMe().createDScene(param);
      //pUser->createDMap(point.m_intNextMapID);
    }
    else
    {
      const SceneBase *nextbase = SceneManager::getMe().getDataByID(point.m_intNextMapID);
      if (nextbase)
      {
        const SceneObject *pNextObject = nextbase->getSceneObject(0);
        if (pNextObject != nullptr)
        {
          const SBornPoint* pPos = pNextObject->getSBornPoint(point.m_dwNextBornPoint);
          if (pPos != nullptr)
          {
            xPos pos = pPos->oPos;
            if (pPos->dwRange)
              nextbase->getRandPos(pPos->oPos, pPos->dwRange, pos);
            pUser->gomap(point.m_intNextMapID, GoMapType::ExitPoint, pos);
          }
          else
          {
            stringstream sstr;
            sstr << "目标地图 " << point.m_intNextMapID << " 没有出生点 " << point.m_dwNextBornPoint;
            MsgManager::sendMsg(pUser->id, 10, MsgParams(sstr.str()));
          }
        }
      }
      else
      {
        stringstream sstr;
        sstr << "找不到目标地图 " << point.m_intNextMapID;
        MsgManager::sendMsg(pUser->id, 10, MsgParams(sstr.str()));
      }
    }
  }
  else if (point.m_intNextMapID < 0)
  {
    // 公会随机副本
    GuildRaidScene* pGuildRaid = dynamic_cast<GuildRaidScene*> (pDScene);
    if (pGuildRaid)
    {
      const SGuildRaidInfo* pRaidInfo = GuildRaidConfig::getMe().getGuildRaidInfo(pGuildRaid->getMapIndex());
      if (pRaidInfo == nullptr)
      {
        XERR << "找不到目标地图, 当前地图:" << pGuildRaid->getRaidID()  << "地图索引:" << pGuildRaid->getMapIndex() << XEND;
      }
      else
      {
        const SGuildRaidLink* pNextMap = pRaidInfo->getNextMapInfo(point.m_dwExitID);
        if (pNextMap)
        {
          CreateDMapParams param;
          param.qwCharID = pUser->id;
          param.dwRaidID = pNextMap->dwNextMapID;
          param.m_dwGuildRaidIndex = pNextMap->getMapIndex();
          const SceneBase* nextbase = SceneManager::getMe().getDataByID(pNextMap->dwNextMapID);
          if (nextbase && nextbase->getSceneObject(pNextMap->dwNextMapID))
          {
            const xPos* pPos = nextbase->getSceneObject(pNextMap->dwNextMapID)->getBornPoint(pNextMap->dwNextBornPoint);
            if (pPos)
              param.m_oEnterPos = *pPos;
          }
          SceneManager::getMe().createDScene(param);
        }
      }
    }

    /*if (point.m_intNextMapID == -2)
    {
      // 公会据点
      CreateDMapParams param;
      param.qwCharID = pUser->id;
      const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFGByEP(pScene->getMapID(), point.m_dwExitID);
      if (pCityCFG != nullptr && pCityCFG->bOpen)
      {
        if (GuildCityManager::getMe().isInFire() == false || GuildCityManager::getMe().isCityInFire(pCityCFG->dwCityID))
        {
          const GuildCityInfo* pCityInfo = GuildCityManager::getMe().getCityInfo(pCityCFG->dwCityID);
          if (pCityInfo == nullptr || pCityInfo->id() != pUser->getGuild().id())
          {
            // 攻方不可进, msg提示
            return;
          }
        }

        param.dwRaidID = pCityCFG->dwRaidID;
        param.m_dwGuildRaidIndex = pCityCFG->dwCityID;
        SceneManager::getMe().createDScene(param);
      }
      else
      {
        XERR << "[公会据点-传送], 传送失败, 找不到地图:" << pScene->getMapID() << "Ep:" << point.m_dwExitID << "对应的据点配置" << XEND;
      }
    }
    */
  }
  else
  {
    const SceneObject *pNextObject = pScene->getSceneObject();
    if (pNextObject != nullptr)
    {
      const SBornPoint* pPos = pNextObject->getSBornPoint(point.m_dwNextBornPoint);
      if (pPos != nullptr)
      {
        xPos pos = pPos->oPos;
        if (pPos->dwRange)
          pScene->getRandPos(pPos->oPos, pPos->dwRange, pos);
        pUser->goTo(pos, true);
      }
      else
      {
        stringstream sstr;
        sstr << "本地图没有出生点 " << point.m_dwNextBornPoint;
        MsgManager::sendMsg(pUser->id, 10, MsgParams(sstr.str()));
      }
    }
  }
}

void MoveAction::stop()
{
  clear();
  sendPos(m_pEntry->getPos());
}

void MoveAction::stopAtonce()
{
  clear();
  Cmd::GoToUserCmd message;
  message.set_charid(m_pEntry->id);
  ScenePos *p = message.mutable_pos();
  xPos pos = m_pEntry->getPos();
  p->set_x(pos.getX());
  p->set_y(pos.getY());
  p->set_z(pos.getZ());
  PROTOBUF(message, send, len);
  m_pEntry->sendCmdToNine(send, len);
}

bool MoveAction::forceSync()
{
  if (empty())
    return true;

  stop();
  return true;

  if (m_pEntry->getEntryType() == SCENE_ENTRY_USER)
  {
    if (getDistance(m_destFinal, m_pEntry->getPos()) < CommonConfig::m_fSkillSyncDis * m_pEntry->getAttr(EATTRTYPE_MOVESPD))
    {
      m_pEntry->setScenePos(m_destFinal);
      stop();
      return true;
    }
    else
    {
      XDBG << "[位置-同步], 位置强制同步错误, 距离过远" << m_pEntry->name << m_pEntry->id << XEND;
      return false;
    }
  }
  else
  {
    stop();
  }
  return true;
}

void MoveAction::moveOnce()
{
  // 有效移动一次
  if (empty())
    return;
  if (m_pEntry->isAttrCanMove() == false || m_pEntry->getMoveSpeed() <= 0)
    return;

  xPos pos = m_pEntry->getPos();
  if (m_fixpath.empty())
  {
    while(!m_path.empty())
    {
      xPos p = m_path.front();
      m_path.pop_front();

      if (getXZDistance(p, pos) < 0.05)
        continue;

      setStraightPoint(p);
    }
  }

  while (!m_fixpath.empty())
  {
    xPos fixp = m_fixpath.front();
    m_fixpath.pop_front();

    if (fixp==m_pEntry->getPos()) continue;

    move(fixp);
    if (m_fixpath.size() == 1 && getXZDistance(m_fixpath.front(), fixp) < 0.5)
      m_fixpath.clear();
    break;
  }
}


#include "BossStep.h"
#include "BossAct.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "SceneManager.h"
#include "BossMgr.h"
#include "BossSCmd.pb.h"

// BossStep - base
bool BossBaseStep::init(xLuaData data)
{
  m_oParams.Clear();

  xLuaData& param = data.getMutableData("Params");
  param.toData(&m_oParams);
  return true;
}

bool BossBaseStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;
  pAct->addStep();
  return true;
}

// BossStep - visit
bool BossVisitStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwNpcID = param.getTableInt("npcid");
  m_dwMonsterID = param.getTableInt("monsterid");
  m_dwMonsterNum = param.getTableInt("monsternum");

  TSetDWORD setYesDialogIDs;
  TSetDWORD setNoDialogIDs;
  param.getMutableData("yes_dialogs").getIDList(setYesDialogIDs);
  param.getMutableData("no_dialogs").getIDList(setNoDialogIDs);

  m_bHasSelect = setYesDialogIDs.empty() == false && setNoDialogIDs.empty() == false;
  return true;
}

bool BossVisitStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr || pUser == nullptr)
    return false;

  SceneNpc* pNpc = pUser->getVisitNpcObj();
  if (pNpc == nullptr || pNpc->getNpcID() != m_dwNpcID)
  {
    XERR << "[Boss-visit步骤]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未访问正确的npc" << XEND;
    return false;
  }

  pAct->addActUser(pUser->id);
  if (m_dwMonsterNum != 0)
  {
    SActProcess& stProcess = pAct->getProcess();
    if (stProcess.isEnableChar(pUser->id) == false)
    {
      XERR << "[Boss-visit步骤]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,该玩家未击杀怪物" << XEND;
      return false;
    }
    stProcess.removeEnableChar(pUser->id);

    ++stProcess.dwProcess;
    if (stProcess.dwProcess < m_dwMonsterNum)
    {
      pAct->notify(pUser);
      return false;
    }
  }

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - summon
bool BossSummonStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_oDefine.load(param);
  m_dwNum = param.getTableInt("num");

  DWORD dwRandPos = param.getTableInt("randpos");
  if (dwRandPos != ERANDPOS_FULLMAP && dwRandPos != ERANDPOS_RADIUS && dwRandPos != ERANDPOS_RANDRADIUS && dwRandPos != ERANDPOS_NPCPOS)
  {
    XERR << "[Boss-summon步骤] id :" << data.getTableInt("id") << "content :" << data.getTableString("Content") << "初始化失败,randpos :" << dwRandPos << "不合法" << XEND;
    return false;
  }
  m_ePos = static_cast<ERandPos>(dwRandPos);
  m_fRadius = param.getTableFloat("radius");
  m_dwNeedNpcPos = param.getTableInt("npcid");
  return true;
}

bool BossSummonStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  Scene* pScene = SceneManager::getMe().getSceneByID(pAct->getMapID());
  if (pScene == nullptr)
  {
    XERR << "[Boss-summon步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行失败,未找到场景" << XEND;
    return false;
  }

  xPos point;
  point.clear();
  if (m_ePos == ERANDPOS_RANDRADIUS)
    pScene->getRandPos(point);
  else if (m_ePos == ERANDPOS_NPCPOS)
  {
    const xPos* pPos = pAct->getNpcPos(m_dwNeedNpcPos);
    if (pPos == nullptr)
      XERR << "[Boss-summon步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行randpos" << m_ePos << "需要npc" << m_dwNeedNpcPos << "的位置,但未找到" << XEND;
    else
      point = *pPos;
  }

  for (DWORD d = 0; d < m_dwNum; ++d)
  {
    xPos p;
    p.x = p.y = p.z = 0.0f;

    switch (m_ePos)
    {
      case ERANDPOS_FULLMAP:
        pScene->getRandPos(p);
        break;
      case ERANDPOS_RADIUS:
        pScene->getRandPosInCircle(p, m_fRadius, p);
        break;
      case ERANDPOS_RANDRADIUS:
      case ERANDPOS_NPCPOS:
        pScene->getRandPosInCircle(point, m_fRadius, p);
        break;
    }
    m_oDefine.setPos(p);

    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(m_oDefine, pScene);
    if (pNpc == nullptr)
      XERR << "[Boss-summon步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行中,召唤" << m_oDefine.getID() << "失败" << XEND;
    else
      pAct->addNpcPos(pNpc->getNpcID(), pNpc->getPos());
  }
  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - clear
bool BossClearStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwMonsterID = param.getTableInt("id");
  return true;
}

bool BossClearStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  Scene* pScene = SceneManager::getMe().getSceneByID(pAct->getMapID());
  if (pScene == nullptr)
  {
    XERR << "[Boss-summon步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行失败,未找到场景" << XEND;
    return false;
  }

  std::list<SceneNpc *> npclist;
  pScene->getSceneNpcByBaseID(m_dwMonsterID, npclist);
  for (auto &s : npclist)
    s->removeAtonce();

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - boss
bool BossBossStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwNeedNpcPos = param.getTableInt("id");
  return true;
}

bool BossBossStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  Cmd::SummonBossBossSCmd scmd;
  scmd.set_npcid(pAct->getBossID());
  scmd.set_mapid(pAct->getMapID());
  scmd.set_lv(pAct->getLv());

  xPos p;
  p.clear();

  if (m_dwNeedNpcPos != 0)
  {
    const xPos* pPos = pAct->getNpcPos(m_dwNeedNpcPos);
    if (pPos == nullptr)
      XERR << "[Boss-boss步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行召唤boss需要npc" << m_dwNeedNpcPos << "的位置,但未获取到,使用随机位置" << XEND;
    else
      p = *pPos;
  }

  do
  {
    if (BossMgr::getMe().summonBoss(scmd, p) == false)
    {
      XERR << "[Boss-boss步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行召唤boss失败,步骤继续执行" << XEND;
      break;
    }
    Scene* pScene = SceneManager::getMe().getSceneByID(pAct->getMapID());
    if (pScene == nullptr)
    {
      XERR << "[Boss-boss步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行召唤boss成功,获取bossid失败, mapid :" << pAct->getMapID() << "未找到地图场景" << XEND;
      break;
    }
    std::list<SceneNpc *> npclist;
    pScene->getSceneNpcByBaseID(pAct->getBossID(), npclist);
    for (auto &s : npclist)
    {
      pAct->getProcess().qwParam = s->id;
      XLOG << "[Boss-boss步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "bossid :" << pAct->getBossID() << "执行召唤boss成功,获取bossid :" << s->id << "成功" << XEND;
    }
  } while(0);

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - limit
bool BossLimitStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  return true;
}

bool BossLimitStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  pAct->setDestTime(now() + m_dwTime);
  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - dialog
bool BossDialogStep::init(xLuaData data)
{
  return BossBaseStep::init(data);
}

bool BossDialogStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - status
bool BossStatusStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  DWORD dwStatus = param.getTableInt("status");
  if (dwStatus <= ECREATURESTATUS_MIN || dwStatus >= ECREATURESTATUS_MAX || ECreatureStatus_IsValid(dwStatus) == false)
  {
    XERR << "[Boss-status步骤] id :" << data.getTableInt("id") << "content :" << data.getTableString("Content") << "初始化失败,status :" << dwStatus << "不合法" << XEND;
    return false;
  }
  m_eStatus = static_cast<ECreatureStatus>(dwStatus);
  m_dwNum = param.getTableInt("num");
  return true;
}

bool BossStatusStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr || pUser == nullptr)
    return false;

  if (pUser->getStatus() != m_eStatus)
    return false;

  SActProcess& rProcess = pAct->getProcess();
  if (rProcess.isEnableChar(pUser->id) == true)
    return false;
  rProcess.addEnableChar(pUser->id);
  pAct->addActUser(pUser->id);
  if (rProcess.setCharIDs.size() < m_dwNum)
    return false;

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - wait
bool BossWaitStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  return true;
}

bool BossWaitStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  SActProcess& rProcess = pAct->getProcess();
  DWORD dwNow = now();
  if (rProcess.dwProcess == 0)
    rProcess.dwProcess = dwNow;
  if (dwNow < rProcess.dwProcess + m_dwTime)
    return false;

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - kill
bool BossKillStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");

  m_setMonsterIDs.clear();
  param.getMutableData("id").getIDList(m_setMonsterIDs);
  m_dwNum = param.getTableInt("num");
  return true;
}

bool BossKillStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr || pUser == nullptr)
    return false;

  pAct->addActUser(pUser->id);
  SActProcess& rProcess = pAct->getProcess();
  rProcess.dwProcess += 1;
  if (rProcess.dwProcess < m_dwNum)
    return false;

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - world
bool BossWorldStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  m_dwTime = param.getTableInt("time");
  return true;
}

bool BossWorldStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  DWORD dwNow = now();
  SActProcess& rProcess = pAct->getProcess();
  if (rProcess.dwProcess == 0)
  {
    rProcess.dwProcess = dwNow;
    WorldBossNtf cmd;
    cmd.set_npcid(pAct->getBossID());
    cmd.set_mapid(pAct->getMapID());
    cmd.set_time(dwNow + m_dwTime);
    cmd.set_open(true);

    PROTOBUF(cmd, send, len);
    MsgManager::sendWorldCmd(send, len);
    XDBG << "[Boss-world步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "全线通知世界boss" << cmd.ShortDebugString() << XEND;

    WorldBossNtfBossSCmd scmd;
    scmd.mutable_ntf()->CopyFrom(cmd);
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);
  }
  if (dwNow < rProcess.dwProcess + m_dwTime)
    return false;

  return BossBaseStep::doStep(pAct, pUser);
}

// BossStep - show
bool BossShowStep::init(xLuaData data)
{
  if (BossBaseStep::init(data) == false)
    return false;

  xLuaData& param = data.getMutableData("Params");
  const string& type = param.getTableString("type");
  if (type == "world")
    m_eType = ESHOWTYPE_WORLD;
  else
  {
    XERR << "[Boss-show步骤] id :" << data.getTableInt("id") << "content :" << data.getTableString("Content") << "初始化失败,type :" << type << "不合法" << XEND;
    return false;
  }

  return true;
}

bool BossShowStep::doStep(BossAct* pAct, SceneUser* pUser)
{
  if (pAct == nullptr)
    return false;

  SActProcess& rProcess = pAct->getProcess();

  switch (m_eType)
  {
    case ESHOWTYPE_WORLD:
      {
        WorldBossNtf cmd;
        cmd.set_npcid(pAct->getBossID());
        cmd.set_mapid(pAct->getMapID());
        cmd.set_time(0);

        SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rProcess.qwParam);
        if (pNpc == nullptr || pNpc->isAlive() == false || pNpc->getStatus() == ECREATURESTATUS_LEAVE)
          cmd.set_open(false);
        else
          cmd.set_open(true);

        PROTOBUF(cmd, send, len);
        MsgManager::sendWorldCmd(send, len);
        XDBG << "[Boss-show步骤] mapid :" << pAct->getMapID() << "actid :" << pAct->getActID() << "全线通知世界boss" << cmd.ShortDebugString() << XEND;

        WorldBossNtfBossSCmd scmd;
        scmd.mutable_ntf()->CopyFrom(cmd);
        PROTOBUF(scmd, ssend, slen);
        thisServer->sendCmdToSession(ssend, slen);

        if (cmd.open() == true)
          return false;
      }
      break;
    default:
      return false;
  }

  return BossBaseStep::doStep(pAct, pUser);
}


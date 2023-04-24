#include "SceneTrap.h"
#include "Scene.h"
#include "SceneUser.h"
#include "xTime.h"
#include "xSceneEntryDynamic.h"
#include "SceneMap.pb.h"
#include "SkillConfig.h"
#include "SceneUserManager.h"
#include "xSceneEntryIndex.h"
#include "SceneNpcManager.h"
#include "CommonConfig.h"

SceneTrap::SceneTrap(QWORD tempid, Scene* scene, const xPos& pos, Cmd::SkillBroadcastUserCmd &cmd, QWORD masterID, const BaseSkill *skill)
{
  set_id(tempid);
  set_tempid(tempid);

  m_oCmd.CopyFrom(cmd);

  m_dwMasterID = masterID;

  setScene(scene);
  setPos(pos);

  m_skill = skill;
  m_bCanImmunedByFieldArea = m_skill->canImmunedByFieldArea();
  m_dwInterval = 1000;
  if (m_skill->getInterval() != 0)
  {
    m_dwInterval = m_skill->getInterval();//data.getTableFloat("interval") * 1000;
  }
  xSceneEntryDynamic* pMaster = xSceneEntryDynamic::getEntryByID(m_dwMasterID);
  if (pMaster)
  {
    DWORD duration = m_skill->getDuration(pMaster);
    if (duration)
    {
      m_dwEndTime = now() + duration;//data.getTableInt("duration");
    }
  }

  if (skill->getSkillType() == ESKILLTYPE_TRAPBARRIER)
  {
    m_eTrapType = ETRAPTYPE_BARRIER;
  }
  else
  {
    m_eTrapType = ETRAPTYPE_NORMAL;
  }

  XLOG << "[Trap]" << tempid << masterID << "创建,(" << getPos().x << getPos().y << getPos().z << ")" << XEND;
}

SceneTrap::~SceneTrap()
{
  XLOG << "[Trap]" << tempid << "析构,(" << getPos().x << getPos().y << getPos().z << ")" << XEND;
}

void SceneTrap::delMeToNine()
{
  if (!getScene())
    return;

  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneTrap::sendMeToNine()
{
  if (!getScene())
    return;

  Cmd::AddMapTrap cmd;
  fillMapTrapData(cmd.add_traps());
  PROTOBUF(cmd, send, len);

  SceneUser* pUser = getScreenUser();

  if (pUser)
  {
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), uSet);
    for (auto &iter : uSet)
    {
      SceneUser *user = (SceneUser *)iter;
      if (!getScene()->inScope(pUser, user))
        continue;
      user->sendCmdToMe(send, len);
    }
  }
  else
  {
    getScene()->sendCmdToNine(getPos(), send, len);
  }
}

void SceneTrap::fillMapTrapData(Cmd::MapTrap* data)
{
  if (!data) return;

  data->set_id(tempid);
  data->set_skillid(m_skill->getSkillID());
  data->set_masterid(m_dwMasterID);
  ScenePos *p = data->mutable_pos();
  p->set_x(getPos().getX());
  p->set_y(getPos().getY());
  p->set_z(getPos().getZ());
  if (m_oCmd.data().has_dir())
  {
    data->set_dir(m_oCmd.data().dir());
  }
}

bool SceneTrap::enterScene(Scene* scene)
{
  setScene(scene);
  if (getScene()->addEntryAtPosI(this))
  {
    sendMeToNine();
    return true;
  }
  return false;
}

void SceneTrap::leaveScene()
{
  if (!getScene())
    return;

  getScene()->delEntryAtPosI(this);
  delMeToNine();
}

xSceneEntryDynamic* SceneTrap::getMaster()
{
  return xSceneEntryDynamic::getEntryByID(m_dwMasterID);
}

SceneUser* SceneTrap::getScreenUser()
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_dwMasterID);
  if (pUser == nullptr)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_dwMasterID);
    if (npc)
      pUser = npc->getMasterUser();
  }

  return pUser;
}

void SceneTrap::timer(QWORD curMSec)
{
  action(curMSec);
  /*if (!action(msec))
  {
    leaveScene();
    m_blNeedClear = true;
  }*/
}

bool SceneTrap::action(QWORD msec)
{
  /*if (!m_dwInterval) return false;
  if (m_qwNextTime > msec) return true;
  if (m_dwMaxCount && m_dwCount >= m_dwMaxCount) return false;
  if (m_dwEndTime && m_dwEndTime < msec/1000) return false;

  ++m_dwCount;
  m_qwNextTime = msec + m_dwInterval;

  xSceneEntryDynamic *pEntry = getMaster();
  if (pEntry && m_skill)
  {
    SkillBroadcastUserCmd& oCmd = pEntry->m_oSkillProcessor.getRunner().getCmd();
    m_skill->collectTarget(pEntry, NULL, getPos(), pEntry->m_oSkillProcessor.getRunner().getCmd());
    //m_oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
    //m_oCmd.clear_charid();
    oCmd.mutable_data()->set_number(ESKILLNUMBER_RET);
    oCmd.clear_charid();

    AttackSkill* pAttackSkill = dynamic_cast<AttackSkill*>(m_skill);
    if (pAttackSkill != nullptr)
      pAttackSkill->fire(pEntry->m_oSkillProcessor.getRunner());
    if (m_dwHitTime)
    {
      if (oCmd.data().hitedtargets_size())
      {
        ++m_dwHitCount;
        XLOG("[Trap],%llu,%s,hit:%u", pEntry->id, pEntry->name, m_dwHitCount);
        if (m_dwHitCount >= m_dwHitTime)
        {
          return false;
        }
      }
      XLOG("[Trap],%llu,%s,hit:%u", pEntry->id, pEntry->name, m_dwHitCount);
    }

    XLOG("[Trap],%llu,%s,count:%u,hit:%u", pEntry->id, pEntry->name, m_dwCount, m_dwHitTime);
  }*/
  return true;
}

TransportTrap::TransportTrap(QWORD tempid, Scene* scene, const xPos& pos, Cmd::SkillBroadcastUserCmd& cmd, QWORD masterID, const BaseSkill* skill) : SceneTrap(tempid, scene, pos, cmd, masterID, skill)
{
}

TransportTrap::~TransportTrap()
{

}

bool TransportTrap::action(QWORD curMSec)
{
  if (m_qwNextTime > curMSec)
    return true;
  m_qwNextTime = curMSec + m_dwInterval;

  if (!getScene())
    return false;

  xSceneEntrySet uset;
  getScene()->getEntryListInBlock(SCENE_ENTRY_USER, getPos(), 2, uset);
  if (uset.empty())
    return true;

  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(m_dwMasterID);
  if (pMaster == nullptr)
    return false;

  for (auto &s : uset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user)
      continue;
    if (user->isMyTeamMember(m_dwMasterID))
    {
      DWORD mapid = pMaster->getUserSceneData().getTransMap();
      xPos pos;
      if (mapid == 0)
        mapid = pMaster->getUserSceneData().getSaveMap();
      else
        pos = pMaster->getUserSceneData().getTransPos();

      user->gomap(mapid, GoMapType::Skill, pos);
    }
  }

  return true;
}


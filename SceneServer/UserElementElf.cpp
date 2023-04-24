#include "UserElementElf.h"
#include "SceneNpcManager.h"
#include "SceneUser.h"
#include "SkillManager.h"
#include "MiscConfig.h"

UserElementElf::UserElementElf(SceneUser* user) : m_pUser(user)
{
}

UserElementElf::~UserElementElf()
{
}

bool UserElementElf::load(const BlobElementElfData& data)
{
  m_dwCurElementID = data.cur_element_id();
  m_dwClearTime = data.clear_time();

  return true;
}

bool UserElementElf::save(BlobElementElfData* data)
{
  if (data == nullptr)
    return false;

  data->set_cur_element_id(m_dwCurElementID);
  data->set_clear_time(m_dwClearTime);

  return true;
}

void UserElementElf::timer(DWORD cur)
{
  if (m_dwTempEndBattleTime && cur >= m_dwTempEndBattleTime)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
    if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    {
      m_qwMasterCurLockID = 0;
      m_dwTempEndBattleTime = 0;
      return;
    }
    if (npc->m_ai.getCurLockID() != 0)
      npc->m_ai.setCurLockID(0);
    m_qwMasterCurLockID = 0;
    m_dwTempEndBattleTime = 0;
  }

  if (m_dwClearTime && cur >= m_dwClearTime)
  {
    reset();
  }
}

void UserElementElf::reset()
{
  m_dwCurElementID = 0;
  m_qwCurElementTempID = 0;
  m_dwClearTime = 0;
  m_qwMasterCurLockID = 0;
  m_dwTempEndBattleTime = 0;
}

SceneNpc* UserElementElf::getCurElementNpc()
{
  if (m_dwCurElementID == 0 || m_qwCurElementTempID == 0)
    return nullptr;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return nullptr;
  return npc;
}

bool UserElementElf::summon(DWORD id, DWORD last_seconds)
{
  if (m_dwCurElementID != 0 || m_qwCurElementTempID != 0)
  {
    //收回元素
    leaveScene();
    XLOG << "[元素-收回]" << m_pUser->accid << m_pUser->id << m_pUser->name << "元素id:" << m_dwCurElementID << "收回成功" << XEND;
    reset();
  }

  m_dwCurElementID = id;
  m_dwClearTime = now() + last_seconds;
  if (enterScene(id) == false)
    return false;
  XLOG << "[元素-召唤]" << m_pUser->accid << m_pUser->id << m_pUser->name << "元素id:" << id << "持续时间" << last_seconds << "召唤成功" << XEND;
  return true;
}

bool UserElementElf::enterScene(DWORD id)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return false;

  if (m_qwCurElementTempID != 0)
    return false;

  NpcDefine def;
  def.setID(m_dwCurElementID);
  def.setPos(m_pUser->getPos());
  def.setRange(1);
  def.m_oVar.m_qwOwnerID = m_pUser->id;
  def.setLife(1);
  def.setBehaviours(def.getBehaviours() & ~BEHAVIOUR_OUT_RANGE_BACK);
  def.setTerritory(0);
  def.setDir(m_pUser->getUserSceneData().getDir() / ONE_THOUSAND);

  SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, pScene);
  if (npc == nullptr)
    return false;
  ElementElfNpc* elementNpc = dynamic_cast<ElementElfNpc*>(npc);
  if (elementNpc == nullptr)
  {
    npc->setClearState();
    return false;
  }

  elementNpc->setClearTime(m_dwClearTime);
  m_qwCurElementTempID = elementNpc->id;
  m_pUser->getBuff().onChangeElement();

  return true;
}

bool UserElementElf::leaveScene()
{
  if (m_qwCurElementTempID == 0)
    return false;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return false;

  npc->removeAtonce();
  m_qwCurElementTempID = 0;
  m_pUser->getBuff().onChangeElement();
  return true;
}

void UserElementElf::onUserEnterScene()
{
  if (m_dwCurElementID == 0)
    return;
  if (now() >= m_dwClearTime)
  {
    reset();
    return;
  }

  enterScene(m_dwCurElementID);
}

void UserElementElf::onUserLeaveScene()
{
  if (m_dwCurElementID == 0 || m_qwCurElementTempID == 0)
    return;
  leaveScene();
}

void UserElementElf::onUserMove(const xPos& dest)
{
  Scene* pScene = m_pUser->getScene();
  if (pScene == nullptr)
    return;
  if (m_qwCurElementTempID == 0)
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  if (getDistance(npc->getPos(), dest) < 1.5)
    return;

  std::list<xPos> list;
  if (m_pUser->getScene()->findingPath(dest, m_pUser->getPos(), list, TOOLMODE_PATHFIND_FOLLOW) == false)
    return;
  if (list.size() < 2)
    return;

  list.pop_front();
  xPos dest1 = list.front();
  xPos mydest = dest1;
  float foldis = 1;
  float dist = getDistance(dest, dest1);
  mydest.x = foldis / dist * (dest1.x - dest.x) + dest.x;
  mydest.z = foldis / dist * (dest1.z - dest.z) + dest.z;

  auto getPosOnCircle = [&](const xPos& pos0, const xPos& pos1, float angle, xPos& out)
    {
      angle = angle * 3.14 / 180.0f;
      out = pos0;
      out.z = pos0.z + (pos1.z - pos0.z) * cos(angle) - (pos1.x - pos0.x) * sin(angle);
      out.x = pos0.x + (pos1.z - pos0.z) * sin(angle) + (pos1.x - pos0.x) * cos(angle);
    };
  xPos outpos;
  getPosOnCircle(dest, mydest, 70, outpos);
  if (pScene->getValidPos(outpos) == false)
    return;
  npc->m_ai.moveTo(outpos);
}

void UserElementElf::onUserGoTo(const xPos& dest)
{
  if (m_qwCurElementTempID == 0)
    return;
  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  xPos pos = dest;
  if (npc->getScene())
    npc->getScene()->getRandPos(m_pUser->getPos(), 5, pos);
  npc->m_oMove.clear();
  npc->goTo(pos);
}

void UserElementElf::onUserDie()
{
  if (m_qwCurElementTempID == 0)
    return;
  leaveScene();
  reset();
}

void UserElementElf::onUserAttack(xSceneEntryDynamic* enemy)
{
  if (enemy == nullptr)
    return;
  if (m_dwCurElementID == 0 || m_qwCurElementTempID == 0)
    return;

  DWORD cur = now();
  m_qwMasterCurLockID = enemy->id;
  m_dwTempEndBattleTime = cur + 3;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(m_qwCurElementTempID);
  if (npc == nullptr || npc->define.m_oVar.m_qwOwnerID != m_pUser->id)
    return;
  npc->m_ai.setCurLockID(enemy->id);
}

void UserElementElf::onProfesChange()
{
  leaveScene();
  reset();
}

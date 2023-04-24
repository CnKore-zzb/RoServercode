#include "SceneItem.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "xTime.h"
#include "MiscConfig.h"
#include "SceneItemManager.h"
#include "DScene.h"

SceneItem::SceneItem(QWORD tempid, Scene* scene, const ItemInfo& rInfo, const xPos& rPos)
{
  set_id(tempid);
  set_tempid(tempid);

  m_dwTimeTick = xTime::getCurSec();

  m_eStatus = ESCENEITEMSTATUS_VALID;

  m_stInfo = rInfo;
  setScene(scene);
  setPos(rPos);

  const SSceneItemCFG& rCFG = MiscConfig::getMe().getSceneItemCFG();
  m_dwDisappearTime = rCFG.dwDisappearTime;
  m_dwOwnerTime = rCFG.dwOwnerTime;

  if (scene && scene->isPollyScene())
  {
    m_dwDisappearTime = MiscConfig::getMe().getPoliFireCFG().dwItemDispTime;
  }

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(rInfo.id());
  if (pCFG != nullptr)
  {
    if (pCFG->eItemType == EITEMTYPE_QUESTITEM || pCFG->eItemType == EITEMTYPE_QUESTITEMCOUNT)
      m_dwOwnerTime = rCFG.dwDisappearTime + 10;
    else if (pCFG->eItemType == EITEMTYPE_PICKEFFECT)
      m_dwDisappearTime = MiscConfig::getMe().getMiscPveCFG().dwItemDispTime;
    else if (pCFG->eItemType == EITEMTYPE_PICKEFFECT_1)
      m_dwDisappearTime = MiscConfig::getMe().getSuperGvgCFG().dwItemDispTime;
  }

  const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(m_stInfo.source_npc());
  if (pNpcCFG)
    m_npcType = pNpcCFG->eNpcType;
  else
    m_npcType = ENPCTYPE_MIN;
}

SceneItem::~SceneItem()
{

}

void SceneItem::delMeToNine()
{
  if (!getScene())
    return;

  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);

  XLOG << "[场景物品-删除] 地图=" << getScene()->getMapID() << "item=" << m_stInfo.id() << XEND;
}

void SceneItem::sendMeToNine()
{
  if (!getScene())
    return;

  Cmd::AddMapItem cmd;
  fillMapItemData(cmd.add_items());

  PROTOBUF(cmd, send, len);

  if (m_dwIsViewLimit)
  {
    for (auto v = m_vecOwners.begin(); v != m_vecOwners.end(); ++v)
    {
      SceneUser *pUser = SceneUserManager::getMe().getUserByID(*v);
      if (pUser == nullptr)
        continue;
      pUser->sendCmdToMe(send, len);
      XLOG << "[场景物品-添加] 地图=" << getScene()->getMapID() << "item=" << m_stInfo.id() << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "可见" << XEND;
    }
    return;
  }

  getScene()->sendCmdToNine(getPos(), send, len);
  XLOG << "[场景物品-添加] 地图=" << getScene()->getMapID() << "item=" << m_stInfo.id() << XEND;
}

void SceneItem::setClearState()
{
  if (m_eStatus == ESCENEITEMSTATUS_INVALID)
    return;

  leaveScene();
}

bool SceneItem::canPickup(SceneUser* pUser, EItemPickUpMode eMode/* = EITEMPICKUP_NORMAL*/)
{
  if (pUser == nullptr /*|| pUser->isAlive() == false*/)
    return false;
  if (getStatus() == ESCENEITEMSTATUS_INVALID)
    return false;
  if (pUser->canPickup() == false)
    return false;

  if (pUser->getScene() && pUser->getScene()->isPollyScene())
  {
    PollyScene* pScene = dynamic_cast<PollyScene*> (pUser->getScene());
    if (pScene == nullptr)
      return false;
    if (m_stInfo.id() != MiscConfig::getMe().getPoliFireCFG().dwAppleItemID)
    {
      if (pScene->canPickUp(pUser->id) == false)
        return false;
    }
    else
    {
      if (eMode == EITEMPICKUP_NORMAL)
      {
        QWORD pickcd = pScene->getNextPickAppleTime(pUser);
        if (pickcd > xTime::getCurMSec())
          return false;
        if (pScene->canPickUpApple(pUser->id) == false)
          return false;
      }
    }
  }
  else if (pUser->getScene() && pUser->getScene()->isAltmanScene())
  {
    if(pUser->getTransform().canPickUp() == false)
      return false;
  }

  if (pUser->isJustInViceZone() && m_stInfo.source_npc() != 0)
  {
    const SNpcCFG* pNpcCFG = NpcConfig::getMe().getNpcCFG(m_stInfo.source_npc());
    if (pNpcCFG == nullptr)
      return false;
    if ((m_npcType == ENPCTYPE_MINIBOSS || m_npcType == ENPCTYPE_MVP) && pNpcCFG->eZoneType == ENPCZONE_FIELD)
      return false;
  }
  if (m_dwOwnerTime != 0)
  {
    if (m_vecOwners.empty())
      return true;
    auto v = find(m_vecOwners.begin(), m_vecOwners.end(), pUser->id);
    return v != m_vecOwners.end();
  }

  return true;
}

bool SceneItem::enterScene(Scene* scene)
{
  setScene(scene);
  return getScene()->addEntryAtPosI(this);
  //sendMeToNine();
}

void SceneItem::leaveScene()
{
  if (!getScene() || m_eStatus == ESCENEITEMSTATUS_INVALID)
    return;

  getScene()->delEntryAtPosI(this);
  delMeToNine();

  m_dwOwnerTime = 0;
  m_eStatus = ESCENEITEMSTATUS_INVALID;
}

void SceneItem::addOwner(QWORD ownerid)
{
  auto v = find(m_vecOwners.begin(), m_vecOwners.end(), ownerid);
  if (v != m_vecOwners.end())
    return;

  m_vecOwners.push_back(ownerid);
}

void SceneItem::fillMapItemData(MapItem* data, DWORD extraTime /*= 0*/)
{
  if (data == NULL)
    return;

  data->set_guid(tempid);

  data->set_id(m_stInfo.id());
  data->set_time(m_dwOwnerTime + extraTime);

  data->mutable_pos()->set_x(getPos().getX());
  data->mutable_pos()->set_y(getPos().getY());
  data->mutable_pos()->set_z(getPos().getZ());

  data->set_sourceid(m_dwSourceID);

  if (m_stInfo.refinelv())
    data->set_refinelv(m_stInfo.refinelv());

  for (auto v = m_vecOwners.begin(); v != m_vecOwners.end(); ++v)
    data->add_owners(*v);
}

void SceneItem::timer(QWORD curMSec)
{
  if (getScene() == nullptr || getStatus() == ESCENEITEMSTATUS_INVALID)
    return;

  DWORD curSec = curMSec / ONE_THOUSAND;
  DWORD delta = curSec >= m_dwTimeTick ? curSec - m_dwTimeTick : 0;

  if (curSec < m_dwTimeTick)
    XERR << "[场景物品] 场景" << getScene()->getMapID() << "上的物品" << m_stInfo.ShortDebugString() << "异常,当前时间 :" << curSec << "小于" << "创建时间 :" << m_dwTimeTick << XEND;

  if (delta != 0)
  {
    m_dwTimeTick = xTime::getCurSec();
    m_dwOwnerTime = m_dwOwnerTime <= delta ? 0 : m_dwOwnerTime - delta;
    m_dwDisappearTime = m_dwDisappearTime <= delta ? 0 : m_dwDisappearTime - delta;
  }

  if (m_dwDisappearTime > delta)
    bePickup();
  else
    leaveScene();
}

bool SceneItem::viewByUser(QWORD userid)
{
  if (!m_dwIsViewLimit)
    return true;
  auto iter = find(m_vecOwners.begin(), m_vecOwners.end(), userid);
  return iter != m_vecOwners.end();
}

void SceneItem::bePickup()
{
  if (MiscConfig::getMe().getItemCFG().ePickupMode != EPICKUPMODE_SERVER)
    return;

  const SSceneItemCFG& rCFG = MiscConfig::getMe().getSceneItemCFG();
  float pickrange = rCFG.dwDropRadius;
  if (getScene() && getScene()->isPollyScene())
  {
    pickrange = MiscConfig::getMe().getPoliFireCFG().fDropRange;
  }

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(m_stInfo.id());
  if (pCFG)
  {
    if (pCFG->eItemType == EITEMTYPE_PICKEFFECT || pCFG->eItemType == EITEMTYPE_PICKEFFECT_1)
      pickrange = rCFG.dwPickEffectRange;
  }

  set<SceneUser*> setUsers;
  if (m_dwOwnerTime > 0 && m_vecOwners.empty() == false)
  {
    for (auto &v : m_vecOwners)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(v);
      if (pUser != nullptr && canPickup(pUser) == true && pUser->getScene() == getScene() && checkDistance(pUser->getPos(), getPos(), pickrange) == true)
        setUsers.insert(pUser);
    }
  }
  else
  {
    xSceneEntrySet uSet;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, getPos(), uSet);
    for (auto &it : uSet)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(it);
      if (pUser != nullptr && canPickup(pUser) == true && pUser->getScene() == getScene() && checkDistance(pUser->getPos(), getPos(), pickrange) == true)
        setUsers.insert(pUser);
    }
  }

  SceneUser** ppUser = randomStlContainer(setUsers);
  if (ppUser != nullptr && *ppUser != nullptr)
  {
    SceneUser* pUser = *ppUser;
    if (pUser != nullptr)
      SceneItemManager::getMe().pickupSceneItem(pUser, id);
  }
  XDBG << "[SceneItem]" << m_stInfo.ShortDebugString() << "捡取" << XEND;
}


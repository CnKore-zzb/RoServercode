#include "GearManager.h"
#include "SceneUser2.pb.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "Scene.h"
#include "DScene.h"

GearManager::GearManager(Scene *s):m_pScene(s)
{
}

GearManager::~GearManager()
{
}

void GearManager::add(SceneNpc *npc)
{
  if (!npc) return;
  if (!npc->define.isGear()) return;
  if (!npc->define.getUniqueID()) return;

  Gear &item = m_oGearList[npc->define.getUniqueID()];
  item.id = npc->define.getUniqueID();
  item.state = npc->define.getGearOrgState();
  item.isPrivate = npc->define.isGearPrivate();
  item.tempid = npc->getTempID();
  XLOG << "[装置]" << npc->id << npc->name << "添加,state:" << item.state << "private:" << item.isPrivate << XEND;
}

void GearManager::set(DWORD gearID, DWORD state, SceneUser *user)
{
  auto it = m_oGearList.find(gearID);
  if (it==m_oGearList.end()) return;

  Gear &item = m_oGearList[gearID];

  Cmd::UserActionNtf message;
  message.set_type(Cmd::EUSERACTIONTYPE_GEAR_ACTION);
  message.set_charid(item.tempid);
  if (item.isPrivate)
  {
    if (user)
    {
      user->m_oGear.set(user->getScene()->getMapID(), gearID, state);
      message.set_value(get(gearID, user)*1000 + 1);
      PROTOBUF(message, send, len);
      user->sendCmdToMe(send, len);
      XLOG << "[装置]" << user->id << user->name << "设置" << item.id << "state:" << item.state << "private:" << item.isPrivate << XEND;
    }
  }
  else
  {
    item.state = state;
    message.set_value(get(gearID)*1000 + 1);
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(item.tempid);
    if (npc)
    {
      PROTOBUF(message, send, len);
      npc->sendCmdToNine(send, len);
      XLOG << "[装置]" << npc->id << npc->name << "设置" << item.id << "state:" << item.state << "private:" << item.isPrivate << XEND;
    }
  }
  if (m_pScene->isDScene())
  {
    DScene* pDScene = dynamic_cast<DScene*>(m_pScene);
    if (pDScene != nullptr)
      pDScene->getFuben().check("wait_gear");
  }
}

DWORD GearManager::get(DWORD gearID, SceneUser *user)
{
  auto it = m_oGearList.find(gearID);
  if (it==m_oGearList.end())
  return 0;

  if (it->second.isPrivate)
  {
    if (user && user->getScene())
    {
      return user->m_oGear.get(user->getScene()->getMapID(), gearID);
    }
  }
  else
  {
    return it->second.state;
  }
  return 0;
}

void GearManager::send(SceneUser *user, std::list<DWORD> &list)
{
  if (!user) return;

  Cmd::UserActionNtf message;
  message.set_type(Cmd::EUSERACTIONTYPE_GEAR_ACTION);
  for (auto &it : list)
  {
    auto iter = m_oGearList.find(it);
    if (iter!=m_oGearList.end())
    {
      message.set_charid(iter->second.tempid);
      message.set_value(get(it, user)*1000+2);
      PROTOBUF(message, send, len);
      user->sendCmdToMe(send, len);
    }
  }
}

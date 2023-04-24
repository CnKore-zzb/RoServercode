#include "Shortcut.h"
#include "SceneUser.h"
#include "ItemManager.h"

Shortcut::Shortcut(SceneUser* pUser) : m_pUser(pUser)
{

}

Shortcut::~Shortcut()
{

}

bool Shortcut::load(const BlobShortcut& rData)
{
  m_vecShortcut.clear();
  for (int i = 0; i < rData.cut_size(); ++i)
    m_vecShortcut.push_back(rData.cut(i));

  m_vecFirstAuto.clear();
  for (int i = 0; i < rData.firstauto_size(); ++i)
    m_vecFirstAuto.push_back(rData.firstauto(i));

  initShortcut();
  return true;
}

bool Shortcut::save(BlobShortcut* pData)
{
  if (pData == nullptr)
  {
    XERR << "[快捷键-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "protobuf error" << XEND;
    return false;
  }

  pData->clear_cut();
  for (auto v = m_vecShortcut.begin(); v != m_vecShortcut.end(); ++v)
  {
    ShortcutItem* pCut = pData->add_cut();
    if (pCut == nullptr)
    {
      XERR << "[快捷键-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "protobuf error" << XEND;
      continue;
    }

    pCut->CopyFrom(*v);
  }

  pData->clear_firstauto();
  for (auto v = m_vecFirstAuto.begin(); v != m_vecFirstAuto.end(); ++v)
    pData->add_firstauto(*v);

  XDBG << "[快捷键-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pData->ByteSize() << XEND;
  return true;
}

void Shortcut::refreshShortcut()
{
  MainPackage* pMainPack = dynamic_cast<MainPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return;

  for (auto v = m_vecShortcut.begin(); v != m_vecShortcut.end(); ++v)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(v->type());
    if (pCFG == nullptr)
      continue;

    if (pCFG->eEquipType != EEQUIPTYPE_MIN)
    {
      if (m_pUser->getPackage().getItemCount(v->type()) == 0)
      {
        v->set_guid("");
        v->set_type(0);

        m_setChangePos.insert(v->pos());
      }
    }
    else
    {
      if (v->guid().empty() == false)
      {
        if (pMainPack->getItemCount(v->type()) == 0)
        {
          v->set_guid("");
          m_setChangePos.insert(v->pos());
        }
      }
      else
      {
        const string& guid = pMainPack->getGUIDByType(v->type());
        if (guid.empty() == false)
        {
          v->set_guid(guid);
          m_setChangePos.insert(v->pos());
        }
      }
    }
  }
}

void Shortcut::onItemAdd(const ItemInfo& rItem)
{
  const TVecDWORD& vecIDs = MiscConfig::getMe().getNewRoleCFG().vecFirstShortcut;
  auto v = find(vecIDs.begin(), vecIDs.end(), rItem.id());
  if (v == vecIDs.end())
    return;

  auto o = find(m_vecFirstAuto.begin(), m_vecFirstAuto.end(), rItem.id());
  if (o != m_vecFirstAuto.end())
    return;

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
    return;

  for (auto v = m_vecShortcut.begin(); v != m_vecShortcut.end(); ++v)
  {
    if (v->type() != 0)
      continue;

    v->set_type(rItem.id());
    v->set_guid(pMainPack->getGUIDByType(rItem.id()));

    m_vecFirstAuto.push_back(rItem.id());
    m_setChangePos.insert(v->pos());

    break;
  }
}

void Shortcut::timer(DWORD curTime)
{
  if (m_setChangePos.empty() == true)
    return;

  for (auto s = m_setChangePos.begin(); s != m_setChangePos.end(); ++s)
  {
    ShortcutItem* pItem = getShortcutItem(*s);
    if (pItem == nullptr)
      continue;

    PutShortcut cmd;
    cmd.mutable_item()->CopyFrom(*pItem);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  m_setChangePos.clear();
}

void Shortcut::sendAllShortcut()
{
  if (m_pUser == nullptr)
    return;

  QueryShortcut cmd;
  for (auto v = m_vecShortcut.begin(); v != m_vecShortcut.end(); ++v)
  {
    ShortcutItem* pItem = cmd.add_list();
    if (pItem != nullptr)
      pItem->CopyFrom(*v);
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Shortcut::putShortcut(const ShortcutItem& rItem)
{
  // update shortcut
  ShortcutItem* pItem = getShortcutItem(rItem.pos());
  if (pItem == nullptr)
    return false;

  if (rItem.guid().empty() == true)
  {
    pItem->set_guid("");
    pItem->set_type(0);
  }
  else
  {
    pItem->CopyFrom(rItem);
  }

  // inform client
  PutShortcut cmd;
  cmd.mutable_item()->CopyFrom(rItem);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  return true;
}

void Shortcut::initShortcut()
{
  DWORD dwMaxShortcut = MiscConfig::getMe().getNewRoleCFG().dwMaxShortCut;
  if (m_vecShortcut.size() != dwMaxShortcut)
    m_vecShortcut.resize(dwMaxShortcut);

  for (size_t i = 0; i < m_vecShortcut.size(); ++i)
    m_vecShortcut[i].set_pos(i);
}

ShortcutItem* Shortcut::getShortcutItem(DWORD dwPos)
{
  auto v = find_if(m_vecShortcut.begin(), m_vecShortcut.end(), [dwPos](const ShortcutItem& r) -> bool{
    return r.pos() == dwPos;
  });
  if (v != m_vecShortcut.end())
    return &(*v);

  return nullptr;
}


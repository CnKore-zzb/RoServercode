#include "UserQuestNpc.h"
#include "SceneUser.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "RecordCmd.pb.h"
#include "MiscConfig.h"

UserQuestNpc::UserQuestNpc(SceneUser *user):m_pUser(user)
{
}

void UserQuestNpc::onUserEnterScene()
{
  if (m_list.empty()) return;
  if (!m_pUser->getScene()) return;
  if (m_pUser->getScene()->m_oImages.isImageScene())
    return;

  auto it = m_list.begin();
  auto tmp = it;
  for ( ;it!=m_list.end(); )
  {
    tmp = it++;
    if (tmp->m_qwTempID) continue;
    if (tmp->bDelete) continue;

    if (tmp->m_dwMapID == m_pUser->getScene()->getMapID())
    {
      tmp->m_oDefine.m_oVar.m_qwQuestOwnerID = m_pUser->id;
      SceneNpc *npc = SceneNpcManager::getMe().createNpc(tmp->m_oDefine, m_pUser->getScene());
      if (npc)
      {
        tmp->m_qwTempID = npc->id;
        npc->setAttr(EATTRTYPE_HP, tmp->m_dwHp);
      }
    }
    else if (tmp->m_dwMapID == 0 && m_pUser->getScene()->isDScene() == false)
    {
      tmp->m_oDefine.m_oVar.m_qwQuestOwnerID = m_pUser->id;
      tmp->m_oDefine.setPos(m_pUser->getPos());
      SceneNpc *npc = SceneNpcManager::getMe().createNpc(tmp->m_oDefine, m_pUser->getScene());
      if (npc)
      {
        tmp->m_qwTempID = npc->id;
        npc->setAttr(EATTRTYPE_HP, tmp->m_dwHp);
      }
      tmp->m_dwMapID = m_pUser->getScene()->getMapID();
    }
  }
}

void UserQuestNpc::onUserLeaveScene()
{
  if (m_list.empty()) return;
  for (auto it=m_list.begin(); it!=m_list.end(); ++it)
  {
    if (!it->m_qwTempID) continue;
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(it->m_qwTempID);
    if (npc && npc->getScene())
    {
      it->m_dwHp = npc->getAttr(EATTRTYPE_HP);
      it->m_qwTempID = 0;

      npc->setStatus(ECREATURESTATUS_LEAVE);
    }
  }
}

void UserQuestNpc::onNpcDie(SceneNpc *npc)
{
  if (!npc) return;

  auto it = m_list.begin();
  //auto tmp = it;
  for ( ; it!=m_list.end();)
  {
    //tmp = it++;
    if (npc->id == it->m_qwTempID)
    {
      it = m_list.erase(it);
      XLOG << "[任务NPC-死亡] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name <<
        " 死亡任务 : " << it->m_dwQuestID << " NPC " << it->m_qwTempID << "," << it->m_oDefine.getID() << "," << npc->name << XEND;
      break;
    }

    ++it;
  }
}

void UserQuestNpc::delNpc(DWORD id, DWORD dwQuestID)
{
  for (auto it = m_list.begin(); it != m_list.end(); ++it)
  {
    if (id == it->m_oDefine.getID() && it->m_dwQuestID == dwQuestID)
    {
      it->bDelete = true;
      //m_list.erase(tmp);
      XLOG << "[任务NPC-删除] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," <<  m_pUser->name
        << " 删除任务 : " << it->m_dwQuestID << "," << " NPC " << it->m_qwTempID << "," << it->m_oDefine.getID() << XEND;
    }
  }
}

void UserQuestNpc::killNpc(DWORD id, DWORD uniqueID)
{
  std::list<pair<QWORD, DWORD>> list;

  for (auto &it : m_list)
  {
    if ((id && id == it.m_oDefine.getID()) || (uniqueID && uniqueID == it.m_oDefine.getUniqueID()))
      list.push_back(pair<QWORD, DWORD>(it.m_qwTempID, it.m_dwQuestID));
  }
  for (auto &it : list)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(it.first);
    if (npc)
    {
      npc->attackMe(npc->getAttr(EATTRTYPE_MAXHP), m_pUser);
      //npc->onNpcDie(m_pUser);
      XLOG << "[任务NPC-击杀] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name
        << " 击杀任务 : " << it.second << " NPC " << npc->id << "," << npc->getNpcID() << "," << npc->name << XEND;
    }
  }
}

void UserQuestNpc::delNpcQuest(DWORD dwQuestID, DWORD dwGroupID /*= 0*/)
{
  for (auto l = m_list.begin(); l != m_list.end(); ++l)
  {
    if (l->m_dwQuestID == dwQuestID)
    {
      if (dwGroupID != 0 && dwGroupID != l->m_dwGroupID)
        continue;
      l->bDelete = true;
      if (dwGroupID != 0)
      {
        SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(l->m_qwTempID);
        if (pNpc != nullptr)
          pNpc->removeAtonce();
      }
      //l = m_list.erase(l);
      XLOG << "[任务NPC-任务完成删除] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name
        << " 击杀任务 : " << dwQuestID << " NPC " << l->m_qwTempID << "," << l->m_oDefine.getID() << XEND;
    }
  }
}

bool UserQuestNpc::addNpc(SceneNpc *npc, DWORD dwQuestID)
{
  if (!npc || !npc->getScene()) return false;

  QuestNpcSave item;
  item.m_dwMapID = npc->getScene()->getMapID();
  item.m_dwQuestID = dwQuestID / QUEST_ID_PARAM;
  item.m_dwGroupID = dwQuestID % QUEST_ID_PARAM;
  item.m_oDefine = npc->define;
  item.m_dwHp = npc->getAttr(EATTRTYPE_HP);
  item.m_qwTempID = npc->id;

  const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(dwQuestID);
  if (pCFG == nullptr)
    XERR << "[任务NPC-添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "任务 :" << dwQuestID << "添加NPC" << npc->id << npc->getNpcID() << npc->name << "设置acc失败,未在 Table_Quest.txt 表中找到" << XEND;
  item.bAcc = pCFG != nullptr && QuestConfig::getMe().isShareQuest(pCFG->eType) == true;

  m_list.push_back(item);
  XLOG << "[任务NPC-添加] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name
    << " 任务 : " << dwQuestID << " 添加NPC " << npc->id << "," << npc->getNpcID() << "," << npc->name << XEND;

  return true;
}

bool UserQuestNpc::addNpc(const NpcDefine& rDefine, DWORD dwMapID, DWORD dwQuestID)
{
  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rDefine.getID());
  if (pCFG == nullptr)
  {
    XERR << "[任务NPC-添加离线] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name
      << " 任务 : " << dwQuestID << " 添加NPC " << rDefine.getID() << "失败" << XEND;
    return false;
  }

  QuestNpcSave item;
  item.m_dwMapID = dwMapID;
  item.m_dwQuestID = dwQuestID / QUEST_ID_PARAM;
  item.m_dwGroupID = dwQuestID % QUEST_ID_PARAM;
  item.m_oDefine = rDefine;
  item.m_dwHp = 0;

  const SQuestCFG* pQuestCFG = QuestConfig::getMe().getQuestCFG(dwQuestID);
  if (pQuestCFG == nullptr)
    XERR << "[任务NPC-添加离线]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << " 任务 :" << dwQuestID << "添加NPC " << rDefine.getID() << pCFG->strName << "设置acc失败,未在 Table_Quest.txt 表中找到"<< XEND;
  item.bAcc = pQuestCFG != nullptr && QuestConfig::getMe().isShareQuest(pQuestCFG->eType) == true;

  m_list.push_back(item);
  XLOG << "[任务NPC-添加离线] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name
    << " 任务 : " << dwQuestID << " 添加NPC " << rDefine.getID() << "," << pCFG->strName << XEND;

  return true;
}

void UserQuestNpc::save(Cmd::BlobQuestNpc* acc_data, Cmd::BlobQuestNpc *data)
{
  if (acc_data == nullptr || data == nullptr)
    return;

  acc_data->Clear();
  data->Clear();
  if (m_list.empty()) return;

  for (auto it=m_list.begin(); it!=m_list.end(); ++it)
  {
    if (it->bDelete)
      continue;

    Cmd::QuestNpcData *pItem = nullptr;
    if (it->bAcc)
      pItem = acc_data->add_list();
    else
      pItem = data->add_list();

    pItem->set_mapid(it->m_dwMapID);
    pItem->set_questid(it->m_dwQuestID);
    pItem->set_groupid(it->m_dwGroupID);
    pItem->set_acc(it->bAcc ? 1 : 0);
    it->m_oDefine.save(pItem->mutable_data());
    pItem->set_hp(it->m_dwHp);
  }

  XDBG << "[任务NPC-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 acc :" << acc_data->ByteSize() << "char :" << data->ByteSize() << XEND;
}

void UserQuestNpc::load(const Cmd::BlobQuestNpc& acc_data, const Cmd::BlobQuestNpc &data)
{
  if (acc_data.list_size() > 0)
  {
    int size = acc_data.list_size();
    for (int i=0; i<size; ++i)
    {
      const Cmd::QuestNpcData &item = acc_data.list(i);
      QuestNpcSave questNpcItem;
      questNpcItem.m_dwMapID = item.mapid();
      questNpcItem.m_dwQuestID = item.questid();
      questNpcItem.m_dwGroupID = item.groupid();
      questNpcItem.m_oDefine.load(item.data());
      questNpcItem.m_dwHp = item.hp();
      questNpcItem.bAcc = item.acc() == 1;

      m_list.push_back(questNpcItem);
      XDBG << "[任务NPC-acc加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "加载npc id :" << questNpcItem.m_oDefine.getID() << "map :" << questNpcItem.m_dwMapID << "questid :" << questNpcItem.m_dwQuestID << "hp :" << questNpcItem.m_dwHp <<
        "pos : (" << questNpcItem.m_oDefine.getPos().x << questNpcItem.m_oDefine.getPos().y << questNpcItem.m_oDefine.getPos().z << ")" << XEND;
    }
  }

  if (data.list_size() > 0)
  {
    int size = data.list_size();
    for (int i=0; i<size; ++i)
    {
      const Cmd::QuestNpcData &item = data.list(i);
      QuestNpcSave questNpcItem;
      questNpcItem.m_dwMapID = item.mapid();
      questNpcItem.m_dwQuestID = item.questid();
      questNpcItem.m_dwGroupID = item.groupid();
      questNpcItem.m_oDefine.load(item.data());
      questNpcItem.m_dwHp = item.hp();
      questNpcItem.bAcc = item.acc() == 1;

      m_list.push_back(questNpcItem);
      XDBG << "[任务NPC-char加载]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "加载npc id :" << questNpcItem.m_oDefine.getID() << "map :" << questNpcItem.m_dwMapID << "questid :" << questNpcItem.m_dwQuestID << "hp :" << questNpcItem.m_dwHp <<
        "pos : (" << questNpcItem.m_oDefine.getPos().x << questNpcItem.m_oDefine.getPos().y << questNpcItem.m_oDefine.getPos().z << ")" << XEND;
    }
  }
}

void UserQuestNpc::getCurMapNpc(std::set<SceneNpc*> & npcset)
{
  if (m_pUser->getScene() == nullptr)
    return;
  for (auto &s : m_list)
  {
    if (s.m_dwMapID != m_pUser->getScene()->getMapID())
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(s.m_qwTempID);
    if (npc == nullptr)
      continue;
    npcset.insert(npc);
  }
}

void UserQuestNpc::setQuestNpcInfo(DWORD dwQuestID, DWORD dwNpcID, DWORD dwMapID)
{
  for (auto &l : m_list)
  {
    if (l.m_dwQuestID == dwQuestID && l.m_oDefine.getID() == dwNpcID)
    {
      l.m_dwMapID = dwMapID;
      XLOG << "[任务NPC-修正]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << dwQuestID << "mapid修正为" << dwMapID << XEND;
    }
  }
}

void UserQuestNpc::setQuestStatus(DWORD dwQuestID, bool bAcc)
{
  for (auto &l : m_list)
  {
    if (l.m_dwQuestID == dwQuestID)
    {
      l.bAcc = bAcc;
      XLOG << "[任务NPC-修正]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "questid :" << dwQuestID << "acc修正为" << bAcc << XEND;
    }
  }
}


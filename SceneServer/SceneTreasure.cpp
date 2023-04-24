#include "SceneTreasure.h"
#include "MiscConfig.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "MsgManager.h"
#include "RewardConfig.h"
#include "SceneItem.h"
#include "SceneItemManager.h"
#include "Scene.h"

SceneTreasure::SceneTreasure(Scene* pScene, const STreasureCFG* pCFG) : m_pScene(pScene), m_pCFG(pCFG)
{

}

SceneTreasure::~SceneTreasure()
{

}

void SceneTreasure::onTreeDie(SceneNpc* npc)
{
  TreeNpc* pNpc = dynamic_cast<TreeNpc*>(npc);
  if (pNpc == nullptr)
    return;

  const STreasureMiscCFG& rCFG = MiscConfig::getMe().getTreasureCFG();
  DWORD dwNow = xTime::getCurSec();

  auto s = m_stGoldTree.setTree.find(pNpc);
  if (s != m_stGoldTree.setTree.end())
  {
    m_stGoldTree.setTree.erase(s);
    m_stGoldTree.setIndex.erase(pNpc->getPosIndex());
    m_stGoldTree.dwNextTime = dwNow + rCFG.dwGoldTreeRefreshTime;

    XDBG << "[北森寻宝-黄金树消失]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "消失了" << XEND;
  }

  s = m_stMagicTree.setTree.find(pNpc);
  if (s != m_stMagicTree.setTree.end())
  {
    m_stMagicTree.setTree.erase(s);
    m_stMagicTree.setIndex.erase(pNpc->getPosIndex());
    m_stMagicTree.dwNextTime = dwNow + rCFG.dwMagicTreeRefreshTime;
    XDBG << "[北森寻宝-魔法树消失]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "消失了" << XEND;
  }

  s = m_stHighTree.setTree.find(pNpc);
  if (s != m_stHighTree.setTree.end())
  {
    m_stHighTree.setTree.erase(s);
    m_stHighTree.setIndex.erase(pNpc->getPosIndex());
    m_stHighTree.dwNextTime = dwNow + rCFG.dwHighTreeRefreshTime;
    XDBG << "[北森寻宝-??树消失]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "消失了" << XEND;
  }

  pNpc->m_oFollower.removeAllTreeMonster();

  TreeListUserCmd cmd;
  cmd.add_dels(pNpc->id);
  PROTOBUF(cmd, send, len);

  xSceneEntrySet set;
  m_pScene->getAllEntryList(SCENE_ENTRY_USER, set);
  for (auto &s : set)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(s);
    if (pUser == nullptr)
      continue;
    if (pUser->m_oBuff.haveBuff(MiscConfig::getMe().getTreasureCFG().dwKnownBuffID) == false)
      continue;

    pUser->sendCmdToMe(send, len);
  }
}

void SceneTreasure::onEnterScene(SceneUser* pUser)
{
  if (pUser == nullptr || m_pCFG == nullptr)
    return;
  if (pUser->m_oBuff.haveBuff(MiscConfig::getMe().getTreasureCFG().dwKnownBuffID) == false)
    return;

  TreeListUserCmd cmd;
  for (auto &s : m_stGoldTree.setTree)
  {
    Tree* pTree = cmd.add_updates();
    if (pTree != nullptr)
    {
      pTree->set_id(s->id);
      pTree->set_typeid_(s->getNpcID());
      pTree->mutable_pos()->set_x(s->getPos().getX());
      pTree->mutable_pos()->set_y(s->getPos().getY());
      pTree->mutable_pos()->set_z(s->getPos().getZ());
      if (s->checkNineScreenShow(pUser))
        s->sendMeToUser(pUser);
      const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
      for (auto &p : monsters)
      {
        if (p->checkNineScreenShow(pUser))
          p->sendMeToUser(pUser);
      }
    }
  }
  for (auto &s : m_stMagicTree.setTree)
  {
    Tree* pTree = cmd.add_updates();
    if (pTree != nullptr)
    {
      pTree->set_id(s->id);
      pTree->set_typeid_(s->getNpcID());
      pTree->mutable_pos()->set_x(s->getPos().getX());
      pTree->mutable_pos()->set_y(s->getPos().getY());
      pTree->mutable_pos()->set_z(s->getPos().getZ());
      if (s->checkNineScreenShow(pUser))
        s->sendMeToUser(pUser);
      const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
      for (auto &p : monsters)
      {
        if (p->checkNineScreenShow(pUser))
          p->sendMeToUser(pUser);
      }
    }
  }
  for (auto &s : m_stHighTree.setTree)
  {
    Tree* pTree = cmd.add_updates();
    if (pTree != nullptr)
    {
      pTree->set_id(s->id);
      pTree->set_typeid_(s->getNpcID());
      pTree->mutable_pos()->set_x(s->getPos().getX());
      pTree->mutable_pos()->set_y(s->getPos().getY());
      pTree->mutable_pos()->set_z(s->getPos().getZ());
      if (s->checkNineScreenShow(pUser))
        s->sendMeToUser(pUser);
      const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
      for (auto &p : monsters)
      {
        if (p->checkNineScreenShow(pUser))
          p->sendMeToUser(pUser);
      }
    }
  }

  if (cmd.updates_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
}

void SceneTreasure::onLeaveScene(SceneUser* pUser)
{
  if (pUser == nullptr || m_pCFG == nullptr)
    return;

  TreeListUserCmd cmd;
  for (auto &s : m_stGoldTree.setTree)
  {
    cmd.add_dels(s->id);
    if (s->checkNineScreenShow(pUser))
      s->delMeToUser(pUser);
    const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
    for (auto &p : monsters)
    {
      if (p->checkNineScreenShow(pUser))
        p->delMeToUser(pUser);
    }
  }
  for (auto &s : m_stMagicTree.setTree)
  {
    cmd.add_dels(s->id);
    if (s->checkNineScreenShow(pUser))
      s->delMeToUser(pUser);
    const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
    for (auto &p : monsters)
    {
      if (p->checkNineScreenShow(pUser))
        p->delMeToUser(pUser);
    }
  }
  for (auto &s : m_stHighTree.setTree)
  {
    cmd.add_dels(s->id);
    if (s->checkNineScreenShow(pUser))
      s->delMeToUser(pUser);
    const std::set<SceneNpc*> monsters = s->m_oFollower.getTreeMonster();
    for (auto &p : monsters)
    {
      if (p->checkNineScreenShow(pUser))
        p->delMeToUser(pUser);
    }
  }

  if (cmd.dels_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
}

void SceneTreasure::goOtherPos(TreeNpc* npc)
{
  if (npc == nullptr || m_pScene == nullptr)
    return;
  const STreasureMiscCFG& rCFG = MiscConfig::getMe().getTreasureCFG();

  auto refreshPos = [&](STreeGroup& group) -> bool{
    if (group.setTree.find(npc) == group.setTree.end())
      return false;

    const STreasureNpc* pData = m_pCFG->getTree(ETREETYPE_GOLD);
    if (pData == nullptr)
      return false;
    DWORD oldIndex = npc->getPosIndex();
    DWORD dwIndex = pData->randPosIndex(m_stGoldTree.setIndex);
    if (dwIndex >= pData->vecPos.size())
      return false;

    xPos pos;
    if (m_pScene->getRandPos(pData->vecPos[dwIndex], 5, pos) == false)
      return false;
    npc->goTo(pos);
    npc->setPosIndex(dwIndex);

    group.setIndex.erase(oldIndex);
    group.setIndex.insert(dwIndex);

    TreeListUserCmd cmd;
    Tree* pTree = cmd.add_updates();
    pTree->set_id(npc->id);
    pTree->set_typeid_(npc->getNpcID());
    pTree->mutable_pos()->set_x(pos.getX());
    pTree->mutable_pos()->set_y(pos.getY());
    pTree->mutable_pos()->set_z(pos.getZ());
    PROTOBUF(cmd, send, len);

    xSceneEntrySet set;
    m_pScene->getAllEntryList(SCENE_ENTRY_USER, set);
    for (auto &s : set)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(s);
      if (pUser != nullptr && pUser->m_oBuff.haveBuff(rCFG.dwKnownBuffID) == true)
      {
        pUser->sendCmdToMe(send, len);
      }
    }

    XLOG << "[北森寻宝-树位置更新], 新的位置" << pos.x << pos.y << pos.z << "index:" << dwIndex << XEND;
    return true;
  };

  if (refreshPos(m_stGoldTree))
    return;
  if (refreshPos(m_stHighTree))
    return;
  if (refreshPos(m_stMagicTree))
    return;
}

void SceneTreasure::timer(DWORD curTime)
{
  if (m_pCFG == nullptr)
    return;

  const STreasureMiscCFG& rCFG = MiscConfig::getMe().getTreasureCFG();

  // gold tree
  if (curTime > m_stGoldTree.dwNextTime && m_stGoldTree.setTree.size() < rCFG.dwMaxGoldTree)
  {
    const STreasureNpc* pData = m_pCFG->getTree(ETREETYPE_GOLD);
    if (pData != nullptr)
    {
      DWORD dwIndex = pData->randPosIndex(m_stGoldTree.setIndex);
      if (dwIndex != DWORD_MAX)
      {
        xPos oPos;
        m_pScene->getRandPos(pData->vecPos[dwIndex], 5.0f, oPos);

        NpcDefine define;
        define.setID(pData->dwNpcID);
        define.setPos(oPos);
        //define.setDisptime(rCFG.dwDisTime);
        define.m_oVar.dwTreeType = ETREETYPE_GOLD;

        TreeNpc* pNpc = dynamic_cast<TreeNpc*>(SceneNpcManager::getMe().createNpc(define, m_pScene));
        if (pNpc != nullptr && m_stGoldTree.setTree.find(pNpc) == m_stGoldTree.setTree.end())
        {
          m_stGoldTree.setTree.insert(pNpc);
          m_stGoldTree.setIndex.insert(dwIndex);
          pNpc->setPosIndex(dwIndex);

          TreeListUserCmd cmd;
          Tree* pTree = cmd.add_updates();
          pTree->set_id(pNpc->id);
          pTree->set_typeid_(pNpc->getNpcID());
          pTree->mutable_pos()->set_x(pNpc->getPos().getX());
          pTree->mutable_pos()->set_y(pNpc->getPos().getY());
          pTree->mutable_pos()->set_z(pNpc->getPos().getZ());
          PROTOBUF(cmd, send, len);

          xSceneEntrySet set;
          m_pScene->getAllEntryList(SCENE_ENTRY_USER, set);
          for (auto &s : set)
          {
            SceneUser* pUser = dynamic_cast<SceneUser*>(s);
            if (pUser != nullptr && pUser->m_oBuff.haveBuff(rCFG.dwKnownBuffID) == true)
            {
              //MsgManager::sendMsg(pUser->id, 10, MsgParams("北森测试-金苹果"));
              pUser->sendCmdToMe(send, len);
            }
          }
        }
      }
    }
  }

  // magic tree
  if (curTime > m_stMagicTree.dwNextTime && m_stMagicTree.setTree.size() < rCFG.dwMaxMagicTree)
  {
    const STreasureNpc* pData = m_pCFG->getTree(ETREETYPE_MAGIC);
    if (pData != nullptr)
    {
      DWORD dwIndex = pData->randPosIndex(m_stMagicTree.setIndex);
      if (dwIndex != DWORD_MAX)
      {
        xPos oPos;
        m_pScene->getRandPos(pData->vecPos[dwIndex], 5.0f, oPos);

        NpcDefine define;
        define.setID(pData->dwNpcID);
        define.setPos(oPos);
        //define.setDisptime(rCFG.dwDisTime);
        define.m_oVar.dwTreeType = ETREETYPE_MAGIC;

        TreeNpc* pNpc = dynamic_cast<TreeNpc*>(SceneNpcManager::getMe().createNpc(define, m_pScene));
        if (pNpc != nullptr && m_stMagicTree.setTree.find(pNpc) == m_stMagicTree.setTree.end())
        {
          m_stMagicTree.setTree.insert(pNpc);
          m_stMagicTree.setIndex.insert(dwIndex);
          pNpc->setPosIndex(dwIndex);

          TreeListUserCmd cmd;
          Tree* pTree = cmd.add_updates();
          pTree->set_id(pNpc->id);
          pTree->set_typeid_(pNpc->getNpcID());
          pTree->mutable_pos()->set_x(pNpc->getPos().getX());
          pTree->mutable_pos()->set_y(pNpc->getPos().getY());
          pTree->mutable_pos()->set_z(pNpc->getPos().getZ());
          PROTOBUF(cmd, send, len);

          xSceneEntrySet set;
          m_pScene->getAllEntryList(SCENE_ENTRY_USER, set);
          for (auto &s : set)
          {
            SceneUser* pUser = dynamic_cast<SceneUser*>(s);
            if (pUser != nullptr && pUser->m_oBuff.haveBuff(rCFG.dwKnownBuffID) == true)
            {
              //MsgManager::sendMsg(pUser->id, 10, MsgParams("北森测试-魔法苹果"));
              pUser->sendCmdToMe(send, len);
            }
          }
        }
      }
    }
  }

  // high tree
  if (curTime > m_stHighTree.dwNextTime && m_stHighTree.setTree.size() < rCFG.dwMaxHighTree)
  {
    const STreasureNpc* pData = m_pCFG->getTree(ETREETYPE_HIGH);
    if (pData != nullptr)
    {
      DWORD dwIndex = pData->randPosIndex(m_stHighTree.setIndex);
      if (dwIndex != DWORD_MAX)
      {
        xPos oPos;
        m_pScene->getRandPos(pData->vecPos[dwIndex], 5.0f, oPos);

        NpcDefine define;
        define.setID(pData->dwNpcID);
        define.setPos(oPos);
        //define.setDisptime(rCFG.dwDisTime);
        define.m_oVar.dwTreeType = ETREETYPE_HIGH;

        TreeNpc* pNpc = dynamic_cast<TreeNpc*>(SceneNpcManager::getMe().createNpc(define, m_pScene));
        if (pNpc != nullptr && m_stHighTree.setTree.find(pNpc) == m_stHighTree.setTree.end())
        {
          m_stHighTree.setTree.insert(pNpc);
          m_stHighTree.setIndex.insert(dwIndex);
          pNpc->setPosIndex(dwIndex);

          TreeListUserCmd cmd;
          Tree* pTree = cmd.add_updates();
          pTree->set_id(pNpc->id);
          pTree->set_typeid_(pNpc->getNpcID());
          pTree->mutable_pos()->set_x(pNpc->getPos().getX());
          pTree->mutable_pos()->set_y(pNpc->getPos().getY());
          pTree->mutable_pos()->set_z(pNpc->getPos().getZ());
          PROTOBUF(cmd, send, len);

          xSceneEntrySet set;
          m_pScene->getAllEntryList(SCENE_ENTRY_USER, set);
          for (auto &s : set)
          {
            SceneUser* pUser = dynamic_cast<SceneUser*>(s);
            if (pUser != nullptr && pUser->m_oBuff.haveBuff(rCFG.dwKnownBuffID) == true)
            {
              MsgManager::sendMsg(pUser->id, 2101);
              pUser->sendCmdToMe(send, len);
            }
          }
        }
      }
    }
  }
}

void SceneTreasure::shakeTree(SceneUser* pUser, QWORD qwNpcID)
{
  if (m_pCFG == nullptr)
    return;

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (npc == nullptr || npc->getScene() != m_pScene)
    return;
  TreeNpc* pNpc = dynamic_cast<TreeNpc*>(npc);
  if (pNpc == nullptr)
    return;
  pNpc->setNextMoveTime(now() +  MiscConfig::getMe().getTreasureCFG().dwDisTime);

  const STreasureNpc* pTreasureNpc = m_pCFG->getTree(pNpc->getTreeType());
  if (pTreasureNpc == nullptr)
    return;

  const STreasureNpcProcess* pProcess = pTreasureNpc->getProcess(pNpc->getTreeIndex());
  if (pProcess == nullptr)
  {
    onTreeDie(pNpc);
    pNpc->setStatus(ECREATURESTATUS_LEAVE);
    return;
  }
  const STreasureNpcProcess* pNextProcess = pTreasureNpc->getProcess(pNpc->getTreeIndex() + 1);

  if (pProcess->eType == ETREESTATUS_MONSTER)
  {
    if (pNpc->m_oFollower.isTreeEmpty() == false)
      return;

    NpcDefine define;
    define.load(pProcess->oNpcData);
    define.setID(pProcess->dwNpcID);
    define.setLife(1);
    define.setPos(pNpc->getPos());
    define.m_oVar.m_dwBuffID = MiscConfig::getMe().getTreasureCFG().dwKnownBuffID;

    for (DWORD d = 0; d < pProcess->dwCount; ++d)
    {
      xPos oPos;
      if (pNextProcess == nullptr)
      {
        SceneNpc* npc = SceneNpcManager::getMe().createNpc(define, m_pScene);
        if (npc != nullptr)
        {
          /*UserActionNtf actioncmd;
          actioncmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
          actioncmd.set_charid(npc->id);
          actioncmd.set_value(npc->isMonster() == true ? MiscConfig::getMe().getTreasureCFG().dwShakeActionMonster : MiscConfig::getMe().getTreasureCFG().dwShakeActionNpc);
          PROTOBUF(actioncmd, actionsend, actionlen);
          npc->sendCmdToNine(actionsend, actionlen);
          */

          pNpc->setDataMark(EUSERDATATYPE_TREESTATUS);
          pNpc->refreshDataAtonce();
        }
      }
      else
      {
        define.m_oVar.m_qwNpcOwnerID = pNpc->id;
        SceneNpc* pMonster = SceneNpcManager::getMe().createNpc(define, m_pScene);
        if (pMonster != nullptr)
          pNpc->m_oFollower.addTreeMonster(pMonster);
      }
    }
    pNpc->addTreeIndex();
  }
  else if (pProcess->eType == ETREESTATUS_REWARD)
  {
    if (pNpc->m_oFollower.isTreeEmpty() == false)
      return;

    TVecItemInfo vecItem;
    RewardManager::roll(pProcess->dwRewardID, nullptr, vecItem, ESOURCE_MONSTERKILL);

    // create scene item
    Cmd::AddMapItem cmd;
    DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
    float fDropRange = MiscConfig::getMe().getSceneItemCFG().getRange(static_cast<DWORD>(vecItem.size()));
    for (size_t i = 0; i < vecItem.size(); ++i)
    {
      xPos dest;
      m_pScene->getRandPos(pNpc->getPos(), fDropRange, dest);
      vecItem[i].set_source(ESOURCE_TREASURE);
      SceneItem* pItem = SceneItemManager::getMe().createSceneItem(m_pScene, vecItem[i], dest);
      if (pItem != nullptr)
      {
        pItem->addOwner(pUser->id);
        pItem->fillMapItemData(cmd.add_items(), extraTime * (i + 1));
      }
    }

    // inform client
    PROTOBUF(cmd, send, len);
    m_pScene->sendCmdToNine(pNpc->getPos(), send, len);

    pNpc->addTreeIndex();
  }
  else
  {
    return;
  }

  if (pNextProcess == nullptr)
  {
    onTreeDie(pNpc);
    pNpc->setStatus(ECREATURESTATUS_LEAVE);
  }
  else
  {
    // npc pre action
    /*UserActionNtf actioncmd;
    actioncmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    actioncmd.set_charid(pNpc->id);
    actioncmd.set_value(MiscConfig::getMe().getTreasureCFG().dwShakeActionNpc);
    PROTOBUF(actioncmd, actionsend, actionlen);
    pNpc->sendCmdToNine(actionsend, actionlen);

    pNpc->setDataMark(EUSERDATATYPE_TREESTATUS);
    pNpc->refreshDataAtonce();*/

    ShakeTreeUserCmd cmd;
    cmd.set_npcid(qwNpcID);
    cmd.set_result(pProcess->eType);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }

  XLOG << "[北森寻宝-摇树]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "在地图 :" << m_pScene->id << m_pScene->name << "摇晃了" << pNpc->id << pNpc->getNpcID() << pNpc->name << "结果为" << pProcess->eType << XEND;
}


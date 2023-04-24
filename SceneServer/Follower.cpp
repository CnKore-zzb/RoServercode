#include "Follower.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"
#include "FeatureConfig.h"
//const char LOG_NAME[] = "Follower";

Follower::Follower(xSceneEntryDynamic *e):m_pEntry(e)
{
}

Follower::~Follower()
{
  clear();
}

void Follower::add(const SFollow& sfData)
{
  if (!m_pEntry || !m_pEntry->getScene()) return;

  NpcDefine def;
  def.setID(sfData.nameID);
  def.resetting();
  xPos p = m_pEntry->getPos() + xPos(1,0,1);
  m_pEntry->getScene()->getValidPos(p);
  def.setPos(p);
  def.setBehaviours(sfData.behaviours);
  def.m_oVar.m_qwFollowerID = m_pEntry->id;
  SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, m_pEntry->getScene());
  if (npc)
  {
    SFollowTemp data;
    data.id = npc->id;
    data.spdRatio = sfData.spdRatio;
    data.questid = sfData.questid;
    data.clearTime = sfData.clearTime;
    m_oListTempData.push_back(data);
    float spd = sfData.spdRatio * m_pEntry->getAttr(EATTRTYPE_MOVESPD);
    npc->setAttr(EATTRTYPE_MOVESPD, spd);
    if (data.clearTime)
      npc->setClearTime(data.clearTime);
  }
}

void Follower::addServant(NpcDefine def, SServantData& data, DWORD num /*=1*/, QWORD priAtkUser /*=0*/)
{
  if (!m_pEntry || !m_pEntry->getScene()) return;
  NpcDefine mDef = def;
  xPos p = mDef.getPos();
  m_pEntry->getScene()->getValidPos(p);
  mDef.setPos(p);
  mDef.m_oVar.m_qwNpcOwnerID = m_pEntry->id;
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (m_pEntry);
  if (pNpc == nullptr)
    return;
  if (data.bShapreDam)
    pNpc->setShareDam(data.bShapreDam);
  for (DWORD i = 0; i < num; ++i)
  {
    SceneNpc *npc = SceneNpcManager::getMe().createNpc(mDef, m_pEntry->getScene());
    if (npc)
    {
      SServantData stData;
      stData.qwTempID = npc->id;
      stData.bSupply = data.bSupply;
      stData.bSaperateLock = data.bSaperateLock;
      stData.bShapreDam = data.bShapreDam;

      m_oVecServants.push_back(stData);
      if (pNpc && pNpc->m_ai.getCurLockID())
        npc->m_ai.setCurLockID(pNpc->m_ai.getCurLockID());
      if (priAtkUser)
        npc->m_ai.setPriAttackUser(priAtkUser);
      npc->setShareDam(stData.bShapreDam);
    }
  }
}

void Follower::removeServant(QWORD guid)
{
  auto it = find_if(m_oVecServants.begin(), m_oVecServants.end(), [guid](const SServantData& r) -> bool
  {
    return r.qwTempID == guid;
  });
  if (it != m_oVecServants.end())
    m_oVecServants.erase(it);
}

void Follower::getServantIDs(TVecQWORD &vecIDs)
{
  if (m_oVecServants.empty())
    return;
  for (auto v = m_oVecServants.begin(); v != m_oVecServants.end(); ++v)
  {
    vecIDs.push_back((*v).qwTempID);
  }
}

void Follower::moveToMaster(xPos dest)
{
  SceneNpc* pNpc = dynamic_cast<SceneNpc*> (m_pEntry);
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;
  DWORD keepdis = FeatureConfig::getMe().getNpcAICFG().stServant.dwKeepDis;
  for (auto &v : m_oVecServants)
  {
    //if (v.bSaperateLock)
     //continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.qwTempID);
    if (npc && npc->getScene() && getDistance(npc->getPos(), dest) > keepdis)
    {
      xPos pos;
      if (npc->getScene()->getRandPos(dest, keepdis, pos))
        npc->m_ai.moveTo(pos);
    }
  }
}

void Follower::setLockByMaster(QWORD id)
{
  SceneNpc* pNpc = dynamic_cast<SceneNpc*>(m_pEntry);
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
    return;
  if (id == pNpc->id)
    return;
  for (auto &v : m_oVecServants)
  {
    if (v.bSaperateLock)
      continue;
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(v.qwTempID);
    if (npc)
    {
      npc->m_ai.setCurLockID(id);
    }
  }
}

void Follower::removeTreeMonster(SceneNpc* pNpc)
{
  auto s = m_setTreeMonster.find(pNpc);
  if (s == m_setTreeMonster.end())
    return;

  m_setTreeMonster.erase(pNpc);

  TreeNpc* npc = dynamic_cast<TreeNpc*>(m_pEntry);
  if (npc != nullptr && isTreeEmpty() == true)
    npc->refreshTreeStatus();
}

void Follower::removeAllTreeMonster()
{
  for (auto &s : m_setTreeMonster)
    s->setClearState();
}

void Follower::del(QWORD tempid)
{
  for (auto s = m_oListTempData.begin(); s != m_oListTempData.end(); )
  {
    if ((*s).id == tempid)
    {
      s = m_oListTempData.erase(s);
      break;
    }
    ++s;
  }
}

void Follower::delquest(DWORD dwQuestID)
{
  set<SceneNpc*> delnpc;
  for (auto s = m_oListTempData.begin(); s != m_oListTempData.end();)
  {
    if (s->questid == dwQuestID)
    {
      SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(s->id);
      if (pNpc != nullptr)
        delnpc.insert(pNpc);
      s = m_oListTempData.erase(s);
      continue;
    }

    ++s;
  }

  for (auto &s : delnpc)
    s->setClearState();
}

void Follower::enterScene()
{
  Scene* pScene = m_pEntry->getScene();
  if (pScene && pScene->getBaseCFG() && pScene->getBaseCFG()->noFollower())
    return;

  if (m_oListData.empty()) return;

  for (auto it=m_oListData.begin(); it!=m_oListData.end(); ++it)
  {
    add(*it);
  }
  m_oListData.clear();
}

void Follower::leaveScene()
{
  Scene* pScene = m_pEntry->getScene();
  if (pScene && pScene->getBaseCFG() && pScene->getBaseCFG()->noFollower())
    return;

  m_oListData.clear();
  std::list<SceneNpc*> npclist;
  for (auto it=m_oListTempData.begin(); it!=m_oListTempData.end(); ++it)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID((*it).id);
    if (npc)
    {
      SFollow sfData;
      sfData.nameID = npc->getNpcID();
      sfData.spdRatio = (*it).spdRatio;
      sfData.behaviours = npc->define.getBehaviours();
      sfData.questid = (*it).questid;
      sfData.clearTime = (*it).clearTime;
      m_oListData.push_back(sfData);
      //npc->setClearState();
      npclist.push_back(npc);
    }
  }

  for (auto &p : npclist)
  {
    if (p != nullptr)
      p->setClearState();
  }

  m_oListTempData.clear();
}

void Follower::clear()
{
  std::list<SceneNpc*> npclist;
  for (auto it=m_oListTempData.begin(); it!=m_oListTempData.end(); ++it)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID((*it).id);
    if (npc)
    {
      npclist.push_back(npc);
      //npc->setClearState();
    }
  }
  for (auto &p : npclist)
  {
    if (p != nullptr)
      p->removeAtonce();
      //p->setClearState();
  }
  m_oListData.clear();
  m_oListTempData.clear();
}

void Follower::get(std::list<SceneNpc *> &list)
{
  if (m_oListTempData.empty()) return;

  for (auto it=m_oListTempData.begin(); it!=m_oListTempData.end(); ++it)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID((*it).id);
    if (npc)
    {
      list.push_back(npc);
    }
  }
}


void Follower::load(const Cmd::BlobFollower &data)
{
  int size = data.list_size();
  if (!size) return;

  XLOG << "[Follower],load size:" << size << XEND;
  for (int i=0; i<size; ++i)
  {
    const FollowerItem& item = data.list(i);
    if (NpcConfig::getMe().getNpcCFG(item.id()) == nullptr)
    {
      XERR << "[Follower-加载]" << m_pEntry->id << m_pEntry->name << "加载 id :" << item.id() << "失败,未在 Table_Npc.txt 表中找到" << XEND;
      continue;
    }

    SFollow sfData;
    sfData.nameID = item.id();
    sfData.spdRatio = item.spdratio();
    sfData.behaviours = item.behaviours();
    sfData.questid = item.questid();
    sfData.clearTime = item.cleartime();
    m_oListData.push_back(sfData);
    XLOG << "[Follower],load:" << sfData.nameID << XEND;
  }
}

void Follower::save(Cmd::BlobFollower *data)
{
  if (!data) return;
  data->Clear();
  if (!m_oListData.empty())
  {
    XLOG << "[Follower],save size:" << m_oListData.size() << XEND;
    for (auto it=m_oListData.begin(); it!=m_oListData.end(); ++it)
    {
      FollowerItem *bf = data->add_list();
      bf->set_id((*it).nameID);
      bf->set_spdratio((*it).spdRatio);
      bf->set_behaviours((*it).behaviours);
      bf->set_questid((*it).questid);
      bf->set_cleartime((*it).clearTime);
      XLOG << "[Follower],save:" << (*it).nameID << XEND;
    }
  }

  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser != nullptr)
    XDBG << "[跟随者-保存]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "数据大小:" << data->ByteSize() << XEND;
}

void Follower::goTo(xPos p)
{
  for (auto &it : m_oListTempData)
  {
    SceneNpc *npc = SceneNpcManager::getMe().getNpcByTempID(it.id);
    if (npc)
    {
      npc->m_oMove.clear();
      npc->goTo(p);
    }
  }
}

float Follower::getMoveSpeed(QWORD id)
{
  for (auto &it : m_oListTempData)
  {
    if (it.id == id)
      return m_pEntry->getMoveSpeed() * (it.spdRatio);
  }
  return 0.0f;
}

void Follower::refreshMe()
{
  std::list<SceneNpc*> npclist;
  get(npclist);
  for (auto &s : npclist)
  {
    s->refreshDataAtonce();
  }
}

void Follower::onServantIDChange(QWORD oldguid, QWORD newguid)
{
  auto it = find_if(m_oVecServants.begin(), m_oVecServants.end(), [oldguid](const SServantData& r){
      return oldguid == r.qwTempID;
      });
  if (it == m_oVecServants.end())
    return;
  it->qwTempID = newguid;
}

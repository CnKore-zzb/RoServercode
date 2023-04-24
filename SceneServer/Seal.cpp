#include "Seal.h"
#include "SceneUser.h"
#include "SceneManager.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"
#include "SceneUserManager.h"
#include "SceneActManager.h"
#include "SceneServer.h"
#include "RedisManager.h"
#include "MsgManager.h"

// seal data
bool SSealItem::fromData(const SealItem& rItem)
{
  oPos.x = rItem.pos().x() / 1000.0f;
  oPos.y = rItem.pos().y() / 1000.0f;
  oPos.z = rItem.pos().z() / 1000.0f;

  pCFG = SealConfig::getMe().getSealCFG(rItem.config());
  if (pCFG == nullptr)
  {
    XLOG << "[Seal-load], ID:" << rItem.config() << "无效, 删除该封印" << XEND;
    return false;
  }
  bOwnSeal = rItem.ownseal();
  dwRefreshTime = rItem.refreshtime();
  bSealing = rItem.issealing();
  return pCFG != nullptr;
}

// toData 包含所有开启封印的地图, (包括不含有seal的sealitem) 存储数据库
bool SSealItem::toData(SealItem* pItem) const
{
  if (pItem == nullptr)
    return false;

  if (pCFG != nullptr)
    pItem->set_config(pCFG->dwID);

  pItem->mutable_pos()->set_x(oPos.x * ONE_THOUSAND);
  pItem->mutable_pos()->set_y(oPos.y * ONE_THOUSAND);
  pItem->mutable_pos()->set_z(oPos.z * ONE_THOUSAND);

  pItem->set_ownseal(bOwnSeal);
  pItem->set_refreshtime(dwRefreshTime);
  pItem->set_sealid(qwSealID);
  pItem->set_issealing(bSealing);
  return true;
}

// toClient 仅包含 含有seal的sealitem 发送client
bool SSealItem::toClient(SealItem* pItem) const
{
  if (pItem == nullptr)
    return false;

  pItem->mutable_pos()->set_x(oPos.x * 1000);
  pItem->mutable_pos()->set_y(oPos.y * 1000);
  pItem->mutable_pos()->set_z(oPos.z * 1000);

  if (bOwnSeal == false)
    return false;
  pItem->set_sealid(qwSealID);
  pItem->set_issealing(bSealing);
  pItem->set_etype(pCFG->eType);
  return true;
}

bool SSealData::fromData(const SealData& rData)
{
  dwMapID = rData.mapid();

  vecItem.clear();
  for (int i = 0; i < rData.items_size(); ++i)
  {
    SSealItem stItem;
    if (stItem.fromData(rData.items(i)) == false)
    {
      //XERR("[SSealData::fromData] id = %u error", rData.items(i).sealid());
      continue;
    }
    vecItem.push_back(stItem);
  }

  return true;
}

bool SSealData::toData(SealData* pData) const
{
  if (pData == nullptr)
    return false;

  pData->set_mapid(dwMapID);

  pData->clear_items();
  for (auto v = vecItem.begin(); v != vecItem.end(); ++v)
  {
    SealItem* pItem = pData->add_items();
    if (pItem == nullptr)
    {
      XERR << "[SSealData::toData] typeid = " << (v->pCFG == nullptr ? 0 : v->pCFG->dwID) << " create protobuf error" << XEND;
      continue;
    }
    v->toData(pItem);
  }

  return true;
}

bool SSealData::toClient(SealData* pData) const
{
  if (pData == nullptr)
    return false;

  pData->set_mapid(dwMapID);

  pData->clear_items();
  for (auto v = vecItem.begin(); v != vecItem.end(); ++v)
  {
    if (v->bOwnSeal == false)
      continue;
    SealItem* pItem = pData->add_items();
    if (pItem == nullptr)
    {
      XERR << "[SSealData::toClient] typeid = " << (v->pCFG == nullptr ? 0 : v->pCFG->dwID) << " create protobuf error" << XEND;
      continue;
    }
    v->toClient(pItem);
  }

  return true;
}

bool SSealData::haveSeal() const
{
  if (vecItem.empty())
    return false;
  auto v = find_if(vecItem.begin(), vecItem.end(), [](const SSealItem& r) -> bool
  {
    return r.bOwnSeal;
  });
  return v != vecItem.end();
}

const SSealItem* SSealData::getItem(QWORD qwID) const
{
  auto v = find_if(vecItem.begin(), vecItem.end(), [qwID](const SSealItem& r) -> bool{
    return r.qwSealID == qwID;
  });
  if (v != vecItem.end())
    return &(*v);

  return nullptr;
}

// seal
Seal::Seal(SceneUser* pUser) : m_pUser(pUser)
{

}

Seal::~Seal()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SEAL, m_pUser->id);
  if (key.length() > 3)
  {
    RedisManager::getMe().delData(key);
  }
}

bool Seal::load(const BlobSeal& oBlob)
{
  m_vecData.clear();
  for (int i = 0; i < oBlob.datas_size(); ++i)
  {
    SSealData stData;
    stData.fromData(oBlob.datas(i));
    m_vecData.push_back(stData);
  }
  m_vecQuestData.clear();
  for (int i = 0; i < oBlob.questseals_size(); ++i)
  {
    SSealData stData;
    stData.fromData(oBlob.questseals(i));
    m_vecQuestData.push_back(stData);
  }
  // 如果配置表改动, 对数据做相应处理
  {
    TVecSealItem diffItems;
    for (auto v = m_vecQuestData.begin(); v != m_vecQuestData.end(); )
    {
      for (auto it = v->vecItem.begin(); it != v->vecItem.end(); )
      {
        if (!it->pCFG)
        {
          it = v->vecItem.erase(it);
          continue;
        }
        if (it->pCFG->dwMapID != v->dwMapID)
        {
          diffItems.push_back(*it);
          it = v->vecItem.erase(it);
          continue;
        }
        ++it;
      }

      if (v->vecItem.empty())
      {
        v = m_vecQuestData.erase(v);
        continue;
      }
      ++v;
    }
    for (auto &v : diffItems)
    {
      if (!v.pCFG) continue;
      auto it = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [&](const SSealData& r) ->bool {
        return r.dwMapID == v.pCFG->dwMapID;
        });
      if (it != m_vecQuestData.end())
      {
        it->vecItem.push_back(v);
        continue;
      }
      SSealData sdata;
      sdata.dwMapID = v.pCFG->dwMapID;
      sdata.vecItem.push_back(v);
      m_vecQuestData.push_back(sdata);
    }
  }
  m_setOpenSealIDs.clear();
  for (int i = 0; i < oBlob.openseals_size(); ++i)
  {
    m_setOpenSealIDs.insert(oBlob.openseals(i));
  }
  m_dwNextProduceTime = oBlob.nexttime();

  // RedisManager
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SEAL, m_pUser->id);
  if (key.length() > 3)
  {
    RedisManager::getMe().setProtoData(key, &oBlob);
  }

  return true;
}

bool Seal::save(BlobSeal* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  for (auto v = m_vecData.begin(); v != m_vecData.end(); ++v)
  {
    SealData* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[Seal::toDataString] id = " << m_pUser->id << " name = " << m_pUser->name << " mapid = " << v->dwMapID << " protobuf error" << XEND;
      continue;
    }
    v->toData(pData);
  }
  for (auto v = m_vecQuestData.begin(); v != m_vecQuestData.end(); ++v)
  {
    SealData* pData = pBlob->add_questseals();
    if (pData == nullptr)
      continue;
    v->toData(pData);
  }
  pBlob->set_nexttime(m_dwNextProduceTime);
  for (auto &v : m_setOpenSealIDs)
    pBlob->add_openseals(v);

  XDBG << "[封印-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 数据大小 : " << pBlob->ByteSize() << XEND;

  // RedisManager
  /*string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SEAL, m_pUser->id);
  if (key.length() > 3)
    RedisManager::getMe().setProtoData(key, pBlob);
    */
  return true;
}

void Seal::sendAllSealInfo()
{
  QuerySeal cmd;
  // send my seal to client
  for (auto v = m_vecData.begin(); v != m_vecData.end(); ++v)
  {
    if (v->haveSeal() == false)
      continue;
    SealData* pData = cmd.add_datas();
    if (pData != nullptr)
      v->toClient(pData);
  }

  /*
  // send my seal to team
  if (cmd.datas_size() > 0)
    sendAllSealToTeam();

  if (m_pUser->getScene()== nullptr)
    return;
  DWORD mapid = m_pUser->getScene()->getMapID();
  // send team members seal to me
  set<SceneUser*> setUser;
  getTeamOtherUser(setUser);
  for (auto &s : setUser)
  {
    if (s == nullptr)
      continue;
    const TVecSealData& teamerSeal = s->getSeal().getSealData();
    for (auto v = teamerSeal.begin(); v != teamerSeal.end(); ++v)
    {
      // only send current map info
      if (v->haveSeal() == false || v->dwMapID != mapid)
        continue;
      SealData* pData = cmd.add_datas();
      if (pData != nullptr)
        v->toClient(pData);
    }
  }

  // send team's sealing seal to me
  if (m_pUser->getTeam() != nullptr)
  {
    const TVecTeamSealData& teamSealData = m_pUser->getTeam()->getSeal().getSealData();
    for (auto &v : teamSealData)
    {
      // only send current map info
      if (v.dwMapID != mapid)
        continue;
      SealData* pData = cmd.add_datas();
      pData->set_mapid(v.dwMapID);
      SealItem* pItem = pData->add_items();
      v.m_stCurItem.toClient(pItem);
    }
  }
  */

  if (cmd.datas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void Seal::sendAllSealToTeam()
{
  set<SceneUser*> setUser;
  getTeamOtherUser(setUser);
  if (setUser.empty())
    return;

  auto toTeam = [&](DWORD mapid, const SSealItem& sItem)
  {
    for (auto &s : setUser)
    {
      if (s == nullptr)
        continue;
      s->getSeal().addUpdateNewSeal(mapid, sItem);
    }
  };

  for (auto &v : m_vecData)
  {
    DWORD mapid = v.dwMapID;
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
        continue;
      // only send current map
      if (m_pUser->getScene() == nullptr || m_pUser->getScene()->getMapID() != mapid)
        continue;
      toTeam(mapid, r);
    }
  }
}

void Seal::onUserOffline()
{
  for (auto &v : m_vecData)
  {
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
        continue;
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
      if (pSeal == nullptr)
        continue;
      pSeal->setClearState();
      addTeamDelSeal(v.dwMapID, r);
    }
  }
}

const SSealItem* Seal::getSealItem(DWORD dwMapID, QWORD qwID)
{
  auto v = find_if(m_vecData.begin(), m_vecData.end(), [dwMapID](const SSealData& r) -> bool{
    return r.dwMapID == dwMapID;
  });
  if (v == m_vecData.end())
    return nullptr;

  auto o = find_if(v->vecItem.begin(), v->vecItem.end(), [qwID](const SSealItem& r) -> bool{
    return r.qwSealID == qwID;
  });
  if (o != v->vecItem.end() && o->bOwnSeal)
    return &(*o);

  return nullptr;
}

// put seal ownship to team, don't refresh map seal info at once, because the map seal info is already here, just to update when some teamer online or in/out team
bool Seal::removeSealItemToTeam(DWORD dwMapID, QWORD qwID)
{
  auto v = find_if(m_vecData.begin(), m_vecData.end(), [dwMapID](const SSealData& r) -> bool{
    return r.dwMapID == dwMapID;
  });
  if (v == m_vecData.end())
    return false;

  auto o = find_if(v->vecItem.begin(), v->vecItem.end(), [qwID](const SSealItem& r) -> bool{
    return r.qwSealID == qwID;
  });
  if (o == v->vecItem.end())
    return false;

  //addTeamDelSeal(dwMapID, *o);
  o->bOwnSeal = false;
  o->qwSealID = 0;
  return true;
}

void Seal::testOpenSeal()
{
  DWORD dwMapID = m_pUser->getScene()->getMapID();
  auto v = find_if(m_vecData.begin(), m_vecData.end(), [dwMapID](const SSealData& r) -> bool{
    return r.dwMapID == dwMapID;
  });
  if (v == m_vecData.end())
    return;
}

void Seal::openSeal(DWORD dwID)
{
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwID);
  if (pCFG == nullptr)
    return;

  if (pCFG->eType == ESEALTYPE_PERSONAL)
    addSeal(dwID);
  else if (pCFG->eType == ESEALTYPE_NORMAL)
  {
    m_setOpenSealIDs.insert(dwID);
    if (m_pUser->getTeamID() == 0)
      return;
    setDataMark();
    //m_pUser->getTeam()->getSeal().openSeal(dwID);
  }
  return;

  /*
  // 后面的代码不用了
  DWORD dwMapID = pCFG->dwMapID;
  auto v = find_if(m_vecData.begin(), m_vecData.end(), [dwMapID](const SSealData& r) -> bool{
    return r.dwMapID == dwMapID;
  });
  if (v == m_vecData.end())
  {
    SSealData stData;
    stData.dwMapID = dwMapID;

    SSealItem stItem;
    stItem.pCFG = pCFG;
    stData.vecItem.push_back(stItem);
    m_vecData.push_back(stData);
    m_dwNextProduceTime = 0;
    XLOG << "[Seal], 开启组队封印功能, 玩家id:" << m_pUser->id << "地图:" << dwMapID << "cofigID:" << dwID;
    return;
  }

  DWORD dwSealID = pCFG->dwID;
  auto o = find_if(v->vecItem.begin(), v->vecItem.end(), [dwSealID](const SSealItem& r) -> bool{
    return r.pCFG == nullptr ? false : r.pCFG->dwID == dwSealID;
  });
  if (o == v->vecItem.end())
  {
    SSealItem stItem;
    stItem.pCFG = pCFG;
    stItem.dwRefreshTime  = 0;//= xTime::getCurSec() + MiscConfig::getMe().getSealCFG().dwRefreshInterval;
    v->vecItem.push_back(stItem);
    m_dwNextProduceTime = 0;
    XLOG << "[Seal], 开启组队封印功能, 玩家id:" << m_pUser->id << ", 地图:" << dwMapID << ", cofigID:" << dwID << XEND;
  }
  */
}

void Seal::closeSeal(DWORD dwID)
{
  if (m_setOpenSealIDs.find(dwID) != m_setOpenSealIDs.end())
  {
    m_setOpenSealIDs.erase(dwID);
    XLOG << "[Seal-删除], 关闭组队封印, 玩家:" << m_pUser->name << m_pUser->id << "封印:" << dwID << XEND;
    return;
  }
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[Seal-删除], ID:" << dwID << "无效, 无法删除, 玩家:" << m_pUser->name << m_pUser->id << XEND;
    return;
  }
  for (auto v = m_vecQuestData.begin(); v != m_vecQuestData.end(); )
  {
    auto it = find_if(v->vecItem.begin(), v->vecItem.end(), [&](const SSealItem& r) ->bool{
      return r.pCFG && r.pCFG->dwID == dwID;
    });
    if (it != v->vecItem.end())
    {
      if (m_pUser->getScene() && m_pUser->getScene()->getMapID() == v->dwMapID)
      {
        SceneNpc* pSealNpc = SceneNpcManager::getMe().getNpcByTempID(it->qwSealID);
        if (pSealNpc)
          pSealNpc->setClearState();
      }
      addUpdateDelSeal(v->dwMapID, *it);
      v->vecItem.erase(it);
    }
    if (v->vecItem.empty())
    {
      XLOG << "[Seal-删除], 删除个人封印, 玩家" << m_pUser->name << m_pUser->id << "封印:" << dwID << XEND;
      v = m_vecQuestData.erase(v);
      continue;
    }
    ++v;
  }
}

void Seal::onQuestSubmit(DWORD dwQuestID)
{
  const SealCFG* pCFG = SealConfig::getMe().getSealCFGByQuest(dwQuestID);
  if (pCFG == nullptr)
    return;
  //openSeal(pCFG->dwID);
  m_setOpenSealIDs.insert(pCFG->dwID);
  setDataMark();
}

void Seal::timer(DWORD curTime)
{
  refreshSealItem(curTime);
  checkAddSeal(curTime);
  update();

  process(curTime);
  if (m_dwDMapID != 0 && m_dwExitDMapTime != 0 && curTime >= m_dwExitDMapTime)
  {
    if (m_pUser->getScene() && m_pUser->getScene()->id == m_dwDMapID)
      m_pUser->gomap(m_pUser->getScene()->getMapID(), GoMapType::Image, m_pUser->getPos());
    m_dwDMapID = 0;
    m_dwExitDMapTime = 0;
  }
  if (m_dwCountDownTime == curTime)
  {
    m_dwCountDownTime = 0;
    MsgManager::sendMsg(m_pUser->id, 1618, MsgParams(MiscConfig::getMe().getSealCFG().dwCountDownTime), EMESSAGETYPE_TIME_DOWN);
  }
}

bool Seal::beginSeal(QWORD qwID, EFinishType etype)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return false;

  if (beginSelfSeal(qwID))
    return true;

  // check team
  SceneTeamSeal* pTeamSeal = m_pUser->getTeamSeal();
  if (pTeamSeal == nullptr)
    return false;

  DWORD mapid = m_pUser->getScene()->getMapID();
  if (pTeamSeal->isSealing(mapid))
    return false;
  if (pTeamSeal->beginSeal(m_pUser, qwID, etype) == false)
    return false;

  XLOG << "[Seal], 开始修复组队封印, 玩家id:" << m_pUser->id << ", 地图:" << mapid << ", time:" << now() << XEND;
  return true;
}

void Seal::refreshSealItem(DWORD curTime)
{
  // 判断时间, 更换位置
  for (auto v = m_vecData.begin(); v != m_vecData.end(); ++v)
  {
    for (auto o = v->vecItem.begin(); o != v->vecItem.end(); ++o)
    {
      if (o->bOwnSeal == false || o->qwSealID == 0)
        continue;
      if (curTime < o->dwRefreshTime)
        continue;

      SceneNpc* pOldSeal = SceneNpcManager::getMe().getNpcByTempID(o->qwSealID);
      if (pOldSeal == nullptr)
        continue;
      xPos p;
      if (getRandomPos(m_pUser->getScene(), *o, p) == false)
        continue;
      pOldSeal->goTo(p);
      o->oPos = p;
      // notify team users to update

      o->dwRefreshTime = curTime + MiscConfig::getMe().getSealCFG().dwChangePosTime;
      XLOG << "[Seal], 封印位置变动, 玩家id:" << m_pUser->id << ", sealid:" << pOldSeal->id << ", 地图:" << v->dwMapID << XEND;

      addTeamNewSeal(v->dwMapID, *o);

      setDataMark();
    }
  }
}

SceneNpc* Seal::createSeal(Scene* pScene, const SSealItem& sItem, bool needNewPos)
{
  const SealCFG* pCFG = sItem.pCFG;
  if (pScene == nullptr || pCFG == nullptr)
    return nullptr;

  xPos desPos;

  if (needNewPos == true)
  {
    if (getRandomPos(pScene, sItem, desPos) == false)
      return nullptr;
  }
  else
  {
    desPos = sItem.oPos;
  }

  NpcDefine oDefine;
  oDefine.setID(MiscConfig::getMe().getSealCFG().dwSealNpcID);
  oDefine.resetting();
  oDefine.setPos(desPos);
  oDefine.m_oVar.m_qwTeamUserID = m_pUser->id;
  SceneNpc* pSeal = SceneNpcManager::getMe().createNpc(oDefine, pScene);
  XLOG << "[Seal], 创建封印, 玩家id:" << m_pUser->id << ", configID:" << pCFG->dwID << XEND;
  return pSeal;
}


void Seal::checkAddSeal(DWORD curTime)
{
  if (curTime < m_dwNextProduceTime)
    return;
  m_dwNextProduceTime = curTime + MiscConfig::getMe().getSealCFG().dwSealRefresh;
  if (haveEnoughSeal() == true)
    return;

  // get a valid map randomly
  DWORD i = 0;
  TVecDWORD vecIndex;
  for (auto &v : m_vecData)
  {
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
      {
        vecIndex.push_back(i);
        break;
      }
    }
    ++i;
  }

  DWORD size = vecIndex.size();
  if (size == 0)
    return;

  DWORD randIndex = randBetween(1, size) - 1;
  randIndex = vecIndex[randIndex];

  TVecSealItem& vecItem = m_vecData[randIndex].vecItem;
  DWORD mapid = m_vecData[randIndex].dwMapID;

  // 仅刷新在当前地图
  //mapid = m_pUser->getScene()->getMapID();
  auto v = find_if(vecItem.begin(), vecItem.end(), [](const SSealItem& r) ->bool
  {
    return r.bOwnSeal == false;
  });
  if (v == vecItem.end())
    return;

  // 若是当前地图，召唤封印,通知当前地图的队友
  if (mapid == m_pUser->getScene()->getMapID())
  {
    SceneNpc* pSeal = createSeal(m_pUser->getScene(), *v, true);
    if (pSeal == nullptr)
      return;
    v->oPos = pSeal->getPos();
    v->qwSealID = pSeal->id;
    v->bOwnSeal = true;
    v->dwRefreshTime = now() + MiscConfig::getMe().getSealCFG().dwChangePosTime;
    addTeamNewSeal(mapid, *v);
  }
  // 若不在当前地图,仅标记当前地图拥有封印,仅自己用于大地图显示
  else
  {
    v->bOwnSeal = true;
    addUpdateNewSeal(mapid, *v);
    XLOG << "[Seal], 玩家 " << m_pUser->id << " 在地图 " << mapid << " 开启封印" << XEND;
  }
}

void Seal::setDataMark()
{
  m_pUser->refreshDataAtonce();
  /*SceneTeam* pTeam = m_pUser->getTeam();
  if (pTeam != nullptr)
  {
    const TVecSceneTeamMembers& teamers = pTeam->getTeamMembers();
    for (auto &t : teamers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(t.getGUID());
      if (pUser == nullptr)
        continue;
      pUser->setDataMark(EUSERDATATYPE_SEAL);
    }
  }
  */
}

void Seal::addTeamNewSeal(DWORD mapid, const SSealItem& sItem)
{
  set<SceneUser*> setUser;
  getTeamOtherUser(setUser);
  setUser.insert(m_pUser);

  for (auto &s : setUser)
  {
    if (s == nullptr)
      continue;
    s->getSeal().addUpdateNewSeal(mapid, sItem);
  }
}

void Seal::addUpdateNewSeal(DWORD mapid, const SSealItem& sItem)
{
  auto v = find_if(m_vecNewData.begin(), m_vecNewData.end(), [&mapid](const SSealData& r) -> bool{
    return r.dwMapID == mapid;
  });
  if (v == m_vecNewData.end())
  {
    SSealData sData;
    sData.dwMapID = mapid;
    sData.vecItem.push_back(sItem);
    m_vecNewData.push_back(sData);
  }
  else
  {
    v->vecItem.push_back(sItem);
  }
}

void Seal::addTeamDelSeal(DWORD mapid, const SSealItem& sItem)
{
  set<SceneUser*> setUser;
  getTeamOtherUser(setUser);
  setUser.insert(m_pUser);

  for (auto &s : setUser)
  {
    if (s == nullptr)
      continue;
    s->getSeal().addUpdateDelSeal(mapid, sItem);
  }
}

void Seal::addUpdateDelSeal(DWORD mapid, const SSealItem& sItem)
{
  auto v = find_if(m_vecDelData.begin(), m_vecDelData.end(), [&mapid](const SSealData& r) -> bool{
    return r.dwMapID == mapid;
  });
  if (v == m_vecDelData.end())
  {
    SSealData sData;
    sData.dwMapID = mapid;
    sData.vecItem.push_back(sItem);
    m_vecDelData.push_back(sData);
  }
  else
  {
    v->vecItem.push_back(sItem);
  }
}


void Seal::getTeamOtherUser(set<SceneUser*>& userSet)
{
  if (m_pUser == nullptr || m_pUser->getTeamID() == 0)
    return;

  const GTeam& rTeam = m_pUser->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m.second.charid());
    if (pUser == nullptr || pUser == m_pUser)
      continue;
    if (pUser->getScene() != m_pUser->getScene())
      continue;
    userSet.insert(pUser);
  }
}

bool Seal::haveEnoughSeal() const
{
  DWORD sealNum = 0;
  for (auto &v : m_vecData)
  {
    for (auto &r : v.vecItem)
    {
      sealNum = (r.bOwnSeal ? sealNum + 1 : sealNum);
    }
  }
  DWORD maxSealNum = 10; // MiscConfig::getMe().getSealConfig().getMaxSealNum();
  return sealNum >= maxSealNum;
}

void Seal::update()
{
  if (m_vecDelData.empty() && m_vecNewData.empty())
    return;

  UpdateSeal cmd;
  for (auto &v : m_vecNewData)
  {
    SealData* pData = cmd.add_newdata();
    if (pData == nullptr)
      continue;
    v.toClient(pData);
  }
  for (auto &v : m_vecDelData)
  {
    SealData* pData = cmd.add_deldata();
    if (pData == nullptr)
      continue;
    v.toClient(pData);
  }
  if (cmd.newdata_size() > 0 || cmd.deldata_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  m_vecDelData.clear();
  m_vecNewData.clear();
}

// 仅在pUser 成为 m_pUser 的队友的时刻调用一次, 若pSeal->sendMeToUser() 多次调用, client会产生重叠
void Seal::addMySealToUser(SceneUser* pUser)
{
  if (pUser == m_pUser)
    return;
  for (auto &v : m_vecData)
  {
    DWORD mapid = v.dwMapID;
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
        continue;
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
      if (pSeal == nullptr)
        continue;
      // 地图 更新
      pUser->getSeal().addUpdateNewSeal(mapid, r);
      // seal 可见
      pSeal->sendMeToUser(pUser);
    }
  }
}

// 仅在pUser 与m_pUser解除队友关系时调用一次
void Seal::delMySealToUser(SceneUser* pUser)
{
  if (pUser == m_pUser)
    return;
  for (auto &v : m_vecData)
  {
    DWORD mapid = v.dwMapID;
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
        continue;
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
      if (pSeal == nullptr)
        continue;
      // 地图 更新
      pUser->getSeal().addUpdateDelSeal(mapid, r);
      // seal 不可见
      pSeal->delMeToUser(pUser);
    }
  }
}

bool Seal::getRandomPos(Scene* pScene, const SSealItem& sItem, xPos& outPos)
{
  if (pScene == nullptr || sItem.pCFG == nullptr)
    return false;

  const TVecSealPos& vecPos = sItem.pCFG->vecRefreshPos;
  DWORD size = vecPos.size();
  if (size == 0)
  {
    XERR << "[Seal], 封印配置错误，找不到坐标点，id = " << sItem.pCFG->dwID << XEND;
    return false;
  }
  DWORD randIndex = randBetween(1, size) - 1;
  xPos pos = vecPos[randIndex].oPos;
  float r = vecPos[randIndex].dwRange;
  if (r == 0)
  {
    pScene->getValidPos(pos, outPos);
    return true;
  }
  else if(pScene->getRandPos(pos, r, outPos) == false)
  {
    XERR << "[Seal], 召唤封印, 找不到合适的坐标点" << XEND;
    return false;
  }
  return true;
}


// 仅刷在当前地图
void Seal::onEnterScene()
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;
  TVecQWORD sealIDs;
  DWORD cur = now();
  for (auto &v : m_vecData)
  {
    if (v.dwMapID != m_pUser->getScene()->getMapID())
    {
      for (auto &r : v.vecItem)
      {
        r.qwSealID = 0;
      }
      continue;
    }
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == true)
      {
        bool newpos = (cur >= r.dwRefreshTime);
        SceneNpc* pseal = createSeal(m_pUser->getScene(), r, newpos);
        if (pseal == nullptr)
          continue;
        r.qwSealID = pseal->id;
        r.oPos = pseal->getPos();
        r.dwRefreshTime = newpos ? cur + MiscConfig::getMe().getSealCFG().dwChangePosTime : r.dwRefreshTime;
        sealIDs.push_back(pseal->id);
        XLOG << "[Seal], 封印npc进入地图, id:" << pseal->id << ", 地图:" << v.dwMapID << XEND;
      }
    }
  }
  // if has team seal send timeinfo to user
  /*if (m_pUser->getTeam() != nullptr)
  {
    m_pUser->getTeam()->getSeal().sendTimerInfo(m_pUser);
  }*/
  // 设置seal 机关状态
  for (auto &q : sealIDs)
  {
    Cmd::UserActionNtf cmd;
    cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
    cmd.set_charid(q);
    cmd.set_value(1001);
    PROTOBUF(cmd, send, len);
    set<SceneUser*> userSet;
    getTeamOtherUser(userSet);
    for (auto &p : userSet)
    {
      p->sendCmdToMe(send, len);
    }
  }
  sendAllSealInfo();
  selfSealEnterScene();
}

void Seal::onLeaveScene()
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;

  DWORD mapid = m_pUser->getScene()->getMapID();
  set<SceneUser*> setUser;
  getTeamOtherUser(setUser);
  // del my seal info to me and teamers
  for (auto &v : m_vecData)
  {
    for (auto &r : v.vecItem)
    {
      if (r.bOwnSeal == false)
        continue;
      // only del current map info to teamers
      addUpdateDelSeal(v.dwMapID, r);
      if (v.dwMapID == mapid)
      {
        for (auto &s : setUser)
          s->getSeal().addUpdateDelSeal(v.dwMapID, r);
      }
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
      if (pSeal == nullptr)
        continue;
      pSeal->setClearState();
      XLOG << "[Seal], 玩家离开地图, 删除场景封印,玩家:" << m_pUser->id << "封印:" << pSeal->id << XEND;
    }
  }

  /*
  // del teamers's seal info to me
  for (auto &s : setUser)
  {
    if (s == nullptr)
      continue;
    const TVecSealData& teamerSeal = s->getSeal().getSealData();
    for (auto v = teamerSeal.begin(); v != teamerSeal.end(); ++v)
    {
      // only del current map info
      if (v->haveSeal() == false || v->dwMapID != mapid)
        continue;
      for (auto &r : v->vecItem)
      {
        addUpdateDelSeal(mapid, r);
      }
    }
  }

  // del team's sealing seal to me
  if (m_pUser->getTeam() != nullptr)
  {
    const TVecTeamSealData& teamSealData = m_pUser->getTeam()->getSeal().getSealData();
    for (auto &v : teamSealData)
    {
      // only del current map info
      if (v.dwMapID != mapid)
        continue;
      addUpdateDelSeal(mapid, v.m_stCurItem);
    }
  }
  */

  for (auto &v : m_vecData)
  {
    if (v.dwMapID != m_pUser->getScene()->getMapID())
      continue;
    for (auto &r : v.vecItem)
    {
      SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
      if (pSeal == nullptr)
        continue;
      pSeal->setClearState();
    }
  }
  selfSealLeaveScene();
  update();
}

//--------------------------------------------- 个人修复封印 ----------------------------------------
// 个人封印
void Seal::addSeal(DWORD dwID)
{
  //if (m_pUser->getScene() == nullptr)
    //return;
  const SealCFG* pCFG = SealConfig::getMe().getSealCFG(dwID);
  if (pCFG == nullptr || pCFG->eType != ESEALTYPE_PERSONAL)
  {
    XERR << "[SealSelf], 不合理的封印添加, user:" << m_pUser->id << " 封印:" << dwID << XEND;
    return;
  }
  DWORD dwMapID = pCFG->dwMapID;
  auto v = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [dwMapID](const SSealData& r) -> bool{
    return r.dwMapID == dwMapID;
  });
  if (v == m_vecQuestData.end())
  {
    SSealData stData;
    stData.dwMapID = dwMapID;

    SSealItem stItem;
    stItem.pCFG = pCFG;
    stItem.bOwnSeal = true;

    if (dwMapID == m_pUser->getUserSceneData().getOnlineMapID())
    {
      SceneNpc* pSeal = createSelfSeal(stItem);
      if (pSeal == nullptr)
        return;
      stItem.oPos = pSeal->getPos();
      stItem.qwSealID = pSeal->id;
    }

    stData.vecItem.push_back(stItem);
    m_vecQuestData.push_back(stData);
    addUpdateNewSeal(dwMapID, stItem);
    XLOG << "[SealSelf], 添加个人封印, user:" << m_pUser->id << "封印:" << dwID << XEND;
    return;
  }

  DWORD dwSealID = pCFG->dwID;
  auto o = find_if(v->vecItem.begin(), v->vecItem.end(), [dwSealID](const SSealItem& r) -> bool{
    return r.pCFG == nullptr ? false : r.pCFG->dwID == dwSealID;
  });
  if (o != v->vecItem.end())
    return;
  SSealItem stItem;
  stItem.pCFG = pCFG;
  stItem.bOwnSeal = true;
  if (dwMapID == m_pUser->getUserSceneData().getOnlineMapID())
  {
    SceneNpc* pSeal = createSelfSeal(stItem);
    if (pSeal == nullptr)
      return;
    stItem.oPos = pSeal->getPos();
    stItem.qwSealID = pSeal->id;
  }
  v->vecItem.push_back(stItem);
  addUpdateNewSeal(dwMapID, stItem);
  XLOG << "[SealSelf], 添加个人封印, user:" << m_pUser->id << "封印:" << dwID << XEND;
}

SceneNpc* Seal::createSelfSeal(const SSealItem& sItem)
{
  const SealCFG* pCFG = sItem.pCFG;
  if (m_pUser->getScene() == nullptr || pCFG == nullptr)
    return nullptr;

  xPos desPos;
  if (sItem.bSealing == false)
  {
    if (getRandomPos(m_pUser->getScene(), sItem, desPos) == false)
      return nullptr;
  }
  else
    desPos = sItem.oPos;

  NpcDefine oDefine;
  oDefine.setID(MiscConfig::getMe().getSealCFG().dwSealNpcID);
  oDefine.resetting();
  oDefine.setPos(desPos);
  oDefine.m_oVar.m_qwTeamUserID = m_pUser->id;
  oDefine.m_oVar.m_qwQuestOwnerID = m_pUser->id;
  if (m_pUser->getScene()->isDScene())
  {
    oDefine.setBehaviours(oDefine.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);
  }
  SceneNpc* pSeal = SceneNpcManager::getMe().createNpc(oDefine, m_pUser->getScene());
  XLOG << "[SealSelf], 创建个人封印npc, user:" << m_pUser->id << "封印:" << pCFG->dwID << XEND;
  return pSeal;
}

bool Seal::beginSelfSeal(QWORD id)
{
  if (m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return false;

  DWORD mapid = m_pUser->getScene()->getMapID();
  auto v = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [&mapid](const SSealData& r) ->bool 
  {
    return r.dwMapID == mapid;
  });
  if (v == m_vecQuestData.end())
    return false;
  auto s = find_if(v->vecItem.begin(), v->vecItem.end(), [&id](const SSealItem& r) -> bool{
      return r.qwSealID == id;
      });
  if (s == v->vecItem.end() || s->pCFG == nullptr)
  {
    //XLOG("[Seal], 个人封印修复, 找不到对应封印, user:%llu, map:%u", m_pUser->id, mapid);
    return false;
  }

  if (!m_pUser->getScene()->isDScene())
  {
    if (m_pUser->getScene()->m_oImages.add(s->pCFG->dwDMapID, m_pUser, s->oPos, MiscConfig::getMe().getSealCFG().dwSealRange, s->pCFG->dwID))
      s->bSealing = true;
    return true;
  }

  // 已在修复
  if (m_vecSealingData.empty() == false)
    return rebeginSeal(id);

  XLOG << "[Seal], 开始修复个人封印, user:" << m_pUser->id << "封印:" << s->pCFG->dwID << XEND;

  // set issealing
  // s->bSealing = true;
  addUpdateNewSeal(mapid, *s);

  SceneActBase* pActBase = SceneActManager::getMe().createSceneAct(m_pUser->getScene(), s->oPos, MiscConfig::getMe().getSealCFG().dwSealRange, 0, EACTTYPE_SEAL);
  if (pActBase == nullptr)
    return false;
  SceneActSeal* pActSeal = dynamic_cast<SceneActSeal*> (pActBase);
  if (pActSeal)
    pActSeal->setPrivateUser(m_pUser);
  if (!pActBase->enterScene(m_pUser->getScene()))
  {
    XERR << "[Seal], 添加Act失败, user:" << m_pUser->id << "封印:" << s->pCFG->dwID << XEND;
    SceneActManager::getMe().delSceneAct(pActBase);
    return false;
  }
  // add seal data
  SRepairSeal sData;
  sData.dwSealStopTime = now() + s->pCFG->dwMaxTime;
  sData.dwMapID = m_pUser->getScene()->id;
  sData.pOwnUser = m_pUser;
  sData.m_stCurItem.qwSealID = s->qwSealID;
  sData.m_stCurItem.pCFG = s->pCFG;
  sData.m_stCurItem.oPos = s->oPos;
  sData.m_setActUserID.insert(m_pUser->id);
  //sData.m_setSealUser.insert(m_pUser);
  sData.qwActID = pActBase->id;
  m_vecSealingData.push_back(sData);

  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(s->qwSealID);
  if (pSeal == nullptr)
    return false;

  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  message.set_value(2001);
  message.set_charid(pSeal->id);
  PROTOBUF(message, gsend, glen);
  pSeal->setGearStatus(2002);
  m_pUser->sendCmdToMe(gsend, glen);

  // send to client to begin
  BeginSeal cmd;
  cmd.set_sealid(s->qwSealID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  return true;
}

bool Seal::rebeginSeal(QWORD id)
{
  if (m_vecSealingData.empty())
    return false;
  if (m_vecSealingData[0].m_stCurItem.qwSealID != id)
    return false;

  if (m_vecSealingData[0].m_stCurItem.pCFG == nullptr || m_vecSealingData[0].dwCurValue == m_vecSealingData[0].m_stCurItem.pCFG->dwMaxValue)
    return false;
  //m_vecSealingData[0].m_setSealUser.insert(m_pUser);
  XLOG << "[SelfSeal], 重新开始修复封印, 玩家id:" << m_pUser->id << "封印:" << id << XEND;
  return true;
}

void Seal::process(DWORD curTime)
{
  // dscene auto begin
  if (m_vecSealingData.empty() && m_pUser->getScene())
  {
    auto v = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [&](const SSealData& r) ->bool{
        for (auto &s : r.vecItem)
        {
          if (s.bSealing && m_pUser->getScene()->getMapID() == r.dwMapID)
            return true;
        }
        return false;
    });
    if (v != m_vecQuestData.end())
    {
      auto s = find_if(v->vecItem.begin(), v->vecItem.end(), [&](const SSealItem& r) -> bool{
          return r.bSealing;
          });
      if (s != v->vecItem.end())
      {
        if (m_pUser->getScene()->isDScene())
          beginSelfSeal(s->qwSealID);
      }
    }
  }

  if (!m_vecSealingData.empty())
  {
    SRepairSeal& sData = m_vecSealingData[0];
    if (sData.checkFailure(curTime) == true)
    {
      sData.failSeal();
      failSeal(sData.m_stCurItem.qwSealID);
      return;
    }
    sData.process(curTime);
    if (sData.checkRepairOk())
    {
      sData.preFinishSeal();
    }
    if (sData.checkOk(curTime))
    {
      sData.finishSeal();
      finishSeal(sData.m_stCurItem.qwSealID);
    }
  }
  if (!m_vecFinishSealData.empty())
  {
    if (m_vecFinishSealData.begin()->checkExitAction(m_dwExitDMapTime))
      m_vecFinishSealData.clear();
  }
}

void Seal::failSeal(QWORD sealid)
{
  if (m_vecSealingData.empty())
    return;
  SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(m_vecSealingData[0].m_stCurItem.qwSealID);
  if (pSeal == nullptr)
    return;

  auto v = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [&](const SSealData& r) -> bool {
    return m_pUser->getScene() && m_pUser->getScene()->getMapID() == r.dwMapID;
      });
  if (v == m_vecQuestData.end())
    return;
  auto s = find_if(v->vecItem.begin(), v->vecItem.end(), [&](const SSealItem& r) -> bool{
    return r.qwSealID == sealid;
      });
  if (s == v->vecItem.end())
    return;
  s->bSealing = false;
  addUpdateNewSeal(v->dwMapID, *s);
  XLOG << "[SealSelf], 个人封印修复失败, user:" << m_pUser->id << "map:" << v->dwMapID << XEND;

  EndSeal cmd;
  cmd.set_success(false);
  if (s->pCFG)
    cmd.set_sealid(s->pCFG->dwID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  Cmd::UserActionNtf ucmd;
  ucmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);
  ucmd.set_charid(pSeal->id);
  ucmd.set_value(1001);
  PROTOBUF(ucmd, usend, ulen);
  m_pUser->sendCmdToMe(usend, ulen);

  pSeal->setGearStatus(1002);

  m_vecSealingData.clear();
  if (m_pUser->getScene() && m_pUser->getScene()->isDScene())
    delSceneImage();
}

void Seal::finishSeal(QWORD sealid)
{
  // do quest
  auto v = find_if(m_vecQuestData.begin(), m_vecQuestData.end(), [&](const SSealData& r) -> bool {
    return m_pUser->getScene() && m_pUser->getScene()->getMapID() == r.dwMapID;
      });
  if (v == m_vecQuestData.end())
    return;
  auto s = find_if(v->vecItem.begin(), v->vecItem.end(), [&](const SSealItem& r) -> bool{
    return r.qwSealID == sealid;
      });
  if (s == v->vecItem.end())
    return;

  EndSeal cmd;
  cmd.set_success(true);
  if (s->pCFG)
    cmd.set_sealid(s->pCFG->dwID);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  addUpdateDelSeal(v->dwMapID, *s);
  // submit quest
  if (s->pCFG)
  {
    m_pUser->getQuest().onSeal(s->pCFG->dwID);
  }

  XLOG << "[SealSelf], 个人封印修复完成, user:" << m_pUser->id << "map:" << v->dwMapID << XEND;
  if (!m_vecSealingData.empty())
  {
    m_vecFinishSealData.clear();
    m_vecFinishSealData.push_back(*(m_vecSealingData.begin()));
    m_vecSealingData.clear();
  }
  v->vecItem.erase(s);
  if (v->vecItem.empty())
    m_vecQuestData.erase(v);

  if (m_pUser->getScene() && m_pUser->getScene()->isDScene())
  {
    delSceneImage();
    DScene* pDScene = dynamic_cast<DScene*>(m_pUser->getScene());
    if (pDScene != nullptr)
    {
      m_dwExitDMapTime = now() + pDScene->getRaidEndTime();
      m_dwDMapID = m_pUser->getScene()->id;
      m_dwCountDownTime = m_dwExitDMapTime - MiscConfig::getMe().getSealCFG().dwCountDownTime;
      //MsgManager::sendMsg(m_pUser->id, 1618, MsgParams(pDScene->getRaidEndTime()), EMESSAGETYPE_TIME_DOWN);
    }
  }
}

void Seal::selfSealEnterScene()
{
  if (m_pUser->getScene() == nullptr)
    return;
  for (auto &v : m_vecQuestData)
  {
    for (auto &r : v.vecItem)
    {
      if (m_pUser->getScene()->isDScene() && r.bSealing == false)
        continue;

      if (v.dwMapID == m_pUser->getScene()->getMapID())
      {
        if (!m_pUser->getScene()->isDScene())
          r.bSealing = false;

        SceneNpc* pSeal = createSelfSeal(r);
        if (pSeal == nullptr)
          continue;
        r.oPos = pSeal->getPos();
        r.qwSealID = pSeal->id;
      }
      r.bOwnSeal = true;
      addUpdateNewSeal(v.dwMapID, r);
    }
  }
}

void Seal::selfSealLeaveScene()
{
  if (m_pUser->getScene() == nullptr)
    return;
  for (auto &v : m_vecSealingData)
  {
    v.failSeal();
    failSeal(v.m_stCurItem.qwSealID);
  }
  m_vecSealingData.clear();
  for (auto &v : m_vecQuestData)
  {
    for (auto &r : v.vecItem)
    {
      if (v.dwMapID == m_pUser->getScene()->getMapID())
      {
        SceneNpc* pSeal = SceneNpcManager::getMe().getNpcByTempID(r.qwSealID);
        if (pSeal == nullptr)
          continue;
        pSeal->setClearState();
      }
      addUpdateDelSeal(v.dwMapID, r);
    }
  }
  m_vecFinishSealData.clear();
  // clear seal mark
  m_dwDMapID = 0;
  m_dwExitDMapTime = 0;
  m_dwCountDownTime = 0;
}

void Seal::onBeBreakSkill()
{
  if (m_vecSealingData.empty())
    return;
  //SRepairSeal& sData = m_vecSealingData[0];
  //sData.m_setSealUser.erase(m_pUser);
  XLOG << "[SealSelf], 个人封印修复被打断, user:" << m_pUser->id << XEND;
}

void Seal::onMonsterDie(SceneNpc* npc)
{
  if (m_vecSealingData.empty())
    return;
  if (npc == nullptr)
    return;
  m_vecSealingData[0].m_setCalledMonster.erase(npc->id);
}

void Seal::delSceneImage()
{
   DScene* pDScene = dynamic_cast<DScene*> (m_pUser->getScene());
   if (pDScene == nullptr)
     return;

   Scene* pRealScene = SceneManager::getMe().getSceneByID(m_pUser->getScene()->getMapID());
   if (pRealScene)
   {
     pRealScene->m_oImages.del(m_pUser->id, pDScene->getRaidID());
     return;
   }
   DelSceneImage cmd;
   cmd.set_guid(m_pUser->id);
   cmd.set_realscene(m_pUser->getScene()->getMapID());
   cmd.set_etype(ESCENEIMAGE_SEAL);
   cmd.set_raid(pDScene->getRaidID());
   PROTOBUF(cmd, send, len);
   thisServer->sendCmdToSession(send, len);
}


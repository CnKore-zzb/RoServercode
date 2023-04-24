#include "SpEffect.h"
#include "xSceneEntryDynamic.h"
#include "SceneUserManager.h"
#include "SceneMap.pb.h"
#include "SceneServer.h"
#include "TableManager.h"
#include "GuidManager.h"
#include "GMCommandRuler.h"
#include "SceneUser.h"

SpEffect::SpEffect(xSceneEntryDynamic *user) :m_pEntry(user)
{
}

SpEffect::~SpEffect()
{
}

void SpEffect::save(Cmd::BlobSpEffect *data)
{
  if (!data)
    return;
  DWORD curSec = now();
  timer(curSec);
  for (auto it = m_mapSpEffectData.begin(); it != m_mapSpEffectData.end(); ++it)
  {
    for (auto &v : it->second)
    {
      data->add_data()->CopyFrom(v);
    }
  }

  for (auto it = m_mapWhoLineMe.begin(); it != m_mapWhoLineMe.end(); ++it)
  {
    BlobLineData* pData = data->add_wholineme();
    if (pData)
    {
      pData->set_otherid(it->first);
      pData->set_expiretime(it->second);
    }
  }
}

void SpEffect::load(const Cmd::BlobSpEffect &data)
{
  DWORD curTime = now();
  for (int i = 0; i < data.data_size(); ++i)
  {
    const Cmd::SpEffectData& spEffectData = data.data(i);
    if (curTime > spEffectData.expiretime())
      continue;
    TVecSpEffectData& rVecData = m_mapSpEffectData[spEffectData.id()];
    rVecData.push_back(spEffectData);
    XLOG << "[特效-加载] " << m_pEntry->getTempID() <<m_pEntry->getName()  << spEffectData.id() << spEffectData.guid() << XEND;
  }
  
  for (int i = 0; i < data.wholineme_size(); ++i)
  {
    const Cmd::BlobLineData& rData = data.wholineme(i);
    if (curTime > rData.expiretime())
      continue;
    m_mapWhoLineMe[rData.otherid()] = rData.expiretime();
  }
}

void SpEffect::timer(DWORD curTime)
{
  std::vector<std::pair<DWORD, std::string>> vecDels;
  for (auto it = m_mapSpEffectData.begin(); it != m_mapSpEffectData.end(); ++it)
  {
    TVecSpEffectData& rVecData = it->second;
    for (auto& v : rVecData)
    {
      if (curTime > v.expiretime())
      {
        vecDels.push_back(std::make_pair(v.id(), v.guid()));
      }
      else
      {
        const SSpEffect* pCFG = TableManager::getMe().getSpEffectCFG(v.id());
        if (pCFG != nullptr)
        {
          bool dieDel = pCFG->isDelWhenDieOrNotSameMap();
          bool outRangeDel = pCFG->isOutRangeDel();
          if (dieDel || outRangeDel)
          {
            for (int i = 0; i < v.entity_size(); ++i)
            {
              xSceneEntryDynamic* pTarget = xSceneEntryDynamic::getEntryByID(v.entity(i));
              if (dieDel)
              {
                if (pTarget == nullptr || !pTarget->isAlive() || !m_pEntry->isAlive() || pTarget->getScene() == nullptr || pTarget->getScene() != m_pEntry->getScene())
                {
                  vecDels.push_back(std::make_pair(v.id(), v.guid()));
                  break;
                }
              }

              if (outRangeDel)
              {
                if (pTarget == nullptr || pTarget->getScene() != m_pEntry->getScene() || getXZDistance(pTarget->getPos(), m_pEntry->getPos()) > pCFG->getMaxDis())
                {
                  vecDels.push_back(std::make_pair(v.id(), v.guid()));
                  break;
                }
              }
            }
          }
        }
      }
    }
  }

  for (auto&v : vecDels)
  {
    del(v.first, v.second);
  }

  for (auto it = m_mapWhoLineMe.begin(); it != m_mapWhoLineMe.end();)
  {
    if (curTime > it->second)
      it = m_mapWhoLineMe.erase(it);
    else
      ++it;
  }
}

void SpEffect::onLeaveScene()
{
  std::vector<std::pair<DWORD, std::string>> vecDels;

  for (auto it = m_mapSpEffectData.begin(); it != m_mapSpEffectData.end(); ++it)
  {
    for (auto& v : it->second)
    {
      const SSpEffect* pCFG = TableManager::getMe().getSpEffectCFG(v.id());
      if (pCFG != nullptr && pCFG->isDelWhenDieOrNotSameMap())
        vecDels.push_back(std::make_pair(v.id(), v.guid()));
    }
  }

  for (auto&v : vecDels)
    del(v.first, v.second);
}

bool SpEffect::add(DWORD id, TSetQWORD& target, bool bTarLined)
{
  const SSpEffect* pCFG = TableManager::getMe().getSpEffectCFG(id);
  if (pCFG == nullptr)
  {
    XERR << "[特效-创建] " << m_pEntry->getTempID() <<m_pEntry->getName()  << " 找不到配置, id" << id << XEND;
    return false;
  }

  if (m_pEntry->getScene() == nullptr)
    return false;
  checkTarget(target);

  DWORD curTime = now();
  DWORD expireTime = 0;
  TVecSpEffectData& rVecData = m_mapSpEffectData[id];
  bool bNew = false;
  TVecSpEffectData::iterator it = rVecData.end();
  if (!rVecData.empty())
  {
    it = std::find_if(rVecData.begin(), rVecData.end(), [&](const Cmd::SpEffectData& item)->bool {
      if ((DWORD)item.entity_size() != target.size())
        return false;
      DWORD i = 0;
      for (auto &v : target)
      {
        if (item.entity(i) != v)
          return false;
        i++;
      }
      return true;
    });

    if (it == rVecData.end())
      bNew = true;
  }

  if (rVecData.empty() || bNew)
  {
    Cmd::SpEffectData blobData;

    std::string guid = GuidManager::getMe().newGuidStr(thisServer->getZoneID(), m_pEntry->getScene()->getMapID());
    blobData.set_guid(guid);
    blobData.set_id(id);
    blobData.set_expiretime(curTime + pCFG->getDuration());
    expireTime = blobData.expiretime();
    for (auto &v : target)
    {
      blobData.add_entity(v);
    }
    rVecData.push_back(blobData);
    onAdd(pCFG, blobData);
    send(blobData, true);
    XLOG << "[特效-创建-新加] " << m_pEntry->getTempID() <<m_pEntry->getName() << blobData.ShortDebugString() << XEND;
  }
  else
  {
    it->set_expiretime(it->expiretime() + pCFG->getDuration());
    expireTime = it->expiretime();
    onAdd(pCFG, *it);
    XLOG << "[特效-创建-追加] " << m_pEntry->getTempID() <<m_pEntry->getName()  <<it->ShortDebugString()<< XEND;
  }
  
  //连线才记录
  SceneUser* pLineUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pLineUser && (pCFG->id == 1 || bTarLined))
  {
    for (auto& s:target)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(s);
      if (!pUser)
        continue;
      pUser->getSpEffect().beLined(pLineUser->id, expireTime);
    }
  }

  return true;
}

void SpEffect::del(DWORD dwId, const std::string& guid)
{
  auto it = m_mapSpEffectData.find(dwId);
  if (it == m_mapSpEffectData.end())
    return;

  for (auto subIt = it->second.begin(); subIt != it->second.end(); ++subIt)
  {
    if (guid == subIt->guid())
    {
      XLOG << "[特效-删除] " << m_pEntry->getTempID() <<m_pEntry->getName()  << subIt->ShortDebugString() << XEND;
      send(*subIt, false);
      it->second.erase(subIt);
      break;
    }
  }
}

void SpEffect::del(DWORD dwId, QWORD qwIgnoreId)
{
  auto it = m_mapSpEffectData.find(dwId);
  if (it == m_mapSpEffectData.end())
    return;

  for (auto subIt = it->second.begin(); subIt != it->second.end();)
  {
    bool ignore = false;
    if (qwIgnoreId != 0)
    {
      for (int i = 0; i < subIt->entity_size(); ++i)
      {
        if (subIt->entity(i) == qwIgnoreId)
        {
          ignore = true;
          break;
        }
      }
    }
    if (ignore)
    {
      ++subIt;
      continue;
    }
    XLOG << "[特效-删除id] " << m_pEntry->getTempID() <<m_pEntry->getName()  << subIt->ShortDebugString() << XEND;
    send(*subIt, false);
    subIt = it->second.erase(subIt);
  }
}

void SpEffect::checkTarget(TSetQWORD& target)
{
  for (auto it = target.begin(); it != target.end(); )
  {
    auto* p = xSceneEntryDynamic::getEntryByID(*it);
    if (p == nullptr)
    {
      it = target.erase(it);
      continue;
    }
    ++it;
  }
}

void SpEffect::collectSpEffectData(Cmd::MapUser* pData)
{
  if (!pData)
    return;

  for (auto it = m_mapSpEffectData.begin(); it != m_mapSpEffectData.end(); ++it)
  {
    for (auto &v : it->second)
    {
      pData->add_speffectdata()->CopyFrom(v);
    }
  }
}

void SpEffect::collectSpEffectData(Cmd::MapNpc* pData)
{
  if (!pData)
    return;

  for (auto it = m_mapSpEffectData.begin(); it != m_mapSpEffectData.end(); ++it)
  {
    for (auto &v : it->second)
    {
      pData->add_speffectdata()->CopyFrom(v);
    }
  }
}

void SpEffect::onAdd(const SSpEffect* pCFG, const Cmd::SpEffectData& blobData)
{
  xLuaData oGmData = pCFG->getOnAdd();
  
  if (blobData.entity_size() == 0)
    return ;
  //use to other user    
  oGmData.setData("GM_Target", 1, true);
  for (int i = 0; i < blobData.entity_size(); ++i)
  {
    QWORD uid = blobData.entity(i);
    SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(uid);
    if (!pTargetUser)
    {
      return;
    }
    //xPos myPos = pUser->getPos();
    //xPos otherPos = pTargetUser->getPos();
    //if (checkDistance(myPos, otherPos, 10) == false)
    //{
    //  XERR << "[物品使用-指定玩家]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "目标用户：" << pTargetUser->id << "距离不满足" << XEND;
    //  MsgManager::sendMsg(pUser->id, 860);
    //  return false;
    //}
    std::stringstream ss;
    ss << "id" << i + 1;      // start from 1 
    oGmData.setData(ss.str(), uid, true);
  }

  // process command  todo 不支持怪物使用gm
  if (GMCommandRuler::getMe().execute(m_pEntry, oGmData) == false)
  {
    //XERR << "[物品使用-指定玩家]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "use item :" << m_pCFG->dwTypeID << "cmd run error" << XEND;
    return ;
  }
}

void SpEffect::onDel(const SSpEffect* pCFG, const Cmd::SpEffectData& blobData)
{

}

void SpEffect::send(const Cmd::SpEffectData& blobData, bool isAdd/* = true*/)
{
  Cmd::SpEffectCmd message;
  Cmd::SpEffectData* pData = message.mutable_data();
  if (!pData)
    return;
  message.set_senderid(m_pEntry->getTempID());
  pData->CopyFrom(blobData);
  message.set_isadd(isAdd);
  PROTOBUF(message, send, len);
  m_pEntry->sendCmdToNine(send, len);
}

void SpEffect::beLined(QWORD charId, DWORD expireTime)
{ 
  m_mapWhoLineMe[charId] = expireTime;
  XDBG << "[连线-被连线] charid" << m_pEntry->id << "连接者" << charId << "到期时间" << expireTime << XEND;
}

DWORD SpEffect::whoLinedMeCnt()
{
  return m_mapWhoLineMe.size();
}

bool SpEffect::hasLine(DWORD id)
{
  auto it = m_mapSpEffectData.find(id);
  return it != m_mapSpEffectData.end() && !it->second.empty();
}

bool SpEffect::hasLineEntry(DWORD lineid, QWORD entryid)
{
  auto it = m_mapSpEffectData.find(lineid);
  if (it == m_mapSpEffectData.end())
    return false;

  for (auto &v : it->second)
  {
    for (int i = 0; i < v.entity_size(); ++i)
    {
      if (v.entity(i) == entryid)
        return true;
    }
  }

  return false;
}

bool SpEffect::beLineByOther(DWORD lineid)
{
  for (auto &m : m_mapWhoLineMe)
  {
    xSceneEntryDynamic* entry = xSceneEntryDynamic::getEntryByID(m.first);
    if (entry && entry->getSpEffect().hasLineEntry(lineid, m_pEntry->id))
      return true;
  }
  return false;
}


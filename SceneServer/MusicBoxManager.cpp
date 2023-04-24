#include "MusicBoxManager.h"
#include "SceneServer.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "SceneAct.h"
#include "SceneActManager.h"
#include "PlatLogManager.h"
#include "SceneUserManager.h"
#include "MsgManager.h"

// music item
void SMusicItem::toData(MusicItem* pData)
{
  if (pData == nullptr)
    return;

  pData->set_charid(qwCharID);
  pData->set_demandtime(dwDemandTime);
  pData->set_mapid(dwMapID);
  pData->set_npcid(dwNpcID);
  pData->set_musicid(dwMusicID);
  pData->set_starttime(dwStartTime);
  pData->set_endtime(dwEndTime);
  pData->set_status(dwState);
  pData->set_name(strName);
}

// music manager
MusicBoxManager::MusicBoxManager()
{

}

MusicBoxManager::~MusicBoxManager()
{

}

bool MusicBoxManager::init()
{
  if (loadMusicItem() == false)
    return false;

  return true;
}

void MusicBoxManager::timer(DWORD dwCurTime)
{
  FUN_TIMECHECK_30();
  for (auto m = m_mapMusicItem.begin(); m != m_mapMusicItem.end(); ++m)
  {
    auto v = find_if(m->second.begin(), m->second.end(), [](const SMusicItem& r) -> bool{
      return r.dwStartTime != 0;
    });
    bool bPlay = v != m->second.end();
    for (auto v = m->second.begin(); v != m->second.end();)
    {
      // update invalid music status
      if (v->dwState == 1 || v->pBase == nullptr)
      {
        MusicUpdateCmd cmd;
        v->toData(cmd.mutable_item());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToData(send, len);

        QWORD qwNpcID = v->qwNpcID;
        XLOG << "[点唱机管理-删除] charid :" << v->qwCharID << "dtime :" << v->dwDemandTime << "musicid :" << v->dwMusicID << "starttime :" << v->dwStartTime << XEND;
        v = m->second.erase(v);
        broadcastMusicList(qwNpcID, false);
        continue;
      }

      // check and play music
      if (!bPlay && v->dwState == 0 && v->dwStartTime == 0)
      {
        MusicNpc* pNpc = dynamic_cast<MusicNpc*>(SceneNpcManager::getMe().getNpcByTempID(v->qwNpcID));
        if (pNpc != nullptr)
        {
          v->dwStartTime = dwCurTime;
          MusicUpdateCmd cmd;
          v->toData(cmd.mutable_item());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToData(send, len);
          pNpc->setMusicData(v->dwMusicID, v->dwStartTime, v->qwCharID);
          broadcastMusicList(v->qwNpcID, true);
          bPlay = true;
          XLOG << "[点唱机管理-播放] music :" << v->dwMusicID << "mapid :" << v->dwMapID << "charid :" << v->qwCharID << "status :" << v->dwState << "开始播放" << XEND;
        }
      }

      // check music stop
      if (v->dwState == 0 && v->dwStartTime != 0 && v->dwStartTime + v->pBase->getTableInt("MusicTim") < dwCurTime)
      {
        MusicNpc* pNpc = dynamic_cast<MusicNpc*>(SceneNpcManager::getMe().getNpcByTempID(v->qwNpcID));
        if (pNpc != nullptr)
        {
          v->dwState = 1;
          v->dwEndTime = dwCurTime;
          pNpc->setMusicData(0, 0, 0);
          XLOG << "[点唱机管理-播放] music :" << v->dwMusicID << "mapid :" << v->dwMapID << "charid :" << v->qwCharID << "status :" << v->dwState << "播放结束" << XEND;
        }
      }

      ++v;
    }
  }

}

bool MusicBoxManager::queryMusicList(SceneUser* pUser, QWORD qwNpcID, bool bSync /*= false*/)
{
  if (pUser == nullptr)
    return false;
  pUser->setBrowse(true);

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->getScene()->base == nullptr || pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
    return false;

  DWORD dwMapID = pNpc->getScene()->getMapID();
  auto m = m_mapMusicItem.find(pNpc->getScene()->getMapID());
  if (m == m_mapMusicItem.end())
  {
    QueryMusicList cmd;
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
    return false;
  }

  DWORD dwMaxCount = 0;
  QueryMusicList cmd;
  for (auto v = m->second.begin(); v != m->second.end(); ++v)
  {
    v->toData(cmd.add_items());
    XLOG << "[点唱机管理-列表请求] charid :" << v->qwCharID << "dtime :" << v->dwDemandTime << "musicid :" << v->dwMusicID << "starttime :" << v->dwStartTime << "status :" << v->dwState << XEND;
    if (++dwMaxCount >= MAX_MUSIC_LIST_COUNT)
      break;
  }

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  if (!bSync)
  {
    XLOG << "[点唱机管理-列表请求]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "在 mapid :" << dwMapID << "请求" << pNpc->id << pNpc->getNpcID() << pNpc->name << "音乐列表, 获得了" << cmd.items_size() << "个音乐列表" << XEND;
    return true;
  }

  XLOG << "[点唱机管理-列表同步]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "在 mapid :" << dwMapID << "请求" << pNpc->id << pNpc->getNpcID() << pNpc->name << "音乐列表, 获得了" << cmd.items_size() << "个音乐列表" << XEND;
  return true;
}

bool MusicBoxManager::demandMusic(SceneUser* pUser, QWORD qwNpcID, DWORD dwMusicID)
{
  if (pUser == nullptr)
  {
    XERR << "[点唱机管理-音乐点播] 未找到玩家" << XEND;
    return false;
  }
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr || pNpc->getScene() == nullptr)
  {
    XERR << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "点播" << dwMusicID << "失败, 未找到" << qwNpcID << "npc" << XEND;
    return false;
  }
  if (pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
  {
    XERR << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, npc" << qwNpcID << pNpc->getNpcID() << "不是点唱机npc" << XEND;
    return false;
  }

  DWORD dwMapID = pNpc->getScene()->getMapID();
  const SMapCFG* pMapBase = MapConfig::getMe().getMapCFG(dwMapID);
  if (pMapBase == nullptr)
  {
    XERR << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, 地图" << dwMapID << "未在 Table_Map.txt 表中找到" << XEND;
    return false;
  }

  const SMusicBase* pMusicBase = TableManager::getMe().getMusicCFG(dwMusicID);
  if (pMusicBase == nullptr)
  {
    XERR << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, 未在 Table_MusicBox.txt 表中找到" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr || pMainPack->checkItemCount(dwMusicID) == false)
  {
    XERR << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败," << EPACKTYPE_MAIN << "中数量不足" << XEND;
    return false;
  }
  pMainPack->reduceItem(dwMusicID, ESOURCE_MUSICBOX);

  SMusicItem stItem;
  stItem.qwCharID = pUser->id;
  stItem.qwNpcID = pNpc->id;

  stItem.dwNpcID = pNpc->getNpcID();
  stItem.dwMapID = dwMapID;
  stItem.dwMusicID = dwMusicID;
  stItem.dwDemandTime = xTime::getCurSec();
  stItem.dwStartTime = 0;
  stItem.dwState = 0;
  stItem.strName = pUser->name;

  stItem.pBase = pMusicBase;

  TVecMusicItem& vecItem = m_mapMusicItem[dwMapID];
  vecItem.push_back(stItem);

  MusicUpdateCmd cmd;
  stItem.toData(cmd.mutable_item());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);

  broadcastMusicList(pNpc->id, true);
  XLOG << "[点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "在 mapid :" << dwMapID << "请求" << qwNpcID << pNpc->getNpcID() << pNpc->name << "成功点播了" << dwMusicID << "音乐" << XEND;

  if (dwMusicID == MUSIC_LOVE_FOREVER && pUser->m_oHands.has() == true)
  {
    SceneUser* pOther = SceneUserManager::getMe().getUserByID(pUser->m_oHands.getOtherID());
    if (pOther != nullptr)
    {
      pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_HANDMUSIC, pOther->id, 1);
      pOther->getShare().addCalcData(ESHAREDATATYPE_MOST_HANDMUSIC, pUser->id, 1);
      pOther->getAchieve().onTravel();
    }
  }
  pUser->getShare().addCalcData(ESHAREDATATYPE_MOST_MUSICCD, dwMusicID, 1);
  pUser->getAchieve().onDemandMusic();
  pUser->getServant().onFinishEvent(ETRIGGER_PLAY_MUSIC);
  pUser->getServant().onGrowthFinishEvent(ETRIGGER_PLAY_MUSIC);

  //platlog
  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_PlayMusic;
  PlatLogManager::getMe().eventLog(thisServer,
    pUser->getUserSceneData().getPlatformId(),
    pUser->getZoneID(),
    pUser->accid,
    pUser->id,
    eid,
    pUser->getUserSceneData().getCharge(), eType, 0, 1);
  PlatLogManager::getMe().ItemOperLog(thisServer,
    pUser->getUserSceneData().getPlatformId(),
    pUser->getZoneID(),
    pUser->accid,
    pUser->id,
    eType,
    eid,
    EItemOperType_PlayMusic,
    dwMusicID,
    1);

  pUser->getEvent().onPlayMusic(dwMusicID);

  return true;
}

void MusicBoxManager::initMusicNpc(SceneNpc* pNpc)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
    return;

  const SceneBase* pBase = pNpc->getScene()->base;
  if (pBase == nullptr)
    return;

  MusicNpc* pMusicNpc = dynamic_cast<MusicNpc*>(pNpc);
  if (pMusicNpc == nullptr)
    return;

  auto m = m_mapMusicItem.find(pBase->getMapInfo().id);
  if (m == m_mapMusicItem.end())
    return;

  for (auto v = m->second.begin(); v != m->second.end(); ++v)
  {
    if (v->dwNpcID == pNpc->getNpcID())
    {
      v->qwNpcID = pNpc->id;
      if (v->dwState != 2 && v->dwStartTime != 0)
      {
        pMusicNpc->setMusicData(v->dwMusicID, v->dwStartTime, v->qwCharID);
        pMusicNpc->setGearStatus(MiscConfig::getMe().getSystemCFG().dwMusicStatusPlay);
        XLOG << "[点唱机管理-初始化]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "继续播放 id :" << v->dwMusicID << "timer :" << v->dwStartTime << "charid :" << v->qwCharID << XEND;
      }
    }
  }
}

bool MusicBoxManager::loadMusicItem()
{
  // load music from db
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "music");
  if (pField == nullptr)
  {
    XERR << "[点唱机管理-加载点唱机] 无法获取数据库" << REGION_DB << XEND;
    return false;
  }

  xRecordSet set;
  char szWhere[64] = {0};
  snprintf(szWhere, 64, "zoneid = %u and status = 0", thisServer->getZoneID());
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, szWhere);
  if (ret == QWORD_MAX)
  {
    XERR << "[点唱机管理-加载点唱机] 查询失败" << XEND;
    return false;
  }

  for (QWORD q = 0; q < ret; ++q)
  {
    SMusicItem stItem;

    stItem.qwCharID = set[q].get<QWORD>("charid");
    stItem.dwNpcID = set[q].get<DWORD>("npcid");
    stItem.dwMapID = set[q].get<DWORD>("mapid");
    stItem.dwMusicID = set[q].get<DWORD>("musicid");
    stItem.dwDemandTime = set[q].get<DWORD>("demandtime");
    stItem.dwStartTime = set[q].get<DWORD>("starttime");
    stItem.dwState = set[q].get<DWORD>("state");
    stItem.strName = set[q].getString("name");

    if (stItem.dwState != 0)
      continue;

    stItem.pBase = TableManager::getMe().getMusicCFG(stItem.dwMusicID);
    if (stItem.pBase == nullptr)
    {
      XERR << "[点唱机管理-加载点唱机] mapid :" << stItem.dwMapID << "userid :" << stItem.qwCharID << "musicid :" << stItem.dwMusicID << "未在Table_MusicBox.txt中找到" << XEND;
      continue;
    }

    XLOG << "[点唱机管理-加载点唱机] charid :" << stItem.qwCharID << "dtime :" << stItem.dwDemandTime << "musicid :" << stItem.dwMusicID << "starttime :" << stItem.dwStartTime << XEND;
    m_mapMusicItem[stItem.dwMapID].push_back(stItem);
  }

  XLOG << "[点唱机管理-加载点唱机] 成功加载" << XEND;
  return true;
}

void MusicBoxManager::broadcastMusicList(QWORD qwNpcID, bool bPlay)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr || pNpc->getScene()== nullptr)
    return;

  const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
  bool bNotify = false;
  if (bPlay)
  {
    if (pNpc->getGearStatus() == rCFG.dwMusicStatusStop || pNpc->getGearStatus() == 0)
      bNotify = true;
  }
  else
  {
    if (pNpc->getScene() != nullptr)
    {
      auto m = m_mapMusicItem.find(pNpc->getScene()->id);
      if (m != m_mapMusicItem.end() && m->second.empty() == true && pNpc->getGearStatus() == rCFG.dwMusicStatusPlay)
        bNotify = true;
    }
  }

  if (bNotify)
    pNpc->setGearStatus(bPlay ? rCFG.dwMusicStatusPlay : rCFG.dwMusicStatusStop);

  xSceneEntrySet uSet;
  pNpc->getScene()->getEntryListInNine(SCENE_ENTRY_USER, pNpc->getPos(), uSet);
  for (auto s = uSet.begin(); s != uSet.end(); ++s)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(*s);
    if (pUser == nullptr)
      continue;
    if (pUser->getBrowse() == true)
      queryMusicList(pUser, qwNpcID, true);

    if (bNotify)
      pNpc->sendGearStatus(pUser);
  }
}

void MusicBoxManager::initActivityMusicNpc(SceneNpc* npc)
{
  if (npc == nullptr || npc->getNpcID() != MiscConfig::getMe().getJoyLimitCFG().dwMusicNpcID)
    return;

  MusicNpc* mnpc = dynamic_cast<MusicNpc*>(npc);
  if (mnpc == nullptr)
    return;

  mnpc->setMusicData(MiscConfig::getMe().getJoyLimitCFG().dwMusicID, now(), 0, true);

  broadcastMusicList(npc->id, true);
}

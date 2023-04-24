#include "GuildMusicBoxManager.h"
#include "SceneServer.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "SceneAct.h"
#include "SceneActManager.h"
#include "PlatLogManager.h"
#include "SceneUserManager.h"
#include "MsgManager.h"
#include "DScene.h"
#include "SceneManager.h"

// music manager
GuildMusicBoxManager::GuildMusicBoxManager()
{

}

GuildMusicBoxManager::~GuildMusicBoxManager()
{

}

void GuildMusicBoxManager::timer(DWORD dwCurTime)
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
        GuildMusicUpdateCmd cmd;
        cmd.set_guildid(m->first);
        v->toData(cmd.mutable_item());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToData(send, len);

        QWORD qwNpcID = v->qwNpcID;
        XLOG << "[公会点唱机管理-定时器-删除] guildid:" << m->first << "charid :" << v->qwCharID << "dtime :" << v->dwDemandTime << "musicid :" << v->dwMusicID << "starttime :" << v->dwStartTime << XEND;
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
          GuildMusicUpdateCmd cmd;
          cmd.set_guildid(m->first);
          v->toData(cmd.mutable_item());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToData(send, len);
          pNpc->setMusicData(v->dwMusicID, v->dwStartTime, v->qwCharID);
          broadcastMusicList(v->qwNpcID, true);
          bPlay = true;
          XLOG << "[公会点唱机管理-定时器-播放] guildid:" << m->first << " music :" << v->dwMusicID << "mapid :" << v->dwMapID << "charid :" << v->qwCharID << "status :" << v->dwState << "开始播放" << XEND;
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
          XLOG << "[公会点唱机管理-定时器-播放] guildid:" << m->first << "music :" << v->dwMusicID << "mapid :" << v->dwMapID << "charid :" << v->qwCharID << "status :" << v->dwState << "播放结束" << XEND;
        }
      }

      ++v;
    }
  }

}

bool GuildMusicBoxManager::queryMusicList(SceneUser* pUser, QWORD qwNpcID, bool bSync /*= false*/)
{
  if (pUser == nullptr)
    return false;
  if (pUser->hasGuild() == false)
    return false;
  pUser->setBrowse(true);

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->getScene()->base == nullptr || pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
    return false;

  DWORD dwMapID = pNpc->getScene()->getMapID();
  auto m = m_mapMusicItem.find(pUser->getGuild().id());
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
    XLOG << "[公会点唱机管理-列表请求] guildid:" << m->first << "charid :" << v->qwCharID << "dtime :" << v->dwDemandTime << "musicid :" << v->dwMusicID << "starttime :" << v->dwStartTime << "status :" << v->dwState << XEND;
    if (++dwMaxCount >= MAX_MUSIC_LIST_COUNT)
      break;
  }

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  if (!bSync)
  {
    XLOG << "[公会点唱机管理-列表请求]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << m->first
      << "在 mapid :" << dwMapID << "请求" << pNpc->id << pNpc->getNpcID() << pNpc->name << "音乐列表, 获得了" << cmd.items_size() << "个音乐列表" << XEND;
    return true;
  }

  XLOG << "[公会点唱机管理-列表同步]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << m->first
    << "在 mapid :" << dwMapID << "请求" << pNpc->id << pNpc->getNpcID() << pNpc->name << "音乐列表, 获得了" << cmd.items_size() << "个音乐列表" << XEND;
  return true;
}

bool GuildMusicBoxManager::demandMusic(SceneUser* pUser, QWORD qwNpcID, DWORD dwMusicID)
{
  if (pUser == nullptr)
  {
    XERR << "[公会点唱机管理-音乐点播] 未找到玩家" << XEND;
    return false;
  }
  if (pUser->hasGuild() == false)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "玩家未加入公会" << XEND;
    return false;
  }
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "点播" << dwMusicID << "失败, 未找到" << qwNpcID << "npc" << XEND;
    return false;
  }
  if (pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, npc" << qwNpcID << pNpc->getNpcID() << "不是点唱机npc" << XEND;
    return false;
  }

  DWORD dwMapID = pNpc->getScene()->getMapID();
  const SMapCFG* pMapBase = MapConfig::getMe().getMapCFG(dwMapID);
  if (pMapBase == nullptr)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, 地图" << dwMapID << "未在 Table_Map.txt 表中找到" << XEND;
    return false;
  }

  const SMusicBase* pMusicBase = TableManager::getMe().getMusicCFG(dwMusicID);
  if (pMusicBase == nullptr)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "点播" << dwMusicID << "失败, 未在 Table_MusicBox.txt 表中找到" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr || pMainPack->checkItemCount(dwMusicID) == false)
  {
    XERR << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
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

  TVecMusicItem& vecItem = m_mapMusicItem[pUser->getGuild().id()];
  vecItem.push_back(stItem);

  GuildMusicUpdateCmd cmd;
  cmd.set_guildid(pUser->getGuild().id());
  stItem.toData(cmd.mutable_item());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);

  broadcastMusicList(pNpc->id, true);
  XLOG << "[公会点唱机管理-音乐点播]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << pUser->getGuild().id()
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

void GuildMusicBoxManager::initMusicNpc(SceneNpc* pNpc)
{
  if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->getNpcID() != MiscConfig::getMe().getSystemCFG().dwMusicBoxNpc)
    return;

  GuildScene* pGuildScene = dynamic_cast<GuildScene*>(pNpc->getScene());
  if (pGuildScene == nullptr)
    return;

  const SceneBase* pBase = pNpc->getScene()->base;
  if (pBase == nullptr)
    return;

  MusicNpc* pMusicNpc = dynamic_cast<MusicNpc*>(pNpc);
  if (pMusicNpc == nullptr)
    return;

  auto m = m_mapMusicItem.find(pGuildScene->getGuildID());
  if (m == m_mapMusicItem.end())
    return;

  bool play = false;
  for (auto v = m->second.begin(); v != m->second.end(); ++v)
  {
    if (v->dwNpcID == pNpc->getNpcID())
    {
      v->qwNpcID = pNpc->id;
      if (play == false && v->dwState != 1 && v->dwStartTime != 0)
      {
        pMusicNpc->setMusicData(v->dwMusicID, v->dwStartTime, v->qwCharID);
        pMusicNpc->setGearStatus(MiscConfig::getMe().getSystemCFG().dwMusicStatusPlay);
        XLOG << "[公会点唱机管理-初始化]" << pNpc->id << pNpc->getNpcID() << pNpc->name << "继续播放 id :" << v->dwMusicID << "timer :" << v->dwStartTime << "charid :" << v->qwCharID << "guildid:" << m->first << XEND;
        play = true;
      }
    }
  }
}

void GuildMusicBoxManager::sendGuildMusicQueryRecord(DWORD sceneid, QWORD guildid)
{
  GuildMusicQueryRecordCmd cmd;
  cmd.set_scenename(thisServer->getServerName());
  cmd.set_sceneid(sceneid);
  cmd.set_guildid(guildid);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);
}

bool GuildMusicBoxManager::loadMusicItem(const GuildMusicQueryRecordCmd& cmd)
{
  GuildScene *pGuildScene = dynamic_cast<GuildScene*>(SceneManager::getMe().getSceneByID(cmd.sceneid()));
  if (pGuildScene == nullptr)
  {
    XERR << "[公会点唱机管理-加载点唱机] sceneid:" << cmd.sceneid() << "公会场景未找到" << XEND;
    return false;
  }

  m_mapMusicItem[cmd.guildid()].clear();

  DWORD starttime = 0, curtime = now();
  bool play = false;
  for (int i = 0; i < cmd.items_size(); ++i)
  {
    SMusicItem item;
    item.qwCharID = cmd.items(i).charid();
    item.dwNpcID = cmd.items(i).npcid();
    item.dwMapID = cmd.items(i).mapid();
    item.dwMusicID = cmd.items(i).musicid();
    item.dwDemandTime = cmd.items(i).demandtime();
    item.dwStartTime = cmd.items(i).starttime();
    item.dwState = cmd.items(i).status();
    item.strName = cmd.items(i).name();

    if (item.dwState != 0)
      continue;

    item.pBase = TableManager::getMe().getMusicCFG(item.dwMusicID);
    if (item.pBase == nullptr)
    {
      XERR << "[公会点唱机管理-加载点唱机] guildid:" << cmd.guildid() << "mapid :" << item.dwMapID << "userid :" << item.qwCharID << "musicid :" << item.dwMusicID << "未在Table_MusicBox.txt中找到" << XEND;
      continue;
    }

    // 场景关闭期间歌曲保持播放
    if (play == false)
    {
      bool update = false;
      if (starttime != 0)
      {
        item.dwStartTime = starttime;
        update = true;
        XLOG << "[公会点唱机管理-加载点唱机] guildid:" << cmd.guildid() << " music :" << item.dwMusicID << "mapid :" << item.dwMapID << "charid :" << item.qwCharID << "status :" << item.dwState << "开始播放" << XEND;
      }
      if (item.dwStartTime != 0)
      {
        if (item.dwStartTime + item.pBase->getTableInt("MusicTim") < curtime)
        {
          item.dwState = 1;
          item.dwEndTime = item.dwStartTime + item.pBase->getTableInt("MusicTim");
          starttime = item.dwEndTime;
          XLOG << "[公会点唱机管理-加载点唱机] guildid:" << cmd.guildid() << "music :" << item.dwMusicID << "mapid :" << item.dwMapID << "charid :" << item.qwCharID << "status :" << item.dwState << "播放结束" << XEND;
          update = false;
        }
        else
        {
          starttime = 0;
          play = true;
        }
      }
      if (update)
      {
        GuildMusicUpdateCmd ucmd;
        ucmd.set_guildid(cmd.guildid());
        item.toData(ucmd.mutable_item());
        PROTOBUF(ucmd, send, len);
        thisServer->sendCmdToData(send, len);
      }
    }

    XLOG << "[公会点唱机管理-加载点唱机] charid :" << item.qwCharID << "dtime :" << item.dwDemandTime << "musicid :" << item.dwMusicID << "starttime :" << item.dwStartTime << "guildid:" << cmd.guildid() << XEND;
    m_mapMusicItem[cmd.guildid()].push_back(item);
  }

  // 清理已播放数据
  auto m = m_mapMusicItem.find(cmd.guildid());
  if (m != m_mapMusicItem.end())
  {
    for (auto v = m->second.begin(); v != m->second.end();)
    {
      if (v->dwState != 0 || v->pBase == nullptr)
      {
        GuildMusicUpdateCmd cmd;
        cmd.set_guildid(m->first);
        v->toData(cmd.mutable_item());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToData(send, len);

        XLOG << "[公会点唱机管理-删除] guildid:" << m->first << "charid :" << v->qwCharID << "dtime :" << v->dwDemandTime << "musicid :" << v->dwMusicID << "starttime :" << v->dwStartTime << XEND;
        v = m->second.erase(v);
        continue;
      }
      ++v;
    }
  }

  xSceneEntrySet set;
  pGuildScene->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto& npc : set)
    initMusicNpc((SceneNpc*)(npc));

  XLOG << "[公会点唱机管理-加载点唱机] 成功加载" << XEND;
  return true;
}

void GuildMusicBoxManager::broadcastMusicList(QWORD qwNpcID, bool bPlay)
{
  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(qwNpcID);
  if (pNpc == nullptr || pNpc->getScene()== nullptr)
    return;

  GuildScene* pGuildScene = dynamic_cast<GuildScene*>(pNpc->getScene());
  if (pGuildScene == nullptr)
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
      auto m = m_mapMusicItem.find(pGuildScene->getGuildID());
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

void GuildMusicBoxManager::onSceneClose(QWORD guildid)
{
  auto it = m_mapMusicItem.find(guildid);
  if (it == m_mapMusicItem.end())
    return;
  m_mapMusicItem.erase(it);
  XLOG << "[公会点唱机管理-公会场景关闭] guildid:" << guildid << "点唱机数据清除成功" << XEND;
}

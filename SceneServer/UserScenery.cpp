#include "UserScenery.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"
#include "SceneUserManager.h"
#include "Manual.h"

UserScenery::UserScenery(SceneUser *user):m_pUser(user)
{
}

UserScenery::~UserScenery()
{
}

void UserScenery::save(Cmd::BlobScenery *acc_data)
{
  using namespace Cmd;
  acc_data->Clear();
  for (auto it : m_list)
  {
    Cmd::SceneryItem *item = acc_data->add_items();
    item->set_sceneryid(it.first);
    item->set_upload(it.second.m_dwUpload);
    item->set_anglez(it.second.m_dwAngleZ);
    item->set_visited(it.second.m_dwVisited);
    item->set_time(it.second.m_dwTime);
  }
  acc_data->set_version(1);
  XDBG << "[景点-保存] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name << " 数据大小 : " << acc_data->ByteSize() << XEND;
}

void UserScenery::load(const Cmd::BlobScenery &acc_data)
{
  using namespace Cmd;
  int version = acc_data.version();

  if (!version)
  {
    DWORD size = acc_data.list_size();
    if (size)
    {
      for (DWORD i=0; i<size; ++i)
      {
        const Cmd::SceneryMapItem &map = acc_data.list(i);
        DWORD scenerysize = map.scenerys_size();
        for (DWORD j=0; j<scenerysize; ++j)
        {
          const Cmd::SceneryItem &item = map.scenerys(j);
          SceneryBase *base = nullptr;

          const Table<SceneryBase>* pSceneryList = TableManager::getMe().getSceneryCFGList();
          if (pSceneryList != nullptr)
          {
            for (auto g_it : pSceneryList->xEntryID::ets_)
            {
              SceneryBase *b = (SceneryBase *)g_it.second;
              if (b->getMapID() == map.mapid() && b->getMapIndex() == item.sceneryid())
              {
                base = b;
                break;
              }
              if (base)
              {
                m_list[b->id].m_dwVisited = item.visited();
              }
            }
          }
        }
      }
    }
  }

  DWORD size = acc_data.items_size();
  for (DWORD i=0; i<size; ++i)
  {
    const Cmd::SceneryItem &item = acc_data.items(i);
    setData(item.sceneryid(), item.visited(), item.upload(), item.anglez(), item.time());
  }

  if (version >= 2)
  {
  }
}

void UserScenery::setData(DWORD sceneryid, DWORD visit, DWORD upload, DWORD anglez, DWORD time)
{
  SceneryData &data = m_list[sceneryid];
  data.m_dwSceneryID = sceneryid;
  data.m_dwVisited = visit;
  data.m_dwUpload = upload;
  data.m_dwAngleZ = anglez;
  data.m_dwTime = time;

  const SceneryBase *base = TableManager::getMe().getSceneryCFG(sceneryid);
  if (base)
  {
    data.m_dwMapID = base->getMapID();
  }
}

const SceneryData* UserScenery::getScenery(DWORD dwSceneryID) const
{
  auto m = m_list.find(dwSceneryID);
  if (m != m_list.end())
    return &m->second;
  return nullptr;
}

void UserScenery::collectPhoto(QueryShowPhotoGuildSCmd& cmd)
{
  SManualItem* pItem = m_pUser->getManual().getManualItem(EMANUALTYPE_SCENERY);
  if (pItem == nullptr)
  {
    XERR << "[玩家-景点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集景点照片失败,未找到冒险手册选项卡" << XEND;
    return;
  }

  struct SPhotoResult
  {
    DWORD dwFrameID = 0;
    DWORD dwNeedCount = 0;

    list<GuildPhoto> listPhotos;
  };

  map<DWORD, SPhotoResult> mapResult;
  TSetString setExists;
  for (int i = 0; i < cmd.exists_size(); ++i)
    setExists.insert(cmd.exists(i));
  for (int i = 0; i < cmd.loads_size(); ++i)
  {
    const PhotoLoad& load = cmd.loads(i);
    SPhotoResult& rResult = mapResult[load.frameid()];
    rResult.dwFrameID = load.frameid();
    rResult.dwNeedCount = load.count();
  }

  for (auto &m : m_list)
  {
    if (m.second.m_dwUpload != 1)
      continue;
    SManualSubItem* pSubItem = pItem->getSubItem(m.first);
    if (pSubItem == nullptr || pSubItem->eStatus <= EMANUALSTATUS_UNLOCK_CLIENT)
      continue;
    const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(m.second.m_dwSceneryID);
    if (pBase == nullptr)
      continue;

    GuildPhoto oPhoto;
    oPhoto.set_charid(m_pUser->id);
    oPhoto.set_anglez(m.second.m_dwAngleZ);
    oPhoto.set_time(m.second.m_dwTime);
    oPhoto.set_mapid(pBase->getMapID());
    oPhoto.set_source(ESOURCE_PHOTO_SCENERY);
    oPhoto.set_sourceid(m.second.m_dwSceneryID);

    if (setExists.find(GGuild::getPhotoGUID(oPhoto)) != setExists.end())
      continue;

    EGuildFrameType eType = GuildConfig::getMe().getAngleFrameType(oPhoto.anglez());
    for (auto &result : mapResult)
    {
      if (GuildConfig::getMe().getPhotoFrameType(result.first) != eType)
        continue;
      if (result.second.listPhotos.size() >= result.second.dwNeedCount)
        continue;
      result.second.listPhotos.push_back(oPhoto);
      break;
    }
  }

  for (auto &r : mapResult)
  {
    PhotoFrame* pFrame = cmd.add_results();
    pFrame->set_frameid(r.first);
    for (auto &l : r.second.listPhotos)
      pFrame->add_photo()->CopyFrom(l);
  }

  cmd.set_action(EPHOTOACTION_UPDATE_FROM_SCENE);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  XLOG << "[玩家-景点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收集景点照片陈宫,同步自动上传";
  for (int i = 0; i < cmd.results_size(); ++i)
  {
    const PhotoFrame& rFrame = cmd.results(i);
    XLOG << "frameid :" << rFrame.frameid() << rFrame.photo_size() << "张";
  }
  XLOG << XEND;
}

void UserScenery::send(DWORD mapid)
{
  Cmd::SceneryUserCmd message;
  message.set_mapid(mapid);
  for (auto it : m_list)
  {
    if (it.second.m_dwMapID == mapid)
    {
      Scenery* pScenery = message.add_scenerys();
      if (pScenery != nullptr)
      {
        pScenery->set_sceneryid(it.first);
        pScenery->set_anglez(it.second.m_dwAngleZ);
      }
    }
  }
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[玩家-景点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步了景点数据" << message.ShortDebugString() << XEND;
}

void UserScenery::sendunsolved()
{
  UnsolvedSceneryNtfUserCmd cmd;
  for (auto &m : m_list)
  {
    if (m.second.m_dwUpload == 0 && m.second.m_dwVisited != 0)
    {
      const SceneryBase* base = TableManager::getMe().getSceneryCFG(m.first);
      if (base && base->getType() == ESCENERYTYPE_USER)
        continue;
      cmd.add_ids(m.first);
    }
  }
  if (cmd.ids_size() <= 0)
    return;
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[玩家-景点]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步了未处理景点数据" << cmd.ShortDebugString() << XEND;
}

void UserScenery::add(DWORD sceneryid)
{
  const SceneryBase *base = TableManager::getMe().getSceneryCFG(sceneryid);
  if (base != nullptr)
    m_pUser->getManual().onScenery(sceneryid, false);

  if (m_list.find(sceneryid)!=m_list.end()) return;

  if (base)
  {
    setData(sceneryid, 0, 0, 0, 0);

    DWORD mapid = base->getMapID();

    XLOG << "[景点拍照] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << "," << m_pUser->name <<",添加,mapid:" << mapid << ",sceneryid:" << sceneryid << XEND;

    if (m_pUser->getScene() == nullptr || m_pUser->getScene()->getMapID() != mapid)
      return;

    if (base->getType() != ESCENERYTYPE_USER)
      send(mapid);
  }
}

void UserScenery::photo(const Cmd::Scenery& rScenery)
{
  if (!m_pUser || !m_pUser->getScene())
    return;

  const SceneryBase *base = TableManager::getMe().getSceneryCFG(rScenery.sceneryid());
  if (base == nullptr)
  {
    XERR << "[景点拍照]"<< m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << ",完成拍照,mapid:" << m_pUser->getScene()->getMapID() << ",scenery:" << rScenery.ShortDebugString() << ",找不到配置" << XEND;
    return;
  }

  DWORD mapid = m_pUser->getScene()->getMapID();
  if (base->getType() == ESCENERYTYPE_USER)
  {
    auto iter = m_list.find(rScenery.sceneryid());
    if (iter == m_list.end())
    {
      add(rScenery.sceneryid());
      iter = m_list.find(rScenery.sceneryid());
      if (iter == m_list.end()) return;
    }

    SceneUser* user = SceneUserManager::getMe().getUserByID(rScenery.charid());
    if (!user || !user->getScene() || user->getScene() != m_pUser->getScene())
    {
      XERR << "[景点拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "scenery:" << rScenery.ShortDebugString() << "不在同一个场景" << XEND;
      return;
    }
    float dist = ::getDistance(m_pUser->getPos(), user->getPos());
    if (dist > 50.0f)
    {
      XERR << "[景点拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "scenery:" << rScenery.ShortDebugString()
           << "dist:" << dist << " 距离过远" << XEND;
      return;
    }
    if (user->m_oBuff.hasSceneryID(rScenery.sceneryid()) == false)
    {
      XERR << "[景点拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "scenery:" << rScenery.ShortDebugString() << "目标玩家不是目标景点" << XEND;
      return;
    }
    m_pUser->getManual().onScenery(rScenery.sceneryid());
    m_pUser->getAchieve().onPhoto(EACHIEVECOND_PHOTO_USER, TSetQWORD{rScenery.sceneryid()});
    iter->second.m_dwVisited = 1;
  }
  else
  {
    // 是否能拍照
    auto iter = m_list.find(rScenery.sceneryid());
    if (iter == m_list.end()) return;

    // 拍过照了不能拍
    // if (iter->second.m_dwVisited) return;

    if (iter->second.m_dwMapID != mapid) return;

    const xPos& oUserPos = m_pUser->getPos();
    xPos oSceneryPos;
    if (base->getPos(oSceneryPos) == true)
    {
      float dist = ::getDistance(oUserPos, oSceneryPos);
      if (dist > 50.0f)
      {
        XERR << "[景点拍照] " << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << ","<< m_pUser->name << ","
             << " mapid:" << mapid << ",scenery:" << rScenery.ShortDebugString() << ",dist:" << dist << " 距离过远" << XEND;
        return;
      }
    }

    m_pUser->getManual().onScenery(rScenery.sceneryid());

    /*
      DWORD dwRewardID = base->getRewardID();
      if (dwRewardID)
      {
      RewardManager::getMe().rollToPack(dwRewardID, m_pUser, true, true);
      }
    */

    iter->second.m_dwVisited = 1;

    // 使用upload决定角度和时间
    //iter->second.m_dwAngleZ = rScenery.anglez();
    //iter->second.m_dwTime = xTime::getCurSec();
  }

  XLOG << "[景点拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << ",完成拍照,mapid:" << mapid << ",scenery:" << rScenery.ShortDebugString() << XEND;
}

void UserScenery::upload(const UploadOkSceneryUserCmd& cmd)
{
  // 是否能上传
  auto iter = m_list.find(cmd.sceneryid());
  if (iter == m_list.end()) return;

  if (!iter->second.m_dwVisited) return;

  iter->second.m_dwUpload = cmd.status();
  iter->second.m_dwAngleZ = cmd.anglez();
  iter->second.m_dwTime = cmd.time();

  m_pUser->getManual().onSceneryUpload(cmd.sceneryid(), iter->second.m_dwAngleZ, iter->second.m_dwTime);

  if (m_pUser->hasGuild() == true)
  {
    PhotoUpdateGuildSCmd cmd;

    cmd.set_guildid(m_pUser->getGuild().id());
    cmd.set_to_guild(true);

    GuildPhoto* pPhoto = cmd.mutable_update();
    pPhoto->set_charid(m_pUser->id);
    pPhoto->set_anglez(iter->second.m_dwAngleZ);
    pPhoto->set_time(iter->second.m_dwTime);
    pPhoto->set_mapid(iter->second.m_dwMapID);
    pPhoto->set_source(ESOURCE_PHOTO_SCENERY);
    pPhoto->set_sourceid(iter->first);

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  XLOG << "[景点拍照]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "完成上传" << cmd.ShortDebugString() << XEND;
}


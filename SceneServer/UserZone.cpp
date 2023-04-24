#include "UserZone.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"

UserZone::UserZone(SceneUser *user):m_pUser(user)
{
}

UserZone::~UserZone()
{
}

void UserZone::save(Cmd::BlobZone *data)
{
  using namespace Cmd;
  data->Clear();
  data->set_version(2);
//  data->set_zoneid(m_dwZoneID);
  data->set_mapid(m_dwMapID);
//  data->set_pastzoneid(m_dwPastZoneID);
  data->set_type(static_cast<DWORD>(m_oType));

  data->set_x(m_oDestPos.x);
  data->set_y(m_oDestPos.y);
  data->set_z(m_oDestPos.z);

  for (auto &it : m_mapRaidZone)
  {
    RaidZone *add = data->add_raidzonelist();
    add->set_raidid(it.first);
    add->set_zoneid(it.second);
  }

  data->set_effectivetime(m_dwEffectiveTime); 
  data->set_roomid(m_qwRoomId);
  data->set_colorindex(m_dwColorIndex);

  XDBG << "[切线-保存] " << m_pUser->accid << m_pUser->id << m_pUser->name << "切线:" << m_pUser->getUserSceneData().getDestZoneID() <<   m_pUser->getUserSceneData().getOriginalZoneID() << "地图:" << m_dwMapID <<"pvp房间:"<< m_qwRoomId << "colorindex" << m_dwColorIndex << XEND;
}

void UserZone::load(const Cmd::BlobZone &data)
{
  using namespace Cmd;
  int version = data.version();

  DWORD dwZoneID = data.zoneid();
  m_dwMapID = data.mapid();
  DWORD dwPastZoneID = data.pastzoneid();
  m_oType = static_cast<GoMapType>(data.type());

  m_oDestPos.x = data.x();
  m_oDestPos.y = data.y();
  m_oDestPos.z = data.z();

  if (m_oDestPos.empty() == false)
    m_pUser->getUserSceneData().setOnlinePos(m_oDestPos);

  m_qwRoomId = data.roomid();

  //if (thisServer->getZoneCategory())
    m_dwColorIndex = data.colorindex();

  for (int i = 0; i < data.raidzonelist_size(); ++i)
  {
    const RaidZone &item = data.raidzonelist(i);
    m_mapRaidZone[item.raidid()] = item.zoneid();
  }

  if (data.has_effectivetime())
    m_dwEffectiveTime = data.effectivetime();

  if (version < 2)
  {
    m_pUser->getUserSceneData().setDestZoneID(dwZoneID);
    m_pUser->getUserSceneData().setOriginalZoneID(dwPastZoneID);
  }

  XDBG << "[切线-加载] " << m_pUser->accid << m_pUser->id << m_pUser->name << "切线:" << m_pUser->getUserSceneData().getDestZoneID() << m_pUser->getUserSceneData().getOriginalZoneID() << "地图:" << m_dwMapID << "pvp房间:" << m_qwRoomId<<"colorindex"<<m_dwColorIndex << XEND;
}

void UserZone::gomap(DWORD zoneid, DWORD mapid, GoMapType type, const xPos& rPos /*= xPos(0, 0, 0)*/)
{
  if (zoneid == thisServer->getZoneID())
  {
    m_pUser->gomap(mapid, type, rPos);
  }
  else
  {
    m_pUser->getUserSceneData().setDestZoneID(zoneid);
    m_dwMapID = mapid;
    m_oType = type;
    m_oDestPos = rPos;
    const SRaidCFG* pCFG = MapConfig::getMe().getRaidCFG(mapid);
    if (pCFG != nullptr && pCFG->eRaidType != ERAIDTYPE_MIN)
    {
      if (0 == m_pUser->getUserSceneData().getOriginalZoneID())
      {
        m_pUser->getUserSceneData().setOriginalZoneID(thisServer->getZoneID());
      }
    }
    m_dwEffectiveTime = now() + 60;
    m_pUser->getUserSceneData().setZoneID(m_pUser->getUserSceneData().getDestZoneID());
    XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "切换到:" << m_pUser->getUserSceneData().getDestZoneID() << m_pUser->getUserSceneData().getOriginalZoneID() << m_dwMapID << XEND;
  }
}

bool UserZone::online(Scene *scene)
{
  if (!scene) return false;
  if (m_pUser->getUserSceneData().getDestZoneID() && m_dwEffectiveTime < now())
  {
    m_pUser->getUserSceneData().setDestZoneID(0);
  }
  if (m_pUser->getUserSceneData().getDestZoneID())
  {
    if (m_pUser->getUserSceneData().getDestZoneID() == thisServer->getZoneID())
    {
      if (m_dwMapID)
      {
        // 已进入目标地图 清除数据
        if (scene->getMapID() == m_dwMapID)
        {
          XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "进入目标地图:" << m_pUser->getUserSceneData().getDestZoneID() << m_pUser->getUserSceneData().getOriginalZoneID() << m_dwMapID << XEND;
          m_pUser->getUserSceneData().setDestZoneID(0);
          m_oType = GoMapType::Null;
          m_oDestPos.clear();
        }
        else
        {
          XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "准备进入地图:" << m_pUser->getUserSceneData().getDestZoneID() << m_pUser->getUserSceneData().getOriginalZoneID() << m_dwMapID << XEND;
          m_pUser->gomap(m_dwMapID, m_oType, m_oDestPos);
          m_oDestPos.clear();
          return true;
        }
      }
    }
    else
    {
      m_pUser->getUserSceneData().setZoneID(m_pUser->getUserSceneData().getDestZoneID());
      XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "切换到:" << m_pUser->getUserSceneData().getDestZoneID() << m_pUser->getUserSceneData().getOriginalZoneID() << m_dwMapID << XEND;
      return true;
    }
  }
  else if (m_pUser->getUserSceneData().getOriginalZoneID())
  {
    //pvp副本掉线后传送到原来的线
    if (scene->isDScene() && !scene->isPVPScene()) return false;
    if (m_pUser->getUserSceneData().getOriginalZoneID() == thisServer->getZoneID())
    {
      m_pUser->getUserSceneData().setOriginalZoneID(0);
      m_dwMapID = 0;
      XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "已返回原线:" << thisServer->getZoneID() << XEND;
    }
    else if (m_dwMapID && m_dwMapID != scene->getMapID())
    {
      m_pUser->getUserSceneData().setZoneID(m_pUser->getUserSceneData().getOriginalZoneID());
      XLOG << "[切线]" << m_pUser->accid << m_pUser->id << m_pUser->name << "返回原线:" << m_pUser->getUserSceneData().getOriginalZoneID() << XEND;
      return true;
    }
    if (scene->isDPvpScene() || scene->isSuperGvg())
    {
      if (scene->getSceneType() != SCENE_TYPE_TEAMPWS) // 组队排位赛掉线后继续进入副本
      {
        m_pUser->getUserSceneData().setZoneID(m_pUser->getUserSceneData().getOriginalZoneID());
        XLOG << "[切线] pvp地图掉线后返回原来线" << m_pUser->accid << m_pUser->id << m_pUser->name << "返回原线:" << m_pUser->getUserSceneData().getOriginalZoneID() << XEND;
        return true;
      }
    }
  }
  return false;
}

DWORD UserZone::getRaidZoneID(DWORD raidid) const
{
  auto it = m_mapRaidZone.find(raidid);
  if (it == m_mapRaidZone.end()) return 0;

  return it->second;
}

void UserZone::setRaidZoneID(DWORD raid, DWORD zoneid)
{
  if (!raid || !zoneid) return;

  m_mapRaidZone[raid] = zoneid;
}

void UserZone::delRaidZoneID(DWORD raid)
{
  m_mapRaidZone.erase(raid);
}


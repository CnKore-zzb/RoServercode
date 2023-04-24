#include "UserGear.h"
#include "RecordCmd.pb.h"
#include "SceneUser.h"

UserGear::UserGear(SceneUser *user):
  m_pUser(user)
{
}

UserGear::~UserGear()
{
}

void UserGear::set(DWORD mapid, DWORD id, DWORD state)
{
  m_list[mapid][id] = state;
}

DWORD UserGear::get(DWORD mapid, DWORD id)
{
  auto it = m_list.find(mapid);
  if (it==m_list.end()) return 0;

  auto iter = it->second.find(id);
  if (iter==it->second.end()) return 0;

  return iter->second;
}

void UserGear::save(Cmd::BlobGears *data)
{
  data->Clear();
  if (!m_list.empty())
  {
    for (auto it : m_list)
    {
      for (auto iter : it.second)
      {
        Cmd::BlobGearItem *p = data->add_list();
        p->set_gearid(iter.first);
        p->set_state(iter.second);
        p->set_sceneid(it.first);
      }
    }
  }
  if (!m_oExitStateList.empty())
  {
    for (auto it : m_oExitStateList)
    {
      for (auto iter : it.second)
      {
        Cmd::BlobGearItem *p = data->add_exitlist();
        p->set_gearid(iter.first);
        p->set_state(iter.second);
        p->set_sceneid(it.first);
      }
    }
  }
  XDBG << "[机关-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
}

void UserGear::load(const Cmd::BlobGears &data)
{
  int size = data.list_size();
  if (size)
  {
    for (int i=0; i<size; ++i)
    {
      const Cmd::BlobGearItem &item = data.list(i);

      m_list[item.sceneid()][item.gearid()] = item.state();
    }
  }
  size = data.exitlist_size();
  if (size)
  {
    for (int i=0; i<size; ++i)
    {
      const Cmd::BlobGearItem &item = data.exitlist(i);

      m_oExitStateList[item.sceneid()][item.gearid()] = item.state();
    }
  }
}

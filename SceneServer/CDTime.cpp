#include "CDTime.h"
#include "xTime.h"
#include "SceneUser.h"
#include "UserCmd.h"
#include "SkillManager.h"
//const char LOG_NAME[] = "CDTime";

CDTimeM::CDTimeM(xSceneEntryDynamic *e):m_pEntry(e)
{
  for (BYTE i=0; i<CD_TYPE_ARRAYSIZE; i++)
    m_list[i].clear();
}

CDTimeM::~CDTimeM()
{
}

void CDTimeM::fixID(CD_TYPE t, DWORD &id)
{
  // 技能用技能id记录  不加等级
  if (CD_TYPE_SKILL==t)
  {
    const BaseSkill* pSkill = SkillManager::getMe().getSkillCFG(id);
    if (pSkill != nullptr)
      id = pSkill->getFamilyID();
  }
}

bool CDTimeM::done(CD_TYPE t, DWORD id)
{
  if (!CD_TYPE_IsValid(t)) return false;

  fixID(t, id);

  QWORD curTime = xTime::getCurUSec()/1000;
  m_iter it = m_list[t].find(id);
  if (it!=m_list[t].end())
  {
    if (it->second < curTime + 100)
    {
      m_list[t].erase(it);
      return true;
    }
    else
    {
      return false;
    }
  }
  return true;
}

bool CDTimeM::skilldone(DWORD id)
{
  return done(CD_TYPE_SKILL, id) && done(CD_TYPE_SKILLDEALY, id);
}

QWORD CDTimeM::getSkillCDEndTime(DWORD id)
{
  QWORD time = 0;
  m_iter it = m_list[CD_TYPE_SKILLDEALY].find(id);
  if (it != m_list[CD_TYPE_SKILLDEALY].end())
    time = it->second;

  fixID(CD_TYPE_SKILL, id);
  it = m_list[CD_TYPE_SKILL].find(id);
  if (it != m_list[CD_TYPE_SKILL].end())
    time = std::max(time, (QWORD)it->second);
  return time;
}

QWORD CDTimeM::getCDTime(CD_TYPE type, DWORD id)
{
  if (CD_TYPE_IsValid(type) == false)
  {
    XERR << "[CD-获取]" << m_pEntry->id << m_pEntry->name << "获取 type :" << type << "失败,type不合法" << XEND;
    return 0;
  }

  std::map<DWORD, QWORD>& mapCD = m_list[type];
  auto m = mapCD.find(id);
  if (m != mapCD.end())
    return m->second;
  return 0;
}

void CDTimeM::add(DWORD id, DWORD elapse, CD_TYPE t)
{
  if (id==0) return;
  if (!CD_TYPE_IsValid(t)) return;
  DWORD trueID = id;
  fixID(t, id);
  m_list[t][id] = xTime::getCurUSec()/1000 + elapse;
  if (m_pEntry && m_pEntry->getEntryType()==SCENE_ENTRY_USER)
    send(t, trueID, m_list[t][id]);
}

void CDTimeM::clear(DWORD id, CD_TYPE t)
{
  if (!CD_TYPE_IsValid(t)) return;
  fixID(t, id);
  m_list[t].erase(id);
}

void CDTimeM::clear()
{
  for (BYTE i = 0; i < CD_TYPE_ARRAYSIZE; ++i)
    m_list[i].clear();
}

void CDTimeM::load(const Cmd::BlobCDTime &data)
{
  int size = data.list_size();
  if (!size) return;

  QWORD curTime = xTime::getCurUSec()/1000;
  for (int i=0; i<size; ++i)
  {
    const Cmd::CDTimeItem &item = data.list(i);
    DWORD id = item.id();
    QWORD time = item.time();
    CD_TYPE type = item.type();
    if (id && time>curTime)
    {
      if (CD_TYPE_IsValid(type))
      {
        m_list[type][id] = time;
      }
    }
  }
}

void CDTimeM::save(Cmd::BlobCDTime *data)
{
  if (data == nullptr)
    return;

  if (m_list[CD_TYPE_SKILL].empty() && m_list[CD_TYPE_ITEM].empty()) return;

  QWORD curTime = xTime::getCurUSec()/1000;

  data->clear_list();
  for (int i=0; i<CD_TYPE_ARRAYSIZE; ++i)
  {
    m_iter it = m_list[i].begin(), end = m_list[i].end();
    for ( ;it!=end; ++it)
    {
      if (it->second > curTime)
      {
        Cmd::CDTimeItem *pItem = data->add_list();

        pItem->set_id(it->first);
        pItem->set_time(it->second);
        pItem->set_type((CD_TYPE)i);
      }
    }
  }

  SceneUser* pUser = dynamic_cast<SceneUser*>(m_pEntry);
  if (pUser != nullptr)
    XDBG << "[CD-保存]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "数据大小 :" << data->ByteSize() << XEND;
}

void CDTimeM::send(CD_TYPE t, DWORD id, QWORD time)
{
  if (!m_pEntry || SCENE_ENTRY_USER!=m_pEntry->getEntryType()) return;
  if (!CD_TYPE_IsValid(t)) return;
  if (t == CD_TYPE_SKILLDEALY) return;

  //fixID(t, id);

  Cmd::CDTimeUserCmd message;
  Cmd::CDTimeItem *pItem = message.add_list();

  pItem->set_id(id);
  pItem->set_time(time);
  pItem->set_type(t);

  PROTOBUF(message, send, len);
  ((SceneUser *)m_pEntry)->sendCmdToMe(send, len);
}

void CDTimeM::send()
{
  if (!m_pEntry || SCENE_ENTRY_USER!=m_pEntry->getEntryType()) return;

  QWORD curTime = xTime::getCurUSec()/1000;

  Cmd::CDTimeUserCmd message;

  for (int i=0; i<CD_TYPE_ARRAYSIZE; ++i)
  {
    m_iter it = m_list[i].begin(), end = m_list[i].end();
    m_iter temp;
    for ( ;it!=end; )
    {
      temp = it++;
      if (temp->second > curTime)
      {
        Cmd::CDTimeItem *pItem = message.add_list();

        pItem->set_id(temp->first);
        pItem->set_time(temp->second);
        pItem->set_type((CD_TYPE)i);
      }
      else
      {
        m_list[i].erase(temp);
      }
    }
  }

  if (message.list_size())
  {
    PROTOBUF(message, send, len);
    ((SceneUser *)m_pEntry)->sendCmdToMe(send, len);
  }
}

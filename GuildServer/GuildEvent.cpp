#include "GuildEvent.h"
#include "Guild.h"
#include "GuidManager.h"
#include "config/MiscConfig.h"

GuildEventM::GuildEventM(Guild *pGuild) : m_pGuild(pGuild)
{
}

GuildEventM::~GuildEventM()
{
}

void GuildEventM::init()
{
  switch (m_dwInitStatus)
  {
    case GUILD_BLOB_INIT_NULL:
      {
        XDBG << "[工会-事件]" << m_pGuild->getName() << "初始化失败，没有数据" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_COPY_DATA:
      {
        fromEventString(m_oBlobEvent);
        m_dwInitStatus = GUILD_BLOB_INIT_OK;
        XDBG << "[工会-事件]" << m_pGuild->getName() << "初始化成功" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_OK:
      break;
    default:
      break;
  }
}

void GuildEventM::setBlobEventString(const char* str, DWORD len)
{
  if (GUILD_BLOB_INIT_OK == m_dwInitStatus)
  {
    XERR << "[工会-事件]" << "初始化异常" << XEND;
    return;
  }
  m_dwInitStatus = GUILD_BLOB_INIT_COPY_DATA;
  m_oBlobEvent.assign(str, len);
  XDBG << "[工会-事件]" << m_pGuild->getName() << "设置blob string" << XEND;
}

bool GuildEventM::toBlobEventString(string& str)
{
  if (GUILD_BLOB_INIT_COPY_DATA == m_dwInitStatus)
  {
    str.assign(m_oBlobEvent.c_str(), m_oBlobEvent.size());
    return true;
  }

  if (GUILD_BLOB_INIT_OK != m_dwInitStatus) return true;

  BlobGuildEvent oEvent;

  for (auto &v : m_listGuildEvent)
    oEvent.add_events()->CopyFrom(v);

  if (oEvent.SerializeToString(&str) == false)
  {
    XERR << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存序列化失败" << XEND;
    return false;
  }

  return true;
}

bool GuildEventM::fromEventString(const string& str)
{
  BlobGuildEvent oEvent;
  if (oEvent.ParseFromString(str) == false)
  {
    XERR << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载反序列化失败" << XEND;
    return false;
  }

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();

  m_listGuildEvent.clear();
  DWORD curSec = now();
  for (int i = 0; i < oEvent.events_size(); ++i)
  {
    const GuildEvent& rEvent = oEvent.events(i);
    if (rCFG.dwEventOverTime && (rEvent.time() + rCFG.dwEventOverTime < curSec))
    {
      XLOG << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "刷新事件" << rEvent.ShortDebugString() << "超时,被删除" << XEND;
      continue;
    }

    //oEvent.mutable_events(i)->set_guid(GuidManager::getMe().getNextEventID());
    GuidManager::getMe().addUsedEventID(rEvent.guid());
    m_listGuildEvent.push_back(rEvent);
    XDBG << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载事件" << rEvent.ShortDebugString() << XEND;
  }

  XLOG << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载了" << m_listGuildEvent.size() << "个事件" << XEND;
  return true;
}

const GuildEvent* GuildEventM::addEvent(EGuildEvent eType, const TVecString& vecParams)
{
  if (eType <= EGUILDEVENT_MIN || eType >= EGUILDEVENT_MAX)
  {
    XERR << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << eType << "事件失败,不合法的类型" << XEND;
    return nullptr;
  }

  GuildEvent oEvent;
  oEvent.set_guid(GuidManager::getMe().getNextEventID());
  oEvent.set_eventid(eType);
  oEvent.set_time(xTime::getCurSec());
  for (auto &v : vecParams)
    oEvent.add_param(v.c_str());

  m_listGuildEvent.push_back(oEvent);
  NewEventGuildCmd cmd;
  cmd.mutable_event()->CopyFrom(oEvent);
  cmd.set_del(false);
  PROTOBUF(cmd, send, len);
  m_pGuild->broadcastCmd(send, len);
  XLOG << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << oEvent.ShortDebugString() << "事件成功" << XEND;

  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if (m_listGuildEvent.size() > rCFG.dwMaxEventCount)
  {
    const GuildEvent& rEvent = *m_listGuildEvent.begin();
    XLOG << "[公会-事件]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << oEvent.ShortDebugString() << "事件成功, 列表超过" << rCFG.dwMaxEventCount << "个上限,移除了"
      << rEvent.ShortDebugString() << "事件" << XEND;
    GuidManager::getMe().removeUsedEventID(rEvent.guid());
    m_listGuildEvent.erase(m_listGuildEvent.begin());
  }

  m_pGuild->setMark(EGUILDDATA_EVENT);
  return getEvent(oEvent.guid());
}

const GuildEvent* GuildEventM::getEvent(DWORD dwID)
{
  auto l = find_if(m_listGuildEvent.begin(), m_listGuildEvent.end(), [&](const GuildEvent& r) -> bool{
    return r.guid() == dwID;
  });
  return l != m_listGuildEvent.end() ? &(*l) : nullptr;
}

void GuildEventM::collectEventList(QueryEventListGuildCmd& cmd)
{
  cmd.clear_events();
  for (auto &v : m_listGuildEvent)
  {
    GuildEvent* pEvent = cmd.add_events();
    if (pEvent != nullptr)
      pEvent->CopyFrom(v);
  }
}


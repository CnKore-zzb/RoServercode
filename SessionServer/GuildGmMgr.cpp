#include "GuildGmMgr.h"
#include "SessionServer.h"
#include "GuidManager.h"
#include "xCommand.h"
#include "xDBFields.h"

GuildGmMgr::GuildGmMgr()
{
  m_dwFlushTick = xTime::getCurSec();
}

GuildGmMgr::~GuildGmMgr()
{

}

bool GuildGmMgr::add(const GMCommandGuildSCmd& cmd)
{
  DWORD dwSessionID = GuidManager::getMe().getNextGuildGmSessionID();
  auto m = m_mapSessionGM.find(dwSessionID);
  if (m != m_mapSessionGM.end())
  {
    XERR << "[公会GM-添加] 添加指令" << cmd.ShortDebugString() << "失败, sessionid :" << dwSessionID << "生成重复" << XEND;
    return false;
  }

  SGuildGM& rGM = m_mapSessionGM[dwSessionID];
  rGM.dwCreateTime = xTime::getCurSec();
  rGM.cmd.CopyFrom(cmd);
  rGM.cmd.mutable_info()->set_zoneid(thisServer->getZoneID());
  rGM.cmd.mutable_info()->set_sessionid(dwSessionID);
  PROTOBUF(rGM.cmd, send, len);
  bool bResult = thisServer->sendCmd(ClientType::guild_server, send, len);
  XDBG << "[公会GM-添加] 添加指令" << cmd.ShortDebugString() << "成功, sessionid :" << dwSessionID << "发送" << (bResult ? "成功" : "失败") << XEND;
  return bResult;
}

bool GuildGmMgr::respond(const GMCommandRespondGuildSCmd& cmd)
{
  auto m = m_mapSessionGM.find(cmd.info().sessionid());
  if (m == m_mapSessionGM.end())
  {
    XERR << "[公会GM-响应] 指令" << cmd.ShortDebugString() << "响应失败, 未找到sessionid :" << cmd.info().sessionid() << "指令" << XEND;
    return false;
  }

  m_mapSessionGM.erase(m);
  XLOG << "[公会GM-响应] 指令" << cmd.ShortDebugString() << "响应成功, sessionid :" << cmd.info().sessionid() << "指令被删除" << XEND;
  return true;
}

void GuildGmMgr::timer(DWORD curSec)
{
  flush(curSec);
}

void GuildGmMgr::flush(DWORD curSec)
{
  if (m_dwFlushTick > curSec)
    return;
  m_dwFlushTick = curSec + CommonConfig::m_dwGuildGMFlushTick;

  if (m_mapSessionGM.empty() == true)
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_gm");
  if (pField == nullptr)
  {
    XERR << "[公会GM-刷新] 获取 guild_gm 数据库表失败" << XEND;
    return;
  }

  xRecordSet recordSet;
  for (auto m = m_mapSessionGM.begin(); m != m_mapSessionGM.end();)
  {
    if (m->second.dwCreateTime + CommonConfig::m_dwGuildGMFlushTick > curSec)
    {
      ++m;
      continue;
    }

    const SGuildGM& rGM = m->second;
    string data;
    if (m->second.cmd.SerializeToString(&data) == false)
    {
      XERR << "[公会GM-刷新] 指令" << m->second.cmd.ShortDebugString() << "插入待存列表失败,序列化失败" << XEND;
    }
    else
    {
      xRecord record(pField);
      record.put("guildid", rGM.cmd.info().guildid());
      record.put("charid", rGM.cmd.info().charid());
      record.putBin("data", (unsigned char *)(data.c_str()), data.size());
      recordSet.push(record);
      XLOG << "[公会GM-刷新] 指令" << m->second.cmd.ShortDebugString() << "插入待存列表成功" << XEND;
    }

    m = m_mapSessionGM.erase(m);
  }

  if (recordSet.size() == 0)
    return;

  QWORD ret = thisServer->getDBConnPool().exeInsertSet(recordSet);
  if (ret == QWORD_MAX)
  {
    XERR << "[公会GM-刷新] 存储待存列表失败 ret :" << ret << "待存列表有" << recordSet.size() << "个" << XEND;
    return;
  }
  XLOG << "[公会GM-刷新] 存储待存列表成功 ret :" << ret << "待存列表有" << recordSet.size() << "个" << XEND;
}


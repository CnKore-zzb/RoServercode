#include "RecordGCityManager.h"
#include "xDBFields.h"
#include "xDBConnPool.h"
#include "GuildRaidConfig.h"
#include "DataServer.h"

RecordGCityManager::RecordGCityManager()
{

}

RecordGCityManager::~RecordGCityManager()
{

}

bool RecordGCityManager::loadCityInfo()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_city");
  if (pField == nullptr)
  {
    XERR << "[公会城池-加载] 在zoneid :" << thisServer->getRegionID() << "加载失败,未发现 guild_city 数据库表" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "zoneid = " << thisServer->getZoneID();

  xRecordSet result;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, result, sstr.str().c_str());
  if (ret == QWORD_MAX)
  {
    XERR << "[公会城池-加载] 在zoneid :" << thisServer->getRegionID() << "加载失败,查询 guild_city 数据库表失败" << XEND;
    return false;
  }

  for (DWORD d = 0; d < result.size(); ++d)
  {
    const xRecord& record = result[d];

    DWORD cityid = record.get<DWORD>("cityid");

    GuildCityInfo& rInfo = m_mapCityInfo[cityid];
    rInfo.set_id(record.get<QWORD>("guildid"));
    rInfo.set_flag(cityid);
    rInfo.set_oldguild(record.get<QWORD>("oldguildid"));
    DWORD times = record.get<DWORD>("times");
    //times = times ? times : 1;
    rInfo.set_times(times);

    XLOG << "[公会城池-加载] 在zoneid :" << thisServer->getRegionID() << "加载城池:" << rInfo.flag() << "成功,由" << rInfo.id() << "占领" << XEND;
  }
  return true;
}

void RecordGCityManager::updateCityInfo(const GuildCityActionGuildSCmd& cmd)
{
  if (cmd.infos_size() == 0)
  {
    XERR << "[公会城池-更新] 在 zoneid :" << thisServer->getZoneID() << "更新" << cmd.ShortDebugString() << "失败,没有城池数据" << XEND;
    return;
  }

  map<DWORD, EGuildCityResult> updateCityInfo; //flagid -> (guildid, enum_result)
  auto addupdate = [&](DWORD flagid, EGuildCityResult result)
  {
    updateCityInfo[flagid] = result;
  };

  if (cmd.status() == EGUILDCITYSTATUS_GIVEUP)
  {
    for (int i = 0; i < cmd.infos_size(); ++i)
    {
      const GuildCityInfo& rInfo = cmd.infos(i);
      auto m = m_mapCityInfo.find(rInfo.flag());
      if (m != m_mapCityInfo.end())
      {
        addupdate(rInfo.flag(), EGUILDCITYRESULT_GIVEUP);
        XLOG << "[公会城池-放弃], 公会:" << m->second.id() << "放弃据点:" << m->first << XEND;
        m->second.set_id(0);
        m->second.set_oldguild(0);
        m->second.set_times(0);
      }
    }
  }
  else if (cmd.status() == EGUILDCITYSTATUS_OCCUPY)
  {
    for (int i = 0; i < cmd.infos_size(); ++i)
    {
      const GuildCityInfo& rInfo = cmd.infos(i);

      for (auto &m : m_mapCityInfo)
      {
        // 之前占据的变野怪据点
        if (m.second.id() == rInfo.id())
        {
          XLOG << "[公会城池-占据], 公会:" << rInfo.id() << "占领新据点, 老的据点:" << m.first << "变为野怪据点" << XEND;
          m.second.set_id(0);
          addupdate(m.first, EGUILDCITYRESULT_NOOWNER);
          break;
        }
      }

      auto it = m_mapCityInfo.find(rInfo.flag());
      if (it != m_mapCityInfo.end())
      {
        // 仅更新最新占据id, 不更新占据次数等信息(公会成员信息发至guildserver获取->场景->前端)
        it->second.set_id(rInfo.id());
      }
      else
      {
        GuildCityInfo& rCity = m_mapCityInfo[rInfo.flag()];
        rCity.CopyFrom(rInfo);
      }

      addupdate(rInfo.flag(), EGUILDCITYRESULT_OCCUPY);
      XLOG << "[公会城池-占据], 公会:" << rInfo.id() << "占领新据点:" << rInfo.flag() << XEND;
    }
  }
  else if (cmd.status() == EGUILDCITYSTATUS_FINISH)
  {
    for (int i = 0; i < cmd.infos_size(); ++i)
    {
      const GuildCityInfo& rInfo = cmd.infos(i);
      auto it = m_mapCityInfo.find(rInfo.flag());
      if (it == m_mapCityInfo.end())
      {
        XERR << "[公会城池-占据], 公会:" << rInfo.id() << "城池:" << rInfo.flag() << "完成占领, 找不到占据信息" << XEND;
        continue;
      }
      if (it->second.oldguild() == it->second.id())
      {
        it->second.set_times(it->second.times() + 1);
      }
      else
      {
        it->second.set_oldguild(it->second.id());
        it->second.set_times(1);
      }
      XLOG << "[公会城池-占据], 公会:" << it->second.id() << "城池:" << it->first << "连续占据次数:" << it->second.times();
      addupdate(rInfo.flag(), EGUILDCITYRESULT_MIN);
    }
  }

  if (updateCityInfo.empty())
    return;

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_city");
  if (pField == nullptr)
  {
    XERR << "[公会城池-更新] 在 zoneid :" << thisServer->getZoneID() << "更新" << cmd.ShortDebugString() << "失败,未发现 guild_city 数据库表" << XEND;
    return;
  }

  for (auto &m : updateCityInfo)
  {
    auto it = m_mapCityInfo.find(m.first);
    if (it == m_mapCityInfo.end())
      continue;

    xRecord record(pField);
    record.put("zoneid", thisServer->getZoneID());
    record.put("cityid", it->first);
    record.put("guildid", it->second.id());
    record.put("oldguildid", it->second.oldguild());
    record.put("times", it->second.times());

    QWORD ret = thisServer->getDBConnPool().exeInsert(record, true, true);
    if (ret == QWORD_MAX)
    {
      XERR << "[公会城池-更新] 在 zoneid :" << thisServer->getZoneID() << "更新" << cmd.ShortDebugString() << "失败,写入 guild_city 数据库表失败" << XEND;
      syncCityInfo(m.first, EGUILDCITYRESULT_OTHER, cmd.scenename());
      continue;
    }

    syncCityInfo(m.first, m.second, cmd.scenename());
    XLOG << "[公会城池-更新] 在 zoneid :" << thisServer->getZoneID() << "更新" << cmd.ShortDebugString() << "成功" << XEND;
  }
}

void RecordGCityManager::syncCityInfo(DWORD dwCityID /*= 0*/, EGuildCityResult eResult /*= EGUILDCITYRESULT_MIN*/, const string& scenename /*= ""*/)
{
  GuildCityActionGuildSCmd cmd;
  cmd.set_action(EGUILDCITYACTION_TO_GUILD_UPDATE);
  cmd.set_result(eResult);
  cmd.set_zoneid(thisServer->getZoneID());
  cmd.set_scenename(scenename);

  if (dwCityID == 0)
  {
    for (auto &m : m_mapCityInfo)
      cmd.add_infos()->CopyFrom(m.second);
  }
  else
  {
    auto m = m_mapCityInfo.find(dwCityID);
    if (m != m_mapCityInfo.end())
      cmd.add_infos()->CopyFrom(m->second);
  }

  if (cmd.infos_size() <= 0)
  {
    XDBG << "[公会城池-同步] 在 zoneid :" << thisServer->getZoneID() << "同步" << cmd.ShortDebugString() << "失败,未有占领信息" << XEND;
    return;
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[公会城池-同步] 在 zoneid :" << thisServer->getZoneID() << "同步" << cmd.ShortDebugString() << "成功,发送至GuildServer处理" << XEND;
}

void RecordGCityManager::saveCityResult(const GvgResultGuildSCmd& cmd)
{
  if (cmd.infos_size() == 0)
    return;
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_city_history");
  if (pField == nullptr)
  {
    XERR << "[公会城池-结果统计] 在 zoneid :" << thisServer->getZoneID() << "更新" << cmd.ShortDebugString() << "失败,未发现 guild_city 数据库表" << XEND;
    return;
  }
  xRecordSet set;
  xRecord rec(pField);
  DWORD date = getFormatDate();
  for (int i = 0; i < cmd.infos_size(); ++i)
  {
    const GvgResultInfo& info = cmd.infos(i);
    rec.put("time", date);
    rec.put("zoneid", thisServer->getZoneID());
    rec.put("cityid", info.cityid());
    rec.put("guildid", info.guildid());
    rec.put("guildname", info.guildname());
    rec.put("leadername", info.leadername());
    rec.put("perfect", info.perfect());
    rec.put("times", info.times());
    set.push(rec);
  }
  QWORD ret = thisServer->getDBConnPool().exeInsertSet(set, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[公会城池-结果统计], 在zoneid:" << thisServer->getZoneID() << "更新异常" << cmd.ShortDebugString() << XEND;
    return;
  }
  XLOG << "[公会城池-结果统计], 更新结果成功, 共更新" << cmd.infos_size() << "条数据" << XEND;
}

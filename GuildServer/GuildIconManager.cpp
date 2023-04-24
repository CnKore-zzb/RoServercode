#include "GuildServer.h"
#include "GuildManager.h"
#include "GuildIconManager.h"
#include "MiscConfig.h"

void SGuildII::add(DWORD dwIndex, DWORD dwTime, std::string strType, EIconState eState, bool isRead)
{
  IconInfo info;
  info.set_index(dwIndex);
  info.set_state(eState);
  info.set_time(dwTime);
  info.set_type(strType);
  info.set_isread(isRead);

  if(m_mapII.end() != m_mapII.find(dwIndex))
    XERR << "[公会图标-图标添加] 重复添加" << dwIndex << eState << dwTime << strType << XEND;

  m_mapII[dwIndex] = info;
}

bool SGuildII::remove(DWORD dwIndex)
{
  if(m_mapII.end() == m_mapII.find(dwIndex))
  {
    XERR << "[公会图标-图标移除] 没有找到index" << dwIndex << XEND;
    return false;
  }

  m_mapII.erase(dwIndex);
  return true;
}

void SGuildII::setState(QWORD qwGuildId, DWORD dwIndex, EIconState eState)
{
  auto item = m_mapII.find(dwIndex);
  if(m_mapII.end() == item)
  {
    XERR << "[公会图标-状态设置] 设置失败（可能已被玩家删除）" << qwGuildId << dwIndex << eState << XEND;
    return ;
  }

  Guild* pGuild = GuildManager::getMe().getGuildByID(qwGuildId);
  if(!pGuild)
  {
    XERR << "[公会图标-状态设置] 设置失败 获取公会失败" << qwGuildId << dwIndex << eState << XEND;
    return ;
  }

  item->second.set_state(eState);

  GMember* pMember = pGuild->getChairman();
  if(!pMember)
  {
    return ;
  }

  GuildIconSyncGuildCmd cmd;
  IconInfo* pInfo = cmd.add_infos();
  if(pInfo)
    pInfo->CopyFrom(item->second);

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
}

void SGuildII::setState(QWORD qwGuildId, EIconState eState)
{
  Guild* pGuild = GuildManager::getMe().getGuildByID(qwGuildId);
  if (pGuild == nullptr)
    return;
  GMember* pMember = pGuild->getChairman();
  if (pMember == nullptr)
    return;

  GuildIconSyncGuildCmd cmd;
  for (auto &m : m_mapII)
  {
    m.second.set_state(eState);
    cmd.add_infos()->CopyFrom(m.second);
  }

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
}

void SGuildII::getData(GuildIconSyncGuildCmd& cmd)
{
  for(auto item = m_mapII.begin(); item != m_mapII.end(); item++)
  {
    IconInfo* pInfo = cmd.add_infos();
    if(pInfo)
      pInfo->CopyFrom(item->second);
  }
}

bool SGuildII::hasIndex(DWORD dwIndex)
{
  return m_mapII.end() != m_mapII.find(dwIndex);
}

bool SGuildII::checkState(DWORD dwIndex, DWORD dwTime, std::string strType)
{
  auto item = m_mapII.find(dwIndex);
  if(m_mapII.end() == item)
    return false;

  if(EICON_PASS != item->second.state())
    return false;

  if(dwTime != item->second.time())
    return false;

  if(strType != item->second.type())
    return false;

  return true;
}

bool SGuildII::hasUnread()
{
  for(auto item = m_mapII.begin(); item != m_mapII.end(); item++)
  {
    if(EICON_INIT != item->second.state() && !item->second.isread())
      return true;
  }

  return false;
}

void SGuildII::setRead(QWORD qwGuildId, bool isRead)
{
  for(auto item = m_mapII.begin(); item != m_mapII.end(); item++)
  {
    if(EICON_INIT != item->second.state() && !item->second.isread())
    {
      item->second.set_isread(isRead);
      GuildIconManager::getMe().readIndex(qwGuildId, item->first, isRead?1:0);
    }
  }
}


void GuildIconManager::clearGuild(QWORD qwGuildId)
{
  if(!del(qwGuildId, 0))
    return;

  if(m_mapGII.end() == m_mapGII.find(qwGuildId))
    return;

  m_mapGII.erase(qwGuildId);

  XLOG << "[公会图标-工会清除] 清除成功 公会id：" << qwGuildId << XEND;
}

bool GuildIconManager::init()
{
  m_mapGII.clear();
  return load();
}

bool GuildIconManager::load()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_icon");
  if(!pField)
  {
    XERR << "[公会icon-加载] 无法加载guild_icon" << XEND;
    return false;
  }

  xRecordSet set;
  char szWhere[64] = {0};
  snprintf(szWhere, 64, "id > 0");
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, szWhere);
  if(QWORD_MAX == retNum)
  {
    XERR << "[公会icon-加载] 查询guild_icon失败" << retNum << XEND;
    return false;
  }

  for (QWORD q = 0; q < retNum; ++q)
  {
    //QWORD id = set[q].get<QWORD>("id");
    //QWORD charid = set[q].get<QWORD>("charid");
    QWORD guildid = set[q].get<QWORD>("guildid");
    DWORD time = set[q].get<DWORD>("createtime");
    DWORD index = set[q].get<DWORD>("iconindex");
    DWORD state = set[q].get<DWORD>("state");
    DWORD read = set[q].get<DWORD>("isread");
    std::string type = set[q].getString("format");

    auto item = m_mapGII.find(guildid);
    if(m_mapGII.end() == item)
    {
      m_mapGII[guildid] = SGuildII();
      item = m_mapGII.find(guildid);
    }

    item->second.add(index, time, type, static_cast<EIconState>(state), 0 == read?false:true);
  }

  XLOG << "[公会图标-图标加载] 加载成功 加载数量：" << retNum << XEND;
  return true;
}

void GuildIconManager::add(QWORD qwGuildId, QWORD qwCharId, DWORD dwIndex, std::string strType)
{
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  if(rCFG.dwIconCount < dwIndex)
  {
    XERR << "[公会图标-图标添加] 添加失败 index超出上限(玩家企图作弊)" << qwGuildId << qwCharId << dwIndex << ":" << rCFG.dwIconCount << XEND;
    return;
  }

  if(!insert(qwGuildId, qwCharId, dwIndex, strType))
    return;

  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
  {
    m_mapGII[qwGuildId] = SGuildII();
    item = m_mapGII.find(qwGuildId);
  }

  item->second.add(dwIndex, xTime::getCurSec(), strType);

  XLOG << "[公会图标-图标添加] 添加成功" << qwGuildId << qwCharId << dwIndex << strType << XEND;
}

bool GuildIconManager::remove(QWORD qwGuildId, DWORD dwIndex)
{
  if(!del(qwGuildId, dwIndex))
    return false;

  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
  {
    XERR << "[公会图标-图标移除] 找不到公会" << qwGuildId << dwIndex << XEND;
    return false;
  }
  return item->second.remove(dwIndex);
}

void GuildIconManager::setState(QWORD qwGuildId, DWORD dwIndex, EIconState eState)
{
  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
  {
    XERR << "[公会图标-状态设置] 设置失败" << qwGuildId << dwIndex << eState << XEND;
    return;
  }

  item->second.setState(qwGuildId, dwIndex, eState);
}

void GuildIconManager::setState(QWORD qwGuildId, EIconState eState)
{
  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
  {
    XERR << "[公会图标-状态设置] 设置失败" << qwGuildId << eState << XEND;
    return;
  }

  item->second.setState(qwGuildId, eState);
}

bool GuildIconManager::insert(QWORD qwGuildId, QWORD qwCharId, DWORD dwIndex, std::string strType)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_icon");
  if(!pField)
  {
    XERR << "[公会图标-图标插入] 获取guild_icon field失败" << XEND;
    return false;
  }

  xRecord record(pField);
  record.put("id", qwGuildId*1000 + dwIndex);
  record.put("guildid", qwGuildId);
  record.put("charid", qwCharId);
  record.put("createtime", xTime::getCurSec());
  record.put("iconindex", dwIndex);
  record.put("state", 0);
  record.put("isread", 0);
  record.put("format", strType);

  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
  if(QWORD_MAX == ret)
  {
    XERR << "[公会图标-图标插入] 插入失败" << qwGuildId << dwIndex << XEND;
    return false;
  }

  XLOG << "[公会图标-图标插入] 插入成功" << qwGuildId << qwCharId << dwIndex << XEND;
  return true;
}

bool GuildIconManager::del(QWORD qwGuildId, DWORD dwIndex)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_icon");
  if(!pField)
  {
    XERR << "[公会图标-图标删除] 获取guild_icon field失败" << XEND;
    return false;
  }

  char szWhere[64] = {0};
  if(0 == dwIndex)
    snprintf(szWhere, 64, "guildid=%llu", qwGuildId);
  else
    snprintf(szWhere, 64, "id=%llu", qwGuildId*1000+dwIndex);

  QWORD ret = thisServer->getDBConnPool().exeDelete(pField, szWhere);
  if(QWORD_MAX == ret)
  {
    XERR << "[公会图标-图标删除] 插入失败" << qwGuildId << dwIndex << XEND;
    return false;
  }

  XLOG << "[公会图标-图标删除] 删除成功" << qwGuildId << dwIndex << XEND;
  return true;
}

bool GuildIconManager::update(QWORD qwGuildId, DWORD dwIndex, DWORD dwRead)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guild_icon");
  if(!pField)
  {
    XERR << "[公会图标-图标更新] 获取guild_icon field失败" << XEND;
    return false;
  }

  xRecord record(pField);
  record.put("isread", dwRead);

  char szWhere[64] = {0};
  snprintf(szWhere, 64, "id=%llu", qwGuildId*1000+dwIndex);
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, szWhere);
  if(QWORD_MAX == ret)
  {
    XERR << "[公会图标-图标删除] 更新失败" << qwGuildId << dwIndex << dwRead << XEND;
    return false;
  }

  XLOG << "[公会图标-图标更新] 更新成功" << qwGuildId << dwIndex << dwRead << XEND;
  return true;
}

void GuildIconManager::syncData(QWORD qwGuildId, GMember* pMember)
{
  if(!pMember)
    return;

  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
    return;

  GuildIconSyncGuildCmd cmd;
  item->second.getData(cmd);

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
}

void GuildIconManager::onUserOnline(QWORD qwGuildId, GMember* pMember)
{
  if(!pMember)
    return;

  syncData(qwGuildId, pMember);

  if(hasUnread(qwGuildId))
    pMember->redTipMessage(EREDSYS_GUILD_ICON);
}

bool GuildIconManager::hasIndex(QWORD qwGuildId, DWORD dwIndex)
{
  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
  {
    return false;
  }

  return item->second.hasIndex(dwIndex);
}

bool GuildIconManager::checkPortrait(QWORD qwGuildId, const string& strPortrait)
{
  TVecString vec;
  stringTok(strPortrait, "_", vec);
  if (2 > vec.size())
    return true;

  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
    return false;

  if(2 == vec.size())
    return item->second.checkState(atoi(vec[0].c_str()), atoi(vec[1].c_str()));
  else if(3 == vec.size())
    return item->second.checkState(atoi(vec[0].c_str()), atoi(vec[1].c_str()), vec[2]);

  return false;
}

bool GuildIconManager::hasUnread(QWORD qwGuildId)
{
  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
    return false;

  return item->second.hasUnread();
}

void GuildIconManager::setRead(QWORD qwGuildId, bool isRead)
{
  auto item = m_mapGII.find(qwGuildId);
  if(m_mapGII.end() == item)
    return;

  item->second.setRead(qwGuildId, isRead);
}

void GuildIconManager::readIndex(QWORD qwGuildId, DWORD dwIndex, DWORD dwRead)
{
  update(qwGuildId, dwIndex, dwRead);
}


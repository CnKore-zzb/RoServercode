#include "SessionGvg.h"
#include "SessionUser.h"
#include "GuildRaidConfig.h"
#include "MsgManager.h"

void SCityShowInfo::toData(CityShowInfo* pData)
{
  if (pData == nullptr)
    return;
  pData->set_cityid(dwCityID);

  if (stCityInfo.id() == 0) // 无人占领
  {
    pData->set_state(EGCITYSTATE_NOOWNER);
  }
  else
  {
    pData->set_state(eState);
    pData->set_guildid(stCityInfo.id());
    pData->set_name(stCityInfo.name());
    pData->set_portrait(stCityInfo.portrait());
    pData->set_lv(stCityInfo.lv());
    pData->set_membercount(stCityInfo.membercount());
  }
}

SessionGvg::SessionGvg()
{
  const TMapGuildCityCFG& rCFG = GuildRaidConfig::getMe().getGuildCityCFGList();
  for (auto &m : rCFG)
    m_mapCityInfo[m.first].dwCityID = m.first;
}

SessionGvg::~SessionGvg()
{

}

void SessionGvg::updateCityInfoFromGuild(const GuildCityActionGuildSCmd& cmd)
{
  if (cmd.action() != EGUILDCITYACTION_TO_SCENE_UPDATE)
  {
    XLOG << "[公会城池-同步]" << cmd.ShortDebugString() << "失败,action不对应" << XEND;
    return;
  }
  for (int i = 0; i < cmd.infos_size(); ++i)
  {
    const GuildCityInfo& rInfo = cmd.infos(i);
    SCityShowInfo& data = m_mapCityInfo[rInfo.flag()];
    data.stCityInfo.CopyFrom(rInfo);

    // 初始化
    if (data.eState == EGCITYSTATE_MIN)
      data.eState = (rInfo.id() != 0 ? EGCITYSTATE_OCCUPY : EGCITYSTATE_NOOWNER);
  }
  XLOG << "[公会城池-同步]" << cmd.ShortDebugString() << XEND;
}

void SessionGvg::updateCityInfoFromGuild(const CityDataUpdateGuildSCmd& cmd)
{
  auto m = m_mapCityInfo.find(cmd.cityid());
  if (m == m_mapCityInfo.end())
    return;

  if (m->second.stCityInfo.membercount() != cmd.membercount())
    m->second.stCityInfo.set_membercount(cmd.membercount());

  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const GuildDataUpdate& rUpdate = cmd.updates(i);
    switch (rUpdate.type())
    {
      case EGUILDDATA_NAME:
        m->second.stCityInfo.set_name(rUpdate.data());
        break;
      case EGUILDDATA_LEVEL:
        m->second.stCityInfo.set_lv(rUpdate.value());
        break;
      case EGUILDDATA_PORTRAIT:
        m->second.stCityInfo.set_portrait(rUpdate.data());
        break;
      default:
        break;
    }
    XLOG << "[公会城池-数据更新] 更新 flag :" << cmd.cityid() << "数据" << rUpdate.ShortDebugString() << XEND;
  }
}

void SessionGvg::queryCityShowInfo(SessionUser* user)
{
  if (user == nullptr)
    return;
  QueryGCityShowInfoGuildCmd cmd;
  for (auto &m : m_mapCityInfo)
  {
    m.second.toData(cmd.add_infos());
  }
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

void SessionGvg::updateCityState(const UpdateCityStateGuildSCmd& cmd)
{
  for (int i = 0; i < cmd.infos_size(); ++i)
  {
    const CityShowInfo& info = cmd.infos(i);
    auto it = m_mapCityInfo.find(info.cityid());
    if (it == m_mapCityInfo.end())
      continue;
    it->second.eState = info.state();
  }
}

void SessionGvg::setFireStatus(bool fire)
{
  if (m_bFire == fire)
    return;
  m_bFire = fire;

  if (m_bFire)
  {
    for (auto &m : m_mapCityInfo)
    {
      m.second.eState = (m.second.stCityInfo.id() ? EGCITYSTATE_NORMALFIRE : EGCITYSTATE_NOOWNER);
      m.second.qwOldGuildID = m.second.stCityInfo.id();
    }
  }
  else
  {
    for (auto &m : m_mapCityInfo)
    {
      m.second.eState = (m.second.stCityInfo.id() ? EGCITYSTATE_OCCUPY : EGCITYSTATE_NOOWNER);

      QWORD qwOldGuildID = m.second.qwOldGuildID;
      QWORD qwNewGuildID = m.second.stCityInfo.id();

      UpdateCityGuildSCmd cmd;
      cmd.set_cityid(m.first);

      if (qwOldGuildID == 0 && qwNewGuildID != 0)
      {
        cmd.set_guildid(qwNewGuildID);
        cmd.set_add(true);

        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);
      }
      else if (qwOldGuildID != 0 && qwNewGuildID == 0)
      {
        cmd.set_guildid(qwOldGuildID);
        cmd.set_add(false);

        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);
      }
      else if (qwOldGuildID != 0 && qwNewGuildID != 0 && qwOldGuildID != qwNewGuildID)
      {
        cmd.set_guildid(qwOldGuildID);
        cmd.set_add(false);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::guild_server, send, len);

        cmd.set_guildid(qwNewGuildID);
        cmd.set_add(true);
        PROTOBUF(cmd, send1, len1);
        thisServer->sendCmd(ClientType::guild_server, send1, len1);
      }
    }
  }

  GvgOpenFireGuildCmd cmd;
  cmd.set_fire(m_bFire);
  PROTOBUF(cmd, send, len);
  MsgManager::sendWorldCmd(send, len);
}


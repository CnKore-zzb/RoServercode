#include "UserGvg.h"
#include "SceneUser.h"
#include "GuildCityManager.h"
#include "DScene.h"
#include "MiscConfig.h"

UserGvg::UserGvg(SceneUser* user) : m_pUser(user), m_oTenSecTimer(10)
{

}

UserGvg::~UserGvg()
{

}

bool UserGvg::load(const BlobGvgData& oData)
{
  m_stGvgData.clear();
  //若公会战已结束则清空数据
  if (now() < oData.expiretime())
  {
    for (int i = 0; i < oData.datas_size(); ++i)
    {
      m_stGvgData.mapType2Data[oData.datas(i).type()] = oData.datas(i).value();
    }
    m_stGvgData.dwExpireTime = oData.expiretime();
  }
  return true;
}

bool UserGvg::save(BlobGvgData* pData)
{
  if (pData == nullptr)
    return false;

  pData->Clear();

  if (now() < m_stGvgData.dwExpireTime)
  {
    for (auto &m : m_stGvgData.mapType2Data)
    {
      GvgData* p = pData->add_datas();
      if (p == nullptr)
        continue;
      p->set_type(m.first);
      p->set_value(m.second);
    }
  }
  pData->set_expiretime(m_stGvgData.dwExpireTime);
  return true;
}

void UserGvg::timer(DWORD cur)
{
  if (m_oTenSecTimer.timeUp(cur))
  {
    if (checkInGvgFire())
    {
      auto it = m_stGvgData.mapType2Data.find(EGVGDATA_PARTINTIME);
      if (it == m_stGvgData.mapType2Data.end())
      {
        m_stGvgData.mapType2Data[EGVGDATA_PARTINTIME] = 10;
        it = m_stGvgData.mapType2Data.find(EGVGDATA_PARTINTIME);
        if (it == m_stGvgData.mapType2Data.end())
          return;
      }
      else
      {
        it->second += 10;
      }

      const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
      const TMapGvgTimes2RewardData& reward = firecfg.getRewardInfo(EGVGDATA_PARTINTIME);
      for (auto &m : reward)
      {
        if (m.first * 60 == it->second)
        {
          givereward(m.second.dwItemID, m.second.dwItemCnt);
          XLOG << "[玩家-gvg], 参战时间奖励达成, 玩家:" << m_pUser->name << m_pUser->id << "时间:" << it->second << "奖励:" << m.second.dwItemID << m.second.dwItemCnt << XEND;
          break;
        }
      }

      // 正分钟更新
      if (it->second % 60 == 0)
        updateToClient(EGVGDATA_PARTINTIME);

      if (it->second == firecfg.dwPartInTime && m_pUser->getGuild().id())
      {
        // 通知公会服,达成参战条件
        GvgUserPartInGuildSCmd cmd;
        cmd.set_guildid(m_pUser->getGuild().id());
        cmd.set_charid(m_pUser->id);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
        m_pUser->getServant().onFinishEvent(ETRIGGER_GVG);
        m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GVG);
      }
      XLOG << "[玩家-gvg], 参战时间增加, 玩家:" << m_pUser->name << m_pUser->id << "当前参战时间:" << m_stGvgData.mapType2Data[EGVGDATA_PARTINTIME] << XEND;
    }
    else if (m_pUser->getScene() && m_pUser->getScene()->isSuperGvg())
    {
      SuperGvgScene* pScene = dynamic_cast<SuperGvgScene*>(m_pUser->getScene());
      if (pScene)
        pScene->addPartinTime(m_pUser, 10); // 增加参战时间
    }
  }
}

bool UserGvg::checkInGvgFire()
{
  if (m_pUser->getScene() && m_pUser->getScene()->getSceneType() == SCENE_TYPE_GUILD_FIRE)
  {
    GuildFireScene* pGScene = dynamic_cast<GuildFireScene*>((m_pUser->getScene()));
    if (pGScene == nullptr)
      return false;
    if (GuildCityManager::getMe().isCityInFire(pGScene->getCityID()))
      return true;
  }
  return false;
}

void UserGvg::onKillMonster()
{
  if (checkInGvgFire() == false)
    return;

  addCount(EGVGDATA_KILLMON, 1);
}

void UserGvg::onReliveUser()
{
  if (checkInGvgFire() == false)
    return;

  addCount(EGVGDATA_RELIVE, 1);
}

void UserGvg::onExpelOther()
{
  if (checkInGvgFire() == false)
    return;

  addCount(EGVGDATA_EXPEL, 1);
}

void UserGvg::onDamMetal(DWORD per)
{
  if (checkInGvgFire() == false)
    return;
  DWORD preper = 0;
  auto it = m_stGvgData.mapType2Data.find(EGVGDATA_DAMMETAL);
  if (it == m_stGvgData.mapType2Data.end())
  {
    m_stGvgData.mapType2Data[EGVGDATA_DAMMETAL] = per;
    it = m_stGvgData.mapType2Data.find(EGVGDATA_DAMMETAL);
    if (it == m_stGvgData.mapType2Data.end())
      return;
  }
  else
  {
    preper = it->second;
    it->second += per;
  }

  const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
  const TMapGvgTimes2RewardData& reward = firecfg.getRewardInfo(EGVGDATA_DAMMETAL);

  // 记录万分比,配置表百分比
  for (auto &m : reward)
  {
    if (m.first > preper / 100 && m.first <= it->second / 100)
    {
      givereward(m.second.dwItemID, m.second.dwItemCnt);
      XLOG << "[玩家-gvg], 任务完成, 任务类型:" << EGVGDATA_DAMMETAL << "玩家:" << m_pUser->name << m_pUser->id << "数量:" << it->second << "奖励:" << m.second.dwItemID << m.second.dwItemCnt << XEND;
      break;
    }
  }

  if (preper / 100 != it->second / 100)
    updateToClient(EGVGDATA_DAMMETAL);
}

void UserGvg::addCount(EGvgDataType eType, DWORD count)
{
  DWORD precnt = 0;
  auto it = m_stGvgData.mapType2Data.find(eType);
  if (it == m_stGvgData.mapType2Data.end())
  {
    m_stGvgData.mapType2Data[eType] = count;
    it = m_stGvgData.mapType2Data.find(eType);
    if (it == m_stGvgData.mapType2Data.end())
      return;
  }
  else
  {
    precnt = it->second;
    it->second += count;
  }

  const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
  const TMapGvgTimes2RewardData& reward = firecfg.getRewardInfo(eType);

  for (auto &m : reward)
  {
    if (m.first > precnt && m.first <= it->second)
    {
      givereward(m.second.dwItemID, m.second.dwItemCnt);
      XLOG << "[玩家-gvg], 任务完成, 任务类型:" << eType << "玩家:" << m_pUser->name << m_pUser->id << "数量:" << it->second << "奖励:" << m.second.dwItemID << m.second.dwItemCnt << XEND;
    }
  }

  XLOG << "[玩家-gvg], 任务更新, 任务类型:" << eType << "玩家:" << m_pUser->name << m_pUser->id << "当前进度:" << it->second << XEND;

  updateToClient(eType);
}

void UserGvg::onKillUser()
{
  auto it = m_stGvgData.mapType2Data.find(EGVGDATA_KILLUSER);
  if (it == m_stGvgData.mapType2Data.end())
  {
    m_stGvgData.mapType2Data[EGVGDATA_KILLUSER] = 1;
    it = m_stGvgData.mapType2Data.find(EGVGDATA_KILLUSER);
    if (it == m_stGvgData.mapType2Data.end())
      return;
  }
  else
  {
    it->second += 1;
  }

  const SGuildFireCFG& firecfg = MiscConfig::getMe().getGuildFireCFG();
  givereward(firecfg.dwKillUserItem, firecfg.dwKillUserItemCnt);

  std::set<SceneUser*> userset = m_pUser->getTeamSceneUser();
  userset.erase(m_pUser);
  for (auto &s : userset)
  {
    s->getUserGvg().givereward(firecfg.dwKillUserItem, firecfg.dwKillUserTeamGetNum);
    XLOG << "[玩家-gvg], 队友击杀, 获取荣誉值, 击杀玩家:" << m_pUser->name << m_pUser->id << "获取奖励玩家:" << s->name << s->id << XEND;
  }

  updateToClient(EGVGDATA_KILLUSER);
}

void UserGvg::updateToClient(EGvgDataType eType) const
{
  auto it = m_stGvgData.mapType2Data.find(eType);
  if (it == m_stGvgData.mapType2Data.end())
    return;
  GvgDataUpdateCmd cmd;
  GvgData* pData = cmd.mutable_data();
  if (pData == nullptr)
    return;
  formatData(pData, eType, it->second);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserGvg::givereward(DWORD itemid, DWORD count)
{
  DWORD realget = count;
  if (itemid == ITEM_HONOR)
  {
    auto it = m_stGvgData.mapType2Data.find(EGVGDATA_HONOR);
    if (it != m_stGvgData.mapType2Data.end())
    {
      DWORD maxcnt = MiscConfig::getMe().getGuildFireCFG().dwMaxHonorCount;
      if (count + it->second > maxcnt)
        realget = maxcnt > it->second ? maxcnt - it->second : 0;
    }
  }

  if (realget)
  {
    ItemInfo item;
    item.set_id(itemid);
    item.set_count(realget);
    item.set_source(ESOURCE_GVG);

    m_pUser->getPackage().addItem(item, EPACKMETHOD_AVAILABLE);
  }
  XLOG << "[玩家-gvg], 获取奖励, 玩家:" << m_pUser->name << m_pUser->id << "奖励:" << itemid << count << "实际获得数量:" << realget << XEND;

  if (realget && itemid == ITEM_HONOR)
    onGvgGetHonor(realget);
}

void UserGvg::onGvgGetHonor(DWORD honor)
{
  if (checkInGvgFire() == false)
    return;

  addCount(EGVGDATA_HONOR, honor);
}


void UserGvg::onKillMetal()
{
  if (checkInGvgFire() == false)
    return;

  addCount(EGVGDATA_KILLMETAL, 1);
}

void UserGvg::syncDataToMe()
{
  GvgDataSyncCmd cmd;
  if (m_stGvgData.dwExpireTime < now())
    return;

  for (auto &m : m_stGvgData.mapType2Data)
  {
    GvgData* pData = cmd.add_datas();
    if (pData == nullptr)
      continue;
    formatData(pData, m.first, m.second);
  }

  if (cmd.datas_size() != 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserGvg::onEnterGvg()
{
  if (GuildCityManager::getMe().isInFire())
  {
    DWORD stoptime = GuildCityManager::getMe().getFireStopTime();
    if (m_stGvgData.dwExpireTime < stoptime)
    {
      m_stGvgData.clear();
      m_stGvgData.dwExpireTime = stoptime;
    }
  }
}

void UserGvg::formatData(GvgData* pData, EGvgDataType eType, DWORD value) const
{
  if (pData == nullptr)
    return;

  pData->set_type(eType);
  switch(eType)
  {
    case EGVGDATA_MIN:
      return;
    case EGVGDATA_PARTINTIME:
      pData->set_value(value / 60); //分钟
      break;
    case EGVGDATA_DAMMETAL:
      pData->set_value(value / 100); //万分比->百分比
      break;
    case EGVGDATA_KILLMON:
    case EGVGDATA_RELIVE:
    case EGVGDATA_EXPEL:
    case EGVGDATA_KILLMETAL:
    case EGVGDATA_HONOR:
    case EGVGDATA_KILLUSER:
      pData->set_value(value);
      break;
    default:
      break;
  }
}


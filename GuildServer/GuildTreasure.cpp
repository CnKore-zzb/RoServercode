#include "GuildTreasure.h"
#include "Guild.h"
#include "GuildServer.h"
#include "RewardConfig.h"
#include "MiscConfig.h"
#include "LuaManager.h"

GTreasure::GTreasure(Guild* pGuild) : m_pGuild(pGuild)
{

}

GTreasure::~GTreasure()
{

}

bool GTreasure::fromData(const Cmd::BlobGuildTreasure& data)
{
  m_mapTreasure.clear();
  for (int i = 0; i < data.treasures_size(); ++i)
  {
    const GuildTreasure& rTreasure = data.treasures(i);
    m_mapTreasure[rTreasure.id()] = rTreasure.count();
  }

  m_mapTreasureResult.clear();
  for (int i = 0; i < data.result_size(); ++i)
  {
    const TreasureResult& rResult = data.result(i);
    m_mapTreasureResult[rResult.eventguid()].CopyFrom(rResult);
  }

  return true;
}

bool GTreasure::toData(Cmd::BlobGuildTreasure* pData)
{
  if (pData == nullptr)
    return false;

  pData->Clear();
  for (auto &m : m_mapTreasure)
  {
    GuildTreasure* pTreasure = pData->add_treasures();
    pTreasure->set_id(m.first);
    pTreasure->set_count(m.second);
  }

  for (auto &m : m_mapTreasureResult)
  {
    TreasureResult* pResult = pData->add_result();
    pResult->CopyFrom(m.second);
  }

  return true;
}

void GTreasure::queryTreasureToScene()
{
  Cmd::QueryTreasureGuildSCmd cmd;
  cmd.set_guildid(m_pGuild->getGUID());
  cmd.set_result(true);

  cmd.clear_treasures();
  for (auto &m : m_mapTreasure)
  {
    Cmd::GuildTreasure* pTreasure = cmd.add_treasures();
    pTreasure->set_id(m.first);
    pTreasure->set_count(m.second);
  }

  cmd.set_bcoin_count(m_pGuild->getMisc().getVarValue(EVARTYPE_BCOIN_TREASURE_COUNT));
  cmd.set_asset_count(m_pGuild->getMisc().getVarValue(EVARTYPE_GUILD_TREASURE_COUNT));

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);
  XDBG << "[公会宝箱-查询]" << m_pGuild->getGUID() << m_pGuild->getName() << "查询结果" << cmd.ShortDebugString() << XEND;
}

void GTreasure::queryTreasureResult(QWORD charid, DWORD dwEventGUID)
{
  GMember* pMember = m_pGuild->getMember(charid);
  if (pMember == nullptr)
  {
    XERR << "[公会宝箱-领奖查询]" << m_pGuild->getGUID() << m_pGuild->getName() << "charid" << charid << "查询id" << dwEventGUID << "失败,不是公会成员" << XEND;
    return;
  }

  auto m = m_mapTreasureResult.find(dwEventGUID);
  if (m == m_mapTreasureResult.end())
  {
    XERR << "[公会宝箱-领奖查询]" << m_pGuild->getGUID() << m_pGuild->getName() << "charid" << charid << "查询id" << dwEventGUID << "失败,未包含事件" << XEND;
    return;
  }

  const GuildEvent* pEvent = m_pGuild->getEvent().getEvent(dwEventGUID);
  if (pEvent == nullptr)
  {
    m = m_mapTreasureResult.erase(m);
    XERR << "[公会宝箱-领奖查询]" << m_pGuild->getGUID() << m_pGuild->getName() << "charid" << charid << "查询id" << dwEventGUID << "失败,该结果已过期" << XEND;
    return;
  }

  QueryTreasureResultGuildCmd cmd;
  cmd.set_eventguid(dwEventGUID);
  cmd.mutable_result()->CopyFrom(m->second);

  SGuildWelfareItem* pItem = m_pGuild->getMisc().getWelfare().getWelfareItem(EGUILDWELFARE_TREASURE, dwEventGUID);
  TreasureResult* pResult = cmd.mutable_result();
  if (pItem != nullptr)
  {
    if (pItem->canGetWelfare(pMember) == true)
    {
      pResult->set_state(ETREASURESTATE_UNGETED);

      for (int i = 0; i < cmd.result().items_size(); ++i)
      {
        const TreasureItem& rItem = cmd.result().items(i);
        if (rItem.charid() == pMember->getCharID())
        {
          cmd.mutable_result()->set_state(ETREASURESTATE_GETED);
          break;
        }
      }
    }
    else
    {
      // 进公会前发放的奖励不可领, 过期的不可领
      if (pMember->getEnterTime() > pItem->dwCreateTime || xTime::getCurSec() >= pItem->dwOverdueTime)
        pResult->set_state(ETREASURESTATE_UNENABLE);
      else
      {
        DWORD weekstart = xTime::getWeekStart(pItem->dwCreateTime, 5 * 3600);
        if(pMember->getEnterTime() >= weekstart)
          pResult->set_state(ETREASURESTATE_UNENABLE);
        else
          pResult->set_state(ETREASURESTATE_GETED);
      }
    }
  }

  PROTOBUF(cmd, send, len);
  pMember->sendCmdToMe(send, len);
  XDBG << "[公会宝箱-领奖查询]" << m_pGuild->getGUID() << m_pGuild->getName() << "charid" << charid << "查询id" << cmd.eventguid() << "成功,信息" << cmd.ShortDebugString() << XEND;
}

bool GTreasure::win(DWORD dwCityID, DWORD dwTimes)
{
  const SGuildCityCFG* pCFG = GuildRaidConfig::getMe().getGuildCityCFG(dwCityID);
  if (pCFG == nullptr)
  {
    XERR << "[公会宝箱-胜利]" << m_pGuild->getGUID() << m_pGuild->getName() << "占领城池" << dwCityID << dwTimes << "次,获取宝箱失败,该城池未在 Table_Guild_StrongHold.txt 表中找到" << XEND;
    return false;
  }

  TMapTreasure mapTreasure;
  if (pCFG->collectTreasure(dwTimes, mapTreasure) == false)
  {
    XERR << "[公会宝箱-胜利]" << m_pGuild->getGUID() << m_pGuild->getName() << "占领城池" << dwCityID << dwTimes << "次,获取宝箱失败,收集宝箱奖励失败" << XEND;
    return false;
  }

  for (auto &m : mapTreasure)
  {
    DWORD& rCount = m_mapTreasure[m.first];
    rCount += m.second;
    XLOG << "[公会宝箱-胜利]" << m_pGuild->getGUID() << m_pGuild->getName() << "占领城池" << dwCityID << dwTimes << "次,宝箱" << m.first << "数量" << rCount - m.second << "->" << rCount << XEND;
  }

  queryTreasureToScene();
  m_pGuild->setMark(EGUILDDATA_TREASURE_GVG_COUNT);
  m_pGuild->updateData(true);
  return true;
}

bool GTreasure::open(QWORD charid, DWORD treasureid)
{
  GMember* pMember = m_pGuild->getMember(charid);
  if (pMember == nullptr)
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员" << charid << "开启宝箱" << treasureid << "失败,不是公会成员" << XEND;
    return false;
  }
  const SGuildTreasureCFG* pCFG = GuildConfig::getMe().getTreasureCFG(treasureid);
  if (pCFG == nullptr)
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启宝箱" << treasureid << "失败,未在 Table_GuildTreasure.txt 表中找到" << XEND;
    return false;
  }

  if (pCFG->dwCityID != 0 && m_pGuild->getMisc().getCityID() != pCFG->dwCityID)
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启宝箱" << treasureid << "失败,城池" << pCFG->dwCityID << "未被占领" << XEND;
    return false;
  }

  DWORD dwGuildTime = 0;
  DWORD dwPrice = 0;
  if (pCFG->eType == EGUILDTREASURETYPE_GVG)
  {
    if (m_pGuild->getMisc().hasAuth(pMember->getJob(), EAUTH_TREASURE_OPT) == false)
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,没有权限" << XEND;
      return false;
    }
    auto m = m_mapTreasure.find(treasureid);
    if (m == m_mapTreasure.end())
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,未包含该宝箱" << XEND;
      return false;
    }
    if (m->second == 0)
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,该宝箱没有数量" << XEND;
      return false;
    }

    --m->second;
    if (m->second == 0)
    {
      m_mapTreasure.erase(m);
      m_pGuild->setMark(EGUILDDATA_TREASURE_GVG_COUNT);
    }
  }
  else if (pCFG->eType == EGUILDTREASURETYPE_GUILD_ASSET)
  {
    DWORD dwTime = m_pGuild->getMisc().getVarValue(EVARTYPE_GUILD_TREASURE_COUNT) + 1;
    dwPrice = MiscConfig::getMe().getGuildCFG().getAssetPrice(dwTime);
    if (dwPrice == DWORD_MAX)
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,获取公会资金价格失败" << XEND;
      return false;
    }
    if (m_pGuild->getAsset() < dwPrice)
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,公会资金不足" << XEND;
      return false;
    }
    m_pGuild->subAsset(dwPrice, ESOURCE_GUILD_TREASURE);

    dwGuildTime = m_pGuild->getMisc().getVarValue(EVARTYPE_GUILD_TREASURE_COUNT);
    m_pGuild->getMisc().setVarValue(EVARTYPE_GUILD_TREASURE_COUNT, dwTime);
  }
  else if (pCFG->eType == EGUILDTREASURETYPE_GUILD_BCOIN)
  {
    GuildMisc& rMisc = m_pGuild->getMisc();
    DWORD dwTime = rMisc.getVarValue(EVARTYPE_BCOIN_TREASURE_COUNT) + 1;
    dwPrice = MiscConfig::getMe().getGuildCFG().getBCoinPrice(dwTime);
    dwGuildTime = rMisc.getVarValue(EVARTYPE_BCOIN_TREASURE_COUNT);
    rMisc.setVarValue(EVARTYPE_BCOIN_TREASURE_COUNT, dwTime);
    pMember->addTotalBCoin(dwPrice);
  }
  else
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,未知宝箱类型" << XEND;
    return false;
  }

  TVecItemInfo vecItems;
  for (auto &s : pCFG->setGuildReward)
  {
    TVecItemInfo vecSingle;
    if (RewardConfig::getMe().roll(s, RewardEntry(), vecSingle, ESOURCE_GUILD_TREASURE) == false)
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,随机" << s << "失败" << XEND;
      return false;
    }
    combinItemInfo(vecItems, vecSingle);
  }

  TSetDWORD setIDs;
  if (pCFG->collectMemberReward(dwGuildTime + 1, setIDs) == false)
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "收集成员奖励,失败" << XEND;
    return false;
  }

  DWORD dwGUID = 0;
  if (pCFG->eType == EGUILDTREASURETYPE_GVG)
  {
    m_pGuild->getEvent().addEvent(EGUILDEVENT_TREASURE_OPEN_GVG, TVecString{pMember->getName(), pCFG->strName});
  }
  else if (pCFG->eType == EGUILDTREASURETYPE_GUILD_ASSET || pCFG->eType == EGUILDTREASURETYPE_GUILD_BCOIN)
  {
    DWORD dwItemID = 0;
    if (pCFG->eType == EGUILDTREASURETYPE_GUILD_ASSET)
      dwItemID = ITEM_ASSET;
    else if (pCFG->eType == EGUILDTREASURETYPE_GUILD_BCOIN)
      dwItemID = ITEM_BCOIN;

    const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(dwItemID);
    if (pItemCFG != nullptr)
    {
      stringstream sstr;
      sstr << dwPrice;
      const GuildEvent* pEvent = m_pGuild->getEvent().addEvent(EGUILDEVENT_TREASURE_OPEN_GUILD, TVecString{pMember->getName(), sstr.str(), pItemCFG->strNameZh, pCFG->strName});
      if (pEvent != nullptr)
        dwGUID = pEvent->guid();

      MsgParams oParams;
      oParams.addString(pMember->getName());
      oParams.addString(sstr.str());
      oParams.addString(pItemCFG->strNameZh);
      oParams.addString(pCFG->strName);

      sstr.str("");
      sstr << "treasure;" << dwGUID;
      oParams.addString(sstr.str());
      m_pGuild->broadcastNpcMsg(MiscConfig::getMe().getGuildCFG().dwTreasureBroadcastNpc, 4038, oParams);
    }
  }

  for (auto &s : setIDs)
  {
    if (m_pGuild->getMisc().getWelfare().addWelfare(EGUILDWELFARE_TREASURE, s, ESOURCE_GUILD_TREASURE, 0, pMember->getName(), treasureid, dwGUID, randBetween(0, 2)) == true)
    {
      XLOG << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "添加成员奖励" << s << "成功" << XEND;
    }
    else
    {
      XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
        << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "添加成员奖励" << s << "失败" << XEND;
    }
  }

  if (dwGUID != 0)
  {
    TreasureResultNtfGuildSCmd cmd;
    TreasureResult* pResult = cmd.mutable_result();
    pResult->set_eventguid(dwGUID);
    pResult->set_treasureid(treasureid);
    pResult->set_totalmember(m_pGuild->getMisc().getWelfare().getCurTotalMember());
    pResult->set_ownerid(pMember->getCharID());
    addResult(cmd);
  }

  if (m_pGuild->getPack().addItem(vecItems) != ESYSTEMMSG_ID_MIN)
  {
    XERR << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
      << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "失败,添加到公会仓库失败" << XEND;
    return false;
  }

  const SDelayMiscCFG& rDelayCFG = MiscConfig::getMe().getDelayCFG();
  for (auto &v : vecItems)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(v.id());
    if (pCFG != nullptr)
    {
      MsgParams oParams;
      oParams.addString(pCFG->strNameZh);
      oParams.addNumber(v.count());
      m_pGuild->broadcastMsg(4036, oParams, EMESSAGETYPE_FRAME, rDelayCFG.dwGuildTreasure);
    }
  }

  if (setIDs.empty() == false)
    m_pGuild->broadcastMsg(4037, MsgParams(), EMESSAGETYPE_FRAME, rDelayCFG.dwGuildTreasure);
  m_pGuild->updateData(true);
  queryTreasureToScene();
  XLOG << "[公会宝箱-开启]" << m_pGuild->getGUID() << m_pGuild->getName()
    << "成员" << pMember->getAccid() << pMember->getCharID() << pMember->getProfession() << pMember->getName() << "开启城池宝箱" << treasureid << "成功" << XEND;
  return true;
}

bool GTreasure::addResult(const TreasureResultNtfGuildSCmd& cmd)
{
  const TreasureResult& rResult = cmd.result();
  if (rResult.eventguid() == 0)
  {
    XERR << "[公会宝箱-添加结果]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << cmd.ShortDebugString() << "失败,eventguid为0"<< XEND;
    return false;
  }

  auto m = m_mapTreasureResult.find(rResult.eventguid());
  if (m == m_mapTreasureResult.end())
  {
    TreasureResult& rDest = m_mapTreasureResult[rResult.eventguid()];
    rDest.CopyFrom(rResult);
    m = m_mapTreasureResult.find(rResult.eventguid());
    if (m == m_mapTreasureResult.end())
    {
      XERR << "[公会宝箱-添加结果]" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << cmd.ShortDebugString() << "失败,插入列表失败"<< XEND;
      return false;
    }
  }

  for (int i = 0; i < rResult.items_size(); ++i)
  {
    TreasureItem* pItem = m->second.add_items();
    pItem->CopyFrom(rResult.items(i));

    GMember* pMember = m_pGuild->getMember(pItem->charid());
    if (pMember != nullptr)
    {
      pItem->set_name(pMember->getName());
    }
    else
    {
      GCharReader oChar(thisServer->getRegionID(), pItem->charid());
      if (oChar.getNameOnly() == true)
        pItem->set_name(oChar.getName());
    }
  }

  XLOG << "[公会宝箱-添加结果]" << m_pGuild->getGUID() << m_pGuild->getName() << "成功添加" << cmd.ShortDebugString() << XEND;
  return true;
}


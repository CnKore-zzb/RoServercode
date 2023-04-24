#include "PlatLogManager.h"
#include "xTime.h"
#include "CommonConfig.h"

void LogProcesser::sendLog()
{  
  if (m_mapAckCache.size() >= MAX_SEND_CACHE_SIZE)
  {
    XERR << "[fluent-日志] tag:"<< m_strTag << "不能发送，ack缓冲区已满" <<m_mapAckCache.size() << XEND;
   /* return;*/
  }

  string newLog;
  newLog.reserve(CommonConfig::m_dwFluentLogCount * 100);
  DWORD size = 0;
  while (true)
  {
    size = m_listUnsend.size();
    concatLog(newLog);
    if (newLog.empty())
      break;

    DWORD expireTime = now() + EXPIRETIME;
    AckCache & ack = m_mapAckCache[m_qwKey];
    ack.dwExpireTime = expireTime;
    ack.log = newLog;
    ack.dwRetryCount = 0;

    PlatLogManager::getMe().sendLog(newLog);
    XDBG << "[fluent-日志] tag:" <<m_strTag <<"发送前待发送缓冲区大小"<< size <<"发送后大小"<<m_listUnsend.size() <<"ack缓冲区大小" <<m_mapAckCache.size() << XEND;
  } 
}

void LogProcesser::processRetry(DWORD curSec)
{
  if (curSec <= m_dwNextRetryTime)
    return;
  m_dwNextRetryTime = curSec + randBetween(5 * 60, 20 * 60);

  for (auto it = m_mapAckCache.begin(); it != m_mapAckCache.end();)
  {
    if (it->second.dwRetryCount >= CommonConfig::m_dwFluentRetryCount)
    {
      XERR << "[fluent-日志] tag" << m_strTag << "重试超过次数，从ack队列删除" << it->second.log << XEND;
      it = m_mapAckCache.erase(it);
      continue;
    }
    if (it->second.dwExpireTime <= curSec)
    {
      it->second.dwExpireTime = curSec + EXPIRETIME;
      it->second.dwRetryCount++;
      PlatLogManager::getMe().sendLog(it->second.log, true, it->second.dwRetryCount == CommonConfig::m_dwFluentRetryCount);
    }
    ++it;
  }
}

void LogProcesser::recvAck(QWORD ack)
{
  auto it = m_mapAckCache.find(ack);
  if (it == m_mapAckCache.end())
    return;
  m_mapAckCache.erase(ack);
  XDBG <<"[fluent-日志] tag" <<m_strTag <<"收到ack" << ack <<"ack缓冲区大小"<<m_mapAckCache.size() <<"发送缓存区大小"<<m_listUnsend.size() << XEND;
}

void LogProcesser::addLog(const string& log)
{
  if (m_listUnsend.size() >= MAX_UNSEND_SIZE)
  {
    XERR << "[fluent-日志] tag:"<<m_strTag << "待发送缓冲区已满,强制发送一波,发送缓冲区大小" << m_listUnsend.size() << XEND;
    sendLog();
  } 

  m_listUnsend.push_back(log);  
  XDBG << "[fluent-日志] 成功添加到发送缓存区，tag" << m_strTag << "发送缓存区大小" << m_listUnsend.size() <<"ack缓存区大小"<<m_mapAckCache.size() << XEND;
}

void LogProcesser::concatLog(string& log)
{
  log.clear();
  if (m_listUnsend.empty())
    return;
  m_qwKey++;

  std::stringstream log_ss;
  log_ss.str("");
  
  log_ss << "[\"" << m_strTag << "\",[";
  auto it = m_listUnsend.begin();
  DWORD count = 0;
  for (; it != m_listUnsend.end();)
  {
    if(count++ >= CommonConfig::m_dwFluentLogCount)
      break;    
    if (count != 1)
      log_ss << ",";

    log_ss << *it;
    it = m_listUnsend.erase(it);
  }

  log_ss << "],{\"chunk\":\"_"<< m_dwCmd <<"_" <<m_qwKey <<"\"}]";
  log = log_ss.str();  
}

PlatLogManager::PlatLogManager() :m_oFiveSecTimer(5), m_oOneMinTimer(1 * 60)
{
}

void PlatLogManager::init(xServer* pServer)
{
  if (!pServer)
    return;
  m_pServer = pServer;

  if (!m_pServer->isOuter() || m_pServer->getRegionID() == 20001)
  {//内网
    reg(Cmd::LOGIN_LOG_CMD, "xd.test.ro.login");
    reg(Cmd::ACCOUNT_LOG_CMD, "xd.test.ro.account");
    reg(Cmd::CREATE_LOG_CMD, "xd.test.ro.create");
    reg(Cmd::CHANGE_FLAG_LOG_CMD, "xd.test.ro.changeflag");
    reg(Cmd::CHARGE_LOG_CMD, "xd.test.ro.charge");
    reg(Cmd::EVENT_LOG_CMD, "xd.test.ro.event");
    reg(Cmd::INCOME_LOG_CMD, "xd.test.ro.income");
    reg(Cmd::CONSUME_LOG_CMD, "xd.test.ro.consume");
    reg(Cmd::ITEM_LOG_CMD, "xd.test.ro.item");
    reg(Cmd::PROPS_LOG_CMD, "xd.test.ro.props");
    reg(Cmd::TRANSACTION_LOG_CMD, "xd.test.ro.transaction");
    reg(Cmd::CHAT_LOG_CMD, "xd.test.ro.chat");
    reg(Cmd::LEVEL_LOG_CMD, "xd.test.ro.level");
    reg(Cmd::ONLINE_LOG_CMD, "xd.test.ro.online");
    reg(Cmd::CHECKPOINT_LOG_CMD, "xd.test.ro.checkpoint");
    reg(Cmd::RANK_LOG_CMD, "xd.test.ro.rank");
    reg(Cmd::CHANGE_LOG_CMD, "xd.test.ro.change");
    reg(Cmd::EQUIP_LOG_CMD, "xd.test.ro.equip");
    reg(Cmd::CARD_LOG_CMD, "xd.test.ro.card");
    reg(Cmd::EQUIPUP_LOG_CMD, "xd.test.ro.equipup");
    reg(Cmd::SOCIAL_LOG_CMD, "xd.test.ro.social");
    reg(Cmd::QUEST_LOG_CMD, "xd.test.ro.quest");
    reg(Cmd::MANUAL_LOG_CMD, "xd.test.ro.manual");
    reg(Cmd::COMPLETE_LOG_CMD, "xd.test.ro.complete");
    reg(Cmd::TOWER_LOG_CMD, "xd.test.ro.tower");
    reg(Cmd::ITEMOPER_LOG_CMD, "xd.test.ro.itemoper");
    reg(Cmd::KILL_LOG_CMD, "xd.test.ro.kill");
    reg(Cmd::REWARD_LOG_CMD, "xd.test.ro.reward");
    reg(Cmd::MAIL_LOG_CMD, "xd.test.ro.mail");
    reg(Cmd::DOJO_LOG_CMD, "xd.test.ro.dojo");
    reg(Cmd::ENCHANT_LOG_CMD, "xd.test.ro.enchant");
    reg(Cmd::GUILDPRAY_LOG_CMD, "xd.test.ro.guildpray");
    reg(Cmd::USESKILL_LOG_CMD, "xd.test.ro.useskill");
    reg(Cmd::TRADE_LOG_CMD, "xd.test.ro.trade");
    reg(Cmd::DELETE_CHAR_LOG_CMD, "xd.test.ro.deletechar");
    reg(Cmd::COMPOSE_LOG_CMD, "xd.test.ro.compose");
    reg(Cmd::JUMPZONE_LOG_CMD, "xd.test.ro.jumpzone");
    reg(Cmd::TEAM_LOG_CMD, "xd.test.ro.team");
    reg(Cmd::PET_CHANGE_LOG_CMD, "xd.test.ro.petchange");
    reg(Cmd::PET_ADVENTURE_LOG_CMD, "xd.test.ro.petadventure");
    reg(Cmd::TRADE_UNTAKE_LOG_CMD, "xd.test.ro.tradeuntake");
    reg(Cmd::INACTIVE_USER_LOG_CMD, "xd.test.ro.inactiveuser");
    reg(Cmd::CREDIT_LOG_CMD, "xd.test.ro.credit");
    reg(Cmd::TRADE_GIVE_LOG_CMD, "xd.test.ro.tradegive");
    reg(Cmd::QUOTA_LOG_CMD, "xd.test.ro.quota");

    // 公会日志(以公会id(gid)作为索引)
    reg(Cmd::GUILD_ITEM_LOG_CMD, "xd.test.ro.guild_item");
  }
  else
  {
    reg(Cmd::LOGIN_LOG_CMD, "xd.game.ro.login");
    reg(Cmd::ACCOUNT_LOG_CMD, "xd.game.ro.account");
    reg(Cmd::CREATE_LOG_CMD, "xd.game.ro.create");
    reg(Cmd::CHANGE_FLAG_LOG_CMD, "xd.game.ro.changeflag");
    reg(Cmd::CHARGE_LOG_CMD, "xd.game.ro.charge");
    reg(Cmd::EVENT_LOG_CMD, "xd.game.ro.event");
    reg(Cmd::INCOME_LOG_CMD, "xd.game.ro.income");
    reg(Cmd::CONSUME_LOG_CMD, "xd.game.ro.consume");
    reg(Cmd::ITEM_LOG_CMD, "xd.game.ro.item");
    reg(Cmd::PROPS_LOG_CMD, "xd.game.ro.props");
    reg(Cmd::TRANSACTION_LOG_CMD, "xd.game.ro.transaction");
    reg(Cmd::CHAT_LOG_CMD, "xd.game.ro.chat");
    reg(Cmd::LEVEL_LOG_CMD, "xd.game.ro.level");
    reg(Cmd::ONLINE_LOG_CMD, "xd.game.ro.online");
    reg(Cmd::CHECKPOINT_LOG_CMD, "xd.game.ro.checkpoint");
    reg(Cmd::RANK_LOG_CMD, "xd.game.ro.rank");
    reg(Cmd::CHANGE_LOG_CMD, "xd.game.ro.change");
    reg(Cmd::EQUIP_LOG_CMD, "xd.game.ro.equip");
    reg(Cmd::CARD_LOG_CMD, "xd.game.ro.card");
    reg(Cmd::EQUIPUP_LOG_CMD, "xd.game.ro.equipup");
    reg(Cmd::SOCIAL_LOG_CMD, "xd.game.ro.social");
    reg(Cmd::QUEST_LOG_CMD, "xd.game.ro.quest");
    reg(Cmd::MANUAL_LOG_CMD, "xd.game.ro.manual");
    reg(Cmd::COMPLETE_LOG_CMD, "xd.game.ro.complete");
    reg(Cmd::TOWER_LOG_CMD, "xd.game.ro.tower");
    reg(Cmd::ITEMOPER_LOG_CMD, "xd.game.ro.itemoper");
    reg(Cmd::KILL_LOG_CMD, "xd.game.ro.kill");
    reg(Cmd::REWARD_LOG_CMD, "xd.game.ro.reward");
    reg(Cmd::MAIL_LOG_CMD, "xd.game.ro.mail");
    reg(Cmd::DOJO_LOG_CMD, "xd.game.ro.dojo");
    reg(Cmd::ENCHANT_LOG_CMD, "xd.game.ro.enchant");
    reg(Cmd::GUILDPRAY_LOG_CMD, "xd.game.ro.guildpray");
    reg(Cmd::USESKILL_LOG_CMD, "xd.game.ro.useskill");
    reg(Cmd::TRADE_LOG_CMD, "xd.game.ro.trade");
    reg(Cmd::DELETE_CHAR_LOG_CMD, "xd.game.ro.deletechar");
    reg(Cmd::COMPOSE_LOG_CMD, "xd.game.ro.compose");
    reg(Cmd::JUMPZONE_LOG_CMD, "xd.game.ro.jumpzone");
    reg(Cmd::TEAM_LOG_CMD, "xd.game.ro.team");
    reg(Cmd::PET_CHANGE_LOG_CMD, "xd.game.ro.petchange");
    reg(Cmd::PET_ADVENTURE_LOG_CMD, "xd.game.ro.petadventure");
    reg(Cmd::TRADE_UNTAKE_LOG_CMD, "xd.game.ro.tradeuntake");
    reg(Cmd::INACTIVE_USER_LOG_CMD, "xd.game.ro.inactiveuser");
    reg(Cmd::CREDIT_LOG_CMD, "xd.game.ro.credit");
    reg(Cmd::TRADE_GIVE_LOG_CMD, "xd.game.ro.tradegive");
    reg(Cmd::QUOTA_LOG_CMD, "xd.game.ro.quota");

    // 公会日志(以公会id(gid)作为索引)
    reg(Cmd::GUILD_ITEM_LOG_CMD, "xd.game.ro.guild_item");
  }
}


void PlatLogManager::loginLog(xServer* pServer, DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  const std::string& ip,
  DWORD dwCharge,
  int logType,
  DWORD dwRoleLv,
  const std::string& strSign, const std::string& strDevice, int isGuest, const std::string& strMac, const std::string& strAgent,
  DWORD dwMapId, DWORD dwOnlineTime,
  DWORD dwTeamTimeLen, bool isNew)
{
  if (pServer == nullptr)
    return;

  Cmd::LoginLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_ip("");
  log.set_type(logType);    //0：logout 1:login
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  log.set_level(dwRoleLv);
  log.set_vip(0);

  log.set_sign(strSign);
  log.set_device(strDevice);
  log.set_guest(isGuest);
  log.set_mac(strMac);
  log.set_agent(strAgent);
  log.set_mapid(dwMapId);
  log.set_onlinetime(dwOnlineTime);
  log.set_teamtimelen(dwTeamTimeLen);
  log.set_isnew(isNew);
  log.set_zoneid(pServer->getZoneID());
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::createCharLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  const std::string& strName,
  int isGuest,
  const std::string& strIp,
  const std::string& strDevice,
  const std::string& strMac,
  const std::string& strAgent,
  DWORD dwGender,
  DWORD dwHair,
  DWORD dwHairColor)
{
  if (pServer == nullptr)
    return;

  Cmd::CreateLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_ip(strIp);
  log.set_name(strName);
  log.set_device(strDevice);
  log.set_guest(isGuest);
  log.set_mac(strMac);
  log.set_agent(strAgent);
  log.set_gender(dwGender);
  log.set_hair(dwHair);
  log.set_haircolor(dwHairColor);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::deleteCharLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId)
{
  if (pServer == nullptr)
    return;

  Cmd::DeleteCharLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(now());
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

//template<typename T>
//void PlatLogManager::changeFlagLog(xServer* pServer,
//  DWORD dwPlatFormId,
//  DWORD dwZoneId,
//  QWORD dwCharId,
//  const std::string& flag, T from, T to)
//{
//  if (pServer == nullptr)
//    return;
//
//  std::stringstream ss1;
//  ss1 << from;
//  std::stringstream ss2;
//  ss2 << to;
//  changeFlagLog(pServer, dwPlatFormId, dwZoneId, dwCharId, flag, ss1.str(), ss2.str());
//}
//
template<>
void PlatLogManager::changeFlagLog<std::string>(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwCharId,
  const std::string& flag, const std::string from, const std::string to,
  QWORD qwParam1)
{
  if (pServer == nullptr)
    return;

  Cmd::ChangeFlagLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_falg(flag);
  log.set_from(from);
  log.set_to(to);
  log.set_param1(qwParam1);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::chargeLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  DWORD dwRoleLv,
  float dwValue,
  const std::string& strName,
  DWORD dwCreateTime,
  DWORD dwItemId,
  DWORD dwCoins,
  const std::string strOrderId,
  const std::string strType,
  const std::string strCurrency,
  const std::string strProvider,
  const std::string strIp,
  const std::string strDevice)
{
  if (pServer == nullptr)
    return;

  Cmd::ChargeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_ip(strIp);
  log.set_oid(strOrderId); //订单id
  log.set_type(strType); //充值渠道类型 /web/ios/android
  log.set_level(dwRoleLv);
  log.set_amount(dwValue);
  log.set_coins(dwCoins); // 兑换的游戏币数量
  log.set_name(strName);
  log.set_device(strDevice);   //TODO
  log.set_ctime(dwCreateTime);
  log.set_currency(strCurrency);    //TODO
  log.set_provider(strProvider); //TODO
  log.set_itemid(dwItemId);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}


void PlatLogManager::eventLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD dwType,
  DWORD dwSubType,
  DWORD dwCount)
{
  return;

  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::EventLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(curSec);
  log.set_microtime(fMSec); //毫秒
  log.set_eid(qwEid);
  int isPay = (dwCharge) ? 1 : 0;
  log.set_ispay(isPay);
  log.set_type(dwType);
  log.set_subtype(dwSubType);
  log.set_count(dwCount);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::eventLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  string strAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD dwType,
  DWORD dwSubType,
  DWORD dwCount)
{
  return;
  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::EventLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  log.set_account(strAccid);
  log.set_pid(dwCharId);
  log.set_time(curSec);
  log.set_microtime(fMSec); //毫秒
  log.set_eid(qwEid);
  int isPay = (dwCharge) ? 1 : 0;
  log.set_ispay(isPay);
  log.set_type(dwType);
  log.set_subtype(dwSubType);
  log.set_count(dwCount);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}


void PlatLogManager::incomeMoneyLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD coinType,
  QWORD dwValue,
  QWORD dwAfter,
  DWORD dwSource)
{
  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::IncomeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(curSec);
  log.set_microtime(fMSec); //毫秒
  log.set_eid(qwEid);
  int isPay = (dwCharge) ? 1 : 0;
  log.set_ispay(isPay);
  log.set_value(dwValue);
  log.set_coin_type(coinType);
  DWORD eventType = EventTypeConvert(dwSource);
  log.set_type(eventType);
  log.set_after(dwAfter);
  log.set_source(dwSource);
  log.set_count(1);

  std::string key = generateKey(log.cid(), log.pid(), log.coin_type(), log.source());
  auto it = m_mapIncomeLog.find(key);
  if (it == m_mapIncomeLog.end())
  {
    m_mapIncomeLog.insert(std::make_pair(key, log));
  }
  else
  {
    it->second.set_value(it->second.value() + log.value());
    it->second.set_after(log.after());
    it->second.set_count(it->second.count() + log.count());
  }
  XDBG << "[后台-日志] 队列：param:" << log.param() << "msg:" << log.ShortDebugString() << XEND;
}

void PlatLogManager::outcomeMoneyLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD coinType,
  QWORD dwValue,
  QWORD dwAfter,
  DWORD dwSource,
  QWORD qwChargeMoney,
  QWORD qwLeftChargeMoney)
{
  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::ConsumeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_time(curSec);
  log.set_microtime(fMSec); //毫秒
  log.set_eid(qwEid); //TODO
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  log.set_value(dwValue);
  log.set_coin_type(coinType);
  DWORD eventType = EventTypeConvert(dwSource);
  log.set_type(eventType);
  log.set_after(dwAfter);
  log.set_kind(1);      //消耗类型 不用
  log.set_source(dwSource);
  log.set_count(1);
  log.set_chargecount(qwChargeMoney);
  log.set_remaincharge(qwLeftChargeMoney);

  if (qwChargeMoney > 0)
  {
    LOGID(log, pServer->getZoneID());
    LOGBUFF(log);
  }
  else
  {
    std::string key = generateKey(log.cid(), log.pid(), log.coin_type(), log.source());
    auto it = m_mapConsumeLog.find(key);
    if (it == m_mapConsumeLog.end())
    {
      m_mapConsumeLog.insert(std::make_pair(key, log));
    }
    else
    {
      it->second.set_value(it->second.value() + log.value());
      it->second.set_after(log.after());
      it->second.set_count(it->second.count() + log.count());
    }
    XDBG << "[后台-日志] 队列：param:" << log.param() << "msg:" << log.ShortDebugString() << XEND;
  }
}


void PlatLogManager::gainItemLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD dwItemId,
  DWORD dwValue,
  DWORD dwAfter,
  DWORD dwSource,
  DWORD eventType /*=0*/)
{
  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::ItemLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);
  std::stringstream ss;
  ss.str("");
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_pid(dwCharId);
  log.set_eid(qwEid);
  log.set_time(curSec);
  log.set_microtime(fMSec);
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  
  if (eventType == 0)
    eventType = EventTypeConvert(dwSource);
  log.set_type(eventType); 
  log.set_itemid(dwItemId);
  log.set_value(dwValue);
  log.set_from_pid(0); //TODO 来源玩家
  log.set_after(dwAfter);
  log.set_amount(0);  //花费一级游戏币
  log.set_amount2(0); //花费二级游戏币
  log.set_source(dwSource);
  log.set_count(1);
    
  std::string key = generateKey(log.cid(), log.pid(), log.itemid(), log.source());
  auto it = m_mapItemLog.find(key);
  if (it == m_mapItemLog.end())
  {
    m_mapItemLog.insert(std::make_pair(key, log));
  }
  else
  {
    it->second.set_value(it->second.value() + log.value());
    it->second.set_after(log.after());
    it->second.set_count(it->second.count() + log.count());
  }
  XDBG << "[后台-日志] 队列：param:" << log.param() << "msg:" << log.ShortDebugString() << XEND;
}

void PlatLogManager::consumeItemLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwCharge,
  DWORD dwItemId,
  DWORD dwValue,
  DWORD dwAfter,
  DWORD dwSource,
  DWORD eventType /*=0*/,
  const std::string& iteminfo
  )
{
  if (pServer == nullptr)
    return;

  QWORD curMSec = xTime::getCurMSec();
  double fMSec = ((double)curMSec) / 1000.000;
  DWORD curSec = curMSec / ONE_THOUSAND;

  Cmd::PropsLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);
  log.set_pid(dwCharId);
  std::stringstream ss;
  ss.str("");
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_eid(qwEid); //TODo
  log.set_time(curSec);
  log.set_microtime(fMSec);
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  log.set_itemid(dwItemId);
  log.set_value(dwValue);
  if (eventType == 0)
    eventType = EventTypeConvert(dwSource);
  log.set_type(eventType);
  log.set_after(dwAfter);
  log.set_source(dwSource);
  log.set_count(1);
  log.set_iteminfo(iteminfo);

  if (!iteminfo.empty())
  {//直接推送
    LOGID(log, pServer->getZoneID());
    LOGBUFF(log);
    return;
  }

  std::string key = generateKey(log.cid(), log.pid(), log.itemid(), log.source());
  auto it = m_mapPropsLog.find(key);
  if (it == m_mapPropsLog.end())
  {
    m_mapPropsLog.insert(std::make_pair(key, log));
  }
  else
  {
    it->second.set_value(it->second.value() + log.value());
    it->second.set_after(log.after());
    it->second.set_count(it->second.count() + log.count());
  }
  XDBG << "[fluent-日志] 队列：param:" << log.param() << "msg:" << log.ShortDebugString() << XEND;
}

void PlatLogManager::chatLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  const std::string strSenderName,
  QWORD dwCharId,
  QWORD qwRecvAccid,
  const std::string strRecvName,
  QWORD qwRecvCharid,
  DWORD type,
  DWORD dwCharge,
  const std::string strContent,
  DWORD dwRoleLv,
  DWORD voiceTime/*=0*/)
{
  if (pServer == nullptr)
    return;

  Cmd::ChatLogCmd log;

  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_time(now());
  log.set_eid(xTime::getCurMSec() * 1000);
  std::stringstream ss;
  ss << dwAccid;
  log.set_from_account(ss.str());
  log.set_from_name(strSenderName);
  log.set_from_pid(dwCharId);

  if (qwRecvAccid)
  {
    std::stringstream ss2;
    ss2.str("");
    ss2 << qwRecvAccid;
    log.set_to_account(ss2.str());
  }
  else
  {
    log.set_to_account("");
  }

  log.set_to_name(strRecvName);
  log.set_to_pid(qwRecvCharid);
  log.set_type(type);         //0私聊
  int isPay = dwCharge > 0 ? 1 : 0;
  log.set_ispay(isPay);
  log.set_content(strContent);
  log.set_vip(0);
  log.set_level(dwRoleLv);

  if (voiceTime != 0)
  {
    log.set_chattype(ECHATTYPE_VOICE);
    log.set_voicelen(voiceTime);
  }
  else
  {
    log.set_chattype(ECHATTYPE_STR);
  }
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);   
}


void PlatLogManager::levelUpLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  DWORD dwFromLv,
  DWORD dwToLv,
  DWORD dwCharge,
  ELEVEL_TYPE type,
  DWORD dwCostTime /*=0*/)
{
  if (pServer == nullptr)
    return;

  Cmd::LevelLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  std::stringstream ss;
  ss << dwAccid;
  log.set_account(ss.str());
  log.set_time(now());
  log.set_from(dwFromLv);
  log.set_to(dwToLv);
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  log.set_type(type);
  LOGID(log, pServer->getZoneID());
  log.set_costtime(dwCostTime);
  LOGBUFF(log);
   
}

void PlatLogManager::onlineCountLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  DWORD dwMinTime,
  DWORD dwTotalCount,
  DWORD dwAndroidCount,
  DWORD dwClientCount,
  DWORD dwIosCount)
{
  if (pServer == nullptr)
    return;

  Cmd::OnlineLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());  /*区id*/
  pServer->getZoneID();
  log.set_time(dwMinTime);  /*时间秒，按分钟取整*/
  log.set_count_all(dwTotalCount);
  log.set_count_android(dwAndroidCount);
  log.set_count_client(dwClientCount);    //TODO
  log.set_count_ios(dwIosCount);
  log.set_lineid(getClientZoneID(pServer->getZoneID()));
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::levelPassLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwCharId,
  QWORD qwEid,
  DWORD dwMapId,
  DWORD dwCharge,
  DWORD dwIsFirst)
{
  if (pServer == nullptr)
    return;

  Cmd::CheckpointLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  log.set_eid(qwEid);
  log.set_time(now());
  log.set_type(1);    //关卡类型，目前没有
  log.set_cpid(dwMapId); //关卡ID
  log.set_result(1);  //失败不打印
  log.set_star(0);
  int isPay = ((dwCharge > 0) ? 1 : 0);
  log.set_ispay(isPay);
  log.set_vip(0);
  log.set_isfirst(dwIsFirst); //是否是首次，目前没记录
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);   
}

template<>
void PlatLogManager::changeLog<std::string>(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  ECHANGE_TYPE flag, const std::string from, const std::string to,
  QWORD qwParam1)
{
  if (pServer == nullptr)
    return;

  Cmd::ChangeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_etype(eType);
  log.set_eid(qwEid);
  log.set_time(now());
  log.set_flag(flag);
  log.set_from(from);
  log.set_to(to);
  log.set_param1(qwParam1);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::CardLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwEquipId,
  const std::string& equipGid,
  DWORD type,   //1:on 2：off
  DWORD dwCardId,
  const std::string& cardGid,
  DWORD dwUseslot,
  DWORD dwMaxSlot)
{

  if (pServer == nullptr)
    return;

  Cmd::CardLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       
  
  log.set_equipid(dwEquipId);
  log.set_equipgid(equipGid);
  log.set_type(type);
  log.set_cardid(dwCardId);
  log.set_cardgid(cardGid);
  log.set_useslot(dwUseslot);
  log.set_maxslot(dwMaxSlot);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);   
}

/*装备日志*/
void PlatLogManager::EquipLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD type,           //1：穿上 2：脱下  3:transfer
  DWORD dwOldEquipId,
  const std::string& oldEquipGuid,
  DWORD dwOldStrengthLv,
  DWORD dwOldRefineLv,
  bool bOldIsDamanged,
  DWORD dwNewEquipId,
  const std::string& newEquipGuid,
  DWORD dwNewStrengthLv,
  DWORD dwNewRefineLv,
  bool bNewIsDamanged)
{
  if (pServer == nullptr)
    return;

  Cmd::EquipLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       

  log.set_type(type);
  log.set_old_equipid(dwOldEquipId);
  log.set_old_equipgid(oldEquipGuid);
  log.set_old_strengthlv(dwOldStrengthLv);
  log.set_old_refinelv(dwOldRefineLv);
  log.set_old_isdamage(bOldIsDamanged);
  
  log.set_new_equipid(dwNewEquipId);
  log.set_new_equipgid(newEquipGuid);
  log.set_new_strengthlv(dwNewStrengthLv);
  log.set_new_refinelv(dwNewRefineLv);
  log.set_new_isdamage(bNewIsDamanged);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::EquipUpLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD type,           //1：强化 2：精炼 
  DWORD count,
  QWORD qwEquipId,
  const std::string& equipGid,
  DWORD dwOldLv,
  DWORD dwNewLv,
  bool bIsFail,
  std::string costMoney,
  std::string costItem,
  bool bIsDamanged)
{
  if (pServer == nullptr)
    return;

  Cmd::EquipUpLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       

  log.set_type(type);
  log.set_equipid(qwEquipId);
  log.set_equipguid(equipGid);
  log.set_count(count);
  log.set_old_lv(dwOldLv);
  log.set_new_lv(dwNewLv);
  log.set_isfail(bIsFail);
  log.set_cost_money(costMoney);
  log.set_cost_item(costItem);
  log.set_isdamage(bIsDamanged);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

void PlatLogManager::SocialLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  ESOCIAL_TYPE type,
  QWORD qwInId,
  QWORD qwAnotherId,
  QWORD qwParam1,
  QWORD qwParam2)
{
  if (pServer == nullptr)
    return;

  Cmd::SocailLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       

  log.set_type(type);
  log.set_inid(qwInId);
  log.set_otherid(qwAnotherId);
  log.set_param1(qwParam1);
  log.set_param2(qwParam2);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*任务日志*/
void PlatLogManager::QuestLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  QWORD qwQuestId,
  EQUEST_TYPE type,
  QWORD qwTargetId,
  DWORD dwBaseExp,
  DWORD dwJobExp,
  std::string& rewardItem,
  DWORD dwLevel)
{
  if (pServer == nullptr)
    return;

  Cmd::QuestLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);      

  log.set_questid(qwQuestId);
  log.set_type(type);
  log.set_targetid(qwTargetId);
  log.set_baseexp(dwBaseExp);
  log.set_jobexp(dwJobExp);
  log.set_rewarditem(rewardItem);
  log.set_level(dwLevel);
  log.set_lineid(getClientZoneID(pServer->getZoneID()));
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*冒险手册日志*/
void PlatLogManager::ManualLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  EMANUAL_TYPE type,
  DWORD by,
  QWORD qwWhat,
  QWORD qwParam1)
{
  if (pServer == nullptr)
    return;

  Cmd::ManualLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);      

  log.set_type(type);
  log.set_by(by);
  log.set_what(qwWhat);
  log.set_param1(qwParam1);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*完成活动相关日志*/
void PlatLogManager::CompleteLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  ECOMPLETE_TYPE type,
  QWORD qwTargetId,
  DWORD dwTodayCount,
  DWORD dwRewardType,
  DWORD dwRewardCount,
  DWORD dwLevel
)
{
  Cmd::CompleteLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       
  LOGID(log, pServer->getZoneID());
  log.set_type(type);
  log.set_targetid(qwTargetId);
  log.set_today_count(dwTodayCount);
  log.set_reward_type(dwRewardType);
  log.set_reward_count(dwRewardCount);
  log.set_level(dwLevel);
  LOGBUFF(log);
   

}

/*完成活动相关日志*/
void PlatLogManager::TowerLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD type,         //1：完成  2：离开
  DWORD dwCurLayer,
  DWORD dwMaxLayer,
  QWORD qwTeamId,
  DWORD dwLevel
)
{
  Cmd::TowerLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       //todo

  log.set_type(type);
  log.set_curlayer(dwCurLayer);
  log.set_maxlayer(dwMaxLayer);
  log.set_teamid(qwTeamId);
  log.set_level(dwLevel);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*物品操作相关日志*/
void PlatLogManager::ItemOperLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  EITEMOPER_TYPE type,
  DWORD dwItemid,
  DWORD dwCount
)
{
  Cmd::ItemOperLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       

  log.set_type(type);
  log.set_itemid(dwItemid);
  log.set_count(dwCount);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*杀怪*/
void PlatLogManager::KillLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwMonsterId,
  QWORD qwMonsterGid,
  DWORD dwMonsterGroup,
  DWORD dwBaseExp,
  DWORD dwJobExp,
  bool isMvp,
  EMONSTER_TYPE type,
  DWORD dwLv,
  DWORD killType
)
{
  //普通怪物的日志暂时不推送
  if (type == EMONSTERTYPE_MONSTER)
    return;

  Cmd::KillLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);       

  log.set_monsterid(dwMonsterId);
  log.set_monstergid(qwMonsterGid);
  log.set_monstergroup(dwMonsterGroup);
  log.set_baseexp(dwBaseExp);
  log.set_jobexp(dwJobExp);
  log.set_ismvp(isMvp);
  log.set_type(type);
  log.set_level(dwLv);
  log.set_killtype(killType);
  log.set_lineid(getClientZoneID(pServer->getZoneID()));
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

/*reward*/
void PlatLogManager::RewardLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwId,
  DWORD dwProfession,
  const string& rewarditem
)
{
  Cmd::RewardLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_id(dwId);
  log.set_profession(dwProfession);
  log.set_rewarditem(rewarditem);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*mail*/
void PlatLogManager::MailLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  QWORD qwId,
  QWORD qwSysid,
  DWORD dwMailType,
  const string& title,
  const string& rewarditem)
{
  Cmd::MailLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_id(qwId);
  log.set_sysid(qwSysid);
  log.set_mailtype(dwMailType);
  log.set_title(title);
  log.set_rewarditem(rewarditem);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*通关道场*/
void PlatLogManager::DojoLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dojoId,
  DWORD dwMapId,
  EDOJOPASS_TYPE passType,
  DWORD dwLevel
)
{
  Cmd::DojoLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_dojoid(dojoId);
  log.set_mapid(dwMapId);
  log.set_passtype(passType);
  log.set_level(dwLevel);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*附魔*/
void PlatLogManager::EnchantLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  const string& equipGuid,
  DWORD dwItemId,
  DWORD enchantType,
  const string& oldAttr,
  const string& newAttr,
  const string& oldBuffid,
  const string& newBuffid,
  DWORD dwCostItemId,
  DWORD dwItemCount,
  DWORD dwMoney
)
{
  Cmd::EnchantLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_equipguid(equipGuid);
  log.set_itemid(dwItemId);
  log.set_enchanttype(enchantType);
  log.set_oldattr(oldAttr);
  log.set_newattr(newAttr);
  log.set_oldbufid(oldBuffid);
  log.set_newbufid(newBuffid);
  log.set_costitemid(dwCostItemId);
  log.set_costitemcount(dwItemCount);
  log.set_costmoney(dwMoney);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*公会祈祷*/
void PlatLogManager::GuildPrayLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwPrayId,
  DWORD dwAddattr,
  DWORD dwCostItem,
  DWORD dwCostMoney,
  DWORD dwCostCon
)
{
  Cmd::GuildPrayLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_prayid(dwPrayId);
  log.set_addattr(dwAddattr);
  log.set_costitem(dwCostItem);
  log.set_costmoney(dwCostMoney);
  log.set_costcon(dwCostCon);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
   
}

/*使用技能*/
void PlatLogManager::UseSkillLog(xServer* pServer,
  DWORD dwPlatFormId,
  DWORD dwZoneId,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwSkillID
)
{
  Cmd::UseSkillLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_skillid(dwSkillID);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

/*交易所*/
void PlatLogManager::TradeLog(xServer* pServer,
  DWORD dwPlatFormId,
  QWORD dwCharId,
  ETRADE_TYPE type,
  DWORD dwItemId,
  DWORD dwCount,
  DWORD dwPrice,
  DWORD dwTax,
  DWORD moneyCount,
  string& itemInfo,
  QWORD otherId /*=0*/
)
{
  Cmd::TradeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  log.set_time(now());

  log.set_type(type);
  log.set_itemid(dwItemId);
  log.set_count(dwCount);
  log.set_price(dwPrice);
  log.set_tax(dwTax);
  log.set_moneycount(moneyCount);
  log.set_iteminfo(itemInfo);
  log.set_otherid(otherId);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);   
}

void PlatLogManager::ComposeLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwItemid,
  std::string itemGuid,
  std::string cost
)
{
  Cmd::ComposeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);
  log.set_itemid(dwItemid);
  log.set_itemguid(itemGuid);
  log.set_cost(cost);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::JumpzoneLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  DWORD dwOldZoneid,
  DWORD dwNewZoneid,
  DWORD dwIsFirst,
  std::string cost
)
{
  Cmd::JumpzoneLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);
  log.set_oldzoneid(dwOldZoneid);
  log.set_newzoneid(dwNewZoneid);
  log.set_isfirst(dwIsFirst);
  log.set_cost(cost);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::TeamLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  QWORD qwInId,
  QWORD qwAnotherId)
{
  if (pServer == nullptr)
    return;

  Cmd::TeamLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_hid(0);  //合服区id
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);
  log.set_inid(qwInId);
  log.set_otherid(qwAnotherId);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

std::string PlatLogManager::generateKey(DWORD platform, QWORD charId, DWORD itemid, DWORD source)
{
  stringstream ss;
  ss << "platform:" << platform << "charid:" << charId << "itemid:" << itemid << "source:" << source;

  return ss.str();
}


void PlatLogManager::processItem()
{
  if (m_pServer == nullptr)
    return;

  for (auto & v : m_mapIncomeLog)
  {
    //event log
    eventLog(m_pServer,
      v.second.cid(),
      v.second.sid(),
      v.second.account(),
      v.second.pid(),
      v.second.eid(),
      v.second.ispay(),
      v.second.type(), 0, 1);
    {
      LOGID(v.second, m_pServer->getZoneID());
      LOGBUFF(v.second);
    }
  }
  if (!m_mapIncomeLog.empty())
    XLOG << "[后台-日志] 队列 一共发送，m_mapIncomeLog size:" << m_mapIncomeLog.size() << XEND;
  m_mapIncomeLog.clear();

  for (auto & v : m_mapConsumeLog)
  {
    //event log
    eventLog(m_pServer,
      v.second.cid(),
      v.second.sid(),
      v.second.account(),
      v.second.pid(),
      v.second.eid(),
      v.second.ispay(),
      v.second.type(), 0, 1);
    {
      LOGID(v.second, m_pServer->getZoneID());
      LOGBUFF(v.second);
    }
  }
  if (!m_mapConsumeLog.empty())
    XLOG << "[后台-日志] 队列 一共发送，m_mapConsumeLog size:" << m_mapConsumeLog.size() << XEND;
  m_mapConsumeLog.clear();

  for (auto & v : m_mapItemLog)
  {
    //event log
    eventLog(m_pServer,
      v.second.cid(),
      v.second.sid(),
      v.second.account(),
      v.second.pid(),
      v.second.eid(),
      v.second.ispay(),
      v.second.type(), 0, 1);
    {
      LOGID(v.second, m_pServer->getZoneID());
      LOGBUFF(v.second);
    }
  }
  if (!m_mapItemLog.empty())
    XLOG << "[后台-日志] 队列 一共发送，m_mapItemLog size:" << m_mapItemLog.size() << XEND;
  m_mapItemLog.clear();

  for (auto & v : m_mapPropsLog)
  {
    //event log
    eventLog(m_pServer,
      v.second.cid(),
      v.second.sid(),
      v.second.account(),
      v.second.pid(),
      v.second.eid(),
      v.second.ispay(),
      v.second.type(), 0, 1);
    {
      LOGID(v.second, m_pServer->getZoneID());
      LOGBUFF(v.second);
    }
  }
  if (!m_mapPropsLog.empty())
    XLOG << "[后台-日志] 队列 一共发送，m_mapPropsLog size:" << m_mapPropsLog.size() << XEND;
  m_mapPropsLog.clear();
}

void PlatLogManager::timeTick(DWORD curSec)
{
  FUN_TIMECHECK(30 * 1000);

  if (m_oFiveSecTimer.timeUp(curSec))
  {
    processItem();

    if (m_oOneMinTimer.timeUp(curSec))
    {
      for (auto&m : m_mapLogProcesser)
      {
        if (m.second)
          m.second->processRetry(curSec);
      }
    }
  }
  
  for (auto& m : m_mapLogProcesser)
  {
    if (m.second)
      m.second->sendLog();
  }
}

void PlatLogManager::PetChangeLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,
  EPetChangeType eChangeType,
  DWORD dwMonsterId,
  string& strName,
  DWORD dwBefore,
  DWORD dwAfter,
  string& strBefore,
  string& strAfter)
{
  if (pServer == nullptr)
    return;

  Cmd::PetChangeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_type(eChangeType);
  log.set_monsterid(dwMonsterId);
  log.set_name(strName);
  log.set_before(dwBefore);
  log.set_after(dwAfter);
  log.set_skill_before(strBefore);
  log.set_skill_after(strAfter);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::PetAdventureLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  EVENT_TYPE eType,
  QWORD qwEid,

  EPetAdventureLogType eAdventureType,
  DWORD dwId,
  string strNames,
  DWORD dwCond)
{
  if (pServer == nullptr)
    return;

  Cmd::PetAdventureLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  log.set_eid(qwEid);
  log.set_etype(eType);

  log.set_type(eAdventureType);
  log.set_id(dwId);
  log.set_names(strNames);
  log.set_cond(dwCond);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::InactiveUserLog(xServer* pServer,
  QWORD dwAccid,
  QWORD dwCharId,
  const string& name,
  DWORD dwJob,
  DWORD dwLevel,
  QWORD qwLeftZeny,
  DWORD dwMapid,
  QWORD qwGuildId,
  DWORD dwCreateTime,
  DWORD dwSendCount
)
{
  if (pServer == nullptr)
    return;

  Cmd::InactiveUserLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << dwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(dwCharId);
  log.set_time(now());
  
  log.set_name(name);
  log.set_job(dwJob);
  log.set_level(dwLevel);
  log.set_left_zeny(qwLeftZeny);
  log.set_mapid(dwMapid);
  log.set_create_time(dwCreateTime);
  log.set_send_count(dwSendCount);
  log.set_guildid(qwGuildId);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}
void PlatLogManager::TradeUntakeLog(xServer* pServer,
  DWORD dwPlatFormId,
  QWORD dwCharId,
  const std::string& strName,
  QWORD qwZeny,
  const std::string& strGuildName
)
{
  Cmd::TradeUntakeLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  log.set_time(now());

  log.set_name(strName);
  log.set_zeny(qwZeny);
  log.set_guildname(strGuildName);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::CreditLog(xServer* pServer,
  QWORD qwCharId,
  const std::string& strName,
  Cmd::ECreditType type,
  QWORD qwBefore,
  QWORD qwAfter
)
{
  Cmd::CreditLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(qwCharId);
  log.set_time(now());
  log.set_name(strName);
  log.set_type(type);
  log.set_before(qwBefore);
  log.set_after(qwAfter);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}
void PlatLogManager::TradeGiveLog(xServer* pServer,
                                  QWORD dwCharId,
                                  Cmd::EGiveEvent event,
                                  DWORD dwItemId,
                                  QWORD qwQuota,
                                  const std::string& itemInfo,
                                  QWORD qwOtherCharId,
                                  const std::string& strName,
                                  const std::string& strOtherName,
                                  DWORD givetime,
                                  Cmd::ELogGiveType type,
                                  DWORD dwItemCount
)
{
  Cmd::TradeGiveLogCmd log;
  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  log.set_pid(dwCharId);
  log.set_time(now());

  log.set_event(event);
  log.set_itemid(dwItemId);
  log.set_quota(qwQuota);
  log.set_iteminfo(itemInfo);
  log.set_otherid(qwOtherCharId);
  log.set_name(strName);
  log.set_othername(strOtherName);
  log.set_givetime(givetime);
  log.set_givetype(type);
  log.set_itemcount(dwItemCount);
  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
}

void PlatLogManager::QuotaLog(xServer* pServer,
                              QWORD qwAccid,
                              QWORD qwCharId,
                              EQuotaOptType eOptType,
                              EQuotaType eQuotaType,
                              QWORD qwChanged,
                              QWORD qwQuota,
                              QWORD qwLock)
{
  if (pServer == nullptr)
    return;

  Cmd::QuotaLogCmd log;

  log.set_cid(pServer->getPlatformID()); /*平台id*/
  log.set_sid(pServer->getRegionID());
  std::stringstream ss;
  std::string strAccoundId;
  ss << qwAccid;
  ss >> strAccoundId;
  log.set_account(strAccoundId);
  log.set_pid(qwCharId);
  log.set_time(now());

  log.set_opttype(eOptType);
  log.set_quotatype(eQuotaType);
  log.set_changed(qwChanged);
  log.set_quota(qwQuota);
  log.set_lock(qwLock);

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);
  XDBG << "[后台-额度变化日志]" << log.ShortDebugString() << XEND;
}

void PlatLogManager::GuildItemLog(xServer* pServer,
                                  QWORD qwGuildId,
                                  DWORD dwItemId,
                                  SQWORD sqwChanged,
                                  DWORD dwCount,
                                  DWORD dwSource)
{
  if (pServer == nullptr)
    return;

  Cmd::GuildItemLogCmd log;
  log.set_cid(pServer->getPlatformID());
  log.set_sid(pServer->getRegionID());
  log.set_gid(qwGuildId);
  log.set_time(now());

  log.set_itemid(dwItemId);
  log.set_changed(sqwChanged);
  log.set_count(dwCount);
  log.set_type(EventTypeConvert(dwSource));

  LOGID(log, pServer->getZoneID());
  LOGBUFF(log);

  XDBG << "[后台-日志] 队列：param:" << log.param() << "msg:" << log.ShortDebugString() << XEND;
}

void PlatLogManager::reg(Cmd::LogParam cmd, const string& tag)
{
  auto it = m_mapLogProcesser.find(cmd);
  if (it != m_mapLogProcesser.end())
    return;
  
  LogProcesser* pProcesser = new LogProcesser;
  if (!pProcesser)
    return;
  pProcesser->setTag(cmd, tag); 
  m_mapLogProcesser[cmd] = pProcesser;
  XLOG << "[fluent-日志] 注册 cmd" <<cmd <<"tag"<<tag << XEND;
}

void PlatLogManager::addLog(Cmd::LogParam cmd, const string&log)
{
  LogProcesser* p = getLogProcesser(cmd);
  if (!p)
  {
    XERR << "[fluent-日志] 未注册, cmd" <<cmd << XEND;
    return;
  }  
  p->addLog(log);
}

bool PlatLogManager::sendLog(const string& log, bool retry/*=false*/, bool bLog/*=false*/)
{
  if (!m_pServer) return false;
  bool ret = m_pServer->sendCmdToLog(log.c_str(), log.length());
  XDBG << "[fluent-日志] 发送日志 ret" <<ret <<"是否是超时重试" << retry <<"日志长度" <<log.length() << log << XEND;

  if (bLog)
    XLOG << "[fluent-日志] 重试发送日志 ret" << ret << "日志" << log << XEND;
    
  return ret;
}

// {"ack":"_1_8"}
bool PlatLogManager::doCmd(BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;
  string str = string((const char*)buf,(size_t)len);  
  TVecQWORD vecParam;
  numTok(str, "_", vecParam);
  if (vecParam.size() != 3)
  {
    XERR << "[fluent-日志] fluent返回消息异常" << str << XEND;
    return false;
  }

  DWORD cmd = vecParam[1];
  QWORD key = vecParam[2];
  XDBG << "[fluent-日志] 处理fluent返回消息" << str <<"cmd"<<cmd <<"key"<<key << XEND;

  LogProcesser* pProcesser = getLogProcesser(cmd);
  if (!pProcesser)
    return false;
  
  pProcesser->recvAck(key);
  return true;
}

LogProcesser* PlatLogManager::getLogProcesser(DWORD cmd)
{
  auto it = m_mapLogProcesser.find(cmd);
  if (it == m_mapLogProcesser.end())
    return nullptr;
  return it->second;
}

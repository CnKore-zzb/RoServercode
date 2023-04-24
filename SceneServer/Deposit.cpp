#include "Deposit.h"
#include "SceneUser.h"
#include "UserEvent.pb.h"
#include "MiscConfig.h"
#include "RedisManager.h"
#include "MsgManager.h"
#include "MailManager.h"
#include "PetWork.h"
#include "UserRecords.h"
#include "SysmsgConfig.h"
#include "PlatLogManager.h"

Deposit::Deposit(SceneUser* pUser) : m_pUser(pUser),
m_dayTimer(5, 0)
{
}

Deposit::~Deposit()
{
}

bool Deposit::load(const BlobDeposit& data)
{
  m_mapType.clear();
  m_vecCard.clear();
  for (int i = 0; i < data.typedatas_size(); ++i)
  {
    const DepositTypeData& typeData =  data.typedatas(i);
    m_mapType[typeData.type()] = typeData;    
    
    auto it = m_mapType.insert(std::make_pair(typeData.type(), typeData)).first;
    if (it != m_mapType.end())
      checkState(it->second);
  }

  for (int i = 0; i < data.carddatas_size(); ++i)
  {
    const DepositCardData& cardData = data.carddatas(i);
    m_vecCard.push_back(cardData);
  }

  //for (int i = 0; i < data.chargedatas_size(); ++i)
  //{
  //  m_chargeDatas.push_back(data.chargedatas(i));
  //}
  
  for (int i = 0; i < data.usedcards_size(); ++i)
  {
    m_setUsedCard.insert(data.usedcards(i));
  }

  for (int i = 0; i < data.quota_detail_size(); ++i)
  {
    m_listQuotaDetail.push_back(data.quota_detail(i));
  }

  for (int i = 0; i < data.quota_log_size(); ++i)
  {
    m_listQuotaLog.push_back(data.quota_log(i));
  }

  refreshQuota();

  auto it = m_mapType.find(ETITLE_TYPE_MONTH);
  if (it != m_mapType.end() && isExpire(it->second) == false)
  {
    m_pUser->validMonthCardSkillPos();
    m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->refreshSlot();
    if(it->second.invalid() == false)
      it->second.set_invalid(true);
  }

  m_bEverHasQuota = data.hasquota();
  m_bEverGetItem = data.getitem();
  m_qwQuotaLock = data.quota_lock();

  return true;
}

bool Deposit::save(BlobDeposit* data)
{
  if (data == nullptr)
    return false;

  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    DepositTypeData* p = data->add_typedatas();
    p->CopyFrom(it->second);
  }

  for (auto & v: m_vecCard)
  {
    DepositCardData* p = data->add_carddatas();
    p->CopyFrom(v);
  }

  //for (auto &v : m_chargeDatas)
  //{
  //  ChargeData* p = data->add_chargedatas();
	 // if (p)
  //  	p->CopyFrom(v);
  //}  

  for (auto &v : m_listQuotaDetail)
  {
    QuotaDetail* p = data->add_quota_detail();
    if (p)
      p->CopyFrom(v);
  }

  for (auto &v : m_listQuotaLog)
  {
    QuotaLog* p = data->add_quota_log();
    if (p)
      p->CopyFrom(v);
  }

  data->set_hasquota(m_bEverHasQuota);
  data->set_getitem(m_bEverGetItem);
  data->set_quota_lock(m_qwQuotaLock);
  
  for (auto &s : m_setUsedCard)
  {
    data->add_usedcards(s);
  }

  XLOG << "[充值-月卡-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 数据大小 : " << data->ByteSize() << "usescardsize" << m_setUsedCard.size() << "是否有过额度" << m_bEverHasQuota << XEND;
  return true;
}

//10分钟tick一次
void Deposit::timeTick(DWORD curSec)
{
  //checkExpire(curSec);
  if (m_dayTimer.timeUp(curSec))
  {//每天5点刷新额度
    refreshQuota();
    for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
    {
      checkState(it->second);
    }
  }  
}

bool Deposit::isExpire(const Cmd::DepositTypeData& rData)
{
  if (rData.expiretime() <= now())
    return true;
  return false;
}

DWORD Deposit::getExpireTime(EDepositCardType eType) const
{
  auto it = m_mapType.find(eType);
  if (it == m_mapType.end())
    return 0;
  return it->second.expiretime();
}

void Deposit::sendDataToClient()
{
  DepositCardInfo cmd;

  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    DepositTypeData* p = cmd.add_card_datas();
    p->CopyFrom(it->second);
  } 
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[月卡-发送]" << m_pUser->id << cmd.ShortDebugString() << XEND;
}

//过期不删除
//void Deposit::checkExpire(DWORD curSec)
//{
//  bool isChanged = false;
//  for (auto it = m_mapType.begin(); it != m_mapType.end();)
//  {
//    if (it->second.expiretime() <= curSec)
//    {
//      XLOG << "[充值-月卡] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 月卡类型 : " << it->first << XEND;
//      it = m_mapType.erase(it);
//      isChanged = true;
//      continue;
//    }
//    else
//      ++it;
//  }
//
//  if (isChanged)
//  {
//    sendDataToClient();
//  }
//}

bool Deposit::useCard(DWORD itemId)
{
  const SDepositCard* pCfg = DepositConfig::getMe().getSDepositCard(itemId);
  if (!pCfg)
  {
    XLOG << "[充值-月卡-使用] 失败" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "找不到相应月卡配置，itemid" << itemId << "月卡类型" << XEND;
    return false;
  }
  DWORD expireTime = 0;
  DWORD oneDayBattleTime = m_pUser->getOneDayBattleTime();
  auto it = m_mapType.find(pCfg->type);
  if (it == m_mapType.end())
  {
    //第一次使用月卡
    Cmd::DepositTypeData data;
    data.set_type(pCfg->type);
    DWORD curSec = now();
    //以5点作为每日分界线
    expireTime = xTime::getDayStart(curSec, 5 *HOUR_T) + pCfg->typeCfg.duration * DAY_T + 5 * HOUR_T;
    data.set_starttime(curSec);
    data.set_expiretime(expireTime);
    data.set_invalid(true);
    it = m_mapType.insert(std::make_pair(pCfg->type, data)).first;
    m_pUser->getManual().onMonthCardLock();
    onMonthCardValid(it->second);
  }
  else
  {
    //重新激活使用
    if (isExpire(it->second))
    {
      DWORD curSec = now();
      //以5点作为每日分界线
      expireTime = xTime::getDayStart(curSec, 5 * HOUR_T) + pCfg->typeCfg.duration * DAY_T + 5 * HOUR_T;
      it->second.set_starttime(curSec);
      it->second.set_expiretime(expireTime);
      it->second.set_invalid(true);
      onMonthCardValid(it->second);
    }
    else
    {
      expireTime = it->second.expiretime() + pCfg->typeCfg.duration * DAY_T;
      it->second.set_expiretime(expireTime);
    }
  }
 
  Cmd::DepositCardData cardData;
  cardData.set_itemid(itemId);
  cardData.set_isused(false);
  m_vecCard.push_back(cardData);

  //send to client
  sendDataToClient();
  {
    char strDate[30];
    time_t currTime = expireTime;
    struct tm tm = *localtime(&currTime);
    strftime(strDate, sizeof(strDate), "%Y-%m-%d", &tm);
    DWORD diffDay = xTime::getDiffDay(expireTime, now(), 5 *HOUR_T);
    MsgManager::sendMsg(m_pUser->id, 1029, MsgParams(strDate, diffDay));
  }
  //MsgManager::sendMsg(m_pUser->id, 1029, MsgParams(extraTime));

  DWORD afterDayTime = m_pUser->getOneDayBattleTime();
  if (afterDayTime > oneDayBattleTime)
  {
    m_pUser->getUserSceneData().setTotalBattleTime(m_pUser->getUserSceneData().getTotalBattleTime() + afterDayTime - oneDayBattleTime);
  }
  // refresh buff
  m_pUser->m_oBuff.onBattleStatusChange();
  m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->refreshSlot();
  m_pUser->getPetWork().setCardExpireTime(expireTime);
  m_pUser->getUserRecords().setCardExpireTime(expireTime);

  // add headdress
  if (pCfg->headdress)
  {
    if (isFirstTimeUse(itemId))
    {
      ItemInfo itemInfo;
      itemInfo.set_id(pCfg->headdress);
      itemInfo.set_count(1);
      itemInfo.set_source(ESOURCE_MONTHCARD);
      //m_pUser->getPackage().addItemAvailable(itemInfo, true);
      m_pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE, true);
      XLOG << "[充值-月卡-使用] 首次使用该月月卡获得头饰" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "itemid" << itemId << "月卡类型" << pCfg->type << "到期时间" << expireTime << XEND;
    }
  }

  XLOG << "[充值-月卡-使用] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name <<"itemid"<<itemId <<"year"<<pCfg->year <<"month"<< pCfg->month << "月卡类型" << pCfg->type << "到期时间" << expireTime << XEND;
  m_setUsedCard.insert(itemId);
  m_pUser->saveDataNow();
  return true;
}

bool Deposit::hasMonthCard()
{
  auto it = m_mapType.find(ETITLE_TYPE_MONTH);
  if (it == m_mapType.end())
    return false;
  if (isExpire(it->second))
    return false;
  return true;
}

// 野外普通魔物掉率和经验(in = 0.33)
float Deposit::getNormalDrop(float in)
{
  DWORD ret = in*100;
  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    if (isExpire(it->second))
      continue;
    ret = DepositConfig::getMe().doFunc(it->first, EFuncType_NORMALDROP, ret);
  }
  XLOG << "[充值-野外普通魔物掉率和经验] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "old" << in*100 << "new" << ret << XEND;
  return (float)ret/100;
}

//addcittime
DWORD Deposit::getAddcitTime(DWORD in)
{
  DWORD ret = in;
  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    if (isExpire(it->second))
      continue;
    ret = DepositConfig::getMe().doFunc(it->first, EFuncType_ADDCITTIME, ret);
  }
  XLOG << "[充值-沉迷延长时间] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "old" << in << "new" << ret << XEND;
  return ret;
}

// pet work
DWORD Deposit::getPetWorkSpaceExtra(DWORD in)
{
  DWORD ret = in;
  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    if (isExpire(it->second))
      continue;
    ret = DepositConfig::getMe().doFunc(it->first, EFuncType_PETWORK, ret);
  }
  XLOG << "[充值-宠物打工] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "old" << in << "new" << ret << XEND;
  return ret;
}

//exp
DWORD Deposit::getExp(EFuncType funcType, DWORD in)
{ 
  DWORD ret = in;
  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    if (isExpire(it->second))
      continue;

    ret = DepositConfig::getMe().doFunc(it->first, funcType, ret);
    //warning 增加的经验小于1 补1点经验
    if (in != 0 && (in == ret))
    {
      ret += 1;
    }
  }

  XLOG << "[充值-exp] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << (funcType == EFuncType_EXP ? "base经验" : "job经验") << "old" << in << "new" << ret << XEND;
  return ret;
}


QWORD Deposit::getGuildHonor(QWORD in)
{
  QWORD ret = in;
  for (auto it = m_mapType.begin(); it != m_mapType.end(); ++it)
  {
    if (isExpire(it->second))
      continue;

    ret = DepositConfig::getMe().doFunc(it->first, EFuncType_GUILDHONOR, ret);
  }

  XLOG << "[充值-公会之证] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "old" << in << "new" << ret << XEND;
  return ret;
}


void Deposit::refreshQuota()
{
  //过期的过期了，删除  
  DWORD expireTime = now() - MiscConfig::getMe().getQuotaCFG().dwDetailExpireDayDel * DAY_T;
  for (auto it = m_listQuotaDetail.begin(); it != m_listQuotaDetail.end();)
  {
    if (it->expire_time() <= expireTime)
    {
      it = m_listQuotaDetail.erase(it);
      continue;
    }
    ++it;
  }

  updateQuota();
}

bool Deposit::addQuota(QWORD num, Cmd::EQuotaType type, DWORD mailId/*=0*/, DWORD expireDay/*=0*/)
{
  if (num == 0)
    return false;

  if(MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_LIMIT) == true)
  {
    return true;
  }

  QWORD old = m_qwQuota;
  DWORD curSec = now();

  QuotaLog log;
  log.set_value(num);
  log.set_time(curSec);
  log.set_type(type);
  addQuotaLog(log);
    
  if (expireDay == 0)
    expireDay = MiscConfig::getMe().getQuotaCFG().dwDefaultExpireDay;
  DWORD expireTime = xTime::getDayStartAddDay(curSec, 5 * HOUR_T, expireDay);

  QuotaDetail detail;
  detail.set_value(num);
  detail.set_left(num);
  detail.set_expire_time(expireTime);
  detail.set_time(curSec);
  addQuotaDetail(type, detail);
  
  if (m_bEverGetItem == false)
  {
    TVecItemInfo vecItemInfo;
    ItemInfo itemInfo;
    itemInfo.set_id(MiscConfig::getMe().getQuotaCFG().dwItemId);
    itemInfo.set_count(1);
    vecItemInfo.push_back(itemInfo);
    if (mailId)
    {
      MailManager::getMe().sendMail(m_pUser->id, 0, "", "", "", EMAILTYPE_NORMAL, mailId, vecItemInfo);
    }
    else
    {
      m_pUser->getPackage().addItem(vecItemInfo, EPACKMETHOD_AVAILABLE);
    }
    m_bEverGetItem = true;
  }
  
  m_bEverHasQuota = true;
  m_pUser->setDataMark(EUSERDATATYPE_HASCHARGE);
  updateQuota();
  XLOG << "[额度] 增加额度" << m_pUser->id << m_pUser->name << "增加前额度" << old << "增加" << num <<"类型" << type << "剩余" << m_qwQuota <<"过期天数"<<expireDay <<"邮件id" << mailId << XEND;

  PlatLogManager::getMe().QuotaLog(thisServer,
                                   m_pUser->accid,
                                   m_pUser->id,
                                   EQuotaOptType_Add,
                                   type,
                                   num,
                                   m_qwQuota,
                                   m_qwQuotaLock
    );

  return true;
}

bool Deposit::subQuota(QWORD quota, Cmd::EQuotaType type)
{
  QWORD old = m_qwQuota;
  QWORD want = quota;
  DWORD curSec = now();
  {
    for (auto it = m_listQuotaDetail.begin(); it != m_listQuotaDetail.end();++it)
    {
      if (isQuotaExpire(it->expire_time(), curSec))
        continue;

      if (it->left() > quota)
      {
        it->set_left(it->left() - quota);
        quota = 0; 
        break;
      }
      else
      {
        //已经用完
        quota -= it->left();
        it->set_left(0);        
      }
    }
  }

  MsgManager::sendMsg(m_pUser->id, 25697, MsgParams(want));

  updateQuota();
  QuotaLog log;
  QWORD trueReduce = old - m_qwQuota;
  log.set_value(trueReduce);
  log.set_time(curSec);
  log.set_type(type);
  addQuotaLog(log);

  XLOG << "[额度] 扣除额度" << m_pUser->id << m_pUser->name << "扣前额度"<< old <<"扣除"<< want <<"剩余" << m_qwQuota <<"类型" << type << XEND;

  PlatLogManager::getMe().QuotaLog(thisServer,
                                   m_pUser->accid,
                                   m_pUser->id,
                                   EQuotaOptType_Sub,
                                   type,
                                   want,
                                   m_qwQuota,
                                   m_qwQuotaLock
    );

  return true;
}

// 锁额度 可过期 不可消耗
bool Deposit::lockQuota(QWORD quota, Cmd::EQuotaType type)
{
  XLOG << "[额度-锁定]" << m_pUser->id << m_pUser->name << "当前额度" << m_qwQuota << "当前锁定额度" << m_qwQuotaLock << "锁定额度" << quota << "锁定类型" << type << XEND;
  if(!quota)
    return true;

  if(!checkQuota(quota))
    return false;

  m_qwQuotaLock += quota;
  updateQuota();

  MsgManager::sendMsg(m_pUser->id, 25698, MsgParams(quota));

  //log
  QuotaLog log;
  log.set_value(quota);
  log.set_time(now());
  log.set_type(type);
  addQuotaLog(log);

  PlatLogManager::getMe().QuotaLog(thisServer,
                                   m_pUser->accid,
                                   m_pUser->id,
                                   EQuotaOptType_Lock,
                                   type,
                                   quota,
                                   m_qwQuota,
                                   m_qwQuotaLock
    );

  return true;
}

// 解锁额度
QWORD Deposit::unlockQuota(QWORD quota, Cmd::EQuotaType type, bool sub /* = false*/)
{
  XLOG << "[额度-解锁]" << m_pUser->id << m_pUser->name << "当前额度" << m_qwQuota << "当前锁定额度" << m_qwQuotaLock << "解锁额度" << quota << "解锁类型" << type << XEND;
  if(!quota)
    return 0;

  QWORD qwUnlockQuota = quota;
  if(m_qwQuotaLock >= quota)
  {
    m_qwQuotaLock -= quota;
  }
  else
  {
    m_qwQuotaLock = 0;
    qwUnlockQuota = m_qwQuotaLock;
  }

  QWORD qwRealUnlockQuota = qwUnlockQuota;
  if(m_qwQuota < qwUnlockQuota)
    qwRealUnlockQuota = m_qwQuota;

  MsgManager::sendMsg(m_pUser->id, 25699, MsgParams(qwRealUnlockQuota));

  //log
  QuotaLog log;
  log.set_value(qwRealUnlockQuota);
  log.set_time(now());
  log.set_type(type);
  addQuotaLog(log);

  PlatLogManager::getMe().QuotaLog(thisServer,
                                   m_pUser->accid,
                                   m_pUser->id,
                                   EQuotaOptType_Unlock,
                                   type,
                                   quota,
                                   m_qwQuota,
                                   m_qwQuotaLock
    );

  // 并扣除
  if(sub)
  {
    if(Cmd::EQuotaType_U_Give_Trade == type)
      subQuota(qwRealUnlockQuota, Cmd::EQuotaType_C_Give_Trade);
    else if(Cmd::EQuotaType_U_Booth == type)
      subQuota(qwRealUnlockQuota, Cmd::EQuotaType_C_Booth);
  }
  else
    updateQuota();

  return qwRealUnlockQuota;
}

// 重新上架
bool Deposit::resell(QWORD num_unlock, QWORD num_lock)
{
  XLOG << "[额度-重新上架]" << m_pUser->id << m_pUser->name << "当前额度" << m_qwQuota << "当前锁定额度" << m_qwQuotaLock << "解锁额度" << num_unlock << "锁定额度" << num_lock << XEND;

  if(m_qwQuota < num_lock)
  {
    XERR << "[额度-重新上架]" << m_pUser->id << m_pUser->name << "上架失败，当前额度" << m_qwQuota << "当前锁定额度" << m_qwQuotaLock << "解锁额度" << num_unlock << "锁定额度" << num_lock << XEND;
    return false;
  }

  unlockQuota(num_unlock, Cmd::EQuotaType_U_Booth);
  return lockQuota(num_lock, Cmd::EQuotaType_L_Booth);
}


void Deposit::clearQuota()
{
  m_listQuotaLog.clear();
  m_listQuotaDetail.clear();
  
  updateQuota();
}

bool Deposit::checkQuota(QWORD quota)
{
  if (quota + m_qwQuotaLock > m_qwQuota)
    return false;

  return true;
}

QWORD Deposit::getQuota()
{
  if(m_qwQuota < m_qwQuotaLock)
    return 0;

  return m_qwQuota - m_qwQuotaLock;
}

QWORD Deposit::getQuotaLock()
{
  return m_qwQuotaLock;
}

QWORD Deposit::updateQuota()
{
  m_qwQuota = 0;
  DWORD curSec = now();
  for (auto it = m_listQuotaDetail.begin(); it != m_listQuotaDetail.end(); ++it)
  {
    if (isQuotaExpire(it->expire_time(), curSec))
      continue;
    m_qwQuota += it->left();
  }

  //锁定额度不大于总额度
  if(m_qwQuotaLock > m_qwQuota)
    m_qwQuotaLock = m_qwQuota;

  m_pUser->setDataMark(EUSERDATATYPE_QUOTA);
  m_pUser->setDataMark(EUSERDATATYPE_QUOTA_LOCK);
  m_pUser->refreshDataAtonce();
  m_pUser->refreshTradeQuota();
  XLOG << "[额度]" << m_pUser->id << m_pUser->name << "当前额度" << m_qwQuota << XEND;
  return m_qwQuota;
}

void Deposit::checkState(Cmd::DepositTypeData& rData)
{
  switch (rData.state())
  {
  case EDEPOSITSTAT_NONE:
  { //老的数据
    if (!isExpire(rData))
    {
      onMonthCardValid(rData);     
    }
  }
  break;
  case EDEPOSITSTAT_VALID:
  {
    if (isExpire(rData))
    {
      onMonthCardInvalid(rData);
      rData.set_state(EDEPOSITSTAT_INVALID);
    }
  }
  break;
  default:
    break;
  } 
}

void Deposit::onMonthCardValid(Cmd::DepositTypeData& rData)
{
  if(!m_pUser)
    return;

  m_pUser->validMonthCardSkillPos();
  TVecDWORD vecMsgId;
  DepositConfig::getMe().getDepositMsg(vecMsgId);
  for(auto& v : vecMsgId)
    MsgManager::sendMsg(m_pUser->id, v);

  XLOG << "[充值-月卡] 月卡有效，增加自动战斗技能栏" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "到期时间" << rData.expiretime() << XEND;
  rData.set_state(EDEPOSITSTAT_VALID);
  m_pUser->setDataMark(EUSERDATATYPE_MONTHCARD);
}

void Deposit::onMonthCardInvalid(Cmd::DepositTypeData& rData)
{
  m_pUser->invalidMonthCardSkillPos();
  XLOG << "[充值-月卡] 月卡到期，减少自动战斗技能栏" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "到期时间" << rData.expiretime() << XEND;
  rData.set_state(EDEPOSITSTAT_INVALID);
  m_pUser->setDataMark(EUSERDATATYPE_MONTHCARD);
  m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->refreshSlot();
}

bool Deposit::isFirstTimeUse(DWORD itemId)
{
  return m_setUsedCard.find(itemId) == m_setUsedCard.end() ? true : false;
}

void Deposit::redisPatch()
{  
  DWORD yearMonthBase = 20170;
  DWORD itemIdBase = 800100;
  for (DWORD i = 1; i <= 7; ++i)
  {
    DWORD yearMonth = yearMonthBase + i;
    std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_MONTH_CARD_HEADDRESS, m_pUser->id, yearMonth);
    DWORD addCount = 0;
    RedisManager::getMe().getData(key, addCount);
    if (addCount > 0)
    {
      DWORD itemId = itemIdBase + i;
      m_setUsedCard.insert(itemId);

      RedisManager::getMe().delData(key);
      XLOG << "[充值-月卡-redis数据转换] " << m_pUser->accid << m_pUser->id << "itemid" << itemId << "yearmonth" << yearMonth << "key" << key << XEND; 
    }
  }
}

DWORD Deposit::getPackageSlot(EPackType eType)
{
  DWORD dwMonthCardSlot = 0;
  if(m_pUser->getDeposit().hasMonthCard() && eType == EPACKTYPE_MAIN)
  {
    const SDepositTypeCFG* pCFG = MiscConfig::getMe().getDepositTypeCFG(ETITLE_TYPE_MONTH);
    if (!pCFG)
      XERR << "[包裹-扩展] 加载配置Table_MonthCard.txt失败, type"<< ETITLE_TYPE_MONTH <<"在GameConfig 中找不到。" << XEND;
    else
      dwMonthCardSlot = pCFG->expandpackage;
  }

  return dwMonthCardSlot;
}

void Deposit::addQuotaLog(Cmd::QuotaLog &log)
{
  m_listQuotaLog.push_front(log);

  while (m_listQuotaLog.size() > MiscConfig::getMe().getQuotaCFG().dwMaxSaveLogCount)
  {
    auto it = m_listQuotaLog.end();
    it--;
    m_listQuotaLog.erase(it);
  }
}

void Deposit::addQuotaDetail(Cmd::EQuotaType type, Cmd::QuotaDetail &detail)
{
  if (type != EQuotaType_G_Auction)
  {
    m_listQuotaDetail.push_back(detail);
    return;
  }
  
  QWORD left = detail.left();
  DWORD curSec = now();

  for (auto it = m_listQuotaDetail.rbegin(); it != m_listQuotaDetail.rend(); ++it)
  {
    if (left == 0)
      break;

    if (it->value() == it->left())
      continue;
    QWORD room = it->value() - it->left();
    if (room >= left)
    {
      it->set_left(it->left() + left);
      left = 0;
    }
    else
    {
      it->set_left(it->value());
      left -= room;
    }

    if (isQuotaExpire(it->expire_time(), curSec))
    {//延迟过期一天
      it->set_expire_time(xTime::getDayStartAddDay(curSec, 5 * HOUR_T, 1));
    }
  }
  
  if (left)
  {
    detail.set_value(left);
    detail.set_left(left);
    detail.set_expire_time(xTime::getDayStartAddDay(curSec, 5 * HOUR_T, 1));
    detail.set_time(curSec - MiscConfig::getMe().getQuotaCFG().dwDefaultExpireDay * DAY_T);
    m_listQuotaDetail.push_front(detail);   //插入到头部
  }
}

void Deposit::reqQuotaLog(Cmd::ReqQuotaLogCmd& rev)
{
  do 
  {
    if(MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_LIMIT) == true)
      break;

    if (m_listQuotaLog.empty())
      break;

    DWORD totalCount = m_listQuotaLog.size() / MiscConfig::getMe().getQuotaCFG().dwLogCountPerPage + 1;

    DWORD pageIndex = rev.page_index() - 1;   //1转成0
    if (pageIndex >= totalCount)   //超页了
      break;

    DWORD startIndex = pageIndex * MiscConfig::getMe().getQuotaCFG().dwLogCountPerPage;
    DWORD endIndex = startIndex + MiscConfig::getMe().getQuotaCFG().dwLogCountPerPage;
    DWORD i = 0;
    for (auto it = m_listQuotaLog.begin(); it != m_listQuotaLog.end(); ++it)
    {
      if (i >= startIndex && i < endIndex)
      {
        rev.add_log()->CopyFrom(*it);
      }
      i++;
    }
  } while (0); 

  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[额度-积分] 请求日志" << m_pUser->id << m_pUser->name << "msg" << rev.ShortDebugString() << XEND;
}

void Deposit::reqQuotaDetail(Cmd::ReqQuotaDetailCmd& rev)
{
  convertQuotaShow();

  do 
  {
    if(MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_LIMIT) == true)
      break;

    if (m_listQuotaDetailShow.empty())
      break;

    DWORD totalCount = m_listQuotaDetailShow.size() / MiscConfig::getMe().getQuotaCFG().dwDetailCountPerPage + 1;
    DWORD pageIndex = rev.page_index() - 1;   //1转成0

    if (pageIndex >= totalCount)   //超页了
      break;

    DWORD startIndex = pageIndex * MiscConfig::getMe().getQuotaCFG().dwDetailCountPerPage;
    DWORD endIndex = startIndex + MiscConfig::getMe().getQuotaCFG().dwDetailCountPerPage;
    DWORD i = 0;
    for (auto it = m_listQuotaDetailShow.begin(); it != m_listQuotaDetailShow.end(); ++it)
    {
      if (i >= startIndex && i < endIndex)
      {
        rev.add_detail()->CopyFrom(*it);
      }
      i++;
    }

  } while (0);  

  PROTOBUF(rev, send, len);
  m_pUser->sendCmdToMe(send, len);

  XDBG << "[额度-积分] 请求积分详情" << m_pUser->id << m_pUser->name << "msg" << rev.ShortDebugString() << XEND;
}

void Deposit::convertQuotaShow()
{
  m_listQuotaDetailShow.clear();
  TListQuotaDetail tempList;
  DWORD curSec = now();  
  DWORD expireShowTime = curSec - MiscConfig::getMe().getQuotaCFG().dwDetailExpireDay * DAY_T;
  for (auto it = m_listQuotaDetail.begin(); it != m_listQuotaDetail.end(); ++it)
  {
    //过期且可以展示
    if (isQuotaExpire(it->expire_time(), curSec) && it->expire_time() > expireShowTime)
    {
      tempList.push_back(*it);
      continue;
    }
    if (it->left() == 0)
    {
      tempList.push_back(*it);
      continue;
    }
   
    m_listQuotaDetailShow.push_back(*it);
  }  
  m_listQuotaDetailShow.insert(m_listQuotaDetailShow.end(), tempList.begin(), tempList.end());
}

void Deposit::onLine()
{
  auto it = m_mapType.find(ETITLE_TYPE_MONTH);
  if (it == m_mapType.end())
    return ;
  if (isExpire(it->second))
  {
    if(m_pUser->getUserSceneData().getOnlineTime() > it->second.expiretime() && it->second.invalid() == true)
    {
      it->second.set_invalid(false);
      Cmd::NTFMonthCardEnd message;
      PROTOBUF(message, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
    return ;
  }

  DWORD addHour = (it->second.expiretime() - now()) % HOUR_T;  //不足一小时按一小时算
  DWORD diffHour = (it->second.expiretime() - now()) / HOUR_T;
  if(addHour != 0)
    diffHour += 1;
  DWORD diffDay = diffHour / 24;
  if(diffHour > 3*24 || m_pUser->getVar().getVarValue(EVARTYPE_DEPOSIT_END_NTF) == 1)
    return;
  m_pUser->getVar().setVarValue(EVARTYPE_DEPOSIT_END_NTF, 1);
  DWORD leftHour = diffHour % 24;
  std::stringstream msg;
  msg.str("");
  if(diffDay > 0)
    msg << diffDay << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_DAY);
  if(diffHour != 0)
    msg << leftHour << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_HOUR);
  MsgManager::sendMsg(m_pUser->id, 1030, MsgParams(msg.str().c_str()));
}

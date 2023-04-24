#include "UserStat.h"
#include "SceneUser.h"
#include "SceneServer.h"

UserStat::UserStat(SceneUser* pUser) : m_pUser(pUser)
{
}

UserStat::~UserStat()
{
}

void UserStat::load(const BlobStatVar& oBlob)
{
  for (int i = 0; i < oBlob.datas_size(); ++i)
  {
    const StatVar& rVar = oBlob.datas(i);
    m_oVar[rVar.key()] = rVar;
  }

  for (int i = 0; i < oBlob.skilldamage_size(); ++i)
  {
    const SkillDamage oSkillDamage = oBlob.skilldamage(i);
    STAT_TYPE t = static_cast<STAT_TYPE>(oSkillDamage.type());
    auto& m = m_mapSkillDamage[t];
    SSkillDamage& rDamage =  m[oSkillDamage.skillid()];
    rDamage.dwDamage = oSkillDamage.damage();
    rDamage.qwTargetId = oSkillDamage.targetid();
  }
  m_mapKillMonsterNum.clear();
  for (int i = 0; i < oBlob.killmonster_size(); ++i)
  {
    const StatKillMonster& data = oBlob.killmonster(i);
    m_mapKillMonsterNum[data.monsterid()] = data.killnum();
  }
}

void UserStat::save(BlobStatVar* pBlob)
{  
  for (auto &v : m_oVar)
  {
    pBlob->add_datas()->CopyFrom(v.second);
  }
  
  for (auto &m : m_mapSkillDamage)
  {
    for (auto&subM : m.second)
    {
      Cmd::SkillDamage* pDamage = pBlob->add_skilldamage();
      if (pDamage)
      {
        pDamage->set_type(m.first);
        pDamage->set_skillid(subM.first);
        pDamage->set_damage(subM.second.dwDamage);
        pDamage->set_targetid(subM.second.qwTargetId);
      }
    }
  }
  for (auto &m : m_mapKillMonsterNum)
  {
    StatKillMonster* pKill = pBlob->add_killmonster();
    if (pKill)
    {
      pKill->set_monsterid(m.first);
      pKill->set_killnum(m.second);
    }
  }
}

void UserStat::onLogin()
{
  if (checkAndSet(ESTATTYPE_ONLINE_COUNT, 0))
    sendStatLog(ESTATTYPE_ONLINE_COUNT, 0, m_pUser->getProfession(), m_pUser->getLevel(), 1);
  if (checkAndSet(ESTATTYPE_SKILL_DAMAGE_USER, 0))
    sendSkillDamage();

  if (checkAndSet(ESTATTYPE_FASHION, 0))
    statFashion();

  if (checkAndSet(ESTATTYPE_MAX_KILL_MONSTER, 0))
  {
    if (!m_mapKillMonsterNum.empty())
    {
      KillMonsterNumStatCmd cmd;
      cmd.set_userid(m_pUser->id);
      cmd.set_zoneid(thisServer->getZoneID());
      cmd.set_professionid(m_pUser->getProfession());
      for (auto &m : m_mapKillMonsterNum)
      {
        StatKillMonster* pkillmonster = cmd.add_killmonster();
        if (pkillmonster)
        {
          pkillmonster->set_monsterid(m.first);
          pkillmonster->set_killnum(m.second);
        }
      }
      PROTOBUF(cmd, send, len);
      thisServer->sendCmd(ClientType::stat_server, send, len);
    }
  }
}

//统计时装穿着数量
void UserStat::statFashion()
{ 
  std::map<DWORD/*itemid*/, std::pair<DWORD/**/, DWORD>> mapPr;
  bool bOn = false;
  auto func = [&](ItemBase* pBase)
  {
    if (pBase == nullptr)
      return;
    if (!ItemConfig::getMe().isFashion(pBase->getCFG()->eItemType))
      return;
    auto& pr = mapPr[pBase->getCFG()->dwTypeID];
    pr.second = 1;
    if (bOn)
      pr.first = 1;
  };
  bOn = false;
  m_pUser->getPackage().getPackage(EPACKTYPE_MAIN)->foreach(func);
  bOn = false;
  m_pUser->getPackage().getPackage(EPACKTYPE_TEMP_MAIN)->foreach(func);
  bOn = false;
  m_pUser->getPackage().getPackage(EPACKTYPE_STORE)->foreach(func);
  bOn = false;
  m_pUser->getPackage().getPackage(EPACKTYPE_PERSONAL_STORE)->foreach(func);
  

  bOn = true;
  m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP)->foreach(func);
  bOn = true;
  m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP)->foreach(func);
  
  for (auto& m : mapPr)
  {
    StatisticsDefine::sendStatLog4(thisServer, ESTATTYPE_FASHION, m.first, 0, 0, m.second.first, m.second.second);
  }
}

bool UserStat::checkAndSet(STAT_TYPE type, QWORD key/*=0*/)
{
  string strKey = getKey(type, key);
  DWORD ret = getVarValue(type, strKey);
  if (ret == 1)
    return false;
  setVarValue(strKey, 1);
  return true;
}

void UserStat::sendStatLog(STAT_TYPE eType, QWORD key, QWORD subKey, DWORD level, DWORD value1, bool isFloat /*=false*/)
{
  if (!m_pUser)
    return;  
  StatisticsDefine::sendStatLog(thisServer, eType, key, subKey, level, value1, isFloat);
}

void UserStat::sendStatLog2(STAT_TYPE type, QWORD key, QWORD subKey, QWORD subKey2, DWORD level, DWORD value1, bool isFloat/* = false*/)
{
  if (!m_pUser)
    return;
  StatisticsDefine::sendStatLog2(thisServer, type, key, subKey, level, subKey2, value1, isFloat);
}

DWORD UserStat::getVarValue(STAT_TYPE eType, const string& key)
{
  
  EVarTimeType timeType = getTimeType(eType);
  static DWORD dwTimeOffset = 0;
  StatVar& rVar = m_oVar[key];
  DWORD value = rVar.value();
  DWORD recTime = rVar.time();
  DWORD nowTime = now();
  switch (timeType)
  {
  case EVARTIMETYPE_DAY:
  {
    //0点更新
    DWORD recTimeZero = xTime::getDayStart(recTime, dwTimeOffset);
    DWORD nowTimeZero = xTime::getDayStart(nowTime, dwTimeOffset);
    if (nowTimeZero >= recTimeZero + 86400)
    {
      value = 0;
      setVarValue(key, value);
    }
  }
  break;
  case EVARTIMETYPE_WEEK:
  {
    DWORD recMondayTimeZero = xTime::getWeekStart(recTime, dwTimeOffset);
    DWORD nowMondayTimeZero = xTime::getWeekStart(nowTime, dwTimeOffset);

    if (nowMondayTimeZero >= recMondayTimeZero + 86400 * 7)
    {
      value = 0;
      setVarValue(key, value);
    }
  }
  break;  
  default:
    break;
  }
  return value;
}

void UserStat::setVarValue(const string& key, DWORD value)
{
  StatVar& varInfo = m_oVar[key];
  varInfo.set_key(key);
  varInfo.set_time(xTime::getCurSec());
  varInfo.set_value(value);
}

EVarTimeType UserStat::getTimeType(STAT_TYPE type)
{
  switch (type)
  {
  case  ESTATTYPE_TOWER_COUNT:
    return EVARTIMETYPE_WEEK;
  default:
    return EVARTIMETYPE_DAY;
    break;
  }
}

string UserStat::getKey(STAT_TYPE type, QWORD key)
{
  stringstream ss;
  ss << type << "," << key;
  return ss.str();
}
void UserStat::setSkillDamage(STAT_TYPE type, DWORD dwSkillId, DWORD damage, QWORD qwTarget)
{
  SSkillDamage& rDamage = m_mapSkillDamage[type][dwSkillId];
  if (rDamage.dwDamage > damage)
    return;

  rDamage.dwDamage = damage;
  rDamage.qwTargetId = qwTarget;
  
  //TODO 
  //sendSkillDamage();
}

void UserStat::sendSkillDamage()
{
  for (auto&m : m_mapSkillDamage)
  {
    for (auto&subM : m.second)
    {
      if (subM.second.dwDamage == 0)
        continue;
      StatisticsDefine::sendStatLog2(thisServer, 
        m.first, 
        subM.first,/*key skillid*/
        m_pUser->id,/*subkey*/
        subM.second.qwTargetId,/*subkey2*/
        0,/*level*/
        subM.second.dwDamage/*value18*/
      );
    }   
  }
  m_mapSkillDamage.clear();
}

void UserStat::sendDayGetZenyCountLog(QWORD qwNormalZeny, QWORD qwChargeZeny)
{
  DayGetZenyCountCmd cmd;
  cmd.set_userid(m_pUser->id);
  cmd.set_username(m_pUser->name);
  cmd.set_baselv(m_pUser->getLevel());
  cmd.set_joblv(m_pUser->getJobLv());
  cmd.set_profession(m_pUser->getProfession());
  cmd.set_normal_zeny(qwNormalZeny);
  cmd.set_charge_zeny(qwChargeZeny);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmd(ClientType::stat_server, send, len);
}

void UserStat::addMonsterCnt(DWORD monsterid, DWORD cnt)
{
  auto it = m_mapKillMonsterNum.find(monsterid);
  if (it == m_mapKillMonsterNum.end())
  {
    m_mapKillMonsterNum[monsterid] = cnt;
    return;
  }
  if (it->second < cnt)
    it->second = cnt;
}

void UserStat::checkAndSendCurLevelToStat()
{
  if (m_pUser == nullptr)
    return;

  bool bNeedSend = false;
  DWORD dwCurTime = now();
  if (xTime::getDayStart(m_pUser->getUserSceneData().getLastOfflineTime()) != xTime::getDayStart(dwCurTime))
  {
    bNeedSend = true;
  }
  else
  {
    if (m_pUser->getUserSceneData().getLastBaseLv() != m_pUser->getLevel())
      bNeedSend = true;
    if (m_pUser->getUserSceneData().getLastJobLv() != m_pUser->getMaxCurJobLv())
      bNeedSend = true;
  }
  if (bNeedSend)
  {
    StatCurLevel cmd;
    cmd.set_userid(m_pUser->id);
    cmd.set_last_offlinetime(m_pUser->getUserSceneData().getLastOfflineTime());
    cmd.set_last_baselv(m_pUser->getUserSceneData().getLastBaseLv());
    cmd.set_last_joblv(m_pUser->getUserSceneData().getLastJobLv());
    cmd.set_cur_time(dwCurTime);
    cmd.set_cur_baselv(m_pUser->getLevel());
    cmd.set_cur_joblv(m_pUser->getMaxCurJobLv());

    PROTOBUF(cmd, send, len);
    thisServer->sendCmd(ClientType::stat_server, send, len);

    m_pUser->getUserSceneData().setLastOfflineTime(dwCurTime);
    m_pUser->getUserSceneData().setLastBaseLv(m_pUser->getLevel());
    m_pUser->getUserSceneData().setLastJobLv(m_pUser->getMaxCurJobLv());
  }
}


#include "SceneUserManager.h"
#include "SceneUser.h"
#include "SceneServer.h"
#include "RegCmd.h"
#include "RedisManager.h"
#include "SceneUserDataThread.h"
#include "SceneManager.h"
#include "GlobalManager.h"

SceneUserManager::SceneUserManager()
{
}

SceneUserManager::~SceneUserManager()
{
  struct CallBack : public xEntryCallBack
  {
    virtual bool exec(xEntry *e)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(e);
      if (pUser != nullptr)
      {
        XLOG << "[玩家管理-服务器关闭] " << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << " 被踢下线" << XEND;
        SceneUserManager::getMe().onUserQuit((SceneUser*)e,  UnregType::ServerStop);
      }
      return true;
    }
  };
  CallBack callback;
  forEach(callback);

  LOGINLIST::iterator it=loginList.begin(),end=loginList.end();
  for(;it!=end;it++)
  {
    SAFE_DELETE(it->second);
  }
  loginList.clear();
}

bool SceneUserManager::init()
{
  return loadSvrMaxBaseLv();
}

bool SceneUserManager::addUser(SceneUser* user)
{
  if (!addEntry(user))
    return false;
  if (this->size() % 10 == 0)
  {
    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SCENEUSER_NUM, thisServer->getZoneID(), thisServer->getServerName());
    if (key.length() > 3)
      RedisManager::getMe().setData(key, this->size());
  }
  return true;
}

void SceneUserManager::delUser(SceneUser* user)
{
  removeEntry(user);
  SAFE_DELETE(user);

  if (this->size() % 10 == 0)
  {
    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SCENEUSER_NUM, thisServer->getZoneID(), thisServer->getServerName());
    if (key.length() > 3)
      RedisManager::getMe().setData(key, this->size());
  }
}

SceneUser* SceneUserManager::getUserByID(QWORD uid)
{
  return (SceneUser*)getEntryByID(uid);
}

SceneUser* SceneUserManager::getUserByAccID(QWORD accid)
{
  return (SceneUser*)getEntryByTempID(accid);
}

bool SceneUserManager::addLoginUser(SceneUser* user)
{
  if(loginList.find(user->id) != loginList.end())
    return false;
  loginList.insert(std::make_pair(user->id,user));
  return true;
}

void SceneUserManager::delLoginUser(SceneUser* user)
{
  loginList.erase(user->id);
}

SceneUser* SceneUserManager::getLoginUserByID(QWORD uid)
{
  LOGINLIST::iterator it=loginList.find(uid);
  if(it != loginList.end())
    return it->second;
  else
    return NULL;
}

#define USER_GROUP 4

bool SceneUserManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();

  xEntryID::Iter it=xEntryID::ets_.begin(),end=xEntryID::ets_.end();
  xEntryID::Iter temp;
  for(;it!=end;)
  {
    temp = it++;
    SceneUser *pUser = dynamic_cast<SceneUser*>(temp->second);
    if (pUser == nullptr)
      continue;
    if (m_dwGroup == pUser->id % CommonConfig::SCENE_USER_GROUP_NUM)
    {
      pUser->refreshMe(curMSec);
    }
    if (pUser->getScene())
    {
      pUser->getScene()->addActiveNineScreen(pUser->getPos());
    }
  }

  m_dwGroup = ((m_dwGroup + 1) % CommonConfig::SCENE_USER_GROUP_NUM);

  return true;
}

void SceneUserManager::load()
{
  FUN_TIMECHECK_30();

  SceneUserDataLoad *pData = SceneUserLoadThread::getMe().get();
  while (pData)
  {
    SceneUserLoadThread::getMe().pop();
    login(pData);
    SAFE_DELETE(pData);

    pData = SceneUserLoadThread::getMe().get();
  }
}

void SceneUserManager::onUserQuit(SceneUser* user, UnregType type)
{
  if (user == nullptr)
    return;
  user->unReg(type);
  delUser(user);
}

void SceneUserManager::reloadconfig(ConfigType type)
{
  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m->second);
    if (pUser == nullptr)
      continue;

    pUser->reloadConfig(type);
  }
}

void SceneUserManager::syncWantedQuest()
{
  QuestCanAcceptListChange cmd;
  PROTOBUF(cmd, send, len);

  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m->second);
    if (pUser == nullptr)
      continue;

    pUser->sendCmdToMe(send, len);
  }
}

void SceneUserManager::syncDailyRate()
{
  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m->second);
    if (pUser == nullptr)
      continue;
    if (pUser->getQuest().hasDailyQuest() == false)
      continue;
    pUser->getQuest().queryOtherData(EOTHERDATA_DAILY);
  }
}

void SceneUserManager::updateWeather(DWORD dwMapID, DWORD dwWeather)
{
  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m->second);
    if (pUser == nullptr || pUser->isOwnWeather() == true || pUser->getUserSceneData().getOnlineMapID() != dwMapID)
      continue;

    pUser->setOwnWeather(dwWeather);
  }
}

void SceneUserManager::updateSky(DWORD dwMapID, DWORD dwSky, DWORD sec)
{
  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(m->second);
    if (pUser == nullptr || pUser->isOwnSky() == true || pUser->getUserSceneData().getOnlineMapID() != dwMapID)
      continue;

    pUser->setOwnSky(dwSky, false, sec);
  }
}

void SceneUserManager::setSvrMaxBaseLv(SceneUser* pUser)
{
  if (pUser == nullptr || pUser->getUserSceneData().getRolelv() <= m_dwSvrMaxBaseLv)
    return;

  DWORD dwOldMaxLv = m_dwSvrMaxBaseLv;
  m_dwSvrMaxBaseLv = pUser->getUserSceneData().getRolelv();

  stringstream sstr;
  sstr << thisServer->getZoneID();
  xLuaData oData;
  oData.setData(sstr.str(), m_dwSvrMaxBaseLv);
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_MAX_BASELV, "data");
  if (RedisManager::getMe().setHash(key, oData) == false)
  {
    XERR << "[玩家管理-最大等级]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "设置 :" << sstr.str() << "最大等级失败" << XEND;
    return;
  }

  XLOG << "[玩家管理-最大等级]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "刷新把服务器最高等级从" << dwOldMaxLv << "级刷新到" << m_dwSvrMaxBaseLv << "级" << XEND;
}

void SceneUserManager::updateGlobalBoss(const DeadBossInfo& rInfo)
{
  if (GlobalManager::getMe().updateGlobalBoss(rInfo) == false)
    return;

  struct NtfBack : public xEntryCallBack
  {
    virtual bool exec(xEntry *e)
    {
      SceneUser* pUser = dynamic_cast<SceneUser*>(e);
      if (pUser != nullptr)
        pUser->getUserSceneData().notifyDeadBoss();
      return true;
    }
  };
  NtfBack ntfback;
  forEach(ntfback);
}

bool SceneUserManager::loadSvrMaxBaseLv()
{
  stringstream sstr;
  sstr << thisServer->getZoneID();
  xLuaData oData;
  oData.setData(sstr.str(), "");
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_MAX_BASELV, "data");
  if (RedisManager::getMe().getHash(key, oData) == false)
  {
    XERR << "[玩家管理-最大等级] 获取 :" << sstr.str() << "最大等级失败" << XEND;
    return false;
  }

  m_dwSvrMaxBaseLv = oData.getTableInt(sstr.str());
  XLOG << "[玩家管理-最大等级] 获取 :" << sstr.str() << "最大等级成功 :" << m_dwSvrMaxBaseLv << XEND;
  return true;
}

void SceneUserManager::login(SceneUserDataLoad *pData)
{
  if (!pData) return;

  RecordUserData &data = pData->getRecordUserData();

  SceneUser *pUser = getLoginUserByID(data.base().charid());
  if (pUser == nullptr)
  {
    XERR << "[登录失败]" << data.base().charid() << "LoginUser管理中找不到玩家" << XEND;
    thisServer->logErr(NULL, data.base().charid(), NULL, REG_ERR_SET_USER_DATA_SCENE, false);
    return;
  }

  if (pData->m_blRet == false)
  {
    delLoginUser(pUser);
    thisServer->logErr(pUser, pUser->id, pUser->name, REG_ERR_SET_USER_DATA_SCENE, false);
    return;
  }

  Scene* pScene = SceneManager::getMe().getSceneByID(data.base().mapid());
  if (pScene == nullptr)
  {
    XERR << "[登录]" << pUser->id << pUser->name << "找不到目标地图" << data.base().mapid() << "登录指定地图" << pUser->getUserMap().getLoginMapID() << XEND;
    pScene = SceneManager::getMe().getSceneByID(pUser->getUserMap().getLoginMapID());
    if (pScene == nullptr)
    {
      delLoginUser(pUser);
      thisServer->logErr(pUser, pUser->id, pUser->name, REG_ERR_ENTER_SCENE, false);
      return;
    }
    else
    {
      UserBaseData *pBase = data.mutable_base();
      pBase->set_mapid(pUser->getUserMap().getLoginMapID());
    }
  }

  pUser->m_blThreadLoad = false;
  delLoginUser(pUser);
  if (!addUser(pUser))
  {
    XERR << "[登录失败]" << data.base().charid() << "管理中添加失败" << XEND;
    thisServer->logErr(pUser, pUser->id, pUser->name, REG_ERR_SET_USER_DATA_SCENE, true);
    return;
  }

  // check nologin
  DWORD dwAccNologinTime = data.acc().nologintime();
  DWORD dwCharNologinTime = data.base().nologintime();
  DWORD dwNow = now();
  if (dwAccNologinTime > dwNow || dwCharNologinTime > dwNow)
  {
    XERR << "[登录失败]" << data.base().charid() << "被封停,封停时间 acc :" << dwAccNologinTime << "char :" << dwCharNologinTime << XEND;
    thisServer->logErr(pUser, pUser->id, pUser->name, REG_ERR_FORBID_REG, true);
    return;
  }

  if (pUser->initChar(pData) == false)
  {
    XERR << "[登录失败]" << data.base().charid() << "initChar失败" << XEND;
    thisServer->logErr(pUser, pUser->id, pUser->name, REG_ERR_SET_USER_DATA_SCENE, true);
    return;
  }
}

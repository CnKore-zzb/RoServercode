#include "SceneAct.h"
#include "Scene.h"
#include "SceneUser.h"
#include "xTime.h"
#include "xSceneEntryDynamic.h"
#include "SceneNpcManager.h"
#include "SceneUserManager.h"
#include "SceneNpc.h"
#include "SceneMap.pb.h"
#include "SceneActManager.h"
#include "SessionWeather.pb.h"

SceneActBase::SceneActBase(QWORD tempid, Scene* scene, DWORD range, xPos pos, QWORD masterID)
{
  set_tempid(tempid);
  set_id(tempid);
  setScene(scene);
  setPos(pos);


  m_dwRange = range;
  m_dwMasterID = masterID;
  m_dwType = EACTTYPE_MIN;
  XLOG << "[SceneAct]" << tempid << masterID << "创建,(" << getPos().x << getPos().y << getPos().z << ")" << XEND;
}

SceneActBase::~SceneActBase()
{
  //XLOG("[SceneAct],%llu,析构,(%f,%f,%f)", tempid, getPos().x, getPos().y, getPos().z);
}

void SceneActBase::delMeToNine()
{
  if (!getScene())
    return;

  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneActBase::sendMeToNine()
{
  if (!getScene())
    return;

  Cmd::AddMapAct cmd;
  fillMapActData(cmd.add_acts());

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneActBase::fillMapActData(Cmd::MapAct* data)
{
  if (!data) return;

  data->set_id(tempid);
  data->set_type(m_dwType);
  data->set_range(m_dwRange);
  data->set_masterid(m_dwMasterID);

  ScenePos *p = data->mutable_pos();
  p->set_x(getPos().getX());
  p->set_y(getPos().getY());
  p->set_z(getPos().getZ());

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(m_dwMasterID);
  if (!pNpc)
    return;
  data->set_actvalue(pNpc->define.getPurify());
}

bool SceneActBase::enterScene(Scene* scene)
{
  if (!scene) return false;

  setScene(scene);
  if (getScene()->addEntryAtPosI(this))
  {
    sendMeToNine();
    return true;
  }
  return false;
}

void SceneActBase::leaveScene()
{
  if (!getScene())
    return;

  getScene()->delEntryAtPosI(this);
  delMeToNine();
}

xSceneEntryDynamic* SceneActBase::getMaster()
{
  return xSceneEntryDynamic::getEntryByID(m_dwMasterID);
}

void SceneActBase::setClearState()
{
  if (!m_blNeedClear)
  {
    leaveScene();
    m_blNeedClear = true;
  }
}

void SceneActBase::delMeToUser(SceneUser* pUser)
{
  if (pUser == nullptr || getScene() == nullptr)
    return;
  if (getScene()->check2PosInNine(getPos(), pUser->getPos()) == false)
    return;
  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void SceneActBase::delMeToUser(QWORD userid)
{
  if (!getScene())
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr)
    return;
  delMeToUser(pUser);
}

void SceneActBase::sendMeToUser(SceneUser* pUser)
{
  if (pUser == nullptr || getScene() == nullptr)
    return;
  if (getScene()->check2PosInNine(getPos(), pUser->getPos()) == false)
    return;
  Cmd::AddMapAct cmd;
  fillMapActData(cmd.add_acts());

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
}

void SceneActBase::sendMeToUser(QWORD userid)
{
  if (!getScene())
    return;

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
  if (pUser == nullptr)
    return;
  sendMeToUser(pUser);
}

void SceneActBase::timer(DWORD cur)
{
  if (m_dwClearTime && cur >= m_dwClearTime)
    setClearState();
}

bool SceneActBase::checkUserInRange(SceneUser* user)
{
  Scene* scene = getScene();
  if (scene == nullptr || user->getScene() != scene)
    return false;
  return checkDistance(getPos(), user->getPos(), m_dwRange);
}

void SceneActBase::getRangeUser(xSceneEntrySet& uset)
{
  Scene* scene = getScene();
  if (!scene)
    return;
  scene->getEntryListInBlock(SCENE_ENTRY_USER, getPos(), m_dwRange, uset);
}

/*-------------------------------------------------------*/
/*-------------------------------------------------------*/
/*-------------------------------------------------------*/

SceneActPurify::SceneActPurify(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD masterID)
  :SceneActBase(tempid, scene, range, pos, masterID)
{
  m_dwType = EACTTYPE_PURIFY;
  init();
}

SceneActPurify::~SceneActPurify()
{

}

void SceneActPurify::init()
{
  if (!getScene())
    return;
  xSceneEntrySet uSet;
  getScene()->getAllEntryList(SCENE_ENTRY_USER, uSet);
  for (auto s = uSet.begin(); s != uSet.end(); ++s)
  {
    m_vecPurifyUserIDs.push_back((*s)->id);
  }
}

bool SceneActPurify::viewByUser(QWORD userid)
{
  auto iter = find(m_vecPurifyUserIDs.begin(), m_vecPurifyUserIDs.end(), userid);
  return iter != m_vecPurifyUserIDs.end();
}

void SceneActPurify::sendMeToNine()
{
  if (!getScene())
    return;

  Cmd::AddMapAct cmd;
  fillMapActData(cmd.add_acts());

  PROTOBUF(cmd, send, len);
  //getScene()->sendCmdToNine(getPos(), send, len);
  for (auto v = m_vecPurifyUserIDs.begin(); v != m_vecPurifyUserIDs.end(); ++v)
  {
    xSceneEntryDynamic* pEntry = xSceneEntryDynamic::getEntryByID(*v);
    if (!pEntry || !pEntry->getScene())
      continue;
    pEntry->sendCmdToMe(send, len);
  }
}

bool SceneActPurify::purifyByUser(QWORD userid)
{
  auto iter = find(m_vecPurifyUserIDs.begin(), m_vecPurifyUserIDs.end(), userid);
  if (iter == m_vecPurifyUserIDs.end())
    return false;
  m_vecPurifyUserIDs.erase(iter);

  if (m_vecPurifyUserIDs.empty())
    setClearState();
  else
    delMeToUser(userid);
  return true;
}

void SceneActPurify::setPrivateUser(QWORD userid)
{
  m_vecPurifyUserIDs.clear();
  m_vecPurifyUserIDs.push_back(userid);
}


SceneActSeal::SceneActSeal(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master)
  :SceneActBase(tempid, scene, range, pos, master)
{
  m_dwType = EACTTYPE_SEAL;
  setTeam(master);
}

SceneActSeal::~SceneActSeal()
{

}

void SceneActSeal::setTeam(QWORD guid)
{
  m_qwTeamID = guid;
}

void SceneActSeal::setPrivateUser(SceneUser* pUser)
{
  if (!pUser) return;
  m_qwUserID = pUser->id;
}

void SceneActSeal::delMeToNine()
{
  if (!getScene() || (!m_qwTeamID && !m_qwUserID))
    return;

  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  if (m_qwUserID)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_qwUserID);
    if (pUser == nullptr || pUser->getScene() != getScene())
      return;
    pUser->sendCmdToMe(send, len);
    return;
  }
  if (m_qwTeamID)
  {
    xSceneEntrySet set;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, this->getPos(), set);
    for (auto &iter : set)
    {
      if (((SceneUser*)iter)->getTeamID() == m_qwTeamID)
        ((SceneUser *)iter)->sendCmdToMe(send, len);
    }
  }
}

void SceneActSeal::sendMeToNine()
{
  if (!getScene() || (!m_qwTeamID && !m_qwUserID))
    return;

  Cmd::AddMapAct cmd;
  fillMapActData(cmd.add_acts());
  PROTOBUF(cmd, send, len);

  if (m_qwUserID)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(m_qwUserID);
    if (pUser == nullptr || pUser->getScene() != getScene())
      return;
    pUser->sendCmdToMe(send, len);
    return;
  }
  if (m_qwTeamID)
  {
    xSceneEntrySet set;
    getScene()->getEntryListInNine(SCENE_ENTRY_USER, this->getPos(), set);
    for (auto &iter : set)
    {
      if (((SceneUser*)iter)->getTeamID() == m_qwTeamID)
        ((SceneUser *)iter)->sendCmdToMe(send, len);
    }
  }
}

bool SceneActSeal::viewByUser(QWORD userid)
{
  if (!getScene() || (!m_qwTeamID && !m_qwUserID))
    return false;
  if (m_qwUserID)
  {
    return m_qwUserID == userid;
  }
  if (m_qwTeamID)
  {
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(userid);
    if (pUser && pUser->getTeamID() == m_qwTeamID)
      return true;
  }
  return false;
}


SceneActMusic::SceneActMusic(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD masterID)
  :SceneActBase(tempid, scene, range, pos, masterID)
{
  m_dwType = EACTTYPE_MUSIC;
}

SceneActMusic::~SceneActMusic()
{

}

/************************************************************************/
/*SceneActEffect                                                                      */
/************************************************************************/
SceneActEffect::SceneActEffect(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD masterID)
  :SceneActBase(tempid, scene, range, pos, masterID)
{
  m_dwType = EACTTYPE_EFFECT;
}

SceneActEffect::~SceneActEffect()
{
}

void SceneActEffect::delMeToNine()
{
  Cmd::EffectUserCmd cmd;
  cmd.set_opt(EEFFECTOPT_DELETE);
  cmd.set_effecttype(EEFFECTTYPE_SCENEEFFECT);
  cmd.set_id(id);
 
  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
  XDBG << "[场景-特效] 九屏删除 charid" << m_qwOwnerId << "id" << id << "路径" << m_strPath << "cleartime" << m_dwExpireTime << XEND;

}

void SceneActEffect::sendMeToNine()
{
  xPos pos = getPos();
  if (!m_oCmd.has_id())
  {
    m_oCmd.set_opt(EEFFECTOPT_PLAY);
    m_oCmd.set_effecttype(EEFFECTTYPE_SCENEEFFECT);
    m_oCmd.set_id(id);
    m_oCmd.set_effect(m_strPath);
    m_oCmd.set_times(0);
    m_oCmd.set_dir(m_dwDir);
    Cmd::ScenePos *spos = m_oCmd.mutable_pos();
    if (!spos)
      return;
    spos->set_x(pos.getX());
    spos->set_y(pos.getY());
    spos->set_z(pos.getZ());
    m_oCmd.set_epbind(false);
    m_oCmd.set_posbind(true);
  } 

  PROTOBUF(m_oCmd, send, len);
  getScene()->sendCmdToNine(pos, send, len);
  XDBG << "[场景-特效] 九屏添加 charid" << m_qwOwnerId << "id" << id << "路径" << m_strPath << "cleartime" << m_dwExpireTime << XEND;
}

void SceneActEffect::timer(DWORD cur)
{
  if (cur >= m_dwExpireTime)
    setClearState();
}

void SceneActEffect::setEffectInfo(string& path, DWORD duration, QWORD ownerId, DWORD dwDir)
{
  m_strPath = path; m_dwExpireTime = now() + duration; m_qwOwnerId = ownerId; 
  SceneActManager::getMe().addEffectCount(ownerId);
  m_dwDir = dwDir;
}

void SceneActEffect::sendMeToUser(SceneUser* pUser)
{
  if (pUser == nullptr || getScene() == nullptr)
    return;
  if (getScene()->check2PosInNine(getPos(), pUser->getPos()) == false)
    return;

  PROTOBUF(m_oCmd, send, len);
  pUser->sendCmdToMe(send, len);
  XDBG << "[场景-特效] 发给玩家 charid" << pUser->id <<"owner" <<m_qwOwnerId << "id" << id << "路径" << m_strPath << "cleartime" << m_dwExpireTime << XEND;
}

SceneActEvent::SceneActEvent(QWORD tempid, Scene* scene, DWORD range, xPos pos, QWORD master) : SceneActBase(tempid, scene, range, pos, master)
{
  m_dwType = EACTTYPE_SCENEEVENT;
}

SceneActEvent::~SceneActEvent()
{

}

void SceneActEvent::sendExtraDataToNine()
{
  xSceneEntrySet userset;
  getRangeUser(userset);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user)
      onUserIn(user);
  }
}

void SceneActEvent::removeExtraDataToNine()
{
  xSceneEntrySet userset;
  getRangeUser(userset);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (user)
      onUserOut(user);
  }
}

void SceneActEvent::sendMeToNine()
{
  SceneActBase::sendMeToNine();
  sendExtraDataToNine();
}

void SceneActEvent::delMeToNine()
{
  SceneActBase::delMeToNine();
  removeExtraDataToNine();
}

void SceneActEvent::onUserIn(SceneUser* user)
{
  if (user == nullptr)
    return;

  switch(m_eEventType)
  {
    case SCENE_EVENT_MIN:
      break;
    case SCENE_EVENT_BGM:
      {
        if (m_strBgm.empty())
          break;
        ChangeBgmCmd cmd;
        cmd.set_bgm(m_strBgm);
        cmd.set_type(EBGM_TYPE_ACTIVITY);
        cmd.set_play(true);
        PROTOBUF(cmd, send, len);

        user->sendCmdToMe(send, len);
      }
      break;
    case SCENE_EVENT_SKY:
      {
        SkyChange cmd;
        cmd.set_id(m_dwSkyID);
        PROTOBUF(cmd, send, len);

        user->sendCmdToMe(send, len);
      }
      break;
    case SCENE_EVENT_WEATHER:
      {
        WeatherChange cmd;
        cmd.set_id(m_dwWeatherID);
        PROTOBUF(cmd, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    case SCENE_EVENT_FRAME:
      if (user->getScene())
      {
        XERR << "[场景-相框进入]" << user->accid << user->id << user->getProfession() << user->name << "进入相框同步失败" << XEND;
        break;
      }
      user->getScene()->sendPhotoWall(user);
      break;
    default:
      break;
  }
}

void SceneActEvent::onUserOut(SceneUser* user)
{
  if (user == nullptr)
    return;

  switch(m_eEventType)
  {
    case SCENE_EVENT_MIN:
      break;
    case SCENE_EVENT_BGM:
      {
        ChangeBgmCmd cmd;
        cmd.set_bgm(m_strBgm);
        cmd.set_type(EBGM_TYPE_ACTIVITY);
        cmd.set_play(false);
        PROTOBUF(cmd, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    case SCENE_EVENT_SKY:
      {
        SkyChange cmd;
        cmd.set_id(getScene()->getSky());
        PROTOBUF(cmd, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    case SCENE_EVENT_WEATHER:
      {
        WeatherChange cmd;
        cmd.set_id(getScene()->getWeather());
        PROTOBUF(cmd, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    default:
      break;
  }
}

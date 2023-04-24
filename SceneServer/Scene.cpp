#include "Scene.h"
#include "SceneNpcManager.h"
#include "SceneUserManager.h"
#include "TableManager.h"
#include "SceneUser.h"
#include "xLuaTable.h"
#include "SceneNpc.h"
#include "SceneTrap.h"
#include "SceneAct.h"
#include "SceneItem.h"
//#include "QuestScene.h"
#include "SceneServer.h"
#include "SceneManager.h"
#include "MsgManager.h"
#include "DScene.h"
#include "TreasureConfig.h"
#include "RedisManager.h"
#include "CommonConfig.h"
#include "MiscConfig.h"
#include "GuildRaidConfig.h"
#include "SceneActManager.h"
#include "SceneWeddingMgr.h"
#include "ActivityManager.h"

// exit point
bool ExitPoint::isVisible(SceneUser *user, Scene *scene) const
{
  if (!user) return false;
  if (!scene) scene = user->getScene();
  if (!scene) return false;

  if (MapConfig::getMe().isRaidMap(m_intNextMapID))
  {
    // gvg入口
    const SGuildCityCFG* pCityCFG = GuildRaidConfig::getMe().getGuildCityCFGByRaid(m_intNextMapID);
    if (pCityCFG && pCityCFG->bOpen == false)
      return false;
  }

  if (m_dwPrivate)
  {
    return user->m_oGear.isVisible(scene->getMapID(), m_dwExitID, m_dwVisible);
  }
  else
  {
    DWORD visible = 0;
    if (scene->m_oExit.get(m_dwExitID, visible))
      return visible!=0;

    return m_dwVisible != 0;
  }

  return false;
}

// Seat
void Seat::seatDown(QWORD charId) 
{
  m_seatUser = charId;
  m_dwSeatDownTime = now();
}

void Seat::seatUp(QWORD charId)
{
  if(m_dwSeatTime != 0)
  {
    SceneUser *pUser = SceneUserManager::getMe().getUserByID(charId);
    if(pUser != nullptr)
    {
      DWORD dwTime = (now() - m_dwSeatDownTime) > m_dwSeatTime ? m_dwSeatTime : (now() - m_dwSeatDownTime);
      DWORD dwAdd = dwTime * MiscConfig::getMe().getJoyLimitCFG().dwSeatAdd;
      pUser->getUserSceneData().addJoyValue(JOY_ACTIVITY_YOYO, dwAdd);
    }
  }
  m_seatUser = 0;
  m_dwSeatDownTime = 0;

}

// object
void SceneObject::load(const xLuaData &data)
{
  clear();
  if (data.has("BornPoints"))
  {
    const xLuaData &d = data.getData("BornPoints");
    for (auto it=d.m_table.begin(); it!=d.m_table.end(); ++it)
    {
      if (it->second.has("ID") && it->second.has("position"))
      {
        const xLuaData &p = it->second.getData("position");
        SBornPoint& bp = m_oBornPoints[it->second.getTableInt("ID")];
        bp.oPos = xPos(p.getTableFloat("1"), p.getTableFloat("2"), p.getTableFloat("3"));
        bp.dwRange = it->second.getTableInt("range");
        //XLOG("[地图Object],加载BornPoints,id:%u,(%f,%f,%f)", it->second.getTableInt("ID"), p.getTableFloat("x"), p.getTableFloat("y"), p.getTableFloat("x"));
      }
    }
  }
  if (data.has("ExitPoints"))
  {
    const xLuaData &d = data.getData("ExitPoints");
    for (auto it=d.m_table.begin(); it!=d.m_table.end(); ++it)
    {
      if (it->second.has("ID"))
      {
        ExitPoint &item = m_oExitPoints[it->second.getTableInt("ID")];
        item.m_dwExitID = it->second.getTableInt("ID");
        const xLuaData &p = it->second.getData("position");
        item.m_oPos = xPos(p.getTableFloat("1"), p.getTableFloat("2"), p.getTableFloat("3"));
        item.m_flRange = it->second.getTableFloat("range");
        item.m_intNextMapID = it->second.getTableInt("next_scene_ID");
        item.m_dwNextBornPoint = it->second.getTableInt("next_scene_born_point_ID");
        item.m_dwVisible = it->second.getTableInt("visible");
        item.m_dwPrivate = it->second.getTableInt("privategear");
        item.m_dwQuestID = it->second.getTableInt("questid");
        //XLOG("[地图Object],加载ExitPoints,id:%u,(%f,%f,%f),range:%f,目标地图:%u,目标地图出生点:%u",
        //it->second.getTableInt("ID"), item.m_oPos.x, item.m_oPos.y, item.m_oPos.z, item.m_flRange, item.m_dwNextMapID, item.m_dwNextBornPoint);
      }
    }
  }
  if (data.has("NPCPoints"))
  {
    const xLuaData &d = data.getData("NPCPoints");
    for (auto it=d.m_table.begin(); it!=d.m_table.end(); ++it)
    {
      if (it->second.has("id"))
      {
        SceneNpcTemplate tmp;
        tmp.m_dwNum = it->second.getTableInt("num");
        tmp.m_oDefine.load(it->second);
        m_oNpcList.push_back(tmp);
        //XLOG("[地图Object],加载NpcPoints,id:%u,count:%u", it->second.getTableInt("id"), tmp.m_dwNum);
      }
    }
  }
  if (data.has("RaidNPCPoints"))
  {
    const xLuaData &d = data.getData("RaidNPCPoints");
    for (auto it=d.m_table.begin(); it!=d.m_table.end(); ++it)
    {
      if (it->second.has("id"))
      {
        SceneNpcTemplate tmp;
        tmp.m_dwNum = it->second.getTableInt("num");
        tmp.m_oDefine.load(it->second);
        if (tmp.m_oDefine.getUniqueID())
        {
          m_oRaidNpcList[tmp.m_oDefine.getUniqueID()] = tmp;
        }
        //XLOG("[地图Object],加载RaidNPCPoints,index:%u,id:%u,count:%u", index, it->second.getTableInt("id"), tmp.m_dwNum);
      }
    }
  }
}

const ExitPoint* SceneObject::getExitPoint(DWORD id) const
{
  auto it = m_oExitPoints.find(id);
  if (it==m_oExitPoints.end()) return NULL;

  return &(it->second);
}

vector<xPos> SceneObject::getNpcPos() const
{
  vector<xPos> ret;
  for (auto &v : m_oNpcList)
  {
    DWORD id = v.m_oDefine.getID();
    if (isMonster(id))
      continue;
    v.m_oDefine.getPos();
    ret.push_back(v.m_oDefine.getPos());
  }
  return ret;
}

const xPos* SceneObject::getBornPoint(DWORD id) const
{
  auto it = m_oBornPoints.find(id);
  if (it == m_oBornPoints.end())
    return nullptr;

  return &(it->second.oPos);
}

const SBornPoint* SceneObject::getSBornPoint(DWORD id) const
{
  auto it = m_oBornPoints.find(id);
  if (it == m_oBornPoints.end())
    return nullptr;

  return &(it->second);
}

const SceneNpcTemplate* SceneObject::getNpcTemplate(DWORD dwID) const
{
  for (auto &s : m_oNpcList)
  {
    if (s.m_oDefine.getID() == dwID)
      return &s;
  }

  return nullptr;
}

const SceneNpcTemplate* SceneObject::getRaidNpcTemplate(DWORD dwID) const
{
  auto m = m_oRaidNpcList.find(dwID);
  if (m != m_oRaidNpcList.end())
    return &m->second;

  return nullptr;
}

// base
SceneBase::SceneBase()
{

}

SceneBase::~SceneBase()
{

}

bool SceneBase::init(const SMapCFG* pCFG)
{
  if (pCFG == nullptr)
    return false;

  m_pCFG = pCFG;
  m_oInfo.id = m_pCFG->dwID;
  strncpy(m_oInfo.name, m_pCFG->strNameZh.c_str(), MAX_NAMESIZE);
  strncpy(m_oInfo.file, m_pCFG->strNameEn.c_str(), MAX_NAMESIZE);
  return true;
}

bool SceneBase::loadFlag(xLuaData& data)
{
  m_mapFlagCFG.clear();
  auto flagf = [&](const string& key, xLuaData& data)
  {
    DWORD dwFlag = data.getTableInt("strongHoldId");
    SFlag& rFlag = m_mapFlagCFG[dwFlag];

    xLuaData& pos = data.getMutableData("position");
    xPos oPos;
    oPos.x = pos.getTableFloat("1");
    oPos.y = pos.getTableFloat("2");
    oPos.z = pos.getTableFloat("3");
    rFlag.listPos.push_back(oPos);
  };
  data.foreach(flagf);

  if (m_mapFlagCFG.empty() == false)
  {
    XLOG << "[场景-旗帜]" << m_pCFG->dwID << "包含旗帜";
    for (auto &m : m_mapFlagCFG)
    {
      XLOG << "flagid :" << m.first << "pos :";
      for (auto &l : m.second.listPos)
        XLOG << "(" << l.x << l.y << l.z << ")";
    }
    XLOG << XEND;
  }
  return true;
}

bool SceneBase::loadFrame(xLuaData& data)
{
  m_mapFrameCFG.clear();

  EFrameType eType = EFRAMETYPE_MIN;
  auto framef = [&](const string& key, xLuaData& data)
  {
    DWORD dwFrameID = atoi(key.c_str());

    SFrame& rFrame = m_mapFrameCFG[dwFrameID];
    rFrame.dwFrameID = dwFrameID;
    rFrame.eType = eType;
    rFrame.oPos.x = data.getTableInt("1");
    rFrame.oPos.y = data.getTableInt("2");
    rFrame.oPos.z = data.getTableInt("3");
  };

  if (data.has("GuildPhotoFrames") == true)
  {
    eType = EFRAMETYPE_GUILD;
    data.getMutableData("GuildPhotoFrames").foreach(framef);
  }
  if (data.has("WeddingPhotoFrames") == true)
  {
    eType = EFRAMETYPE_WEDDING;
    data.getMutableData("WeddingPhotoFrames").foreach(framef);
  }

  if (m_mapFrameCFG.empty() == false)
  {
    XLOG << "[场景-相框]" << m_pCFG->dwID << "包含相框";
    for (auto &m : m_mapFrameCFG)
      XLOG << "id :" << m.first << "type :" << m.second.eType << "pos :(" << m.second.oPos.x << m.second.oPos.y << m.second.oPos.z << ")";
    XLOG << XEND;
  }
  return true;
}

bool SceneBase::loadNoFlyPoints(xLuaData& data)
{
  m_oInfo.vecNoFly.clear();

  auto noflyf = [&](const string& key, xLuaData& data)
  {
    NoFlyRange stNoFly;

    xLuaData& pos = data.getMutableData("position");
    stNoFly.x = pos.getTableInt("1");
    stNoFly.z = pos.getTableInt("3");

    xLuaData& size = data.getMutableData("size");
    stNoFly.width = size.getTableInt("1");
    stNoFly.length = size.getTableInt("2");

    m_oInfo.vecNoFly.push_back(stNoFly);
  };
  data.foreach(noflyf);

  XDBG << "[地图-禁止] mapid :" << getMapID();
  for (auto &v : m_oInfo.vecNoFly)
    XDBG << "forbid :" << v.x << v.width << v.z << v.length;
  XDBG << XEND;
  return true;
}

const SceneObject* SceneBase::getSceneObject(DWORD raid) const
{
  if (raid)
  {
    auto it = m_oObjectList.find(raid);
    if (it!=m_oObjectList.end())
      return &it->second;
    return NULL;
  }
  else
  {
    if (isPvPMap())
    {
      return &m_oObjectPVP;
    }
    return &m_oObject;
  }
  return NULL;
}

const SFrame* SceneBase::getFrameCFG(DWORD dwFrameID) const
{
  auto m = m_mapFrameCFG.find(dwFrameID);
  if (m != m_mapFrameCFG.end())
    return &m->second;
  return nullptr;
}

Seat* SceneBase::getSeat(DWORD seatId) const
{
  auto it = m_oSeats.find(seatId);
  if (it == m_oSeats.end())
    return nullptr;

  return const_cast<Seat*>(&it->second);
}

const SFlag* SceneBase::getFlag(DWORD dwFlagID) const
{
  auto m = m_mapFlagCFG.find(dwFlagID);
  if (m != m_mapFlagCFG.end())
    return &m->second;
  return nullptr;
}

// scene
Scene::Scene(DWORD sID, const char* sName, const SceneBase *sBase):
  base(sBase)
  ,m_oGear(this)
  ,m_oExit(this)
  ,m_oImages(this)
  ,m_oTreasure(this, TreasureConfig::getMe().getTreasureCFG(sID))
  ,m_oOneSecTimer(1)
  ,m_oFiveSecTimer(5)
{
  set_id(sID);
  set_name(sName);
  m_dwMapID = sID;
}

Scene::~Scene()
{
  xSceneEntrySet set;
  getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto &it : set)
  {
    SceneNpc* pNpc = (SceneNpc *)(it);
    pNpc->removeAtonce();
    XDBG << "[场景-析构]" << id << name << "npc" << pNpc->id << pNpc->getNpcID() << XEND;
  }
  set.clear();
  getAllEntryList(SCENE_ENTRY_TRAP, set);
  for (auto &it : set)
  {
    SceneTrap *p = (SceneTrap *)(it);
    p->setClearState();
    XDBG << "[场景-析构]" << id << name << "trap" << XEND;
  }
  set.clear();
  getAllEntryList(SCENE_ENTRY_ITEM, set);
  for (auto &it : set)
  {
    SceneItem *p = (SceneItem *)(it);
    p->setClearState();
    XDBG << "[场景-析构]" << id << name << "item" << XEND;
  }
  set.clear();
  getAllEntryList(SCENE_ENTRY_ACT, set);
  for (auto &it : set)
  {
    SceneActBase *p = (SceneActBase *)(it);
    p->setClearState();
    XDBG << "[场景-析构]" << id << name << "act" << XEND;
  }
}

bool Scene::init()
{
  if (base == nullptr || base->getCFG() == nullptr)
    return false;

  MapInfo oInfo = base->getMapInfo();
  define.init(oInfo.range);

  initIndex();
  initNpc();

  m_stWeatherCFG = base->getCFG()->stWeatherCFG;
  m_stSkyCFG = base->getCFG()->stSkyCFG;

  if (base != nullptr)
  {
    const TMapFrameCFG& mapCFG = base->getFrameListCFG();
    for (auto &m : mapCFG)
    {
      SceneActBase* pAct = SceneActManager::getMe().createSceneAct(this, m.second.oPos, 20, 0, EACTTYPE_SCENEEVENT);
      if (pAct == nullptr)
      {
        XERR << "[场景-相框初始化]" << id << "frameid :" << m.first << "创建相框act失败" << XEND;
        continue;
      }
      SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (pAct);
      if (pActEvent == nullptr)
      {
        XERR << "[场景-相框初始化]" << id << "frameid :" << m.first << "创建相框act类型不正确" << XEND;
        SAFE_DELETE(pAct);
        continue;
      }
      pActEvent->setFrame();
      pActEvent->enterScene(this);
    }
  }
  return true;
}

void Scene::entryAction(QWORD curMSec)
{
  xSceneEntrySet activeNpcSet;
  if (!m_oActiveScreenSet.empty())
  {
    //ExecutionTime_Scope_NoAdd;
    for (auto &iter : m_oActiveScreenSet)
    {
      getEntryListInScreen(SCENE_ENTRY_NPC, iter, activeNpcSet);
    }
    m_oActiveScreenSet.clear();
  }

  if (!activeNpcSet.empty())
  {
    //ExecutionTime_Scope_NoAdd;
    xSceneEntrySet clearNpcSet;
    DWORD group = SceneNpcManager::getMe().getGroup();
    for (auto &iter : activeNpcSet)
    {
      SceneNpc *npc = dynamic_cast<SceneNpc *>(iter);
      DWORD base = 2;
      if (npc->isBoss())
        base = 1;
      QWORD refreshid = npc->define.m_oVar.m_qwNpcOwnerID ? npc->define.m_oVar.m_qwNpcOwnerID : npc->tempid;/*boss招的小怪同一帧刷新*/
      if (0 == (group + refreshid) % (NPC_GROUP / base))
      {
        if (npc->getStatus() == ECREATURESTATUS_CLEAR)
        {
          clearNpcSet.insert(iter);
        }
        else
        {
          npc->refreshMe(curMSec);
        }
      }
    }
    if (!clearNpcSet.empty())
    {
      for (auto &iter : clearNpcSet)
      {
        delEntryAtPosI(iter);
        SceneNpcManager::getMe().delNpc((SceneNpc *)iter);
      }
    }
  }

  if (m_mapFramePhoto.empty() == false)
    refreshPhotoWall(curMSec / ONE_THOUSAND);
}

bool Scene::isPVPScene()
{
  if(getSceneBase() == nullptr)
    return false;

  return getSceneBase()->isPvPMap();
}

const SceneObject* Scene::getSceneObject()
{
  if (isDScene())
  {
    DScene* pScene = dynamic_cast<DScene*>(this);
    if (pScene == nullptr)
      return nullptr;

    if (!m_oImages.isImageScene())
    {
      return pScene->base->getSceneObject(pScene->getRaidID());
    }
    else
    {
      return pScene->base->getSceneObject(pScene->getRaidID()) ? pScene->base->getSceneObject(pScene->getRaidID()) : base->getSceneObject(0);
    }
  }

  return base->getSceneObject(0);
}

bool Scene::initNpc()
{
  if (VIRTUAL_SCENE_ID == getMapID())
    return true;
  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return false;

  const list<SceneNpcTemplate>& npclist = pObject->getNpcList();
  for (auto &l : npclist)
  {
    if (!hideNpc(l.m_oDefine.getID()))
      SceneNpcManager::getMe().createNpc(this, l.m_oDefine, l.m_dwNum);
  }

  return true;
}

bool Scene::findingPath(xPos f, xPos t, std::list<xPos> &path, ToolMode mode)
{
  path.clear();
  if (base->getPathFinding())
    return base->getPathFinding()->finding(f, t, path, mode);
  return false;
  /*
  XLOG("[寻路],from:(%f,%f,%f),target:(%f,%f,%f)", f.x, f.y, f.z, t.x, t.y, t.z);
  int i = 1;
  for (auto iter=path.begin(); iter!=path.end(); ++iter)
  {
    XLOG("[寻路],step:%d,(%f,%f,%f)", i++, iter->x, iter->y, iter->z);
  }
  */
}

bool Scene::addQuestsAllUser(TVecDWORD& vecQuests)
{
  xSceneEntrySet eSet;
  getAllEntryList(SCENE_ENTRY_USER, eSet);
  for(auto &v : eSet)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(v);
    if(!pUser)
      continue;

    for(auto &q : vecQuests)
      pUser->getQuest().acceptQuest(q, true);
  }

  return true;
}

bool Scene::delQuestsAllUser(TVecDWORD& vecQuests)
{
  xSceneEntrySet eSet;
  getAllEntryList(SCENE_ENTRY_USER, eSet);
  for(auto &v : eSet)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(v);
    if(!pUser)
      continue;

    for(auto &q : vecQuests)
      pUser->getQuest().abandonGroup(q, true);
  }

  return true;
}

void Scene::kickAllUser(bool bForceAll /*=false*/)
{
  auto kickusers = [&](xSceneEntrySet& eSet)
  {
    xSceneEntrySet::iterator it=eSet.begin(),end=eSet.end();
    for (;it!=end;it++)
    {
      if(!(*it)) continue;
      if((*it)->getEntryType()!=SCENE_ENTRY_USER) continue;
      addKickList((SceneUser *)(*it));
    }
  };

  if (bForceAll)
  {
    xSceneEntrySet eSet;
    getAllEntryList(SCENE_ENTRY_USER, eSet);
    kickusers(eSet);
  }
  else
  {
    DWORD cur = now();
    if (m_dwKickUserTimeTick && cur < m_dwKickUserTimeTick)
      return;
    xSceneEntrySet eSet;
    getAllEntryList(SCENE_ENTRY_USER, eSet);
    if (eSet.size() <= CommonConfig::m_dwOneSecMaxKickUserNum)
    {
      kickusers(eSet);
    }
    else
    {
      xSceneEntrySet kickset;
      auto it = eSet.begin();
      std::advance(it, CommonConfig::m_dwOneSecMaxKickUserNum);
      kickset.insert(eSet.begin(), it);
      kickusers(kickset);
      m_dwKickUserTimeTick = cur + 1;
    }
  }
}

void Scene::kickUser(SceneUser* user)
{
  if (!user || !user->getScene()) return;
  if (!user->isAlive())
  {
    user->relive(ERELIVETYPE_RETURN);
    return;
  }
  if (m_oImages.isImageScene())
  {
    user->gomap(getMapID(), GoMapType::Image, user->getPos());
  }
  else
  {
    DWORD tomapid;
    xPos topos(0,0,0);

    do
    {
      if (isDScene())
      {
        tomapid = base->getReliveMapID();
        const SceneBase* pBase = SceneManager::getMe().getDataByID(tomapid);
        if (!pBase)
          break;

        DWORD raidId = 0;
        if (pBase->isStaticMap())
          raidId = 0;
        else
        {
          raidId = tomapid;
          const SRaidCFG* pRaidCfg = MapConfig::getMe().getRaidCFG(tomapid);
          if (pRaidCfg && pRaidCfg->eRaidType == ERAIDTYPE_GUILD)
          {
            //进公会已经被踢了，此处处理不通用
            if (user->getGuild().id() == 0)
            {
              tomapid = user->getUserMap().getLastStaticMapID();
              topos = user->getUserMap().getLastStaticMapPos();
              break;
            }
          }          
        }
        if (isDPvpScene())
        {
          tomapid = user->getUserMap().getLastStaticMapID();
          topos = user->getUserMap().getLastStaticMapPos();
          break;
        }

        const SceneObject* pObject = pBase->getSceneObject(raidId);
        if (pObject != nullptr)
        {
          const xPos* pPos = pObject->getBornPoint(base->getReliveBp());
          if (pPos)
            topos = *pPos;
        }
      }
      else
      {
        tomapid = user->getUserSceneData().getLastMapID();
      }
    } while (0);
    user->gomap(tomapid, GoMapType::KickUser, topos);
  }
}

void Scene::timer(QWORD curMSec)
{
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);

  m_oImages.timer(curSec);
  m_oTreasure.timer(curSec);

  // entry action
  entryAction(curMSec);

  if (m_oOneSecTimer.timeUp(curSec))
  {
    if (!m_oKickUserList.empty())
    {
      for (auto it : m_oKickUserList)
      {
        SceneUser *pUser = SceneUserManager::getMe().getUserByID(it);
        if (pUser && pUser->getScene() && (pUser->getScene() == this))
        {
          kickUser(pUser);
        }
      }
      m_oKickUserList.clear();
    }
    if (m_oFiveSecTimer.timeUp(curSec))
      onFiveSecTimeUp(curSec);
  }
}

void Scene::onFiveSecTimeUp(DWORD curSec)
{ 
  auto wetherChange = [&]()
  {
    if (m_envSetting.mode == EENVMODE_ALLDAY)
    {
      //weather change
      if (m_envSetting.weatherid)
      {
        if (m_dwWeatherId != m_envSetting.weatherid)
        {
          m_dwWeatherId = m_envSetting.weatherid;
          SceneUserManager::getMe().updateWeather(getMapID(), m_dwWeatherId);
          XLOG << "[天气变化] mapid :" << getMapID() << "mode" << m_envSetting.mode << "weather :" << m_dwWeatherId << "timer :" << m_stWeatherCFG.dwDestTime - curSec << XEND;
        }
        return;
      }      
    }

    if ((m_envSetting.mode == EENVMODE_DAY && !MiscConfig::getMe().getSystemCFG().isNight(curSec)) || (m_envSetting.mode == EENVMODE_NIGHT &&MiscConfig::getMe().getSystemCFG().isNight(curSec)))
    {
      if (m_envSetting.weatherid)
      {
        if (m_dwWeatherId != m_envSetting.weatherid)
        {
          m_dwWeatherId = m_envSetting.weatherid;
          SceneUserManager::getMe().updateWeather(getMapID(), m_dwWeatherId);
          XLOG << "[天气变化] mapid :" << getMapID() << "mode" << m_envSetting.mode << "weather :" << m_dwWeatherId << "timer :" << m_stWeatherCFG.dwDestTime - curSec << XEND;
        }
        return;
      }
    }

    if (curSec >= m_stWeatherCFG.dwDestTime)
    {
      DWORD dwRand = randBetween(0, m_stWeatherCFG.dwMaxRate);
      auto v = find_if(m_stWeatherCFG.vecItems.begin(), m_stWeatherCFG.vecItems.end(), [dwRand](const SWeatherSkyItem& r) -> bool {
        return r.dwRate >= dwRand;
      });
      if (v != m_stWeatherCFG.vecItems.end())
      {
        if (v->dwID == FINE_WEATHER_ID)
          m_stWeatherCFG.dwDestTime = curSec + v->dwMaxTime;
        else
          m_stWeatherCFG.dwDestTime = curSec + randBetween(1, v->dwMaxTime);

        m_dwWeatherId = v->dwID;
        if (!v->vecSky.size())
        {
          DWORD skyIndex = MiscConfig::getMe().getSystemCFG().getSkyTypeIndex(curSec);
          if (skyIndex < v->vecSky.size())
          {
            DWORD skyId = v->vecSky[skyIndex];
            if (canChangeSky(curSec))
            {
              changeSkyForce(skyId, (m_stWeatherCFG.dwDestTime - curSec), curSec);
              XINF << "[天空变化] 天气导致天空变化 mapid:" << getMapID() << "sky index" << skyIndex << "sky id:" << m_dwCurSkyId << XEND;
            }
          }
        }
        SceneUserManager::getMe().updateWeather(getMapID(), m_dwWeatherId);
        XLOG << "[天气变化] mapid :" << getMapID() << "weather :" << m_dwWeatherId << "timer :" << m_stWeatherCFG.dwDestTime - curSec << XEND;
      }
    }
  };

  auto skyChange = [&]()
  {
    if (m_envSetting.mode == EENVMODE_ALLDAY)
    {
      if (m_envSetting.skyid)
      {
        if (m_dwCurSkyId != m_envSetting.skyid)
        {
          m_dwCurSkyId = m_envSetting.skyid;
          SceneUserManager::getMe().updateSky(getMapID(), m_dwCurSkyId, 0);
          XINF << "[天空变化] changeSkyForce mapid:" << getMapID() << "mode" << m_envSetting.mode << "sky id:" << m_dwCurSkyId << XEND;
        }
        return;
      }
    }

    if ((m_envSetting.mode == EENVMODE_DAY && !MiscConfig::getMe().getSystemCFG().isNight(curSec))  || (m_envSetting.mode == EENVMODE_NIGHT &&MiscConfig::getMe().getSystemCFG().isNight(curSec)))
    {
      //sky change
      if (m_envSetting.skyid)
      {
        if (m_dwCurSkyId != m_envSetting.skyid)
        {
          m_dwCurSkyId = m_envSetting.skyid;
          SceneUserManager::getMe().updateSky(getMapID(), m_dwCurSkyId, 0);
          XINF << "[天空变化] changeSkyForce mapid:" << getMapID() <<"mode"<<m_envSetting.mode<< "sky id:" << m_dwCurSkyId << XEND;
        }
        return;
      }
    }

    // change sky depend on time 
    do
    {
      if (m_stSkyCFG.size() == 0)
        break;
      if (!canChangeSky(curSec))
        break;
      DWORD skyIndex = MiscConfig::getMe().getSystemCFG().getSkyTypeIndex(curSec);
      if (skyIndex >= m_stSkyCFG.size())
      {
        XERR << "[天空变化] sky index 异常, mapid:" << getMapID() << "sky index:" << skyIndex << m_stSkyCFG.size() << XEND;
        break;
      }
      DWORD skyId = m_stSkyCFG[skyIndex];
      if (m_dwCurSkyId == skyId)
        break;
      m_dwCurSkyId = skyId;
      SceneUserManager::getMe().updateSky(getMapID(), m_dwCurSkyId, MiscConfig::getMe().getSystemCFG().dwSkyPassTime);
      XINF << "[天空变化] mapid:" << getMapID() << "mode" << m_envSetting.mode << "sky index:" << skyIndex << "sky id:" << m_dwCurSkyId << XEND;
    } while (0);
  };

  wetherChange();
  skyChange();   
}

bool Scene::canChangeSky(DWORD curSec)
{
  if (m_envSetting.mode == EENVMODE_NONE)
    return true;
  if (m_envSetting.skyid == 0)
    return true;
  
  if ((m_envSetting.mode == EENVMODE_DAY && !MiscConfig::getMe().getSystemCFG().isNight(curSec)) || (m_envSetting.mode == EENVMODE_NIGHT &&MiscConfig::getMe().getSystemCFG().isNight(curSec)))
  {
    return false;
  }

  if (curSec > m_dwNextSkyAutoChangeTime)
    return true;

  return true;
}

void Scene::changeSkyForce(DWORD skyId, DWORD duration, DWORD curSec/* = 0*/)
{
  if (skyId == 0)
  {
    //clear 
    m_dwNextSkyAutoChangeTime = 0;
    return;
  }
  if (curSec == 0)
    curSec = now();
  m_dwCurSkyId = skyId;
  m_dwNextSkyAutoChangeTime = curSec + duration;
  SceneUserManager::getMe().updateSky(getMapID(), m_dwCurSkyId, 0);
  XINF << "[天空变化] changeSkyForce mapid:" << getMapID() << "sky id:" << m_dwCurSkyId << "duration:" << duration << "next change time:" << m_dwNextSkyAutoChangeTime << XEND;

}

void Scene::setBgm(Cmd::EBgmType type, bool play, DWORD times, const std::string& bgmPath)
{
  if (play && times == 0)
  {
    m_bgmPlay = true;
    m_bgmPath = bgmPath;
  }
  else
  {
    m_bgmPlay = false;
    m_bgmPath = "";
  }
  
  m_bgmType = type;
  Cmd::ChangeBgmCmd cmd;
  cmd.set_type(type);
  cmd.set_play(play);
  cmd.set_times(times);
  cmd.set_bgm(bgmPath);

  PROTOBUF(cmd, send, len);
  sendCmdToAll(send, len);
  XLOG << "[场景-bgm] bgm 设置,通知所有场景玩家"<<getMapID() <<"type"<<type<< play << times << bgmPath << XEND;
}

bool Scene::queryFrame(SceneUser* pUser, DWORD dwFrameID)
{
  if (pUser == nullptr)
    return false;

  auto m = m_mapFramePhoto.find(dwFrameID);
  if (m == m_mapFramePhoto.end())
  {
    XERR << "[场景-相框查询]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "查询相框" << dwFrameID << "失败,未找到该相框配置" << XEND;
    return false;
  }

  QueryFramePhotoListPhotoCmd cmd;
  cmd.set_frameid(dwFrameID);
  for (auto &l : m->second)
    cmd.add_photos()->CopyFrom(l);

  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  XLOG << "[场景-相框查询]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "查询相框" << dwFrameID << "成功,数据" << cmd.ShortDebugString() << XEND;
  return true;
}

bool Scene::frameAction(SceneUser* pUser, const FrameActionPhotoCmd& cmd)
{
  if (pUser == nullptr)
    return false;

  const WeddingInfo& rInfo = SceneWeddingMgr::getMe().getWeddingInfo();
  if (rInfo.charid1() != pUser->id && rInfo.charid2() != pUser->id)
  {
    MsgManager::sendDebugMsg(pUser->id, "测试log:不是会场新人");
    XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "失败,不是会场新人" << XEND;
    return false;
  }

  if (base == nullptr)
  {
    XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "失败,场景未包含正确配置" << XEND;
    return false;
  }

  const SFrame* pFrame = base->getFrameCFG(cmd.frameid());
  if (pFrame == nullptr)
  {
    XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "失败,场景未包含该相框" << XEND;
    return false;
  }
  if (pFrame->eType != EFRAMETYPE_WEDDING)
  {
    MsgManager::sendDebugMsg(pUser->id, "测试log:该相框不是结婚相框");
    XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "失败,该相框不是结婚相框" << XEND;
    return false;
  }

  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  TListGuildPhoto& listPhoto = m_mapFramePhoto[cmd.frameid()];
  TVecGuildPhoto vecRightPhoto;

  if (cmd.action() == EFRAMEACTION_UPLOAD)
  {
    if (listPhoto.size() + cmd.photos_size() > rCFG.dwMaxFramePhotoCount)
    {
      MsgManager::sendMsg(pUser->id, 999);
      return false;
    }

    for (int i = 0; i < cmd.photos_size(); ++i)
    {
      GuildPhoto oPhoto;
      oPhoto.CopyFrom(cmd.photos(i));
      if (modifyPhoto(pUser, oPhoto) == false)
      {
        XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在相框" << cmd.frameid() << "上传" << oPhoto.ShortDebugString() << "失败,照片信息设置失败" << XEND;
        continue;
      }
      auto l = find_if(listPhoto.begin(), listPhoto.end(), [&](const GuildPhoto& r) -> bool{
        return GGuild::isSame(r, oPhoto);
      });
      if (l != listPhoto.end())
      {
        XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在相框" << cmd.frameid() << "上传" << oPhoto.ShortDebugString() << "失败,该照片已存在" << XEND;
        continue;
      }
      listPhoto.push_back(oPhoto);
      vecRightPhoto.push_back(cmd.photos(i));
    }
  }
  else if (cmd.action() == EFRAMEACTION_REMOVE)
  {
    for (int i = 0; i < cmd.photos_size(); ++i)
    {
      GuildPhoto oPhoto;
      oPhoto.CopyFrom(cmd.photos(i));
      if (modifyPhoto(pUser, oPhoto) == false)
      {
        XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在相框" << cmd.frameid() << "移除" << oPhoto.ShortDebugString() << "失败,照片信息设置失败" << XEND;
        continue;
      }
      auto l = find_if(listPhoto.begin(), listPhoto.end(), [&](const GuildPhoto& r) -> bool{
        return GGuild::isSame(r, oPhoto);
      });
      if (l == listPhoto.end())
      {
        XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "在相框" << cmd.frameid() << "移除" << oPhoto.ShortDebugString() << "失败,未找到该照片" << XEND;
        continue;
      }
      listPhoto.erase(l);
      vecRightPhoto.push_back(oPhoto);
    }
  }
  else
  {
    XERR << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "失败,未知action类型" << XEND;
    return false;
  }

  FrameActionPhotoCmd ret;
  ret.CopyFrom(cmd);
  ret.clear_photos();
  for (auto &v : vecRightPhoto)
    ret.add_photos()->CopyFrom(v);
  PROTOBUF(ret, send, len);
  pUser->sendCmdToMe(send, len);
  pUser->getAchieve().onWedding(EACHIEVECOND_WEDDING_PHOTO);

  XLOG << "[场景-相框操作]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "操作相框" << cmd.ShortDebugString() << "成功,处理结果" << ret.ShortDebugString() << XEND;
  sendPhotoWall();
  return true;
}

void Scene::sendPhotoWall(SceneUser* pUser /*= nullptr*/)
{
  for (auto &m : m_mapFramePhoto)
  {
    const SFrame* pCFG = base->getFrameCFG(m.first);
    if (pCFG == nullptr)
      continue;

    UpdateFrameShowPhotoCmd cmd;
    FrameShow* pShow = cmd.add_shows();

    DWORD& rIndex = m_mapFrameIndex[m.first];
    if (rIndex >= m.second.size())
    {
      XERR << "[场景-相框展示] 场景" << id << name << "frameid :" << m.first << "同步九屏当前展示失败,index :" << rIndex << "超过相框列表大小" << m.second.size() << XEND;
      continue;
    }

    auto l = m.second.begin();
    std::advance(l, rIndex);
    if (l == m.second.end())
      continue;
    pShow->set_frameid(m.first);
    pShow->mutable_photo()->CopyFrom(*l);

    PROTOBUF(cmd, send, len);
    if (pUser == nullptr)
    {
      sendCmdToNine(pCFG->oPos, send, len);
      XLOG << "[场景-相框展示] 场景" << id << name << "frameid :" << m.first << "同步九屏当前展示" << cmd.ShortDebugString() << XEND;
    }
    else
    {
      pUser->sendCmdToMe(send, len);
      XLOG << "[场景-相框展示] 场景" << id << name << "frameid :" << m.first << "同步玩家" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "当前展示" << cmd.ShortDebugString() << XEND;
    }
  }
}

void Scene::refreshPhotoWall(DWORD curSec)
{
  if (m_mapFramePhoto.empty() == true || m_dwFrameTick > curSec)
    return;
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  m_dwFrameTick = curSec + rCFG.dwPhotoRefreshTime;

  for (auto &m : m_mapFramePhoto)
  {
    const SFrame* pCFG = base->getFrameCFG(m.first);
    if (pCFG == nullptr)
      continue;

    DWORD& rIndex = m_mapFrameIndex[m.first];
    ++rIndex;
    if (rIndex >= m.second.size())
      rIndex = 0;
  }

  sendPhotoWall();
}

void Scene::resetPhotoWall(EFrameType eType)
{
  if (eType == EFRAMETYPE_WEDDING)
  {
    UpdateFrameShowPhotoCmd cmd;
    xPos oPos;
    for (auto &m : m_mapFramePhoto)
    {
      const SFrame* pCFG = base->getFrameCFG(m.first);
      if (pCFG != nullptr)
        oPos = pCFG->oPos;
      FrameShow* pShow = cmd.add_shows();
      pShow->set_frameid(m.first);
    }
    PROTOBUF(cmd, send, len);
    sendCmdToNine(oPos, send, len);

    m_dwFrameTick = 0;
    m_mapFramePhoto.clear();

    XLOG << "[场景-相框清空] 场景" << id << name << "婚礼结束,相框清空" << XEND;
  }
}

bool Scene::modifyPhoto(SceneUser* pUser, GuildPhoto& r)
{
  if (pUser == nullptr)
    return false;

  if (r.source() == ESOURCE_PHOTO_SCENERY)
  {
    SManualItem* pItem = pUser->getManual().getManualItem(EMANUALTYPE_SCENERY);
    if (pItem == nullptr)
      return false;
    SManualSubItem* pSubItem = pItem->getSubItem(r.sourceid());
    if (pSubItem == nullptr)
      return false;
    const SceneryData* pData = pUser->getScenery().getScenery(r.sourceid());
    if (pData == nullptr)
      return false;
    r.set_accid_svr(pUser->accid);
    r.set_mapid(pData->m_dwMapID);
    r.set_anglez(atoi(pSubItem->getDataParam(0).c_str()));
    r.set_time(atoi(pSubItem->getDataParam(1).c_str()));

    if (pSubItem->getDataParam(2).empty() == false)
      r.set_charid(pUser->id);
    else
      r.set_accid(pUser->accid);
    return true;
  }
  else if (r.source() == ESOURCE_PHOTO_SELF)
  {
    PhotoItem* pItem = pUser->getPhoto().get(r.sourceid());
    if (pItem == nullptr)
      return false;
    r.set_accid_svr(pUser->accid);
    r.set_charid(pUser->id);
    r.set_anglez(pItem->anglez());
    r.set_time(pItem->time());
    r.set_mapid(pItem->mapid());
    return true;
  }

  return false;
}

void Scene::getSceneNpcByBaseID(DWORD baseid, std::list<SceneNpc *> &npclist)
{
  xSceneEntrySet set;
  this->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto &iter : set)
  {
    SceneNpc *npc = (SceneNpc *)iter;
    if (npc && npc->getNpcID()==baseid)
      npclist.push_back(npc);
  }
}

void Scene::onNpcGuidChange(QWORD qwOldID, QWORD qwNewID)
{
  auto it = m_setSummonedNpcs.find(qwOldID);
  if (it == m_setSummonedNpcs.end())
    return;
  m_setSummonedNpcs.erase(it);
  m_setSummonedNpcs.insert(qwNewID);
}

void Scene::setEnvSetting(DWORD mode, DWORD skyId, DWORD weatherId)
{
  m_envSetting.mode = static_cast<EENVMODE>(mode);
  m_envSetting.skyid = skyId;
  m_envSetting.weatherid = weatherId;
  
  if (m_envSetting.mode == EENVMODE_NONE)
  {
    m_envSetting.weatherid = 0;
    m_envSetting.skyid = 0;
  }

  //refresh weather at once
  m_stWeatherCFG.dwDestTime = 0;
  XINF << "[天空天气-设置] GM mapid:" << getMapID() << "mode" << m_envSetting.mode << "old sky id:" << m_dwCurSkyId <<"sky id"<<m_envSetting.skyid<<"old weather"<<m_dwWeatherId<<"weather id"<< m_envSetting.weatherid << XEND;
}

void Scene::sendCmdToAll(const void *cmd, WORD len)
{
  thisServer->broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE_MAP, id, cmd, len);
}

void Scene::userEnter(SceneUser *user) 
{
  if (user == nullptr)
    return;
  m_oTreasure.onEnterScene(user);

  FuBenClearInfoCmd cmd;
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);

  //set bgm
  if (m_bgmPlay)
  {
    Cmd::ChangeBgmCmd cmd;
    cmd.set_type(m_bgmType);
    cmd.set_play(m_bgmPlay);
    cmd.set_bgm(m_bgmPath);
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
    XLOG << "[场景-bgm] 玩家进入场景，设置bgm" <<user->accid <<user->id << getMapID() <<m_bgmType << m_bgmPath << XEND;
  }
  else if (getBaseCFG() && getBaseCFG()->strMapBgm.empty() == false)
  {
    Cmd::ChangeBgmCmd cmd;
    cmd.set_type(EBGM_TYPE_REPLACE);
    cmd.set_play(true);
    cmd.set_bgm(getBaseCFG()->strMapBgm);
    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }
}

void Scene::addKickList(SceneUser* user)
{
  if (user)
    m_oKickUserList.insert(user->id);
}

void Scene::addVisibleNpc(SceneNpc* pNpc)
{
  if(pNpc == nullptr)
    return;

  auto m = m_mapVisibleNpcs.find(pNpc->id);
  if(m != m_mapVisibleNpcs.end())
    return;

  SVisibleNpc stNpc;
  stNpc.uniqueid = pNpc->id;
  stNpc.npcid = pNpc->getNpcID();
  stNpc.oPos = pNpc->getPos();
  m_mapVisibleNpcs.insert(std::make_pair(pNpc->id, stNpc));

  Cmd::NtfVisibleNpcUserCmd usercmd;
  usercmd.set_type(1);
  refreshVisibleNpc(usercmd, pNpc);
  PROTOBUF(usercmd, send, len);
  sendCmdToAll(send, len);

  XLOG << "[场景-小地图可见怪物] 添加，可见怪物" << "map:" <<getMapID() << "怪物信息："<< usercmd.ShortDebugString() << XEND;
}

void Scene::delVisibleNpc(SceneNpc* pNpc)
{
  if(pNpc == nullptr)
    return;

  auto m = m_mapVisibleNpcs.find(pNpc->id);
  if(m == m_mapVisibleNpcs.end())
    return;

  m_mapVisibleNpcs.erase(m);

  Cmd::NtfVisibleNpcUserCmd usercmd;
  usercmd.set_type(0);
  refreshVisibleNpc(usercmd, pNpc);
  PROTOBUF(usercmd, send, len);
  sendCmdToAll(send, len);

  XLOG << "[场景-小地图可见怪物] 删除，可见怪物" << "map:" <<getMapID() << "怪物信息："<< usercmd.ShortDebugString() << XEND;
}

void Scene::refreshVisibleNpc(Cmd::NtfVisibleNpcUserCmd& cmd, SceneNpc* pNpc)
{
  if(pNpc == nullptr)
    return;

  Cmd::VisibleNpc* data = cmd.add_npcs();
  data->set_npcid(pNpc->getNpcID());
  data->set_uniqueid(pNpc->getGUID());
  data->mutable_pos()->set_x(pNpc->getPos().getX());
  data->mutable_pos()->set_y(pNpc->getPos().getY());
  data->mutable_pos()->set_z(pNpc->getPos().getZ());
}

void Scene::sendVisibleNpc(SceneUser* pUser, bool show)
{
  TSetQWORD setVisible;
  Cmd::NtfVisibleNpcUserCmd usercmd;
  if(show == true)
    usercmd.set_type(1);
  else
    usercmd.set_type(0);
  for(auto &p : m_mapVisibleNpcs)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(p.first);
    if (npc == nullptr)
      continue;

    refreshVisibleNpc(usercmd, npc);
    setVisible.insert(npc->id);
  }

  const TSetDWORD& setShowNpc = pUser->getUserSceneData().getShowNpcs();
  for(auto s : setShowNpc)
  {
    const SNpcCFG* pNpcCfg = NpcConfig::getMe().getNpcCFG(s);
    if(pNpcCfg == nullptr || pNpcCfg->strMapIcon.empty() == true)
      continue;

    std::list<SceneNpc *> npclist;
    getSceneNpcByBaseID(s, npclist);
    for(auto &iter : npclist)
    {
      if(iter == nullptr || iter->getMapID() != getMapID())
        continue;
      auto m = setVisible.find(iter->id);
      if(m != setVisible.end())
        continue;

      refreshVisibleNpc(usercmd, &(*iter));
      setVisible.insert(iter->id);
    }
  }

  std::set<SceneNpc*> npcset;
  pUser->getQuestNpc().getCurMapNpc(npcset);
  for(auto s : npcset)
  {
    const SNpcCFG* pNpcCfg = s->getCFG();
    if(pNpcCfg == nullptr || pNpcCfg->strMapIcon.empty() == true)
      continue;
    auto m = setVisible.find(s->id);
    if(m != setVisible.end())
      continue;

    refreshVisibleNpc(usercmd, &(*s));
  }

  if(usercmd.npcs_size() > 0)
  {
    PROTOBUF(usercmd, send, len);
    pUser->sendCmdToMe(send, len);
    XLOG << "[场景-小地图可见怪物] 玩家进入场景，消息内容" << pUser->accid << pUser->id << getMapID() << "怪物信息："<< usercmd.ShortDebugString() << XEND;
  }
}

bool Scene::hideNpc(DWORD npcId)
{
  if (npcId == MiscConfig::getMe().getAuctionMiscCFG().dwNpcId && MiscConfig::getMe().isSystemForbid(ESYSTEM_FORBID_AUCTION) == true)
  {
    XLOG << "场景-隐藏npc创建] 拍卖npc" << npcId << XEND;
    return true;
  }

  return false;
}

bool Scene::getRandPosOverRange(const xPos& pos, float r, xPos& tpos)
{
  int i = 0;
  while (i++ < 30)
  {
    if (getRandPos(tpos) && getXZDistance(pos, tpos) > r)
      return true;
  }
  return false;
}

void Scene::addSeat(DWORD seatid)
{
  m_setSeat.insert(seatid);

  Seat* pSeat = this->base->getSeat(seatid);
  if(pSeat != nullptr && pSeat->m_bOpen == false)
    pSeat->m_bOpen = true;

  XLOG << "[场景-座位] 添加 " << "map:" <<getMapID() << "座位："<< seatid << XEND;
}

void Scene::delSeat(DWORD seatid)
{
  m_setSeat.erase(seatid);

  Seat* pSeat = this->base->getSeat(seatid);
  if(pSeat != nullptr && pSeat->m_seatUser != 0)
  {
    pSeat->m_dwSeatDownTime = 0;
    SceneUser* pUser = SceneUserManager::getMe().getUserByID(pSeat->m_seatUser);
    if(pUser != nullptr)
      pUser->seatUp();
  }

  XLOG << "[场景-座位] 删除" << "map:" <<getMapID() << "座位："<< seatid << XEND;
}

void Scene::sendSeatToUser(SceneUser* user)
{
  if(user == nullptr || m_setSeat.empty() == true)
    return;

  Cmd::ShowSeatUserCmd cmd;
  cmd.set_show(SEAT_SHOW_VISIBLE);
  for(auto s : m_setSeat)
  {
    cmd.add_seatid(s);
  }
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

bool Scene::canEnter(SceneUser* pUser)
{
  if (!pUser)
    return false;
  
  const SMapCFG *pMapCfg = getBaseCFG();
  if (pMapCfg == nullptr)
    return false;
  if (pMapCfg->enterCond.type == EENTERCONDTYPE_NONE)
    return true;
  
  if (pMapCfg->enterCond.type == EENTERCONDTYPE_ACTIVITY)
  {  
    ActivityBase* pAct = ActivityManager::getMe().getActivityByUid(pMapCfg->enterCond.param);
    if (pAct == nullptr)
    {
      XLOG <<"[地图-传送] 活动已经关闭，不能进入地图。 charid" <<pUser->id <<pUser->name <<"mapid" <<pMapCfg->dwID <<"活动uid" <<pMapCfg->enterCond.type << XEND;
      return false;
    }  
  }
  return true;
}

bool Scene::isNoramlScene()
{
  const SMapCFG *pMapCfg = getBaseCFG();
  if (pMapCfg == nullptr)
    return true;
  if (pMapCfg->enterCond.type == EENTERCONDTYPE_NONE)
    return true;
  return false;
}

// DScene
DScene::DScene(DWORD sID, const char* sName, const SceneBase* sBase, const SRaidCFG* pRaidCFG) :
  Scene(sID, sName, sBase)
  , m_pRaidCFG(pRaidCFG)
  , m_oFuben(this)
{
  m_dwNoUserProtectTick = xTime::getCurSec() + NOUSERPROTECTTICK;
}

bool DScene::init()
{
  if (Scene::init() == false)
    return false;

  MapInfo oInfo = base->getMapInfo();
  define.init(oInfo.range, isShowAllNpc());

  if (m_oFuben.add(getRaidID()) == false)
    XLOG << "[副本创建] raid :" << getRaidID() << "无" << XEND;

  onFiveSecTimeUp(now());

  return true;
}

void DScene::entryAction(QWORD curMSec)
{
  Scene::entryAction(curMSec);
  //Scene::entryAction 一定要在m_oFuben.timer 前
  m_oFuben.timer(curMSec);
  DWORD dwNow = curMSec / ONE_THOUSAND;

  if (getState() == xScene::SCENE_STATE_WAIT_CLOSE)
  {
    //有人活了
    if (!isAllDead())
    {
      //关闭倒计时
      stopCountDown();
    }
  }

  if ((getState() == xScene::SCENE_STATE_RUN || getState() == xScene::SCENE_STATE_WAIT_CLOSE) && getCloseTime() != 0 && getCloseTime() < dwNow)
    setCloseState();

  if (getState() == xScene::SCENE_STATE_RUN && m_dwCloseTimeNoUser == 0 && dwNow > m_dwNoUserProtectTick)
  {
    xSceneEntrySet set;
    getAllEntryList(SCENE_ENTRY_USER, set);
    if (set.empty() == true)
      setCloseTime(dwNow + getRaidNoUserTime(), true);

    m_dwNoUserProtectTick = dwNow + NOUSERPROTECTTICK;
  }
  if (m_dwWaitWaitReplyTime && m_dwWaitWaitReplyTime < dwNow)
  {
    m_dwWaitWaitReplyTime = 0;
    m_setWaitReplyUsers.clear();
  }
}

void DScene::userEnter(SceneUser *user)
{
  if (user == nullptr)
    return;

  user->setFubenBeAttackCount(0);
  m_dwCloseTimeNoUser = 0;

  Scene::userEnter(user);
  m_oFuben.enter(user);
  if (m_oFuben.getMonsterCountShow() == true)
    MonsterCountPhase::notify(this);
  if (isPVPScene() == true || isGvg())
    user->setCollectMark(ECOLLECTTYPE_GUILD);

  if (needCountDown())
    stopCountDown();
}

void DScene::userLeave(SceneUser *user)
{
  if (user == nullptr)
    return;

  Scene::userLeave(user);
  getFuben().leave(user);
  if (isPVPScene() == true || isGvg())
    user->setCollectMark(ECOLLECTTYPE_GUILD);

  // check no user
  xSceneEntrySet userset;
  getAllEntryList(SCENE_ENTRY_USER, userset);
  if (userset.empty() == true)
  {
    setCloseTime(now() + getRaidNoUserTime(), true);

    if (m_oImages.isImageScene())
    {
      QWORD indexguid = m_oImages.getImageIndex();

      if (indexguid != 0)
      {
        if (m_pRaidCFG && m_pRaidCFG->eRaidType == ERAIDTYPE_ITEMIMAGE)
        {
          ItemImageScene* pItemScene = dynamic_cast<ItemImageScene*>(this);
          if (pItemScene)
          {
            //clear creator
            /*SceneTeam* pTeam = SceneTeamManager::getMe().getTeamByID(indexguid);
            if (pTeam)
            {
              pTeam->setImageCreator(0);
            }*/

            //播放关闭特效
            SceneNpc* pNpc = pItemScene->getNpc();
            if (pNpc)
            {
              UserActionNtf message;
              message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
              message.set_value(2001);
              message.set_charid(pNpc->id);
              pNpc->setGearStatus(2002);
            }

            std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ITEMIMAGE, indexguid, thisServer->getZoneID(), "user");
            RedisManager::getMe().delData(key);
            user->sendItemImage(true);
            XLOG << "[道具镜像-关闭]" << id << "image index" << indexguid << XEND;
          }
        }

        Scene* pRealScene = SceneManager::getMe().getSceneByID(getMapID());
        if (pRealScene)
        {
          pRealScene->m_oImages.del(indexguid, getRaidID());
        }
        else
        {
          DelSceneImage cmd;
          cmd.set_guid(indexguid);
          cmd.set_realscene(getMapID());
          cmd.set_raid(getRaidID());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
    }
  }

  m_setReliveLeaveUser.erase(user->id);
}

void DScene::onNpcDie(SceneNpc *npc, xSceneEntryDynamic *killer)
{
  if (npc == nullptr)
    return;

  for (auto it = m_oFuben.m_oVars.begin(); it != m_oFuben.m_oVars.end(); ++it)
  {
    if ((it->second.m_oKillNpcIDSet.find(npc->getNpcID())!=it->second.m_oKillNpcIDSet.end()) ||
        (it->second.m_dwGroupID && (it->second.m_dwGroupID == npc->define.getUniqueID())))
    {
      it->second.m_dwKillNpcNum++;
      m_oFuben.check("kill", it->first);
    }
    if (npc->define.getUniqueID())
    {
      it->second.m_dwKillUniqueID = npc->define.getUniqueID();
      m_oFuben.check("killAll", it->first);
      m_oFuben.check("collect", it->first);
    }
  }

  if (m_oFuben.getMonsterCountShow() == true)
    MonsterCountPhase::notify(this);
}

void DScene::onUserDie(SceneUser *user, xSceneEntryDynamic *killer)
{
  if (needCountDown())
  {
    if (isAllDead())
      startCountDown();
  }
}

void DScene::onClose()
{
}

void DScene::setCloseState()
{
  //if (getState() != xScene::SCENE_STATE_RUN)
  //  return;

  this->onClose();
  setState(xScene::SCENE_STATE_PRE_CLOSE);
  kickAllUser();

  Cmd::DeleteDMapSessionCmd message;
  message.set_mapid(id);
  PROTOBUF(message, send, len);
  thisServer->sendCmdToSession(send, len);

  XLOG << "[动态地图]" << id << name << "设置关闭状态" << XEND;
}

//全员死亡是否需要倒计时
bool DScene::needCountDown()
{
  ERaidType type = getRaidType();
  if (type == ERAIDTYPE_DOJO || type == ERAIDTYPE_TOWER || type == ERAIDTYPE_PVECARD)
    return true;
  
  return false;
}

bool DScene::isAllDead()
{
  xSceneEntrySet entrySet;
  getAllEntryList(SCENE_ENTRY_USER, entrySet);
  if (entrySet.empty())
  {
    return true;
  }

  for (auto it = entrySet.begin(); it != entrySet.end(); ++it)
  {
    SceneUser* pUser = dynamic_cast<SceneUser*>(*it);
    if (pUser)
    {
      auto it2 = m_setReliveLeaveUser.find(pUser->id);
      if (it2 != m_setReliveLeaveUser.end())
        continue;

      if (pUser->isAlive())
      {
        return false;
      }
    }
  }
  return true;
}

void DScene::stopCountDown()
{
  if (getState() == xScene::SCENE_STATE_WAIT_CLOSE)
  {
    setState(xScene::SCENE_STATE_RUN);
    xSceneEntrySet userset;
    getAllEntryList(SCENE_ENTRY_USER, userset);
    setCloseTime(now() + getRaidLimitTime());
    //MsgManager::sendMsg(this->id, 2907, MsgParams(10), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
    for (auto &s : userset)
      MsgManager::sendMsg(s->id, 2907, MsgParams(10), EMESSAGETYPE_TIME_DOWN, EMESSAGEACT_DEL);
    XLOG << "[动态地图] id:" << this->id << "关闭全员死亡倒计时" << XEND;
  }
}

void DScene::startCountDown()
{
  if (m_oFuben.getMonsterCountShow() == true)
    MonsterCountPhase::notify(this, true);      //关闭怪物个数提示

  setState(xScene::SCENE_STATE_WAIT_CLOSE);
  setCloseTime(xTime::getCurSec() + 10);
  MsgManager::sendMapMsg(this->id, 2907, MsgParams(10), EMESSAGETYPE_TIME_DOWN);
  XLOG << "[动态地图] id:" << this->id << "开启全员死亡倒计时" << XEND;
}

DWORD DScene::getAllAliveMonsterNum()
{
  DWORD count = 0;
  xSceneEntrySet npcSet;
  getAllEntryList(SCENE_ENTRY_NPC, npcSet);
  for (auto &it : npcSet)
  {
    SceneNpc *npc = (SceneNpc *)it;
    if (npc)
    {
      if (!npc->isAlive()) continue;
      if (npc->define.isGear()) continue;
      if (npc->m_ai.isNotSkillSelect()) continue;
      if (npc->getNpcType() == ENPCTYPE_NPC) continue;
      if (npc->define.m_oVar.m_qwFollowerID) continue;
      if (npc->isWeaponPet()) continue;
      if (npc->getNpcType() == ENPCTYPE_PETNPC) continue;
      if (npc->getNpcType() == ENPCTYPE_BEING) continue;
      ++count;
    }
  }

  return count;
}

void DScene::inviteSummonDeadBoss(SceneUser* user)
{
  if (!user)
    return;
  if (user->getTeamLeaderID() != user->id)
    return;
  if (checkSummonDeadBoss() == false)
    return;

  if (m_setDeadBossSummonUsers.empty() == false)
  {
    // 副本已经召唤过
    MsgManager::sendMsg(user->id, 25935);
    return;
  }

  DWORD maxcnt = 0;
  EVarType e = EVARTYPE_MIN;
  switch(getRaidType())
  {
    case ERAIDTYPE_PVECARD:
      {
        const SPveMiscCFG& rCFG = MiscConfig::getMe().getMiscPveCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        PveCardScene* pCardScene = dynamic_cast<PveCardScene*>(this);
        if (pCardScene == nullptr)
          return;
        switch(pCardScene->getDifficulty())
        {
          case 2:
            e = EVARTYPE_DEADBOSS_COUNT_PVECARD2;
            break;
          case 3:
            e = EVARTYPE_DEADBOSS_COUNT_PVECARD3;
            break;
          case 4:
            e = EVARTYPE_DEADBOSS_COUNT_PVECARD4;
            break;
          default:
            return;
        }
      }
      break;
    case ERAIDTYPE_GUILDRAID:
      {
        const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        e = EVARTYPE_DEADBOSS_COUNT_GUILD;
      }
      break;
    case ERAIDTYPE_TOWER:
      {
        const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        e = EVARTYPE_DEADBOSS_COUNT_TOWER;
      }
      break;
    default:
      return;
  }

  if (user->getVar().getVarValue(e) >= maxcnt)
  {
    MsgManager::sendMsg(user->id, 25922);
    return;
  }

  // 在等待中
  if (!m_setWaitReplyUsers.empty())
    return;
  // 已召唤
  if (!m_setDeadBossSummonUsers.empty())
    return;

  std::set<SceneUser*> mems = user->getTeamSceneUser();
  mems.erase(user);

  std::set<SceneUser*> hasCntUsers;
  std::set<SceneUser*> noCntUsers;
  for (auto &s : mems)
  {
    if (s->getVar().getVarValue(e) < maxcnt)
      hasCntUsers.insert(s);
    else
      noCntUsers.insert(s);
  }

  if (hasCntUsers.empty())
  {
    // 直接召唤
    doSummonDeadBoss();
  }
  else
  {
    // 允许召唤, 弹框显示
    ReplySummonBossFubenCmd selfcmd;
    selfcmd.set_charid(user->id);
    selfcmd.set_agree(true);
    PROTOBUF(selfcmd, s1, l1);
    user->sendCmdToMe(s1, l1);

    // 通知队长
    if (!noCntUsers.empty())
    {
      ReplySummonBossFubenCmd retcmd;
      retcmd.set_isfull(true);
      for (auto &s : noCntUsers)
      {
        retcmd.set_charid(s->id);
        PROTOBUF(retcmd, retsend, retlen);
        user->sendCmdToMe(retsend, retlen);
      }
    }

    // 邀请
    m_dwWaitWaitReplyTime = now() + 35;
    InviteSummonBossFubenCmd cmd;
    PROTOBUF(cmd, send, len);
    for (auto &s : hasCntUsers)
    {
      m_setWaitReplyUsers.insert(s->id);
      s->sendCmdToMe(send, len);
    }
  }
}

void DScene::replySummonDeadBoss(SceneUser* user, bool agree)
{
  if (!user)
    return;
  if (m_setWaitReplyUsers.empty())
    return;
  if (m_setWaitReplyUsers.find(user->id) == m_setWaitReplyUsers.end())
    return;
  SceneUser* pLeader = SceneUserManager::getMe().getUserByID(user->getTeamLeaderID());
  if (!pLeader || pLeader->getScene() != this)
    return;

  // 回复队长
  ReplySummonBossFubenCmd cmd;
  cmd.set_agree(agree);
  cmd.set_charid(user->id);
  PROTOBUF(cmd, send, len);
  pLeader->sendCmdToMe(send, len);

  if (agree)
  {
    m_setWaitReplyUsers.erase(user->id);

    // 所有人都已同意
    if (m_setWaitReplyUsers.empty())
    {
      m_dwWaitWaitReplyTime = 0;
      doSummonDeadBoss();
    }
  }
  else
  {
    // 拒绝, 设置取消
    m_setWaitReplyUsers.clear();
    m_dwWaitWaitReplyTime = 0;
  }
}

void DScene::doSummonDeadBoss()
{
  std::list<pair<DWORD,DWORD>> listMonsterID2Uid;
  getRandomDeadBoss(listMonsterID2Uid);
  if (listMonsterID2Uid.empty())
    return;

  const SceneObject *pObject = getSceneObject();
  if (pObject == nullptr)
    return;
  DWORD maxcnt = 0;
  EVarType e = EVARTYPE_MIN;
  switch(getRaidType())
  {
    case ERAIDTYPE_PVECARD:
      {
        const SPveMiscCFG& rCFG = MiscConfig::getMe().getMiscPveCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        e = EVARTYPE_DEADBOSS_COUNT_PVECARD;
        setCloseTime(DWORD_MAX);
      }
      break;
    case ERAIDTYPE_GUILDRAID:
      {
        const SGuildRaidCFG& rCFG = MiscConfig::getMe().getGuildRaidCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        e = EVARTYPE_DEADBOSS_COUNT_GUILD;
      }
      break;
    case ERAIDTYPE_TOWER:
      {
        const SEndlessTowerCFG& rCFG = MiscConfig::getMe().getEndlessTowerCFG();
        maxcnt = rCFG.dwDeadBossSummonTime;
        e = EVARTYPE_DEADBOSS_COUNT_TOWER;
      }
      break;
    default:
      return;
  }

  // 设置成员var
  xSceneEntrySet userset;
  getAllEntryList(SCENE_ENTRY_USER, userset);
  for (auto &s : userset)
  {
    SceneUser* user = dynamic_cast<SceneUser*> (s);
    if (!user || user->getScene() != this)
      continue;
    DWORD cnt = user->getVar().getVarValue(e);
    if (cnt < maxcnt)
    {
      user->getVar().setVarValue(e, ++cnt);
      m_setDeadBossSummonUsers.insert(user->id);
      XLOG << "[亡者boss-召唤], 设置玩家亡者boss次数, 玩家:" << user->name << user->id << "次数:" << cnt << XEND;
    }
  }

  // 召唤
  for (auto &l : listMonsterID2Uid)
  {
    const SceneNpcTemplate* pTemplate = pObject->getRaidNpcTemplate(l.second);
    if (pTemplate == nullptr)
      return;
    NpcDefine def = pTemplate->m_oDefine;
    def.setID(l.first);
    SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, this);
    if (npc)
    {
      XLOG << "[亡者boss-召唤], 召唤成功, 怪物:" << npc->name << npc->id << "副本:" << name << id << XEND;
    }
  }
  XLOG << "[亡者boss-召唤], 执行召唤成功, 副本:" << name << id << XEND;
}

void DScene::onNpcDieReward(SceneNpc* npc, SceneUser* user)
{
  if (npc == nullptr || user == nullptr)
    return;
  if (npc->getNpcZoneType() == ENPCZONE_RAIDDEADBOSS)
  {
    const SRaidDeadBossCFG* pCFG = NpcConfig::getMe().getRaidDeadBossCFG(npc->getNpcID());
    if (pCFG == nullptr)
      return;

    for (auto &s : m_setDeadBossSummonUsers)
    {
      SceneUser* user = SceneUserManager::getMe().getUserByID(s);
      if (user == nullptr || user->getScene() != this)
      {
        XLOG << "[亡者boss-奖励], 玩家已不再当前地图, 玩家:" << s << "副本:" << name << id << "怪物:" << npc->name << npc->id << XEND;
        continue;
      }
      for (auto &d : pCFG->setRewards)
      {
        user->getPackage().rollReward(d, EPACKMETHOD_AVAILABLE, false, true);
      }
      XLOG << "[亡者boss-奖励], 玩家获取奖励, 玩家:" << user->name << user->id << "副本:" << name << id << "怪物:" << npc->name << npc->id << XEND;
    }
    if (getRaidType() == ERAIDTYPE_PVECARD)
      setCloseTime(now() + 30);
  }
}


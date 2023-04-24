#include "SceneManager.h"
#include "SceneUserManager.h"
#include "SceneUser.h"
#include "SceneServer.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "DScene.h"
#include "MapConfig.h"
#include "MsgManager.h"
#include "TeamSealManager.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
  clear();
}

bool SceneManager::addScene(Scene* scene)
{
  return addEntry(scene);
}

void SceneManager::delScene(Scene* scene)
{
  removeEntry(scene);
  SAFE_DELETE(scene);
}

Scene* SceneManager::getSceneByID(DWORD sid)
{
  return (Scene*)getEntryByID(sid);
}

Scene* SceneManager::getSceneByName(const char* name)
{
  return (Scene *)getEntryByName(name);
}

bool SceneManager::init()
{
  DWORD index = atoi(thisServer->getServerName()+11);
  if (index)
  {
    for (auto iter=m_baseCfg.begin(); iter!=m_baseCfg.end(); ++iter)
    {
      if (iter->second->isStaticMap())
      {
#ifdef _DEBUG
        if (iter->second->getStaticGroup() % 2 + 1 == index)
#else
        if (iter->second->getStaticGroup() == index)
#endif
        {
          createScene(iter->second);
        }
      }
    }
  }
  createVirtualScene();
  return true;
}

bool SceneManager::loadSceneConfig()
{
  clear();

  bool bCorrect = true;
  const TMapMapCFG& mapCFG = MapConfig::getMe().getMapCFGList();
  for (auto &m : mapCFG)
  {
    SceneBase* pBase = (SceneBase *)getDataByID(m.first);
    if (pBase == nullptr)
      pBase = NEW SceneBase();

    if (pBase == nullptr)
    {
      XERR << "[场景管理-加载]" << m.first << "创建SceneBase失败" << XEND;
      bCorrect = false;
      continue;
    }
    if (pBase->init(&m.second) == false)
    {
      XERR << "[场景管理-加载]" << m.first << "初始化失败" << XEND;
      SAFE_DELETE(pBase);
      bCorrect = false;
      continue;
    }

    if (!SceneManager::getMe().loadClientExport(pBase))
    {
      XERR << "[场景管理-初始化]" << m.first << "读取client-export失败" << XEND;;
      SAFE_DELETE(pBase);
      bCorrect = false;
      continue;
    }

    XLOG << "[场景管理-初始化]" << m.first << "加载" << XEND;;
    m_baseCfg[m.first] = pBase;
  }

  // load guild flag map info
  for (auto &it : m_baseCfg)
  {
    const TMapFlagCFG& mapflag = it.second->getFlagInfo();
    for (auto &m : mapflag)
    {
      m_mapFlag2MapID[m.first].insert(it.first);
    }
  }

  //bCorrect = true;
  return bCorrect;
}

bool SceneManager::loadClientExport(SceneBase *base)
{
  if (base == nullptr)
    return false;

  if (!loadSceneInfo(base))
  {
    XERR << "[场景管理-加载地图文件]" << base->getMapID() << base->m_oInfo.file  << "SceneInfo加载失败" << XEND;
    return false;
  }

  if (!loadNavmesh(base))
  {
    XERR << "[场景管理-加载地图文件]" << base->getMapID() << base->m_oInfo.file  << "NavMesh加载失败" << XEND;
    return false;
  }

  return true;
}

bool SceneManager::loadSceneInfo(SceneBase *base)
{
  if (!base) return false;

  const char *dir = "client-export/Scene/";

  std::stringstream file_string;
  file_string.str("");
  file_string << dir << "Scene" << base->m_oInfo.file << "/SceneInfo.lua";
  {
    if (!xLuaTable::getMe().open(file_string.str().c_str()))
    {
//      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "file:" << file_string.str() << "加载失败" << XEND;
      return false;
    }
    xLuaData data;
    if (!xLuaTable::getMe().getLuaData("Root", data))
    {
      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "file:" << file_string.str() << "获取Root根节点失败" << XEND;
      return false;
    }
    base->m_oObject.load(data);

    if (data.has("Raids"))
    {
      const xLuaData &d = data.getData("Raids");
      for (auto it=d.m_table.begin(); it!=d.m_table.end(); ++it)
        base->m_oObjectList[atoi(it->first.c_str())].load(it->second);
    }
    if (data.has("PVP"))
    {
      const xLuaData &d = data.getData("PVP");
      base->m_oObjectPVP.load(d);
    }
    if (data.has("GuildFlags"))
      base->loadFlag(data.getMutableData("GuildFlags"));
    if (data.has("NoFlyPoints"))
      base->loadNoFlyPoints(data.getMutableData("NoFlyPoints"));
    base->loadFrame(data);
  }

  //load Seat
  loadSceneSeat(base);

  return true;
}

bool SceneManager::loadSceneSeat(SceneBase* base)
{
  if (!base) return false;

  const char *dir = "Lua/Table/";

  std::stringstream  table_name;
  table_name.str("");
  table_name << "Table_Seat_" << base->m_oInfo.file;
  std::string strTableName = table_name.str();

  std::stringstream file_string;
  file_string.str("");
  //Lua/Table/Table_Seat_Prontera.lua
  file_string << dir << strTableName << ".txt";
  {
    if (!xLuaTable::getMe().open(file_string.str().c_str()))
    {
      XERR << "[场景管理-加载座位表]" << base->getMapID() << "file:" << file_string.str() << "加载失败" << XEND;
      return true;
    }
    xLuaData tableData;
    if (!xLuaTable::getMe().getLuaData(strTableName.c_str(), tableData))
    {
      XERR << "[场景管理-加载座位表]" << base->getMapID() << "file:" << file_string.str() << "获取根节点失败" << XEND;
      return true;
    }

    auto fun = [&](const string& key, xLuaData& data)
    {
      Seat seat;
      seat.m_dwSeatId = data.getTableInt("id");
      xLuaData& rPos = data.getMutableData("SeatPot");
      seat.m_oSeatPos.x = rPos.getTableFloat("1");
      seat.m_oSeatPos.y = rPos.getTableFloat("2");
      seat.m_oSeatPos.z = rPos.getTableFloat("3");
      seat.m_dwSeatTime = data.getTableInt("SeatTime");
      seat.m_bOpen = (data.getTableInt("ServerShow") == 0) ? true : false;
      auto costfunc = [&](const string& subkey, xLuaData& subdata)
      {
        DWORD dwItem = subdata.getTableInt("item");
        DWORD dwNum = subdata.getTableInt("num");
        seat.m_mapCost.insert(std::make_pair(dwItem, dwNum));
      };
      data.getMutableData("Cost").foreach(costfunc);

      base->m_oSeats[seat.m_dwSeatId] = seat;
      XDBG << "[场景管理-加载座位表]" << base->getMapID() << strTableName << seat.m_dwSeatId << seat.m_oSeatPos.getX() << seat.m_oSeatPos.getY() << seat.m_oSeatPos.getZ() << XEND;
    };
    tableData.foreach(fun);
  }
  return true;
}

bool SceneManager::loadNavmesh(SceneBase *base)
{
  if (!base) return false;

  auto it = m_oPathFindingList.find(base->m_oInfo.file);
  if (it == m_oPathFindingList.end())
  {
    const char *dir = "client-export/Scene/";
    std::stringstream file_string;
    file_string.str("");
    file_string << dir << "Scene" << base->m_oInfo.file << "/NavMesh.lua";

    if (!xLuaTable::getMe().open(file_string.str().c_str()))
    {
      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "file:" << file_string.str() << "加载失败" << XEND;
      return false;
    }

    xLuaData mess;
    if (!xLuaTable::getMe().getLuaData("Root", mess))
    {
      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "file:" << file_string.str() << "获取Root跟节点失败" << XEND;
      return false;
    }

    const xLuaData &vertices = mess.getData("vertices");
    if (vertices.m_table.empty())
    {
      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "file:" << file_string.str() << "获取vertices顶点数据失败" << XEND;
      return false;
    }

    PathFindingTile *pTile = NEW PathFindingTile();
    m_oPathFindingList[base->m_oInfo.file] = pTile;

    base->m_pPathfinding = pTile;
    pTile->m_strFile = base->m_oInfo.file;
    if (!pTile->init(mess))
    {
      XERR << "[场景管理-加载地图文件]" << base->getMapID() << "NavMesh:Scene" << base->m_oInfo.file << "初始化失败" << XEND;
      return false;
    }
  }
  else
  {
    base->m_pPathfinding = it->second;
  }
  return true;
}

Scene* SceneManager::createScene(SceneBase *base)
{
  if (!base->m_pPathfinding->isInitOk())
  {
    XERR << "[场景管理-创建地图]" << base->m_oInfo.id << base->m_oInfo.name << "NavMesh加载失败,创建失败" << XEND;
    stringstream sstr;
    sstr << "NavMesh" << base->m_oInfo.id << " " << base->m_oInfo.name << " 加载失败";
    MsgManager::alter_msg(xServer::getFullName(), sstr.str(), EPUSHMSG_MSG);
 //   return nullptr;
  }
  const float* bmin = base->m_pPathfinding->getMeshBoundsMin();
  const float* bmax = base->m_pPathfinding->getMeshBoundsMax();
  base->m_oInfo.range.xMin = (int)bmin[0] - 1;
  base->m_oInfo.range.xMax = (int)bmax[0] + 1;
  base->m_oInfo.range.zMin = (int)bmin[2] - 1;
  base->m_oInfo.range.zMax = (int)bmax[2] + 1;
  base->m_oInfo.range.yMin = (int)bmin[1] - 1;
  base->m_oInfo.range.yMax = (int)bmax[1] + 1;

  Scene *scene = NEW SScene(base->m_oInfo.id, base->m_oInfo.name, base);
  if (!scene)
  {
    XERR << "[场景管理-创建地图]" << base->m_oInfo.id << base->m_oInfo.name << "创建失败" << XEND;
    return nullptr;
  }

  if (scene->init() == false)
  {
    XERR << "[场景管理-创建地图]" << base->m_oInfo.id << base->m_oInfo.name << "初始化失败" << XEND;
    SAFE_DELETE(scene);
    return nullptr;
  }

  if (!addScene(scene))
  {
    XERR << "[场景管理-创建地图]" << scene->id << scene->name << "添加到管理器失败" << XEND;
    SAFE_DELETE(scene);
    return nullptr;
  }

  MapRegSessionCmd cmd;
  cmd.set_mapid(scene->id);
  cmd.set_mapname(scene->name);
  cmd.set_scenename(thisServer->getServerName());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  scene->setState(xScene::SCENE_STATE_VERTIFY);
  XLOG << "[场景管理-创建地图]" << scene->id << scene->name << "创建成功,到会话注册" << XEND;
  return scene;
}

Scene* SceneManager::createVirtualScene()
{
  SceneBase *base = SceneManager::getMe().m_baseCfg[VIRTUAL_SCENE_ID];
  if (base == nullptr)
    return nullptr;

  XLOG << "[加载地图]" << thisServer->getServerName() << "(" << atoi(thisServer->getServerName()+11) << ")" << XEND;
  Scene *scene = NEW SScene(VIRTUAL_SCENE_ID + atoi(thisServer->getServerName()+11), thisServer->getServerName(), base);
  if (!scene)
  {
    XERR << "[加载地图]" << base->m_oInfo.id << base->m_oInfo.name << " NEW failed" << XEND;
    return NULL;
  }

  if (!scene->init())
  {
    XERR << "[加载地图]" << scene->id << scene->name << "初始化失败" << XEND;
    SAFE_DELETE(scene);
    return NULL;
  }
  if (!addScene(scene))
  {
    XERR << "[加载地图]" << scene->id << scene->name << "添加到管理器失败" << XEND;
    SAFE_DELETE(scene);
    return NULL;
  }

  MapRegSessionCmd cmd;
  cmd.set_mapid(scene->id);
  cmd.set_mapname(scene->name);
  cmd.set_scenename(thisServer->getServerName());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  scene->setState(xScene::SCENE_STATE_VERTIFY);
  XLOG << "[加载地图],加载地图" << scene->name << "(" << scene->id << ")成功,到会话注册" << XEND;
  return scene;
}

bool SceneManager::createDScene(const CreateDMapParams& params)
{
  const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(params.dwRaidID);
  if (pBase == nullptr)
    return false;

  CreateRaidMapSessionCmd cmd;
  RaidMapData* pData = cmd.mutable_data();
  if (pData == nullptr)
    return false;

  pData->set_raidid(params.dwRaidID);
  pData->set_charid(params.qwCharID);
  pData->set_sealid(params.m_dwSealID);
  pData->set_dojoid(params.m_dwDojoId);
  pData->set_layer(params.dwLayer);
  pData->set_roomid(params.m_qwRoomId);
  pData->mutable_guildinfo()->CopyFrom(params.oGuildInfo);
  pData->set_nomonsterlayer(params.m_dwNoMonsterLayer);

  for (auto v = params.vecMembers.begin(); v != params.vecMembers.end(); ++v)
    pData->add_memberlist(*v);

  if (params.m_oType == GoMapType::Hands)
    pData->set_follow(1);

  if (!params.m_oImageCenter.empty())
  {
    pData->mutable_imagecenter()->set_x(params.m_oImageCenter.getX());
    pData->mutable_imagecenter()->set_y(params.m_oImageCenter.getY());
    pData->mutable_imagecenter()->set_z(params.m_oImageCenter.getZ());
    pData->set_imagerange(params.m_dwImageRange);
  }
  if (params.m_dwNpcId)
    pData->set_npcid(params.m_dwNpcId);

  if (!params.m_oEnterPos.empty())
  {
    pData->mutable_enterpos()->set_x(params.m_oEnterPos.getX());
    pData->mutable_enterpos()->set_y(params.m_oEnterPos.getY());
    pData->mutable_enterpos()->set_z(params.m_oEnterPos.getZ());
  }

  if (pBase->eRaidType == ERAIDTYPE_FERRISWHEEL)
  {
    for (auto &v : params.vecMembers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(v);
      if (pUser != nullptr)
        pUser->m_oHands.breakup();
    }
  }
  
  if (params.eRestrict != ERAIDRESTRICT_MIN)//RaidMapRestrict::Null)
    pData->set_restrict(static_cast<DWORD>(params.eRestrict));
  else
    pData->set_restrict((DWORD)pBase->eRestrict);//getRestrict());

  if (pBase->eRaidType == ERAIDTYPE_PVP_LLH || pBase->eRaidType == ERAIDTYPE_PVP_SMZL || pBase->eRaidType == ERAIDTYPE_PVP_HLJS
      || pBase->eRaidType == ERAIDTYPE_PVP_POLLY || pBase->eRaidType == ERAIDTYPE_MVPBATTLE || pBase->eRaidType == ERAIDTYPE_TEAMPWS)
  {
    pData->set_restrict(ERAIDRESTRICT_PVP_ROOM);
  }
  else if (pBase->eRaidType == ERAIDTYPE_SUPERGVG)
  {
    pData->set_restrict(ERAIDRESTRICT_PVP_ROOM);
  }

  ERaidRestrict eRestrict = static_cast<ERaidRestrict>(pData->restrict());
  switch (eRestrict)
  {
    case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(params.qwCharID);
        if (pUser != nullptr)
          pUser->m_oHands.breakup();
      }
      break;
    case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
    case ERAIDRESTRICT_HONEYMOON:
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(params.qwCharID);
        if (pUser == nullptr)
          return false;
        const GTeam& rTeam = pUser->getTeam();
        if (rTeam.getTeamID() == 0)
        {
          MsgManager::sendMsg(pUser->id, 324);
          return false;
        }
        pData->set_teamid(rTeam.getTeamID());
      }
      break;
    case ERAIDRESTRICT_SYSTEM://RaidMapRestrict::System:
      break;
    case ERAIDRESTRICT_GUILD:
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(params.qwCharID);
        if (pUser == nullptr)
          return false;
        pData->mutable_guildinfo()->set_id(pUser->getGuild().id());
        pData->mutable_guildinfo()->set_lv(pUser->getGuild().lv());
      }
      break;
    case ERAIDRESTRICT_GUILD_TEAM:
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(params.qwCharID);
        if (pUser == nullptr)
          return false;
        const GTeam& rTeam = pUser->getTeam();
        if (rTeam.getTeamID() == 0)
        {
          MsgManager::sendMsg(pUser->id, 324);
          return false;
        }
        pData->set_teamid(rTeam.getTeamID());
        pData->mutable_guildinfo()->set_id(pUser->getGuild().id());
      }
      break;
    case ERAIDRESTRICT_USER_TEAM:
      break;
    case ERAIDRESTRICT_PVP_ROOM:
      break;
    case ERAIDRESTRICT_GUILD_RANDOM_RAID:
      {
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(params.qwCharID);
        if (pUser == nullptr)
          return false;
        const GTeam& rTeam = pUser->getTeam();
        if (rTeam.getTeamID() == 0)
        {
          MsgManager::sendMsg(pUser->id, 324);
          return false;
        }
        pData->set_teamid(rTeam.getTeamID());
        pData->mutable_guildinfo()->set_id(pUser->getGuild().id());
        pData->set_guildraidindex(params.m_dwGuildRaidIndex);
      }
      break;
    case ERAIDRESTRICT_GUILD_FIRE:
      {
        pData->set_guildraidindex(params.m_dwGuildRaidIndex);
      }
      break;
    case ERAIDRESTRICT_WEDDING:
      {
        pData->set_roomid(params.m_qwRoomId);
      }
      break;
    case ERAIDRESTRICT_MIN:
    case ERAIDRESTRICT_MAX:
      return false;
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  XLOG << "[场景管理-副本创建请求]" << params.qwCharID << "svr:" << thisServer->getServerName() << "请求创建 raidid:" << params.dwRaidID << "副本" << "RapMapData" << pData->ShortDebugString() << XEND;
  return true;
}

DWORD SceneManager::getCreatureCount(SCENE_ENTRY_TYPE eType, DWORD dwMapID)
{
  Scene* pScene = getSceneByID(dwMapID);
  if (pScene == nullptr)
    return 0;

  xSceneEntrySet set;
  pScene->getAllEntryList(eType, set);
  return static_cast<DWORD>(set.size());
}

Scene* SceneManager::createDScene(const CreateRaidMapSessionCmd &cmd)
{
  const SRaidCFG *raidbase = MapConfig::getMe().getRaidCFG(cmd.data().raidid());
  if (!raidbase) return NULL;

  SceneBase *base = (SceneBase *)getDataByID(raidbase->dwMapID);
  if (!base || !base->getCFG()) return NULL;

  const float* bmin = base->m_pPathfinding->getMeshBoundsMin();
  const float* bmax = base->m_pPathfinding->getMeshBoundsMax();
  MapInfo *mapinfo = (MapInfo *)(&base->m_oInfo);
  mapinfo->range.xMin = (int)bmin[0] - 1;
  mapinfo->range.xMax = (int)bmax[0] + 1;
  mapinfo->range.yMin = (int)bmin[1] - 1;
  mapinfo->range.yMax = (int)bmax[1] + 1;
  mapinfo->range.zMin = (int)bmin[2] - 1;
  mapinfo->range.zMax = (int)bmax[2] + 1;

  if (base->getCFG()->nIndexRangeMin)
  {
    if (base->getCFG()->nIndexRangeMin < base->m_oInfo.range.xMin)
    {
      base->m_oInfo.range.xMin = base->getCFG()->nIndexRangeMin;
    }
    if (base->getCFG()->nIndexRangeMin < base->m_oInfo.range.yMin)
    {
      base->m_oInfo.range.yMin = base->getCFG()->nIndexRangeMin;
    }
    if (base->getCFG()->nIndexRangeMin < base->m_oInfo.range.zMin)
    {
      base->m_oInfo.range.zMin = base->getCFG()->nIndexRangeMin;
    }
  }
  if (base->getCFG()->nIndexRangeMax)
  {
    if (base->getCFG()->nIndexRangeMax > base->m_oInfo.range.xMax)
    {
      base->m_oInfo.range.xMax = base->getCFG()->nIndexRangeMax;
    }
    if (base->getCFG()->nIndexRangeMax > base->m_oInfo.range.yMax)
    {
      base->m_oInfo.range.yMax = base->getCFG()->nIndexRangeMax;
    }
    if (base->getCFG()->nIndexRangeMax > base->m_oInfo.range.zMax)
    {
      base->m_oInfo.range.zMax = base->getCFG()->nIndexRangeMax;
    }
  }


  std::stringstream n;
  n << raidbase->strNameZh << cmd.data().index();
  Scene *scene = nullptr;
  switch (raidbase->eRaidType)
  {
    case ERAIDTYPE_MIN:
    case ERAIDTYPE_MAX:
      break;
    case ERAIDTYPE_FERRISWHEEL:
      scene = NEW FerrisWheelScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_DIVORCE_ROLLER_COASTER:
      scene = new DivorceRollerCoasterScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_SEAL:
    case ERAIDTYPE_NORMAL:
    case ERAIDTYPE_EXCHANGE:
    case ERAIDTYPE_EXCHANGEGALLERY:
    case ERAIDTYPE_RAIDTEMP2:
    case ERAIDTYPE_RAIDTEMP4:
      scene = NEW RaidScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_ITEMIMAGE:
      {
        xPos pos;
        pos.set(cmd.data().imagecenter().x(), cmd.data().imagecenter().y(), cmd.data().imagecenter().z());
        scene = NEW ItemImageScene(cmd.data().index(), n.str().c_str(), base, raidbase, pos, cmd.data().npcid());
      }
      break;
    case ERAIDTYPE_TOWER:
      scene = NEW TowerScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().layer(), cmd.data().nomonsterlayer());
      break;
    case ERAIDTYPE_LABORATORY:
      scene = NEW LaboratoryScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_DOJO:
      scene = NEW DojoScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().dojoid());
      break;
    case ERAIDTYPE_GUILD:
      scene = NEW GuildScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().guildinfo());
      break;
    case ERAIDTYPE_PVP_LLH:
      scene = NEW MonkeyScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_PVP_SMZL:
      scene = NEW DesertWolfScene(cmd.data().index(), n.str().c_str(), base, raidbase);  //TODO
      break;
    case ERAIDTYPE_PVP_HLJS:
      scene = NEW GlamMetalScene(cmd.data().index(), n.str().c_str(), base, raidbase);  //TODO
      break;
    case ERAIDTYPE_GUILDRAID:
      scene = NEW GuildRaidScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().guildinfo().id(), cmd.data().teamid(), cmd.data().guildraidindex());
      break;
    case ERAIDTYPE_DATELAND:
      scene = NEW DateLandScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_GUILDFIRE:
      scene = NEW GuildFireScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().guildraidindex(), false);
      break;
    case ERAIDTYPE_PVP_POLLY:
      scene = NEW PollyScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_WEDDING:
      scene = new WeddingScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().roomid());
      break;
    case ERAIDTYPE_PVECARD:
      scene = NEW PveCardScene(cmd.data().index(), n.str().c_str(), base, raidbase, cmd.data().roomid());
      break;
    case ERAIDTYPE_MVPBATTLE:
      scene = NEW MvpBattleScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_SUPERGVG:
      scene = NEW SuperGvgScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_ALTMAN:
      scene = NEW AltmanScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
    case ERAIDTYPE_TEAMPWS:
      scene = NEW TeamPwsScene(cmd.data().index(), n.str().c_str(), base, raidbase);
      break;
  }
  if (!scene)
  {
    XERR << "[场景管理-加载地图]" << cmd.data().index() << n.str() << " NEW failed" << XEND;
    return NULL;
  }
  if (scene->init() == false)
  {
    SAFE_DELETE(scene);
    return nullptr;
  }

  if (!addScene(scene))
  {
    XERR << "[场景管理-加载地图]" << scene->id << scene->name << "添加到管理器失败" << XEND;
    SAFE_DELETE(scene);
    return NULL;
  }

  {
    Cmd::MapRegSessionCmd message;
    message.set_mapid(scene->id);
    message.set_mapname(scene->name);
    message.set_scenename(thisServer->getServerName());
    Cmd::RaidMapData *pData = message.mutable_data();
    pData->CopyFrom(cmd.data());
    PROTOBUF(message, send, len);
    thisServer->sendCmdToSession(send, len);
    scene->setState(xScene::SCENE_STATE_VERTIFY);
    XLOG << "[场景管理-加载地图],加载地图" << scene->name << "(" << scene->id << ")成功,到会话注册" << XEND;
  }

  if (cmd.data().charid() || cmd.data().memberlist_size())
  {
    Cmd::ChangeSceneSessionCmd message;
    message.set_mapid(scene->id);
    if (cmd.data().charid())
    {
      message.add_charid(cmd.data().charid());
    }
    for (int i=0; i<cmd.data().memberlist_size(); ++i)
    {
      message.add_charid(cmd.data().memberlist(i));
    }
    if (cmd.data().has_enterpos())
    {
      message.mutable_pos()->CopyFrom(cmd.data().enterpos());
    }
    PROTOBUF(message, send, len);
    thisServer->sendCmdToSession(send, len);
  }
  if (cmd.data().has_imagecenter())
  {
    xPos p;
    p.set(cmd.data().imagecenter().x(), cmd.data().imagecenter().y(), cmd.data().imagecenter().z());
    scene->m_oImages.setRaidData(p, cmd.data().imagerange(), cmd.data().teamid() != 0 ? cmd.data().teamid() : cmd.data().charid());
  }
  if (cmd.data().sealid() != 0)
  {
    QWORD teamid = cmd.data().teamid();
    xPos pos;
    pos.x = cmd.data().imagecenter().x() / 1000.0f;
    pos.y = cmd.data().imagecenter().y() / 1000.0f;
    pos.z = cmd.data().imagecenter().z() / 1000.0f;
    SceneTeamSeal* pTeamSeal = TeamSealManager::getMe().getTeamSealByID(teamid);
    if (!pTeamSeal)
    {
      pTeamSeal = TeamSealManager::getMe().createOneTeamSeal(teamid);
    }
    if (pTeamSeal)
    {
      pTeamSeal->openSeal(cmd.data().sealid(), scene, pos);
    }
  }  
  //
  checkSummonNpc(scene);

  return scene;
}

void SceneManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();
  vector<Scene*> vecDelScene;
  for (auto &it : xEntryID::ets_)
  {
    Scene *scene = dynamic_cast<Scene *>(it.second);
    if (scene)
    {
      if (scene->getState()==xScene::SCENE_STATE_CLOSE)
      {
        if (scene->isEmpty())
          vecDelScene.push_back(scene);
        else
        {
          scene->kickAllUser();
          scene->timer(curMSec);
        }
      }
      else
      {
        scene->timer(curMSec);
      }
    }
  }

  for (auto v = vecDelScene.begin(); v != vecDelScene.end(); ++v)
    SceneManager::getMe().closeScene(*v);
}

void SceneManager::registerAllMapWhenConnect()
{
  for (auto m = xEntryID::ets_.begin(); m != xEntryID::ets_.end(); ++m)
  {
    Scene* pScene = dynamic_cast<Scene*>(m->second);
    if (pScene == nullptr)
      continue;

    MapRegSessionCmd cmd;
    cmd.set_mapid(pScene->id);
    cmd.set_mapname(pScene->name);
    cmd.set_scenename(thisServer->getServerName());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

void SceneManager::closeScene(Scene *scene)
{
  if (!scene) return;
  XLOG << "[地图]" << scene->id << scene->name << "关闭" << XEND;
  delScene(scene);
}

void SceneManager::clear()
{
  struct CallBack : public xEntryCallBack
  {
    virtual bool exec(xEntry *e)
    {
      Scene* pScene = dynamic_cast<Scene*>(e);
      if (pScene == nullptr)
        return false;
      SceneManager::getMe().delScene(pScene);
      return true;
    }
  };
  CallBack callback;
  forEach(callback);

  for (auto m = m_baseCfg.begin(); m != m_baseCfg.end(); ++m)
    SAFE_DELETE(m->second);
  m_baseCfg.clear();

  for (auto m = m_oPathFindingList.begin(); m != m_oPathFindingList.end(); ++m)
    SAFE_DELETE(m->second);
  m_oPathFindingList.clear();
}

void SceneManager::closeSceneByType(SCENE_TYPE eType)
{
  DWORD cur = now();
  for (auto &it : xEntryID::ets_)
  {
    DScene *scene = dynamic_cast<DScene*>(it.second);
    if (scene && scene->getSceneType() == eType)
    {
      scene->setCloseTime(cur);
      scene->setCloseTime(cur, true);
    }
  }
}

void SceneManager::checkSummonNpc(Scene* pScene)
{
  if (!pScene)
    return;
  DScene* pDscene = dynamic_cast<DScene*>(pScene);
  if (!pDscene)
    return;

  auto it = m_mapDsceneSummon.find(pDscene->getRaidID());
  if (it == m_mapDsceneSummon.end())
    return;
  
  xLuaData& params = it->second;

  DWORD raidId = params.getTableInt("raidid");
  if (raidId == 0)
    return ;

  DWORD num = 1;
  if (params.has("num"))
    num = params.getTableInt("num");

  NpcDefine def;
  def.load(params);
  xPos pos = def.getPos();
  bool randomPos = false;
  if (pos.empty())
  {
    randomPos = true;
  }

  for (DWORD i = 0; i < num; ++i)
  {
    if (randomPos)
    {
      if (pDscene->getRandPos(pos))
        def.setPos(pos);
    }
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(def, pDscene);
    if (pNpc == nullptr)
    {
      XERR << "[Scene-GM], 副本招怪失败" << def.getID() << XEND;
      return ;
    }
    if (def.getVisibleInMap() == true)
    {
      pDscene->addVisibleNpc(pNpc);
      XLOG << "[Scene-GM] 副本招怪, 场景招小地图可见怪" << "Mapid" << pDscene->getMapID() << "raidid" << pDscene->getRaidID() << def.getID() << "总数" << num << XEND;
    }
    pDscene->addSummonNpc(pNpc->id);
  }
}

const TSetDWORD& SceneManager::getMapByFlag(DWORD flagid) const
{
  auto it = m_mapFlag2MapID.find(flagid);
  if (it == m_mapFlag2MapID.end())
  {
    static const TSetDWORD emptymap;
    return emptymap;
  }
  return it->second;
}

void SceneManager::getAllSceneByType(SCENE_TYPE eType, std::set<Scene*> scenes) const
{
  for (auto &it : xEntryID::ets_)
  {
    Scene *scene = dynamic_cast<Scene *>(it.second);
    if (scene && scene->getSceneType() == eType)
      scenes.insert(scene);
  }
}


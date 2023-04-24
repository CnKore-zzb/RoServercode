#include <algorithm>
#include "SessionSceneManager.h"
#include "SessionScene.h"
#include "SessionServer.h"
#include "SessionUserManager.h"
#include "xNetProcessor.h"
#include "SessionCmd.pb.h"
#include "RedisManager.h"
#include "SessionUser.h"
#include "DojoConfig.h"
//#include "TeamManager.h"
#include "GuidManager.h"
#include "MatchSCmd.pb.h"
#include "GuildRaidConfig.h"

SessionSceneManager::SessionSceneManager()
{
}

SessionSceneManager::~SessionSceneManager()
{
}

void SessionSceneManager::final()
{
  struct DelSceneCallBack:public xEntryCallBack
  {
    virtual bool exec(xEntry* entry)
    {
      if(!entry) return true;
      SessionScene* scene = dynamic_cast<SessionScene*>(entry);
      SessionSceneManager::instance()->delScene(scene);
      return true;
    }
  };
  DelSceneCallBack callBack;
  forEach(callBack);
}

bool SessionSceneManager::addScene(SessionScene* scene)
{
  return addEntry(scene);
}

void SessionSceneManager::delScene(SessionScene* scene)
{
  if (!scene) return;
  XLOG << "[SessionSceneManager::delScene] id : " << scene->id << XEND;
  SessionUserManager::getMe().removeUserOnScene(scene);
  scene->onClose();
  onRaidSceneClose(scene);
  removeEntry(scene);
  SAFE_DELETE(scene);
  //DELETE(scene);
}

void SessionSceneManager::delOffLineScene()
{
  // SessionUserManager::getMe().removeUserOnScene();
  struct DelSceneCallBack : public xEntryCallBack
  {
    DelSceneCallBack()
    {
    }
    virtual bool exec(xEntry* entry)
    {
      SessionScene* scene = dynamic_cast<SessionScene*>(entry);
      //因为场景连接断开时,SessionScene的ServerClient已设置为NULL，所以scene->task->getName()取不到该连接的名字,这里把连接为空的场景全部删除
      if (scene && scene->task && scene->task->getTask()==NULL)
      {
        XLOG << "[注销场景]," << scene->id << "," << scene->name << XEND;
        SessionSceneManager::instance()->delScene(scene);
      }
      return true;
    }
  };
  DelSceneCallBack callBack;
  forEach(callBack);
}

SessionScene* SessionSceneManager::mapRedirect(SessionScene *scene)
{
  if (!scene || !scene->getTask())
    return nullptr;

  DWORD id = VIRTUAL_SCENE_ID + atoi(scene->getTask()->getName()+11);
  SessionScene *pScene = getSceneByID(id);
  if (pScene && !pScene->isDScene())
  {
    if (pScene->task != scene->task)
      return nullptr;
    return pScene;
  }

  return nullptr;
}

SessionScene* SessionSceneManager::getSceneByID(DWORD id)
{
  return (SessionScene*)getEntryByID(id);
}

SessionScene* SessionSceneManager::getSceneByName(const char *name)
{
  return (SessionScene*)getEntryByName(name);
}

SessionScene* SessionSceneManager::getSceneByGuildID(QWORD qwGuildID)
{
  auto m = m_oGuildRaidList.find(qwGuildID);
  if (m == m_oGuildRaidList.end() || m->second.empty() == true)
    return nullptr;

  return (SessionScene*)(getEntryByID(m->second.begin()->second.dwSceneID));
}

SessionScene* SessionSceneManager::createScene(DWORD id, const char* name, ServerTask* task)
{
  if (id == 0 || name == nullptr || task == nullptr)
    return nullptr;
  return NEW SessionScene(id, name, task);
}

DWORD SessionSceneManager::createRaidMap(Cmd::CreateRaidMapSessionCmd &cmd, bool isRetry/*=false*/)
{
  DWORD sceneID = 0;
  const Cmd::RaidMapData &data = cmd.data();
  switch ((ERaidRestrict)(data.restrict()))
  {
    case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
    case ERAIDRESTRICT_HONEYMOON:
      {
        if (data.teamid() == 0)
          return 0;
        auto it = m_oTeamRaidList.find(data.teamid());
        if (it != m_oTeamRaidList.end())
        {
          auto m = it->second.find(data.raidid());
          if (m != it->second.end())
          {
            SessionScene* pScene = getSceneByID(m->second.dwSceneID);
            if (pScene != nullptr)
            {
              sceneID = pScene->id;
            }
            else
            {
              if (xTime::getCurSec() - m->second.dwTime < 10)
                return 0;

              it->second.erase(m);
            }
          }
        }
      }
      break;
    case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
      {
        if (data.charid() == 0)
          return 0;

        auto it = m_oPrivateRaidList.find(data.charid());
        if (it != m_oPrivateRaidList.end())
        {
          auto m = it->second.find(data.raidid());
          if (m != it->second.end())
          {
            SessionScene* pScene = getSceneByID(m->second.dwSceneID);
            if (pScene != nullptr)
            {
              sceneID = pScene->id;
            }
            else
            {
              if (xTime::getCurSec() - m->second.dwTime < 10)
                return 0;

              it->second.erase(m);
            }
          }
        }
      }
      break;
    case ERAIDRESTRICT_SYSTEM://RaidMapRestrict::System:
      break;
    case ERAIDRESTRICT_GUILD:
      {
        if (data.guildinfo().id() == 0)
          return 0;
        auto it = m_oGuildRaidList.find(data.guildinfo().id());
        if (it != m_oGuildRaidList.end())
        {
          auto m = it->second.find(data.raidid());
          if (m != it->second.end())
          {
            SessionScene* pScene = getSceneByID(m->second.dwSceneID);
            if (pScene != nullptr)
            {
              sceneID = pScene->id;
            }
            else
            {
              if (m->second.stat == ESCENESTAT_CREATING && isRetry == false)
              {
                XLOG << "[公会领地-进入] 正在创建,放入重试列表" << data.guildinfo().id() << "sceneid" << m->second.dwSceneID << XEND;
                pushRetryList(m->second.dwSceneID, cmd);
                return 0;
              }
              else
              {
                it->second.erase(m);
              }
            }
          }
        }
      }
      break;
    case ERAIDRESTRICT_GUILD_TEAM:
      {
        if (data.guildinfo().id() == 0 || data.teamid() == 0)
          return 0;
        std::string key = generateGuildTeamKey(data.guildinfo().id(), data.teamid());
        auto it = m_oGuildTeamRaidList.find(key);
        if (it != m_oGuildTeamRaidList.end())
        {
          auto m = it->second.find(data.raidid());
          if (m != it->second.end())
          {
            SessionScene* pScene = getSceneByID(m->second.dwSceneID);
            if (pScene != nullptr)
            {
              sceneID = pScene->id;
            }
            else
            {
              if (xTime::getCurSec() - m->second.dwTime < 1)
                return 0;

              it->second.erase(m);
            }
          }
        }
      }
      break;
    case ERAIDRESTRICT_USER_TEAM:
    {
      if (data.charid() == 0)
        return 0;

      auto it = m_oUserTeamRaidList.find(data.charid());
      if (it != m_oUserTeamRaidList.end())
      {
        auto m = it->second.find(data.raidid());
        if (m != it->second.end())
        {
          SessionScene* pScene = getSceneByID(m->second.dwSceneID);
          if (pScene != nullptr)
          {
            sceneID = pScene->id;
          }
          else
          {
            if (xTime::getCurSec() - m->second.dwTime < 10)
              return 0;

            it->second.erase(m);
          }
        }
      }

      if (sceneID == 0)
      {
        SessionUser* pUser = SessionUserManager::getMe().getUserByID(data.charid());
        if (pUser == nullptr || pUser->getTeamID() == 0)
          break;
        GTeam& userteam = pUser->getTeam();
        for (auto &m : userteam.getTeamMemberList())
        {
          if (data.charid() == m.first)
            continue;
          auto it = m_oUserTeamRaidList.find(m.first);
          if (it == m_oUserTeamRaidList.end())
            continue;
          auto iter = it->second.find(data.raidid());
          if (iter == it->second.end())
            continue;
          SessionScene* pScene = getSceneByID(iter->second.dwSceneID);
          if (pScene)
          {
            sceneID = pScene->id;
            break;
          }
        }
      }
    }
    break;    
    case ERAIDRESTRICT_PVP_ROOM:
    {
      DWORD roomid = data.roomid();
#ifdef _DEBUG
      if (roomid == 0 && data.raidid() == 9090)
      {
        for (auto &t : m_oPvpRoomRaidList)
        {
          auto it = t.second.find(data.raidid());
          if (it != t.second.end())
          {
            roomid = t.first;
            break;
          }
        }
      }
#endif
      if (roomid == 0)
        return 0;
      auto it = m_oPvpRoomRaidList.find(roomid);
      if (it != m_oPvpRoomRaidList.end())
      {
        auto m = it->second.find(data.raidid());
        if (m != it->second.end())
        {
          SessionScene* pScene = getSceneByID(m->second.dwSceneID);
          if (pScene != nullptr)
          {
            sceneID = pScene->id;
          }
          else
          {            
            if (m->second.stat == ESCENESTAT_CREATING && isRetry == false)
            {
              XDBG << "[场景-进入]进入一个正在创建的场景 roomid,放入重试列表" << roomid << "sceneid" << m->second.dwSceneID << XEND;
              pushRetryList(m->second.dwSceneID, cmd);
              return 0;
            }
            else
            {
              it->second.erase(m);
            }
          }
        }
      }
    }
    break;

    case ERAIDRESTRICT_GUILD_RANDOM_RAID:
    {
      if (data.guildraidindex() == 0 || data.guildinfo().id() == 0 || data.teamid() == 0)
        return 0;
      string key = genGuildRandRaidKey(data.guildinfo().id(), data.teamid());
      auto it = m_oGuildRandomRaidList.find(key);
      if (it != m_oGuildRandomRaidList.end())
      {
        auto m = it->second.find(data.guildraidindex());
        if (m != it->second.end())
        {
          SessionScene* pScene = getSceneByID(m->second.dwSceneID);
          if (pScene != nullptr)
          {
            sceneID = pScene->id;
          }
          else
          {
            if (xTime::getCurSec() - m->second.dwTime < 10)
              return 0;

            it->second.erase(m);
          }
        }
      }
    }
    break;
    case ERAIDRESTRICT_GUILD_FIRE:
    {
      if (data.guildraidindex() == 0)
        return 0;
      auto it = m_oGuildFireRaidList.find(data.guildraidindex());
      if (it != m_oGuildFireRaidList.end())
      {
        auto m = it->second.find(data.raidid());
        if (m != it->second.end())
        {
          SessionScene* pScene = getSceneByID(m->second.dwSceneID);
          if (pScene != nullptr)
          {
            sceneID = pScene->id;
          }
          else
          {
            if (xTime::getCurSec() - m->second.dwTime < 10)
              return 0;

            it->second.erase(m);
          }
        }
      }
    }
    break;
    case ERAIDRESTRICT_WEDDING:
    {
      if (data.roomid() == 0)
        return 0;
      auto it = m_oWeddingRaidList.find(data.roomid());
      if (it != m_oWeddingRaidList.end())
      {
        auto m = it->second.find(data.raidid());
        if (m != it->second.end())
        {
          SessionScene* pScene = getSceneByID(m->second.dwSceneID);
          if (pScene != nullptr)
          {
            sceneID = pScene->id;
          }
          else
          {
            if (xTime::getCurSec() - m->second.dwTime < 10)
              return 0;

            it->second.erase(m);
          }
        }
      }
    }
    break;
    case ERAIDRESTRICT_MIN:
    case ERAIDRESTRICT_MAX:
      return 0;
  }

  if (cmd.data().follow())
  {
    if (!sceneID) return 0;
  }

  if (sceneID)
  {
    Cmd::ChangeSceneSessionCmd message;
    message.set_mapid(sceneID);
    message.add_charid(data.charid());
    for (int i = 0; i < cmd.data().memberlist_size(); ++i)
    {
      if (data.charid() == cmd.data().memberlist(i))
        continue;
      message.add_charid(cmd.data().memberlist(i));
    }

    if (cmd.data().has_enterpos())
    {
      message.mutable_pos()->CopyFrom(cmd.data().enterpos());
    }
    PROTOBUF(message, send, len);
    thisServer->doSessionCmd((const BYTE *)send, len);
    return sceneID;
  }

  ServerTask* server = getASceneServer(cmd.data().charid(), &data);
  if (!server) return 0;

  DWORD index = getDMapIndex();
  if (index == 0 || index == DWORD_MAX)
    return 0;

  using namespace Cmd;
  RaidMapData *pData = cmd.mutable_data();
  if (pData)
    pData->set_index(index);
  PROTOBUF(cmd, send, len);
  server->sendCmd(send, len);

  switch ((ERaidRestrict)(data.restrict()))
  {
    case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
    case ERAIDRESTRICT_HONEYMOON:
      {
        TMapSceneInfo& mapInfo = m_oTeamRaidList[data.teamid()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
      {
        TMapSceneInfo& mapInfo = m_oPrivateRaidList[data.charid()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_SYSTEM://RaidMapRestrict::System:
      break;
    case ERAIDRESTRICT_GUILD:
      {
        TMapSceneInfo& mapInfo = m_oGuildRaidList[data.guildinfo().id()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
        rInfo.oGuildInfo.CopyFrom(data.guildinfo());
      }
      break;
    case ERAIDRESTRICT_GUILD_TEAM:
      {
        string key = generateGuildTeamKey(data.guildinfo().id(), data.teamid());
        TMapSceneInfo& mapInfo = m_oGuildTeamRaidList[key];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_USER_TEAM:
      {
        TMapSceneInfo& mapInfo = m_oUserTeamRaidList[data.charid()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_PVP_ROOM:
     {
      TMapSceneInfo& mapInfo = m_oPvpRoomRaidList[data.roomid()];
      SSceneInfo& rInfo = mapInfo[data.raidid()];
      rInfo.dwRaidID = data.raidid();
      rInfo.dwSceneID = index;
      rInfo.qwRoomId = data.roomid();
      rInfo.dwTime = xTime::getCurSec();
      rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_GUILD_RANDOM_RAID:
      {
        string key = genGuildRandRaidKey(data.guildinfo().id(), data.teamid());
        TMapSceneInfo& mapInfo = m_oGuildRandomRaidList[key];
        SSceneInfo& rInfo = mapInfo[data.guildraidindex()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
      }
      break;
    case ERAIDRESTRICT_GUILD_FIRE:
      {
        TMapSceneInfo& mapInfo = m_oGuildFireRaidList[data.guildraidindex()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_WEDDING:
      {
        TMapSceneInfo& mapInfo = m_oWeddingRaidList[data.roomid()];
        SSceneInfo& rInfo = mapInfo[data.raidid()];
        rInfo.dwRaidID = data.raidid();
        rInfo.dwSceneID = index;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_CREATING;
      }
      break;
    case ERAIDRESTRICT_MIN:
    case ERAIDRESTRICT_MAX:
      return 0;
  }

  return index;
}

DWORD SessionSceneManager::getDMapIndex()
{
  return GuidManager::getMe().getNextRaidID();
}

void SessionSceneManager::putDMapIndex(DWORD id)
{
  //dMapIDManager->putUniqueID(id);
}

ServerTask* SessionSceneManager::getASceneServer(QWORD userid, const Cmd::RaidMapData* pRaidData)
{
  std::vector<ServerTask *> list;

  thisServer->getConnectedServer("SceneServer", "", list);
  if (list.empty()) return nullptr;

  // 决战专用场景
  if (CommonConfig::m_strSuperGvgSceneName.empty() == false)
  {
    auto it = find_if(list.begin(), list.end(), [&](ServerTask * s)->bool{
        return s && s->getName() == CommonConfig::m_strSuperGvgSceneName;
        });
    if (it != list.end())
    {
      if (isSuperGvgRaid(pRaidData))
        return *it;
      else
        list.erase(it);
    }
  }

  if (pRaidData != nullptr)
  {
    switch((ERaidRestrict)pRaidData->restrict())
    {
      case ERAIDRESTRICT_GUILD_FIRE:
        {
          auto it = m_oGuildFireRaidList.find(pRaidData->guildraidindex());
          if (it != m_oGuildFireRaidList.end())
          {
            // 同城的3个副本分配到相同场景
            for (auto &m : it->second)
            {
              SessionScene* pScene = getSceneByID(m.second.dwSceneID);
              if (pScene)
              {
                return pScene->getTask();
              }
            }
          }
          else if (!m_oGuildFireRaidList.empty())
          {
            // 不同城池副本平均到各个场景
            std::map<ServerTask*, DWORD> task2CityNum;
            for (auto &v : list)
              task2CityNum[v] = 0;

            for (auto &m : m_oGuildFireRaidList)
            {
              for (auto &n : m.second)
              {
                SessionScene* pScene = getSceneByID(n.second.dwSceneID);
                if (pScene && pScene->getTask())
                {
                  task2CityNum[pScene->getTask()] ++;
                  break;
                }
              }
            }
            ServerTask* task = nullptr;
            DWORD minnum = DWORD_MAX;
            for (auto &m : task2CityNum)
            {
              if (m.second < minnum)
              {
                minnum = m.second;
                task = m.first;
              }
            }
            if (task)
              return task;
          }
        }
        break;
      default:
        break;
    }
  }

  ServerTask* pOldServer = nullptr;
  ServerTask* pMinServer = nullptr;
  DWORD dwOldUserNum = 0;
  DWORD dwMinUserNum = 0;

  // 玩家数量在一定范围之内, 不切换进程
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(userid);
  if (pUser && pUser->getScene() && pUser->getScene()->getTask())
  {
    pOldServer = pUser->getScene()->getTask();

    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SCENEUSER_NUM, thisServer->getZoneID(), pOldServer->getName());
    RedisManager::getMe().getData(key, dwOldUserNum);

    if (dwOldUserNum < MIN_USERNUM_CHANGE_SCENE)
      return pOldServer;
  }

  TVecDWORD vecUserNum;
  for (auto &s : list)
  {
    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_SCENEUSER_NUM, thisServer->getZoneID(), s->getName());
    DWORD num = 0;
    RedisManager::getMe().getData(key, num);
    vecUserNum.push_back(num);
  }
  if (vecUserNum.empty())
    return nullptr;

  auto it = std::min_element(vecUserNum.begin(), vecUserNum.end());
  if (it == vecUserNum.end())
    return nullptr;
  dwMinUserNum = *it;
  DWORD index = std::distance(std::begin(vecUserNum), it);
  pMinServer = list[index];

  // 非玩家创建副本, 取玩家数量最小的场景
  if (pOldServer == nullptr)
    return pMinServer;

  // 当前场景玩家数量 大于 最小场景 一定数量时, 取最小场景
  return (dwOldUserNum > dwMinUserNum + MIN_DIFVALUE_CHANGE_SCENE) ? pMinServer : pOldServer;
}

void SessionSceneManager::onRaidSceneOpen(SessionScene *scene)
{
  if (!scene) return;

  switch (scene->m_oRaidData.m_oRestrict)
  {
    case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
    case ERAIDRESTRICT_HONEYMOON:
      {
        TMapSceneInfo& mapInfo = m_oTeamRaidList[scene->m_oRaidData.m_qwTeamID];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_OK;

        // check tower
        if (scene->m_oRaidData.m_dwLayer != 0)
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
          cmd.set_state(EDOJOSTATE_OPEN);
          cmd.set_raidid(scene->m_oRaidData.m_dwRaidID);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
        }
        if (scene->m_oRaidData.m_qwRoomId)
        {
          const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(scene->m_oRaidData.m_dwRaidID);
          if (pBase && pBase->eRaidType == ERAIDTYPE_PVECARD)
          {
            CardSceneSyncSocialCmd cmd;
            cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
            cmd.set_open(true);
            PROTOBUF(cmd, send, len);
            thisServer->sendCmdToServer(send, len, "TeamServer");
          }
        }
        const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(scene->m_oRaidData.m_dwRaidID);
        if (pBase && pBase->eRaidType == ERAIDTYPE_ALTMAN)
        {
          TeamRaidSceneSyncSocialCmd cmd;
          cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
          cmd.set_open(true);
          cmd.set_raid_type(pBase->eRaidType);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
        }
      }
      break;
    case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
      {
        TMapSceneInfo& mapInfo = m_oPrivateRaidList[scene->m_oRaidData.m_qwCharID];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_OK;
      }
      break;
    case ERAIDRESTRICT_SYSTEM://RaidMapRestrict::System:
      break;
    case ERAIDRESTRICT_GUILD:
      {
        TMapSceneInfo& mapInfo = m_oGuildRaidList[scene->m_oRaidData.m_qwGuildID];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_OK;
      }
      break;
    case ERAIDRESTRICT_GUILD_TEAM:
      {
        string key = generateGuildTeamKey(scene->m_oRaidData.m_qwGuildID, scene->m_oRaidData.m_qwTeamID);
        TMapSceneInfo& mapInfo = m_oGuildTeamRaidList[key];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = xTime::getCurSec();
        rInfo.stat = ESCENESTAT_OK;
        // check dojo
        if (DojoConfig::getMe().isDojoMap(scene->m_oRaidData.m_dwRaidID))
        {
          DojoStateNtfSocialCmd cmd;
          cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
          cmd.set_guildid(scene->m_oRaidData.m_qwGuildID);
          cmd.set_state(EDOJOSTATE_OPEN);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
          XLOG << "[道场-开启] 道场副本真正开启 dmapid:" << scene->m_oRaidData.m_dwRaidID << XEND;
        }
      }
      break;
    case ERAIDRESTRICT_PVP_ROOM:
    {
      TMapSceneInfo& mapInfo = m_oPvpRoomRaidList[scene->m_oRaidData.m_qwRoomId];
      SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
      rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
      rInfo.dwSceneID = scene->id;
      rInfo.qwRoomId = scene->m_oRaidData.m_qwRoomId;
      rInfo.dwTime = xTime::getCurSec();
      rInfo.stat = ESCENESTAT_OK;

      // 通知match, 副本开启
      {
        SyncRaidSceneMatchSCmd cmd;
        cmd.set_roomid(scene->m_oRaidData.m_qwRoomId);
        cmd.set_open(true);
        cmd.set_zoneid(thisServer->getZoneID());
        cmd.set_sceneid(scene->id);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmd(ClientType::match_server, send, len);
      }
    }
    break;    
    case ERAIDRESTRICT_GUILD_RANDOM_RAID:
      {
        string key = genGuildRandRaidKey(scene->m_oRaidData.m_qwGuildID, scene->m_oRaidData.m_qwTeamID);
        TMapSceneInfo& mapInfo = m_oGuildRandomRaidList[key];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwGuildRaidIndex];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = now();
      }
      break;
    case ERAIDRESTRICT_GUILD_FIRE:
      {
        TMapSceneInfo& mapInfo = m_oGuildFireRaidList[scene->m_oRaidData.m_dwGuildRaidIndex];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = now();
        rInfo.stat = ESCENESTAT_OK;
      }
      break;
    case ERAIDRESTRICT_WEDDING:
      {
        TMapSceneInfo& mapInfo = m_oGuildFireRaidList[scene->m_oRaidData.m_qwRoomId];
        SSceneInfo& rInfo = mapInfo[scene->m_oRaidData.m_dwRaidID];
        rInfo.dwRaidID = scene->m_oRaidData.m_dwRaidID;
        rInfo.dwSceneID = scene->id;
        rInfo.dwTime = now();
        rInfo.stat = ESCENESTAT_OK;
      }
      break;
    default:
      break;
  }

  if (scene->m_oRaidData.m_dwRaidID != 0)
    m_mapDMapRaid[scene->id] = scene->m_oRaidData.m_dwRaidID;

  popRetryList(scene->id);
}

void SessionSceneManager::onRaidSceneClose(SessionScene *scene)
{
  if (!scene) return;

  switch (scene->m_oRaidData.m_oRestrict)
  {
    case ERAIDRESTRICT_PRIVATE://RaidMapRestrict::Private:
      {
        auto m = m_oPrivateRaidList.find(scene->m_oRaidData.m_qwCharID);
        if (m != m_oPrivateRaidList.end())
        {
          auto n = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (n != m->second.end())
            m->second.erase(n);
        }
      }
      break;
    case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
    case ERAIDRESTRICT_HONEYMOON:
      {
        // check tower
        if (scene->m_oRaidData.m_dwLayer != 0)
        {
          TowerSceneSyncSocialCmd cmd;
          cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
          cmd.set_state(EDOJOSTATE_CLOSE);
          cmd.set_raidid(scene->m_oRaidData.m_dwRaidID);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
        }
        if (scene->m_oRaidData.m_qwRoomId)
        {
          const SRaidCFG* pBase = MapConfig::getMe().getRaidCFG(scene->m_oRaidData.m_dwRaidID);
          if (pBase && pBase->eRaidType == ERAIDTYPE_PVECARD)
          {
            CardSceneSyncSocialCmd cmd;
            cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
            cmd.set_open(false);
            PROTOBUF(cmd, send, len);
            thisServer->sendCmdToServer(send, len, "TeamServer");
          }
        }

        auto m = m_oTeamRaidList.find(scene->m_oRaidData.m_qwTeamID);
        if (m != m_oTeamRaidList.end())
        {
          auto n = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (n != m->second.end())
            m->second.erase(n);
        }
      }
      break;
    case ERAIDRESTRICT_GUILD:
      {
        auto m = m_oGuildRaidList.find(scene->m_oRaidData.m_qwGuildID);
        if (m != m_oGuildRaidList.end())
        {
          auto n = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (n != m->second.end())
            m->second.erase(n);
        }
      }
      break;
    case ERAIDRESTRICT_GUILD_TEAM:
      {
        // check dojo
        if (DojoConfig::getMe().isDojoMap(scene->m_oRaidData.m_dwRaidID))
        {
          /*Team* pTeam = TeamManager::getMe().getTeamByID(scene->m_oRaidData.m_qwTeamID);
          if (pTeam)
          {
            XLOG << "[道场-关闭] 道场副本真正关闭 dmapid:" << scene->m_oRaidData.m_dwRaidID << XEND;
            pTeam->setDojoClose(scene->m_oRaidData.m_qwGuildID);
          }*/

          DojoStateNtfSocialCmd cmd;
          cmd.set_teamid(scene->m_oRaidData.m_qwTeamID);
          cmd.set_guildid(scene->m_oRaidData.m_qwGuildID);
          cmd.set_state(EDOJOSTATE_CLOSE);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToServer(send, len, "TeamServer");
          XLOG << "[道场-关闭] 道场副本真正关闭 dmapid:" << scene->m_oRaidData.m_dwRaidID << XEND;
        }
      }
      break;
    case ERAIDRESTRICT_USER_TEAM:
      {
        auto m = m_oUserTeamRaidList.find(scene->m_oRaidData.m_qwCharID);
        if (m != m_oUserTeamRaidList.end())
        {
          auto n = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (n != m->second.end())
            m->second.erase(n);
        }
      }
      break;
    case ERAIDRESTRICT_PVP_ROOM:
    {      
      auto m = m_oPvpRoomRaidList.find(scene->m_oRaidData.m_qwRoomId);
      if (m != m_oPvpRoomRaidList.end())
      {
        auto n = m->second.find(scene->m_oRaidData.m_dwRaidID);
        if (n != m->second.end())
        {
          SyncRaidSceneMatchSCmd cmd;
          cmd.set_roomid(scene->m_oRaidData.m_qwRoomId);
          cmd.set_open(false);
          cmd.set_zoneid(thisServer->getZoneID());
          PROTOBUF(cmd, send, len);
          thisServer->sendCmd(ClientType::match_server, send, len);
          m->second.erase(n);
        }
      }
    }
    break;
    case ERAIDRESTRICT_GUILD_RANDOM_RAID:
      {
        string key = genGuildRandRaidKey(scene->m_oRaidData.m_qwGuildID, scene->m_oRaidData.m_qwTeamID);
        auto m = m_oGuildRandomRaidList.find(key);
        if (m != m_oGuildRandomRaidList.end())
        {
          auto it = m->second.find(scene->m_oRaidData.m_dwGuildRaidIndex);
          if (it != m->second.end())
            m->second.erase(it);
        }
      }
      break;
    case ERAIDRESTRICT_GUILD_FIRE:
      {
        auto m = m_oGuildFireRaidList.find(scene->m_oRaidData.m_dwGuildRaidIndex);
        if (m != m_oGuildFireRaidList.end())
        {
          auto it = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (it != m->second.end())
            m->second.erase(it);
        }
      }
      break;
    case ERAIDRESTRICT_WEDDING:
      {
        auto m = m_oWeddingRaidList.find(scene->m_oRaidData.m_qwRoomId);
        if (m != m_oWeddingRaidList.end())
        {
          auto it = m->second.find(scene->m_oRaidData.m_dwRaidID);
          if (it != m->second.end())
            m->second.erase(it);
        }
      }
      break;
    default:
      break;
  }
}

DWORD SessionSceneManager::getMapIDByDMap(DWORD id) const
{
  auto m = m_mapDMapRaid.find(id);
  if (m == m_mapDMapRaid.end())
    return 0;

  const SRaidCFG* pRaidCFG = MapConfig::getMe().getRaidCFG(m->second);
  if (pRaidCFG == nullptr)
    return 0;
  const SMapCFG* pMapCFG = MapConfig::getMe().getMapCFG(pRaidCFG->dwMapID);
  return pMapCFG == nullptr ? 0 : pMapCFG->dwReliveMap;
}

std::string SessionSceneManager::generateGuildTeamKey(QWORD guildId, QWORD teamId)
{
  std::stringstream ss;
  ss << guildId << "," << teamId;
  return ss.str();
}

void SessionSceneManager::pushRetryList(QWORD sceneId, Cmd::CreateRaidMapSessionCmd& cmd)
{
  auto it = m_mapRetryCmd.find(sceneId);
  if (it == m_mapRetryCmd.end())
  {
    std::list<Cmd::CreateRaidMapSessionCmd> listCmd;
    listCmd.push_back(cmd);
    m_mapRetryCmd.insert(std::make_pair(sceneId, listCmd));
    return;
  }  
  it->second.push_back(cmd);
  XLOG << "[场景-进入] 插入重试列表 sceneId" << sceneId << "size" << it->second.size() << XEND;
}

void SessionSceneManager::popRetryList(QWORD sceneId)
{
  auto it = m_mapRetryCmd.find(sceneId);
  if (it == m_mapRetryCmd.end())
    return;

  for (auto &v : it->second)
  {
    createRaidMap(v, true);
  } 
  
  XLOG << "[场景-进入] 重试列表处理完毕 sceneId" << sceneId << "size" << it->second.size() << XEND;
  m_mapRetryCmd.erase(sceneId);
}

void SessionSceneManager::clearPvpRoom()
{
  XLOG << "MatcherServer宕机，清除场景数据,size" << m_oPvpRoomRaidList.size() << XEND;
  m_oPvpRoomRaidList.clear();
}

void SessionSceneManager::clearOnePvpRoom(SessionScene *pScene)
{
  if (!pScene)
    return;
  m_oPvpRoomRaidList.erase(pScene->m_oRaidData.m_qwRoomId);
}

std::string SessionSceneManager::genGuildRandRaidKey(QWORD guildid, QWORD teamid)
{
  std::stringstream ss;
  ss.str("");
  //ss << randindex << "_" << guildid << "_" << teamid;
  ss << guildid << "_" << teamid;
  return ss.str();
}

void SessionSceneManager::closeGuildRaidGroup(QWORD guildid, QWORD teamid, DWORD index)
{
  string key = genGuildRandRaidKey(guildid, teamid);
  auto m = m_oGuildRandomRaidList.find(key);
  if (m == m_oGuildRandomRaidList.end())
    return;

  GuildRaidCloseSessionCmd cmd;
  for (auto &it : m->second)
  {
    if (it.first / ONE_THOUSAND != index / ONE_THOUSAND)
      continue;
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(it.second.dwSceneID);
    if (!pScene)
      continue;
    cmd.set_mapid(pScene->id);
    PROTOBUF(cmd, send, len);
    pScene->sendCmd(send, len);
  }
}

bool SessionSceneManager::isSuperGvgRaid(const Cmd::RaidMapData* pRaidData) const
{
  if (pRaidData == nullptr)
    return false;
  if ((ERaidRestrict)pRaidData->restrict() != ERAIDRESTRICT_PVP_ROOM)
    return false;
  const SRaidCFG* base = MapConfig::getMe().getRaidCFG(pRaidData->raidid());
  return base && base->eRaidType == ERAIDTYPE_SUPERGVG;
}

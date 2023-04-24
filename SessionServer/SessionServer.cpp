#include "SessionServer.h"
#include "RegCmd.h"
#include "xNetProcessor.h"
#include "SessionUser.h"
#include "SessionSceneManager.h"
#include "SessionUserManager.h"
#include "GatewayCmd.h"
#include "MiscConfig.h"
#include "TableManager.h"
#include "Boss.h"
#include "Mail.h"
#include "TowerManager.h"
#include "LuaManager.h"
#include "UserActive.h"
#include "SessionActivityMgr.h"
#include "SealManager.h"
#include "TimerM.h"
#include "ChatManager_SE.h"
#include "MatchSCmd.pb.h"
#include "ActivityManager.h"
#include "ShopMgr.h"
#include "SessionWeddingMgr.h"
#include "SessionThread.h"
#include "WorldLevelManager.h"
#include "GlobalManager.h"

SessionActivityMgr& getSeActMgr()
{
  return SessionActivityMgr::getMe();
}

//SessionServer *thisServer = 0;
extern ConfigEnum ConfigEnums[];

SessionServer::SessionServer(OptArgs &args):ZoneServer(args),m_oTickOneSec(1),m_oTickOneMin(60),m_oTickOneHour(3600)
{
}

SessionServer::~SessionServer()
{

}

bool SessionServer::loadConfig()
{
  bool bResult = true;

  // 配置函数注册
  ConfigEnums[boss].loadfunc = []()
  {
    return BossList::getMe().loadConfig();
  };
  ConfigEnums[timer].loadfunc = []()
  {
    return TimerM::getMe().load();
  };

  ConfigEnums[activity].loadfunc = []()
  {
    SessionActivityMgr::getMe().reload();   //reload is diff from load
    return true;
  };
  ConfigEnums[actshortcutpower].loadfunc = []()
  {
    ActivityManager::getMe().updateShortcutPower();
    return true;
  };

  registerLuaFunc();
  if (LuaManager::getMe().load() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("LuaManager");
  }

  if (ConfigManager::getMe().loadSessionConfig() == false)
  {
    bResult = false;
    CHECK_LOAD_CONFIG("ConfigManager");
  }

  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

void SessionServer::registerLuaFunc()
{
  // LuaManager::getMe().setregister([](){
  //   lua_State* _L = LuaManager::getMe().getLuaState();    
  // });
}
bool SessionServer::v_init()
{
  if (!addDataBase(REGION_DB, true))
  {
    XERR << "[数据库Trade],初始化数据库连接失败:" << REGION_DB << XEND;
    return false;
  }

  bool bResult = true;

  if (loadConfig() == false)
  {
    XERR << "[加载配置]" << "失败" << XEND;
    bResult = false;
  }
  const ServerData& rServerData = xServer::getServerData();
  {
    const TIpPortPair* pPair = rServerData.getIpPort("trade-server");
    if (pPair)
      addClient(ClientType::trade_server, pPair->first, pPair->second);
  }
  {
    const TIpPortPair* pPair = rServerData.getIpPort("GuildServer");
    if (pPair)
      addClient(ClientType::guild_server, pPair->first, pPair->second);
  }
  {
    const TIpPortPair* pPair = rServerData.getIpPort("GlobalServer");
    if (pPair)
      addClient(ClientType::global_server, pPair->first, pPair->second);
  }
  {
    const TIpPortPair* pPair = rServerData.getIpPort("GZoneServer");
    if (pPair)
      addClient(ClientType::gzone_server, pPair->first, pPair->second);
  }

  {
    const TIpPortPair* pPair = rServerData.getIpPort("MatchServer");
    if (pPair)
      addClient(ClientType::match_server, pPair->first, pPair->second);
  }

  {
    const TIpPortPair* pPair = rServerData.getIpPort("AuctionServer");
    if (pPair)
      addClient(ClientType::auction_server, pPair->first, pPair->second);
  }

  {
    const TIpPortPair* pPair = rServerData.getIpPort("WeddingServer");
    if (pPair)
      addClient(ClientType::wedding_server, pPair->first, pPair->second);    
  }

  UserActive::getMe().init();

  if (MailManager::getMe().loadAllSystemMail() == false)
  {
    XERR << "[SessionServer], load all system mail error!" << XEND;
    bResult = false;
  }

  //BossList::getMe().load();

  if (!TowerManager::getMe().loadDataFromDB())
  {
    XERR << "[SessionServer], towermanger init error!" << XEND;
    bResult = false;
  }
  if (!SealManager::getMe().loadDataFromDB())
  {
    XERR << "[SessionServer], seal init error!" << XEND;
    bResult = false;
  }
  if (!GlobalManager::getMe().loadGlobalData())
  {
    XERR << "[SessionServer], global init error!" << XEND;
    bResult = false;
  }

  ActivityManager::getMe().load();

  ShopMgr::getMe().load();

  ActivityEventManager::getMe().load();

  if (!SessionThread::getMe().thread_start())
  {
    XERR << "[SessionThread]" << "创建失败" << XEND;
    return false;
  }

  XLOG << "[SessionServer] init finish" << XEND;
  return BaseConfig::getMe().configCheck() == true ? bResult : true;
}

void SessionServer::init_ok()
{
  ZoneServer::init_ok();
}

void SessionServer::initOkServer(ServerTask *task)
{
  ZoneServer::initOkServer(task);

  if (strncmp(task->getTask()->name, "SceneServer", 11)==0)
  {
    TowerManager::getMe().syncScene(task->getTask());
    //SessionSceneManager::getMe().createScene(task);
    SessionActivityMgr::getMe().init();
    TimerM::getMe().sceneStart(task);
    ShopMgr::getMe().sceneStart(task);
    ChatManager_SE::getMe().queryZoneStatus(xTime::getCurSec());
    SessionUserManager::getMe().syncGlobalBossScene(task);
  }
  if (strncmp(task->getTask()->name, "TeamServer", 10)==0)
  {
    SessionUserManager::getMe().onRegistServer("TeamServer");
  }
  if (strncmp(task->getTask()->name, "SocialServer", 12) == 0)
  {
    SessionUserManager::getMe().onRegistServer("SocialServer");

    bool bStart = SessionActivityMgr::getMe().isOpen(static_cast<DWORD>(GACTIVITY_RECALL));
    if (bStart)
    {
      GlobalActivityStartSessionCmd cmd;
      cmd.set_id(static_cast<DWORD>(GACTIVITY_RECALL));
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");
    }
    else
    {
      GlobalActivityStopSessionCmd cmd;
      cmd.set_id(static_cast<DWORD>(GACTIVITY_RECALL));
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToServer(send, len, "SocialServer");
    }
  }
  XLOG << "[服务器初始化]" << task->getTask()->name << "完成" << XEND;
}

void SessionServer::v_final()
{
  while (!v_stop())
  {
    usleep(100);
  }

  BossList::getMe().delMe();
  SessionSceneManager::getMe().final();
  //ChatManager::getMe().final();
  ZoneServer::v_final();
  SessionThread::getMe().thread_stop();
  SessionThread::getMe().thread_join();
}

bool SessionServer::v_stop()
{
  if (!m_oServerList.empty())
  {
    for (auto it : m_oServerList)
    {
      for (auto sub_it : it.second)
      {
        if (sub_it.second->getTask() && it.first=="SceneServer")
        {
          XLOG << "[进程关闭],等待 " << sub_it.second->getName() << " 断开连接" << XEND;
          return false;
        }
      }
    }
  }
  return true;
}

void SessionServer::v_closeServer(xNetProcessor *np)
{
  if (!np) return;
  if (strcmp(np->name,"SuperServer")==0)
  {
    stop();
  }
  else if (strcmp(np->name,"RecordServer")==0)
  {

  }
  else if (strncmp(np->name,"GateServer", 10)==0)
  {

  }
  else if (strncmp(np->name,"SceneServer", 11)==0)
  {
  }
}

void SessionServer::v_verifyOk(xNetProcessor *task)
{
  if (!task) return;

  if (strncmp(task->name, "SceneServer", 11) == 0)
  {
    ServerTask* net = thisServer->getConnectedServer("SceneServer", task->name);
    if (net)
    {
      ActivityEventManager::getMe().notifyScene(net);
      SessionWeddingMgr::getMe().syncWeddingInfo2Scene(net);
    }
  }
}

void SessionServer::loginErr(QWORD accid, Cmd::RegErrRet ret, ServerTask* net)
{
  if (!net) return;
  using namespace Cmd;
  RegErrUserCmd message;
  message.set_accid(accid);
  message.set_ret(ret);
  PROTOBUF(message, send, len);
  net->sendCmd(send, len);
}

void SessionServer::broadcastScene(unsigned char *buf, unsigned short len)
{
  sendCmdToServer(buf,len,"SceneServer");
}

void SessionServer::broadcastOneLevelIndexCmd(ONE_LEVEL_INDEX_TYPE t, QWORD index, const void *cmd, WORD len, QWORD excludeID, DWORD ip)
{
  DWORD sendlen = sizeof(BroadcastOneLevelIndexGatewayCmd) + len;
  BUFFER_CMD_SIZE(forward, BroadcastOneLevelIndexGatewayCmd, sendlen);
  forward->indexT = t;
  forward->i = index;
  forward->len = len;
  forward->exclude = excludeID;
  bcopy(cmd, forward->data, (DWORD)len);
  sendCmdToGate(forward, sendlen);
}

bool SessionServer::sendCmdToMe(QWORD charid, const void* data, unsigned short len)
{
  SessionUser *pUser = SessionUserManager::getMe().getUserByID(charid);
  if (pUser)
  {
    pUser->sendCmdToMe(data, len);
    return true;
  }

  return false;
}

bool SessionServer::sendCmdToLoginUser(QWORD accid, const void* data, unsigned short len, ServerTask *task)
{
  if (!task) return false;

  BUFFER_CMD_SIZE(send, ForwardToLoginUserGatewayCmd, sizeof(ForwardToLoginUserGatewayCmd) + len);
  send->accid = accid;
  send->len = len;
  bcopy(data, send->data, len);
  task->sendCmd(send, sizeof(ForwardToLoginUserGatewayCmd) + len);

  return true;
}

//通过global server转达消息给scene
bool SessionServer::sendCmdToWorldUser(QWORD charid, const void *buf, WORD len, Cmd::EDir dir)
{
  Cmd::GlobalForwardCmdSocialCmd message;
  message.set_charid(charid);
  message.set_data(buf, len);
  message.set_len(len);
  message.set_dir(dir);
  PROTOBUF(message, send, len2);
  if (thisServer->sendCmd(ClientType::global_server, send, len2) == false)
  {
    return false;
  }
  return true;
}

bool SessionServer::doGatewayCmd(const BYTE* buf, WORD len)
{
  GatewayCmd* cmd = (GatewayCmd*)buf;
  if (cmd == nullptr || len == 0)
    return false;

  switch (cmd->param)
  {
    case FORWARD_USERCMD_GATEWAYCMD:
      {
        ForwardUserCmdGatewayCmd* rev = (ForwardUserCmdGatewayCmd *)cmd;

        if (rev->len < 2)
        {
          XERR << "[消息处理]," << rev->accid << ",长度小于2" << XEND;
          return false;
        }

        m_oMessageStat.start(rev->data[0], rev->data[1]);

        SessionUser *pUser = SessionUserManager::getMe().getUserByAccID(rev->accid);
        if (pUser)
        {
          if (!pUser->doSessionUserCmd(rev->data, rev->len))
          {
            XERR << "[消息处理]," << pUser->accid << "," << pUser->id << "," << pUser->name << ",处理错误," << rev->data[0] << "," << rev->data[1] << XEND;
          }
        }
        else
        {
          XERR << "[消息处理]," << rev->data[0] << "," << rev->data[1] << ",找不到角色(accid:" << rev->accid << ")" << XEND;
        }

        m_oMessageStat.end();

        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

void SessionServer::onRegistRegion(ClientType type)
{
  switch (type)
  {
    case ClientType::global_server:
    case ClientType::guild_server:
      {
        SessionUserManager::getMe().registRegion(type);

        GuildCityActionGuildSCmd cmd;
        cmd.set_action(EGUILDCITYACTION_GUILD_QUERY);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToData(send, len);
        XDBG << "[公会城池-请求] QueryGuildCityGuildSCmd 请求发送" << XEND;
      }
      break;
    case ClientType::match_server:
    {
      SessionUserManager::getMe().registRegion(type);
      SessionActivityMgr::getMe().registRegion(type);

      XDBG << "[斗技场-向Matchserver注册 成功] category" << getZoneCategory() << "zoneid" << this->getZoneID() << XEND;
     // if (getZoneCategory())
      {
        //register category
        RegPvpZoneMatch cmd;
        cmd.set_category(getZoneCategory());
        cmd.set_zoneid(this->getZoneID());
        PROTOBUF(cmd, send, len);
        bool ret = thisServer->sendCmd(ClientType::match_server, (BYTE*)send, len);
        m_dwMatchServerConnectCount++;
        XLOG << "[斗技场-向Matchserver注册] category" << getZoneCategory() << "zoneid" << this->getZoneID() <<"m_dwMatchServerConnectCount"<< m_dwMatchServerConnectCount <<"ret" << ret << XEND;
        if (m_dwMatchServerConnectCount > 1)
        {
          //clear room
          SessionSceneManager::getMe().clearPvpRoom();
        }
      }           
    }
    break;
    case ClientType::wedding_server:
    {
      SessionUserManager::getMe().registRegion(type);
    }
    break;
    case ClientType::stat_server:
    {
      WorldLevelManager::getMe().registRegion(type);
    }
    break;
    default:
      break;
  }
}

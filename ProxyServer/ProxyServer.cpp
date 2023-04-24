#include "ProxyServer.h"
#include "xDBConnPool.h"
#include "ProxyUser.h"
#include "ProxyUserManager.h"
#include "xNetProcessor.h"
#include "RedisManager.h"
#include "LoginUserCmd.pb.h"
#include "xlib/xSha1.h"
#include "SystemCmd.pb.h"
#include "CommonConfig.h"
#include "ProxyUserDataThread.h"

//ProxyServer* thisServer = 0;

ProxyServer::ProxyServer(OptArgs &args) : xServer(args)
{
  setTCPTaskMax(PROXY_SERVER_TCP_TASK_LIMIT);
}

void ProxyServer::v_final()
{
  ProxyUserDataThread::getMe().thread_stop();
  ProxyUserDataThread::getMe().thread_join();
}

bool ProxyServer::init()
{
  if (!addDataBase(RO_DATABASE_NAME, false))
  {
    XERR << "[加载RO信息],初始化数据库连接失败:" << RO_DATABASE_NAME << XEND;
    return false;
  }
  XLOG << "[加载RO信息],加载数据库" << RO_DATABASE_NAME << XEND;

  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "proxy"); // 启动加载
  if (field)
  {
    {
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "id = %u", atoi(s_oOptArgs.m_strProxyID.c_str()));

      xRecordSet set;
      QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, nullptr);
      if (QWORD_MAX != ret)
      {
        if (ret)
        {
          setServerPort(set[0].get<DWORD>("port"));
          XLOG << "[加载Proxy信息],加载port:" << set[0].get<DWORD>("port") << XEND;
        }
      }
      else
      {
        return false;
      }
    }

    {
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "id = 0");

      xRecordSet set;
      QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where, nullptr);
      if (QWORD_MAX != ret)
      {
        if (ret)
        {
          m_strGProxyIP = set[0].getString("ip");
          m_dwGProxyPort = set[0].get<DWORD>("port");
          addClient(ClientType::g_proxy_server, m_strGProxyIP, m_dwGProxyPort);
          XLOG << "[加载Proxy信息],加载port:" << set[0].get<DWORD>("port") << XEND;
        }
      }
    }
  }

  if (!listen())
  {
    XERR << "[" << getServerName() << "],监听 Zone 失败" << XEND;
    return false;
  }

  {
    const xLuaData& data = getBranchConfig().getData("Redis");
    RedisManager::getMe().init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port"));
  }

  if (!ProxyUserDataThread::getMe().thread_start())
  {
    XERR << "[ProxyUserDataThread]" << "创建失败" << XEND;
    return false;
  }

  return true;
}


void ProxyServer::v_closeNp(xNetProcessor* np)
{
  if (!np) return;

  ProxyUserManager::getMe().onClose(np);
}

bool ProxyServer::v_callback()
{
  switch (getServerState())
  {
    case ServerState::create:
      {
        setServerState(ServerState::init);

        return true;
      }
      break;
    case ServerState::init:
      {
        if (!init()) stop();

        setServerState(ServerState::run);

        return true;
      }
      break;
    case ServerState::run:
      {
        return true;
      }
      break;
    case ServerState::stop:
    case ServerState::finish:
      {
        return false;
      }
      break;
    default:
      break;
  }
  return false;
}

bool ProxyServer::doCmd(xNetProcessor* np, unsigned char* buf, unsigned short len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    case Cmd::LOGIN_USER_PROTOCMD:
      {
        return doLoginUserCmd(np, buf, len);
      }
      break;
    case Cmd::SYSTEM_PROTOCMD:
      {
        using namespace Cmd;
        switch (cmd->param)
        {
          case COMMON_RELOAD_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(CommonReloadSystemCmd, message);
              commonReload(message);
              return true;
            }
            break;
          case REGIST_PROXY_SYSCMD:
            {
              ProxyUserManager::getMe().syncTaskNum();
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

void ProxyServer::v_timetick()
{
  xTime oFrameTimer;

  xServer::v_timetick();

  QWORD _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-timetick]," << _e << " 微秒" << XEND;

  oFrameTimer.elapseStart();
  process();
  _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-process]," << _e << " 微秒" << XEND;

  oFrameTimer.elapseStart();
  ProxyUserManager::getMe().timer();
  _e = oFrameTimer.uElapse();
  if (_e > 30000)
    XLOG << "[帧耗时-usertimer]," << _e << " 微秒" << XEND;
}

void ProxyServer::process()
{
  ProxyUserManager::getMe().process();
}

bool ProxyServer::parseLoginUserCmd(xNetProcessor* np, xCommand* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  using namespace Cmd;

  switch (buf->param)
  {
    case REQ_LOGIN_PARAM_USER_CMD:
      {
#ifdef _ALL_SUPER_GM
        PARSE_CMD_PROTOBUF(ReqLoginParamUserCmd, message);
        if (!message.has_accid()) return true;
        message.set_timestamp(now());
        
        std::stringstream ss;
        ss << message.accid() << "_" << message.timestamp() << "_" << "rtyuio@#$%^&";

        char sha1[SHA1_LEN + 1];
        bzero(sha1, sizeof(sha1));
        getSha1Result(sha1, ss.str().c_str(), ss.str().size());
        message.set_sha1(sha1);

        PROTOBUF(message, send, len);
        np->sendCmd(send, len);

        XLOG << "[请求验证码],accid:" << message.accid() << "sha1:" << message.sha1() << XEND;
#endif

        return true;
      }
      break;
    case REQ_LOGIN_USER_CMD:
      {
        PARSE_CMD_PROTOBUF(ReqLoginUserCmd, message);

        if (!inVerifyList(np))
        {
          return true;
        }

        std::stringstream ss;

//        if (message.has_version())
 //         ss << message.accid() << "_" << message.timestamp() << "_" << "rtyuio@#$%^&";
  //      else
        ss << message.accid() << "_" << message.timestamp() << "_" << CommonConfig::m_strLoginSign;

        if (!checkSha1(message.sha1().c_str(), ss.str().c_str(), ss.str().size()))
        {
          XERR << "[请求登录],accid:" << message.accid() << "zoneid:" << message.zoneid() << "sha1:" << message.sha1() <<"phone" << message.phone() << "sha1验证错误" << XEND;

          Cmd::RegErrUserCmd errcmd;
          errcmd.set_ret(REG_ERR_PASSWD_INVALID);
          PROTOBUF(errcmd, errsend, errlen);
          np->sendCmd(errsend, errlen);

          return true;
        }

        DWORD curTime = now();
        if (message.timestamp() + MAX_GATE_WAIT_TIME < curTime)
        {
          XERR << "[请求登录]" << message.accid() << "验证超时,timestamp:" << message.timestamp() << "cur:" << curTime << XEND;

          Cmd::RegErrUserCmd errcmd;
          errcmd.set_ret(REG_ERR_OVERTIME);
          PROTOBUF(errcmd, errsend, errlen);
          np->sendCmd(errsend, errlen);

          return true;
        }

        XLOG << "[请求登录],accid:" << message.accid() << "zoneid:" << message.zoneid() << "sha1:" << message.sha1() << "phone" << message.phone() <<  "domain:" << message.domain() << "ip:" << message.ip() << "language:" << message.language() << XEND;

        removeVerifyList(np);

        ProxyUser *pUser = NEW ProxyUser(message.accid(), message.zoneid(), np);

        ProxyUserManager::getMe().add(pUser);

        pUser->m_strServerVersion = message.version();
        pUser->m_dwLanguage = message.language();

        if (!pUser->checkRegion()) return true;

        pUser->m_dwHeartBeat = curTime;
        pUser->m_strDomain = message.domain();
        pUser->m_strTaskIP = message.ip();
        pUser->m_strDevice = message.device();
        pUser->m_strPhone = message.phone();

        // check safe device
        std::stringstream ss1;

        //        if (message.has_version())
        //         ss << message.accid() << "_" << message.timestamp() << "_" << "rtyuio@#$%^&";
        //      else
        ss1 << message.timestamp() << "_" << "1" << "_" << CommonConfig::m_strSafeDeviceSign;
        if (!checkSha1(message.safe_device().c_str(), ss1.str().c_str(), ss1.str().size()))
        {
          XLOG << "[登录-不是合法设备],accid:" << message.accid() << "zoneid:" << message.zoneid() << "safedevice:" << message.safe_device() << "sha1验证错误" << XEND;
          pUser->m_bSafeDevice = false;
        }
        else
        {
          pUser->m_bSafeDevice = true;
        }

        pUser->setRealAuthorize(message.timestamp(), message.authorize());

        ProxyUserDataThread::getMe().add(pUser->createProxyUserData(ProxyUserDataAction_LOGIN));

        return true;
      }
      break;
    case HEART_BEAT_USER_CMD:
      {
        np->sendCmd(buf, len);

        return true;
      }
      break;
    default:
      break;
  }

  return true;
}

bool ProxyServer::doLoginUserCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return true;

  static DWORD dwXCommandSize = sizeof(xCommand);
  static DWORD dwNoncelenSize = sizeof(WORD);

  if (len < dwXCommandSize + dwNoncelenSize) return false;

  xCommand *cmd = (xCommand *)buf;

  WORD noncelen = *((WORD *)(cmd->probuf));
  if (len >= noncelen + dwXCommandSize + dwNoncelenSize)
  {
    /*
    BYTE* nonce = (BYTE *)&(cmd->probuf[dwXCommandSize]);
    Nonce mess;
    if (!mess.ParseFromArray(nonce, noncelen))
      return false;

    if (!checkNonce((DWORD)mess.timestamp(), (DWORD)mess.index(), mess.sign()))
    {
      return false;
    }
    */

    BUFFER_CMD(send, xCommand);
    WORD protobuflen = len - noncelen - dwXCommandSize - dwNoncelenSize;
    send->cmd = cmd->cmd;
    send->param = cmd->param;
    if (protobuflen)
    {
      bcopy((BYTE *)(&(cmd->probuf[noncelen + dwNoncelenSize])), send->probuf, protobuflen);
    }

    return parseLoginUserCmd(np, send, protobuflen + dwXCommandSize);
  }

  return false;
}

#include <arpa/inet.h>
#include "ProxyUser.h"
#include "ProxyServer.h"
#include "ProxyUserManager.h"
#include "xNetProcessor.h"
#include "LoginUserCmd.pb.h"
#include "xCommand.h"
#include "RedisManager.h"
#include "ClientCmd.h"
#include "RecordCmd.pb.h"
#include "CommonConfig.h"
#include "xSha1.h"
#include "ZoneList.h"
#include "GCharManager.h"
#include "MiscConfig.h"
#include "Authorize.pb.h"
#include "MsgManager.h"
#include "ProxyUserDataThread.h"

extern "C"
{
#include "xlib/md5/md5.h"
}

ReconnectController::ReconnectController()
{
}

ReconnectController::~ReconnectController()
{
}

bool ReconnectController::need()
{
  if (!m_blNeed) return false;

  DWORD cur = now();

  if (m_dwNextTime < cur)
  {
    ++m_dwCount;

    DWORD add = 0;
    switch (m_dwCount)
    {
      case 1:
        {
          add = 3;
        }
        break;
      case 2:
        {
          add = 5;
        }
        break;
      case 3:
        {
          add = 15;
        }
        break;
      case 4:
        {
          add = 30;
        }
        break;
      default:
        {
          add = 60;
        }
        break;
    }
    m_dwNextTime = cur + add;
    return true;
  }

  return false;
}

void ReconnectController::set()
{
  m_blNeed = true;

  if (!m_dwNextTime)
    m_dwNextTime = now();
//  m_dwCount = 0;
}

ProxyUser::ProxyUser(QWORD aid, DWORD regionid, xNetProcessor* client_task):m_oGateSpeedStat(aid, "gatetask"), m_oClientSpeedStat(aid, "client")
{
  accid = aid;
  m_dwRegionID = regionid;
  m_pClientTask = client_task;
  if (m_pClientTask)
  {
    m_pClientTask->setComp(true);
    m_pClientTask->setTick(true);
    m_pClientTask->set_id(accid);
  }
  setState(ProxyUser_State::create);
}

ProxyUser::~ProxyUser()
{
  if (m_pClientTask)
  {
    thisServer->addCloseList(m_pClientTask, TerminateMethod::terminate_active, "ProxyUser析构");
    m_pClientTask = nullptr;
  }
  if (m_pGateServerTask)
  {
    thisServer->addCloseList(m_pGateServerTask, TerminateMethod::terminate_active, "ProxyUser析构");
    m_pGateServerTask = nullptr;
  }

  XLOG << "[ProxyUser]" << tempid << id << name << "析构" << XEND;
}

bool ProxyUser::forwardClientTask(void* cmd, unsigned short len)
{
  if (ProxyUser_State::quit == getState())
  {
    XLOG << "[forwardClientTask]" << accid << id << name << "玩家已退出" << XEND;
    return false;
  }
#ifdef _LX_DEBUG
  //xCommand *c = (xCommand *)cmd;
  //XLOG("[forwardClientServer],%llu,%llu,%s,发送,%u,%u", accid, id, name, c->cmd, c->param);
#endif
  if (m_pClientTask)
  {
    return m_pClientTask->sendCmd(cmd, len);
  }
  else
  {
#ifdef _LX_DEBUG
    XLOG << "[forwardClientTask]" << accid << id << name << "发送失败" << XEND;
#endif
  }
  return false;
}

bool ProxyUser::forwardGateServer(void* cmd, unsigned short len)
{
  if (ProxyUser_State::quit == getState())
  {
    XLOG << "[forwardGateServer]" << accid << id << name << "玩家已退出" << XEND;
    return false;
  }
#ifdef _LX_DEBUG
  //xCommand *c = (xCommand *)cmd;
  //XLOG("[forwardGateServer],%llu,%llu,%s,发送,%u,%u", accid, id, name, c->cmd, c->param);
#endif

  if (m_pGateServerTask)
  {
    return m_pGateServerTask->sendCmd(cmd, len);
  }
  else
  {
#ifdef _LX_DEBUG
    XLOG << "[forwardGateServer]" << accid << id << name << "发送失败" << XEND;
#endif
  }
  return false;
}

/*
 * byte cmd
 * byte param
 * word noncelen
 * data[noncelen]
 * pbdata[pblen]
 */

bool ProxyUser::doClientCmd(xCommand *buf, unsigned short len)
{
  if (!buf || !len) return true;

  static DWORD dwXCommandSize = sizeof(xCommand);
  static DWORD dwNoncelenSize = sizeof(WORD);

  if (len < dwXCommandSize + dwNoncelenSize) return false;

  addCmdCount(buf);

  if (isCmdOverLimit(buf) == true) return false;

  WORD noncelen = *((WORD *)(buf->probuf));
  if (len >= noncelen + dwXCommandSize + dwNoncelenSize)
  {
    BYTE* nonce = (BYTE *)&(buf->probuf[dwXCommandSize]);
    Nonce mess;
    if (!mess.ParseFromArray(nonce, noncelen))
      return false;

    if (!checkNonce((DWORD)mess.timestamp(), (DWORD)mess.index(), mess.sign()))
    {
      return false;
    }

    BUFFER_CMD(send, xCommand);
    WORD protobuflen = len - noncelen - dwXCommandSize - dwNoncelenSize;
    send->cmd = buf->cmd;
    send->param = buf->param;
    if (protobuflen)
    {
      bcopy((BYTE *)(&(buf->probuf[noncelen + dwNoncelenSize])), send->probuf, protobuflen);
    }

    return parseClientCmd(send, protobuflen + dwXCommandSize);
  }
  return false;
}

bool ProxyUser::parseClientCmd(xCommand *buf, unsigned short len)
{
  if (!buf || !len) return true;

  DWORD dwCurTime = now();
  m_dwHeartBeat = dwCurTime;

  switch (buf->cmd)
  {
    /*
    case SCENE_USER_ITEM_PROTOCMD:
      {
        switch (buf->param)
        {
          case ITEMPARAM_SELLITEM:
            {
              PARSE_CMD_PROTOBUF(SellItem, rev);
              std::set<string> tmp;
              for (int i = 0; i < rev.items_size(); ++i)
              {
                const SItem& rItem = rev.items(i);
                if (tmp.find(rItem.guid()) != tmp.end())
                  return false;
                tmp.insert(rItem.guid());
              }
            }
            break;
          default:
            break;
        }
      }
      break;
      */
    case Cmd::LOGIN_USER_PROTOCMD:
      {
        switch (buf->param)
        {
          // 取消删号
          case CANCEL_DELETE_CHAR_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::CancelDeleteCharUserCmd, rev);

              if (rev.id() != m_oAccBaseData.m_qwDeleteCharID)
                return true;

              ProxyUserDataThread::getMe().add(createProxyUserData(ProxyUserDataAction_CANCEL_DELETE));

              return true;
            }
            break;
            // 屏蔽客户端来的消息
          case KICK_PARAM_USER_CMD:
            {
              return true;
            }
            break;
            // 系统时间
          case SERVERTIME_USER_CMD:
            {
              if (!m_pGateServerTask)
              {
                ServerTimeUserCmd message;
                message.set_time(xTime::getCurMSec());
                PROTOBUF(message, send, len);
                forwardClientTask(send, len);
                return true;
              }
            }
            break;
            // 创建角色
          case CREATE_CHAR_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::CreateCharUserCmd, rev);

              rev.set_accid(this->accid);
              rev.set_version(m_strServerVersion);

              DWORD idx = rev.sequence();

              if (idx == 0 || idx > MAX_CHAR_NUM) return true;

              if (m_oSnapShotDatas[idx - 1].id != 0 || m_oSnapShotDatas[idx - 1].m_dwOpen == 0)
              {
                Cmd::RegErrUserCmd errCmd;
                errCmd.set_ret(REG_ERR_SEQUENCE);
                PROTOBUF(errCmd, errSend, errLen);
                forwardClientTask(errSend, errLen);
                return true;
              }

              xTime frameTimer;
              DWORD zoneid = ZoneList::getMe().getANewZoneID(m_dwRegionID, m_dwLanguage);
              XLOG << "[创建角色]" << accid << "请求创建,zoneid:" << zoneid << "耗时:" << frameTimer.uElapse() << "微秒" << XEND;

              if (this->connectGateServer(zoneid) == false)
              {
                XERR << "[创建角色]" << accid << "连接服务器失败:" << zoneid << XEND;

                return true;
              }

              XLOG << "[创建角色]" << accid << "请求创建,zoneid:" << zoneid << XEND;

              PROTOBUF(rev, send, len);
              forwardGateServer(send, len);

              return true;
            }
            break;
            // 选择角色
          case SELECT_ROLE_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::SelectRoleUserCmd, rev);

              selectChar(rev.id(), rev.deviceid());

              return true;
            }
            break;
            // 心跳
          case HEART_BEAT_USER_CMD:
            {
              if (!m_pGateServerTask)
              {
                forwardClientTask(buf, len);
                return true;
              }
            }
            break;
            // 删除角色
          case DELETE_CHAR_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::DeleteCharUserCmd, rev);

              if (m_blConfirmed == false)
                return true;

              ProxyUserData *pData = createProxyUserData(ProxyUserDataAction_SET_DELETE);
              pData->m_qwDeleteCharID = rev.id();
              ProxyUserDataThread::getMe().add(pData);

              return true;
            }
            break;
          case CLIENT_FRAME_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::ClientFrameUserCmd, rev);

              DWORD dwFrame = rev.frame();

              if (0 == m_dwClientFrame)
              {
                m_dwClientFrame = dwFrame;
                m_dwClientFrameTimestamp = dwCurTime;

                return true;
              }
              if (dwFrame < m_dwClientFrame)
              {
                XLOG << "[客户端帧数]" << accid << "帧数变小:" << m_dwClientFrame << dwFrame << XEND;
                return true;
              }

              if (m_dwClientFrameTimestamp < dwCurTime)
              {
                DWORD dwFrameCount = dwFrame - m_dwClientFrame;
                DWORD dwLast = dwCurTime - m_dwClientFrameTimestamp;

                m_dwClientFrame = dwFrame;
                m_dwClientFrameTimestamp = dwCurTime;

                DWORD dwFramePer = dwFrameCount / (dwLast + 1);
                if (dwFramePer > 30)
                {
                  XLOG << "[客户端帧数]" << accid << "帧数异常:" << dwFramePer << dwFrameCount << dwLast << XEND;
                }
                else
                {
                  XDBG << "[客户端帧数]" << accid << "帧数:" << dwFramePer << dwFrameCount << dwLast << XEND;
                }
              }

              return true;
            }
            break;
            //安全密码验证
          case CONFIRM_AUTHORIZE_USER_CMD:
            {
              PARSE_CMD_PROTOBUF(Cmd::ConfirmAuthorizeUserCmd, rev);
              confirmAuthorize(rev.password());
              return true;
            }
            break;
          case REAL_AUTHORIZE_USER_CMD:
          {
            PARSE_CMD_PROTOBUF(Cmd::RealAuthorizeUserCmd, rev);
            setRealAuthorize(0, rev.authoriz_state());
            syncRealAuthorize2Gate();
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

  if (m_pGateServerTask)
  {
    m_oClientSpeedStat.add(len);
    return forwardGateServer(buf, len);
  }
  else
  {
  }
  return false;
}

// 处理游戏服务器返回的消息
bool ProxyUser::doGateServerCmd(xCommand *cmd, unsigned short len)
{
  if (!cmd || !len) return true;

  switch (cmd->cmd)
  {
    case CLIENT_CMD:
      {
        switch (cmd->param)
        {
          // 消息集合 直接转发
          case CMDSET_USER_CMD:
            {
              /*

              Cmd::CmdSetUserCmd *rev = (Cmd::CmdSetUserCmd *)cmd;

              DWORD place = 0;

              for (DWORD i=0; i<rev->num; ++i)
              {
                WORD size = *(WORD *)(rev->data + place);
                place += sizeof(WORD);

                xCommand *buf = (xCommand *)(rev->data + place);

                forwardClientTask(buf, size);
                place += size;
              }

              return true;
              */
            }
            break;
          case SELECT_CHAR_CLIENT_USER_CMD:
            {
              SelectCharClientUserCmd *rev = (SelectCharClientUserCmd *)cmd;

              autoSelectChar(rev->charid);

              return true;
            }
            break;
          case REFRESH_SNAPSHOT_CLIENT_USER_CMD:
            {
              sendSnapShotToMe();

              return true;
            }
            break;
          case RECONNECT_CLIENT_USER_CMD:
            {
              ReconnectClientUserCmd *rev = (ReconnectClientUserCmd *)cmd;
              if (rev->refresh)
              {
                m_oReconnectController.reset();
              }
              m_oReconnectController.set();
              thisServer->addCloseList(m_pGateServerTask, TerminateMethod::terminate_active, "RECONNECT_CLIENT_USER_CMD");
              return true;
            }
            break;
          case DISCONNECT_CLIENT_USER_CMD:
            {
              thisServer->addCloseList(m_pGateServerTask, TerminateMethod::terminate_active, "DISCONNECT_CLIENT_USER_CMD");
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
  }
  m_oGateSpeedStat.add(len);
  return forwardClientTask(cmd, len);
}

void ProxyUser::setState(ProxyUser_State state)
{
  m_oProxyUserState = state;
  switch (m_oProxyUserState)
  {
    case ProxyUser_State::create:
      XLOG << "[ProxyUser_State]" << id << name << this << "创建" << XEND;
      break;
    case ProxyUser_State::running:
      XLOG << "[ProxyUser_State]" << id << name << this << "运行" << XEND;
      break;
    case ProxyUser_State::quit:
      XLOG << "[ProxyUser_State]" << id << name << this << "退出" << XEND;
      break;
    default:
      break;
  }
}

bool ProxyUser::connectCharServer(DWORD dwRegionID, QWORD charid, bool checkZ)
{
  DWORD zoneid = 0;
  GCharReader gchar(dwRegionID, charid);
  if (gchar.get())
  {
    if (checkZ)
    {
      zoneid = checkZone(gchar.getZoneID(), charid, gchar.getOriginalZoneID());
    }
    else
    {
      zoneid = gchar.getZoneID();
    }
  }

  if (!zoneid)
  {
    for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
    {
      if (charid == m_oSnapShotDatas[i].id)
      {
        if (checkZ)
        {
          zoneid = checkZone(m_oSnapShotDatas[i].zoneid, m_oSnapShotDatas[i].id, m_oSnapShotDatas[i].originalzoneid);
        }
        else
        {
          zoneid = m_oSnapShotDatas[i].zoneid;
        }

        break;
      }
    }
  }

  if (!zoneid) return false;

  return connectGateServer(zoneid);
}

bool ProxyUser::connectGateServer(DWORD zoneid)
{
  if (m_pGateServerTask)
  {
    if (m_dwZoneID != zoneid)
    {
      xNetProcessor *task = m_pGateServerTask;
      m_pGateServerTask = nullptr;
      thisServer->addCloseList(task, TerminateMethod::terminate_active, "connectGateServer");
    }
    else
    {
      return true;
    }
  }

  if (!checkRegion()) return false;

  string key = RedisManager::getMe().getKeyByParam(m_dwRegionID, EREDISKEYTYPE_GATE_INFO, zoneid);

  xLuaData oData;
  if (RedisManager::getMe().getHashAll(key, oData))
  {
    if (oData.m_table.empty() == true)
      return false;

    std::multimap<DWORD, std::string> list;
    for (auto &it : oData.m_table)
    {
      list.insert(std::make_pair(it.second.getInt(), it.first));
    }

    for (auto &it : list)
    {
      XDBG << "[选择网关]" << "网关:" << it.second << ":" << it.first << XEND;
      std::vector<std::string> vec;
      stringTok(it.second, ":", vec);
      if (vec.size() != 2) continue;

      DWORD ip = atoi(vec[0].c_str());
      DWORD port = atoi(vec[1].c_str());

      if (!ip)
      {
        XERR << "[ProxyUser], 网关配置错误, zoneid=" << zoneid << XEND;
        continue;
      }
      m_pGateServerTask = thisServer->newClient();

      if (!m_pGateServerTask->connect(inet_ntoa(*((in_addr *)&ip)), port))
      {
        if (!m_oReconnectController.isNeed())
        {
          notifyError(REG_ERR_SERVER_STOP);
        }
        SAFE_DELETE(m_pGateServerTask);
      }
      else
      {
        m_dwZoneID = zoneid;
        m_oReconnectController.reset();
        thisServer->getTaskThreadPool().add(m_pGateServerTask);
        m_pGateServerTask->id = accid;

        XLOG << "[ProxyUser],accid:" << accid << "zoneid:" << zoneid << "connect," << inet_ntoa(*((in_addr *)&ip)) << "(" << ip << "):" << port << ",ok" << XEND;
        return true;
      }
    }
  }
  XLOG << "[ProxyUser],accid:" << accid << "connect zone " << zoneid << " failed" << XEND;
  notifyError(REG_ERR_SERVER_STOP);
  return false;
}

ProxyUserData* ProxyUser::createProxyUserData(ProxyUserDataAction act)
{
  ProxyUserData *pData = NEW ProxyUserData(accid, m_dwRegionID, act, this);
  return pData;
}

void ProxyUser::sendSnapShotToMe()
{
  ProxyUserDataThread::getMe().add(createProxyUserData(ProxyUserDataAction_SEND));
}

bool ProxyUser::get(ProxyUserData *pData)
{
  if (!pData) return false;
  if (pData->m_blRet == false) return false;

  bcopy(&(pData->m_oAccBaseData), &m_oAccBaseData, sizeof(m_oAccBaseData));
  bcopy(pData->m_oSnapShotDatas, m_oSnapShotDatas, sizeof(m_oSnapShotDatas));
  return true;
}

void ProxyUser::realSendSnapShotToMe(ProxyUserData *pData)
{
  if (!pData) return;

  if (!pData->m_blRet)
  {
    notifyError(REG_ERR_SERVER_STOP);
    XERR << "[快照]" << this->accid << "查询失败" << this->m_dwRegionID << XEND;
    return;
  }
  // bzero(&m_oAccBaseData, sizeof(m_oAccBaseData));
  // bzero(m_oSnapShotDatas, sizeof(m_oSnapShotDatas));

  get(pData);

  Cmd::SnapShotUserCmd mess;
  DWORD count = 0;
  bool deletechar = m_dwNotifyDelChar == 0 ? false : true;
  for (DWORD i = 0; i < MAX_CHAR_NUM; ++i)
  {
    const SnapShotData& rData = m_oSnapShotDatas[i];
    Cmd::SnapShotDataPB *data = mess.add_data();
    if (data)
    {
      data->set_sequence(i + 1);
      data->set_isopen(rData.m_dwOpen);
      if (rData.id)
      {
        data->set_id(rData.id);
        data->set_name(rData.name);
        data->set_baselv(rData.level);
        data->set_hair(rData.hair);
        data->set_haircolor(rData.haircolor);
        data->set_lefthand(rData.lefthand);
        data->set_righthand(rData.righthand);
        data->set_body(rData.body);
        data->set_head(rData.head);
        data->set_back(rData.back);
        data->set_face(rData.face);
        data->set_tail(rData.tail);
        data->set_mount(rData.mount);
        data->set_eye(rData.eye);
        data->set_partnerid(rData.partnerid);
        data->set_portrait(rData.portrait);
        data->set_mouth(rData.mouth);
        data->set_clothcolor(rData.clothcolor);

        data->set_gender(static_cast<EGender>(rData.male));
        data->set_profession(static_cast<EProfession>(rData.type));

        if (m_oAccBaseData.m_qwDeleteCharID && m_oAccBaseData.m_qwDeleteCharID == rData.id)
        {
          data->set_deletetime(m_oAccBaseData.m_dwDeleteTime);
        }

        ++count;

        if (m_dwNotifyDelChar == rData.id)
          deletechar = false;
      }
    }
  }
  if (deletechar) m_dwNotifyDelChar = 0;
  mess.set_deletechar(deletechar);
  mess.set_lastselect(m_oAccBaseData.m_qwLastSelect);
  mess.set_maincharid(m_oAccBaseData.m_qwMainCharId);
  DWORD deletecdtime = m_oAccBaseData.m_dwLastDeleteTime + CommonConfig::m_dwDelCharCD;
  if (deletecdtime > now())
    mess.set_deletecdtime(deletecdtime);
  else
    mess.set_deletecdtime(0);
  PROTOBUF(mess, send, len);
  this->forwardClientTask(send, len);
  XLOG << "[快照]" << this->accid << "返回" << count << "个角色" << XEND;
}

void ProxyUser::selectChar(QWORD charid, const string& deviceid)
{
  m_qwCharID = charid;

  if (m_oAccBaseData.m_qwCharID && charid != m_oAccBaseData.m_qwCharID)
  {
    DWORD dwNow = xTime::getCurSec();
    if (dwNow >= m_dwKickCharTick)
    {
      kickChar(m_oAccBaseData.m_qwCharID);
      m_dwKickCharTick = dwNow + 3;
      return;
    }
  }
  if (m_oAccBaseData.m_qwDeleteCharID == charid)
  {
    return;
  }

  if (!checkNoLoginTime())
    return;

  Cmd::SelectRoleUserCmd message;
  message.set_accid(this->accid);
  message.set_deviceid(deviceid);
  message.set_version(m_strServerVersion);
  message.mutable_extradata()->set_phone(m_strPhone);
  message.mutable_extradata()->set_safedevice(m_bSafeDevice);
  message.set_realauthorized(m_bAuthorize);
  for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
  {
    if (charid == m_oSnapShotDatas[i].id)
    {
      if (m_oSnapShotDatas[i].m_dwNoLoginTime && m_oSnapShotDatas[i].m_dwNoLoginTime > now())
      {
        RegErrUserCmd errMessage;
        errMessage.set_accid(this->accid);
        errMessage.set_ret(REG_ERR_FORBID_REG);
        errMessage.add_args(m_oSnapShotDatas[i].m_dwNoLoginTime);
        PROTOBUF(errMessage, errSend, errLen);
        forwardClientTask(errSend, errLen);

        return;
      }

      // select role close gate first to prevent gateuser status error(GateUser::login and GateUser::select_role)
      if (m_pGateServerTask != nullptr)
      {
        xNetProcessor *task = m_pGateServerTask;
        m_pGateServerTask = nullptr;
        thisServer->addCloseList(task, TerminateMethod::terminate_active, "connectGateServer");
      }
      if (this->connectCharServer(m_dwRegionID, charid) == false)
      {
        XERR << "[选择角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << "连接服务器失败:" << m_oSnapShotDatas[i].zoneid << XEND;

        return;
      }

      message.set_id(charid);
      message.set_name(m_oSnapShotDatas[i].name);
      message.set_ignorepwd(m_blConfirmed);
      message.set_password(m_oAccBaseData.m_strPassword);
      message.set_resettime(m_oAccBaseData.m_dwPwdResetTime);
      message.set_language(m_dwLanguage);
      message.set_maxbaselv(getMaxBaseLv());

      XLOG << "[选择角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << XEND;

      PROTOBUF(message, send, len);
      forwardGateServer(send, len);
      return;
    }
  }
  XERR << "[选择角色]" << accid << "没有找到该角色" << charid << XEND;

  Cmd::LoginResultUserCmd cmd;
  cmd.set_ret(REG_ERR_PROFESSION_NOOPEN);
  PROTOBUF(cmd, send, len);
  forwardClientTask(send, len);
}

void ProxyUser::realDeleteChar()
{
  if (m_oAccBaseData.m_qwCharID && m_oAccBaseData.m_qwDeleteCharID == m_oAccBaseData.m_qwCharID)
  {
    kickChar(m_oAccBaseData.m_qwCharID);
  }

  DWORD cur = now();
  if (m_oAccBaseData.m_dwDeleteTime && m_oAccBaseData.m_dwDeleteTime < cur)
  {
    for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
    {
      if (m_oAccBaseData.m_qwDeleteCharID == m_oSnapShotDatas[i].id)
      {
        if (this->connectGateServer(m_oSnapShotDatas[i].zoneid) == false)
        {
          XERR << "[删除角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << "连接服务器失败:" << m_oSnapShotDatas[i].zoneid << XEND;
          DWORD dwZoneID = checkZone(m_oSnapShotDatas[i].zoneid, m_oSnapShotDatas[i].id, m_oSnapShotDatas[i].originalzoneid);
          if (dwZoneID != m_oSnapShotDatas[i].zoneid)
            return;

          m_oSnapShotDatas[i].zoneid = dwZoneID;
          if (this->connectGateServer(m_oSnapShotDatas[i].zoneid) == false)
          {
            XERR << "[删除角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << "连接服务器失败:" << m_oSnapShotDatas[i].zoneid << XEND;
            return;
          }
        }

        Cmd::DeleteCharUserCmd message;
        message.set_accid(accid);
        message.set_id(m_oAccBaseData.m_qwDeleteCharID);
        message.set_version(m_strServerVersion);
        PROTOBUF(message, send, len);
        forwardGateServer(send, len);
        XLOG << "[删除角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << XEND;

        m_oAccBaseData.m_dwDeleteTime = 0;

        m_dwNotifyDelChar = m_oAccBaseData.m_qwDeleteCharID;
        return;
      }
    }
    m_oAccBaseData.m_dwDeleteTime = 0;
    XERR << "[删除角色]" << accid << "没有找到该角色" << m_oAccBaseData.m_qwDeleteCharID << XEND;
  }
}

void ProxyUser::autoSelectChar(QWORD charid)
{
  selectChar(charid, "");
}

void ProxyUser::timer(DWORD cur)
{
  if (m_dwHeartBeat && m_dwHeartBeat + 60 < cur)
  {
    if (m_pClientTask)
    {
      XLOG << "[心跳-断开]" << accid << "上次心跳时间:" << m_dwHeartBeat << "当前时间:" << cur << XEND;
      thisServer->addCloseList(m_pClientTask, TerminateMethod::terminate_socket_error, "心跳超时");
    }
    return;
  }
  if (m_oAccBaseData.m_dwDeleteTime && m_oAccBaseData.m_dwDeleteTime < cur)
  {
    ProxyUserDataThread::getMe().add(createProxyUserData(ProxyUserDataAction_DELETE_CHAR));
  }
  if (m_dwKickCharTick > cur || m_oReconnectController.need())
  {
    if (m_qwCharID)
    {
      autoSelectChar(m_qwCharID);
    }
    else
    {
      m_oReconnectController.reset();
    }
    // 一定放在autoSelectChar之后,否则会重复调用kickChar
    m_dwKickCharTick = cur;
  }
  if(cur % 10 == 0)
    checkResetTime();
}

bool ProxyUser::checkRegion()
{
  RegionStatusInfo info;
  if (ZoneList::getMe().getRegionStatus(m_dwRegionID, info))
  {
    if ((info.m_blOpen == false) || (info.m_blMaintain == true))
    {
      DWORD whiteListFlag = getWhiteListFlag();
      if (info.m_blOpen == false)
      {
        if (whiteListFlag < 1)
        {
          notifyError(REG_ERR_SERVER_UNOPENED);

          return false;
        }
      }
      if (info.m_blMaintain == true)
      {
        if (whiteListFlag < 2)
        {
//          notifyError(REG_ERR_SERVER_STOP);
          maintainNtf(info);
          return false;
        }
      }
    }
#ifndef _ALL_SUPER_GM
    if (!info.m_strServerVersion.empty() && (info.m_strServerVersion != m_strServerVersion))
    {
      Cmd::RegErrUserCmd errcmd;

      errcmd.set_ret(REG_ERR_NEW_VERSION);
      PROTOBUF(errcmd, errsend, errlen);
      forwardClientTask(errsend, errlen);

      XLOG << "[请求登录]" << accid << "版本不正确,客户端版本:" << m_strServerVersion << "服务器版本:" << info.m_strServerVersion << XEND;

      return false;
    }
#endif
  }
  else
  {
    notifyError(REG_ERR_SERVER_STOP);

    return false;
  }
  return true;
}

DWORD ProxyUser::checkZone(DWORD zoneid, QWORD charid, DWORD originalzoneid)
{
  ZoneDBInfo info;
  if (ZoneList::getMe().getZoneDBInfo(zoneid, info))
  {
    switch (info.m_dwStatus)
    {
      case ZONE_STATUS_CLOSE:
        {
          DWORD dwNewZoneID = 0;
          if (originalzoneid)
            dwNewZoneID = originalzoneid;
          if (info.m_dwMergeID)
            dwNewZoneID = info.m_dwMergeID;

          if (!dwNewZoneID) return zoneid;

          const char *dbname = ZoneList::getMe().getRegionDBName(m_dwRegionID);
          xField *field = thisServer->getDBConnPool().getField(dbname, "charbase");   // 登录关闭的服务器 写
          if (field)
          {
            xRecord record(field);
            record.put("zoneid", dwNewZoneID);
            record.put("destzoneid", 0);
            record.put("originalzoneid", 0);
            char where[32] = {0};
            snprintf(where, sizeof(where), "charid=%llu", charid);
            QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
            if (ret == QWORD_MAX)
            {
              XERR << "[切换线-失败]" << charid << "原线:" << zoneid << "目标:" << dwNewZoneID << XEND;
              return zoneid;
            }
            return dwNewZoneID;
          }
        }
        break;
      default:
        break;
    }
  }
  return zoneid;
}

void ProxyUser::notifyError(Cmd::RegErrRet ret)
{
    Cmd::RegErrUserCmd message;
    message.set_ret(ret);
    PROTOBUF(message, send, len);
    if (forwardClientTask(send, len))
    {
      XLOG << "[错误提示]" << accid << XEND;
    }
}

void ProxyUser::maintainNtf(const RegionStatusInfo& info)
{
  Cmd::MaintainUserCmd message;
  message.set_content(info.getContent(m_dwLanguage));
  message.set_tip(info.getTip(m_dwLanguage));
  message.set_picture(info.m_strPicture);
  PROTOBUF(message, send, len);
  forwardClientTask(send, len);
}

DWORD ProxyUser::getWhiteListFlag()
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "whitelist"); // 维护时 只读
  if (field)
  {
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "accid=%llu", accid);

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX!=ret && ret)
    {
      return set[0].get<DWORD>("flag");
    }
  }
  return 0;
}

void ProxyUser::kickChar(QWORD charid)
{
  for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
  {
    if (charid == m_oSnapShotDatas[i].id)
    {
      if (connectCharServer(m_dwRegionID, charid))
      //if (connectGateServer(m_oSnapShotDatas[i].zoneid))
      {
        if (m_pGateServerTask)
        {
          Cmd::KickParamUserCmd message;
          message.set_charid(charid);
          message.set_accid(accid);
          PROTOBUF(message, send, len);
          forwardGateServer(send, len);
        }
      }

      XLOG << "[踢出角色]" << accid << m_oSnapShotDatas[i].id << m_oSnapShotDatas[i].name << XEND;
      return;
    }
  }
}

void ProxyUser::addCmdCount(xCommand *cmd)
{
  if (!cmd) return;

  if (m_dwCmdCountTime + CommonConfig::m_dwClientCmdSec < now())
  {
    clearCmdCount();
  }

  if (CommonConfig::isIngoreCmd(cmd->cmd, cmd->param) == false)
  {
    ++m_dwCmdCount;
    ++m_oCmdList[*((WORD*)cmd)];
  }

  if (m_dwCmdCount > CommonConfig::m_dwClientCmdMax)
  {
    printCmdCount();
    if (m_pClientTask)
    {
      thisServer->addCloseList(m_pClientTask, TerminateMethod::terminate_active, "消息发送太快");
    }
    XLOG << "[消息发送太快]" << accid << "数量:" << m_dwCmdCount << XEND;

    clearCmdCount();
  }
}

void ProxyUser::printCmdCount()
{
  for (auto &it : m_oCmdList)
  {
    XLOG << "[消息频率检查]" << accid << "cmd:" << (DWORD)(it.first & 0xff) << "param:" << (DWORD)(it.first >> 8) << "次数:" << it.second << XEND;
  }
}

void ProxyUser::clearCmdCount()
{
  m_dwCmdCount = 0;
  m_oCmdList.clear();
  m_dwCmdCountTime = now();
}

// 检测消息发送频率是否超出限制
bool ProxyUser::isCmdOverLimit(xCommand *cmd)
{
  if (!cmd) return true;

  if (m_dwCmdLimitTime + CommonConfig::m_dwClientCmdLimitSec < now())
  {
    clearCmdLimit();
  }

  auto cnt = ++m_oCmdLimitList[*((WORD *)cmd)];

  if (cnt > CommonConfig::getCmdLimit(cmd->cmd, cmd->param))
  {
    XLOG << "[消息发送频率超过限制]" << accid << "cmd:" << cmd->cmd << "param:" << cmd->param << "数量:" << m_oCmdLimitList[*((WORD *)cmd)] << XEND;
    return true;
  }

  return false;
}

// 清空消息频率统计
void ProxyUser::clearCmdLimit()
{
  m_oCmdLimitList.clear();
  m_dwCmdLimitTime = now();
}

bool ProxyUser::checkNonce(DWORD timestamp, DWORD index, const std::string &sign)
{
  if (timestamp < m_dwNonceTime)
  {
    XLOG << "[Nonce]" << accid << id << name << "时间戳变小失败" << timestamp << m_dwNonceTime << XEND;
    return false;
  }

  std::stringstream ss;
  ss << timestamp << "_" << index << "_" << "!^ro&";
  if (!checkSha1(sign.c_str(), ss.str().c_str(), ss.str().size()))
  {
    XLOG << "[Nonce]" << accid << id << name << "sha1失败" << sign.c_str() << ss.str().c_str() << XEND;
    return false;
  }

  if (m_dwNonceTime != timestamp)
  {
    m_dwNonceIndex = 0;
    m_dwNonceTime = timestamp;
  }

  if (index <= m_dwNonceIndex)
  {
    XLOG << "[Nonce]" << accid << id << name << "编号减小" << index << m_dwNonceIndex << XEND;
    return false;
  }

  return true;
}

bool ProxyUser::confirmAuthorize(const string& str)
{
  if(checkResetTime() == true)
    return true;

  string pwd(&m_oAccBaseData.m_strPassword[0],&m_oAccBaseData.m_strPassword[strlen(m_oAccBaseData.m_strPassword)]);
  if(pwd.empty() == true)
  {
    syncAuthorizeToUser(true,false);
    return true;
  }

  DWORD cursec = xTime::getCurSec();

  if(m_dwValidTime >= cursec)
  {
    Cmd::NotifyAuthorizeUserCmd cmd;
    PROTOBUF(cmd, send, len);
    forwardClientTask(send, len);
    return false;
  }

  char sign[1024];
  bzero(sign, sizeof(sign));
  upyun_md5(str.c_str(), str.size(), sign);

  if(strncmp(sign,pwd.c_str(),1024) == 0)
  {
    m_blConfirmed = true;
    syncAuthorizeToUser(true,true);
    syncAuthorizeToGate();
    XLOG << "[安全密码-验证] 成功 " << accid << XEND;
    return true;
  }
  else
  {
    m_dwFailTimes++;
    DWORD maxfail = CommonConfig::m_dwPwdFailTimes;
    if(m_dwFailTimes >= maxfail)
    {
      DWORD interval = CommonConfig::m_dwPwdIntervalTime;
      m_dwValidTime = interval + cursec;
      m_dwFailTimes = 0;
    }

    syncAuthorizeToUser(false,true);
    XLOG << "[安全密码-验证] 失败" << accid << "失败次数: "<< m_dwFailTimes << XEND;
    return false;
  }
}

void ProxyUser::syncAuthorizeToUser(bool ignorePwd, bool hasSet)
{
  Cmd::ConfirmAuthorizeUserCmd cmd;
  cmd.set_success(ignorePwd);
  cmd.set_resettime(m_oAccBaseData.m_dwPwdResetTime);
  cmd.set_hasset(hasSet);
  PROTOBUF(cmd, send, len);
  forwardClientTask(send, len);
}

void ProxyUser::syncAuthorizeToGate()
{
  if(m_pGateServerTask == nullptr)
    return;

  Cmd::SyncAuthorizeGateCmd cmd;
  cmd.set_ignorepwd(m_blConfirmed);
  cmd.set_password(m_oAccBaseData.m_strPassword);
  cmd.set_version(m_strServerVersion);
  cmd.set_accid(accid);
  PROTOBUF(cmd, send, len);
  forwardGateServer(send, len);
}

void ProxyUser::onLineAuthorize()
{
  string pwd(&m_oAccBaseData.m_strPassword[0],&m_oAccBaseData.m_strPassword[strlen(m_oAccBaseData.m_strPassword)]);

  if(pwd.empty() == true)
  {
    m_blConfirmed = true;
    syncAuthorizeToUser(true,false);
    syncAuthorizeToGate();
    return;
  }

  if(checkResetTime() == false)
    syncAuthorizeToUser(false,true);
}

bool ProxyUser::checkResetTime()
{
  if(m_oAccBaseData.m_dwPwdResetTime == 0)
    return false;

  string pwd(&m_oAccBaseData.m_strPassword[0],&m_oAccBaseData.m_strPassword[strlen(m_oAccBaseData.m_strPassword)]);
  DWORD cursec = xTime::getCurSec();
  if(m_oAccBaseData.m_dwPwdResetTime <= cursec && pwd.empty() == false)
  {
    m_blConfirmed = true;
    m_oAccBaseData.m_dwPwdResetTime = 0;
    memset(m_oAccBaseData.m_strPassword,0,sizeof(m_oAccBaseData.m_strPassword));
    syncAuthorizeToUser(true,false);
    syncAuthorizeToGate();
    return true;
  }

  return false;
}

bool ProxyUser::checkNoLoginTime()
{
  if (!accid || !m_dwRegionID)
    return false;
  DWORD dwNoLoginTime = m_oAccBaseData.m_dwNoLoginTime;
  DWORD curSec = now();
  if (dwNoLoginTime && curSec < dwNoLoginTime)
  {
    RegErrUserCmd errMessage;
    errMessage.set_accid(accid);
    errMessage.set_ret(REG_ERR_ACC_FORBID);
    errMessage.add_args(dwNoLoginTime);
    PROTOBUF(errMessage, errSend, errLen);
    forwardClientTask(errSend, errLen);
    XLOG << "[账号-封停]" << accid << "封停结束时间" << dwNoLoginTime << XEND;
    return false;
  }
  return true;
}

void ProxyUser::setRealAuthorize(DWORD timeStamp, string strAuthorize)
{
  std::stringstream ss1;
  ss1 << timeStamp << "_" << "1" << "_" << CommonConfig::m_strAuthorizeSign;
  if (!checkSha1(strAuthorize.c_str(), ss1.str().c_str(), ss1.str().size()))
    m_bAuthorize = false;
  else
    m_bAuthorize = true;

  //res client
  {
    RealAuthorizeUserCmd cmd;
    cmd.set_authorized(m_bAuthorize);
    PROTOBUF(cmd, send, len);
    forwardClientTask(send, len);
  }

  XLOG << "[登录-实名认证],accid:" << accid << "authorize:" << strAuthorize << m_bAuthorize << XEND;
}

void ProxyUser::syncRealAuthorize2Gate()
{
  RealAuthorizeServerCmd cmd;
  cmd.set_authorized(m_bAuthorize);
  PROTOBUF(cmd, send, len);
  forwardGateServer(send, len);
  XLOG << "[登录-实名认证] 发送给gateserver,accid:" << accid << m_bAuthorize << XEND;
}

DWORD ProxyUser::getMaxBaseLv()
{
  DWORD maxbaselv = 0;
  for (DWORD i = 0; i < MAX_CHAR_NUM; ++i)
  {
    const SnapShotData& rData = m_oSnapShotDatas[i];
    if (rData.id && rData.id != m_oAccBaseData.m_qwDeleteCharID && rData.level > maxbaselv)
      maxbaselv = rData.level;
  }
  return maxbaselv;
}

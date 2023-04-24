#include "GateUserManager.h"
#include "GateServer.h"
#include "GateUser.h"
#include "xNetProcessor.h"
#include "RegCmd.h"
#include <list>
#include "GateSuper.pb.h"
#include "ClientCmd.h"
#include "ErrorUserCmd.pb.h"

GateUserManager::GateUserManager()
{
}

GateUserManager::~GateUserManager()
{
}

bool GateUserManager::addUser(GateUser *user)
{
  if (!addEntry(user))
    return false;

  GateToSuperUserNum cmd;
  cmd.set_num(this->size());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSuper(send, len);

  return true;
}

void GateUserManager::delUser(GateUser *user)
{
  removeEntry(user);

  GateToSuperUserNum cmd;
  cmd.set_num(this->size());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSuper(send, len);
}

GateUser* GateUserManager::getUserByAccID(QWORD accid)
{
  return (GateUser *)getEntryByTempID(accid);
}

void GateUserManager::final()
{
}

bool GateUserManager::timer(DWORD cur)
{
  for (auto &it : xEntryTempID::ets_)
  {
    ((GateUser *)it.second)->timer(cur);
  }
  //2秒后将人物从内存删除
  while (!waitForDelUser.empty())
  {
    if (waitForDelUser.front().first + 2 <= cur)
    {
      SAFE_DELETE(waitForDelUser.front().second);
      waitForDelUser.pop_front();
    }
    else
    {
      break;
    }
  }

  return true;
}

void GateUserManager::process()
{
  xNetProcessor *np = nullptr;
  GateUser *pUser = nullptr;

  xEntryTempID::Iter it = xEntryTempID::ets_.begin(), temp=it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    temp = it++;
    pUser = (GateUser *)(temp->second);

    pUser->sendAllCmd();

    np = pUser->client_task();
    if (!np) continue;
    CmdPair *pair = np->getCmd();
    while (pair)
    {
      Cmd::UserCmd *cmd = (Cmd::UserCmd *)(pair->second);
      thisServer->m_oMessageStat.start(cmd->cmd, cmd->param);
      if (!pUser->doUserCmd(cmd, pair->first))
        XERR << "[消息处理错误]" << pUser->id << pUser->name << "GateUser:(" << cmd->cmd << cmd->param << ")" << XEND;
      thisServer->m_oMessageStat.end();
      pUser->addCmdCounter(cmd->cmd, cmd->param);
      np->popCmd();
      pair = np->getCmd();
    }
  }
  auto iter = loginlist.begin();
  auto tmp = iter;
  for ( ; iter!=loginlist.end(); )
  {
    tmp = iter++;
    pUser = (GateUser *)(tmp->second);

    np = pUser->client_task();
    if (!np) continue;
    CmdPair *pair = np->getCmd();
    while (pair)
    {
      Cmd::UserCmd *cmd = (Cmd::UserCmd *)(pair->second);
      thisServer->m_oMessageStat.start(cmd->cmd, cmd->param);

      if (!pUser->doUserCmd(cmd, pair->first))
        XERR << "[消息处理错误]" << pUser->id << pUser->name << "LoginGateUser:(" << cmd->cmd << cmd->param << ")" << XEND;
      thisServer->m_oMessageStat.end();
      pUser->addCmdCounter(cmd->cmd, cmd->param);
      np->popCmd();
      pair = np->getCmd();
    }
  }
}

void GateUserManager::loginOut(GateUser *user, bool bDelete /*= false*/)
{
#ifndef _DEBUG
  bDelete = false;
#endif

  if (!user) return;

  XLOG << "[网关请求退出]" << user->accid << user->id << user->name << (user->getState()==GateUser_State::running?"运行时":"未进入场景") << XEND;

  LoginOutRegCmd send;
  send.id = user->id;
  send.accid = user->accid;
  send.deletechar = bDelete;
  strncpy(send.gateName, thisServer->getServerName(), MAX_NAMESIZE);
  thisServer->sendCmdToSession(&send,sizeof(send));
}

void GateUserManager::onUserQuit(GateUser *user)
{
  XLOG << "[离线]" << user->accid << user->id << user->name << XEND;
  delUser(user);
  addWaitForDelUser(user);
}

void GateUserManager::addWaitForDelUser(GateUser *user)
{
  waitForDelUser.push_back(std::make_pair(now(), user));
  user->setState(GateUser_State::quit);
}

GateUser* GateUserManager::getLoginUserByAccID(QWORD accid)
{
  auto iter = loginlist.find(accid);
  if (iter!=loginlist.end()) return iter->second;

  return nullptr;
}

bool GateUserManager::addLoginUser(GateUser *user)
{
  if (!user) return false;

  if (loginlist.find(user->accid)!=loginlist.end())
    return false;

  loginlist[user->accid] = user;

  return true;
}

void GateUserManager::delLoginUser(GateUser *user)
{
  if (!user) return;

  loginlist.erase(user->accid);
}

void GateUserManager::kickUserOnScene(xNetProcessor *np)
{
  if (!np) return;

  auto it = xEntryTempID::ets_.begin(), temp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    temp = it++;
    if (temp->second)
    {
      GateUser *user = dynamic_cast<GateUser *>(temp->second);
      if (user && user->scene_task() && (user->scene_task()->getTask() == np))
      {
        removeEntry(user);
        addWaitForDelUser(user);
      }
    }
  }
}

void GateUserManager::sendAllCmd(void *data, DWORD len)
{
  for (auto &it : xEntryTempID::ets_)
  {
    GateUser *pUser = dynamic_cast<GateUser *>(it.second);
    if (pUser)
    {
      pUser->sendCmdToMe(data, len);
    }
  }
}

GateUser* GateUserManager::newLoginUser(xNetProcessor* np, QWORD accid, const std::string &version)
{
  if (!np || !accid) return nullptr;


  /*
#ifndef _ALL_SUPER_GM
  if (thisServer->getServerVersion() != version)
  {
    Cmd::RegErrUserCmd errcmd;

    errcmd.set_ret(REG_ERR_NEW_VERSION);
    PROTOBUF(errcmd, errsend, errlen);
    np->sendCmd(errsend, errlen);

    XLOG << "[请求登录]" << accid << "版本不正确,客户端版本:" << version << "服务器版本:" << thisServer->getServerVersion() << XEND;
    return nullptr;
  }
#endif
*/
  if (!thisServer->inVerifyList(np)) return nullptr;

  np->set_id(accid);

  GateUser *pLoginUser = getLoginUserByAccID(accid);
  if (pLoginUser)
  {
    if (pLoginUser->client_task() == np)
    {
      return pLoginUser;
    }

    ReconnectClientUserCmd send;
    np->sendCmd(&send, sizeof(send));

    XDBG << "[请求登录]" << accid << "LoginUser已存在,稍后重连" << np << XEND;

    return nullptr;
  }
  else
  {
    pLoginUser = NEW GateUser(accid, np);

    if (!addLoginUser(pLoginUser))
    {
      XERR << "[请求登录]" << accid << "添加LoginUser失败" << XEND;

      thisServer->addCloseList(np, TerminateMethod::terminate_active, "添加LoginUser失败");
      SAFE_DELETE(pLoginUser);
      return nullptr;
    }
    XDBG << "[请求登录]" << accid << "添加LoginUser" << XEND;
  }

  thisServer->removeVerifyList(np);

  return pLoginUser;
}

void GateUserManager::kickUser(GateUser *pUser)
{
  if (!pUser) return;

  Cmd::DisconnectClientUserCmd send;
  pUser->sendCmdToMe(&send, sizeof(send));

  /*
  thisServer->addCloseList(pUser->client_task(), TerminateMethod::terminate_active);
  pUser->set_client_task(nullptr);
  */
}

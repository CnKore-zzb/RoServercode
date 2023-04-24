#include "Authorize.h"
#include "SessionUser.h"
#include "Authorize.pb.h"
#include "RecordCmd.pb.h"
#include "MsgManager.h"
#include "LoginUserCmd.pb.h"
#include "CommonConfig.h"
#include "SessionScene.h"
#include "PlatLogManager.h"

extern "C"
{
#include "xlib/md5/md5.h"
}

Authorize::Authorize(SessionUser* pUser) : m_pUser(pUser)
{
}

Authorize::~Authorize()
{
}

bool Authorize::checkResetTime()
{
  DWORD cursec = xTime::getCurSec();
  if(m_password.empty() == false && m_pwdResetTime != 0 && cursec >= m_pwdResetTime)
  {
    deletepwd();
    return true;
  }

  return false;
}

void Authorize::syncInfoToGuild()
{
//  AuthorizeInfoSyncSocialCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_ignorepwd(m_ignorePwd);
//  PROTOBUF(cmd, send, len);
//  thisServer->sendCmdToSession(send, len);
}

void Authorize::syncInfoToSocial()
{
//  AuthorizeInfoSessionCmd cmd;
//  cmd.set_charid(m_pUser->id);
//  cmd.set_ignorepwd(m_ignorePwd);
//  PROTOBUF(cmd, send, len);
//  thisServer->sendCmdToSession(send, len);
}

bool Authorize::setPwd(const string& str, const string& oldpwd)
{
  DWORD size = str.size();
  if(size < 6 || size > 12)
  {
    MsgManager::sendMsg(m_pUser->id, 6000);
    XERR << "[安全密码-设置] 错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
    return false;
  }

  checkResetTime();

  char oldsign[1024];
  bzero(oldsign, sizeof(oldsign));
  upyun_md5(oldpwd.c_str(), oldpwd.size(), oldsign);

  if(oldpwd.empty() == false && strncmp(oldsign,m_password.c_str(),1024) != 0)
  {
    m_failtimes++;
    DWORD maxfail = CommonConfig::m_dwPwdFailTimes;
    if(m_failtimes >= maxfail)
    {
      Cmd::NotifyAuthorizeUserCmd cmd;
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
    }
    else
      MsgManager::sendMsg(m_pUser->id, 6006);
    XERR << "[安全密码-设置] 错误 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "密码错误" << XEND;
    return false;
  }

  char sign[1024];
  bzero(sign, sizeof(sign));
  upyun_md5(str.c_str(), str.size(), sign);

  if(str == oldpwd)
  {
    MsgManager::sendMsg(m_pUser->id, 6014);
    return false;
  }

  m_password = sign;
  m_ignorePwd = true;
  m_pwdResetTime = 0;
  syncInfoToUser();
  syncInfoToScene();

  ChangeAuthorizeRecordCmd cmd;
  cmd.set_accid(m_pUser->accid);
  cmd.set_password(sign);
  cmd.set_resettime(0);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToRecord(send, len);

  if(oldpwd.empty() == true)
    MsgManager::sendMsg(m_pUser->id, 6002);
  else
    MsgManager::sendMsg(m_pUser->id, 6005);
  
  string strOldSign = oldsign;
  PlatLogManager::getMe().changeFlagLog(thisServer,
    0,
    0,
    m_pUser->id,
    EChangeFlag_Safe_PWD,
    strOldSign, m_password);

  XLOG << "[安全密码-设置] 成功 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "密码: " << str << XEND;
  return true;
}

void Authorize::deletepwd()
{
  string oldpwd = m_password;
  m_password.clear();
  m_ignorePwd = true;
  m_pwdResetTime = 0;
  syncInfoToUser();
  syncInfoToSocial();
  syncInfoToGuild();
  syncInfoToScene();

  ChangeAuthorizeRecordCmd cmd;
  cmd.set_accid(m_pUser->accid);
  cmd.set_password(m_password);
  cmd.set_resettime(0);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToRecord(send, len);

  string empty = "";
  PlatLogManager::getMe().changeFlagLog(thisServer,
    0,
    0,
    m_pUser->id,
    EChangeFlag_Safe_PWD,
    oldpwd, empty);

  XLOG << "[安全密码-删除] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
}

void Authorize::syncInfoToUser()
{
  ConfirmAuthorizeUserCmd cmd;
  cmd.set_success(m_ignorePwd);
  cmd.set_resettime(m_pwdResetTime);
  if(m_password.empty() == true)
    cmd.set_hasset(false);
  else
    cmd.set_hasset(true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void Authorize::resetPwd(bool reset)
{
  if(m_password.empty() == true)
  {
    XERR << "[安全密码-重置] 失败 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << reset << XEND;
    return;
  }

  if(reset == true && m_pwdResetTime == 0)
  {
    DWORD cursec = xTime::getCurSec();
    DWORD resetTime = MiscConfig::getMe().getAuthorize().dwResetTime;
    m_pwdResetTime = resetTime + cursec;
    syncInfoToUser();
    MsgManager::sendMsg(m_pUser->id, 6004);
    XLOG << "[安全密码-重置] 成功 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
  }
  else if(reset == false && m_pwdResetTime!= 0)
  {
    m_pwdResetTime = 0;
    syncInfoToUser();
    MsgManager::sendMsg(m_pUser->id, 6009);
    XLOG << "[安全密码-取消重置] 成功 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << XEND;
  }
  else
  {
    XLOG << "[安全密码-重置] 失败 " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << reset
      << m_pwdResetTime << XEND;
    return;
  }

  ChangeAuthorizeRecordCmd cmd;
  cmd.set_accid(m_pUser->accid);
  cmd.set_password(m_password);
  cmd.set_resettime(m_pwdResetTime);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToRecord(send, len);
}

void Authorize::setAuthorize(const string& str, bool ignore, DWORD resettime, bool sync)
{
  m_password = str;
  m_ignorePwd = ignore;
  m_pwdResetTime = resettime;

  if(checkResetTime() == false && sync == true)
    syncInfoToScene();
}

void Authorize::syncInfoToScene()
{
  if(m_pUser->getScene())
  {
    AuthorizeInfoSessionCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_ignorepwd(m_ignorePwd);
    PROTOBUF(cmd, send, len);
    bool ret = m_pUser->getScene()->sendCmd(send, len);
    XLOG << "[安全密码-同步场景] 成功" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name 
      << "m_ignorePwd " << m_ignorePwd << "ret" << ret << m_pUser->getScene()->name << XEND;
  }
  else
  {
    XERR << "[安全密码-同步场景] 失败" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name 
      << "m_ignorePwd " << m_ignorePwd << XEND;
  }
}

void Authorize::setRealAuthorize(bool bAuthorized)
{ 
  m_bRealAuthorized = bAuthorized; 
  XLOG << "[实名认证] 设置 accid" << m_pUser->accid << m_pUser->id << m_pUser->name << m_bRealAuthorized << XEND;
}

/**
 * @file Authorize.h
 * @brief 
 * @author liangyongqiang, liangyongqiang@xindong.com
 * @version v1
 * @date 2017-05-12
 */

#pragma once

#include "xDefine.h"

using std::string;
using namespace Cmd;

class SessionUser;

class Authorize
{
  public:
    Authorize(SessionUser* pUser);
    ~Authorize();

    bool setPwd(const string& str, const string& oldpwd);
    void resetPwd(bool reset);
    void syncInfoToUser();
    void syncInfoToGuild();
    void syncInfoToSocial();
    void syncInfoToScene();
    void setIgnorePwd(const string& str, bool ignore, DWORD resettime);
    void setAuthorize(const string& str, bool ignore, DWORD resettime, bool sync=true);
    void deletepwd();
    bool getConfirmed() { return m_ignorePwd; }
    bool checkResetTime();
    void setRealAuthorize(bool bAuthorized);
    bool getRealAutorize() { return m_bRealAuthorized; }
  private:
    SessionUser* m_pUser = nullptr;

    string m_password;
    DWORD m_pwdResetTime = 0;
    bool m_ignorePwd = false;
    DWORD m_failtimes = 0;
    bool m_bRealAuthorized = false; //实名认证
};

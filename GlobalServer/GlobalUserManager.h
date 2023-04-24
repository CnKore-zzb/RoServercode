/**
 * @file GlobalUserManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-02-06
 */

#include "xSingleton.h"
#include "SocialCmd.pb.h"

using namespace Cmd;
using std::map;

class GlobalUser
{
  public:
    GlobalUser(const SocialUser& rUser);
    ~GlobalUser();

    QWORD charid() const { return m_oUser.charid(); }
    DWORD zoneid() const { return m_oUser.zoneid(); }

    bool doSocialCmd(const BYTE* buf, WORD len);
  private:
    SocialUser m_oUser;
};

typedef map<QWORD, GlobalUser*> TMapGlobalUser;
class GlobalUserManager : public xSingleton<GlobalUserManager>
{
  friend class xSingleton<GlobalUserManager>;
  private:
    GlobalUserManager();
  public:
    virtual ~GlobalUserManager();

    void onUserOnline(const SocialUser& rUser);
    void onUserOffline(const SocialUser& rUser);

    GlobalUser* getGlobalUser(QWORD qwCharID);
  private:
    TMapGlobalUser m_mapGlobalUser;
};


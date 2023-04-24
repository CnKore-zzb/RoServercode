
#include "xSingleton.h"
#include "SocialCmd.pb.h"
#include "SceneTrade.pb.h"

using namespace Cmd;
using std::map;

class TradeUser
{
  public:
    TradeUser(const SocialUser& rUser);
    ~TradeUser();

    QWORD charid() const { return m_oUser.charid(); }
    DWORD zoneid() const { return m_oUser.zoneid(); }
    const string& name()const { return m_oUser.name(); }
    void setPermission(EPermission per, DWORD value);
    DWORD getPerValue(EPermission per);
  private:
    SocialUser m_oUser;
    std::map<EPermission, DWORD> m_mapPer;
};

typedef map<QWORD, TradeUser*> TMapTradeUser;
class TradeUserMgr : public xSingleton<TradeUserMgr>
{
  friend class xSingleton<TradeUserMgr>;
  private:
    TradeUserMgr();
  public:
    virtual ~TradeUserMgr();

    void onUserOnline(const SocialUser& rUser);
    void onUserOffline(const SocialUser& rUser);

    TradeUser* getTradeUser(QWORD qwCharID);
  private:
    TMapTradeUser m_mapTradeUser;
};


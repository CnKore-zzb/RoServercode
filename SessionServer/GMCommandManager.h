/**
 * @file GMCommandManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-06-23
 */

#pragma once

#include "xSingleton.h"
#include "json/json.h"
#include "GMTools.pb.h"
#include "xLuaTable.h"
#include <list>

using namespace Cmd;
using Json::Value;
using std::string;

struct SDeposit;
struct SGlobalActCFG;

enum EJsonRetType
{
  EJSONRET_MESSAGE = 0,
  EJSONRET_DATA = 1,
};

typedef bool (*GmSessionFun)(const xLuaData &);
struct GmSessionCmd
{
  char cmd[MAX_NAMESIZE];
  GmSessionFun p;
  BYTE pri;
  char desc[256];
};

struct GMRetry
{
  DWORD mapId;
  DWORD count;
  SessionGMCmd cmd;
};

class SessionUser;
class GMCommandManager : public xSingleton<GMCommandManager>
{
  friend class xSingleton<GMCommandManager>;
  private:
    GMCommandManager();
  public:
    virtual ~GMCommandManager();

    const TVecString& processGMCommand(const ExecGMCmd& cmd);
  private:
    const TVecString& getPlayerInfo(const Value& root);
    const TVecString& execGMCommand(const Value& root, const ExecGMCmd& cmd);
    const TVecString& sendNotice(const Value& root, const ExecGMCmd& cmd);
    const TVecString& sendMail(const Value& root);
    const TVecString& chargeMoney(const Value& root);
    const TVecString& loadConfig(const Value& root);
    const TVecString& gagPlayer(const Value& root);
    const TVecString& lockPlayer(const Value& root);
    const TVecString& lockAccount(const Value& root);
    const TVecString& checkCharge(const Value& root);
    const TVecString& moveGuildZone(const Value& root);
    const TVecString& useGiftCode(const Value& root);
    const TVecString& tradeSecurityCMD(const Value& root);
    const TVecString& modifyAuctionTime(const Value& root);
    const TVecString& stopAuction();
    const TVecString& verifyGuildIcon(const Value& root);
    const TVecString& clearMailTemplate(const Value& root, const ExecGMCmd& cmd);

    void setJson(EJsonRetType eType, const string& str);
    void appendData(EJsonRetType eType, const string& str);
    void appendData(EJsonRetType eType, QWORD id);

  public:
    void execute(SessionUser* pUser, const string& command);
    void execute(SessionUser* pUser, const xLuaData& command);
  public:
    void timerTick(DWORD curSec);
    bool execute(const xLuaData& command, const TSetDWORD& extraMap);
    static bool broadcast(const xLuaData& data);
    static bool replace_reward(const xLuaData& data);
    static bool recover_reward(const xLuaData& data);
    static bool set_wantedactive(const xLuaData& data);
    void pushtyrantdb(QWORD accid, QWORD charId, std::string orderid, DWORD amount, DWORD itemCount, string productId, string chargeType); 
    bool canBuy(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym, bool bFromWeb, const SGlobalActCFG** pActCfg,bool&bUpdateLimit);
  /* bool canBuyFuDai(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym);
    bool updateFuDaiDb(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD ym, DWORD count);*/
    bool updateBuyLimitDb(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD ym, DWORD count);
    bool getDepositLimit(QWORD charid, DWORD ym, xLuaData& data);

    static bool startglobalactivity(const xLuaData& data);
    static bool stopglobalactivity(const xLuaData& data);

    static bool add_ntfactprogress(const xLuaData& data);
    static bool del_ntfactprogress(const xLuaData& data);
  private:
    bool canBuyLimit(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym, bool bFromWeb);

  private:
    TVecString m_vecResult;
    std::list<GMRetry> m_retryList;
    xTimer m_oTenSecTimer;
};


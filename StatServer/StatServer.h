#pragma once
#include <set>
#include "xDefine.h"
#include "RegionServer.h"
#include "StatCmd.pb.h"

class xNetProcessor;

class StatServer : public RegionServer
{
  public:
    StatServer(OptArgs &args);
    virtual ~StatServer();

  public:
    virtual ServerType getServerType() { return SERVER_TYPE_STAT; }

  protected:
    virtual void v_final();
    virtual bool v_init();
    virtual void v_closeNp(xNetProcessor *np);
    virtual void v_timetick();

  protected:
    virtual bool regist(xNetProcessor* np, DWORD zoneid, DWORD serverType);

  protected:
    virtual bool doStatCmd(xNetProcessor *np, const BYTE* buf, WORD len);

  public:
    bool broadCmdToAll(const void* data, unsigned short len);

  private:
    bool loadConfig();
    bool updateMysql();
    bool exeSqlFile(const char *file);
    bool tradelog(const TradeToStatLogCmd& cmd);

    //
    void checkTradeLogDel(DWORD curSec);
    void delDb();
    //生成道具兑换码
    void generateItemCode(DWORD curSec);

  private:
    std::set<xNetProcessor *> m_list;
    xTimer m_oTickOneSec;
    xTimer m_oTickOneHour;
    xDayTimer m_oTickOneDay;
    bool m_dbOk = false;

    DWORD m_dwPublicityBuyingOffset = 0;
    DWORD m_dwBuydelOffset = 0;
    DWORD m_dwSellDelOffset = 0;
    DWORD m_dwCanTakeOffsetB = 0;
    DWORD m_dwCanTakeOffsetS = 0;

    DWORD m_dwItemCodeTime = 0;

};

extern StatServer *thisServer;

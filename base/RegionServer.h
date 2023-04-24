#pragma once
#include "xServer.h"
#include "SessionCmd.pb.h"

class RegionServer : public xServer
{
  public:
    RegionServer(OptArgs &args);
    virtual ~RegionServer() {}

  public:
    virtual bool isZoneServer() { return false; }
    virtual bool isRegionServer() { return true; }

    void collectZoneIDs(TSetDWORD& setIDs) const;
  protected:
    virtual void v_final();
    virtual bool v_init() { return true; }
    virtual bool v_callback();
    virtual void v_timetick();
    virtual void v_closeNp(xNetProcessor*);
    
    virtual void v_zoneDel(DWORD zoneId) {}
  private:
    virtual bool init();
  private:
    void process();

  protected:
    virtual bool doCmd(xNetProcessor* np, BYTE* buf, WORD len);
    virtual bool doRegCmd(xNetProcessor *np, BYTE* buf, WORD len) { return false; }
    bool doRegionCmd(xNetProcessor *np, BYTE* buf, WORD len);
    virtual bool doStatCmd(xNetProcessor *np, const BYTE* buf, WORD len) { return false; }
    virtual bool doTradeCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doMatchCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doAuctionCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doWeddingCmd(BYTE* buf, WORD len) { return false; }
  public:
    virtual bool doSocialCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doTeamCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doGuildCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doGZoneCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doSessionCmd(BYTE* buf, WORD len) { return false; }
    virtual bool doBossSCmd(BYTE* buf, WORD len) { return false; }
    virtual bool sendMail(Cmd::SendMail& cmd) { return false;}
    virtual void onSessionRegist(xNetProcessor *np, DWORD dwZoneId) {}

  public:
    bool sendCmdToZone(DWORD zoneid, const void* data, unsigned short len);
    bool sendCmdToOneZone(const void* data, unsigned short len);
    bool sendCmdToAllZone(const void* data, unsigned short len, DWORD except = 0);
  protected:
    virtual bool regist(xNetProcessor* np, DWORD zoneid, DWORD serverType);
  private:
    std::map<DWORD, xNetProcessor *> m_oZoneList;

    bool m_blAcceptZone = true;
    xTime m_oFrameTime;
};

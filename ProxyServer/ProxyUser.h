#pragma once
//#include "Define.h"
#include "xEntry.h"
#include "ProxyServer.h"
#include "xTime.h"
#include "UserData.h"
#include "ErrorUserCmd.pb.h"
#include "ProxyUserDataThread.h"
#include "xlib/xSpeedStat.h"

class ProxyUserManager;
class xNetProcessor;
class ProxyUserData;

enum class ProxyUser_State
{
  create      = 0,
  running     = 1,
  quit        = 2,
};

class ReconnectController
{
  public:
    ReconnectController();
    ~ReconnectController();

  public:
    bool isNeed() { return m_blNeed; }
    void reset()
    {
      m_blNeed = false;
      m_dwNextTime = 0;
      m_dwCount = 0;
    }
    bool need();
    void set();

  private:
    bool m_blNeed = false;
    DWORD m_dwNextTime = 0;
    DWORD m_dwCount = 0;
};

struct RegionStatusInfo;

class ProxyUser : public xEntry
{
  friend class ProxyUserManager;
  public:
    ProxyUser(QWORD accid, DWORD reginid, xNetProcessor* client_task);
    virtual ~ProxyUser();

  public:
    bool forwardGateServer(void* cmd, unsigned short len);
    bool forwardClientTask(void* cmd, unsigned short len);

    bool doClientCmd(xCommand *cmd, unsigned short len);
    bool parseClientCmd(xCommand *cmd, unsigned short len);
    bool doGateServerCmd(xCommand *buf, unsigned short len);

  public:
    void timer(DWORD cur);

  public:
    bool checkRegion();
    DWORD checkZone(DWORD zoneid, QWORD charid, DWORD originalzoneid);
  private:
    DWORD getWhiteListFlag();
    void notifyError(Cmd::RegErrRet ret);
    void maintainNtf(const RegionStatusInfo& info);
    bool connectCharServer(DWORD dwRegionID, QWORD charid, bool checkZ=true);
    bool connectGateServer(DWORD zoneid);

  private:
    xNetProcessor *m_pGateServerTask = nullptr;
    xSpeedStat m_oGateSpeedStat;
  private:
    xNetProcessor *m_pClientTask = nullptr;
    xSpeedStat m_oClientSpeedStat;

  public:
    ProxyUserData* createProxyUserData(ProxyUserDataAction act);
    bool get(ProxyUserData *pData);
  public:
    DWORD m_dwHeartBeat = 0;

  public:
    void sendSnapShotToMe();
    void realSendSnapShotToMe(ProxyUserData *pData);
    void selectChar(QWORD charid, const string& deviceid);
    void autoSelectChar(QWORD charid);
    void kickChar(QWORD charid);
  private:
    void realDeleteChar();
  public:
    QWORD accid = 0;
    DWORD m_dwRegionID = 0;
    SnapShotData m_oSnapShotDatas[MAX_CHAR_NUM];
    AccBaseData m_oAccBaseData;
    // 已经连接的区
    DWORD m_dwZoneID = 0;
    // 记录选择的角色
    QWORD m_qwCharID = 0;
    DWORD m_dwKickCharTick = 0;
    // 记录客户端版本
    std::string m_strServerVersion;
    // 连接的域名
    std::string m_strDomain;
    // 连接的ip
    std::string m_strTaskIP;
    // 设备类型
    std::string m_strDevice;
    //绑定的手机号
    std::string m_strPhone;
    //安全设备
    bool m_bSafeDevice = false;
    //实名认证
    bool m_bAuthorize = false;

    // 重连管理
    ReconnectController m_oReconnectController;

    //安全密码
    //Authorize m_oAuthorize;
    DWORD m_dwFailTimes = 0;
    bool m_blConfirmed = false;
    DWORD m_dwValidTime = 0;

  public :
    bool confirmAuthorize(const string& str);
    void syncAuthorizeToUser(bool ignorePwd, bool hasSet);
    void syncAuthorizeToGate();
    void onLineAuthorize();
    bool checkResetTime();
    bool checkNoLoginTime();

  public:
    ProxyUser_State getState() { return m_oProxyUserState; }
    void setState(ProxyUser_State state);
  private:
    ProxyUser_State m_oProxyUserState = ProxyUser_State::create;

    // 消息频率监控
  public:
    void addCmdCount(xCommand *cmd);
  private:
    void printCmdCount();
    void clearCmdCount();
  private:
    DWORD m_dwCmdCount = 0;
    DWORD m_dwCmdCountTime = 0;
    std::map<DWORD, DWORD> m_oCmdList;

    // 消息频率控制
  public:
    bool isCmdOverLimit(xCommand *cmd);
  private:
    void clearCmdLimit();
    DWORD m_dwCmdLimitTime = 0;
    std::map<DWORD, DWORD> m_oCmdLimitList;

    // nonce
  private:
    bool checkNonce(DWORD timestamp, DWORD index, const std::string &sign);
    DWORD m_dwNonceTime = 0;
    DWORD m_dwNonceIndex = 0;

    // 客户端帧数
  private:
    DWORD m_dwClientFrame = 0;
    DWORD m_dwClientFrameTimestamp = 0;

  private:
    DWORD m_dwNotifyDelChar = false;
  public:
    DWORD m_dwLanguage = 0;
    void setRealAuthorize(DWORD timeStamp, string strAuthorize);
    void syncRealAuthorize2Gate();
  private:
    DWORD getMaxBaseLv();
};

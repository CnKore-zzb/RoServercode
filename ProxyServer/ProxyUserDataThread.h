#pragma once
#include "xThread.h"
#include "xDefine.h"
#include "xQueue.h"
#include "xSingleton.h"
#include "base/UserData.h"
#include "xNoncopyable.h"
#include "base/xlib/xDBConnPool.h"

class ProxyUser;

enum ProxyUserDataAction
{
  ProxyUserDataAction_NULL            = 0,
  ProxyUserDataAction_SEND            = 1,
  ProxyUserDataAction_SET_DELETE      = 2,
  ProxyUserDataAction_DELETE_CHAR     = 3,
  ProxyUserDataAction_CANCEL_DELETE   = 4,
  ProxyUserDataAction_LOGIN           = 5,
};

enum ProxyUserDeleteRet
{
  ProxyUserDeleteRet_NULL = 0,
  ProxyUserDeleteRet_OK = 1,
  ProxyUserDeleteRet_LOCKED  = 2,
  ProxyUserDeleteRet_ERROR = 3,
  ProxyUserDeleteRet_MSG_1067 = 4,
};

class ProxyUserData : private xNoncopyable
{
  public:
    ProxyUserData(QWORD accid, DWORD regionid, ProxyUserDataAction act, ProxyUser *user);
    virtual ~ProxyUserData();

  public:
    bool getSnapShot();
    void setDeleteChar();
    void cancelDeleteChar();

  public:
    SnapShotData m_oSnapShotDatas[MAX_CHAR_NUM];
    AccBaseData m_oAccBaseData;

    QWORD m_qwAccID = 0;
    DWORD m_dwRegionID = 0;
    bool m_blRet = false;

    ProxyUserDataAction m_oAction = ProxyUserDataAction_NULL;
    ProxyUser *m_pUser = nullptr;

    // ProxyUserDataAction_SET_DELETE
    QWORD m_qwDeleteCharID = 0;
    ProxyUserDeleteRet m_oDeleteRet = ProxyUserDeleteRet_NULL;

    // ProxyUserDataAction_CANCEL_DELELTE
    bool m_blCancelDeleteRet = false;
};

class ProxyUserDataThread : public xThread, public xSingleton<ProxyUserDataThread>
{
  friend class ProxyUserData;
  public:
    ProxyUserDataThread();
    virtual ~ProxyUserDataThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    void addDataBase(std::string dbname);

  public:
    void add(ProxyUserData *data);
    ProxyUserData* get();
    void pop();

  private:
    xQueue<ProxyUserData> m_oPrepareQueue;
    xQueue<ProxyUserData> m_oFinishQueue;

  private:
    void check();
    void exec(ProxyUserData *data);

  private:
    DBConnPool& getDBConnPool()
    {
      return m_oDBConnPool;
    }
    DBConnPool m_oDBConnPool;

  private:
    const char* getRegionDBName(DWORD regionid);
  private:
    // region dbname
    std::map<DWORD, std::string> m_mapRegionDB;

    // 平台数据
  private:
    std::string getPlatName(DWORD platid);
  private:
    std::map<DWORD, std::string> m_mapPlatIDName;
};

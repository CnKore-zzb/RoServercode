#pragma once

#include <list>
#include "xDefine.h"
#include "xSingleton.h"
#include "xCommand.h"
#include "ChatCmd.pb.h"
#include "GateServer.h"
#include "GateUser.h"

enum EFilterResult
{
  ERESULT_MIN = 0,
  ERESULT_NORMAL = 1, // 正常
  ERESULT_WARN = 2, // 可疑
  ERESULT_ADVERT = 3, // 广告
  ERESULT_FORBID = 4, // 非法
  ERESULT_BLACK = 5, // 黑名单
  ERESULT_REPEAT = 6, // 刷屏
  ERESULT_MAX = 7,
};

class CmdInfo
{
  public:
    CmdInfo() {}
    ~CmdInfo() {}

  public:
    void set(QWORD id, const BYTE* c, WORD l);
  public:
    bool sendToScene(std::string str = "");
    bool sendToSelf();

  private:
    xData m_oData;
    QWORD m_qwAccID = 0;
};

class ChatFilterManager : public xSingleton<ChatFilterManager>
{
  friend xSingleton<ChatFilterManager>;

  public:
    virtual ~ChatFilterManager(){}

  private:
    ChatFilterManager() { init(); }

  public:
    void init();


  private:
    typedef std::pair<DWORD, DWORD> pairIT; // <id, timestamp>

  private:
    bool m_bBusy = false;  // 忙碌区分，用于缓解平台压力
    DWORD m_dwCmd = 0;
    std::map<DWORD, CmdInfo> m_mapCmd;
    std::list<pairIT> m_list;

  private:
    DWORD generateId();

    void pushList(pairIT it) { m_list.push_back(it); }
    void popList() { m_list.pop_front(); }
    DWORD sizeList() { return m_list.size(); }

    void addCmd(DWORD id, QWORD accid, const BYTE *buf, WORD len);
    void delCmd(DWORD id);
    void freeCmd(DWORD id);
    CmdInfo* getCmd(DWORD id);

  public:
    bool isBusy() { return m_bBusy; }
    bool checkFilter(GateUser* pUser, const UserCmd* cmd, WORD len);
    bool doCmd(BYTE* buf, WORD len);
    void timer(DWORD cur);

    // test
    /*GateUser* m_pUser;

    ChatCmd m_cmd;
    string getChat();
    void setTest(GateUser* p, ChatCmd* c);
    void test();
    string arrFilter[10] = {" hello ", " i ", " world ", " word ", " love ", " c++ ", " some ", " thing ", " is ", " wrong "};
    */
};

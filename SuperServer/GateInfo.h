#pragma once
#include "xSingleton.h"
#include "xDBConnPool.h"
#include <arpa/inet.h>

class xNetProcessor;

class GateInfoM : public xSingleton<GateInfoM>
{
  friend class xSingleton<GateInfoM>;

  public:
    GateInfoM();
    ~GateInfoM();

  public:
    bool init();
    void addGate(xNetProcessor *np);
    void delGate(xNetProcessor *np);
    void delAll();
    void updateUserNum(xNetProcessor* np, WORD num);

  private:
    // port num
    std::map<DWORD, DWORD> m_list;
    // port ip:port
    std::map<DWORD, std::string> m_oKeyList;
};

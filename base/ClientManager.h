#pragma once
#include <map>
#include "xDefine.h"

enum class ClientType
{
  stat_server = 0,
  trade_server = 1,
  global_server = 2,
  log_server = 3,
  guild_server = 5,
  gzone_server = 6,
  match_server = 7,
  auction_server = 8,
  plat_server = 10,
  g_proxy_server = 11,
  wedding_server = 12,
  max
};

const DWORD CLIENT_TYPE_MAX = (DWORD)ClientType::max;

class xServer;
class xNetProcessor;

enum class ClientStatus
{
  create = 0,
  verified,
  establish,
  disconnect,
};

struct ClientProcessor
{
  bool m_blConnect = false;
  std::string m_strIP;
  DWORD m_dwPort = 0;
  xNetProcessor *m_pTask = nullptr;
  ClientStatus m_oStatus = ClientStatus::create;
  ClientType m_eType = ClientType::max;
  bool m_blTask = true;

  inline void init(ClientType type, std::string ip, DWORD port)
  {
    m_strIP = ip;
    m_dwPort = port;
    m_blConnect = true;
    m_eType = type;
    m_blTask = false;
  }
  void connect(xServer *p);
};

class ClientManager
{
  public:
    ClientManager(xServer *p);
    ~ClientManager();

  public:
    void closeNp(xNetProcessor *np);
    void timer();

    // add client
    void add(ClientType type, std::string ip, DWORD port);
    // add task
    void add(ClientType type, xNetProcessor *np);
    bool sendCmd(ClientType type, BYTE *buf, DWORD len);
    bool process();
  public:
    xServer *m_pServer = nullptr;
  private:
    ClientProcessor m_list[CLIENT_TYPE_MAX];
};

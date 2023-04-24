#pragma once
#include "xSocket.h"
#include "xTaskThread.h"
#include "xTime.h"
#include "xEntry.h"

enum class NPState
{
  create = 0,
  verified,
  establish,
  disconnect,
};

enum NET_PROCESSOR_TYPE
{
  NET_PROCESSOR_TYPE_CLIENT = 1,
  NET_PROCESSOR_TYPE_TASK   = 2,
  NET_PROCESSOR_TYPE_PLAT_CLIENT   = 3,
  NET_PROCESSOR_TYPE_FLUENT_CLIENT = 4,
  NET_PROCESSOR_TYPE_MAX
};

class xCommand;

class xNetProcessor : public xEntry
{
  friend class xServer;
  public:
    xNetProcessor(const char *n, NET_PROCESSOR_TYPE net_processor_type);
    virtual ~xNetProcessor();

  public:
    void setName(const char *n);
  public:
    bool isClient()
    {
      return NET_PROCESSOR_TYPE_CLIENT == net_processor_type_;
    }
    bool isTask()
    {
      return NET_PROCESSOR_TYPE_TASK == net_processor_type_;
    }
    bool isPlatClient()
    {
      return NET_PROCESSOR_TYPE_PLAT_CLIENT == net_processor_type_;
    }
    bool isFluentClient()
    {
      return NET_PROCESSOR_TYPE_FLUENT_CLIENT == net_processor_type_;
    }

  private:
    NET_PROCESSOR_TYPE net_processor_type_;

  public:
    void disconnect();

    // 心跳
  public:
    void tick(DWORD cur)
    {
      if (!m_blTick) return;

      if (m_dwTickTime && m_dwTickTime < cur)
      {
        m_dwTickTime = cur;
        sendNoDataCmd();

        return;
      }

      if (!m_dwTickTime)
      {
        m_dwTickTime = cur;
      }
    }
    void setTick(bool flag)
    {
      m_blTick = true;
    }
  private:
    DWORD m_dwTickTime = 0;
    xCommand m_oCmd;
    bool m_blTick = false;

    // socket
  public:
    void addEpoll(int epfd)
    {
      if (isTerminate()) return;

      m_nEpollFD = epfd;

      epoll_event ev;
      bzero(&ev, sizeof(ev));
      ev.data.ptr = this;
      ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
      epoll_ctl(m_nEpollFD, EPOLL_CTL_ADD, m_oSocket.getSockFD(), &ev);

      XLOG_T("[xNetProcessor],add epoll,%u,%s,%p", epfd, name, this);
    }
    void addEpoll()
    {
      if (isTerminate()) return;

      if (m_nEpollFD == INVALID_SOCKET) return;

      epoll_event ev;
      bzero(&ev, sizeof(ev));
      ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
      ev.data.ptr = this;
      epoll_ctl(m_nEpollFD, EPOLL_CTL_MOD, m_oSocket.getSockFD(), &ev);
    }
    void delEpoll()
    {
      XDBG_T("[xNetProcessor],%u,del epoll epfd:%u", m_oSocket.getSockFD(), m_nEpollFD);
      epoll_event ev;
      bzero(&ev, sizeof(ev));
      ev.data.ptr = this;
      ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
      epoll_ctl(m_nEpollFD, EPOLL_CTL_DEL, m_oSocket.getSockFD(), &ev);
    }
    int getEpollFD() { return m_nEpollFD; }
  private:
    SOCKET m_nEpollFD;

  public:
    bool isValid()
    {
      return m_oSocket.valid();
    }
    bool connect(const char *ip, int port)
    {
      return m_oSocket.connect(ip, port);
    }
    bool accept(int sockfd, const sockaddr_in &addr)
    {
      return m_oSocket.accept(sockfd, addr);
    }
    void setComp(bool flag)
    {
      m_oSocket.setComp(flag);
    }
    const xSocket& getSocket() const
    {
      return m_oSocket;
    }
    in_addr& getIP()
    {
      return m_oSocket.getIP();
    }
    WORD getPort()
    {
      return m_oSocket.getPort();
    }
  private:
    xSocket m_oSocket;

  public:
    void terminate(TerminateMethod method, const char *desc);
    inline bool isTerminate() { return m_eTerminateMethod!=TerminateMethod::terminate_no; }
  private:
    volatile TerminateMethod m_eTerminateMethod;

  public:
    inline int sendData()
    {
      return m_oSocket.sendData();
    }
    bool recvData();
    bool sendRawData(void* data, DWORD len);
    //发送消息
    bool sendCmd(const void *cmd, unsigned short len);
    bool sendNoDataCmd();
    void readCmd();
    CmdPair* getCmd();
    void popCmd();

  private:
    xCmdQueue m_oQueue;

  public:
    NPState getNpState() { return m_oState; }
    void setNpState(NPState state) { m_oState = state; }
  private:
    NPState m_oState = NPState::create;

  public:
//    xTaskThread *m_oThread = nullptr;
};

#include "xNetProcessor.h"
#include "ClientCmd.h"

xNetProcessor::xNetProcessor(const char *n, NET_PROCESSOR_TYPE net_processor_type):net_processor_type_(net_processor_type),m_oSocket(this)
{
  setName(n);
  m_nEpollFD = INVALID_SOCKET;
  m_eTerminateMethod = TerminateMethod::terminate_no;

  m_oCmd.cmd = 1;
  m_oCmd.param = 10;

  XLOG_T("[xNetProcessor],%s,create:%p", name, this);
}

xNetProcessor::~xNetProcessor()
{
}

void xNetProcessor::setName(const char *n)
{
  set_name(n);
  m_oSocket.setName();
}

void xNetProcessor::disconnect()
{
  delEpoll();
  m_oSocket.close();
  XLOG_T("[Socket],%s,disconnect:%p", name, this);
}

bool xNetProcessor::sendCmd(const void *cmd, unsigned short len)
{
  if (isTerminate()) return false;

  if (!cmd || !len) return false;

  if (!m_oSocket.sendCmd(cmd, len))
  {
    XERR_T("[xNetProcessor],%s,sendCmd failed:%p", name, this);
    return false;
  }
  else
  {
    addEpoll();
  }
  return true;
}

bool xNetProcessor::sendNoDataCmd()
{
#ifdef _LX_DEBUG
  XLOG_T("[HeartBeat],%llu,%s", id, name);
#endif

  return sendCmd(&m_oCmd, sizeof(m_oCmd));

  /*
  if (isTerminate()) return false;

  if (!m_oSocket.sendNoDataCmd())
  {
    return false;
  }
  else
  {
    addEpoll();
  }
  return true;
  */
}

bool xNetProcessor::sendRawData(void* data, DWORD len)
{
  if (isTerminate()) return false;
  if (!data || !len) return false;

  if (!m_oSocket.valid()) return false;
  m_oSocket.writeToBuf(data, len);

  addEpoll();
  return true;
}

bool xNetProcessor::recvData()
{
  while (1)
  {
    int ret = m_oSocket.recvData();
    if (ret == 0)
    {
      // 接受完成
      return true;
    }
    else if (ret > 0)
    {
      readCmd();
    }
    else
    {
      return false;
    }
  }

  return false;
}

void xNetProcessor::readCmd()
{
  if(isPlatClient())
  {
    m_oSocket.getPlatCmd(m_oQueue);
  }
  else if (isFluentClient())
  {
    m_oSocket.getFluentCmd(m_oQueue);
  }
  else
  {
    m_oSocket.getCmd(m_oQueue);
  }
}

CmdPair* xNetProcessor::getCmd()
{
  return m_oQueue.get();
}

void xNetProcessor::popCmd()
{
  m_oQueue.erase();
}

void xNetProcessor::terminate(TerminateMethod method, const char *desc)
{
  m_eTerminateMethod = method;
  switch (m_eTerminateMethod)
  {
    case TerminateMethod::terminate_socket_error:
      {
        XLOG_T("[xNetProcessor],%u,%s,socket异常,%s", id, name, desc);
      }
      break;
    case TerminateMethod::terminate_active:
      {
        XLOG_T("[xNetProcessor],%u,%s,task主动关闭,%s", id, name, desc);
      }
      break;
    default:
      XLOG_T("[xNetProcessor],%u,%s,terminate,%s", id, name, desc);
      break;
  }
}

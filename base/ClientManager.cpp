#include "ClientManager.h"
#include "xServer.h"
#include "xNetProcessor.h"
#include "SystemCmd.pb.h"

void ClientProcessor::connect(xServer *pServer)
{
  if (!pServer) return;
  if (m_pTask) return;
//  if (!m_dwPort) return;

  if(ClientType::plat_server == m_eType)
  {
    m_pTask = pServer->newPlatClient();
  }
  else if (ClientType::log_server == m_eType)
  {
    m_pTask = pServer->newFluentClient();
  }
  else
  {
    m_pTask = pServer->newClient();
  }

  m_pTask->setName("");

  if (!m_pTask->connect(m_strIP.c_str(), m_dwPort))
  {
    XERR << "[连接]" << m_strIP << ":" << m_dwPort << "失败," << strerror(errno) << XEND;
    SAFE_DELETE(m_pTask);
    return;
  }

  pServer->getTaskThreadPool().add(m_pTask);
  switch (m_eType)
  {
    case ClientType::log_server:
    case ClientType::plat_server:
      {
      }
      break;
    case ClientType::g_proxy_server:
      {
        Cmd::RegistProxySystemCmd message;
        message.set_proxyid(pServer->s_oOptArgs.m_strProxyID);
        PROTOBUF(message, send, len);
        m_pTask->sendCmd(send, len);
      }
      break;
    default:
      {
        Cmd::RegistRegionSystemCmd message;
        message.set_zoneid(pServer->getZoneID());
        message.set_regiontype((DWORD)m_eType);
        message.set_servertype((DWORD)pServer->getServerType());
        message.set_client(1);
        PROTOBUF(message, send, len);
        m_pTask->sendCmd(send, len);
      }
      break;
  }

  XLOG << "[连接]" << (DWORD)m_eType << "连接成功" << XEND;
}

ClientManager::ClientManager(xServer *p):m_pServer(p)
{
}

ClientManager::~ClientManager()
{
  for (DWORD d = 0; d < CLIENT_TYPE_MAX; ++d)
    SAFE_DELETE(m_list[d].m_pTask);
}

void ClientManager::closeNp(xNetProcessor *np)
{
  if (!np) return;

  for (DWORD i=0; i<CLIENT_TYPE_MAX; ++i)
  {
    if (m_list[i].m_pTask == np)
    {
      m_list[i].m_pTask = nullptr;
      XLOG << "[连接]" << (DWORD)m_list[i].m_eType << "断开连接" << XEND;
      break;
    }
  }
}

void ClientManager::timer()
{
  if (m_pServer->getServerState() >= ServerState::stop) return;
  for (DWORD i=0; i<CLIENT_TYPE_MAX; ++i)
  {
    if (m_list[i].m_blConnect && m_list[i].m_blTask == false && m_list[i].m_pTask == nullptr)
    {
      m_list[i].connect(m_pServer);
    }
  }
}

void ClientManager::add(ClientType type, std::string ip, DWORD port)
{
  if (type >= ClientType::max) return;

  m_list[(DWORD)type].init(type, ip, port);
}

void ClientManager::add(ClientType type, xNetProcessor *np)
{
  if (!np) return;

  m_list[(DWORD)type].m_pTask = np;
  m_list[(DWORD)type].m_eType = type;
}

bool ClientManager::sendCmd(ClientType type, BYTE *buf, DWORD len)
{
  if (type >= ClientType::max) return false;
  if (m_list[(DWORD)type].m_pTask)
  {
    if (type == ClientType::log_server || ClientType::plat_server == type)
    {
      return m_list[(DWORD)type].m_pTask->sendRawData(buf, len);
    }
    else
      return m_list[(DWORD)type].m_pTask->sendCmd(buf, len);
  }
  return false;
}

bool ClientManager::process()
{
  if (!m_pServer)
    return false;
  for (DWORD i = 0; i < CLIENT_TYPE_MAX; ++i)
  {
    if (m_list[i].m_pTask)
    {
      m_pServer->doCmd(m_list[i].m_pTask);
    }
  }
  return true;
}

#include "GProxyServer.h"
#include "xNetProcessor.h"

bool GProxyServer::doCmd(xNetProcessor* np, unsigned char* buf, unsigned short len)
{
  if (!np || !buf || !len || len < sizeof(xCommand)) return false;

  xCommand* cmd = (xCommand*)buf;

  switch (cmd->cmd)
  {
    case Cmd::SYSTEM_PROTOCMD:
      {
        using namespace Cmd;
        switch (cmd->param)
        {
          case INFO_PROXY_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(InfoProxySystemCmd, rev);

              std::string id = rev.proxyid();

              m_oProxyList[id].m_dwTaskNum = rev.tasknum();

              XLOG << "[Proxy]" << "id:" << id << "Task Num:" << rev.tasknum() << XEND;
              return true;
            }
            break;
          case REGIST_PROXY_SYSCMD:
            {
              PARSE_CMD_PROTOBUF(RegistProxySystemCmd, rev);

              if (!inVerifyList(np))
              {
                return true;
              }
              removeVerifyList(np);

              std::string id = rev.proxyid();
              auto it = m_oProxyList.find(id);
              if (it != m_oProxyList.end())
              {
                if (it->second.m_pNetProcessor != nullptr)
                {
                  XERR << "[Proxy注册]" << id << np << "重复添加" << XEND;
                  addCloseList(np, TerminateMethod::terminate_active, "Proxy注册重复添加");
                  return true;
                }
              }

              m_oProxyList[id].m_pNetProcessor = np;
              np->sendCmd(buf, len);
              XLOG << "[Proxy注册]" << id << np << "注册成功" << XEND;

              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }
  return false;
}


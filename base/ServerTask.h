/**
 * @file ServerTask.h
 * @brief 管理server的连接
 * @author liuxin, liuxin@xindong.com
 * @version v1
 * @date 2015-07-22
 */
#pragma once
#include "xNetProcessor.h"

class ZoneServer;

enum class ServerTask_State
{
  notuse  = 0,
  verify  = 1,
  sync    = 2,
  okay    = 3,
  recycle = 4
};

#define SERVER_HEARTBEAT 20

class ServerTask
{
  public:
    ServerTask()
    {
    }
    ~ServerTask(){}

    bool sendCmd(const void *cmd, unsigned short len)
    {
      if (cmd && len && m_pTask)
        return m_pTask->sendCmd(cmd, len);
      return false;
    }

    CmdPair* getCmd()
    {
      if (m_pTask)
        return m_pTask->getCmd();
      return NULL;
    }
    void popCmd()
    {
      if (m_pTask)
        m_pTask->popCmd();
    }
    xNetProcessor *getTask()
    {
      return m_pTask;
    }
    void setTask(xNetProcessor *t)
    {
      m_pTask = t;
      if (m_pTask)
      {
        setState(ServerTask_State::verify);
      }
      else
      {
        setState(ServerTask_State::notuse);
      }
    }
    bool isTask()
    {
      if (m_pTask) return m_pTask->isTask();
      return false;
    }
    bool isClient()
    {
      if (m_pTask) return m_pTask->isClient();
      return false;
    }
    const char* getName()
    {
      if (m_pTask) return m_pTask->name;
      return "";
    }
    bool check()
    {
      m_nHeartbeat--;
      if (m_nHeartbeat <= 0)
        return false;
      return true;
    }
    void reset()
    {
      m_nHeartbeat = SERVER_HEARTBEAT;
    }
    bool isValid() const
    {
      return (ServerTask_State::okay==m_oState) && m_pTask;
    }

    void setState(ServerTask_State s)
    {
      m_oState = s;
    }
    ServerTask_State getState()
    {
      return m_oState;
    }

  private:
    xNetProcessor *m_pTask = nullptr;
    int m_nHeartbeat = SERVER_HEARTBEAT;
    ServerTask_State m_oState = ServerTask_State::notuse;
};

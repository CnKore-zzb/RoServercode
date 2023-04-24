#include "GateServer.h"
#include "GateUserManager.h"
#include "ChatFilterManager.h"

void GateServer::v_timetick()
{
  ZoneServer::v_timetick();

  DWORD cur = now();
  GateUserManager::getMe().process();

  if (m_oOneSecTimer.timeUp(cur))
  {
    GateUserManager::getMe().timer(cur);
    ChatFilterManager::getMe().timer(cur);
    if (m_oTenMinTimer.timeUp(cur))
    {
      thisServer->getMsgCounter().printByCountDesc();
      thisServer->getMsgCounter().printByLenDesc();
      thisServer->getMsgCounter().clear();
    }    
    processGMOverTime(cur);
  }
}

void GateServer::processGMOverTime(DWORD curSec)
{
  for (auto it = m_mapGMCon.begin(); it != m_mapGMCon.end();)
  {
    if (it->second.expiretime <= curSec)
    {
      if (it->second.pNetProcess)
      {
        RetExecGMCmd cmd;
        cmd.set_ret("sessionserver process overtime");
        PROTOBUF(cmd, send, len);
        it->second.pNetProcess->sendCmd(send, len);
      }  
      it = m_mapGMCon.erase(it);
      continue;
    }
    ++it;   
  }
}

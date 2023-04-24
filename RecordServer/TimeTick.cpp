#include "RecordServer.h"
#include "xExecutionTime.h"
#include "RecordUserManager.h"
ExecutionTime_setInterval(300);

void RecordServer::v_timetick()
{
  xTime frameTimer;
  ZoneServer::v_timetick();

  QWORD _e = frameTimer.uElapse();
  if (_e < 2*1000*1000) // 超过2s, 暂不执行定时保存
  {
    DWORD curTime = xTime::getCurSec();
    RecordUserManager::getMe().timetick(curTime);
  }
  else
  {
    DWORD curTime = xTime::getCurSec();
    RecordUserManager::getMe().timetick(curTime, true);
    XLOG << "[Record-帧耗时], 消息处理时间超过2s, 跳过执行定时保存" << XEND;
  }
}

#include "MessageStat.h"
#include "xTools.h"
#include "xServer.h"
#include "MsgManager.h"
#include "GateSuper.pb.h"

MessageStat::MessageStat()
{
}

MessageStat::~MessageStat()
{
}

void MessageStat::start(DWORD cmd, DWORD param)
{
  m_dwMessageKey = (cmd << 16) + param;
  m_oFrameTimer.elapseStart();
}

void MessageStat::end()
{
  if (!m_dwMessageKey) return;

  QWORD cost = m_oFrameTimer.uElapse();
  stMessageStat &stat = m_oStatList[m_dwMessageKey];
  stat.m_qwDuration += cost;
  ++stat.m_dwCount;

  if (stat.m_qwMaxTime < cost)
  {
    stat.m_qwMaxTime = cost;
  }

  m_dwMessageKey = 0;
}

void MessageStat::print()
{
  for (auto &it : m_oStatList)
  {
    if (it.second.m_dwCount == 0) continue;

    QWORD cost = it.second.m_qwDuration/it.second.m_dwCount;

    float min = it.second.m_qwDuration / 1000.0 / 1000.0 / 60.0;

    XLOG << "[消息处理统计],"<< m_strFlag << (it.first >> 16) << "," << (it.first & 0xffff) << ",平均处理时长:" << cost << ",次数:" << it.second.m_dwCount << ",总时长:" << it.second.m_qwDuration << "("<< min << "分钟 ),Max:" << it.second.m_qwMaxTime << XEND;
    if (cost > 100000)
    {
      if (!xServer::isOuter())
      {
        std::stringstream stream;
        stream.str("");
        stream << "[消息处理统计],cmd:" << (it.first >> 16) << ",";
        stream << "param:" << (it.first & 0xffff) << ",";
        stream << "cost:" << cost << ",";
        stream << "count:" << it.second.m_dwCount;
        MsgManager::alter_msg(xServer::getFullName(), stream.str(), EPUSHMSG_MESSAGE_STAT);
      }
    }
  }

  m_oStatList.clear();
}

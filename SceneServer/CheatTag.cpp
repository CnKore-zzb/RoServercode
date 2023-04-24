#include "CheatTag.h"
#include "SceneUser.h"

CheatTag::CheatTag(SceneUser* pUser)
  :m_pUser(pUser)
{}

CheatTag::~CheatTag()
{}

void CheatTag::timer()
{
  save();
}

void CheatTag::save()
{
  if(m_bChangeMark)
  {
    CheatTagRecordCmd cmd;
    cmd.set_charid(m_pUser->id);
    cmd.set_mininterval(m_dwMinInterval);
    cmd.set_frame(m_dwFrame);
    cmd.set_count(m_dwCount);

    PROTOBUF(cmd,send,len);
    thisServer->sendCmdToData(send,len);
    m_bChangeMark = false;
  }
  XDBG << "[反脚本-保存] 保存执行, changemark:" << m_bChangeMark << " interval:" << m_dwMinInterval << " frame:" << m_dwFrame << " count:" << m_dwCount << XEND;
}

void CheatTag::load()
{
  m_bLoading = true;

  CheatTagQueryRecordCmd cmd;
  cmd.set_scenename(thisServer->getServerName());
  cmd.set_charid(m_pUser->id);
  
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToData(send, len);
  XDBG << "[反脚本-加载] 加载执行, interval:" << m_dwMinInterval << " frame:" << m_dwFrame << " count:" << m_dwCount << XEND;
}

void CheatTag::assignData(CheatTagRecordCmd& cmd)
{
  m_dwMinInterval = cmd.mininterval();
  m_dwFrame = cmd.frame();
  m_dwCount = cmd.count();
  
  // 发送数据
  sendCheatTagStat();

  m_bLoading = false;
  flush();
  XDBG << "[反脚本-获取数据] 获取数据执行, interval:" << m_dwMinInterval << " frame:" << m_dwFrame << " count:" << m_dwCount << XEND;
}

void CheatTag::sendCheatTagStat()
{
  CheatTagStatUserCmd cmd;
  cmd.set_count(m_dwCount);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[反脚本-发送数据] 发送数据执行, interval:" << m_dwMinInterval << " frame:" << m_dwFrame << " count:" << m_dwCount << XEND;
}
  
void CheatTag::recCheatTag(CheatTagUserCmd& cmd)
{
  if(m_bLoading)
  {
    m_vecCmdQueue.push_back(cmd);
    return;
  }
  if(cmd.interval()==0)
  {
    XERR << "[反脚本-统计] 操作间隔为0, charid:" << m_pUser->id << XEND;
    return;
  }

  if(m_dwCount == 0 || m_dwMinInterval > cmd.interval())
  {
    m_dwMinInterval = cmd.interval();
    m_dwFrame = cmd.frame();
    XLOG << "[反脚本-统计] 最短间隔改变, charid:" << m_pUser->id << " interval:" << m_dwMinInterval << "ms frame:" << m_dwFrame << XEND;
  }
  m_dwCount++;
  if(!m_bChangeMark)
    m_bChangeMark = true;
  XDBG << "[反脚本-统计] 收到消息, charid:" << m_pUser->id << " interval:" << m_dwMinInterval << "ms frame:" << m_dwFrame << XEND;
}

void CheatTag::flush()
{
  for(auto cmd:m_vecCmdQueue)
  {
    recCheatTag(cmd);
  }
  XDBG << "[反脚本-统计] 处理缓存队列, size:" << m_vecCmdQueue.size() << XEND;
  m_vecCmdQueue.clear();
}

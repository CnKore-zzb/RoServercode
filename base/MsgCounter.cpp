#include "MsgCounter.h"
#include "xLog.h"

static bool cmpCountDesc(MsgItem* a, MsgItem* b)   //count 降序
{
  return a->count > b->count;
}

static bool cmpLenDesc(MsgItem* a, MsgItem* b)   //len 降序
{
  return a->len > b->len;
}


void MsgCounter::add(void *cmd, unsigned short len)
{
  DWORD dwCmd = (DWORD)(*(((BYTE *)cmd)));
  DWORD dwParam = (DWORD)(*(((BYTE *)cmd) + 1));

  DWORD key = dwCmd << 16 | dwParam;

  auto it = m_mapMsgs.find(key);
  if (it == m_mapMsgs.end())
  {
    MsgItem item;
    item.cmd = dwCmd;
    item.param = dwParam;
    item.count = 1;
    item.len = len;
    m_mapMsgs.insert(std::make_pair(key, item));
  }
  else
  {
    it->second.count += 1;
    it->second.len += len;
  }
//  XLOG("[消息统计] add  key:%lu, len:%d, ", key, len);
}

void MsgCounter::clear()
{
  m_mapMsgs.clear();
  //XLOG("[消息统计] 清空消息");
}


void MsgCounter::printByCountDesc()
{
  std::vector<MsgItem*> vecItem;
  vecItem.reserve(m_mapMsgs.size());
  for (auto it = m_mapMsgs.begin(); it != m_mapMsgs.end(); ++it)
  {
    vecItem.push_back(&it->second);
  }
  std::sort(vecItem.begin(), vecItem.end(), cmpCountDesc);

  for (auto it = vecItem.begin(); it != vecItem.end(); ++it)
  {
    if (*it && (*it)->count)
      XLOG << "[消息次数统计],cmd:" << (*it)->cmd << "param:" << (*it)->param << "count:" << (*it)->count << "len:" << (*it)->len << "per:" << (*it)->len/(*it)->count << XEND;
  }
}

void MsgCounter::printByLenDesc()
{
  std::vector<MsgItem*> vecItem;
  vecItem.reserve(m_mapMsgs.size());
  for (auto it = m_mapMsgs.begin(); it != m_mapMsgs.end(); ++it)
  {
    vecItem.push_back(&it->second);
  }
  std::sort(vecItem.begin(), vecItem.end(), cmpLenDesc);

  for (auto it = vecItem.begin(); it != vecItem.end(); ++it)
  {
    if (*it && (*it)->count)
      XLOG << "[消息长度统计],cmd:" << (*it)->cmd << "param:" << (*it)->param << "count:" << (*it)->count << "len:" << (*it)->len << "per:" << (*it)->len/(*it)->count << XEND;
  }
}

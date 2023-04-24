#include "UserTicket.h"
#include "SceneUser.h"

UserTicket::UserTicket(SceneUser* user):m_pUser(user)
{

}

UserTicket::~UserTicket()
{

}

void UserTicket::load(const BlobTicket& data)
{
  for (int i = 0; i < data.tickdata_size(); ++i)
  {
    const BlobTicketData& ticketData = data.tickdata(i);
    m_mapTicketDta[ticketData.guid()] = ticketData;
  }

  for (int i = 0; i < data.cache_cmds_size(); ++i)
  {
    const BlobTicketCacheCmd& rCmd = data.cache_cmds(i);
    m_mapTicketCmd[rCmd.guid()] = rCmd;
  }

  XLOG << "[去重-票据] 加载" << m_pUser->id << "size" << m_mapTicketDta.size() <<"cmdsize"<< m_mapTicketCmd.size() << XEND;
}

void UserTicket::save(BlobTicket* pData)
{
  if (!pData)
    return;
  DWORD offSet = now() - 40 * DAY_T;
  for (auto& v : m_mapTicketDta)
  {
    //40天前的不保存了
    if (v.second.time() < offSet)
    {
      continue;
    }
    BlobTicketData* pTicketData = pData->add_tickdata();
    if (pTicketData)
      pTicketData->CopyFrom(v.second);
  }  

  offSet = now() - 3 * DAY_T;
  for (auto& v : m_mapTicketCmd)
  {
    if (v.second.time() < offSet)
    {
      continue;
    }
    BlobTicketCacheCmd* pCmd = pData->add_cache_cmds();
    if (pCmd)
      pCmd->CopyFrom(v.second);
  }

  XLOG << "[去重-票据] 保存" << m_pUser->id << "size" << m_mapTicketDta.size()<<"cmd size"<< m_mapTicketCmd.size() << XEND;
}

ETicketRet UserTicket::getTicketRet(const string& key)
{
  auto it = m_mapTicketDta.find(key);
  if (it == m_mapTicketDta.end())
    return ETickRet_No;
  
  return it->second.ret();
}

void UserTicket::setTicketRet(ETicketType type, const string&key, ETicketRet ret)
{
  BlobTicketData ticketData;
  ticketData.set_guid(key);
  ticketData.set_ret(ret);
  ticketData.set_time(now());
  ticketData.set_type(type);
  m_mapTicketDta[key] = ticketData;
}

void UserTicket::addTicketCmd(ETicketCmdType type, void* buf, WORD len, QWORD orderId)
{
  string key = getKeyByParam(type, orderId);

  BlobTicketCacheCmd data;
  data.set_type(type);
  data.set_guid(key);
  data.set_data(buf, len);
  data.set_len(len);
  data.set_time(now());
  m_mapTicketCmd[key] = data;
  XDBG << "[去重-票据] 添加cmd" << type << orderId << key << XEND;
}

void UserTicket::delTicektCmd(ETicketCmdType type, QWORD orderId)
{
  string key = getKeyByParam(type, orderId);
  m_mapTicketCmd.erase(key);
  XDBG <<"[去重-票据] 删除cmd" << type << orderId <<key << XEND;
}

void UserTicket::cmdResend(DWORD curSec)
{
  for (auto&m : m_mapTicketCmd)
  {
    if (curSec >= m.second.time() - 5 * 60)
    {
      switch (m.second.type())
      {
      case ETicketCmdType_Auction:
      {
        thisServer->forwardCmdToAuction(m_pUser->id, m_pUser->name, m.second.data().c_str(), m.second.len());
        XLOG << "[去重-票据] 重发cmd" << m.second.type() << m.second.guid() << XEND;
        break;
      }
      default:
        break;
      }
    }
  }
}

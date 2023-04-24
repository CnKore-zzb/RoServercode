#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include <map>
#include <string>
#include <sstream>

using namespace Cmd;
using namespace std;

class SceneUser;
class UserTicket
{
  public:
    UserTicket(SceneUser* user);
    ~UserTicket();

  public:
    void save(BlobTicket* pData);
    void load(const BlobTicket& data);
    
    template<typename T>
    string getKeyByParam(ETicketType type, const T& t1);
    template<typename T1, typename T2>
    string getKeyByParam(ETicketType type, const T1& t1, const T2& t2);    
    ETicketRet getTicketRet(const string& key);
    void setTicketRet(ETicketType type, const string&key, ETicketRet ret);
    
    template<typename T>
    string getKeyByParam(ETicketCmdType type, const T& t1);
    void addTicketCmd(ETicketCmdType type, void* buf, WORD len, QWORD orderId);
    void delTicektCmd(ETicketCmdType type, QWORD orderId);
    void cmdResend(DWORD curSec);
  private:
    SceneUser* m_pUser = nullptr;
    std::map<string/*guid*/, BlobTicketData> m_mapTicketDta;
    std::map<string/*guid*/, BlobTicketCacheCmd> m_mapTicketCmd;
};

template<typename T>
string UserTicket::getKeyByParam(ETicketType type, const T& t1)
{
  return getKeyByParam(type, t1, "null");
}

template<typename T1, typename T2>
string UserTicket::getKeyByParam(ETicketType type, const T1& t1, const T2& t2)
{
  std::stringstream stream;
  stream.str("");
  switch (type)
  {
  case ETicketType_AddGiveItem:
  {
    stream << "AddGiveItem";
  }
  break;
  case ETicketType_AuctionTake:
  {
    stream << "AcutionTake";
  }
  break;
  default:
  {
    return "";
  }
  break;
  }
  stream << ":" << t1 << ":"<<t2;
  return stream.str().c_str();
}

template<typename T1>
string UserTicket::getKeyByParam(ETicketCmdType type, const T1& t1)
{
  std::stringstream stream;
  stream.str("");
  switch (type)
  {
  case ETicketCmdType_Auction:
  {
    stream << "Auction";
  }
  break;
  default:
  {
    return "";
  }
  break;
  }
  stream << ":" << t1;
  return stream.str().c_str();
}


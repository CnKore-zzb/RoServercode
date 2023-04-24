
#pragma once

#include "xSingleton.h"
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "SceneItem.pb.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "ItemConfig.h"
#include "MessageStat.h"
#include "RedisManager.h"


#define MSG_INTERVAL1        1000      //毫秒
#define MSG_INTERVAL2        900      //毫秒

typedef std::pair<DWORD, DWORD> DWORDPAIR;

struct CmpByValue1 {   //从大到小排序
  bool operator()(const DWORDPAIR& lhs, const DWORDPAIR& rhs) {
    return lhs.second > rhs.second;
  }
};

struct CmpByValue2 {   //从小到大排序
  bool operator()(const DWORDPAIR& lhs, const DWORDPAIR& rhs) {
    return lhs.second < rhs.second;
  }
};

struct BriefListCache {
  BriefPendingListRecordTradeCmd cmd;
  DWORD expireTime;
};


class RecordTradeMgr : public xSingleton<RecordTradeMgr>
{
  public:
    RecordTradeMgr();
    ~RecordTradeMgr();
    void init();

    bool doTradeUserCmd(QWORD charId, const BYTE* buf, WORD len);
    inline bool checkCharId(QWORD charId1, QWORD charId2);
    //send
    bool sendCmdToMe(QWORD charId, const BYTE* buf, WORD len);

    //获取热门列表
    void fetchHotPendingList(QWORD charId, Cmd::HotItemidRecordTrade& rev);
    void getHotId(DWORD dwJob, DWORD dwShowType,DWORD dwCount, std::set<DWORD>& resultSet);
    //获取可购买列表
    void fetchAllBriefPendingList(QWORD charId, Cmd::BriefPendingListRecordTradeCmd& rev);
    //获取可购买列表
    void fetchDetailPendingList(QWORD charId, Cmd::DetailPendingListRecordTradeCmd& rev);
    //获取可购买列表
    void fetchItemSellInfoList(QWORD charId,  Cmd::ItemSellInfoRecordTradeCmd& rev);
    //获取可购买列表
    void fetchMyPendingList(QWORD charId,  Cmd::MyPendingListRecordTradeCmd& rev);
    //获取可购买列表
    //void fetchMyTradeLogList(QWORD charId,  Cmd::MyTradeLogRecordTradeCmd& rev);

  private:
    bool canMerge( Cmd::EOperType type, DWORD itemId, DWORD price, DWORD time);
    std::vector<LogItemInfo> m_vecItemInfo;
    std::vector<LogItemInfo>::reverse_iterator m_lastIter ;

    std::map<QWORD, BriefListCache> m_mapBriefListCache;

public:
  MessageStat m_oMessageStat;
};

class TradeMsgGuard : public xSingleton<TradeMsgGuard>
{
public:
  bool lock(DWORD type, QWORD charId);

  bool unlock(DWORD type, QWORD charId);
private:  
  std::unordered_map<DWORD/*key*/, std::unordered_map<QWORD/*charId*/, QWORD/*time*/>> m_mapGuard;
};

class MessageStatHelper
{
public:
  MessageStatHelper(DWORD cmd, DWORD param);
  ~MessageStatHelper();
 
};





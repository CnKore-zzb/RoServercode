/**
 * @file SessionChatManager_SE.h
 * @brief handle chat message
 * @author tianyiheng, tianyiheng@xindong.com
 * @version v1
 * @data 2015-09-07
 */

#pragma once

#include "xSingleton.h"
#include "ChatCmd.pb.h"
#include "SessionCmd.pb.h"
#include "xDBFields.h"
#include "WeddingCCmd.pb.h"

using namespace Cmd;
using std::map;
using std::string;
using std::vector;
using std::set;

class SessionUser;
class xNetProcessor;

const DWORD QUERY_ZONE_TICK = 300;

enum
{
  OFFLINE_MSG_LOAG_TYPE_NULL  = 0,
  OFFLINE_MSG_LOAG_TYPE_ONE   = 1,
  OFFLINE_MSG_LOAG_TYPE_TWO   = 2,
  OFFLINE_MSG_LOAG_TYPE_THREE = 3,
};

// offline msg
class OfflineMsgBase
{
public:
  OfflineMsgBase();
  virtual ~OfflineMsgBase();

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);

  virtual EOfflineMsg getType() const = 0;

  virtual bool send(SessionUser* pUser) = 0;

  QWORD getId() const { return m_qwId; }
  virtual void getId(TSetQWORD& rSetId) const { rSetId.insert(m_qwId); }
  void setId(QWORD id) { m_qwId = id; }

  void setTargetID(QWORD qwTargetID) { m_qwTargetID = qwTargetID; }
  QWORD getTargetID() const { return m_qwTargetID; }

  void setSenderID(QWORD qwSenderID) { m_qwSenderID = qwSenderID; }
  QWORD getSenderID() const { return m_qwSenderID; }

  protected:
    QWORD m_qwTargetID = 0;
    QWORD m_qwSenderID = 0;
    DWORD m_dwTime = 0;

    QWORD m_qwId = 0;       // 数据库里对应的id
    bool m_isNew = true;    // 默认是true
};

// offline msg chat
class OfflineMsgUser : public OfflineMsgBase
{
  public:
    OfflineMsgUser();
    virtual ~OfflineMsgUser();

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const { return EOFFLINEMSG_USER; }

    virtual bool send(SessionUser* pUser);

    void init(const ChatRetCmd& cmd) { m_oCmd.CopyFrom(cmd); }
  private:
    ChatRetCmd m_oCmd;
};

// offline msg chat
class OfflineMsgSys2 : public OfflineMsgBase
{
public:
  OfflineMsgSys2();
  virtual ~OfflineMsgSys2();

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);

  virtual EOfflineMsg getType() const { return EOFFLINEMSG_SYS2; }

  virtual bool send(SessionUser* pUser);

  void init(const SysMsg& cmd) { m_oCmd.CopyFrom(cmd); }
private:
  SysMsg m_oCmd;
};

// offline msg trade
class OfflineMsgTrade : public OfflineMsgBase
{
  public:
    OfflineMsgTrade();
    virtual ~OfflineMsgTrade();

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const { return EOFFLINEMSG_TRADE; }

    virtual bool send(SessionUser* pUser);

    void insertId(QWORD qwId) { m_setId.insert(qwId); }
    virtual void getId(TSetQWORD& rSetId) const { for (auto &v : m_setId) { rSetId.insert(v); } }
    void setItemID(DWORD dwItemID) { m_dwItemId = dwItemID; }
    void setPrice(DWORD dwPrice) { m_dwPrice = dwPrice; }
    void addCount(DWORD dwCount) { m_dwCount += dwCount; }
    void addGiveMoney(DWORD dwGiveMoney) { m_dwGiveMoney += dwGiveMoney; }
    void setMoneyType(EMoneyType eType) { m_eMoneyType = eType; }
  private:
    TSetQWORD m_setId;
    DWORD m_dwItemId = 0;
    DWORD m_dwPrice = 0;
    DWORD m_dwCount = 0;
    DWORD m_dwGiveMoney = 0;
    EMoneyType m_eMoneyType = EMONEYTYPE_MIN;
};

// offline msg sys
class OfflineMsgSys : public OfflineMsgBase
{
  public:
    OfflineMsgSys();
    virtual ~OfflineMsgSys();

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const { return EOFFLINEMSG_SYS; }

    virtual bool send(SessionUser* pUser);

    void setMsgID(DWORD dwMsgID) { m_dwMsgID = dwMsgID; }
    void setStr(const string& str) { m_str = str; }
  private:
    DWORD m_dwMsgID = 0;

    string m_str;
};

// offline gm msg
class OfflineMsgGM : public OfflineMsgBase
{
  public:
    OfflineMsgGM();
    virtual ~OfflineMsgGM() {}

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const { return EOFFLINEMSG_GM; }

    virtual bool send(SessionUser* pUser);

    void setGM(const std::string& strGM) { m_strGmcmd = strGM; }
  private:
    std::string m_strGmcmd;
};

// additem
class OfflineMsgAddItem : public OfflineMsgBase
{
  public:
    virtual ~OfflineMsgAddItem() {}

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const {return EOFFLINEMSG_ADD_ITEM; }

    virtual bool send(SessionUser* pUser);

    void init(const ItemData& data) { m_data.CopyFrom(data); }
  private:
    ItemData m_data;
};

// relation msg
class OfflineMsgRelation : public OfflineMsgBase
{
  public:
    OfflineMsgRelation();
    virtual ~OfflineMsgRelation() {}

    virtual bool fromData(const xRecord& rRecord);
    virtual bool toData(xRecord& rRecord);

    virtual EOfflineMsg getType() const { return m_eType; }

    virtual bool send(SessionUser* pUser);

    void setRelation(DWORD dwRelation) { m_dwRelation = dwRelation; }
    void setType(EOfflineMsg eType) { m_eType = eType; }
  private:
    EOfflineMsg m_eType = EOFFLINEMSG_MIN;
    DWORD m_dwRelation = 0;
};

// tutor reward
class OfflineMsgTutorReward : public OfflineMsgBase
{
public:
  OfflineMsgTutorReward();
  virtual ~OfflineMsgTutorReward() {}

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);
  virtual EOfflineMsg getType() const { return EOFFLINEMSG_TUTOR_REWARD; }
  virtual bool send(SessionUser* pUser);

  bool saveToDB();
  void addReward(const TutorReward& reward);
private:
  map<QWORD, TutorReward> m_mapRewards;
};

// user add item
class OfflineMsgUserAddItem : public OfflineMsgBase
{
public:
  OfflineMsgUserAddItem();
  virtual ~OfflineMsgUserAddItem() {}

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);
  virtual EOfflineMsg getType() const { return EOFFLINEMSG_USER_ADD_ITEM; }
  virtual bool send(SessionUser* pUser);
  void init(const OffMsgUserAddItem& data) { m_oData.CopyFrom(data); }
private:
  OffMsgUserAddItem m_oData;
};


// offline msg wedding
class OfflineMsgWedding : public OfflineMsgBase
{
public:
  OfflineMsgWedding();
  virtual ~OfflineMsgWedding();

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);

  virtual EOfflineMsg getType() const { return EOFFLINEMSG_WEDDING; }

  virtual bool send(SessionUser* pUser);

  void init(const WeddingEventMsgCCmd& cmd) { m_oCmd.CopyFrom(cmd); }
private:
  WeddingEventMsgCCmd m_oCmd;
};

// offline msg quota
class OfflineMsgQuota : public OfflineMsgBase
{
public:
  OfflineMsgQuota();
  virtual ~OfflineMsgQuota();

  virtual bool fromData(const xRecord& rRecord);
  virtual bool toData(xRecord& rRecord);

  virtual EOfflineMsg getType() const { return EOFFLINEMSG_USER_QUOTA; }

  virtual bool send(SessionUser* pUser);

  void init(const Cmd::OffMsgUserQuotaData& data) { m_oData.CopyFrom(data); }
private:
  Cmd::OffMsgUserQuotaData m_oData;
};

typedef vector<OfflineMsgBase*> TVecOfflineMsg;
typedef map<QWORD, TVecOfflineMsg> TMapOfflineMsg;
typedef map<QWORD, OfflineMsgBase*> TMapCharid2OfflineMsg;

// chat manager
class ChatManager_SE : public xSingleton<ChatManager_SE>
{
  public:
    ChatManager_SE();
    virtual ~ChatManager_SE();

    void onUserOnline(SessionUser* pUser);
    void final();

    void addOfflineMsg(QWORD qwCharID, const ItemData& cmd);
    void addOfflineMsg(QWORD qwCharID, const ChatRetCmd& cmd);
    void addOfflineMsg(QWORD qwCharID, const SysMsg& cmd);
    void addOfflineMsg(QWORD destUser, DWORD itemId, DWORD price, DWORD count, DWORD giveMoney, EMoneyType moneyType);
    void addOfflineMsg(QWORD qwTargetID, DWORD dwMsgID, const string& str = "");
    void addOfflineMsg(QWORD qwTargetID, QWORD qwDestID, EOfflineMsg eType, DWORD dwRelation);
    void addOfflineGmMsg(QWORD qwTargetID, const std::string& gmcmd);
    void addOfflineMsg(QWORD qwTargetID, const TutorReward& reward);
    void addOfflineMsg(QWORD qwTargetID, const OffMsgUserAddItem& cmd);
    void addOfflineMsg(QWORD qwTargetID, const WeddingEventMsgCCmd& cmd);
    void addOfflineMsg(QWORD qwTargetID, const OffMsgUserQuotaData& data);

    bool doChatCmd(SessionUser* pUser, const BYTE* buf, WORD len);

    bool queryZoneStatus(DWORD dwNow, xNetProcessor* pTask = nullptr);
    void timer(DWORD curTime);
    bool reLoadDbByType(SessionUser* pUser, EOfflineMsg eType);

  private:
    OfflineMsgBase* createOfflineMsg(EOfflineMsg eType);

    void insertDb(OfflineMsgBase* pMsgBase);
  public:
    bool preLoadDb(QWORD targetId, TVecDWORD& typeVec, DWORD type);
    bool loadDb(QWORD targetId, TVecOfflineMsg& resVec, xRecordSet &set);
    void delDb(const TSetQWORD&/*id*/ setId);
  private:
    void chatLog(QWORD sID, QWORD sAccID, DWORD sPlat, DWORD sZone, DWORD sLv, QWORD rID, QWORD rAccID, const string rname, const ChatRetCmd& cmd);

  private:
    QueryZoneStatusSessionCmd m_oZoneCmd;
    DWORD m_dwZoneTime = 0;

  public:
    void sendAndDelMsg(SessionUser* pUser);
    void eraseMapCharID2Msgs(QWORD charid);
    void addMapCharID2Msgs(QWORD charid, TVecOfflineMsg &vec);
  private:
    map<QWORD, TVecOfflineMsg> m_mapCharID2Msgs;
};

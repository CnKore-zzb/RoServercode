/**
 * @file MailManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-05-21
 */

#pragma once

#include "xDefine.h"
#include "xLog.h"
#include "xSingleton.h"
#include "SessionCmd.pb.h"
#include "SocialCmd.pb.h"
#include "SceneTrade.pb.h"
#include "MsgManager.h"

using namespace Cmd;
using std::map;
using std::string;

const DWORD MAIL_INVALID_CHECK_TICK = 120;
const DWORD MAIL_DELETE_LIMIT_COUNT = 500;

typedef map<QWORD, MailData> TMapMailData;

class MailManager : public xSingleton<MailManager>
{
  friend class xSingleton<MailManager>;
  private:
    MailManager();
  public:
    virtual ~MailManager();

    const TMapMailData& getSystemMailList() const { return m_mapSystemMail; }
    bool addSysMail(QWORD qwMailID, const MailData& rData);
    const MailData* getSysMail(QWORD qwMailID) const;

    bool loadAllSystemMail();
    bool sendMail(QWORD qwTargetID, DWORD dwMailID, MsgParams params = MsgParams());
    bool sendMail(QWORD qwTargetID, QWORD qwSenderID, const string& strSender, const string& strTitle, const string& strMsg,
        EMailType eType = EMAILTYPE_NORMAL, DWORD dwMailID = 0, const TVecItemInfo& vecItemInfo = TVecItemInfo{}, const TVecItemData& vecItemDatas = TVecItemData{},
        EMailAttachType attachType=EMAILATTACHTYPE_ITEM, bool bAccMail = false, MsgParams params = MsgParams(),  DWORD startTime=0, DWORD endTime=0, DWORD chargeMoney = 0, bool bSplit = true, QWORD qwQuota =0);
    bool sendMail(QWORD qwTargetID, DWORD dwMailID, const TVecItemInfo& vecItemInfo, MsgParams params = MsgParams(), bool bAccMail = false, bool bSplit = true);
    bool sendMail(QWORD qwTargetID, DWORD dwMailID, const TVecItemData& vecItemData, MsgParams params = MsgParams(), bool bAccMail = false, bool bSplit = true, EMailType eType = EMAILTYPE_NORMAL);

    bool insertNormalMailToDB(MailData* pData);
    bool insertSysMailToDB(MailData* pData);

    bool addOfflineTradeItem(Cmd::AddItemRecordTradeCmd& rev);
    bool addOfflineTradeMoney(Cmd::AddMoneyRecordTradeCmd& rev);
    bool sendChargeMail(const Cmd::ChargeSessionCmd& rev, QWORD qwQuota, bool bVirgin = false);
    void checkInvalidMail(DWORD curTime);
  public:
    bool sendMail(SendMail& cmd);
    bool sendWeddingEventMsgMail(QWORD qwTargetID, const Cmd::WeddingEventMsgCCmd& cmd);
    bool addEventMail(QWORD qwTargetID, const UserEventMailCmd& cmd);
    void addMailMsg(Cmd::SendMail &cmd, MsgParams params = MsgParams());
    void translateMailMsg(MailData* pData, DWORD dwLanguage);
  private:
    TMapMailData m_mapSystemMail;

    DWORD m_dwInvalidCheckTick = 0;
};


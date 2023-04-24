/**
 * @file Mail.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-10
 */

#pragma once

#include "MailManager.h"

using std::map;
using std::set;

// mail
class SessionUser;
class xRecordSet;
typedef std::map<QWORD/*id*/, DWORD/*expiretime*/> TMapId2ExpireTime;

class Mail
{
  public:
    Mail(SessionUser* pUser);
    ~Mail();

    void onUserOnline();
    void onUserOffline();

    bool getAttach(QWORD id, bool bAuto = false, bool fromWedding = false, bool weddingValid = false);
    bool processGetAttach(const GetMailAttachSessionCmd& cmd);
    void timer(DWORD curTime);

    void updateMail();
    void syncGingerToScene();
    void syncGingerToScene(TMapId2ExpireTime& rMap);
    void loadAllMail(xRecordSet &set);
  private:
    void sendAllMail();
    void giveTimer(DWORD curTime);
  public:
    bool checkSystemMail();
    bool addMail(const MailData& rData, bool addDb = true);
    bool processWeddingMsg();
    bool processEventMail();
  private:
    bool processTradeMail();
    void processLotterGiveMail();
    MailData* getMailData(QWORD id);
    void addLotteryMail(QWORD id, DWORD createTime, bool bSync2Scene);
    void delLotteryMail(QWORD id);
    bool processWeddingMsg(const MailData& rData);
    bool processEventMail(const MailData& rData);
    void processMailLanguage();
    bool isMailValid(QWORD mailid);

  private:
    SessionUser* m_pUser = nullptr;

    TMapMailData m_mapMail;
    TMapMailData m_mapTradeMail;

    TSetQWORD m_setSysMailIDs;
    TSetQWORD m_setUpdateIDs;
    QWORD m_qwLastMailID = 0;
    TMapId2ExpireTime m_mapLotterGive;   //Å¤µ°ÔùËÍ
    DWORD m_dwNextTickTime = 0;
    TMapMailData m_mapWeddingEventMail;
    TMapMailData m_mapUserEventMail;
};


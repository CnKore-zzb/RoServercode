#pragma once

#include "xSingleton.h"
#include "SceneAugury.pb.h"
#include <list>
#include <unordered_map>

class SceneUser;
struct AuguryUser
{
  DWORD answerId = 0;
  QWORD qwUserId = 0;
};

enum EAuguryState
{
  EAuguryState_None = 1,
  EAuguryState_Create = 2,
  EAuguryState_Starting = 3,
  EAuguryState_End = 3,
};

class Augury
{
  friend class AuguryMgr;
public:
  Augury(Cmd::EAuguryType type):m_type(type)
  {
  }

  bool start();
  void sendTitle();
  void chat(QWORD senderId, const Cmd::AuguryChat& rev);
  void answer(SceneUser* pUser, const Cmd::AuguryAnswer& rev);
  void addUser(SceneUser* pUser);
  bool quit(SceneUser* pUser);  
  QWORD getUid() { return m_uid; }

private:
  void broadcastCmd(const void * cmd, DWORD len, QWORD except);
  AuguryUser* getAuguryUser(QWORD charId);

  bool checkNext();
  DWORD getStarMonth(DWORD curSec);
private:
  Cmd::EAuguryType m_type;
  EAuguryState m_state = EAuguryState_Create;
  QWORD m_uid = 0;
  DWORD m_dwAnswerCount = 0;    //回答的人个数
  DWORD m_curTitleId = 0;
  std::vector<AuguryUser> m_auguryUsers;
  DWORD m_dwMonth = 0;
};

class AuguryMgr : public xSingleton<AuguryMgr>
{
  friend class xSingleton<AuguryMgr>;
  private:
    AuguryMgr();
  public:
    virtual ~AuguryMgr();

    Augury* create(SceneUser* pUser, Cmd::EAuguryType type);  //创建
    bool enter(SceneUser* pUser, QWORD uid);   //进入
    bool quit(SceneUser* pUser);    //退出
    
    QWORD getUid(SceneUser* pUser);
    Augury* getAugury(SceneUser* pUser);
    
    bool useFreeCount(SceneUser* pUser, bool justCheck);
    bool useExtraCount(SceneUser* pUser, bool isInviter, bool justCheck);

  private:
    std::unordered_map<QWORD/*uid*/, Augury> m_mapAugury;
    std::unordered_map<QWORD/*charid*/, QWORD/*uid*/> m_mapCharidUid;
    QWORD m_uid = 0;
};


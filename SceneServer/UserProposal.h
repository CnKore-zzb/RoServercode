#pragma once
#include "xNoncopyable.h"
#include "xDefine.h"
#include "xTime.h"
#include "SceneUser2.pb.h"
#include "xCmd.pb.h"

class SceneUser;
struct xPos;

class UserProposal : private xNoncopyable
{
  public:
    UserProposal(SceneUser *user);
    ~UserProposal();

    // 待办： 添加求婚成功后的动作
    // 待办： 添加非玩具戒指求婚成功后的buff和特效
    // 待办： 检查前端和后端的消息验证
  private:
    enum {
      ACTIONID_FAKEDEAD = 40,
      ACTIONID_KNEE = 95, //临时值
      // Knee = 58, //策划未配置完毕
      // 求婚成功动作 求婚者
      // 求婚成功动作 被求婚者
    };
    enum{
      EMOJIID_HEART = 2,
      EMOJIID_SORRY = 16,
    };
    //播放特效未完成

  public:
    bool popTheQuest(QWORD targetID, DWORD itemID);
    void doReplyCmd(const Cmd::MarriageProposalReplyCmd& cmd);
    inline void clear() { setTargetID(0); setStartTime(0);}
    inline bool isHoldProposal() { return (m_qwTargetID != 0) && !isTimeOver(); }
    inline bool isTimeOver() { return xTime::getCurMSec() > m_qwStartTime + m_qwWaitTime; }
    inline QWORD getTargetID() { return m_qwTargetID; }
    inline QWORD getStartTime() { return m_qwStartTime; }
    inline DWORD getItemID() { return m_dwItemID; }
    bool canBeAttack(bool isMVP);
    inline bool isToyRing() { return m_bIsToyRing; }
    const TSetDWORD& getBuffs() { return m_setBuffIDs; }

  private:
    bool loadMiscCFG();
    bool loadItemCFG(DWORD itemID);
    inline void setTargetID(QWORD uid) { m_qwTargetID = uid; }
    inline void setStartTime(QWORD curMSec) { m_qwStartTime = curMSec; }
    inline void setItemID(DWORD itemID) { m_dwItemID = itemID; }
    void breakHandAndFollow(QWORD uid);
    void doKnee();
    void doSorry(QWORD uid);
    void doFakeDead(QWORD uid);
    void doAction(QWORD uid,DWORD actionid);
    bool consumeItem();
    void monsterShowLove();
    void sendProposalCmd();

  private:
    SceneUser* m_pUser = nullptr;
    QWORD m_qwTargetID = 0;
    QWORD m_qwStartTime = 0;

    QWORD m_qwWaitTime = 0;

    DWORD m_dwItemID = 0;
    bool m_bIsToyRing = false;
    DWORD m_dwMaxRange = 0;
    TSetDWORD m_setBuffIDs;
};

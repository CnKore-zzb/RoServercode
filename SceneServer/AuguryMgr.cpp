#include "AuguryMgr.h"
#include "ValentineConfig.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "MiscConfig.h"
#include "MsgManager.h"
#include "ActivityManager.h"

void Augury::addUser(SceneUser* pUser)
{
  if (!pUser)
    return;
  AuguryUser user;
  user.answerId = 0;
  user.qwUserId = pUser->id;
  m_auguryUsers.push_back(user);
}

bool Augury::start()
{
  m_state = EAuguryState_Starting;
  
  m_dwMonth = getStarMonth(now());
    
  const SValentineCFG* pCfg = ValentineConfig::getMe().getFirstTitleCfg(m_type, m_dwMonth);
  if (pCfg == nullptr)
  {
    XERR << "[占卜] 找不到第一题 type" << m_type  << m_uid << XEND;
    return false;
  }
  
  m_curTitleId = pCfg->m_dwId;
  
  XLOG << "[占卜-开始]  当前题目 type" << m_type <<"月份" << m_dwMonth << m_curTitleId<<"uid"<<m_uid << XEND;
  sendTitle();
  return true;
}

void Augury::sendTitle()
{
  //clear
  m_dwAnswerCount = 0;
  for (auto &v : m_auguryUsers)
  {
    v.answerId = 0;
  }

  AuguryTitle cmd;
  cmd.set_titleid(m_curTitleId);
  cmd.set_type(m_type);
  cmd.set_subtableid(m_dwMonth);
  PROTOBUF(cmd, send, len);
  broadcastCmd(send, len, 0);
  XLOG << "[占卜-发布题目]  当前题目 type" <<"月份" << m_dwMonth << m_type << m_curTitleId <<XEND;
}

void Augury::chat(QWORD senderId, const Cmd::AuguryChat& rev)
{
  PROTOBUF(rev, send, len);
  broadcastCmd(send, len, 0);
}

void Augury::answer(SceneUser* pUser, const Cmd::AuguryAnswer& rev)
{
  if (!pUser)
    return;
  if (m_curTitleId != rev.titleid())
    return;
  
  AuguryUser* pAuguryUser = getAuguryUser(pUser->id);
  if (!pAuguryUser)
  {
    return;
  }
  if (pAuguryUser->answerId != 0)
    return;

  pAuguryUser->answerId = rev.answer();
  
  m_dwAnswerCount++;
  
  PROTOBUF(rev, send, len);
  broadcastCmd(send, len, 0);

  //check next
  checkNext();
}

bool Augury::checkNext()
{
  if (m_auguryUsers.size() < 2)
    return false ;

  if (m_dwAnswerCount != m_auguryUsers.size())
    return false;

  //next title
  const SValentineCFG* pCfg = ValentineConfig::getMe().getCfg(m_type, m_dwMonth, m_curTitleId);
  if (!pCfg)
    return false;
  const SValentineCFG* pNextCfg = pCfg->getNextOption(m_auguryUsers[0].answerId, m_auguryUsers[1].answerId);
  if (pNextCfg == nullptr)
  {
    XERR << "[占卜] 找不到下一题， 当前题目 type" << m_type << "月份" << m_dwMonth << m_curTitleId<<"uid"<<m_uid << "答案1" << m_auguryUsers[0].answerId << "答案2" << m_auguryUsers[1].answerId << XEND;
    return false;
  }

  XLOG << "[占卜] 进入下一题，当前题目 type" << m_type << "月份" << m_dwMonth << m_curTitleId << "uid" << m_uid <<  "答案1" << m_auguryUsers[0].answerId << "答案2" << m_auguryUsers[1].answerId <<"下一题"<<pNextCfg->m_dwId << XEND;

  m_curTitleId = pNextCfg->m_dwId;

  sendTitle();

  if (pNextCfg->isEnd())
  {
    //reward
    TVecItemInfo vecReward;

    bool isOld = m_type == EAUGURYTYPE_VALENTINE;

    SceneUser* pInviter = SceneUserManager::getMe().getUserByID(m_auguryUsers[0].qwUserId);
    if (pInviter == nullptr)
      return false;
    bool bExtra = false;

    for (auto&v2 : m_auguryUsers)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(v2.qwUserId);
      if (!pUser)
        continue;

      if(m_type == EAUGURYTYPE_ACTIVITY)
      {
        DWORD dwJoy = MiscConfig::getMe().getJoyLimitCFG().dwAuguryAdd;
        pUser->getUserSceneData().addJoyValue(JOY_ACTIVITY_AUGURY, dwJoy);
      }
      pUser->getServant().onFinishEvent(ETRIGGER_AUGURY);

      if (!isOld)
      {
        if (pUser == pInviter)
        {
          if (!AuguryMgr::getMe().useExtraCount(pUser, true, false))
          {
            if (!AuguryMgr::getMe().useFreeCount(pUser, false))
            {
              MsgManager::sendMsg(pUser->id, 937);
              XLOG << "[占卜-奖励] 邀请者使用免费次数失败,不给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
              continue;
            }
            XLOG << "[占卜-奖励] 邀请者使用免费次数成功，给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
          }
          else
          {
            bExtra = true;
            XLOG << "[占卜-奖励] 邀请者使用额外次数成功，给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
          }
        }
        else
        {
          //邀请者使用道具邀请，优先扣除被邀请者的额外次数，不满足再去扣除免费次数。
          if (bExtra)
          {
            if (!AuguryMgr::getMe().useExtraCount(pUser, false, false))
            {
              if (!AuguryMgr::getMe().useFreeCount(pUser, false))
              {
                XLOG << "[占卜-奖励] 邀请者使用额外次数，被邀请者使用免费次数失败，不给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
                MsgManager::sendMsg(pUser->id, 937);
                continue;
              }
              XLOG << "[占卜-奖励] 邀请者使用额外次数，被邀请者使用免费次数成功，给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
            }
            else 
              XLOG << "[占卜-奖励] 邀请者使用额外次数，被邀请者使用额外次数成功，给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
          }
          else if (!AuguryMgr::getMe().useFreeCount(pUser, false))
          {
            MsgManager::sendMsg(pUser->id, 937);
            XLOG << "[占卜-奖励] 邀请者使用免费次数，被邀请者使用免费次数失败，不给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
            continue;
          }
          else
            XLOG << "[占卜-奖励] 邀请者使用免费次数，被邀请者使用免费次数成功，给奖励 type" << m_type << m_uid << "charid" << pUser->id << "邀请人" << pInviter->id << XEND;
        }             
      }

      for (auto &v : pNextCfg->m_vecReward)
      {
        vecReward.clear();
        if (RewardManager::roll(v, pUser, vecReward, ESOURCE_OPERATE) == true)
        {
          pUser->getPackage().addItem(vecReward, EPACKMETHOD_AVAILABLE);
          XLOG << "[占卜] 给奖励成功 type" << m_type << m_uid << "charid" << pUser->id << "rewarid" << v << XEND;
        }
      }
      //AuguryMgr::getMe().getCelebrationReward(v2.pUser);
      pUser->getActivityReward();
    }
    XLOG << "[占卜] 最后一题,给奖励 type"<< m_type << "uid" << m_uid << m_curTitleId << XEND;
  }

  return true;
}

void Augury::broadcastCmd(const void * cmd, DWORD len, QWORD except)
{
  for (auto it = m_auguryUsers.begin(); it != m_auguryUsers.end(); ++it)
  {
    if (it->qwUserId && it->qwUserId != except)
    {
      SceneUser* pUser = SceneUserManager::getMe().getUserByID(it->qwUserId);
      if (pUser)
      {
        pUser->sendCmdToMe(cmd, len);
    }
  }
  }
}

AuguryUser* Augury::getAuguryUser(QWORD charId)
{
  for (auto it = m_auguryUsers.begin(); it != m_auguryUsers.end(); ++it)
  {
    if (it->qwUserId == charId)
      return &(*it);
  }
  return nullptr;
}

bool Augury::quit(SceneUser* pUser)
{
  if (!pUser)
    return false;
  AuguryUser* pAuguryUser = getAuguryUser(pUser->id);
  if (!pAuguryUser)
    return false;
  
  AuguryQuit cmd;
  PROTOBUF(cmd, send, len);
  broadcastCmd(send, len, 0);
  XLOG << "[占卜-退出房间] 当前题目" << m_curTitleId << "uid" << m_uid << XEND;
  return true;
}

DWORD Augury::getStarMonth(DWORD curSec)
{
  DWORD month = xTime::getMonth(curSec);
  DWORD day = xTime::getDay(curSec);

  const std::map<DWORD/*monty*100+day*/, DWORD/*month*/> sMapStartDay = {
    { 120, 1},
    { 219,2 },
    { 321,3 },
    { 420,4 },
    { 521,5 },
    { 622,6 },
    { 723,7 },
    { 823,8 },
    { 923,9 },
    { 1024,10 },
    { 1123,11 },
    { 1222,12 },
  };
  
  DWORD monthDay = month * 100 + day;
  DWORD last = 12;
  for (auto&m : sMapStartDay)
  {
    if (monthDay < m.first)
      break;
    last = m.second;
  }
  XDBG << "[占卜-星座] 根据月份获取星座开始月 month" << month << "day" << day << "result" << last << XEND;
  return last;
}
////
AuguryMgr::AuguryMgr()
{
  m_uid = now();
}

AuguryMgr::~AuguryMgr()
{

}

Augury* AuguryMgr::create(SceneUser* pUser, Cmd::EAuguryType type)
{
  if (!pUser)
    return nullptr;

  Augury* pAugury = getAugury(pUser);
  if (pAugury)
  {
    XERR << "[占卜-创建房间] 房间已经存在,创建者" << pUser->id << "房间id" << pAugury->getUid() << XEND;
    return nullptr;
  }

  m_uid++;
  Augury augury(type);
  augury.m_uid = m_uid;
  augury.addUser(pUser);
  auto it = m_mapAugury.insert(std::make_pair(m_uid, augury)).first;
  if (it == m_mapAugury.end())
    return nullptr;
  m_mapCharidUid[pUser->id] = m_uid;
  return &(it->second);
}

bool AuguryMgr::enter(SceneUser* pUser, QWORD uid)
{
  if (!pUser)
    return false;

  //
  auto it = m_mapAugury.find(uid);
  if (it == m_mapAugury.end())
    return false;
  
  //重复进入
  auto it2 = m_mapCharidUid.find(pUser->id);
  if (it2 != m_mapCharidUid.end())
    return false;
  
  m_mapCharidUid[pUser->id] = uid;

  Augury& rAugury = it->second;
  rAugury.addUser(pUser);
  return true;
}

bool AuguryMgr::quit(SceneUser* pUser)
{
  Augury* pAugury = getAugury(pUser);
  if (pAugury == nullptr)
    return false;

  if (pAugury->quit(pUser) == false)
    return false;

  //del 
  for (auto & v : pAugury->m_auguryUsers)
  {
    m_mapCharidUid.erase(v.qwUserId);
  }
  
  m_mapAugury.erase(pAugury->m_uid);
  
  XLOG << "[占卜] 删除占卜房间" <<pUser->id <<pUser->name <<"uid"<< pAugury->m_uid << XEND;
  return true;
}

QWORD AuguryMgr::getUid(SceneUser* pUser)
{
  if (!pUser)
    return 0;
  auto it = m_mapCharidUid.find(pUser->id);
  if (it == m_mapCharidUid.end())
    return 0;

  return it->second;
}

Augury* AuguryMgr::getAugury(SceneUser* pUser)
{
  if (!pUser)
    return nullptr;

  QWORD uid = getUid(pUser);
  if (uid == 0)
    return nullptr;
  
  auto it = m_mapAugury.find(uid);
  if (it == m_mapAugury.end())
    return nullptr;
  
  return &(it->second);
}

bool AuguryMgr::useFreeCount(SceneUser* pUser, bool justCheck)
{
  if (pUser == nullptr)
    return false;
  DWORD cnt = pUser->getVar().getVarValue(EVARTYPE_NEWAUGURY_REWARD);
  if (cnt >= MiscConfig::getMe().getAuguryCfg().dwMaxRewardCount)
    return false;

  if (justCheck)
    return true;

  //使用免费次数
  cnt++;
  pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_REWARD, cnt);
  return true;
}

bool AuguryMgr::useExtraCount(SceneUser* pUser, bool isInviter, bool justCheck)
{
  if (pUser == nullptr)
    return false;
  DWORD cnt = pUser->getVar().getVarValue(EVARTYPE_NEWAUGURY_EXTRACOUNT);
  if (cnt >= 1)
    return false;

  //只扣除邀请者的道具
  MainPackage* pPackage = nullptr;
  DWORD itemId = 0;

  if (isInviter)
  {
    pPackage = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
    itemId = MiscConfig::getMe().getAuguryCfg().dwExtraItemId;
    if (pPackage == nullptr)
      return false;
    if (itemId == 0)
      return false;
    if (pPackage->checkItemCount(itemId, 1) == false)
    {
      return false;
    }   
  }

  if (justCheck)
  {
    return true;
  }
  //使用额外次数
  if (isInviter)
    pPackage->reduceItem(itemId, ESOURCE_USEITEM);  
  pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_EXTRACOUNT, 1);
  return true;
}

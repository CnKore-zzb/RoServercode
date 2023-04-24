#include "UserProposal.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "SceneUser2.pb.h"
#include "xLuaTable.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"
#include "ChatManager_SC.h"
#include "SysmsgConfig.h"

extern "C"
{
#include "xlib/md5/md5.h"
}


UserProposal::UserProposal(SceneUser *user) : m_pUser(user)
{
}

UserProposal::~UserProposal()
{
}

bool UserProposal::popTheQuest(QWORD targetID, DWORD itemID)
{
  SceneUser* pOther = SceneUserManager::getMe().getUserByID(targetID);
  if (m_pUser == nullptr || pOther == nullptr)
    return false;
  //可以中途更换求婚对象
  if(isHoldProposal() && getTargetID() == targetID)
  {
    MsgManager::sendMsg(m_pUser->id, 3227, MsgParams(SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_WEDDING_RING)));
    return false;
  }
  //主要是载入是否为玩具戒指
  if (loadItemCFG(itemID) == false)
  {
    XERR << "[求婚-加载物品配置]" << itemID  << "加载失败" << XEND;
    return false;
  }
  if (loadMiscCFG() == false)
  {
    XERR << "[求婚-加载GameConfig]" << "WeddingMiscCFG加载失败" << XEND;
    return false;
  }

  breakHandAndFollow(m_pUser->id);
  setTargetID(targetID);
  setStartTime(xTime::getCurMSec());
  sendProposalCmd();
  doKnee();

  return true;
}

bool UserProposal::loadItemCFG(DWORD itemID)
{
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(itemID);
  if (pCFG == nullptr)
  {
    XERR << "[求婚-加载物品配置]" << itemID << "获取SItemCFG失败" << XEND;
    return false;
  }

  const xLuaData& oGmData = pCFG->oGMData;
  /*if(oGmData.has("istoy") == false || oGmData.has("maxrange") == false
      || oGmData.has("buff") == false)
  {
    XERR << "[求婚-加载物品配置]" << itemID << "物品配置错误" << XEND;
    return false;
  }
  */

  setItemID(itemID);
  m_bIsToyRing = (oGmData.getTableInt("istoy") == 1) ? true : false;
  m_dwMaxRange = oGmData.getTableInt("maxrange");
  m_setBuffIDs.clear();
  xLuaData buffd = oGmData.getData("buff");
  buffd.getIDList(m_setBuffIDs);
  return true;
}

bool UserProposal::loadMiscCFG()
{
  const SWeddingMiscCFG& rCFG = MiscConfig::getMe().getWeddingMiscCFG();
  m_qwWaitTime =  rCFG.dwCourtshipInviteOverTime * ONE_THOUSAND;

  if (m_qwWaitTime == 0)
    return false;

  return true;
}

void UserProposal::sendProposalCmd()
{
  if(m_pUser == nullptr)
    return;
  SceneUser* pOther = SceneUserManager::getMe().getUserByID(getTargetID());
  if (pOther == nullptr)
  {
    XLOG << "[求婚-用户]" << "无法获取目标用户" << getTargetID()  << XEND;
    return;
  }

  MarriageProposalCmd cmd;
  cmd.set_masterid(m_pUser->id);
  cmd.set_mastername(m_pUser->name);
  cmd.set_itemid(m_dwItemID);
  cmd.set_time(now() + 30);

  char sign[1024];
  bzero(sign, sizeof(sign));
  std::stringstream ss;
  ss << pOther->id << "_" << cmd.masterid() << "_" << cmd.time() << "_" << "#$%^&";
  upyun_md5(ss.str().c_str(), ss.str().size(), sign);

  cmd.set_sign(sign);

  PROTOBUF(cmd, send, len);

  pOther->sendCmdToMe(send, len);
}

void UserProposal::doKnee()
{
  SceneUser* pMaster = m_pUser;
  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(getTargetID());
  if(pMaster == nullptr || pTarget == nullptr)
    return;

  pMaster->m_oMove.stop();

  NpcChangeAngle cmd;
  cmd.set_guid(pMaster->id);
  cmd.set_targetid(pTarget->id);
  DWORD angle = calcAngle(pMaster->getPos(), pTarget->getPos());
  cmd.set_angle(angle);
  PROTOBUF(cmd, send, len);
  pMaster->sendCmdToNine(send, len);

  //已使用上面的NpcChangeAngel替代
  //pMaster->getUserSceneData().setDir(static_cast<DWORD>(angle * ONE_THOUSAND));

  doAction(pMaster->id, ACTIONID_KNEE);
}

void UserProposal::doAction(QWORD uid, DWORD actionid)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(uid);
  if(pUser == nullptr)
    return;

  pUser->m_oMove.stop();
  UserActionNtf cmd;
  cmd.set_type(EUSERACTIONTYPE_MOTION);
  cmd.set_value(actionid);
  cmd.set_charid(uid);
  pUser->setAction(actionid);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToNine(send, len);
}

void UserProposal::doSorry(QWORD uid)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(uid);
  if(pUser == nullptr)
    return;
  pUser->playEmoji(EMOJIID_SORRY);
}

void UserProposal::breakHandAndFollow(QWORD uid)
{
  SceneUser* pUser = SceneUserManager::getMe().getUserByID(uid);
  if(pUser == nullptr)
    return;
  if (pUser->m_oHands.has())
    pUser->m_oHands.breakup();
  if (pUser->getWeaponPet().haveHandCat())
    pUser->getWeaponPet().breakHand();
  if (pUser->getHandNpc().haveHandNpc())
    pUser->getHandNpc().delHandNpc();
  if (pUser->getUserPet().handPet())
    pUser->getUserPet().breakHand();
  if (pUser->getUserSceneData().getFollowerID())
    pUser->getUserSceneData().setFollowerIDNoCheck(0);
}

void UserProposal::doReplyCmd(const Cmd::MarriageProposalReplyCmd &rev)
{
  SceneUser* pTarget = m_pUser;
  if (pTarget == nullptr)
    return;
  char sign[1024];
  bzero(sign, sizeof(sign));
  std::stringstream ss;
  ss << pTarget->id << "_" << rev.masterid() << "_" << rev.time() << "_" << "#$%^&";
  upyun_md5(ss.str().c_str(), ss.str().size(), sign);

  if (strncmp(sign, rev.sign().c_str(), 1024) != 0)
  {
    XERR << "[求婚-reply]" << "sign验证失败" << XEND;
    return;
  }

  SceneUser* pMaster = SceneUserManager::getMe().getUserByID(rev.masterid());
  //对象不符合，忽略,或者提示对方已取消求婚（一般是改变了求婚对象）
  if(pMaster == nullptr || pMaster->getProposal().getTargetID() != pTarget->id)
    return;

  //超时处理
  if (pMaster->isAlive() == false || pTarget->isAlive() == false
      || pMaster->getProposal().isHoldProposal() == false)
  {
    MsgManager::sendMsg(pMaster->id,9636);
    pMaster->getProposal().clear();
    return;
  }

  switch (rev.reply())
  {
    case EPROPOSALREPLY_YES:
      {
        //TODO 双方动作,NPC发附近消息

        breakHandAndFollow(pMaster->id);
        breakHandAndFollow(pTarget->id);
        pMaster->getProposal().consumeItem();
        pTarget->getProposal().clear();
        pMaster->getProposal().clear();

        if(pMaster->getProposal().isToyRing() == false)
        {
          //TODO 播放特效与添加buff
          //
          monsterShowLove();
          std::stringstream stream;
          stream.str("");
          stream << pMaster->name << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_TO) << pTarget->name << SysmsgConfig::getMe().getSysmsgCFG(SYSMSG_PROPOSE_SUCCESS);
          ChatManager_SC::getMe().sendChatMsgToNine(pMaster, stream.str().c_str(), ECHAT_CHANNEL_ROUND);

          const TSetDWORD& buffs = pMaster->getProposal().getBuffs();
          if (!buffs.empty())
          {
            for (auto &d : buffs)
            {
              pMaster->m_oBuff.add(d);
              pTarget->m_oBuff.add(d);
            }
          }
        }

        MarriageProposalSuccessCmd mastercmd;
        mastercmd.set_charid(pTarget->id);
        mastercmd.set_ismaster(true);
        PROTOBUF(mastercmd, mastersend, masterlen);
        pMaster->sendCmdToMe(mastersend, masterlen);

        MarriageProposalSuccessCmd targetcmd;
        targetcmd.set_charid(pMaster->id);
        targetcmd.set_ismaster(false);
        PROTOBUF(targetcmd, targetsend, targetlen);
        pTarget->sendCmdToMe(targetsend, targetlen);
      }
      break;
    case EPROPOSALREPLY_NO:
      {
        doSorry(pTarget->id);
        doFakeDead(pMaster->id);
        pMaster->getProposal().clear();
      }
      break;
    case EPROPOSALREPLY_CANCEL:
      {
        //如果能通过上面各种判断，作为超时处理
        MsgManager::sendMsg(pMaster->id,9636);
        pMaster->getProposal().clear();
      }
      break;
    case EPROPOSALREPLY_OUTRANGE:
      {
        MsgManager::sendMsg(pTarget->id, 9633);
        MsgManager::sendMsg(pMaster->id, 9632);
        pMaster->getProposal().clear();
      }
      break;
    default:
      break;
  }
}

void UserProposal::monsterShowLove()
{
  if(m_pUser == nullptr || m_pUser->getScene() == nullptr)
    return;
  xSceneEntrySet tarSet;
  m_pUser->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, m_pUser->getPos(), tarSet);
  float maxDis = 15.0f;
  for (auto s = tarSet.begin(); s != tarSet.end(); ++s)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID((*s)->id);
    if (pNpc == nullptr || pNpc->getScene() == nullptr || pNpc->isMonster() == false)
      continue;
    float dis = getDistance(m_pUser->getPos(), pNpc->getPos());
    if (dis > maxDis)
      continue;
    pNpc->playEmoji(EMOJIID_HEART);
  }
}

bool UserProposal::consumeItem()
{
  SceneUser* pMaster = m_pUser;
  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(getTargetID());
  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(getItemID());
  if (pMaster == nullptr || pTarget == nullptr)
    return false;
  if(pCFG == nullptr)
  {
    XLOG << "[求婚-物品消耗]" << getItemID() << "获取物品信息失败" << XEND;
    return false;
  }

  BasePackage* pPackage = pMaster->getPackage().getPackage(EPACKTYPE_MAIN);
  if(pPackage == nullptr)
    return false;

  pTarget->getEvent().onItemBeUsed(getItemID());
  pMaster->getQuest().onItemUse(pTarget->id, getItemID());
  pMaster->getEvent().onItemUsed(getItemID(), 1);
  pMaster->getAchieve().onWedding(EACHIEVECOND_WEDDING_PROPOSE);

  pPackage->reduceItem(getItemID(), ESOURCE_ACTSKILL, 1);
  if (ItemConfig::getMe().isUseItem(pCFG->eItemType))
  {
    const string& guid = pPackage->getGUIDByType(getItemID());
    ItemBase* pBase = pPackage->getItem(guid);
    if (pBase)
      pBase->setCD(xTime::getCurMSec());
    pMaster->m_oCDTime.add(pCFG->dwTypeID, pCFG->dwCD, CD_TYPE_ITEM);
  }

  MsgManager::sendMsg(pMaster->id, 3055, MsgParams(pCFG->strNameZh, static_cast<DWORD>(1)) );
  XLOG << "[求婚-物品消耗]" << getItemID() << "消耗成功" << XEND;
  return true;
}

void UserProposal::doFakeDead(QWORD uid)
{
  doAction(uid, ACTIONID_FAKEDEAD);
}

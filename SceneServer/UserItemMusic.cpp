#include "UserItemMusic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneServer.h"
#include "Scene.h"
#include "SceneUser2.pb.h"
#include "SceneNpcManager.h"
#include "MsgManager.h"
#include "GMCommandRuler.h"
#include "ItemConfig.h"

UserItemMusic::UserItemMusic(SceneUser *user):m_pUser(user)
{
}

UserItemMusic::~UserItemMusic()
{
}

bool UserItemMusic::startMusicItem(std::string uri, DWORD dwExpireTime, DWORD dwNpcId, DWORD dwRange, DWORD dwNum, DWORD itemId)
{
  if (!m_pUser)
    return false;
  if (!m_pUser->getScene())
    return false;
  if (!checkCanUse())
    return false;

  if (!m_musicUri.empty())
  {
    stopMusicItem();
  }

  if (dwNpcId)
  {
    NpcDefine def;
    def.setID(dwNpcId);
    //def.m_oVar.m_qwItemUserID = m_pUser->id;
    def.setBehaviours(def.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);
    def.setTerritory(0);
    xPos lastPos;
    for (DWORD i = 0; i < dwNum; ++i)
    {
      xPos dest;
      DWORD dwCount = 0;
      while (++dwCount < 30)
      {
        if (m_pUser->getScene()->getCircleRoundPos(m_pUser->getPos(), 2.0f, dest) == false)
        {
          dest = m_pUser->getPos();
          continue;
        }
        if (!lastPos.empty())
        {
          if (getDistance(lastPos, dest) < 1.5f)
          {
            dest = m_pUser->getPos();
            continue;
          }
        }
        lastPos = dest;
        break;
      }
      def.setPos(dest);
      DWORD dir = calcAngle(dest, m_pUser->getPos());
      def.setDir(dir);
      SceneNpc *pNpc = SceneNpcManager::getMe().createNpc(def, m_pUser->getScene());
      if (pNpc)
      {
        pNpc->m_blGod = true;
        m_setNpcGuid.insert(pNpc->getTempID());
      }
    }
  }
  
  m_musicItemPos = m_pUser->getPos();
  m_musicUri = uri;
  m_dwMusicItemStartTime = xTime::getCurSec();
  m_dwMusicItemEndTime = m_dwMusicItemStartTime + dwExpireTime;
  m_dwMusicItemRange = dwRange;
  m_dwLastEffectItemId = itemId;
  
  const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(m_dwLastEffectItemId);
  if (pItemCfg)
  {
    m_dataType = convertEquipType2DataType(pItemCfg->eEquipType);
  }

  XLOG << "[音乐道具-开始使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->getName() << "music uri:" << m_musicUri << "starttime:" << m_dwMusicItemStartTime << "npcid" << dwNpcId <<"物品id" << m_dwLastEffectItemId <<"m_dataType" << m_dataType << XEND;
  sendMusicToMe(false, true);
  return true;
}

bool UserItemMusic::checkCanUse()
{
  if (m_pUser->isInMusicNpcRange())
  {
    MsgManager::sendMsg(m_pUser->id, 72);
    return false;
  }
  return true;
}

bool UserItemMusic::stopMusicItem()
{
  for (auto &v : m_setNpcGuid)
  {
    SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(v);
    if (pNpc)
    {
      pNpc->setClearState();
    }
  }
  XLOG << "[音乐道具-结束使用]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->getName() << "uri:" << m_musicUri << "starttime:" << m_dwMusicItemStartTime << XEND;
  m_musicUri.clear();
  m_dwMusicItemStartTime = 0;
  m_dwMusicItemStartTime = 0;
  m_dwMusicItemRange = 0;
  m_setNpcGuid.clear();
  m_dwLastEffectItemId = 0;
  sendMusicToMe(false, false);
  return true;
}

void UserItemMusic::sendMusicToMe(bool bOnlyHander, bool bAdd)
{
  auto sendtof = [&](SceneUser* pUser)
  {
    if (!pUser)
      return;

    for (auto &v : m_setNpcGuid)
    {
      SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(v);
      if (pNpc)
      {
        if (bAdd)
        {
          pNpc->sendMeToUser(pUser);
        }
        else
        {
          pNpc->delMeToUser(pUser);
        }
      }
    }
    ItemMusicNtfUserCmd msg;
    if (bAdd)
    {
      msg.set_add(true);
      msg.set_uri(m_musicUri);
      msg.set_starttime(m_dwStartTime);
    }
    else {
      msg.set_add(false);
    }
    PROTOBUF(msg, send, len);
    pUser->sendCmdToMe(send, len);

    XLOG << "[音乐道具-通知玩家]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "add" << bAdd << "music uri:" << m_musicUri << "starttime:" << m_dwMusicItemStartTime << XEND;
  };

  if (m_pUser->m_oHands.has())
  {
    SceneUser* pOther = m_pUser->m_oHands.getOther();
    if (pOther)
    sendtof(pOther);
  }

  if (!bOnlyHander)
    sendtof(m_pUser);
}

void UserItemMusic::checkMusicItem(DWORD dwCurTime)
{
  if (m_musicUri.empty())
    return;
  if (dwCurTime == 0)
    dwCurTime = now();
  if (dwCurTime > m_dwMusicItemEndTime)
  {
    stopMusicItem();
    return;
  }

  if (checkDistance(m_pUser->getPos(), m_musicItemPos, m_dwMusicItemRange) == false)
  {
    stopMusicItem();
    return;
  }
}

void UserItemMusic::leaveScene()
{
  if (m_musicUri.empty())
    return;
  stopMusicItem();
}

bool UserItemMusic::checkHeadEffect(DWORD itemId, DWORD actionId, EUserDataType dataType)
{
  auto stop = [&]()
  {
    if (m_dwLastEffectItemId && dataType == m_dataType)
    {
      XLOG << "[头饰-音乐] 符合条件关闭音乐 charid" << m_pUser->id << m_pUser->name << "头饰id" << itemId << "actionid" << actionId << "上次id" << m_dwLastEffectItemId<<"datatype"<< m_dataType << XEND;
      stopMusicItem();
    }
  };
 
  const SHeadEffect* pCfg = ItemConfig::getMe().getHeadEffectCFG(itemId);
  if (pCfg == nullptr)
  {
    stop();
    return false;
  }

  TVecQWORD vecParams;
  vecParams.push_back(actionId);
  if (!pCfg->checkCond(SHEADCONDTYPE_ACTION, vecParams))
  {
    stop();
    return false;
  }
  XLOG << "[头饰-音乐] 符合条件播放音乐 charid" << m_pUser->id << m_pUser->name << "头饰id" << itemId << "actionid" << actionId <<"datatype" << dataType << "上次id" << m_dwLastEffectItemId <<"上次datatype" << m_dataType << XEND;
  xLuaData data = pCfg->oGMData;
  GMCommandRuler::getMe().execute(m_pUser, data);
  return true;
}

void UserItemMusic::onActionChange(DWORD actionId)
{ 
  if (actionId == 0)
  {
    if (m_dwLastEffectItemId)
    {
      XLOG << "[头饰-音乐] 动作停止，符合条件关闭音乐 charid" << m_pUser->id << m_pUser->name << "头饰id" << m_dwLastEffectItemId << "actionid" << actionId << "上次id" << m_dwLastEffectItemId << XEND;
      stopMusicItem();
    }
    return;
  }
  
  if (checkHeadEffect(m_pUser->getUserSceneData().getHead(), actionId, EUSERDATATYPE_HEAD))
    return;

  if (checkHeadEffect(m_pUser->getUserSceneData().getBack(), actionId, EUSERDATATYPE_BACK))
    return;

  if (checkHeadEffect(m_pUser->getUserSceneData().getFace(), actionId, EUSERDATATYPE_FACE))
    return;

  if (checkHeadEffect(m_pUser->getUserSceneData().getTail(), actionId, EUSERDATATYPE_TAIL))
    return;

  if (checkHeadEffect(m_pUser->getUserSceneData().getMouth(), actionId, EUSERDATATYPE_MOUTH))
    return;
}

EUserDataType UserItemMusic::convertEquipType2DataType(EEquipType equipType)
{
  switch (equipType)
  {
  case EEQUIPTYPE_BACK:
    return EUSERDATATYPE_BACK;
  case EEQUIPTYPE_HEAD:
    return EUSERDATATYPE_HEAD;
  case EEQUIPTYPE_FACE:
    return EUSERDATATYPE_FACE;
  case EEQUIPTYPE_TAIL:
    return EUSERDATATYPE_TAIL;
  case EEQUIPTYPE_MOUTH:
    return EUSERDATATYPE_MOUTH;
  default:    
    break;
  }
  return EUSERDATATYPE_MIN;
}
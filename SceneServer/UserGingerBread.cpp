#include "UserGingerBread.h"
#include "SceneUser.h"
#include "MiscConfig.h"
#include "GuidManager.h"
#include "MsgManager.h"


void GingerBread::sendMsgBeforeDel(QWORD charId, DWORD curSec)
{
  if (bSend)
    return;
  if (curSec < blobInfo.expiretime() - 60)
    return;

  MsgManager::sendMsg(charId, 25102);
  bSend = true;
}

UserGingerBread::UserGingerBread(SceneUser* user):m_pUser(user)
{ 
}

UserGingerBread::~UserGingerBread()
{
}

void UserGingerBread::onEnterScene()
{
  Cmd::SyncGiveItemSceneTradeCmd cmd;
  cmd.set_charid(m_pUser->id);
  
  PROTOBUF(cmd, send, len);
  if (thisServer->sendCmdToSession(send, len) == false)
  {
    XERR << "[交易-姜饼人-赠送] 进入场景，向session请求赠送信息失败" << m_pUser->id << XEND;
    return;
  }
  XLOG << "[交易-姜饼人-赠送] 进入场景，向session请求赠送信息" << m_pUser->id << XEND;
}
void UserGingerBread::onLeaveScene()
{
  m_mapGingerBread.clear();
}

void UserGingerBread::syncFromSession(const Cmd::SyncGiveItemSceneTradeCmd& rev)
{
  for (int i = 0; i < rev.iteminfo_size(); ++i)
  {
    const GiveItemInfo& rInfo = rev.iteminfo(i);
    TMapId2Ginger& rMapGinger = m_mapGingerBread[rev.type()];
    auto it = rMapGinger.find(rInfo.id());
    if (it != rMapGinger.end())
    {
      continue;
    }
    if (isExpire(rInfo.expiretime()))
      continue;
    addOne(rev.type(), rInfo.id(), rInfo.expiretime());
  }
  XLOG << "[交易-姜饼人], 玩家" << m_pUser->name << m_pUser->id << "从session同步, type" <<rev.type() << "size"<<rev.iteminfo_size() << XEND;
}

bool UserGingerBread::addOne(EGiveType type, QWORD id, DWORD expireTime)
{
  if (m_pUser->getScene() == nullptr)
  {
    XERR << "[交易-姜饼人], 玩家不在场景" << m_pUser->id << id << XEND;
    return false;
  }
  QWORD qwGUID = GuidManager::getMe().getNextNpcID();
  if (qwGUID == 0)
  {
    XERR << "[交易-姜饼人], 生成guid失败"<<m_pUser->id << id << XEND;
    return false;
  }

  GingerBreadNpcCmd cmd;
  cmd.set_isadd(true);
  cmd.set_userid(m_pUser->id);

  GingerBreadNpcData* pData = cmd.mutable_data();
  if (pData == nullptr)
  {
    XERR << "[交易-姜饼人], 生成proto失败" << m_pUser->id << id << XEND;
    return false;
  }
  pData->set_giveid(id);
  pData->set_npcid(4364);
  pData->set_guid(qwGUID);
  pData->set_expiretime(expireTime);
  pData->set_type(type);

  xPos dest;
  float fRange = 7;
  if (m_pUser->getScene()->getRandPos(m_pUser->getPos(), fRange, dest) == false)
  {
    XERR << "[交易-姜饼人], 生成出生位置失败" << m_pUser->id << id << XEND;
    return false;
  }
  ScenePos *pPos = cmd.mutable_bornpos();
  if (pPos)
  {
    pPos->set_x(dest.getX());
    pPos->set_y(dest.getY());
    pPos->set_z(dest.getZ());
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  GingerBread gingerBread;
  gingerBread.blobInfo = *pData;
  TMapId2Ginger& rMapGinger = m_mapGingerBread[type];
  rMapGinger[id] = gingerBread;

  XLOG << "[交易-姜饼人], 玩家" << m_pUser->name << m_pUser->id << "增加姜饼人" <<"类型" <<type << "giveid" << pData->giveid() << "guid" << pData->guid() << "expitetime" << pData->expiretime() << "x" << cmd.bornpos().x() << "y" << cmd.bornpos().y() << "z" << cmd.bornpos().z() << XEND;
  return true;
}

bool UserGingerBread::delOne(EGiveType type, QWORD id, bool accept, bool isExpire)
{ 
  auto i = m_mapGingerBread.find(type);
  if (i == m_mapGingerBread.end())
    return false;

  auto it = i->second.find(id);
  if (it == i->second.end())
    return false;

  //play dialog
  if (accept)
  {
    //sign
    playDialog(it->second.blobInfo.guid(), MiscConfig::getMe().getTradeCFG().dwSignDialog);
  }
  else
  {
    if (type != EGiveType_Lottery)
      playDialog(it->second.blobInfo.guid(), MiscConfig::getMe().getTradeCFG().dwRefuseDialog);
  }

  GingerBreadNpcCmd cmd;
  cmd.set_isadd(false);
  cmd.set_userid(m_pUser->id);

  GingerBreadNpcData* pData = cmd.mutable_data();
  if (pData == nullptr)
  {
    return false;
  }
  pData->CopyFrom(it->second.blobInfo); 

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
  
  i->second.erase(id);

  if (isExpire && type != EGiveType_Lottery)
  {
    //由于您长时间未进行领取，姜饼人带着礼物消失了(/ω＼)…
    MsgManager::sendMsg(m_pUser->id, 25103);
  }

  XLOG << "[交易-姜饼人], 玩家" << m_pUser->name << ", " << m_pUser->id << "删除姜饼人"<<"type"<<type <<"giveid"<<pData->giveid()<<"guid"<<pData->guid() << "expitetime" << pData->expiretime() << "accept" << accept <<"是否超时"<<isExpire << XEND;
  return true;
}

//one sec tick
void UserGingerBread::timer(DWORD dwCurTime)
{
  TSetQWORD delSet;
  for (auto jt = m_mapGingerBread.begin(); jt != m_mapGingerBread.end(); ++jt)
  {
    TMapId2Ginger& rMapGinger = jt->second;
    for (auto it = rMapGinger.begin(); it != rMapGinger.end(); ++it)
    {
      if (isExpire(it->second.blobInfo.expiretime()))
      {
        delSet.insert(it->second.blobInfo.giveid());
      }
    }
    for (auto& v : delSet)
      delOne(jt->first, v, false, true);
  }

  for (auto jt = m_mapGingerBread.begin(); jt != m_mapGingerBread.end(); ++jt)
  {
    TMapId2Ginger& rMapGinger = jt->second;
    for (auto it = rMapGinger.begin(); it != rMapGinger.end(); ++it)
    {
      if (it->second.nextDialogTime <= dwCurTime)
      {
        setDialogId();
        playDialog(it->second.blobInfo.guid(), m_dwDialogId);
        it->second.nextDialogTime = dwCurTime + MiscConfig::getMe().getTradeCFG().dwDialogInterval;
      }
      it->second.sendMsgBeforeDel(m_pUser->id, dwCurTime);
    }
  }
}

void UserGingerBread::collectNineData(Cmd::MapUser* pData)
{
  if (!pData)
    return;

  for (auto jt = m_mapGingerBread.begin(); jt != m_mapGingerBread.end(); ++jt)
  {
    TMapId2Ginger& rMapGinger = jt->second;
    for (auto it = rMapGinger.begin(); it != rMapGinger.end(); ++it)
    {
      pData->add_givenpcdatas()->CopyFrom(it->second.blobInfo);
    }
  }
}

void UserGingerBread::playDialog(QWORD guid, DWORD id)
{

  TalkInfo cmd;
  cmd.set_guid(guid);
  cmd.set_talkid(id);
  if (id == 83018 || id == 83019)
  {//显示玩家的名字
    MsgParam* pParam = cmd.add_params();
    if (pParam)
      pParam->set_param(m_pUser->name);    
  }
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
}

void UserGingerBread::playEmoji(DWORD id)
{
}

bool UserGingerBread::isExpire(DWORD expireTime)
{
  DWORD curSec = now();
  if (expireTime <= curSec)
    return true;
  return false;
}

inline void UserGingerBread::setDialogId()
{
  if (m_dwDialogId)
    return;

  if (m_pUser->getUserSceneData().getGender() == EGENDER_MALE)
  {
    m_dwDialogId = MiscConfig::getMe().getTradeCFG().dwMaleDialog;
  }
  else
  {
    m_dwDialogId = MiscConfig::getMe().getTradeCFG().dwFemaleDialog;
  }
  XDBG << "[交易-姜饼人] dialog" << m_dwDialogId << m_pUser->getUserSceneData().getGender() << XEND;
}

void UserGingerBread::sendLotterLetter2Client(const Cmd::ItemData& itemData)
{
  LoveLetterNtf scmd;
  scmd.set_name(itemData.letter().sendusername());
  scmd.set_content(itemData.letter().content());
  scmd.set_content2(itemData.letter().content2());
  scmd.set_type(ELETTERTYPE_LOTTERY);
  scmd.set_configid(itemData.letter().configid());
  scmd.set_letterid(0);
  scmd.set_bg(itemData.letter().bg());
  PROTOBUF(scmd, send, len);
  m_pUser->sendCmdToMe(send, len);

}

void UserGingerBread::resetGingerPos(const xPos& pos)
{
  for (auto jt = m_mapGingerBread.begin(); jt != m_mapGingerBread.end(); ++jt)
  {
    TMapId2Ginger& rMapGinger = jt->second;
    for (auto it = rMapGinger.begin(); it != rMapGinger.end(); ++it)
    {
      if (isExpire(it->second.blobInfo.expiretime()) == false)
      {
        GingerBreadNpcCmd cmd;
        cmd.set_isadd(true);
        cmd.set_userid(m_pUser->id);
        GingerBreadNpcData* pData = cmd.mutable_data();
        if(pData == nullptr)
          continue;
        pData->set_giveid(it->second.blobInfo.giveid());
        pData->set_npcid(4364);
        pData->set_guid(it->second.blobInfo.guid());
        pData->set_expiretime(it->second.blobInfo.expiretime());
        pData->set_type(it->second.blobInfo.type());
        ScenePos *pPos = cmd.mutable_bornpos();
        if(pPos)
        {
          pPos->set_x(pos.getX());
          pPos->set_y(pos.getY());
          pPos->set_z(pos.getZ());
        }
        PROTOBUF(cmd, send, len);
        m_pUser->sendCmdToNine(send, len);
      }
    }
  }
}

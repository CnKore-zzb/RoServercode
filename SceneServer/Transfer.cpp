#include "Transfer.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"
#include "MapConfig.h"
#include "MsgManager.h"
#include "SceneUserManager.h"
#include "ActivityEventManager.h"
#include "GMCommandRuler.h"

Transfer::Transfer(SceneUser *u):
  m_pUser(u)
  {
  }

Transfer::~Transfer()
{
}

void Transfer::save(Cmd::BlobTransfer *data)
{
  data->Clear();
  data->set_map1allactivated(m_bMap1AllActivated);
  data->set_map2allactivated(m_bMap2AllActivated);
  for(auto it = m_setActiveTransferIDs.begin();it != m_setActiveTransferIDs.end();it++)
  {
    data->add_npcid(*it);
  }
  XDBG << "[Transfer-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
}

void Transfer::load(const Cmd::BlobTransfer &data)
{
  m_bMap1AllActivated = data.map1allactivated();
  m_bMap2AllActivated = data.map2allactivated();
  m_setActiveTransferIDs.clear();
  for (int i=0;i < data.npcid_size();i++){
    /*
    const SNpcCFG* pNpc = NpcConfig::getMe().getNpcCFG(data.npcid(i));
    if(pNpc == nullptr){
      XERR << "[传送阵-读取]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "npcid:" << data.npcid(i) << "在Table_Npc.txt表中未找到" << XEND;
    }
    */
    m_setActiveTransferIDs.insert(data.npcid(i));
  }
}

void Transfer::goTransfer(const Cmd::UseDeathTransferCmd& cmd)
{
  DWORD toNpcId = cmd.tonpcid();
  DWORD fromNpcId = cmd.fromnpcid();

  const SDeathTransferConfig* toCFG  = DeathTransferConfig::getMe().getDeathTransferCfg(toNpcId);
  const SDeathTransferConfig* fromCFG  = DeathTransferConfig::getMe().getDeathTransferCfg(fromNpcId);
  if(toCFG == nullptr || fromCFG == nullptr)
  {
    XERR << "[传送阵-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "fromid:" << fromNpcId << "toid:" << toNpcId << "在Table_DeathTransferMap.txt表中未找到" << XEND;
    return; 
  }

  if(!canTransfer(fromCFG, toCFG))
  {
    XERR << "[传送阵-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "fromid:" << fromNpcId << "toid:" << toNpcId << "玩家没有达到传送条件" << XEND;
    return;
  }

  // 是否是同图传送
  if(m_pUser->getMapID() == toCFG->mapid)
  {
    m_pUser->goTo(toCFG->pos, false);
  }
  else
  {
    m_pUser->m_oSkillProcessor.useTransportSkill(m_pUser, ETRANSPORTTYPE_TRANSFER, toCFG->mapid, &(toCFG->pos));
  }
  // 播放特效音效
  const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();

  xLuaData data;
  data.setData("type", "effect");
  data.setData("effect", configPath.strEnterSceneEffect);
  data.setData("posbind", 1);
  GMCommandRuler::getMe().execute(m_pUser, data);

  xLuaData sound;
  sound.setData("type", "sound_effect");
  sound.setData("se", configPath.strEnterSceneSound);
  sound.setData("sync", 1);
  GMCommandRuler::getMe().execute(m_pUser, sound);
  
  // 推进任务
  m_pUser->getQuest().onTransfer(fromNpcId, toNpcId);

  XLOG << "[传送阵-传送]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "fromid:" << fromNpcId << "toid:" << toNpcId << "传送成功" << XEND;
}

bool Transfer::canTransfer(const SDeathTransferConfig* from, const SDeathTransferConfig* to)
{
  // 都激活了
  if(m_bMap1AllActivated && m_bMap2AllActivated)
  {
    return true;
  }

  //更新激活状态
  m_bMap1AllActivated = DeathTransferConfig::getMe().isMapAllActivated(1, m_setActiveTransferIDs);
  m_bMap2AllActivated = DeathTransferConfig::getMe().isMapAllActivated(2, m_setActiveTransferIDs);
  if(m_bMap1AllActivated && m_bMap2AllActivated)
  {
    return true;
  }

  //某一个没激活
  if(!m_pUser->getTransfer().hasActivated(from->npcid) || !m_pUser->getTransfer().hasActivated(to->npcid))
  {
    return false;
  }

  // 同图
  if(from->mapid == to->mapid)
  {
    // 所在地图未全部激活
    if((from->deathMapNumber == 1 && !m_bMap1AllActivated) || (from->deathMapNumber == 2 && !m_bMap2AllActivated))
    {
      // 都是子传送器则无法传送
      return !(from->npcType == ETransferType::Child && to->npcType == ETransferType::Child);
    }
  }
  // 不同图
  else
  {
    // 若不都是母传送器,则无法传送
    return from->npcType == ETransferType::Parent && to->npcType == ETransferType::Parent;
  }
  
  return true;
}

void Transfer::sendList()
{
  DeathTransferListCmd cmd;
  for(auto it = m_setActiveTransferIDs.begin(); it != m_setActiveTransferIDs.end(); it++){
    cmd.add_npcid(*it);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

bool Transfer::hasActivated(DWORD npcid)
{
  auto it = m_setActiveTransferIDs.find(npcid);
  return it != m_setActiveTransferIDs.end();
}

bool Transfer::activateTransfer(DWORD npcid, bool bNotify /*= true*/)
{
  if(hasActivated(npcid)){
    XERR << "[传送阵-激活]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "npcid :" << npcid << "notify :" << bNotify << "失败,已激活" << XEND;
    return false; 
  }
  
  m_setActiveTransferIDs.insert(npcid);

  //判断是否都激活了
  if(DeathTransferConfig::getMe().isMapAllActivated(1, m_setActiveTransferIDs) != m_bMap1AllActivated){
    m_bMap1AllActivated = true;
    MsgManager::sendMsg(m_pUser->id, 25803);
  }
  if(DeathTransferConfig::getMe().isMapAllActivated(2, m_setActiveTransferIDs) != m_bMap2AllActivated){
    m_bMap2AllActivated = true;
    MsgManager::sendMsg(m_pUser->id, 25804);
  }

  if(bNotify){
    NewDeathTransferCmd cmd;
    cmd.set_npcid(npcid);

    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }

  XLOG << "[传送阵-激活]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "npcid :" << npcid << "notify :" << bNotify << XEND;
  return true;
}

bool Transfer::canUseWingOfFly(DWORD mapid)
{
  if((mapid == DEATH_MAP1_ID && !m_bMap1AllActivated) || (mapid == DEATH_MAP2_ID && !m_bMap2AllActivated)){
    return false;
  }
  return true;
}

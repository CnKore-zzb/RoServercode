#include "HighRefine.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "Package.h"
#include "MsgManager.h"
#include "LuaManager.h"
#include "HighRefineConfig.h"
#include "MiscConfig.h"
#include "DScene.h"

HighRefine::HighRefine(SceneUser* pUser) : m_pUser(pUser)
{
}

HighRefine::~HighRefine()
{
}

void HighRefine::load(const Cmd::BlobHighRefine& data)
{
  for (int i = 0; i < data.datas_size(); ++i)
  {
    const Cmd::HighRefineData& refineData = data.datas(i);
    auto it = m_mapHighRefineData.find(refineData.pos());
    if (it == m_mapHighRefineData.end())
    {
      TVecDWORD vec;
      it = m_mapHighRefineData.insert(std::make_pair(refineData.pos(), vec)).first;
    }
    if (it == m_mapHighRefineData.end())
      continue;
    
    for (int j = 0; j < refineData.level_size(); ++j)
    {
      it->second.push_back(refineData.level(j));
    }
  }
  XLOG << "[极限精炼-加载] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "m_mapHighRefineData size: " << m_mapHighRefineData.size() << XEND;
}

void HighRefine::save(Cmd::BlobHighRefine* data)
{
  if (!data)
    return;

  for (auto &m : m_mapHighRefineData)
  {
    Cmd::HighRefineData* pRefineDatadata = data->add_datas();
    if (!pRefineDatadata)
      continue;
    pRefineDatadata->set_pos(m.first);
    for (auto &v : m.second)
    {
      pRefineDatadata->add_level(v);
    }
  }
  XDBG << "[极限精炼-保存] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << " 数据大小 : " << data->ByteSize() << XEND;
}

void HighRefine::sendHighRefineData()
{
  Cmd::NtfHighRefineDataCmd cmd;
  
  for (auto &m : m_mapHighRefineData)
  {
    HighRefineData* pRefineDatadata = cmd.add_datas();
    if (!pRefineDatadata)
      continue;
    pRefineDatadata->set_pos(m.first);
    for (auto &v : m.second)
    {
      pRefineDatadata->add_level(v);
    }
  }
  
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XDBG << "[极限精炼-推送信息给前端] " << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "msg" << cmd.ShortDebugString() << XEND;
}

bool HighRefine::matCompose(Cmd::HighRefineMatComposeCmd& rev)
{
  if (MiscConfig::getMe().getFuncForbidCFG().isForbid(FUNC_FORBID_HIGHREINE))
    return false;

  SceneNpc* pNpc = SceneNpcManager::getMe().getNpcByTempID(rev.npcid());
  if (!pNpc)
  {
    XERR << "[极限精炼-炼金合成]，找不到npc" <<m_pUser->accid <<m_pUser->id <<m_pUser->name <<"npcid"<<rev.npcid() << XEND;
    return false;
  }  
  const SMatCompose* pMatCfg = HighRefineConfig::getMe().getMatComposeConfig(rev.dataid());
  if (!pMatCfg)
  {
    XERR << "[极限精炼-炼金合成]，找不到炼金合成的配置" << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() << XEND;
    return false;
  }
  
  //check groupid

  BasePackage* pMainPkg = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (!pMainPkg)
    return false;

  //checkout mainmaterial
  TVecItemInfo vecMainMat;
  TVecItemInfo vecViceMat;

  auto checkMat = [&](bool main, TVecItemInfo& vecItemInfo)->bool
  {
    if (!pMainPkg->checkItemCount(vecItemInfo))
    {
      XERR << "[极限精炼-炼金合成]，材料背包个数不够" << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() << XEND;
      return false;
    }
    
    //check percent
    if (!pMatCfg->checkPercent(main, vecItemInfo))
    {
      XERR << "[极限精炼-炼金合成]，未达到100%，主料" << main << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() << XEND;
      return false;
    }
    return true;
  };
  
  //检查主料
  for (int i = 0; i < rev.mainmaterial_size(); ++i)
  {
    const MatItemInfo& rInfo = rev.mainmaterial(i);
    ItemInfo itemInfo;
    itemInfo.set_id(rInfo.itemid());
    itemInfo.set_count(rInfo.num());
    vecMainMat.push_back(itemInfo);
  }
  if (checkMat(true, vecMainMat) == false)
    return false;

  //检查辅料
  for (int i = 0; i < rev.vicematerial_size(); ++i)
  {
    const MatItemInfo& rInfo = rev.vicematerial(i);
    ItemInfo itemInfo;
    itemInfo.set_id(rInfo.itemid());
    itemInfo.set_count(rInfo.num());
    vecViceMat.push_back(itemInfo);
  }
  if (checkMat(false, vecViceMat) == false)
    return false;

  //check money
  if (!m_pUser->checkMoney(EMONEYTYPE_SILVER, pMatCfg->dwCostZeny))
  {
    XERR << "[极限精炼-炼金合成]，zeny不足" << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() << "zeny" << pMatCfg->dwCostZeny << XEND;
    return false;
  }

  DWORD count = pMatCfg->randomCount();
  ItemInfo addItem;
  addItem.set_id(pMatCfg->dwProductId);
  addItem.set_count(count);
  addItem.set_source(ESOURCE_MAT_COMPOSE);

  if (pMainPkg->checkAddItem(addItem, EPACKMETHOD_CHECK_WITHPILE) == false)
  {
    MsgManager::sendMsg(m_pUser->id, 989);
    XERR << "[极限精炼-炼金合成]，背包空间不足" << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() <<"产出物品"<<pMatCfg->dwProductId <<"数量"<<count << XEND;
    return false;
  }

  //reduce item and money
  m_pUser->subMoney(EMONEYTYPE_SILVER, pMatCfg->dwCostZeny, ESOURCE_MAT_COMPOSE);
  
  pMainPkg->reduceItem(vecMainMat, ESOURCE_MAT_COMPOSE);
  pMainPkg->reduceItem(vecViceMat, ESOURCE_MAT_COMPOSE);

  pMainPkg->addItem(addItem);

  XLOG << "[极限精炼-炼金合成] 成功" << m_pUser->accid << m_pUser->id << m_pUser->name << "dataid" << rev.dataid() << "产出物品" << pMatCfg->dwProductId << "数量" << count <<"消耗zeny"<< pMatCfg->dwCostZeny << XEND;
  return true;
}

bool HighRefine::highRefine(Cmd::HighRefineCmd& rev)
{ 
  if (MiscConfig::getMe().getFuncForbidCFG().isForbid(FUNC_FORBID_HIGHREINE))
  {
    XERR << "[极限精炼-极限精炼]，功能没开放，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid()<< XEND;
    return false;
  }

  if (!m_pUser->getScene())
    return false;
  
  GuildScene* pScene = dynamic_cast<GuildScene*>(m_pUser->getScene());
  if (!pScene)
  {
    XERR << "[极限精炼-极限精炼]，不在公会场景里，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << XEND;
    return false;
  }
  const SHighRefine*pCfg = HighRefineConfig::getMe().getHighRefineConfig(rev.dataid());
  if (!pCfg)
    return false;
  
  DWORD buildingLv = pScene->getGuild().getBuildingLevel(EGUILDBUILDING_HIGH_REFINE);
  
  const SGuildBuildingCFG*pBuildCfg = GuildConfig::getMe().getGuildBuildingCFG(EGUILDBUILDING_HIGH_REFINE, buildingLv);
  if (!pBuildCfg)
  {
    XERR << "[极限精炼-极限精炼]，找不到配置，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << "位置pos" << pCfg->ePos << "建筑等级" << buildingLv << XEND;
    return false;
  }
  if (!pBuildCfg->isHighRefinePosUnlock(pCfg->ePos, pCfg->dwType))
  {
    XERR << "[极限精炼-极限精炼]，公会建筑等级不够，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << "位置pos" << pCfg->ePos << "建筑等级" << buildingLv << pCfg->dwType << XEND;
    return false;
  }

  if (checkLevel(pCfg->ePos, pCfg->dwLevel))
  {
    XERR << "[极限精炼-极限精炼]，该等级已经精炼过，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << "pos" << pCfg->ePos << "level" << pCfg->dwLevel << XEND;
    return false;
  }
  
  DWORD lv = pCfg->dwLevel % 1000;
  DWORD preLevel = 0;
  if (pCfg->dwPreLevel)
    preLevel = pCfg->dwPreLevel;
  else
  {
    if (lv > 1)
      preLevel = pCfg->dwLevel - 1;
  }

  if (preLevel)
  {//check preLevel
    if (!checkLevel(pCfg->ePos, preLevel))
    {
      XERR <<"[极限精炼-极限精炼]，前置等级没有解锁，charid" <<m_pUser->id <<"name"<<m_pUser->name<<"dataid"<< rev.dataid() <<"pos" << pCfg->ePos <<"level"<<pCfg->dwLevel <<"preLv"<<preLevel << XEND;
      return false;
    }
  }
  
  BasePackage* pMainPkg = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (!pMainPkg)
    return false;
 
  if (!pMainPkg->checkItemCount(pCfg->oCost))
  {
    XERR << "[极限精炼-极限精炼]，材料个数不够，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << "pos" << pCfg->ePos << "level" << pCfg->dwLevel << "preLv" << preLevel << XEND;
    return false;
  }

  pMainPkg->reduceItem(pCfg->oCost, ESOURCE_HIGH_REFINE);
  
  auto it = m_mapHighRefineData.find(pCfg->ePos);
  if (it == m_mapHighRefineData.end())
  {
    it = m_mapHighRefineData.insert(std::make_pair(pCfg->ePos, TVecDWORD())).first;
  }
  it->second.push_back(pCfg->dwLevel);
  
  //刷新属性
  m_pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  m_pUser->refreshDataAtonce();
  //send update to client
  UpdateHighRefineDataCmd cmd;
  cmd.mutable_data()->set_pos(pCfg->ePos);
  cmd.mutable_data()->add_level(pCfg->dwLevel);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[极限精炼-极限精炼]，成功，charid" << m_pUser->id << "name" << m_pUser->name << "dataid" << rev.dataid() << "pos" << pCfg->ePos << "level" << pCfg->dwLevel << "preLv" << preLevel << XEND;
  return true;
}

bool HighRefine::checkLevel(Cmd::EEquipPos pos,DWORD lv)
{
  auto it = m_mapHighRefineData.find(pos);
  if (it == m_mapHighRefineData.end())
    return false;
  
  auto subIt = std::find(it->second.begin(), it->second.end(), lv);
  if (subIt == it->second.end())
    return false;
  return true;
}

//void HighRefine::collectEquipAttr(Cmd::EEquipPos pos, DWORD refineLv, TVecAttrSvrs& vecAttrs)
void HighRefine::collectEquipAttr(Cmd::EEquipPos pos, DWORD refineLv)
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;

  pos = HighRefineConfig::getMe().getValidPos(pos);

  auto it = m_mapHighRefineData.find(pos);
  if (it == m_mapHighRefineData.end())
    return;

  for (auto &v : it->second)
  {
    const SHighRefine*pCfg = HighRefineConfig::getMe().getHighRefineConfigByLv(pos, v);
    if (!pCfg)
      continue;

    if (refineLv < pCfg->dwRefineLevel)
      continue;

    for (auto&v1 : pCfg->vecAttr)
    {
      if (!v1.checkJob(m_pUser->getProfession()))
        continue;

      for (auto& v2 : v1.vecAttr)
        pAttr->modifyCollect(ECOLLECTTYPE_EQUIP, v2);
        /*{
        float value = vecAttrs[v2.type()].value() + v2.value();
        vecAttrs[v2.type()].set_type(v2.type());
        vecAttrs[v2.type()].set_value(value);
      }*/
      XDBG << "[极限精炼-极限精炼] 属性生效 " <<m_pUser->accid<<m_pUser->id<<m_pUser->name<<"job"<<m_pUser->getProfession() <<"pos"<<pos <<"等级" <<pCfg->dwRefineLevel <<"精炼等级"<<refineLv << XEND;
    }
  }
}

#include "StatMgr.h"
#include "SceneUser.h"
#include "SceneItem.pb.h"
#include "SceneServer.h"

void StatMgr::onItemUse(SceneUser* pUser, const Cmd::ItemInfo& rInfo)
{
  if (pUser == nullptr)
    return;

  const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rInfo.id());
  if (pCFG == nullptr)
  {
    XERR << "[统计管理-物品使用]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "统计" << rInfo.ShortDebugString() << "失败,未在 Table_Item.txt 表中找到" << XEND;
    return;
  }

  // pet wear
  if (ItemConfig::getMe().isPetWearItem(rInfo.id()) == true)
  {
    SPetWearStat& rStat = m_mapPetWearStat[pUser->id];
    rStat.charid = pUser->id;
    rStat.mapQualityCount[pCFG->eQualityType] += rInfo.count();
  }
}

void StatMgr::timer(DWORD curSec)
{
  if (m_dwFlushTick > curSec)
    return;
  m_dwFlushTick = curSec + MIN_T;

  flushPetWear();
}

void StatMgr::flushPetWear()
{
  if (m_mapPetWearStat.empty() == true)
    return;

  PetWearUseCountStatCmd scmd;
  for (auto m = m_mapPetWearStat.begin(); m != m_mapPetWearStat.end();)
  {
    PetWear* pWear = scmd.add_wears();
    const SPetWearStat& rStat = m->second;
    pWear->set_charid(rStat.charid);
    for (auto &q : rStat.mapQualityCount)
    {
      pWear->add_types(q.first);
      pWear->add_counts(q.second);
    }

    if (scmd.ByteSize() > TRANS_BUFSIZE)
      break;

    m = m_mapPetWearStat.erase(m);
  }

  if (scmd.wears_size() < 0)
    return;

  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmd(ClientType::stat_server, ssend, slen);

  XDBG << "[统计管理-宠物装扮] 本次发送至StatServer统计数据 :" << scmd.ShortDebugString() << XEND;
}


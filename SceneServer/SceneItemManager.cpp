#include "SceneItemManager.h"
#include "SceneUser.h"
#include "PlatLogManager.h"
#include "SceneServer.h"
#include "GuidManager.h"

SceneItemManager::SceneItemManager()
{

}

SceneItemManager::~SceneItemManager()
{

}

bool SceneItemManager::init()
{
  //TODO
  return true;
}

void SceneItemManager::final()
{

}

void SceneItemManager::timer(QWORD curMSec)
{
  FUN_TIMECHECK_30();
  if (curMSec < m_qwPickupTick)
    return;
  m_qwPickupTick = curMSec + MiscConfig::getMe().getSceneItemCFG().qwPickupInterval;

  auto it = xEntryTempID::ets_.begin();
  auto tmp = it;
  for ( ; it!=xEntryTempID::ets_.end(); )
  {
    tmp = it++;
    if (tmp->second)
    {
      SceneItem* item = (SceneItem *)(tmp->second);
      if (item->getStatus() == ESCENEITEMSTATUS_INVALID)
      {
        delItem(item);
      }
      else
      {
        item->timer(curMSec);
      }
    }
  }
}

SceneItem* SceneItemManager::getSceneItem(QWORD tempid)
{
  return dynamic_cast<SceneItem*>(getEntryByTempID(tempid));
}

SceneItem* SceneItemManager::createSceneItem(Scene* scene, const ItemInfo& rInfo, const xPos& rPos)
{
  if (scene == nullptr)
    return nullptr;

  DWORD tempid = GuidManager::getMe().getNextSceneItemID();
  if (tempid == 0)
    return nullptr;
  SceneItem* pTemp = getSceneItem(tempid);
  if (pTemp != nullptr)
    return pTemp;

  // create scene item
  SceneItem* pItem = NEW SceneItem(tempid, scene, rInfo, rPos);
  if (pItem == nullptr)
    return nullptr;
  if (addItem(pItem) == false)
  {
    SAFE_DELETE(pItem);
    return nullptr;
  }

  // add to scene
  if (!pItem->enterScene(scene))
  {
    XERR << "[ITEM]" << pItem->tempid << scene->name << "创建item失败:" << rInfo.id() << XEND;
    delItem(pItem);
    return nullptr;
  }

  XLOG << "[ITEM]" << pItem->tempid << scene->name << "创建item:" << rInfo.id() << "(" << pItem->getPos().x << pItem->getPos().y << pItem->getPos().z << ")" << XEND;
  return pItem;
}

bool SceneItemManager::addItem(SceneItem* item)
{
  return addEntry(item);
}

void SceneItemManager::delItem(SceneItem* item)
{
  removeEntry(item);
  SAFE_DELETE(item);
}

bool SceneItemManager::pickupSceneItem(SceneUser* pUser, QWORD tempid)
{
  if (pUser == nullptr)
    return false;

  bool bSuccess = false;
  SceneItem* pItem = nullptr;

  do
  {
    // get scene item
    pItem = SceneItemManager::getMe().getSceneItem(tempid);
    if (pItem == nullptr)
      break;

    // check can pickup
    if (pItem->canPickup(pUser) == false)
      break;

    // check once item
    if (pUser->getPackage().isOnce(pItem->getItemInfo().id()) == true)
    {
      XERR << "[道具-拾取]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "无法添加" << pItem->getItemInfo().ShortDebugString() << "该道具已获得过" << XEND;
      break;
    }

    const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(pItem->getItemInfo().id());
    bool bShow = pItemCFG && ItemConfig::getMe().isCard(pItemCFG->eItemType);
    // add item
    bool bNoShare = pItemCFG && (pItemCFG->eItemType == EITEMTYPE_PICKEFFECT || pItemCFG->eItemType == EITEMTYPE_PICKEFFECT_1);
    const GTeam& rTeam = pUser->getTeam();
    if (pItem->getOwnTime() && rTeam.getTeamID() && rTeam.isPickupShare() && !bNoShare)
    {
      std::set<SceneUser*> setUser = pUser->getTeamSceneUser();
      if (setUser.empty())
        break;
      if (pItem->getItemInfo().source() == ESOURCE_SEAL)
      {
        const SealMiscCFG& rSealCFG = MiscConfig::getMe().getSealCFG();
        for (auto it = setUser.begin(); it != setUser.end();)
        {
          if (*it)
          {
            DWORD sealednum = (*it)->getVar().getVarValue(EVARTYPE_SEAL);
            if (sealednum > rSealCFG.dwMaxDaySealNum)
            {
              it = setUser.erase(it);
              continue;
            }
            if (pUser != (*it) && pItem->canPickup(*it) == false)
            {
              it = setUser.erase(it);
              continue;
            }
          }
          ++it;
        }
      }
      ENpcType npcType = pItem->getNpcType();
      if (npcType == ENPCTYPE_MVP || npcType == ENPCTYPE_MINIBOSS)
      {
        for (auto s = setUser.begin();s != setUser.end();)
        {
          if ((*s)->isJustInViceZone())
          {
            s = setUser.erase(s);
            continue;
          }
          ++s;
        }
      }
      
      std::map<SceneUser*, DWORD> mapCount;
      
      if (npcType == ENPCTYPE_MVP || npcType ==ENPCTYPE_MINIBOSS || pItem->getItemInfo().source() == ESOURCE_SEAL || pItem->getOwners().empty())
      {
        for (DWORD i = 0; i < pItem->getItemInfo().count(); i++)
        {
          SceneUser** ppUser = randomStlContainer(setUser);
          if (ppUser && *ppUser)
          {
            mapCount[*ppUser] += 1;
          }
        }
      }
      else
      {
        //weight
        std::map<DWORD, SceneUser*> mapWeight;    //按权重从小到大排序了
        DWORD maxWeight = 0;
        DWORD weight = 0;
        for (auto &v : setUser)
        {
          if (v)
          {
            weight = v->getAddictRatio() * 100;
            maxWeight += weight;
            mapWeight.insert(std::make_pair(maxWeight, v));
          }
        }
        for (DWORD i = 0; i < pItem->getItemInfo().count(); i++)
        {
          DWORD r = randBetween(0, maxWeight);
          for (auto &v : mapWeight)
          {
            if (r <= v.first)
            {
              mapCount[v.second] += 1;
              break;
            }
          }
        }
      }

      for (auto it = mapCount.begin(); it != mapCount.end(); ++it)
      {
        if (it->second == 0)
          continue;

        SceneUser* pWho = it->first;
        ItemInfo itemInfo = pItem->getItemInfo();
        itemInfo.set_count(it->second);

        ItemInfo extraitem;
        extraitem.CopyFrom(itemInfo);
        pWho->m_oBuff.onPickUpItem(extraitem);

        pWho->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE, bShow, true);
        XLOG << "[道具-拾取]，队友均分模式，charid" << pWho->id << "itemid:" << itemInfo.id() << "count:" << it->second << "total count:" << pItem->getItemInfo().count() << XEND;

        // 商人系技能处理
        handleMerchantSkill(pWho, pItem, it->second);
      }
    }
    else
    {
      ItemInfo oItem = pItem->getItemInfo();
      ItemInfo extraitem;
      extraitem.CopyFrom(oItem);
      pUser->m_oBuff.onPickUpItem(extraitem);

      pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE, bShow, true);

      // 商人系技能处理
      handleMerchantSkill(pUser, pItem, pItem->getItemInfo().count());
    }
    // set mark
    bSuccess = true;

  } while (0);

  PickupItem cmd;
  cmd.set_success(bSuccess);
  if (bSuccess)
  {
    cmd.set_playerguid(pUser->id);
    cmd.set_itemguid(tempid);

    PROTOBUF(cmd, send, len);
    pUser->sendCmdToNine(send, len);
    pUser->setPickupTime();

    pItem->leaveScene();

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_PickItem;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);
    PlatLogManager::getMe().ItemOperLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      EItemOperType_Pick,
      pItem->getItemInfo().id(),
      pItem->getItemInfo().count());
  }
  else
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }

  return true;
}

// 商人系技能
// 集资: 当商人获得Zeny时，将额外获得X%的Zeny
// 贪婪: 商人自己获得物品时，将有X%概率额外获得1份
void SceneItemManager::handleMerchantSkill(SceneUser* pUser, const SceneItem* pItem, DWORD realCount)
{
  if (pUser == nullptr || pItem == nullptr || realCount <= 0)
    return;

  const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(pItem->getItemInfo().id());
  if (pItemCFG == nullptr)
    return;

  if (pItem->getNpcType() != ENPCTYPE_MVP && pItem->getNpcType() != ENPCTYPE_MINIBOSS && // MVP,MINI不触发
      pItem->getScene() != nullptr && !pItem->getScene()->isDScene() && // 副本不触发
      pItem->getItemInfo().source() == ESOURCE_PICKUP // 魔物掉落的才触发
    ) {
    bool bShow = pItemCFG && ItemConfig::getMe().isCard(pItemCFG->eItemType);

    switch (pItemCFG->eItemType) {
    case EITEMTYPE_SILVER:    // 集资, 掉落硬币
    {
      DWORD percent = LuaManager::getMe().call<DWORD>("CalcMerchantRaisePercent", (xSceneEntryDynamic*)(pUser));
      if (percent <= 0)
        break;

      DWORD count = 1.0 * realCount * percent / 1000.0;
      if (count <= 0)
        break;

      ItemInfo itemInfo = pItem->getItemInfo();
      itemInfo.set_count(count);

      ItemInfo extraitem;
      extraitem.CopyFrom(itemInfo);
      pUser->m_oBuff.onPickUpItem(extraitem);

      pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE, bShow, true);
      XLOG << "[道具-商人集资]: charid" << pUser->id << "itemid:" << itemInfo.id() << "count:" << count << XEND;

      break;
    }

    case EITEMTYPE_MATERIAL:  // 贪婪, 掉落材料
    {
      DWORD prob = LuaManager::getMe().call<DWORD>("CalcMerchantGreedProb", (xSceneEntryDynamic*)(pUser));
      if (DWORD(randBetween(1, 1000)) > prob)
        break;

      DWORD count = 1;

      ItemInfo itemInfo = pItem->getItemInfo();
      itemInfo.set_count(count);

      ItemInfo extraitem;
      extraitem.CopyFrom(itemInfo);
      pUser->m_oBuff.onPickUpItem(extraitem);

      pUser->getPackage().addItem(itemInfo, EPACKMETHOD_AVAILABLE, bShow, true);
      XLOG << "[道具-商人贪婪]: charid" << pUser->id << "itemid:" << itemInfo.id() << "count:" << count << XEND;

      break;
    }

    default:
      break;
    }
  }
}

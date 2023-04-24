/**
 * @file ItemManager.h
 * @brief item manager
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-05-04
 */

#pragma once

#include "ItemConfig.h"
#include "Item.h"
#include "SceneManual.pb.h"

using namespace Cmd;
using std::string;
using std::map;
using std::set;

class ItemManager : public xSingleton<ItemManager>
{
  public:
    ItemManager();
    virtual ~ItemManager();

    const SItemCFG* getItemCFG(DWORD dwTypeID) const { return ItemConfig::getMe().getItemCFG(dwTypeID); }
    const SMasterCFG* getMasterCFG(EMasterType eType, DWORD lv) const { return ItemConfig::getMe().getMasterCFG(eType, lv); }
    const SSuitCFG* getSuitCFG(DWORD id) const { return ItemConfig::getMe().getSuitCFG(id); }
    const SRefineCFG* getRefineCFG(EItemType eType, EQualityType eQuality, DWORD lv) const { return ItemConfig::getMe().getRefineCFG(eType, eQuality, lv); }

    DWORD getCardIDByMonsterID(DWORD monsterid) const { return ItemConfig::getMe().getCardIDByMonsterID(monsterid); }
    const TSetDWORD& getSuitIDs(DWORD equipTypeID) const { return ItemConfig::getMe().getSuitIDs(equipTypeID); }

    //DWORD getSortIndex(EItemType eType);
    DWORD getSortWeaponIndex(EItemType eType, EProfession eProfession);

    bool isFashion(EEquipType eType) const { return eType == EEQUIPTYPE_HEAD || eType == EEQUIPTYPE_BACK ||
      eType == EEQUIPTYPE_FACE || eType == EEQUIPTYPE_TAIL || eType == EEQUIPTYPE_MOUTH || 
      eType == EEQUIPTYPE_WEAPON || eType == EEQUIPTYPE_ARMOUR || eType == EEQUIPTYPE_MOUNT; }
    bool isEquip(EItemType eType) const;
    bool isWeapon(EItemType eType) const;
    bool isLetter(EItemType eType) const;
    bool isHead(EItemType eType) const { return eType == EITEMTYPE_HEAD || eType == EITEMTYPE_FACE || eType == EITEMTYPE_MOUTH || eType == EITEMTYPE_BACK || eType == EITEMTYPE_TAIL; }
    bool isGUIDValid(const string& guid) const { return m_setAllItemGUIDs.find(guid) == m_setAllItemGUIDs.end(); }
    void addGUID(const string& guid) { m_setAllItemGUIDs.insert(guid); }

    ItemBase* createItem(const ItemInfo& rInfo);
    ItemBase* createItem(DWORD id);
    ItemBase* createItemLua(DWORD id, DWORD dwCount, DWORD dwSceneID);
  private:
    set<string> m_setAllItemGUIDs;
};


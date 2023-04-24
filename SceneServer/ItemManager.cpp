#include "ItemManager.h"
#include "xTime.h"
#include "SceneUser.h"
#include "GMCommandRuler.h"
#include "UserConfig.h"
#include "GuidManager.h"

//extern Table<SComposeBase>* g_pComposeBaseM;

ItemManager::ItemManager()
{

}

ItemManager::~ItemManager()
{

}

/*DWORD ItemManager::getSortIndex(EItemType eType)
{
  switch (eType)
  {
    case EITEMTYPE_MIN:
    case EITEMTYPE_GOLD:
    case EITEMTYPE_SILVER:
    case EITEMTYPE_DIAMOND:
    case EITEMTYPE_FRIENDSHIP:
    case EITEMTYPE_MORA:
    case EITEMTYPE_GARDEN:
    case EITEMTYPE_CONTRIBUTE:
    case EITEMTYPE_ASSET:
    case EITEMTYPE_BASEEXP:
    case EITEMTYPE_JOBEXP:
    case EITEMTYPE_HONOR:
    case EITEMTYPE_PURIFY:
    case EITEMTYPE_MANUALSPOINT:
    case EITEMTYPE_MANUALPOINT:
    case EITEMTYPE_PVPCOIN:
    case EITEMTYPE_LOTTERY:
      return 0;
    case EITEMTYPE_STREASURE:
    case EITEMTYPE_TREASURE:
      return 3;
    case EITEMTYPE_STUFF:
    case EITEMTYPE_HAIRSTUFF:
    case EITEMTYPE_FUNCTION:
    case EITEMTYPE_STUFFNOCUT:
    case EITEMTYPE_ARROW:
    case EITEMTYPE_USESKILL:
    case EITEMTYPE_GHOSTLAMP:
    case EITEMTYPE_MULTITIME:
    case EITEMTYPE_MONTHCARD:
    case EITEMTYPE_QUEST_ONCE:
    case EITEMTYPE_QUEST_TIME:
    case EITEMTYPE_ACTIVITY:
      return 2;
    case EITEMTYPE_SHEET:
    case EITEMTYPE_CONSUME:
    case EITEMTYPE_CONSUME_2:
    case EITEMTYPE_MATERIAL:
    case EITEMTYPE_CARD_WEAPON:
    case EITEMTYPE_CARD_ASSIST:
    case EITEMTYPE_CARD_ARMOUR:
    case EITEMTYPE_CARD_ROBE:
    case EITEMTYPE_CARD_SHOES:
    case EITEMTYPE_CARD_ACCESSORY:
    case EITEMTYPE_CARD_HEAD:
    case EITEMTYPE_MOUNT:
    case EITEMTYPE_BARROW:
    case EITEMTYPE_PET:
    case EITEMTYPE_EGG:
    case EITEMTYPE_CARDPIECE:
    case EITEMTYPE_EQUIPPIECE:
    case EITEMTYPE_QUESTITEM:
    case EITEMTYPE_QUESTITEMCOUNT:
    case EITEMTYPE_PORTRAIT:
    case EITEMTYPE_FRAME:
    case EITEMTYPE_WATER_ELEMENT:
    case EITEMTYPE_COLLECTION:
    case EITEMTYPE_RANGE:
    case EITEMTYPE_LETTER:
    case EITEMTYPE_FOOD_MEAT:
    case EITEMTYPE_FOOD_FISH:
    case EITEMTYPE_FOOD_VEGETABLE:
    case EITEMTYPE_FOOD_FRUIT:
    case EITEMTYPE_FOOD_SEASONING:
    case EITEMTYPE_FOOD:
      return 0;
    case EITEMTYPE_WEAPON_LANCE:
    case EITEMTYPE_WEAPON_SWORD:
    case EITEMTYPE_WEAPON_WAND:
    case EITEMTYPE_WEAPON_KNIFE:
    case EITEMTYPE_WEAPON_BOW:
    case EITEMTYPE_WEAPON_HAMMER:
    case EITEMTYPE_WEAPON_AXE:
    case EITEMTYPE_WEAPON_BOOK:
    case EITEMTYPE_WEAPON_DAGGER:
    case EITEMTYPE_WEAPON_INSTRUMEMT:
    case EITEMTYPE_WEAPON_WHIP:
    case EITEMTYPE_WEAPON_TUBE:
    case EITEMTYPE_WEAPON_FIST:
    case EITEMTYPE_ARMOUR:
    case EITEMTYPE_SHIELD:
    case EITEMTYPE_PEARL:
    case EITEMTYPE_EIKON:
    case EITEMTYPE_BRACER:
    case EITEMTYPE_BRACELET:
    case EITEMTYPE_TROLLEY:
    case EITEMTYPE_ROBE:
    case EITEMTYPE_SHOES:
    case EITEMTYPE_ACCESSORY:
    case EITEMTYPE_HEAD:
    case EITEMTYPE_BACK:
    case EITEMTYPE_FACE:
    case EITEMTYPE_HAIR:
    case EITEMTYPE_HAIR_MALE:
    case EITEMTYPE_HAIR_FEMALE:
    case EITEMTYPE_TAIL:
    case EITEMTYPE_MOUTH:
      return 1;
    case EITEMTYPE_MAX:
      return 0;
  }

  return 0;
}*/

DWORD ItemManager::getSortWeaponIndex(EItemType eType, EProfession eProfession)
{
  EProfession base = RoleConfig::getMe().getBaseProfession(eProfession);
  if (ItemConfig::getMe().isArtifact(eType))
    return 0;
  if (eType == EITEMTYPE_WEAPON_LANCE ||
      eType == EITEMTYPE_WEAPON_SWORD)
    return base == EPROFESSION_WARRIOR ? 1 : 2;
  if (eType == EITEMTYPE_WEAPON_HAMMER ||
      eType == EITEMTYPE_WEAPON_BOOK ||
      eType == EITEMTYPE_WEAPON_FIST)
    return base == EPROFESSION_ACOLYTE || base == EPROFESSION_MAGICIAN ? 1 : 2;
  if (eType == EITEMTYPE_WEAPON_BOW ||
      eType == EITEMTYPE_WEAPON_INSTRUMEMT ||
      eType == EITEMTYPE_WEAPON_WHIP)
    return base ==  EPROFESSION_ARCHER ? 1 : 2;
  if (eType == EITEMTYPE_WEAPON_KNIFE ||
      eType == EITEMTYPE_WEAPON_DAGGER)
    return base == EPROFESSION_THIEF ? 1 : 2;
  if (eType == EITEMTYPE_WEAPON_AXE ||
      eType == EITEMTYPE_WEAPON_TUBE)
    return base == EPROFESSION_MERCHANT ? 1 : 2;
  if (eType == EITEMTYPE_ARMOUR ||
      eType == EITEMTYPE_SHIELD ||
      eType == EITEMTYPE_PEARL ||
      eType == EITEMTYPE_EIKON ||
      eType == EITEMTYPE_BRACER ||
      eType == EITEMTYPE_BRACELET ||
      eType == EITEMTYPE_TROLLEY ||
      eType == EITEMTYPE_ROBE ||
      eType == EITEMTYPE_SHOES ||
      eType == EITEMTYPE_ACCESSORY ||
      eType == EITEMTYPE_HEAD ||
      eType == EITEMTYPE_BACK ||
      eType == EITEMTYPE_FACE ||
      eType == EITEMTYPE_HAIR ||
      eType == EITEMTYPE_EYE_MALE ||
      eType == EITEMTYPE_EYE_FEMALE ||
      eType == EITEMTYPE_HAIR_MALE ||
      eType == EITEMTYPE_HAIR_FEMALE ||
      eType == EITEMTYPE_TAIL)
    return 3;

  return 4;
}

bool ItemManager::isEquip(EItemType eType) const
{
  switch (eType)
  {
    case EITEMTYPE_MOUNT:
    case EITEMTYPE_BARROW:
    case EITEMTYPE_WEAPON_LANCE:
    case EITEMTYPE_WEAPON_SWORD:
    case EITEMTYPE_WEAPON_WAND:
    case EITEMTYPE_WEAPON_KNIFE:
    case EITEMTYPE_WEAPON_BOW:
    case EITEMTYPE_WEAPON_HAMMER:
    case EITEMTYPE_WEAPON_AXE:
    case EITEMTYPE_WEAPON_BOOK:
    case EITEMTYPE_WEAPON_DAGGER:
    case EITEMTYPE_WEAPON_INSTRUMEMT:
    case EITEMTYPE_WEAPON_WHIP:
    case EITEMTYPE_WEAPON_TUBE:
    case EITEMTYPE_WEAPON_FIST:
    case EITEMTYPE_ARMOUR:
    case EITEMTYPE_SHIELD:
    case EITEMTYPE_PEARL:
    case EITEMTYPE_EIKON:
    case EITEMTYPE_BRACER:
    case EITEMTYPE_BRACELET:
    case EITEMTYPE_TROLLEY:
    case EITEMTYPE_ROBE:
    case EITEMTYPE_SHOES:
    case EITEMTYPE_ACCESSORY:
    case EITEMTYPE_HEAD:
    case EITEMTYPE_BACK:
    case EITEMTYPE_FACE:
    case EITEMTYPE_TAIL:
    case EITEMTYPE_ARTIFACT_LANCE:
    case EITEMTYPE_ARTIFACT_SWORD:
    case EITEMTYPE_ARTIFACT_WAND:
    case EITEMTYPE_ARTIFACT_KNIFE:
    case EITEMTYPE_ARTIFACT_BOW:
    case EITEMTYPE_ARTIFACT_HAMMER:
    case EITEMTYPE_ARTIFACT_AXE:
    case EITEMTYPE_ARTIFACT_DAGGER:
    case EITEMTYPE_ARTIFACT_FIST:
    case EITEMTYPE_ARTIFACT_INSTRUMEMT:
    case EITEMTYPE_ARTIFACT_WHIP:
    case EITEMTYPE_ARTIFACT_BOOK:
    case EITEMTYPE_ARTIFACT_HEAD:
    case EITEMTYPE_ARTIFACT_BACK:
      return true;
    default:
      return false;
  }
  return false;
}

bool ItemManager::isWeapon(EItemType eType) const
{
  return eType >= EITEMTYPE_WEAPON_LANCE && eType <= EITEMTYPE_WEAPON_FIST;
}

bool ItemManager::isLetter(EItemType eType) const
{
  return eType == EITEMTYPE_LETTER;
}

ItemBase* ItemManager::createItem(const ItemInfo& rInfo)
{
  // get config
  const SItemCFG* pCFG = getItemCFG(rInfo.id());
  if (pCFG == nullptr)
    return nullptr;

  // create item
  ItemBase* pBase = nullptr;
  switch (pCFG->eItemType)
  {
    case EITEMTYPE_MIN:
    case EITEMTYPE_HONOR:
    case EITEMTYPE_SHEET:
    case EITEMTYPE_CONSUME:
    case EITEMTYPE_CONSUME_2:
    case EITEMTYPE_MATERIAL:
    case EITEMTYPE_PET:
    case EITEMTYPE_CARDPIECE:
    case EITEMTYPE_EQUIPPIECE:
    case EITEMTYPE_FASHION_PIECE:
    case EITEMTYPE_QUESTITEM:
    case EITEMTYPE_WATER_ELEMENT:
    case EITEMTYPE_MORA:
    case EITEMTYPE_FRIENDSHIP:
    case EITEMTYPE_RANGE:
    case EITEMTYPE_FOOD_MEAT:
    case EITEMTYPE_FOOD_FISH:
    case EITEMTYPE_FOOD_VEGETABLE:
    case EITEMTYPE_FOOD_FRUIT:
    case EITEMTYPE_FOOD_SEASONING:
    case EITEMTYPE_FOOD:
    case EITEMTYPE_GARDEN:
    case EITEMTYPE_POLLY_COIN:
    case EITEMTYPE_PET_WEARSHEET:
      pBase = NEW Item(pCFG);
      break;
    case EITEMTYPE_CARD_WEAPON:
    case EITEMTYPE_CARD_ASSIST:
    case EITEMTYPE_CARD_ARMOUR:
    case EITEMTYPE_CARD_ROBE:
    case EITEMTYPE_CARD_SHOES:
    case EITEMTYPE_CARD_ACCESSORY:
    case EITEMTYPE_CARD_HEAD:
      pBase = NEW ItemCard(pCFG);
      break;
    case EITEMTYPE_STREASURE:
      break;
    case EITEMTYPE_TREASURE:
      pBase = NEW ItemTreasure(pCFG);
      break;
    case EITEMTYPE_STUFF:
    case EITEMTYPE_HAIRSTUFF:
    case EITEMTYPE_ARROW:
    case EITEMTYPE_USESKILL:
    case EITEMTYPE_STUFFNOCUT:
    case EITEMTYPE_FUNCTION:
    case EITEMTYPE_GHOSTLAMP:
    case EITEMTYPE_MULTITIME:
    case EITEMTYPE_MONTHCARD:
    case EITEMTYPE_QUEST_ONCE:
    case EITEMTYPE_QUEST_TIME:
    case EITEMTYPE_COLLECTION:
    case EITEMTYPE_ACTIVITY:
    case EITEMTYPE_PET_CONSUME:
    case EITEMTYPE_FRIEND:
    case EITEMTYPE_PET_WEARUNLOCK:
      pBase = NEW ItemStuff(pCFG);
      break;
    case EITEMTYPE_GOLD:
    case EITEMTYPE_SILVER:
    case EITEMTYPE_DIAMOND:
    case EITEMTYPE_DEADCOIN:
    case EITEMTYPE_CONTRIBUTE:
    case EITEMTYPE_ASSET:
    case EITEMTYPE_MANUALSPOINT:
    case EITEMTYPE_MANUALPOINT:
    case EITEMTYPE_BASEEXP:
    case EITEMTYPE_JOBEXP:
    case EITEMTYPE_PURIFY:
    case EITEMTYPE_QUESTITEMCOUNT:
    case EITEMTYPE_PORTRAIT:
    case EITEMTYPE_FRAME:
    case EITEMTYPE_HAIR:
    case EITEMTYPE_EYE_MALE:
    case EITEMTYPE_EYE_FEMALE:
    case EITEMTYPE_HAIR_MALE:
    case EITEMTYPE_HAIR_FEMALE:
    case EITEMTYPE_PVPCOIN:
    case EITEMTYPE_LOTTERY:
    case EITEMTYPE_COOKER_EXP:
    case EITEMTYPE_GUILDHONOR:
    case EITEMTYPE_GOLDAPPLE:
    case EITEMTYPE_GETSKILL:
    case EITEMTYPE_PICKEFFECT:
    case EITEMTYPE_PICKEFFECT_1:
    case EITEMTYPE_QUOTA:
      break;
    case EITEMTYPE_MOUNT:
    case EITEMTYPE_BARROW:
    case EITEMTYPE_WEAPON_LANCE:
    case EITEMTYPE_WEAPON_SWORD:
    case EITEMTYPE_WEAPON_WAND:
    case EITEMTYPE_WEAPON_KNIFE:
    case EITEMTYPE_WEAPON_BOW:
    case EITEMTYPE_WEAPON_HAMMER:
    case EITEMTYPE_WEAPON_AXE:
    case EITEMTYPE_WEAPON_BOOK:
    case EITEMTYPE_WEAPON_DAGGER:
    case EITEMTYPE_WEAPON_INSTRUMEMT:
    case EITEMTYPE_WEAPON_WHIP:
    case EITEMTYPE_WEAPON_TUBE:
    case EITEMTYPE_WEAPON_FIST:
    case EITEMTYPE_ARMOUR:
    case EITEMTYPE_ARMOUR_FASHION:
    case EITEMTYPE_SHIELD:
    case EITEMTYPE_PEARL:
    case EITEMTYPE_EIKON:
    case EITEMTYPE_BRACER:
    case EITEMTYPE_BRACELET:
    case EITEMTYPE_TROLLEY:
    case EITEMTYPE_ROBE:
    case EITEMTYPE_SHOES:
    case EITEMTYPE_ACCESSORY:
    case EITEMTYPE_HEAD:
    case EITEMTYPE_BACK:
    case EITEMTYPE_FACE:
    case EITEMTYPE_TAIL:
    case EITEMTYPE_MOUTH:
    case EITEMTYPE_PET_EQUIP:
    case EITEMTYPE_ARTIFACT_LANCE:
    case EITEMTYPE_ARTIFACT_SWORD:
    case EITEMTYPE_ARTIFACT_WAND:
    case EITEMTYPE_ARTIFACT_KNIFE:
    case EITEMTYPE_ARTIFACT_BOW:
    case EITEMTYPE_ARTIFACT_HAMMER:
    case EITEMTYPE_ARTIFACT_AXE:
    case EITEMTYPE_ARTIFACT_DAGGER:
    case EITEMTYPE_ARTIFACT_FIST:
    case EITEMTYPE_ARTIFACT_INSTRUMEMT:
    case EITEMTYPE_ARTIFACT_WHIP:
    case EITEMTYPE_ARTIFACT_BOOK:
    case EITEMTYPE_ARTIFACT_HEAD:
    case EITEMTYPE_ARTIFACT_BACK:
      pBase = NEW ItemEquip(pCFG);
      break;
    case EITEMTYPE_LETTER:
      pBase = NEW ItemLetter(pCFG);
      break;
    case EITEMTYPE_EGG:
      pBase = NEW ItemEgg(pCFG);
      break;
    case EITEMTYPE_CODE:
    case EITEMTYPE_KFC_CODE:
      pBase = NEW ItemCode(pCFG);
      break;
    case EITEMTYPE_WEDDING_CERT:
    case EITEMTYPE_WEDDING_INVITE:
    case EITEMTYPE_WEDDING_MANUAL:
    case EITEMTYPE_WEDDING_RING:
      pBase = new ItemWedding(pCFG);
      break;
    case EITEMTYPE_MAX:
      break;
  }
  if (pBase == nullptr)
    return pBase;

  // init data
  if (pBase->init() == false)
  {
    SAFE_DELETE(pBase);
    return nullptr;
  }

  return pBase;
}

ItemBase* ItemManager::createItem(DWORD id)
{
  ItemInfo stInfo;
  stInfo.set_id(id);
  stInfo.set_count(1);
  return createItem(stInfo);
}

ItemBase* ItemManager::createItemLua(DWORD id, DWORD dwCount, DWORD dwSceneID)
{
  ItemBase* pBase = createItem(id);
  if (pBase == nullptr)
    return nullptr;
  pBase->setCount(dwCount);

  string guid = GuidManager::getMe().newGuidStr(thisServer->getZoneID(), dwSceneID);
  if (isGUIDValid(guid) == false)
  {
    SAFE_DELETE(pBase);
    return nullptr;
  }
  pBase->setGUID(guid);
  return pBase;
}


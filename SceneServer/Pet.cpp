#include "Pet.h"
#include "SceneUser.h"
#include "NpcConfig.h"

// pet item
bool SPetItem::fromData(const PetData& rData)
{
  dwID = rData.id();
  return true;
}

bool SPetItem::toData(PetData* pData)
{
  if (pData == nullptr)
    return false;

  pData->set_id(dwID);
  return true;
}

// pet
Pet::Pet(SceneUser* pUser) : m_pUser(pUser)
{

}

Pet::~Pet()
{

}

bool Pet::load(const BlobPet& oData)
{
  m_dwActive = oData.activepet();
  m_dwActivePartner = oData.activepartner();

  m_vecPetItems.clear();
  for (int i = 0; i < oData.datas_size(); ++i)
  {
    const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(oData.datas(i).id());
    if (pCFG == nullptr)
    {
      XERR << "[Pet::fromDataString] id =" << oData.datas(i).id() << "can not found in Table_Npc.txt" << XEND;
      continue;
    }

    SPetItem stItem;
    stItem.fromData(oData.datas(i));
    m_vecPetItems.push_back(stItem);
  }

  return true;
}

bool Pet::save(BlobPet* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  pBlob->set_activepet(m_dwActive);
  pBlob->set_activepartner(m_dwActivePartner);

  for (auto v = m_vecPetItems.begin(); v != m_vecPetItems.end(); ++v)
  {
    PetData* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[Pet::tpBlob->tring] id =" << v->dwID << "create error" << XEND;
      continue;
    }

    v->toData(pData);
  }

  XDBG << "[宠物-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

DWORD Pet::getPartnerID() const
{
  if (m_pUser == nullptr)
    return 0;
  if (m_pUser->getTransform().isInTransform())
    return 0;
  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  if (rCFG.isValidPartner(m_pUser->getProfession(), m_dwActivePartner) == false)
    return 0;
  if (m_pUser->getBuff().haveBuffType(EBUFFTYPE_RIDEWOLF))
    return 0;
  return m_dwActivePartner;
}

bool Pet::setPartnerID(DWORD dwID)
{
  if (m_pUser == nullptr)
    return false;

  if (dwID == 0)
  {
    m_dwActivePartner = dwID;
    m_pUser->setDataMark(EUSERDATATYPE_PET_PARTNER);
    m_pUser->refreshDataAtonce();
    XLOG << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "curpro =" << m_pUser->getUserSceneData().getProfession() << "搭档移除" << XEND;
    return true;
  }

  const SPetMiscCFG& rCFG = MiscConfig::getMe().getPetCFG();
  EProfession eProfession = m_pUser->getUserSceneData().getProfession();
  if (rCFG.isValidPartner(eProfession, dwID) == false)
  {
    XERR << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "curpro =" << m_pUser->getProfession() << "无法获得搭档" << dwID << XEND;
    return false;
  }

  const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
      << "curpro =" << m_pUser->getUserSceneData().getProfession() << "partner =" << dwID << "在Table_Npc.txt表中未找到" << XEND;
    return false;
  }

  /*if (eProfession >= EPROFESSION_MERCHANT && eProfession <= EPROFESSION_MECHANIC)
  {
    EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    if (pEquipPack == nullptr)
    {
      XERR << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "curpro =" << m_pUser->getUserSceneData().getProfession() << "partner =" << dwID << "失败,未找到" << EPACKTYPE_EQUIP << XEND;
      return false;
    }
    ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_MOUNT);
    if (pEquip == nullptr || pEquip->getCFG() == nullptr || pEquip->getCFG()->eItemType != EITEMTYPE_BARROW)
    {
      XERR << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "curpro =" << m_pUser->getUserSceneData().getProfession() << "partner =" << dwID << "失败,未装备手推车" << XEND;
      return false;
    }
  }*/

  m_dwActivePartner = dwID;
  m_pUser->setDataMark(EUSERDATATYPE_PET_PARTNER);
  m_pUser->refreshDataAtonce();

  XLOG << "[宠物-获得搭档]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "curpro =" << m_pUser->getUserSceneData().getProfession() << "获得为搭档" << dwID << XEND;
  return true;
}


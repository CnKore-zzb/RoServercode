/**
 * @file Pet.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-06
 */

#pragma once

#include "xDefine.h"
#include "ScenePet.pb.h"

namespace Cmd
{
  class BlobPet;
};

using namespace Cmd;
using std::string;
using std::vector;

const DWORD ARCHER_PARTNER = 1065;
const DWORD MERCHANT_PARTNER = 4425;

// pet item
struct SPetItem
{
  DWORD dwID = 0;

  SPetItem() {}

  bool fromData(const PetData& rData);
  bool toData(PetData* pData);
};
typedef vector<SPetItem> TVecPetItem;

// pet
class SceneUser;
class Pet
{
  public:
    Pet(SceneUser* pUser);
    ~Pet();

    bool load(const BlobPet& oBlob);
    bool save(BlobPet* pBlob);

    DWORD getPartnerID() const;
    DWORD getActivePartnerID() const { return m_dwActivePartner; }
    bool setPartnerID(DWORD dwID);
  private:
    SceneUser* m_pUser = nullptr;

    DWORD m_dwActivePartner = 0;
    DWORD m_dwActive = 0;

    TVecPetItem m_vecPetItems;
};


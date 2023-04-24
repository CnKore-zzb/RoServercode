/**
 * @file ManualConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-12-11
 */

#pragma once

#include "xSingleton.h"
#include "SceneManual.pb.h"
#include "SceneItem.pb.h"
#include "SceneTip.pb.h"
#include "TableManager.h"

using namespace Cmd;
using std::vector;
using std::map;
using std::string;

// config data
struct SManualLvCFG : public SBaseCFG
{
  DWORD dwLevel = 0;
  DWORD dwNeedExp = 0;
  DWORD dwSkillPoint = 0;

  map<EAttrType, UserAttrSvr> mapAttrs;

  SManualLvCFG() {
    SBaseCFG();
  }
};
typedef map<DWORD, SManualLvCFG> TMapManualLvCFG;

// manual type
typedef map<EItemType, EManualType> TMapManualType;

enum EManualQuest
{
  EMANUALQUEST_MIN = 0,
  EMANUALQUEST_KILL = 1,
  EMANUALQUEST_PHOTO = 2,
  EMANUALQUEST_ACTIVE = 3,
  EMANUALQUEST_MAX
};
struct SManualQuestCFG : public SBaseCFG
{
  DWORD dwID = 0;
  DWORD dwTargetID = 0;
  DWORD dwRewardID = 0;
  DWORD dwBuffID = 0;

  TSetDWORD setPreIDs;
  TVecDWORD vecParams;

  EManualType eManualType = EMANUALTYPE_MIN;
  EManualQuest eQuestType = EMANUALQUEST_MIN;

  SManualQuestCFG() {
    SBaseCFG();
  }
};
typedef vector<SManualQuestCFG> TVecManualQuestCFG;
typedef map<DWORD, SManualQuestCFG> TMapManualQuestCFG;
typedef map<DWORD, TVecManualQuestCFG> TMapTypeMQuestCFG;

struct SManualGroupCFG : public SBaseCFG
{
  DWORD dwGroupID = 0;
  DWORD dwQuestID = 0;

  TSetDWORD setManualIDs;
  TSetDWORD setRewardIDs;

  map<EAttrType, UserAttrSvr> mapAttrs;

  void toData(TVecAttrSvrs& vecAttrs) const
  {
    vecAttrs.clear();
    for (auto &m : mapAttrs)
      vecAttrs.push_back(m.second);
  }
};
typedef map<DWORD, SManualGroupCFG> TMapManualGroupCFG;

// return
struct SManualReturnCFG : public SBaseCFG
{
  DWORD dwID = 0;
  TVecItemInfo vecItems;

  TVecItemInfo vecUnlock1Items;
  TVecItemInfo vecUnlock2Items;
};
typedef map<DWORD, SManualReturnCFG*> TMapManualReturn;

// config
class ManualConfig : public xSingleton<ManualConfig>
{
  friend class xSingleton<ManualConfig>;
  private:
    ManualConfig();
  public:
    virtual ~ManualConfig();

    bool loadConfig();
    bool checkConfig();

    EManualType getManualType(EItemType eType) const;
    bool isShareQuest(EManualType eType) const { return eType == EMANUALTYPE_COLLECTION; }

    const SManualLvCFG* getManualLvCFG(DWORD dwLv) const;
    const SManualQuestCFG* getManualQuestCFG(DWORD dwID) const;
    const SManualGroupCFG* getManualGroupCFG(DWORD dwID) const;
    const TMapManualGroupCFG& getManualGroupList() const { return m_mapManualGroupCFG; }
    const SManualReturnCFG* getManualReturnCFG(DWORD dwID) const;
    const TMapManualLvCFG& getManualLvCFG() const { return m_mapManualLvCFG; }

    void collectQuest(EManualType eType, DWORD dwID, TVecManualQuestCFG& vecCFG);
  private:
    bool loadItemType();
    bool loadAdventureLevelConfig();
    bool loadAdventureAppendConfig();
    bool loadCollectionConfig();
    bool loadReturnConfig();

    EManualQuest getManualQuest(const string& str);
  private:
    TMapManualLvCFG m_mapManualLvCFG;

    TMapManualType m_mapManualType;

    TMapManualQuestCFG m_mapManualQuestCFG;
    TMapTypeMQuestCFG m_mapTypeMQuestCFG[EMANUALTYPE_MAX];

    TMapManualGroupCFG m_mapManualGroupCFG;
    TMapManualReturn m_mapReturnCFG;
};


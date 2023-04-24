/**
 * @file Attribute.h
 * @brief object attribute
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-04-22
 */

#pragma once

#include <bitset>
#include "xPos.h"
#include "xDefine.h"
#include "xPool.h"
#include "SceneUser.pb.h"
#include "SceneUser2.pb.h"
#include "SessionCmd.pb.h"
#include "SceneMap.pb.h"

namespace Cmd
{
  class BlobAttr;
};

using namespace Cmd;
using std::string;
using std::vector;
using std::bitset;
using std::pair;

enum EAttrOper
{
  EATTROPER_ADD,
  EATTROPER_SET,
};

enum ECollectType
{
  ECOLLECTTYPE_BASE = 1,
  //ECOLLECTTYPE_SKILL = 2,
  ECOLLECTTYPE_EQUIP = 4,
  //ECOLLECTTYPE_CARD = 8,
  ECOLLECTTYPE_STATIC_BUFF = 16, // 计算静态buff
  ECOLLECTTYPE_CHARACTER = 32,
  ECOLLECTTYPE_GUILD = 64,
  ECOLLECTTYPE_ASTROLABE = 128,
  ECOLLECTTYPE_ACHIEVEMENT = 256,
  ECOLLECTTYPE_FOOD = 512,
  ECOLLECTTYPE_PROFESSION = 1024,
  ECOLLECTTYPE_DYNAMIC_BUFF = 2048, // 仅计算动态buff
  ECOLLECTTYPE_NONE = 4096, // 没有部分发生变化, 但需计算一次属性 (ex:最多属性,最小属性变化时标记)
};

typedef std::set<DWORD> SetAttributeValidEnum;

class AttributeValidEnum
{
  public:
    static const SetAttributeValidEnum& get()
    {
      return s_setAttributeValidEnum;
    }
    static void initAttributeValidEnum();
    static SetAttributeValidEnum s_setAttributeValidEnum;
};

// base attribute
class xSceneEntryDynamic;
class Attribute
{
  public:
    Attribute(xSceneEntryDynamic* pEntry);
    virtual ~Attribute();

    virtual void updateAttribute();

    float getBaseAttr(EAttrType eType) const;
    float getOtherAttr(EAttrType eType) const;
    float getBuffAttr(EAttrType eType) const;
    float getAttr(EAttrType eType) const;
    float getTimeBuffAttr(EAttrType eType) const;
    float getPointAttr(EAttrType eType) const;
    bool setAttr(EAttrType eType, float value);
    bool setPointAttr(EAttrType eType, float value);
    bool setGMAttr(EAttrType eType, float value);
    bool setShowAttr(EAttrType eType, float value);
    //bool setGuildAttr(EAttrType eType, float value);
    //bool setEquipAttr(EAttrType eType, float value);
    void clearShowAttr() { m_vecShow.clear(); }
    const TVecAttrSvrs& getShowAttr() const { return m_vecShow; }

    //void setAttrMark(EAttrType eType) { if (eType > EATTRTYPE_MIN && eType < EATTRTYPE_MAX) m_bitset.set(eType); }
    void setMark(EAttrType eType = EATTRTYPE_MIN) { if (eType != EATTRTYPE_MIN) { m_bitset.set(eType); return; } m_bitset.set(); }
    void setCollectMark(ECollectType eType);

    DWORD getEquipMasterLv() const { return m_dwEquipMasterLv; }
    void setEquipMasterLv(DWORD lv);

    DWORD getRefineMasterLv() const { return m_dwRefineMasterLv; }
    void setRefineMasterLv(DWORD lv);

    virtual bool fromBlobAttr(const BlobAttr& rBlob) { return true; }
    virtual bool toBlobAttr(BlobAttr* pBlob) { return true; }
    virtual bool toPreviewBlobAttr(BlobAttr* pBlob) { return true; }

    // 属性计算刚刚计算完成
    bool checkNeedRefreshBuff() { return m_bNeedRefreshBuff && m_dwCollectType == 0; }
    void setRefreshBuff() { m_dwCollectType |= ECOLLECTTYPE_DYNAMIC_BUFF; m_bNeedRefreshBuff = false; }

    void modifyCollect(ECollectType eCollect, const UserAttrSvr& rAttr, EAttrOper eOper = EATTROPER_ADD);
    bool test(EAttrType eType);
  protected:
    bool initAttr(TVecAttrSvrs& vecAttrs);
    void modifyAttr(TVecAttrSvrs& vecLast, const TVecAttrSvrs& vecCur);

    virtual void collectBaseAttr();
    //virtual void collectSkillAttr();
    virtual void collectEquipAttr();
    //virtual void collectCardAttr();
    virtual void collectBuffAttr(bool bDynamic);
    virtual void collectCharacterAttr();
    virtual void collectGuildAttr();
    virtual void collectAstrolabe();
    virtual void collectAchieveTitle();
    virtual void collectFood();
    virtual void collectProfession();
  protected:
    bitset<EATTRTYPE_MAX> m_bitset;

    TVecAttrSvrs m_vecFinalAttrs;
    TVecAttrSvrs m_vecFinalPointAttrs;

    TVecAttrSvrs m_vecSyncFinalAttrs;
    TVecAttrSvrs m_vecSyncFinalPointAttrs;

    TVecAttrSvrs m_vecBase;
    //TVecAttrSvrs m_vecSkill;
    TVecAttrSvrs m_vecEquip;
    //TVecAttrSvrs m_vecCard;
    TVecAttrSvrs m_vecDynamicBuff;
    TVecAttrSvrs m_vecStaticBuff;

    TVecAttrSvrs m_vecCharacter;
    TVecAttrSvrs m_vecGM;
    TVecAttrSvrs m_vecTimeBuff;
    TVecAttrSvrs m_vecGuild;
    TVecAttrSvrs m_vecAstrolabe;
    TVecAttrSvrs m_vecAchieveTitle;
    TVecAttrSvrs m_vecFood;
    TVecAttrSvrs m_vecProfession;

    TVecAttrSvrs m_vecShow;

    TVecAttrSvrs m_vecLastBuff;
    TVecAttrSvrs m_vecLastEquip;
    TVecAttrSvrs m_vecLastGuild;
    TVecAttrSvrs m_vecLastStaticBuff;
    TVecAttrSvrs m_vecLastDynamicBuff;

    DWORD m_dwCollectType = 0;
    DWORD m_dwEquipMasterLv = 0;
    DWORD m_dwRefineMasterLv = 0;

    // 所有属性计算完成, 重新刷一遍buff属性
    bool m_bNeedRefreshBuff = false;

    xSceneEntryDynamic* m_pEntry = nullptr;
};

// npc attr
class SceneNpc;
class NpcAttribute : public Attribute, public xObjectPool<NpcAttribute>
{
  public:
    NpcAttribute(SceneNpc* pNpc);
    virtual ~NpcAttribute();

    virtual void updateAttribute();
    void collectSyncAttrCmd(NpcDataSync& rCmd);
    void toData(MapNpc* cmd);
  private:
    virtual void collectBaseAttr();
    virtual void collectCharacterAttr();
  private:
    SceneNpc* m_pNpc = nullptr;
};

// user attr
class SceneUser;
class UserAttribute : public Attribute, public xObjectPool<UserAttribute>
{
  public:
    UserAttribute(SceneUser* pUser);
    virtual ~UserAttribute();

    virtual void updateAttribute();
    void collectSyncAttrCmd(UserSyncCmd& rCmd, UserDataSync& session, UserNineSyncCmd& nine);
    virtual bool fromBlobAttr(const BlobAttr& rBlob);
    virtual bool toBlobAttr(BlobAttr* pBlob);
    virtual bool toPreviewBlobAttr(BlobAttr* pBlob);
    void setPrayLevelup(bool b) { m_bPrayLevelup = b; }
  private:
    virtual void collectBaseAttr();
    virtual void collectEquipAttr();
    virtual void collectGuildAttr();
    virtual void collectAstrolabe();
    virtual void collectAchieveTitle();
    virtual void collectFood();
    virtual void collectProfession();
  private:
    void updateMonsterAttr();
    void collectMonsterAttr();

    bool needSync(EAttrType eType);
    void setSyncAttr(EAttrType eType, float fValue);
  private:
    SceneUser* m_pUser = nullptr;
    bool m_bPrayLevelup = false;
    DWORD m_dwAttrIndex = 0;
    DWORD m_dwLockMeNum = 0;
};


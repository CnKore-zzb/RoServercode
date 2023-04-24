#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "FighterSkill.h"
#include <list>
using namespace std;
using std::map;
using std::vector;
using std::list;

enum ETransformType
{
  ETRANSFORM_NORMAL = 1,
  ETRANSFROM_POLIFIRE = 2,
  ETRANSFORM_ALTMAN = 3,
};

class SceneUser;
class Transform
{
  public:
    Transform(SceneUser* pUser);
    ~Transform();

  public:
    DWORD getMonsterID() const { return m_dwMonsterID; }
    //void setTransform(bool flag, DWORD monsterid) { m_bInTransform = flag; m_dwMonsterID = monsterid; }

    bool isInTransform() { return m_bInTransform; }
    bool isMonster() { return m_bInTransform && m_dwMonsterID != 0; }

    void enterTransform(DWORD monsterid, ETransformType etype = ETRANSFORM_NORMAL);
    void exitTransform() { m_bInTransform = false; }
    bool checkSkill(DWORD skillid) const;

    bool addSkill(DWORD skillid);
    void delSkill(DWORD skillid);
    void onUseSkill(DWORD skillid);
    void clearSkill();

    void addReplaceSkill(DWORD oldid, DWORD newid);
    void delReplaceSkill(DWORD oldid, DWORD newid);

    DWORD getNormalSkillID() const;

    void getPoliFireDropSkills(TVecDWORD& vec);

    ETransformType getTransformType() { return m_eType; }
    bool canPickUp();
  private:
    void sendSkillInfo();
  private:
    DWORD m_dwMonsterID = 0;
    bool m_bInTransform = false;
    SceneUser* m_pUser = nullptr;
    ETransformType m_eType = ETRANSFORM_NORMAL;
    map<DWORD, DWORD> m_mapReplaceSkill;

    list<SSkillItem> m_listSkillItems;

    vector<SSkillItem> m_vecSkillItems;
};

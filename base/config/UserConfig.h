/**
 * @file UserConfig.h
 * @brief table config for user
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-04-29
 */

#pragma once

#include "xSingleton.h"

using namespace Cmd;
using std::vector;
using std::string;
using std::pair;

// base level config
struct SUserBaseLvCFG
{
  DWORD   lv;
  DWORD   needExp;
  DWORD   point;

  SUserBaseLvCFG() : lv(0), needExp(0), point(0) {}
};
typedef vector<SUserBaseLvCFG> TVecUserBaseLvCFG;

// job level config
struct SUserJobLvCFG
{
  DWORD   lv;
  DWORD   needExp;

  SUserJobLvCFG() : lv(0), needExp(0) {}
};
typedef vector<SUserJobLvCFG> TVecUserJobLvCFG;

struct SBranchCFG
{
  DWORD id;
  DWORD baseId;
  DWORD itemId;
  DWORD taskProfession;
  DWORD taskPeak;
  DWORD professionPeak;
  DWORD giftBranch;
  Cmd::EGender eGender;

  vector<DWORD> vecProfession;
  vector<DWORD> vecDelTask;
  vector<DWORD> vecJobSkill;
  vector<DWORD> vecBrotherId;

  bool hasProfession(DWORD dwProfession) const { auto it = std::find(vecProfession.begin(), vecProfession.end(), dwProfession); return vecProfession.end() != it;}
  bool checkGender(Cmd::EGender gender) const
  {
    if(Cmd::EGENDER_MIN == eGender)
      return true;
    return eGender == gender;
  }
};
typedef std::map<DWORD, SBranchCFG> TMapBranchCFG;

// attr point
typedef pair<DWORD, DWORD> PAttr2Point;
typedef vector<PAttr2Point> TVecAttrPoint;

// role base
typedef vector<EProfession> TVecProfession;
typedef pair<EProfession, TVecDWORD> TPairRoleSkill;
typedef vector<TPairRoleSkill> TVecRoleSkill;
struct SRoleUnlockCFG
{
  DWORD dwJobLv = 0;

  DWORD dwMaleMailID = 0;
  DWORD dwFemaleMailID = 0;
};
typedef vector<SRoleUnlockCFG> TVecRoleUnlockCFG;
struct SRoleBaseCFG
{
  EProfession profession = EPROFESSION_MIN;

  DWORD maleBody = 0;
  DWORD femaleBody = 0;
  DWORD maleEye = 0;
  DWORD femaleEye = 0;
  DWORD defaultWeapon = 0;

  DWORD normalSkill = 0;
  DWORD strengthSkill = 0;
  DWORD maxJobLv = 0;

  DWORD maxSkillPos = 0;

  DWORD dwType = 0;
  DWORD dwTypeBranch = 0;
  DWORD dwPeakJobLv = 0;
  Cmd::EGender eGender = Cmd::EGENDER_MIN; // 性别要求（舞娘：女 诗人：男）

  bool bRideAction = false;

  std::pair<float, float> damRandom;

  TVecProfession vecAdvancePro;
  TVecRoleSkill vecEnableSkill;
  TVecRoleUnlockCFG vecUnlock;
  TVecAttrSvrs vecUnlockAttr;

  string strName;
  EProfession ePreProfession = EPROFESSION_MIN;

  SRoleBaseCFG() {}

  bool haveSkill(DWORD id) const;
  bool canExchange(EProfession eProfession) const;
  bool addSkill(EProfession eProfession, DWORD id);
  bool checkGender(Cmd::EGender gender) const;
};
typedef vector<SRoleBaseCFG> TVecRoleBaseCFG;


class RoleConfig : public xSingleton<RoleConfig>
{
  friend class xSingleton<RoleConfig>;
  private:
    RoleConfig();
  public:
    virtual ~RoleConfig();

    bool loadConfig();
    bool loadProfessionConfig();

    const SRoleBaseCFG* getRoleBase(EProfession profession);
    EProfession getBaseProfession(EProfession eProfession);
    EProfession getTypeProfession(EProfession eProfession);
    void getAdvanceProPath(EProfession pro, TVecProfession& path);

    bool isNormalSkill(DWORD skillid);
    bool isStrengthSkill(DWORD skillid);
    bool isFirstProfession(EProfession profession);
    DWORD getProfessionNum(std::set<EProfession>& setPro);

    const SBranchCFG* getBranchCFG(DWORD id);

  private:
    TVecRoleBaseCFG m_vecRoleBaseCFG;
    TMapBranchCFG m_mapBranchCFG;
};

class AttributePointConfig : public xSingleton<AttributePointConfig>
{
  friend class xSingleton<AttributePointConfig>;
  private:
    AttributePointConfig();
  public:
    virtual ~AttributePointConfig();

    bool loadConfig();

    const PAttr2Point* getAttrPointCFG(DWORD value);

  private:
    TVecAttrPoint m_vecAttrPoint;
};

class BaseLevelConfig : public xSingleton<BaseLevelConfig>
{
  friend class xSingleton<BaseLevelConfig>;
  private:
    BaseLevelConfig();
  public:
    virtual ~BaseLevelConfig();

    bool loadConfig();

    const SUserBaseLvCFG* getBaseLvCFG(DWORD lv);

  private:
    TVecUserBaseLvCFG m_vecBaseLvCFG;
};

class JobLevelConfig : public xSingleton<JobLevelConfig>
{
  friend class xSingleton<JobLevelConfig>;
  private:
    JobLevelConfig();
  public:
    virtual ~JobLevelConfig();

    bool loadConfig();

    const SUserJobLvCFG* getJobLvCFG(DWORD lv);

  private:
    TVecUserJobLvCFG m_vecJobLvCFG;
};


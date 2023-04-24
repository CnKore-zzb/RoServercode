/**
 * @file FeatureConfig.h
 * @brief
 * @author gengshengjie, gengshengjie@xindong.com
 * @version 1
 * @date 2015-11-16
 */
#pragma once

#include "xSingleton.h"

using namespace Cmd;
using std::string;
using std::vector;
using std::pair;

struct SNpcTeamAttack
{
  DWORD dwRange = 0;
  DWORD dwResponseTime = 0;
  DWORD dwEmojiID = 0;
};

struct SNpcGhost
{
  DWORD dwFlashDamage = 0;
  TVecDWORD vecBuff;
  //DWORD dwBirthBuff = 0;
  //DWORD dwAttBuff = 0;
};

struct SNpcDemon
{
  DWORD dwBirthBuff = 0;
  DWORD dwDizzyVal = 0;
};

struct SNpcGoCamera
{
  DWORD dwRangeFind = 0;
  DWORD dwRangeStop = 0;
  DWORD dwInterval = 0;
  DWORD dwBuff = 0;
};

struct SNpcSmile
{
  DWORD dwRange = 0;
  DWORD dwInterval = 0;
  DWORD dwBuff = 0;
  DWORD dwStayTime = 0;
};

struct SNpcNaughty
{
  DWORD dwFormerEmoji = 0;
  DWORD dwLatterEmoji = 0;
  DWORD dwResponseTime = 0;
  DWORD dwSkillID = 0;
};

struct SNpcAlert
{
  DWORD dwRange = 0;
  DWORD dwResponseTime = 0;
  DWORD dwSkillID = 0;
  DWORD dwEmoji = 0;
  DWORD dwInterval = 0;
};

struct SNpcExpel
{
  DWORD dwRange = 0;
  DWORD dwInterval = 0;
  DWORD dwSkillID = 0;
  DWORD dwEmoji = 0;
};

struct SNpcFly
{
  TVecDWORD vecImmuneSkill;
  DWORD dwBirthBuff = 0;
};

struct SNpcJealous
{
  DWORD dwRange = 0;
  DWORD dwEmoji = 0;
};

struct SNpcReactCFG
{
  DWORD dwRange = 0;
};

struct SNpcLeaveBattle
{
  DWORD dwNormalRange = 0;
  DWORD dwNormalTime = 0;
  DWORD dwMvpRange = 0;
  DWORD dwMvpTime = 0;
};

struct SNpcNightCFG
{
  TVecDWORD vecBuffs;
};

struct SNpcEndureCFG
{
  DWORD dwCD = 0;
  DWORD dwSkill = 0;
};

struct SNpcStatusAttack
{
  DWORD dwFindRange = 0;
  vector<pair<DWORD, DWORD>> vecStatus2SKill;
  DWORD getSkillByStatus(DWORD status) const{
    auto v = find_if(vecStatus2SKill.begin(), vecStatus2SKill.end(), [&status](const pair<DWORD, DWORD>& r) { return r.first == status; });
    return v != vecStatus2SKill.end() ? v->second : 0;
  }
  SNpcStatusAttack() {}
};

struct SNpcServant
{
  DWORD dwKeepDis = 0;
};

struct SNpcAIParams
{
  DWORD dwRunAwayTime = 0;
};

// NpcAI
struct SNpcAICFG
{
  DWORD dwPickupRange = 0;
  DWORD dwPickupMaxItem = 0;
  DWORD dwTeamHelpRange = 0;
  DWORD dwTeamHelpHp = 0;
  TVecDWORD vecGoBackBuff;

  SNpcTeamAttack dwTeamAttack;
  SNpcGhost stGhost;
  SNpcGoCamera dwGoCamera;
  SNpcAlert dwAlert;
  SNpcDemon dwDemon;
  SNpcExpel dwExpel;
  SNpcSmile dwSmile;
  SNpcFly dwFly;
  SNpcNaughty dwNaughty;
  SNpcJealous stJealous;
  SNpcReactCFG stReaction;
  SNpcLeaveBattle stLeaveBattle;
  SNpcStatusAttack stStatusAttack;
  SNpcNightCFG stNightCFG;
  SNpcEndureCFG stEndureCFG;
  SNpcServant stServant;
  SNpcAIParams stAIParams;

  SNpcAICFG() {}
};

class FeatureConfig : public xSingleton<FeatureConfig>
{
  public:
    FeatureConfig();
    virtual ~FeatureConfig();

    bool loadConfig();
    const SNpcAICFG& getNpcAICFG() { return m_stNpcAICFG; }
  private:
    SNpcAICFG m_stNpcAICFG;
};



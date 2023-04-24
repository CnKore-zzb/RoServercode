/**
 * @file SkillManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-10-21
 */

#pragma once

#include "xSingleton.h"
#include "SkillItem.h"
#include "SkillConfig.h"

using std::map;

typedef map<DWORD, BaseSkill*> TMapSkillItemCFG;

class SkillManager : public xSingleton<SkillManager>
{
  friend class xSingleton<SkillManager>;
  private:
    SkillManager();
  public:
    virtual ~SkillManager();

    bool init();

    const BaseSkill* getSkillCFG(DWORD dwID) const;
  private:
    BaseSkill* createSkill(DWORD dwID, ESkillType eType, ESkillLogic eLogic, ESkillCamp eCamp, xLuaData& params);

    void clear();
    bool reload();
  private:
    TMapSkillItemCFG m_mapSkillItemCFG;
};


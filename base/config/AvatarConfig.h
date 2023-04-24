/**
 * @file AvatarConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-06-24
 */

#pragma once

#include "xSingleton.h"

using std::map;

const int STATIC_EXPRESSION_LAUGH = 17;
const int STATIC_EXPRESSION_HEART = 2;
const int STATIC_EXPRESSION_KISS = 4;

enum EAvatarExpression
{
  EAVATAREXPRESSION_MIN = 0,
  EAVATAREXPRESSION_BLINK = 1,
  EAVATAREXPRESSION_HIT = 2,
  EAVATAREXPRESSION_SMILE = 3,
  EAVATAREXPRESSION_DEAD = 4,
  EAVATAREXPRESSION_CLOSEEYE = 5,
  EAVATAREXPRESSION_DBUFF_LOOP = 6,
  EAVATAREXPRESSION_MAX,
};

// config data
struct SAvatarCFG
{
  EAvatarExpression eExpression = EAVATAREXPRESSION_MIN;

  DWORD dwRate = 0;
  DWORD dwAdventureSkill = 0;

  SAvatarCFG() {}
};
typedef map<EAvatarExpression, SAvatarCFG> TMapAvatarCFG;

// config
class AvatarConfig : public xSingleton<AvatarConfig>
{
  friend class xSingleton<AvatarConfig>;
  private:
    AvatarConfig();
  public:
    virtual ~AvatarConfig();

    bool loadConfig();
    bool checkConfig();

    const SAvatarCFG* getAvatarCFG(EAvatarExpression eExpression) const;
  private:
    TMapAvatarCFG m_mapAvatarCFG;
};


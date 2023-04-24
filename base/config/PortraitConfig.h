/**
 * @file PortraitConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-09-17
 */

#pragma once

#include "xSingleton.h"
#include "ProtoCommon.pb.h"

using std::vector;
using namespace Cmd;

// config
enum EPortraitType
{
  EPORTRAITTYPE_MIN = 0,
  EPORTRAITTYPE_USERPORTRAIT = 1,
  EPORTRAITTYPE_PETPORTRAIT = 2,
  EPORTRAITTYPE_FRAME = 3,
  EPORTRAITTYPE_MAX
};
struct SPortraitCFG
{
  DWORD dwID = 0;

  EPortraitType eType = EPORTRAITTYPE_MIN;
  EGender eGender = EGENDER_MIN;

  SPortraitCFG() {}
};
typedef vector<SPortraitCFG> TVecPortraitCFG;

// config
class PortraitConfig : public xSingleton<PortraitConfig>
{
  public:
    PortraitConfig();
    virtual ~PortraitConfig();

    bool loadConfig();

    const SPortraitCFG* getPortraitCFG(DWORD id) const;
    template<class T> void foreach(T func) { for_each(m_vecPortraitCFG.begin(), m_vecPortraitCFG.end(), [func](const SPortraitCFG& r) {func(r);}); }
  private:
    TVecPortraitCFG m_vecPortraitCFG;
};

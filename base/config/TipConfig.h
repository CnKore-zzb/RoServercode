/**
 * @file TipConfig.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-01-18
 */

#pragma once

#include "xSingleton.h"
#include "SceneTip.pb.h"

using namespace Cmd;
using std::vector;

// config data
enum ETipTriggerType
{
};

struct STipCFG
{
  ERedSys eRedType = EREDSYS_MIN;
  DWORD dwLocalRemote = 0;
};
typedef vector<STipCFG> TVecTipCFG;

// config
class TipConfig : public xSingleton<TipConfig>
{
  friend class xSingleton<TipConfig>;
  private:
    TipConfig();
  public:
    virtual ~TipConfig();

    bool loadConfig();
    bool checkConfig();

    const STipCFG* getTipConfig(ERedSys eType) const;
    const TVecTipCFG& getTipCFGList() const { return m_vecTipCFG; }
  private:
    TVecTipCFG m_vecTipCFG;
};


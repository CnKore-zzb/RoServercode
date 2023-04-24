/**
 * @file TowerManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-12-08
 */

#pragma once

#include "xSingleton.h"
#include "TowerConfig.h"
#include "SessionCmd.pb.h"
#include "Var.h"

class xNetProcessor;

using namespace Cmd;
using std::vector;

// tower mangaer
class TowerManager : public xSingleton<TowerManager>
{
  friend class xSingleton<TowerManager>;
  private:
    TowerManager();
  public:
    virtual ~TowerManager();

    bool loadDataFromDB();

    void syncScene(xNetProcessor* pTask = nullptr);
    void saveData();
    void timer(DWORD curTime);
    void refreshMonsterGM() { m_oVar.setVarValue(EVARTYPE_TOWER_MONSTER, 0); }
  private:
    void toSceneData(TowerInfo* pInfo);
    bool toDataString(string& str);
  private:
    bool m_bInit = false;
    Variable m_oVar;
};


/**
 * @file GlobalManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-08-03
 */

#pragma once

#include "xSingleton.h"
#include "BossSCmd.pb.h"

class GlobalManager : public xSingleton<GlobalManager>
{
  friend class xSingleton<GlobalManager>;
  private:
    GlobalManager();
  public:
    virtual ~GlobalManager();

    bool loadGlobalData();

    const Cmd::DeadBossInfo& getGlobalBoss() const { return m_oGlobalBoss; }
    bool updateGlobalBoss(const Cmd::DeadBossInfo& rInfo);
  private:
    bool loadGlobalBoss();

    void parseGlobalBoss(const string& str);
    void serialGlobalBoss(string& str);
  private:
    Cmd::DeadBossInfo m_oGlobalBoss;
};


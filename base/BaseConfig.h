/**
 * @file BaseConfig.h
 * @brief base config
 * @author liuxin, liuxin@xindong.com
 * @version v1
 * @date 2015-09-10
 */

#pragma once

#include "xSingleton.h"
#include "xLuaTable.h"

enum
{
  BRANCH_DEBUG = 0,
  BRANCH_TF = 1,
  BRANCH_PUBLISH = 2,
};

class BaseConfig : public xSingleton<BaseConfig>
{ 
  public:
    BaseConfig();
    ~BaseConfig();

  public:
    bool loadConfig(bool isTest = false);
    bool configCheck() const { return m_oData.getTableInt("configcheck") != 0; }
    bool versionCheck() const { return m_oData.getTableInt("versioncheck") != 0; }
    bool needBuildNavMesh(std::string &str);
    const std::string& getTapdbAppid() const { return m_tapdbAppid; }
    DWORD getTradeGiveStartTime() const { return m_dwTradeGiveStartTime; }
    const std::string& getStrTradeGiveTime() const { return m_strTradeGiveStartTime; }
    const std::string& getPlatIp() const { return m_strPlatIp; }
    DWORD getPlatPort() const { return m_dwPlatPort; }

  public:
    xLuaData m_oData;

  public:
    const xLuaData& getBranchData() { return m_oBranchData; }
    DWORD getBranch() { return m_dwBranchID; }
    DWORD getInnerBranch();

  private:
    DWORD m_dwBranchID = BRANCH_DEBUG;
    DWORD m_dwInnerBranchID = BRANCH_DEBUG;
    xLuaData m_oBranchData;
    std::string m_tapdbAppid;
    std::string m_strPlatIp;
    DWORD m_dwPlatPort;
    DWORD m_dwTradeGiveStartTime = 0;
    std::string m_strTradeGiveStartTime;
};

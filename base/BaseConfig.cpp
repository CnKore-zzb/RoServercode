#include "BaseConfig.h"
#include "xLog.h"

BaseConfig::BaseConfig()
{

}

BaseConfig::~BaseConfig()
{

}

bool BaseConfig::loadConfig(bool isTest)
{
  // BranchConfig
  {
    if (!xLuaTable::getMe().open("branch.lua"))
    {
      XERR << "[BranchConfig], 加载配置branch.lua失败" << XEND;
      return false;
    }

    xLuaData data;
    xLuaTable::getMe().getLuaData("BranchID", data);
    m_dwBranchID = data.getTableInt("id");
    m_dwInnerBranchID = data.getTableInt("inner_id");
    XLOG << "[BranchConfig]" << "加载" << m_dwBranchID << XEND;
  }

  {
    if (!xLuaTable::getMe().open("BranchConfig.lua"))
    {
      XERR << "[BranchConfig]" << "加载配置BranchConfig.lua失败" << XEND;
      return false;
    }
    xLuaData data;
    xLuaTable::getMe().getLuaData("BranchConfig", data);
    switch (m_dwBranchID)
    {
      case BRANCH_DEBUG:
        {
          m_oBranchData = data.getData("Debug");
        }
        break;
      case BRANCH_TF:
        {
          if (isTest)
          {
            m_oBranchData = data.getData("TF_Test");
          }
          else
          {
            m_oBranchData = data.getData("TF");
          }
        }
        break;
      case BRANCH_PUBLISH:
        {
          if (isTest)
          {
            m_oBranchData = data.getData("Publish_Test");
          }
          else
          {
            m_oBranchData = data.getData("Publish");
          }
        }
        break;
      default:
        {
          XERR << "[BranchConfig]" << "加载BranchConfig.lua失败,无效的分支" << m_dwBranchID << XEND;
          return false;
        }
    }
  }

  if (!xLuaTable::getMe().open("base.lua"))
  {
    XERR << "[BaseConfig], 加载配置base.lua失败" << XEND;
    return false;
  }

  xLuaTable::getMe().getLuaData("BaseConfig", m_oData);

  m_tapdbAppid = m_oBranchData.getTableString("tapdb_appid");
  m_strPlatIp = m_oBranchData.getTableString("plat_ip");
  m_dwPlatPort = m_oBranchData.getTableInt("plat_port");
  m_strTradeGiveStartTime = m_oBranchData.getTableString("trade_give_starttime");
  if (!m_strTradeGiveStartTime.empty())
  {
    parseTime(m_strTradeGiveStartTime.c_str(), m_dwTradeGiveStartTime);
  }
  
  XLOG << "[BaseConfig] 加载配置base.lua trade_give_starttime"<< m_strTradeGiveStartTime << "m_dwTradeGiveStartTime"<< m_dwTradeGiveStartTime << XEND;
  return true;
  // DWORD check = m_oData.getTableInt("configcheck");
  // return check;
}

bool BaseConfig::needBuildNavMesh(std::string &str)
{
  const xLuaData &data = m_oData.getData("BuildNavMesh");
  bool isEmpty = true;
  for (auto it : data.m_table)
  {
    isEmpty = false;
    if (it.second.getString() == str) return true;
  }

  if (isEmpty) return true;

  return false;
}

DWORD BaseConfig::getInnerBranch()
{
  if(m_dwBranchID == BRANCH_DEBUG)
    return m_dwInnerBranchID;

  return m_dwBranchID;
}


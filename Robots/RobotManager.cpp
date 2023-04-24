#include "RobotManager.h"
#include "xNetProcessor.h"
#include "Robot.h"
#include "xLuaTable.h"

RobotManager::RobotManager():m_oOneSecTimer(1), m_eFuncType(EFUNCTYPE_SKILL)
{
}

RobotManager::~RobotManager()
{
}

bool RobotManager::add(Robot *robot)
{
  if (!robot) return false;

  return addEntry(robot);
}

bool RobotManager::del(Robot *robot)
{
  if (!robot) return false;

  removeEntry(robot);
  SAFE_DELETE(robot);

  return true;
}

void RobotManager::timer()
{
  DWORD cur = now();
  if (m_oOneSecTimer.timeUp(cur))
  {
    for (auto &it : xEntryID::ets_)
    {
      Robot *pRobot = (Robot *)it.second;
      if (pRobot)
      {
        pRobot->timer(cur);
      }
    }
  }
}

void RobotManager::process()
{
  xNetProcessor *np;

  for (auto &it : xEntryID::ets_)
  {
    Robot *pRobot = (Robot *)it.second;
    if (pRobot)
    {
      np = pRobot->m_pTask;
      if (np)
      {
        CmdPair *pair = np->getCmd();
        while (pair)
        {
          if (!pRobot->doServerCmd((xCommand *)(pair->second), pair->first))
          {
            XDBG << "[Robot消息处理]" << pRobot->id << pRobot->name << "error," << ((xCommand *)(pair->second))->cmd << ((xCommand *)(pair->second))->param << XEND;
            // std::cout << pRobot->id << " 消息处理错误:" << (DWORD)((xCommand *)(pair->second))->cmd << "," << (DWORD)((xCommand *)(pair->second))->param << "," << pair->first << std::endl;
          }
          // std::cout << pRobot->id << " 消息处理:" << (DWORD)((xCommand *)(pair->second))->cmd << "," << (DWORD)((xCommand *)(pair->second))->param << "," << pair->first << std::endl;
          np->popCmd();
          pair = np->getCmd();
        }
      }
    }
  }
}

void RobotManager::onClose(xNetProcessor* np)
{
  if (!np) return;

  for (auto &it : xEntryID::ets_)
  {
    Robot *pRobot = (Robot *)it.second;
    if (pRobot->m_pTask == np)
    {
      pRobot->m_pTask = nullptr;
      XLOG << "[连接]" << pRobot->id << pRobot->m_dwRegionID << "关闭游戏服连接" << XEND;
      return;
    }
  }
}

void RobotManager::init()
{
  if (!loadConfig()) return;
  if (!loadData()) return;
}

bool RobotManager::loadConfig()
{
  if (!xLuaTable::getMe().open("robot.lua"))
  {
    XERR<<"[RobotManager] load robot.lua failed!"<<XEND;
    return false;
  }

  xLuaData data;
  xLuaTable::getMe().getLuaData("ParamList", data);

  m_strProxyIp = data.getTableString("proxy_ip");
  m_dwProxyPort = data.getTableInt("proxy_port");
  m_dwZoneId = data.getTableInt("zone_id");
  m_qwAccId = data.getTableInt("acc_id");
  m_dwCount = data.getTableInt("count");
  m_dwMapID = data.getTableInt("mapid");
  m_dwRange = data.getTableInt("range");
  m_eFuncType = static_cast<EFuncType>(data.getTableInt("func_type"));

  xLuaData& pos = data.getMutableData("pos");
  m_posCenter.set_x(pos.getTableInt("1"));
  m_posCenter.set_y(pos.getTableInt("2"));
  m_posCenter.set_z(pos.getTableInt("3"));

  return true;
}

bool RobotManager::loadData()
{
  if (m_strProxyIp.empty() || !m_dwProxyPort || !m_dwZoneId || !m_dwCount || !m_qwAccId)
  {
    XERR << "[RobotManager] loadData failed! robot.lua config error!" << XEND;
    return false;
  }

  for (DWORD index=0; index<m_dwCount; index++)
  {
    Robot *pRobot = new Robot(m_qwAccId+index, m_dwZoneId);
    add(pRobot);
  }

  return true;
}

void RobotManager::getScenePos(Cmd::ScenePos& pos)
{
  pos.set_x(randBetween(m_posCenter.x() - m_dwRange, m_posCenter.x() + m_dwRange));
  pos.set_z(randBetween(m_posCenter.z() - m_dwRange, m_posCenter.z() + m_dwRange));
  pos.set_y(m_posCenter.y());
}


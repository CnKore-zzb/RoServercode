#include "TimerM.h"
#include "TableManager.h"
#include "SessionServer.h"
#include "GMCommandManager.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "BaseConfig.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::init(const TimerBase *base)
{
  m_pBase = base;
  if (m_pBase)
  {
    if (!m_pBase->getStartTime().empty())
      parseTime(m_pBase->getStartTime().c_str(), m_dwStartTime);
    if (!m_pBase->getEndTime().empty())
      parseTime(m_pBase->getEndTime().c_str(), m_dwEndTime);

    DWORD cur = now();
    //m_dwRepeatTime = 0;
    if (inTime(cur))
    {
      if ((cur>m_dwStartTime) && m_pBase->getInterval())
      {
        m_dwRepeatTime = ((cur - m_dwStartTime) / (m_pBase->getInterval())) + 1;
      }
    }
    XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "初始化" << m_pBase->getStartTime() << "(" << m_dwStartTime << ")"
      << m_pBase->getEndTime() << "(" << m_dwEndTime << ")" << "Repeat:" << m_dwRepeatTime << XEND;
    if (inTime(cur))
    {
      setState(TimerState::start);
    }
  }
}

void Timer::setState(TimerState state)
{
  if (!m_pBase) return;

  m_oState = state;
  switch (m_oState)
  {
    case TimerState::start:
      {
        XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "设置开始状态" << XEND;
      }
      break;
    case TimerState::end:
      {
        XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "设置结束状态" << XEND;
      }
      break;
    case TimerState::wait:
      {
        XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "设置等待状态" << XEND;
      }
      break;
    default:
      break;
  }

}

void Timer::check(DWORD cur)
{
  if (!m_pBase) return;

  if (inTime(cur))
  {
    if (m_dwRepeatTime < m_pBase->getRepeat() + 1)
    {
      DWORD curProcessTime = 1;
      if (m_pBase->getInterval())
        curProcessTime = ((cur - m_dwStartTime) / (m_pBase->getInterval())) + 1;

      switch (m_oState)
      {
        case TimerState::wait:
        case TimerState::start:
          {
            if (curProcessTime > m_dwRepeatTime)
            {
              setState(TimerState::start);

              if (m_dwRepeatTime == 0)
              {
                exec(TimerType::start);
                XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "开始" << "Repeat:" << curProcessTime << XEND;
              }
              else
              {
                exec(TimerType::reapeat);
                XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "循环开始" << "Repeat:" << curProcessTime << XEND;
              }
              m_dwRepeatTime = curProcessTime;
            }
          }
          break;
        default:
          break;
      }
    }
  }
  else
  {
    if (TimerState::start==m_oState)
    {
      m_oState = TimerState::end;
      exec(TimerType::end);

      XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "结束" << "Repeat:" << m_dwRepeatTime << XEND;
    }
  }
}

bool Timer::inTime(DWORD cur)
{
  if ((m_dwStartTime && m_dwStartTime <= cur) && (!m_dwEndTime || m_dwEndTime > cur))
    return true;

  return false;
}

DWORD Timer::getCmdKeyID()
{
  if (!m_pBase)
    return 0;
  DWORD count = m_pBase->getInterval() ? m_dwRepeatTime : 0;
  return m_pBase->getTableInt("id") * 10000 + count * 1000 + (DWORD)m_oState;
}

void Timer::exec(TimerType type, ServerTask *task)
{
  static xLuaData cmd;
  cmd.clear();
  TSetDWORD extraMap;
  switch (type)
  {
    case TimerType::start:
      {
        if (task)
          break;
        cmd = m_pBase->getStartCmd();
      }
      break;
    case TimerType::restart:
      {
        if (!task)
          break;
        cmd = m_pBase->getRestartCmd();
        if (cmd.isTable())
        {
          if (cmd.has("map"))
          {
            xLuaData& rMap = cmd.getMutableData("map");          
            auto func = [&](const string& key, xLuaData& data)
            {
              DWORD dwMapID = data.getInt();
              SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(dwMapID);
              if (pScene && pScene->getTask() == task)
              {
                extraMap.insert(dwMapID);
              }
            };
            rMap.foreach(func);
            if (extraMap.empty())
            {
              cmd.clear();
              break;
            }
          }
        }
      }
      break;
    case TimerType::reapeat:
      {
        if (task)
          break;
        cmd = m_pBase->getReapeatCmd();
      }
      break;
    case TimerType::end:
      {
        if (task)
          break;
        cmd = m_pBase->getEndCmd();
      }
      break;
    default:
      break;
  }
  if (!cmd.isTable()) return;

  cmd.setData("Timer_Key", getCmdKeyID());
  cmd.setData("Timer_Id", m_pBase->id);

  std::stringstream stream;
  stream.str("");
  cmd.toString(stream);

  if (GMCommandManager::getMe().execute(cmd, extraMap) == false)
  {
    XERR << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "执行指令失败" << (DWORD)type << stream.str() << "Repeat:" << m_dwRepeatTime << XEND;
    return;
  }

  XLOG << "[定时器]" << m_pBase->id << m_pBase->getInfo() << "执行指令" << (DWORD)type << stream.str() << "Repeat:" << m_dwRepeatTime << XEND;
}

/**************************************************/
/**************************************************/
/**************************************************/

TimerM::TimerM()
{
}

TimerM::~TimerM()
{
}

void TimerM::sceneStart(ServerTask *task)
{
  if (!task) return;

  std::string sceneName = task->getTask()->name;
  auto it = m_setOkScene.find(sceneName);
  //第一次启动不算重启
  if (it == m_setOkScene.end())
  {
    m_setOkScene.insert(sceneName);
    return;
  }
  
  //restart， scene 重启，session没重启
  DWORD curTime = now();

  for (auto &it : m_list)
  {
    if (it.second.inTime(curTime))
    {
      it.second.exec(TimerType::restart, task);
    }
  }
}

bool TimerM::load()
{
  Table<TimerBase>* pTimerBaseM = TableManager::getMe().m_pTimerBaseM;
  TSetDWORD setHas;
  if (pTimerBaseM->load())
  {
    auto func = [&](const xEntry* pEntry)
    {
      const TimerBase* pBase = static_cast<const TimerBase *>(pEntry);
      if (pBase == nullptr)
        return;

      DWORD branchID = pBase->getServerManager();
      switch (branchID)
      {
        case BRANCH_DEBUG:
          {
            if(BaseConfig::getMe().getInnerBranch() != branchID)
              return;
          }
          break;
        case BRANCH_TF:   //内网+预言之地(策划要求)
          {
            if(BaseConfig::getMe().getInnerBranch() == BRANCH_PUBLISH)
              return;
          }
          break;
        case BRANCH_PUBLISH:
          {
            if(BaseConfig::getMe().getInnerBranch() != branchID)
              return;
          }
          break;
        default:
          return;
      }

      this->add(pBase);
      setHas.insert(pBase->id);
    };
    pTimerBaseM->foreachNoConst(func);
    for (auto it = m_list.begin(); it != m_list.end();)
    {
      if (setHas.find(it->first) == setHas.end())
      {
        XLOG << "[Timer-重加载] 移除已经不存在的配置，id" << it->first << XEND;
        it = m_list.erase(it);
        continue;
      }
      ++it;
    }
  }
  return true;
}

void TimerM::add(const TimerBase *base)
{
  if (!base || !base->id) return;

  auto it = m_list.find(base->id);
  if (it != m_list.end())
  {
    it->second.init(base);
    return;
  }

  m_list[base->id].init(base);
}

void TimerM::timer(DWORD curTime)
{
  for (auto &it : m_list)
  {
    it.second.check(curTime);
  }
}

bool TimerM::getStartEndTime(DWORD dwId, DWORD& dwStartTime, DWORD& dwEndTime)
{
  dwStartTime = 0;
  dwEndTime = 0;
  
  auto it = m_list.find(dwId);
  if (it == m_list.end())
    return false;
  dwStartTime = it->second.getStartTime();
  dwEndTime = it->second.getEndTime();
  return true;
}

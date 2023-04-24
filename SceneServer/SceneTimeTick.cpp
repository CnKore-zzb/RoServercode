#include "SceneServer.h"
#include "SceneUserManager.h"
#include "SceneNpcManager.h"
#include "SceneItemManager.h"
#include "SceneTrapManager.h"
#include "SceneActManager.h"
#include "SceneManager.h"
#include "ChatRoomManager.h"
#include "Quest.h"
#include "SceneTower.h"
#include "MusicBoxManager.h"
#include "ActivityManager.h"
#include "TeamSealManager.h"
#include "GuildRaidConfig.h"
#include "GuildMusicBoxManager.h"
#include "GuildCityManager.h"
#include "ActivityEventManager.h"
#include "PveCardConfig.h"
#include "DressUpStageMgr.h"
#include "BossMgr.h"
#include "StatMgr.h"

void SceneServer::v_timetick()
{
  ZoneServer::v_timetick();

  QWORD curMSec = xTime::getCurMSec();
  DWORD curSec = static_cast<DWORD>(curMSec / ONE_THOUSAND);
  static UINT recursiveCounter = 0;

  xTime frameTimer;
  QWORD _e = frameTimer.uElapse();
  if ((recursiveCounter % 2) == 0)
  {
    SceneUserManager::getMe().timer(curMSec);

    SceneUserManager::getMe().load();

    SceneTrapManager::getMe().timer(curMSec);

    SceneManager::getMe().timer(curMSec);

    SceneItemManager::getMe().timer(curMSec);

    SceneNpcManager::getMe().addGroup();
  }

  ++recursiveCounter;

  if (m_oOneSecTimer.timeUp(curSec))
  {
    SceneNpcManager::getMe().timer(curMSec);

    ChatRoomManager::getMe().timer(curSec);

    SceneActManager::getMe().timer(curSec);

    QuestManager::getMe().timer(curSec);

    MusicBoxManager::getMe().timer(curSec);

    ActivityManager::getMe().timer(curSec);

    TeamSealManager::getMe().timer(curSec);

    GuildMusicBoxManager::getMe().timer(curSec);

    GuildCityManager::getMe().timer(curSec);

    DressUpStageMgr::getMe().timer(curSec);

    ItemConfig::getMe().timer(curSec);

    BossMgr::getMe().timer(curSec);

    if (m_oTickOneMin.timeUp(curSec))
    {
      GuildRaidConfig::getMe().createGuildRaid();
      StatMgr::getMe().timer(curSec);
      if(m_oDayTimer.timeUp(curSec))
      {
        ItemConfig::getMe().loadLotteryConfig();
      }

      frameTimer.elapseStart();
      PveCardConfig::getMe().randSystemCard();
      _e = frameTimer.uElapse();
      if (_e > 30 * 1000)
        XLOG << "[帧耗时]" << "pvecard" << _e << " 微秒" << XEND;

      DWORD ret = ExecutionTime_Reset(false);
      // if (ret >= 7000)
      if (ret >= 70)
      {
        std::ostringstream stream;
        stream.str("");
        stream << getServerName() << ",timetick占用时间过高:" << ret << "%";
        MsgManager::alter_msg(getFullName(), stream.str(), EPUSHMSG_CORE_DUMP);
      }

      XLOG << "[管理器计数]" << "SceneUserManager" << SceneUserManager::getMe().size() << XEND;
      XLOG << "[管理器计数]" << "SceneManager" << SceneManager::getMe().size() << XEND;
      XLOG << "[管理器计数]" << "SceneNpcManager" << SceneNpcManager::getMe().size() << XEND;
      XLOG << "[管理器计数]" << "SceneItemManager" << SceneItemManager::getMe().size() << XEND;
      XLOG << "[管理器计数]" << "SceneTrapManager" << SceneTrapManager::getMe().size() << XEND;
      XLOG << "[管理器计数]" << "SceneActManager" << SceneActManager::getMe().size() << XEND;
    }
  }

  _e = frameTimer.uElapse();
  if (_e > 100000)
  {
    XLOG << "[帧耗时]" << "timetick" << _e << " 微秒" << XEND;
  }
}

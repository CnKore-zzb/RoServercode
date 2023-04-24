#include "FeatureConfig.h"
#include "xLuaTable.h"

FeatureConfig::FeatureConfig()
{

}

FeatureConfig::~FeatureConfig()
{

}

bool FeatureConfig::loadConfig()
{
  if (!xLuaTable::getMe().open("Lua/NpcFeatures.txt"))
  {
    XERR << "[NpcFeatures], NpcFeatures加载失败" << XEND;
    return false;
  }

  xLuaData table;
  xLuaTable::getMe().getLuaData("NpcFeatures", table);

  for (auto m = table.m_table.begin(); m != table.m_table.end(); ++m)
  {
    if (m->first == "TeamWork")
    {
      m_stNpcAICFG.dwTeamHelpHp = m->second.getTableInt("TeamHelpHp");
      m_stNpcAICFG.dwTeamHelpRange = m->second.getTableInt("TeamHelpRange");
      xLuaData& attdata = m->second.getMutableData("TeamAttack");
      m_stNpcAICFG.dwTeamAttack.dwRange = attdata.getTableInt("range");
      m_stNpcAICFG.dwTeamAttack.dwResponseTime = attdata.getTableInt("responseTime");
      m_stNpcAICFG.dwTeamAttack.dwEmojiID = attdata.getTableInt("emoji");
    }
    else if (m->first == "Ghost")
    {
      //m_stNpcAICFG.dwGhost.dwBirthBuff = m->second.getTableInt("BirthBuff");
      //m_stNpcAICFG.dwGhost.dwAttBuff = m->second.getTableInt("AttackBuff");
      m_stNpcAICFG.stGhost.dwFlashDamage = m->second.getTableInt("DamPer");
      xLuaData& buff = m->second.getMutableData("BirthBuff");
      auto fun = [&](const string& str, xLuaData& data)
      {
        m_stNpcAICFG.stGhost.vecBuff.push_back(data.getInt());
      };
      buff.foreach(fun);
    }
    else if (m->first == "Demon")
    {
      m_stNpcAICFG.dwDemon.dwBirthBuff = m->second.getTableInt("BuffID");
      m_stNpcAICFG.dwDemon.dwDizzyVal = m->second.getTableInt("DizzyVal");
    }
    else if (m->first == "Flight")
    {
      auto fun = [this](const string& str, xLuaData& data)
      {
        m_stNpcAICFG.dwFly.vecImmuneSkill.push_back(data.getInt());
      };
      xLuaData& imudata = m->second.getMutableData("ImmuneSkill");
      imudata.foreach(fun);
      m_stNpcAICFG.dwFly.dwBirthBuff = m->second.getTableInt("BuffID");
    }
    else if (m->first == "ShowMoe")
    {
      m_stNpcAICFG.dwSmile.dwInterval = m->second.getTableInt("ShowMoeVal");
      m_stNpcAICFG.dwSmile.dwRange = m->second.getTableInt("ShowMoeRange");
      m_stNpcAICFG.dwSmile.dwBuff = m->second.getTableInt("BuffID");
      m_stNpcAICFG.dwSmile.dwStayTime = m->second.getTableInt("StayTime");
    }
    else if (m->first == "SceneStealing")
    {
      m_stNpcAICFG.dwGoCamera.dwBuff = m->second.getTableInt("BuffID");
      m_stNpcAICFG.dwGoCamera.dwRangeFind = m->second.getTableInt("Distance");
      m_stNpcAICFG.dwGoCamera.dwRangeStop = m->second.getTableInt("Pos");
      m_stNpcAICFG.dwGoCamera.dwInterval = m->second.getTableInt("Val");
    }
    else if (m->first == "Mischievous")
    {
      m_stNpcAICFG.dwNaughty.dwResponseTime = m->second.getTableInt("ResponseTime");
      m_stNpcAICFG.dwNaughty.dwSkillID = m->second.getTableInt("SkillID");
      m_stNpcAICFG.dwNaughty.dwFormerEmoji = m->second.getTableInt("Emoji1");
      m_stNpcAICFG.dwNaughty.dwLatterEmoji = m->second.getTableInt("Emoji2");
    }
    else if (m->first == "Alert")
    {
      m_stNpcAICFG.dwAlert.dwRange = m->second.getTableInt("FindRange");
      m_stNpcAICFG.dwAlert.dwSkillID = m->second.getTableInt("SkillID");
      m_stNpcAICFG.dwAlert.dwResponseTime = m->second.getTableInt("ResponseTime");
      m_stNpcAICFG.dwAlert.dwEmoji = m->second.getTableInt("Emoji");
      m_stNpcAICFG.dwAlert.dwInterval = m->second.getTableInt("SkillVal");
    }
    else if (m->first == "Expel")
    {
      m_stNpcAICFG.dwExpel.dwRange = m->second.getTableInt("FindRange");
      m_stNpcAICFG.dwExpel.dwInterval = m->second.getTableInt("ExpelVal");
      m_stNpcAICFG.dwExpel.dwSkillID = m->second.getTableInt("SkillID");
      m_stNpcAICFG.dwExpel.dwEmoji = m->second.getTableInt("Emoji");
    }
    else if (m->first == "PickUp")
    {
      m_stNpcAICFG.dwPickupMaxItem = m->second.getTableInt("PickupMaxItem");
      m_stNpcAICFG.dwPickupRange = m->second.getTableInt("PickupRange");
    }
    else if (m->first == "Jealous")
    {
      m_stNpcAICFG.stJealous.dwRange = m->second.getTableInt("FindRange");
      m_stNpcAICFG.stJealous.dwEmoji = m->second.getTableInt("Emoji");
    }
    else if (m->first == "ItemFind")
    {
      m_stNpcAICFG.stReaction.dwRange = m->second.getTableInt("FindRange");
    }
    else if (m->first == "LeaveBattle")
    {
      m_stNpcAICFG.stLeaveBattle.dwNormalRange = m->second.getTableInt("LeaveRange");
      m_stNpcAICFG.stLeaveBattle.dwNormalTime = m->second.getTableInt("LeaveTime");
      m_stNpcAICFG.stLeaveBattle.dwMvpRange = m->second.getTableInt("MVPLeaveRange");
      m_stNpcAICFG.stLeaveBattle.dwMvpTime = m->second.getTableInt("MVPLeaveTime");
    }
    else if (m->first == "TeamBattle")
    {
      m_stNpcAICFG.stStatusAttack.dwFindRange = m->second.getTableInt("FindRange");
      auto getstatus = [&](const string& str, xLuaData& data)
      {
        DWORD status = data.getTableInt("status");
        DWORD skill = data.getTableInt("skill");
        if (status != 0)
          m_stNpcAICFG.stStatusAttack.vecStatus2SKill.push_back(pair<DWORD, DWORD>(status, skill));
      };
      m->second.foreach(getstatus);
    }
    else if (m->first == "Night")
    {
      auto getbuff = [&](const string& str, xLuaData& data)
      {
        m_stNpcAICFG.stNightCFG.vecBuffs.push_back(data.getInt());
      };
      m->second.getMutableData("Buffer").foreach(getbuff);
    }
    else if (m->first == "Endure")
    {
      m_stNpcAICFG.stEndureCFG.dwCD = m->second.getTableInt("CD");
      m_stNpcAICFG.stEndureCFG.dwSkill = m->second.getTableInt("skill");
    }
    else if (m->first == "Servant")
    {
      m_stNpcAICFG.stServant.dwKeepDis = m->second.getTableInt("keep_distance");
    }
    else if (m->first == "GoBack")
    {
      m_stNpcAICFG.vecGoBackBuff.clear();
      auto getbuff = [&](const string& key, xLuaData& data)
      {
        m_stNpcAICFG.vecGoBackBuff.push_back(data.getInt());
      };
      m->second.getMutableData("buff").foreach(getbuff);
    }
    else if (m->first == "AIParams")
    {
      m_stNpcAICFG.stAIParams.dwRunAwayTime = m->second.getTableInt("run_away_time");
    }
  }
  return true;
}


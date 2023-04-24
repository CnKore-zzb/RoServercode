#include "GMCommandMgr.h"
#include "GuildManager.h"
#include "RewardConfig.h"
#include "GuildRaidConfig.h"
#include "GuildServer.h"
#include "PlatLogManager.h"
#include "GuildIconManager.h"

bool GMCommandMgr::exec(const GMCommandGuildSCmd& rev)
{
  xLuaData param;
  if (param.fromJsonString(rev.command()) == false)
    return false;

  string cmd = param.getTableString("cmd");
  if (cmd == "building")
    return building(rev.info(), param);
  else if (cmd == "challenge")
    return challenge(rev.info(), param);
  else if (cmd == "additem")
    return additem(rev.info(), param);
  else if (cmd == "reduceitem")
    return reduceitem(rev.info(), param);
  else if (cmd == "artifact")
    return artifact(rev.info(), param);
  else if (cmd == "quest")
    return quest(rev.info(), param);
  else if (cmd == "addcon")
    return addcon(rev.info(), param);
  else if (cmd == "subcon")
    return subcon(rev.info(), param);
  else if (cmd == "gvg")
    return gvg(rev.info(), param);
  else if (cmd == "addasset")
    return addasset(rev.info(), param);
  else if (cmd == "reward")
    return reward(rev.info(), param);
  else if (cmd == "setvar")
    return setvar(rev.info(), param);
  else if (cmd == "auth")
    return auth(rev.info(), param);
  else if (cmd == "iconstate")
    return iconstate(rev.info(), param);
  return false;
}

Guild* GMCommandMgr::getGuild(const GuildGM& info)
{
  Guild* pGuild = nullptr;
  if (info.has_guildid() == true)
  {
    pGuild = GuildManager::getMe().getGuildByID(info.guildid());
    if (pGuild != nullptr)
      return pGuild;
  }
  if (info.has_name() == true)
  {
    pGuild = GuildManager::getMe().getGuildByGuildName(info.name());
    if (pGuild != nullptr)
      return pGuild;
  }
  if (info.has_charid() == true)
  {
    pGuild = GuildManager::getMe().getGuildByUserID(info.charid());
    if (pGuild != nullptr)
      return pGuild;
  }
  return nullptr;
}

bool GMCommandMgr::building(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = GuildManager::getMe().getGuildByUserID(info.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(info.charid());
  if (pMember == nullptr)
    return false;

  if (data.getTableInt("submitcount") == 1)
  {
    pMember->resetSubmitCount(true);
    XLOG << "[公会GM-重置材料提交次数]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "重置成功" << XEND;
    return true;
  }

  DWORD type = data.getTableInt("type");
  if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX || EGuildBuilding_IsValid(type) == false)
    return false;
  EGuildBuilding bt = static_cast<EGuildBuilding>(type);

  DWORD lv = data.getTableInt("lv");
  if (lv)
  {
    pGuild->getMisc().getBuilding().levelup(bt, lv);
    XLOG << "[公会GM-建筑升级]" << pGuild->getGUID() << pGuild->getName() << "建筑类型:" << bt << "增加等级:" << lv << XEND;
  }

  if (data.getTableInt("clearbuildcd") == 1)
  {
    if (pGuild->getMisc().getBuilding().clearBuildCD(bt))
      XLOG << "[公会GM-清除建筑建造cd]" << pGuild->getGUID() << pGuild->getName() << bt << "清除成功" << XEND;
  }

  if (data.getTableInt("cancelbuild") == 1)
  {
    if (pGuild->getMisc().getBuilding().cancelBuild(bt))
      XLOG << "[公会GM-取消建造状态]" << pGuild->getGUID() << pGuild->getName() << bt << "取消成功" << XEND;
  }

  return true;
}

bool GMCommandMgr::challenge(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = GuildManager::getMe().getGuildByUserID(info.charid());
  if (pGuild == nullptr)
    return false;
  GMember* pMember = pGuild->getMember(info.charid());
  if (pMember == nullptr)
    return false;

  DWORD groupid = data.getTableInt("gid");
  DWORD progress = data.getTableInt("prog");
  if (groupid && progress)
  {
    const TVecDWORD& challenge = GuildConfig::getMe().getGuildChallengeIDByGroup(groupid);
    if (challenge.empty())
      return false;

    vector<GuildChallengeItem*> items;
    for (auto& id : challenge)
    {
      const SGuildChallengeCFG* cfg = GuildConfig::getMe().getGuildChallengeCFG(id);
      if (cfg == nullptr)
        continue;

      GuildChallengeItem* item = NEW GuildChallengeItem();
      if (item == nullptr)
        continue;
      item->set_id(cfg->dwID);
      item->set_progress(999);

      items.push_back(item);
    }

    pGuild->getMisc().getChallenge().updateProgress(pMember, items, progress, false);

    XLOG << "[公会GM-设置挑战进度]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "进度:" << progress << "挑战任务:";
    for (auto& item : items)
    {
      XLOG << item->id() << item->progress();
      SAFE_DELETE(item);
    }
    XLOG << XEND;
  }

  if (data.getTableInt("reset") == 1)
  {
    pGuild->getMisc().getChallenge().resetChallenge(true);
    XLOG << "[公会GM-重置挑战进度]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "重置成功";
  }

  if (data.getTableInt("clearexittime") == 1)
  {
    pMember->setLastExitTime(0);
    XLOG << "[公会GM-挑战清除玩家上一次退会时间]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "清除成功";
  }

  return true;
}

bool GMCommandMgr::additem(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  string guid = data.getTableString("guid");
  DWORD itemid = data.getTableInt("id");
  DWORD itemcount = data.getTableInt("count");
  DWORD source = data.getTableInt("source");
  if (itemid && itemcount)
  {
    if (data.has("artifact"))
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(itemid);
      if (cfg && ItemConfig::getMe().isArtifact(cfg->eItemType))
      {
        pGuild->getMisc().getArtifact().addArtifact(itemid);
        XLOG << "[公会GM-添加神器]" << pGuild->getGUID() << pGuild->getName() << "神器:" << itemid << "添加成功" << XEND;
      }
    }
    else
    {
      ItemData item;
      if (guid.empty() == false)
        item.mutable_base()->set_guid(guid);
      item.mutable_base()->set_id(itemid);
      item.mutable_base()->set_count(itemcount);
      if (source)
        item.mutable_base()->set_source(static_cast<ESource>(source));
      pGuild->getPack().addItem(item);
      XLOG << "[公会GM-添加道具]" << pGuild->getGUID() << pGuild->getName() << "道具" << itemid << itemcount << guid << "添加成功" << XEND;
    }
  }

  return true;
}

bool GMCommandMgr::artifact(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = GuildManager::getMe().getGuildByUserID(info.charid());
  if (pGuild == nullptr)
    return false;

  if (data.has("fixpack"))
  {
    if (data.has("all"))
    {
      bool open = data.getTableInt("fixpack") == 1;
      GuildManager::getMe().openArtifactFixPack(open);
      XLOG << "[公会GM-神器修复包裹]" << "open:" << open << "收回成功" << XEND;
    }
    else
    {
      pGuild->getMisc().getArtifact().fixPack(true);
      XLOG << "[公会GM-神器修复包裹]" << pGuild->getGUID() << pGuild->getName() << "执行成功" << XEND;
    }
    return true;
  }

  GMember* pMember = pGuild->getMember(info.charid());
  if (pMember == nullptr)
    return false;

  DWORD itemid = data.getTableInt("id");
  if (itemid && data.getTableInt("mat") > 0)
  {
    const SArtifactCFG* cfg = GuildConfig::getMe().getArtifactCFG(itemid);
    if (cfg == nullptr)
      return false;
    TVecItemInfo materials;
    if (pGuild->getMisc().getArtifact().getProduceMaterial(itemid, materials) == false)
      return false;
    TVecItemInfo items;
    for (auto& v : materials)
    {
      const SItemCFG* cfg = ItemConfig::getMe().getItemCFG(v.id());
      if (cfg && ItemConfig::getMe().isArtifact(cfg->eItemType))
      {
        for (DWORD i = 0; i < v.count(); ++i)
          pGuild->getMisc().getArtifact().addArtifact(v.id());
      }
      else
      {
        ItemInfo item;
        item.set_id(v.id());
        item.set_count(v.count());
        combinItemInfo(items, TVecItemInfo{item});
      }
    }
    pGuild->getPack().addItem(items);
    XLOG << "[公会GM-添加神器材料]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "神器:" << itemid << "添加成功" << XEND;
  }

  if (data.getTableInt("clrdis") == 1)
  {
    pGuild->getMisc().getArtifact().clearDistributeCount();
    XLOG << "[公会GM-清除神器分配时间]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "清除成功" << XEND;
  }

  if (data.getTableInt("retrieve") == 1)
  {
    pGuild->getMisc().getArtifact().retrieve(true);
    XLOG << "[公会GM-立即收回]" << pGuild->getGUID() << pGuild->getName() << pMember->getAccid() << pMember->getCharID() << pMember->getName() << "收回成功" << XEND;
  }

  return true;
}

bool GMCommandMgr::quest(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  GuildQuestMgr& rQuestMgr = pGuild->getMisc().getQuest();
  const string& action = data.getTableString("action");

  if (action == "update")
    rQuestMgr.updateQuest(info.charid(), data.getTableInt("id"));

  return true;
}

bool GMCommandMgr::reduceitem(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  string guid = data.getTableString("guid");
  DWORD id = data.getTableInt("id");
  DWORD count = data.getTableInt("count");
  DWORD source = data.getTableInt("source");

  if (guid.empty() == false)
    return pGuild->getPack().reduceItem(guid, count, source ? static_cast<ESource>(source) : ESOURCE_GM);

  return pGuild->getPack().reduceItem(id, count, source ? static_cast<ESource>(source) : ESOURCE_GM);
}

bool GMCommandMgr::addcon(const GuildGM& info, const xLuaData& data)
{
  if (info.charid() == 0)
    return false;

  SocialUser oUser;
  oUser.set_zoneid(info.zoneid());
  oUser.set_charid(info.charid());

  DWORD dwValue = data.getTableInt("num");
  DWORD dwSource = data.getTableInt("source");
  GuildManager::getMe().addContribute(oUser, dwValue, dwSource);
  return true;
}

bool GMCommandMgr::subcon(const GuildGM& info, const xLuaData& data)
{
  if (info.charid() == 0)
    return false;

  SocialUser oUser;
  oUser.set_zoneid(info.zoneid());
  oUser.set_charid(info.charid());

  DWORD dwValue = data.getTableInt("num");
  GuildManager::getMe().subContribute(oUser, dwValue);
  return true;
}

bool GMCommandMgr::gvg(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  const string& action = data.getTableString("action");
  bool bResult = false;

  if (action == "treasure_add")
    bResult = pGuild->getMisc().getTreasure().win(data.getTableInt("id"), data.getTableInt("times"));
  else if (action == "treasure_open")
    bResult = pGuild->getMisc().getTreasure().open(info.charid(), data.getTableInt("id"));
  else if (action == "set_supergvg")
  {
    DWORD cnt = data.getTableInt("count");
    DWORD score = data.getTableInt("score");
    pGuild->getMisc().getGvg().setFireCntAndScore(cnt, score);
    bResult = true;
  }

  if (bResult)
    XLOG << "[公会GM-gvg]" << info.ShortDebugString() << "执行成功" << XEND;
  else
    XERR << "[公会GM-gvg]" << info.ShortDebugString() << "执行失败" << XEND;
  return bResult;
}

bool GMCommandMgr::addasset(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  bool bSelf = data.getTableInt("self") == 1;
  bool bNoCheck = data.getTableInt("nocheck") == 1;
  ESource eSource = static_cast<ESource>(data.getTableInt("source"));
  DWORD dwNum = data.getTableInt("num");

  if (bSelf)
  {
    GMember* pMember = pGuild->getMember(info.charid());
    if (pMember == nullptr)
      return false;
    pMember->setAsset(pMember->getAsset() + dwNum);

    PlatLogManager::getMe().incomeMoneyLog(thisServer,
        0,
        pMember->getZoneID(),
        pMember->getAccid(),
        pMember->getCharID(),
        xTime::getCurUSec(),
        0,  //charge
        EMONEYTYPE_GUILDASSET, dwNum, pMember->getAsset(),
        eSource);
  }
  else
  {
    pGuild->addAsset(dwNum, bNoCheck, eSource == ESOURCE_MIN ? ESOURCE_GM : eSource);
  }

  return true;
}

bool GMCommandMgr::reward(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  DWORD dwRewardID = data.getTableInt("id");
  TVecItemInfo vecItems;
  if (RewardConfig::getMe().roll(dwRewardID, RewardEntry(), vecItems, ESOURCE_GM) == false)
  {
    XERR << "[公会GM-reward]" << info.ShortDebugString() << "执行失败,随机reward失败" << XEND;
    return false;
  }

  return pGuild->getPack().addItem(vecItems);
}

bool GMCommandMgr::setvar(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = GuildManager::getMe().getGuildByID(data.getTableQWORD("guildid"));
  if (pGuild == nullptr)
    return false;

  DWORD dwVar = data.getTableInt("var");
  if (dwVar <= EVARTYPE_MIN || dwVar >= EVARTYPE_MAX || EVarType_IsValid(dwVar) == false)
    return false;

  DWORD dwValue = data.getTableInt("value");
  pGuild->getMisc().setVarValue(static_cast<EVarType>(dwVar), dwValue);

  pGuild->setMark(EGUILDDATA_ASSET_DAY);
  pGuild->setMark(EGUILDDATA_TREASURE_GVG_COUNT);
  pGuild->setMark(EGUILDDATA_TREASURE_GUILD_COUNT);
  pGuild->setMark(EGUILDDATA_TREASURE_BCOIN_COUNT);
  pGuild->updateData();
  return true;
}

bool GMCommandMgr::auth(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  //bool modifyAuth(bool bAdd, EModify eModify, EGuildJob eJob, EAuth eAuth);
  DWORD dwModify = data.getTableInt("modify");
  if (dwModify <= EMODIFY_MIN || dwModify >= EMODIFY_MAX || EModify_IsValid(dwModify) == false)
    return false;
  DWORD dwJob = data.getTableInt("job");
  if (dwJob <= EGUILDJOB_MIN || dwJob >= EGUILDJOB_MAX || EGuildJob_IsValid(dwJob) == false)
    return false;
  DWORD dwAuth = data.getTableInt("auth");
  if (dwAuth <= EAUTH_MIN || dwAuth >= EAUTH_MAX || EAuth_IsValid(dwAuth) == false)
    return false;
  bool bAdd = data.getTableInt("add") == 1;

  return pGuild->getMisc().modifyAuth(bAdd, static_cast<EModify>(dwModify), static_cast<EGuildJob>(dwJob), static_cast<EAuth>(dwAuth));
}

bool GMCommandMgr::iconstate(const GuildGM& info, const xLuaData& data)
{
  Guild* pGuild = getGuild(info);
  if (pGuild == nullptr)
    return false;

  DWORD dwState = data.getTableInt("state");
  if (EIconState_IsValid(dwState) == false)
    return false;

  DWORD dwIndex = data.getTableInt("index");
  if (dwIndex != 0)
    GuildIconManager::getMe().setState(pGuild->getGUID(), dwIndex, static_cast<EIconState>(dwState));
  else
    GuildIconManager::getMe().setState(pGuild->getGUID(), static_cast<EIconState>(dwState));
  return true;
}


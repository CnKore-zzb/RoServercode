#include "UserSceneData.h"
#include "SceneUser.h"
#include "UserConfig.h"
#include "DScene.h"
#include "SceneServer.h"
#include "ChatRoomManager.h"
#include "ClientCmd.h"
#include "RegCmd.h"
#include "SceneUserManager.h"
#include "StatisticsDefine.h"
#include "RedisManager.h"
#include "ActivityConfig.h"
#include "MsgManager.h"
#include "GuildRaidConfig.h"
#include "ShopConfig.h"
#include "PlatLogManager.h"
#include "GlobalManager.h"

UserSceneData::UserSceneData(SceneUser* pUser) : m_pUser(pUser)
{

}

UserSceneData::~UserSceneData()
{

}

void UserSceneData::setOnlineMapPos(DWORD mapID, const xPos &pos)
{
  setOnlineMapID(mapID);
  setOnlinePos(pos);
}

DWORD UserSceneData::getRaidid() const
{
  if (m_pUser == nullptr)
    return 0;

  DScene* pDScene = dynamic_cast<DScene*>(m_pUser->getScene());
  return pDScene == nullptr ? 0 : pDScene->getRaidID();
}

// data
bool UserSceneData::fromAccData(const UserAccData& rData)
{
  m_oAccData.CopyFrom(rData);
  return true;
}

bool UserSceneData::fromBaseData(const UserBaseData& rData)
{
  m_oBaseData.CopyFrom(rData);

  // 首次登录设置最高职业
  if(!m_oBaseData.maxpro())
    m_oBaseData.set_maxpro(rData.profession());

  const SMapCFG* pCFG = MapConfig::getMe().getMapCFG(rData.mapid());
  m_bRaid = pCFG != nullptr;
  return true;
}

bool UserSceneData::toBaseData(UserBaseData& rData)
{
  m_oBaseData.set_guildid(m_pUser->getGuild().id());

  m_oBaseData.set_hair(m_pUser->getHairInfo().getRealHair());
  m_oBaseData.set_haircolor(m_pUser->getHairInfo().getRealHairColor());

  m_oBaseData.set_body(getRealBody());
  m_oBaseData.set_lefthand(getLefthand());
  m_oBaseData.set_righthand(getRighthand());
  m_oBaseData.set_head(getHead(true));
  m_oBaseData.set_back(getBack(true));
  m_oBaseData.set_face(getFace(true));
  m_oBaseData.set_tail(getTail(true));
  m_oBaseData.set_mount(getMount());
  m_oBaseData.set_title(m_pUser->getTitle().getCurTitle(ETITLE_TYPE_MANNUAL));
  m_oBaseData.set_partnerid(m_pUser->getPet().getPartnerID());
  m_oBaseData.set_portrait(m_pUser->getPortrait().getCurPortrait(true));
  m_oBaseData.set_mouth(getMouth());
  m_oBaseData.set_eye(m_pUser->getEye().getCurID(true));
  m_oBaseData.set_clothcolor(getClothColor(true));
  m_oBaseData.set_name(m_pUser->name);

  rData.CopyFrom(m_oBaseData);
  return true;
}

bool UserSceneData::toPreviewAppearanceData(ProfessionData& rData)
{
  rData.clear_appearance_data();

  add_data(rData.add_appearance_data(), EUSERDATATYPE_ROLELEVEL, getRolelv());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_SEX, getGender());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_BODY, getBody(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_BODYSCALE, getBodyScale(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_HAIR, m_pUser->getHairInfo().getRealHair());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_HAIRCOLOR, m_pUser->getHairInfo().getRealHairColor());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_CLOTHCOLOR, getClothColor());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_LEFTHAND, getLefthand());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_RIGHTHAND, getRighthand());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_HEAD, getHead(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_BACK, getBack(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_FACE, getFace(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_TAIL, getTail(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_MOUNT, getMount(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_PORTRAIT, m_pUser->getPortrait().getCurPortrait());
  add_data(rData.add_appearance_data(), EUSERDATATYPE_EYE, m_pUser->getEye().getCurID(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_MOUTH, getMouth(true));
  add_data(rData.add_appearance_data(), EUSERDATATYPE_SHADERCOLOR, getShaderColor());

  return true;
}

bool UserSceneData::load(const BlobAccData& oAccBlob, const BlobData& oBlob)
{
  // user data
  const BlobAccUser& oAccUser = oAccBlob.user();
  const BlobUser& oUser = oBlob.user();
  m_stBlobData.oPos.x = oUser.x();
  m_stBlobData.oPos.y = oUser.y();
  m_stBlobData.oPos.z = oUser.z();

  m_stBlobData.dwClothColor = oUser.clothcolor();
  m_stBlobData.dwPurify = oUser.purify();
  m_stBlobData.dwSaveMap = (oUser.savemap() != 0 ? oUser.savemap() : MiscConfig::getMe().getSystemCFG().dwNewCharMapID);
  m_stBlobData.dwLastMap = oUser.lastmapid();
  m_stBlobData.dwLastRealMap = oUser.lastrealmapid();

  m_stBlobData.setShowNpcs.clear();
  for (int i = 0; i < oUser.shownpcs_size(); ++i)
  {
    m_stBlobData.setShowNpcs.insert(oUser.shownpcs(i));
    m_setShowNpc.insert(oUser.shownpcs(i));
  }
  for (int i = 0; i < oAccUser.shownpcs_size(); ++i)
  {
    m_stAccData.setShowNpcs.insert(oAccUser.shownpcs(i));
    m_setShowNpc.insert(oAccUser.shownpcs(i));
  }
  m_stBlobData.setMapArea.clear();
  for (int i = 0; i < oUser.mapareas_size(); ++i)
    m_stBlobData.setMapArea.insert(oUser.mapareas(i));
  m_stBlobData.setPatchVersion.clear();
  for (int i = 0; i < oUser.patchversion_size(); ++i)
    m_stBlobData.setPatchVersion.insert(oUser.patchversion(i));
  setTeamTimeLen(oUser.teamtimelen());
  //setFollowerIDNoCheck(oUser.followerid());

  m_stBlobData.qwFollowerID = oUser.followerid();
  m_stBlobData.dwLevelUpTime = oUser.leveluptime();

  m_stBlobData.qwZenyMax = oUser.zeny_max();
  m_stBlobData.qwZenyDebt = oUser.zeny_debt();

  m_stBlobData.dwPvpCoin = oUser.pvp_coin();

  /*m_stBlobData.dwCon = oUser.con();
  m_stBlobData.bConInit = oUser.coninit();*/

  m_stBlobData.dwLotteryCoin = oUser.lottery_coin();

  m_stBlobData.dwRenameTime = oUser.rename_time();

  m_stBlobData.dwGuildHonor = oUser.guild_honor();
  m_stBlobData.bDivorceRollerCoaster = oUser.divorce_roller_coaster();

  // first action
  const BlobFirstActionDone& oFirst = oBlob.action();
  m_stBlobData.dwFirstAction = oFirst.action();

  // show
  const BlobShow& oShow = oBlob.show();
  m_stBlobData.setActions.clear();
  for (int i = 0; i < oShow.actions_size(); ++i)
    m_stBlobData.setActions.insert(oShow.actions(i));
  m_stBlobData.setExpressions.clear();
  for (int i = 0; i < oShow.expressions_size(); ++i)
    m_stBlobData.setExpressions.insert(oShow.expressions(i));

  // known map
  const BlobKnownMaps& oKnownMap = oBlob.knownmap();
  m_stBlobData.dwKnownMapVer = oKnownMap.version();
  m_stBlobData.setKnownMaps.clear();
  for (int i = 0; i < oKnownMap.list_size(); ++i)
    m_stBlobData.setKnownMaps.insert(oKnownMap.list(i));

  // gm effect
  const BlobGMEffects& oEffect = oBlob.effect();
  for (int i = 0; i < oEffect.list_size(); ++i)
  {
    SGmEffect onegm;
    onegm.mapid = oEffect.list(i).mapid();
    onegm.index = oEffect.list(i).index();
    onegm.gmeffect = oEffect.list(i).gmcommand();
    m_stBlobData.gms.push_back(onegm);
  }

  // trace
  const BlobTrace& oTrace = oBlob.trace();
  m_stBlobData.vecTraceItem.clear();
  for (int i = 0; i < oTrace.items_size(); ++i)
  {
    STraceItem stItem(oTrace.items(i).itemid(), oTrace.items(i).monsterid());
    auto v = find_if(m_stBlobData.vecTraceItem.begin(), m_stBlobData.vecTraceItem.end(), [stItem](const STraceItem& r) -> bool{
      return r.dwItemID == stItem.dwItemID && r.dwMonsterID == stItem.dwMonsterID;
    });
    if (v == m_stBlobData.vecTraceItem.end())
      m_stBlobData.vecTraceItem.push_back(stItem);
  }

  // option
  const BlobOption& oOption = oBlob.opt();
  m_stOptionData.eType = oOption.type();
  if (m_stOptionData.eType == EQUERYTYPE_MIN)
    m_stOptionData.eType = MiscConfig::getMe().getNewRoleCFG().eDefaultQueryType;
  m_stOptionData.m_dwNormalSkillOption = oOption.normalskill_option();
  m_stOptionData.m_dwFashionHide = oOption.fashionhide();  
  
  std::bitset<64> tmpSet(oOption.bitopt());
  m_stOptionData.m_btSet = tmpSet;
  m_stOptionData.m_mapSkillOpt.clear();
  for (int i = 0; i < oOption.skillopts_size(); ++i)
  {
    m_stOptionData.m_mapSkillOpt[oOption.skillopts(i).opt()] = oOption.skillopts(i).value();
  }
  m_stOptionData.eWeddingType = oOption.wedding_type();
  if (m_stOptionData.eWeddingType == EQUERYTYPE_MIN)
  {
    setQueryWeddingType(EQUERYTYPE_WEDDING_ALL);
  }

  // userzone
  const BlobUserZone& oUserZone = oBlob.userzone();
  for (int i = 0; i < oUserZone.infos_size(); ++i)
  {
    const RecentZoneInfo& rInfo = oUserZone.infos(i);
    m_stBlobData.vecRecentZones.push_back(SRecentZoneInfo(rInfo.type(), rInfo.zoneid()));
  }

  m_stBlobData.dwLastSMapid = oUser.lastsmapid();
  m_stBlobData.lastSPos.x = oUser.sx();
  m_stBlobData.lastSPos.y = oUser.sy();
  m_stBlobData.lastSPos.z = oUser.sz();
  //heal count
  m_stBlobData.dwHealCount = oUser.healcount();

  // activity data
  const BlobActivity& activity = oBlob.activity();
  m_stBlobData.mapActivityData.clear();
  for (int i = 0; i < activity.activitydatas_size(); ++i)
  {
    const ActivityCommonData& comdata = activity.activitydatas(i);
    SActivityData& actdata = m_stBlobData.mapActivityData[comdata.name()];
    for (int j = 0; j < comdata.params_size(); ++j)
      actdata.vecParams.push_back(comdata.params(j));
  }
  m_stBlobData.dwTotalBattleTime = oBlob.battle().totalbattletime();

  const BlobSeeNpc& seenpc = oBlob.seenpc();
  m_stBlobData.setSeeNpcs.clear();
  m_stBlobData.setHideNpcs.clear();
  for (int i = 0; i < seenpc.see_size(); ++i)
    m_stBlobData.setSeeNpcs.insert(seenpc.see(i));
  for (int i = 0; i < seenpc.hide_size(); ++i)
    m_stBlobData.setHideNpcs.insert(seenpc.hide(i));
  // relation
  /*const BlobRelation& relation = oBlob.relation();
  for (int i = 0; i < relation.black_size(); ++i)
    m_stBlobData.setBlackIDs.insert(relation.black(i));*/

  // guild raid
  const BlobGuildRaid& guildraid = oBlob.guildraid();
  m_stBlobData.mapGuildRaidData.clear();
  for (int i = 0; i < guildraid.raid_size(); ++i) {
    const GuildRaid& raid = guildraid.raid(i);
    SGuildRaidData& raiddata = m_stBlobData.mapGuildRaidData[raid.npcid()];
    raiddata.dwNpcId = raid.npcid();
    raiddata.eState = raid.state();
    for (int j = 0; j < raid.killedboss_size(); ++j)
      raiddata.setKilledBoss.insert(raid.killedboss(j));
  }
  m_stBlobData.m_dwGuildRaidVersion = guildraid.raidversion();

  m_stBlobData.dwTransMap = oBlob.settings().transmap();
  m_stBlobData.oTransPos.x = oBlob.settings().trans_x();
  m_stBlobData.oTransPos.y = oBlob.settings().trans_y();
  m_stBlobData.oTransPos.z = oBlob.settings().trans_z();

  m_stBlobData.qwChargeZeny = oUser.charge_zeny();
  m_stBlobData.qwChargeLottery = oUser.charge_lottery();
  m_stBlobData.qwSaveIndex = oUser.save_index();

  m_stBlobData.qwDailyNormalZeny = oUser.daily_normal_zeny();
  m_stBlobData.qwDailyChargeZeny = oUser.daily_charge_zeny();
  m_stBlobData.dwLastOfflineTime = oUser.last_offlinetime();
  m_stBlobData.dwLastBaseLv = oUser.last_baselv();
  m_stBlobData.dwLastJobLv = oUser.last_joblv();

  m_stBlobData.dwTutorbattletime = oUser.tutorbattletime();
  m_stBlobData.dwUsedtutorbattletime = oUser.usedtutorbattletime();
  m_stBlobData.dwDeadCoin = oUser.dead_coin();
  m_stBlobData.dwDeadLv = oUser.dead_lv();
  m_stBlobData.dwDeadExp = oUser.dead_exp();

  return true;
}

bool UserSceneData::save(BlobAccData& oAccBlob, BlobData& oBlob)
{
  // user data
  BlobUser* pUser = oBlob.mutable_user();
  if (pUser != nullptr)
  {
    pUser->Clear();

    pUser->set_x(m_stBlobData.oPos.x);
    pUser->set_y(m_stBlobData.oPos.y);
    pUser->set_z(m_stBlobData.oPos.z);

    pUser->set_clothcolor(m_stBlobData.dwClothColor);
    pUser->set_purify(m_stBlobData.dwPurify);
    pUser->set_savemap(m_stBlobData.dwSaveMap);
    pUser->set_lastmapid(m_stBlobData.dwLastMap);
    pUser->set_lastrealmapid(m_stBlobData.dwLastRealMap);

    for (auto &s : m_stBlobData.setShowNpcs)
      pUser->add_shownpcs(s);
    for (auto &s : m_stBlobData.setMapArea)
      pUser->add_mapareas(s);
    for (auto &s : m_stBlobData.setPatchVersion)
      pUser->add_patchversion(s);

    pUser->set_teamtimelen(m_stBlobData.dwTeamTimeLen);
    pUser->set_followerid(m_stBlobData.qwFollowerID);
    pUser->set_leveluptime(m_stBlobData.dwLevelUpTime);

    pUser->set_lastsmapid(m_stBlobData.dwLastSMapid);
    pUser->set_sx(m_stBlobData.lastSPos.x);
    pUser->set_sy(m_stBlobData.lastSPos.y);
    pUser->set_sz(m_stBlobData.lastSPos.z);

    //heal count
    pUser->set_healcount(m_stBlobData.dwHealCount);

    pUser->set_zeny_max(m_stBlobData.qwZenyMax);
    pUser->set_zeny_debt(m_stBlobData.qwZenyDebt);

    pUser->set_pvp_coin(m_stBlobData.dwPvpCoin);

    /*pUser->set_con(m_stBlobData.dwCon);
    pUser->set_coninit(m_stBlobData.bConInit);*/
    pUser->set_charge_zeny(m_stBlobData.qwChargeZeny);
    pUser->set_charge_lottery(m_stBlobData.qwChargeLottery);
    pUser->set_pvp_coin(m_stBlobData.dwPvpCoin);
    pUser->set_lottery_coin(m_stBlobData.dwLotteryCoin);
    pUser->set_rename_time(m_stBlobData.dwRenameTime);
    pUser->set_guild_honor(m_stBlobData.dwGuildHonor);
    pUser->set_save_index(++m_stBlobData.qwSaveIndex);
    pUser->set_divorce_roller_coaster(m_stBlobData.bDivorceRollerCoaster);

    pUser->set_daily_normal_zeny(m_stBlobData.qwDailyNormalZeny);
    pUser->set_daily_charge_zeny(m_stBlobData.qwDailyChargeZeny);

    pUser->set_tutorbattletime(m_stBlobData.dwTutorbattletime);
    pUser->set_usedtutorbattletime(m_stBlobData.dwUsedtutorbattletime);
    pUser->set_last_offlinetime(m_stBlobData.dwLastOfflineTime);
    pUser->set_last_baselv(m_stBlobData.dwLastBaseLv);
    pUser->set_last_joblv(m_stBlobData.dwLastJobLv);
    pUser->set_dead_coin(m_stBlobData.dwDeadCoin);
    pUser->set_dead_lv(m_stBlobData.dwDeadLv);
    pUser->set_dead_exp(m_stBlobData.dwDeadExp);
  }

  BlobAccUser* pAccUser = oAccBlob.mutable_user();
  if (pAccUser != nullptr)
  {
    pAccUser->Clear();

    for (auto &s : m_stAccData.setShowNpcs)
      pAccUser->add_shownpcs(s);
  }

  // first action
  BlobFirstActionDone* pFirst = oBlob.mutable_action();
  if (pFirst != nullptr)
  {
    pFirst->Clear();
    pFirst->set_action(m_stBlobData.dwFirstAction);
  }

  // show
  BlobShow* pShow = oBlob.mutable_show();
  if (pShow != nullptr)
  {
    pShow->Clear();
    for (auto &s : m_stBlobData.setActions)
      pShow->add_actions(s);
    for (auto &s : m_stBlobData.setExpressions)
      pShow->add_expressions(s);
  }

  // known map
  BlobKnownMaps* pKnownMap = oBlob.mutable_knownmap();
  if (pKnownMap != nullptr)
  {
    pKnownMap->Clear();
    pKnownMap->set_version(m_stBlobData.dwKnownMapVer);
    for (auto &s : m_stBlobData.setKnownMaps)
      pKnownMap->add_list(s);
  }

  // gm effect
  BlobGMEffects* pEffect = oBlob.mutable_effect();
  if (pEffect != nullptr)
  {
    for (auto &v : m_stBlobData.gms)
    {
      BlobGMEffectItem* pGm = pEffect->add_list();
      if (pGm != nullptr)
      {
        pGm->set_mapid(v.mapid);
        pGm->set_index(v.index);
        pGm->set_gmcommand(v.gmeffect);
      }
    }
  }

  // trace
  BlobTrace* pTrace = oBlob.mutable_trace();
  if (pTrace != nullptr)
  {
    pTrace->Clear();
    for (auto v = m_stBlobData.vecTraceItem.begin(); v != m_stBlobData.vecTraceItem.end(); ++v)
    {
      TraceItem* pItem = pTrace->add_items();
      pItem->set_itemid(v->dwItemID);
      pItem->set_monsterid(v->dwMonsterID);
    }
  }

  // option
  BlobOption* pOption = oBlob.mutable_opt();
  if (pOption != nullptr)
  {
    pOption->Clear();
    pOption->set_type(m_stOptionData.eType);
    pOption->set_wedding_type(m_stOptionData.eWeddingType);
    pOption->set_normalskill_option(m_stOptionData.m_dwNormalSkillOption);
    pOption->set_fashionhide(m_stOptionData.m_dwFashionHide);
    pOption->set_bitopt(m_stOptionData.m_btSet.to_ulong());
    for (auto &m : m_stOptionData.m_mapSkillOpt)
    {
      SkillOption* pOpt = pOption->add_skillopts();
      if (pOpt == nullptr)
        continue;
      pOpt->set_opt(m.first);
      pOpt->set_value(m.second);
    }
  }

  // userzone
  BlobUserZone* pUserZone = oBlob.mutable_userzone();
  if (pUserZone != nullptr)
  {
    pUserZone->Clear();
    for (auto &v : m_stBlobData.vecRecentZones)
    {
      RecentZoneInfo* pInfo = pUserZone->add_infos();
      if (pInfo != nullptr)
      {
        pInfo->set_type(v.eType);
        pInfo->set_zoneid(v.dwZoneID);
      }
    }
  }

  // activity data
  for (auto &m : m_stBlobData.mapActivityData)
  {
    // 活动结束, 无效数据清除
    if (ActivityConfig::getMe().hasActivity(m.first) == false)
    {
      XLOG << "[玩家活动数据-保存], 活动" << m.first << "已经结束" << "玩家:" << m_pUser->name << m_pUser->id << "清空活动数据" << XEND;
      continue;
    }
    ActivityCommonData* pData = oBlob.mutable_activity()->add_activitydatas();
    if (pData == nullptr)
      continue;
    pData->set_name(m.first);
    for (auto &v : m.second.vecParams)
      pData->add_params(v);
  }

  BlobBattle* pBattle = oBlob.mutable_battle();
  if (pBattle)
    pBattle->set_totalbattletime(m_stBlobData.dwTotalBattleTime);
  // relation
  /*BlobRelation* pRelation = oBlob.mutable_relation();
  for (auto &s : m_stBlobData.setBlackIDs)
    pRelation->add_black(s);*/
  BlobSeeNpc* pSeeNpc = oBlob.mutable_seenpc();
  if (pSeeNpc)
  {
    for (auto &s : m_stBlobData.setSeeNpcs)
      pSeeNpc->add_see(s);
    for (auto &s : m_stBlobData.setHideNpcs)
      pSeeNpc->add_hide(s);
  }

  // guild raid
  BlobGuildRaid* pGuildRaid = oBlob.mutable_guildraid();
  if (pGuildRaid) {
    pGuildRaid->Clear();
    for (auto &m : m_stBlobData.mapGuildRaidData) {
      GuildRaid* pRaid = pGuildRaid->add_raid();
      if (pRaid == nullptr)
        continue;
      pRaid->set_npcid(m.second.dwNpcId);
      pRaid->set_state(m.second.eState);
      for (auto v : m.second.setKilledBoss)
        pRaid->add_killedboss(v);
    }
    if (m_stBlobData.m_dwGuildRaidVersion)
      pGuildRaid->set_raidversion(m_stBlobData.m_dwGuildRaidVersion);
  }

  // settings
  BlobSettings* pSettings = oBlob.mutable_settings();
  if (pSettings)
  {
    pSettings->set_transmap(m_stBlobData.dwTransMap);
    pSettings->set_trans_x(m_stBlobData.oTransPos.x);
    pSettings->set_trans_y(m_stBlobData.oTransPos.y);
    pSettings->set_trans_z(m_stBlobData.oTransPos.z);
  }

  return true;
}

bool UserSceneData::loadActivity(const BlobActivityEvent& acc_data, const BlobActivityEvent& char_data)
{
  m_stBlobData.stActivityEvent.fromData(acc_data, char_data);
  return true;
}

bool UserSceneData::saveActivity(BlobActivityEvent* acc_data, BlobActivityEvent* char_data)
{
  if (acc_data == nullptr || char_data == nullptr)
    return false;
  m_stBlobData.stActivityEvent.toData(acc_data, char_data);
  return true;
}

bool UserSceneData::loadCredit(const BlobNewCredit& oCredit)
{
  // credit
  //BlobCredit creditData;
  //string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_USER_CREDIT, m_pUser->accid);
  //RedisManager::getMe().getProtoData(key, &creditData);
  m_stBlobData.stCredit.fromData(oCredit);
  // 多账号公用信用度, 且同时被禁言
  if (m_stBlobData.stCredit.dwForbidRecoverTime > m_oBaseData.gagtime())
    setGagTime(m_stBlobData.stCredit.dwForbidRecoverTime);
  return true;
}

bool UserSceneData::saveCredit(BlobNewCredit* pCredit)
{
  // credit data
  //BlobCredit creditData;
  //m_stBlobData.stCredit.toData(&creditData);
  //string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_USER_CREDIT, m_pUser->accid);
  //RedisManager::getMe().setProtoData(key, &creditData);

  if (pCredit == nullptr)
  {
    XERR << "[玩家-信用数据存储]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "序列化失败" << XEND;
    return false;
  }
  m_stBlobData.stCredit.toData(pCredit);
  return true;
}

bool UserSceneData::loadAccUser(const BlobAccUser& rData)
{
  m_stAccData.fromData(rData);

  // updateMaxBaseLv(getRolelv());
  return true;
}

bool UserSceneData::saveAccUser(BlobAccUser* pData)
{
  if (pData == nullptr)
    return false;
  m_stAccData.toData(pData);
  return true;
}

bool UserSceneData::loadBoss(const BlobBoss& rData)
{
  m_oBoss.CopyFrom(rData);
  return true;
}

bool UserSceneData::saveBoss(BlobBoss* pData)
{
  if (pData == nullptr)
    return false;
  pData->CopyFrom(m_oBoss);
  return true;
}

void UserSceneData::setOnlineMapID(DWORD mapID)
{
  if (m_oBaseData.mapid() != mapID)
    m_oBaseData.set_mapid(mapID);
}

void UserSceneData::setGender(EGender gender)
{
  if (gender <= EGENDER_MIN || gender >= EGENDER_MAX || EGender_IsValid(gender) == false)
    return;

  m_oBaseData.set_gender(gender);
  m_pUser->setDataMark(EUSERDATATYPE_SEX);
}

void UserSceneData::setRolelv(DWORD value)
{
  m_oBaseData.set_rolelv(value);
  m_pUser->setDataMark(EUSERDATATYPE_ROLELEVEL);
}

void UserSceneData::setRoleexp(QWORD value)
{
  m_oBaseData.set_roleexp(value);
  m_pUser->setDataMark(EUSERDATATYPE_ROLEEXP);
}

void UserSceneData::setPurify(DWORD value)
{
  m_stBlobData.dwPurify = value;
  m_pUser->setDataMark(EUSERDATATYPE_PURIFY);
}

void UserSceneData::setOnlineTime()
{
  m_pUser->setDataMark(EUSERDATATYPE_ONLINETIME);
}

void UserSceneData::setOfflineTime(DWORD time)
{
  m_oBaseData.set_offlinetime(time);
  m_pUser->setDataMark(EUSERDATATYPE_OFFLINETIME);
}

void UserSceneData::setCreateTime(DWORD dwTime)
{
  m_oBaseData.set_createtime(dwTime);
}

void UserSceneData::setProfession(EProfession eProfession)
{
  if (eProfession <= EPROFESSION_MIN || eProfession >= EPROFESSION_MAX)
    return;

  m_oBaseData.set_profession(eProfession);
  m_pUser->setDataMark(EUSERDATATYPE_PROFESSION);
}

EProfession UserSceneData::getProfession() const
{
 // if (m_pUser->m_oTmpData.m_eProfession) return m_pUser->m_oTmpData.m_eProfession;
  return m_oBaseData.profession();
}

void UserSceneData::setMaxPro()
{
  DWORD dwProfession = static_cast<DWORD>(m_oBaseData.profession());
  if(dwProfession%10 <= m_oBaseData.maxpro()%10)
    return;

  m_oBaseData.set_maxpro(dwProfession);
}

void UserSceneData::setCharge(DWORD value)
{
  m_oBaseData.set_charge(value);
  m_pUser->setDataMark(EUSERDATATYPE_CHARGE);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setDiamond(DWORD value)
{
  m_oBaseData.set_diamond(value);
  m_pUser->setDataMark(EUSERDATATYPE_DIAMOND);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setSilver(QWORD value)
{
  m_oBaseData.set_silver(value);
  refreshDebt();
  m_pUser->setDataMark(EUSERDATATYPE_SILVER);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setGold(DWORD value)
{
  m_oBaseData.set_gold(value);
  m_pUser->setDataMark(EUSERDATATYPE_GOLD);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setGarden(DWORD value)
{
  m_oBaseData.set_garden(value);
  m_pUser->setDataMark(EUSERDATATYPE_GARDEN);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setFriendShip(DWORD value)
{
  if (m_pUser == nullptr)
    return;
  m_oBaseData.set_friendship(value);
  m_pUser->setDataMark(EUSERDATATYPE_FRIENDSHIP);
  m_pUser->refreshDataAtonce();
}

DWORD UserSceneData::getFriendShip()
{
  if (m_pUser == nullptr)
    return 0;
  return m_oBaseData.friendship();
}

void UserSceneData::addZenyDebt(QWORD qwMax, QWORD qwDebt)
{
  if (m_pUser == nullptr)
    return;
  m_stBlobData.qwZenyMax = qwMax;
  m_stBlobData.qwZenyDebt += qwDebt;
  refreshDebt();
  m_pUser->setDataMark(EUSERDATATYPE_ZENY_DEBT);
}

void UserSceneData::setClothColor(DWORD value, bool bRealUse /*= true*/)
{
  if(bRealUse == false)
  {
    m_pairClothColor.second = m_pairClothColor.first;
    m_pairClothColor.first = value;
  }
  else
    m_stBlobData.dwClothColor = value;

  m_pUser->setDataMark(EUSERDATATYPE_CLOTHCOLOR);
  m_pUser->refreshDataAtonce();

  // use this proto inform client to nine
  UseDressing cmd;
  cmd.set_type(EDRESSTYPE_CLOTH);
  cmd.set_id(value);
  cmd.set_charid(m_pUser->id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);
}

DWORD UserSceneData::getClothColor(bool bReal /*= false*/) const
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pairClothColor.first != 0)
    return m_pairClothColor.first;

  if (bReal)
  {
    if (m_pUser->getProfession() % 10 < 4)
      return 1;
    return m_stBlobData.dwClothColor == 0 ? 1 : m_stBlobData.dwClothColor;
  }

  if (m_pUser->isBeMagicMachine() && MiscConfig::getMe().getItemCFG().isNoColorMachineMount(m_pUser->getEquipID(EEQUIPPOS_MOUNT)))
    return 0;

  if (isFashionHide(EFASHIONHIDETYPE_BODY) == false)
  {
    auto getFromEquip = [&](BasePackage* pPkg)->DWORD {
      if (pPkg == nullptr)
        return 0;
      ItemEquip* pEquip = pPkg->getEquip(EEQUIPPOS_ARMOUR);
      if (!pEquip || pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false || pEquip->getCFG() == nullptr || pEquip->getCFG()->dwBody == 0)
        return 0;
      return pEquip->getBodyColor();
    };
    FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
    DWORD color = getFromEquip(pFashionEquipPack);
    if (color)
     return color;
    EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    color = getFromEquip(pPack);
    if (color)
      return color;
  }

  if (m_pUser->getProfession() % 10 < 4)
    return 1;

  return m_stBlobData.dwClothColor == 0 ? 1 : m_stBlobData.dwClothColor;
}

DWORD UserSceneData::getBody(bool bGetReal/* = false*/)
{
  if (!m_pUser) return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARMOUR) != 0)
  {
    const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARMOUR));
    if (pCFG != nullptr && pCFG->dwBody != 0)
      return pCFG->dwBody;
  }

  //setClothColor(0);
  m_pUser->setDataMark(EUSERDATATYPE_CLOTHCOLOR);
  if (m_pUser->m_oTmpData.m_dwBody)
    return m_pUser->m_oTmpData.m_dwBody;

  const SRoleBaseCFG* pCFG = m_pUser->getRoleBaseCFG();
  if (pCFG == nullptr)
    return 0;

  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_BODY);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_BODY) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_BODY);

  auto getBodyFromEquip = [&](BasePackage* pPkg)->DWORD {
    if (pPkg == nullptr)
      return 0;
    ItemEquip* pEquip = pPkg->getEquip(EEQUIPPOS_ARMOUR);
    if (!pEquip)
      return 0;
    if (pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == false)
      return 0;
    if (pEquip->getCFG() == nullptr)
      return 0;
    if (pEquip->getCFG()->dwBody == 0)
      return 0;

    //setClothColor(pEquip->getBodyColor());
    return pEquip->getCFG()->dwBody;
  };

  if (isFashionHide(EFASHIONHIDETYPE_BODY) == false)
  {
    FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
    DWORD body = getBodyFromEquip(pFashionEquipPack);
    if (body)
     return body;
    EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
    body = getBodyFromEquip(pPack);
    if (body)
      return body;
  }

  if (getGender() == EGENDER_MALE)
    return pCFG->maleBody;
  else if (getGender() == EGENDER_FEMALE)
    return pCFG->femaleBody;

  return 0;
}

DWORD UserSceneData::getRealBody()
{
  return getBody(true);
}

DWORD UserSceneData::getBodyScale(bool bGetReal/* = false*/)
{
  if (!m_pUser)
    return 0;
  float buffscaleper = m_pUser->m_oBuff.getBodyScalePer();
  if (buffscaleper == 0)
    buffscaleper = 1;
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_BODYSCALE) * buffscaleper;
  //if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_BODYSCALE))
    //return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_BODYSCALE);
  return 100 * buffscaleper;
}

DWORD UserSceneData::getLefthand(bool bGetReal/* = false*/)
{
  if (!m_pUser)
    return 0;
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_LEFTHAND);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_LEFTHAND) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_LEFTHAND);

  if(m_pUser->getDressUp().getDressUpStatus() != 0)
  {
    if (getProfession() >= EPROFESSION_ASSASSIN && getProfession() <= EPROFESSION_GUILLOTINECROSS && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON);
    else if (getProfession() >= EPROFESSION_CRUSADER && getProfession() <= EPROFESSION_ROYALGUARD && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_SHIELD) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_SHIELD);
    else if (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON) != 0)
    {
      const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON));
      if (pCFG != nullptr && pCFG->eItemType != EITEMTYPE_WEAPON_FIST && pCFG->eItemType != EITEMTYPE_ARTIFACT_FIST)
        return 0;
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON);
    }
  }

  ItemEquip* pFashionEquip = nullptr;
  FashionEquipPackage* pFEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFEquipPack != nullptr)
  {
    if (getProfession() >= EPROFESSION_ASSASSIN && getProfession() <= EPROFESSION_GUILLOTINECROSS)
      pFashionEquip = pFEquipPack->getEquip(EEQUIPPOS_WEAPON);
    else if (getProfession() >= EPROFESSION_CRUSADER && getProfession() <= EPROFESSION_ROYALGUARD)
      pFashionEquip = pFEquipPack->getEquip(EEQUIPPOS_SHIELD);
    else if (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA)
    {
      pFashionEquip = pFEquipPack->getEquip(EEQUIPPOS_WEAPON);
      if (pFashionEquip == nullptr || pFashionEquip->getType() != EITEMTYPE_WEAPON_FIST)
        pFashionEquip = nullptr;
    }
  }

  ItemEquip* pEquip = nullptr;
  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  bool bIsArtifact = false;
  if (pPack != nullptr)
  {
    if ((getProfession() >= EPROFESSION_ASSASSIN && getProfession() <= EPROFESSION_GUILLOTINECROSS) ||
        (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA))
    {
      pEquip = pPack->getEquip(EEQUIPPOS_ARTIFACT);
      if (pEquip == nullptr)
        pEquip = pPack->getEquip(EEQUIPPOS_WEAPON);
      else
        bIsArtifact = true;
    }
    else if (getProfession() >= EPROFESSION_CRUSADER && getProfession() <= EPROFESSION_ROYALGUARD)
    {
      pEquip = pPack->getEquip(EEQUIPPOS_SHIELD);
      if (pEquip != nullptr)
      {
        ItemEquip* pRightEquip = pPack->getEquip(EEQUIPPOS_WEAPON);
        if (pRightEquip != nullptr &&
            (pRightEquip->getType() != EITEMTYPE_WEAPON_SWORD && pRightEquip->getType() != EITEMTYPE_WEAPON_DAGGER &&
             pRightEquip->getType() != EITEMTYPE_WEAPON_HAMMER && pRightEquip->getType() != EITEMTYPE_WEAPON_AXE &&
             pRightEquip->getType() != EITEMTYPE_WEAPON_LANCE))
          pEquip = nullptr;
        ItemEquip* pRightArtifact = pPack->getEquip(EEQUIPPOS_ARTIFACT);
        if (pRightArtifact != nullptr &&
            (pRightArtifact->getType() != EITEMTYPE_ARTIFACT_SWORD && pRightArtifact->getType() != EITEMTYPE_ARTIFACT_DAGGER &&
             pRightArtifact->getType() != EITEMTYPE_ARTIFACT_HAMMER && pRightArtifact->getType() != EITEMTYPE_ARTIFACT_AXE &&
             pRightArtifact->getType() != EITEMTYPE_ARTIFACT_LANCE))
          pEquip = nullptr;
      }
    }

    if (pEquip != nullptr && pEquip->canEquip(getProfession()) == true)
    {
      if (pFashionEquip != nullptr && (pEquip->getType() == pFashionEquip->getType() || bIsArtifact))
      {
        if (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA && pFashionEquip->getType() != EITEMTYPE_WEAPON_FIST)
          return 0;
        else
          return pFashionEquip->getTypeID();
      }
      else
      {
        if (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA && pEquip->getType() != EITEMTYPE_WEAPON_FIST && pEquip->getType() != EITEMTYPE_ARTIFACT_FIST)
          return 0;
        else
          return pEquip->getTypeID();
      }
    }
  }

  if (pFashionEquip != nullptr)
    return pFashionEquip->getTypeID();

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr && pFighter->getRoleCFG() != nullptr)
  { 
    if ((getProfession() >= EPROFESSION_ASSASSIN && getProfession() <= EPROFESSION_GUILLOTINECROSS) ||
        (getProfession() >= EPROFESSION_MONK && getProfession() <= EPROFESSION_SHURA))
      return pFighter->getRoleCFG()->defaultWeapon;
      else if (getProfession() >= EPROFESSION_CRUSADER && getProfession() <= EPROFESSION_ROYALGUARD)
      {
        if (pEquip != nullptr)
          return pEquip->getTypeID();
      }
  }
  return 0;
}

DWORD UserSceneData::getRighthand(bool bGetReal/* = false*/)
{
  if (!m_pUser)
    return 0;
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_RIGHTHAND);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_RIGHTHAND) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_RIGHTHAND);

  if(m_pUser->getDressUp().getDressUpStatus() != 0)
  {
    if(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON) == 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARTIFACT) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARTIFACT);
    else if(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_WEAPON);
  }

  ItemEquip* pFashionEquip = nullptr;
  FashionEquipPackage* pFEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFEquipPack != nullptr)
  {
    pFashionEquip = pFEquipPack->getEquip(EEQUIPPOS_WEAPON);
  }

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    if (pFashionEquip == nullptr)
    {
      ItemEquip* pArtifact = pPack->getEquip(EEQUIPPOS_ARTIFACT);
      if (pArtifact != nullptr && pArtifact->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
        return pArtifact->getTypeID();
    }

    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_WEAPON);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
    {
      if (pFashionEquip != nullptr && pEquip->getType() == pFashionEquip->getType())
        return pFashionEquip->getTypeID();
      else
        return pEquip->getTypeID();
    }
  }

  if (pFashionEquip != nullptr)
    return pFashionEquip->getTypeID();

  SceneFighter* pFighter = m_pUser->getFighter();
  if (pFighter != nullptr && pFighter->getRoleCFG() != nullptr)
    return pFighter->getRoleCFG()->defaultWeapon;
  return 0;
}

DWORD UserSceneData::getHead(bool bGetReal/* = false*/)
{
  if (!m_pUser)
    return 0;
  if (m_pUser->m_oTmpData.m_dwHead) return m_pUser->m_oTmpData.m_dwHead;

  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_HEAD);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_HEAD) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_HEAD);

  if (isFashionHide(EFASHIONHIDETYPE_HEAD) == true)
    return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_HEAD) != 0)
    return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_HEAD);

  FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFashionEquipPack != nullptr)
  {
    ItemEquip* pEquip = pFashionEquipPack->getEquip(EEQUIPPOS_HEAD);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    ItemEquip* pArtifact = pPack->getEquip(EEQUIPPOS_ARTIFACT_HEAD);
    if (pArtifact && pArtifact->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pArtifact->getTypeID();
    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_HEAD);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  return 0;
}

DWORD UserSceneData::getBack(bool bGetReal/* = false*/)
{
  //变身后，去掉背部装饰
  if (!m_pUser)
    return 0;
  if (m_pUser->m_oTmpData.m_dwBack) return m_pUser->m_oTmpData.m_dwBack;
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_BACK);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_BACK) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_BACK);

  if (isFashionHide(EFASHIONHIDETYPE_BACK) == true)
    return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0)
  {
    if(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_BACK) == 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARTIFACT_BACK) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_ARTIFACT_BACK);
    else if(m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_BACK) != 0)
      return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_BACK);
  }

  FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFashionEquipPack != nullptr)
  {
    ItemEquip* pEquip = pFashionEquipPack->getEquip(EEQUIPPOS_BACK);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    ItemEquip* pArtifact = pPack->getEquip(EEQUIPPOS_ARTIFACT_BACK);
    if (pArtifact && pArtifact->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pArtifact->getTypeID();
    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_BACK);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  return 0;
}

DWORD UserSceneData::getFace(bool bGetReal)
{
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_FACE);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_FACE) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_FACE);

  if (isFashionHide(EFASHIONHIDETYPE_FACE) == true)
    return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_FACE) != 0)
    return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_FACE);

  FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFashionEquipPack != nullptr)
  {
    ItemEquip* pEquip = pFashionEquipPack->getEquip(EEQUIPPOS_FACE);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_FACE);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  return 0;
}

DWORD UserSceneData::getTail(bool bGetReal)
{
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_TAIL);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_TAIL) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_TAIL);

  if (isFashionHide(EFASHIONHIDETYPE_TAIL) == true)
    return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_TAIL) != 0)
    return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_TAIL);

  FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFashionEquipPack != nullptr)
  {
    ItemEquip* pEquip = pFashionEquipPack->getEquip(EEQUIPPOS_TAIL);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_TAIL);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  return 0;
}

DWORD UserSceneData::getMount(bool bGetReal)
{
  if (m_pUser->m_oTmpData.m_dwMount) return m_pUser->m_oTmpData.m_dwMount;

  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_MOUNT);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_MOUNT) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_MOUNT);

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_MOUNT) != 0)
    return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_MOUNT);

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack != nullptr)
  {
    ItemEquip* pEquip = pEquipPack->getEquip(EEQUIPPOS_MOUNT);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) )
    {
      FashionEquipPackage* pFEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
      if(pFEquipPack != nullptr)
      {
        ItemEquip* pFashionEquip = pFEquipPack->getEquip(EEQUIPPOS_MOUNT);
        if(pFashionEquip != nullptr && pFashionEquip->canEquip(m_pUser->getUserSceneData().getProfession()) )
          return pFashionEquip->getTypeID();
      }
      return pEquip->getTypeID();
    }
  }
   return 0;
}

DWORD UserSceneData::getMouth(bool bGetReal)
{
  if (!bGetReal && m_pUser->getTransform().isInTransform() && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getTransform(EUSERDATATYPE_MOUTH);
  if (!bGetReal && m_pUser->m_oBuff.hasPartTransform(EUSERDATATYPE_MOUTH) && m_pUser->getDressUp().getDressUpStatus() == 0)
    return m_pUser->m_oBuff.getPartTransform(EUSERDATATYPE_MOUTH);

  if (isFashionHide(EFASHIONHIDETYPE_MOUTH) == true)
    return 0;

  if(m_pUser->getDressUp().getDressUpStatus() != 0 && m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_MOUTH) != 0)
    return m_pUser->getDressUp().getDressUpEquipID(EEQUIPPOS_MOUTH);

  FashionEquipPackage* pFashionEquipPack = dynamic_cast<FashionEquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_FASHIONEQUIP));
  if (pFashionEquipPack != nullptr)
  {
    ItemEquip* pEquip = pFashionEquipPack->getEquip(EEQUIPPOS_MOUTH);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }
  EquipPackage* pPack = dynamic_cast<EquipPackage*>(m_pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pPack != nullptr)
  {
    ItemEquip* pEquip = pPack->getEquip(EEQUIPPOS_MOUTH);
    if (pEquip != nullptr && pEquip->canEquip(m_pUser->getUserSceneData().getProfession()) == true)
      return pEquip->getTypeID();
  }

  return 0;
}

void UserSceneData::setBlink(bool bBlink)
{
  m_stOnlineData.bBlink = bBlink;
  m_pUser->setDataMark(EUSERDATATYPE_BLINK);
}

DWORD UserSceneData::getShaderColor()
{
  // default value : 0
  return m_pUser->m_oBuff.getShaderColor();
}

// blob data
bool UserSceneData::isNewMap(DWORD mapid)
{
  return m_stBlobData.setKnownMaps.find(mapid) == m_stBlobData.setKnownMaps.end();
}

bool UserSceneData::addNewMap(DWORD mapid)
{
  if (isNewMap(mapid) == false)
    return false;

  m_stBlobData.setKnownMaps.insert(mapid);
  XLOG << "[玩家场景数据-添加新地图] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << ", 添加 mapid : " << mapid << " 成功" << XEND;

  addMapArea(mapid);
  return true;
}

void UserSceneData::setSaveMap(DWORD mapid)
{
  m_stBlobData.dwSaveMap = mapid;
  m_pUser->setDataMark(EUSERDATATYPE_SAVEMAP);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setDeadCoin(DWORD dwCount)
{
  if (m_stBlobData.dwDeadCoin == dwCount)
    return;
  m_stBlobData.dwDeadCoin = dwCount;
  m_pUser->setDataMark(EUSERDATATYPE_DEADCOIN);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setDeadLv(DWORD dwLv)
{
  if (m_stBlobData.dwDeadLv == dwLv)
    return;
  m_stBlobData.dwDeadLv = dwLv;
  m_pUser->setDataMark(EUSERDATATYPE_DEADLV);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setDeadExp(DWORD dwExp)
{
  if (m_stBlobData.dwDeadExp == dwExp)
    return;
  m_stBlobData.dwDeadExp = dwExp;
  m_pUser->setDataMark(EUSERDATATYPE_DEADEXP);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setFollowerID(QWORD qwID, EFollowType eType/*=EFOLLOWTYPE_MIN*/)
{
  if (qwID == 0)
  {
    setFollowerIDNoCheck(0);
    return;
  }
  else if(m_pUser->getDressUp().getDressUpStatus() != 0)
    return;

  if (m_pUser->m_oHands.has() && m_pUser->m_oHands.isFollower())
  {
    if (qwID != m_pUser->m_oHands.getOtherID())
      m_pUser->m_oHands.breakup();
    else if (eType != EFOLLOWTYPE_HAND)
      m_pUser->m_oHands.breakup();
  }

  switch(eType)
  {
    case EFOLLOWTYPE_MIN:
      {
        if (m_stBlobData.qwFollowerID == qwID)
        {
          setFollowerIDNoCheck(qwID);
          return;
        }
      }
      break;
    case EFOLLOWTYPE_HAND:
      {
        setFollowerIDNoCheck(qwID, eType);
        return;
      }
      break;
    default:
      break;
  }

  FollowerIDCheck cmd;
  cmd.set_userid(m_pUser->id);
  cmd.set_followid(qwID);
  cmd.set_etype(eType);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void UserSceneData::setFollowerIDNoCheck(QWORD qwID, EFollowType eType/* = EFOLLOWTYPE_MIN*/)
{
  if (m_pUser->m_oHands.has() && m_pUser->m_oHands.isFollower())
  {
    if (qwID != m_pUser->m_oHands.getOtherID())
      m_pUser->m_oHands.breakup();
  }

  switch(eType)
  {
    case EFOLLOWTYPE_MIN:
      {
        //if (m_stBlobData.qwFollowerID == qwID)
        //  break;

        QWORD beFollowerId = m_stBlobData.qwFollowerID;
        m_stBlobData.qwFollowerID = qwID;
        m_pUser->setDataMark(EUSERDATATYPE_FOLLOWID);
        m_pUser->refreshDataAtonce();

        EFollowType newType = EFOLLOWTYPE_MIN;
        if (qwID == 0)
        {
          newType = EFOLLOWTYPE_BREAK;
        }
        else
        {
          beFollowerId = qwID;
          newType = EFOLLOWTYPE_MIN;
        }

        BeFollowUserCmd message;
        message.set_userid(m_pUser->id);
        message.set_etype(newType);
        PROTOBUF(message, send, len);
        thisServer->sendCmdToMe(beFollowerId, send, len);
      }
      break;
    case EFOLLOWTYPE_HAND:
      {
        SceneUser* user = SceneUserManager::getMe().getUserByID(qwID);
        if (user == nullptr || user->getUserSceneData().getOnlineMapID() != getOnlineMapID() || user->m_oHands.has())
          return;
        if (m_pUser->m_oHands.has())
          return;

        if (m_stBlobData.qwFollowerID != qwID)
        { 
          //发送打断跟随给原来跟随的人
          BeFollowUserCmd message;
          message.set_userid(m_pUser->id);
          message.set_etype(EFOLLOWTYPE_BREAK);
          PROTOBUF(message, send, len);
          thisServer->sendCmdToMe(m_stBlobData.qwFollowerID, send, len);                 
          
          m_stBlobData.qwFollowerID = qwID;
          m_pUser->setDataMark(EUSERDATATYPE_FOLLOWID);
          m_pUser->refreshDataAtonce();
        }
        m_pUser->m_oHands.setWaitStatus();
        m_pUser->m_oHands.setMaster(qwID);
        user->m_oHands.setWaitStatus();
        user->m_oHands.setFollower(m_pUser->id);

        BeFollowUserCmd message;
        message.set_userid(m_pUser->id);
        message.set_etype(EFOLLOWTYPE_HAND);
        PROTOBUF(message, send, len);
        user->sendCmdToMe(send, len);
      }
      break;
    default:
      break;
  }
  FollowerUser cmd;
  cmd.set_userid(qwID);
  cmd.set_etype(eType);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[跟随] " << m_pUser->id << m_pUser->name << "type" << eType << "跟随" << qwID << XEND;
}

void UserSceneData::setLastMapID(DWORD dwLastMapID, DWORD dwLastRealMapID)
{
  m_stBlobData.dwLastMap = dwLastMapID;
  m_stBlobData.dwLastRealMap = dwLastRealMapID;
}

void UserSceneData::setLevelUpTime(DWORD dwTime)
{
  m_stBlobData.dwLevelUpTime = dwTime;
}

void UserSceneData::setDir(DWORD dwDir)
{
  if (m_stOnlineData.dwDir == dwDir)
    return;

  m_stOnlineData.dwDir = dwDir;
  m_pUser->setDataMark(EUSERDATATYPE_DIR);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setQueryType(EQueryType eType)
{
  if (eType <= EQUERYTYPE_MIN || eType >= EQUERYTYPE_MAX || EQueryType_IsValid(eType) == false)
    return;

  m_stOptionData.eType = eType;
  m_pUser->setDataMark(EUSERDATATYPE_QUERYTYPE);
  m_pUser->refreshDataAtonce();

  GCharWriter gChar(thisServer->getRegionID(), m_pUser->id);
  gChar.setVersion();
  gChar.setQueryType(eType);
  gChar.save();

  XLOG << "[玩家场景数据-查询标志]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置标志" << m_stOptionData.eType << XEND;
}

void UserSceneData::setQueryWeddingType(EQueryType eType)
{
  if (eType <= EQUERYTYPE_MIN || eType >= EQUERYTYPE_MAX || EQueryType_IsValid(eType) == false)
    return;
  if (m_stOptionData.eWeddingType == eType)
    return;

  m_stOptionData.eWeddingType = eType;
  m_pUser->setDataMark(EUSERDATATYPE_QUERYWEDDINGTYPE);
  m_pUser->refreshDataAtonce();

  GCharWriter gChar(thisServer->getRegionID(), m_pUser->id);
  gChar.setVersion();
  gChar.setQueryWeddingType(eType);
  gChar.save();

  XLOG << "[玩家场景数据-查询婚姻标志]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置标志" << m_stOptionData.eWeddingType << XEND;
}

void UserSceneData::setNormalSkillOption(DWORD option) 
{
  m_stOptionData.m_dwNormalSkillOption = option;
  m_pUser->setDataMark(EUSERDATATYPE_NORMALSKILL_OPTION);
  m_pUser->refreshDataAtonce();
}

bool UserSceneData::addPatchVersion(DWORD dwVersion)
{
  if (hasPatchVersion(dwVersion) == true)
    return false;
  m_stBlobData.setPatchVersion.insert(dwVersion);
  return true;
}

bool UserSceneData::addAccPatchVersion(QWORD qwVersion)
{
  if (hasAccPatchVersion(qwVersion) == true)
    return false;
  m_stAccData.setPatchVersion.insert(qwVersion);
  return true;
}

void UserSceneData::addMapArea(DWORD dwMapID)
{
  const SMapCFG* pBase = MapConfig::getMe().getMapCFG(dwMapID);
  if (pBase == nullptr)
    return;

  DWORD dwArea = pBase->dwArea;
  if (dwArea == 0)
    return;

  auto s = m_stBlobData.setMapArea.find(dwArea);
  if (s != m_stBlobData.setMapArea.end())
    return;

  m_stBlobData.setMapArea.insert(dwArea);

  NewMapAreaNtf cmd;
  cmd.set_area(dwArea);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[玩家场景数据-添加area地图] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 添加 area : " << dwArea << " mapid : " << dwMapID << " 成功" << XEND;
}

void UserSceneData::sendMapAreaList()
{
  QueryMapArea cmd;
  for (auto s : m_stBlobData.setMapArea)
    cmd.add_areas(s);

  if (cmd.areas_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void UserSceneData::addGMCommand(DWORD index, string gm)
{
  SGmEffect oneGm;
  oneGm.mapid = m_oBaseData.mapid();
  oneGm.index = index;
  oneGm.gmeffect = gm;
  m_stBlobData.gms.push_back(oneGm);
}

void UserSceneData::delGMCommand(DWORD index)
{
  while(true)
  {
    auto iter = find_if(m_stBlobData.gms.begin(), m_stBlobData.gms.end(), [index, this](const SGmEffect& r) ->bool {
      return r.index == index && r.mapid == m_oBaseData.mapid();
    });

    if (iter == m_stBlobData.gms.end())
      break;
    m_stBlobData.gms.erase(iter);
  }
}

void UserSceneData::getGMCommand(vector<string>& gms) const
{
  for (auto &v : m_stBlobData.gms)
  {
    if (v.mapid == m_oBaseData.mapid())
      gms.push_back(v.gmeffect);
  }
}

void UserSceneData::addShowNpc(DWORD id, bool bShare /*= false*/)
{
  if (bShare)
    m_stAccData.setShowNpcs.insert(id);
  else
    m_stBlobData.setShowNpcs.insert(id);

  m_setShowNpc.insert(id);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::delShowNpc(DWORD id)
{
  m_stAccData.setShowNpcs.erase(id);
  m_stBlobData.setShowNpcs.erase(id);
  m_setShowNpc.erase(id);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::addAction(DWORD id)
{
  const SActionAnimBase* pBase = TableManager::getMe().getActionAnimCFG(id);
  if (pBase == nullptr)
  {
    XERR << "[玩家场景数据-添加新动作] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 添加新动作 " << id << " 未在 Table_ActionAnim.txt 表中找到" << XEND;
    return;
  }

  if (haveAction(id))
    return;

  m_stBlobData.setActions.insert(id);

  QueryShow cmd;
  cmd.add_actionid(id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[玩家场景数据-添加新动作] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 添加新动作 " << id << XEND;
}

void UserSceneData::delAction(DWORD id)
{
  const SActionAnimBase* pBase = TableManager::getMe().getActionAnimCFG(id);
  if (pBase == nullptr)
  {
    XERR << "[玩家场景数据-删除动作] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 删除动作 " << id << " 未在 Table_ActionAnim.txt 表中找到" << XEND;
    return;
  }

  if (!haveAction(id))
    return;
  
  m_stBlobData.setActions.erase(id);

  sendAllActions();

  XLOG << "[玩家场景数据-删除动作] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 删除动作 " << id << XEND;
}

void UserSceneData::addExpression(DWORD id)
{
  const SExpression* pBase = TableManager::getMe().getExpressionCFG(id);
  if (pBase == nullptr)
  {
    XERR << "[玩家场景数据-添加新表情] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << "  添加新表情 " << id << " 未在 Table_ActionAnim.txt 表中找到" << XEND;
    return;
  }

  if (haveExpression(id))
    return;
  m_stBlobData.setExpressions.insert(id);
  m_pUser->getAchieve().onExpressAdd();

  QueryShow cmd;
  cmd.add_expression(id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  XLOG << "[玩家场景数据-添加新表情] " << m_pUser->accid << ", " << m_pUser->id << ", " << m_pUser->getProfession() << ", " << m_pUser->name << " 添加新表情 " << id << XEND;
}

void UserSceneData::updateTraceItem(const UpdateTraceList& rCmd)
{
  for (int i = 0; i < rCmd.dels_size(); ++i)
  {
    DWORD dwItemID = rCmd.dels(i);
    auto v = find_if(m_stBlobData.vecTraceItem.begin(), m_stBlobData.vecTraceItem.end(), [dwItemID](const STraceItem& r) -> bool{
      return r.dwItemID == dwItemID;
    });
    if (v != m_stBlobData.vecTraceItem.end())
      m_stBlobData.vecTraceItem.erase(v);
  }

  for (int i = 0; i < rCmd.updates_size(); ++i)
  {
    STraceItem stItem(rCmd.updates(i).itemid(), rCmd.updates(i).monsterid());
    auto v = find_if(m_stBlobData.vecTraceItem.begin(), m_stBlobData.vecTraceItem.end(), [stItem](const STraceItem& r) -> bool{
      return r.dwItemID == stItem.dwItemID && r.dwMonsterID == stItem.dwMonsterID;
    });
    if (v != m_stBlobData.vecTraceItem.end())
      *v = stItem;
    else
      m_stBlobData.vecTraceItem.push_back(stItem);
  }
}

void UserSceneData::sendAllActions()
{
  QueryShow cmd;
  for (auto &s : m_stBlobData.setActions)
  {
    cmd.add_actionid(s);
  }
  for (auto &s : m_stBlobData.setExpressions)
  {
    cmd.add_expression(s);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserSceneData::sendTraceItemList()
{
  if (m_pUser == nullptr)
    return;

  QueryTraceList cmd;
  for (auto v = m_stBlobData.vecTraceItem.begin(); v != m_stBlobData.vecTraceItem.end(); ++v)
  {
    TraceItem* pItem = cmd.add_items();
    pItem->set_itemid(v->dwItemID);
    pItem->set_monsterid(v->dwMonsterID);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserSceneData::initDefaultData()
{
  const SNewRoleCFG& rCFG = MiscConfig::getMe().getNewRoleCFG();

  for (auto& v : rCFG.vecMapArea)
  {
    auto s = m_stBlobData.setMapArea.find(v);
    if (s == m_stBlobData.setMapArea.end())
      m_stBlobData.setMapArea.insert(v);
  }
}

void UserSceneData::refreshDebt()
{
  if (m_stBlobData.qwZenyDebt == 0 && m_stBlobData.qwZenyMax == 0)
    return;

  if (m_oBaseData.silver() > m_stBlobData.qwZenyMax)
  {
    QWORD qwDebt = m_oBaseData.silver() - m_stBlobData.qwZenyMax;
    m_oBaseData.set_silver(m_stBlobData.qwZenyMax);

    if (m_stBlobData.qwZenyDebt > qwDebt)
      m_stBlobData.qwZenyDebt -= qwDebt;
    else
    {
      QWORD qwLeft = qwDebt - m_stBlobData.qwZenyDebt;
      m_oBaseData.set_silver(m_oBaseData.silver() + qwLeft);
      m_stBlobData.qwZenyDebt = 0;
    }
    if (m_stBlobData.qwZenyDebt == 0)
      m_stBlobData.qwZenyMax = 0;
    m_pUser->setDataMark(EUSERDATATYPE_ZENY_DEBT);

    if (!m_stOnlineData.bDebtNtf)
    {
      MsgManager::sendMsg(m_pUser->id, ESYSTEMMSG_ID_DEBT_START);
      m_stOnlineData.bDebtNtf = true;
    }
  }
  else
  {
    m_stOnlineData.bDebtNtf = false;
  }

  XLOG << "[玩家数据-zeny负债]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
    << "zenymax :" << getZenyMax() << "zeny :" << getSilver() << "zenydebt :" << getZenyDebt() << XEND;
}

void UserSceneData::refreshPhotoMd5()
{
  DWORD dwNow = xTime::getCurSec();
  for (auto l = m_stAccData.listPhotoMd5.begin(); l != m_stAccData.listPhotoMd5.end();)
  {
    if (l->time() + DAY_T < dwNow)
    {
      XLOG << "[玩家-md5刷新]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << l->ShortDebugString() << "在" << dwNow << "超时,被删除" << XEND;
      l = m_stAccData.listPhotoMd5.erase(l);
      continue;
    }

    ++l;
  }
}

bool UserSceneData::addFirstActionDone(EFirstActionType eType, bool ntfClient /*= true*/)
{
  DWORD index = static_cast<DWORD> (eType);
  if (index <= 0 || index >= 31)
    return false;
  index -= 1;

  if (m_stBlobData.dwFirstAction & (1 << index))
    return false;

  m_stBlobData.dwFirstAction = m_stBlobData.dwFirstAction | (1 << index);
  m_pUser->refreshDataAtonce();

  if (ntfClient)
    sendFirstAction();
  return true;
}

void UserSceneData::addClientFirstAction(DWORD dwAction)
{
  if (dwAction == 0)
    return;
  if (m_stBlobData.dwFirstAction & dwAction)
    return;

  m_stBlobData.dwFirstAction = m_stBlobData.dwFirstAction | dwAction;
}

void UserSceneData::sendFirstAction()
{
  // 服务端触发第一次, 通知客户端
  FirstActionUserEvent cmd;
  cmd.set_firstaction(m_stBlobData.dwFirstAction);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserSceneData::setZoneID(DWORD zoneid, EJumpZone eMethod /*= EJUMPZONE_MIN*/)
{
  if (zoneid == m_oBaseData.zoneid())
    return;

  if (eMethod != EJUMPZONE_MIN)
  {
    if (eMethod <= EJUMPZONE_MIN || eMethod >= EJUMPZONE_MAX || EJumpZone_IsValid(eMethod) == false)
    {
      XERR << "[玩家-设置区]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置区 :" << zoneid << "方式method" << eMethod << "不合法" << XEND;
      return;
    }

    if (eMethod == EJUMPZONE_USER)
    {
      DWORD dwClientZone = getClientZoneID(zoneid);
      auto v = find_if(m_stBlobData.vecRecentZones.begin(), m_stBlobData.vecRecentZones.end(), [&](const SRecentZoneInfo& r) -> bool{
        return r.eType == eMethod && r.dwZoneID == dwClientZone;
      });
      if (v == m_stBlobData.vecRecentZones.end())
        m_stBlobData.vecRecentZones.push_back(SRecentZoneInfo(eMethod, dwClientZone));

      const SZoneMiscCFG& rCFG = MiscConfig::getMe().getZoneCFG();
      if (m_stBlobData.vecRecentZones.size() >= rCFG.dwMaxRecent)
        m_stBlobData.vecRecentZones.erase(m_stBlobData.vecRecentZones.begin());
    }

    m_pUser->getServant().onFinishEvent(ETRIGGER_WORLD_FREYJA);
  }

  m_oBaseData.set_zoneid(zoneid);
  if (m_pUser->m_oHands.has() == true)
    m_pUser->m_oHands.breakup();

  if (getFollowerID())
    setFollowerIDNoCheck(0);    //切线打断跟随

  // 切线关闭聊天室
  if (m_pUser->hasChatRoom())
  {
    ChatRoomManager::getMe().exitRoom(m_pUser, m_pUser->getChatRoomID());
  }

  XLOG << "[玩家-设置区]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "设置区:" << zoneid << "方式method" << eMethod << XEND;
  // m_pUser->unReg(UnregType::Normal);
  //
  ReconnectClientUserCmd send;
  send.charid = m_pUser->id;
  send.refresh = 1;
  thisServer->sendCmdToSession(&send, sizeof(send));
}

void UserSceneData::setDestZoneID(DWORD zoneid)
{
  m_oBaseData.set_destzoneid(zoneid);
  m_pUser->setDataMark(EUSERDATATYPE_DEST_ZONEID);
}

void UserSceneData::setOriginalZoneID(DWORD zoneid)
{
  m_oBaseData.set_originalzoneid(zoneid);
  m_pUser->setDataMark(EUSERDATATYPE_ORIGINAL_ZONEID);
}

void UserSceneData::setTotalBattleTime(DWORD timelen)
{
  if (m_stBlobData.dwTotalBattleTime == timelen)
    return;
  XLOG << "[玩家-无损战斗时间设置], 玩家:" << m_pUser->name << m_pUser->id << "设置前:" << m_stBlobData.dwTotalBattleTime << "设置为:" << timelen << XEND;

  m_stBlobData.dwTotalBattleTime = timelen;
}

void UserSceneData::setBattleTime(DWORD timelen)
{
  m_oBaseData.set_battletime(timelen);
  m_pUser->setDataMark(EUSERDATATYPE_BATTLETIME);
  XLOG << "[玩家-战斗时间], 战斗时间更新, 玩家: " << m_pUser->name << ", " << m_pUser->accid << ", " << m_pUser->id << "," << m_pUser->getProfession() << " 今日战斗时长:" << timelen << " s" << XEND;
}

void UserSceneData::setReBattleTime(DWORD timelen)
{
  m_oBaseData.set_rebattletime(timelen);
  m_pUser->setDataMark(EUSERDATATYPE_REBATTLETIME);
  XLOG << "[玩家-战斗时间], 获得额外战斗时间, 玩家: " << m_pUser->name << "," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << " 今日已获得额外时长:" << timelen << " s" << XEND;
}

void UserSceneData::setUsedBattleTime(DWORD timelen)
{
  m_oBaseData.set_usedbattletime(timelen);
  m_pUser->setDataMark(EUSERDATATYPE_USEDBATTLETIME);
  XLOG << "[玩家-战斗时间], 消耗额外战斗时间, 玩家: " << m_pUser->name << "," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << " 今日已消耗额外时长:" << timelen << " s" << XEND;
}

void UserSceneData::setTutorBattleTime(DWORD timelen)
{
  m_stBlobData.dwTutorbattletime = timelen;
  m_pUser->setDataMark(EUSERDATATYPE_TUTORBATTLETIME);
  XLOG << "[玩家-战斗时间], 获得导师额外战斗时间, 玩家: " << m_pUser->name << "," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << " 今日已消耗额外时长:" << timelen << " s" << XEND;
}

void UserSceneData::setUsedTutorBattleTime(DWORD timelen)
{
  m_stBlobData.dwUsedtutorbattletime = timelen;
  m_pUser->setDataMark(EUSERDATATYPE_USEDTUTORBATTLETIME);
  XLOG << "[玩家-战斗时间], 消耗导师额外战斗时间, 玩家: " << m_pUser->name << "," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << " 今日已消耗额外时长:" << timelen << " s" << XEND;
}

void UserSceneData::setAddictTipsTime(DWORD timelen)
{
  m_oBaseData.set_addicttipstime(timelen);
  m_pUser->setDataMark(EUSERDATATYPE_ADDICTTIPSTIME);
  XLOG << "[玩家-防沉迷提示时间], 防沉迷提示时间更新, 玩家: " << m_pUser->name << "," << m_pUser->accid << "," << m_pUser->id << "," << m_pUser->getProfession() << " 防沉迷提示时间超过时间:" << timelen << " s" << XEND;
}

void UserSceneData::setGagTime(DWORD dwTime, string reason/*=""*/)
{
  // 禁言关闭后, 清除信用度禁言时间戳,避免上线后被再次禁言
  if (dwTime == 0)
  {
    m_stBlobData.stCredit.dwForbidRecoverTime = 0;
  }

  m_oBaseData.set_gagtime(dwTime);
  m_oBaseData.set_gag_reason(reason);
  m_pUser->setDataMark(EUSERDATATYPE_GAGTIME);
}

void UserSceneData::setNologinTime(DWORD dwTime, string reason/*=""*/)
{
  m_oBaseData.set_nologintime(dwTime);
  m_oBaseData.set_lock_reason(reason);
  m_pUser->setDataMark(EUSERDATATYPE_NOLOGINTIME);
}

void UserSceneData::setTeamTimeLen(DWORD teamTimeLen)
{
  m_stBlobData.dwTeamTimeLen = teamTimeLen;
}

void UserSceneData::addHealCount()
{
  //StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_HEAL_COUNT, m_pUser->id, 0, 0, (DWORD)1);

  StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_HEAL_COUNT_SUM, 0, 0, 0, (DWORD)1);
  if (m_pUser->m_oUserStat.checkAndSet(ESTATTYPE_HEAL_COUNT_USER_COUNT))
    m_pUser->m_oUserStat.sendStatLog(ESTATTYPE_HEAL_COUNT_USER_COUNT, 0, 0, 0, (DWORD)1);

  m_stBlobData.dwHealCount += 1;
}

void UserSceneData::addCredit(DWORD value)
{
  DWORD old = m_stBlobData.stCredit.iCurCredit;
  DWORD maxvalue = MiscConfig::getMe().getCreditCFG().dwMaxLimitValue;
  m_stBlobData.stCredit.iCurCredit += value;
  if (m_stBlobData.stCredit.iCurCredit > (int)maxvalue)
    m_stBlobData.stCredit.iCurCredit = maxvalue;

  PlatLogManager::getMe().CreditLog(thisServer,
    m_pUser->id,
    m_pUser->name,
    ECreditType_Add,
    old,
    m_stBlobData.stCredit.iCurCredit);

  XLOG << "[玩家-信用度增加], 玩家:" << m_pUser->name << m_pUser->id << "新加信用度:" << value << "当前信用度:" << m_stBlobData.stCredit.iCurCredit << XEND;
}

void UserSceneData::setCredit(int value)
{
  DWORD old = m_stBlobData.stCredit.iCurCredit;
  m_stBlobData.stCredit.iCurCredit = value;

  PlatLogManager::getMe().CreditLog(thisServer,
    m_pUser->id,
    m_pUser->name,
    ECreditType_Set,
    old,
    m_stBlobData.stCredit.iCurCredit);

  XLOG << "[玩家-信用度重置], 玩家:" << m_pUser->name << m_pUser->id << "当前信用度:" << m_stBlobData.stCredit.iCurCredit << XEND;
}

void UserSceneData::setAuguryRewardTime()
{
  DWORD curTime = xTime::getCurSec();
  m_stAccData.dwAuguryRewardTime = curTime;
  XLOG << "[运营-活动] 占卜, 玩家:" << m_pUser->name << m_pUser->id << "当前领奖次数:" << curTime << XEND;
}

DWORD UserSceneData::getJoyByType(EJoyActivityType etype)
{
  if(m_pUser->getVar().getAccVarValue(EACCVARTYPE_JOY) == 0)
  {
    m_stAccData.m_mapJoy.clear();
    return 0;
  }

  auto s = m_stAccData.m_mapJoy.find(etype);
  if(s != m_stAccData.m_mapJoy.end())
    return s->second;

  return 0;
}

bool UserSceneData::addJoyValue(EJoyActivityType etype, DWORD value)
{
  if(EJoyActivityType_IsValid(etype) == false || value == 0)
    return false;

  if(m_pUser->getVar().getAccVarValue(EACCVARTYPE_JOY) == 0)
    m_stAccData.m_mapJoy.clear();

  DWORD dwOldValue = getTotalJoy();
  if(m_stAccData.addJoyValue(etype, value) == true)
  {
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_JOY, 1);
    m_pUser->setDataMark(EUSERDATATYPE_JOY);
    m_pUser->refreshDataAtonce();
    DWORD dwNowValue = getTotalJoy();
    DWORD dwReward = MiscConfig::getMe().getJoyReward(dwOldValue, dwNowValue);
    if(dwReward != 0)
    {
      m_pUser->getPackage().rollReward(dwReward, EPACKMETHOD_AVAILABLE, false, true);
    }
    MsgManager::sendMsg(m_pUser->id, 3614, MsgParams(dwNowValue-dwOldValue));
    XLOG << "[玩家-欢乐值增加]:" << m_pUser->name << m_pUser->id << "类型: " << etype << "增加前:" << dwOldValue << "当前总值: " << dwNowValue << XEND;
    return true;
  }
  else
    MsgManager::sendMsg(m_pUser->id, 3615);

  return false;
}

DWORD UserSceneData::getTotalJoy()
{
  DWORD dwTotal = 0;
  if(m_pUser->getVar().getAccVarValue(EACCVARTYPE_JOY) == 0)
    return dwTotal;

  for( auto s : m_stAccData.m_mapJoy)
    dwTotal += s.second;

  return dwTotal;
}

void UserSceneData::decCredit(DWORD value)
{
  DWORD old = m_stBlobData.stCredit.iCurCredit;
  m_stBlobData.stCredit.iCurCredit -= value;
  XLOG << "[玩家-信用度减少], 玩家:" << m_pUser->name << m_pUser->id << "减少信用度:" << value << "当前信用度:" << m_stBlobData.stCredit.iCurCredit << XEND;

  const SCreditCFG& rCFG = MiscConfig::getMe().getCreditCFG();
  if (rCFG.bPunish && m_stBlobData.stCredit.iCurCredit < rCFG.iValueForbid)
  {
    DWORD cur = now();
    if (cur > m_stBlobData.stCredit.dwForbidRecoverTime)
    {
      // 设置禁言
      DWORD timeToTalk = cur + rCFG.dwTimeForbid;
      if (timeToTalk > getGagTime())
        setGagTime(timeToTalk);
      m_stBlobData.stCredit.dwForbidRecoverTime = cur + rCFG.dwTimeForbid;
      XLOG << "[玩家-信用度减少], 玩家:" << m_pUser->name << m_pUser->id << "禁言时长:" << rCFG.dwTimeForbid << XEND;
    }
  }

  PlatLogManager::getMe().CreditLog(thisServer,
    m_pUser->id,
    m_pUser->name,
    ECreditType_Dec,
    old,
    m_stBlobData.stCredit.iCurCredit);
}

void UserSceneData::addMonsterCredit(DWORD value)
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_USER_CREDIT) == 0)
  {
    m_stBlobData.stCredit.dwMonsterValue = 0;
    m_pUser->getVar().setVarValue(EVARTYPE_USER_CREDIT, 1);
  }

  DWORD maxmstvalue = MiscConfig::getMe().getCreditCFG().dwMaxMstValue;
  if (m_stBlobData.stCredit.dwMonsterValue >= maxmstvalue)
    return;

  DWORD delta = m_stBlobData.stCredit.dwMonsterValue - maxmstvalue;
  value = value > delta ? delta : value;

  m_stBlobData.stCredit.dwMonsterValue += value;
  addCredit(value);
}


DWORD UserSceneData::getExtraRewardTimes(EExtraRewardType eType)
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_ACTIVITY_REWARD) == 0)
  {
    m_pUser->getVar().setVarValue(EVARTYPE_ACTIVITY_REWARD, 1);
    auto it = m_stBlobData.mapActivityData.find("extra_reward");
    if (it != m_stBlobData.mapActivityData.end())
      it->second.vecParams.clear();
    return 0;
  }

  auto it = m_stBlobData.mapActivityData.find("extra_reward");
  if (it == m_stBlobData.mapActivityData.end())
    return 0;

  if (it->second.vecParams.size() < (DWORD)eType + 1)
    return 0;

  return it->second.vecParams[(DWORD)eType];
}

void UserSceneData::setExtraRewardTimes(EExtraRewardType eType, DWORD times)
{
  SActivityData& sData = m_stBlobData.mapActivityData["extra_reward"];
  DWORD size = sData.vecParams.size();
  DWORD index = (DWORD)eType;
  for (DWORD i = size; i < index + 1; ++i)
    sData.vecParams.push_back(0);
  if (sData.vecParams.size() <= index)
    return;
  sData.vecParams[index] = times;

  XLOG << "[玩家-获得额外奖励], 计数, 玩家" << m_pUser->name << m_pUser->id << "当前已获得, 类型" << eType << "次数" << times << XEND;
}

/*bool UserSceneData::addBlackID(QWORD qwCharID)
{
  auto s = m_stBlobData.setBlackIDs.find(qwCharID);
  if (s != m_stBlobData.setBlackIDs.end())
    return false;
  m_stBlobData.setBlackIDs.insert(qwCharID);

  SceneUser* pUser = SceneUserManager::getMe().getUserByID(qwCharID);
  if (pUser != nullptr)
  {
    pUser->getUserSceneData().decCredit(MiscConfig::getMe().getCreditCFG().iBlack);
    MsgManager::sendMsg(pUser->id, 463);
  }
  else
    XERR << "[玩家-黑名单]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << qwCharID << "到黑名单减少信誉度失败,不在线"<< XEND;

  XLOG << "[玩家-黑名单]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加 :" << qwCharID << "到黑名单" << XEND;
  return true;
}

bool UserSceneData::removeBlackID(QWORD qwCharID)
{
  auto s = m_stBlobData.setBlackIDs.find(qwCharID);
  if (s == m_stBlobData.setBlackIDs.end())
    return false;

  XLOG << "[玩家-黑名单]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "从黑名单中移除 :" << qwCharID << XEND;
  m_stBlobData.setBlackIDs.erase(s);
  return true;
}

bool UserSceneData::checkBlack(QWORD charid) const
{
  if (m_pUser->isMyTeamMember(charid))
    return false;
  return m_stBlobData.setBlackIDs.find(charid) != m_stBlobData.setBlackIDs.end();
}*/

void UserSceneData::addSeeNpcs(DWORD id)
{
  m_stBlobData.setSeeNpcs.insert(id);
  XLOG << "[玩家-SeeNpc], 玩家:" << m_pUser->name << m_pUser->id << "设置对npc" << id << "可见" << XEND;
}

void UserSceneData::delSeeNpcs(DWORD id)
{
  m_stBlobData.setSeeNpcs.erase(id);
  XLOG << "[玩家-SeeNpc], 玩家:" << m_pUser->name << m_pUser->id << "设置对npc" << id << "取消可见" << XEND;
}

void UserSceneData::addHideNpcs(DWORD id)
{
  m_stBlobData.setHideNpcs.insert(id);
  XLOG << "[玩家-SeeNpc], 玩家:" << m_pUser->name << m_pUser->id << "设置对npc" << id << "不可见" << XEND;
}

void UserSceneData::delHideNpcs(DWORD id)
{
  m_stBlobData.setHideNpcs.erase(id);
  XLOG << "[玩家-SeeNpc], 玩家:" << m_pUser->name << m_pUser->id << "设置对npc" << id << "取消不可见" << XEND;
}

void UserSceneData::resetGuildRaidData()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_GUILD_RAID) == 0)
  {
    m_stBlobData.mapGuildRaidData.clear();
    m_pUser->getVar().setVarValue(EVARTYPE_GUILD_RAID, 1);

    for (auto &itCfg : MiscConfig::getMe().getGuildRaidCFG().mapNpcID2GuildRaid) {
      newGuildRaidData(itCfg.second);
    }
    XLOG << "[公会副本] 副本数据重置, 玩家:" << m_pUser->name << "id:" << m_pUser->id << XEND;
  }
  if (GuildRaidConfig::getMe().getResetVersion() != 0)
  {
    DWORD raidversion = GuildRaidConfig::getMe().getResetVersion();
    if (m_stBlobData.m_dwGuildRaidVersion < raidversion)
    {
      m_stBlobData.mapGuildRaidData.clear();
      for (auto &itCfg : MiscConfig::getMe().getGuildRaidCFG().mapNpcID2GuildRaid) {
        newGuildRaidData(itCfg.second);
      }
      m_stBlobData.m_dwGuildRaidVersion = raidversion;
      XLOG << "[公会副本], 非时间重置, 玩家:" << m_pUser->name << m_pUser->id << "当前版本号:" << raidversion << XEND;
    }
  }
}

void UserSceneData::newGuildRaidData(const SGuildRaid& rCfg)
{
  SGuildRaidData& data = m_stBlobData.mapGuildRaidData[rCfg.dwNpcID];
  data.dwNpcId = rCfg.dwNpcID;
  if (rCfg.bSpecial || m_pUser->getGuild().lv() < rCfg.dwGuildLevel)
    data.eState = EGUILDGATESTATE_LOCK;
  else
    data.eState = EGUILDGATESTATE_CLOSE;
}

void UserSceneData::clearGuildRaidData()
{
  m_pUser->getVar().setVarValue(EVARTYPE_GUILD_RAID, 0);
  m_stBlobData.mapGuildRaidData.clear();
  XLOG << "[公会副本] 副本数据清空, 玩家:" << m_pUser->name << "id:" << m_pUser->id << XEND;
}

bool UserSceneData::addQuestNtf(DWORD dwQuestID)
{
  if (isQuestNtf(dwQuestID) == true)
    return false;
  m_oBaseData.add_questmapntf(dwQuestID);
  return true;
}

bool UserSceneData::isQuestNtf(DWORD dwQuestID)
{
  for (int i = 0; i < m_oBaseData.questmapntf_size(); ++i)
  {
    if (m_oBaseData.questmapntf(i) == dwQuestID)
      return true;
  }
  return false;
}

TMapNpcID2GuildRaidData& UserSceneData::getGuildRaids()
{
  resetGuildRaidData();
  return m_stBlobData.mapGuildRaidData;
}

bool UserSceneData::canEnterGuildRaid(DWORD gatenpcid)
{
  resetGuildRaidData();
  auto it = m_stBlobData.mapGuildRaidData.find(gatenpcid);
  if (it == m_stBlobData.mapGuildRaidData.end())
    return false;
  return it->second.eState == EGUILDGATESTATE_OPEN;
}

bool UserSceneData::hasGotGRaidReward(DWORD gatenpcid, DWORD mapindex)
{
  resetGuildRaidData();
  auto it = m_stBlobData.mapGuildRaidData.find(gatenpcid);
  if (it == m_stBlobData.mapGuildRaidData.end())
    return false;
  auto s = it->second.setKilledBoss.find(mapindex);
  if (s == it->second.setKilledBoss.end())
    return false;

  return true;
}

void UserSceneData::markGotGRaidReward(DWORD gatenpcid, DWORD mapindex)
{
  auto it = m_stBlobData.mapGuildRaidData.find(gatenpcid);
  if (it == m_stBlobData.mapGuildRaidData.end())
  {
    SGuildRaidData& gdata = m_stBlobData.mapGuildRaidData[gatenpcid];
    gdata.dwNpcId = gatenpcid;
    it = m_stBlobData.mapGuildRaidData.find(gatenpcid);
  }
  if (it == m_stBlobData.mapGuildRaidData.end())
    return;

  it->second.setKilledBoss.insert(mapindex);
}

DWORD UserSceneData::getGRaidKilledBossCnt(DWORD gatenpcid)
{
  auto it = m_stBlobData.mapGuildRaidData.find(gatenpcid);
  if (it == m_stBlobData.mapGuildRaidData.end())
    return 0;
  return it->second.setKilledBoss.size();
}

void UserSceneData::setFashionHide(DWORD data)
{
  DWORD old = m_stOptionData.m_dwFashionHide;
  m_stOptionData.m_dwFashionHide = data & ~(~DWORD(0) << EFASHIONHIDETYPE_MAX);
  m_pUser->setDataMark(EUSERDATATYPE_FASHIONHIDE);

  DWORD v = old ^ m_stOptionData.m_dwFashionHide;
  for (DWORD i = 0; i < EFASHIONHIDETYPE_MAX; ++i, v >>= 1) {
    if ((v & DWORD(1)) == 0)
      continue;

    switch (static_cast<EFashionHideType>(i)) {
    case EFASHIONHIDETYPE_HEAD:
      m_pUser->setDataMark(EUSERDATATYPE_HEAD); break;
    case EFASHIONHIDETYPE_BACK:
      m_pUser->setDataMark(EUSERDATATYPE_BACK); break;
    case EFASHIONHIDETYPE_FACE:
      m_pUser->setDataMark(EUSERDATATYPE_FACE); break;
    case EFASHIONHIDETYPE_TAIL:
      m_pUser->setDataMark(EUSERDATATYPE_TAIL); break;
    case EFASHIONHIDETYPE_MOUTH:
      m_pUser->setDataMark(EUSERDATATYPE_MOUTH); break;
    case EFASHIONHIDETYPE_BODY:
      m_pUser->setDataMark(EUSERDATATYPE_BODY); break;
    default:
      break;
    }
  }
  m_pUser->refreshDataAtonce();
}

bool UserSceneData::isFashionHide(EFashionHideType type) const
{
  if (type >= EFASHIONHIDETYPE_MAX || !EFashionHideType_IsValid(type))
    return false;
  return (m_stOptionData.m_dwFashionHide & (DWORD(1) << type)) != 0;
}

void UserSceneData::setPvpCoin(DWORD coinnum)
{
  if (m_stBlobData.dwPvpCoin == coinnum)
    return;
  m_stBlobData.dwPvpCoin = coinnum;
  m_pUser->setDataMark(EUSERDATATYPE_PVPCOIN);
  m_pUser->refreshDataAtonce();
}

/*void UserSceneData::setCon(DWORD value)
{
  m_stBlobData.dwCon = value;
  m_pUser->setDataMark(EUSERDATATYPE_CONTRIBUTE);
  m_pUser->refreshDataAtonce();
}*/

void UserSceneData::setOption(EOptionType type, DWORD flag)
{
  if (!EOptionType_IsValid(type))
    return;
  if (flag)
    m_stOptionData.m_btSet.set(type);
  else
    m_stOptionData.m_btSet.reset(type);
  
  switch (type)
  {
  case EOPTIONTYPE_USE_SLIM:
  {
    m_pUser->getSceneFood().refreshSlim(flag);
    break;
  }
  default:
    break;
  }
  m_pUser->setDataMark(EUSERDATATYPE_OPTION);
  m_pUser->refreshDataAtonce();
  XDBG << "[选项-设置] charid " << m_pUser->id << "type" << type << flag << XEND;
}

DWORD UserSceneData::getOption(EOptionType type)
{
  if (!EOptionType_IsValid(type))
    return 0;
  return m_stOptionData.m_btSet.test(type);
}

QWORD UserSceneData::getOptionSet()
{
  return m_stOptionData.m_btSet.to_ulong();
}

void UserSceneData::setLotteryCoin(DWORD newNum)
{
  if (m_stBlobData.dwLotteryCoin == newNum)
    return;
  m_stBlobData.dwLotteryCoin = newNum;
  m_pUser->setDataMark(EUSERDATATYPE_LOTTERY);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::setGuildHonor(DWORD num)
{
  if (m_stBlobData.dwGuildHonor == num)
    return;
  XLOG << "[玩家-荣誉], 荣誉值更新, 更新前:" << m_stBlobData.dwGuildHonor << "更新后:" << num << XEND;
  m_stBlobData.dwGuildHonor = num;
  m_pUser->setDataMark(EUSERDATATYPE_GUILDHONOR);
  m_pUser->refreshDataAtonce();
}

void UserSceneData::queryPhotoMd5List()
{
  refreshPhotoMd5();

  QueryMd5ListPhotoCmd cmd;
  for (auto &l : m_stAccData.listPhotoMd5)
  {
    cmd.add_item()->CopyFrom(l);
    if (cmd.ByteSize() > TRANS_BUFSIZE)
    {
      PROTOBUF(cmd, send, len);
      m_pUser->sendCmdToMe(send, len);
      XLOG << "[玩家-md5同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据超过" << TRANS_BUFSIZE << "先同步" << cmd.ShortDebugString() << XEND;
      cmd.Clear();
    }
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  XLOG << "[玩家-md5同步]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "同步列表" << cmd.ShortDebugString() << XEND;
}

bool UserSceneData::addPhotoMd5(const PhotoMd5& md5)
{
  refreshPhotoMd5();

  if (md5.source() == ESOURCE_PHOTO_SCENERY)
  {
    const Table<SceneryBase>* pSceneyBase = TableManager::getMe().getSceneryCFGList();
    if (pSceneyBase == nullptr)
    {
      XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,未找到景点配置列表" << XEND;
      return false;
    }
    DWORD dwTotal = 0;
    for (auto &l : m_stAccData.listPhotoMd5)
    {
      if (l.source() == ESOURCE_PHOTO_SCENERY)
        ++dwTotal;
    }
    if(dwTotal >= pSceneyBase->xEntryID::ets_.size())
    {
      XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,数量超过景点配置列表" << XEND;
      return false;
    }
  }
  else if (md5.source() == ESOURCE_PHOTO_SELF)
  {
    PhotoItem* pItem = m_pUser->getPhoto().get(md5.sourceid());
    if (pItem == nullptr)
    {
      XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,未找到个人照片" << XEND;
      return false;
    }
  }
  else if (md5.source() == ESOURCE_PHOTO_GUILD)
  {
    const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
    DWORD dwTotal = 0;
    for (auto &l : m_stAccData.listPhotoMd5)
    {
      if (l.source() == ESOURCE_PHOTO_GUILD)
        ++dwTotal;
    }
    if(rCFG.dwIconCount <= dwTotal)
    {
      XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name
        << "添加" << md5.ShortDebugString() << "失败,超过公会图标上限" << rCFG.dwIconCount  << XEND;
      return false;
    }
  }
  else
  {
    XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,未知source :" << md5.source() << XEND;
    return false;
  }

  if (md5.md5().empty() == true)
  {
    XERR << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,md5为空" << XEND;
    return false;
  }

  PhotoMd5 oCopy;
  oCopy.CopyFrom(md5);
  oCopy.set_time(xTime::getCurSec());

  auto l = find_if(m_stAccData.listPhotoMd5.begin(), m_stAccData.listPhotoMd5.end(), [&](const PhotoMd5& r) -> bool{
    return r.source() == oCopy.source() && r.sourceid() == oCopy.sourceid();
  });
  if (l != m_stAccData.listPhotoMd5.end())
    l->CopyFrom(oCopy);
  else
    m_stAccData.listPhotoMd5.push_back(oCopy);

  XLOG << "[玩家-md5添加]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << oCopy.ShortDebugString() << "成功" << XEND;
  return true;
}

bool UserSceneData::removePhotoMd5(const PhotoMd5& md5)
{
  refreshPhotoMd5();

  auto l = find_if(m_stAccData.listPhotoMd5.begin(), m_stAccData.listPhotoMd5.end(), [&](const PhotoMd5& r) -> bool{
    return r.source() == md5.source() && r.sourceid() && md5.sourceid();
  });
  if (l == m_stAccData.listPhotoMd5.end())
  {
    XERR << "[玩家-md5删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "失败,未找到" << XEND;
    return false;
  }

  m_stAccData.listPhotoMd5.erase(l);
  XLOG << "[玩家-md5删除]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "添加" << md5.ShortDebugString() << "成功" << XEND;
  return true;
}

void UserSceneData::updateMaxBaseLv(DWORD oldLv)
{
  DWORD cur = now();
  if (xTime::getDayStart(cur, MAXBASELV_RESETTIME_OFFSET) < xTime::getDayStart(m_stAccData.dwMaxBaseLvResetTime, MAXBASELV_RESETTIME_OFFSET) + MAXBASELV_RESETTIME)
    return;
  m_stAccData.dwCurMaxBaseLv = std::max(oldLv, m_stAccData.dwCurMaxBaseLv);
  m_stAccData.dwMaxBaseLv = m_stAccData.dwCurMaxBaseLv;
  m_stAccData.dwMaxBaseLvResetTime = cur;
}

DWORD UserSceneData::getMaxBaseLv()
{
  updateMaxBaseLv(m_pUser->getLevel());
  return m_stAccData.dwMaxBaseLv;
}

void UserSceneData::setSkillOpt(ESkillOption opt, DWORD value)
{
  m_stOptionData.m_mapSkillOpt[opt] = value;
  XLOG << "[技能-设置], 玩家:" << m_pUser->name << m_pUser->id << "类型:" << opt << "value:" << value << XEND;

  SkillOptionSkillCmd cmd;
  SkillOption* p = cmd.add_all_opts();
  if (p == nullptr)
    return;
  p->set_opt(opt);
  p->set_value(value);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserSceneData::notifyActivityEventToClient(EAERewardMode mode/* = EAEREWARDMODE_MIN*/)
{
  resetAEReward(false);
  ActivityEventUserDataNtf ntf;
  m_stBlobData.stActivityEvent.toActivityEventUserDataNtf(&ntf, mode);
  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void UserSceneData::resetAEReward(bool ntf/* = true*/)
{
  bool update = false;
  if (m_pUser->getVar().getVarValue(EVARTYPE_ACTIVITY_EVENT_REWARD) == 0)
  {
    m_stBlobData.stActivityEvent.resetChar();
    m_pUser->getVar().setVarValue(EVARTYPE_ACTIVITY_EVENT_REWARD, 1);
    update = true;
  }
  if (m_pUser->getVar().getAccVarValue(EACCVARTYPE_ACTIVITY_EVENT_REWARD) == 0)
  {
    m_stBlobData.stActivityEvent.resetAcc();
    m_pUser->getVar().setAccVarValue(EACCVARTYPE_ACTIVITY_EVENT_REWARD, 1);
    update = true;
  }
  if (update && ntf)
    notifyActivityEventToClient();
}

DWORD UserSceneData::getAERewardDayCount(EAERewardMode mode)
{
  resetAEReward();
  auto it = m_stBlobData.stActivityEvent.mapReward.find(mode);
  if (it == m_stBlobData.stActivityEvent.mapReward.end())
    return 0;
  return it->second.dwDayCount;
}

void UserSceneData::addAERewardDayCount(EAERewardMode mode, DWORD cnt)
{
  resetAEReward();
  if (m_stBlobData.stActivityEvent.mapReward.find(mode) == m_stBlobData.stActivityEvent.mapReward.end())
  {
    m_stBlobData.stActivityEvent.mapReward[mode].eMode = mode;
    m_stBlobData.stActivityEvent.mapReward[mode].dwDayCount = 0;
  }
  m_stBlobData.stActivityEvent.mapReward[mode].dwDayCount += cnt;
  notifyActivityEventToClient(mode);
}

QWORD UserSceneData::getAERewardAccLimitCharID(EAERewardMode mode)
{
  resetAEReward();
  auto it = m_stBlobData.stActivityEvent.mapReward.find(mode);
  if (it == m_stBlobData.stActivityEvent.mapReward.end())
    return 0;
  return it->second.qwAccLimitCharid;
}

void UserSceneData::setAERewardAccLimitCharID(EAERewardMode mode, QWORD charid)
{
  resetAEReward();
  if (m_stBlobData.stActivityEvent.mapReward.find(mode) == m_stBlobData.stActivityEvent.mapReward.end())
    m_stBlobData.stActivityEvent.mapReward[mode].eMode = mode;
  if (m_stBlobData.stActivityEvent.mapReward[mode].qwAccLimitCharid == charid)
    return;
  m_stBlobData.stActivityEvent.mapReward[mode].qwAccLimitCharid = charid;
  notifyActivityEventToClient(mode);
}

DWORD UserSceneData::getAERewardMulDayCount(EAERewardMode mode)
{
  resetAEReward();
  auto it = m_stBlobData.stActivityEvent.mapReward.find(mode);
  if (it == m_stBlobData.stActivityEvent.mapReward.end())
    return 0;
  return it->second.dwMulDayCount;
}

void UserSceneData::addAERewardMulDayCount(EAERewardMode mode, DWORD cnt)
{
  resetAEReward();
  if (m_stBlobData.stActivityEvent.mapReward.find(mode) == m_stBlobData.stActivityEvent.mapReward.end())
  {
    m_stBlobData.stActivityEvent.mapReward[mode].eMode = mode;
    m_stBlobData.stActivityEvent.mapReward[mode].dwMulDayCount = 0;
  }
  m_stBlobData.stActivityEvent.mapReward[mode].dwMulDayCount += cnt;
  notifyActivityEventToClient(mode);
}

QWORD UserSceneData::getAERewardMulAccLimitCharID(EAERewardMode mode)
{
  resetAEReward();
  auto it = m_stBlobData.stActivityEvent.mapReward.find(mode);
  if (it == m_stBlobData.stActivityEvent.mapReward.end())
    return 0;
  return it->second.qwMulAccLimitCharid;
}

void UserSceneData::setAERewardMulAccLimitCharID(EAERewardMode mode, QWORD charid)
{
  resetAEReward();
  if (m_stBlobData.stActivityEvent.mapReward.find(mode) == m_stBlobData.stActivityEvent.mapReward.end())
    m_stBlobData.stActivityEvent.mapReward[mode].eMode = mode;
  if (m_stBlobData.stActivityEvent.mapReward[mode].qwMulAccLimitCharid == charid)
    return;
  m_stBlobData.stActivityEvent.mapReward[mode].qwMulAccLimitCharid = charid;
  notifyActivityEventToClient(mode);
}

DWORD UserSceneData::getSkillOptValue(ESkillOption opt)
{
  auto it = m_stOptionData.m_mapSkillOpt.find(opt);
  if (it == m_stOptionData.m_mapSkillOpt.end())
    return 0;
  return it->second;
}

void UserSceneData::setDivorceRollerCoaster(bool b)
{
  m_stBlobData.bDivorceRollerCoaster = b;
  m_pUser->setDataMark(EUSERDATATYPE_DIVORCE_ROLLERCOASTER);
  m_pUser->refreshDataAtonce();
  XDBG << "[婚礼-离婚过山车] 设置" << m_pUser->id << m_pUser->name << m_stBlobData.bDivorceRollerCoaster << XEND;
}

DWORD UserSceneData::getActivityEventCnt(EActivityEventType eventType, QWORD id, EUserType userType)
{
  TMapType2ActivityEvent* pMapEventData = nullptr;
  if (userType == EUserType_Char)
    pMapEventData = &(m_stBlobData.stActivityEvent.mapCharEventCnt);
  else
    pMapEventData = &(m_stBlobData.stActivityEvent.mapAccEventCnt);
  if (!pMapEventData)
    return 0;
  auto it = pMapEventData->find(eventType);
  if (it == pMapEventData->end())
    return 0;
  auto subIt = it->second.find(id);
  if (subIt == it->second.end())
    return 0;
  return subIt->second.count();
}

void UserSceneData::setActivityEventCnt(EActivityEventType eventType, QWORD id, EUserType userType, DWORD count)
{
  TMapType2ActivityEvent* pMapEventData = nullptr;
  if (userType == EUserType_Char)
    pMapEventData = &(m_stBlobData.stActivityEvent.mapCharEventCnt);
  else
    pMapEventData = &(m_stBlobData.stActivityEvent.mapAccEventCnt);
  if (!pMapEventData)
    return ;
  
  TMapId2ActivityEvent& rId2Event = (*pMapEventData)[eventType];
  auto it = rId2Event.find(id);
  if (it == rId2Event.end())
  {
    ActivityEventCnt oCnt;
    oCnt.set_type(eventType);
    oCnt.set_id(id);
    oCnt.set_count(count);
    rId2Event[id] = oCnt;
  }
  else
  {
    it->second.set_count(count);
  }
}

bool UserSceneData::haveAction(DWORD id)
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0)
    return true;

  return m_stBlobData.setActions.find(id) != m_stBlobData.setActions.end(); 
}

bool UserSceneData::haveExpression(DWORD id)
{
  if(m_pUser->getDressUp().getDressUpStatus() != 0)
    return true;

  return m_stBlobData.setExpressions.find(id) != m_stBlobData.setExpressions.end();
}

void UserSceneData::notifyDeadBoss()
{
  if (m_oBoss.open_ntf() == true || m_pUser == nullptr)
    return;

  const DeadBossInfo& rInfo = GlobalManager::getMe().getGlobalBoss();
  if (rInfo.charid() == 0)
    return;

  const SBossMiscCFG& rCFG = MiscConfig::getMe().getBossCFG();

  // play dialog
  UserActionNtf cmd;
  cmd.set_charid(m_pUser->id);
  cmd.set_value(rCFG.dwDeadBossOpenNtf);
  cmd.set_type(EUSERACTIONTYPE_DIALOG);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);

  // red tip
  m_pUser->getTip().addRedTip(EREDSYS_DEAD_BOSS);

  m_oBoss.set_open_ntf(true);
  XLOG << "[世界boss-通知]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "收到世界boss开启成功,信息" << rInfo.ShortDebugString() << XEND;
}


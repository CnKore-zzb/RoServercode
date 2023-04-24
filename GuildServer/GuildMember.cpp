#include "GuildMember.h"
#include "Guild.h"
#include "GuildServer.h"
#include "GGuild.h"
#include "MiscConfig.h"
#include "GuildManager.h"

// guild member
GMember::GMember(Guild* guild, const SocialUser& rUser, EGuildJob eJob) : m_oGCharData(thisServer->getRegionID(), rUser.charid()), m_pGuild(guild)
{
  m_oGCharData.setAccID(rUser.accid());
  m_oGCharData.setZoneID(rUser.zoneid());

  m_eJob = eJob;
  m_dwEnterTime = xTime::getCurSec();
}

GMember::~GMember()
{

}

void GMember::fromData(const GuildMember& rData)
{
  if (m_pGuild == nullptr)
  {
    XERR << "[公会成员-加载]" << "加载 :" << rData.charid() << "失败,未初始化公会" << XEND;
    return;
  }

  m_oGCharData.setCharID(rData.charid());

  xTime frameTime;
  if (!m_oGCharData.getByGuild())
  {
    XERR << "[公会成员-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << getCharID() << "redis失败,未获取全局玩家" << XEND;
    return;
  }
  XDBG << "[公会成员-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << getCharID() << "redis成功,耗时" << frameTime.uElapse() << "微秒" << XEND;

  BlobVar oVar;
  if (oVar.ParseFromString(rData.var()) == true)
  {
    if (m_oVar.load(oVar) == false)
      XERR << "[公会成员-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << rData.charid() << "失败,var加载失败" << XEND;
  }
  else
  {
    XERR << "[公会成员-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << rData.charid() << "失败,var序列化失败" << XEND;
  }

  m_dwWeekCon = rData.weekcontribution();
  m_dwCon = rData.contribution();
  m_dwTotalCon = rData.totalcontribution();
  m_dwEnterTime = rData.entertime();
  m_dwGiftPoint = rData.giftpoint();
  m_dwWeekAsset = rData.weekasset();
  m_dwAsset = rData.asset();
  m_dwExchangeZone = rData.exchangezone();
  m_dwWeekBCoin = rData.weekbcoin();
  m_dwTotalBCoin = rData.totalcoin();

  m_eJob = rData.job();

  m_bLevelupEffect = rData.levelupeffect();
  m_bExchangeZone = rData.exchangezone();

  m_vecPrays.clear();
  const BlobGuildPray& rPray = rData.pray();
  for (int i = 0; i < rPray.prays_size(); ++i)
    m_vecPrays.push_back(rPray.prays(i));

  m_vecDonateItems.clear();
  const BlobDonate& rDonate = rData.donate();
  for (int i = 0; i < rDonate.items_size(); ++i)
  {
    DonateItem oItem;
    oItem = rDonate.items(i);
    DWORD contribute = getAssetByID(rDonate.items(i).configid());
    DWORD medal = getMedalByID(rDonate.items(i).configid());
    oItem.set_contribute(contribute);
    oItem.set_medal(medal);
    m_vecDonateItems.push_back(oItem);
  }
  m_dwDonateTime1 = rDonate.donatetime1();
  m_dwDonateTime2 = rDonate.donatetime2();
  m_dwDonateTime3 = rDonate.donatetime3();
  m_dwDonateTime4 = rDonate.donatetime4();

  m_mapGuildBuilding.clear();
  const BlobGuildBuilding& rBuilding = rData.building();
  for (int i = 0; i < rBuilding.buildings_size(); ++i)
    m_mapGuildBuilding[rBuilding.buildings(i).type()].CopyFrom(rBuilding.buildings(i));

  m_qwChallenge = rData.challenge();
  m_dwLastExitTime = rData.lastexittime();
  m_qwRedTip = rData.redtip();
  m_bBuildingEffect = rData.buildingeffect();
  m_bRealtimeVoice = rData.realtimevoice();
}

void GMember::toData(GuildMember* pData, bool bClient /*= false*/)
{
  if (pData == nullptr || m_pGuild == nullptr)
  {
    XERR << "[公会成员-保存]" << "保存 :" << getCharID() << "失败,数据或公会未初始化" << XEND;
    return;
  }

  pData->set_charid(m_oGCharData.getCharID());
  pData->set_baselevel(m_oGCharData.getBaseLevel());
  pData->set_portrait(m_oGCharData.getPortrait());
  pData->set_frame(m_oGCharData.getFrame());
  pData->set_offlinetime(m_oGCharData.getOfflineTime());
  pData->set_hair(m_oGCharData.getHair());
  pData->set_haircolor(m_oGCharData.getHairColor());
  pData->set_body(m_oGCharData.getBody());
  pData->set_zoneid(m_oGCharData.getZoneID());
  pData->set_gender(m_oGCharData.getGender());
  pData->set_profession(m_oGCharData.getProfession());
  pData->set_name(m_oGCharData.getName());
  pData->set_head(m_oGCharData.getHead());
  pData->set_face(m_oGCharData.getFace());
  pData->set_mouth(m_oGCharData.getMouth());
  pData->set_eye(m_oGCharData.getEye());

  pData->set_weekcontribution(getWeekContribution());
  pData->set_contribution(m_dwCon);
  pData->set_totalcontribution(m_dwTotalCon);
  pData->set_entertime(m_dwEnterTime);
  pData->set_giftpoint(m_dwGiftPoint);
  pData->set_weekasset(m_dwWeekAsset);
  pData->set_job(m_eJob);
  pData->set_levelupeffect(m_bLevelupEffect);
  pData->set_exchangezone(m_bExchangeZone);
  pData->set_challenge(m_qwChallenge);
  pData->set_lastexittime(m_dwLastExitTime);
  pData->set_redtip(m_qwRedTip);
  pData->set_buildingeffect(m_bBuildingEffect);
  pData->set_weekbcoin(getWeekBCoin());
  pData->set_totalcoin(m_dwTotalBCoin);
  pData->set_realtimevoice(m_bRealtimeVoice);

  XDBG << "[公会成员-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << "weekbcoin :" << m_dwWeekBCoin << "totalbcoin :" << m_dwTotalBCoin << XEND;

  if (!bClient)
  {
    BlobVar oVar;
    if (m_oVar.save(&oVar) == true)
    {
      if (oVar.SerializeToString(pData->mutable_var()) == false)
        XERR << "[公会成员-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存 :" << getCharID() << "失败,var序列化失败" << XEND;
    }
    else
    {
      XERR << "[公会成员-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "保存 :" << getCharID() << "失败,var保存失败" << XEND;
    }

    pData->clear_pray();
    for (auto &v : m_vecPrays)
      pData->mutable_pray()->add_prays()->CopyFrom(v);

    pData->clear_donate();
    for (auto &v : m_vecDonateItems)
      pData->mutable_donate()->add_items()->CopyFrom(v);

    pData->mutable_donate()->set_donatetime1(m_dwDonateTime1);
    pData->mutable_donate()->set_donatetime2(m_dwDonateTime2);
    pData->mutable_donate()->set_donatetime3(m_dwDonateTime3);
    pData->mutable_donate()->set_donatetime4(m_dwDonateTime4);

    for (auto& v : m_mapGuildBuilding)
      pData->mutable_building()->add_buildings()->CopyFrom(v.second);
  }
  else
  {
    pData->set_zoneid(getClientZoneID(getZoneID()));
    if (isOnline())
      pData->set_offlinetime(0);
    else
      pData->set_offlinetime(m_oGCharData.getOfflineTime());
  }
}

void GMember::fromData(const GuildApply& rData)
{
  m_oGCharData.setCharID(rData.charid());

  xTime frameTime;
  if (!m_oGCharData.get())
  {
    XERR << "[公会申请-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << getCharID() << "redis失败,未获取全局玩家" << XEND;
    return;
  }
  XDBG << "[公会申请-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载 :" << getCharID() << "redis成功, 耗时" << frameTime.uElapse() << "微秒" << XEND;
}

void GMember::toData(GuildApply* pData)
{
  if (pData == nullptr || m_pGuild == nullptr)
    return;

  pData->set_charid(m_oGCharData.getCharID());
  pData->set_baselevel(m_oGCharData.getBaseLevel());
  pData->set_portrait(m_oGCharData.getPortrait());
  pData->set_frame(m_oGCharData.getFrame());
  pData->set_hair(m_oGCharData.getHair());
  pData->set_haircolor(m_oGCharData.getHairColor());
  pData->set_body(m_oGCharData.getBody());
  pData->set_zoneid(m_oGCharData.getZoneID());
  pData->set_gender(m_oGCharData.getGender());
  pData->set_profession(m_oGCharData.getProfession());
  pData->set_name(m_oGCharData.getName());
  pData->set_head(m_oGCharData.getHead());
  pData->set_face(m_oGCharData.getFace());
  pData->set_mouth(m_oGCharData.getMouth());
  pData->set_eye(m_oGCharData.getEye());

  pData->set_entertime(m_dwEnterTime);
}

bool GMember::fromPrayData(const string& str)
{
  BlobGuildPray oBlob;
  if (oBlob.ParseFromString(str) == false)
    return false;

  m_vecPrays.clear();
  for (int i = 0; i < oBlob.prays_size(); ++i)
    setPrayLv(oBlob.prays(i).pray(), oBlob.prays(i).lv());
  return true;
}

bool GMember::toPrayData(string& str)
{
  BlobGuildPray oBlob;

  oBlob.clear_prays();
  for (auto &v : m_vecPrays)
    oBlob.add_prays()->CopyFrom(v);

  return oBlob.SerializeToString(&str);
}

bool GMember::fromDonateData(const string& str)
{
  BlobDonate oDonate;
  if (oDonate.ParseFromString(str) == false)
    return false;

  m_dwDonateTime1 = oDonate.donatetime1();
  m_dwDonateTime2 = oDonate.donatetime2();
  m_dwDonateTime3 = oDonate.donatetime3();
  m_dwDonateTime4 = oDonate.donatetime4();

  m_vecDonateItems.clear();
  for (int i = 0; i < oDonate.items_size(); ++i)
  {
    const DonateItem& rItem = oDonate.items(i);
    addDonateItem(rItem.configid(), rItem.time(), rItem.itemid(), rItem.itemcount(), rItem.count());
  }
  return true;
}

bool GMember::toDonateData(string& str)
{
  BlobDonate oDonate;
  for (auto &v : m_vecDonateItems)
    oDonate.add_items()->CopyFrom(v);

  oDonate.set_donatetime1(m_dwDonateTime1);
  oDonate.set_donatetime2(m_dwDonateTime2);
  oDonate.set_donatetime3(m_dwDonateTime3);
  oDonate.set_donatetime4(m_dwDonateTime4);

  return oDonate.SerializeToString(&str);
}

bool GMember::fromVarData(const string& str)
{
  BlobVar var;
  if (var.ParseFromString(str) == false)
    return false;
  return m_oVar.load(var);
}

bool GMember::toVarData(string& str)
{
  BlobVar var;
  if (m_oVar.save(&var) == false)
    return false;
  return var.SerializeToString(&str);
}

bool GMember::fromBuildingData(const string& str)
{
  BlobGuildBuilding data;
  if (data.ParseFromString(str) == false)
    return false;
  m_mapGuildBuilding.clear();
  for (int i = 0; i < data.buildings_size(); ++i)
    m_mapGuildBuilding[data.buildings(i).type()].CopyFrom(data.buildings(i));
  return true;
}

bool GMember::toBuildingData(string& str)
{
  BlobGuildBuilding data;
  for (auto& v : m_mapGuildBuilding)
    data.add_buildings()->CopyFrom(v.second);
  return data.SerializeToString(&str);
}

void GMember::toScenePrayData(GuildUserInfoSyncGuildCmd& cmd)
{
  cmd.set_charid(getCharID());
  for (auto &v : m_vecPrays)
  {
    GuildMemberPray* pPray = cmd.mutable_info()->add_prays();
    if (pPray != nullptr)
      pPray->CopyFrom(v);
  }

  cmd.mutable_info()->set_contribute(getContribution());
  cmd.mutable_info()->set_giftpoint(getGiftPoint());
}

void GMember::toData(GuildSMember* pMember)
{
  if (pMember == nullptr)
    return;

  pMember->set_accid(getAccid());
  pMember->set_charid(getCharID());
  pMember->set_onlinetime(getOnlineTime());
  pMember->set_offlinetime(getOfflineTime());
  pMember->set_entertime(getEnterTime());
  pMember->set_job(getJob());
  pMember->set_name(getName());
  pMember->set_auth(m_pGuild->getMisc().getAuth(getJob()));
}

void GMember::updateData(const UserInfo& rInfo, bool bInit /*= false*/)
{
  //m_oGCharData.set_id(rInfo.user().charid());
  //m_oGCharData.setName(rInfo.user().name());

  const SocialUser& rUser = rInfo.user();
  for (int i = 0; i < rInfo.datas_size(); ++i)
  {
    const UserData& rData = rInfo.datas(i);
    switch (rData.type())
    {
      case EUSERDATATYPE_ROLELEVEL:
        m_oGCharData.setBaseLevel(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_BASELEVEL);
        break;
      case EUSERDATATYPE_PORTRAIT:
        m_oGCharData.setPortrait(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_PORTRAIT);
        break;
      case EUSERDATATYPE_BODY:
        m_oGCharData.setBody(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_BODY);
        break;
      case EUSERDATATYPE_HAIR:
        m_oGCharData.setHair(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_HAIR);
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        m_oGCharData.setHairColor(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_HAIRCOLOR);
        break;
      case EUSERDATATYPE_FRAME:
        m_oGCharData.setFrame(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_FRAME);
        break;
      case EUSERDATATYPE_PROFESSION:
        m_oGCharData.setProfession(static_cast<EProfession>(rData.value()));
        m_bitset.set(EGUILDMEMBERDATA_PROFESSION);
        break;
      case EUSERDATATYPE_SEX:
        m_oGCharData.setGender(static_cast<EGender>(rData.value()));
        m_bitset.set(EGUILDMEMBERDATA_GENDER);
        break;
      case EUSERDATATYPE_NAME:
        if(m_pGuild && !bInit && m_oGCharData.getName() != rData.data())
          m_pGuild->broadcastMsg(ESYSTEMMSG_ID_GUILD_USER_RENAME, MsgParams(m_oGCharData.getName(), rData.data()));

        m_oGCharData.setName(rData.data());
        m_bitset.set(EGUILDMEMBERDATA_NAME);
        break;
      case EUSERDATATYPE_HEAD:
        m_oGCharData.setHead(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_HEAD);
        break;
      case EUSERDATATYPE_FACE:
        m_oGCharData.setFace(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_FACE);
        break;
      case EUSERDATATYPE_MOUTH:
        m_oGCharData.setMouth(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_MOUTH);
        break;
      case EUSERDATATYPE_EYE:
        m_oGCharData.setEye(rData.value());
        m_bitset.set(EGUILDMEMBERDATA_EYE);
        break;
      default:
        XDBG << "[公会-数据同步]"<< rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "收到数据 type :" << rData.type() << "value :" << rData.value() << "未处理" << XEND;
        break;
    }
    XDBG << "[公会-数据同步]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "收到数据 type :" << rData.type() << "value :" << rData.value() << XEND;
  }

  if (bInit)
    m_bitset.reset();
  else
    updateData();
}

DWORD GMember::getWeekContribution()
{
  DWORD dwValue = m_oVar.getVarValue(EVARTYPE_GUILD_CONTRIBUTION);
  if (m_dwWeekCon != dwValue)
  {
    m_dwWeekCon = dwValue;
    m_bitset.set(EGUILDMEMBERDATA_WEEKCONTRIBUTION);
    updateData();
  }

  return dwValue;
}

void GMember::setContribution(DWORD c, bool bNtf /*= true*/, bool bAddRecord/* = true*/)
{
  bool subcon = true;
  if (m_dwCon < c)
  {
    if (bAddRecord)
    {
      DWORD value = m_oVar.getVarValue(EVARTYPE_GUILD_CONTRIBUTION) + c - m_dwCon;
      m_oVar.setVarValue(EVARTYPE_GUILD_CONTRIBUTION, value);
      m_dwWeekCon = value;
      m_bitset.set(EGUILDMEMBERDATA_WEEKCONTRIBUTION);

      m_dwTotalCon = m_dwTotalCon + c - m_dwCon;
      m_bitset.set(EGUILDMEMBERDATA_TOTALCONTRIBUTION);
    }

    subcon = false;
  }

  if (bNtf && c > m_dwCon)
  {
    if (isOnline() == true)
    {
      MsgParams params;
      params.addNumber(140);
      params.addNumber(140);
      params.addNumber(c - m_dwCon);
      thisServer->sendMsg(getZoneID(), getCharID(), 6, params);
    }
  }
  XLOG << "[公会-贡献] 个人贡献:" << "accid" << getAccid() << "charid" << getCharID() << "zoneid" << getZoneID() << "guildid"
    << (m_pGuild == nullptr ? 0 : m_pGuild->getGUID()) << "old" << m_dwCon << "after" << c << XEND;
  m_dwCon = c;
  m_bitset.set(EGUILDMEMBERDATA_CONTRIBUTION);
  updateData();
  if (getFrameStatus() == false)
  {
    setFrameStatus(true);
    setFrameStatus(false);
  }

  SocialUser oUser;
  oUser.set_charid(getCharID());
  oUser.set_zoneid(getZoneID());
  GuildManager::getMe().syncPrayToScene(oUser, subcon ? GUILDOPTCONTYPE_SUB : GUILDOPTCONTYPE_ADD, false);
}

DWORD GMember::getWeekAsset()
{
  DWORD dwValue = m_oVar.getVarValue(EVARTYPE_GUILD_ASSET);
  if (m_dwWeekAsset != dwValue)
  {
    m_dwWeekAsset = dwValue;
    m_bitset.set(EGUILDMEMBERDATA_WEEKASSET);
    updateData();
  }

  return dwValue;
}

void GMember::setAsset(DWORD a)
{
  DWORD old = m_dwAsset;
  m_dwAsset = a;
  if (m_dwAsset > old)
  {
    DWORD dwValue = m_dwAsset - old;
    m_dwWeekAsset = getWeekAsset() + dwValue;
    m_oVar.setVarValue(EVARTYPE_GUILD_ASSET, m_dwWeekAsset);
    m_bitset.set(EGUILDMEMBERDATA_WEEKASSET);

    DWORD dwAssetID = MiscConfig::getMe().getGuildCFG().dwAssetItemID;
    MsgParams params;
    params.addNumber(dwAssetID);
    params.addNumber(dwAssetID);
    params.addNumber(dwValue);
    thisServer->sendMsg(getZoneID(), getCharID(), 6, params);
  }
  XLOG << "[公会-资金] 个人资金:" << "accid"<<getAccid()<<"charid" << getCharID()<<"zoneid"<<getZoneID()<< "guildid" <<
    ( m_pGuild == nullptr ? 0:m_pGuild->getGUID()) << "old" << old<<"after"<<m_dwAsset << XEND;
  m_bitset.set(EGUILDMEMBERDATA_ASSET);
  updateData();
}

void GMember::setJob(EGuildJob eJob)
{
  if (eJob <= EGUILDJOB_MIN || eJob >= EGUILDJOB_MAX)
    return;
  bool bAuth = m_pGuild->getMisc().hasAuth(m_eJob, EAUTH_ARTIFACT_QUEST) || m_pGuild->getMisc().hasAuth(eJob, EAUTH_ARTIFACT_QUEST);
  m_eJob = eJob;
  m_bitset.set(EGUILDMEMBERDATA_JOB);
  updateData(true);
  if (bAuth)
    m_pGuild->getMisc().getQuest().questSyncToZone(getCharID());

  // 会长/副会长默认开启实时语音发言权限
  if ((eJob == EGUILDJOB_CHAIRMAN || eJob == EGUILDJOB_VICE_CHAIRMAN) && isRealtimeVoiceOpen() == false && m_pGuild->canAddRealtimeVoice())
    openRealtimeVoice(true);
}

bool GMember::sendCmdToMe(const void* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;
  if (isOnline() == false)
    return false;

  return thisServer->sendCmdToMe(getZoneID(), getCharID(), buf, len);
}

bool GMember::sendCmdToZone(const void* buf, WORD len)
{
  if (buf == nullptr || len == 0)
    return false;
  if (isOnline() == false)
    return false;

  return thisServer->sendCmdToZone(getZoneID(), buf, len);
}

/*void GMember::patch_checkLvOverflow()
{
  for (auto &v : m_vecPrays)
  {
    const SGuildPrayCFG* pPrayCFG = GuildConfig::getMe().getGuildPrayCFG(v.pray());
    if (pPrayCFG == nullptr || pPrayCFG->eType != EPRAYTYPE_GODDESS)
      continue;
    const SGuildCFG* pCFG = m_pGuild->getGuildCFG();
    if (pCFG == nullptr)
      continue;
    if (pCFG && v.lv() > pCFG->dwMaxPrayLv)
    {
      XLOG << "[Pray-等级溢出补丁], 玩家:" << getCharID() << "id:" << v.pray() << "原等级:" << v.lv() << "设置为:" << pCFG->dwMaxPrayLv << XEND;
      v.set_lv(pCFG->dwMaxPrayLv);
    }
  }
}*/

DWORD GMember::getPrayLv(DWORD dwPrayID) const
{
  auto v = find_if(m_vecPrays.begin(), m_vecPrays.end(), [&](const GuildMemberPray& r) -> bool{
    return r.pray() == dwPrayID;
  });
  if (v != m_vecPrays.end())
    return v->lv();

  return 0;
}

void GMember::setPrayLv(DWORD dwPrayID, DWORD dwLv)
{
  if (m_pGuild == nullptr)
  {
    XERR << "[公会成员-祈祷] 成员" << getAccid() << getCharID() << getProfession() << getName() << "将祈祷" << dwPrayID << "等级变更为" << dwLv << "失败,未包含正确公会指针" << XEND;
    return;
  }
  auto v = find_if(m_vecPrays.begin(), m_vecPrays.end(), [&](const GuildMemberPray& r) -> bool{
    return r.pray() == dwPrayID;
  });
  if (v != m_vecPrays.end())
  {
    v->set_lv(dwLv);
  }
  else
  {
    GuildMemberPray oPray;
    oPray.set_pray(dwPrayID);
    oPray.set_lv(dwLv);
    m_vecPrays.push_back(oPray);
    v = find_if(m_vecPrays.begin(), m_vecPrays.end(), [&](const GuildMemberPray& r) -> bool{
      return r.pray() == dwPrayID;
    });
    if (v == m_vecPrays.end())
      return;
  }

  m_pGuild->setMark(EGUILDDATA_MEMBER);

  XLOG << "[公会成员-祈祷] 成员" << getAccid() << getCharID() << getProfession() << getName()
    << "在公会" << m_pGuild->getGUID() << m_pGuild->getName() << "将祈祷" << dwPrayID << "等级变更为" << dwLv << XEND;
}

void GMember::setGiftPoint(DWORD dwPoint)
{
  m_dwGiftPoint = dwPoint;
  if (m_pGuild != nullptr)
    m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::setLevelupEffect(bool bEffect)
{
  m_bLevelupEffect = bEffect;

  if (isOnline() == false)
    return;

  GuildMemberDataUpdateGuildCmd cmd;
  cmd.set_type(EGUILDLIST_MEMBER);
  cmd.set_charid(getCharID());

  GuildMemberDataUpdate* pData = cmd.add_updates();
  if (pData == nullptr)
    return;

  pData->set_type(EGUILDMEMBERDATA_LEVELUPEFFECT);
  pData->set_value(getLevelupEffect());
  PROTOBUF(cmd, send, len);

  thisServer->sendCmdToMe(getZoneID(), getCharID(), send, len);
}

DonateItem* GMember::getDonateItem(DWORD dwConfigID, DWORD dwTime)
{
  auto v = find_if(m_vecDonateItems.begin(), m_vecDonateItems.end(), [&](const DonateItem& r) -> bool{
    return r.configid() == dwConfigID && r.time() == dwTime;
  });
  if (v != m_vecDonateItems.end())
    return &(*v);

  return nullptr;
}

void GMember::collectDonateList(DonateListGuildCmd& cmd)
{
  //refreshDonate();
  refreshDonateInOrder();

  for (auto &v : m_vecDonateItems)
  {
    if (v.itemid() != 0)
      cmd.add_items()->CopyFrom(v);
  }
}

//void GMember::collectDonateStep(TSetDWORD& setStep)
//{
//  for (int i = 0; i < m_oData.donate().steps_size(); ++i)
//    setStep.insert(m_oData.donate().steps(i));
//}

bool GMember::addDonateItem(DWORD dwConfigID, DWORD dwTime, DWORD dwItemID, DWORD dwItemCount, DWORD donateCount/* = 0*/)
{
  if (m_pGuild == nullptr || m_pGuild->getGuildCFG() == nullptr)
    return false;

  DonateItem* pItem = getDonateItem(dwConfigID, dwTime);
  if (pItem != nullptr)
    return false;

  DWORD contribute = getAssetByID(dwConfigID);
  DWORD medal = getMedalByID(dwConfigID);

  DonateItem oItem;
  oItem.set_configid(dwConfigID);
  oItem.set_time(dwTime);
  oItem.set_itemid(dwItemID);
  oItem.set_itemcount(dwItemCount);
  oItem.set_count(donateCount);
  oItem.set_contribute(contribute);
  oItem.set_medal(medal);
  m_vecDonateItems.push_back(oItem);

  UpdateDonateItemGuildCmd cmd;
  cmd.mutable_item()->CopyFrom(oItem);

  DWORD dwMaxCount = m_pGuild->getGuildCFG()->dwDonateList;
  //if (dwMaxCount != 0 && static_cast<DWORD>(m_vecDonateItems.size()) > dwMaxCount)
  while(dwMaxCount != 0 && getDonateItemCount() > dwMaxCount)
  {
    cmd.mutable_del()->CopyFrom(m_vecDonateItems[0]);
    m_vecDonateItems.erase(m_vecDonateItems.begin());
  }

  if (isOnline() == true && getDonateFrame() == true)
  {
    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
  }

  // save
  if (m_pGuild != nullptr)
    m_pGuild->setMark(EGUILDDATA_MEMBER);
  return true;
}

void GMember::refreshDonate()
{
  DWORD dwNow = xTime::getCurSec();
  refreshDonate(m_dwDonateTime1, dwNow, EDONATETYPE_NORMAL);
  refreshDonate(m_dwDonateTime2, dwNow, EDONATETYPE_HIGHER);
  refreshDonate(m_dwDonateTime3, dwNow, EDONATETYPE_SILVER);
  refreshDonate(m_dwDonateTime4, dwNow, EDONATETYPE_BOSS);
}

void GMember::updateData(bool bNoCache /*= false*/)
{
  if (m_bitset.any() == false)
    return;

  GuildMemberDataUpdateGuildCmd cmd;
  GuildMemberDataUpdateGuildSCmd scmd;
  bool bNeedSave = false;
  set<EGuildMemberData> setDatas;

  fetchData(m_bitset, cmd, scmd, bNeedSave);
  for (int i = 0; i < EGUILDMEMBERDATA_MAX; ++i)
  {
    if (m_bitset.test(i) == true)
      setDatas.insert(static_cast<EGuildMemberData>(i));
  }
  m_bitset.reset();

  if (m_pGuild != nullptr && cmd.updates_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    if (cmd.type() == EGUILDLIST_APPLY)
    {
      m_pGuild->broadcastCmd(send, len, true);
    }
    else
    {
      TVecGuildMember& vecMembers = m_pGuild->getAllMemberList();
      for (auto &v : vecMembers)
      {
        if (v->isOnline() == false)
          continue;
        if (bNoCache || v->getFrameStatus() == true || v->getDonateFrame() == true)
        {
          v->sendCmdToMe(send, len);
          XDBG << "[公会成员-数据更新] 成员" << v->getAccid() << v->getCharID() << v->getProfession() << v->getName() << "收到更新" << cmd.ShortDebugString() << XEND;
          continue;
        }
        for (auto &s : setDatas)
          v->setMemberMark(getCharID(), s);
      }
      if (bNeedSave)
        m_pGuild->setMark(EGUILDDATA_MEMBER);
    }

    XDBG << "[公会成员-数据更新]" << getCharID() << getProfession() << getName() << "同步客户端" << cmd.ShortDebugString() << XEND;
  }

  if (m_pGuild != nullptr && scmd.updates_size() > 0)
  {
    const TVecGuildMember& vecMembers = m_pGuild->getMemberList();
    for (auto &v : vecMembers)
    {
      if (v->isOnline() == false)
        continue;
      scmd.set_charid(v->getCharID());
      PROTOBUF(scmd, ssend, slen);
      thisServer->sendCmdToZone(v->getZoneID(), ssend, slen);
    }
    XDBG << "[公会成员-数据更新]" << getCharID() << getProfession() << getName() << "同步场景" << cmd.ShortDebugString() << XEND;
  }
}

void GMember::fetchData(const bitset<EGUILDMEMBERDATA_MAX>& bit, GuildMemberDataUpdateGuildCmd& cmd, GuildMemberDataUpdateGuildSCmd& scmd, bool& bNeedSave)
{
  cmd.set_charid(getCharID());
  scmd.set_destid(getCharID());
  if (m_eJob == EGUILDJOB_APPLY)
    cmd.set_type(EGUILDLIST_APPLY);
  else if (m_eJob >= EGUILDJOB_CHAIRMAN && m_eJob <= EGUILDJOB_MEMBER10 && m_eJob != EGUILDJOB_APPLY && m_eJob != EGUILDJOB_INVITE)
    cmd.set_type(EGUILDLIST_MEMBER);

  for (int i = EGUILDMEMBERDATA_MIN + 1; i < EGUILDMEMBERDATA_MAX; ++i)
  {
    if (bit.test(i) == false)
      continue;

    EGuildMemberData eType = static_cast<EGuildMemberData>(i);
    switch (eType)
    {
      case EGUILDMEMBERDATA_MIN:
        break;
      case EGUILDMEMBERDATA_BASELEVEL:
        GGuild::add_mdata(cmd.add_updates(), eType, getBaseLevel());
        break;
      case EGUILDMEMBERDATA_WEEKCONTRIBUTION:
        GGuild::add_mdata(cmd.add_updates(), eType, getWeekContribution());
        break;
      case EGUILDMEMBERDATA_CONTRIBUTION:
        GGuild::add_mdata(cmd.add_updates(), eType, getContribution());
        bNeedSave = true;
        break;
      case EGUILDMEMBERDATA_TOTALCONTRIBUTION:
        GGuild::add_mdata(cmd.add_updates(), eType, getTotalContribution());
        bNeedSave = true;
        break;
      case EGUILDMEMBERDATA_ENTERTIME:
        GGuild::add_mdata(cmd.add_updates(), eType, getEnterTime());
        GGuild::add_mdata(scmd.add_updates(), eType, getEnterTime());
        break;
      case EGUILDMEMBERDATA_OFFLINETIME:
        GGuild::add_mdata(cmd.add_updates(), eType, isOnline() == true ? 0 : getOfflineTime());
        GGuild::add_mdata(scmd.add_updates(), eType, getOfflineTime());
        break;
      case EGUILDMEMBERDATA_PROFESSION:
        GGuild::add_mdata(cmd.add_updates(), eType, getProfession());
        break;
      case EGUILDMEMBERDATA_GENDER:
        GGuild::add_mdata(cmd.add_updates(), eType, getGender());
        break;
      case EGUILDMEMBERDATA_PORTRAIT:
        GGuild::add_mdata(cmd.add_updates(), eType, getPortrait());
        break;
      case EGUILDMEMBERDATA_FRAME:
        GGuild::add_mdata(cmd.add_updates(), eType, getFrame());
        break;
      case EGUILDMEMBERDATA_HAIR:
        GGuild::add_mdata(cmd.add_updates(), eType, getHair());
        break;
      case EGUILDMEMBERDATA_HAIRCOLOR:
        GGuild::add_mdata(cmd.add_updates(), eType, getHairColor());
        break;
      case EGUILDMEMBERDATA_BODY:
        GGuild::add_mdata(cmd.add_updates(), eType, getBody());
        break;
      case EGUILDMEMBERDATA_JOB:
        GGuild::add_mdata(cmd.add_updates(), eType, getJob());
        GGuild::add_mdata(scmd.add_updates(), eType, getJob());
        break;
      case EGUILDMEMBERDATA_LEVELUPEFFECT:  // only send to me
        break;
      case EGUILDMEMBERDATA_WEEKASSET:
        GGuild::add_mdata(cmd.add_updates(), eType, getWeekAsset());
        break;
      case EGUILDMEMBERDATA_ASSET:
        GGuild::add_mdata(cmd.add_updates(), eType, getAsset());
        break;
      case EGUILDMEMBERDATA_ZONEID:
        GGuild::add_mdata(cmd.add_updates(), eType, getClientZoneID(getZoneID()));
        break;
      case EGUILDMEMBERDATA_NAME:
        GGuild::add_mdata(cmd.add_updates(), eType, 0, getName());
        break;
      case EGUILDMEMBERDATA_ONLINETIME:
        GGuild::add_mdata(scmd.add_updates(), eType, getOnlineTime());
        break;
      case EGUILDMEMBERDATA_HEAD:
        GGuild::add_mdata(cmd.add_updates(), eType, getHead());
        break;
      case EGUILDMEMBERDATA_FACE:
        GGuild::add_mdata(cmd.add_updates(), eType, getFace());
        break;
      case EGUILDMEMBERDATA_MOUTH:
        GGuild::add_mdata(cmd.add_updates(), eType, getMouth());
        break;
      case EGUILDMEMBERDATA_EYE:
        GGuild::add_mdata(cmd.add_updates(), eType, getEye());
        break;
      case EGUILDMEMBERDATA_BUILDINGEFFECT:
        GGuild::add_mdata(scmd.add_updates(), eType, m_bBuildingEffect);
        break;
      case EGUILDMEMBERDATA_WEEKBCOIN:
        GGuild::add_mdata(cmd.add_updates(), eType, getWeekBCoin());
        break;
      case EGUILDMEMBERDATA_TOTALBCOIN:
        GGuild::add_mdata(cmd.add_updates(), eType, getTotalBCoin());
        break;
      case EGUILDMEMBERDATA_REALTIMEVOICE:
        GGuild::add_mdata(cmd.add_updates(), eType, m_bRealtimeVoice);
        break;
      case EGUILDMEMBERDATA_MAX:
        break;
    }
  }
}

void GMember::resetWeek()
{
  if (m_oVar.getVarValue(EVARTYPE_GUILD_DONATE) != 0)
    return;
  m_vecDonateItems.clear();
  m_oVar.setVarValue(EVARTYPE_GUILD_DONATE, 1);

  if (m_pGuild != nullptr)
  {
    m_pGuild->setMark(EGUILDDATA_MEMBER);
    m_pGuild->setMark(EGUILDDATA_MISC);
  }
}

void GMember::refreshDonate(DWORD& dwTime, DWORD curTime, EDonateType eType)
{
  if (m_pGuild == nullptr) return;

  if (m_pGuild->getGuildCFG() == nullptr)
  {
    XERR << "[公会成员-捐赠刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << getBaseLevel()
      << "进行" << eType << "捐赠列表刷新失败,无公会信息" << XEND;
    return;
  }

  //if (dwTime > curTime)
  //  return;

  DWORD dwLastTime = dwTime;
  //DWORD dwGuildTime = eType == EDONATETYPE_NORMAL ? m_pGuild->getMisc().getDonateTime1() : m_pGuild->getMisc().getDonateTime2();
  //DWORD dwInterval = eType == EDONATETYPE_NORMAL ? m_pGuild->getGuildCFG()->dwDonateRefreshInterval1 : m_pGuild->getGuildCFG()->dwDonateRefreshInterval2;
  DWORD dwGuildTime = 0;
  switch (eType)
  {
    case EDONATETYPE_NORMAL:
      {
        dwGuildTime = m_pGuild->getMisc().getDonateTime1();
      }
      break;
    case EDONATETYPE_HIGHER:
      {
        dwGuildTime = m_pGuild->getMisc().getDonateTime2();
      }
      break;
    case EDONATETYPE_SILVER:
      {
        dwGuildTime = m_pGuild->getMisc().getDonateTime3();
      }
      break;
    case EDONATETYPE_BOSS:
      {
        dwGuildTime = m_pGuild->getMisc().getDonateTime4();
      }
      break;
    default:
      XERR << "[公会成员-捐赠刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
        getBaseLevel() << "进行" << eType << "捐赠列表刷新失败,没有此种捐赠类型" << XEND;
      return;
  }
  dwTime = dwGuildTime;

  m_pGuild->setMark(EGUILDDATA_MEMBER);
  //resetWeek();

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().randGuildDonateCFG(getBaseLevel(), eType);
  if (pCFG == nullptr)
  {
    XERR << "[公会成员-捐赠刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << getBaseLevel()
      << "在" << curTime << "进行" << eType << "捐赠列表刷新失败,未找到配置" << XEND;
    return;
  }
  const SGuildDonateItem* pItem = pCFG->randDonateItem();
  if (pItem == nullptr)
  {
    XERR << "[公会成员-捐赠刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << getBaseLevel()
      << "在" << curTime << "进行" << eType << "捐赠列表刷新失败,随机物品失败" << XEND;
    return;
  }

  addDonateItem(pCFG->dwID, curTime, pItem->dwItemID, pItem->dwCount);
  XLOG << "[公会成员-捐赠刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << getBaseLevel()
    << "在" << curTime << "进行" << eType << "捐赠列表刷新成功,id :" << pCFG->dwID << "itemid :" << pItem->dwItemID << "count :" << pItem->dwCount
    << "上一次刷新时间 :" << dwLastTime << "下一次刷新时间 :" << dwTime << XEND;
}

bool GMember::updateDonateItem(DWORD nextid, EDonateType eType, DWORD time, DWORD configid)
{
  if (m_pGuild == nullptr)
    return false;

  if (nextid == 0)
    return false;

  DWORD curTime = xTime::getCurSec();
  if (eType <= EDONATETYPE_MIN || eType >= EDONATETYPE_MAX)
  {
    XERR << "[公会-捐赠替换] 失败" << m_pGuild->getGUID() << m_pGuild->getName() << "添加" << eType << "失败,不合法的类型" << curTime << XEND;
    return false;
  }

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(nextid);
  if(pCFG == nullptr || pCFG->eType != eType)
    return false;

  const SGuildDonateItem* pItem = pCFG->randDonateItem();
  if (pItem == nullptr)
  {
    XERR << "[公会成员-捐赠替换] 失败" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
      getBaseLevel() << "在" << curTime << "进行" << eType << "捐赠列表刷新失败,随机物品失败" << XEND;
    return false;
  }

  DonateItem* pPreItem = getDonateItem(configid, time);
  if (pPreItem == nullptr)
  {
    XERR << "[公会成员-捐赠替换] 失败" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
      getBaseLevel() << "在" << curTime << "进行" << configid << eType << "捐赠列表刷新失败,没有找到捐赠物品" << XEND;
    return false;
  }

  DWORD contribute = getAssetByID(nextid);
  DWORD medal = getMedalByID(nextid);
  pPreItem->set_configid(nextid);
  pPreItem->set_itemid(pItem->dwItemID);
  pPreItem->set_itemcount(pItem->dwCount);
  pPreItem->set_count(0);
  pPreItem->set_contribute(contribute);
  pPreItem->set_medal(medal);

  UpdateDonateItemGuildCmd cmd;
  cmd.mutable_item()->CopyFrom(*pPreItem);

  if (isOnline() == true && getDonateFrame() == true)
  {
    PROTOBUF(cmd, send, len);
    sendCmdToMe(send, len);
  }

  // save
  m_pGuild->setMark(EGUILDDATA_MEMBER);
  //addDonateItem(pCFG->dwID, time, pItem->dwItemID, pItem->dwCount);
  XLOG << "[公会成员-捐赠替换] 成功" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
    getBaseLevel() << "在" << curTime << "进行" << eType << "捐赠列表刷新成功,id :" << pCFG->dwID << "itemid :" << pItem->dwItemID << 
    "count :" << pItem->dwCount << "捐赠id : " << configid  << XEND;
  return true;
}

DWORD GMember::getDonateItemCount()
{
  DWORD count = 0;
  for (auto &v : m_vecDonateItems)
  {
    if(v.count() == 0)
      count++;
  }

  return count;
}

void GMember::refreshDonateInOrder()
{
  DWORD dwNow = xTime::getCurSec();
  bool blrefresh1 = m_dwDonateTime1 > dwNow;
  bool blrefresh2 = m_dwDonateTime2 > dwNow;
  bool blrefresh3 = m_dwDonateTime3 > dwNow;
  bool blrefresh4 = m_dwDonateTime4 > dwNow;

  if(blrefresh1 && blrefresh2 && blrefresh3 && blrefresh4)
    return;
  else if(m_dwDonateTime1 == 0)
  {
    refreshDonate(m_dwDonateTime1, dwNow + EDONATETYPE_NORMAL, EDONATETYPE_NORMAL);
    refreshDonate(m_dwDonateTime2, dwNow + EDONATETYPE_HIGHER, EDONATETYPE_HIGHER);
    refreshDonate(m_dwDonateTime3, dwNow + EDONATETYPE_SILVER, EDONATETYPE_SILVER);
    refreshDonate(m_dwDonateTime4, dwNow + EDONATETYPE_BOSS, EDONATETYPE_BOSS);

    XLOG << "[公会成员-捐赠顺序刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
      getBaseLevel() << "在" << dwNow << "捐赠列表第一次刷新成功" << XEND;
    return;
  }

  DWORD dwInterval1 = m_pGuild->getGuildCFG()->dwDonateRefreshInterval1;
  DWORD lastRefreshTime1 = m_dwDonateTime1 - dwInterval1;
  DWORD dwInterval2 = m_pGuild->getGuildCFG()->dwDonateRefreshInterval2;
  DWORD lastRefreshTime2 = m_dwDonateTime2 - dwInterval2;
  DWORD dwInterval3 = m_pGuild->getGuildCFG()->dwDonateRefreshInterval3;
  DWORD lastRefreshTime3 = m_dwDonateTime3 - dwInterval3;
  DWORD dwInterval4 = m_pGuild->getGuildCFG()->dwDonateRefreshInterval4;
  DWORD lastRefreshTime4 = m_dwDonateTime4 - dwInterval4;
  if(dwInterval1*dwInterval2*dwInterval3*dwInterval4 == 0)
    return;

  DWORD dwCommonDivisor = m_pGuild->getGuildCFG()->dwCommonDivisor;
  DWORD dwDonate = dwCommonDivisor;
  DWORD dwCount = 0;
  bool refresh = true;

  while(refresh)
  {
    if(!blrefresh1 && dwDonate % dwInterval1 == 0 && dwDonate/dwInterval1 != 0 && lastRefreshTime1 + dwDonate <= dwNow)
    {
      refreshDonate(m_dwDonateTime1, dwNow + dwCount, EDONATETYPE_NORMAL);
      dwCount++;
    }
    else if(lastRefreshTime1 + dwDonate > dwNow)
      blrefresh1 = true;

    if(!blrefresh2 && dwDonate % dwInterval2 == 0 && dwDonate/dwInterval2 != 0 && lastRefreshTime2 + dwDonate <= dwNow)
    {
      refreshDonate(m_dwDonateTime2, dwNow + dwCount, EDONATETYPE_HIGHER);
      dwCount++;
    }
    else if(lastRefreshTime2 + dwDonate > dwNow)
      blrefresh2 = true;

    if(!blrefresh3 && dwDonate % dwInterval3 == 0 && dwDonate/dwInterval3 != 0 && lastRefreshTime3 + dwDonate <= dwNow)
    {
      refreshDonate(m_dwDonateTime3, dwNow + dwCount, EDONATETYPE_SILVER);
      dwCount++;
    }
    else if(lastRefreshTime3 + dwDonate > dwNow)
      blrefresh3 = true;

    if(!blrefresh4 && dwDonate % dwInterval4 == 0 && dwDonate/dwInterval4 != 0 && lastRefreshTime4 + dwDonate <= dwNow)
    {
      refreshDonate(m_dwDonateTime4, dwNow + dwCount, EDONATETYPE_BOSS);
      dwCount++;
    }
    else if(lastRefreshTime4 + dwDonate > dwNow)
      blrefresh4 = true;

    dwDonate += dwCommonDivisor;
    if(blrefresh1 && blrefresh2 && blrefresh3 && blrefresh4)
      refresh = false;
  }

  XLOG << "[公会成员-捐赠顺序刷新] 公会" << m_pGuild->getGUID() << m_pGuild->getName() << "中成员" << getCharID() << getProfession() << 
    getBaseLevel() << "在" << dwNow << "进行捐赠列表刷新成功" << dwCount << XEND;
}

DWORD GMember::getAssetByID(DWORD dwConfigID)
{
  DWORD contribute = 0;

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(dwConfigID);
  if(pCFG == nullptr)
    return 0;

  DWORD ASSET = static_cast<DWORD>(EITEMTYPE_ASSET);
  for (auto &v : pCFG->vecUserReward)
  {
    if(v.id() == ASSET)
    {
      contribute = v.count();
      break;
    }
  }

  return contribute;
}

void GMember::resetDonateTime(DWORD time)
{
  m_dwDonateTime1 = time;
  m_dwDonateTime2 = time;
  m_dwDonateTime3 = time;
  m_dwDonateTime4 = time;
}

DWORD GMember::getMedalByID(DWORD dwConfigID)
{
  DWORD contribute = 0;

  const SGuildDonateCFG* pCFG = GuildConfig::getMe().getGuildDonateCFG(dwConfigID);
  if(pCFG == nullptr)
    return 0;

  DWORD ASSET = 5261;
  for (auto &v : pCFG->vecUserReward)
  {
    if(v.id() == ASSET)
    {
      contribute = v.count();
      break;
    }
  }

  return contribute;
}

void GMember::setFrameStatus(bool bFrame, bool bOffline /*= false*/)
{
  m_bFrameStatus = bFrame;
  XLOG << "[公会成员-界面状态]" << getAccid() << getCharID() << getProfession() << getName() << "设置界面状态" << m_bFrameStatus << XEND;

  if (bOffline)
  {
    m_guildbitset.reset();
    m_setPackUpdate.clear();
    m_setMemberUpdate.clear();
    m_mapMemberBitset.clear();
    return;
  }

  // guild
  if (m_guildbitset.any() == true && m_pGuild != nullptr)
  {
    GuildDataUpdateGuildCmd cmd;
    GuildDataUpdateGuildSCmd scmd;
    CityDataUpdateGuildSCmd ccmd;

    m_pGuild->fetchData(m_guildbitset, cmd, scmd, ccmd);
    if (cmd.updates_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
      XDBG << "[公会成员-界面状态]" << getAccid() << getCharID() << getProfession() << getName() << "同步公会数据缓存" << cmd.ShortDebugString() << XEND;
    }
  }
  m_guildbitset.reset();

  // pack
  if (m_setPackUpdate.empty() == false && m_pGuild != nullptr)
  {
    PackUpdateGuildCmd cmd;

    m_pGuild->getPack().fetch(m_setPackUpdate, cmd);
    if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
    }
    XDBG << "[公会成员-界面状态]" << getAccid() << getCharID() << getProfession() << getName() << "同步仓库缓存" << cmd.ShortDebugString() << XEND;
  }
  m_setPackUpdate.clear();

  // member
  if (m_setMemberUpdate.empty() == false && m_pGuild != nullptr)
  {
    GuildMemberUpdateGuildCmd cmd;
    GuildMemberUpdateGuildSCmd scmd;

    m_pGuild->fetchMember(m_setMemberUpdate, cmd, scmd);
    if (cmd.updates_size() > 0 || cmd.dels_size() > 0)
    {
      PROTOBUF(cmd, send, len);
      sendCmdToMe(send, len);
      XDBG << "[公会成员-界面状态]" << getAccid() << getCharID() << getProfession() << getName() << "同步成员缓存" << cmd.ShortDebugString() << XEND;
    }
  }
  m_setMemberUpdate.clear();

  // member data
  if (m_mapMemberBitset.empty() == false && m_pGuild != nullptr)
  {
    for (auto &m : m_mapMemberBitset)
    {
      GMember* pTarget = m_pGuild->getMember(m.first);
      if (pTarget == nullptr)
        continue;

      GuildMemberDataUpdateGuildCmd cmd;
      GuildMemberDataUpdateGuildSCmd scmd;
      bool b = false;
      pTarget->fetchData(m.second, cmd, scmd, b);

      if (cmd.updates_size() > 0)
      {
        PROTOBUF(cmd, send, len);
        sendCmdToMe(send, len);
        XDBG << "[公会成员-界面状态]" << getAccid() << getCharID() << getProfession() << getName() << "同步成员数据缓存" << cmd.ShortDebugString() << XEND;
      }
    }
  }
  m_mapMemberBitset.clear();
}

void GMember::setRedTip(bool bRed, bool sync)
{
  bool oldRed = m_bHasRedTip;
  if(bRed == true && m_pGuild->getApplyCount() == 0)
    bRed = false;

  m_bHasRedTip = bRed;
  if(sync == false)
    return;

  if(oldRed != m_bHasRedTip && isOnline() == true)
  {
    redTipMessage(EREDSYS_GUILD_APPLY, m_bHasRedTip);
    XLOG << "[公会成员-红点] 变化" << getAccid() << getCharID() << getProfession() << getName()
      << "在公会" << m_pGuild->getGUID() << m_pGuild->getName() << "将红点变为: " << m_bHasRedTip << XEND;
  }
}

void GMember::redTipMessage(ERedSys eRedSys, bool isAdd)
{
  SyncRedTipSocialCmd cmd;
  cmd.set_dwid(m_pGuild->getGUID());
  cmd.set_charid(getCharID());
  cmd.set_red(eRedSys);
  cmd.set_add(isAdd);

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(getZoneID(), send, len);
}

bool GMember::canSubmitMaterial(EGuildBuilding type, DWORD count)
{
  if (count <= 0)
    return false;

  resetSubmitCount();

  auto it = m_mapGuildBuilding.find(type);
  if (it != m_mapGuildBuilding.end())
    count += it->second.submitcount();

  return count <= MiscConfig::getMe().getGuildBuildingCFG().dwMaxSubmitCount;
}

void GMember::addSubmitCount(EGuildBuilding type, DWORD count)
{
  if (nullptr == m_pGuild) return;
  if (count <= 0)
    return;

  resetSubmitCount();

  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end())
  {
    m_mapGuildBuilding[type].set_type(type);
    it = m_mapGuildBuilding.find(type);
    if (it == m_mapGuildBuilding.end())
      return;
  }
  it->second.set_submitcount(it->second.submitcount() + count);
  it->second.set_submitcounttotal(it->second.submitcounttotal() + count);
  it->second.set_submittime(now());
  m_pGuild->getMisc().getBuilding().updateSubmitRank(type, this);
  m_pGuild->setMark(EGUILDDATA_MEMBER);

  sendSubmitCountToMe(type);
}

void GMember::resetSubmitCount(bool force/* = false*/)
{
  if (nullptr == m_pGuild) return;
  if (force == false && m_oVar.getVarValue(EVARTYPE_GUILD_BUILDING_SUBMIT_DAY) == 1)
    return;
  m_oVar.setVarValue(EVARTYPE_GUILD_BUILDING_SUBMIT_DAY, 1);
  for (auto& v : m_mapGuildBuilding)
  {
    if (v.second.submitcount())
    {
      v.second.set_submitcount(0);
    }
  }
  m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::resetSubmitCountTotal(EGuildBuilding type)
{
  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end() || it->second.submitcounttotal() == 0)
    return;
  it->second.set_submitcounttotal(0);
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

DWORD GMember::getSubmitCount(EGuildBuilding type)
{
  resetSubmitCount();

  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end())
    return 0;

  return it->second.submitcount();
}

void GMember::sendSubmitCountToMe(EGuildBuilding type)
{
  resetSubmitCount();

  BuildingSubmitCountGuildCmd cmd;
  cmd.set_type(type);
  auto it = m_mapGuildBuilding.find(type);
  if (it != m_mapGuildBuilding.end())
    cmd.set_count(it->second.submitcount());
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

const UserGuildBuilding* GMember::getBuildingData(EGuildBuilding type)
{
  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end())
    return nullptr;
  return &it->second;
}

void GMember::clearVarWhenRemovedFromGuild()
{
  m_oVar.clearExcept(set<EVarType>{EVARTYPE_GUILD_BUILDING_SUBMIT_DAY});
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::buildingLevelUpEffect(const set<EGuildBuilding>& types, bool add)
{
  if (nullptr == m_pGuild) return;
  for (auto type : types)
  {
    if (type <= EGUILDBUILDING_MIN || type >= EGUILDBUILDING_MAX)
      continue;
    if (m_mapGuildBuilding.find(type) == m_mapGuildBuilding.end())
      m_mapGuildBuilding[type].set_type(type);
    m_mapGuildBuilding[type].set_levelupeffect(add);
    m_pGuild->setMark(EGUILDDATA_MEMBER);
  }

  BuildingLvupEffGuildCmd cmd;
  for (auto& v : m_mapGuildBuilding)
    if (v.second.levelupeffect())
    {
      BuildingLvupEffect* p = cmd.add_effects();
      if (p)
      {
        p->set_type(v.first);
        p->set_level(m_pGuild->getMisc().getBuilding().getBuildingLevel(v.first));
      }
    }

  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

void GMember::clearBuildingDataWhenExitGuild()
{
  for (auto& v : m_mapGuildBuilding)
  {
    v.second.set_levelupeffect(false);
    v.second.set_submitcounttotal(0);
    v.second.set_submittime(0);
  }
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

bool GMember::canGetBuildingWelfare(EGuildBuilding type)
{
  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end())
    return true;;
  return now() >= it->second.nextwelfaretime();
}

void GMember::setBuildingNextWelfareTime(EGuildBuilding type, DWORD time)
{
  auto it = m_mapGuildBuilding.find(type);
  if (it == m_mapGuildBuilding.end())
    m_mapGuildBuilding[type].set_type(type);
  m_mapGuildBuilding[type].set_nextwelfaretime(time);
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::setBuildingEffect()
{
  m_bBuildingEffect = true;
  m_bitset.set(EGUILDMEMBERDATA_BUILDINGEFFECT);
  updateData();
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::resetBuildingEffect()
{
  m_bBuildingEffect = false;
  m_bitset.set(EGUILDMEMBERDATA_BUILDINGEFFECT);
  updateData();
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

bool GMember::hasWelfare(EGuildWelfare type, QWORD id/* = 0*/)
{
  return m_pGuild->getMisc().getWelfare().isMemberHasWelfare(this, type, id);
}

void GMember::addTotalBCoin(DWORD dwCount)
{
  m_dwTotalBCoin += dwCount;
  m_bitset.set(EGUILDMEMBERDATA_TOTALBCOIN);

  DWORD dwVar = m_oVar.getVarValue(EVARTYPE_GUILD_MEMBER_WEEKBCOIN);
  dwVar += dwCount;
  m_oVar.setVarValue(EVARTYPE_GUILD_MEMBER_WEEKBCOIN, dwVar);
  if (m_dwWeekBCoin != dwVar)
  {
    m_dwWeekBCoin = dwVar;
    m_bitset.set(EGUILDMEMBERDATA_WEEKBCOIN);
  }

  updateData();
  XLOG << "[公会成员-B格猫金币]" << m_pGuild->getGUID() << m_pGuild->getName()
    << "中成员" << getAccid() << getCharID() << getProfession() << getName() << "增加" << dwCount << "金币, total :" << m_dwTotalBCoin << "weektotal :" << dwVar << XEND;
}

DWORD GMember::getWeekBCoin()
{
  DWORD dwVar = m_oVar.getVarValue(EVARTYPE_GUILD_MEMBER_WEEKBCOIN);
  if (m_dwWeekBCoin != dwVar)
  {
    m_dwWeekBCoin = dwVar;
    m_bitset.set(EGUILDMEMBERDATA_WEEKBCOIN);
    updateData();
  }
  return m_dwWeekBCoin;
}

void GMember::setRedTip(ERedSys type, bool add)
{
  add ? (m_qwRedTip |= QWORD(1) << DWORD(type)) : (m_qwRedTip &= ~(QWORD(1) << DWORD(type)));
  syncRedTipToScene(type);
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::syncRedTipToScene(ERedSys type)
{
  if (isOnline() == false)
    return;

  SyncRedTipSocialCmd cmd;
  cmd.set_dwid(m_pGuild->getGUID());
  cmd.set_charid(getCharID());
  cmd.set_red(type);
  cmd.set_add(isRedTipSet(type));

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToZone(getZoneID(), send, len);
}

void GMember::setLastExitTime(DWORD time)
{
  m_dwLastExitTime = time;
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::setChallengeFinished(DWORD index)
{
  m_qwChallenge |= QWORD(1) << index;
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

void GMember::resetChallenge()
{
  m_qwChallenge = 0;
  if (m_pGuild) m_pGuild->setMark(EGUILDDATA_MEMBER);
}

bool GMember::openRealtimeVoice(bool open)
{
  if (open == m_bRealtimeVoice)
    return false;
  m_bRealtimeVoice = open;
  m_bitset.set(EGUILDMEMBERDATA_REALTIMEVOICE);
  updateData(true);
  if (m_pGuild)
  {
    m_pGuild->setMark(EGUILDDATA_MEMBER);
    m_pGuild->getEvent().addEvent(open ? EGUILDEVENT_OPEN_REALTIME_VOICE : EGUILDEVENT_CLOSE_REALTIME_VOICE, TVecString{getName()});
  }
  if (open == true) sendRealtimeVoiceID();
  return true;
}

void GMember::sendRealtimeVoiceID()
{
  if (m_pGuild == nullptr || isRealtimeVoiceOpen())
    return;

  QueryRealtimeVoiceIDCmd cmd;
  cmd.set_channel(ECHAT_CHANNEL_GUILD);
  cmd.set_id(m_pGuild->getRealtimeVoiceID());
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

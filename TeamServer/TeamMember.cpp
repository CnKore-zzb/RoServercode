#include "Team.h"
#include "GCharManager.h"
#include "TeamServer.h"
#include "TeamManager.h"
#include "GTeam.h"
#include "MatchSCmd.pb.h"

// team member
TMember::TMember(Team* pTeam, DWORD dwAccid, QWORD qwCharID, DWORD dwZoneID, ETeamJob eJob) : m_pTeam(pTeam), m_oGCharData(thisServer->getRegionID(), qwCharID), m_eJob(eJob)
{
  m_dwCreateTime = xTime::getCurSec();
  m_oGCharData.setAccID(dwAccid);
  m_oGCharData.setZoneID(dwZoneID);
}

TMember::TMember(Team* pTeam, const TeamMember &data) : m_pTeam(pTeam), m_oGCharData(thisServer->getRegionID(), data.guid())
{
  m_dwCreateTime = xTime::getCurSec();
  load(data);
}

TMember::TMember(Team* pTeam, const UserInfo& data, ETeamJob eJob) : m_pTeam(pTeam), m_oGCharData(thisServer->getRegionID(), data.user().charid()), m_eJob(eJob)
{
  m_dwCreateTime = xTime::getCurSec();

  const SocialUser& rUser = data.user();

  m_oGCharData.setAccID(rUser.accid());
  m_oGCharData.setCharID(rUser.charid());
  m_oGCharData.setZoneID(rUser.zoneid());
  m_oGCharData.setName(rUser.name());

  if (getCatID() == 0)
    m_oGCharData.getByTeam();

  updateData(data, true);
}

TMember::~TMember()
{

}

bool TMember::initData()
{
  if (!m_oGCharData.getCharID())
  {
    XERR << "[队伍-成员初始化] 没有初始化" << XEND;
    return false;
  }

  if (m_oGCharData.get())
  {
    m_oGCharData.debug_log();

    XLOG << "[队伍-成员初始化] 初始化成功" << XEND;
#ifdef _LX_DEBUG
    XLOG << "[组队-加载队员]" << getGUID() << getName() << getZoneID() << XEND;
#endif
    return true;
  }
  else
  {
  }

  XERR << "[队伍-成员初始化] 初始化失败" << XEND;
  return false;
}

void TMember::load(const TeamMember& rData)
{
  m_oGCharData.setCharID(rData.guid());
  for (int i = 0; i < rData.datas_size(); ++i)
  {
    if (rData.datas(i).type() == EMEMBERDATA_JOB)
      m_eJob = static_cast<ETeamJob>(rData.datas(i).value());
    else if (rData.datas(i).type() == EMEMBERDATA_CAT)
      m_dwCatID = rData.datas(i).value();
    else if (rData.datas(i).type() == EMEMBERDATA_BASELEVEL)
      m_oGCharData.setBaseLevel(rData.datas(i).value());
    else if (rData.datas(i).type() == EMEMBERDATA_RELIVETIME)
      m_dwCatReliveTime = rData.datas(i).value();
    else if (rData.datas(i).type() == EMEMBERDATA_EXPIRETIME)
      m_dwCatExpireTime = rData.datas(i).value();
    else if (rData.datas(i).type() == EMEMBERDATA_CAT_OWNER)
      m_qwCatOwnerID = rData.datas(i).value();
    else if (rData.datas(i).type() == EMEMBERDATA_ENTERTIME)
      m_dwEnterTime = rData.datas(i).value();
  }
  if (m_dwCatID != 0)
    return;

  if (!initData())
  {
    m_oGCharData.setCharID(rData.guid());
    m_oGCharData.setName(rData.name());
    m_oGCharData.setAccID(rData.accid());

    for (int i = 0; i < rData.datas_size(); ++i)
    {
      const MemberData& data = rData.datas(i);

      if (data.type() == EMEMBERDATA_BASELEVEL)
        m_oGCharData.setBaseLevel(data.value());
      else if (data.type() == EMEMBERDATA_MAPID)
        m_oGCharData.setMapID(data.value());
      else if (data.type() == EMEMBERDATA_PORTRAIT)
        m_oGCharData.setPortrait(data.value());
      else if (data.type() == EMEMBERDATA_BODY)
        m_oGCharData.setBody(data.value());
      else if (data.type() == EMEMBERDATA_HEAD)
        m_oGCharData.setHead(data.value());
      else if (data.type() == EMEMBERDATA_FACE)
        m_oGCharData.setFace(data.value());
      else if (data.type() == EMEMBERDATA_BACK)
        m_oGCharData.setBack(data.value());
      else if (data.type() == EMEMBERDATA_TAIL)
        m_oGCharData.setTail(data.value());
      else if (data.type() == EMEMBERDATA_HAIR)
        m_oGCharData.setHair(data.value());
      else if (data.type() == EMEMBERDATA_HAIRCOLOR)
        m_oGCharData.setHairColor(data.value());
      else if (data.type() == EMEMBERDATA_CLOTHCOLOR)
        m_oGCharData.setClothColor(data.value());
      else if (data.type() == EMEMBERDATA_LEFTHAND)
        m_oGCharData.setLeftHand(data.value());
      else if (data.type() == EMEMBERDATA_RIGHTHAND)
        m_oGCharData.setRightHand(data.value());
      /*else if (data.type() == EMEMBERDATA_FRAME)
        m_dwFrame = data.value();*/
      else if (data.type() == EMEMBERDATA_PROFESSION)
        m_oGCharData.setProfession(static_cast<EProfession>(data.value()));
      else if (data.type() == EMEMBERDATA_GENDER)
        m_oGCharData.setGender(static_cast<EGender>(data.value()));
      else if (data.type() == EMEMBERDATA_BLINK)
        m_oGCharData.setBlink(data.value() == 1);
      else if (data.type() == EMEMBERDATA_ZONEID)
        m_oGCharData.setZoneID(data.value());
      else if (data.type() == EMEMBERDATA_OFFLINE)
        m_oGCharData.setOfflineTime(data.value());
      else if (data.type() == EMEMBERDATA_CAT)
        m_dwCatID = data.value();
      else
        XERR << "[队伍-成员加载]" << m_oGCharData.getAccID() << m_oGCharData.getCharID() << "type :" << data.type() << "value :" << data.value() << "没有被处理" << XEND;
    }
  }
}

void TMember::toData(TeamMember* pData, bool bClient /*= false*/)
{
  if (pData == nullptr)
    return;

  pData->set_guid(m_oGCharData.getCharID());
  pData->set_name(m_oGCharData.getName());
  pData->set_accid(m_oGCharData.getAccID());

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BASELEVEL, m_oGCharData.getBaseLevel());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_PROFESSION, m_oGCharData.getProfession());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MAPID, m_oGCharData.getMapID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_PORTRAIT, m_oGCharData.getPortrait());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_RAIDID, m_dwRaidID);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BODY, m_oGCharData.getBody());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HEAD, m_oGCharData.getHead());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_FACE, m_oGCharData.getFace());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BACK, m_oGCharData.getBack());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_TAIL, m_oGCharData.getTail());

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HAIR, m_oGCharData.getHair());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HAIRCOLOR, m_oGCharData.getHairColor());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_CLOTHCOLOR, m_oGCharData.getClothColor());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_LEFTHAND, m_oGCharData.getLeftHand());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_RIGHTHAND, m_oGCharData.getRightHand());

  bool online = false;
  if (m_dwCatID != 0)
    online = !TeamManager::getMe().isOnline(getCatOwnerID());
  else
    online = !TeamManager::getMe().isOnline(m_oGCharData.getCharID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_OFFLINE, online);

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_JOB, m_eJob);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_TARGETID, m_qwTargetID);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GENDER, m_oGCharData.getGender());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BLINK, m_oGCharData.getBlink());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_ZONEID, bClient ? getClientZoneID(m_oGCharData.getZoneID()) : m_oGCharData.getZoneID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_AUTOFOLLOW, m_bAutoFollow);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_EYE, m_oGCharData.getEye());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MOUTH, m_oGCharData.getMouth());

  // follow no save
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_JOINHANDID, m_qwHandID);

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GUILDID, m_oGCharData.getGuildID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GUILDNAME, 0, m_oGCharData.getGuildName());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_CAT, getCatID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_CAT_OWNER, getCatOwnerID());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_RELIVETIME, getReliveTime());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_EXPIRETIME, getExpireTime());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_ENTERTIME, m_dwEnterTime);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GUILDRAIDINDEX, m_dwGuildRaidIndex);

  if (bClient)
    toEnsembleSkillMemberData(pData->add_datas());
}

void TMember::toData(TeamApply* pData)
{
  if (pData == nullptr)
    return;

  pData->set_guid(m_oGCharData.getCharID());
  pData->set_zoneid(m_oGCharData.getZoneID());
  pData->set_name(m_oGCharData.getName());
  pData->set_accid(m_oGCharData.getAccID());

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BASELEVEL, m_oGCharData.getBaseLevel());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_PROFESSION, m_oGCharData.getProfession());
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MAPID, m_dwMapID);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_PORTRAIT, m_oGCharData.getPortrait());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BODY, m_oGCharData.getBody());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HEAD, m_oGCharData.getHead());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_FACE, m_oGCharData.getFace());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BACK, m_oGCharData.getBack());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_TAIL, m_oGCharData.getTail());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MOUTH, m_oGCharData.getMouth());

  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HAIR, m_oGCharData.getHair());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HAIRCOLOR, m_oGCharData.getHairColor());
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_CLOTHCOLOR, m_dwClothColor);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_LEFTHAND, m_dwLeftHand);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_RIGHTHAND, m_dwRightHand);

  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_FRAME, m_dwFrame);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_RAIDID, m_dwRaidid);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_OFFLINE, isOnline() == false);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_HP, m_dwHp);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MAXHP, m_dwMaxHp);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_SP, m_dwSp);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_MAXSP, m_dwMaxSp);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_JOB, m_eJob);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_TARGETID, m_qwTargetID);
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GENDER, m_oGCharData.getGender());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_BLINK, m_oGCharData.getBlink());
  GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_EYE, m_oGCharData.getEye());

  // follow no save
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_JOINHANDID, m_qwHandID);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GUILDID, m_qwGuildID);
  //GTeam::add_mdata(pData->add_datas(), EMEMBERDATA_GUILDNAME, 0, m_strGuildName);
}

void TMember::toData(TeamMemberInfo* pInfo)
{
  if (pInfo == nullptr)
    return;

  pInfo->set_charid(getGUID());
  pInfo->set_catid(getCatID());
  pInfo->set_mapid(getMapID());
  //pInfo->set_raidid(getRaidID());
  pInfo->set_zoneid(getZoneID());
  pInfo->set_gender(getGender());
  pInfo->set_name(getName());
  pInfo->set_guildraidindex(m_dwGuildRaidIndex);
  pInfo->set_level(getBaseLv());

  bool online = false;
  if (m_dwCatID != 0)
    online = TeamManager::getMe().isOnline(getCatOwnerID());
  else
    online = TeamManager::getMe().isOnline(getGUID());
  pInfo->set_online(online);
}

void TMember::updateData(const UserInfo& rInfo, bool bInit /*= false*/)
{
  if (m_oGCharData.getCharID() == 0)
    m_oGCharData.setCharID(rInfo.user().charid());
  //m_oGCharData.setName(rInfo.user().name());

  //bool bBroadcast = false;
  const SocialUser& rUser = rInfo.user();
  for (int i = 0; i < rInfo.datas_size(); ++i)
  {
    const UserData& rData = rInfo.datas(i);
    switch (rData.type())
    {
      case EUSERDATATYPE_MAPID:
        m_oGCharData.setMapID(rData.value());
        m_bitset.set(EMEMBERDATA_MAPID);
        break;
      case EUSERDATATYPE_RAIDID:
        m_dwRaidID = rData.value();
        m_bitset.set(EMEMBERDATA_RAIDID);
        break;
      case EUSERDATATYPE_ROLELEVEL:
        {
          m_oGCharData.setBaseLevel(rData.value());
          m_bitset.set(EMEMBERDATA_BASELEVEL);
        }
        break;
      case EUSERDATATYPE_PORTRAIT:
        {
          m_oGCharData.setPortrait(rData.value());
          m_bitset.set(EMEMBERDATA_PORTRAIT);
        }
        break;
      case EUSERDATATYPE_BODY:
        {
          m_oGCharData.setBody(rData.value());
          m_bitset.set(EMEMBERDATA_BODY);
        }
        break;
      case EUSERDATATYPE_HEAD:
        {
          m_oGCharData.setHead(rData.value());
          m_bitset.set(EMEMBERDATA_HEAD);
        }
        break;
      case EUSERDATATYPE_FACE:
        {
          m_oGCharData.setFace(rData.value());
          m_bitset.set(EMEMBERDATA_FACE);
        }
        break;
      case EUSERDATATYPE_BACK:
        {
          m_oGCharData.setBack(rData.value());
          m_bitset.set(EMEMBERDATA_BACK);
        }
        break;
      case EUSERDATATYPE_TAIL:
        {
          m_oGCharData.setTail(rData.value());
          m_bitset.set(EMEMBERDATA_TAIL);
        }
        break;
      case EUSERDATATYPE_HAIR:
        {
          m_oGCharData.setHair(rData.value());
          m_bitset.set(EMEMBERDATA_HAIR);
        }
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        {
          m_oGCharData.setHairColor(rData.value());
          m_bitset.set(EMEMBERDATA_HAIRCOLOR);
        }
        break;
      case EUSERDATATYPE_CLOTHCOLOR:
        {
          m_oGCharData.setClothColor(rData.value());
          m_bitset.set(EMEMBERDATA_CLOTHCOLOR);
        }
        break;
      case EUSERDATATYPE_LEFTHAND:
        {
          m_oGCharData.setLeftHand(rData.value());
          m_bitset.set(EMEMBERDATA_LEFTHAND);
        }
        break;
      case EUSERDATATYPE_RIGHTHAND:
        {
          m_oGCharData.setRightHand(rData.value());
          m_bitset.set(EMEMBERDATA_RIGHTHAND);
        }
        break;
      /*case EUSERDATATYPE_FRAME:
        m_dwFrame = rData.value();
        m_bitset.set(EMEMBERDATA_FRAME);
        break;*/
      case EUSERDATATYPE_PROFESSION:
        {
          m_oGCharData.setProfession(static_cast<EProfession>(rData.value()));
          m_bitset.set(EMEMBERDATA_PROFESSION);
        }
        break;
      case EUSERDATATYPE_SEX:
        {
          m_oGCharData.setGender(static_cast<EGender>(rData.value()));
          m_bitset.set(EMEMBERDATA_GENDER);
        }
        break;
      case EUSERDATATYPE_BLINK:
        {
          m_oGCharData.setBlink(rData.value());
          m_bitset.set(EMEMBERDATA_BLINK);
        }
        break;
      case EUSERDATATYPE_HANDID:
        {
          m_qwHandID = rData.value();
          m_bitset.set(EMEMBERDATA_JOINHANDID);
        }
        break;
      case EUSERDATATYPE_EYE:
        {
          m_oGCharData.setEye(rData.value());
          m_bitset.set(EMEMBERDATA_EYE);
        }
        break;
      case EUSERDATATYPE_NAME:
        {
          m_oGCharData.setName(rData.data());
          m_bitset.set(EMEMBERDATA_NAME);
        }
        break;
      case EUSERDATATYPE_MOUTH:
        {
          m_oGCharData.setMouth(rData.value());
          m_bitset.set(EMEMBERDATA_MOUTH);
        }
        break;
      case EUSERDATATYPE_GUILDRAIDINDEX:
        {
          m_dwGuildRaidIndex = rData.value();
          m_bitset.set(EMEMBERDATA_GUILDRAIDINDEX);
        }
        break;
      case EUSERDATATYPE_ENSEMBLESKILL:
        {
          setEnsembleSkill(rData.data());
        }
        break;
      default:
        XDBG << "[队伍-成员数据同步]"<<rUser.accid()<<rUser.charid()<<rUser.profession() << rUser.name() << "收到数据 type :" << rData.type() << "value :" << rData.value() << "未处理" << XEND;
        break;
    }
    XDBG << "[队伍-成员数据同步]" << rUser.accid() << rUser.charid() << rUser.profession() << rUser.name() << "收到数据 type :" << rData.type() << "value :" << rData.value() << XEND;
  }

  // attr
  /*for (int i = 0; i < rInfo.attrs_size(); ++i)
  {
    const UserAttr& rAttr = rInfo.attrs(i);
    switch (rAttr.type())
    {
      case EATTRTYPE_HP:
        m_dwHp = rAttr.value();
        m_bitset.set(EMEMBERDATA_HP);
        break;
      case EATTRTYPE_MAXHP:
        m_dwMaxHp = rAttr.value();
        m_bitset.set(EMEMBERDATA_MAXHP);
        break;
      case EATTRTYPE_SP:
        m_dwSp = rAttr.value();
        m_bitset.set(EMEMBERDATA_SP);
        break;
      case EATTRTYPE_MAXSP:
        m_dwMaxSp = rAttr.value();
        m_bitset.set(EMEMBERDATA_MAXSP);
        break;
      default:
        break;
    }
  }*/

  if (bInit)
    m_bitset.reset();
  else
    updateData();
}

bool TMember::sendCmdToMe(const void* buf, WORD len) const
{
  if (buf == nullptr || len == 0)
    return false;

  if (isOnline() == false)
    return false;

  return thisServer->sendCmdToMe(getZoneID(), getGUID(), buf, len);
}

void TMember::updateSocialData(const SocialListUpdateSocialCmd& cmd)
{
  // update
  for (int i = 0; i < cmd.updates_size(); ++i)
  {
    const SocialItem& rItem = cmd.updates(i);
    m_oGCharData.updateRelation(rItem.charid(), rItem.relation());
  }

  // dels
  for (int i = 0; i < cmd.dels_size(); ++i)
  {
    if (m_oGCharData.delRelation(cmd.dels(i)) == false)
    {
      XERR << "[队伍成员-社交更新]" << m_oGCharData.getAccID() << m_oGCharData.getCharID() << m_oGCharData.getProfession() << m_oGCharData.getName()
        << "删除了一个不存在的 charid :" << cmd.dels(i) << "关系" << XEND;
    }
  }

#ifdef _DEBUG
  m_oGCharData.debug_log();
#endif
}

void TMember::updateData()
{
  if (m_pTeam == nullptr || m_bitset.any() == false)
    return;

  MemberDataUpdate cmd;
  MemberDataUpdateTeamCmd scmd;
  PvpMemberDataUpdateSCmd pvpcmd;
  MatchTeamMemDataUpdateInfo* pPvpInfo = pvpcmd.mutable_data();
  cmd.set_id(getGUID());
  scmd.set_updatecharid(getGUID());

  for (int i = 0; i < EMEMBERDATA_MAX; ++i)
  {
    if (m_bitset.test(i) == false)
      continue;

    EMemberData eType = static_cast<EMemberData>(i);
    switch (eType)
    {
      case EMEMBERDATA_MIN:
        break;
      case EMEMBERDATA_BASELEVEL:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getBaseLevel());
        GTeam::add_mdata(scmd.add_updates(), eType, m_oGCharData.getBaseLevel());
        break;
      case EMEMBERDATA_PROFESSION:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getProfession());
        break;
      case EMEMBERDATA_MAPID:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getMapID());
        GTeam::add_mdata(scmd.add_updates(), eType, m_oGCharData.getMapID());
        break;
      case EMEMBERDATA_PORTRAIT:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getPortrait());
        break;
      /*case EMEMBERDATA_FRAME:
        pData->set_value(m_dwFrame);
        break;*/
      case EMEMBERDATA_RAIDID:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwRaidID);
        GTeam::add_mdata(scmd.add_updates(), eType, m_dwRaidID);
        break;
      case EMEMBERDATA_OFFLINE:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwCatID != 0 ? 10 : isOnline() == false);
        if (pPvpInfo && m_pTeam->isInPvpRoom() && m_dwCatID == 0)
        {
          GTeam::add_mdata(pPvpInfo->add_members(), eType, m_dwCatID != 0 ? 10 : isOnline() == false);
        }
        break;
      case EMEMBERDATA_JOB:
        GTeam::add_mdata(cmd.add_members(), eType, m_eJob);
        GTeam::add_mdata(scmd.add_updates(), eType, m_eJob);
        if (pPvpInfo && m_pTeam->isInPvpRoom() && m_dwCatID == 0)
        {
          GTeam::add_mdata(pPvpInfo->add_members(), eType, m_eJob);
        }
        break;
      case EMEMBERDATA_TARGETID:
        GTeam::add_mdata(cmd.add_members(), eType, m_qwTargetID);
        break;
      case EMEMBERDATA_JOINHANDID:
        GTeam::add_mdata(cmd.add_members(), eType, m_qwHandID);
        break;
      case EMEMBERDATA_BODY:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getBody());
        break;
      case EMEMBERDATA_HEAD:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getHead());
        break;
      case EMEMBERDATA_BACK:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getBack());
        break;
      case EMEMBERDATA_FACE:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getFace());
        break;
      case EMEMBERDATA_TAIL:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getTail());
        break;
      case EMEMBERDATA_HAIR:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getHair());
        break;
      case EMEMBERDATA_HAIRCOLOR:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getHairColor());
        break;
      case EMEMBERDATA_CLOTHCOLOR:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getClothColor());
        break;
      case EMEMBERDATA_LEFTHAND:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getLeftHand());
        break;
      case EMEMBERDATA_RIGHTHAND:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getRightHand());
        break;
      case EMEMBERDATA_GUILDID:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getGuildID());
        break;
      case EMEMBERDATA_GUILDNAME:
        GTeam::add_mdata(cmd.add_members(), eType, 0, m_oGCharData.getGuildName());
        break;
      case EMEMBERDATA_GENDER:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getGender());
        break;
      case EMEMBERDATA_BLINK:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getBlink());
        break;
      case EMEMBERDATA_ZONEID:
        GTeam::add_mdata(cmd.add_members(), eType, getClientZoneID(m_oGCharData.getZoneID()));
        GTeam::add_mdata(scmd.add_updates(), eType, m_oGCharData.getZoneID());
        break;
      case EMEMBERDATA_AUTOFOLLOW:
        GTeam::add_mdata(cmd.add_members(), eType, m_bAutoFollow);
        break;
      case EMEMBERDATA_EYE:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getEye());
        break;
      case EMEMBERDATA_NAME:
        GTeam::add_mdata(cmd.add_members(), eType, 0, m_oGCharData.getName());
        break;
      case EMEMBERDATA_MOUTH:
        GTeam::add_mdata(cmd.add_members(), eType, m_oGCharData.getMouth());
        break;
      /*case EMEMBERDATA_HP:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwHp);
        break;
      case EMEMBERDATA_MAXHP:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwMaxHp);
        break;
      case EMEMBERDATA_SP:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwSp);
        break;
      case EMEMBERDATA_MAXSP:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwMaxSp);
        break;*/
      case EMEMBERDATA_RELIVETIME:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwCatReliveTime);
        break;
      case EMEMBERDATA_EXPIRETIME:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwCatExpireTime);
        break;
      case EMEMBERDATA_CAT_OWNER:
        GTeam::add_mdata(cmd.add_members(), eType, m_qwCatOwnerID);
        break;
      case EMEMBERDATA_ENTERTIME:
        GTeam::add_mdata(cmd.add_members(), eType, m_dwEnterTime);
        break;
      case EMEMBERDATA_GUILDRAIDINDEX:
        GTeam::add_mdata(scmd.add_updates(), eType, m_dwGuildRaidIndex);
        GTeam::add_mdata(cmd.add_members(), eType, m_dwGuildRaidIndex);
        break;
      case EMEMBERDATA_ENSEMBLESKILL:
        toEnsembleSkillMemberData(cmd.add_members());
        break;
      case EMEMBERDATA_MAX:
        break;
      default:
        break;
    }
  }

  if (cmd.members_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pTeam->broadcastCmd(send, len);

#ifdef _DEBUG
    for (int i = 0; i < cmd.members_size(); ++i)
    {
      const MemberData& rData = cmd.members(i);
      XDBG << "[队伍成员-数据前端更新]" << m_oGCharData.getCharID() << m_oGCharData.getName() << "type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << XEND;
    }
#endif

    // update cat
    if (m_dwCatID == 0)
    {
      const TMapTeamMember& mapMember = m_pTeam->getTeamMembers();
      for (auto &m : mapMember)
      {
        if (m.second->getCatID() == 0 || m.second->getCatOwnerID() != m_oGCharData.getCharID())
          continue;

        MemberDataUpdate ccmd;
        ccmd.set_id(m.first);
        for (int i = 0; i < cmd.members_size(); ++i)
        {
          const MemberData& rData = cmd.members(i);
          if (catSync(rData.type()) == true)
          {
            MemberData* pData = ccmd.add_members();
            if (pData == nullptr)
            {
              XERR << "[队伍成员-数据更新]" << m.first << m.second->getName() << "type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << "失败" << XEND;
              continue;
            }
            pData->CopyFrom(cmd.members(i));
          }
        }
        if (ccmd.members_size() > 0)
        {
          PROTOBUF(ccmd, csend, clen);
          m_pTeam->broadcastCmd(csend, clen);
        }
      }
    }
  }
  if (scmd.updates_size() > 0)
  {
    const TMapTeamMember& mapList = m_pTeam->getTeamMembers();
    for (auto &m : mapList)
    {
      if (m.second->isOnline() == true)
      {
        scmd.set_charid(m.first);
        PROTOBUF(scmd, ssend, slen);
        thisServer->sendCmdToSession(ssend, slen);
      }
    }

#ifdef _DEBUG
    for (int i = 0; i < scmd.updates_size(); ++i)
    {
      const MemberData& rData = scmd.updates(i);
      XDBG << "[队伍成员-数据前端更新]" << m_oGCharData.getCharID() << m_oGCharData.getName() << "type :" << rData.type() << "value :" << rData.value() << "data :" << rData.data() << XEND;
    }
#endif
  }

  if (pPvpInfo->members_size() > 0)
  {
    pPvpInfo->set_teamid(m_pTeam->getGUID());
    pPvpInfo->set_roomid(m_pTeam->getPvpRoomId());
    pPvpInfo->set_zoneid(thisServer->getZoneID());
    pPvpInfo->set_charid(getGUID());
    PROTOBUF(pvpcmd, send, len);
    bool ret = thisServer->forwardCmdToMatch(send, len);
    XDBG << "[斗技场-同步队员变化消息到Matchserver] teamid" << m_pTeam->getGUID() << "roomid" << m_pTeam->getPvpRoomId() << "ret" << ret << "msg" << cmd.ShortDebugString() << XEND;
  }

  m_bitset.reset();
}

bool TMember::isOnline() const
{
  return TeamManager::getMe().isOnline(getGUID());
}

void TMember::syncBeLeader()
{
  if(isLeader() == true)
  {
    BeLeaderTeamCmd cmd;
    cmd.set_charid(getGUID());
    cmd.set_teamjob(m_eJob);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }
}

void TMember::sendRealtimeVoiceID()
{
  if (m_pTeam == nullptr)
    return;
  QueryRealtimeVoiceIDCmd cmd;
  cmd.set_channel(ECHAT_CHANNEL_TEAM);
  cmd.set_id(m_pTeam->getRealtimeVoiceID());
  PROTOBUF(cmd, send, len);
  sendCmdToMe(send, len);
}

void TMember::setEnsembleSkill(const string& data)
{
  m_setEnsembleSkill.clear();
  if (data.empty() == false)
  {
    vector<string> ids;
    stringTok(data, ",", ids);
    for (auto& id : ids)
    {
      DWORD sid = atoi(id.c_str());
      if (sid)
        m_setEnsembleSkill.insert(sid);
    }
  }

  m_bitset.set(EMEMBERDATA_ENSEMBLESKILL);
  updateData();
}

void TMember::toEnsembleSkillMemberData(MemberData* pData)
{
  if (pData == nullptr)
    return;
  GTeam::add_mdata(pData, EMEMBERDATA_ENSEMBLESKILL, 0);
  for (auto id : m_setEnsembleSkill)
    pData->add_values(id);
}

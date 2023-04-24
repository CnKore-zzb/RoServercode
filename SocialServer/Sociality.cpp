#include "Sociality.h"
#include "SocialServer.h"
#include "RecordCmd.pb.h"
#include "PlatLogManager.h"
#include "SocialCmd.pb.h"
#include "MiscConfig.h"
#include "UserCmd.h"
#include "TeamCmd.pb.h"
#include "UserConfig.h"
#include "CommonConfig.h"
#include "GSocial.h"
#include "RedisManager.h"

// sociality
Sociality::Sociality() : m_oGCharData(thisServer->getRegionID(), 0)
{
}

Sociality::~Sociality()
{

}

bool Sociality::fromData(const xRecord& rRecord)
{
  m_dwRelation = rRecord.get<DWORD>("relation");
  if (m_dwRelation == 0 || m_dwRelation == ESOCIALRELATION_MIN)
  {
    m_dwRelation = 0;
    return false;
  }

  /*if (!m_oGCharData.get())
  {
    XERR << "[社交-加载对象] charid :" << m_oGCharData.getCharID() << "未正确获取gchar" << XEND;
    return false;
  }*/

  m_oGCharData.setOfflineTime(m_oGCharData.getOnlineTime() > m_oGCharData.getOfflineTime() ? 0 : m_oGCharData.getOfflineTime());
  GSocial::parseRelationTime(rRecord.getString("relationtime"), m_mapRelationTime);

  /*stringstream sstr(rRecord.getString("relationtime"));
  ESocialRelation eRelation = ESOCIALRELATION_MIN;
  DWORD dwRelation = 0;
  DWORD dwTime = 0;
  while (sstr >> dwRelation)
  {
    if (dwRelation == 0)
      break;
    if (dwRelation <= ESOCIALRELATION_MIN || dwRelation >= ESOCIALRELATION_MAX)
      continue;

    eRelation = static_cast<ESocialRelation>(dwRelation);

    sstr >> dwTime;
    m_mapRelationTime[eRelation] = dwTime;
  }*/

  return true;
}

bool Sociality::toData(xRecord& rRecord, QWORD qwID)
{
  rRecord.put("id", qwID);
  rRecord.put("destid", m_oGCharData.getCharID());
  rRecord.put("relation", getRelation());

  string str;
  GSocial::serialRelationTime(m_mapRelationTime, str);
  /*ostringstream ostr;
  for (auto m = m_mapRelationTime.begin(); m != m_mapRelationTime.end(); ++m)
    ostr << m->first << " " << m->second << " ";*/
  rRecord.putString("relationtime", str);

  return true;
}

bool Sociality::toData(SocialData* pData)
{
  if (pData == nullptr)
    return false;

  xTime frameDebug;
  if (!m_bInit)
  {
    EError eError = m_oGCharData.getBySocial();
    if (eError != EERROR_SUCCESS)
    {
      XERR << "[社交-加载对象] charid :" << m_oGCharData.getCharID() << "未正确获取gchar, error :" << eError << XEND;
      return eError != EERROR_NOT_EXIST;
    }
  }
  m_bInit = true;
  XDBG << "[社交-加载对象] charid :" << m_oGCharData.getCharID() << "耗时" << frameDebug.uElapse() << "微秒" << "recall :" << isRecall() << "berecall" << canBeRecall() << XEND;

  pData->set_guid(m_oGCharData.getCharID());
  pData->set_accid(m_oGCharData.getAccID());

  pData->set_level(m_oGCharData.getBaseLevel());
  pData->set_portrait(m_oGCharData.getPortrait());
  pData->set_frame(m_oGCharData.getFrame());
  pData->set_hair(m_oGCharData.getHair());
  pData->set_haircolor(m_oGCharData.getHairColor());
  pData->set_body(m_oGCharData.getBody());
  pData->set_head(m_oGCharData.getHead());
  pData->set_face(m_oGCharData.getFace());
  pData->set_mouth(m_oGCharData.getMouth());
  pData->set_profic(m_oGCharData.getProfic());
  pData->set_eye(m_oGCharData.getEye());
  pData->set_offlinetime(m_oGCharData.getOfflineTime());
  pData->set_relation(m_dwRelation);

  pData->set_adventurelv(m_oGCharData.getManualLv());
  pData->set_adventureexp(m_oGCharData.getManualExp());
  pData->set_appellation(m_oGCharData.getTitleID());

  pData->set_mapid(m_oGCharData.getMapID());
  pData->set_zoneid(getClientZoneID(m_oGCharData.getZoneID()));
  pData->set_blink(m_oGCharData.getBlink());
  pData->set_recall(isRecall());
  pData->set_canrecall(canBeRecall());

  pData->set_profession(m_oGCharData.getProfession());
  pData->set_gender(m_oGCharData.getGender());

  pData->set_name(m_oGCharData.getName());
  pData->set_guildname(m_oGCharData.getGuildName());
  pData->set_guildportrait(m_oGCharData.getGuildPortrait());

  pData->set_createtime(getCreateTime());

  m_bitmark.reset();
  return true;
}

bool Sociality::toData(SocialItem* pItem)
{
  if (pItem == nullptr)
    return false;
  pItem->set_charid(getGUID());
  pItem->set_relation(getRelation());
  ostringstream ostr;
  for (auto m = m_mapRelationTime.begin(); m != m_mapRelationTime.end(); ++m)
     ostr << m->first << " " << m->second << " ";
  pItem->set_createtime(ostr.str());
  return true;
}

bool Sociality::isRecall()
{
  return m_oGCharData.getRelationCount(ESOCIALRELATION_BERECALL) >= 1;
}

bool Sociality::canBeRecall()
{
  if (m_oGCharData.getRelationCount(ESOCIALRELATION_BERECALL) >= 1)
    return false;
  const SRecallCFG& rCFG = MiscConfig::getMe().getRecallCFG();
  if (xTime::getCurSec() - m_oGCharData.getOfflineTime() < rCFG.dwNeedOffline)
    return false;
  if (m_oGCharData.getBaseLevel() < rCFG.dwNeedBaseLv)
    return false;
  return true;
}

const string& Sociality::getCreateTime()
{
  static const set<ESocialRelation> setRelation = set<ESocialRelation>{
      ESOCIALRELATION_FRIEND,
      ESOCIALRELATION_MERRY,
      ESOCIALRELATION_CHAT,
      ESOCIALRELATION_TEAM,
      ESOCIALRELATION_APPLY,
      ESOCIALRELATION_BLACK,
      ESOCIALRELATION_BLACK_FOREVER,
      ESOCIALRELATION_TUTOR,
      ESOCIALRELATION_TUTOR_APPLY,
      ESOCIALRELATION_STUDENT,
      ESOCIALRELATION_STUDENT_APPLY,
      ESOCIALRELATION_STUDENT_RECENT,
      ESOCIALRELATION_TUTOR_PUNISH,
      ESOCIALRELATION_TUTOR_CLASSMATE,
      ESOCIALRELATION_RECALL,
      ESOCIALRELATION_BERECALL,
  };
  stringstream sstr;
  for (auto &s : setRelation)
    sstr << getRelationTime(s) << ":";
  m_strCreateTime = sstr.str();
  return m_strCreateTime;
}

bool Sociality::addRelation(ESocialRelation eRelation)
{
  if (checkRelation(eRelation) == true)
    return false;

  m_dwRelation |= eRelation;
  m_bitmark.set(ESOCIALDATA_RELATION);
  m_bitmark.set(ESOCIALDATA_CREATETIME);
  m_mapRelationTime[eRelation] = xTime::getCurSec();
  return true;
}

bool Sociality::removeRelation(ESocialRelation eRelation)
{
  if (checkRelation(eRelation) == false)
    return false;

  m_dwRelation &= (~eRelation);
  m_bitmark.set(ESOCIALDATA_RELATION);
  m_bitmark.set(ESOCIALDATA_CREATETIME);
  m_mapRelationTime.erase(eRelation);
  return true;
}

DWORD Sociality::getRelationTime(ESocialRelation eRelation) const
{
  auto m = m_mapRelationTime.find(eRelation);
  if (m != m_mapRelationTime.end())
    return m->second;
  return 0;
}

void Sociality::fetchChangeData(SocialDataUpdate& cmd)
{
  if (m_bitmark.any() == false)
    return;

  auto add_data = [&](ESocialData eType, DWORD dwValue, const string& data = "")
  {
    SocialDataItem* pItem = cmd.add_items();
    if (pItem == nullptr)
      return;

    pItem->set_type(eType);
    pItem->set_value(dwValue);
    pItem->set_data(data);
  };

  for (int i = ESOCIALDATA_MIN + 1; i < ESOCIALDATA_MAX; ++i)
  {
    if (m_bitmark.test(i) == false)
      continue;

    ESocialData eType = static_cast<ESocialData>(i);
    switch(eType)
    {
      case ESOCIALDATA_MIN:
        break;
      case ESOCIALDATA_LEVEL:
        add_data(eType, getLevel());
        break;
      case ESOCIALDATA_OFFLINETIME:
        add_data(eType, getOfflineTime());
        break;
      case ESOCIALDATA_RELATION:
        add_data(eType, getRelation());
        break;
      case ESOCIALDATA_PROFESSION:
        add_data(eType, getProfession());
        break;
      case ESOCIALDATA_PORTRAIT:
        add_data(eType, getPortrait());
        break;
      case ESOCIALDATA_FRAME:
        add_data(eType, getFrame());
        break;
      case ESOCIALDATA_HAIR:
        add_data(eType, getHair());
        break;
      case ESOCIALDATA_HAIRCOLOR:
        add_data(eType, getHairColor());
        break;
      case ESOCIALDATA_BODY:
        add_data(eType, getBody());
        break;
      case ESOCIALDATA_ADVENTURELV:
        add_data(eType, getAdventureLv());
        break;
      case ESOCIALDATA_ADVENTUREEXP:
        add_data(eType, getAdventureExp());
        break;
      case ESOCIALDATA_APPELLATION:
        add_data(eType, getAppellation());
        break;
      case ESOCIALDATA_GENDER:
        add_data(eType, getGender());
        break;
      case ESOCIALDATA_GUILDNAME:
        add_data(eType, 0, getGuildName());
        break;
      case ESOCIALDATA_GUILDPORTRAIT:
        add_data(eType, 0, getGuildPortrait());
        break;
      /*case ESOCIALDATA_MAP:
        add_data(eType, getMapID());
        break;*/
      case ESOCIALDATA_ZONEID:
        add_data(eType, getClientZoneID(getZoneID()));
        break;
      case ESOCIALDATA_BLINK:
        add_data(eType, getBlink());
        break;
      case ESOCIALDATA_NAME:
        add_data(eType, 0, getName());
        break;
      case ESOCIALDATA_CREATETIME:
        add_data(eType, 0, getCreateTime());
        break;
      case ESOCIALDATA_HEAD:
        add_data(eType, getHead());
        break;
      case ESOCIALDATA_FACE:
        add_data(eType, getFace());
        break;
      case ESOCIALDATA_MOUTH:
        add_data(eType, getMouth());
        break;
      case ESOCIALDATA_EYE:
        add_data(eType, getEye());
        break;
      case ESOCIALDATA_TUTOR_PROFIC:
        add_data(eType, getProfic());
        break;
      case ESOCIALDATA_RECALL:
        add_data(eType, isRecall());
        break;
      case ESOCIALDATA_CANRECALL:
        add_data(eType, canBeRecall());
        break;
      case ESOCIALDATA_MAX:
        break;
    }
  }

  m_bitmark.reset();
}


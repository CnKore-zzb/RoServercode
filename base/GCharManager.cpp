#include "GCharManager.h"
#include "RedisManager.h"
#include "xDBConnPool.h"
#include "xServer.h"
#include "RecordCmd.pb.h"
#include "CommonConfig.h"
#include "config/UserConfig.h"

extern xServer* thisServer;

// GChar
GChar::GChar(DWORD dwRegionID, QWORD charid):m_qwCharID(charid),m_dwRegionID(dwRegionID)
{
  m_strRedisKey.clear();
  m_strRedisKey = RedisManager::getMe().getKeyByParam(dwRegionID, EREDISKEYTYPE_GLOBAL_CHAR_DATA, m_qwCharID);
}

GChar& GChar::operator=(const GChar& rChar)
{
  m_qwCharID = rChar.m_qwCharID;

  m_mapSocial = rChar.m_mapSocial;

  m_oData = rChar.m_oData;
  m_strRedisKey = rChar.m_strRedisKey;

  m_dwRegionID = rChar.m_dwRegionID;

  return *this;
}

GChar::~GChar()
{
}

void GChar::setCharID(QWORD qwCharID)
{
  m_qwCharID = qwCharID;
  m_strRedisKey.clear();
  m_strRedisKey = RedisManager::getMe().getKeyByParam(m_dwRegionID, EREDISKEYTYPE_GLOBAL_CHAR_DATA, m_qwCharID);
}

bool GChar::checkRelation(QWORD qwCharID, DWORD dwRelation)
{
  const TMapSocial& mapSocial = getSocial();
  auto m = mapSocial.find(qwCharID);
  if (m == mapSocial.end())
    return false;
  return (m->second & dwRelation) != 0;
}

bool GChar::updateRelation(QWORD qwCharID, DWORD dwRelation)
{
  auto m = m_mapSocial.find(qwCharID);
  if (m == m_mapSocial.end())
  {
    m_mapSocial[qwCharID] = dwRelation;
    return true;
  }

  m->second = dwRelation;
  return true;
}

bool GChar::addRelation(QWORD qwCharID, DWORD dwRelation)
{
  auto m = m_mapSocial.find(qwCharID);
  if (m == m_mapSocial.end())
  {
    m_mapSocial[qwCharID] = dwRelation;
    return true;
  }

  m->second |= dwRelation;
  return true;
}

bool GChar::delRelation(QWORD qwCharID)
{
  auto m = m_mapSocial.find(qwCharID);
  if (m == m_mapSocial.end())
    return false;
  m_mapSocial.erase(m);
  return true;
}

DWORD GChar::getRelationCount(DWORD dwRelation)
{
  DWORD dwCount = 0;
  for (auto &m : m_mapSocial)
  {
    if (checkRelation(m.first, dwRelation) == true)
      ++dwCount;
  }
  return dwCount;
}

DWORD GChar::getRecallCount()
{
  DWORD dwCount = 0;
  for (auto &m : m_mapSocial)
  {
    if (checkRelation(m.first, ESOCIALRELATION_BERECALL) == true)
      continue;
    if (checkRelation(m.first, ESOCIALRELATION_RECALL) == true)
      ++dwCount;
  }
  return dwCount;
}

void GChar::debug_log()
{
  XDBG << "[GChar-debug] charid :" << getCharID() << "zone :" << getZoneID() << "accid :" << getAccID() << "baselv :" << getBaseLevel() << "mapid :" << getMapID() << "portrait :" << getPortrait()
    << "body :" << getBody() << "head :" << getHead() << "face :" << getFace() << "back :" << getBack() << "tail :" << getTail() << "hair :" << getHair() << "haircolor :" << getHairColor()
    << "clothcolor :" << getClothColor() << "lefthand :" << getLeftHand() << "righthand :" << getRightHand() << "frame :" << getFrame() << "eye :" << getEye()
    << "offlinetime :" << getOfflineTime() << "manuallv :" << getManualLv() << "manualexp :" << getManualExp() << "title :" << getTitleID() << "querytype :" << getQueryType()
    << "profession :" << getProfession() << "gender :" << getGender() << "blink :" << getBlink() << "name :" << getName() << "guildid :" << getGuildID() << "guildname :" << getGuildName()
    << "guildportrait :" << getGuildPortrait() << "social :";
  for (auto &m : m_mapSocial)
    XDBG << m.first << ":" << m.second << ";";
  XDBG << XEND;
}

void GChar::parseSocial()
{
  m_mapSocial.clear();
  TVecString vecString;
  stringTok(m_oData.getTableString("social"), ";", vecString);
  for (auto &v : vecString)
  {
    TVecQWORD vecValue;
    numTok(v, ":", vecValue);
    if (vecValue.size() == 2)
      m_mapSocial[vecValue[0]] = vecValue[1];
  }
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/

GCharReader::GCharReader(DWORD dwRegionID, QWORD charid) : GChar(dwRegionID, charid)
{
}

GCharReader::~GCharReader()
{
}

bool GCharReader::get()
{
  if (!m_dwRegionID || !m_qwCharID)
  {
    XDBG << "[GChar-加载]" << "失败" << m_dwRegionID << m_qwCharID << XEND;
    return false;
  }

  if (RedisManager::getMe().getHashAll(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis数据库失败" << XEND;
    return false;
  }
  parseSocial();

//  if (m_oData.getTableQWORD("accid") == 0)
//  {
//    if (CommonConfig::getMe().IsGCharLoadDBNeed() == false)
//    {
//      XERR << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 数据库加载选项未开启, 获取失败" << XEND;
//      return false;
//    }
//
//    xField* pField = xFieldsM::getMe().getField(REGION_DB, "charbase");
//    if (pField == nullptr)
//    {
//      XERR << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 从数据库加载失败, 获取 charbase 数据库表失败" << XEND;
//      return false;
//    }
//
//    stringstream sstr;
//    sstr << "charid = " << id;
//    xRecordSet set;
//    QWORD ret = thisServer->getDBConn(pField).exeSelect(pField, set, sstr.str().c_str());
//    if (ret == QWORD_MAX || ret == 0)
//    {
//      XERR << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 从数据库加载失败, 查询失败 ret :" << ret << XEND;
//      return false;
//    }
//
//    setAccID(set[0].get<QWORD>("accid"));
//    setName(set[0].getString("name"));
//    setBaseLevel(set[0].get<DWORD>("rolelv"));
//    setMapID(set[0].get<DWORD>("mapid"));
//    setPortrait(set[0].get<DWORD>("portrait"));
//    setBody(set[0].get<DWORD>("body"));
//    setHead(set[0].get<DWORD>("head"));
//    setFace(set[0].get<DWORD>("face"));
//    setBack(set[0].get<DWORD>("back"));
//    setTail(set[0].get<DWORD>("tail"));
//    setHair(set[0].get<DWORD>("hair"));
//    setHairColor(set[0].get<DWORD>("haircolor"));
//    setClothColor(set[0].get<DWORD>("clothcolor"));
//    setLeftHand(set[0].get<DWORD>("lefthand"));
//    setRightHand(set[0].get<DWORD>("righthand"));
//    //oData.setData("frame", set[0].get<DWORD>("frame"));
//    setProfession(static_cast<EProfession>(set[0].get<DWORD>("profession")));
//    setGender(static_cast<EGender>(set[0].get<DWORD>("gender")));
//    //oData.setData("blink", set[0].get<DWORD>());
//    setOfflineTime(set[0].get<DWORD>("offlinetime"));
//    setEye(set[0].get<DWORD>("eye"));
//    setZoneID(set[0].get<DWORD>("zoneid"));
//    setMouth(set[0].get<DWORD>("mouth"));
//
//    string data;
//    data.assign((const char*)set[0].getBin("data"), set[0].getBinSize("data"));
//    if (uncompress(data, data) == false)
//    {
//      XERR << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 从数据库加载失败, 解压失败" << XEND;
//      return false;
//    }
//    BlobData oBlobData;
//    if (oBlobData.ParseFromString(data) == false)
//    {
//      XERR << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 从数据库加载失败, 序列化失败" << XEND;
//      return false;
//    }
//    const BlobManual& rManual = oBlobData.manual();
//    setManualLv(rManual.data().level());
//    setManualExp(rManual.data().point());
//
//    const BlobTitle& rTitle = oBlobData.title();
//    setTitleID(rTitle.curtitle());
//
//    const BlobOption& rOption = oBlobData.opt();
//    setQueryType(rOption.type());
//
//    /*oData.getTableQWORD("guild_id");
//    const string& guild_name = oData.getTableString("guild_name");
//    setGuildName(guild_name);
//    const string& guild_portrait = oData.getTableString("guild_portrait");
//    setGuildPortrait(guild_portrait);
//    const string& social = oData.getTableString("social");
//    setSocial(social);*/
//    save();
//    XLOG << "[GChar-加载]" << m_strRedisKey << "在redis未发现数据, 从数据库加载成功" << XEND;
//    return true;
//  }

  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis加载" << XEND;
  return true;
}

bool GCharReader::get(xLuaData &data)
{
  if (!m_dwRegionID || !m_qwCharID)
  {
    XDBG << "[GChar-加载]" << "失败" << m_dwRegionID << m_qwCharID << XEND;
    return false;
  }

  if (RedisManager::getMe().getHash(m_strRedisKey, data) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis数据库失败" << XEND;
    return false;
  }

  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis加载" << XEND;
  return true;
}

DWORD GCharReader::fetchMapID()
{
  xLuaData data;
  data.setData("mapid", "");
  if (get(data))
  {
    return data.getTableInt("mapid");
  }
  return 0;
}

EError GCharReader::getBySocial()
{
  m_oData.setData("rolelv", "");
  m_oData.setData("onlinetime", "");
  m_oData.setData("offlinetime", "");
  m_oData.setData("profession", "");
  m_oData.setData("portrait", "");
  m_oData.setData("hair", "");
  m_oData.setData("haircolor", "");
  m_oData.setData("body", "");
  m_oData.setData("manuallv", "");
  m_oData.setData("titleid", "");
  m_oData.setData("gender", "");
  m_oData.setData("guild_name", "");
  m_oData.setData("guild_portrait", "");
  m_oData.setData("blink", "");
  m_oData.setData("zoneid", "");
  m_oData.setData("head", "");
  m_oData.setData("face", "");
  m_oData.setData("mouth", "");
  m_oData.setData("profic", "");
  m_oData.setData("name", "");
  m_oData.setData("eye", "");
  m_oData.setData("social", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,查询失败" << XEND;
    return EERROR_REDIS_ERROR;
  }

  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (pField == nullptr)
    {
      XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,从charbase加载失败,没有数据库表" << XEND;
      return EERROR_DB_ERROR;
    }

    stringstream sstr;
    sstr << "charid = " << m_qwCharID;

    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str(), nullptr);
    if (ret == QWORD_MAX || ret == 0 || set.empty() == true)
    {
      XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,从charbase加载失败,查询失败 ret :" << ret << XEND;
      return EERROR_NOT_EXIST;
    }

    string zipdata;
    zipdata.assign((const char *)set[0].getBin("data"), set[0].getBinSize("data"));

    string data;
    if (uncompress(zipdata, data) == false)
    {
      XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,从charbase加载失败,解压失败" << XEND;
      return EERROR_DATA_ERROR;
    }

    BlobData oData;
    if (oData.ParseFromString(data) == false)
    {
      XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,从charbase加载失败,序列化失败" << XEND;
      return EERROR_DATA_ERROR;
    }

    GCharWriter oWriter(m_dwRegionID, m_qwCharID);

    oWriter.setBaseLevel(set[0].get<DWORD>("rolelv"));
    oWriter.setOnlineTime(set[0].get<DWORD>("onlinetime"));
    oWriter.setOfflineTime(set[0].get<DWORD>("offlinetime"));
    oWriter.setProfession(static_cast<EProfession>(set[0].get<DWORD>("profession")));
    oWriter.setPortrait(set[0].get<DWORD>("portrait"));
    oWriter.setHair(set[0].get<DWORD>("hair"));
    oWriter.setHairColor(set[0].get<DWORD>("haircolor"));
    oWriter.setBody(set[0].get<DWORD>("body"));
    oWriter.setManualLv(oData.manual().data().level());
    oWriter.setTitleID(oData.title().curtitle());
    oWriter.setGender(static_cast<EGender>(set[0].get<DWORD>("gender")));
    //oWriter.setData("guild_name", "");
    //oWriter.setData("guild_portrait", "");
    oWriter.setBlink(set[0].get<DWORD>("blink"));
    oWriter.setZoneID(set[0].get<DWORD>("zoneid"));
    oWriter.setHead(set[0].get<DWORD>("head"));
    oWriter.setFace(set[0].get<DWORD>("face"));
    oWriter.setMouth(set[0].get<DWORD>("mouth"));
    oWriter.setName(set[0].getString("name"));
    oWriter.setEye(oData.eye().curid());

    oWriter.save();

    if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
    {
      XERR << "[GChar-加载]" << m_strRedisKey << "从charbase加载后,读取redis_social数据库失败" << XEND;
      return EERROR_REDIS_ERROR;
    }
    XLOG << "[GChar-加载]" << m_strRedisKey << "从charbase加载" << XEND;
  }

  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败,未查询到数据" << XEND;
    return EERROR_NOT_EXIST;
  }
  if (m_oData.getTableInt("eye") == 0)
  {
    const SRoleBaseCFG* pCFG = RoleConfig::getMe().getRoleBase(static_cast<EProfession>(m_oData.getTableInt("profession")));
    if (pCFG != nullptr)
      setEye(m_oData.getTableInt("gender") == EGENDER_MALE ? pCFG->maleEye : pCFG->femaleEye);
    else
      XERR << "[GChar-加载]" << m_strRedisKey << "charid :" << getCharID() << "profession :" << getProfession() << "eye设置失败" << XEND;
  }

  DWORD dwOfflineTime = m_oData.getTableInt("offlinetime");
  DWORD dwOnlineTime = m_oData.getTableInt("onlinetime");

  DWORD dwServerDownTime = 1518057941;
  if (dwOnlineTime < dwServerDownTime && dwOnlineTime > dwOfflineTime)
  {
    dwOfflineTime = dwOnlineTime;
    GCharWriter oWriter(m_dwRegionID, m_qwCharID);

    dwOnlineTime = dwOfflineTime = dwServerDownTime;
    oWriter.setOnlineTime(dwOnlineTime);
    oWriter.setOfflineTime(dwOfflineTime);

    oWriter.save();
    XLOG << "[GChar-加载]" << m_strRedisKey << "非正常下线,重置上下线时间为" << dwServerDownTime << XEND;
  }

  m_oData.setData("offlinetime", dwOnlineTime > dwOfflineTime ? 0 : dwOfflineTime);
  parseSocial();
  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_social加载" << XEND;
  return EERROR_SUCCESS;
}

bool GCharReader::getByGuild()
{
  m_oData.setData("accid", "");
  m_oData.setData("rolelv", "");
  m_oData.setData("onlinetime", "");
  m_oData.setData("offlinetime", "");
  m_oData.setData("profession", "");
  m_oData.setData("portrait", "");
  m_oData.setData("hair", "");
  m_oData.setData("haircolor", "");
  m_oData.setData("body", "");
  m_oData.setData("gender", "");
  m_oData.setData("zoneid", "");
  m_oData.setData("name", "");
  m_oData.setData("guild_id", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }
  if (m_oData.getTableQWORD("rolelv") == 0)
  { 
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }

  DWORD dwOfflineTime = m_oData.getTableInt("offlinetime");
  DWORD dwOnlineTime = m_oData.getTableInt("onlinetime");
  m_oData.setData("offlinetime", dwOnlineTime > dwOfflineTime ? 0 : dwOfflineTime);
  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_guild加载" << XEND;
  return true;
}

bool GCharReader::getByTeam()
{
  m_oData.setData("guild_id", "");
  m_oData.setData("guild_name", "");
  m_oData.setData("social", "");
  m_oData.setData("wedding_partner", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_social数据库失败" << XEND;
    return false;
  }
  parseSocial();
  return true;
}

bool GCharReader::getByPvpTeam()
{
  m_oData.setData("accid", "");
  m_oData.setData("rolelv", "");
  m_oData.setData("onlinetime", "");
  m_oData.setData("offlinetime", "");
  m_oData.setData("profession", "");
  m_oData.setData("portrait", "");
  m_oData.setData("hair", "");
  m_oData.setData("haircolor", "");
  m_oData.setData("body", "");
  m_oData.setData("gender", "");
  m_oData.setData("name", "");
  m_oData.setData("guild_id", "");
  m_oData.setData("guild_name", "");
  m_oData.setData("eye", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }
  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }

  DWORD dwOfflineTime = m_oData.getTableInt("offlinetime");
  DWORD dwOnlineTime = m_oData.getTableInt("onlinetime");
  m_oData.setData("offlinetime", dwOnlineTime > dwOfflineTime ? 0 : dwOfflineTime);
  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_guild加载" << XEND;
  return true;
}

bool GCharReader::getNameOnly()
{
  m_oData.setData("name", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_name数据库失败" << XEND;
    return false;
  }
  string name = m_oData.getTableString("name");
  if (name.empty() == true)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_name数据库失败" << XEND;
    return false;
  }

  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_name加载" << XEND;
  return true;
}

bool GCharReader::getByTutor()
{
  m_oData.setData("accid", "");
  m_oData.setData("rolelv", "");
  m_oData.setData("onlinetime", "");
  m_oData.setData("offlinetime", "");
  m_oData.setData("tutor", "");
  m_oData.setData("social", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }
  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }

  parseSocial();
  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_guild加载" << XEND;
  return true;
}

bool GCharReader::getByQuery()
{
  m_oData.setData("rolelv", "");
  m_oData.setData("querytype", "");
  m_oData.setData("queryweddingtype", "");
  m_oData.setData("social", "");
  m_oData.setData("wedding_partner", "");


  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    m_oData.setData("querytype", 3);
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }
  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    m_oData.setData("querytype", 3);
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }

  parseSocial();
  XDBG << "[GChar-加载]" << m_strRedisKey << "成功从redis_guild加载" << XEND;
  return true;
}

bool GCharReader::getByWedding()
{
  m_oData.setData("rolelv", "");
  m_oData.setData("zoneid", "");
  m_oData.setData("onlinetime", "");
  m_oData.setData("offlinetime", "");

  if (RedisManager::getMe().getHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }
  if (m_oData.getTableQWORD("rolelv") == 0)
  {
    XERR << "[GChar-加载]" << m_strRedisKey << "读取redis_guild数据库失败" << XEND;
    return false;
  }

  return true;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/

GCharWriter::GCharWriter(DWORD dwRegionID, QWORD charid) : GChar(dwRegionID, charid)
{
}

GCharWriter::~GCharWriter()
{
}

bool GCharWriter::save()
{
  if (!m_dwRegionID || !m_qwCharID)
  {
    XDBG << "[GChar-save]" << "失败" << m_dwRegionID << m_qwCharID << XEND;
    return false;
  }

  if (m_oData.m_table.empty())
    return false;

#ifdef _ALL_SUPER_GM
  xTime frameTimer;
#endif
  if (RedisManager::getMe().setHash(m_strRedisKey, m_oData) == false)
  {
    XERR << "[GChar]" << m_qwCharID << "保存失败" << XEND;
    return false;
  }

#ifdef _ALL_SUPER_GM
  XDBG << "[GChar] 更新数据";
  for (auto &m : m_oData.m_table)
    XDBG << m.first << m.second.getString() << " ";
  XDBG << XEND;
  XDBG << "[GChar]" << m_qwCharID << "保存耗时 :" << frameTimer.uElapse() << "微秒" << XEND;
#endif

  m_oData.clear();
  return true;
}

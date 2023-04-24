#include "RecordUserManager.h"
#include "xDBFields.h"
#include "RecordServer.h"
#include "RegCmd.h"
#include "RedisManager.h"
#include "ClientCmd.h"
#include "PlatLogManager.h"
#include "GCharManager.h"
#include "BaseConfig.h"
#include "ShopConfig.h"
#include "MailManager.h"

RecordUserManager::RecordUserManager()
{

}

RecordUserManager::~RecordUserManager()
{

}

void RecordUserManager::final()
{
  for (auto m = m_mapID2User.begin(); m != m_mapID2User.end(); ++m)
  {
    m->second->saveData();
    SAFE_DELETE(m->second);
  }
  m_mapID2User.clear();
}

RecordUser* RecordUserManager::getData(USER_ID id, USER_ID accid /*= 0*/)
{
  auto m = m_mapID2User.find(id);
  if (m != m_mapID2User.end())
    return m->second;

  RecordUser* pUser = NEW RecordUser(accid, id);
  if (pUser == nullptr)
    return nullptr;
  if (pUser->loadData() == false)
  {
    SAFE_DELETE(pUser);
    return nullptr;
  }

  m_mapID2User[id] = pUser;
  m = m_mapID2User.find(id);
  if (m != m_mapID2User.end())
    return m->second;
  return nullptr;
}

RecordUser* RecordUserManager::getDataNoQuery(USER_ID id, USER_ID accid /*= 0*/)
{
  auto m = m_mapID2User.find(id);
  if (m != m_mapID2User.end())
    return m->second;
  if (accid != 0)
  {
    for (auto &m : m_mapID2User)
    {
      if (m.second->accid() == accid)
        return m.second;
    }
  }
  return nullptr;
}

bool RecordUserManager::delData(QWORD accid, USER_ID charid)
{
  auto m = m_mapID2User.find(charid);
  if (m != m_mapID2User.end())
  {
    // temp : 防止意外删除数据,在删除前先刷新下脏数据
    XLOG << "[玩家数据管理-删除]" << m->second->accid() << m->first << m->second->profession() << m->second->name() << "删除" << XEND;
    if (m->second->onlinetime() == m->second->offlinetime())
      m->second->setOfflineTime(m->second->onlinetime() + 1);
    m->second->saveData(true);

    SAFE_DELETE(m->second);
    m_mapID2User.erase(m);
  }
  else
  {
    XERR << "[玩家数据管理-删除]" << accid << charid << "删除失败,未找到数据" << XEND;
  }

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (field)
  {
    xRecord record(field);
    char updateWhere[256] = {0};
    snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=%llu and zoneid=%u", accid, charid, thisServer->getZoneID());
    record.put("charid", 0);
    record.put("zoneid", 0);
    QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
    if (QWORD_MAX == updateRet)
    {
      XERR << "[帐号退出]" << charid << "更新帐号失败" << XEND;
    }
  }

  XLOG << "[玩家数据管理-数量]" << m_mapID2User.size() << XEND;
  return true;
}

void RecordUserManager::timetick(DWORD curTime, bool bTimeOut/*=false*/)
{
  for (auto m = m_mapID2User.begin(); m != m_mapID2User.end(); ++m)
  {
    if (curTime >= m->second->timetick())
    {
      m->second->setTimeTick(curTime + USER_SAVE_TICK);

      if (!bTimeOut)
      {
        m->second->saveData();
        if (now() >= curTime + 2)
          bTimeOut = true;
      }
    }
  }
}

void RecordUserManager::delChar(USER_ID id, USER_ID accid)
{
  if (!id) return;
  char where[256];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "charid=%llu", id);

  RetDeleteCharRegCmd send;
  send.id = id;
  send.accid = accid;

  GCharReader oGChar(thisServer->getRegionID(), id);
  RedisManager::getMe().delData(oGChar.getKey());

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pField)
  {
    QWORD ret = thisServer->getDBConnPool().exeDelete(pField, where);
    if (ret != QWORD_MAX)
    {
      send.ret = 1;

      XLOG << "[删除角色]" << accid << id << XEND;
      delData(accid, id);
      xField *field = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
      if (field)
      {
        char sWhere[256] = {0};
        snprintf(sWhere, sizeof(sWhere), "accid=%llu", accid);
        xRecordSet set;
        QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, sWhere);
        DWORD count = 0;
        DWORD cur = now();
        DWORD days = getDays(cur);
        if (QWORD_MAX != retNum && retNum)
        {
          count = set[0].get<DWORD>("deletecount");
          if (days != set[0].get<DWORD>("deletecountdays"))
          {
            count = 0;
          }
          else
          {
            count = set[0].get<DWORD>("deletecount");
          }
        }
        xRecord record(field);
        char updateWhere[256] = {0};
        snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and deletecharid=%llu", accid, id);
        record.put("deletecharid", 0);
        record.put("deletetime", 0);
        record.put("deletecount", count + 1);
        record.put("deletecountdays", days);
        QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
        if (QWORD_MAX == updateRet)
        {
          XERR << "[删除角色]" << accid << id << "更新帐号数据失败" << XEND;
        }

        xRecordSet userset;
        QWORD userRetNum = thisServer->getDBConnPool().exeSelect(pField, userset, sWhere);
        if (QWORD_MAX != userRetNum && userRetNum)
        {
          // 同账号下其他角色, 添加删除事件
          UserEventMailCmd cmd;
          cmd.set_etype(EEVENTMAILTYPE_DELCAHR);
          cmd.add_param64(id);
          for (DWORD i = 0; i < userRetNum; ++i)
          {
            QWORD charid = userset[i].get<QWORD>("charid");
            MailManager::getMe().addEventMail(charid, cmd);
          }
        }
      }
    }
  }
  thisServer->sendCmdToServer(&send, sizeof(send), "SessionServer");

  //删角色日志
  PlatLogManager::getMe().deleteCharLog(thisServer,
    thisServer->getPlatformID(),
    0,
    accid,
    id
  );
}

void RecordUserManager::patch1()
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pField == nullptr)
    return;

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, nullptr);
  if (ret == QWORD_MAX)
    return;

  for (DWORD q = 0; q < set.size(); ++q)
  {
    string data;
    data.assign((const char *)set[q].getBin("data"), set[q].getBinSize("data"));

    if (uncompress(data, data) == false)
      continue;

    BlobData oData;
    if (oData.ParseFromString(data) == false)
      continue;

    const BlobQuest& rQuest = oData.quest();
    for (int item = 0; item < rQuest.accept_size(); ++item)
    {
      /*if (rQuest.accept(item).id() == 350010001)
      {
        XDBG << "[补偿]" << set[q].get<QWORD>("charid") << set[q].getString("name");
        XDBG << "中级任务id :" << rQuest.accept(item).id() << "补偿";
        DWORD dwStep = rQuest.accept(item).step();
        if (dwStep >= 15)
          XDBG << "itemid :" << 52104 << "count :" << 10;
        if (dwStep >= 21)
          XDBG << "itemid :" << 52203 << "count :" << 10;
        if (dwStep >= 27)
          XDBG << "itemid :" << 52204 << "count :" << 5;
        if (dwStep >= 31)
          XDBG << "itemid :" << 135 << "count :" << 5;
        XDBG << XEND;
      }
      else if (rQuest.accept(item).id() == 350020002)
      {
        XDBG << "[补偿]" << set[q].get<QWORD>("charid") << set[q].getString("name");
        XDBG << "高级任务id :" << rQuest.accept(item).id() << "补偿";
        DWORD dwStep = rQuest.accept(item).step();
        if (dwStep >= 6)
          XDBG << "itemid :" << 52305 << "count :" << 3;
        if (dwStep >= 12)
          XDBG << "itemid :" << 52307 << "count :" << 3;
        if (dwStep >= 18)
          XDBG << "itemid :" << 52306 << "count :" << 3;
        if (dwStep >= 27)
          XDBG << "itemid :" << 135 << "count :" << 10;
        XDBG << XEND;
      }*/
      if (rQuest.accept(item).id() == 390040001)
      {
        DWORD dwStep = rQuest.accept(item).step();
        if (dwStep >= 26)
        {
          XDBG << "[补偿]" << set[q].get<QWORD>("charid") << set[q].getString("name");
          XDBG << "进阶-D级冒险者任务 id :" << rQuest.accept(item).id() << "补偿";
          XDBG << "itemid :" << 52307 << "count :" << 3;
          XDBG << XEND;
        }
      }
    }
  }
}

#include "zlib/zlib.h"
void RecordUserManager::patch()
{
}

/*// user data
bool SRecordUserData::saveData(bool redis)
{
  mark.set();
  if (mark.any() == false)
    return true;

  // get field - charbase
  xField* pField = xFieldsM::getMe().getField(REGION_DB, "charbase");
  if (pField == NULL)
  {
    XERR << "[玩家管理-存储]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "获取数据库charbase失败" << XEND;
    return false;
  }

  // get sql record
  xRecord record(pField);
  char where[32] = {0};
  bool bIsUpdate = false;
  snprintf(where, sizeof(where), "charid=%llu", static_cast<QWORD>(oBase.charid()));

  record.put("guildid", oBase.guildid());

  for (BYTE b = 0; b < EUSERDATATYPE_MAX; ++b)
  {
    EUserDataType eType = static_cast<EUserDataType>(b);
    if (mark.test(eType) == false)
      continue;

    bIsUpdate = true;

    switch (eType)
    {
      case EUSERDATATYPE_MIN:
        break;
      case EUSERDATATYPE_MAPID:
        record.put("mapid", oBase.mapid());
        break;
      case EUSERDATATYPE_SEX:
        record.put("gender", oBase.gender());
        break;
      case EUSERDATATYPE_PROFESSION:
        record.put("profession", oBase.profession());
        break;
      case EUSERDATATYPE_DESTPROFESSION: // 一次写入，不做更新
        break;
      case EUSERDATATYPE_JOBLEVEL:    // 存储在userdata中，此枚举和客户端同步用
        break;
      case EUSERDATATYPE_ROLELEVEL:
        record.put("rolelv", oBase.rolelv());
        break;
      case EUSERDATATYPE_JOBEXP:      // 存储在userdata中，此枚举和客户端同步用
        break;
      case EUSERDATATYPE_ROLEEXP:
        record.put("roleexp", oBase.roleexp());
        break;
      case EUSERDATATYPE_CHARGE:
        record.put("charge", oBase.charge());
        break;
      case EUSERDATATYPE_DIAMOND:
        record.put("diamond", oBase.diamond());
        break;
      case EUSERDATATYPE_SILVER:
        record.put("silver", oBase.silver());
        break;
      case EUSERDATATYPE_GOLD:
        record.put("gold", oBase.gold());
        break;
      case EUSERDATATYPE_GARDEN:
        record.put("garden", oBase.garden());
        break;
      case EUSERDATATYPE_FRIENDSHIP:
        record.put("friendship", oBase.friendship());
        break;
      case EUSERDATATYPE_ONLINETIME:
        record.put("onlinetime", oBase.onlinetime());
        break;
      case EUSERDATATYPE_OFFLINETIME:
        record.put("offlinetime", oBase.offlinetime());
        break;
      case EUSERDATATYPE_CREATETIME:
        record.put("createtime", oBase.createtime());
        break;
      case EUSERDATATYPE_ADDICT:
        record.put("addict", oBase.addict());
        break;
      case EUSERDATATYPE_BATTLETIME:
        record.put("battletime", oBase.battletime());
        break;
      case EUSERDATATYPE_ADDICTTIPSTIME:
        record.put("addicttipstime", oBase.addicttipstime());
        break;
      case EUSERDATATYPE_REBATTLETIME:
        record.put("rebattletime", oBase.rebattletime());
        break;
      case EUSERDATATYPE_USEDBATTLETIME:
        record.put("usedbattletime", oBase.usedbattletime());
        break;
      case EUSERDATATYPE_HAIR:
        record.put("hair", oBase.hair());
        break;
      case EUSERDATATYPE_HAIRCOLOR:
        record.put("haircolor", oBase.haircolor());
        break;
      case EUSERDATATYPE_PURIFY:      // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_CLOTHCOLOR:  // 存储在userdata中，此枚举和客户端同步用
        break;
      case EUSERDATATYPE_LEFTHAND:
        record.put("lefthand", oBase.lefthand());
        break;
      case EUSERDATATYPE_RIGHTHAND:
        record.put("righthand", oBase.righthand());
        break;
      case EUSERDATATYPE_BODY:
        record.put("body", oBase.body());
        break;
      case EUSERDATATYPE_SHADERCOLOR: // 记录在buff中, 不需要存储
      case EUSERDATATYPE_BODYSCALE:   // 存储在userdata中，此枚举和客户端同步用
        break;
      case EUSERDATATYPE_HEAD:
        record.put("head", oBase.head());
        break;
      case EUSERDATATYPE_BACK:
        record.put("back", oBase.back());
        break;
      case EUSERDATATYPE_FACE:
        record.put("face", oBase.face());
        break;
      case EUSERDATATYPE_TAIL:
        record.put("tail", oBase.tail());
        break;
      case EUSERDATATYPE_MOUNT:
        record.put("mount", oBase.mount());
        break;
      case EUSERDATATYPE_CUR_TITLE:
        record.put("title", oBase.title());
        break;
      case EUSERDATATYPE_PET_PARTNER:
        record.put("partnerid", oBase.partnerid());
        break;
      case EUSERDATATYPE_PORTRAIT:
        record.put("portrait", oBase.portrait());
        break;
      case EUSERDATATYPE_MOUTH:
        record.put("mouth", oBase.mouth());
        break;
      case EUSERDATATYPE_EYE:
        record.put("eye", oBase.eye());
        break;
      case EUSERDATATYPE_NAME:
        //name只能通过操作数据库来修改
        //record.putString("name", oBase.name());
        break;
      case EUSERDATATYPE_STRPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_INTPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_AGIPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_DEXPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_VITPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_LUKPOINT:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_TOTALPOINT:  // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_USEDPOINT:   // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_NORMAL_SKILL:// 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_COLLECT_SKILL:// 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_TRANS_SKILL: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_SKILL_POINT: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_STATUS:      // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_EQUIPMASTER: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_REFINEMASTER:// 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_FRAME:       // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_BATTLEPOINT: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_RAIDID:      // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_PET_SELF:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_SAVEMAP:     // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_FOLLOWID:    // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_HANDID:      // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_CARRIER:     // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_MUSIC_CURID: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_MUSIC_START: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_MUSIC_DEMAND:// 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_DIR:         // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_GIFTPOINT:   // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_MANUAL_LV:   // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_MANUAL_EXP:  // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_KILLERNAME:  // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_DROPBASEEXP: // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_QUERYTYPE:   // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_BLINK:       // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_ALPHA:       // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_ZENY_DEBT:   // 存储在userdata中，此枚举和客户端同步用
      case EUSERDATATYPE_QUOTA:
      case EUSERDATATYPE_PVP_COLOR:
      case EUSERDATATYPE_GUILDRAIDINDEX:
      case EUSERDATATYPE_NORMALSKILL_OPTION:
      case EUSERDATATYPE_HASCHARGE:
      case EUSERDATATYPE_FASHIONHIDE:
      case EUSERDATATYPE_MONTHCARD:
      case EUSERDATATYPE_PVPCOIN:
      case EUSERDATATYPE_COOKER_EXP:
      case EUSERDATATYPE_COOKER_LV:
      case EUSERDATATYPE_TASTER_EXP:
      case EUSERDATATYPE_TASTER_LV:
      case EUSERDATATYPE_SATIETY:
      case EUSERDATATYPE_OPTION:
      case EUSERDATATYPE_LOTTERY:
      case EUSERDATATYPE_TUTOR_PROFIC:
      case EUSERDATATYPE_TUTOR_ENABLE:
      case EUSERDATATYPE_GUILDHONOR:
      case EUSERDATATYPE_PEAK_EFFECT:
        break;
      case EUSERDATATYPE_GAGTIME:
        //record.put("gagtime", oBase.gagtime());
        //record.put("gagreason", oBase.gag_reason());
        break;
      case EUSERDATATYPE_NOLOGINTIME:
        //record.put("nologintime", oBase.nologintime());
        //record.put("lockreason", oBase.lock_reason());
        break;
      case EUSERDATATYPE_DATA:
        record.putBin("data", (unsigned char *)(oBlob.c_str()), oBlob.size());
        break;
      case EUSERDATATYPE_ZONEID:
        if (oBase.zoneid())
        {
          record.put("zoneid", oBase.zoneid());
        }
        break;
      case EUSERDATATYPE_DEST_ZONEID:
        record.put("destzoneid", oBase.destzoneid());
        break;
      case EUSERDATATYPE_ORIGINAL_ZONEID:
        record.put("originalzoneid", oBase.originalzoneid());
        break;
      case EUSERDATATYPE_TREESTATUS:
      case EUSERDATATYPE_CONTRIBUTE:
      case EUSERDATATYPE_MAX:
        break;
    }
  }

  // update to db
  mark.reset();
  if (bIsUpdate)
  {
    xTime frameTimer;
    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX)
    {
      XERR << "[玩家管理-数据存储]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "存储失败 ret :" << ret << XEND;
    }
    else
    {
      XLOG << "[玩家管理-数据存储]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name()
        << "存储成功 data:" << oBlob.size() << "dbdata :" << record.getBinSize("data") << "耗时" << frameTimer.uElapse() << "微秒" << XEND;
    }
  }

  saveCommonData();
  saveRedis();

  return true;
}

bool SRecordUserData::saveRedis()
{
  GCharWriter gChar(thisServer->getRegionID(), oBase.charid());

  gChar.setVersion();

  gChar.setName(oBase.name());
  gChar.setAccID(oBase.accid());
  gChar.setCharID(oBase.charid());
  gChar.setBaseLevel(oBase.rolelv());
  gChar.setMapID(oBase.mapid());
  gChar.setPortrait(oRedis.portrait());
  gChar.setBody(oBase.body());
  gChar.setHead(oBase.head());
  gChar.setFace(oBase.face());
  gChar.setBack(oBase.back());
  gChar.setTail(oBase.tail());
  gChar.setHair(oBase.hair());
  gChar.setHairColor(oBase.haircolor());
  gChar.setClothColor(oRedis.clothcolor());
  gChar.setLeftHand(oBase.lefthand());
  gChar.setRightHand(oBase.righthand());
  gChar.setOnlineTime(oBase.onlinetime());
  gChar.setOfflineTime(oBase.offlinetime());
  gChar.setEye(oBase.eye());
  gChar.setMouth(oBase.mouth());
  gChar.setZoneID(oBase.zoneid());
  gChar.setOriginalZoneID(oBase.originalzoneid());
  gChar.setManualLv(oRedis.manuallv());
  gChar.setManualExp(oRedis.manualexp());
  gChar.setTitleID(oBase.title());
  DWORD dwQueryType = oRedis.querytype();
  if (dwQueryType > EQUERYTYPE_MIN && dwQueryType << EQUERYTYPE_MAX && EQueryType_IsValid(dwQueryType) == true)
    gChar.setQueryType(dwQueryType);
  gChar.setProfession(oBase.profession());
  gChar.setGender(oBase.gender());
  gChar.setBlink(oRedis.blink());
  gChar.setTutor(oRedis.canbetutor());
  gChar.setProfic(oRedis.profic());

  // 选角界面格子编号
  if (m_dwSequence)
  {
    gChar.setSequence(m_dwSequence);
  }
  // 坐骑
  gChar.setMount(oBase.mount());
  // 称号 冒险登记
  //gChar.setTitle(oBase.title());
  // 宠物
  gChar.setPartnerID(oBase.partnerid());
  // 封号时间
  gChar.setNologinTime(oBase.nologintime());

  xTime frameTimer;
  if (gChar.save() == false)
  {
    XERR << "[玩家管理-redis数据]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "存储失败" << XEND;
    return false;
  }

  XLOG << "[玩家管理-redis数据]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "存储成功,耗时 :" << frameTimer.uElapse() << "微秒" << XEND;
  return true;
}

bool SRecordUserData::updateRedis(EUserDataType eType)
{
  GCharWriter gChar(thisServer->getRegionID(), oBase.charid());

  gChar.setVersion();

  if (eType == EUSERDATATYPE_ONLINETIME)
    gChar.setOnlineTime(oBase.onlinetime());
  gChar.save();
  return true;
}

bool SRecordUserData::loadStore()
{
  oStore.clear();

  xField* pField = xFieldsM::getMe().getField(REGION_DB, "char_store");
  if (pField == nullptr)
  {
    XERR << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "获取char_store失败" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "accid=" << oBase.accid();
  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str(), NULL);
  if (QWORD_MAX == retNum)
  {
    XERR << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "查询失败 ret :" << retNum << XEND;
    return false;
  }

  PackageData oStoreData;
  oStoreData.set_type(EPACKTYPE_STORE);

  for (QWORD q = 0; q < retNum; ++q)
  {
    ItemData* pData = oStoreData.add_items();
    if (pData == nullptr)
    {
      XERR << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "创建ItemData protobuf失败" << XEND;
      continue;
    }

    string item;
    item.assign((const char *)set[q].getBin("item"), set[q].getBinSize("item"));
    if (pData->ParseFromString(item) == false)
    {
      XERR << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "序列化ItemData失败" << XEND;
      continue;
    }
    pData->mutable_base()->set_isnew(false);
  }
  if (oStoreData.SerializeToString(&oStore) == false)
  {
    XERR << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "序列化PackageData失败" << XEND;
    return false;
  }

  XLOG << "[玩家管理-加载仓库]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "成功加载" << XEND;
  return true;
}

bool SRecordUserData::saveCommonData()
{
  xField* pField = xFieldsM::getMe().getField(REGION_DB, "accbase");
  if (pField == nullptr)
  {
    XERR << "[玩家管理-通用数据]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "存储失败, 未找到 accbase 数据库表" << XEND;
    return false;
  }

  xRecord record(pField);
  char where[32] = {0};
  snprintf(where, sizeof(where), "accid=%llu", static_cast<QWORD>(oBase.accid()));

  record.putBin("credit", (unsigned char*)(oCredit.c_str()), oCredit.size());
  record.putBin("quest", (unsigned char*)(oAccQuest.c_str()), oAccQuest.size());

  xTime frameTimer;
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家管理-通用数据]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "存储失败, ret :" << ret << XEND;
    return false;
  }
  XLOG << "[玩家管理-通用数据]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name()
    << "存储成功, 数据大小 credit :" << oCredit.size() << "quest :" << oAccQuest.size() << "耗时:" << frameTimer.uElapse() << "微妙" << XEND;
  return true;
}

bool SRecordUserData::updateData(const UserDataRecordCmd& rCmd)
{
  if (rCmd.first() == true)
    oBlobTemp.clear();
  if (rCmd.over() == false)
  {
    oBlobTemp.append(rCmd.data().c_str(), rCmd.data().size());
    return false;
  }

  oBlobTemp.append(rCmd.data().c_str(), rCmd.data().size());

  RecordUserData oData;
  if (oData.ParseFromString(oBlobTemp) == false)
  {
    XERR << "[玩家管理-更新]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "反序列化失败" << XEND;
    return false;
  }

  oBase.CopyFrom(oData.base());
  oRedis.CopyFrom(oData.redis());

  oBlob.assign(oData.data().c_str(), oData.data().size());
  oCredit.assign(oData.credit().c_str(), oData.credit().size());
  oAccQuest.assign(oData.acc_quest().c_str(), oData.acc_quest().size());

  mark.set();
  oBlobTemp.clear();

  return true;
}*/

/*void RecordUserManager::final()
{
  for (auto m = m_mapID2UserData.begin(); m != m_mapID2UserData.end(); ++m)
    m->second.saveData(true);
}*/
//SRecordUserData* RecordUserManager::getData(USER_ID id, USER_ID accid /*= 0*/, UnregType eType /*= UnregType::ChangeScene*/)
/*{
  auto m = m_mapID2UserData.find(id);
  if (m != m_mapID2UserData.end())
  {
    if (eType == UnregType::ChangeScene)
      m->second.loadStore();
    return &m->second;
  }

  if (loadData(id, accid) == false)
    return NULL;

  m = m_mapID2UserData.find(id);
  if (m != m_mapID2UserData.end())
    return &m->second;

  return NULL;
}*/

/*bool RecordUserManager::delData(QWORD accid, USER_ID charid)
{
  xField *field = xFieldsM::getMe().getField(REGION_DB, "accbase");
  if (field)
  {
    xRecord record(field);
    char updateWhere[256] = {0};
    snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=%llu and zoneid=%u", accid, charid, thisServer->getZoneID());
    record.put("charid", 0);
    record.put("zoneid", 0);
    QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
    if (QWORD_MAX == updateRet)
    {
      XERR << "[帐号退出]" << charid << "更新帐号失败" << XEND;
    }
  }

  auto m = m_mapID2UserData.find(charid);
  if (m == m_mapID2UserData.end())
    return false;

  // temp : 防止意外删除数据,在删除前先刷新下脏数据
  XLOG << "[玩家数据管理-删除]" << m->second.oBase.accid() << m->first << m->second.oBase.profession() << m->second.oBase.name() << "删除" << XEND;
  if (m->second.oBase.onlinetime() == m->second.oBase.offlinetime())
    m->second.oBase.set_offlinetime(m->second.oBase.onlinetime() + 1);
  m->second.saveData();

  m_mapID2UserData.erase(m);
  XLOG << "[玩家数据管理-数量]" << m_mapID2UserData.size() << XEND;
  return true;
}*/

/*bool RecordUserManager::loadData(USER_ID id, USER_ID accid)
{
  auto m = m_mapID2UserData.find(id);
  if (m != m_mapID2UserData.end())
    return false;

  string creditData;
  string questData;
  QWORD deletecharid = 0;
  {
    xField *field = xFieldsM::getMe().getField(REGION_DB, "accbase");
    if (field)
    {
      char where[256] = {0};
      snprintf(where, sizeof(where), "accid=%llu", accid);

      xRecordSet set;
      QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
      if (QWORD_MAX == ret)
      {
        XERR << "[帐号登录]" << accid << id << "查询数据库失败" << XEND;
        return false;
      }
      if (0 == ret)
      {
        xRecord record(field);
        record.put("accid", accid);
        QWORD insertRet = thisServer->getDBConnPool().exeInsert(record);
        if (QWORD_MAX == insertRet)
        {
          XERR << "[帐号登录]" << accid << id << "初始化插入数据库失败" << XEND;
          return false;
        }
        ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
        if (QWORD_MAX == ret)
        {
          XERR << "[帐号登录]" << accid << id << "初始化后查询数据库失败" << XEND;
          return false;
        }
      }
      if (0 == ret)
      {
        XERR << "[帐号登录]" << accid << id << "找不到帐号数据" << XEND;
        return false;
      }

      xRecord record(field);
      if (ret)
      {
        QWORD charid = set[0].get<QWORD>("charid");
        if (charid)
        {
          if (charid != id)
          {
            Cmd::ReconnectClientUserCmd send;
            send.charid = id;
            thisServer->sendCmdToSession(&send, sizeof(send));
            XLOG << "[帐号登录]" << accid << id << "其他角色正在登录，等待退出" << charid << XEND;
            return false;
          }
          if (set[0].get<DWORD>("zoneid") != thisServer->getZoneID())
          {
            char updateWhere[256] = {0};
            snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=%llu", accid, charid);
            record.put("zoneid", thisServer->getZoneID());
            QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
            if (QWORD_MAX == updateRet)
            {
              XERR << "[帐号登录]" << accid << id << "更新帐号失败" << XEND;
              return false;
            }
            if (0==updateRet)
            {
              XERR << "[帐号登录]" << accid << id << "更新帐号失败,无该条件数据" << XEND;
              return false;
            }
          }
        }
        else
        {
          char updateWhere[256] = {0};
          snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=0", accid);
          record.put("charid", id);
          record.put("lastselect", id);
          record.put("zoneid", thisServer->getZoneID());
          QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
          if (QWORD_MAX == updateRet)
          {
            XERR << "[帐号登录]" << accid << id << "更新帐号失败" << XEND;
            return false;
          }
          if (0==updateRet)
          {
            XERR << "[帐号登录]" << accid << id << "更新帐号失败,无该条件数据" << XEND;
            return false;
          }
        }
        {
          xRecordSet vSet;
          QWORD vRet = thisServer->getDBConnPool().exeSelect(field, vSet, (const char *)where, NULL);
          if (QWORD_MAX == vRet)
          {
            XERR << "[帐号登录]" << accid << id << "查询数据库失败" << XEND;
            return false;
          }
          if (0 == vRet)
          {
            XERR << "[帐号登录]" << accid << id << "查询数据库失败" << XEND;
            return false;
          }
          if (vSet[0].get<QWORD>("charid") != id)
          {
            Cmd::ReconnectClientUserCmd send;
            send.charid = id;
            thisServer->sendCmdToSession(&send, sizeof(send));
            XLOG << "[帐号登录]" << accid << id << "其他角色正在登录，等待退出" << charid << XEND;
            return false;
          }
        }
        deletecharid = set[0].get<QWORD>("deletecharid");
        if (deletecharid == id)
        {
          return false;
        }
        creditData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));
        questData.assign((const char*)set[0].getBin("quest"), set[0].getBinSize("quest"));
      }
    }
  }

  // load from db
  DWORD size = 0;
  SRecordUserData stData;
  UserBaseData& oBase = stData.oBase;
  {
    xField *field = xFieldsM::getMe().getField(REGION_DB, "charbase");
    if (field)
    {
      char where[256] = {0};
      snprintf(where, sizeof(where), "accid=%llu AND charid=%llu", accid, id);

      xRecordSet set;
      QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
      if (QWORD_MAX == retNum || 1 != retNum || retNum == 0)
        return false;

      if (set[0].get<DWORD>("zoneid") != thisServer->getZoneID())
      {
        GCharWriter gChar(thisServer->getRegionID(), id);
        gChar.setZoneID(set[0].get<DWORD>("zoneid"));
        gChar.save();

        Cmd::ReconnectClientUserCmd send;
        send.charid = id;
        thisServer->sendCmdToSession(&send, sizeof(send));

        XERR << "[登录-区]" << accid << id << "zoneid不一致:" << set[0].get<DWORD>("zoneid") << thisServer->getZoneID() << XEND;
        return false;
      }

      stData.m_dwSequence = set[0].get<DWORD>("sequence");

      // base data
      stData.oBase.set_platformid(set[0].get<DWORD>("platformid"));
      stData.oBase.set_zoneid(set[0].get<DWORD>("zoneid"));
      stData.oBase.set_destzoneid(set[0].get<DWORD>("destzoneid"));
      stData.oBase.set_originalzoneid(set[0].get<DWORD>("originalzoneid"));
      stData.oBase.set_accid(set[0].get<QWORD>("accid"));
      stData.oBase.set_charid(set[0].get<QWORD>("charid"));
      stData.oBase.set_name(set[0].getString("name"));
      stData.oBase.set_createtime(set[0].get<DWORD>("createtime"));
      DWORD mapid = set[0].get<DWORD>("mapid");
      stData.oBase.set_mapid(mapid);
      stData.oBase.set_gender(static_cast<EGender>(set[0].get<WORD>("gender")));
      stData.oBase.set_profession(static_cast<EProfession>(set[0].get<DWORD>("profession")));
      stData.oBase.set_destprofession(static_cast<EProfession>(set[0].get<DWORD>("destprofession")));
      stData.oBase.set_rolelv(set[0].get<WORD>("rolelv"));
      stData.oBase.set_roleexp(set[0].get<QWORD>("roleexp"));
      stData.oBase.set_charge(set[0].get<DWORD>("charge"));
      stData.oBase.set_diamond(set[0].get<DWORD>("diamond"));
      stData.oBase.set_silver(set[0].get<QWORD>("silver"));
      stData.oBase.set_gold(set[0].get<DWORD>("gold"));
      stData.oBase.set_garden(set[0].get<DWORD>("garden"));
      stData.oBase.set_friendship(set[0].get<DWORD>("friendship"));
      stData.oBase.set_onlinetime(xTime::getCurSec());//set[0].get<DWORD>("onlinetime"));
      stData.oBase.set_offlinetime(set[0].get<DWORD>("offlinetime"));
      stData.oBase.set_addict(set[0].get<WORD>("addict"));
      stData.oBase.set_battletime(set[0].get<DWORD>("battletime"));
      stData.oBase.set_rebattletime(set[0].get<DWORD>("rebattletime"));
      stData.oBase.set_usedbattletime(set[0].get<DWORD>("usedbattletime"));
      stData.oBase.set_addicttipstime(set[0].get<DWORD>("addicttipstime"));
      stData.oBase.set_body(set[0].get<DWORD>("body"));
      stData.oBase.set_gagtime(set[0].get<DWORD>("gagtime"));
      stData.oBase.set_nologintime(set[0].get<DWORD>("nologintime"));
      stData.oBase.set_hair(set[0].get<DWORD>("hair"));
      stData.oBase.set_haircolor(set[0].get<DWORD>("haircolor"));
      stData.oBase.set_lefthand(set[0].get<DWORD>("lefthand"));
      stData.oBase.set_righthand(set[0].get<DWORD>("righthand"));
      stData.oBase.set_head(set[0].get<DWORD>("head"));
      stData.oBase.set_back(set[0].get<DWORD>("back"));
      stData.oBase.set_face(set[0].get<DWORD>("face"));
      stData.oBase.set_tail(set[0].get<DWORD>("tail"));
      stData.oBase.set_mount(set[0].get<DWORD>("mount"));
      stData.oBase.set_title(set[0].get<DWORD>("title"));
      stData.oBase.set_eye(set[0].get<DWORD>("eye"));
      stData.oBase.set_partnerid(set[0].get<DWORD>("partnerid"));
      stData.oBase.set_portrait(set[0].get<DWORD>("portrait"));
      stData.oBase.set_mouth(set[0].get<DWORD>("mouth"));

      stData.oBlob.assign((const char *)set[0].getBin("data"), set[0].getBinSize("data"));
      size = set[0].getBinSize("data");
      XLOG << "[玩家管理-加载-front]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "data :" << stData.oBlob.size() << "dbdata :" << size << XEND;
    }
  }

  // 账号下所有角色最大等级
  {
    xField field(REGION_DB, "charbase");
    field.m_list["maxbaselv"] = MYSQL_TYPE_NEWDECIMAL;
    stringstream maxlvsql;
    maxlvsql << "select max(rolelv) as maxbaselv from " << field.m_strDatabase.c_str() << "." << field.m_strTable.c_str() << " where accid=" << accid << " and charid!=" << deletecharid;
    xRecordSet maxlvset;
    QWORD maxlvret = thisServer->getDBConnPool().exeRawSelect(&field, maxlvset, maxlvsql.str());
    if (maxlvret != 1)
    {
      XERR << "[帐号登录]" << accid << id << "查询最大角色等级失败" << XEND;
      if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
        return false;
    }
    else
    {
      BlobCredit credit;
      if (credit.ParseFromString(creditData) == false)
      {
        XERR << "[帐号登录]" << accid << id << "反序列化最大角色等级失败" << XEND;
        if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
          return false;
      }
      else
      {
        DWORD maxbaselv = maxlvset[0].get<DWORD>("maxbaselv");
        DWORD cur = now();
        if (xTime::getDayStart(cur, MAXBASELV_RESETTIME_OFFSET) >= xTime::getDayStart(credit.maxbaselv_resettime(), MAXBASELV_RESETTIME_OFFSET) + MAXBASELV_RESETTIME)
        {
          credit.set_maxbaselv(maxbaselv);
          credit.set_maxbaselv_resettime(cur);
          if (credit.SerializeToString(&creditData) == false)
          {
            XERR << "[帐号登录]" << accid << id << "序列化最大角色等级失败" << XEND;
            if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
              return false;
          }
        }
      }
    }
  }

  // load share data
  stData.oCredit.assign(creditData.c_str(), creditData.size());
  stData.oAccQuest.assign(questData.c_str(), questData.size());
  if (stData.loadStore() == false)
  {
    XERR << "[玩家管理-加载]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "加载仓库数据失败" << XEND;
    return false;
  }
  XLOG << "[玩家管理-加载-middle]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "data :" << stData.oBlob.size() << "dbdata :" << size << XEND;
  XLOG << "[玩家管理-加载-last]" << oBase.accid() << oBase.charid() << oBase.profession() << oBase.name() << "data :" << stData.oBlob.size() << "dbdata :" << size << XEND;

  // save online quick for social
  if (oBase.onlinetime() == oBase.offlinetime())
    oBase.set_onlinetime(oBase.offlinetime() + 1);
  stData.updateRedis(EUSERDATATYPE_ONLINETIME);
  stData.timeTick = oBase.onlinetime() + USER_SAVE_TICK;

  // insert to list
  m_mapID2UserData[id] = stData;

  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ONLINE_MAPID, id);
  string data;
  RedisManager::getMe().delData(key);
  return true;
}*/
/*void RecordUserManager::timetick(DWORD curTime)
{
  // check changed data
  for (auto m = m_mapID2UserData.begin(); m != m_mapID2UserData.end(); ++m)
  {
    SRecordUserData& rData = m->second;

    // check time tick
    if (curTime < rData.timeTick)
      continue;

    rData.timeTick = curTime + USER_SAVE_TICK;
    rData.saveData(false);
  }
}*/

#include "Var.h"
#include "xLog.h"
#include "xTime.h"

// var manager
VarManager::VarManager()
{
  DWORD offsetSec = 5 * 3600;
  registerVar(EVARTYPE_QUEST_WANTED, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_QUEST_WANTED_RESET, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_SHOP, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_TOWER, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_SEAL, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_ANTI_ADDICT_DAILY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_LABORATORY, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_GUILD_EXCHANGECHAIR, EVARTIMETYPE_DAY, 0, true);
  registerVar(EVARTYPE_GUILD_CONTRIBUTION, EVARTIMETYPE_WEEK, 0, true);
  registerVar(EVARTYPE_USEITEM_DAY, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_QUEST_DAILY_1, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_QUEST_DAILY_3, EVARTIMETYPE_DAY, 86400 * 3 + offsetSec, false);
  registerVar(EVARTYPE_QUEST_DAILY_7, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_QUEST_DAILY_MAP, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_ASSET, EVARTIMETYPE_WEEK, 0, false);
  registerVar(EVARTYPE_CAMERA_SUMMON_DAILY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACTIVITY_QUEST, EVARTIMETYPE_DAY, 3600*12, false);
  registerVar(EVARTYPE_GUILD_DONATE, EVARTIMETYPE_WEEK, 0, false);
  registerVar(EVARTYPE_OPERATE_REWARD, EVARTIMETYPE_ALWAYS, 0, false);
  registerVar(EVARTYPE_SINGLE_DOG, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_QUEST_REWARD, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_FIRST_EXCHANGEZONE, EVARTIMETYPE_ALWAYS, 0, true);
  registerVar(EVARTYPE_FRIENDSHIP_FRIEND, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_FRIENDSHIP_GUILD, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_TOWER_MONSTER, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_USEITEM_WEEK, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_USER_CREDIT, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAY_ONLINE_FIRST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACTIVITY_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_CHAT, EVARTIMETYPE_WEEK, 0, false);
  registerVar(EVARTYPE_AUGURY_REWARD, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_SELL_WARNING_LAST, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_SELL_WARNING_CUR, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_GUILD_RAID, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_GUILD_RAID_BAN, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_LABORATORY_EXTASKREWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_QUEST, EVARTIMETYPE_ALWAYS, 0, false);
  registerVar(EVARTYPE_CHILD_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_MAXASSET, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_NEWAUGURY_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_NEWAUGURY_EXTRACOUNT, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_LABORATORY_POINT, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GETITEM_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GETITEM_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_PVPCOIN_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_PVPCOIN_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_QUEST_DAILY_RESET, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACHIEVE_CHAT_WORLD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACHIEVE_CHAT_GUILD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_AUGURY_CELEBRATION_DAY, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_CELEBRATION_ONE, EVARTIMETYPE_ALWAYS, offsetSec, false);
  registerVar(EVARTYPE_CELEBRATION_TWO, EVARTIMETYPE_ALWAYS, offsetSec, false);
  registerVar(EVARTYPE_CELEBRATION_THREE, EVARTIMETYPE_ALWAYS, offsetSec, false);
  registerVar(EVARTYPE_GUILD_QUEST_EXTRAREWARD, EVARTIMETYPE_ALWAYS, 0, false);
  registerVar(EVARTYPE_GUILD_DONATE_EXTRAREWARD, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_SCENERY_UPLOAD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACHIEVE_CAT, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_USERPET_TOUCH, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_USERPET_FEED, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_USERPET_GIFT, EVARTIMETYPE_DAY, offsetSec, false);
  //registerVar(EVARTYPE_PETADVENTURE_RESET, EVARTIMETYPE_WEEK, offsetSec, false);
  //registerVar(EVARTYPE_PETADVENTURE_LIST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_SHOP_RANDOM, EVARTIMETYPE_DAY, offsetSec, false); 
  registerVar(EVARTYPE_ACC_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_LOTTERY_CNT_EQUIP, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_LOTTERY_CNT_CARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACCDAILY_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_SHOP_RANDOM_BY_LV, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_TUTOR_TASK_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_TUTOR_TASK_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_TUTOR_TASK_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_PHOTO_LOAD, EVARTIMETYPE_ALWAYS, offsetSec, false);
  registerVar(EVARTYPE_DOUBLE_LAB, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ACC_CHOICE_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DEPOSIT_END_NTF, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_CHRISTMAS_CAKE, EVARTIMETYPE_DAY, offsetSec, false);

  registerVar(EVARTYPE_ACTIVITY_EVENT_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_LABORATORY_COUNT, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_DONATE_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAILY_MAPRAND, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_TOWER_RESETTIME, EVARTIMETYPE_WEEK, offsetSec, false);  
  registerVar(EVARTYPE_GUILD_BUILDING_SUBMIT_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_CHALLENGE_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_SHOP_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_SHOP_MONTH, EVARTIMETYPE_WEEK, offsetSec, false);

  registerVar(EVARTYPE_QUEST_SIGN, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_FIRST_SHARE, EVARTIMETYPE_ALWAYS, 0, true);
  registerVar(EVARTYPE_DAY_LOTTERY_BUY_GIVE_CNT, EVARTIMETYPE_DAY, offsetSec, true);

  registerVar(EVARTYPE_EXTRARWD_WANTEDQUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_DAILYMONSTER, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_SEAL, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_LABORATORY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_ENDLESS, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_GUILD_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_GUILD_DONATE, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_EXTRARWD_PVECARD, EVARTIMETYPE_DAY, offsetSec, false);

  registerVar(EVARTYPE_DOUBLERWD_WANTEDQUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DOUBLERWD_DAILYMONSTER, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DOUBLERWD_SEAL, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DOUBLERWD_LABORATORY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DOUBLERWD_ENDLESS, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DOUBLERWD_PVECARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAY_LOTTERY_CNT_HEAD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAY_LOTTERY_CNT_EQUIP, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAY_LOTTERY_CNT_CARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DAY_LOTTERY_CNT_MAGIC, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ARTIFACT_DISTRIBUTE_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_GUILD_TREASURE_COUNT, EVARTIMETYPE_WEEK, 0, false);
  registerVar(EVARTYPE_BCOIN_TREASURE_COUNT, EVARTIMETYPE_WEEK, 0, false);
  registerVar(EVARTYPE_QUEST_WEDDINGDAILY, EVARTIMETYPE_DAY, 0, false);
  registerVar(EVARTYPE_PVECARD_DIFFICULTY_1, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_PVECARD_DIFFICULTY_2, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_PVECARD_DIFFICULTY_3, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_ALTMAN_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_ALTMAN_KILL, EVARTIMETYPE_ALWAYS, 0, false);

  registerVar(EVARTYPE_MVPREWARDNUM, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_MINIREWARDNUM, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_GUILD_MEMBER_WEEKBCOIN, EVARTIMETYPE_WEEK, offsetSec, true);

  registerVar(EVARTYPE_RECOMMEND_DAY, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_RECOMMEND_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_EXCHANGECARD_DRAWMAX, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_DAY_GET_ZENY_COUNT, EVARTIMETYPE_DAY, offsetSec, true);

  // 不使用,保留代码
  registerVar(EVARTYPE_TUTOR_EXTRABATTLETIME, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_DEAD_COIN, EVARTIMETYPE_DAY, offsetSec, true);
  registerVar(EVARTYPE_DEAD_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerVar(EVARTYPE_TEAMPWS_COUNT, EVARTIMETYPE_WEEK, offsetSec, true, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_MVP, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_PVECARD, EVARTIMETYPE_WEEK, offsetSec, false);
  registerVar(EVARTYPE_DEADBOSS_COUNT_TOWER, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_GUILD, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_PVECARD2, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_PVECARD3, EVARTIMETYPE_WEEK, offsetSec, true);
  registerVar(EVARTYPE_DEADBOSS_COUNT_PVECARD4, EVARTIMETYPE_WEEK, offsetSec, true);

  //
  registerAccVar(EACCVARTYPE_LOTTERY_CNT_EQUIP, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_LOTTERY_CNT_CARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EVARTYPE_SHOP_RANDOM_BY_ACCLV, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DAILY_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_INACTIVE_USER_SEND_COUNT, EVARTIMETYPE_WEEK, offsetSec, false);
  registerAccVar(EACCVARTYPE_INACTIVE_USER_LAST_SEND_DAY, EVARTIMETYPE_WEEK, offsetSec, false);
  registerAccVar(EACCVARTYPE_QUEST_WANTED_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLE_LAB, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_ACTIVITY_EVENT_REWARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_POLLY_FIRST, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_POLLY_DAY_SCORE, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_SHOP_RANDOM_ACC, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_SHOP_GOT, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_QUEST_DAILY_RESET, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_QUEST_DAILY_1, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_QUEST_DAILY_3, EVARTIMETYPE_DAY, 86400 * 3 + offsetSec, false);
  registerAccVar(EACCVARTYPE_QUEST_DAILY_7, EVARTIMETYPE_WEEK, offsetSec, false);
  registerAccVar(EACCVARTYPE_JOY, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_SHOP_GOT_WEEK, EVARTIMETYPE_WEEK, offsetSec, false);
  registerAccVar(EACCVARTYPE_SHOP_GOT_MONTH, EVARTIMETYPE_MONTH, offsetSec, false);


  registerAccVar(EACCVARTYPE_EXTRARWD_WANTEDQUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_DAILYMONSTER, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_SEAL, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_LABORATORY, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_ENDLESS, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_GUILD_QUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_GUILD_DONATE, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_EXTRARWD_PVECARD, EVARTIMETYPE_DAY, offsetSec, false);

  registerAccVar(EACCVARTYPE_DOUBLERWD_WANTEDQUEST, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLERWD_DAILYMONSTER, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLERWD_SEAL, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLERWD_LABORATORY, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLERWD_ENDLESS, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_DOUBLERWD_PVECARD, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_PETWORK_EXCHANGE, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_OPERATE_REWARD, EVARTIMETYPE_ALWAYS, offsetSec, false);

  registerAccVar(EACCVARTYPE_FAVORABILITY, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_FAVORABILITY_STATUS, EVARTIMETYPE_DAY, offsetSec, false);
  registerAccVar(EACCVARTYPE_STAY_FAVORABILITY, EVARTIMETYPE_DAY, offsetSec, false);
}


VarManager::~VarManager()
{

}

const SVarInfo* VarManager::getVarInfo(EVarType eType)
{
  if (eType <= EVARTYPE_MIN || eType >= EVARTYPE_MAX)
    return nullptr;

  return &m_stVar[eType];
}

const SAccVarInfo* VarManager::getAccVarInfo(EAccVarType eType)
{
  if (eType <= EACCVARTYPE_MIN || eType >= EACCVARTYPE_MAX)
    return nullptr;

  return &m_stAccVar[eType];
}

void VarManager::registerVar(EVarType eType, EVarTimeType eTimeType, DWORD offset, bool bClient, bool bSession /*=false*/)
{
  if (eType <= EVARTYPE_MIN || eType >= EVARTYPE_MAX)
    return;
  if (eTimeType <= EVARTIMETYPE_MIN || eTimeType >= EVARTIMETYPE_MAX)
    return;

  SVarInfo& rInfo = m_stVar[eType];
  rInfo.eType = eType;
  rInfo.eTimeType = eTimeType;
  rInfo.setTimeOffset(offset);
  rInfo.bClient = bClient;

  if (bSession)
    m_setSessionVars.insert(eType);
}

void VarManager::registerAccVar(EAccVarType eType, EVarTimeType eTimeType, DWORD offset, bool bClient)
{
  if (eType <= EACCVARTYPE_MIN || eType >= EACCVARTYPE_MAX)
    return;
  if (eTimeType <= EVARTIMETYPE_MIN || eTimeType >= EVARTIMETYPE_MAX)
    return;

  SAccVarInfo& rInfo = m_stAccVar[eType];
  rInfo.eType = eType;
  rInfo.eTimeType = eTimeType;
  rInfo.setTimeOffset(offset);
  rInfo.bClient = bClient;
}

// variable
Variable::Variable()
{
  for (int i = 0; i < EVARTYPE_MAX; ++i)
  {
    if (EVarType_IsValid(i) == true)
      m_oVar[i].set_type(static_cast<EVarType>(i));
  }

  for (int i = 0; i < EACCVARTYPE_MAX; ++i)
  {
    if (EAccVarType_IsValid(i) == true)
      m_oAccVar[i].set_type(static_cast<EAccVarType>(i));
  }
}

Variable::~Variable()
{

}

bool Variable::load(const BlobVar& oBlob)
{
  for (int i = 0; i < oBlob.datas_size(); ++i)
  {
    const Var& rVar = oBlob.datas(i);
    if (rVar.type() <= EVARTYPE_MIN || rVar.type() >= EVARTYPE_MAX || EVarType_IsValid(rVar.type()) == false)
    {
    //   XERR << "[Var-加载] type :" << rVar.type() << "不合法" << XEND;
      continue;
    }

    m_oVar[rVar.type()].CopyFrom(oBlob.datas(i));
    getVarValue(rVar.type());
  }

  return true;
}

bool Variable::save(BlobVar* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  for (int i = 0; i < EVARTYPE_MAX; ++i)
  {
    if (EVarType_IsValid(m_oVar[i].type()) == false)
    {
      XERR << "[变量-保存] create var type :" << m_oVar[i].type() << "不合法" << XEND;
      continue;
    }
    Var* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[变量-保存] create var type :" << i << "error" << XEND;
      continue;
    }

    pData->CopyFrom(m_oVar[i]);
  }

  XDBG << "[变量-保存] 数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}

bool Variable::loadAcc(const BlobAccVar& oBlob)
{
  for (int i = 0; i < oBlob.datas_size(); ++i)
  {
    const AccVar& rVar = oBlob.datas(i);
    if (rVar.type() <= EACCVARTYPE_MIN || rVar.type() >= EACCVARTYPE_MAX || EAccVarType_IsValid(rVar.type()) == false)
    {
      //   XERR << "[Var-加载] type :" << rVar.type() << "不合法" << XEND;
      continue;
    }

    m_oAccVar[rVar.type()].CopyFrom(oBlob.datas(i));
    getAccVarValue(rVar.type());
  }

  return true;
}

bool Variable::saveAcc(BlobAccVar* pBlob)
{
  if (pBlob == nullptr)
    return false;
  pBlob->Clear();

  for (int i = 0; i < EACCVARTYPE_MAX; ++i)
  {
    if (EAccVarType_IsValid(m_oAccVar[i].type()) == false)
    {
      XERR << "[变量-保存] create var type :" << m_oVar[i].type() << "不合法" << XEND;
      continue;
    }
    AccVar* pData = pBlob->add_datas();
    if (pData == nullptr)
    {
      XERR << "[变量-保存] acc create var type :" << i << "error" << XEND;
      continue;
    }

    pData->CopyFrom(m_oAccVar[i]);
  }

  XDBG << "[变量-保存] acc 数据大小 :" << pBlob->ByteSize() << XEND;
  return true;
}


DWORD Variable::getVarValue(EVarType eType)
{
  const SVarInfo* pInfo = VarManager::getMe().getVarInfo(eType);
  if (pInfo == nullptr)
    return 0;

  DWORD value = m_oVar[eType].value();
  DWORD recTime = m_oVar[eType].time();
  DWORD nowTime = now();
  switch (pInfo->eTimeType)
  {
    case EVARTIMETYPE_MIN:
      break;
    case EVARTIMETYPE_ALWAYS:
      {
      }
      break;
    case EVARTIMETYPE_DAY:
      {
        //0点更新
        DWORD recTimeZero = xTime::getDayStart(recTime, pInfo->getTimeOffset());
        DWORD nowTimeZero = xTime::getDayStart(nowTime, pInfo->getTimeOffset());
        if (nowTimeZero >= recTimeZero + 86400)
        {
          setVarValue(eType, 0);
        }
      }
      break;
    case EVARTIMETYPE_WEEK:
      {
        DWORD recMondayTimeZero = xTime::getWeekStart(recTime, pInfo->getTimeOffset());
        DWORD nowMondayTimeZero = xTime::getWeekStart(nowTime, pInfo->getTimeOffset());

        if (nowMondayTimeZero >= recMondayTimeZero + 86400 * 7)
        {
          setVarValue(eType, 0);
        }
      }
      break;
    case EVARTIMETYPE_MONTH:
      {
        if (xTime::isSameMonth(recTime, nowTime, pInfo->getTimeOffset()) && nowTime < recTime + 86400 * 32)//同一月份且年份相同
          return value;
        setVarValue(eType, 0);
      }
      break;
    case EVARTIMETYPE_MAX:
      break;
  }

  return m_oVar[eType].value();
}

DWORD Variable::getAccVarValue(EAccVarType eType)
{
  const SAccVarInfo* pInfo = VarManager::getMe().getAccVarInfo(eType);
  if (pInfo == nullptr)
    return 0;

  DWORD value = m_oAccVar[eType].value();
  DWORD recTime = m_oAccVar[eType].time();
  DWORD nowTime = now();
  switch (pInfo->eTimeType)
  {
  case EVARTIMETYPE_MIN:
    break;
  case EVARTIMETYPE_ALWAYS:
  {
  }
  break;
  case EVARTIMETYPE_DAY:
  {
    //0点更新
    DWORD recTimeZero = xTime::getDayStart(recTime, pInfo->getTimeOffset());
    DWORD nowTimeZero = xTime::getDayStart(nowTime, pInfo->getTimeOffset());
    if (nowTimeZero >= recTimeZero + 86400)
    {
      setAccVarValue(eType, 0);
    }
  }
  break;
  case EVARTIMETYPE_WEEK:
  {
    DWORD recMondayTimeZero = xTime::getWeekStart(recTime, pInfo->getTimeOffset());
    DWORD nowMondayTimeZero = xTime::getWeekStart(nowTime, pInfo->getTimeOffset());

    if (nowMondayTimeZero >= recMondayTimeZero + 86400 * 7)
    {
      setAccVarValue(eType, 0);
    }
  }
  break;
  case EVARTIMETYPE_MONTH:
  {
    if (xTime::isSameMonth(recTime, nowTime, pInfo->getTimeOffset()) && nowTime < recTime + 86400 * 32)//同一月份且年份相同
      return value;
    setAccVarValue(eType, 0);
  }
  break;
  case EVARTIMETYPE_MAX:
    break;
  }

  return m_oAccVar[eType].value();
}


void Variable::setVarValue(EVarType eType, DWORD value)
{
  m_oVar[eType].set_value(value);
  m_oVar[eType].set_time(xTime::getCurSec());
  m_setVars.insert(eType);
}

void Variable::setAccVarValue(EAccVarType eType, DWORD value)
{
  m_oAccVar[eType].set_value(value);
  m_oAccVar[eType].set_time(xTime::getCurSec());
}


void Variable::collectVar(VarUpdate& cmd, bool bFull /*= false*/)
{
  if (bFull)
  {
    clearNew();
    for (int i = EVARTYPE_MIN + 1; i < EVARTYPE_MAX; ++i)
    {
      if (m_oVar[i].value() == 0)
        continue;

      Var* pVar = cmd.add_vars();
      if (pVar == nullptr)
        continue;

      pVar->CopyFrom(m_oVar[i]);
    }

    return;
  }

  for (auto &s : m_setVars)
  {
    Var* pVar = cmd.add_vars();
    if (pVar == nullptr)
      continue;

    pVar->CopyFrom(m_oVar[s]);
  }
  clearNew();
}

void Variable::refreshAllVar()
{
  for (int i = EVARTYPE_MIN + 1; i < EVARTYPE_MAX; ++i)
    getVarValue(static_cast<EVarType>(i));
}

DWORD Variable::getVarDayNum(EVarType eType, DWORD dwBeginTime, DWORD dwEndTime)
{
  const SVarInfo* pInfo = VarManager::getMe().getVarInfo(eType);
  if (pInfo == nullptr)
    return 0;
  if (pInfo->eTimeType == EVARTIMETYPE_DAY)
  {
    DWORD beginVarTime = xTime::getDayStart(dwBeginTime, pInfo->getTimeOffset());
    DWORD endVarTime = xTime::getDayStart(dwEndTime, pInfo->getTimeOffset());
    if (endVarTime <= beginVarTime)
      return 0;
    return (endVarTime - beginVarTime) / DAY_T;
  }
  return 0;
}

void Variable::clearExcept(set<EVarType> types)
{
  for (int i = EVARTYPE_MIN + 1; i < EVARTYPE_MAX; ++i)
  {
    if (types.find(static_cast<EVarType>(i)) == types.end())
    {
      m_oVar[i].set_value(0);
      m_oVar[i].set_time(0);
    }
  }
}

void Variable::collectSessionVars(SyncUserVarSessionCmd& cmd)
{
  const std::set<EVarType>& vars = VarManager::getMe().getSessionVars();
  if (vars.empty())
    return;
  for (auto &s : vars)
  {
    getVarValue(s);

    if (m_oVar[s].value() == 0)
      continue;

    Var* pVar = cmd.add_vars();
    if (pVar == nullptr)
      continue;

    pVar->CopyFrom(m_oVar[s]);
  }
}


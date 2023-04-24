#include "DepositConfig.h"
#include "RedisManager.h"
#include "MiscConfig.h"

bool SDeposit::canBuy(DWORD regionId, QWORD charid) const
{
  string key;
  DWORD count;
  return canBuy(regionId, charid, key, count);
}

bool SDeposit::canBuy(DWORD regionId, QWORD charid, string &outKey, DWORD &outCount) const
{
  if (monthLimit == 0)
    return true;

  outKey = RedisManager::getMe().getKeyByParam(regionId, EREDISKEYTYPE_MONTH_CARD, charid, type);
  outCount = 0;
  RedisManager::getMe().getData(outKey, outCount);
  if (outCount >= monthLimit)
  {
    return false;
  }
  return true;
}

DWORD SDeposit::getItemId() const
{  
  switch (type)
  {
  case ETITLE_TYPE_MONEY:
  case ETITLE_TYPE_LOTTERY:
  case ETITLE_TYPE_FUDAI:
    return itemId;
  case ETITLE_TYPE_MONTH:
  {
    DWORD curSec = now() - 5 * 3600;
    DWORD year = xTime::getYear(curSec);
    DWORD month = xTime::getMonth(curSec);
    const SDepositCard* pCfg = DepositConfig::getMe().getDepositCard(type, year, month);
    if (pCfg == nullptr)
    {
      XERR << "[充值-月卡] 找不到配置" << type << year << month << XEND;
      return 0;
    }
    return pCfg->itemId;
  }
  default:
    return itemId;
  }
  XERR << "[充值-月卡] 找不到配置" << type << XEND;
  return 0;
}

DWORD SDeposit::getFromId() const
{
  DWORD fromId = 0;
  DWORD toId = 0;
  if (MiscConfig::getMe().getSDepositMiscCFG().getDiscountPair(id, fromId, toId) == false)
  {
    return id;
  }
  return fromId;
}

DWORD SDeposit::getToId() const
{
  DWORD fromId = 0;
  DWORD toId = 0;
  if (MiscConfig::getMe().getSDepositMiscCFG().getDiscountPair(id, fromId, toId) == false)
  {
    return id;
  }
  return toId;
}

// config
DepositConfig::DepositConfig()
{

}

DepositConfig::~DepositConfig()
{

}

bool DepositConfig::loadConfig()
{
  bool bResult = true;
  if (loadDepositConfig() == false)
    bResult = false;
  if (loadDepositCardConfig() == false)
    bResult = false;
  if (loadDepositFuncConfig() == false)
    bResult = false;
  return bResult;
}

bool DepositConfig::checkConfig()
{
  bool bCorrect = true;
  for (auto it = m_mapCardCFG.begin(); it != m_mapCardCFG.end(); ++it)
  {
    const SDepositTypeCFG* pCFG = MiscConfig::getMe().getDepositTypeCFG(static_cast<EDepositCardType>(it->second.type));
    if (!pCFG)
    {
      XERR << "[充值-功能配置] 加载配置Table_MonthCard.txt失败, type"<< it->second.type <<"在GameConfig 中找不到。" << XEND;
      bCorrect = false;
      continue;
    }
    it->second.typeCfg = *pCFG;

    for (auto &v : pCFG->vecFuns)
    {
      SDepositFunc*p =  getSDepositFunc(v);
      if (!p)
      {
        XERR << "[充值-功能配置] 加载配置Table_DepositFunction.txt失败, type" << it->second.type << "的功能"<<v<< "不能在 Table_DepositFunction.txt中找到" << XEND;
        bCorrect = false;
        continue;
      }
    }
  }
  
  return bCorrect;
}

bool DepositConfig::loadDepositConfig()
{
  m_mapDepositCFG.clear();
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_Deposit.txt"))
  {
    XERR << "[充值-配置] 加载配置Table_Deposit.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Deposit", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SDeposit stCFG;
    stCFG.id = m->first;
    stCFG.productId = m->second.getTableString("ProductID");
    stCFG.rmb = m->second.getTableFloat("Rmb");
    stCFG.cardVersion = m->second.getTableInt("CardVersion");
    stCFG.itemId = m->second.getTableInt("ItemId");
    stCFG.count = m->second.getTableInt("Count");
    stCFG.count2 = m->second.getTableInt("Count2");
    stCFG.count3 = m->second.getTableInt("Count3");
    stCFG.type = static_cast<EDepositCardType>(m->second.getTableInt("Type"));
    stCFG.monthLimit = m->second.getTableInt("MonthLimit");
    stCFG.mailId = m->second.getTableInt("MailID");
    stCFG.npcId = m->second.getTableInt("NpcID");
    stCFG.duration = m->second.getTableInt("Duration");
    stCFG.limitType = static_cast<ELimitType>(m->second.getTableInt("LimitType"));
    stCFG.strDbColumn = m->second.getTableString("DbColumn");
    stCFG.dwActivityDiscount = m->second.getTableInt("ActivityDiscount");

    stCFG.quota = m->second.getTableInt("Quota");

    stCFG.virginTag = m->second.getTableInt("VirginTag");
    stCFG.virginCount = m->second.getTableInt("VirginCount");

    if ((stCFG.limitType == ELimitType_Account || stCFG.limitType == ELimitType_Char) && stCFG.strDbColumn.empty())
    {
      bCorrect = false;
      XERR << "[充值-配置] Table_Deposit.txt,严重错误， 限购商品没有数据库字段配置 id" << stCFG.id << XEND;
      continue;
    }

    if (!stCFG.strDbColumn.empty())
    {
      if (m_mapName2Id.find(stCFG.strDbColumn) != m_mapName2Id.end())
      {
        bCorrect = false;
        XERR << "[充值-配置] Table_Deposit.txt,严重错误， 数据库字段重复 id"<<stCFG.id <<stCFG.strDbColumn << XEND;
        continue;
      }
      m_mapName2Id[stCFG.strDbColumn] = stCFG.id;
    }
    m_mapDepositCFG[m->first] = stCFG;
  }

  if (bCorrect)
    XLOG << "[充值-配置] 成功加载配置Table_Deposit.txt" << XEND;
  return bCorrect;
}

bool DepositConfig::loadDepositCardConfig()
{
  m_mapCardCFG.clear();
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_MonthCard.txt"))
  {
    XERR << "[充值-卡片配置] 加载配置Table_MonthCard.txt失败" << XEND;
    return false;
  }

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_MonthCard", table);
  m_mapCardCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SDepositCard stCFG;
    stCFG.itemId = m->first;
    stCFG.type = static_cast<EDepositCardType>(m->second.getTableInt("Type"));
    stCFG.year = m->second.getTableInt("Year");
    stCFG.month = m->second.getTableInt("Month");
    stCFG.headdress = m->second.getTableInt("HeadDress");
    m_mapCardCFG[m->first] = stCFG;
  }

  if(bCorrect)
    XLOG << "[充值-卡片配置] 成功加载配置Table_MonthCard.txt" << XEND;
  return bCorrect;
}

bool DepositConfig::loadDepositFuncConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_DepositFunction.txt"))
  {
    XERR << "[充值-功能配置] 加载配置Table_DepositFunction.txt失败" << XEND;
    return false;
  }

  m_mapFuncCFG.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_DepositFunction", table);
  m_mapFuncCFG.clear();
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SDepositFunc stCFG;
    stCFG.id = m->first;
    stCFG.type = static_cast<EFuncType>(m->second.getTableInt("Type"));
    //if (stCFG.type == EFuncType_NONE)
    //  continue;

    xLuaData& rData = m->second.getMutableData("Argument");
    std::string type = rData.getTableString("type");
    if (type == "add")
      stCFG.oper = EFunOper_ADD;
    else if (type == "multiply")
      stCFG.oper = EFunOper_MULTIPLY;
    else if (type == "open")
      stCFG.oper = EFunOper_OPEN;
    else
      stCFG.oper = EFunOper_NONE;
      
    stCFG.param = rData.getMutableData("param");

    stCFG.desc = m->second.getTableInt("Desc");
    stCFG.msgid = m->second.getTableInt("SysMsg");
    m_mapFuncCFG[m->first] = stCFG;
  }

  if (bCorrect)
    XLOG << "[充值-功能配置] 成功加载配置Table_DepositFunction.txt" << XEND;
  return bCorrect;
}

bool DepositConfig::hasFunc(EDepositCardType cardType, EFuncType funcType)
{
  const SDepositTypeCFG* pCfg = MiscConfig::getMe().getDepositTypeCFG(cardType);
  if (!pCfg)
    return false;
    
  for (auto &v : pCfg->vecFuns)
  {
    const SDepositFunc*pCfg = getSDepositFunc(v);
    if (!pCfg)
      continue;
    if (pCfg->type == funcType)
      return true;
  }  
  return false;
}

const SDepositCard* DepositConfig::getDepositCard(EDepositCardType type, DWORD year, DWORD month) const
{
  for (auto &v : m_mapCardCFG)
  {
    if (v.second.type == type && v.second.year == year && v.second.month == month)
    {
      return &(v.second);
    }
  }
  return nullptr;
}

void DepositConfig::getVirginList(std::map<DWORD, DWORD>& map_virgin)
{
  for(auto &v : m_mapDepositCFG)
  {
    if(0 != v.second.virginTag)
      map_virgin[v.first] = v.second.virginTag;
  }
}


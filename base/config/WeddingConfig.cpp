#include "WeddingConfig.h"
#include "xLuaTable.h"
#include "xlib/xSha1.h"

WeddingConfig::WeddingConfig()
{
}

WeddingConfig::~WeddingConfig()
{
}

bool WeddingConfig::loadConfig()
{
  bool bCorrect = true;

  if (loadWeddingCFG() == false)
    bCorrect = false;
  if (loadWeddingService() == false)
    bCorrect = false;

  return bCorrect;
}

bool WeddingConfig::checkConfig()
{
  bool bCorrect = true;

  return bCorrect;
}

const SWeddingServiceCFG* WeddingConfig::getWeddingServiceCFG(DWORD id)
{
  auto it = m_mapID2Service.find(id);
  if (it == m_mapID2Service.end())
    return nullptr;
  return &it->second;
}

bool WeddingConfig::getPackagePrice(DWORD& price, DWORD itemid, DWORD packageid, const TSetDWORD& gotpackages/* = TSetDWORD()*/)
{
  if (!itemid)
    return false;
  const SWeddingServiceCFG* cfg = getWeddingServiceCFG(packageid);
  if (cfg == nullptr || cfg->eType != EWEDDINGSERVICE_PACKAGE || cfg->setServiceID.size() <= 0)
    return false;
  TSetDWORD gotids;
  for (auto id : gotpackages)
  {
    const SWeddingServiceCFG* c = getWeddingServiceCFG(id);
    if (c == nullptr || c->eType != EWEDDINGSERVICE_PACKAGE)
      return false;
    for (auto sid : c->setServiceID)
      gotids.insert(sid);
  }
  price = 0;
  for (auto id : cfg->setServiceID)
  {
    if (gotids.find(id) == gotids.end())
    {
      const SWeddingServiceCFG* c = getWeddingServiceCFG(id);
      if (c == nullptr || c->eType != EWEDDINGSERVICE_PLAN)
        return false;
      DWORD pr = c->getPrice(itemid);
      if (pr <= 0)
        return false;
      price += pr;
    }
  }
  return true;
}

bool WeddingConfig::isPackageFree(DWORD id)
{
  const SWeddingServiceCFG* cfg = getWeddingServiceCFG(id);
  if (cfg == nullptr || cfg->eType != EWEDDINGSERVICE_PACKAGE || cfg->setServiceID.size() <= 0)
    return false;
  for (auto id : cfg->setServiceID)
  {
    const SWeddingServiceCFG* c = getWeddingServiceCFG(id);
    if (c == nullptr || c->eType != EWEDDINGSERVICE_PLAN || !c->mapPrice.empty())
      return false;
  }
  return true;
}

bool WeddingConfig::loadWeddingCFG()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_Wedding.txt"))
  {
    XERR << "[婚礼配置-时间配置] 加载配置Table_Wedding.txt失败" << XEND;
    return false;
  }

  m_mapId2Wedding.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_Wedding", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SWeddingCFG cfg;
    cfg.dwId = m->second.getTableInt("id");
    cfg.m_dwStartHour = m->second.getTableInt("starttime");
    cfg.m_dwEndHour = m->second.getTableInt("endtime");
    cfg.m_dwPrice = m->second.getTableInt("price");
    m_mapId2Wedding[cfg.dwId] = cfg;
  }

  if (bCorrect)
    XLOG << "[婚礼配置-时间配置] 成功加载配置Table_Wedding.txt" << XEND;
  return bCorrect;
}

bool WeddingConfig::loadWeddingService()
{
  bool bCorrect = true;

  if (!xLuaTable::getMe().open("Lua/Table/Table_WeddingService.txt"))
  {
    XERR << "[婚礼配置-婚礼服务] 加载配置Table_WeddingService.txt失败" << XEND;
    return false;
  }

  m_mapID2Service.clear();
  m_setFreePackageID.clear();

  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_WeddingService", table);
  for (auto m = table.begin(); m != table.end(); ++m)
  {
    SWeddingServiceCFG cfg;

    cfg.dwID = m->second.getTableInt("id");
    DWORD type = m->second.getTableInt("Type");
    if (type <= EWEDDINGSERVICE_MIN || type >= EWEDDINGSERVICE_MAX)
    {
      XERR << "[婚礼配置-婚礼服务] id:" << cfg.dwID << "type:" << type << "type非法" << XEND;
      continue;
    }
    cfg.eType = static_cast<EWeddingServiceType>(type);
    cfg.strName = m->second.getTableString("Name");

    auto pricef = [&](const string& key, xLuaData& data)
    {
      DWORD itemid = data.getTableInt("id");
      DWORD itemcount = data.getTableInt("num");
      if (itemid && itemcount)
        cfg.mapPrice[itemid] = itemcount;
    };
    m->second.getMutableData("Price").foreach(pricef);

    if (cfg.eType == EWEDDINGSERVICE_PACKAGE)
    {
      auto servicef = [&](const string& key, xLuaData& data)
      {
        DWORD id = data.getInt();
        if (id)
          cfg.setServiceID.insert(id);
      };
      m->second.getMutableData("Service").foreach(servicef);
    }

    cfg.vecEffectGMData.clear();
    cfg.vecSuccessGMData.clear();
    DWORD tmptype = 0;
    auto geteffect = [&](const string& k, xLuaData& d)
    {
      if (d.has("type"))
      {
        if (tmptype == 1)
        {
          cfg.vecEffectGMData.push_back(d);
          cfg.vecEffectGMData.rbegin()->setData("wedding", 1);
        }
        else if (tmptype == 2)
        {
          cfg.vecSuccessGMData.push_back(d);
          cfg.vecSuccessGMData.rbegin()->setData("wedding", 1);
        }
      }
    };
    tmptype = 1;
    m->second.getMutableData("Effect").foreach(geteffect);
    tmptype = 2;
    m->second.getMutableData("SuccessEffect").foreach(geteffect);


    m_mapID2Service[cfg.dwID] = cfg;
  }

  for (auto& v : m_mapID2Service)
  {
    if (v.second.eType != EWEDDINGSERVICE_PACKAGE)
      continue;
    if (isPackageFree(v.first))
    {
      m_setFreePackageID.insert(v.first);
    }
    for (auto& m : m_mapID2Service)
    {
      if (m.first == v.first || m.second.eType != EWEDDINGSERVICE_PACKAGE || m.second.setServiceID.empty())
        continue;
      bool issub = true;
      for (auto& id : m.second.setServiceID)
      {
        if (v.second.setServiceID.find(id) == v.second.setServiceID.end())
        {
          issub = false;
          break;
        }
      }
      if (issub)
        v.second.setSubPackageID.insert(m.first);
    }
  }

  if (bCorrect)
    XLOG << "[婚礼配置-婚礼服务] 成功加载配置Table_WeddingService.txt" << XEND;
  return bCorrect;
}

string WeddingConfig::getSign(QWORD qwCharid, DWORD dwDate, DWORD dwConfigId, DWORD dwTime, bool bTicket, DWORD dwZoneId)
{
  std::stringstream ss;
  ss << qwCharid << "_" << dwDate << "_" << dwConfigId << "_" << dwTime <<"_" <<bTicket <<"_"<<dwZoneId << "rt13yuio@#$%^&";
  char sha1[SHA1_LEN + 1];
  bzero(sha1, sizeof(sha1));
  getSha1Result(sha1, ss.str().c_str(), ss.str().size());
  return string(sha1);
}

DWORD WeddingConfig::getUnixTime(DWORD dwDate, DWORD hour)
{
  return (dwDate + (hour * HOUR_T));
}

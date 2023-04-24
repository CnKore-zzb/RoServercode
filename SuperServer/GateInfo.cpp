#include "GateInfo.h"
#include "xNetProcessor.h"
#include "SuperServer.h"
#include "RedisManager.h"

GateInfoM::GateInfoM()
{
}

GateInfoM::~GateInfoM()
{
}

bool GateInfoM::init()
{
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, thisServer->getZoneID());
  RedisManager::getMe().delData(key);

  // string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, "online");
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "gateinfo");
  if (field)
  {
    char where[1024];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid=%u", thisServer->getZoneID());

    QWORD id = thisServer->getDBConnPool().exeDelete(field, where);
    if (id!=QWORD_MAX)
    {
      XLOG << "[网关信息],删除,zone:" << thisServer->getZoneID() << XEND;
    }
    else
    {
      XERR << "[网关信息],删除失败,zone:" << thisServer->getZoneID() << XEND;
    }
  }
  return true;
}

void GateInfoM::addGate(xNetProcessor *np)
{
  if (!np) return;

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "gateinfo");
  if (field)
  {
    xRecord record(field);
    record.put("zoneid", thisServer->getZoneID());
    record.put("ip", inet_addr(thisServer->m_oServerConfig["GateServer"][np->name].ip.c_str()));
    record.put("ipstr", thisServer->m_oServerConfig["GateServer"][np->name].ip);
    DWORD port = thisServer->m_oServerConfig["GateServer"][np->name].port;
    record.put("port", port);
    record.put("onlinenum", 0);

    QWORD ret = thisServer->getDBConnPool().exeInsert(record, true, true);
    if (QWORD_MAX != ret)
    {
      XLOG << "[网关信息],添加,zone:" << record.get<DWORD>("zoneid") << ",ip:" << record.get<DWORD>("ip") << ",port:" << port << XEND;
      std::stringstream stream;
      stream.str("");
      stream << record.get<DWORD>("ip") << ":" << port;
      m_oKeyList[port] = stream.str();
      XLOG << "[网关信息],添加,key:" << port << stream.str() << XEND;

      xLuaData data;
      data.setData(m_oKeyList[port], 0);

      string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, thisServer->getZoneID());
      RedisManager::getMe().setHash(key, data);

    }
    else
    {
      XERR << "[网关信息],添加失败,zone:" << record.get<DWORD>("zoneid") << ",ip:" << record.get<DWORD>("ip") << ",port:" << port << XEND;
    }
  }
}

void GateInfoM::delGate(xNetProcessor *np)
{
  if (!np) return;


  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "gateinfo");
  if (field)
  {
    DWORD ip = inet_addr(thisServer->m_oServerConfig["GateServer"][np->name].ip.c_str());
    DWORD port = thisServer->m_oServerConfig["GateServer"][np->name].port;

    xLuaData data;
    data.setData(m_oKeyList[port], 0);
    string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, thisServer->getZoneID());
    RedisManager::getMe().delHash(key, data);

    char where[1024];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "ip=%u AND port=%u", ip, port);

    m_oKeyList.erase(port);

    QWORD id = thisServer->getDBConnPool().exeDelete(field, where);
    if (id!=QWORD_MAX)
    {
      XLOG << "[网关信息],删除,zone:" << thisServer->getZoneID() << ",ip:" << ip << ",port:" << port << XEND;
    }
    else
    {
      XERR << "[网关信息],删除失败,zone:" << thisServer->getZoneID() << ",ip:" << ip << ",port:" << port << XEND;
    }
  }

}

void GateInfoM::delAll()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "gateinfo");
  if (field)
  {
    char where[1024];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "zoneid=%u", thisServer->getZoneID());

    m_oKeyList.clear();

    QWORD id = thisServer->getDBConnPool().exeDelete(field, where);
    if (id!=QWORD_MAX)
    {
      XLOG << "[网关信息],清空,zone:" << thisServer->getZoneID() << ",ret:" << id << XEND;
    }
    else
    {
      XERR << "[网关信息],清空失败,zone:" <<  thisServer->getZoneID() << XEND;
    }
  }
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, thisServer->getZoneID());
  RedisManager::getMe().delData(key);
}

void GateInfoM::updateUserNum(xNetProcessor* np, WORD num)
{
  auto it = thisServer->m_oServerConfig.find("GateServer");
  if (it == thisServer->m_oServerConfig.end()) return;

  auto iter = it->second.find(np->name);
  if (iter == it->second.end()) return;

  DWORD ip = inet_addr(iter->second.ip.c_str());
  DWORD port = iter->second.port;
  if (!ip || !port) return;

  /*
  num = num / 10 * 10;
  DWORD dwNum = 0;
  auto p_iter = m_list.find(port);
  if (p_iter != m_list.end())
  {
    dwNum = p_iter->second;
  }

  if (num == dwNum)
  {
    return;
  }

  m_list[port] = num;
  */

  xLuaData data;
  data.setData(m_oKeyList[port], num);

  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_GATE_INFO, thisServer->getZoneID());
  if (RedisManager::getMe().setHash(key, data) == false)
  {
    XERR << "[GateInfo]" << m_oKeyList[port] << "保存失败" << XEND;
  }
  else
  {
    XLOG << "[网关信息]" << "在线人数,zone:" << thisServer->getZoneID() << ",ip:" << inet_ntoa(*((in_addr *)&ip)) << ",port:" << port << ",num:" << num << XEND;
  }
}

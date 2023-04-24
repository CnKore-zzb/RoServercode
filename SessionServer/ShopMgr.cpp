#include "ShopMgr.h"
#include "TableManager.h"
#include "SessionServer.h"
#include "MiscConfig.h"
#include "ServerTask.h"

ShopMgr::ShopMgr()
{
}

ShopMgr::~ShopMgr()
{
}

void ShopMgr::load()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "shop");
  if (!field)
    return;

  char where[1024];
  bzero(where, sizeof(where));
  snprintf(where, sizeof(where), "zoneid=%u", thisServer->getZoneID());

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (1 != ret) return;

  m_dwRefreshTime = set[0].get<DWORD>("refresh");
  std::string data((const char *)set[0].getBin("data"), set[0].getBinSize("data"));
  SessionShopData blobData;
  if (!data.empty())
    blobData.ParseFromString(data);
  
  for (int i = 0; i < blobData.old_size(); ++i)
  {
    m_vecOldGroupId.push_back(blobData.old(i));
  }
  
  m_curGoupId = blobData.now();

  XLOG <<"[SHOP] 加载，上次刷新时间" <<m_dwRefreshTime << XEND;
}

void ShopMgr::save()
{
  if (m_curGoupId == 0)
    return;

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "shop");
  if (!field)
    return;
  
  xRecord record(field);
  record.put("zoneid", thisServer->getZoneID());
  record.put("refresh", m_dwRefreshTime);
  SessionShopData blobData;
  for (auto& v: m_vecOldGroupId)
  {
    blobData.add_old(v);
  }
  blobData.set_now(m_curGoupId);

  std::string strData;
  blobData.SerializeToString(&strData);
  if (!strData.empty())
    record.putBin("data", (unsigned char*)strData.c_str(), strData.size());
  
  QWORD ret = thisServer->getDBConnPool().exeInsert(record, true, true);
  
  XLOG << "[SHOP] 保存，刷新时间" << m_dwRefreshTime << "groupid" << m_curGoupId << "ret" << ret << XEND;
}

void ShopMgr::refresh(DWORD curSec)
{
  DWORD weekTime = xTime::getWeekStart(curSec - 5 * 24 * 3600);
  if (m_dwRefreshTime && (m_dwRefreshTime == weekTime))
  {
    return;
  }
  
  while (m_vecOldGroupId.size() > 4)
  {
    m_vecOldGroupId.erase(m_vecOldGroupId.begin());
  }
  
  const SShopCFG& rCFG = MiscConfig::getMe().getShopCFG();

  DWORD newGroupId = 0;

  TMapGroupId2Rand mapRand(rCFG.mapWeekShopRand.begin(), rCFG.mapWeekShopRand.end());
  for (auto& v : m_vecOldGroupId)
  {
    mapRand.erase(v);
  }
  if (mapRand.empty())
  {
    // XERR << "[SHOP] 随机，找不到新配置了" << XEND;
    return;
  }
  DWORD totalWeight = 0;
  for (auto&m : mapRand)
  {
    totalWeight += m.second.dwWeight;
  }
  srand(weekTime);
  DWORD v = randBetween(1, totalWeight);
  srand(xTime::getCurUSec());

  DWORD offset = 0;
  for (auto& m : mapRand)
  {
    if (v <= offset + m.second.dwWeight)
    {
      newGroupId = m.second.dwGroupId;
      break;
    }
    offset += m.second.dwWeight;
  }

  if (newGroupId == 0)
  {
    XERR << "[SHOP] 随机，出错" << XEND;
    return;
  }  
  m_vecOldGroupId.push_back(m_curGoupId);
  DWORD old = m_curGoupId;
  m_curGoupId = newGroupId;
  DWORD oldTime = m_dwRefreshTime;
  m_dwRefreshTime = weekTime;
  XLOG << "[SHOP] 刷新成功，当前时间" << curSec << "周时间" << weekTime <<"上次刷新时间" << oldTime << "刷新结果" << m_curGoupId << "之前的结果" << old <<"随机出来的值"<< v <<"最大区间"<< totalWeight << XEND;
  save();
  syncToSceneServer();
}

void ShopMgr::timeTick(DWORD curSec)
{
  refresh(curSec);
}

void ShopMgr::syncToSceneServer(ServerTask *task/* = nullptr*/)
{
  if (!m_curGoupId)
    return;

  SyncShopSessionCmd cmd;  
  cmd.set_item(m_curGoupId);
 
  PROTOBUF(cmd, send, len);
  if (task)
    task->sendCmd(send, len);
  else
    thisServer->broadcastScene((unsigned char *)send, len);

  XLOG << "[SHOP] 同步数据给场景服, groupid" << m_curGoupId <<"刷新时间"<< m_dwRefreshTime <<"全场景"<<task<< XEND;
}

void ShopMgr::sceneStart(ServerTask *task)
{
  if (!task) return;
  
  syncToSceneServer(task);
}

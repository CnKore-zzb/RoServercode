#include "TowerManager.h"
#include "xDBFields.h"
#include "SessionServer.h"
#include "RecordCmd.pb.h"

// tower manager
TowerManager::TowerManager()
{

}

TowerManager::~TowerManager()
{

}

bool TowerManager::loadDataFromDB()
{
  // load from db
  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "globaldata");
  if (nullptr == pField)
  {
    XERR << "[恩德勒斯塔管理-加载] 获取数据库失败" << XEND;
    return false;
  }

  // query data
  xRecordSet rSet;
  char szWhere[64] = {0};
  snprintf(szWhere, 64, "name=\'tower\' and zoneid=%u", thisServer->getZoneID());
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, rSet, szWhere);
  if (retNum == QWORD_MAX)
  {
    XERR << "[恩德勒斯塔管理-加载] 查询失败" << XEND;
    return false;
  }
  if (retNum == 0)
  {
    xRecord record(pField);
    record.put("zoneid", thisServer->getZoneID());
    record.putString("name", "tower");

    QWORD ret = thisServer->getDBConnPool().exeInsert(record);
    if (ret == QWORD_MAX)
    {
      XERR << "[恩德勒斯塔管理-加载] 插入失败" << XEND;
      return false;
    }
    retNum = thisServer->getDBConnPool().exeSelect(pField, rSet, szWhere);
    if (retNum == QWORD_MAX)
    {
      XERR << "[恩德勒斯塔管理-加载] 查询失败" << XEND;
      return false;
    }
  }

  // assign data
  string data;
  data.assign((const char *)rSet[0].getBin("data"), rSet[0].getBinSize("data"));

  BlobGlobalTower cmd;
  if (cmd.ParseFromString(data) == false)
  {
    XERR << "[恩德勒斯塔管理-加载] 数据序列化失败" << XEND;
    return false;
  }
  if (m_oVar.load(cmd.var()) == false)
  {
    XERR << "[恩德勒斯塔管理-加载] 数据var反序列化失败" << XEND;
    return false;
  }

  XLOG << "[恩德勒斯塔管理-加载] 加载成功" << XEND;
  m_bInit = TowerConfig::getMe().initDataFromDB(cmd.info());
  return m_bInit;
}

void TowerManager::syncScene(xNetProcessor* pTask /*= nullptr*/)
{
  SceneTowerUpdate cmd;
  toSceneData(cmd.mutable_info());
  PROTOBUF(cmd, send, len);
  if (pTask != nullptr)
    pTask->sendCmd(send, len);
  else
    thisServer->sendCmdToAllScene(send, len);

  XLOG << "[恩德勒斯塔管理-同步] 同步最大层数 " << cmd.info().maxlayer() << " 到场景" << XEND;
}

void TowerManager::saveData()
{
  xField *pField = thisServer->getDBConnPool().getField(REGION_DB, "globaldata");
  if (nullptr == pField)
  {
    XERR << "[恩德勒斯塔管理-保存] 获取数据库失败" << XEND;
    return;
  }

  string str;
  if (toDataString(str) == false)
  {
    XERR << "[恩德勒斯塔管理-保存] 数据序列化失败" << XEND;
    return;
  }

  xRecord record(pField);
  record.put("zoneid", thisServer->getZoneID());
  record.put("name", "tower");
  record.putBin("data", (unsigned char *)str.c_str(), str.size());
  QWORD ret = thisServer->getDBConnPool().exeReplace(record);
  if (ret == QWORD_MAX)
  {
    XERR << "[恩德勒斯塔管理-保存] 保存失败" << XEND;
    return;
  }

  XLOG << "[恩德勒斯塔管理-保存] 保存成功 str :" << str.size() << XEND;
}

void TowerManager::timer(DWORD curTime)
{
  if (!m_bInit)
    return;

  if (m_oVar.getVarValue(EVARTYPE_TOWER_MONSTER) != 0)
    return;
  /*if (curTime > 0)
  {
    if (TowerConfig::getMe().getClearTime() > curTime)
      return;
  }*/

  TowerConfig::getMe().generateMonster();
  syncScene();
  saveData();
  m_oVar.setVarValue(EVARTYPE_TOWER_MONSTER, 1);
}

void TowerManager::toSceneData(TowerInfo* pInfo)
{
  if (pInfo == nullptr)
    return;

  //pInfo->set_cleartime(TowerConfig::getMe().getClearTime());
  pInfo->set_maxlayer(TowerConfig::getMe().getMaxLayer());

  const set<DWORD>& setList = TowerConfig::getMe().getKillMonsterList();
  for (auto s = setList.begin(); s != setList.end(); ++s)
    pInfo->add_killmonsters(*s);

  const TMapTowerLayerCFG& mapList = TowerConfig::getMe().getTowerConfigList();
  for (auto m = mapList.begin(); m != mapList.end(); ++m)
  {
    TowerLayer* pLayer = pInfo->add_layers();
    m->second.toData(pLayer);
  }
}

bool TowerManager::toDataString(string& str)
{
  BlobGlobalTower oBlob;

  oBlob.mutable_info()->set_maxlayer(TowerConfig::getMe().getMaxLayer());
  //oBlob.mutable_info()->set_cleartime(TowerConfig::getMe().getClearTime());

  const set<DWORD>& setList = TowerConfig::getMe().getKillMonsterList();
  for (auto s = setList.begin(); s != setList.end(); ++s)
    oBlob.mutable_info()->add_killmonsters(*s);

  const TMapTowerLayerCFG& mapList = TowerConfig::getMe().getTowerConfigList();
  for (auto m = mapList.begin(); m != mapList.end(); ++m)
  {
    TowerLayer* pLayer = oBlob.mutable_info()->add_layers();
    m->second.toData(pLayer);
  }

  m_oVar.save(oBlob.mutable_var());

  XLOG << "TowerManager::toDataString layer :" << mapList.size() << XEND;
  return oBlob.SerializeToString(&str);
}


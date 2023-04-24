#include "RecordTool.h"
#include "xTools.h"
#include "xLuaTable.h"
#include "xDBConnPool.h"
#include "xDBFields.h"
#include "RecordServer.h"
#include "RecordUserManager.h"
#include "RecordCmd.pb.h"
#include "SceneItem.pb.h"

#include "iostream"
using namespace std;

SItemInfo* SPackInfo::getItemInfo(QWORD id)
{
  TMapItemIter it = m_mapItem.find(id);
  if(m_mapItem.end() == it)
    return nullptr;
  return &(it->second);
}

void SPackInfo::addItemInfo(SItemInfo& info)
{
  SItemInfo* pInfo = getItemInfo(info.m_qwItem);
  if(nullptr == pInfo)
    m_mapItem[info.m_qwItem] = info;
  else
    pInfo->m_dwCount += info.m_dwCount;
}

RecordTool::RecordTool()
{
}

RecordTool::~RecordTool()
{
}

bool RecordTool::loadConfig()
{
  if(!xLuaTable::getMe().open("tool.lua"))
  {
    XERR<<"[RecordTool], 加载配置tool.lua失败"<<XEND;
    return false;
  }

  xLuaData data;
  xLuaTable::getMe().getLuaData("ParamPack", data);
  m_qwActiveTime = data.getTableInt("active_time");
  m_bStore = data.getTableInt("has_store");

  char cSplit = ',';
  string items;
  items = data.getTableString("item_list");
  stringToNum<TVecQWORD>(items, &cSplit, m_vecItem);

  string types;
  types = data.getTableString("except_type");
  stringToNum<TVecDWORD>(types, &cSplit, m_vecExceptType);

  return true;
}

bool RecordTool::init()
{
  if(!loadConfig()) return false;
  if(!loadData()) return false;

  log();
  return true;
}

bool RecordTool::loadStore(QWORD accid)
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
  if(nullptr == field) return false;

  char where[256] = {0};
  snprintf(where, sizeof(where), "accid = %llu", accid);

  xRecordSet set;
  QWORD num = thisServer->getDBConnPool().exeSelect(field, set, (const char*)where, NULL);
  if(QWORD_MAX == num || 0 == num) return false;
  for(QWORD i=0; i<num; ++i)
  {
    string item;
    item.assign((const char*)set[i].getBin("item"), set[i].getBinSize("item"));

    ItemData data;
    data.ParseFromString(item);
    if(checkItem(data.base().id()))
    {
      SItemInfo info;
      info.m_qwAcc = accid;
      info.m_qwItem = data.base().id();
      info.m_dwCount = data.base().count();

      addAccPack(info);
    }
  }

  return true;
}

bool RecordTool::loadData()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if(nullptr == field) return false;

  DWORD cur = now();
  char where[256] = {0};
  snprintf(where, sizeof(where), "offlinetime > %llu", cur - m_qwActiveTime);

  xRecordSet set;
  QWORD num = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
  if(QWORD_MAX == num || 0 == num) return false;


  TVecQWORD vecAcc;
  auto addacc = [&](QWORD acc)
  {
    if(find(vecAcc.begin(), vecAcc.end(), acc) == vecAcc.end())
      vecAcc.push_back(acc);
  };

  for(QWORD i=0; i<num; ++i)
  {
    string oBlob;
    oBlob.assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));
    QWORD accid = set[i].get<QWORD>("accid");
    QWORD charid = set[i].get<QWORD>("charid");

    addacc(accid);
    loadPack(oBlob, charid, accid);
  }

  if(m_bStore)
  {
    for(auto &it : vecAcc)
    {
      loadStore(it);
    }
  }

  return true;
}

bool RecordTool::loadPack(const string& oBlob, QWORD charid, QWORD accid)
{
  string blob;
  uncompress(oBlob, blob);

  BlobData data;
  data.ParseFromString(blob);

  const BlobPack& pack = data.pack();
  for(int i=0; i<pack.datas_size(); ++i)
  {
    const PackageData& rData = pack.datas(i);
    if(!checkType((DWORD)rData.type())) continue;

    for(int j=0; j<rData.items_size(); ++j)
    {
      const ItemData iData = rData.items(j);
      if(checkItem(iData.base().id()))
      {
        SItemInfo info;
        info.m_qwChar = charid;
        info.m_qwAcc = accid;
        info.m_qwItem = iData.base().id();
        info.m_dwCount = iData.base().count();

        addCharPack(info);
      }
    }
  }

  return true;
}

SPackInfo* RecordTool::getCharPack(QWORD charid)
{
  TMapPackIter it = m_mapCharPack.find(charid);
  if(m_mapCharPack.end() == it)
    return nullptr;
  return &(it->second);
}

SPackInfo* RecordTool::getAccPack(QWORD accid)
{
  TMapPackIter it = m_mapAccPack.find(accid);
  if(m_mapAccPack.end() == it)
    return nullptr;
  return &(it->second);
}

void RecordTool::addCharPack(SItemInfo& info)
{
  SPackInfo* pInfo = getCharPack(info.m_qwChar);
  if(nullptr == pInfo)
  {
    SPackInfo sPackInfo;
    sPackInfo.addItemInfo(info);
    m_mapCharPack[info.m_qwChar] = sPackInfo;
  }
  else
    pInfo->addItemInfo(info);
}

void RecordTool::addAccPack(SItemInfo& info)
{
  SPackInfo* pInfo = getAccPack(info.m_qwAcc);
  if(nullptr == pInfo)
  {
    SPackInfo sPackInfo;
    sPackInfo.addItemInfo(info);
    m_mapAccPack[info.m_qwAcc] = sPackInfo;
  }
  else
    pInfo->addItemInfo(info);
}

void RecordTool::log()
{
  for(auto &it : m_mapAccPack)
  {
    it.second.log();
  }

  for(auto &it : m_mapCharPack)
  {
    it.second.log();
  }
}





#pragma once

#include "xSingleton.h"
#include "RecordCmd.pb.h"
#include "UserData.h"
#include <vector>
#include <map>

struct SItemInfo
{
  QWORD m_qwChar = 0;
  QWORD m_qwAcc = 0;
  QWORD m_qwItem = 0;
  DWORD m_dwCount = 0;

  void log()
  {
    XLOG<<"[RecordTool]"<<m_qwAcc<<m_qwChar<<m_qwItem<<m_dwCount<<XEND;
  }
};

typedef std::map<QWORD, SItemInfo> TMapItem;
typedef std::map<QWORD, SItemInfo>::iterator TMapItemIter;
struct SPackInfo
{
  public:
    SItemInfo* getItemInfo(QWORD id);
    void addItemInfo(SItemInfo& info);
    void log() { for(auto it : m_mapItem) it.second.log();}

  private:
    TMapItem m_mapItem; // <itemid, info>
};

typedef std::map<QWORD, SPackInfo> TMapPack;
typedef std::map<QWORD, SPackInfo>::iterator TMapPackIter;
class RecordTool : public xSingleton<RecordTool>
{
  friend class xSingleton<RecordTool>;
  private:
    RecordTool();
  public:
    virtual ~RecordTool();

  public:
    bool init();

  private:
    bool loadConfig();
    bool loadData();
    bool loadStore(QWORD accid);
    bool loadPack(const string& oBlob, QWORD charid, QWORD accid);
    void log();

  private:
    bool m_bStore = true;
    QWORD m_qwActiveTime = 0;
    TVecQWORD m_vecItem;
    TVecDWORD m_vecExceptType;
    TMapPack m_mapCharPack; // <charid, pack>
    TMapPack m_mapAccPack; // <accid, pack>
  private:
    bool checkItem(QWORD item)  {return std::find(m_vecItem.begin(),m_vecItem.end(),item) != m_vecItem.end();}
    bool checkType(DWORD type) {return std::find(m_vecExceptType.begin(),m_vecExceptType.end(), type) == m_vecExceptType.end();}
    SPackInfo* getCharPack(QWORD charid);
    SPackInfo* getAccPack(QWORD accid);
    void addCharPack(SItemInfo& info);
    void addAccPack(SItemInfo& info);
};

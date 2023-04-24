#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "GameStruct.h"
#include "GuildCmd.pb.h"
#include "SysMsg.pb.h"
#include "ItemConfig.h"
#include <list>

class Guild;
using namespace Cmd;
using std::map;
using std::string;

struct SGuildItem
{
  ItemData oData;
  const SItemCFG* pCFG = nullptr;

  const string& getGUID() const { return oData.base().guid(); }

  DWORD getCreateTime() const { return oData.base().createtime(); }
  void setCreateTime(DWORD dwNow) { oData.mutable_base()->set_createtime(dwNow); }
  DWORD getCount() const { return oData.base().count(); }
  void setCount(DWORD dwCount) { oData.mutable_base()->set_count(dwCount); }
  DWORD getItemID() const { return oData.base().id(); }
};

class GuildPack : private xNoncopyable
{
  public:
    GuildPack(Guild *pGuild);
    virtual ~GuildPack();

  public:
    void toData(QueryPackGuildCmd& cmd);

    void fetch(const TSetString& setIDs, PackUpdateGuildCmd& cmd);
    void update(bool bNoCache = false);

    string getGUIDByType(DWORD dwTypeID);
    DWORD getItemCount(DWORD dwTypeID);
    SGuildItem* getItem(const string& guid);

    ESysMsgID addItem(const ItemInfo& rInfo);
    ESysMsgID addItem(const TVecItemInfo& vecInfo);
    ESysMsgID addItem(const ItemData& rData);
    ESysMsgID addItem(const TVecItemData& vecData);

    ESysMsgID reduceItem(const string& guid, DWORD dwCount = 1, ESource eSource = ESOURCE_NORMAL);
    ESysMsgID reduceItem(DWORD dwItemID, DWORD dwCount = 1, ESource eSource = ESOURCE_NORMAL);
    ESysMsgID reduceItem(const TVecItemInfo& vecItems, ESource eSource);
    ESysMsgID reduceItem(const TVecItemData& vecDatas, ESource eSource);

    void itemLog(DWORD dwItemID, SQWORD sqwChanged, DWORD dwCount, ESource eSource);
  public:
    bool toBlobPackString(std::string& str);

  public:
    bool isInit() { return m_dwInitStatus == GUILD_BLOB_INIT_OK; }
    void init();
  private:
    DWORD m_dwInitStatus = GUILD_BLOB_INIT_NULL;

  public:
    void setBlobPackString(const char* str, DWORD len);
  private:
    std::string m_oBlobPack;

  private:
    bool fromPackString(const std::string& str);

  public:
    map<string, SGuildItem> m_mapID2Item;
    map<DWORD, TSetString> m_mapType2Item;

    TSetString m_setUpdateIDs;
  private:
    Guild *m_pGuild = nullptr;
};


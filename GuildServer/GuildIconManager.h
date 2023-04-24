#pragma once

#include "xSingleton.h"
#include "Guild.h"
#include <string.h>

typedef std::map<DWORD, IconInfo> TMapII;

struct SGuildII
{
  public:
    SGuildII() { m_mapII.clear(); }
    ~SGuildII() { m_mapII.clear(); }

  public:
    void add(DWORD dwIndex, DWORD dwTimestamp, std::string strType, EIconState eState = EICON_INIT, bool isRead = false);
    bool remove(DWORD dwIndex);
    void setState(QWORD qwGuildId, DWORD dwIndex, EIconState eState);
    void setState(QWORD qwGuildId, EIconState eState);
    void getData(GuildIconSyncGuildCmd& cmd);
    bool hasIndex(DWORD dwIndex);
    bool checkState(DWORD dwIndex, DWORD dwTime, std::string strType = "");
    bool hasUnread();
    void setRead(QWORD qwGuildId, bool isRead = true);

  private:
    TMapII m_mapII;
};

typedef std::map<QWORD, SGuildII> TMapGII;

class GuildIconManager : public xSingleton<GuildIconManager>
{
  friend xSingleton<GuildIconManager>;

  public:
    GuildIconManager() { m_mapGII.clear(); }
    virtual ~GuildIconManager() { m_mapGII.clear(); }

  public:
    bool init();

  public:
    void add(QWORD qwGuildId, QWORD qwCharId, DWORD dwIndex, std::string strType);
    bool remove(QWORD qwGuildId, DWORD dwIndex);
    void clearGuild(QWORD qwGuildId);
    void setState(QWORD qwGuildId, DWORD dwIndex, EIconState eState);
    void setState(QWORD qwGuildId, EIconState eState);
    bool hasIndex(QWORD qwGuildId, DWORD dwIndex);
    bool checkPortrait(QWORD qwGuildId, const std::string& strPortrait);
    void setRead(QWORD qwGuildId, bool isRead = true);
    void readIndex(QWORD qwGuild, DWORD dwIndex, DWORD dwRead);
    bool hasUnread(QWORD qwGuildId);

  public:
    void syncData(QWORD qwGuildId, GMember* pMember);
    void onUserOnline(QWORD qwGuildId, GMember* pMember);

  private:
    bool load();
    bool insert(QWORD qwGuildId, QWORD qwCharId, DWORD dwIndex, std::string strType);
    bool update(QWORD qwGuildId, DWORD dwIndex, DWORD dwRead);
    bool del(QWORD qwGuildId, DWORD dwIndex = 0);

  private:
    TMapGII m_mapGII;
};


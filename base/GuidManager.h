#pragma once

#include "xSingleton.h"
#include "xUniqueIDManager.h"

using std::string;
const DWORD ERRNUM = -1;
const DWORD GUIDPARAM = 100000000;

class GuidManager : public xSingleton<GuidManager>
{
  friend class xSingleton<GuidManager>;
  private:
    GuidManager();
  public:
    ~GuidManager() {}

    string newGuidStr(DWORD zoneID, DWORD sceneID);

    DWORD getNextNpcID();
    DWORD getNextSceneItemID();
    DWORD getNextBroadID();
    DWORD getNextChatRoomID();
    DWORD getNextInterID();
    DWORD getNextActID();
    DWORD getNextTrapID();
    //DWORD getNextVoiceID();
    DWORD getNextRaidID();
    DWORD getNextEventID();
    DWORD getNextLetterID();
    DWORD getNextGuildGmSessionID();

    void addUsedEventID(DWORD dwID) { m_oEventIDManager.pushUniqueID(dwID); }
    void removeUsedEventID(DWORD dwID) { m_oEventIDManager.putUniqueID(dwID); }
  private:
    inline DWORD getIndex(const string& guidStr)
    {
      DWORD m_index = 0;
      DWORD intTemp = 0;
      if(sscanf(guidStr.c_str(), "%d-%d-%d-%d", &m_index, &intTemp, &intTemp, &intTemp) == 4)
        return m_index;
      return ERRNUM;
    }

    inline DWORD getZoneID(const string& guidStr)
    {
      DWORD m_zoneID = 0;
      DWORD intTemp = 0;

      if(sscanf(guidStr.c_str(), "%d-%d-%d-%d", &intTemp, &m_zoneID, &intTemp, &intTemp) == 4)
        return m_zoneID;
      return ERRNUM;
    }

    inline DWORD getMapID(const string& guidStr)
    {
      DWORD m_mapID = 0;
      DWORD intTemp = 0;
      if(sscanf(guidStr.c_str(), "%d-%d-%d-%d", &intTemp, &intTemp, &m_mapID, &intTemp) == 4)
        return m_mapID;
      return ERRNUM;
    }

    inline DWORD getTime(const string& guidStr)
    {
      DWORD m_time = 0;
      DWORD intTemp = 0;
      if(sscanf(guidStr.c_str(), "%d-%d-%d-%d", &intTemp, &intTemp, &intTemp, &m_time) == 4)
        return m_time;
      return ERRNUM;
    }

  private:
    struct IndexM
    {
      DWORD lastCreateStaticEntryTime = 0;
      WORD lastCreateStaticEntryIndex = 0;
      IndexM() {}
    };
    typedef std::map<DWORD,IndexM> ZoneIndexM;

    ZoneIndexM zoneIndexM;
    UniqueDWORDIDManager m_oTempIDManager;
    UniqueDWORDIDManager m_oRaidIDManager;
    UniqueDWORDIDManager m_oChatIDManager;
    UniqueDWORDIDManager m_oEventIDManager;
};


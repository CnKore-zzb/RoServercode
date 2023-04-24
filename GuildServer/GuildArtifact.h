#pragma once

#include "xNoncopyable.h"
#include "GuildConfig.h"

using std::map;

class Guild;
class GMember;

struct SGuildArtifactItem
{
  string strGuid;
  DWORD dwItemID = 0;
  DWORD dwDistributeCount = 0;
  DWORD dwRetrieveTime = 0;
  QWORD qwOwnerID = 0;

  bool toData(GuildArtifactItem* data);
  bool fromData(const GuildArtifactItem& data);

  void giveback()
  {
    dwRetrieveTime = 0;
    qwOwnerID = 0;
  }
};

struct SGuildArtifactData
{
  DWORD dwType = 0;
  DWORD dwProduceCount = 0;
  DWORD dwMaxLevel = 0;

  bool toData(GuildArtifactData* data);
  bool fromData(const GuildArtifactData& data);
};

class GuildArtifactMgr : private xNoncopyable
{
public:
  GuildArtifactMgr(Guild* guild);
  virtual ~GuildArtifactMgr();

  bool toData(GuildArtifact* data);
  bool fromData(const GuildArtifact& data);
  bool toGuildInfo(GuildInfo* data);

  void timer(DWORD cur);

  DWORD getArtifactCount(EGuildBuilding type);
  DWORD getArtifactCountByType(DWORD type, const set<string>& except = set<string>{});
  SGuildArtifactItem* getArtifact(const string& guid);
  SGuildArtifactData* getArtifactData(DWORD type);

  bool addArtifact(DWORD itemid);
  void removeArtifact(const set<string>& guids);
  bool getProduceMaterial(DWORD itemid, TVecItemInfo& materials);
  bool produce(GMember* member, DWORD itemid, bool noreduceitem = false);
  bool operate(GMember* member, EArtifactOptType type, const set<string>& guids, QWORD charid);
  bool distribute(GMember* member, const set<string>& guid, QWORD targetid);
  void retrieve(bool force = false);
  bool retrieve(GMember* member, const set<string>& guid);
  bool cancelRetrieve(GMember* member, const set<string>& guid);
  bool giveback(GMember* member, const set<string>& guid);
  void notifyAllData(GMember* member);

  void clearDistributeCount();

  void onRemoveMember(GMember* member);

  void fixPack(bool force = false);

private:
  void resetDistributeCount();
  void updateArtifactItem(const set<string>& guids);
  void getUnusedArtifact(DWORD itemid, DWORD count, vector<string>& guids);
  void giveback(SGuildArtifactItem* item);

  Guild* m_pGuild = nullptr;

  map<string, SGuildArtifactItem> m_mapGuid2Artifact;
  map<DWORD, SGuildArtifactData> m_mapType2ArtifactData;
  xTimer m_oTenMinTimer;
  bool m_bFixed = false;
};

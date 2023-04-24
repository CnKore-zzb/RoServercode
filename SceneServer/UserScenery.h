#pragma once

#include <set>
#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "GuildConfig.h"

class SceneUser;
namespace Cmd
{
  class BlobScenery;
  class Scenery;
};

struct SceneryData
{
  DWORD m_dwSceneryID = 0;

  DWORD m_dwVisited = 0;
  DWORD m_dwUpload = 0;
  DWORD m_dwMapID = 0;
  DWORD m_dwAngleZ = 0;
  DWORD m_dwTime = 0;
};

enum ESceneryType
{
  ESCENERYTYPE_USER = 3, // 玩家获得特定buff后成为一个景点
};

class UserScenery : private xNoncopyable
{
  public:
    UserScenery(SceneUser *u);
    ~UserScenery();

  public:
    void save(Cmd::BlobScenery *acc_data);
    void load(const Cmd::BlobScenery &acc_data);

    void send(DWORD mapid);
    void sendunsolved();
    void add(DWORD sceneryid);
    void photo(const Cmd::Scenery& rScenery);
    void upload(const UploadOkSceneryUserCmd& cmd);

    void setData(DWORD sceneryid, DWORD visit, DWORD upload, DWORD anglez, DWORD time);
    DWORD count() const { return m_list.size(); }

    const SceneryData* getScenery(DWORD dwSceneryID) const;
    const std::map<DWORD, SceneryData>& getList() const { return m_list; }

    void collectPhoto(QueryShowPhotoGuildSCmd& cmd);
  private:
    EGuildFrameType getFrameType(DWORD dwAngleZ) const;
  public:
    SceneUser *m_pUser = nullptr;

  private:
    std::map<DWORD, SceneryData> m_list;
};

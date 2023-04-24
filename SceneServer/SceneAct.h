/**
* @file SceneAct.h
* @brief
* @author gengshengjie, gengshengjie@xindong.com
* @version v1
* @date 2015-09-10
*/
#pragma once

#include "xSceneEntry.h"
#include "SceneMap.pb.h"

namespace Cmd
{
  class MapAct;
};

using namespace Cmd;
class Scene;
class SceneUser;
class SceneTeam;
class SceneActBase : public xSceneEntry
{
  public:
    SceneActBase(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master);
    virtual ~SceneActBase();

    virtual SCENE_ENTRY_TYPE getEntryType() const { return SCENE_ENTRY_ACT; }

    virtual void delMeToNine();
    virtual void sendMeToNine();

    virtual void sendMeToUser(SceneUser* pUser);
    void sendMeToUser(QWORD userid);
    void delMeToUser(SceneUser* pUser);
    void delMeToUser(QWORD userid);

    virtual bool viewByUser(QWORD userid) { return true; }
    virtual void timer(DWORD cur);

    bool enterScene(Scene* scene);
    void fillMapActData(Cmd::MapAct* data);

    xSceneEntryDynamic* getMaster();
    EActType getActType() { return m_dwType; }
    void setClearState();
    bool needDel() { return m_blNeedClear; }

    bool checkUserInRange(SceneUser* user);
    void getRangeUser(xSceneEntrySet& uset);
    DWORD getRange() const { return m_dwRange; }
    void setClearTime(DWORD time) { m_dwClearTime = time; }

  protected:
    QWORD m_dwMasterID = 0;
    DWORD m_dwRange = 0;
    EActType m_dwType = EACTTYPE_MIN;
    bool m_blNeedClear = false;
    DWORD m_dwClearTime = 0;

  private:
    void leaveScene();
};

class SceneActPurify : public SceneActBase
{
  public:
    SceneActPurify(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master);
    virtual ~SceneActPurify();

    virtual void sendMeToNine();
    virtual bool viewByUser(QWORD userid);
  public:
    void init();
    bool purifyByUser(QWORD userid);
    void setPrivateUser(QWORD userid);
  private:
    TVecQWORD m_vecPurifyUserIDs;
};

class SceneActSeal : public SceneActBase
{
  public:
    SceneActSeal(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master);
    virtual ~SceneActSeal();

    virtual void delMeToNine();
    virtual void sendMeToNine();

    virtual bool viewByUser(QWORD userid);

    //SceneTeam* getTeam();
    void setTeam(QWORD guid);
    void setPrivateUser(SceneUser* pUser);
  private:
    QWORD m_qwTeamID = 0;
    QWORD m_qwUserID = 0;
    //SceneTeam* m_pTeam = nullptr;
    //SceneUser* m_pUserPrivate = nullptr;
};

class SceneActMusic : public SceneActBase
{
  public:
    SceneActMusic(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master);
    virtual ~SceneActMusic();
};

//场景特效
class SceneActEffect : public SceneActBase
{
public:
  SceneActEffect(QWORD tempid, Scene *scene, DWORD range, xPos pos, QWORD master);
  virtual ~SceneActEffect();

  virtual void delMeToNine();
  virtual void sendMeToNine();
  virtual void timer(DWORD cur);
  
  virtual void sendMeToUser(SceneUser* pUser);
  void setEffectInfo(string& path, DWORD duration, QWORD ownerId, DWORD dwDir);
  QWORD getOwnerID() { return m_qwOwnerId; }

private:
  DWORD m_dwExpireTime = 0;
  string m_strPath;
  QWORD m_qwOwnerId = 0;
  DWORD m_dwDir = 0;
  Cmd::EffectUserCmd m_oCmd;
};

enum SceneEventType
{
  SCENE_EVENT_MIN = 0,
  SCENE_EVENT_BGM = 1,
  SCENE_EVENT_SKY = 2,
  SCENE_EVENT_WEATHER = 3,
  SCENE_EVENT_FRAME = 4,
};

// 场景事件
class SceneActEvent : public SceneActBase
{
  public:
    SceneActEvent(QWORD tempid, Scene* scene, DWORD range, xPos pos, QWORD master);
    virtual ~SceneActEvent();

    virtual void delMeToNine();
    virtual void sendMeToNine();

    void onUserIn(SceneUser* user);
    void onUserOut(SceneUser* user);

    void setBgm(const char* bgm) { m_eEventType = SCENE_EVENT_BGM; m_strBgm = bgm; }
    void setSky(DWORD skyid) { m_eEventType = SCENE_EVENT_SKY; m_dwSkyID = skyid; }
    void setWeather(DWORD weatherid) { m_eEventType = SCENE_EVENT_WEATHER; m_dwWeatherID = weatherid; }
    void setFrame() { m_eEventType = SCENE_EVENT_FRAME; }

    void setEventType(SceneEventType eType) { m_eEventType = eType; }
    SceneEventType getEventType() const { return m_eEventType; }
  private:
    void sendExtraDataToNine();
    void removeExtraDataToNine();
  private:
    SceneEventType m_eEventType = SCENE_EVENT_MIN;

    string m_strBgm;
    DWORD m_dwSkyID = 0;
    DWORD m_dwWeatherID = 0;
};


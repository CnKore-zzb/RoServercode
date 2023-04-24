#pragma once

#include "xDefine.h"
#include <set>
#include <list>
#include "xPos.h"
#include "SceneDefine.h"

using std::vector;

class SceneNpc;
class xSceneEntryDynamic;
namespace Cmd
{
  //class BlobUserData;
  class BlobFollower;
};

struct SFollow
{
  DWORD nameID = 0;
  float spdRatio = 1.0f;
  DWORD behaviours = 0;
  DWORD questid = 0;
  DWORD clearTime = 0;
};

struct SFollowTemp
{
  QWORD id = 0;
  float spdRatio = 1.0f;
  DWORD questid = 0;
  DWORD clearTime = 0;
};

struct SServantData
{
  QWORD qwTempID = 0;
  bool bSupply = false;
  bool bShapreDam = false;
  bool bSaperateLock = false;
};
typedef std::vector<SServantData> TVecServantData;

class Follower
{
  public:
    Follower(xSceneEntryDynamic *e);
    ~Follower();

  public:
    void add(const SFollow& sfData);
    void del(QWORD tempid);
    void delquest(DWORD dwQuestID);
    void clear();
    void get(std::list<SceneNpc *> &list);

    void addServant(NpcDefine def, SServantData& data, DWORD num = 1, QWORD priAtkUser = 0);
    bool hasServant() const { return !m_oVecServants.empty(); }

    const TVecServantData& getServant() const { return m_oVecServants; }
    void getServantIDs(TVecQWORD& vecIDs);
    void onServantIDChange(QWORD oldguid, QWORD newguid);
    void removeServant(QWORD guid);

    void addTreeMonster(SceneNpc* pNpc) { m_setTreeMonster.insert(pNpc); }
    void removeTreeMonster(SceneNpc* pNpc);
    void removeAllTreeMonster();
    bool isTreeEmpty() { return m_setTreeMonster.empty(); }
    const std::set<SceneNpc*>& getTreeMonster() { return m_setTreeMonster; }

    void save(Cmd::BlobFollower *data);
    void load(const Cmd::BlobFollower &data);

    void enterScene();
    void leaveScene();

    void goTo(xPos p);
    void moveToMaster(xPos dest);
    void setLockByMaster(QWORD id);

    float getMoveSpeed(QWORD id);

    void refreshMe();
  private:
    xSceneEntryDynamic *m_pEntry;

  public:
    //std::set<QWORD> m_oSetNpcTempID;
    //std::list<DWORD> m_oSetNpcID;
    std::list<SFollowTemp> m_oListTempData;
    std::list<SFollow> m_oListData;
    TVecServantData m_oVecServants;

    std::set<SceneNpc*> m_setTreeMonster;
};


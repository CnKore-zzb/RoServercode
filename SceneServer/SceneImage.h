#pragma once
#include "xDefine.h"
#include "xPos.h"
#include "xDefine.h"
#include "ProtoCommon.pb.h"

using namespace Cmd;
using std::vector;

class Scene;
class DScene;
class SceneUser;
class SceneNpc;

struct ImageItem
{
  DWORD dwRaidID = 0;
  xPos oCenterPos;
  DWORD dwRange = 0;

  DWORD dwSealConfig = 0;
  bool bImage = false;
  std::vector<SceneNpc*> m_vecNpcs;
  DWORD dwNpcId = 0;    //绝对领域的特效npc
};

typedef vector<ImageItem> TVecImageItem;

class SceneImage
{
  public:
    SceneImage(Scene *s);
    ~SceneImage();

  public:
    static bool isImages(DWORD lRealMapID, DWORD lGuid, DWORD rRealMapID, DWORD rGuid)
    {
      if (!lGuid || !rGuid || !lRealMapID || !rRealMapID) return false;
      if (lGuid == rGuid) return false;
      if (lRealMapID!=lGuid && rRealMapID!=rGuid) return false;
      if (lRealMapID == rRealMapID) return true;
      return false;
    }

  public:
    void timer(DWORD dwCurTime);
    bool add(DWORD raidid, SceneUser *user, xPos center, DWORD range, DWORD sealid = 0, DWORD npcId = 0);
    void check(SceneUser *user);
    void checkTeam(SceneUser *user);

    void setRaidData(xPos &p, DWORD range, QWORD index)
    {
      m_oRaidData.oCenterPos = p;
      m_oRaidData.dwRange = range;
      m_oRaidData.bImage = true;
      m_qwImageIndex = index;
    }
    void del(QWORD guid, DWORD raidid);
    void goRealMap(SceneUser* pUser);
    bool isImageScene() { return m_oRaidData.bImage; }
    QWORD getImageIndex() const { return m_qwImageIndex; }

    void onImageClose(const ImageItem& imageItem);
    bool userCanEnter(SceneUser* user);

  private:
    Scene *m_pScene = nullptr;

    // static map data
    std::map<QWORD, ImageItem> m_oPrivateList;
    std::map<QWORD, ImageItem> m_oUserTeamList;
    std::map<QWORD, TVecImageItem> m_oTeamList;
    // raid map data
    ImageItem m_oRaidData;

    QWORD m_qwImageIndex = 0;

    std::map<QWORD, DWORD> m_mapUser2Outtime;
};

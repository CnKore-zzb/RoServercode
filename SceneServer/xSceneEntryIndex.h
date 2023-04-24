#ifndef _XSCENE_ENTRY_INDEX
#define _XSCENE_ENTRY_INDEX

#include "xDefine.h"
#include "xEntry.h"
#include <map>
#include <set>
#include <algorithm>
#include "SceneDefine.h"
#include "UserCmd.h"
#include "SceneStruct.h"
#include "xSceneEntry.h"
#include "GatewayCmd.h"
#include "xSceneEntryScope.h"

#define SCREEN_X_GRID 15
#define SCREEN_Z_GRID 15

class SceneUser;
class xSceneEntry;
class xSceneEntryDynamic;
class SceneNpc;

typedef std::vector<SceneNpc *> VecSceneNpc;

struct MapRange
{
  MapRange()
  {
    bzero(this, sizeof(*this));
  }
  int xMax;
  int xMin;
  int yMax;
  int yMin;
  int zMax;
  int zMin;
};

struct NoFlyRange
{
  float x = 0.0f;
  float width = 0.0f;
  float z = 0.0f;
  float length = 0.0f;

  bool isNoFly(const xPos& pos) const { return pos.x > x - width / 2.0f && pos.x < x + width / 2.0f && pos.z > z - length / 2.0f && pos.z < z + length / 2.0f; }
};

struct MapInfo
{
  DWORD id;
  char name[MAX_NAMESIZE];
  char file[MAX_NAMESIZE];
  MapRange range;
  std::vector<NoFlyRange> vecNoFly;
  MapInfo()
  {
    id = 0;
    bzero(name, sizeof(name));
    bzero(file, sizeof(file));
  }
  bool isNoFly(const xPos& pos) const
  {
    for (auto &v : vecNoFly)
    {
      if (v.isNoFly(pos) == true)
        return true;
    }
    return false;
  }
};

// 地图基本信息
struct MapDefine
{
  MapDefine()
  {
  }

  void init(MapRange &r, bool noScreen=false)
  {
    range = r;

    xLength = range.xMax - range.xMin;
    zLength = range.zMax - range.zMin;

    if (!xLength) xLength = 1;
    if (!zLength) zLength = 1;

    if (noScreen)
    {
      xScreenGrid = xLength;
      zScreenGrid = zLength;
    }

    xScreenNum = xLength / xScreenGrid;
    if (xLength % xScreenGrid)
      ++xScreenNum;

    zScreenNum = zLength / zScreenGrid;
    if (zLength % zScreenGrid)
      ++zScreenNum;

    screenSize = xScreenNum * zScreenNum;
  }

  // 地图的四个顶点
  MapRange range;

  UINT xLength = 0;
  UINT zLength = 0;

  UINT xScreenNum = 0;
  UINT zScreenNum = 0;

  UINT screenSize = 0;
  UINT xScreenGrid = SCREEN_X_GRID;
  UINT zScreenGrid = SCREEN_Z_GRID;
};

class xSceneEntryCallback;
class xSceneEntryIndex
{
  public:
    xSceneEntryIndex();
    virtual ~xSceneEntryIndex();
    //初始化
    inline bool isValidEntryType(BYTE type) const { return type < SCENE_ENTRY_MAX; }
    void initIndex();

  public:
    inline xPosI xPos2xPosI(xPos pos) const
    {
      return ((int)pos.z - (int)define.range.zMin) / (int)define.zScreenGrid * (int)define.xScreenNum + ((int)pos.x - (int)define.range.xMin) / (int)define.xScreenGrid;
    }

    bool check2PosInNine(xPos p1,xPos p2)
    {
      NineScreen::iterator iter = ninescreen.find(xPos2xPosI(p1));
      if (iter != ninescreen.end())
      {
        return std::find(iter->second.begin(), iter->second.end(), xPos2xPosI(p2)) != iter->second.end();
      }
      return false;
    }

    inline int get_x_max() const
    {
      return define.range.xMax;
    }
    inline int get_x_min() const
    {
      return define.range.xMin;
    }
    inline int get_z_max() const
    {
      return define.range.zMax;
    }
    inline int get_z_min() const
    {
      return define.range.zMin;
    }
    inline DWORD getXScreenNum() const
    {
      return define.xScreenNum;
    }
    inline DWORD getZScreenNum() const
    {
      return define.zScreenNum;
    }
    inline DWORD getScreenSize() const
    {
      return define.screenSize;
    }

    xPos& checkPos(xPos& sourcePos)
    {
      sourcePos.x = sourcePos.x>=get_x_max()?get_x_max():sourcePos.x;
      sourcePos.x = sourcePos.x<=get_x_min()?get_x_min():sourcePos.x;
      sourcePos.z = sourcePos.z>=get_z_max()?get_z_max():sourcePos.z;
      sourcePos.z = sourcePos.z<=get_z_min()?get_z_min():sourcePos.z;
      return sourcePos;
    }

  protected:
    MapDefine define;

  public:
    bool isValidPos(const xPos& pos)const
    {
      return pos.x<=get_x_max() && pos.x>=get_x_min() && pos.z<=get_z_max() && pos.z>=get_z_min();
    }
    inline DWORD getColumn(xPosI p) const { return p % define.xScreenNum + 1; }
    inline DWORD getLine(xPosI p) const { return p / define.xScreenNum + 1; }

    void fillNineScreenMap();
    void fillDirectNineScreen();
    void fillRDirectNineScreen();

    void addFunNpc(xSceneEntryDynamic *);
    void delFunNpc(xSceneEntryDynamic *);
    void sendFunNpc(xSceneEntryDynamic *);
    //遍历user
    void getNineScreen(xPosI posi, std::set<DWORD> &uSet);
    void getEntryListInScreen( SCENE_ENTRY_TYPE entryType, xPosI posi, xSceneEntrySet& uSet);
    void getEntryListInNine( SCENE_ENTRY_TYPE entryType, xPosI posi, xSceneEntrySet& uSet);
    void getEntryListInNine( SCENE_ENTRY_TYPE entryType, xPos upos, xSceneEntrySet& uSet)
    { getEntryListInNine(entryType, xPos2xPosI(checkPos(upos)), uSet); }
    void getAllEntryList( SCENE_ENTRY_TYPE entryType, xSceneEntrySet& uSet);
    void getEntryListInBlock( SCENE_ENTRY_TYPE entryType, xPos upos, float w, float h, xSceneEntrySet &uSet);
    void getEntryListInBlock( SCENE_ENTRY_TYPE entryType, xPos upos, float radius, xSceneEntrySet &uSet);
    void getEntryListInRing( SCENE_ENTRY_TYPE entryType, xPos upos, float minRadius, float maxRadius, xSceneEntrySet &uSet);
    void foreachEntryInBlock(const SCENE_ENTRY_TYPE entryType, xPos upos, const float radius, xSceneEntryCallback& strategy);
    void foreachEntryInBlock(const SCENE_ENTRY_TYPE entryType, xPos upos, const float minRadius, float maxRadius, xSceneEntryCallback& strategy);
    void getEntryList( xPos upos, float radius, xSceneEntrySet &uSet);
    void getEntryList(xPos pos, float w, float h, xSceneEntrySet &set);
    void getNpcVecByUniqueID(DWORD uniqueid, VecSceneNpc &vec);

    void getEntryIn2Pos(const xPos& pos1, const xPos& pos2, float w, xSceneEntrySet &uSet);
    //block
    bool addEntryAtPosI(xSceneEntry* entry);
    void delEntryAtPosI(xSceneEntry* entry, xPosI posi);
    void delEntryAtPosI(xSceneEntry* entry);
    void delEntryAtOldPosI(xSceneEntry* entry);
    inline bool inOneScreen(xPos pos1, xPos pos2) const { return ( xPos2xPosI(pos1)==xPos2xPosI(pos2) ); }
    void changeScreen(xSceneEntryDynamic* entry);
    void changeScreenDelOldEntry(SceneUser *pUser, BYTE dir, xPosI oldPosI, xPosI newPosI);
    void changeScreenAddNewEntry(SceneUser *pUser, BYTE dir, xPosI oldPosI, xPosI newPosI);
    Direction getDirectByPos(const xPos from, const xPos to);
    Direction getDirectByPosI(const xPosI from, const xPosI to);

    void sendCmdToNine(xPos pos, const void* cmd, unsigned short len, GateIndexFilter filter=GateIndexFilter());
    void sendCmdToScreen(const xPosIVector &nine, const void* cmd, unsigned short len, GateIndexFilter filter=GateIndexFilter());
    void delNineToUser(SceneUser* user,xPosI posi);

    inline DWORD height() const { return define.zLength; }
    inline DWORD width() const { return define.xLength; }
    inline DWORD screenX() { return define.xScreenNum; }
    inline DWORD screenZ() { return define.zScreenNum; }

    bool isEmpty()const;
    DWORD getUserNum()const;
    DWORD getNpcNum()const;
    DWORD getNineScreenUserNum(xPosI posi);
    DWORD getNineScreenUserNum(xPos pos)
    {
      return getNineScreenUserNum(xPos2xPosI(checkPos(pos)));
    }

    void getNineScreen(xPos pos, xPosIVector &vec);

  protected:
    DWORD m_dwMapID = 0;
  private:
    void printNineScreen();

    typedef std::map<xPosI, xSceneEntrySet> PosIIndex;
    PosIIndex index[SCENE_ENTRY_MAX];

    typedef std::map<xPosI, xPosIVector> NineScreen;
    NineScreen ninescreen;
    NineScreen direct_ninescreen[9];
    NineScreen rdirect_ninescreen[9];

    std::set<xSceneEntryDynamic *> funNpcIndex;
    //bool directSendCmdToUser;

  public:
    //屏消息

    void delDynamicEntry(SceneUser *pUser, xPosI posi);

    void sendNpcEntry(SceneUser *entry, xPosI posi);

    // 视野
  public:
    xSceneEntryScope m_oScope;
  public:
    void addScope(SceneUser *from, SceneUser *to);
    void removeScope(SceneUser *from, SceneUser *to);
    bool inScope(SceneUser *from, SceneUser *to);
    void refreshScope(SceneUser *from);
    void addScope(SceneUser *from);
    void destroyScope(SceneUser *from);
    void sendCmdToScope(SceneUser *pUser, const void *cmd, DWORD len);
    void sendUserToScope(SceneUser *pUser);
    void sendUserToUser(SceneUser *from, SceneUser *to);
    void hideMeToScope(SceneUser *pUser);
};

class xSceneEntryCallback
{
  public:
    virtual ~xSceneEntryCallback() {};
    virtual bool checkCondition(xSceneEntry* pEntry) { return true; }
    void collect(xSceneEntry* pEntry)
    {
      if (pEntry)
        rs.insert(pEntry);
    }
    bool isEnough() { return rs.size() >= getLimit(); }
    DWORD count() { return rs.size(); }
    void execute()
    {
      for (xSceneEntrySet::iterator it = rs.begin(); it != rs.end(); ++it)
        execImpl(*it);
    }
  protected:
    virtual DWORD getLimit() { return DWORD_MAX; }
    virtual void execImpl(xSceneEntry* pEntry) = 0;
  private:
    xSceneEntrySet rs;
};

#endif

#pragma once

#include <list>
#include "xDefine.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "xPos.h"
#include "Recast.h"

class InputGeom;
class xLuaData;

enum SamplePolyAreas
{
  SAMPLE_POLYAREA_GROUND,
  SAMPLE_POLYAREA_WATER,
  SAMPLE_POLYAREA_ROAD,
  SAMPLE_POLYAREA_DOOR,
  SAMPLE_POLYAREA_GRASS,
  SAMPLE_POLYAREA_JUMP,
};

enum SamplePolyFlags
{
  SAMPLE_POLYFLAGS_WALK   = 0x01,   // Ability to walk (ground, grass, road)
  SAMPLE_POLYFLAGS_SWIM   = 0x02,   // Ability to swim (water).
  SAMPLE_POLYFLAGS_DOOR   = 0x04,   // Ability to move through doors.
  SAMPLE_POLYFLAGS_JUMP   = 0x08,   // Ability to jump.
  SAMPLE_POLYFLAGS_DISABLED = 0x10,   // Disabled polygon
  SAMPLE_POLYFLAGS_ALL    = 0xffff  // All abilities.
};

enum SamplePartitionType
{
  SAMPLE_PARTITION_WATERSHED,
  SAMPLE_PARTITION_MONOTONE,
  SAMPLE_PARTITION_LAYERS,
};

enum ToolMode
{
  TOOLMODE_PATHFIND_FOLLOW,
  TOOLMODE_PATHFIND_STRAIGHT,
  TOOLMODE_PATHFIND_SLICED,
  TOOLMODE_RAYCAST,
  TOOLMODE_DISTANCE_TO_WALL,
  TOOLMODE_FIND_POLYS_IN_CIRCLE,
  TOOLMODE_FIND_POLYS_IN_SHAPE,
  TOOLMODE_FIND_LOCAL_NEIGHBOURHOOD,
};

class PathFindingTile
{
  public:
    PathFindingTile();
    ~PathFindingTile();

  private:
    InputGeom* m_geom = nullptr;
    dtNavMesh* m_navMesh = nullptr;
    dtNavMeshQuery* m_navQuery = nullptr;
    dtQueryFilter m_filter;
    dtStatus m_pathFindStatus = 0;
    //unsigned char m_navMeshDrawFlags;

    void resetCommonSettings();
    float m_cellSize = 0.0f;
    float m_cellHeight = 0.0f;
    float m_agentHeight = 0.0f;
    float m_agentRadius = 0.0f;
    float m_agentMaxClimb = 0.0f;
    float m_agentMaxSlope = 0.0f;
    float m_regionMinSize = 0.0f;
    float m_regionMergeSize = 0.0f;
    float m_edgeMaxLen = 0.0f;
    float m_edgeMaxError = 0.0f;
    float m_vertsPerPoly = 0.0f;
    float m_detailSampleDist = 0.0f;
    float m_detailSampleMaxError = 0.0f;
    int m_partitionType = 0;

    rcContext *m_ctx = nullptr;

  protected:
    bool m_keepInterResults = false;
    bool m_buildAll = true;
    float m_totalBuildTimeMs = 0.0f;

    unsigned char* m_triareas;
    rcHeightfield* m_solid = nullptr;
    rcCompactHeightfield* m_chf = nullptr;
    rcContourSet* m_cset = nullptr;
    rcPolyMesh* m_pmesh = nullptr;
    rcPolyMeshDetail* m_dmesh = nullptr;
    rcConfig m_cfg;

    int m_maxTiles = 0;
    int m_maxPolysPerTile = 0;
    float m_tileSize = 32;

    unsigned int m_tileCol = 0;
    float m_tileBmin[3];
    float m_tileBmax[3];
    float m_tileBuildTime = 0.0f;
    float m_tileMemUsage = 0.0f;
    int m_tileTriCount = 0;

    unsigned char* buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize);

    void cleanup();

  private:
    ToolMode m_toolMode = TOOLMODE_PATHFIND_FOLLOW;

    int m_straightPathOptions = 0;

    static const int MAX_POLYS = 256;
    static const int MAX_SMOOTH = 2048;

    dtPolyRef m_startRef = 0;
    dtPolyRef m_endRef = 0;
    dtPolyRef m_polys[MAX_POLYS];
    dtPolyRef m_parent[MAX_POLYS];
    int m_npolys = 0;
    float m_straightPath[MAX_POLYS*3];
    unsigned char m_straightPathFlags[MAX_POLYS];
    dtPolyRef m_straightPathPolys[MAX_POLYS];
    int m_nstraightPath = 0;
    float m_polyPickExt[3];
    float m_smoothPath[MAX_SMOOTH*3];
    int m_nsmoothPath = 0;
    float m_queryPoly[4*3];

    static const int MAX_RAND_POINTS = 64; 
    //float m_randPoints[MAX_RAND_POINTS*3];
    //int m_nrandPoints;
    //bool m_randPointsInCircle;

    float m_spos[3];
    float m_epos[3];
    float m_hitPos[3];
    float m_hitNormal[3];
    bool m_hitResult = false;
    float m_distanceToWall = 0.0f;
    float m_neighbourhoodRadius = 0.0f;
    float m_randomRadius = 0.0f;

    int m_pathIterNum = 0;
    //dtPolyRef m_pathIterPolys[MAX_POLYS]; 
    //int m_pathIterPolyCount;
    //float m_prevIterPos[3], m_iterPos[3], m_steerPos[3], m_targetPos[3];

    static const int MAX_STEER_POINTS = 10; 
    //float m_steerPoints[MAX_STEER_POINTS*3];
    //int m_steerPointCount;
  public:
    bool init(const xLuaData &mess);
    inline bool isInitOk() { return m_navMesh != nullptr; }
    bool build();

    void saveAll(const char* path, const dtNavMesh* mesh);
    dtNavMesh* loadAll(const char* path);

    void getTilePos(const float* pos, int& tx, int& ty);

    void buildTile(const float* pos);
    void removeTile(const float* pos);
    void buildAllTiles();
    void removeAllTiles();

    bool finding(xPos &start, xPos &end);
    bool finding(xPos &start, xPos &end, std::list<xPos> &outlist, ToolMode mode=TOOLMODE_PATHFIND_STRAIGHT);
    bool findRandomPoint(xPos source, float radius, xPos &out);
    bool getValidPos(xPos source, xPos &out);
    bool resetPosHeight(xPos &source);

    const float* getMeshBoundsMin();
    const float* getMeshBoundsMax();

  public:
    std::string m_strFile;
};

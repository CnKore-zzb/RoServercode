#include "PathFindingTile.h"
#include "InputGeom.h"
#include "DetourNavMeshBuilder.h"
#include "xLuaTable.h"
#include "DetourCommon.h"
#include "SceneServer.h"
#include "BaseConfig.h"

inline unsigned int nextPow2(unsigned int v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

inline unsigned int ilog2(unsigned int v)
{
  unsigned int r;
  unsigned int shift;
  r = (v > 0xffff) << 4; v >>= r;
  shift = (v > 0xff) << 3; v >>= shift; r |= shift;
  shift = (v > 0xf) << 2; v >>= shift; r |= shift;
  shift = (v > 0x3) << 1; v >>= shift; r |= shift;
  r |= (v >> 1);
  return r;
}

static float frand()
{
  //	return ((float)(rand() & 0xffff)/(float)0xffff);
  return (float)rand()/(float)RAND_MAX;
}

inline bool inRange(const float* v1, const float* v2, const float r, const float h)
{
  const float dx = v2[0] - v1[0];
  const float dy = v2[1] - v1[1];
  const float dz = v2[2] - v1[2];
  return (dx*dx + dz*dz) < r*r && fabsf(dy) < h;
}


static int fixupCorridor(dtPolyRef* path, const int npath, const int maxPath,
    const dtPolyRef* visited, const int nvisited)
{
  int furthestPath = -1;
  int furthestVisited = -1;

  // Find furthest common polygon.
  for (int i = npath-1; i >= 0; --i)
  {
    bool found = false;
    for (int j = nvisited-1; j >= 0; --j)
    {
      if (path[i] == visited[j])
      {
        furthestPath = i;
        furthestVisited = j;
        found = true;
      }
    }
    if (found)
      break;
  }

  // If no intersection found just return current path. 
  if (furthestPath == -1 || furthestVisited == -1)
    return npath;

  // Concatenate paths.	

  // Adjust beginning of the buffer to include the visited.
  const int req = nvisited - furthestVisited;
  const int orig = rcMin(furthestPath+1, npath);
  int size = rcMax(0, npath-orig);
  if (req+size > maxPath)
    size = maxPath-req;
  if (size)
    memmove(path+req, path+orig, size*sizeof(dtPolyRef));

  // Store visited
  for (int i = 0; i < req; ++i)
    path[i] = visited[(nvisited-1)-i];				

  return req+size;
}

// This function checks if the path has a small U-turn, that is,
// a polygon further in the path is adjacent to the first polygon
// in the path. If that happens, a shortcut is taken.
// This can happen if the target (T) location is at tile boundary,
// and we're (S) approaching it parallel to the tile edge.
// The choice at the vertex can be arbitrary, 
//  +---+---+
//  |:::|:::|
//  +-S-+-T-+
//  |:::|   | <-- the step can end up in here, resulting U-turn path.
//  +---+---+
static int fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
{
  if (npath < 3)
    return npath;

  // Get connected polygons
  static const int maxNeis = 16;
  dtPolyRef neis[maxNeis];
  int nneis = 0;

  const dtMeshTile* tile = 0;
  const dtPoly* poly = 0;
  if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
    return npath;

  for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
  {
    const dtLink* link = &tile->links[k];
    if (link->ref != 0)
    {
      if (nneis < maxNeis)
        neis[nneis++] = link->ref;
    }
  }

  // If any of the neighbour polygons is within the next few polygons
  // in the path, short cut to that polygon directly.
  static const int maxLookAhead = 6;
  int cut = 0;
  for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
    for (int j = 0; j < nneis; j++)
    {
      if (path[i] == neis[j]) {
        cut = i;
        break;
      }
    }
  }
  if (cut > 1)
  {
    int offset = cut-1;
    npath -= offset;
    for (int i = 1; i < npath; i++)
      path[i] = path[i+offset];
  }

  return npath;
}

static bool getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
    const float minTargetDist,
    const dtPolyRef* path, const int pathSize,
    float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
    float* outPoints = 0, int* outPointCount = 0)
{
  // Find steer target.
  static const int MAX_STEER_POINTS = 3;
  float steerPath[MAX_STEER_POINTS*3];
  unsigned char steerPathFlags[MAX_STEER_POINTS];
  dtPolyRef steerPathPolys[MAX_STEER_POINTS];
  int nsteerPath = 0;
  navQuery->findStraightPath(startPos, endPos, path, pathSize,
      steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
  if (!nsteerPath)
    return false;

  if (outPoints && outPointCount)
  {
    *outPointCount = nsteerPath;
    for (int i = 0; i < nsteerPath; ++i)
      dtVcopy(&outPoints[i*3], &steerPath[i*3]);
  }


  // Find vertex far enough to steer to.
  int ns = 0;
  while (ns < nsteerPath)
  {
    // Stop at Off-Mesh link or when point is further than slop away.
    if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
        !inRange(&steerPath[ns*3], startPos, minTargetDist, 10.0f))
      break;
    ns++;
  }
  // Failed to find good point to steer to.
  if (ns >= nsteerPath)
    return false;

  dtVcopy(steerPos, &steerPath[ns*3]);
  steerPos[1] = startPos[1];
  steerPosFlag = steerPathFlags[ns];
  steerPosRef = steerPathPolys[ns];

  return true;
}

PathFindingTile::PathFindingTile():
  m_totalBuildTimeMs(0),
  m_triareas(0),
  m_solid(0),
  m_chf(0),
  m_cset(0)
{
  resetCommonSettings();
  memset(m_tileBmin, 0, sizeof(m_tileBmin));
  memset(m_tileBmax, 0, sizeof(m_tileBmax));
  bzero(m_polys, sizeof(m_polys));
  bzero(m_parent, sizeof(m_parent));
  bzero(m_straightPath, sizeof(m_straightPath));
  bzero(m_straightPathFlags, sizeof(m_straightPathFlags));
  bzero(m_straightPathPolys, sizeof(m_straightPathPolys));
  bzero(m_smoothPath, sizeof(m_smoothPath));
  bzero(m_queryPoly, sizeof(m_queryPoly));
  bzero(m_spos, sizeof(m_spos));
  bzero(m_epos, sizeof(m_epos));
  bzero(m_hitPos, sizeof(m_hitPos));
  bzero(m_hitNormal, sizeof(m_hitNormal));
}

PathFindingTile::~PathFindingTile()
{
  cleanup();
  dtFreeNavMesh(m_navMesh);
  m_navMesh = 0;

  SAFE_DELETE(m_geom);
  SAFE_DELETE(m_ctx);
}

static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
  int magic;
  int version;
  int numTiles;
  dtNavMeshParams params;
};

struct NavMeshTileHeader
{
  dtTileRef tileRef;
  int dataSize;
};

bool PathFindingTile::init(const xLuaData &data)
{
  // if (!data.has("tileSize") || !data.has("cellSize") || !data.has("walkableHeight") || !data.has("walkableRadius") || !data.has("walkableClimb")) return false;
  // m_tileSize = data.getTableFloat("tileSize");
  // m_cellSize = data.getTableFloat("cellSize");

  // float walkableHeight = ceilf(data.getTableFloat("walkableHeight"));
  // float walkableRadius = data.getTableFloat("walkableRadius");
  // float walkableClimb = data.getTableFloat("walkableClimb");

  /*
  m_agentHeight = m_tileSize * walkableHeight;
  m_agentRadius = m_cellSize * walkableRadius;
  m_agentMaxClimb = m_tileSize * walkableClimb;
  */
  /*
  m_agentHeight = walkableHeight;
  m_agentRadius = walkableRadius;
  m_agentMaxClimb = walkableClimb;
  */

  XLOG << "[地图初始化]" << m_strFile << "m_agentHeight:" << m_agentHeight << "m_agentMaxClimb:" << m_agentMaxClimb << "m_agentRadius:" << m_agentRadius << "cellSize:" << m_cellSize << "tileSize:" << m_tileSize << "cellHeight:" << m_cellHeight << XEND;
  m_geom = NEW InputGeom();


  if (m_geom->load(NULL, data))
  {
    const float* bmin = m_geom->getMeshBoundsMin();
    const float* bmax = m_geom->getMeshBoundsMax();
    int gw = 0, gh = 0;
    rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
    const int ts = (int)m_tileSize;
    const int tw = (gw + ts-1) / ts;
    const int th = (gh + ts-1) / ts;

    // Max tiles and max polys affect how the tile IDs are caculated.
    // There are 22 bits available for identifying a tile and a polygon.
    int tileBits = rcMin((int)ilog2(nextPow2(tw*th)), 14);
    if (tileBits > 14) tileBits = 14;
    int polyBits = 22 - tileBits;
    m_maxTiles = 1 << tileBits;
    m_maxPolysPerTile = 1 << polyBits;
  }
  else
  {
    return false;
  }
  m_navQuery = dtAllocNavMeshQuery();
  m_ctx = NEW rcContext;

  m_neighbourhoodRadius = m_agentRadius * 20.0f;
  m_randomRadius = m_agentRadius * 30.0f;
  std::stringstream file_string;
  file_string.str("");
  file_string << "client-export/Scene/Scene" << m_strFile << "/NavMesh.obj";
  if (xServer::s_oOptArgs.m_blBuild && BaseConfig::getMe().needBuildNavMesh(m_strFile))
  {
    build();
    saveAll(file_string.str().c_str(), m_navMesh);
  }
  else
  {
    m_navMesh = loadAll(file_string.str().c_str());
    if (!m_navMesh) return false;
    m_navQuery->init(m_navMesh, 4096);
  }

  return true;
}

bool PathFindingTile::build()
{
  if (!m_geom || !m_geom->getMesh())
  {
    XLOG << "buildTiledNavigation: No vertices and triangles." << XEND;
    return false;
  }

  dtFreeNavMesh(m_navMesh);

  m_navMesh = dtAllocNavMesh();
  if (!m_navMesh)
  {
    XLOG << "buildTiledNavigation: Could not allocate navmesh." << XEND;
    return false;
  }

  m_polyPickExt[0] = 2;
  m_polyPickExt[1] = 4;
  m_polyPickExt[2] = 2;

  dtNavMeshParams params;
  rcVcopy(params.orig, m_geom->getMeshBoundsMin());
  params.tileWidth = m_tileSize*m_cellSize;
  params.tileHeight = m_tileSize*m_cellSize;
  params.maxTiles = m_maxTiles;
  params.maxPolys = m_maxPolysPerTile;
  XLOG << "[NavMesh],tileWidth:" << params.tileWidth << "tileHeight:" << params.tileHeight << "maxTiles:" << params.maxTiles << "maxPolys:" << params.maxPolys << XEND;

  dtStatus status;

  status = m_navMesh->init(&params);
  if (dtStatusFailed(status))
  {
    XLOG << "buildTiledNavigation: Could not init navmesh." << XEND;
    return false;
  }

  if (m_buildAll)
    buildAllTiles();

  m_polyPickExt[0] = 2;
  m_polyPickExt[1] = 4;
  m_polyPickExt[2] = 2;

  status = m_navQuery->init(m_navMesh, 4096);
  if (dtStatusFailed(status))
  {
    XLOG << "buildTiledNavigation: Could not init Detour navmesh query" << XEND;
    return false;
  }

  return true;
}

void PathFindingTile::buildTile(const float* pos)
{
  if (!m_geom) return;
  if (!m_navMesh) return;

  const float* bmin = m_geom->getMeshBoundsMin();
  const float* bmax = m_geom->getMeshBoundsMax();

  const float ts = m_tileSize*m_cellSize;
  const int tx = (int)((pos[0] - bmin[0]) / ts);
  const int ty = (int)((pos[2] - bmin[2]) / ts);

  m_tileBmin[0] = bmin[0] + tx*ts;
  m_tileBmin[1] = bmin[1];
  m_tileBmin[2] = bmin[2] + ty*ts;

  m_tileBmax[0] = bmin[0] + (tx+1)*ts;
  m_tileBmax[1] = bmax[1];
  m_tileBmax[2] = bmin[2] + (ty+1)*ts;

  int dataSize = 0;
  unsigned char* data = buildTileMesh(tx, ty, m_tileBmin, m_tileBmax, dataSize);

  // Remove any previous data (navmesh owns and deletes the data).
  m_navMesh->removeTile(m_navMesh->getTileRefAt(tx,ty,0),0,0);

  // Add tile, or leave the location empty.
  if (data)
  {
    // Let the navmesh own the data.
    dtStatus status = m_navMesh->addTile(data,dataSize,DT_TILE_FREE_DATA,0,0);
    if (dtStatusFailed(status))
      dtFree(data);
  }
}

void PathFindingTile::getTilePos(const float* pos, int& tx, int& ty)
{
  if (!m_geom) return;

  const float* bmin = m_geom->getMeshBoundsMin();

  const float ts = m_tileSize*m_cellSize;
  tx = (int)((pos[0] - bmin[0]) / ts);
  ty = (int)((pos[2] - bmin[2]) / ts);
}

void PathFindingTile::removeTile(const float* pos)
{
  if (!m_geom) return;
  if (!m_navMesh) return;

  const float* bmin = m_geom->getMeshBoundsMin();
  const float* bmax = m_geom->getMeshBoundsMax();

  const float ts = m_tileSize*m_cellSize;
  const int tx = (int)((pos[0] - bmin[0]) / ts);
  const int ty = (int)((pos[2] - bmin[2]) / ts);

  m_tileBmin[0] = bmin[0] + tx*ts;
  m_tileBmin[1] = bmin[1];
  m_tileBmin[2] = bmin[2] + ty*ts;

  m_tileBmax[0] = bmin[0] + (tx+1)*ts;
  m_tileBmax[1] = bmax[1];
  m_tileBmax[2] = bmin[2] + (ty+1)*ts;

  m_navMesh->removeTile(m_navMesh->getTileRefAt(tx,ty,0),0,0);
}

void PathFindingTile::buildAllTiles()
{
  if (!m_geom) return;
  if (!m_navMesh) return;

  const float* bmin = m_geom->getMeshBoundsMin();
  const float* bmax = m_geom->getMeshBoundsMax();
  int gw = 0, gh = 0;
  rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
  const int ts = (int)m_tileSize;
  const int tw = (gw + ts-1) / ts;
  const int th = (gh + ts-1) / ts;
  const float tcs = m_tileSize*m_cellSize;

  // Start the build process.

  for (int y = 0; y < th; ++y)
  {
    for (int x = 0; x < tw; ++x)
    {
      m_tileBmin[0] = bmin[0] + x*tcs;
      m_tileBmin[1] = bmin[1];
      m_tileBmin[2] = bmin[2] + y*tcs;

      m_tileBmax[0] = bmin[0] + (x+1)*tcs;
      m_tileBmax[1] = bmax[1];
      m_tileBmax[2] = bmin[2] + (y+1)*tcs;

      int dataSize = 0;
      unsigned char* data = buildTileMesh(x, y, m_tileBmin, m_tileBmax, dataSize);
 //     XLOG("[BuildTileMesh],x:%d/%d,y:%d/%d,m_tileBmin:(%f,%f,%f),m_tileBmax:(%f,%f,%f)", x, tw, y, th, m_tileBmin[0], m_tileBmin[1], m_tileBmin[2], m_tileBmax[0], m_tileBmax[1], m_tileBmax[2]);
      if (data)
      {
        // Remove any previous data (navmesh owns and deletes the data).
        m_navMesh->removeTile(m_navMesh->getTileRefAt(x,y,0),0,0);
        // Let the navmesh own the data.
        dtStatus status = m_navMesh->addTile(data,dataSize,DT_TILE_FREE_DATA,0,0);
        if (dtStatusFailed(status))
          dtFree(data);
      }
    }
  }
}

void PathFindingTile::removeAllTiles()
{
  const float* bmin = m_geom->getMeshBoundsMin();
  const float* bmax = m_geom->getMeshBoundsMax();
  int gw = 0, gh = 0;
  rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
  const int ts = (int)m_tileSize;
  const int tw = (gw + ts-1) / ts;
  const int th = (gh + ts-1) / ts;

  for (int y = 0; y < th; ++y)
    for (int x = 0; x < tw; ++x)
      m_navMesh->removeTile(m_navMesh->getTileRefAt(x,y,0),0,0);
}

unsigned char* PathFindingTile::buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize)
{
  if (!m_geom || !m_geom->getMesh() || !m_geom->getChunkyMesh())
  {
    XLOG << "buildNavigation: Input mesh is not specified." << XEND;
    return 0;
  }

  m_tileMemUsage = 0;
  m_tileBuildTime = 0;

  cleanup();

  const float* verts = m_geom->getMesh()->getVerts();
  const int nverts = m_geom->getMesh()->getVertCount();
  // const int ntris = m_geom->getMesh()->getTriCount();
  const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();

  // Init build configuration from GUI
  memset(&m_cfg, 0, sizeof(m_cfg));
  m_cfg.cs = m_cellSize;
  m_cfg.ch = m_cellHeight;
  m_cfg.walkableSlopeAngle = m_agentMaxSlope;
  m_cfg.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
  m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
  m_cfg.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
  m_cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
  m_cfg.maxSimplificationError = m_edgeMaxError;
  m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);              // Note: area = size*size
  m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);  // Note: area = size*size
  m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
  m_cfg.tileSize = (int)m_tileSize;
  m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
  m_cfg.width = m_cfg.tileSize + m_cfg.borderSize*2;
  m_cfg.height = m_cfg.tileSize + m_cfg.borderSize*2;
  m_cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
  m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;

  // Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
  //
  // This is done in order to make sure that the navmesh tiles connect correctly at the borders,
  // and the obstacles close to the border work correctly with the dilation process.
  // No polygons (or contours) will be created on the border area.
  //
  // IMPORTANT!
  //
  //   :''''''''':
  //   : +-----+ :
  //   : |     | :
  //   : |     |<--- tile to build
  //   : |     | :  
  //   : +-----+ :<-- geometry needed
  //   :.........:
  //
  // You should use this bounding box to query your input geometry.
  //
  // For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
  // you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
  // or use the bounding box below to only pass in a sliver of each of the 8 neighbours.
  rcVcopy(m_cfg.bmin, bmin);
  rcVcopy(m_cfg.bmax, bmax);
  m_cfg.bmin[0] -= m_cfg.borderSize*m_cfg.cs;
  m_cfg.bmin[2] -= m_cfg.borderSize*m_cfg.cs;
  m_cfg.bmax[0] += m_cfg.borderSize*m_cfg.cs;
  m_cfg.bmax[2] += m_cfg.borderSize*m_cfg.cs;


  // Allocate voxel heightfield where we rasterize our input data to.
  m_solid = rcAllocHeightfield();
  if (!m_solid)
  {
    //m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
    return 0;
  }
  if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
  {
    XERR << "buildNavigation: Could not create solid heightfield." << XEND;
    return 0;
  }

  // Allocate array that can hold triangle flags.
  // If you have multiple meshes you need to process, allocate
  // and array which can hold the max number of triangles you need to process.
  m_triareas = NEW unsigned char[chunkyMesh->maxTrisPerChunk];
  if (!m_triareas)
  {
    XERR << "buildNavigation: Out of memory 'm_triareas'" << chunkyMesh->maxTrisPerChunk << XEND;
    return 0;
  }

  float tbmin[2], tbmax[2];
  tbmin[0] = m_cfg.bmin[0];
  tbmin[1] = m_cfg.bmin[2];
  tbmax[0] = m_cfg.bmax[0];
  tbmax[1] = m_cfg.bmax[2];
  int cid[512];// TODO: Make grow when returning too many items.
  const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
  if (!ncid)
    return 0;

  m_tileTriCount = 0;

  for (int i = 0; i < ncid; ++i)
  {
    const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
    const int* ctris = &chunkyMesh->tris[node.i*3];
    const int nctris = node.n;

    m_tileTriCount += nctris;

    memset(m_triareas, 0, nctris*sizeof(unsigned char));
    rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle,
        verts, nverts, ctris, nctris, m_triareas);

    rcRasterizeTriangles(m_ctx, verts, nverts, ctris, m_triareas, nctris, *m_solid, m_cfg.walkableClimb);
  }

  if (!m_keepInterResults)
  {
    delete [] m_triareas;
    m_triareas = 0;
  }

  // Once all geometry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
  rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
  rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

  // Compact the heightfield so that it is faster to handle from now on.
  // This will result more cache coherent data as well as the neighbours
  // between walkable cells will be calculated.
  m_chf = rcAllocCompactHeightfield();
  if (!m_chf)
  {
    XERR << "buildNavigation: Out of memory 'chf'." << XEND;
    return 0;
  }
  if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
  {
    XERR << "buildNavigation: Could not build compact data." << XEND;
    return 0;
  }

  if (!m_keepInterResults)
  {
    rcFreeHeightField(m_solid);
    m_solid = 0;
  }

  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
  {
    XERR << "buildNavigation: Could not erode." << XEND;
    return 0;
  }

  // (Optional) Mark areas.
  const ConvexVolume* vols = m_geom->getConvexVolumes();
  for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
    rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

  // Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
  // There are 3 martitioning methods, each with some pros and cons:
  // 1) Watershed partitioning
  //   - the classic Recast partitioning
  //   - creates the nicest tessellation
  //   - usually slowest
  //   - partitions the heightfield into nice regions without holes or overlaps
  //   - the are some corner cases where this method creates produces holes and overlaps
  //      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
  //      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
  //   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
  // 2) Monotone partioning
  //   - fastest
  //   - partitions the heightfield into regions without holes and overlaps (guaranteed)
  //   - creates long thin polygons, which sometimes causes paths with detours
  //   * use this if you want fast navmesh generation
  // 3) Layer partitoining
  //   - quite fast
  //   - partitions the heighfield into non-overlapping regions
  //   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
  //   - produces better triangles than monotone partitioning
  //   - does not have the corner cases of watershed partitioning
  //   - can be slow and create a bit ugly tessellation (still better than monotone)
  //     if you have large open areas with small obstacles (not a problem if you use tiles)
  //   * good choice to use for tiled navmesh with medium and small sized tiles

  // m_partitionType = SAMPLE_PARTITION_LAYERS;
  m_partitionType = SAMPLE_PARTITION_WATERSHED;

  if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
  {
    // Prepare for region partitioning, by calculating distance field along the walkable surface.
    if (!rcBuildDistanceField(m_ctx, *m_chf))
    {
      XERR << "buildNavigation: Could not build distance field." << XEND;
      return 0;
    }

    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
    {
      XERR << "buildNavigation: Could not build watershed regions." << XEND;
      return 0;
    }
  }
  else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
  {
    // Partition the walkable surface into simple regions without holes.
    // Monotone partitioning does not need distancefield.
    if (!rcBuildRegionsMonotone(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
    {
      XERR << "buildNavigation: Could not build monotone regions." << XEND;
      return 0;
    }
  }
  else // SAMPLE_PARTITION_LAYERS
  {
    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildLayerRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea))
    {
      XERR << "buildNavigation: Could not build layer regions." << XEND;
      return 0;
    }
  }

  // Create contours.
  m_cset = rcAllocContourSet();
  if (!m_cset)
  {
    XERR << "buildNavigation: Out of memory 'cset'." << XEND;
    return 0;
  }
  if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
  {
    XERR << "buildNavigation: Could not create contours." << XEND;
    return 0;
  }

  if (m_cset->nconts == 0)
  {
    return 0;
  }

  // Build polygon navmesh from the contours.
  m_pmesh = rcAllocPolyMesh();
  if (!m_pmesh)
  {
    XERR << "buildNavigation: Out of memory 'pmesh'." << XEND;
    return 0;
  }
  if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
  {
    XERR << "buildNavigation: Could not triangulate contours." << XEND;
    return 0;
  }

  // Build detail mesh.
  m_dmesh = rcAllocPolyMeshDetail();
  if (!m_dmesh)
  {
    m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
    return 0;
  }

  if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf,
        m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
        *m_dmesh))
  {
    m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
    return 0;
  }

  if (!m_keepInterResults)
  {
    rcFreeCompactHeightfield(m_chf);
    m_chf = 0;
    rcFreeContourSet(m_cset);
    m_cset = 0;
  }

  unsigned char* navData = 0;
  int navDataSize = 0;
  if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
  {
    if (m_pmesh->nverts >= 0xffff)
    {
      // The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
      m_ctx->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
      return 0;
    }

    // Update poly flags from areas.
    for (int i = 0; i < m_pmesh->npolys; ++i)
    {
      if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
        m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

      if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
          m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
          m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
      {
        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
      }
      else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
      {
        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
      }
      else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
      {
        m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
      }
    }

    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = m_pmesh->verts;
    params.vertCount = m_pmesh->nverts;
    params.polys = m_pmesh->polys;
    params.polyAreas = m_pmesh->areas;
    params.polyFlags = m_pmesh->flags;
    params.polyCount = m_pmesh->npolys;
    params.nvp = m_pmesh->nvp;
    params.detailMeshes = m_dmesh->meshes;
    params.detailVerts = m_dmesh->verts;
    params.detailVertsCount = m_dmesh->nverts;
    params.detailTris = m_dmesh->tris;
    params.detailTriCount = m_dmesh->ntris;
    params.walkableHeight = m_agentHeight;
    params.walkableRadius = m_agentRadius;
    params.walkableClimb = m_agentMaxClimb;
    params.tileX = tx;
    params.tileY = ty;
    params.tileLayer = 0;
    rcVcopy(params.bmin, m_pmesh->bmin);
    rcVcopy(params.bmax, m_pmesh->bmax);
    params.cs = m_cfg.cs;
    params.ch = m_cfg.ch;
    params.buildBvTree = true;

  //  XLOG("[NavMesh],nverts:%d,npolys:%d,maxpolys:%d,nvp:%d", m_pmesh->nverts, m_pmesh->npolys, m_pmesh->maxpolys, m_pmesh->nvp);


    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
      m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
      return 0;
    }
  }
  m_tileMemUsage = navDataSize/1024.0f;

  m_ctx->stopTimer(RC_TIMER_TOTAL);

  dataSize = navDataSize;
  return navData;
}

void PathFindingTile::cleanup()
{
  delete [] m_triareas;
  m_triareas = 0;
  rcFreeHeightField(m_solid);
  m_solid = 0;
  rcFreeCompactHeightfield(m_chf);
  m_chf = 0;
  rcFreeContourSet(m_cset);
  m_cset = 0;
  rcFreePolyMesh(m_pmesh);
  m_pmesh = 0;
  rcFreePolyMeshDetail(m_dmesh);
  m_dmesh = 0;
}

void PathFindingTile::resetCommonSettings()
{
  m_cellSize = 0.1f;//0.3f;
  m_cellHeight = 0.1f;//0.2f;
  m_agentHeight = 2.0f;
  m_agentRadius = 0.2f; //0.6f;
  m_cellSize = m_agentRadius / 3;
  m_agentMaxClimb = 0.9f;
  m_agentMaxSlope = 80.0f;
  m_regionMinSize = 8;
  m_regionMergeSize = 20;
  m_edgeMaxLen = 12.0f;
  m_edgeMaxError = 1.3f;
  m_vertsPerPoly = 6.0f;
  m_detailSampleDist = 6.0f;
  m_detailSampleMaxError = 1.0f;
  m_partitionType = SAMPLE_PARTITION_WATERSHED;

  m_polyPickExt[0] = 2;
  m_polyPickExt[1] = 4;
  m_polyPickExt[2] = 2;
  m_filter.setIncludeFlags(0xffff);
  m_filter.setExcludeFlags(0);
}

bool PathFindingTile::finding(xPos &start, xPos &end)
{
  if (!m_navMesh)
    return false;

  m_spos[0] = start.x;
  m_spos[1] = start.y;
  m_spos[2] = start.z;
  m_epos[0] = end.x;
  m_epos[1] = end.y;
  m_epos[2] = end.z;

  m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);
  m_navQuery->findNearestPoly(m_epos, m_polyPickExt, &m_filter, &m_endRef, 0);

  if (!m_startRef || !m_endRef)
  {
    // XLOG("[path],(%f,%f,%f),m_startRef:%u,(%f,%f,%f)m_endRef:%u", m_spos[0], m_spos[1], m_spos[2], m_startRef,
    //   m_epos[0], m_epos[1], m_epos[2], m_endRef);
  }

  m_pathFindStatus = DT_FAILURE;

  if (m_toolMode == TOOLMODE_PATHFIND_FOLLOW)
  {
    m_pathIterNum = 0;
    if (m_startRef && m_endRef)
    {
#ifdef DUMP_REQS
      printf("pi  %f %f %f  %f %f %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags()); 
#endif

      m_navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, &m_npolys, MAX_POLYS);

      m_nsmoothPath = 0;

      if (m_npolys)
      {
        // Iterate over the path to find smooth path on the detail mesh surface.
        dtPolyRef polys[MAX_POLYS];
        memcpy(polys, m_polys, sizeof(dtPolyRef)*m_npolys); 
        int npolys = m_npolys;

        float iterPos[3], targetPos[3];
        m_navQuery->closestPointOnPoly(m_startRef, m_spos, iterPos, 0);
        m_navQuery->closestPointOnPoly(polys[npolys-1], m_epos, targetPos, 0);

        static const float STEP_SIZE = 1.0f;
        static const float SLOP = 0.02f;

        m_nsmoothPath = 0;

        dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
        m_nsmoothPath++;

        // Move towards target a small advancement at a time until target reached or
        // when ran out of memory to store the path.
        while (npolys && m_nsmoothPath < MAX_SMOOTH)
        {
          // Find location to steer towards.
          float steerPos[3];
          unsigned char steerPosFlag;
          dtPolyRef steerPosRef;

          if (!getSteerTarget(m_navQuery, iterPos, targetPos, SLOP,
                polys, npolys, steerPos, steerPosFlag, steerPosRef))
            break;

          bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
          bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

          // Find movement delta.
          float delta[3], len;
          dtVsub(delta, steerPos, iterPos);
          len = dtMathSqrtf(dtVdot(delta, delta));
          // If the steer target is end of path or off-mesh link, do not move past the location.
          if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
            len = 1;
          else
            len = STEP_SIZE / len;
          float moveTgt[3];
          dtVmad(moveTgt, iterPos, delta, len);

          // Move
          float result[3];
          dtPolyRef visited[16];
          int nvisited = 0;
          m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &m_filter,
              result, visited, &nvisited, 16);

          npolys = fixupCorridor(polys, npolys, MAX_POLYS, visited, nvisited);
          npolys = fixupShortcuts(polys, npolys, m_navQuery);

          float h = 0;
          m_navQuery->getPolyHeight(polys[0], result, &h);
          result[1] = h;
          dtVcopy(iterPos, result);
          // Handle end of path and off-mesh links when close enough.
          if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
          {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (m_nsmoothPath < MAX_SMOOTH)
            {
              dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
              m_nsmoothPath++;
            }
            break;
          }
          else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
          {
            // Reached off-mesh connection.
            float startPos[3], endPos[3];

            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = 0, polyRef = polys[0];
            int npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
              prevRef = polyRef;
              polyRef = polys[npos];
              npos++;
            }
            for (int i = npos; i < npolys; ++i)
              polys[i-npos] = polys[i];
            npolys -= npos;

            // Handle the connection.
            dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
            if (dtStatusSucceed(status))
            {
              if (m_nsmoothPath < MAX_SMOOTH)
              {
                dtVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
                m_nsmoothPath++;
                // Hack to make the dotted path not visible during off-mesh connection.
                if (m_nsmoothPath & 1)
                {
                  dtVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
                  m_nsmoothPath++;
                }
              }
              // Move position at the other side of the off-mesh link.
              dtVcopy(iterPos, endPos);
              float eh = 0.0f;
              m_navQuery->getPolyHeight(polys[0], iterPos, &eh);
              iterPos[1] = eh;
            }
          }

          // Store results.
          if (m_nsmoothPath < MAX_SMOOTH)
          {
            dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
            m_nsmoothPath++;
          }
        }
        return true;
      }
      return false;
    }
    else
    {
      m_npolys = 0;
      m_nsmoothPath = 0;
      return false;
    }
  }
  else if (m_toolMode == TOOLMODE_PATHFIND_STRAIGHT)
  {
    if (m_startRef && m_endRef)
    {
#ifdef DUMP_REQS
      printf("ps  %f %f %f  %f %f %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags()); 
#endif
      m_navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, &m_npolys, MAX_POLYS);
      m_nstraightPath = 0;
      if (m_npolys)
      {
        // In case of partial path, make sure the end point is clamped to the last polygon.
        float epos[3];
        dtVcopy(epos, m_epos);
        if (m_polys[m_npolys-1] != m_endRef)
          m_navQuery->closestPointOnPoly(m_polys[m_npolys-1], m_epos, epos, 0);

        m_navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
            m_straightPath, m_straightPathFlags,
            m_straightPathPolys, &m_nstraightPath, MAX_POLYS, m_straightPathOptions);
        return true;
      }
      return false;
    }
    else
    {
      m_npolys = 0;
      m_nstraightPath = 0;
      return false;
    }
  }
  else if (m_toolMode == TOOLMODE_PATHFIND_SLICED)
  {
    if (m_startRef && m_endRef)
    {
#ifdef DUMP_REQS
      printf("ps  %f %f %f  %f %f %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags()); 
#endif
      m_npolys = 0;
      m_nstraightPath = 0;

      m_pathFindStatus = m_navQuery->initSlicedFindPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, DT_FINDPATH_ANY_ANGLE);
    }
    else
    {
      m_npolys = 0;
      m_nstraightPath = 0;
    }
  }
  else if (m_toolMode == TOOLMODE_RAYCAST)
  {
    m_nstraightPath = 0;
    if (m_startRef)
    {
#ifdef DUMP_REQS
      printf("rc  %f %f %f  %f %f %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags()); 
#endif
      float t = 0;
      m_npolys = 0;
      m_nstraightPath = 2;
      m_straightPath[0] = m_spos[0];
      m_straightPath[1] = m_spos[1];
      m_straightPath[2] = m_spos[2];
      m_navQuery->raycast(m_startRef, m_spos, m_epos, &m_filter, &t, m_hitNormal, m_polys, &m_npolys, MAX_POLYS);
      if (t > 1)
      {
        // No hit
        dtVcopy(m_hitPos, m_epos);
        m_hitResult = false;
      }
      else
      {
        // Hit
        dtVlerp(m_hitPos, m_spos, m_epos, t);
        m_hitResult = true;
      }
      // Adjust height.
      if (m_npolys > 0)
      {
        float h = 0;
        m_navQuery->getPolyHeight(m_polys[m_npolys-1], m_hitPos, &h);
        m_hitPos[1] = h;
      }
      dtVcopy(&m_straightPath[3], m_hitPos);
    }
  }
  else if (m_toolMode == TOOLMODE_DISTANCE_TO_WALL)
  {
    m_distanceToWall = 0;
    if (m_startRef)
    {
#ifdef DUMP_REQS
      printf("dw  %f %f %f  %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], 100.0f,
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags()); 
#endif
      m_distanceToWall = 0.0f;
      m_navQuery->findDistanceToWall(m_startRef, m_spos, 100.0f, &m_filter, &m_distanceToWall, m_hitPos, m_hitNormal);
    }
  }
  else if (m_toolMode == TOOLMODE_FIND_POLYS_IN_CIRCLE)
  {
    if (m_startRef)
    {
      const float dx = m_epos[0] - m_spos[0];
      const float dz = m_epos[2] - m_spos[2];
      float dist = sqrtf(dx*dx + dz*dz);
#ifdef DUMP_REQS
      printf("fpc  %f %f %f  %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], dist,
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags());
#endif
      m_navQuery->findPolysAroundCircle(m_startRef, m_spos, dist, &m_filter,
          m_polys, m_parent, 0, &m_npolys, MAX_POLYS);

    }
  }
  else if (m_toolMode == TOOLMODE_FIND_POLYS_IN_SHAPE)
  {
    if (m_startRef)
    {
      const float nx = (m_epos[2] - m_spos[2])*0.25f;
      const float nz = -(m_epos[0] - m_spos[0])*0.25f;
      const float agentHeight = 0;

      m_queryPoly[0] = m_spos[0] + nx*1.2f;
      m_queryPoly[1] = m_spos[1] + agentHeight/2;
      m_queryPoly[2] = m_spos[2] + nz*1.2f;

      m_queryPoly[3] = m_spos[0] - nx*1.3f;
      m_queryPoly[4] = m_spos[1] + agentHeight/2;
      m_queryPoly[5] = m_spos[2] - nz*1.3f;

      m_queryPoly[6] = m_epos[0] - nx*0.8f;
      m_queryPoly[7] = m_epos[1] + agentHeight/2;
      m_queryPoly[8] = m_epos[2] - nz*0.8f;

      m_queryPoly[9] = m_epos[0] + nx;
      m_queryPoly[10] = m_epos[1] + agentHeight/2;
      m_queryPoly[11] = m_epos[2] + nz;

#ifdef DUMP_REQS
      printf("fpp  %f %f %f  %f %f %f  %f %f %f  %f %f %f  0x%x 0x%x\n",
          m_queryPoly[0],m_queryPoly[1],m_queryPoly[2],
          m_queryPoly[3],m_queryPoly[4],m_queryPoly[5],
          m_queryPoly[6],m_queryPoly[7],m_queryPoly[8],
          m_queryPoly[9],m_queryPoly[10],m_queryPoly[11],
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags());
#endif
      m_navQuery->findPolysAroundShape(m_startRef, m_queryPoly, 4, &m_filter,
          m_polys, m_parent, 0, &m_npolys, MAX_POLYS);
    }
  }
  else if (m_toolMode == TOOLMODE_FIND_LOCAL_NEIGHBOURHOOD)
  {
    if (m_startRef)
    {
#ifdef DUMP_REQS
      printf("fln  %f %f %f  %f  0x%x 0x%x\n",
          m_spos[0],m_spos[1],m_spos[2], m_neighbourhoodRadius,
          m_filter.getIncludeFlags(), m_filter.getExcludeFlags());
#endif
      m_navQuery->findLocalNeighbourhood(m_startRef, m_spos, m_neighbourhoodRadius, &m_filter,
          m_polys, m_parent, &m_npolys, MAX_POLYS);
    }
  }
  return true;
}

bool PathFindingTile::finding(xPos &start, xPos &end, std::list<xPos> &outlist, ToolMode mode)
{
  // mode = TOOLMODE_PATHFIND_STRAIGHT;
  m_toolMode = mode;

  if (finding(start, end))
  {
    switch (m_toolMode)
    {
      case TOOLMODE_PATHFIND_FOLLOW:
        {
          for (int i=0; i<m_nsmoothPath; ++i)
          {
            outlist.push_back(xPos(m_smoothPath[i*3], m_smoothPath[i*3+1], m_smoothPath[i*3+2]));
          }
        }
        break;
      case TOOLMODE_PATHFIND_STRAIGHT:
        {
          for (int i=0; i<m_nstraightPath; ++i)
          {
            outlist.push_back(xPos(m_straightPath[i*3], m_straightPath[i*3+1], m_straightPath[i*3+2]));
          }
        }
        break;
      case TOOLMODE_RAYCAST:
        {
          for (int i=0; i<m_nstraightPath; ++i)
          {
            outlist.push_back(xPos(m_straightPath[i*3], m_straightPath[i*3+1], m_straightPath[i*3+2]));
          }
        }
        break;
      default:
        break;
    }
    return true;
  }

  return false;
}

bool PathFindingTile::findRandomPoint(xPos source, float radius, xPos &out)
{
  if (!m_navQuery) return false;
  m_spos[0] = source.x;
  m_spos[1] = source.y;
  m_spos[2] = source.z;

  m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);

  if (m_navQuery->findRandomPointAroundCircle(m_startRef, m_spos, radius, &m_filter, frand, &m_endRef, m_epos))
  {
    out.x = m_epos[0];
    out.y = m_epos[1];
    out.z = m_epos[2];
    return true;
  }

  return false;
}

bool PathFindingTile::getValidPos(xPos source, xPos &out)
{
  if (!m_navQuery) return false;
  m_spos[0] = source.x;
  m_spos[1] = source.y;
  m_spos[2] = source.z;

  m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);

  bool posOverPoly = true;
  if (dtStatusFailed(m_navQuery->closestPointOnPoly(m_startRef, m_spos, m_epos, &posOverPoly)))
    return false;
  /*
  if (dtStatusFailed(m_navQuery->closestPointOnPolyBoundary(m_startRef, m_spos, m_epos)))
    return false;
    */

  out.x = m_epos[0];
  out.y = m_epos[1];
  out.z = m_epos[2];
  return true;
}

bool PathFindingTile::resetPosHeight(xPos &source)
{
  if (!m_navQuery) return false;
  m_spos[0] = source.x;
  m_spos[1] = source.y;
  m_spos[2] = source.z;

  m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);

  float h = 0.0f;
  if (dtStatusFailed(m_navQuery->getPolyHeight(m_startRef, m_spos, &h)))
    return false;

  source.y = h;
  return true;
}

const float* PathFindingTile::getMeshBoundsMin()
{
  return m_geom->getMeshBoundsMin();
}
const float* PathFindingTile::getMeshBoundsMax()
{
  return m_geom->getMeshBoundsMax();
}

void PathFindingTile::saveAll(const char* path, const dtNavMesh* mesh)
{
  if (!mesh) return;

  FILE* fp = fopen(path, "wb");
  if (!fp)
    return;

  // Store header.
  NavMeshSetHeader header;
  header.magic = NAVMESHSET_MAGIC;
  header.version = NAVMESHSET_VERSION;
  header.numTiles = 0;
  for (int i = 0; i < mesh->getMaxTiles(); ++i)
  {
    const dtMeshTile* tile = mesh->getTile(i);
    if (!tile || !tile->header || !tile->dataSize) continue;
    header.numTiles++;
  }
  memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
  fwrite(&header, sizeof(NavMeshSetHeader), 1, fp);

  // Store tiles.
  for (int i = 0; i < mesh->getMaxTiles(); ++i)
  {
    const dtMeshTile* tile = mesh->getTile(i);
    if (!tile || !tile->header || !tile->dataSize) continue;

    NavMeshTileHeader tileHeader;
    tileHeader.tileRef = mesh->getTileRef(tile);
    tileHeader.dataSize = tile->dataSize;
    fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

    fwrite(tile->data, tile->dataSize, 1, fp);
  }

  fclose(fp);
}

dtNavMesh* PathFindingTile::loadAll(const char* path)
{
  FILE* fp = fopen(path, "rb");
  if (!fp) return 0;

  // Read header.
  NavMeshSetHeader header;
  size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
  if (readLen != 1)
  {
    fclose(fp);
    return 0;
  }
  if (header.magic != NAVMESHSET_MAGIC)
  {
    fclose(fp);
    return 0;
  }
  if (header.version != NAVMESHSET_VERSION)
  {
    fclose(fp);
    return 0;
  }

  dtNavMesh* mesh = dtAllocNavMesh();
  if (!mesh)
  {
    fclose(fp);
    return 0;
  }
  dtStatus status = mesh->init(&header.params);
  if (dtStatusFailed(status))
  {
    fclose(fp);
    return 0;
  }

  // Read tiles.
  for (int i = 0; i < header.numTiles; ++i)
  {
    NavMeshTileHeader tileHeader;
    readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
    if (readLen != 1)
    {
      fclose(fp);
      return 0;
    }

    if (!tileHeader.tileRef || !tileHeader.dataSize)
      break;

    unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
    if (!data) break;
    memset(data, 0, tileHeader.dataSize);
    readLen = fread(data, tileHeader.dataSize, 1, fp);
    if (readLen != 1)
    {
      fclose(fp);
      return 0;
    }

    mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
  }

  fclose(fp);

  XLOG << "[地图初始化]" << m_strFile << "loadAll" << XEND;

  return mesh;
}

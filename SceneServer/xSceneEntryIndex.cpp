#include "xSceneEntryIndex.h"
#include "SceneUser.h"
#include "GatewayCmd.h"
#include "SceneServer.h"
#include "SceneNpc.h"
#include "SceneItem.h"
#include "SceneUserManager.h"
#include "xSceneEntryDynamic.h"
#include "SceneTrap.h"
#include "GearManager.h"
#include "SceneAct.h"
#include "SceneBooth.h"
#include "CommonConfig.h"

xSceneEntryIndex::xSceneEntryIndex()
{
  funNpcIndex.clear();
}

xSceneEntryIndex::~xSceneEntryIndex()
{
}

/*
 **   场景坐标
 * (0,0)_________Z
 *     |
 *     |
 *  	  |
 **    X 
 * */
void xSceneEntryIndex::initIndex()
{
  fillNineScreenMap();
  fillDirectNineScreen();
  fillRDirectNineScreen();
}

/*
 ** 填充9屏坐标
 */
void xSceneEntryIndex::fillNineScreenMap()
{
  int temp = -1;
  ninescreen.clear();
  for (DWORD i = 0; i<getScreenSize(); i++)
  {
    xPosIVector &vec = ninescreen[i];
    vec.push_back(i);
    //左上
    temp = i-getXScreenNum()-1;
    if( temp>=0 && (temp%getXScreenNum()+1)==i%getXScreenNum() )
      vec.push_back(temp);
    //上
    temp = i-getXScreenNum();
    if(temp>=0 && (temp%getXScreenNum())==i%getXScreenNum() )
      vec.push_back( temp );
    //右上
    temp = i-getXScreenNum()+1;
    if(temp>=0 && (temp%getXScreenNum()-1)==i%getXScreenNum() )
      vec.push_back(temp);
    //左
    temp = i-1;
    if(temp>=0 && (temp%getXScreenNum()+1)==i%getXScreenNum() )
      vec.push_back(temp);
    //右
    temp = i+1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-1)==i%getXScreenNum() )
      vec.push_back(temp);
    //左下
    temp = i+getXScreenNum()-1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()+1)==i%getXScreenNum())
      vec.push_back(temp);
    //右下
    temp = i+getXScreenNum()+1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-1)==i%getXScreenNum())
      vec.push_back(temp);
    //下
    temp = i+getXScreenNum();
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum())==i%getXScreenNum())
      vec.push_back(temp);
  }
#ifdef _LX_DEBUG
  // XLOG("getScreenSize,%u,getXScreenNum:%u", getScreenSize(), getXScreenNum());
  // printNineScreen();
#endif
}

void xSceneEntryIndex::printNineScreen()
{
  for(DWORD i = 0; i<getScreenSize(); i++)
  {
    DWORD num = ninescreen[i].size();
    for(DWORD m=0;m<num;m++)
      XLOG << "Scene posi:" << i << ", npos:" << ninescreen[i][m] << XEND;
  }
}

/*
 ** 移动时如果切屏需要加入的屏
 ** 方向下标如下图 
 ** 8      1      2
 **  ______|_____
 ** |      |     |
 ** |      |     |
 **7|______|_____|3_
 ** |      |     |
 ** |      |     | 
 ** |______|_____|
 ** 6	  5     4
 */
void xSceneEntryIndex::fillDirectNineScreen()
{
  int temp = -1;
  for (DWORD i=0; i<9; i++)
  {
    direct_ninescreen[i].clear();
  }
  DWORD w = getXScreenNum();
  for( DWORD i=0; i<getScreenSize(); i++)//getScreenSize()个坐标
  {
    temp = i-w-w-2;
    if(temp>=0 && (temp%getXScreenNum()+2)==i%getXScreenNum())
    {
      direct_ninescreen[8][i].push_back(temp);
    }	
    temp = i-w-w-1;
    if(temp>=0 && (temp%getXScreenNum()+1)==i%getXScreenNum())
    {
      direct_ninescreen[8][i].push_back(temp);
      direct_ninescreen[1][i].push_back(temp);
    }
    temp = i-w-w;
    if(temp>=0 && (temp%getXScreenNum())==i%getXScreenNum())
    {
      direct_ninescreen[8][i].push_back(temp);
      direct_ninescreen[1][i].push_back(temp);
      direct_ninescreen[2][i].push_back(temp);
    }
    temp = i-w-w+1;
    if(temp>=0 && (temp%getXScreenNum()-1)==i%getXScreenNum())
    {
      direct_ninescreen[1][i].push_back(temp);
      direct_ninescreen[2][i].push_back(temp);
    }
    temp = i-w-w+2;
    if(temp>=0 && (temp%getXScreenNum()-2)==i%getXScreenNum())
    {
      direct_ninescreen[2][i].push_back(temp);
    }
    temp = i-w-2;
    if(temp>=0 && (temp%getXScreenNum()+2)==i%getXScreenNum())
    {
      direct_ninescreen[8][i].push_back(temp);
      direct_ninescreen[7][i].push_back(temp);
    }
    temp = i-w+2;
    if(temp>=0 && temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-2)==i%getXScreenNum())
    {
      direct_ninescreen[2][i].push_back(temp);
      direct_ninescreen[3][i].push_back(temp);
    }
    temp = i-2;
    if( temp>=0 && (temp%getXScreenNum()+2)==i%getXScreenNum())
    {
      direct_ninescreen[8][i].push_back(temp);
      direct_ninescreen[7][i].push_back(temp);
      direct_ninescreen[6][i].push_back(temp);
    }
    temp = i+2;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-2)==i%getXScreenNum())
    {
      direct_ninescreen[2][i].push_back(temp);
      direct_ninescreen[3][i].push_back(temp);
      direct_ninescreen[4][i].push_back(temp);
    }
    temp = i+w-2;
    if(temp>=0 && temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()+2)==i%getXScreenNum())
    {
      direct_ninescreen[7][i].push_back(temp);
      direct_ninescreen[6][i].push_back(temp);
    }
    temp = i+w+2;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-2)==i%getXScreenNum())
    {
      direct_ninescreen[3][i].push_back(temp);
      direct_ninescreen[4][i].push_back(temp);
    }
    temp = i+w+w-2;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()+2)==i%getXScreenNum())
    {
      direct_ninescreen[6][i].push_back(temp);
    }
    temp = i+w+w-1;
    if (temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()+1)==i%getXScreenNum())
    {
      direct_ninescreen[6][i].push_back(temp);
      direct_ninescreen[5][i].push_back(temp);
    }
    temp = i+w+w;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum())==i%getXScreenNum())
    {
      direct_ninescreen[6][i].push_back(temp);
      direct_ninescreen[5][i].push_back(temp);
      direct_ninescreen[4][i].push_back(temp);
    }
    temp = i+w+w+1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-1)==i%getXScreenNum())
    {
      direct_ninescreen[5][i].push_back(temp);
      direct_ninescreen[4][i].push_back(temp);
    }
    temp = i+w+w+2;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-2)==i%getXScreenNum())
    {
      direct_ninescreen[4][i].push_back(temp);
    }
  }
  /*
#ifdef _LX_DEBUG
for(DWORD r=0; r<8;r++)
for( DWORD i=0; i<getScreenSize(); i++)
{
for(DWORD m=0;m<direct_ninescreen[r][i].size();m++)
XDBG("Scene r:%u,posi:%u, rpos:%u", r,i,direct_ninescreen[r][i][m]);
}
#endif
*/
}

/*
 ** 移动时如果切屏需要删除的屏，移动方向同direct_ninescreen
 ** 8      1      2
 **  ______|_____
 ** |      |     |
 ** |      |     |
 **7|______|_____|3
 ** |      |     |
 ** |      |     | 
 ** |______|_____|
 ** 6	  5     4
 */
void xSceneEntryIndex::fillRDirectNineScreen()
{
  int temp = -1;
  for (DWORD i=0; i<9; i++)
  {
    rdirect_ninescreen[i].clear();
  }
  for( DWORD i=0; i<getScreenSize(); i++)//getScreenSize()个坐标
  {
    temp = i+getXScreenNum()+1;//右下
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-1)==i%getXScreenNum())
    {
      rdirect_ninescreen[8][i].push_back(temp);
      rdirect_ninescreen[1][i].push_back(temp);
      rdirect_ninescreen[2][i].push_back(temp);
      rdirect_ninescreen[7][i].push_back(temp);
      rdirect_ninescreen[6][i].push_back(temp);
    }
    temp = i+getXScreenNum();
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum())==i%getXScreenNum() )
    {
      rdirect_ninescreen[8][i].push_back(temp);
      rdirect_ninescreen[1][i].push_back(temp);
      rdirect_ninescreen[2][i].push_back(temp);
    }
    temp = i+getXScreenNum()-1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()+1)==i%getXScreenNum() )
    {
      rdirect_ninescreen[8][i].push_back(temp);
      rdirect_ninescreen[1][i].push_back(temp);
      rdirect_ninescreen[2][i].push_back(temp);
      rdirect_ninescreen[3][i].push_back(temp);
      rdirect_ninescreen[4][i].push_back(temp);
    }
    temp = i+1;
    if(temp<(SDWORD)getScreenSize() && (temp%getXScreenNum()-1)==i%getXScreenNum() )
    {
      rdirect_ninescreen[8][i].push_back(temp);
      rdirect_ninescreen[7][i].push_back(temp);
      rdirect_ninescreen[6][i].push_back(temp);
    }
    temp = i-1;
    if(temp>=0 && (temp%getXScreenNum()+1)==i%getXScreenNum())
    {
      rdirect_ninescreen[2][i].push_back(temp);
      rdirect_ninescreen[3][i].push_back(temp);
      rdirect_ninescreen[4][i].push_back(temp);
    }
    temp = i-getXScreenNum()+1;
    if(temp>=0 && (temp%getXScreenNum()-1)==i%getXScreenNum())
    {
      rdirect_ninescreen[8][i].push_back(temp);
      rdirect_ninescreen[7][i].push_back(temp);
      rdirect_ninescreen[6][i].push_back(temp);
      rdirect_ninescreen[5][i].push_back(temp);
      rdirect_ninescreen[4][i].push_back(temp);
    }
    temp = i-getXScreenNum();
    if(temp>=0 && (temp%getXScreenNum())==i%getXScreenNum())
    {
      rdirect_ninescreen[6][i].push_back(temp);
      rdirect_ninescreen[5][i].push_back(temp);
      rdirect_ninescreen[4][i].push_back(temp);
    }
    temp = i-getXScreenNum()-1;
    if(temp>=0 && (temp%getXScreenNum()+1)==i%getXScreenNum())
    {
      rdirect_ninescreen[2][i].push_back(temp);
      rdirect_ninescreen[3][i].push_back(temp);
      rdirect_ninescreen[6][i].push_back(temp);
      rdirect_ninescreen[5][i].push_back(temp);
      rdirect_ninescreen[4][i].push_back(temp);
    }
  }
  /*#ifdef _WUWENJUAN_DEBUG
    for(DWORD r=0; r<8;r++)
    for( DWORD i=0; i<getScreenSize(); i++)
    {
    for(DWORD m=0;m<rdirect_ninescreen[r][i].size();m++)
    XDBG("Scene rr:%u,posi:%u, rpos:%u", r,i,rdirect_ninescreen[r][i][m]);
    }
#endif
*/
}

void xSceneEntryIndex::getNineScreen(xPosI posi, std::set<DWORD> &uSet)
{
  NineScreen::iterator iter = ninescreen.find(posi);
  if (iter != ninescreen.end())
    uSet.insert(iter->second.begin(), iter->second.end());
}

void xSceneEntryIndex::getEntryListInScreen( SCENE_ENTRY_TYPE entryType, xPosI posi, xSceneEntrySet& uSet)
{
  if (!isValidEntryType(entryType)) return;

  PosIIndex::const_iterator indexIter = index[entryType].find(posi);
  if (indexIter != index[entryType].end())
    uSet.insert(indexIter->second.begin(), indexIter->second.end());
}

void xSceneEntryIndex::getEntryListInNine(SCENE_ENTRY_TYPE entryType, xPosI posi, xSceneEntrySet& uSet)
{
  if (!isValidEntryType(entryType)) return;

  NineScreen::iterator iter = ninescreen.find(posi);
  if (iter != ninescreen.end())
  {
    xPosIVector::const_iterator posIter = iter->second.begin();
    for ( ;posIter!= iter->second.end(); ++posIter)
    {
      PosIIndex::const_iterator indexIter = index[entryType].find(*posIter);
      if (indexIter != index[entryType].end())
        uSet.insert(indexIter->second.begin(), indexIter->second.end());
    }
  }
}

void xSceneEntryIndex::getAllEntryList( SCENE_ENTRY_TYPE entryType, xSceneEntrySet& uSet ) 
{
  if (!isValidEntryType(entryType))
    return;

  PosIIndex::const_iterator posIter = index[entryType].begin();
  for(; posIter!=index[entryType].end(); posIter++)
  {
    uSet.insert( posIter->second.begin(), posIter->second.end());
  }
}

/*
 **	          w
 ** upos _____________ 
 **     |             |
 **     |             |
 **     |             |  h
 **     |             |
 **     |_____________|
 */
void xSceneEntryIndex::getEntryListInBlock( SCENE_ENTRY_TYPE entryType, xPos upos, float w, float h, xSceneEntrySet &uSet ) 
{
  if(!isValidEntryType(entryType) || !isValidPos(upos))
    return;

  if (w > 1000 || h > 1000)
    return;

  w = abs(w);
  h = abs(h);
  //xPos sourcePos = upos;                  
  //xPos targetPos(sourcePos.x+w, 0, sourcePos.z+h);
   /*                                          
  **>          w                             
  **      _____________-                     
  **     |             |                     
  **     |             |                     
  **     |    upos     |  h                  
  **     |             |                     
  **     |_____________|                     
  */                                         
  xPos sourcePos(upos.x - w/2, 0, upos.z - h/2);
  xPos targetPos(upos.x + w/2, 0, upos.z + h/2);
  targetPos = checkPos(targetPos);
  sourcePos = checkPos(sourcePos);

  xPosIVector posVec;
  xPosIVector checkPosVec;

  xPosI x1 = xPos2xPosI(sourcePos);
  xPos p(targetPos.x, 0, sourcePos.z);
  xPosI x2 = xPos2xPosI( checkPos(p) );
  p = xPos(sourcePos.x, 0, targetPos.z);
  xPosI y1 = xPos2xPosI(checkPos(p));

  DWORD width = abs((int)getColumn(x2) - (int)getColumn(x1) + 1);
  DWORD height = abs((int)getLine(y1) - (int)getLine(x1) + 1);

  for(DWORD i=0; i<width; i++)
  {
    for(DWORD n=0; n<height; n++)
    {
      DWORD tp = x1+i+n*getXScreenNum();
      if( getLine(tp)==getLine(x1) || getLine(tp)==getLine(y1) || 
          getColumn(tp)==getColumn(x1) || getColumn(tp)==getColumn(x2) )
        checkPosVec.push_back(tp);
      else
        posVec.push_back(tp);
    }
  }

  PosIIndex::const_iterator indexIter;
  xPosIVector::const_iterator posVecIter = posVec.begin();
  for (; posVecIter != posVec.end(); ++posVecIter)
  {
    indexIter = index[entryType].find(*posVecIter);
    if (indexIter != index[entryType].end())
      uSet.insert(indexIter->second.begin(), indexIter->second.end());
  }

  Rect rect(sourcePos, targetPos.x-sourcePos.x, targetPos.z-sourcePos.z);
  posVecIter = checkPosVec.begin();
  for (; posVecIter != checkPosVec.end(); ++posVecIter)
  {
    indexIter = index[entryType].find(*posVecIter);
    if (indexIter == index[entryType].end())
      continue;

    xSceneEntrySet::const_iterator entryIter = indexIter->second.begin();
    for (; entryIter != indexIter->second.end(); ++entryIter)
    {
      if ((*entryIter) && rect.InRect((*entryIter)->getPos()))
        uSet.insert(*entryIter);
    }
  }
}

void xSceneEntryIndex::foreachEntryInBlock(const SCENE_ENTRY_TYPE entryType, xPos upos, const float radius, xSceneEntryCallback& strategy)
{
  if(!isValidEntryType(entryType) || !isValidPos(upos))
    return;

  xPos sourcePos = xPos(upos.x-radius, upos.y, upos.z-radius);
  xPos targetPos(upos.x+radius, upos.y, upos.z+radius); 
  sourcePos = checkPos(sourcePos);
  targetPos = checkPos(targetPos);

  xPosIVector posVec;

  xPosI x1 = xPos2xPosI(sourcePos);
  xPos p(targetPos.x, 0, sourcePos.z);
  xPosI x2 = xPos2xPosI( checkPos(p) );
  p = xPos(sourcePos.x, 0, targetPos.z);
  xPosI y1 = xPos2xPosI( checkPos(p) );

  DWORD width = abs((int)getColumn(x2) - (int)getColumn(x1) + 1);
  DWORD height = abs((int)getLine(y1) - (int)getLine(x1) + 1);

  for(DWORD i=0; i<width; i++)
  {
    for(DWORD n=0; n<height; n++)
    {
      posVec.push_back(x1+i+n*getXScreenNum());
    }
  }

  for (xPosIVector::const_iterator posVecIter = posVec.begin(); posVecIter!=posVec.end(); ++posVecIter)
  {
    PosIIndex::const_iterator indexIter = index[entryType].find(*posVecIter);
    if (indexIter == index[entryType].end())
      continue;

    xSceneEntrySet::const_iterator entryIter = indexIter->second.begin();
    for (; entryIter != indexIter->second.end();)
    {
      xSceneEntrySet::const_iterator curIter = entryIter++;
      if (*curIter && checkDistance(upos, (*curIter)->getPos(), radius))
      {
        if (SCENE_ENTRY_NPC==entryType)
        {
          /*
             if (((SceneNpc *)(*curIter))->base 
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_MONSTER
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_NO_BLOCK_MONSTER
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_NO_SELECT
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_USER
             )
             continue;
             */
        }

        if (strategy.checkCondition(*curIter))
        {
          strategy.collect(*curIter);
          if (strategy.isEnough())
            return;
        }
      }
    }
  }
}

void xSceneEntryIndex::getEntryListInBlock( SCENE_ENTRY_TYPE entryType, xPos upos, float radius, xSceneEntrySet &uSet )
{
  if(!isValidEntryType(entryType) || !isValidPos(upos))
    return;

  if (radius > 1000)
    return;

  class InsertToSet: public xSceneEntryCallback
  {
    private:
      xSceneEntrySet& eSet;
    public: 
      InsertToSet(xSceneEntrySet& eSet): eSet(eSet) {}
      virtual void execImpl(xSceneEntry* pEntry)
      {
        if (pEntry)
          eSet.insert(pEntry);
      }
  };

  InsertToSet its(uSet);
  foreachEntryInBlock(entryType, upos, radius, its);
  its.execute();
}

void xSceneEntryIndex::foreachEntryInBlock(const SCENE_ENTRY_TYPE entryType, xPos upos, const float minRadius, const float maxRadius, xSceneEntryCallback& strategy)
{
  if(!isValidEntryType(entryType) || !isValidPos(upos))
    return;

  xPos sourcePos = xPos(upos.x-maxRadius, upos.y, upos.z-maxRadius);
  xPos targetPos(upos.x+maxRadius, upos.y, upos.z+maxRadius); 
  sourcePos = checkPos(sourcePos);
  targetPos = checkPos(targetPos);

  xPosIVector posVec;

  xPosI x1 = xPos2xPosI(sourcePos);
  xPos p(targetPos.x, 0, sourcePos.z);
  xPosI x2 = xPos2xPosI( checkPos(p) );
  p = xPos(sourcePos.x, 0, targetPos.z);
  xPosI y1 = xPos2xPosI( checkPos(p) );

  DWORD width = abs((int)getColumn(x2) - (int)getColumn(x1) + 1);
  DWORD height = abs((int)getLine(y1) - (int)getLine(x1) + 1);

  for(DWORD i=0; i<width; i++)
  {
    for(DWORD n=0; n<height; n++)
    {
      posVec.push_back(x1+i+n*getXScreenNum());
    }
  }

  for (xPosIVector::const_iterator posVecIter = posVec.begin(); posVecIter!=posVec.end(); ++posVecIter)
  {
    PosIIndex::const_iterator indexIter = index[entryType].find(*posVecIter);
    if (indexIter == index[entryType].end())
      continue;

    xSceneEntrySet::const_iterator entryIter = indexIter->second.begin();
    for (; entryIter != indexIter->second.end();)
    {
      xSceneEntrySet::const_iterator curIter = entryIter++;
      if (*curIter && checkDistance(upos, (*curIter)->getPos(), maxRadius))
      {
        if (SCENE_ENTRY_NPC==entryType)
        {
          /*
             if (((SceneNpc *)(*curIter))->base 
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_MONSTER
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_NO_BLOCK_MONSTER
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_NO_SELECT
             && ((SceneNpc *)(*curIter))->base->type!=NPC_TYPE_USER
             )
             continue;
             */
        }

        if (*curIter && checkDistance(upos, (*curIter)->getPos(), minRadius))
          continue;

        if (strategy.checkCondition(*curIter))
        {
          strategy.collect(*curIter);
          if (strategy.isEnough())
            return;
        }
      }
    }
  }
}

void xSceneEntryIndex::getEntryListInRing( SCENE_ENTRY_TYPE entryType, xPos upos, float minRadius, float maxRadius, xSceneEntrySet &uSet )
{
  if(!isValidEntryType(entryType) || !isValidPos(upos))
    return;

  if(minRadius >= maxRadius)
    return;

  if (maxRadius > 1000)
    return;

  class InsertToSet: public xSceneEntryCallback
  {
    private:
      xSceneEntrySet& eSet;
    public: 
      InsertToSet(xSceneEntrySet& eSet): eSet(eSet) {}
      virtual void execImpl(xSceneEntry* pEntry)
      {
        if (pEntry)
          eSet.insert(pEntry);
      }
  };

  InsertToSet its(uSet);
  foreachEntryInBlock(entryType, upos, minRadius, maxRadius, its);
  its.execute();
}

void xSceneEntryIndex::getEntryList(xPos pos, float r, xSceneEntrySet &set)
{
  set.clear();

  getEntryListInBlock(SCENE_ENTRY_USER, pos, r, set);

  getEntryListInBlock(SCENE_ENTRY_NPC, pos, r, set);
}

void xSceneEntryIndex::getEntryList(xPos pos, float w, float h, xSceneEntrySet &set)
{
  set.clear();

  getEntryListInBlock(SCENE_ENTRY_USER, pos, w, h, set);

  getEntryListInBlock(SCENE_ENTRY_NPC, pos, w, h, set);
}

void xSceneEntryIndex::sendCmdToScreen(const xPosIVector &nine, const void* cmd, unsigned short len, GateIndexFilter filter)
{
  BUFFER_CMD(forward, BroadcastTwoLevelIndexGatewayCmd);

  for (auto &iter : nine)
  {
    forward->i2s[forward->num++] = iter;
    if (forward->num>=9) break;
  }
  if (forward->num)
  {
    forward->i = m_dwMapID;
    forward->len = len;
    forward->filter = filter;
    bcopy(cmd, forward->data, (DWORD)len);
    thisServer->sendCmdToServer(forward, sizeof(BroadcastTwoLevelIndexGatewayCmd) + len, "GateServer");
  }
}

void xSceneEntryIndex::sendCmdToNine(xPos pos, const void* cmd, unsigned short len, GateIndexFilter filter)
{
  xPosI posi = xPos2xPosI(pos);
  NineScreen::const_iterator screenIter = ninescreen.find(posi);
  if (screenIter != ninescreen.end())
  {
    BUFFER_CMD(forward, BroadcastTwoLevelIndexGatewayCmd);

    for (auto &posIter : screenIter->second)
    {
      forward->i2s[forward->num++] = posIter;
      if (forward->num >= 9) break;
    }
    if (forward->num)
    {
      forward->i = m_dwMapID;
      forward->len = len;
      forward->filter = filter;
      bcopy(cmd, forward->data, (DWORD)len);
      thisServer->sendCmdToServer(forward, sizeof(BroadcastTwoLevelIndexGatewayCmd) + len, "GateServer");
    }
  }
}

void xSceneEntryIndex::getNineScreen(xPos pos, xPosIVector &vec)
{
  vec.clear();
  xPosI posi = xPos2xPosI(pos);
  NineScreen::const_iterator screenIter = ninescreen.find(posi);
  if (screenIter != ninescreen.end())
  {
    xPosIVector::const_iterator posIter = screenIter->second.begin();
    for(; posIter!= screenIter->second.end(); ++posIter)
      vec.push_back(*posIter);
  }
}

bool xSceneEntryIndex::addEntryAtPosI(xSceneEntry* entry)
{
  if (!entry)
    return false;
  SCENE_ENTRY_TYPE entryType = entry->getEntryType();
  if (!isValidEntryType(entryType))
  {
    XERR << "[添加屏索引]," << entry->id << "," << entry->name << "," << entryType << XEND;
    return false;
  }
  if (!isValidPos(entry->getPos()))
  {
    DWORD sceneid = entry->getScene() == nullptr ? 0 : entry->getScene()->id;
    string scenename = entry->getScene() == nullptr ? "" : entry->getScene()->name;
    XERR << "[添加屏索引]" << entry->id << entry->name << entryType
      << "在地图" << sceneid << scenename << "min_x :" << get_x_min() << "max_x :" << get_x_max() << "min_z :" << get_z_min() << "max_z :" << get_z_max()
      << "(" << entry->getPos().x << entry->getPos().y << ")坐标不合法" << XEND;
    return false;
  }
  xPosI posi = entry->getPosI();
  index[entryType][posi].insert(entry);
  if (SCENE_ENTRY_USER == entryType)
  {
    SceneUser *pUser = (SceneUser *)entry;
    pUser->addTwoLevelIndex(TWO_LEVEL_INDEX_TYPE_SCREEN, m_dwMapID, posi);
  }
//#ifdef _LX_DEBUG
//  XDBG("[添加索引],%s,%llu,posi:%u,entry:%p", entry->name, entry->id, posi, entry);
//#endif
  return true;
}

void xSceneEntryIndex::delEntryAtPosI(xSceneEntry* entry, xPosI posi)
{
  if(!entry) return;
  SCENE_ENTRY_TYPE entryType = entry->getEntryType();
  if(!isValidEntryType(entryType)) return;
//#ifdef _LX_DEBUG
//  XDBG("[删除索引],%s,%llu,posi:%u,entry:%p", entry->name, entry->id, posi, entry);
//#endif
  PosIIndex::iterator indexIter = index[entryType].find(posi);
  if (indexIter != index[entryType].end())
  {
    xSceneEntrySet::iterator entryIter = indexIter->second.find(entry);
    if(entryIter != indexIter->second.end())
    {
      indexIter->second.erase(entry);
      if (SCENE_ENTRY_USER == entryType)
        ((SceneUser *)entry)->delTwoLevelIndex(TWO_LEVEL_INDEX_TYPE_SCREEN, m_dwMapID, posi);
      return;
    }
  }
  XERR << "[删除屏索引]," << entry->id << "," << entry->name << "," << entry->getTempID() << ",Did't find at posi:" << posi << XEND;
}

void xSceneEntryIndex::delEntryAtPosI(xSceneEntry* entry)
{
  if (entry)
    delEntryAtPosI(entry, entry->getPosI());
}

void xSceneEntryIndex::delEntryAtOldPosI(xSceneEntry* entry)
{
  if (entry)
    delEntryAtPosI(entry, entry->getOldPosI());
}

Direction xSceneEntryIndex::getDirectByPos(const xPos from, const xPos to)
{
  if (from == to) return DIR_DEFAULT;
  if (from.x<to.x)
  {
    if (from.y<to.y)
      return DIR_RIGHTDOWN;
    else if (from.y>to.y)
      return (DIR_UPRIGHT);
    else
      return (DIR_RIGHT);
  }
  else if (from.x>to.x)
  {
    if (from.y<to.y)
      return (DIR_LEFTDOWN);
    else if (from.y>to.y)
      return(DIR_UPLEFT);
    else
      return (DIR_LEFT);
  }
  else
  {
    if (from.y<to.y)
      return (DIR_DOWN);
    else if (from.y>to.y)
      return (DIR_UP);
    else
      return DIR_DEFAULT;
  }
  return DIR_DEFAULT;
}

Direction xSceneEntryIndex::getDirectByPosI(const xPosI from, const xPosI to)
{
  if (from == to) return DIR_DEFAULT;
  DWORD fx = from % getXScreenNum(); 
  DWORD fy = from / getXScreenNum() + 1; 
  DWORD tx = to % getXScreenNum(); 
  DWORD ty = to / getXScreenNum() + 1; 
  if (fx<tx)
  {
    if (fy<ty)
      return DIR_RIGHTDOWN;
    else if (fy>ty)
      return (DIR_UPRIGHT);
    else
      return (DIR_RIGHT);
  }
  else if (fx>tx)
  {
    if (fy<ty)
      return (DIR_LEFTDOWN);
    else if (fy>ty)
      return(DIR_UPLEFT);
    else
      return (DIR_LEFT);
  }
  else
  {
    if (fy<ty)
      return (DIR_DOWN);
    else if (fy>ty)
      return (DIR_UP);
    else
      return DIR_DEFAULT;
  }
  return DIR_DEFAULT;
}

void xSceneEntryIndex::changeScreen(xSceneEntryDynamic* entry)
{
  if (!entry ||!entry->getScene()) return;

  BYTE dir = getDirectByPosI(entry->getOldPosI(), entry->getPosI());
  bool inNine = check2PosInNine(entry->getOldPos(), entry->getPos());

  delEntryAtOldPosI(entry);
  addEntryAtPosI(entry);

  NineScreen::const_iterator rposVecIter = rdirect_ninescreen[dir].find(entry->getOldPosI());
  NineScreen::const_iterator posVecIter = direct_ninescreen[dir].find(entry->getOldPosI());

  switch (entry->getEntryType())
  {
    case SCENE_ENTRY_USER:
      {
        SceneUser *pUser = (SceneUser *)entry;

        if (inNine)
        {
          changeScreenDelOldEntry(pUser, dir, entry->getOldPosI(), entry->getPosI());
          changeScreenAddNewEntry(pUser, dir, entry->getOldPosI(), entry->getPosI());
        }
        else
        {
          delNineToUser(pUser, entry->getOldPosI());
          pUser->sendNineToMe();
        }
      }
      break;
    case SCENE_ENTRY_NPC:
      {
        SceneNpc *npc = (SceneNpc *)entry;
        if (npc->isMask())
          return;

        //删除自己
        Cmd::DeleteEntryUserCmd delMe;
        delMe.add_list(entry->getTempID());
        PROTOBUF(delMe, delme, delmeSize);

        if (inNine)
        {
          if (rposVecIter != rdirect_ninescreen[dir].end())
          {
            xSceneEntrySet set;
            for (auto &iter : rposVecIter->second)
            {
              getEntryListInScreen(SCENE_ENTRY_USER, iter, set);
            }
            for (auto &iter : set)
            {
              if (npc->isVisableToSceneUser((SceneUser *)iter))
              {
                ((SceneUser *)iter)->sendCmdToMe(delme, delmeSize);
                npc->informUserDel((SceneUser*)iter);
              }
            }
          }
          if (posVecIter != direct_ninescreen[dir].end())
          {
            //添加自己
            Cmd::AddMapNpc cmd;
            npc->fillMapNpcData(cmd.add_npcs());
            PROTOBUF(cmd, send, len);

            bool checkhide = npc->getAttr(EATTRTYPE_HIDE) && npc->getScene() && npc->getScene()->isHideUser();

            xSceneEntrySet set;
            for (auto &iter : posVecIter->second)
            {
              getEntryListInScreen(SCENE_ENTRY_USER, iter, set);
            }
            for (auto &iter : set)
            {
              if (npc->isVisableToSceneUser((SceneUser *)iter))
              {
                if (npc->isScreenLimit((SceneUser*)iter))
                  continue;
                if (checkhide && npc->isHideUser((SceneUser*)iter))
                  continue;
                ((SceneUser *)iter)->sendCmdToMe(send, len);
                npc->informUserAdd((SceneUser*)iter);
              }
            }
          }
        }
        else
        {
          xSceneEntrySet set;
          getEntryListInNine(SCENE_ENTRY_USER, npc->getOldPos(), set);
          for (auto &iter : set)
          {
            if (npc->isVisableToSceneUser((SceneUser *)iter))
            {
              ((SceneUser *)iter)->sendCmdToMe(delme, delmeSize);
              npc->informUserDel((SceneUser*)iter);
            }
          }

          npc->sendMeToNine();
        }
      }
      break;
    default:
      break;
  }
}

//删除反向屏对象
void xSceneEntryIndex::changeScreenDelOldEntry(SceneUser *pUser, BYTE dir, xPosI oldPosI, xPosI newPosI)
{
  NineScreen::const_iterator rposVecIter = rdirect_ninescreen[dir].find(oldPosI);
  if (rposVecIter != rdirect_ninescreen[dir].end())
  {
    for (xPosIVector::const_iterator posIter = rposVecIter->second.begin(); posIter!= rposVecIter->second.end(); ++posIter)
    {
      if (newPosI==*posIter) continue;
      delDynamicEntry(pUser, *posIter);
    }
    refreshScope(pUser);
  }
}

//添加正向屏对象
void xSceneEntryIndex::changeScreenAddNewEntry(SceneUser *pUser,BYTE dir,xPosI oldPosI,xPosI newPosI)
{
  if (!pUser) return;

  NineScreen::const_iterator posVecIter = direct_ninescreen[dir].find(oldPosI);
  if (posVecIter != direct_ninescreen[dir].end())
  {
    for (xPosIVector::const_iterator posIter = posVecIter->second.begin(); posIter!= posVecIter->second.end(); ++posIter)
    {
      if (newPosI==*posIter) continue;
      {
        PosIIndex::const_iterator userIndexIter = index[SCENE_ENTRY_USER].find(*posIter);
        if (userIndexIter != index[SCENE_ENTRY_USER].end())
        {
          for (xSceneEntrySet::const_iterator it = userIndexIter->second.begin(); it != userIndexIter->second.end(); ++it)
          {
            // 限制隐匿可见性
            SceneUser *user = (SceneUser *)(*it);
            m_oScope.add(pUser, user);
          }
          refreshScope(pUser);
        }

        xSceneEntrySet uSet1;
        getEntryListInScreen(SCENE_ENTRY_ITEM, *posIter, uSet1);
        if (!uSet1.empty())
        {
          Cmd::AddMapItem cmd;
          //xSceneEntrySet::iterator it = uSet.begin(),end = uSet.end();
          for (auto it=uSet1.begin(); it!=uSet1.end(); ++it)
          {
            SceneItem* item = (SceneItem *)(*it);
            if (item != nullptr)
              item->fillMapItemData(cmd.add_items());
          }
          PROTOBUF(cmd, send, len);
          pUser->sendCmdToMe(send, len);
        }

        {
          xSceneEntrySet uSet1;
          getEntryListInScreen(SCENE_ENTRY_TRAP, *posIter, uSet1);
          if (!uSet1.empty())
          {
            Cmd::AddMapTrap cmd;
            //xSceneEntrySet::iterator it = uSet.begin(),end = uSet.end();
            for (auto it=uSet1.begin(); it!=uSet1.end(); ++it)
            {
              SceneTrap* item = (SceneTrap *)(*it);
              if (item != nullptr)
              {
                SceneUser* user = item->getScreenUser();
                if (user && !inScope(user, pUser))
                  continue;

                item->fillMapTrapData(cmd.add_traps());
              }
            }
            if (cmd.traps_size() > 0)
            {
              PROTOBUF(cmd, send, len);
              pUser->sendCmdToMe(send, len);
            }
          }
        }

        {
          xSceneEntrySet uSet1;
          getEntryListInScreen(SCENE_ENTRY_ACT, *posIter, uSet1);
          if (!uSet1.empty())
          {
            //xSceneEntrySet::iterator it = uSet.begin(),end = uSet.end();
            for (auto it=uSet1.begin(); it!=uSet1.end(); ++it)
            {
              SceneActBase* pact = (SceneActBase *)(*it);
              if (pact == nullptr || pUser == nullptr)
                continue;
              if (pact->viewByUser(pUser->id) == false)
                continue;
              pact->sendMeToUser(pUser);
            }
          }
        }

        {
          xSceneEntrySet uSet2;
          getEntryListInScreen(SCENE_ENTRY_BOOTH, *posIter, uSet2);
          if (!uSet2.empty())
          {
            Cmd::AddMapUser cmd;
            //xSceneEntrySet::iterator it = uSet.begin(),end = uSet.end();
            for (auto it=uSet2.begin(); it!=uSet2.end(); ++it)
            {
              SceneBooth* booth = (SceneBooth*)(*it);
              if (!booth || !pUser)
                continue;

              booth->fillData(cmd.add_users());
            }
            if(cmd.users_size() > 0)
            {
              PROTOBUF(cmd, send, len);
              pUser->sendCmdToMe(send, len);
            }
          }
        }

        sendNpcEntry(pUser, *posIter);
      }
    }
  }
}

void xSceneEntryIndex::addFunNpc(xSceneEntryDynamic *npc)
{
  if (!npc) return;

  funNpcIndex.insert(npc);
}

void xSceneEntryIndex::delFunNpc(xSceneEntryDynamic *npc)
{
  if (!npc) return;

  funNpcIndex.erase(npc);
}

void xSceneEntryIndex::sendFunNpc(xSceneEntryDynamic *user)
{
  if (!user) return;

  Cmd::FuntionNpcListUserCmd cmd;

  for (std::set<xSceneEntryDynamic *>::iterator it=funNpcIndex.begin(); it!=funNpcIndex.end(); it++)
  {
    if ((*it) && SCENE_ENTRY_NPC==(*it)->getEntryType())
    {
      Cmd::FunNpcData *pData = cmd.add_list();
      pData->set_npcid(((SceneNpc *)(*it))->getNpcID());
      pData->set_tempid(((SceneNpc *)(*it))->getTempID());
      Cmd::ScenePos *p = pData->mutable_pos();
      p->set_x((*it)->getPos().getX());
      p->set_y((*it)->getPos().getY());
      p->set_z((*it)->getPos().getZ());
    }
  }
  PROTOBUF(cmd, send, len);
  user->sendCmdToMe(send, len);
}

bool xSceneEntryIndex::isEmpty()const
{
  PosIIndex::const_iterator it=index[SCENE_ENTRY_USER].begin(),end=index[SCENE_ENTRY_USER].end();
  for(;it!=end;it++)
  {
    if(!it->second.empty())
      return false;
  }
  return true;
}

DWORD xSceneEntryIndex::getUserNum()const
{
  DWORD num=0;
  PosIIndex::const_iterator it=index[SCENE_ENTRY_USER].begin(),end=index[SCENE_ENTRY_USER].end();
  for(;it!=end;it++)
  {
    num+=it->second.size();
  }
  return num;
}

DWORD xSceneEntryIndex::getNpcNum()const
{
  DWORD num=0;
  PosIIndex::const_iterator it=index[SCENE_ENTRY_NPC].begin(),end=index[SCENE_ENTRY_NPC].end();
  for(;it!=end;it++)
  {
    num+=it->second.size();
  }
  return num;
}

DWORD xSceneEntryIndex::getNineScreenUserNum(xPosI posi)
{
  //DWORD dwNum = 0;

  xSceneEntrySet uSet;
  NineScreen::iterator iter = ninescreen.find(posi);
  if (iter != ninescreen.end())
  {
    xPosIVector::const_iterator posIter = iter->second.begin();
    for ( ; posIter != iter->second.end(); ++posIter)
    {
      PosIIndex::const_iterator indexIter = index[SCENE_ENTRY_USER].find(*posIter);
      if (indexIter != index[SCENE_ENTRY_USER].end())
      {
        uSet.insert(indexIter->second.begin(), indexIter->second.end());
        //dwNum += indexIter->second.size();
      }
    }
  }

  return uSet.size();
  //return dwNum;
}

void xSceneEntryIndex::delNineToUser(SceneUser* entry,xPosI posi)
{
  if (!entry) return;

  xPosIVector::const_iterator posIter = ninescreen[posi].begin(), posEnd = ninescreen[posi].end();
  for ( ; posIter!=posEnd; ++posIter)
  {
    delDynamicEntry(entry, *posIter);
  }
  refreshScope(entry);
}

void xSceneEntryIndex::sendNpcEntry(SceneUser *entry, xPosI posi)
{
  if (!entry) return;
  PosIIndex::const_iterator npcIndexIter = index[SCENE_ENTRY_NPC].find(posi);
  if (npcIndexIter != index[SCENE_ENTRY_NPC].end())
  {
    Cmd::AddMapNpc cmd;

    std::list<DWORD> gearlist;
    std::set<SceneNpc*> npclist;
    std::set<SceneNpc*> addnpc;

    DWORD i = 0;
    bool checkhide = entry->getScene() && entry->getScene()->isHideUser();
    for (xSceneEntrySet::const_iterator iter = npcIndexIter->second.begin(); iter != npcIndexIter->second.end(); ++iter)
    {
      if ((*iter) && SCENE_ENTRY_NPC == (*iter)->getEntryType())
      {
        SceneNpc* npc = (SceneNpc *)(*iter);
        if (npc && npc->getCFG() != nullptr)
        {
          if (!npc->checkNineScreenShow(entry)) continue;
          if (npc->isScreenLimit(entry)) continue;
          if (checkhide && npc->isHideUser(entry)) continue;

          npc->fillMapNpcData(cmd.add_npcs());
          addnpc.insert(npc);

          if (npc->define.isGear())
          {
            gearlist.push_back(npc->define.getUniqueID());
          }

          if (++i>=MAX_SEND_NPC_NUM)
          {
            PROTOBUF(cmd, send, len);
            entry->sendCmdToMe(send, len);
            i = 0;
            cmd.Clear();
          }
          if (npc->getGearStatus() != 0 || npc->getSpecialGearStatus())
          {
            npclist.insert(npc);
          }
        }
      }
    }
    if (cmd.npcs_size())
    {
      PROTOBUF(cmd, send, len);
      entry->sendCmdToMe(send, len);
    }
    if (!gearlist.empty())
    {
      if (entry->getScene())
        entry->getScene()->m_oGear.send(entry, gearlist);
    }
    for (auto &p : npclist)
      p->sendGearStatus(entry);
    for (auto &p : addnpc)
      p->informUserAdd(entry);
  }
 // XLOG("[NpcIndex],%llu,%s,添加posi:%u", entry->id, entry->name, posi);
}

void xSceneEntryIndex::delDynamicEntry(SceneUser *pUser, xPosI posi)
{
  if (!pUser) return;

  PosIIndex::const_iterator userIndexIter = index[SCENE_ENTRY_USER].find(posi);
  if (userIndexIter != index[SCENE_ENTRY_USER].end())
  {
    for (xSceneEntrySet::const_iterator it = userIndexIter->second.begin(); it != userIndexIter->second.end(); ++it)
    {
      m_oScope.remove(pUser, (SceneUser *)(*it));
    }
  }

  Cmd::DeleteEntryUserCmd cmd;

  PosIIndex::const_iterator npcIndexIter = index[SCENE_ENTRY_NPC].find(posi);
  if (npcIndexIter != index[SCENE_ENTRY_NPC].end())
  {
    for (xSceneEntrySet::const_iterator it = npcIndexIter->second.begin(); it != npcIndexIter->second.end(); ++it)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*it);
      if (pUser != nullptr && pNpc != nullptr)
        pNpc->informUserDel(pUser);
      cmd.add_list((*it)->getTempID());
      if (cmd.list_size()>=5000)
      {
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        cmd.Clear();
      }
    }
  }

  PosIIndex::const_iterator itemIndexIter = index[SCENE_ENTRY_ITEM].find(posi);
  if (itemIndexIter != index[SCENE_ENTRY_ITEM].end())
  {
    for (xSceneEntrySet::const_iterator it = itemIndexIter->second.begin(); it != itemIndexIter->second.end(); ++it)
    {
      cmd.add_list((*it)->getTempID());
      if (cmd.list_size()>=5000)
      {
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        cmd.Clear();
      }
    }
  }

  PosIIndex::const_iterator trapIndexIter = index[SCENE_ENTRY_TRAP].find(posi);
  if (trapIndexIter != index[SCENE_ENTRY_TRAP].end())
  {
    for (xSceneEntrySet::const_iterator it = trapIndexIter->second.begin(); it != trapIndexIter->second.end(); ++it)
    {
      cmd.add_list((*it)->getTempID());
      if (cmd.list_size()>=5000)
      {
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        cmd.Clear();
      }
    }
  }

  PosIIndex::const_iterator actIndexIter = index[SCENE_ENTRY_ACT].find(posi);
  if (actIndexIter != index[SCENE_ENTRY_ACT].end())
  {
    for (xSceneEntrySet::const_iterator it = actIndexIter->second.begin(); it != actIndexIter->second.end(); ++it)
    {
      SceneActEvent* pAct = dynamic_cast<SceneActEvent*>(*it);
      if (pAct)
        pAct->onUserOut(pUser);

      cmd.add_list((*it)->getTempID());
      if (cmd.list_size()>=5000)
      {
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        cmd.Clear();
      }
    }
  }

  PosIIndex::const_iterator boothIndexIter = index[SCENE_ENTRY_BOOTH].find(posi);
  if (boothIndexIter != index[SCENE_ENTRY_BOOTH].end())
  {
    for (xSceneEntrySet::const_iterator it = boothIndexIter->second.begin(); it != boothIndexIter->second.end(); ++it)
    {
      cmd.add_list((*it)->getTempID());
      if (cmd.list_size()>=5000)
      {
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToMe(send, len);
        cmd.Clear();
      }
    }
  }

  if (cmd.list_size())
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
  // XLOG("[NpcIndex],%llu,%s,删除posi:%u", entry->id, entry->name, posi);
}

void xSceneEntryIndex::getNpcVecByUniqueID(DWORD uniqueid, VecSceneNpc &vec)
{
  if (!uniqueid) return;

  for (auto it : index[SCENE_ENTRY_NPC])
  {
    for (auto &iter : it.second)
    {
      SceneNpc *npc = (SceneNpc *)iter;
      if (npc->define.getUniqueID() == uniqueid)
      {
        vec.push_back(npc);
      }
    }
  }
}

/*
 *
 * ___________________________
 *|                           |
 *|                           |
 *|p1                      p2 | w
 *|                           |
 *|___________________________|
 *
*/

void xSceneEntryIndex::getEntryIn2Pos(const xPos& pos1, const xPos& pos2, float w, xSceneEntrySet& uSet)
{
  float l = getDistance(pos1, pos2);

  // get entrys in circle
  xSceneEntrySet cirSet;
  xPos pos0 = pos1;
  pos0.x = (pos1.x + pos2.x) / 2;
  pos0.z = (pos1.z + pos2.z) / 2;
  getEntryList(pos0, (l + w) / 2, cirSet);

  // p1 -> p2 : line equation (Ax + Bz + C = 0)
  float A = pos1.z - pos2.z;
  float B = pos2.x - pos1.x;
  float C = pos1.x * pos2.z - pos2.x * pos1.z;
  if (A == 0 && B == 0)
    return;

  auto checkAngeleIn90 = [](const xPos& p0, const xPos& p1, const xPos& p2) ->bool
  {
    //v1⋅v2=||v1||||v2||cosθ
    float v1x = p1.x - p0.x;
    float v1z = p1.z - p0.z;
    float v2x = p2.x - p0.x;
    float v2z = p2.z - p0.z;

    float v1v2 = v1x * v2x + v1z * v2z;
    float v1_len = sqrt(v1x * v1x + v1z * v1z);
    float v2_len = sqrt(v2x * v2x + v2z * v2z);

    return v1v2 / (v1_len * v2_len) >= 0;
  };
  // check (point : line |Ax+By+C| / √(A^2 + B^2)) < w/2  &&  dis < maxDis
  auto checkIn = [&] (xPos p) -> bool
  {
    float dis2Line = abs(A * p.x + B * p.z + C) / sqrt(A * A + B * B);
    if (dis2Line > w / 2.0)
      return false;

    if (!checkAngeleIn90(pos1, pos2, p) || !checkAngeleIn90(pos2, pos1, p))
      return false;
    return true;
  };

  for (auto &s : cirSet)
  {
    if (checkIn(s->getPos()))
      uSet.insert(s);
  }
}

void xSceneEntryIndex::addScope(SceneUser *from, SceneUser *to)
{
  if (!from || !to) return;

  m_oScope.add(from, to);
}

bool xSceneEntryIndex::inScope(SceneUser *from, SceneUser *to)
{
  if (!from || !to) return false;

  return m_oScope.inScope(from, to);
}

void xSceneEntryIndex::removeScope(SceneUser *from, SceneUser *to)
{
  if (!from || !to) return;

  m_oScope.remove(from, to);
}

void xSceneEntryIndex::refreshScope(SceneUser *from)
{
  if (!from) return;

  xScope *pUserScope = m_oScope.get(from);
  if (pUserScope)
  {
    pUserScope->refresh(&m_oScope);
  }
}

void xSceneEntryIndex::addScope(SceneUser *from)
{
  if (!from) return;

  m_oScope.get(from);
}

void xSceneEntryIndex::destroyScope(SceneUser *from)
{
  if (!from) return;

  m_oScope.destroy(from);
}

void xSceneEntryIndex::sendCmdToScope(SceneUser *pUser, const void *cmd, DWORD len)
{
  if (!pUser) return;

  xScope *pUserScope = m_oScope.get(pUser->id);
  if (pUserScope)
  {
    pUserScope->sendCmdToAll(cmd, len);
  }
}

void xSceneEntryIndex::sendUserToScope(SceneUser *pUser)
{
  m_oScope.sendUserToScope(pUser);
}

void xSceneEntryIndex::sendUserToUser(SceneUser *from, SceneUser *to)
{
  m_oScope.sendUserToUser(from, to);
}

void xSceneEntryIndex::hideMeToScope(SceneUser *pUser)
{
  m_oScope.hideMeToScope(pUser);
}

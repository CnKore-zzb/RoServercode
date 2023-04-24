#pragma once
#include "SceneDefine.h"
#include "xPos.h"

typedef std::map<BYTE,BYTE> BYTE_MAP;
typedef std::pair<BYTE,BYTE> BYTE_MAP_PAIR;
typedef std::set<DWORD> DYNAMIC_ENTRY_SET;

/* 长方形或菱形
 * 长方形时pos为坐上点坐标
 * 菱形时pos为中心点坐标
 */
struct Rect
{
  xPos pos;
  DWORD width;
  DWORD height;
  Rect() { width=0; height=0; }
  Rect(DWORD sx,DWORD sy, DWORD w, DWORD h) { pos.x=sx; pos.z=sy; width=w; height=h; }
  Rect(xPos p, DWORD w, DWORD h) { pos=p; width=w; height=h; }

  //是否在长方形区域内
  bool InRect(xPos p)const { return (p.x>=pos.x && p.x<=(pos.x+width) && p.z>=pos.z && p.z<=(pos.z+height)); }
  //是否在菱形区域内
  bool InDiamond(xPos p)const
  {
    if(p.x>(pos.x+width)|| p.z>(pos.z+height) || (p.x+width)<pos.x || (p.z+height)<pos.z)
      return false;
    if(p.x>pos.x)
      p.x=p.x-(p.x-pos.x)*2;
    if(p.z>pos.z)
      p.z=p.z-(p.z-pos.z)*2;
    float l=(float)std::abs((pos.z-p.z))/(float)std::abs(pos.x-p.x-width);
    return ( l<= float(height)/float(width));
  }
  bool isIn(BYTE type,xPos pos)const
  {
    if(type==1)
      return InDiamond(pos);
    else
      return InRect(pos);
  }
};

//地图特殊区域
struct Area : public Rect
{
  /*区域id的作用:
    进入或者离开区域时判断当前所处区域和上个时钟周期所处区域是否是同一个区域，以方便调用AREA_TIMER_EVENT,ENTER_AREA_EVENT,LEAVE_AREA_EVENT等函数
    如果不需要调用以上函数，id可填为0.*/
  BYTE id;
  DWORD type;
  DWORD scriptID;
  BYTE tax;
  BYTE shape;		//0方形,1菱形 
  DWORD endTime;
  BYTE country;	//所属国家
  Area()
  {
    id=0;
    type=0;
    scriptID=0;
    tax=0;
    shape=0;
    endTime=0;
    country=0;
  }
  bool timeUp(DWORD cur)const{return endTime && endTime<cur;}
  bool isIn(xPos pos)const{ return Rect::isIn(shape,pos);}
};

struct WayPoint
{
  WORD tomap;
  WORD tocountry;
  WORD worldlevel;
  WORD minlevel;
  WORD maxlevel;
  WORD isweakcountry;
  std::vector<xPos> posVec;
  WayPoint()
  {
    tomap=0;
    tocountry=0;
    worldlevel = minlevel = maxlevel = 0;
    isweakcountry = 0;
  }
  xPos getRandPos()const
  {
    return posVec[randBetween(0,posVec.size()-1)];
  }
};


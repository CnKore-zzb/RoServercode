#pragma once
#include "xTools.h"

/*
 ** 8   1    2
 **  _______
 ** |   |   |
 **7|___|___|3_
 ** |   |   |
 ** |___|___|
 ** 6   5    4
 */
enum Direction
{
  DIR_NULL  = 0,
  DIR_UP    = 1,
  DIR_UPRIGHT = 2,
  DIR_RIGHT = 3,
  DIR_RIGHTDOWN = 4,
  DIR_DOWN = 5,
  DIR_LEFTDOWN	= 6,
  DIR_LEFT		= 7,
  DIR_UPLEFT		= 8,
  DIR_DEFAULT		= DIR_DOWN,
};

#define FLOAT_TO_DWORD 1000

struct xPos
{
  xPos() { x = 0.0; y = 0.0;	z = 0.0; }
  xPos(float fx, float fy, float fz):x(fx),y(fy),z(fz){}
  void set(int fx, int fy, int fz)
  {
    x = (float)fx / FLOAT_TO_DWORD;
    y = (float)fy / FLOAT_TO_DWORD;
    z = (float)fz / FLOAT_TO_DWORD;
  }
  bool empty() const { return x == 0.0 && y == 0.0 && z == 0.0; }
  void clear() { x = 0.0; y = 0.0; z = 0.0; }

  bool operator==(const xPos& p1)const{ return (p1.x==this->x && p1.y==this->y && p1.z==this->z);}
  bool operator!=(const xPos& p1)const{ return (p1.x!=this->x || p1.y!=this->y || p1.z!=this->z);}
  //不要改变此处的 大于、小于 判断，寻路算法依赖于此
  bool operator > (const xPos& p1) const { return z != p1.z ? z > p1.z : x > p1.x; }
  bool operator > (const float &dist) const { return x > dist && z > dist;}
  bool operator < (const xPos& p1) const { return z != p1.z ? z < p1.z : x < p1.x; }
  bool operator < (const float &dist) const { return x < dist && z < dist;}
  xPos& operator = (const xPos& p)
  {
    this->x = p.x;
    this->y = p.y;
    this->z = p.z;
    return *this;
  }

  // 传给客户端的值
  int getX() const { return FLOAT_TO_DWORD * x; }  
  int getY() const { return FLOAT_TO_DWORD * y; }  
  int getZ() const { return FLOAT_TO_DWORD * z; }  

  float x;
  float y;    // 纵轴
  float z;
};

inline xPos operator + (const xPos &p1, const xPos &p2)
{ return xPos(p1.x + p2.x, p1.y, p1.z + p2.z); }

inline xPos operator - (const xPos &p1, const xPos &p2)
{ return xPos(p1.x - p2.x, p1.y, p1.z - p2.z); }

inline xPos operator - (const xPos &p1, float d)
{ return xPos(p1.x - d, p1.y, p1.z - d); }

inline xPos operator * (const xPos &p1, const xPos &p2)
{ return xPos(p1.x * p2.x, p1.y, p1.z * p2.z); }

inline xPos operator * (const xPos &p1, float step)
{ return xPos(p1.x * step, p1.y, p1.z * step); }

inline float getDistance(const xPos &p1, const xPos &p2)
{
  return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y)+(p1.z-p2.z)*(p1.z-p2.z));
}

inline float getXZDistance(const xPos &p1, const xPos &p2)
{
  return sqrt((p1.x-p2.x)*(p1.x-p2.x)+ (p1.z-p2.z)*(p1.z-p2.z));
}

inline bool checkDistance(const xPos &p1, const xPos &p2, float distance)
{ return (p1.x-p2.x)*(p1.x-p2.x)+(p1.z-p2.z)*(p1.z-p2.z) <= distance*distance; }

inline float getRadius(const xPos &p1, const xPos &p2)
{ return sqrt(square(p1.x - p2.x) + square(p1.z - p2.z)); }

inline bool checkRadius(const xPos &p1, const xPos &p2, const float &radius)
{ return (square(p1.x - p2.x) + square(p1.z - p2.z)) <= square(radius); }

struct xRotation
{
  xRotation()
  {
    x = y = z = w = 0.0;
  }
  xRotation(float fx, float fy, float fz, float fw):x(fx),y(fy),z(fz),w(fw){}
  // 传给客户端的值
  int getX() const { return FLOAT_TO_DWORD * x; }
  int getY() const { return FLOAT_TO_DWORD * y; }
  int getZ() const { return FLOAT_TO_DWORD * z; }
  int getW() const { return FLOAT_TO_DWORD * w; }
  float x;
  float y;
  float z;
  float w;
};

inline xPos getPosByDir(const xPos &source, const xPos &target, float dist)
{
  xPos ret;

  float dx = target.x - source.x;
  float dz = target.z - source.z;

  float d = getDistance(source, target);
  if (d < 1.0f)
    d = 1.0f;

  ret.x = dx * dist / d + source.x;
  ret.z = dz * dist / d + source.z;
  ret.y = source.y;

  return ret;
}

inline float calcAngle(const xPos& mePos, const xPos& tarPos)
{
  // 没有考虑 y 轴影响
  /*
    x 90°
    |
    |
    |__________________z 0°
    |
    |
    |
    |
  */
  float zdelta = tarPos.z - mePos.z;
  float xdelta = tarPos.x - mePos.x;
  zdelta = (zdelta != 0 ? zdelta : 0.001f);
  float slope = xdelta / zdelta;

  float angle = 0.0f;
  if (zdelta > 0 && xdelta >= 0) // phase 1
  {
    angle = atan(slope) * 180 / 3.14;
  }
  else if (zdelta < 0 && xdelta >= 0) // phase 2
  {
    angle = (atan(slope) + 3.14) * 180 / 3.14;
  }
  else if (zdelta < 0 && xdelta <= 0) // phase 3
  {
    angle = (atan(slope) + 3.14) * 180 / 3.14;
  }
  else // phase 4
  {
    angle = (atan(slope) + 6.28) * 180 / 3.14;
  }

  return angle > 0 ? angle : 0;
}


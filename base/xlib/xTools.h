#pragma once

#include "xDefine.h"
#include <string.h>
#include <random>
#include <sys/socket.h>
#include <stdarg.h>
#include <cassert>
#include "xTime.h"
#include "xCommand.h"
#include "zlib.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <iostream>
#include <sstream>
#include <iconv.h>
#include "xLog.h"

struct Tools
{
  public:
    Tools(){};

    static std::map<std::string, std::string> global;
    static bool initGlobal(const char *configFile);

  private:
    // static std::uniform_int<> distribution(0, 99);
    static std::mt19937 _engine;
};

UINT stringTok(std::string s, std::string k, std::vector<std::string> &v);
UINT stringTokAll(std::string s, std::string k, std::vector<std::string> &v);

template<typename T>
inline void numTok(std::string s,std::string k,std::vector<T> &v)
{
  std::vector<std::string> sv;
  stringTok(s,k,sv);
  for(UINT n=0;n<sv.size();n++)
  {
    v.push_back(atol(sv[n].c_str()));
  }
}

template<typename T>
inline void numTok(std::string s,std::string k,std::set<T> &v)
{
  std::vector<std::string> sv;
  stringTok(s,k,sv);
  for(UINT n=0;n<sv.size();n++)
  {
    v.insert(atol(sv[n].c_str()));
  }
}

template <typename T>
inline T* constructInPlace(T* p)
{
  return new(p) T;
}

/*template <typename T>
  inline void* constructInPlace(T* p)
  {
  new (static_cast<void *>(p)) T();
  }
  */

//#define SAFE_DELETE(p) do {delete p; p = NULL;} while(false)

//#define SAFE_DELETE_VEC(p) do {delete[] p; p = NULL;} while(false)

/*
   inline void SAFE_CLOSE_HANDLE(HANDLE &h)
   {
   XDBG("[Handle]close %p", h);
   CloseHandle(h);
   h = 0;
   }
   */

inline int randBetween(int min, int max)    // [min, max]
{
  if (max==min) return min;
  unsigned int gap = abs(max-min);
  int ret = max>min?min:max;
  ret += rand()%(gap+1);
  return ret;
}

/*@brief:从一个stl 容器里随机一个值，注意返回的是指针*/
template<typename A>
typename A::pointer randomStlContainer(A& container)
{
  typename A::pointer ret;
  if (container.empty())
    return nullptr;
  if (container.size() == 1)
  {
    auto it = container.begin();
    ret = const_cast<typename A::pointer>(&(*it));
    return ret;
  }
  DWORD s = randBetween(0, container.size() - 1);
  auto it = container.begin();
  std::advance(it, s);
  ret = const_cast<typename A::pointer>(&(*it));
  return ret;
}

inline float randFBetween(float min, float max)
{
  if (max==min) return min;
  max *= 1000.0f;
  min *= 1000.0f;
  unsigned int gap = std::abs(max - min);

  float random = rand() % (gap + 1);

  float ret = (max > min) ? min : max;

  random = random * 10 / 10.0f;
  ret += random;

  return ret / 1000.0f;
}

inline bool selectByPercent(int value)
{
  if(value>=randBetween(1,100))
    return true;
  return false;
}
inline bool selectByThousand(int value)
{
  if(value>=randBetween(1,1000))
    return true;
  return false;
}
inline bool selectByTenThousand(int value)
{
  if(value>=randBetween(1,10000))
    return true;
  return false;
}

#define parseFormat(buffer, fmt) \
  va_list argptr;\
va_start(argptr, fmt);\
vsnprintf(buffer, sizeof(buffer), fmt, argptr);\
va_end(argptr)

inline string formatArgs(const char* fmt, ...)
{
  char szBuff[2048] = {0};
  va_list arg;
  va_start(arg, fmt);
  vsnprintf(szBuff, sizeof(szBuff), fmt, arg);
  va_end(arg);
  return std::string(szBuff);
}

template<typename T, typename M>
inline void Clamp(T &val, const M &min, const M &max)
{
  if (val < min) { val = min; return; }
  if (val > max) { val = max; return; }
}

template<typename T>
inline T square(const T &val)
{
  return val * val;
}

template<typename T>
inline T cube(const T &val)
{
  return val * val * val;
}

struct Parser
{
  Parser()
  {
    map.clear();
  }
  void reset()
  {
    for (std::map<std::string, std::string>::iterator it=map.begin(); it!=map.end(); it++)
      it->second.clear();
  }
  void key(std::string s)
  {
    std::string str("");
    map.insert(std::make_pair(s, str));
  }
  std::string value(std::string s)
  {
    return map[s];
  }
  std::map<std::string, std::string> map;
};

inline void moneyNumToStr(UINT money,std::stringstream& str)
{
  str.str("");
  if (!money)
  {
    str << "0文";
    return;
  }
  UINT t=money/1000/1000;
  if(t)
  {
    str<<t<<"锭";
    money=money-t*1000*1000;
  }
  t=money/1000;
  if(t)
  {
    str<<t<<"两";
    money=money-t*1000;
  }
  t=money;
  if(t)
    str<<t<<"文";
}

inline void timeNumToStr(UINT time,std::stringstream& str)
{
  str.str("");
  if(time==0)
  {
    str<<"0秒";
    return;
  }
  UINT day = time/60/60/24;
  if(day)
  {
    str<<day<<"天";
    time-=day*24*60*60;
  }
  UINT h=time/60/60;
  if(h)
  {
    str<<h<<"小时";
    time=time-h*60*60;
  }
  h=time/60;
  if(h)
  {
    str<<h<<"分钟";
    time=time-h*60;
  }
  h=time;
  if(h)
    str<<h<<"秒";
}

static const char *CHINESE_NUM_STR[] = 
{ "零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", 
  "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十", 
  "二十一", "二十二", "二十三", "二十四", "二十五", "二十六", "二十七", "二十八", "二十九", "三十" };
inline const char *getChineseNumStr(UINT i)
{
  if (i < sizeof(CHINESE_NUM_STR) / sizeof(const char *))
    return CHINESE_NUM_STR[i];
  return "";
}

static const char* CHINESE_DAY[] = {"日","一","二","三","四","五","六"};
inline const char* getChineseDay(UINT i)
{
  if ( i<sizeof(CHINESE_DAY)/sizeof(char*))
    return CHINESE_DAY[i];
  else
    return "";
}

static const char *PROFESSION_NAME_ARRAY[] = {"战士","弓手","谋士","道士"};
inline const char *getProfessionName(UINT profession)
{
  for(UINT n=0;n<4;n++)
  {
    if( ( (0x01<<n)&profession)!=0 )
      return PROFESSION_NAME_ARRAY[n];
  }
  return "";
}

const UINT QUALITY_COLOR[] = { 0xffffffff, 0xff307af4, 0xffffdc00, 0xff50f050, 0xffc85afa, 0xfff07903 };
inline UINT GetQualityColor(BYTE color)
{
  if(color>6)
    return QUALITY_COLOR[5];
  return (color >= 1 && color <= 6) ? QUALITY_COLOR[color - 1] : 0xffffffff;
}

inline UINT genAccIDKey(UINT zoneID, UINT accid)
{
  return (zoneID << 48) | accid;
}

inline void addslashes(std::string &str)
{
  std::string::size_type p = str.find("\\\\\\");
  while (p != std::string::npos){
    str.replace(p, 3, "\\"); 
    p = str.find("\\\\\\");
  }

  p = str.find('\'');
  while (p != std::string::npos){
    if (p == 0 || (p > 0 && str[p - 1] != '\\') || (p > 1 && str[p - 1] == '\\' && str[p - 2] == '\\'))
    {
      str.replace(p, 1, "\\\'"); 
      p = str.find('\'', p + 2);
    }
    else
      p = str.find('\'', p + 1);
  }

  //去除最后一个'\'
  if (!str.empty() && str[str.size() - 1] == '\\')
    str[str.size() - 1] = '0';
}

inline void addslashes(char *in, DWORD inputLen, DWORD outputLen)
{
  std::string str(in, inputLen);
  addslashes(str);
  if (outputLen < str.size())
  {
    XLOG << "[addslashes]" << "输出异常" << "input:" << in << inputLen << "output:" << str << str.size() << XEND;
    strncpy(in, str.c_str(), outputLen);
    return;
  }
  strncpy(in, str.c_str(), str.size());
}

static const char *COLOR_STR[] = { "白", "蓝", "黄", "绿", "紫", "橙" };
inline const char* getColorStr(UINT color)
{
  return (color >= 1 && color <= 6) ? COLOR_STR[color - 1] : "";
}

  template <typename T>
inline void stringToNum(std::string str,const char* tok, T& target)
{
  std::vector<std::string> strvec;
  stringTok(str,tok,strvec);
  for(UINT n=0;n<strvec.size();n++)
  {
    target.push_back(atoi(strvec[n].c_str()));
  }
}

//cur到2011年1月3号的周数
inline UINT getWeek(UINT cur)
{
  DWORD begin = 0;
  parseTime("2011-01-03 00:00:00", begin);
  if (cur<=begin) return 0;

  return (cur-begin)/(60*60*24*7);
}

//cur到2011年1月3号的天数
inline UINT getDays(UINT cur)
{
  DWORD begin = 0;
  parseTime("2011-01-03 00:00:00", begin);
  if (cur<=begin) return 0;

  return (cur-begin)/(60*60*24);
}

  template <typename T>
T clamp(const T& x, const T& min, const T& max)
{
  return std::min<T>(std::max<T>(min , x), max);
};

//按频率分布随机选择一个返回下标
  template<typename FreqT>
UINT randomSelect(const std::vector<FreqT>& distribution)
{
  UINT sumFreq = 0;
  typedef typename std::vector<FreqT>::const_iterator Iter;
  for (Iter iter = distribution.begin(); iter != distribution.end(); ++iter)
  {
    sumFreq += *iter;
  }

  const UINT r = randBetween(1, sumFreq);
  UINT ptSum = 1;
  for (UINT i = 0; i < distribution.size(); ++i)
  {
    if (r >= ptSum && r < ptSum + distribution[i])
    {
      return i;
    }
    ptSum += distribution[i];
  }

  return distribution.size();
}

  template<typename HasFreq, typename FreqT>
UINT randomSelect(const std::vector<HasFreq>& distribution)
{
  std::vector<FreqT> distVec;
  typedef typename std::vector<HasFreq>::const_iterator Iter;
  for (Iter iter = distribution.begin(); iter != distribution.end(); ++iter)
  {
    distVec.push_back(iter->dist);
  }
  return randomSelect(distVec);
}

inline void replaceAll(std::string& str, const char* key, const char* newKey)
{
  std::size_t pos = 0;
  while ((pos = str.find(key, pos)) != std::string::npos)
  {
    str.replace(pos, 1, newKey);
  }
}

inline const char* getSortTitle(UINT sort)
{
  switch(sort)
  {
    case 1:
      return "冠军";
    case 2:
      return "亚军";
    case 3:
      return "季军";
    case 4:
      return "殿军";
    default:
      return "";
  }
}

inline void require(bool condition)
{
#ifdef _DEBUG
  assert(condition);
#endif
}

static std::string upyun_api_key = "BHsE9P9E5gr3Daf0WQTc58nshL4=";

bool base64Encode(const std::string& input, std::string* output);
bool base64Decode(const std::string& input, std::string* output);
bool upyun_form_str(std::string savekey, std::string &out_policy, std::string &out_signature);

// add bin data base64Encode
void base64Encode(const unsigned char* bindata, char* base64, int binlength);

DWORD getWordCount(const std::string& str);

inline bool compress(const std::string& in, std::string& out)
{
  if (in.empty() == true)
    return true;

  std::vector<BYTE> vec;
  vec.resize(MAX_BINDATA_SIZE);
  QWORD newLen = vec.size();
  int result = ::compress2((Bytef *)&vec[0], (uLongf *)&newLen, (Bytef *)&in[0], (uLong)in.size(), 6);
  if (Z_OK == result)
  {
    out.clear();
    out.assign((char*)&vec[0], newLen);
    return true;
  }

  XERR << "[失败失败] result :" << result << XEND;
  return false;
}

inline bool uncompress(const std::string& in, std::string& out)
{
  if (in.empty() == true)
    return true;
  std::vector<BYTE> vec;
  vec.resize(MAX_BINDATA_SIZE);
  QWORD newLen = vec.size();
  /*if (Z_OK != ::uncompress((Bytef *)&vec[0], (uLongf *)&newLen, (Bytef *)in.c_str(), (uLong)in.size()))
    return false;*/
  int result = ::uncompress((Bytef *)&vec[0], (uLongf *)&newLen, (Bytef *)in.c_str(), (uLong)in.size());
  if (result != Z_OK)
  {
    XERR << "[解压失败] result :" << result << XEND;
    return false;
  }

  out.clear();
  out.assign((char*)&vec[0], newLen);
  return true;
}

inline DWORD getClientZoneID(DWORD dwZoneID)
{
  if (dwZoneID == 0)
    return 0;
  return dwZoneID % (dwZoneID / 10000 * 10000);
}

inline DWORD getServerZoneID(DWORD dwSvrZone, DWORD dwLine)
{
  if (dwLine == 0 || dwSvrZone == 0)
    return 0;
  return dwSvrZone / 10000 * 10000 + dwLine;
}

inline bool add_data(UserData* pData, EUserDataType eType, QWORD qwValue, const string& data = "")
{
  if (pData == nullptr)
    return false;

  pData->set_type(eType);
  pData->set_value(qwValue);
  pData->set_data(data);
  return true;
}

inline bool add_attr(UserAttr* pAttr, EAttrType eType, DWORD dwValue)
{
  if (pAttr == nullptr)
    return false;

  pAttr->set_type(eType);
  pAttr->set_value(dwValue);
  return true;
}

inline std::string getZoneName(DWORD dwZoneId)
{
  static std::vector<string> sVecSuffix = { "", "a", "c", "d", "e", "f", "g", "h", "k", "m", "n", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "aa", "cc", "dd", "ee", "ff", "gg", "hh", "kk" };

  DWORD t = dwZoneId % 10000;
  if (t >= 9000)
    return "斗技场线";

  DWORD len = sVecSuffix.size();
  float f = ((float)t) / len;
  DWORD n1 = std::ceil(f);
  DWORD n2 = t % len;
  if (n2 == 0)
  {
    n2 = len;
  }
  n2 -= 1;

  stringstream ss;
  ss << n1 << sVecSuffix[n2];
  return ss.str();
}

unsigned char ToHex(unsigned char x);

unsigned char FromHex(unsigned char x);

std::string UrlEncode(const std::string& str);

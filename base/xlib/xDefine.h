#pragma once

//#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <algorithm>
#include <list>

#include "xNewMem.h"

#include "SceneItem.pb.h"
#include "SceneUser.pb.h"

//#pragma warning(disable:4099)  // LNK4099
//#pragma warning(disable:4996)
//#pragma warning(disable:4200)
//#pragma warning(disable:4355)

typedef long long INT;
typedef unsigned long long UINT;

typedef char CHAR;
typedef char SBYTE;
typedef short SWORD;
typedef int SDWORD;
typedef long long SQWORD;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long long QWORD;

typedef std::vector<DWORD> TVecDWORD;
typedef std::vector<QWORD> TVecQWORD;
typedef std::vector<float> TVecFloat;
typedef std::vector<std::string> TVecString;
typedef std::set<QWORD> TSetQWORD;
typedef std::set<DWORD> TSetDWORD;
typedef std::set<std::string> TSetString;
typedef std::list<DWORD> TListDWORD;

typedef std::map<std::string, std::string> LuaTable;
typedef std::map<std::string, LuaTable> LuaMultiTable;


typedef UINT ACCID;
typedef UINT CHARID;
typedef CHARID USER_ID;
typedef SWORD xPosI;

typedef std::vector<Cmd::ItemInfo> TVecItemInfo;
typedef std::vector<Cmd::ItemData> TVecItemData;
typedef std::vector<Cmd::UserAttrSvr> TVecAttrSvrs;
typedef std::vector<Cmd::EItemType> TVecItemType;

#define _TY(x) #x
#define _S(x) _TY(x)
//#define SAFE_DELETE(p) do { delete p; p = NULL;} while(false)
//#define SAFE_DELETE_VEC(p) do {delete[] p; p = NULL;} while(false)

const std::string STRING_EMPTY = "";

const SBYTE SBYTE_MIN = static_cast<SBYTE>(0x80);  // -128
const SBYTE SBYTE_MAX = static_cast<SBYTE>(0x7f);  // 127
const SWORD SWORD_MIN = static_cast<SWORD>(0x8000);  // -32768
const SWORD SWORD_MAX = static_cast<SWORD>(0x7fff);  // 32767
const SDWORD SDWORD_MIN = static_cast<SDWORD>(0x80000000);  // -2147483648
const SDWORD SDWORD_MAX = static_cast<SDWORD>(0x7fffffff);  // 2147483647
const SQWORD SQWORD_MIN = static_cast<SQWORD>(0x8000000000000000); // -9223372036854775808
const SQWORD SQWORD_MAX = static_cast<SQWORD>(0x7fffffffffffffff); // 9223372036854775807

const BYTE BYTE_MIN = static_cast<BYTE>(0);
const BYTE BYTE_MAX = static_cast<BYTE>(0xff);  // 255
const WORD WORD_MIN = static_cast<WORD>(0);
const WORD WORD_MAX = static_cast<WORD>(0xffff);  // 65535
const DWORD DWORD_MIN = static_cast<DWORD>(0);
const DWORD DWORD_MAX = static_cast<DWORD>(0xffffffff);// 4294967295
const QWORD QWORD_MIN = static_cast<QWORD>(0);
const QWORD QWORD_MAX = static_cast<QWORD>(0xffffffffffffffff); // 18446744073709551615

const DWORD ONE_THOUSAND = 1000;
const DWORD TEN_THOUSAND = 10000;
const DWORD ONE_MILLION = 1000000;
const DWORD ONE_BILLION = 1000000000;

const DWORD MIN_T = 60;
const DWORD HOUR_T = 3600;
const DWORD DAY_T = 86400;
const DWORD WEEK_T = (7 * 86400);
const DWORD DAYS_30_T = (30 * DAY_T);

const DWORD LEN_8 = 8;
const DWORD LEN_16 = 16;
const DWORD LEN_32 = 32;
const DWORD LEN_40 = 40;
const DWORD LEN_64 = 64;
const DWORD LEN_128 = 128;
const DWORD LEN_256 = 256;
const DWORD LEN_512 = 512;
const DWORD LEN_1024 = 1024;
const DWORD MAX_NAMESIZE = 64;

const DWORD HMAC_SHA1_LEN = 20;
const DWORD SHA1_LEN = 40;
const DWORD MAX_SERVER_EVENT = 256;

const DWORD USER_SOURCE_LEN = 128;
const DWORD ACC_NAME_LEN = 64;
const DWORD VIRTUAL_SCENE_ID = 8000;
const DWORD MAX_GATE_WAIT_TIME = 86400;

const DWORD MAX_BINDATA_SIZE = (1 << 19);
const DWORD MAX_USER_DATA_SIZE = 256;
const DWORD MAX_CHAR_NUM = 3;
const DWORD BASE_CHAR_NUM = 2;
const DWORD ACC_UNCOMPRESS_VERSION = 3;

template <class T>
class __mt_alloc
{
  public:
    char* allocate(size_t len) { return (char *)malloc(len); }
    void deallocate(unsigned char* ptr, size_t len) { free(ptr); }
};

enum class TerminateMethod
{
  terminate_no = 0,
  terminate_socket_error = 1,     // 检测到socket异常
  terminate_active = 2,           // task主动断开
};

enum ONE_LEVEL_INDEX_TYPE
{
  ONE_LEVEL_INDEX_TYPE_MAP  = 0,
  ONE_LEVEL_INDEX_TYPE_TEAM = 2,
  ONE_LEVEL_INDEX_TYPE_ROOM = 3,
  ONE_LEVEL_INDEX_TYPE_BARRAGE = 4,
  ONE_LEVEL_INDEX_TYPE_MAX,
};

enum TWO_LEVEL_INDEX_TYPE
{
  TWO_LEVEL_INDEX_TYPE_SCREEN = 0,
  TWO_LEVEL_INDEX_TYPE_MAX = 1,
};

enum EError
{
  EERROR_SUCCESS = 1,
  EERROR_FAIL = 2,
  EERROR_DB_ERROR = 3,
  EERROR_REDIS_ERROR = 4,
  EERROR_DATA_ERROR = 5,
  EERROR_NOT_EXIST = 6,
  EERROR_PET_NOT_EXIST = 7,
  EERROR_PET_TOUCH_MAX = 8,
  EERROR_PET_TOUCH_CD = 9,
  EERROR_PET_FEED_MAX = 10,
  EERROR_PET_FEED_CD = 11,
};

struct IDValue
{
  DWORD id;
  DWORD value;
  IDValue()
  {
    bzero(this, sizeof(*this));
  }
};

struct MinMax
{
  DWORD min = 0;
  DWORD max = 0;
  DWORD getID()
  {
    return (min << 16) + max;
  }
};

//#define SERIAL_LEN 36
//#define _CRT_SECURE_NO_WARNINGS
//#define DEFAULT_CONFIG_PATH "./Config/"
//#define _IsUnused __attribute__ ((__unused__))
//#define MAX_DBDATA_SIZE (1<<24)
//#define RAND_CODE_LEN 20  // 随机码长度
//#define MD5_LEN 32  // md5码长度
//#define CITIZEN_ID_LEN 18  // 公民身份证长度
//#define CITIZEN_NAME_LEN 20  // 公民姓名长度
//#define ZONE_ID_MASK 32

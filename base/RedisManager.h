/**
 * @file RedisManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-01-28
 */

#pragma once

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <sstream>
#include <list>
#include "xSingleton.h"
#include "xLog.h"
//#include "Redis.pb.h"
#include "xLuaTable.h"

using std::string;
using std::vector;
using std::map;
using std::stringstream;
using google::protobuf::Message;
using namespace Cmd;

enum ERedisKeyType
{
  EREDISKEYTYPE_MIN = 0,
  EREDISKEYTYPE_SEAL = 1,
  EREDISKEYTYPE_SCENEUSER_NUM = 2,
  EREDISKEYTYPE_ONLINE_MAPID = 5,
  EREDISKEYTYPE_ITEMSHOW = 7,
  EREDISKEYTYPE_USERINFO = 11,
  EREDISKEYTYPE_TOWER = 12,
  EREDISKEYTYPE_ITEMIMAGE = 13,
  EREDISKEYTYPE_GUILD_DONATE_PATCH = 14,
  EREDISKEYTYPE_GLOBAL_CHAR_DATA = 15,
  EREDISKEYTYPE_CHAT_VOICE = 16,
  EREDISKEYTYPE_MAX_BASELV = 17,
  EREDISKEYTYPE_MONTH_CARD = 18,
  EREDISKEYTYPE_GUILD_BOSSRAID = 19,
  EREDISKEYTYPE_OPERATE_RWARD = 20,
  EREDISKEYTYPE_GATE_INFO = 21,
  EREDISKEYTYPE_USER_CREDIT = 22,
  EREDISKEYTYPE_TRADE_COUNT = 23,
  EREDISKEYTYPE_TRADE_DATA = 24,
  EREDISKEYTYPE_CREATE_CHAR_ZONEID = 25,
  EREDISKEYTYPE_ZONE_INFO = 26,
  EREDISKEYTYPE_REGION_INFO = 27,
  EREDISKEYTYPE_TRADE_PRICE = 28,
  EREDISKEYTYPE_MONTH_CARD_HEADDRESS = 29,
  EREDISKEYTYPE_ACTIVITY_INFO = 30,
  EREDISKEYTYPE_AUCTION_ORDERID = 31,
  EREDISKEYTYPE_AUCTION_ORDERIDSET = 32,
  EREDISKEYTYPE_TADE_UNTAKELOG = 33,
  EREDISKEYTYPE_TUTOR_REFUSE_PROTECT = 34,
  EREDISKEYTYPE_TUTOR_PUNISH = 35,
  EREDISKEYTYPE_ACTIVITY_EVENT = 36,
  EREDISKEYTYPE_TUTOR_REWARD_PUNISH = 37,
  EREDISKEYTYPE_TUTOR_TASK = 38,
  EREDISKEYTYPE_GUILD_OPT = 39,
  EREDISKEYTYPE_DEPOSIT_LIMIT = 40,
  EREDISKEYTYPE_TUTOR_MATCH = 41,
};

class RedisManager : public xSingleton<RedisManager>
{
  friend class xSingleton<RedisManager>;
  private:
    RedisManager();
  public:
    virtual ~RedisManager();

    bool init(const string& ip, const string& passwd, DWORD port);

    // set get
    template<typename T> bool setData(const string& key, const T& data, DWORD dwExpireTime = 0);
    template<typename T> bool getData(const string& key, T& out);

    // hset hget hgetall
    template<typename T> bool setListData(const string& listname, const string& key, const T& data);
    template<typename T> bool getListData(const string& listname, const string& key, T& out);
    template<typename T> bool getList(const string& listname, map<string, T>& mapDatas);

    // hash
    bool setHash(const string& key, const xLuaData &data);
    bool getHash(const string& key, xLuaData &data);
    bool getHashAll(const string& key, xLuaData &data);
    bool delHash(const string& key, const xLuaData &data);

    // set
    template<typename T> bool addSet(const string& key, T v);
    template<typename T> bool delSet(const string& key, T v);
    template<typename T> bool setExist(const string& key, T& v);

    bool addSet(const string& key, const std::list<string> &data);
    bool delSet(const string& key, const std::list<string> &data);
    bool getSetARand(const string& key, std::string &data);

    // set get
    bool setProtoData(const string& key, const Message* pCmd, DWORD dwExpireTime = 0);
    bool getProtoData(const string& key, Message* pCmd);
    bool delData(const string& key);

    // hset hget hgetall
    bool setProtoListData(const string& listname, const string& key, const Message* pCmd);
    bool getProtoListData(const string& listname, const string& key, Message* pCmd);
    bool getProtoList(const string& listname, TSetString& setStrings);
    bool delListData(const string& listname, const string& key);
    bool clearListData(const string& listname);

    //incr 将键的整数值加1
    bool incr(const string& key, QWORD& ret);

  public:
    template<typename T> string getKeyByParam(DWORD regionID, ERedisKeyType eType, const T& key, string field = string("data"));
    template<typename T, typename T2> string getKeyByParam(DWORD regionID, ERedisKeyType eType, const T& key, const T2& param2, string field = string("data"));
    string getTradePriceKey(DWORD regionID, ERedisKeyType eType);
    bool setExpire(const string& key, DWORD sec);

  private:
    bool checkConnect();
    bool checkStatus();
  private:
    bool disconnect();
    bool reconnect();
  private:
    redisContext* m_pConnect = nullptr;
};

// ------------------------------默认函数实现------------------------------

template<typename T>
bool RedisManager::setData(const string& key, const T& data, DWORD dwExpireTime)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "set ";
  sstr << key << " ";
  sstr << data;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-设置数据] key :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-设置数据] key :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-设置数据],key:" << key << "设置失败type:" << pReply->type << "err:" << pReply->str << sstr.str() << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  XLOG << "[Redis-设置数据],key:" << key << "设置数据:" << sstr.str() << XEND;
  if (dwExpireTime)
  {
    setExpire(key, dwExpireTime);
  }
  else
  {
    setExpire(key, DAYS_30_T);
  }
  return true;
}

template<typename T>
bool RedisManager::getData(const string& key, T& out)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr.str("");
  sstr << "get ";
  sstr << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-获取数据] key =" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-获取数据] key =" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-获取数据] key =" << key << "type =" << pReply->type << "err =" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  if (pReply->str == nullptr)
  {
    XERR << "[Redis-获取数据] key :" << key << "不存在" << XEND;
    freeReplyObject(pReply);
    return false;
  }

  sstr.str(pReply->str);
  sstr >> out;

  freeReplyObject(pReply);

  return true;
}

// hset hget hgetall
template<typename T>
bool RedisManager::setListData(const string& listname, const string& key, const T& data)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hset " << listname << " " << key << " " << data;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据设置] listname :" << listname << " key :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据设置] listname :" << listname << " key :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据设置] listname :" << listname << " key :" << key << " type :" << pReply->type << " err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  setExpire(key, DAYS_30_T);

  XLOG << "[Redis-列表数据设置] listname :" << listname << " key :" << key << "设置成功" << XEND;
  return true;
}

template<typename T>
bool RedisManager::getListData(const string& listname, const string& key, T& out)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hget " << listname << " " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据获取] listname :" << listname << "key :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据获取] listname :" << listname << "key :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "key :" << key << "type :" << pReply->type << "err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  if (pReply->str == nullptr)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "key :" << key << "不存在" << XEND;
    freeReplyObject(pReply);
    return false;
  }

  sstr.str(pReply->str);
  sstr >> out;

  freeReplyObject(pReply);
  return true;
}

template<typename T>
bool RedisManager::getList(const string& listname, map<string, T>& mapDatas)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hgetall " << listname;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据获取] listname :" << listname << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据获取] listname :" << listname << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "type :" << pReply->type << "err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }

  mapDatas.clear();
  for (size_t i = 0; i < pReply->elements; i += 2)
  {
    if (pReply->element[i]->str == nullptr || pReply->element[i + 1]->str == nullptr)
      continue;

    stringstream tstr(pReply->element[i + 1]->str);
    tstr >> mapDatas[pReply->element[i]->str];

    if (i + 2 > pReply->elements)
      break;
  }

  freeReplyObject(pReply);
  return true;
}

template<typename T>
bool RedisManager::setExist(const string& key, T& v)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "SISMEMBER " << key;
  sstr << " " << v;
  
  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Set检测是否存在],key:" << key <<sstr.str() << "重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Set检测是否存在],key:" << key <<sstr.str() << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-Set检测是否存在],key:" << key << sstr.str() << "err:" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }

  if (pReply->type != REDIS_REPLY_INTEGER)
  {
    freeReplyObject(pReply);
    return false;
  }
  bool bExist = false;
  if (pReply->integer == 1)
    bExist = true;
  freeReplyObject(pReply);

  XLOG << "[Redis-Set检测是否存在],key:" << key << sstr.str() << bExist << XEND;
  return bExist;
}


template<typename T>
string RedisManager::getKeyByParam(DWORD regionID, ERedisKeyType eType, const T& param, string field)
{
  return getKeyByParam(regionID, eType, param, "default", field);
}

template<typename T, typename T2>
string RedisManager::getKeyByParam(DWORD regionID, ERedisKeyType eType, const T& param, const T2& param2, string field)
{
  std::stringstream stream;
  stream.str("");
  stream << regionID << ":";
  switch (eType)
  {
    case EREDISKEYTYPE_SEAL:
      {
        stream << "SealOpenedUser";
      }
      break;
    case EREDISKEYTYPE_GLOBAL_CHAR_DATA:
      {
        stream << "gchardata";
      }
      break;
    case EREDISKEYTYPE_GATE_INFO:
      {
        stream << "gateinfo";
      }
      break;
    case EREDISKEYTYPE_SCENEUSER_NUM:
      {
        stream << "SceneServerOnline";
      }
      break;
    case EREDISKEYTYPE_ONLINE_MAPID:
      {
        stream << "onlinemapid";
      }
      break;
    case EREDISKEYTYPE_ITEMSHOW:
      {
        stream << "itemshow";
      }
      break;
    case EREDISKEYTYPE_USERINFO:
      {
        stream << "char_info";
      }
      break;
    case EREDISKEYTYPE_ITEMIMAGE:
      {
        stream << "itemimage";
      }
      break;
    case EREDISKEYTYPE_GUILD_DONATE_PATCH:
      {
        stream << "guild_donate_patch";
      }
      break;
    case EREDISKEYTYPE_CHAT_VOICE:
      {
        stream << "chat_voice";
      }
      break;
    case EREDISKEYTYPE_GUILD_BOSSRAID:
      {
        stream << "guild_bossraid";
      }
      break;
    case EREDISKEYTYPE_MAX_BASELV:
      {
        stream << "server_max_level";
      }
      break;
    case EREDISKEYTYPE_MONTH_CARD:
      {
        //check redis
        DWORD curSec = now();
        DWORD month = xTime::getMonth(curSec);
        stream << "month_card" <<month ;
      }
      break;
    case EREDISKEYTYPE_OPERATE_RWARD:
      {
        stream << "operate_reward";
      }
      break;
    case EREDISKEYTYPE_USER_CREDIT:
      {
        stream << "user_credit";
      }
      break;
    case EREDISKEYTYPE_TRADE_COUNT:
      {
        stream << "trade_count";
      }
      break;
    case EREDISKEYTYPE_TRADE_DATA:
      {
        stream << "trade_data";
      }
      break;
    case EREDISKEYTYPE_CREATE_CHAR_ZONEID:
      {
        stream << "create_char_zoneid";
      }
      break;
    case EREDISKEYTYPE_ZONE_INFO:
      {
        stream << "zone_info";
      }
      break;
    case EREDISKEYTYPE_REGION_INFO:
      {
        stream << "region_info";
      }
      break;
    case EREDISKEYTYPE_MONTH_CARD_HEADDRESS:
      {
        stream << "month_card_headdress";
      }
      break;
    case EREDISKEYTYPE_ACTIVITY_INFO:
      {
        stream << "activity_info";
      }
      break;
    case EREDISKEYTYPE_AUCTION_ORDERID:
    {
      stream << "auction_orderid";
    }
    break;
    case EREDISKEYTYPE_AUCTION_ORDERIDSET:
    {
      stream << "auction_orderidset";
    }
    break;
    case EREDISKEYTYPE_TADE_UNTAKELOG:
    {
      stream << "trade_untakelog";
    }
    break;
    case EREDISKEYTYPE_TUTOR_REFUSE_PROTECT:
      {
        stream << "tutor_refuse_protect";
      }
      break;
    case EREDISKEYTYPE_TUTOR_PUNISH:
      {
        stream << "tutor_punish";
      }
      break;
    case EREDISKEYTYPE_TUTOR_REWARD_PUNISH:
      {
        stream << "tutor_reward_punish";
      }
      break;
    case EREDISKEYTYPE_TUTOR_TASK:
      {
        stream << "tutor_task";
      }
      break;
    case EREDISKEYTYPE_ACTIVITY_EVENT:
    {
      stream << "activity_event";
    }
    break;
    case EREDISKEYTYPE_GUILD_OPT:
    {
      stream << "guild_opt";
    }
    break;
    case EREDISKEYTYPE_DEPOSIT_LIMIT:
    {
      stream << "deposit_limit";
    }
    break;
    case EREDISKEYTYPE_TUTOR_MATCH:
    {
      stream << "tutor_match";
    }
    break;
    default:
      {
        return "";
      }
      break;
  }
  stream << ":" << param << ":" << param2;
  if (!field.empty())
  {
    stream << ":" << field;
  }
  return stream.str().c_str();
}

template<typename T>
bool RedisManager::addSet(const string& key, T v)
{
  std::stringstream stream;
  stream.str("");
  stream << v;
  
  std::list<string> l;
  l.push_back(stream.str());
  return addSet(key, l);
}
template<typename T>
bool RedisManager::delSet(const string& key, T v)
{
  std::stringstream stream;
  stream.str("");
  stream << v;

  std::list<string> l;
  l.push_back(stream.str());
  return delSet(key, l);
}

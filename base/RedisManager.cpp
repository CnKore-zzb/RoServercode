#include "RedisManager.h"
#include "xServer.h"
//#include "BaseConfig.h"
//#include <iostream>
//#include <fstream>

RedisManager::RedisManager()
{

}

RedisManager::~RedisManager()
{
  disconnect();
}

bool RedisManager::init(const string& ip, const string& passwd, DWORD port)
{
  disconnect();

  m_pConnect = redisConnect(ip.c_str(), port);
  if (m_pConnect == nullptr)
  {
    XERR << "[Redis-初始化] 无法连接到 ip:" << ip << "port:" << port << "redis服务" << XEND;
    return false;
  }

  if (m_pConnect->err)
  {
    XERR << "[Redis-初始化] 无法连接到 ip:" << ip << "port:" << port << "redis服务" << m_pConnect->err << m_pConnect->errstr << XEND;
    disconnect();

    return false;
  }

  if (passwd.size())
  {
    stringstream sstr;
    sstr << "auth " << passwd;
    redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-初始化] 认证失败" << XEND;
      return false;
    }
    if (pReply->type == REDIS_REPLY_ERROR)
    {
      XERR << "[Redis-初始化] 认证失败 type:" << pReply->type << "err:" << pReply->str << XEND;
      freeReplyObject(pReply);
      return false;
    }
    freeReplyObject(pReply);
  }

  /*sstr.str("");
  sstr << "flushall";
  pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr || pReply->type == REDIS_REPLY_ERROR)
  {
    XERR("[Redis-初始化] 清空redis缓存数据失败 type : %u, err : %s", pReply == nullptr ? 0 : pReply->type, pReply == nullptr ? "" : pReply->str);
    return false;
  }*/

  XLOG << "[Redis-初始化] 成功连接 ip:" << ip << "port:" << port << XEND;
  return true;
}

bool RedisManager::setProtoData(const string& key, const Message* pCmd, DWORD dwExpireTime /*= 0*/)
{
  if (pCmd == nullptr)
    return false;
  if (!checkConnect()) return false;

  string data;
  if (pCmd->SerializeToString(&data) == false)
  {
    XERR << "[Redis-设置数据] key :" << key << "序列化失败" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << key;
  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, "set %b %b", sstr.str().c_str(), sstr.str().size(), data.c_str(), data.size()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-设置数据] key:" << key << "执行指令失败" << XEND;
    return false;
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-设置数据] key:" << key << "设置失败 type:" << pReply->type << "err:" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  if (dwExpireTime > 0)
  {
    setExpire(key, dwExpireTime);
  }
  else
  {
    setExpire(key, DAYS_30_T);
  }

  XLOG << "[Redis-设置数据] key:" << key << "设置数据 超时:" << dwExpireTime << XEND;
  return true;
}

bool RedisManager::getProtoData(const string& key, Message* pCmd)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr.str("");
  sstr << "get ";
  sstr << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-获取数据] key =" << key << "执行指令失败" << XEND;
    return false;
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-获取数据] key =" << key << "type =" << pReply->type << "err=" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  if (pReply->str == nullptr)
  {
    XERR << "[Redis-获取数据] key:" << key << " 不存在" << XEND;
    freeReplyObject(pReply);
    return false;
  }

  string data;
  data.assign(pReply->str, pReply->len);

  freeReplyObject(pReply);
  return pCmd->ParseFromString(data);
}

bool RedisManager::delData(const string& key)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "del " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-删除数据] key =" << key << "执行指令失败" << XEND;
    return false;
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-删除数据] key =" << key << "type =" << pReply->type << "err =" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  XLOG << "[Redis-删除数据] key :" << key << " 删除成功" << XEND;
  return true;
}

bool RedisManager::setProtoListData(const string& listname, const string& key, const Message* pCmd)
{
  if (pCmd == nullptr)
    return false;
  if (!checkConnect()) return false;

  string data;
  if (pCmd->SerializeToString(&data) == false)
    return false;

  stringstream sstrlist;
  stringstream sstrkey;

  sstrlist << listname;
  sstrkey << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, "hset %b %b %b",
        sstrlist.str().c_str(), sstrlist.str().size(), sstrkey.str().c_str(), sstrkey.str().size(), data.c_str(), data.size()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-列表数据设置] listname :" << listname << " key :" << key << "执行指令失败" << XEND;
    return false;
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据设置] listname :" << listname << " key :" << key << " type :" << pReply->type << "err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  setExpire(key, DAYS_30_T);

  XLOG << "[Redis-列表数据设置] listname :" << listname << " key :" << key << " 设置成功" << XEND;
  return true;
}

bool RedisManager::getProtoListData(const string& listname, const string& key, Message* pCmd)
{
  if (pCmd == nullptr)
    return false;
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hget " << listname << " " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "key :" << key << "执行指令失败" << XEND;
    return false;
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

  string data;
  data.assign(pReply->str, pReply->len);

  freeReplyObject(pReply);
  return pCmd->ParseFromString(data);
}

bool RedisManager::getProtoList(const string& listname, TSetString& setStrings)
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

  setStrings.clear();
  for (size_t i = 0; i < pReply->elements; i += 2)
  {
    if (pReply->element[i]->str == nullptr || pReply->element[i + 1]->str == nullptr)
      continue;

    string tstr;
    tstr.assign(pReply->element[i + 1]->str, pReply->element[i + 1]->len);
    setStrings.insert(tstr);
  }

  freeReplyObject(pReply);
  return true;
}

bool RedisManager::delListData(const string& listname, const string& key)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hdel " << listname << " " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "执行指令失败" << XEND;
    return false;
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << listname << "type :" << pReply->type << "err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);
  return true;
}

bool RedisManager::clearListData(const string& listname)
{
  return delData(listname);
}

bool RedisManager::checkConnect()
{
  if (m_pConnect == nullptr)
  {
    if (!reconnect()) return false;
    return (m_pConnect != nullptr);
  }
  return true;

  if (checkStatus())
  {
    return true;
  }
  else
  {
    if (!reconnect()) return false;
    return (m_pConnect != nullptr);
  }

  return false;
}

bool RedisManager::checkStatus()
{
  if (!m_pConnect) return false;

  redisReply *pReply = (redisReply *)redisCommand(m_pConnect, "ping");
  if (pReply == nullptr) return false;

  freeReplyObject(pReply);

  if (pReply->type != REDIS_REPLY_STATUS) return false;
  if (strcasecmp(pReply->str, "PONG") != 0) return false;
  return true;
}

bool RedisManager::disconnect()
{
  if (m_pConnect == nullptr)
    return false;

  redisFree(m_pConnect);
  m_pConnect = nullptr;
  XLOG << "[Redis-关闭连接]" << XEND;
  return true;
}

bool RedisManager::reconnect()
{
  const xLuaData& data = xServer::getBranchConfig().getData("Redis");
  bool b = init(data.getTableString("ip"), data.getTableString("password"), data.getTableInt("port"));
  XLOG << "[Redis-重连]" << (b ? "成功" : "失败") << XEND;
  return b;
}

bool RedisManager::setHash(const string& key, const xLuaData &data)
{
  if (!checkConnect()) return false;

  if (data.m_table.empty()) return false;

  vector<const char *> argv(data.m_table.size() * 2 + 2 );
  vector<size_t> argvlen(data.m_table.size() * 2 + 2 );

  DWORD j = 0;
  static char hmsetcmd[] = "HMSET";
  argv[j] = hmsetcmd;
  argvlen[j] = sizeof(hmsetcmd)-1;
  ++j;

  argvlen[j] = key.size();
  argv[j] = key.c_str();
  ++j;

  XLOG << "[Redis-列表数据设置],key:" << key << "设置:";
  for (auto &it : data.m_table)
  {
    argvlen[j] = it.first.size();
    argv[j] = it.first.c_str();
  //  memset((void *)argv[j], 0, argvlen[j]);
  //  memcpy((void*)argv[j], it.first.c_str(), it.first.size());
    ++j;

    if (it.second.getSize())
    {
      argvlen[j] = it.second.getSize() - 1;
    }
    else
    {
      argvlen[j] = 0;
    }
    argv[j] = it.second.getString();
    // memset((void*)argv[j], 0, argvlen[j]);
    // memcpy((void*)argv[j], it.second.getString(), it.second.getSize());
    ++j;

    XLOG << it.first << it.second.getString();
  }
  XLOG << XEND;

  redisReply* pReply = static_cast<redisReply*>(redisCommandArgv(m_pConnect, argv.size(), &(argv[0]), &(argvlen[0])));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Hash数据设置],key:" << key << "失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommandArgv(m_pConnect, argv.size(), &(argv[0]), &(argvlen[0])));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Hash数据设置],key:" << key << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据设置],key:" << key << "type:" << pReply->type << "err:" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  XLOG << "[Redis-列表数据设置],key:" << key << "设置成功" << XEND;
  return true;
}

bool RedisManager::delHash(const string& key, const xLuaData &data)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hdel " << key;

  for (auto &it : data.m_table)
  {
    sstr << " " << it.first;
  }

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << key << "type :" << pReply->type << "err :" << pReply->str << "cmd:" << sstr.str() << XEND;
    freeReplyObject(pReply);
    return false;
  }

  freeReplyObject(pReply);
  return true;
}

bool RedisManager::getHash(const string& key, xLuaData &data)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hmget " << key;

  for (auto &it : data.m_table)
  {
    sstr << " " << it.first;
  }

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << key << "type :" << pReply->type << "err :" << pReply->str << "cmd:" << sstr.str() << XEND;
    freeReplyObject(pReply);
    return false;
  }

  DWORD i = 0;
  for (auto &it : data.m_table)
  {
    if (i < pReply->elements)
    {
      if (pReply->element[i]->str != nullptr)
      {
        it.second.set(pReply->element[i]->str);
      }
    }
    else
    {
      break;
    }
    ++i;
  }

  freeReplyObject(pReply);
  return true;
}

bool RedisManager::getHashAll(const string& key, xLuaData &data)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "hgetall " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-列表数据获取] listname :" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-列表数据获取] listname :" << key << "type :" << pReply->type << "err :" << pReply->str << "cmd:" << sstr.str() << XEND;
    freeReplyObject(pReply);
    return false;
  }
  if (!pReply->elements)
  {
    XERR << "[Redis-列表数据获取] listname :" << key << "type :" << pReply->type << "cmd:" << sstr.str() << "没有数据" << XEND;
    freeReplyObject(pReply);
    return false;
  }

  for (DWORD i = 0; i+1 < pReply->elements; i+=2)
  {
    if (pReply->element[i]->str != nullptr && pReply->element[i+1]->str != nullptr)
    {
      data.setData(pReply->element[i]->str, pReply->element[i+1]->str);
      XDBG << "[RedisManager::getHashAll] key :" << pReply->element[i]->str << "value :" << pReply->element[i+1]->str << XEND;
    }
  }

  freeReplyObject(pReply);
  return true;
}

bool RedisManager::addSet(const string& key, const std::list<string> &data)
{
  if (!checkConnect()) return false;

  if (data.empty()) return false;

  stringstream sstr;
  sstr << "sadd " << key;
  for (auto it : data)
  {
    sstr << " " << it;
  }

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Set数据设置],key:" << key << "失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Set数据设置],key:" << key << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-Set数据设置],key:" << key << "type:" << pReply->type << "err:" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  setExpire(key, DAYS_30_T);

  XLOG << "[Redis-Set数据设置],key:" << key << "设置成功" << sstr.str() << XEND;
  return true;
}

bool RedisManager::delSet(const string& key, const std::list<string> &data)
{
  if (!checkConnect()) return false;

  if (data.empty()) return false;

  stringstream sstr;
  sstr << "srem " << key;
  for (auto it : data)
  {
    sstr << " " << it;
  }

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Set数据设置],key:" << key << "失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Set数据设置],key:" << key << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-Set数据设置],key:" << key << "type:" << pReply->type << "err:" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }
  freeReplyObject(pReply);

  XLOG << "[Redis-Set数据设置],key:" << key << "设置成功" << sstr.str() << XEND;
  return true;
}

bool RedisManager::getSetARand(const string& key, std::string &data)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "srandmember " << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Set数据获取] listname:" << key << " 执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Set数据获取] listname:" << key << " 执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-Set数据获取] listname :" << key << " type :" << pReply->type << " err :" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }

  if (pReply->str != nullptr)
  {
    data = pReply->str;
    freeReplyObject(pReply);
    return true;
  }

  freeReplyObject(pReply);
  return false;
}

bool RedisManager::setExpire(const string& key, DWORD sec)
{
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr << "expire " << key << " " << sec;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-Expire],key:" << key << "执行指令失败" << sstr.str() << "重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-Expire],key:" << key << "执行指令失败" << sstr.str() << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-Expire],key:" << key << "type:" << pReply->type << "err:" << pReply->str << "str:" << sstr.str() << XEND;
    freeReplyObject(pReply);
    return false;
  }

  freeReplyObject(pReply);
  return true;
}

//交易所服务器物品服务器价格。 key: ro-trade-10014:price
string RedisManager::getTradePriceKey(DWORD regionID, ERedisKeyType eType)
{
  std::stringstream stream;
  stream.str("");
  stream << "ro-trade-" << regionID << ":";
  switch (eType)
  {
    case EREDISKEYTYPE_TRADE_PRICE:
      {
        stream << "price";
      }
      break;
    default:
      {
        return "";
      }
      break;
  }
  return stream.str().c_str();
}

//incr 将键的整数值加1
bool RedisManager::incr(const string& key, QWORD& ret)
{
  ret = 0;
  if (!checkConnect()) return false;

  stringstream sstr;
  sstr.str("");
  sstr << "INCR ";
  sstr << key;

  redisReply* pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
  if (pReply == nullptr)
  {
    if (reconnect() == false)
    {
      XERR << "[Redis-加1] key =" << key << "执行指令失败,重连失败" << XEND;
      return false;
    }
    pReply = static_cast<redisReply*>(redisCommand(m_pConnect, sstr.str().c_str()));
    if (pReply == nullptr)
    {
      XERR << "[Redis-加1] key =" << key << "执行指令失败" << XEND;
      return false;
    }
  }
  if (pReply->type == REDIS_REPLY_ERROR)
  {
    XERR << "[Redis-加1] key =" << key << "type =" << pReply->type << "err =" << pReply->str << XEND;
    freeReplyObject(pReply);
    return false;
  }

  if (pReply->type != REDIS_REPLY_INTEGER)
  {
    freeReplyObject(pReply);
    return false;
  }
  
  ret = pReply->integer;
  
  freeReplyObject(pReply);
  return true;
}

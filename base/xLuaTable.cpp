#include "xLuaTable.h"
#include "xServer.h"
#include "json/json.h"
#include "config/MiscConfig.h"

void xLuaData::toData(ConfigParam* pConfig) const
{
  for (auto &m : m_table)
  {
    Param* pParam = pConfig->add_params();
    pParam->set_key(m.first);

    if (m.second.isTable() == true)
      m.second.toData(pParam->add_items());
    else
      pParam->set_value(m.second.getString());
  }
}

void xLuaData::toData(Param* pParam) const
{
  for (auto &m : m_table)
  {
    Param* pSubParam = pParam->add_items();
    pSubParam->set_key(m.first);
    if (m.second.isTable() == true)
      m.second.toData(pSubParam);
    else
      pSubParam->set_value(m.second.getString());
  }
}

bool xLuaData::parseFromString(std::string str, const char *table)
{
  if (!table) return false;
  return xLuaTable::getMe().load(str, table, *this);
}

void xLuaData::toJsonString(std::stringstream &stream) const
{
  toJsonData(stream, true);
}

void xLuaData::toJsonData(std::stringstream &stream, bool isFirst /*= false*/) const
{
  auto checkStr = [&](const string& str) -> bool
  {
    for (auto &s : str)
    {
      if (s >= 'a' && s <= 'z')
        return true;
      if (s >= 'A' && s <= 'Z')
        return true;
      if (s&0x80)
        return true;
    }
    return false;
  };

  if (isTable())
  {
    stream << "{";
    for (auto it = m_table.begin(); it != m_table.end(); ++it)
    {
      auto tmp = it;
      tmp++;

      stream << "\"" << it->first << "\":";
      if (it->second.isTable())
      {
        bool first = (tmp == m_table.end());
        it->second.toJsonData(stream, first);
      }
      else
      {
        bool isstr = checkStr(it->second.getString());

        if (isstr)
          stream << "\"";

        stream << it->second.getString();

        if (isstr)
          stream << "\"";

        if (tmp != m_table.end())
          stream << ",";
      }
    }
    stream << "}";
    if (!isFirst)
      stream << ",";
  }
}

void xLuaData::getJsonValue(Json::Value root)
{
  Json::Value::Members mems = root.getMemberNames();
  for (auto &o : mems)
  {
    const char* key = o.c_str();
    if (root[o].isObject())
    {
      xLuaData& data = getMutableData(key);
      data.getJsonValue(root[o]);
    }
    else
    {
      if (root[o].isString())
        setData(key, root[o].asString());
      else if (root[o].isInt())
        setData(key, root[o].asInt());
      else if (root[o].isUInt())
        setData(key, root[o].asUInt());
      else if (root[o].isDouble())
        setData(key, root[o].asDouble());
    }
  }
}

bool xLuaData::fromJsonString(std::string str)
{
  Json::Reader reader;
  Json::Value root;

  if (reader.parse(str, root) == false)
  {
    XERR << "json 解析失败" << str << XEND;
    return false;
  }

  getJsonValue(root);
  return true;
}

void xLuaData::getIDList(TSetDWORD& set)
{
  for (auto it = m_table.begin(); it != m_table.end(); ++it)
    set.insert(it->second.getInt());
}

void xLuaData::getIDList(TVecDWORD& vec)
{
  if (m_table.size() < 10)
  {
    for (auto it = m_table.begin(); it != m_table.end(); ++it)
      vec.push_back(it->second.getInt());
  }
  else
  {
    DWORD size = m_table.size();
    std::stringstream stream;
    for (DWORD i = 1; i <= size; ++ i)
    {
      stream.str("");
      stream << i;
      vec.push_back(getTableInt(stream.str()));
    }
  }
}

void xLuaData::setIDList(const char* key, const TSetDWORD& set)
{
  if (set.empty() || !key)
    return;
  xLuaData& ids = getMutableData(key);
  DWORD index = 1;
  stringstream stream;
  for (auto &s : set)
  {
    stream.str("");
    stream << index;
    ids.setData(stream.str(), s);

    index ++;
  }
}

xLuaTable::xLuaTable()
{
  m_state = luaL_newstate();
}

xLuaTable::~xLuaTable()
{
  lua_close(m_state);
}

bool xLuaTable::open(const char *file)
{
  luaopen_base(m_state);
  return 0==luaL_dofile(m_state, file);
}

bool xLuaTable::load(std::string str, const char *table, xLuaData &out)
{
  luaopen_base(m_state);
  if (0==luaL_dostring(m_state, str.c_str()))
  {
    lua_getglobal(m_state, table);
    return getValue(out);
  }
  return false;
}

bool xLuaTable::getLuaTable(const char *table, xLuaTableData &out)
{
  if (!table) return false;

  lua_getglobal(m_state, table);

  // 现在的栈：-1 => table
  if (lua_istable(m_state, -1))
  {
    lua_pushnil(m_state);
    // 现在的栈：-1 => nil, -2 => table
    while (lua_next(m_state, -2))
    {
      // 现在的栈：-1 => value; -2 => key; -3 => table

      lua_pushvalue(m_state, -2);
      // 现在的栈：-1 => key; -2 => value; -3 => key; -4 => table

      DWORD key = lua_tonumber(m_state, -1);
      lua_pop(m_state, 1);
      // 现在的栈：-1 => value; -2 => key; -3 => table

    //  XLOG("[LoadLua],get,key:%u", key);
      getValue(out[key]);
    }
    for (auto it = out.begin(); it != out.end();)
    {
      if (filterTable(table, it->first))
      {
        it = out.erase(it);
        continue;
      }
      ++it;
    }

    return true;
  }
  
  return false;
}

bool xLuaTable::getLuaData(const char *table, xLuaData &out)
{
  if (!table) return false;

  lua_getglobal(m_state, table);

  return getValue(out);
}

bool xLuaTable::getValue(xLuaData &data)
{
  if (lua_istable(m_state, -1))
  {
   // XLOG("[LoadLua],table,subTable");
    getSubTable(data);
  }
  else if (lua_isboolean(m_state, -1))
  {
    lua_pushvalue(m_state, -1);
    data.set<bool>(lua_toboolean(m_state, -1));
   // XLOG("[LoadLua],table,bool:%u", lua_toboolean(m_state, -1));
    lua_pop(m_state, 1);
  }
  else if (lua_isstring(m_state, -1))
  {
    lua_pushvalue(m_state, -1);
    data.set<const char *>(lua_tostring(m_state, -1));
    //XLOG("[LoadLua],table,string:%s", lua_tostring(m_state, -1));
    lua_pop(m_state, 1);
  }
  else if (lua_isnumber(m_state, -1))
  {
    lua_pushvalue(m_state, -1);
    data.set<int>((int)lua_tonumber(m_state, -1));
   // XLOG("[LoadLua],table,number:%d", lua_tonumber(m_state, -1));
    lua_pop(m_state, 1);
  }
  else
  {
    XLOG << "[LoadLua],table,unknow type" << XEND;
  }
  lua_pop(m_state, 1);
  return true;
}

bool xLuaTable::getSubTable(xLuaData &out)
{
  lua_pushnil(m_state);
  // 现在的栈：-1 => nil, -2 => table
  while (lua_next(m_state, -2))
  {
    // 现在的栈：-1 => value; -2 => key; -3 => table
    lua_pushvalue(m_state, -2);
    // 现在的栈：-1 => key; -2 => value; -3 => key; -4 => table

    const char* key = lua_tostring(m_state, -1);
    lua_pop(m_state, 1);
    // 现在的栈：-1 => value; -2 => key; -3 => table
    
    xLuaData &data = out.getMutableData(key);

    if (lua_istable(m_state, -1))
    {
     // XLOG("[LoadLua],table,key:%s", key);
      getSubTable(data);
    }
    else if (lua_isboolean(m_state, -1))
    {
      lua_pushvalue(m_state, -1);
      data.set<bool>(lua_toboolean(m_state, -1));
      //XLOG("[LoadLua],table,bool:%u", lua_toboolean(m_state, -1));
      lua_pop(m_state, 1);
    }
    else if (lua_isstring(m_state, -1))
    {
      lua_pushvalue(m_state, -1);
      data.set<const char *>(lua_tostring(m_state, -1));
      //XLOG("[LoadLua],table,key:%s,string:%s", key, lua_tostring(m_state, -1));
      lua_pop(m_state, 1);
    }
    else if (lua_isnumber(m_state, -1))
    {
      lua_pushvalue(m_state, -1);
      data.set<double>((double)lua_tonumber(m_state, -1));
      //XLOG("[LoadLua],table,number:%d", lua_tonumber(m_state, -1));
      lua_pop(m_state, 1);
    }
    else
    {
      XLOG << "[LoadLua],table,unknow type" << XEND;
    }
    lua_pop(m_state, 1);
  }
  return true;
}

bool xLuaTable::getMultiTable(const char *table, LuaMultiTable &out)
{
  if (!table) return false;

  lua_getglobal(m_state, table);

  lua_pushnil(m_state);
  // 现在的栈：-1 => nil; index => table
  int index = -2;
  while (lua_next(m_state, index))
  {
    // 现在的栈：-1 => table; -2 => key; index => table
    lua_pushvalue(m_state, -2);
    // 现在的栈：-1 => key; -2 => table; -3 => key; index => table

    const char* key = lua_tostring(m_state, -1);
    lua_pop(m_state, 1);

    if (lua_istable(m_state, -1))
    {
      lua_pushnil(m_state); 
      // 现在的栈：-1 => nil; -2 => table; -3 => key; index => table
      int idx = -2;
      while (lua_next(m_state, idx))
      {
        // 现在的栈：-1 => value; -2 => key; idx => table; -4 => key; index => table
        // 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
        lua_pushvalue(m_state, -2);
        // 现在的栈：-1 => key; -2 => value; -3 => key; idx => table; -4 => key; index => table

        if (lua_isboolean(m_state, -2))
        {
          out[key][lua_tostring(m_state, -1)] = lua_toboolean(m_state, -2);
        }
        else if (lua_istable(m_state, -2))
        {
        }
        else
        {
          out[key][lua_tostring(m_state, -1)] = lua_tostring(m_state, -2);
        }
        // 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
        lua_pop(m_state, 2);
        // 现在的栈：-1 => key; idx => table; -3 => key; index => table
      }
    }
    else
    {
        lua_pushvalue(m_state, -1);
        out[key][lua_tostring(m_state, -1)] = "";
        lua_pop(m_state, 1);
    }
    lua_pop(m_state, 1);
  }
  
  return true;
}

bool xLuaTable::getTable(const char *table, DWORD key, std::map<std::string, std::string> &out)
{
  if (!table) return false;

  lua_getglobal(m_state, table);

  lua_pushnumber(m_state, key);
  lua_gettable(m_state, -2);

  lua_pushnil(m_state); 
  int index = -2;
  while (lua_next(m_state, index))
  {
    lua_pushvalue(m_state, -2);

    out[lua_tostring(m_state, -1)] = lua_tostring(m_state, -2);

    lua_pop(m_state, 2);
  }

  return true;
}

bool xLuaTable::getTableValue(const char *table, DWORD key, const char *field, DWORD &out)
{
  if (!table || !field) return false;

  lua_getglobal(m_state, table);
  lua_pushnumber(m_state, key);
  lua_gettable(m_state, -2);
  lua_pushstring(m_state, field);
  lua_gettable(m_state, -2);

  out = lua_tonumber(m_state, -1);

  return true;
}

bool xLuaTable::getValue(const char* name, DWORD &out)
{
  if (!name) return false;

  lua_getglobal(m_state, name);
  out = lua_tonumber(m_state, -1);

  return true;
}

void xLuaTable::test()
{
  if (!xLuaTable::getMe().open("Lua/Config/Table_RoleData.txt"))
  {
    return;
  }
  LuaMultiTable table;
  xLuaTable::getMe().getMultiTable("Table_RoleData", table);
  for (auto it=table.begin(); it!=table.end(); ++it)
  {
    for (auto iter=it->second.begin(); iter!=it->second.end(); ++iter)
      XLOG << "[RoleData]" << it->first << iter->first << iter->second << XEND;
  }
}

bool xLuaTable::filterTable(const char* table, DWORD id)
{
  if (MiscConfig::getMe().isForbid(table, id))
  {
    XLOG << "[加载配置-屏蔽了配置] table" << table << "id" << id << XEND;
    return true;
  }
  return false;
}

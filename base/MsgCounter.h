#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "xDefine.h"
#include <sstream>

struct MsgItem
{
  BYTE cmd;
  BYTE param;
  QWORD count;
  QWORD len;

};


class MsgCounter
{
  public:
    void add(void *cmd, unsigned short len);
    void clear();
    void printByCountDesc();
    void printByLenDesc();
  private:
    std::unordered_map<DWORD, MsgItem> m_mapMsgs;
};

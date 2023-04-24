/**
 * @file BufferManager.h
 * @brief 
 * @author gengshengjie, gengshengjie@xindong.com
 * @version v1
 * @date 2015-08-29
 */
#pragma once

#include "xDefine.h"
#include <map>
#include <set>
#include <memory>
#include "BuffCondition.h"
//#include "BufferState.h"

using std::string;
class BufferState;
typedef std::shared_ptr<BaseCondition> TPtrBuffCond;
typedef std::shared_ptr<BufferState> TPtrBufferState;

typedef std::map<DWORD, TPtrBufferState> TMapBuff;

class BufferManager : public xSingleton<BufferManager>
{
  public:
    bool loadConfig();
    TPtrBufferState getBuffById(DWORD id);
    TPtrBuffCond createBuffCondition(xLuaData data);
    TPtrBufferState createBuffer(DWORD id, xLuaData data, TPtrBuffCond buffCond);
    DWORD getBuffStatus(DWORD buffid);

  private:
    TMapBuff m_mapID2Buff;

};


/**
 * @file Var.h
 * @brief variable
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-06-24
 */

#pragma once

#include "xSingleton.h"
#include "RecordCmd.pb.h"
#include "xDefine.h"
#include "config/MiscConfig.h"
#include "SessionCmd.pb.h"

using namespace Cmd;
using std::string;
using std::vector;
using std::pair;
using std::set;

struct SVarInfo
{
private:
  DWORD dwTimeOffset = 0;

public:
  EVarType eType = EVARTYPE_MIN;
  EVarTimeType eTimeType = EVARTIMETYPE_MIN;

  bool bClient = false;

  SVarInfo() {}

  DWORD getTimeOffset() const { return dwTimeOffset + MiscConfig::getMe().getVarMiscCFG().getOffset(eType); }
  void setTimeOffset(DWORD offset) { dwTimeOffset = offset; }
};

struct SAccVarInfo
{
private:
  DWORD dwTimeOffset = 0;

public:
  EAccVarType eType = EACCVARTYPE_MIN;
  EVarTimeType eTimeType = EVARTIMETYPE_MIN;

  bool bClient = false;

  SAccVarInfo() {}

  DWORD getTimeOffset() const { return dwTimeOffset + MiscConfig::getMe().getVarMiscCFG().getOffset(eType); }
  void setTimeOffset(DWORD offset) { dwTimeOffset = offset; }
};

// var manager
class VarManager : public xSingleton<VarManager>
{
  public:
    VarManager();
    virtual ~VarManager();

    const SVarInfo* getVarInfo(EVarType eType);
    const SAccVarInfo* getAccVarInfo(EAccVarType eType);
    const std::set<EVarType>& getSessionVars() { return m_setSessionVars; }
  private:
    void registerVar(EVarType eType, EVarTimeType eTimeType, DWORD offset, bool bClient, bool bSession = false);
    void registerAccVar(EAccVarType eType, EVarTimeType eTimeType, DWORD offset, bool bClient);
  private:
    SVarInfo m_stVar[EVARTYPE_MAX];
    SAccVarInfo m_stAccVar[EACCVARTYPE_MAX];

    // scene->session 同步的var
    std::set<EVarType> m_setSessionVars;
};

// variable
class Variable
{
  public:
    Variable();
    ~Variable();

    bool load(const BlobVar& oBlob);
    bool save(BlobVar* pBlob);

    bool loadAcc(const BlobAccVar& oBlob);
    bool saveAcc(BlobAccVar* pBlob);

    DWORD getVarValue(EVarType eType);
    DWORD getAccVarValue(EAccVarType eType);
    
    void setVarValue(EVarType eType, DWORD value);
    void setAccVarValue(EAccVarType eType, DWORD value);

    bool hasNew() const { return m_setVars.empty() == false; }
    void clearNew() { m_setVars.clear(); }
    void collectVar(VarUpdate& cmd, bool bFull = false);
    void refreshAllVar();
    DWORD getVarDayNum(EVarType eType, DWORD dwBeginTime, DWORD dwEndTime);
    void clearExcept(set<EVarType> types);

    void collectSessionVars(SyncUserVarSessionCmd& cmd);
  private:
    Var m_oVar[EVARTYPE_MAX];
    set<EVarType> m_setVars;
    AccVar m_oAccVar[EACCVARTYPE_MAX];
};


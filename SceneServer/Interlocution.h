/**
 * @file Interlocution.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-27
 */

#pragma once

#include "xDefine.h"
#include "SceneInterlocution.pb.h"
#include "RecordCmd.pb.h"

using namespace Cmd;
using std::vector;

// data
struct SInterData
{
  DWORD dwGUID = 0;
  DWORD dwInterID = 0;

  ESource eSource = ESOURCE_MIN;

  SInterData() {}

  bool fromData(const InterData& rData);
  bool toData(InterData* pData);
};
typedef vector<SInterData> TVecInterData;

namespace Cmd
{
  class BlobInter;
};

// interlocution
class SceneUser;
class Interlocution
{
  public:
    Interlocution(SceneUser* pUser);
    ~Interlocution();

    bool load(const BlobInter& rData);
    bool save(BlobInter* rData);

    void addInterlocution(DWORD dwID, ESource eSource);
    void sendInterlocution();

    bool answer(DWORD dwGUID, DWORD dwAnswer);
  private:
    SceneUser* m_pUser = nullptr;

    TVecInterData m_vecDatas;
};


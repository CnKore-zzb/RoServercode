#pragma once
#include "xNoncopyable.h"
#include "base/protobuf/RecordCmd.pb.h"
#include "xDefine.h"

using namespace Cmd;

class SceneUserData : private xNoncopyable
{
  public:
    SceneUserData();
    virtual ~SceneUserData();

  public:
    void setRecordUserData(const RecordUserData &data)
    {
      m_oRecordUserData = data;
    }
    RecordUserData& getRecordUserData()
    {
      return m_oRecordUserData;
    }
  protected:
    // 消息
    RecordUserData m_oRecordUserData;
  public:
    // 帐号数据
    BlobAccData m_oBlobAccData;
    // 角色数据
    BlobData m_oBlobData;
};

class SceneUserDataLoad : public SceneUserData
{
  public:
    SceneUserDataLoad();
    virtual ~SceneUserDataLoad();

  public:
    bool fromCmdToData();

  public:
    // 是否解析成功
    bool m_blRet = false;
};

class SceneUserDataSave : public SceneUserData
{
  public:
    SceneUserDataSave();
    virtual ~SceneUserDataSave();

  public:
    void fromDataToCmd();

  public:
    UserDataRecordCmd m_oUserDataRecordCmd;
};

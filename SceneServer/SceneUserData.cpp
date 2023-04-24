#include "SceneUserData.h"
#include "base/xlib/xTools.h"
#include "SceneServer.h"

SceneUserData::SceneUserData()
{
}

SceneUserData::~SceneUserData()
{
}

SceneUserDataLoad::SceneUserDataLoad()
{
}

SceneUserDataLoad::~SceneUserDataLoad()
{
}

bool SceneUserDataLoad::fromCmdToData()
{
  RecordUserData &oCmd = m_oRecordUserData;

  std::string acc_data;
  if (uncompress(oCmd.acc_data(), acc_data) == false)
  {
    XERR_T("[玩家-初始化],%llu,%llu,%s, 解压acc失败,ori:%u, cur:%u", oCmd.base().accid(), oCmd.base().charid(), oCmd.base().name().c_str(), oCmd.acc_data().size(), acc_data.size());
    return false;
  }
  std::string char_data;
  if (uncompress(oCmd.char_data(), char_data) == false)
  {
    XERR_T("[玩家-初始化],%llu,%llu,%s, 解压char失败,ori:%u, cur:%u", oCmd.base().accid(), oCmd.base().charid(), oCmd.base().name().c_str(), oCmd.char_data().size(), char_data.size());
    return false;
  }

  if (m_oBlobData.ParseFromString(char_data) == false)
  {
    XERR_T("[玩家-初始化],%llu,%llu,%s, 序列化BlobData失败", oCmd.base().accid(), oCmd.base().charid(), oCmd.base().name().c_str());
    return false;
  }
  if (m_oBlobAccData.ParseFromString(acc_data) == false)
  {
    XERR_T("[玩家-初始化],%llu,%llu,%s, 序列化BlobAccData失败", oCmd.base().accid(), oCmd.base().charid(), oCmd.base().name().c_str());
    return false;
  }

  m_blRet = true;

  return true;
}

SceneUserDataSave::SceneUserDataSave()
{
}

SceneUserDataSave::~SceneUserDataSave()
{
}

void SceneUserDataSave::fromDataToCmd()
{
  UserDataRecordCmd &cmd = m_oUserDataRecordCmd;
  BlobData &oData = m_oBlobData;
  BlobAccData &oAccData = m_oBlobAccData;
  RecordUserData &data = m_oRecordUserData;

  std::string acc_data;
  if (oAccData.SerializeToString(&acc_data) == false)
  {
    XERR_T("[玩家-保存],%llu,%llu, 序列化BlobAccData失败", cmd.accid(), cmd.charid());
    return;
  }
  std::string char_data;
  if (oData.SerializeToString(&char_data) == false)
  {
    XERR_T("[玩家-保存],%llu,%llu, 序列化BlobData失败", cmd.accid(), cmd.charid());
    return;
  }

  DWORD dwSaveIndex = oData.user().save_index();

  DWORD dwOldAccDataSize = acc_data.size();
  if (compress(acc_data, acc_data) == false)
  {
    XERR_T("[玩家-保存],%llu,%llu, 压缩BlobAccData失败", cmd.accid(), cmd.charid());
    return;
  }

  DWORD dwOldCharDataSize = char_data.size();
  if (compress(char_data, char_data) == false)
  {
    XERR_T("[玩家-保存],%llu,%llu, 压缩BlobData失败", cmd.accid(), cmd.charid());
    return;
  }

  data.set_acc_data(acc_data.c_str(), acc_data.size());
  data.set_char_data(char_data.c_str(), char_data.size());

  std::string user_data;
  if (data.SerializeToString(&user_data) == false)
  {
    XERR_T("[玩家-保存],%llu,%llu, 序列化RecordUserData失败", cmd.accid(), cmd.charid());
    return;
  }
  XLOG_T("[玩家-保存],%llu,%llu, 压缩BlobAccData:%u,%u, 压缩BlobData:%u,%u, RecordUserData:%u, saveindex:%u", cmd.accid(), cmd.charid(), dwOldAccDataSize, acc_data.size(), dwOldCharDataSize, char_data.size(), user_data.size(), dwSaveIndex);

  // send data
  DWORD dwMaxBuf = TRANS_BUFSIZE;
  cmd.set_first(true);
  if (user_data.size() < dwMaxBuf)
  {
    cmd.set_data(user_data.c_str(), user_data.size());
    cmd.set_over(true);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToRecord(send, len);
  }
  else
  {
    DWORD dwIndex = 0;
    while (true)
    {
      if (user_data.size() - dwIndex * dwMaxBuf > dwMaxBuf)
      {
        cmd.set_data(user_data.c_str() + dwIndex * dwMaxBuf, dwMaxBuf);
        cmd.set_over(false);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToRecord(send, len);
        cmd.set_first(false);
      }
      else
      {
        cmd.set_data(user_data.c_str() + dwIndex * dwMaxBuf, user_data.size() - dwIndex * dwMaxBuf);
        cmd.set_over(true);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToRecord(send, len);
        cmd.set_first(false);
        break;
      }
      ++dwIndex;
      cmd.clear_data();
      XWRN_T("[玩家-保存],%llu,%llu, 数据传输index:%u", cmd.accid(), cmd.charid(), dwIndex);
    }
  }
}

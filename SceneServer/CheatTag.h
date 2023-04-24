#pragma once

#include "xDefine.h"
#include "SceneUser2.pb.h"
#include "RecordCmd.pb.h"

using namespace Cmd;
using std::vector;

typedef vector<CheatTagUserCmd> TVecCheatTagCmd;
class SceneUser;

class CheatTag
{
public:
  CheatTag(SceneUser* pUser);
  ~CheatTag();

  void timer();
  void save();
  void load();
  void assignData(CheatTagRecordCmd& cmd);
  void sendCheatTagStat(); // 发送次数统计信息

  void flush(); // 处理cmd队列

  void recCheatTag(CheatTagUserCmd& cmd);
private:
  SceneUser* m_pUser;

  DWORD m_dwMinInterval = 0; // 玩家最小操作间隔
  DWORD m_dwFrame = 0; // 最小操作间隔时对应的帧数
  DWORD m_dwCount = 0; // 嫌疑外挂操作次数

  bool m_bChangeMark = false; // 数据是否修改标记

  TVecCheatTagCmd m_vecCmdQueue; // loading中时的cmd缓存
  bool m_bLoading = false; // 判断是否正在向dataserver请求数据
};

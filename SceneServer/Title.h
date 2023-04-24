#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "TableStruct.h"
#include "UserEvent.pb.h"
#include "RecordCmd.pb.h"
using namespace std;

class SceneUser;
class Title
{
public:
  Title(SceneUser* pUser);
  ~Title();

  bool load(const BlobTitle& acc_data, const BlobTitle& char_data);
  bool save(BlobTitle* acc_data, BlobTitle* char_data);

  void sendTitleData();
  bool checkAddTitle(DWORD titleId);
  bool addTitle(DWORD titleId);
  bool hasTitle(DWORD titleId);
  DWORD getCurTitle(ETitleType type);

  bool changeCurTitle(ChangeTitle& data);
  //void collectAttr(TVecAttrSvrs& attrs);
  void collectAttr();
  void sendCurrentTitle(ETitleType type);

  void reloadConfigCheck();
  DWORD getTitleNum(ETitleType type);

  bool delTitle(ETitleType type, DWORD titleId);
private:
  bool canAdd(ETitleType type, DWORD titleId);

private:
  SceneUser* m_pUser = nullptr;
  std::map<ETitleType, std::vector<DWORD>> m_mapTitle;
  DWORD curAhieveTitle = 0;
};

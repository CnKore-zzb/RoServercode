#pragma once

#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"
#include "PhotoCmd.pb.h"
#include "RecordCmd.pb.h"

using namespace Cmd;
using std::map;
using std::list;

class SceneUser;

class UserPhoto : private xNoncopyable
{
public:
  UserPhoto(SceneUser* pUser);
  virtual ~UserPhoto();

  bool load(const BlobPhoto &data);
  bool save(BlobPhoto* data);

  bool sendPhotoList(DWORD size = 0);
  bool handlePhotoOpt(PhotoOptCmd& cmd);

  void onAddSkill(DWORD skillid);
  void onDelSkill(DWORD skillid);

  PhotoItem* get(DWORD index);

  void updateUploadPhoto(const QueryUserPhotoListGuildSCmd& cmd);
  void queryUserPhotoList();
  void sendUserPhotoList();
private:
  DWORD getMaxSize();
  bool isIndexValid(DWORD index) { return index >= 1 && index <= getMaxSize(); }
  bool add(DWORD index, DWORD anglez, DWORD mapid);
  bool upload(DWORD index);
  bool remove(DWORD index);
  bool replace(DWORD index, DWORD anglez, DWORD mapid);

  SceneUser* m_pUser = nullptr;
  map<DWORD, PhotoItem> m_mapPhoto;

  DWORD m_dwUploadTick = 0;
  map<DWORD, list<GuildPhoto>> m_mapUploadCache;
};

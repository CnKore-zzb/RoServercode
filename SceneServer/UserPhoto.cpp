#include "UserPhoto.h"
#include "SceneUser.h"
#include "MsgManager.h"

UserPhoto::UserPhoto(SceneUser* pUser) : m_pUser(pUser)
{
}

UserPhoto::~UserPhoto()
{
}

bool UserPhoto::load(const BlobPhoto &data)
{
  m_mapPhoto.clear();

  for (int i = 0; i < data.items_size(); ++i) {
    PhotoItem& ritem = m_mapPhoto[data.items(i).index()];
    ritem.CopyFrom(data.items(i));
    if (ritem.charid() == 0)
      ritem.set_charid(m_pUser->id);
  }

  return true;
}

bool UserPhoto::save(BlobPhoto* data)
{
  if (data == nullptr)
    return false;

  for (auto& v : m_mapPhoto) {
    PhotoItem* pPhoto = data->add_items();
    if (pPhoto == nullptr) {
      XERR << "[个人相册-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "photo index:" << v.first << "protobuf error" << XEND;
      continue;
    }
    pPhoto->CopyFrom(v.second);
  }

  XDBG << "[个人相册-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "数据大小:" << data->ByteSize() << XEND;
  return true;
}

bool UserPhoto::sendPhotoList(DWORD size/* = 0*/)
{
  PhotoQueryListCmd cmd;
  for (auto& v : m_mapPhoto) {
    PhotoItem *pPhoto = cmd.add_photos();
    if (pPhoto == nullptr)
      continue;
    pPhoto->CopyFrom(v.second);
  }
  if (size == 0)
    cmd.set_size(getMaxSize());
  else
    cmd.set_size(size);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
  return true;
}

bool UserPhoto::handlePhotoOpt(PhotoOptCmd& cmd)
{
  bool ret = false;
  switch (cmd.opttype())
  {
  case EPHOTOOPTTYPE_ADD:
    ret = add(cmd.index(), cmd.anglez(), cmd.mapid());
    break;
  case EPHOTOOPTTYPE_UPLOAD:
    ret = upload(cmd.index());
    break;
  case EPHOTOOPTTYPE_REMOVE:
    ret = remove(cmd.index());
    break;
  case EPHOTOOPTTYPE_REPLACE:
    ret = replace(cmd.index(), cmd.anglez(), cmd.mapid());
    break;
  default:
    break;
  }

  if (ret) {
    PhotoUpdateNtf res;
    res.set_opttype(cmd.opttype());
    PhotoItem* resphoto = res.mutable_photo();
    if (resphoto != nullptr) {
      PhotoItem* pPhoto = get(cmd.index());
      if (pPhoto != nullptr)
        resphoto->CopyFrom(*pPhoto);
      else
        resphoto->set_index(cmd.index());
    }
    PROTOBUF(res, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  return ret;
}

void UserPhoto::onAddSkill(DWORD skillid)
{
  if (MiscConfig::getMe().getPhotoCFG().isPhotoSkill(skillid)) {
    XLOG << "[个人相册-摄影大师激活]" << m_pUser->accid << m_pUser->id << m_pUser->name << XEND;
    DWORD addsize = MiscConfig::getMe().getPhotoCFG().getSizeBySkillID(skillid);
    sendPhotoList(getMaxSize() + addsize);
    if (addsize != 0)
      MsgManager::sendMsg(m_pUser->id, 996, MsgParams(addsize));
  }
}

void UserPhoto::onDelSkill(DWORD skillid)
{
  if (MiscConfig::getMe().getPhotoCFG().isPhotoSkill(skillid)) {
    XLOG << "[个人相册-摄影大师移除]" << m_pUser->accid << m_pUser->id << m_pUser->name << XEND;
    sendPhotoList();
  }
}

PhotoItem* UserPhoto::get(DWORD index)
{
  auto it = m_mapPhoto.find(index);
  if (it == m_mapPhoto.end())
    return nullptr;
  return &it->second;
}

void UserPhoto::updateUploadPhoto(const QueryUserPhotoListGuildSCmd& cmd)
{
  m_mapUploadCache.clear();
  for (int i = 0; i < cmd.frames_size(); ++i)
  {
    const PhotoFrame& rFrame = cmd.frames(i);
    list<GuildPhoto>& rList = m_mapUploadCache[rFrame.frameid()];
    for (int j = 0; j < rFrame.photo_size(); ++j)
      rList.push_back(rFrame.photo(j));
  }
}

void UserPhoto::queryUserPhotoList()
{
  if (m_pUser->hasGuild() == false)
    return;

  DWORD dwNow = xTime::getCurSec();
  if (m_dwUploadTick > dwNow)
  {
    sendUserPhotoList();
    return;
  }
  m_dwUploadTick = dwNow + 600;

  QueryUserPhotoListGuildSCmd cmd;
  cmd.set_result(false);
  cmd.set_guildid(m_pUser->getGuild().id());
  m_pUser->toData(cmd.mutable_user());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[个人相册-上传照片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "向公会请求上传照片" << XEND;
}

void UserPhoto::sendUserPhotoList()
{
  QueryUserPhotoListPhotoCmd cmd;
  for (auto &m : m_mapUploadCache)
  {
    PhotoFrame* pFrame = cmd.add_frames();
    pFrame->set_frameid(m.first);

    for (auto &l : m.second)
      pFrame->add_photo()->CopyFrom(l);
  }
  cmd.set_maxframe(MiscConfig::getMe().getGuildCFG().dwMaxFramePhotoCount);
  cmd.set_maxphoto(MiscConfig::getMe().getGuildCFG().dwMaxPhotoPerMember);

  if (cmd.frames_size() > 0)
  {
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
  XLOG << "[个人相册-上传照片]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "请求上传照片" << cmd.ShortDebugString() << XEND;
}

DWORD UserPhoto::getMaxSize()
{
  DWORD size = MiscConfig::getMe().getPhotoCFG().dwDefaultSize;
  if (m_pUser->getCurFighter() != nullptr) {
    for (auto& v : MiscConfig::getMe().getPhotoCFG().vecSkillIncreaseSize) {
      if (m_pUser->getCurFighter()->getSkill().isSkillEnable(v.first))
        size += v.second;
    }
  }
  return size;
}

bool UserPhoto::add(DWORD index, DWORD anglez, DWORD mapid)
{
  if (!isIndexValid(index))
    return false;
  if (m_mapPhoto.size() >= getMaxSize())
    return false;
  auto it = m_mapPhoto.find(index);
  if (it != m_mapPhoto.end()) {
    XERR << "[个人相册-添加照片]" << m_pUser->accid << m_pUser->id << m_pUser->name << "index:" << index << "anglez:" << anglez << "失败,index已存在" << XEND;
    return false;
  }

  PhotoItem& rphoto = m_mapPhoto[index];
  rphoto.set_index(index);
  rphoto.set_mapid(mapid);
  rphoto.set_anglez(anglez);
  rphoto.set_time(now());
  rphoto.set_isupload(false);
  rphoto.set_charid(m_pUser->id);

  XLOG << "[个人相册-添加照片]" << m_pUser->accid << m_pUser->id << m_pUser->name << "index:" << rphoto.index() << "anglez:" << rphoto.anglez() << XEND;
  return true;
}

bool UserPhoto::upload(DWORD index)
{
  auto it = m_mapPhoto.find(index);
  if (it == m_mapPhoto.end())
    return false;
  it->second.set_isupload(true);

  if (m_pUser->hasGuild() == true)
  {
    PhotoUpdateGuildSCmd cmd;

    cmd.set_guildid(m_pUser->getGuild().id());
    cmd.set_to_guild(true);

    GuildPhoto* pPhoto = cmd.mutable_update();
    pPhoto->set_charid(m_pUser->id);
    pPhoto->set_anglez(it->second.anglez());
    pPhoto->set_time(it->second.time());
    pPhoto->set_mapid(it->second.mapid());
    pPhoto->set_source(ESOURCE_PHOTO_SELF);
    pPhoto->set_sourceid(index);

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  XLOG << "[个人相册-上传照片]" << m_pUser->accid << m_pUser->id << m_pUser->name << "index:" << it->second.index() << XEND;
  return true;
}

bool UserPhoto::remove(DWORD index)
{
  auto it = m_mapPhoto.find(index);
  if (it == m_mapPhoto.end())
    return false;

  if (it->second.isupload() && m_pUser->hasGuild() == true)
  {
    PhotoUpdateGuildSCmd cmd;
    cmd.set_guildid(m_pUser->getGuild().id());
    cmd.set_to_guild(true);

    GuildPhoto* pPhoto = cmd.mutable_del();
    pPhoto->set_charid(m_pUser->id);
    pPhoto->set_anglez(it->second.anglez());
    pPhoto->set_time(it->second.time());
    pPhoto->set_mapid(it->second.mapid());
    pPhoto->set_source(ESOURCE_PHOTO_SELF);
    pPhoto->set_sourceid(index);

    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToSession(send, len);
  }

  m_mapPhoto.erase(it);
  XLOG << "[个人相册-删除照片]" << m_pUser->accid << m_pUser->id << m_pUser->name << "index:" << index << XEND;
  return true;
}

bool UserPhoto::replace(DWORD index, DWORD anglez, DWORD mapid)
{
  auto it = m_mapPhoto.find(index);
  if (it == m_mapPhoto.end())
    return false;
  // 技能增加格子, 在技能重置后消失, 玩家只有在照片数小于等于格子数之后才能替换
  if (m_mapPhoto.size() > getMaxSize())
    return false;
  it->second.set_mapid(mapid);
  it->second.set_anglez(anglez);
  it->second.set_time(now());
  it->second.set_isupload(false);
  XLOG << "[个人相册-替换照片]" << m_pUser->accid << m_pUser->id << m_pUser->name << "index:" << it->second.index() << "anglez:" << it->second.anglez() << XEND;
  return true;
}

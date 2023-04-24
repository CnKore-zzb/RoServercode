#include "GuildPhoto.h"
#include "Guild.h"
#include "GGuild.h"
#include "GuildServer.h"
#include "MiscConfig.h"

GuildPhotoMgr::GuildPhotoMgr(Guild *pGuild) : m_pGuild(pGuild)
{
}

GuildPhotoMgr::~GuildPhotoMgr()
{
}

void GuildPhotoMgr::init()
{
  switch (m_dwInitStatus)
  {
    case GUILD_BLOB_INIT_NULL:
      {
        XDBG << "[公会墙-初始化]" << m_pGuild->getName() << "初始化失败，没有数据" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_COPY_DATA:
      {
        fromPhotoString(m_oBlobPhoto);
        m_dwInitStatus = GUILD_BLOB_INIT_OK;
        XDBG << "[公会墙-初始化]" << m_pGuild->getName() << "初始化成功" << XEND;
      }
      break;
    case GUILD_BLOB_INIT_OK:
      break;
    default:
      break;
  }
}

void GuildPhotoMgr::setBlobPhotoString(const char* str, DWORD len)
{
  if (GUILD_BLOB_INIT_OK == m_dwInitStatus)
  {
    XERR << "[公会墙-数据]" << "初始化异常" << XEND;
    return;
  }
  m_dwInitStatus = GUILD_BLOB_INIT_COPY_DATA;
  m_oBlobPhoto.assign(str, len);
  XDBG << "[公会墙-数据]" << m_pGuild->getName() << "设置blob string" << XEND;
}

bool GuildPhotoMgr::toBlobPhotoString(string& str)
{
  if (GUILD_BLOB_INIT_COPY_DATA == m_dwInitStatus)
  {
    str.assign(m_oBlobPhoto.c_str(), m_oBlobPhoto.size());
    return true;
  }

  if (GUILD_BLOB_INIT_OK != m_dwInitStatus) return true;

  refreshPhoto();

  BlobGuildPhoto oBlob;
  for (auto &m : m_mapFramePhoto)
  {
    if (m.first == 0)
    {
      XERR << "[公会墙-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "序列化 frameid : 0 被忽略" << XEND;
      continue;
    }

    PhotoFrame* pFrame = oBlob.add_frames();
    pFrame->set_frameid(m.first);
    for (auto &photo : m.second)
      pFrame->add_photo()->CopyFrom(photo.second);
  }

  bool bSuccess = oBlob.SerializeToString(&str);
  if (!bSuccess)
    XERR << "[公会墙-保存]" << m_pGuild->getGUID() << m_pGuild->getName() << "序列化失败" << XEND;
  return bSuccess;
}

void GuildPhotoMgr::queryPhotoList()
{
  refreshPhoto();

  QueryPhotoListGuildSCmd cmd;
  cmd.set_guildid(m_pGuild->getGUID());
  cmd.set_result(true);

  for (auto &m : m_mapFramePhoto)
  {
    PhotoFrame* pFrame = cmd.add_frames();
    if (pFrame == nullptr)
      continue;
    pFrame->set_frameid(m.first);
    for (auto &photo : m.second)
      pFrame->add_photo()->CopyFrom(photo.second);
  }

  PROTOBUF(cmd, send, len);
  if (thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len) == true)
    XLOG << "[公会墙-同步]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求照片成功" << cmd.ShortDebugString() << "size :" << cmd.ByteSize() << XEND;
  else
    XERR << "[公会墙-同步]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求照片失败" << cmd.ShortDebugString() << "size :" << cmd.ByteSize() << XEND;

  queryAutoPhoto();
}

void GuildPhotoMgr::queryUserPhotoList(const SocialUser& rUser)
{
  refreshPhoto();

  TMapFramePhoto mapFramePhoto;
  for (auto &m : m_mapFramePhoto)
  {
    TMapIDPhoto& mapPhoto = mapFramePhoto[m.first];
    for (auto &p : m.second)
  {
      if (rUser.accid() == p.second.accid_svr())
        mapPhoto[GGuild::getPhotoGUID(p.second)].CopyFrom(p.second);
    }
  }

  QueryUserPhotoListGuildSCmd scmd;
  scmd.set_result(true);
  scmd.set_guildid(m_pGuild->getGUID());
  scmd.mutable_user()->CopyFrom(rUser);
  for (auto &m : mapFramePhoto)
  {
    if (m.second.empty() == true)
      continue;

    PhotoFrame* pFrame = scmd.add_frames();
    pFrame->set_frameid(m.first);

    for (auto &p : m.second)
      pFrame->add_photo()->CopyFrom(p.second);
  }

  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToZone(rUser.zoneid(), ssend, slen);
  XLOG << "[公会墙-个人照片]" << m_pGuild->getGUID() << m_pGuild->getName() << "成员" << rUser.ShortDebugString() << "请求个人照片成功" << scmd.ShortDebugString() << XEND;
}

void GuildPhotoMgr::updateFrame(const FrameUpdateGuildSCmd& cmd)
{
  if (cmd.update().accid_svr() != 0)
    addFramePhoto(cmd.frameid(), cmd.update());

  if (cmd.del().accid_svr() != 0)
    removeFramePhoto(cmd.frameid(), cmd.del());

  m_pGuild->setMark(EGUILDDATA_PHOTO);
  XLOG << "[公会墙-相框更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框更新" << cmd.ShortDebugString() << XEND;
}

void GuildPhotoMgr::updatePhoto(const PhotoUpdateGuildSCmd& cmd)
{
  if (cmd.update().accid_svr() != 0)
  {
    const string& guid = GGuild::getPhotoGUID(cmd.update());
    bool bUpdate = false;
    for (auto &m : m_mapFramePhoto)
    {
      auto p = m.second.find(guid);
      if (p != m.second.end())
      {
        p->second.CopyFrom(cmd.update());
        bUpdate = true;
        XDBG << "[公会墙-照片更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框" << m.first << "更新照片" << cmd.ShortDebugString() << "成功" << XEND;
        break;
      }
    }
    if (!bUpdate)
    {
      XERR << "[公会墙-照片更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "更新照片" << cmd.ShortDebugString() << "失败" << XEND;
      return;
  }
  }
  if (cmd.del().accid_svr() != 0)
  {
    const string& guid = GGuild::getPhotoGUID(cmd.del());
    bool bUpdate = false;
    for (auto &m : m_mapFramePhoto)
    {
      auto p = m.second.find(guid);
      if (p != m.second.end())
      {
        XLOG << "[公会墙-照片更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框" << m.first << "删除照片" << cmd.ShortDebugString() << "成功" << XEND;
        removeFramePhoto(m.first, p->second);
        bUpdate = true;
        break;
      }
    }
    if (!bUpdate)
    {
      XERR << "[公会墙-照片更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "删除照片" << cmd.ShortDebugString() << "失败" << XEND;
      return;
    }
  }

  PhotoUpdateGuildSCmd scmd;
  scmd.CopyFrom(cmd);
  scmd.set_to_guild(false);
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToZone(m_pGuild->getZoneID(), ssend, slen);

  XLOG << "[公会墙-照片更新]" << m_pGuild->getGUID() << m_pGuild->getName() << "更新照片成功,发送至" << scmd.ShortDebugString() << "zoneid :" << m_pGuild->getZoneID() << "处理" << XEND;
}

void GuildPhotoMgr::refreshPhoto()
{
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  DWORD dwNow = xTime::getCurSec();

  TMapFramePhoto mapFramePhoto;
  for (auto m = m_mapFramePhoto.begin(); m != m_mapFramePhoto.end(); ++m)
  {
    TMapIDPhoto& mapPhoto = mapFramePhoto[m->first];
    for (auto p = m->second.begin(); p != m->second.end(); ++p)
    {
      GMember* pMember = m_pGuild->getMember(0, p->second.accid_svr());
      if (pMember == nullptr || (pMember->isOnline() == false && dwNow > pMember->getOfflineTime() + rCFG.dwPhotoMemberActiveDay * DAY_T))
      {
        XERR << "[公会墙-刷新]" << m_pGuild->getGUID() << m_pGuild->getName() << "照片刷新 accid :" << p->second.accid_svr() << "不是公会成员或者长时间未登陆,被忽略" << XEND;
        mapPhoto[GGuild::getPhotoGUID(p->second)].CopyFrom(p->second);
        continue;
      }
    }
  }

  for (auto &m : mapFramePhoto)
  {
    for (auto &p : m.second)
    {
      PhotoUpdateGuildSCmd cmd;
      cmd.set_guildid(m_pGuild->getGUID());
      cmd.mutable_del()->CopyFrom(p.second);
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToZone(m_pGuild->getZoneID(), send, len);

      removeFramePhoto(m.first, p.second);
    }
  }
}

bool GuildPhotoMgr::fromPhotoString(const string& str)
{
  BlobGuildPhoto oPhoto;
  if (oPhoto.ParseFromString(str) == false)
  {
    XERR << "[公会墙-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "反序列化失败" << XEND;
    return false;
  }

  for (int i = 0; i < oPhoto.frames_size(); ++i)
  {
    const PhotoFrame& rFrame = oPhoto.frames(i);
    if (rFrame.frameid() == 0)
    {
      XERR << "[公会墙-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "反序列化 frameid : 0 被忽略" << XEND;
      continue;
    }
    EGuildFrameType eType = GuildConfig::getMe().getPhotoFrameType(rFrame.frameid());
    if (eType == EGUILDFRAMETYPE_MAX)
    {
      XERR << "[公会墙-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "反序列化 frameid :" << rFrame.frameid() << "失败,未在 Table_ScenePhotoFrame.txt 表中找到" << XEND;
      continue;
    }

    TMapIDPhoto& mapPhoto = m_mapFramePhoto[rFrame.frameid()];
    for (int j = 0; j < rFrame.photo_size(); ++j)
    {
      const GuildPhoto& rPhoto = rFrame.photo(j);
      GMember* pMember = m_pGuild->getMember(rPhoto.charid(), rPhoto.accid_svr());
      if (pMember == nullptr)
      {
        XERR << "[公会墙-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载" << rPhoto.ShortDebugString() << "失败, 该照片不属于公会成员" << XEND;
        continue;
      }

      GuildPhoto oPhoto;
      oPhoto.CopyFrom(rPhoto);
      if (oPhoto.accid_svr() == 0)
        oPhoto.set_accid_svr(pMember->getAccid());
      string guid = GGuild::getPhotoGUID(oPhoto);
      mapPhoto[guid].CopyFrom(oPhoto);
      XLOG << "[公会墙-加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "加载" << rPhoto.ShortDebugString() << "成功" << XEND;
    }
  }

  return true;
}

void GuildPhotoMgr::queryAutoPhoto(QWORD qwAccID /*= 0*/, DWORD dwZoneID /*= 0*/, EPhotoAction eAction /*= EPHOTOACTION_LOAD_FROM_RECORD*/)
{
  if (eAction != EPHOTOACTION_LOAD_FROM_RECORD && eAction != EPHOTOACTION_LOAD_FROM_SCENE)
  {
    XERR << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据失败,无效的action" << eAction << XEND;
    return;
  }

  if (eAction == EPHOTOACTION_LOAD_FROM_RECORD)
  {
    if (m_pGuild->getMisc().getVarValue(EVARTYPE_GUILD_PHOTO_LOAD) != 0)
      return;
    m_pGuild->getMisc().setVarValue(EVARTYPE_GUILD_PHOTO_LOAD, 1);
  }
  else
  {
    if (m_pGuild->getMember(0, qwAccID) == nullptr)
    {
      XERR << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据失败 accid :" << qwAccID << "不是本公会成员" << XEND;
      return;
    }
  }

  QueryShowPhotoGuildSCmd cmd;
  cmd.set_guildid(m_pGuild->getGUID());
  cmd.set_action(eAction);

  const TMapPhotoFrameCFG& mapCFG = GuildConfig::getMe().getPhotoFrameList();
  const SGuildMiscCFG& rCFG = MiscConfig::getMe().getGuildCFG();
  for (auto &m : mapCFG)
  {
    for (auto &frame : m.second)
    {
      if (rCFG.isValidFrame(frame) == false)
        continue;
      auto f = m_mapFramePhoto.find(frame);
      if (f != m_mapFramePhoto.end())
      {
        for (auto &p : f->second)
          cmd.add_exists(GGuild::getPhotoGUID(p.second));
      }

      if (f == m_mapFramePhoto.end() || f->second.size() < MAX_FRAME_OFFLINE_LOADCOUNT)
      {
        PhotoLoad* pLoad = cmd.add_loads();
        pLoad->set_frameid(frame);
        DWORD dwCount = f == m_mapFramePhoto.end() ? 0 : f->second.size();
        pLoad->set_count(MAX_FRAME_OFFLINE_LOADCOUNT - dwCount);
      }
    }
  }

  if (cmd.loads_size() <= 0)
  {
    XLOG << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据,相框已满无需加载" << XEND;
    return;
  }

  if (qwAccID == 0)
  {
    const TVecGuildMember& vecMember = m_pGuild->getMemberList();
    vector<pair<QWORD, DWORD>> vecPMember;
    DWORD dwNow = xTime::getCurSec();
    for (auto &v : vecMember)
    {
      pair<QWORD, DWORD> p;
      p.first = v->getAccid();
      p.second = v->getOfflineTime() == 0 ? dwNow : v->getOfflineTime();
      vecPMember.push_back(p);
    }
    sort(vecPMember.begin(), vecPMember.end(), [](const pair<QWORD, DWORD>& r1, const pair<QWORD, DWORD>& r2) -> bool{
        return r1.second < r2.second;
        });
    for (auto &v : vecPMember)
    {
      cmd.add_members(v.first);
      XDBG << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据,添加公会 accid :" << v.first << "time :" << v.second << XEND;
    }
  }
  else
  {
    cmd.add_members(qwAccID);
    XDBG << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据,添加指定 accid :" << qwAccID << XEND;
  }

  PROTOBUF(cmd, send, len);
  DWORD dwRealZoneID = dwZoneID != 0 ? dwZoneID : m_pGuild->getZoneID();
  thisServer->sendCmdToZone(dwRealZoneID, send, len);
  XLOG << "[公会墙-离线加载]" << m_pGuild->getGUID() << m_pGuild->getName() << "请求数据成功,发送至RecordServer处理" << XEND;
}

void GuildPhotoMgr::addFramePhoto(DWORD dwFrameID, const GuildPhoto& rPhoto)
{
  string guid = GGuild::getPhotoGUID(rPhoto);
  if (rPhoto.accid_svr() == 0)
  {
    XERR << "[公会墙-添加照片]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框" << dwFrameID << "设置照片" << guid << rPhoto.ShortDebugString() << "失败,accid_svr为0" << XEND;
      return;
    }

  TMapIDPhoto& mapPhoto = m_mapFramePhoto[dwFrameID];
  mapPhoto[guid].CopyFrom(rPhoto);
  XLOG << "[公会墙-添加照片]" << m_pGuild->getGUID() << m_pGuild->getName()
    << "相框" << dwFrameID << "设置照片" << guid << rPhoto.ShortDebugString() << "成功,该相框包含" << mapPhoto.size() << "张照片" << XEND;
}

void GuildPhotoMgr::removeFramePhoto(DWORD dwFrameID, const GuildPhoto& rPhoto)
{
  string guid = GGuild::getPhotoGUID(rPhoto);

  TMapIDPhoto& mapPhoto = m_mapFramePhoto[dwFrameID];

  auto p = mapPhoto.find(guid);
  if (p == mapPhoto.end())
  {
    XERR << "[公会墙-删除照片]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框" << dwFrameID << "删除照片" << guid << rPhoto.ShortDebugString() << "失败,未找到该照片" << XEND;
    return;
  }
  mapPhoto.erase(p);
  XLOG << "[公会墙-删除照片]" << m_pGuild->getGUID() << m_pGuild->getName() << "相框" << dwFrameID << "删除照片" << guid << rPhoto.ShortDebugString() << "成功" << XEND;
}


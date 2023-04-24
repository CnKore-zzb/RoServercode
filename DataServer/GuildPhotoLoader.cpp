#include "GuildPhotoLoader.h"
#include "GuildConfig.h"
#include "DataServer.h"
#include "RecordCmd.pb.h"
#include "GGuild.h"
#include "CommonConfig.h"

bool SPhotoLoad::isComplete()
{
  if (setMembers.empty() == true)
    return true;
  for (auto &m : mapResult)
  {
    if (m.second.listPhotos.size() < m.second.dwNeedCount)
      return false;
  }

  return true;
}

GuildPhotoLoader::GuildPhotoLoader()
{

}

GuildPhotoLoader::~GuildPhotoLoader()
{

}

bool GuildPhotoLoader::addLoad(const QueryShowPhotoGuildSCmd& cmd)
{
  SPhotoLoad& rLoad = m_mapLoad[cmd.guildid()];

  rLoad.qwGuildID = cmd.guildid();

  for (int i = 0; i < cmd.loads_size(); ++i)
  {
    const PhotoLoad& load = cmd.loads(i);
    SPhotoResult& rResult = rLoad.mapResult[load.frameid()];
    rResult.dwFrameID = load.frameid();
    rResult.dwNeedCount = load.count();
  }

  rLoad.setExists.clear();
  for (int i = 0; i < cmd.exists_size(); ++i)
    rLoad.setExists.insert(cmd.exists(i));
  rLoad.setMembers.clear();
  for (int i = 0; i < cmd.members_size(); ++i)
    rLoad.setMembers.insert(cmd.members(i));
  return true;
}

void GuildPhotoLoader::timer(DWORD curSec)
{
  processLoad();
}

void GuildPhotoLoader::processLoad()
{
  xTime frameTime;
  for (auto m = m_mapLoad.begin(); m != m_mapLoad.end();)
  {
    SPhotoLoad& rLoad = m->second;

    for (auto accid = rLoad.setMembers.begin(); accid != rLoad.setMembers.end();)
    {
      BlobAccData oAccData;
      xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
      if (pField == nullptr)
      {
        XERR << "[公会墙-离线加载] guildid :" << m->first << "加载成员" << *accid << "失败,未获取到 accbase 数据库表" << XEND;
        accid = rLoad.setMembers.erase(accid);
        continue;
      }

      stringstream sstr;
      sstr << "accid = " << *accid;

      xRecordSet set;
      QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str(), nullptr);
      if (ret == QWORD_MAX || set.empty() == true)
      {
        XERR << "[公会墙-离线加载] guildid :" << m->first << "加载成员" << *accid << "失败,未查询到 ret :" << ret << XEND;
        accid = rLoad.setMembers.erase(accid);
        continue;
      }

      string acc_quest;
      acc_quest.assign((const char*)set[0].getBin("quest"), set[0].getBinSize("quest"));
      BlobAccQuest oQuest;
      if (oQuest.ParseFromString(acc_quest) == false)
      {
        XERR << "[公会墙-离线加载] guildid :" << m->first << "加载成员" << *accid << "失败,反序列化accquest失败" << XEND;
        accid = rLoad.setMembers.erase(accid);
        continue;
      }

      string acc_data;
      acc_data.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));
      if (oQuest.version() >= ACC_UNCOMPRESS_VERSION && uncompress(acc_data, acc_data) == false)
      {
        XERR << "[公会墙-离线加载] guildid :" << m->first << "加载成员" << *accid << "失败,解压失败" << XEND;
        accid = rLoad.setMembers.erase(accid);
        continue;
      }
      if (oAccData.ParseFromString(acc_data) == false)
      {
        XERR << "[公会墙-离线加载] guildid :" << m->first << "加载成员" << *accid << "失败,反序列化accData失败" << XEND;
        accid = rLoad.setMembers.erase(accid);
        continue;
      }

      const BlobManual& rManual = oAccData.manual();
      map<DWORD, ManualSubItem> mapScenery;
      for (int i = 0; i < rManual.data().items_size(); ++i)
      {
        const ManualItem& rItem = rManual.data().items(i);
        if (rItem.type() == EMANUALTYPE_SCENERY)
        {
          for (int j = 0; j < rItem.items_size(); ++j)
          {
            const ManualSubItem& rSubItem = rItem.items(j);
            if (rSubItem.status() > EMANUALSTATUS_UNLOCK_CLIENT)
              mapScenery[rSubItem.id()].CopyFrom(rSubItem);
          }
          break;
        }
      }

      const BlobScenery& rScenery = oAccData.scenery();
      for (int s = 0; s < rScenery.items_size(); ++s)
      {
        const SceneryItem& rItem = rScenery.items(s);
        if (rItem.upload() != 1)
          continue;
        auto m = mapScenery.find(rItem.sceneryid());
        if (m == mapScenery.end())
          continue;
        if (m->second.data_params_size() < 3)
          continue;
        const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(rItem.sceneryid());
        if (pBase == nullptr)
          continue;

        GuildPhoto oPhoto;
        oPhoto.set_accid_svr(*accid);
        oPhoto.set_anglez(rItem.anglez());
        oPhoto.set_time(rItem.time());
        oPhoto.set_mapid(pBase->getMapID());
        oPhoto.set_source(ESOURCE_PHOTO_SCENERY);
        oPhoto.set_sourceid(rItem.sceneryid());

        if (m->second.data_params(2).empty() == false)
          oPhoto.set_charid(atol(m->second.data_params(2).c_str()));
        else
          oPhoto.set_accid(*accid);

        if (rLoad.setExists.find(GGuild::getPhotoGUID(oPhoto)) != rLoad.setExists.end())
          continue;

        EGuildFrameType eType = GuildConfig::getMe().getAngleFrameType(oPhoto.anglez());
        for (auto &result : rLoad.mapResult)
        {
          if (GuildConfig::getMe().getPhotoFrameType(result.first) != eType)
            continue;
          if (result.second.listPhotos.size() >= result.second.dwNeedCount)
            continue;
          result.second.listPhotos.push_back(oPhoto);
          break;
        }
      }

      accid = rLoad.setMembers.erase(accid);

      if (rLoad.isComplete() || frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
        break;
    }

    if (rLoad.isComplete() == false)
    {
      ++m;
      if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
      {
        XLOG << "[公会墙-离线加载] guildid :" << m->first << "加载超过" << CommonConfig::m_dwSocialLoadTime << "毫秒, 还剩余" << m_mapLoad.size() << "等待加载" << XEND;
        break;
      }
      continue;
    }

    syncLoad(m->first);
    m = m_mapLoad.erase(m);

    if (frameTime.uElapse() / ONE_THOUSAND > CommonConfig::m_dwSocialLoadTime)
    {
      XLOG << "[公会墙-离线加载] guildid :" << m->first << "加载超过" << CommonConfig::m_dwSocialLoadTime << "毫秒, 还剩余" << m_mapLoad.size() << "等待加载" << XEND;
      break;
    }
  }
}

void GuildPhotoLoader::syncLoad(QWORD qwID)
{
  auto m = m_mapLoad.find(qwID);
  if (m == m_mapLoad.end())
    return;

  QueryShowPhotoGuildSCmd cmd;
  cmd.set_guildid(qwID);
  cmd.set_action(EPHOTOACTION_UPDATE_FROM_RECORD);
  for (auto &result : m->second.mapResult)
  {
    PhotoFrame* pFrame = cmd.add_results();
    pFrame->set_frameid(result.first);
    for (auto &l : result.second.listPhotos)
      pFrame->add_photo()->CopyFrom(l);
  }

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  XLOG << "[公会墙-离线加载] guildid :" << qwID << "离线加载成功,同步自动上传";
  for (int i = 0; i < cmd.results_size(); ++i)
  {
    const PhotoFrame& rFrame = cmd.results(i);
    XLOG << "frameid :" << rFrame.frameid() << rFrame.photo_size() << "张";
  }
  XLOG << XEND;
}


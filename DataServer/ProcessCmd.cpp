#include <algorithm>
#include "DataServer.h"
#include "RecordCmd.pb.h"
#include "GuildPhotoLoader.h"
#include "RecordGCityManager.h"

bool DataServer::doRecordDataCmd(const BYTE* buf, WORD len)
{
  if (!buf || !len) return false;

  using namespace Cmd;
  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case RECORDPARAM_MUSIC_DATA:
      {
        PARSE_CMD_PROTOBUF(MusicUpdateCmd, rev);

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "music");
        if (pField == nullptr)
        {
          XERR << "[点唱机-存储]" << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "获取数据表失败" << XEND;
          return true;
        }

        xRecord record(pField);
        record.put("zoneid", thisServer->getZoneID());
        record.put("charid", rev.item().charid());
        record.put("demandtime", rev.item().demandtime());
        record.put("status", rev.item().status());
        record.put("mapid", rev.item().mapid());
        record.put("npcid", rev.item().npcid());
        record.put("musicid", rev.item().musicid());
        record.put("starttime", rev.item().starttime());
        record.put("endtime", rev.item().endtime());
        record.put("name", rev.item().name());

        QWORD ret = thisServer->getDBConnPool().exeReplace(record);
        if (ret == QWORD_MAX)
        {
          XERR << "[点唱机-存储]" << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "数据存储失败" << XEND;
          return true;
        }
        XLOG << "[点唱机-存储]" << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "存储成功" << XEND;

        return true;
      }
      break;
    case RECORDPARAM_USER_PROFESSION:
      {
        PARSE_CMD_PROTOBUF(ReqUserProfessionCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (!net)
        {
          XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return true;
        }

        // 填充数据
        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
        if (pField == nullptr)
        {
          XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败,未找到charbase" << XEND;
          return true;
        }

        xRecordSet set;
        stringstream ss;
        ss << "accid = " << rev.accid();
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, ss.str().c_str());
        if(QWORD_MAX == ret || set.empty())
        {
          XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败,select charbase failed! where:" << ss.str() << XEND;
          return true;
        }

        for(DWORD d=0; d<set.size(); ++d)
        {
          QWORD qwCharId = set[d].get<QWORD>("charid");
          if(qwCharId == rev.charid())
            continue;

          DWORD dwProfession = set[d].get<DWORD>("profession");
          string data;
          data.assign((const char *)set[d].getBin("data"), set[d].getBinSize("data"));
          if(!uncompress(data, data))
          {
            XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败, uncompress data failed!" << XEND;
            continue;
          }
          BlobData oData;
          if(!oData.ParseFromString(data))
          {
            XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败, parse data failed!" << XEND;
            continue;
          }
          const BlobFighter& fighter = oData.fighter();
          DWORD dwJobLv = 0;
          for(int i=0; i<fighter.datas_size(); ++i)
          {
            if(dwProfession != fighter.datas(i).profession())
              continue;

            dwJobLv = fighter.datas(i).joblv();
            break;
          }

          UserProfessionData* pData = rev.add_datas();
          if(!pData)
          {
            XERR << "[Record协议-获取账号职业数据]" << rev.ShortDebugString() << "失败, get UserProfessionData failed!" << XEND;
            continue;
          }
          pData->set_profession(dwProfession);
          pData->set_joblv(dwJobLv);
        }

        PROTOBUF(rev, send, len);
        net->sendCmd(send, len);
        return true;
      }
      break;
    case RECORDPARAM_GUILD_MUSIC_QUERY:
      {
        PARSE_CMD_PROTOBUF(GuildMusicQueryRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net == nullptr)
        {
          XERR << "[Record协议-查询公会点唱机]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return true;
        }

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guildmusic");
        if (pField == nullptr)
        {
          XERR << "[Record协议-查询公会点唱机] 无法获取数据库" << REGION_DB << XEND;
          return true;
        }

        stringstream sstr;
        sstr << "guildid = " << rev.guildid() << " and status = 0 and endtime = 0 order by demandtime asc";
        xRecordSet set;
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-查询公会点唱机] 查询失败" << XEND;
          return true;
        }

        for (QWORD q = 0; q < ret; ++q)
        {
          MusicItem* item = rev.add_items();
          if (item == nullptr)
          {
            XERR << "[Record协议-查询公会点唱机] protobuf错误" << XEND;
            continue;
          }

          item->set_charid(set[q].get<QWORD>("charid"));
          item->set_npcid(set[q].get<DWORD>("npcid"));
          item->set_mapid(set[q].get<DWORD>("mapid"));
          item->set_musicid(set[q].get<DWORD>("musicid"));
          item->set_demandtime(set[q].get<DWORD>("demandtime"));
          item->set_starttime(set[q].get<DWORD>("starttime"));
          item->set_status(set[q].get<DWORD>("status"));
          item->set_name(set[q].getString("name"));
        }

        PROTOBUF(rev, send, len);
        net->sendCmd(send, len);
        XLOG << "[Record协议-查询公会点唱机] 公会:" << rev.guildid() << " 查询成功" << XEND;

        return true;
      }
      break;
    case RECORDPARAM_GUILD_MUSIC_UPDATE:
      {
        PARSE_CMD_PROTOBUF(GuildMusicUpdateCmd, rev);

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guildmusic");
        if (pField == nullptr)
        {
          XERR << "[公会点唱机-存储]" << rev.guildid() << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "获取数据表失败" << XEND;
          return true;
        }

        if (rev.item().status() == 1)
        {
          stringstream sstr;
          sstr << "guildid = " << rev.guildid() << " and charid = " << rev.item().charid() << " and demandtime = " << rev.item().demandtime();
          QWORD ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
          if (ret == QWORD_MAX)
          {
            XERR << "[公会点唱机-删除已播放音乐]" << rev.guildid() << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "删除失败" << XEND;
            return true;
          }
          XLOG << "[公会点唱机-删除已播放音乐]" << rev.guildid() << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "删除成功" << XEND;
        }
        else
        {
          xRecord record(pField);
          record.put("guildid", rev.guildid());
          record.put("charid", rev.item().charid());
          record.put("demandtime", rev.item().demandtime());
          record.put("status", rev.item().status());
          record.put("mapid", rev.item().mapid());
          record.put("npcid", rev.item().npcid());
          record.put("musicid", rev.item().musicid());
          record.put("starttime", rev.item().starttime());
          record.put("endtime", rev.item().endtime());
          record.put("name", rev.item().name());

          QWORD ret = thisServer->getDBConnPool().exeReplace(record);
          if (ret == QWORD_MAX)
          {
            XERR << "[公会点唱机-存储]" << rev.guildid() << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "数据存储失败" << XEND;
            return true;
          }
          XLOG << "[公会点唱机-存储]" << rev.guildid() << rev.item().charid() << rev.item().demandtime() << rev.item().musicid() << "存储成功" << XEND;
        }

        return true;
      }
      break;
    case RECORDPARAM_GUILD_MUSIC_DELETE:
      {
        PARSE_CMD_PROTOBUF(GuildMusicDeleteRecordCmd, rev);

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "guildmusic");
        if (pField == nullptr)
        {
          XERR << "[Record协议-删除公会点唱机] 公会:" << rev.guildid() << "失败,未找到guildmusic表" << XEND;
          return true;
        }
        stringstream sstr;
        sstr << "guildid = " << rev.guildid();
        QWORD ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-删除公会点唱机] 公会:" << rev.guildid() << "删除失败" << XEND;
          return true;
        }
        XLOG << "[Record协议-删除公会点唱机] 公会:" << rev.guildid() << "删除成功" << XEND;

        return true;
      }
      break;
    case RECORDPARAM_CHAT_SAVE:
      {
        PARSE_CMD_PROTOBUF(ChatSaveRecordCmd, rev);
        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "chat_msg");
        if (pField == nullptr)
        {
          XERR << "[Record协议-聊天保存] charid :" << rev.charid() << "保存失败,未找到 chat_msg 数据库表" << XEND;
          return true;
        }

        xRecord record(pField);
        record.put("charid", rev.charid());
        record.put("time", rev.time());
        record.put("portrait", rev.portrait());
        record.putBin("data", (unsigned char*)rev.data().c_str(), rev.data().size());
        QWORD ret = thisServer->getDBConnPool().exeInsert(record);
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-聊天保存] charid :" << rev.charid() << "保存失败,插入到 chat_msg 数据库表失败 ret :" << ret << XEND;
          return true;
        }
        XLOG << "[Record协议-聊天保存] charid :" << rev.charid() << "保存成功,插入到 chat_msg 数据库" << XEND;

        return true;
      }
      break;
    case RECORDPARAM_CHAT_QUERY:
      {
        PARSE_CMD_PROTOBUF(QueryChatRecordCmd, rev);
        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net == nullptr)
        {
          XERR << "[Record协议-聊天查询] 查询" << rev.ShortDebugString() << "失败,来自未知的场景 :" << rev.scenename() << XEND;
          return true;
        }
        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "chat_msg");
        if (pField == nullptr)
        {
          XERR << "[Record协议-聊天查询] 查询" << rev.ShortDebugString() << "失败,未找到 chat_msg 数据库表" << XEND;
          return true;
        }

        stringstream sstr;
        sstr << "(charid = " << rev.charid() << " or charid = " << rev.targetid() << ") and (time >= " << rev.start() << " and time <= " << rev.end() << ")";
        xRecordSet set;
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-聊天查询] 查询" << rev.ShortDebugString() << "失败,查询 chat_msg 数据库失败 ret :" << ret << XEND;
          return true;
        }

        if (set.empty() == false)
        {
          for (DWORD d = 0; d < set.size(); ++d)
          {
            QWORD qwCharID = set[d].get<QWORD>("charid");
            if (qwCharID == rev.charid())
              rev.set_selfport(set[d].get<DWORD>("portrait"));
            else
              rev.set_targetport(set[d].get<DWORD>("portrait"));

            string* p = rev.add_datas();
            p->assign((const char*)set[d].getBin("data"), set[d].getBinSize("data"));
          }

          PROTOBUF(rev, send, len);
          net->sendCmd(send, len);
        }
        XLOG << "[Record协议-聊天查询] 查询" << rev.ShortDebugString() << "成功, 从 chat_msg 数据库查询到" << set.size() << "条聊天记录" << XEND;

        return true;
      }
      break;
    case RECORDPARAM_LOTTERY_RESULT:
      {
        PARSE_CMD_PROTOBUF(LotteryResultRecordCmd, rev);

        xField *field = thisServer->getDBConnPool().getField(REGION_DB, "lottery");
        if(!field)
        {
          XERR << "[Record协议-扭蛋结果插入] 获取field失败, charid" << rev.charid() << "item" << rev.itemid() << XEND;
          return false;
        }

        xRecord record(field);
        record.put("zoneid", thisServer->getZoneID());
        record.put("charid", rev.charid());
        record.putString("name", rev.name());
        record.put("itemid", rev.itemid());
        record.putString("itemname", rev.itemname());
        record.put("type", rev.type());
        record.put("rate", rev.rate());
        record.put("timestamp", now());

        QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
        if(QWORD_MAX == ret)
        {
          XERR << "[Record协议-扭蛋结果插入] insert lottery失败, charid" << rev.charid() << "item" << rev.itemid() << XEND;
          return false;
        }

        return true;
      }
      break;
    case RECORDPARAM_PROFESSION_SAVE:
      {
        PARSE_CMD_PROTOBUF(ProfessionSaveRecordCmd, rev);

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "char_profession");
        if(!pField)
        {
          XERR << "[Record协议-职业分支保存]" << rev.ShortDebugString() << "失败,获取field失败" << XEND;
          return true;
        }

        xRecord record(pField);
        record.put("charid", rev.charid());
        record.put("branch", rev.branch());
        record.putBin("data", (unsigned char*)rev.data().c_str(), rev.data().size());
        QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
        if(QWORD_MAX == ret)
        {
          XERR << "[Record协议-职业分支保存]" << rev.ShortDebugString() << "失败,插入数据库失败 ret :" << ret << XEND;
          return true;
        }

        XLOG << "[Record协议-职业分支保存]" << rev.ShortDebugString() << "成功!"<< XEND;
        return true;
      }
      break;
    case RECORDPARAM_PROFESSION_QUERY:
      {
        PARSE_CMD_PROTOBUF(ProfessionQueryRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if(!net)
        {
          XERR << "[Record协议-职业分支查询]" << rev.ShortDebugString() << "失败,来自未知的场景 :" << rev.scenename() << XEND;
          return true;
        }

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "char_profession");
        if(!pField)
        {
          XERR << "[Record协议-职业分支查询]" << rev.ShortDebugString() << "失败,获取field失败" << XEND;
          return true;
        }

        stringstream ss;
        ss << "charid = " << rev.charid();
        xRecordSet set;
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, ss.str().c_str());
        if(QWORD_MAX == ret)
        {
          XERR << "[Record协议-职业分支查询]" << rev.ShortDebugString() << "失败,插入数据库失败 ret :" << ret << XEND;
          return true;
        }

        for(DWORD i=0; i<set.size(); ++i)
        {
          QWORD qwCharId = set[i].get<QWORD>("charid");
          if(qwCharId != rev.charid())
            continue;
          string* p = rev.add_datas();
          if(!p)
          {
            XERR << "[Record协议-职业分支查询]" << rev.ShortDebugString() << "失败,add_datas failed!" << XEND;
            return true;
          }

          p->assign((const char*)set[i].getBin("data"), set[i].getBinSize("data"));
        }

        PROTOBUF(rev, send, len);
        net->sendCmd(send, len);
        XLOG << "[Record协议-职业分支查询]" << rev.ShortDebugString() << "成功!"<< XEND;
        return true;
      }
      break;
    case RECORDPARAM_CHEAT_TAG:
      {
        PARSE_CMD_PROTOBUF(CheatTagRecordCmd, rev);
        
        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "cheat_tag");
        if (pField == nullptr)
        {
          XERR << "[Record协议-反脚本标记] 无法获取数据库" << REGION_DB << XEND;
          return false;
        }
        
        xRecord record(pField);
        record.put("charid", rev.charid());
        record.put("mininterval", rev.mininterval());
        record.put("frame", rev.frame());
        record.put("count", rev.count());

        QWORD ret = thisServer->getDBConnPool().exeReplace(record);
        if(QWORD_MAX == ret)
        {
          XERR << "[Record协议-反脚本标记]" << rev.ShortDebugString() << "失败,插入数据库失败 ret :" << ret << XEND;
          return true;
        }

        XLOG << "[Record协议-反脚本标记]" << rev.ShortDebugString() << "成功!"<< XEND;
        return true;
      }
    case RECORDPARAM_CHEAT_TAG_QUERY:
      {
        PARSE_CMD_PROTOBUF(CheatTagQueryRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if(!net)
        {
          XERR << "[Record协议-反脚本标记]" << rev.ShortDebugString() << "失败,来自未知的场景 :" << rev.scenename() << XEND;
          return true;
        }

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB,"cheat_tag");

        if (pField == nullptr)
        {
          XERR << "[Record协议-反脚本标记] 无法获取数据库" << REGION_DB << XEND;
          return false;
        }

        xRecordSet set;
        stringstream ss;
        ss << "charid = " << rev.charid();
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, ss.str().c_str());
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-反脚本标记] 查询失败" << XEND;
          return false;
        }

        CheatTagRecordCmd cmd;
        cmd.set_charid(rev.charid());
        if(ret > 1)
        {
          XERR << "[Record协议-反脚本标记] 数据重复, charid:" << rev.charid() << XEND;
          return false;
        }
        else if(ret == 0)
        {
          // 尚未有记录
          PROTOBUF(cmd, send, len);
          net->sendCmd(send, len);
          XDBG << "[Record协议-反脚本标记] 尚未有记录, charid:" << rev.charid() << XEND;
          return true;
        }

        cmd.set_mininterval(set[0].get<DWORD>("mininterval"));
        cmd.set_frame(set[0].get<DWORD>("frame"));
        cmd.set_count(set[0].get<DWORD>("count"));
        PROTOBUF(cmd, send, len);
        net->sendCmd(send, len);
        XDBG << "[Record协议-反脚本标记] 查询成功, charid:" << rev.charid() << XEND;
        return true;
      }
    default:
      break;
  }
  return true;
}

bool DataServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    case GUILDSPARAM_QUERY_SHOWPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryShowPhotoGuildSCmd, rev);
        if (rev.action() == EPHOTOACTION_LOAD_FROM_RECORD)
          GuildPhotoLoader::getMe().addLoad(rev);
        else
          XERR << "[公会消息]" << rev.ShortDebugString() << "未处理" << XEND;
      }
      break;
      case GUILDSPARAM_GUILD_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(GuildCityActionGuildSCmd, rev);
        if (rev.action() == EGUILDCITYACTION_GUILD_QUERY)
          RecordGCityManager::getMe().syncCityInfo();
        else
          RecordGCityManager::getMe().updateCityInfo(rev);
      }
      break;
      case GUILDSPARAM_GVG_RESULT:
      {
        PARSE_CMD_PROTOBUF(GvgResultGuildSCmd, rev);
        RecordGCityManager::getMe().saveCityResult(rev);
      }
      break;
    default:
      return false;
  }

  return true;
}


#include "RecordServer.h"
#include "RegCmd.h"
#include "RecordUserManager.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "PlatLogManager.h"
#include "TableManager.h"
#include "ErrorUserCmd.pb.h"
#include "ConfigManager.h"
#include "RedisManager.h"
#include "CommonConfig.h"
#include "RecordTradeMgr.h"

extern xLog srvLog;

bool RecordServer::doRegCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  RegCmd *cmd = (RegCmd *)buf;
  switch (cmd->param)
  {
    case LOGIN_OUT_SCENE_REGCMD:
      {
        LoginOutSceneRegCmd *rev = (LoginOutSceneRegCmd *)cmd;

        RecordUserManager::getMe().delData(rev->accid, rev->charid);

        sendCmdToSession(buf, len);

        return true;
      }
      break;
    case DELETE_CHAR_REGCMD:
      {
        DelCharRegCmd *rev = (DelCharRegCmd *)cmd;
        RecordUserManager::getMe().delChar(rev->id, rev->accid);
        return true;
      }
      break;
    case CREATE_CHAR_REGCMD:
      {
        CreateCharRegCmd *rev = (CreateCharRegCmd *)cmd;

        RegErrRet eErr = REG_ERR_SUCC;
        do
        {
          if (rev->role_sex <= EGENDER_MIN || rev->role_sex >= EGENDER_MAX)
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "gender :" << rev->role_sex << "不合法" << XEND;
            break;
          }
          if (rev->role_career <= EPROFESSION_MIN || rev->role_career >= EPROFESSION_MAX)
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "profession :" << rev->role_career << "不合法" << XEND;
            break;
          }

          const SItemCFG* pBase = ItemConfig::getMe().getHairCFG(rev->role_hairtype);
          if (pBase == nullptr)
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "hair:" << rev->role_hairtype << "在Table_HairStyle.txt表中找到" << XEND;
            break;
          }
          const SHairColor* pColor = TableManager::getMe().getHairColorCFG(rev->role_haircolor);
          if (pColor == nullptr)
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "haircolor:" << rev->role_haircolor << "未在Table_HairColor.txt表中找到" << XEND;
            break;
          }

          const SNewRoleCFG& rRoleCFG = MiscConfig::getMe().getNewRoleCFG();
          auto v = find(rRoleCFG.vecHair.begin(), rRoleCFG.vecHair.end(), rev->role_hairtype);
          if (v == rRoleCFG.vecHair.end())
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "hair:" << rev->role_hairtype << "未拥有该发型" << XEND;
            break;
          }

          const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
          const SRoleBaseCFG* pRoleCFG = RoleConfig::getMe().getRoleBase(rCFG.eNewCharPro);
          if (pRoleCFG == nullptr)
          {
            eErr = REG_ERR_PROFESSION_NOOPEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "novice job" << "未在Table_Class.txt表中找到" << XEND;
            break;
          }
          DWORD dwNameLen = getWordCount(rev->name);
          if (dwNameLen == 0)
          {
            eErr = REG_ERR_NAME_EMPTY;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "名字为空" << XEND;
            break;
          }
          if (dwNameLen < rCFG.dwNameMinSize)
          {
            eErr = REG_ERR_NAME_OVERMAXLEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "name:" << rev->name << "名字太短" << dwNameLen << XEND;
            break;
          }
          if (dwNameLen > rCFG.dwNameMaxSize)
          {
            eErr = REG_ERR_NAME_OVERMAXLEN;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "name:" << rev->name << "名字太长" << dwNameLen << XEND;
            break;
          }
          if (rCFG.checkNameValid(rev->name, ENAMETYPE_USER) != 0)
          {
            eErr = REG_ERR_NAME_INVALID;
            XERR << "[RecordServer-创建角色] accid :" << rev->accid << "zone :" << rev->zoneID << "name:" << rev->name << "名字含有非法字符" << XEND;
            break;
          }

          xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
          if (field == nullptr)
          {
            XERR << "[RecordServer-创建角色]" << rev->zoneID << "获取数据库 charbase 失败" << XEND;
            eErr = REG_ERR_PROFESSION_NOOPEN;
            break;
          }

          char szWhere[64] = {0};
          snprintf(szWhere, 64, "accid = %llu", rev->accid);
          xRecordSet set;
          QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, szWhere);
          if (ret == QWORD_MAX || ret >= MAX_CHAR_NUM)
          {
            XERR << "[RecordServer-创建角色]" << rev->zoneID << "accid :" << rev->accid << "ret :" << ret << XEND;
            eErr = REG_ERR_PROFESSION_NOOPEN;
            break;
          }

          xRecord record(field);
          record.put("platformid", thisServer->getPlatformID());
          record.put("zoneid", getZoneID());
          record.put("accid", rev->accid);
          record.put("sequence", rev->sequence);
          record.put("name", rev->name);
          record.put("profession", rCFG.eNewCharPro);
          record.put("destprofession", rev->role_career);
          record.put("rolelv", 1);
          record.put("mapid", rCFG.dwNewCharMapID);
          record.put("gender", rev->role_sex);
          record.put("hair", rev->role_hairtype);
          record.put("haircolor", rev->role_haircolor);
          record.put("body", rev->role_sex == EGENDER_MALE ? pRoleCFG->maleBody : pRoleCFG->femaleBody);
          record.put("eye", rev->role_sex == EGENDER_MALE ? pRoleCFG->maleEye : pRoleCFG->femaleEye);

          ret = thisServer->getDBConnPool().exeInsert(record, true);
          if (QWORD_MAX == ret)
          {
            eErr = REG_ERR_NAME_DUPLICATE;
            break;
          }

          RecordUser oData(rev->accid, ret);
          oData.base().set_zoneid(getZoneID());
          oData.base().set_accid(rev->accid);
          oData.base().set_charid(ret);
          oData.base().set_name(rev->name);
          oData.base().set_profession(rCFG.eNewCharPro);
          oData.base().set_rolelv(1);
          oData.base().set_mapid(rCFG.dwNewCharMapID);
          oData.base().set_gender(static_cast<EGender>(rev->role_sex));
          oData.base().set_hair(rev->role_hairtype);
          oData.base().set_haircolor(rev->role_haircolor);
          oData.base().set_body(rev->role_sex == EGENDER_MALE ? pRoleCFG->maleBody : pRoleCFG->femaleBody);
          oData.base().set_eye(rev->role_sex == EGENDER_MALE ? pRoleCFG->maleEye : pRoleCFG->femaleEye);
          oData.saveRedis();

          XLOG << "[RecordServer-创建角色]" << ret << rev->name << "地图:" << rCFG.dwNewCharMapID << XEND;

          {
            //创建角色日志
            PlatLogManager::getMe().createCharLog(thisServer,
              thisServer->getPlatformID(),
              rev->zoneID,
              rev->accid,
              ret,
              rev->name,
              0,  /*is guest*/
              "",/*ip*/
              "",/*device*/
              ""/*mac*/,
              ""/*agent*/,
              rev->role_sex,
              rev->role_hairtype,
              rev->role_haircolor
              );
          }

          CreateOKCharRegCmd send;
          send.accid = rev->accid;
          send.charid = ret;
          strncpy(send.gatename, rev->gateName, MAX_NAMESIZE);
          thisServer->sendCmdToSession(&send, sizeof(send));
        } while(0);

        if (eErr != REG_ERR_SUCC)
        {
          RetCreateCharRegCmd retsend;
          retsend.accid = rev->accid;
          retsend.zoneID = rev->zoneID;
          retsend.ip = rev->ip;
          retsend.port = rev->port;
          retsend.ret = eErr;
          strncpy(retsend.name, rev->name, MAX_NAMESIZE);

          thisServer->sendCmdToServer(&retsend, sizeof(retsend), "SessionServer");
        }

        return true;
      }
      break;
    default:
      break;
  }
  return true;
}

bool RecordServer::doRecordDataCmd(const BYTE* buf, WORD len)
{
  xCommand* cmd = (xCommand*)buf;
  switch(cmd->param)
  {
    case RECORDPARAM_NOTIFYLOGIN:
      {
        PARSE_CMD_PROTOBUF(NotifyLoginRecordCmd, rev);
        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net)
        {
          RecordUser* pData = RecordUserManager::getMe().getData(rev.id(), rev.accid());
          if (pData == nullptr)
          {
            ErrUserCharBaseRecordCmd cmd;
            cmd.set_id(rev.id());
            PROTOBUF(cmd, send, len);
            net->sendCmd(send, len);
          }
          else
          {
            DWORD dwMaxBuf = TRANS_BUFSIZE;
            UserDataRecordCmd cmd;

            cmd.set_charid(pData->charid());

            RecordUserData data;
            /*data.mutable_base()->CopyFrom(pData->oBase);
            data.set_data(pData->oBlob.c_str(), pData->oBlob.size());
            data.set_credit(pData->oCredit.c_str(), pData->oCredit.size());
            data.set_acc_quest(pData->oAccQuest.c_str(), pData->oAccQuest.size());
            data.set_store(pData->oStore.c_str(), pData->oStore.size());*/
            pData->toData(data);
            string blob;
            data.SerializeToString(&blob);

            if (blob.size() <= dwMaxBuf)
            {
              cmd.set_data(blob.c_str(), blob.size());
              cmd.set_over(true);
              PROTOBUF(cmd, send, len);
              net->sendCmd(send, len);
            }
            else
            {
              DWORD dwIndex = 0;
              while (true)
              {
                if (blob.size() - dwIndex * dwMaxBuf > dwMaxBuf)
                {
                  cmd.set_data(blob.c_str() + dwIndex * dwMaxBuf, dwMaxBuf);
                  cmd.set_over(false);
                  PROTOBUF(cmd, send, len);
                  net->sendCmd(send, len);
                }
                else
                {
                  cmd.set_data(blob.c_str() + dwIndex * dwMaxBuf, blob.size() - dwIndex * dwMaxBuf);
                  cmd.set_over(true);
                  PROTOBUF(cmd, send, len);
                  net->sendCmd(send, len);
                  break;
                }
                ++dwIndex;
                cmd.clear_data();
              }
            }
          }
        }
      }
      return true;
    case RECORDPARAM_USERDATA:
      {
        PARSE_CMD_PROTOBUF(UserDataRecordCmd, rev);

        RecordUser* pData = RecordUserManager::getMe().getData(rev.charid(), rev.accid());
        if (pData == nullptr || pData->updateData(rev) == false)
          return true;

        UnregType rType = (UnregType)rev.unregtype();
        switch (rType)
        {
          case UnregType::Normal:
          case UnregType::Delete:
            {
              LoginOutSceneRegCmd send;
              send.accid = rev.accid();
              send.charid = rev.charid();

              //SRecordUserData* pData = RecordUserManager::getMe().getData(rev.charid());
              if (pData != nullptr)
              {
                QWORD qwAccID = pData->accid();
                if (rType == UnregType::Delete)
                  XLOG << "[RecordServer-删号]" << pData->accid() << pData->charid() << pData->profession() << pData->name() << XEND;
                else
                  XLOG << "[RecordServer-下线]" << pData->accid() << pData->charid() << pData->profession() << pData->name() << XEND;
                RecordUserManager::getMe().delData(rev.accid(), rev.charid());
                if (rType == UnregType::Delete)
                  RecordUserManager::getMe().delChar(rev.charid(), qwAccID);
              }
              thisServer->sendCmdToSession(&send, sizeof(send));
            }
            break;
          case UnregType::ChangeScene:
            {
              //SRecordUserData* pData = RecordUserManager::getMe().getData(rev.charid(), rev.accid());
              if (pData)
              {
                //pData->loadStore();
                ChangeSceneSingleSessionCmd cmd;
                cmd.set_charid(rev.charid());
                cmd.set_mapid(pData->mapid());
                PROTOBUF(cmd, send, len);
                thisServer->sendCmdToSession(send, len);
              }
            }
            break;
          case UnregType::ServerStop:
            {
              //SRecordUserData* pData = RecordUserManager::getMe().getData(rev.charid());
              if (pData != nullptr)
              {
                bool bSuccess = pData->saveData();
                XLOG << "[RecordServer-关闭]" << pData->accid() << pData->charid() << pData->profession() << pData->name() << "保存:" << bSuccess << XEND;
                RecordUserManager::getMe().delData(pData->accid(), rev.charid());
              }
            }
            break;
          case UnregType::GMSave:
            {
              if (pData)
              {
                bool bSuccess = pData->saveData();
                XLOG << "[RecordServer-GMSAVE] 命令保存数据" << pData->accid() << pData->charid() << pData->profession() << pData->name() << "保存:" << bSuccess << XEND;
              }
            }
            break;
          default:
            break;
        }
        return true;
      }
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
      }
      return true;
    case RECORDPARAM_LOADLUA:
      {
        PARSE_CMD_PROTOBUF(LoadLuaSceneRecordCmd, rev);

        // lua to do..

        if (rev.has_log() == true)
          srvLog.reload();
        if (rev.has_table() == true)
        {
          TVecConfigType vec;
          ConfigManager::getMe().getType(rev.table(), vec);
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (cfg.isReload() && cfg.isRecordLoad())
                ConfigManager::getMe().loadConfig(cfg);
            }
          }
          for (auto &it : vec)
          {
            ConfigEnum cfg;
            if (ConfigManager::getMe().getConfig(it, cfg))
            {
              if (cfg.isReload() && cfg.isRecordLoad())
                ConfigManager::getMe().checkConfig(cfg);
            }
          }
        }
      }
      return true;
    /*case RECORDPARAM_STORE_PUT:
      {
        PARSE_CMD_PROTOBUF(PutStoreRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net == nullptr)
        {
          XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return true;
        }

        do
        {
          xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
          if (pField == nullptr)
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,未找到 accbase 数据表" << XEND;
            rev.set_msg("未找到accbase数据表");
            break;
          }
          pField->setValid("accid, charid");
          xRecordSet set;
          stringstream sstr;
          sstr << "accid = " << rev.accid();
          QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
          if (ret == QWORD_MAX || ret == 0)
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,查询失败 :" << ret << XEND;
            rev.set_msg("未从accbase找到玩家");
            break;
          }
          if (set[0].get<QWORD>("charid") != rev.charid())
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,未是当前登录的角色" << XEND;
            rev.set_msg("未是当前登录的角色");
            break;
          }

          pField = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
          if (pField == nullptr)
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,未找到 char_store 数据表" << XEND;
            rev.set_msg("未找到char_store数据表");
            break;
          }

          string item;
          if (rev.data().SerializeToString(&item) == false)
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,序列化失败" << XEND;
            rev.set_msg("序列化失败");
            break;
          }

          xRecord record(pField);
          record.put("accid", rev.accid());
          record.putString("itemid", rev.data().base().guid());
          record.putBin("item", (unsigned char *)(item.c_str()), item.size());
          ret = thisServer->getDBConnPool().exeInsert(record, true, true);
          if (ret == QWORD_MAX)
          {
            XERR << "[仓库存入]" << rev.ShortDebugString() << "失败,插入失败" << XEND;
            rev.set_msg("插入失败");
            break;
          }

          XLOG << "[仓库存入]" << rev.ShortDebugString() << "成功" << XEND;
        } while (0);

        PROTOBUF(rev, send, len);
        net->sendCmd(send, len);
      }
      return true;
    case RECORDPARAM_STORE_OFF:
      {
        PARSE_CMD_PROTOBUF(OffStoreRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net == nullptr)
        {
          XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return true;
        }

        do
        {
          xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
          if (pField == nullptr)
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,未找到 accbase 数据表" << XEND;
            rev.set_msg("未找到accbase数据表");
            break;
          }
          pField->setValid("accid, charid");
          xRecordSet set;
          stringstream sstr;
          sstr << "accid = " << rev.accid();
          QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
          if (ret == QWORD_MAX || ret == 0)
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,查询失败 :" << ret << XEND;
            rev.set_msg("未从accbase找到玩家");
            break;
          }
          if (set[0].get<QWORD>("charid") != rev.charid())
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,未是当前登录的角色" << XEND;
            rev.set_msg("未是当前登录的角色");
            break;
          }

          pField = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
          if (pField == nullptr)
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,未找到 char_store 数据表" << XEND;
            rev.set_msg("未找到 char_store 数据表");
            break;
          }

          sstr.str("");
          sstr << "accid=" << rev.accid() << " and itemid=\"" << rev.data().base().guid() << "\"";
          set.clear();
          ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
          if (ret == QWORD_MAX || ret == 0)
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,未找到该物品" << XEND;
            rev.set_msg("未找到该物品");
            break;
          }

          string item;
          item.assign((const char *)set[0].getBin("item"), set[0].getBinSize("item"));

          ItemData oData;
          if (oData.ParseFromString(item) == false)
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,反序列化失败" << XEND;
            rev.set_msg("反序列化失败");
            break;
          }
          if (rev.data().base().count() > oData.base().count())
          {
            XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,数量不足" << XEND;
            rev.set_msg("数量不足");
            break;
          }

          DWORD dwLeft = oData.base().count() - rev.data().base().count();
          if (dwLeft == 0)
          {
            ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
            if (ret == QWORD_MAX)
            {
              XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,删除失败" << XEND;
              rev.set_msg("删除失败");
              break;
            }
            XLOG << "[仓库取出]" << rev.ShortDebugString() << "成功取出" << XEND;
          }
          else
          {
            oData.mutable_base()->set_count(dwLeft);
            item.clear();
            if (oData.SerializeToString(&item) == false)
            {
              XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,序列化失败" << XEND;
              rev.set_msg("序列化失败");
              break;
            }

            sstr.str("");
            sstr << "accid=" << rev.accid() << " and itemid=\"" << rev.data().base().guid() << "\"";
            xRecord record(pField);
            record.putBin("item", (unsigned char *)item.c_str(), item.size());
            ret = thisServer->getDBConnPool().exeUpdate(record, sstr.str().c_str());
            if (ret == 0)
            {
              XERR << "[仓库取出]" << rev.ShortDebugString() << "失败,更新失败" << XEND;
              rev.set_msg("更新失败");
              break;
            }
          }
        } while (0);

        PROTOBUF(rev, send, len);
        net->sendCmd(send, len);
      }
      return true;*/
    case RECORDPARAM_STORE_ITEMMODIFY:
      {
        PARSE_CMD_PROTOBUF(ItemModifyRecordCmd, rev);

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (net == nullptr)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return true;
        }

        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
        if (pField == nullptr)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,未找到 accbase 数据表" << XEND;
          break;
        }
        pField->setValid("accid, charid");
        xRecordSet set;
        stringstream sstr;
        sstr << "accid = " << rev.accid();
        QWORD ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
        if (ret == QWORD_MAX || ret == 0)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,查询失败 :" << ret << XEND;
          break;
        }
        if (set[0].get<QWORD>("charid") != rev.charid())
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,未是当前登录的角色" << XEND;
          break;
        }

        pField = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
        if (pField == nullptr)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,未找到 char_store 数据表" << XEND;
          break;
        }

        sstr.str("");
        sstr << "accid=" << rev.accid() << " and itemid=\"" << rev.guid() << "\"";
        set.clear();
        ret = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str());
        if (ret == QWORD_MAX || ret == 0)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,未找到该物品" << XEND;
          break;
        }

        string item;
        item.assign((const char *)set[0].getBin("item"), set[0].getBinSize("item"));

        ItemData oData;
        if (oData.ParseFromString(item) == false)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,反序列化失败" << XEND;
          break;
        }

        if (oData.base().id() == rev.newid())
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,id未发生变化" << XEND;
          break;
        }
        oData.mutable_base()->set_id(rev.newid());

        item.clear();
        if (oData.SerializeToString(&item) == false)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,序列化失败" << XEND;
          break;
        }

        xRecord record(pField);
        record.put("accid", rev.accid());
        record.putString("itemid", oData.base().guid());
        record.putBin("item", (unsigned char*)item.c_str(), item.size());
        ret = thisServer->getDBConnPool().exeInsert(record, true, true);
        if (ret == QWORD_MAX)
        {
          XERR << "[仓库-修改]" << rev.ShortDebugString() << "失败,覆盖失败" << XEND;
          break;
        }
        XLOG << "[仓库-修改]" << rev.ShortDebugString() << "成功" << XEND;
      }
      return true;
    case RECORDPARAM_DEL_PATCH_CHAR:
      {
        PARSE_CMD_PROTOBUF(DelPatchCharRecordCmd, rev);
        string dbname;
        if (rev.type() == EPATCHTYPE_QUEST)
          dbname = "quest_patch";
        else
        {
          XERR << "[Record协议-删除补丁号] 删除 charid :" << rev.charid() << "失败,未找到" << rev.type() << "对应数据库表" << XEND;
          return true;
        }
        xField* pField = thisServer->getDBConnPool().getField(REGION_DB, dbname.c_str());
        if (pField == nullptr)
        {
          XERR << "[Record协议-删除补丁号] 删除 charid :" << rev.charid() << "失败,未找到" << dbname << "数据库表" << XEND;
          return true;
        }

        stringstream sstr;
        sstr << "charid = " << rev.charid();
        QWORD ret = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
        if (ret == QWORD_MAX)
        {
          XERR << "[Record协议-删除补丁号] 从" << dbname << "删除 charid :" << rev.charid() << "失败,删除失败 ret :" << ret << XEND;
          return true;
        }

        XLOG << "[Record协议-删除补丁号] 从" << dbname << "删除 charid :" << rev.charid() << "成功"<< XEND;
      }
      return true;
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
      }
      return true;
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
      }
      return true;
    case RECORDPARAM_AUTHORIZE_CHANGE:
      {
        PARSE_CMD_PROTOBUF(ChangeAuthorizeRecordCmd, rev);

        do
        {
          xField *field = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
          if (field == nullptr)
          {
            XERR << "[安全密码:修改]" << rev.ShortDebugString() << "失败,未找到 accbase 数据表" << XEND;
            break;
          }
          else
          {
            xRecord record(field);
            char updateWhere[256] = {0};
            snprintf(updateWhere, sizeof(updateWhere), "accid=%lu ", rev.accid());
            record.put("password", rev.password());
            record.put("pwdresettime", rev.resettime());
            QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
            if (QWORD_MAX == updateRet)
            {
              XERR << "[安全密码:修改]" << rev.accid() << "更新失败" << XEND;
              break;
            }
          }

          XLOG << "[安全密码:修改]" << rev.ShortDebugString() << "成功" << XEND;
        } while (0);
      }
      return true;
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
      }
      return true;
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
      }
      return true;
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
      }
      return true;
    case RECORDPARAM_USER_RENAME_QUERY:
      {
        PARSE_CMD_PROTOBUF(UserRenameQueryRecordCmd, rev);

        const SSystemCFG& rCFG = MiscConfig::getMe().getSystemCFG();
        const SRoleBaseCFG* pRoleCFG = RoleConfig::getMe().getRoleBase(rCFG.eNewCharPro);
        if(!pRoleCFG) return false;

        ServerTask *net = thisServer->getConnectedServer("SceneServer", rev.scenename());
        if (!net)
        {
          XERR << "[角色改名]" << rev.ShortDebugString() << "失败,未找到场景" << rev.scenename() << XEND;
          return false;
        }

        RecordUser* pData = RecordUserManager::getMe().getData(rev.charid(), rev.accid());
        if(!pData)
        {
          XERR << "[角色改名]" << rev.ShortDebugString() << "失败,未找到角色" << rev.charid() << XEND;
          return false;
        }

        DWORD dwNameLen = getWordCount(rev.newname());
        if (dwNameLen == 0)
        {
          XERR << "[RecordServer-角色改名] accid :" << rev.accid() << "charid :" << rev.charid() << "名字为空" << XEND;
          break;
        }
        if (dwNameLen < rCFG.dwNameMinSize)
        {
          XERR << "[RecordServer-角色改名] accid :" << rev.accid() << "charid :" << rev.charid() << "name:" << rev.newname() << "名字太短" << dwNameLen << XEND;
          break;
        }
        if (dwNameLen > rCFG.dwNameMaxSize)
        {
          XERR << "[RecordServer-创建角色] accid :" << rev.accid() << "charid :" << rev.charid() << "name:" << rev.newname() << "名字太长" << dwNameLen << XEND;
          break;
        }
        if (rCFG.checkNameValid(rev.newname(), ENAMETYPE_USER) != 0)
        {
          XERR << "[RecordServer-角色改名] accid :" << rev.accid() << "charid :" << rev.charid() << "name:" << rev.newname() << "名字含有非法字符" << XEND;
          break;
        }

        xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
        if (!field)
        {
          XERR << "[角色名称:修改]" << rev.ShortDebugString() << "失败,未找到 charbase 数据表" << XEND;
          break;
        }
        else
        {
          xRecord record(field);
          char updateWhere[256] = {0};
          snprintf(updateWhere, sizeof(updateWhere), "charid=%lu ", rev.charid());
          record.put("name", rev.newname());
          QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
          if (QWORD_MAX == updateRet)
          {
            UserRenameQueryRecordCmd cmd;
            cmd.CopyFrom(rev);
            cmd.set_code(ERENAME_CONFLICT);

            PROTOBUF(cmd, send, len);
            net->sendCmd(send, len);
            XERR << "[角色名称:修改]" << rev.accid() << "更新失败" << XEND;
            break;
          }

          XLOG << "[角色名称:修改]" << rev.accid() << "更改" << rev.oldname() << "为" << rev.newname() << XEND;
          net->sendCmd(buf, len);
        }
      }
      break;
    case RECORDPARAM_USER_RENAME_RESULT:
      {
        PARSE_CMD_PROTOBUF(UserRenameResultRecordCmd, rev);

        if(rev.success())
        {
          RecordUser* pData = RecordUserManager::getMe().getData(rev.charid(), rev.accid());
          if(!pData)
          {
            XERR << "[角色改名结果]" << rev.ShortDebugString() << "成功,未找到角色" << rev.charid() << XEND;
            return false;
          }

          pData->base().set_name(rev.newname());
          pData->saveRedis();
        }
        else
        {
          xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
          if (!field)
          {
            XERR << "[角色名称:改回]" << rev.ShortDebugString() << "失败,未找到 charbase 数据表" << XEND;
            break;
          }
          else
          {
            xRecord record(field);
            char updateWhere[256] = {0};
            snprintf(updateWhere, sizeof(updateWhere), "charid=%lu ", rev.charid());
            record.put("name", rev.oldname());
            QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
            if (QWORD_MAX == updateRet)
            {
              XERR << "[角色名称:改回]" << rev.charid() << "更新失败" << XEND;
              break;
            }

            XLOG << "[角色名称:改回]" << rev.charid() << "改回原名" << rev.oldname() << XEND;
          }
        }
      }
      break;
    default:
      break;
  }

  return true;
}

bool RecordServer::doTradeCmd(const BYTE* buf, WORD len)
 {
   if (!buf || !len) return false;
   using namespace Cmd;
   xCommand* cmd = (xCommand*)buf;
   switch (cmd->param)
   {
   case FORWARD_USERCMD_TO_RECORD:
   {
     if (!CommonConfig::getMe().IsTradeServerOpen())
     {
       XERR << "[交易] 交易所服已关闭" << XEND;
       return true;
     }
     PARSE_CMD_PROTOBUF(Cmd::ForwardUserCmdToRecordCmd, rev);
     return RecordTradeMgr::getMe().doTradeUserCmd(rev.charid(), (const BYTE*)rev.data().c_str(), rev.len());
   }
   }
   return false;
 }

bool RecordServer::doGuildCmd(BYTE* buf, WORD len)
{
  if (!buf || !len)
    return false;

  xCommand* cmd = (xCommand*)buf;
  switch (cmd->param)
  {
    /*
    case GUILDSPARAM_GUILD_CITY_ACTION:
      {
        PARSE_CMD_PROTOBUF(GuildCityActionGuildSCmd, rev);
        if (rev.action() == EGUILDCITYACTION_GUILD_QUERY)
          RecordGCityManager::getMe().syncCityInfo();
        else
          RecordGCityManager::getMe().updateCityInfo(rev);
      }
      break;
      */
    /*case GUILDSPARAM_QUERY_SHOWPHOTOLIST:
      {
        PARSE_CMD_PROTOBUF(QueryShowPhotoGuildSCmd, rev);
        if (rev.action() == EPHOTOACTION_LOAD_FROM_RECORD)
          RecordGPhotoManager::getMe().addLoad(rev);
        else
          XERR << "[公会消息]" << rev.ShortDebugString() << "未处理" << XEND;
      }
      break;*/
    default:
      return false;
  }

  return true;
}


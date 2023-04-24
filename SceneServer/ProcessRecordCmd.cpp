#include "SceneServer.h"
//#include "Define.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "Scene.h"
#include "SceneManager.h"
#include "RecordTrade.pb.h"
#include "SceneTrade.pb.h"
#include "MsgManager.h"
#include "zlib/zlib.h"
#include "StatisticsDefine.h"
#include "GuildMusicBoxManager.h"
#include "GuildCityManager.h"
#include "SceneUserDataThread.h"
#include "Menu.h"

bool SceneServer::doRecordDataCmd(const BYTE* buf, WORD len)
{
  using namespace Cmd;//::Record;
  xCommand* cmd = (xCommand*)buf;
  switch(cmd->param)
  {
    case RECORDPARAM_ERROR_USERDATA:
      {
        PARSE_CMD_PROTOBUF(ErrUserCharBaseRecordCmd, rev);
        SceneUser *user = SceneUserManager::getMe().getLoginUserByID(rev.id());
        if (user)
        {
          XERR << "[登录]" << user->accid << user->id << user->name << "读档失败" << XEND;
          SceneUserManager::getMe().delLoginUser(user);
        }
        logErr(user, rev.id(), NULL, Cmd::REG_ERR_GET_USER_DATA, false);
        return true;
      }
      break;
    case RECORDPARAM_USERDATA:
      {
        PARSE_CMD_PROTOBUF(UserDataRecordCmd, rev);

        //SceneUser* pUser = SceneUserManager::getMe().getLoginUserByID(rev.data().base().charid());
        SceneUser* pUser = SceneUserManager::getMe().getLoginUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[登录失败]" << rev.charid() << "LoginUser管理中找不到玩家" << XEND;
          logErr(NULL, rev.charid(), NULL, REG_ERR_SET_USER_DATA_SCENE, false);
          return true;
        }

        if (pUser->m_blThreadLoad)
        {
          XERR << "[登录失败]" << rev.charid() << "LoginUser已经在加载队列" << XEND;
          return true;
        }

        pUser->appendData(rev);
        if (rev.over() == false)
        {
          XLOG << "[登陆]" << rev.charid() << "消息未发送完毕" << XEND;
          return true;
        }

        pUser->m_blThreadLoad = true;

        RecordUserData data;
        data.ParseFromString(pUser->getData());
        SceneUserLoadThread::getMe().add(data);

        XLOG << "[玩家-登录]" << pUser->accid << pUser->id << pUser->name << "收到数据" << XEND;

        return true;
      }
      break;
    /*case RECORDPARAM_STORE_PUT:
      {
        PARSE_CMD_PROTOBUF(PutStoreRecordCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[玩家-仓库]" << rev.charid() << "把" << rev.ShortDebugString() << "存仓库,玩家不在线" << XEND;
          return true;
        }

        MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
        StorePackage* pStorePack = dynamic_cast<StorePackage*>(pUser->getPackage().getPackage(EPACKTYPE_STORE));
        if (pMainPack == nullptr || pStorePack == nullptr)
        {
          XERR << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "把" << rev.ShortDebugString() << "存仓库,背包错误" << XEND;
          return true;
        }

        if (rev.msg().empty() != true)
        {
          ItemBase* pBase = pStorePack->getItem(rev.data().base().guid());
          if (pBase == nullptr)
          {
            XERR << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "把" << rev.ShortDebugString() << "存仓库失败,物品消失了" << XEND;
            return true;
          }

          if (ItemManager::getMe().isEquip(rev.data().base().type()) == false)
          {
            ItemData oData;
            pBase->fromItemData(oData);
            oData.mutable_base()->set_source(ESOURCE_PUBLIC_PUTSTORE);
            pMainPack->addItem(oData, EPACKMETHOD_NOCHECK);
            pStorePack->reduceItem(rev.data().base().guid(), ESOURCE_PUBLIC_PUTSTORE, oData.base().count());
            XERR << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "把" << rev.ShortDebugString() << "存仓库,还原了" << XEND;
            return true;
          }
          else
          {
            pStorePack->reduceItem(rev.data().base().guid(), ESOURCE_PUBLIC_PUTSTORE, rev.data().base().count(), false);
            pMainPack->addItemObj(pBase, true, ESOURCE_PUBLIC_PUTSTORE);
          }

          XERR << "[玩家-仓库]" << rev.charid() << "把" << rev.ShortDebugString() << "存仓库,失败" << rev.msg() << XEND;
          return true;
        }

        pUser->getPackage().setStoreTick(0);
        XLOG << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "把" << rev.ShortDebugString() << "存仓库,成功" << XEND;
      }
      return true;
    case RECORDPARAM_STORE_OFF:
      {
        PARSE_CMD_PROTOBUF(OffStoreRecordCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          AddOfflineMsgSocialCmd cmd;
          cmd.mutable_msg()->set_type(EOFFLINEMSG_ADD_ITEM);
          cmd.mutable_msg()->set_targetid(rev.charid());
          cmd.mutable_msg()->mutable_itemdata()->CopyFrom(rev.data());

          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);

          XLOG << "[玩家-仓库]" << rev.charid() << "从仓库取出" << rev.ShortDebugString() << "玩家不在线, 存到离线消息" << XEND;
          return true;
        }

        pUser->getPackage().clearOffStoreLock();

        EPackType eType = rev.msg().empty() == true ? EPACKTYPE_MAIN : EPACKTYPE_STORE;
        const SFoodMiscCFG& rCFG = MiscConfig::getMe().getFoodCfg();

        if (rev.msg().empty() == true)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rev.data().base().id());
          if (pCFG != nullptr && ItemConfig::getMe().isQuest(pCFG->eItemType) == true)
          {
            eType = EPACKTYPE_QUEST;
            XLOG << "[玩家-仓库]" << rev.charid() << "从仓库取出" << rev.ShortDebugString() << "成功,该道具为任务道具,修改" << EPACKTYPE_MAIN << "为" << EPACKTYPE_QUEST << XEND;
          }
          else if (pCFG != nullptr && pUser->getMenu().isOpen(EMENUID_FOOD_PACK) == true && rCFG.isFoodItem(pCFG->eItemType) == true)
          {
            eType = EPACKTYPE_FOOD;
            XLOG << "[玩家-仓库]" << rev.charid() << "从仓库取出" << rev.ShortDebugString() << "成功,该道具为料理道具,修改" << EPACKTYPE_MAIN << "为" << EPACKTYPE_FOOD << XEND;
          }
        }

        BasePackage* pPack = pUser->getPackage().getPackage(eType);
        if (pPack == nullptr)
        {
          XERR << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "从仓库取出" << rev.ShortDebugString() << (rev.msg().empty() == true ? "成功" : "失败") << "未发现" << eType << XEND;
          return true;
        }

        rev.mutable_data()->mutable_base()->set_source(ESOURCE_PUBLIC_OFFSTORE);
        if (ItemManager::getMe().isEquip(rev.data().base().type()) == false)
        {
          ItemData oData;
          oData.CopyFrom(rev.data());
          pPack->addItem(oData, EPACKMETHOD_NOCHECK);
        }
        else
        {
          ItemBase* pBase = ItemManager::getMe().createItem(rev.data().base());
          if (pBase == nullptr)
          {
            XERR << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
              << "从仓库取出" << rev.ShortDebugString() << (rev.msg().empty() == true ? "成功" : "失败") << "创建失败" << XEND;
            return true;
          }
          pBase->fromItemData(rev.data());
          pPack->addItemObj(pBase, true, ESOURCE_PUBLIC_OFFSTORE);
        }

        pUser->getPackage().setStoreTick(0);
        XLOG << "[玩家-仓库]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
          << "从仓库取出" << rev.ShortDebugString() << (rev.msg().empty() == true ? "成功" : "失败") << XEND;
      }
      return true;*/
    case RECORDPARAM_CHAT_QUERY:
      {
        PARSE_CMD_PROTOBUF(QueryChatRecordCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (pUser == nullptr)
        {
          XERR << "[Record消息-聊天查询] charid :" << rev.charid() << "不在线" << XEND;
          return true;
        }

        TListChatItem listItem;
        for (int i = 0; i < rev.datas_size(); ++i)
        {
          string data;
          if (uncompress(rev.datas(i), data) == false)
            continue;

          BlobChatItem oItem;
          if (oItem.ParseFromString(data) == false)
            continue;

          for (int j = 0; j < oItem.items_size(); ++j)
            listItem.push_back(oItem.items(j));
        }

        vector<ChatItem> vecItem;
        vecItem.insert(vecItem.end(), listItem.begin(), listItem.end());
        sort(vecItem.begin(), vecItem.end(), [](const ChatItem& rItem1, const ChatItem& rItem2) -> bool{
          return rItem1.time() < rItem2.time();
        });
        for (auto &v : vecItem)
        {
          ChatRetCmd cmd;

          /*if (v.charid() == rev.charid())
            cmd.set_portrait(rev.selfport());
          else
            cmd.set_portrait(rev.targetport());*/

          cmd.set_channel(ECHAT_CHANNEL_CHAT);
          cmd.set_name(v.name());
          cmd.set_str(v.msg());
          PROTOBUF(cmd, send, len);
          pUser->sendCmdToMe(send, len);
        }
        XLOG << "[Record消息-聊天查询] charid :" << rev.charid() << "查询了" << vecItem.size() << "条记录" << XEND;
      }
      return true;
    case RECORDPARAM_GUILD_MUSIC_QUERY:
      {
        PARSE_CMD_PROTOBUF(GuildMusicQueryRecordCmd, rev);
        GuildMusicBoxManager::getMe().loadMusicItem(rev);
      }
      return true;
    case RECORDPARAM_USER_PROFESSION:
      {
        PARSE_CMD_PROTOBUF(ReqUserProfessionCmd, rev);
        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if(!pUser)
          return false;

        //检测acc补偿区分
        if(pUser->getMainCharId())
          return true;

        std::vector<std::pair<DWORD, DWORD>> vecProfessions;
        for(int i=0; i<rev.datas_size(); ++i)
        {
          vecProfessions.push_back(std::make_pair(rev.datas(i).profession(), rev.datas(i).joblv()));
        }

        if(!pUser->fixBranch(vecProfessions))
          return false;

        // 设置acc补偿区分
        pUser->setMainCharId(pUser->id);
      }
      return true;
    case RECORDPARAM_USER_RENAME_QUERY:
      {
        PARSE_CMD_PROTOBUF(UserRenameQueryRecordCmd, rev);

        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if (!pUser) return true;

        // recordserver改名失败直接返回
        if (ERENAME_SUCCESS != rev.code())
        {
          XLOG << "[Record消息-角色改名] scene server 改名失败" << rev.ShortDebugString() << XEND;

          UserRenameCmd cmd;
          cmd.set_name(rev.newname());
          cmd.set_code(rev.code());

          PROTOBUF(cmd, send, len);
          pUser->sendCmdToMe(send, len);
          return true;
        }

        // 防止连续两次改名协议，导致逻辑异常
        if (rev.oldname() != pUser->name || rev.newname() == pUser->name)
        {
          XLOG << "[Record消息—角色改名] scene server 改名失败, 当前名字" << pUser->name << "修改前名字" << rev.oldname() << "修改后名字" << rev.newname() << XEND;
          return false;
        }

        UserRenameResultRecordCmd cmd;

        cmd.set_charid(rev.charid());
        cmd.set_accid(rev.accid());
        cmd.set_newname(rev.newname());
        cmd.set_oldname(rev.oldname());

        do{
          if (!pUser)
          {
            XLOG << "[Record消息-角色改名] 角色不在当前场景" << rev.ShortDebugString() << XEND;
            break;
          }

          DWORD itemId = MiscConfig::getMe().getPlayerRenameCFG().dwRenameItemId;
          MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
          if(!pMainPack || !pMainPack->checkItemCount(itemId, 1))
          {
            XLOG << "[Record消息-角色改名] 扣除改名卡失败 itemid:" << itemId << "rev:" << rev.ShortDebugString() << XEND;
            break;
          }

          pMainPack->reduceItem(itemId,ESOURCE_USER_RENAME, 1);
          pUser->rename(rev.newname());
          cmd.set_success(true);
        }while(0);

        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToRecord(send, len);
      }
      break;
    case RECORDPARAM_PROFESSION_QUERY:
      {
        PARSE_CMD_PROTOBUF(ProfessionQueryRecordCmd, rev);

        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if(!pUser)
        {
          XERR << "[Record消息-职业分支查询] charid :" << rev.charid() << "不在线" << XEND;
          return true;
        }

        pUser->m_oProfession.fixBranch(rev);
        return true;
      }
      break;
    case RECORDPARAM_CHEAT_TAG:
      {
        PARSE_CMD_PROTOBUF(CheatTagRecordCmd, rev);

        SceneUser* pUser = SceneUserManager::getMe().getUserByID(rev.charid());
        if(!pUser)
        {
          XERR << "[Record消息-嫌疑玩家标记] charid:" << rev.charid() << "不在线" << XEND;
          return true;
        }

        pUser->getCheatTag().assignData(rev);
        return true;
      }
    default:
      break;
  }
  return false;
}

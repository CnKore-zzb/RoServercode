#include "GMCommandManager.h"
#include "SessionUser.h"
#include "SessionUserManager.h"
#include "MsgManager.h"
#include "SessionServer.h"
#include "xTools.h"
#include "SessionCmd.pb.h"
#include "SessionScene.h"
#include "SessionSceneManager.h"
#include "RewardConfig.h"
#include "SocialCmd.pb.h"
#include "TableManager.h"
#include "PlatLogManager.h"
#include "UserEvent.pb.h"
#include "DepositConfig.h"
#include "RedisManager.h"
#include "CommonConfig.h"
#include "QuestConfig.h"
#include "GuildSCmd.pb.h"
#include "ConfigManager.h"
#include "GateSuper.pb.h"
#include "SceneTrade.pb.h"
#include "SessionActivityMgr.h"
#include "AuctionSCmd.pb.h"
#include "ActivityConfig.h"
#include "MailTemplateManager.h"

GMCommandManager::GMCommandManager():m_oTenSecTimer(3)
{
  m_vecResult.resize(2);
}

GMCommandManager::~GMCommandManager()
{
}

const TVecString& GMCommandManager::processGMCommand(const ExecGMCmd& cmd)
{
  XLOG<<"[GM-接收-充值] cmd"<<cmd.ShortDebugString()<<XEND;
  setJson(EJSONRET_MESSAGE, "success");
  setJson(EJSONRET_DATA, "");  
  Json::Reader reader;
  Json::Value root;
  if (reader.parse(cmd.data().c_str(), root) == false)
  {
    setJson(EJSONRET_MESSAGE, "json格式错误");
    return m_vecResult;
  }

  if (cmd.act() == exec_gm_cmd_player)
    return execGMCommand(root, cmd);
  else if (cmd.act() == send_notice)
    return sendNotice(root, cmd);
  else if (cmd.act() == send_mail)
    return sendMail(root);
  else if (cmd.act() == charge)
    return chargeMoney(root);
  else if (cmd.act() == load_config)
    return loadConfig(root);
  else if (cmd.act() == gag_player)
    return gagPlayer(root);
  else if (cmd.act() == lock_player)
    return lockPlayer(root);
  else if (cmd.act() == check_charge)
    return checkCharge(root);
  else if (cmd.act() == move_guild)
    return moveGuildZone(root);
  else if (cmd.act() == use_code)
    return useGiftCode(root);
  else if (cmd.act() == trade_security_cmd)
    return tradeSecurityCMD(root);
  else if (cmd.act() == modify_auction_time)
    return modifyAuctionTime(root);
  else if (cmd.act() == stop_auction)
    return stopAuction();
  else if (cmd.act() == lock_account)
    return lockAccount(root);
  else if (cmd.act() == verify_guildicon)
    return verifyGuildIcon(root);
  else if (cmd.act() == clear_mail_template)
    return clearMailTemplate(root, cmd);
  else
    setJson(EJSONRET_MESSAGE, "未知的操作");

  return m_vecResult;
}

const TVecString& GMCommandManager::execGMCommand(const Value& root, const ExecGMCmd& cmd)
{
  std::vector<std::string> sVec;
  std::string commandStr;
  std::string strId;
  
  try
  {
    strId = root["player_id"].asString();
    commandStr = root["command"].asString();
    stringTok(strId, ",", sVec);
    if (sVec.empty())
    {
      setJson(EJSONRET_MESSAGE, "参数解析错误");
      return m_vecResult;
    }
  }
  catch(...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  if (strId == "0")
  {  
    ExecGMCmdSessionCmd sessionCmd;
    sessionCmd.set_gmcmd(commandStr);

    PROTOBUF(sessionCmd, send, len);

    //forward to other line
    {
      ForwardToAllSessionSocialCmd cmd2;
      cmd2.set_data((const char*)send);
      cmd2.set_len(len);
      cmd2.set_except(thisServer->getZoneID());
      PROTOBUF(cmd2, send2, len2);
      thisServer->sendCmd(ClientType::global_server, send2, len2);
    }
    thisServer->sendCmdToAllScene(send, len);
    XLOG << "[全线-GM] 收到来自gmtools的消息，转发到scene 以及其他线" << sessionCmd.gmcmd() << XEND;
  }
  else
  {
    for (auto& s : sVec)
    {
      QWORD qwCharID = atoll(s.c_str());
      SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwCharID);
      if (pUser == nullptr)
      {
        setJson(EJSONRET_MESSAGE, "玩家不在线,修改已经放到离线信息里");
        appendData(EJSONRET_DATA, s);

        // send offline gm msg
        /*      ChatManager::getMe().addOfflineGmMsg(qwCharID, cmd.data());    */
        AddOfflineMsgSocialCmd msg;
        msg.mutable_msg()->set_type(EOFFLINEMSG_GM);
        msg.mutable_msg()->set_targetid(qwCharID);
        msg.mutable_msg()->set_gmcmd(cmd.data());
        PROTOBUF(msg, send, len);
        thisServer->doSocialCmd((const BYTE*)send, len);
        XLOG << "[GM-执行] 收到来自gmtools的消息，玩家不在线，存到离线消息" <<qwCharID<< commandStr << XEND;
        continue;
      }

      PROTOBUF(cmd, send, len);
      pUser->sendCmdToScene(send, len);
      XLOG << "[GM-执行] 收到来自gmtools的消息，转发到玩家场景" <<pUser->id << commandStr << XEND;
    }
  }  
  return m_vecResult;
}

const TVecString& GMCommandManager::sendNotice(const Value& root, const ExecGMCmd& cmd)
{
  string starttime;
  string str;
  DWORD dwId = 0;
  DWORD dwAction = 0;
  DWORD dwMsgID = 60;     //公告的id 固定为60
  DWORD dwInterval = 0;
  DWORD dwMaxCount = 0;
  DWORD dwNewId = 0;
  MsgParam langparam;

  try
  {
    dwAction = atoi(root["action"].asString().c_str());
    dwId = atoi(root["id"].asString().c_str());
    starttime = root["start_time"].asString();
    str = root["content"].asString();
    dwInterval = atoi(root["interval"].asString().c_str());
    dwMaxCount = atoi(root["count"].asString().c_str());

    const string& contentdata = root["contentdata"].asString();
    if (contentdata.empty() == false)
    {
      if (langparam.ParseFromString(contentdata) == false)
      {
        setJson(EJSONRET_MESSAGE, "contentdata参数解析错误");
        return m_vecResult;
      }
    }
  }
  catch(...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  dwNewId = MsgManager::addBroadInfo(dwAction, dwId, starttime, str, dwMsgID, dwInterval, dwMaxCount, langparam);
  if (dwAction == 0 || dwAction == 1) {
    if (dwNewId == 0) {
      setJson(EJSONRET_MESSAGE, "找不到相应id 的公告");
      return m_vecResult;
    }
  }
  else {
    appendData(EJSONRET_DATA, dwNewId);
  }
   
  {
    if (cmd.serverid() == thisServer->getZoneID())
    {
      //forward to other line
      ExecGMCmd newCmd = cmd;
      if (dwId == 0 && dwNewId)
      {
        stringstream ss;
        ss << dwNewId;
        Value newVal = root;
        newVal["id"] = ss.str();
        Json::FastWriter fw;
        std::string&& newJsonStr = fw.write(newVal);
        newCmd.set_data(newJsonStr);
      }      
      PROTOBUF(newCmd, send, len);

      ForwardToAllSessionSocialCmd cmd2;
      cmd2.set_data((const char*)send);
      cmd2.set_len(len);
      cmd2.set_except(thisServer->getZoneID());
      PROTOBUF(cmd2, send2, len2);
      thisServer->sendCmd(ClientType::global_server, send2, len2);
    }
  }

  return m_vecResult;
}

const TVecString& GMCommandManager::sendMail(const Value& root)
{
  std::vector<std::string> sVec;
  string strTitle;
  string strMsg;
  bool bAccMail = false;
  bool bItemData = false;
  bool bSysMail = false;

  TVecItemInfo vecItems;
  TVecItemData vecItemDatas;
  DWORD startTime = 0;
  DWORD endTime = 0;
  string jsonStr;
  try
  {
    std::string strId = root["player_id"].asString();
    stringTok(strId, ",", sVec);
    if (sVec.empty())
    {
      setJson(EJSONRET_MESSAGE, "参数解析错误");
      return m_vecResult;
    }

    bSysMail = atoi(root["sysmail"].asString().c_str()) != 0;

    strTitle = root["title"].asString();
    strMsg = root["msg"].asString();
    bAccMail = atoi(root["acc"].asString().c_str()) != 0;

    startTime = atoi(root["starttime"].asString().c_str());
    endTime = atoi(root["endtime"].asString().c_str());
    if (endTime != 0)
    {
      setJson(EJSONRET_MESSAGE, "endtime暂时不允许设置");
      return m_vecResult;
    }
    if (startTime > endTime)
      startTime = endTime;
    
    bItemData = atoi(root["itemdata"].asString().c_str()) != 0;
    jsonStr = root["json"].asString();
    if (!jsonStr.empty()) 
      bItemData = true;
    if (!bItemData)
    {
      ItemInfo oItem;
      oItem.set_source(ESOURCE_GM);

      oItem.set_id(atoi(root["item1"].asString().c_str()));
      oItem.set_count(atoi(root["count1"].asString().c_str()));
      if (oItem.id() != 0)
        vecItems.push_back(oItem);
      oItem.set_id(atoi(root["item2"].asString().c_str()));
      oItem.set_count(atoi(root["count2"].asString().c_str()));
      if (oItem.id() != 0)
        vecItems.push_back(oItem);
      oItem.set_id(atoi(root["item3"].asString().c_str()));
      oItem.set_count(atoi(root["count3"].asString().c_str()));
      if (oItem.id() != 0)
        vecItems.push_back(oItem);
      oItem.set_id(atoi(root["item4"].asString().c_str()));
      oItem.set_count(atoi(root["count4"].asString().c_str()));
      if (oItem.id() != 0)
        vecItems.push_back(oItem);
    }
    else
    {     
      //base
      ItemData oItemData;
      if (jsonStr.empty())
      {
        ItemInfo* pItemInfo = oItemData.mutable_base();
        pItemInfo->set_id(atoi(root["item1"].asString().c_str()));
        pItemInfo->set_count(atoi(root["count1"].asString().c_str()));
        pItemInfo->set_source(ESOURCE_GM);
        const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(pItemInfo->id());
        if (pItemCfg == nullptr)
        {
          setJson(EJSONRET_MESSAGE, "错误的道具id");
          return m_vecResult;
        }

        //equip
        DWORD refineLv = atoi(root["refinelv"].asString().c_str());
        if (refineLv > 15)
        {
          setJson(EJSONRET_MESSAGE, "精炼等级超过15级");
          return m_vecResult;
        }

        DWORD strengthLv = atoi(root["strengthlv"].asString().c_str());
        bool damage = atoi(root["damage"].asString().c_str());
        DWORD upgradeLv = atoi(root["upgradelv"].asString().c_str());
        if (upgradeLv > 6)
          upgradeLv = 6;
        if (refineLv || strengthLv || damage || upgradeLv)
        {
          EquipData* pEquipData = oItemData.mutable_equip();
          if (pEquipData)
          {
            pEquipData->set_strengthlv(strengthLv);
            pEquipData->set_refinelv(refineLv);
            pEquipData->set_damage(damage);
            pEquipData->set_lv(upgradeLv);
          }
        }

        //card
        auto checkCardf = [&](DWORD cardId)
        {
          const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(cardId);
          if (pItemCfg == nullptr)
          {
            XERR << "[GM-邮件] cardid" << cardId << "道具表读取不到配置" << XEND;
            return false;
          }
          if (ItemConfig::getMe().isCard(pItemCfg->eItemType) == false)
          {
            XERR << "[GM-邮件] cardid" << cardId << "不是卡片" << XEND;
            return false;
          }
          oItemData.add_card()->set_id(cardId);
          return true;
        };

        DWORD cardId1 = atoi(root["card1"].asString().c_str());
        DWORD cardId2 = atoi(root["card2"].asString().c_str());
        DWORD cardId3 = atoi(root["card3"].asString().c_str());
        DWORD size = 0;
        if (cardId1 && !checkCardf(cardId1))
        {
          size++;
          setJson(EJSONRET_MESSAGE, "卡片不合法");
          return m_vecResult;
        }
        if (cardId2 && !checkCardf(cardId2))
        {
          size++;
          setJson(EJSONRET_MESSAGE, "卡片不合法");
          return m_vecResult;
        }
        if (cardId3 && !checkCardf(cardId3))
        {
          size++;
          setJson(EJSONRET_MESSAGE, "卡片不合法");
          return m_vecResult;
        }

        if (size > pItemCfg->dwCardSlot)
        {
          XERR << "[GM-邮件] itemid" << pItemInfo->id() << "洞数不够, maxslot" << pItemCfg->dwCardSlot << "size" << size << XEND;
          setJson(EJSONRET_MESSAGE, "卡片洞数不合法");
          return m_vecResult;
        }

        //enchant
        DWORD enchantType = atoi(root["enchant_type"].asString().c_str());
        if (enchantType)
        {
          if (pItemCfg->eEquipType == EEQUIPTYPE_MIN)
          {
            setJson(EJSONRET_MESSAGE, "不是装备不可附魔");
            XERR << "[GM-邮件] 不是装备不可附魔 itemid" << pItemInfo->id() << "eEquipType" << pItemCfg->eEquipType << XEND;
            return m_vecResult;
          }
          if (EEnchantType_IsValid(enchantType) == false)
          {
            setJson(EJSONRET_MESSAGE, "非法的附魔类型");
            return m_vecResult;
          }
          EnchantData enchantData;
          EEnchantType eType = static_cast<EEnchantType>(enchantType);
          const SEnchantCFG* pCfg = ItemConfig::getMe().getEnchantCFG(eType);
          if (pCfg == nullptr)
          {
            setJson(EJSONRET_MESSAGE, "错误的附魔类型");
            XERR << "[GM-邮件] 错误的附魔类型 itemid" << pItemInfo->id() << "eEquipType" << pItemCfg->eEquipType << "enchanttype" << eType << XEND;
            return m_vecResult;
          }

          enchantData.set_type(eType);

          auto enchantF = [&](string attrStr, string valueStr)
          {
            DWORD dwAttr = atoi(root[attrStr].asString().c_str());
            float fValue = atof(root[valueStr].asString().c_str());
            if (dwAttr == 0 && fValue == 0)
              return true;

            EnchantAttr* pAttr = enchantData.add_attrs();
            if (pAttr == nullptr)
            {
              XERR << "[GM-邮件-添加属性装备] 创建attr protobuf失败" << XEND;
              return false;
            }
            const RoleData* pData = RoleDataConfig::getMe().getRoleData(dwAttr);
            if (pData == nullptr)
            {
              XERR << "[GM-邮件-添加属性装备]" << "失败 attr :" << dwAttr << "不合法" << XEND;
              return false;
            }
            pAttr->set_type(static_cast<EAttrType>(dwAttr));
            pAttr->set_value(pData->bPercent ? fValue * FLOAT_TO_DWORD : fValue);
            return true;
          };

          if (enchantF("attr1", "value1") == false)
          {
            setJson(EJSONRET_MESSAGE, "非法的附魔属性");
            return m_vecResult;
          }

          if (enchantF("attr2", "value2") == false)
          {
            setJson(EJSONRET_MESSAGE, "非法的附魔属性");
            return m_vecResult;
          }

          if (enchantF("attr3", "value3") == false)
          {
            setJson(EJSONRET_MESSAGE, "非法的附魔属性");
            return m_vecResult;
          }

          DWORD dwEnchantBuf = atoi(root["buffid"].asString().c_str());
          if (dwEnchantBuf)
          {
            if (!pCfg->gmCheckAndSet(pItemCfg->eItemType, enchantData, dwEnchantBuf))
            {
              setJson(EJSONRET_MESSAGE, "错误的附魔buffid");
              XERR << "[GM-邮件] 错误的附魔buffid itemid" << pItemInfo->id() << "eEquipType" << pItemCfg->eEquipType << "enchanttype" << eType << "buffid" << dwEnchantBuf << XEND;
              return m_vecResult;
            }
          }

          oItemData.mutable_enchant()->CopyFrom(enchantData);
        }

        if (ItemConfig::getMe().isEquip(pItemCfg->eItemType))
        {
          EquipData* pEquipData = oItemData.mutable_equip();
          if (pEquipData)
          {
            pEquipData->set_cardslot(pItemCfg->dwCardSlot);
          }
        }

        jsonStr = pb2json(oItemData);
        
        XDBG << "[GM-邮件-属性装备] jsonstr:" << jsonStr << XEND;
      }
      else
      {
        json2pb(oItemData, jsonStr.c_str(), jsonStr.size());
      }
      XLOG << "[GM-邮件-属性装备] receid:"<< strId <<"itemdata:" << oItemData.ShortDebugString() << XEND;
      vecItemDatas.push_back(oItemData);
    }
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  if (bSysMail == false)
  {
    for (auto& s : sVec)
    {
      QWORD targetId = atoll(s.c_str());
      if (targetId == 0)
      {
        XERR << "[GM-邮件] 个人邮件id不可为0 " << targetId << XEND;
        setJson(EJSONRET_MESSAGE, "个人邮件id不可为0 ");
        appendData(EJSONRET_DATA, s);
        continue;
      }
      EMailType eType = EMAILTYPE_NORMAL;
      if (MailManager::getMe().sendMail(targetId, 0, "GM发送", strTitle, strMsg, eType, 0, vecItems, vecItemDatas, EMAILATTACHTYPE_ITEM, bAccMail, MsgParams(), startTime, endTime) == false)
      {
        setJson(EJSONRET_MESSAGE, "发送失败");
        appendData(EJSONRET_DATA, s);
        continue;
      }
    }
  }
  else
  {
    EMailType eType = EMAILTYPE_SYSTEM;
    if (MailManager::getMe().sendMail(0, 0, "GM发送", strTitle, strMsg, eType, 0, vecItems, vecItemDatas, EMAILATTACHTYPE_ITEM, bAccMail, MsgParams(), startTime, endTime) == false)
    {
      setJson(EJSONRET_MESSAGE, "系统邮件发送失败");
    }
  }
  return m_vecResult;
}

const TVecString& GMCommandManager::chargeMoney(const Value& root)
{
  QWORD qwTargetID = 0;
  float fCharge = 0;
  DWORD dwId = 0;
  std::string strSource;
  std::string orderid;
  std::string chargeType;
  std::string productId;
  DWORD dwChargeCount = 0;    //淘宝 充值次数
  bool bFromWeb = false;
  bool errorHuanSuan = false; //异常情况，以 fCharge*10000 发放zeny
  try
  {
    //  OrderID   string `json:"order_id"`
    //    CharID    string `json:"player_id"`
    //    Charge    string `json:"charge"`     //amount
    //    ChargeType string `json:"charge_type"` //支付方式
    //    ProductID string `json:"product_id"` //
    //    DataID    string `json:"data_id"`    //策划表id
    //    Source    string `json:"source"`     //来源 web，anysdk

    orderid = root["order_id"].asString();
    qwTargetID = atoll(root["player_id"].asString().c_str());
    fCharge = atof(root["charge"].asString().c_str());
    dwId = atoi(root["data_id"].asString().c_str());
    strSource = root["source"].asString();
    chargeType = root["charge_type"].asString();
    productId = root["product_id"].asString();
    dwChargeCount = atoi(root["count"].asString().c_str());
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "接口参数解析错误");
    return m_vecResult;
  }

  const SDeposit* pCFG = nullptr;
  
  DWORD dwCfgMoneyYuan = 0;
  if (strSource == "ios" || strSource == "newanysdk") {
    //以productid 为准
    pCFG = DepositConfig::getMe().getSDeposit(productId);
    if (pCFG == nullptr)
    {
      XERR << "[GM-充值] order id" << orderid << "charge" << fCharge << "charid" << qwTargetID <<"productid:"<<productId << "data_id" << dwId << "source" << strSource << " 找不到配置" << XEND;
      setJson(EJSONRET_MESSAGE, "策划表找不到充值id的配置");
      return m_vecResult;
    }

    dwCfgMoneyYuan = (DWORD)(pCFG->rmb);

    if (chargeType == "alipay" && strSource == "ios")
    {//check gold
      if (dwCfgMoneyYuan != (DWORD)(fCharge))
      {
        XERR << "[GM-充值] order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "充值的金钱和策划表不对应" << "charge" << fCharge << "need money" << pCFG->rmb << XEND;
        setJson(EJSONRET_MESSAGE, "充值的金钱和策划表不对应");
        return m_vecResult;
      }
    }
    else if (chargeType == "appstore")
    {//check data id
      fCharge = pCFG->rmb;
    }
    else if (chargeType == "newanysdk" && strSource == "newanysdk")
    {//B.如果payment字段为其它值，需要验证gold字段和 product_id 字段是否相符，如果相符，按照product_id发放道具，如果不相符，直接按照gold字段折算成对应的游戏货币发放。
      if (dwCfgMoneyYuan != (DWORD)(fCharge))
      {
        XERR << "[GM-充值] order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "充值的金钱和策划表不对应" << "charge" << fCharge << "need money" << pCFG->rmb << "以充值的钱换算为zeny发给玩家" << XEND;
        errorHuanSuan = true;
      }
    }
  }
  else
  { 
    if (strSource == "web")
    {
      bFromWeb = true;
      //dataid
      pCFG = DepositConfig::getMe().getSDeposit(dwId);
      if (pCFG == nullptr)
      {
        XERR << "[GM-充值] order id" << orderid << "charge" << fCharge << "charid" << qwTargetID << "productid:" << productId << "data_id" << dwId << "source" << strSource << " 找不到配置" << XEND;
        setJson(EJSONRET_MESSAGE, "策划表找不到充值id的配置");
        return m_vecResult;
      }
    }
    else //anysdk + taobao
    {
      //productid
      pCFG = DepositConfig::getMe().getSDeposit(productId);
      if (pCFG == nullptr)
      {
        XERR << "[GM-充值] order id" << orderid << "charge" << fCharge << "charid" << qwTargetID << "productid:" << productId << "data_id" << dwId << "source" << strSource << " 找不到配置" << XEND;
        setJson(EJSONRET_MESSAGE, "策划表找不到充值id的配置");
        return m_vecResult;
      }
    }
    dwCfgMoneyYuan = (DWORD)(pCFG->rmb);
    if (dwCfgMoneyYuan != (DWORD)(fCharge))
    {
      XERR << "[GM-充值] order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "充值的金钱和策划表不对应" << "充值了" << fCharge << "策划表money" << pCFG->rmb << XEND;

      if (strSource == "anysdk") //只有anysdk才需要转换
        errorHuanSuan = true;
    }
  }

  //check orderid
  {
    char where[512];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "orderid='%s'", orderid.c_str());
    QWORD num = thisServer->getDBConnPool().checkExist(REGION_DB, "charge", where);
    if (num != 0)
    {
      XERR << "[充值-订单验重] 失败，重复订单 order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "charge" << fCharge << XEND;
      //setJson(EJSONRET_MESSAGE, "重复订单");
      //return success
      return m_vecResult;
    }
  }

  SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwTargetID);
  QWORD accid = 0;
  if (!pUser)
  {
    GCharReader gCharReader(thisServer->getRegionID(), qwTargetID);
    if (!gCharReader.get())
    {
      XERR << "[GM-充值] order id" << orderid << "charge" << fCharge << "charid" << qwTargetID << "productid:" << productId << "data_id" << dwId << "source" << strSource << " redis里找不到玩家" << XEND;
      setJson(EJSONRET_MESSAGE, "找不到玩家账号");
      return m_vecResult;
    }
    accid = gCharReader.getAccID();
  }
  else
    accid = pUser->accid;
  
  ////月卡购买限制
  //if (pCFG->monthLimit)
  //{
  //  DWORD buyCount = 0;
  //  DWORD ym = 0;
  //  if (canBuyMonthCard(accid, qwTargetID, pCFG, buyCount, ym, bFromWeb) == false)
  //  {
  //    //waring 返回内容gmtools 用到，不可修改
  //    setJson(EJSONRET_MESSAGE, "月卡购买检测不通过");
  //    XERR << "[充值-购买条件] 失败，月卡购买检测不通过  order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "charge" << fCharge << key << buyCount << XEND;
  //    return m_vecResult;
  //  }
  //  buyCount = buyCount + 1;
  //  if (updateMontCardDb(accid, qwTargetID, pCFG, ym, buyCount) == false)
  //  {
  //    setJson(EJSONRET_MESSAGE, "月卡购买插入数据库错误");
  //    return m_vecResult;
  //  }
  //}


  DWORD buyCount = 0;
  DWORD ym = 0;
  const SGlobalActCFG* pGlobalActCfg = nullptr;
  bool bUpdateLimit = false;
  if (canBuy(accid, qwTargetID, pCFG, buyCount, ym, bFromWeb, &pGlobalActCfg, bUpdateLimit) == false)
  {
    //waring 返回内容gmtools 用到，不可修改
    setJson(EJSONRET_MESSAGE, "月卡购买检测不通过");
    XERR << "[充值-购买条件] 失败，月卡购买检测不通过  order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "charge" << fCharge << buyCount << XEND;
    return m_vecResult;
  }
  if (pGlobalActCfg)
  {
    //活动次数加1
    if (SessionActivityMgr::getMe().addUserActivityCnt(pGlobalActCfg->m_dwId, accid, qwTargetID) == false)
    {
      setJson(EJSONRET_MESSAGE, "活动插入数据库错误");
      return m_vecResult;
    }
  }
  else if (bUpdateLimit)
  {
    buyCount = buyCount + 1;
    if (updateBuyLimitDb(accid, qwTargetID, pCFG, ym, buyCount) == false)
    {
      setJson(EJSONRET_MESSAGE, "月卡购买插入数据库错误");
      return m_vecResult;
    }
  }

  //插入订单id
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charge");
  if (field)
  {
    xRecord record(field);
    record.put("orderid", orderid); // orderid is index key
    QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, false);
    if (ret == QWORD_MAX)
    {
      XERR << "[充值-订单验重] 不应该存在的日志，失败，重复订单 order id" << orderid << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "charge" << fCharge << XEND;
      //setJson(EJSONRET_MESSAGE, "重复订单");
      //return success
      return m_vecResult;
    }
  }

  //检测首充
  bool bVirgin = false;
  if(pCFG->virginTag)
  {
    xField *fieldVirgin = thisServer->getDBConnPool().getField(REGION_DB, "charge_virgin");
    if (fieldVirgin)
    {
      char where[64] = { 0 };
      snprintf(where, sizeof(where), "accid=%llu", accid);
      xRecordSet set;
      QWORD ret = thisServer->getDBConnPool().exeSelect(fieldVirgin, set, where, NULL);
      if (QWORD_MAX == ret)
      {
        setJson(EJSONRET_MESSAGE, "select charge_virgin表错误");
        return m_vecResult;
      }
      else if(0 == ret)
      {
        //第一次充值
        bVirgin = true;
        xRecord record(fieldVirgin);
        record.put("accid", accid);
        record.putString("tag", std::to_string(pCFG->virginTag));
        QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, false);
        if(QWORD_MAX == ret)
        {
          XERR << "[充值-首充]插入表charge_virgin失败, accid" << accid << XEND;
          setJson(EJSONRET_MESSAGE, "insert charge_virgin表错误");
          return m_vecResult;
        }
      }
      else
      {
        TVecDWORD vec;
        std::string strTag = set[0].getString("tag");
        numTok(strTag, ",", vec);
        auto iter = std::find(vec.begin(), vec.end(), pCFG->virginTag);
        if(vec.end() == iter)
        {
          bVirgin = true;
          stringstream ss;
          ss.str("");
          for(auto &v : vec)
            ss << v << ",";
          ss << pCFG->virginTag;
          xRecord record(fieldVirgin);
          record.putString("tag", ss.str());
          QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
          if(QWORD_MAX == ret || 0 == ret)
          {
            XERR << "[充值-首充]更新表charge_virgin失败, accid" << accid << "strTag" << strTag << "virginTag" << pCFG->virginTag << XEND;
            setJson(EJSONRET_MESSAGE, "update charge_virgin表错误");
            return m_vecResult;
          }
        }
      }
    }
  }

  // 通知客户端首充成功
  if(bVirgin && pUser)
  {
    QueryChargeVirginCmd cmd3;
    cmd3.set_del(pCFG->id);

    PROTOBUF(cmd3, send3, len3);
    pUser->sendCmdToMe(send3, len3);
  }

  //ok  
  DWORD itemId = 0;
  DWORD count = 0;
  //wanring 如果是送审的配置，直接发放zeny币
  if (pCFG->count3)
  {
    itemId = 100;
    count = pCFG->count3;
  }
  else
  {
    itemId = pCFG->getItemId();
    count = pCFG->getTotalCount(bVirgin);
  }

  //
  if (strSource == "taobao")
  {
    count = count * dwChargeCount;
  }
    //异常
  DWORD dwRate = 1;
  if (errorHuanSuan)
  {
    itemId = 100; //zeny
    count = fCharge * 10000;
  }
  else if (pGlobalActCfg)
  {
    //获取活动效果
    SessionActivityMgr::getMe().getActivityEffect(pGlobalActCfg, dwRate);
    if (dwRate) //翻倍活动
    {
      count *= dwRate;
    }
  }

  dwId = pCFG->id;
  ChargeSessionCmd cmd;
  cmd.set_charid(qwTargetID);
  cmd.set_charge(fCharge);
  cmd.set_itemid(itemId);
  cmd.set_count(count);
  cmd.set_source(strSource);
  cmd.set_orderid(orderid);
  cmd.set_dataid(dwId);
  
  QWORD qwQuota = 0;
  if (dwChargeCount)
    qwQuota = pCFG->quota * dwChargeCount;
  else
    qwQuota = pCFG->quota;
  if (errorHuanSuan)
    qwQuota = fCharge * 10000;

  if (MailManager::getMe().sendChargeMail(cmd, qwQuota, bVirgin) == false)
  {
    XERR << "[GM-充值], 离线邮件发送失败 order id" << orderid << "charge" << fCharge << "charid" << qwTargetID << "data_id" << dwId << "source" << strSource << "itemid" << itemId << "count" << cmd.count() << XEND;
    setJson(EJSONRET_MESSAGE, "离线邮件发送失败");
    return m_vecResult;
  }
  //充值成功

  //log 
  auto logFunc = [&](DWORD accid, DWORD level, std::string name)
  {
    PlatLogManager::getMe().chargeLog(thisServer,
      0,
      thisServer->getZoneID(),
      accid,
      cmd.charid(),
      level,
      cmd.charge(),
      name,
      0, /*createtime*/
      itemId,
      cmd.count(), /*count*/
      cmd.orderid()/*order id*/,
      strSource/*type*/,
      "rmb"/*currency*/,
      ""/*provider*/,
      "" /*ip*/,
      "" /*device*/
    );
    {
      pushtyrantdb(accid, cmd.charid(), orderid, fCharge * 100, count, pCFG->productId, chargeType);
    }
  };
    
  if (pUser == nullptr)
  {
    logFunc(accid, 0, "");
    return m_vecResult;
  }
  
  logFunc(pUser->accid, pUser->getBaseLevel(), pUser->name);

  //ntf client 
  ChargeNtfUserEvent ntf;
  ntf.set_charid(cmd.charid());
  ntf.set_dataid(dwId);
  PROTOBUF(ntf, send2, len2);
  bool sr = pUser->sendCmdToMe(send2, len2);
  XLOG << "[充值-成功] 充值成功，返回消息给客户端，charid" << cmd.charid() << "id" << dwId <<"ret" <<sr <<"使用活动id"<<(pGlobalActCfg ==nullptr?0:pGlobalActCfg->m_dwId) <<"翻倍"<<dwRate << XEND;

  return m_vecResult;
}

const TVecString& GMCommandManager::loadConfig(const Value& root)
{
  string strCFGName = "";
  string strLua;
  EComLoadType comLoadType = EComLoadType_None;
  try
  {
    strCFGName = root["cfgname"].asString();
    strLua = root["cfglua"].asString();
    DWORD t = atoi(root["commonlua"].asString().c_str());
    if (EComLoadType_IsValid(t))
    {
      comLoadType = static_cast<EComLoadType>(t);
    }
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  LoadLuaSessionCmd cmd;
  if (!strCFGName.empty())
    cmd.set_table(strCFGName);
  if (strLua.empty() == false)
    cmd.set_lua(strLua);
  if (comLoadType != EComLoadType_None)
    cmd.set_load_type(comLoadType);
  cmd.set_serverid(thisServer->getZoneID());
  cmd.set_allzone(true);
  PROTOBUF(cmd, send, len);

  //forward to other line
  {
    ForwardToAllSessionSocialCmd cmd2;
    cmd2.set_data((const char*)send);
    cmd2.set_len(len);
    cmd2.set_except(thisServer->getZoneID());
    PROTOBUF(cmd2, send2, len2);
    thisServer->sendCmd(ClientType::global_server, send2, len2);
  }

  if (thisServer->doSessionCmd((BYTE*)send, len) == false)
    setJson(EJSONRET_MESSAGE, "失败");

  return m_vecResult;
}

const TVecString& GMCommandManager::gagPlayer(const Value& root)
{
  TSetQWORD setPlayerIDs;
  DWORD dwGagTime = 0;
  string reason;
  try
  {
    TVecString vecPlayerStrIDs;
    std::string strId = root["player_id"].asString();
    stringTok(strId, ",", vecPlayerStrIDs);
    if (vecPlayerStrIDs.empty())
    {
      setJson(EJSONRET_MESSAGE, "参数解析错误");
      return m_vecResult;
    }
    for (auto &v : vecPlayerStrIDs)
      setPlayerIDs.insert(atoll(v.c_str()));
    dwGagTime = now() + atol(root["block_time"].asString().c_str());

    reason = root["reason"].asString();
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  auto func = [&](QWORD qwPlayerID) -> bool
  {
    if (qwPlayerID == 0)
      return false;

    SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwPlayerID);
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (field == nullptr)
    {
      XERR << "[GM-禁言] 玩家id为0" << qwPlayerID << dwGagTime << XEND;
      return false;
    }
    xRecord record(field);
    record.put("gagtime", dwGagTime);
    //record.put("gagreason", reason);
    char where[256] = { 0 };
    snprintf(where, sizeof(where), "charid=%llu", qwPlayerID);

    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX || ret == 0)
    {
      XERR << "[GM-禁言] 数据库更新失败，玩家id" << qwPlayerID << dwGagTime << XEND;
      return false;
    }
    GagSessionCmd cmd;
    cmd.set_charid(qwPlayerID);
    cmd.set_time(dwGagTime);
    cmd.set_reason(reason);
    PROTOBUF(cmd, send, len);
    if (pUser == nullptr)
      thisServer->sendCmdToWorldUser(qwPlayerID, send, len, EDir_ToScene);
    else
      pUser->sendCmdToScene(send, len);

    XLOG << "[GM控制-禁言] " << qwPlayerID << ", 被设置了禁言时间 " << dwGagTime << XEND;
    return true;
  };

  for (auto &s : setPlayerIDs)
  {
    if (func(s) == false)
    {
      setJson(EJSONRET_MESSAGE, "禁言失败");
      appendData(EJSONRET_DATA, s);
    }
  }

  return m_vecResult;
}

const TVecString& GMCommandManager::lockPlayer(const Value& root)
{
  TSetQWORD setPlayerIDs;
  DWORD dwNologinTime = 0;
  string reason;

  try
  {
    TVecString vecPlayerStrIDs;
    std::string strId = root["player_id"].asString();
    stringTok(strId, ",", vecPlayerStrIDs);
    if (vecPlayerStrIDs.empty())
    {
      setJson(EJSONRET_MESSAGE, "参数解析错误");
      return m_vecResult;
    }
    for (auto &v : vecPlayerStrIDs)
      setPlayerIDs.insert(atoll(v.c_str()));
    dwNologinTime = now() + atol(root["block_time"].asString().c_str());

    reason = root["reason"].asString();
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  auto func = [&](QWORD qwPlayerID) -> bool
  {
    if (qwPlayerID == 0)
    {
      XERR << "[GM-封角色] 玩家id为0" << XEND;
      return false;
    }

    SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwPlayerID);
    //modif db
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (field == nullptr)
    {
      XERR << "[GM-封角色] 数据库错误，玩家id" <<qwPlayerID << XEND;
      return false;
    }
    xRecord record(field);
    record.put("nologintime", dwNologinTime);

    char where[256] = { 0 };
    snprintf(where, sizeof(where), "charid=%llu", qwPlayerID);

    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX || ret == 0)
    {
      XERR << "[GM-封角色] 数据库修改失败，玩家id" << qwPlayerID << XEND;
      return false;
    }
    LockSessionCmd cmd;
    cmd.set_charid(qwPlayerID);
    cmd.set_time(dwNologinTime);
    cmd.set_reason(reason);
    PROTOBUF(cmd, send, len);
    if (pUser == nullptr)
      thisServer->sendCmdToWorldUser(qwPlayerID, send, len, EDir_ToScene);
    else
      pUser->sendCmdToScene(send, len);

    XLOG << "[GM控制-封号] " << qwPlayerID << ", 被设置了封号时间 " << dwNologinTime << XEND;
    return true;
  };

  for (auto &s : setPlayerIDs)
  {
    if (func(s) == false)
    {
      setJson(EJSONRET_MESSAGE, "封号失败");
      appendData(EJSONRET_DATA, s);
    }
  }

  return m_vecResult;
}

const TVecString& GMCommandManager::lockAccount(const Value& root)
{
  QWORD qwCharId = 0;
  QWORD qwAccid = 0;
  DWORD dwNologinTime = 0;

  try
  {
    qwCharId = atol(root["player_id"].asString().c_str());
    qwAccid = atol(root["account"].asString().c_str());
    dwNologinTime = now() + atol(root["block_time"].asString().c_str());
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  auto func = [&]() -> bool
  {
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
    if (field == nullptr)
      return false;

    xRecord record(field);
    record.put("nologintime", dwNologinTime);

    char where[256] = { 0 };
    snprintf(where, sizeof(where), "accid=%llu", qwAccid);
    QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
    if (ret == QWORD_MAX || ret == 0)
      return false;

    SessionUser* pUser = SessionUserManager::getMe().getUserByID(qwCharId);
    if (pUser)
    {
      LockSessionCmd cmd;
      cmd.set_charid(qwCharId);
      cmd.set_time(dwNologinTime);
      cmd.set_account(true);
      PROTOBUF(cmd, send, len);
      pUser->sendCmdToScene(send, len);
    }

    XLOG << "[GM控制-账号封号] " <<"accid" <<qwAccid <<"charid" << qwCharId << ", 被设置了封号时间 " << dwNologinTime << XEND;
    return true;
  };

  if (func() == false)
  {
    setJson(EJSONRET_MESSAGE, "账号封号失败");
    return m_vecResult;
  }
  return m_vecResult;
}

const TVecString& GMCommandManager::checkCharge(const Value& root)
{
  QWORD charId = 0;
  DWORD dataId = 0;

  try
  {
    charId = atol(root["player_id"].asString().c_str());
    dataId = atol(root["data_id"].asString().c_str());
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  const SDeposit* pCFG = DepositConfig::getMe().getSDeposit(dataId);
  if (pCFG == nullptr)
  {
    XLOG << "[充值-检测] 策划表找不到充值id的配置" << charId << ", dataId " << dataId<< XEND;
    setJson(EJSONRET_MESSAGE, "策划表找不到充值id的配置");
    return m_vecResult;
  }

  DWORD count = 0;
  DWORD ym = 0;
  GCharReader gCharReader(thisServer->getRegionID(), charId);
  if (!gCharReader.get())
  {
    XERR << "[充值-检测] redis里找不到玩家" << charId << ", dataId " << dataId << XEND; 
    setJson(EJSONRET_MESSAGE, "找不到玩家账号");
    return m_vecResult;
  }
  QWORD accid = gCharReader.getAccID();
  const SGlobalActCFG* pGlobalActCfg = nullptr;
  bool bUpdateLimit = false;
  if (canBuy(accid, charId, pCFG, count, ym, true, &pGlobalActCfg, bUpdateLimit) == false)
  {
    XLOG << "[充值-检测] 不可充值" << charId << ", dataId " << dataId <<  count << XEND;
    setJson(EJSONRET_MESSAGE, "不可充值");
    return m_vecResult;
  }

  XLOG << "[充值-检测] 可充值" << charId << ", dataId " << count << XEND;  
  return m_vecResult;
}

const TVecString& GMCommandManager::moveGuildZone(const Value& root)
{
  DWORD dwOriZoneID = 0;
  DWORD dwNewZoneID = 0;

  try
  {
    dwOriZoneID = atol(root["ori_zoneid"].asString().c_str());
    dwNewZoneID = atol(root["new_zoneid"].asString().c_str());

    if (dwOriZoneID == 0 || dwNewZoneID == 0 || dwOriZoneID == dwNewZoneID)
    {
      setJson(EJSONRET_MESSAGE, "zoneid不合法");
      return m_vecResult;
    }
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  MoveGuildZoneSocialCmd cmd;
  cmd.set_orizone(dwOriZoneID);
  cmd.set_newzone(dwNewZoneID);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmd(ClientType::guild_server, send, len);

  setJson(EJSONRET_MESSAGE, "成功发送至GuildServer进行处理");
  return m_vecResult;
}

const TVecString& GMCommandManager::useGiftCode(const Value& root)
{
  string code;
  string name;
  QWORD id = 0;
  QWORD accid = 0;

  try
  {
    code = root["code"].asString();
    id = atoll(root["player_id"].asString().c_str());

    if (id == 0 || code.empty() == true)
    {
      setJson(EJSONRET_MESSAGE, "参数不合法");
      return m_vecResult;
    }
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  //toupper
  std::transform(code.begin(), code.end(), code.begin(), (int(*)(int))toupper);

  stringstream sstr;
  if (accid == 0)
  {
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (!field)
    {
      sstr << "[礼包码] 查找玩家accid 失败,数据库找不到" << id;
      XERR << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
    field->setValid("accid, name");

    char where[64] = { 0 };
    snprintf(where, sizeof(where), "charid=%llu", id);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
    if (1 != ret)
    {
      sstr << "[礼包码] 查找玩家accid 失败。";
      XERR << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
    else
    {
      accid = set[0].get<QWORD>("accid");
      name = set[0].getString("name");
    }
  }

  //check code
  //select t2.code, t1.id,t1.type,t1.regionids,t1.zoneids, t1.items, t1.expiretime  from gift_batch_list as t1,gift_code as t2 where t1.id = t2.batchid and code="XDROTAU4GS84";
  xField field(RO_DATABASE_NAME, "");
  field.setValid("t2.code, t1.id,t1.groupid,t1.type,t1.maxcount,t1.regionids,t1.zoneids,t1.channelids, t1.items, t1.expiretime, t1.mail_title, t1.mail_msg");
  field.m_list["code"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["id"] = MYSQL_TYPE_LONG;
  field.m_list["groupid"] = MYSQL_TYPE_LONG;
  field.m_list["type"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["maxcount"] = MYSQL_TYPE_NEWDECIMAL;
  field.m_list["regionids"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["zoneids"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["channelids"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["items"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["expiretime"] = MYSQL_TYPE_LONG;
  field.m_list["mail_title"] = MYSQL_TYPE_VAR_STRING;
  field.m_list["mail_msg"] = MYSQL_TYPE_VAR_STRING;

  char sql[1024];
  bzero(sql, sizeof(sql));

  snprintf(sql, sizeof(sql), "select t2.code, t1.id,t1.groupid,t1.type,t1.maxcount,t1.regionids,t1.zoneids,t1.channelids, t1.items, t1.expiretime, t1.mail_title, t1.mail_msg from %s.gift_batch_list as t1,%s.gift_code as t2 where t1.id = t2.batchid and code=\"%s\"", field.m_strDatabase.c_str(),
    field.m_strDatabase.c_str(), code.c_str());

  std::string strSql(sql);
  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeRawSelect(&field, set, strSql);
  if (ret == QWORD_MAX)
  {
    //MsgManager::sendMsg(id, 1061);
    sstr << "[礼包码] 数据库异常";
    setJson(EJSONRET_MESSAGE, sstr.str());
    return m_vecResult;
  }
  if (ret == 0)
  {
    //MsgManager::sendMsg(id, 1058);
    sstr << "[礼包码] 对应码不存在";
    setJson(EJSONRET_MESSAGE, sstr.str());
    return m_vecResult;
  }
  DWORD batchId = set[0].get<DWORD>("id");
  QWORD groupId = set[0].get<QWORD>("groupid");
  DWORD maxCount = set[0].get<DWORD>("maxcount");
  DWORD type = set[0].get<DWORD>("type");
  DWORD expireTime = set[0].get<DWORD>("expiretime");
  std::string items = set[0].getString("items");
  std::string mailTitle = set[0].getString("mail_title");
  std::string mailMsg = set[0].getString("mail_msg");
  std::string channels = set[0].getString("channelids");
  DWORD curSec = now();
  if (curSec > expireTime)
  {
    //MsgManager::sendMsg(id, 1057);
    sstr << "[礼包码] 已经过期";
    setJson(EJSONRET_MESSAGE, sstr.str());
    return m_vecResult;
  }

  std::string regionIds = set[0].getString("regionids");

  if (!regionIds.empty() && regionIds != "0")
  {
    std::vector<DWORD> vecReg;
    numTok(regionIds, ",", vecReg);
    auto it = std::find(vecReg.begin(), vecReg.end(), thisServer->getRegionID());
    if (it == vecReg.end())
    {
      //MsgManager::sendMsg(id, 1050);
      sstr << "[礼包码] 超出服使用范围";
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
  }

  //check channel TODO
  if (!channels.empty() && channels != "0")
  {
    /*  std::vector<DWORD> vecChannel;
    numTok(channels, ",", vecChannel);
    auto it = std::find(vecChannel.begin(), vecChannel.end(), thisServer->getRegionID());
    if (it == vecChannel.end())
    {
    //thisServer->sendMsg(zoneId, id, 1050);
    XERR << "[礼包码] 超出服使用范围， regid" << thisServer->getRegionID() << "regids" << regionIds << XEND;
    return false;
    }*/
  }

  //一般码
  if (type == 0)
  {
    //check code used
    char where[64];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "code=\"%s\"", code.c_str());
    QWORD r = thisServer->getDBConnPool().checkExist(RO_DATABASE_NAME, "gift_code_use", where);
    if (r == QWORD_MAX)
    {
      //MsgManager::sendMsg(id, 1061);
      sstr << "[礼包码] 数据库异常";
      XERR << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
    if (r == 1)
    {
      //MsgManager::sendMsg(id, 1059);
      sstr << "[礼包码] 已经被使用";
      XINF << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }

    //check use count
    xField field(RO_DATABASE_NAME, "");
    field.setValid("count(groupid) as count");
    field.m_list["count"] = MYSQL_TYPE_NEWDECIMAL;

    char sql[1024];
    bzero(sql, sizeof(sql));

    snprintf(sql, sizeof(sql), "select count(groupid) as count from %s.gift_code_use where accid=%llu and groupid=%llu",
      field.m_strDatabase.c_str(), accid, groupId);

    std::string strSql(sql);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeRawSelect(&field, set, strSql);
    if (ret == QWORD_MAX)
    {
      //MsgManager::sendMsg(id, 1061);
      sstr << "[礼包码] 数据库异常";
      XERR << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
    DWORD useCount = 0;
    if (ret == 1)
    {
      useCount = set[0].get<DWORD>("count");
    }
    if (useCount >= maxCount)
    {
      //MsgManager::sendMsg(id, 1072);
      sstr << "[礼包码] 礼包码已达到最大使用次数,已使用次数：" << useCount << "批次最大使用次数：" << maxCount;
      XERR << sstr.str() << XEND;
      setJson(EJSONRET_MESSAGE, sstr.str());
      return m_vecResult;
    }
  }

  //insert use db
  xField *pField = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "gift_code_use");
  if (!pField)
  {
    //MsgManager::sendMsg(id, 1061);
    sstr << "[礼包码] 数据库异常";
    XERR << sstr.str() << XEND;
    setJson(EJSONRET_MESSAGE, sstr.str());
    return m_vecResult;
  }
  //mysql index:charid code 
  xRecord record(pField);
  record.put("accid", accid);        //用accid
  record.put("charid", id);
  record.putString("code", code);
  record.put("batchid", batchId);
  record.put("usetime", curSec);
  record.putString("username", name);
  record.put("groupid", groupId);
  ret = thisServer->getDBConnPool().exeInsert(record);
  if (ret == QWORD_MAX)
  {
    //MsgManager::sendMsg(id, 1060);
    sstr << "[礼包码] 您已经领取过该批次的兑换码";
    XERR << sstr.str() << XEND;
    setJson(EJSONRET_MESSAGE, sstr.str());
    return m_vecResult;
  }

  //give gift  
  TVecItemInfo vecItemInfo;
  std::vector<std::string> vecItem1;
  stringTok(items, ";", vecItem1);

  for (auto &v : vecItem1)
  {
    std::vector<DWORD> vecItem2;
    numTok(v, ",", vecItem2);
    if (vecItem2.size() != 2)
    {
      //MsgManager::sendMsg(id, 1061);
      sstr << "[礼包码] 错误的物品配置";
      XERR << sstr.str() << XEND;
      return m_vecResult;
    }
    ItemInfo oItem;
    oItem.set_id(vecItem2[0]);
    oItem.set_count(vecItem2[1]);
    vecItemInfo.push_back(oItem);
  }
  
  bool bR = MailManager::getMe().sendMail(id, 0, "礼包码管理员", mailTitle, mailMsg, EMAILTYPE_NORMAL, 0, vecItemInfo, TVecItemData{}, EMAILATTACHTYPE_ITEM);
  if (bR)
  {
    MsgManager::sendMsg(id, 1062);
    sstr << "[礼包码] 恭喜您，礼包码兑换成功,请注意查收邮件";
    XLOG << sstr.str() << XEND;
  }
  else {
    //MsgManager::sendMsg(id, 1061);
    sstr << "[礼包码] 礼包码兑换失败";
    XERR << sstr.str() << XEND;
  }

  setJson(EJSONRET_MESSAGE, sstr.str());
  return m_vecResult;
}

const TVecString& GMCommandManager::tradeSecurityCMD(const Value& root)
{
  QWORD accid = 0;
  DWORD itemId = 0;
  int refineLv = 0;
  bool bValid = false;
  QWORD qwKey = 0;
  Cmd::ESecurityType type;
  try
  {
    accid = atoll(root["accid"].asString().c_str());
    itemId = atoi(root["itemid"].asString().c_str());
    refineLv = atoi(root["refinelv"].asString().c_str());
    bValid = atoi(root["valid"].asString().c_str());
    char *pend;
    qwKey = strtoul(root["key"].asString().c_str(), &pend, 10);
    type = static_cast<ESecurityType>(atoi(root["type"].asString().c_str()));
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  if (qwKey == 0 && itemId == 0)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误，itemid为0");
    return m_vecResult;
  }

  SecurityCmdSceneTradeCmd cmd;
  cmd.set_valid(bValid);
  cmd.set_itemid(itemId);
  cmd.set_refinelv(refineLv);
  cmd.set_type(type);
  cmd.set_key(qwKey);
  
  auto sendToTrade = [&]()
  {
    PROTOBUF(cmd, send, len);
    bool ret = thisServer->sendCmd(ClientType::trade_server, send, len);
    XLOG << "[GM-交易所安全指令] 发送到tradeserver ret" << ret << cmd.ShortDebugString() << XEND;
  };
  
  if (accid > 0)
  {
    xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
    if (!field)
    {
      setJson(EJSONRET_MESSAGE, "数据库找不到");
      return m_vecResult;
    }
    field->setValid("charid,name");

    char where[64] = { 0 };
    snprintf(where, sizeof(where), "accid=%llu", accid);
    xRecordSet set;
    QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
    if (QWORD_MAX == ret)
    {
      setJson(EJSONRET_MESSAGE, "找不到玩家角色id");
      return m_vecResult;
    }

    QWORD charId = 0;
    for (DWORD i = 0; i < ret; ++i)
    {
      charId = set[i].get<QWORD>("charid");
      cmd.set_charid(charId);
      sendToTrade();
    }
  }
  else
  {
    cmd.set_charid(0);
    sendToTrade();
  }
  
  return m_vecResult;
}

const TVecString& GMCommandManager::modifyAuctionTime(const Value& root)
{
  DWORD time = 0;
  try
  {
    time = atoi(root["auction_time"].asString().c_str());
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }
    
  GmModifyAuctionTimeSCmd cmd;
  cmd.set_auction_time(time);
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmd(ClientType::auction_server, send, len);
  XLOG << "[GM-修改拍卖开始时间] 发送到拍卖服 ret" << ret <<"时间"<< time << XEND;

  return m_vecResult;
}

const TVecString& GMCommandManager::stopAuction()
{
  GmStopAuctionSCmd cmd;
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmd(ClientType::auction_server, send, len);
  XLOG << "[GM-停止拍卖] 发送到拍卖服 ret" << ret << XEND;

  return m_vecResult;
}

const TVecString& GMCommandManager::verifyGuildIcon(const Value& root)
{
  EIconState state = EICON_INIT;
  std::vector<std::string> sVec;
  try
  {
    string strId = root["idstr"].asString();
    stringTok(strId, ",", sVec);
    if (sVec.empty())
    {
      setJson(EJSONRET_MESSAGE, "参数解析错误");
      return m_vecResult;
    }
    DWORD s = root["state"].asUInt();
    if (EIconState_IsValid(s))
      state = EIconState(s);
  }
  catch (...)
  {
    setJson(EJSONRET_MESSAGE, "参数解析错误");
    return m_vecResult;
  }

  GuildIconStateGuildSCmd cmd;
  for (auto id : sVec)
    cmd.add_ids(atoll(id.c_str()));
  cmd.set_state(state);
  PROTOBUF(cmd, send, len);
  bool ret = thisServer->sendCmd(ClientType::guild_server, send, len);
  XLOG << "[GM-审核公会图标] 发送到公会服 ret" << ret << "id:";
  for (auto id : sVec)
    XLOG << id;
  XLOG << "state" << state << XEND;

  return m_vecResult;
}

void GMCommandManager::setJson(EJsonRetType eType, const string& str)
{
  if (eType >= m_vecResult.size())
    return;

  m_vecResult[eType] = str;
}

void GMCommandManager::appendData(EJsonRetType eType, const string& str)
{
  if (eType >= m_vecResult.size())
    return;
  if (!m_vecResult[eType].empty()) {
    m_vecResult[eType] += ",";
  }
  m_vecResult[eType] += str;
}

void GMCommandManager::appendData(EJsonRetType eType, QWORD id)
{
  std::stringstream ss;
  ss << id;
  appendData(eType, ss.str());
}

void GMCommandManager::execute(SessionUser* pUser, const string& command)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;
  SessionGMCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_data(command);

  PROTOBUF(cmd, send, len);
  pUser->getScene()->sendCmd(send, len);
}

void GMCommandManager::execute(SessionUser* pUser, const xLuaData& command)
{
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return;

  stringstream str;
  str.str("");
  command.toJsonString(str);

  SessionGMCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_data(str.str());

  PROTOBUF(cmd, send, len);
  pUser->getScene()->sendCmd(send, len);
}

// --------------------------------session gm---------------------

GmSessionCmd GMSessionCmds[]=
{
  {"broadcast", GMCommandManager::broadcast, HUMAN_NORMAL, "发送公告"},
  {"replace_reward", GMCommandManager::replace_reward, HUMAN_NORMAL, "奖励替换"},
  {"recover_reward", GMCommandManager::recover_reward, HUMAN_NORMAL, "奖励恢复"},
  {"set_wantedactive", GMCommandManager::set_wantedactive, HUMAN_NORMAL, "看板追加" },  
  {"add_ntfactprogress", GMCommandManager::add_ntfactprogress, HUMAN_NORMAL, "全服发送活动进度"},
  {"del_ntfactprogress", GMCommandManager::del_ntfactprogress, HUMAN_NORMAL, "删除全服活动进度"},
};

GmSessionCmd GMSessionSceneCmds[]=
{
  {"startglobalactivity", GMCommandManager::startglobalactivity, HUMAN_NORMAL, "开启所有场景活动"},
  {"stopglobalactivity", GMCommandManager::stopglobalactivity, HUMAN_NORMAL, "关闭所有场景活动"},
};

void GMCommandManager::timerTick(DWORD curSec)
{
  if (m_oTenSecTimer.timeUp(curSec))
  {
    for (auto it = m_retryList.begin(); it != m_retryList.end();)
    {
      //3秒 * 100
      if (it->count >= 100)
      {
        it = m_retryList.erase(it);
        continue;
      }

      SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(it->mapId);
      if (pScene)
      {        
        PROTOBUF(it->cmd, send, len);
        pScene->sendCmd(send, len);
        XINF << "[session-gm-重试], 发送gm 到指定场景" << "mapid" << it->mapId <<"cmd:"<< it->cmd.ShortDebugString() << XEND;
        it = m_retryList.erase(it);
        continue;
      }
      XERR << "[session-gm-重试], 场景尚未启动OK " << "mapid" << it->mapId <<"第"<<it->count<<"次重试" << "cmd:" << it->cmd.ShortDebugString() << XEND;
      it->count++;
      ++it;
    }
  }
}

bool GMCommandManager::execute(const xLuaData& command,const TSetDWORD& extraMap)
{
  GmSessionFun pFun = 0;
  for(DWORD n=0;n<(sizeof(GMSessionCmds)/sizeof(GmSessionCmd));n++)
  {
    if(0==strncmp(GMSessionCmds[n].cmd, command.getTableString("type"), MAX_NAMESIZE))
    {
      pFun = GMSessionCmds[n].p;
    }
  }
  if (pFun)
    return (*pFun)(command);

  GmSessionFun pSceneFun = 0;
  for(DWORD n=0;n<(sizeof(GMSessionSceneCmds)/sizeof(GmSessionCmd));n++)
  {
    if(0==strncmp(GMSessionSceneCmds[n].cmd, command.getTableString("type"), MAX_NAMESIZE))
    {
      pSceneFun = GMSessionSceneCmds[n].p;
    }
  }
  if (pSceneFun && ((*pSceneFun)(command)) == false)
  {
    return false;
  }

  // -- send to scene
  stringstream str;
  str.str("");
  command.toJsonString(str);
  
  SessionGMCmd cmd;
  cmd.set_data(str.str());

  auto sendf = [&](bool extra, DWORD mapid)
  {
    SessionScene* pScene = SessionSceneManager::getMe().getSceneByID(mapid);
    cmd.set_mapid(mapid);
    if (pScene)
    {
      PROTOBUF(cmd, send, len);
      pScene->sendCmd(send, len);
      XLOG << "[session-gm], 发送gm 到指定场景, extra"<< extra<< "mapid"<<mapid <<"gmcmd:" << str.str() << XEND;
    }
    else
    {
      GMRetry retry;
      retry.mapId = mapid;
      retry.count = 0;
      retry.cmd = cmd;
      m_retryList.push_back(retry);
      XERR << "[session-gm], 场景尚未启动OK, extra" << extra << "mapid" << mapid <<"gmcmd:" << str.str() << XEND;
    }
  };

  if (extraMap.empty())
  {
    if (command.has("map"))
    {
      auto func = [&](const string& key, xLuaData& data)
      {
        DWORD dwMapID = data.getInt();
        sendf(false, dwMapID);
      };
      const xLuaData& rMap = command.getData("map");
      xLuaData t = rMap;
      t.foreach(func);
    }
    else
    {
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToAllScene(send, len);
    }
  }
  else
  {
    for (auto&v : extraMap)
    {
      sendf(true, v);
    }
  }

  return true;
}

bool GMCommandManager::broadcast(const xLuaData& data)
{
  if (data.has("msgid") == false && data.has("effectid") == false)
    return false;
  if (data.has("msgid"))
  {
    DWORD msgid = data.getTableInt("msgid");
    MsgManager::sendWorldMsg(msgid);
    XLOG << "[session-gm], 发送公告, 公告id:" << msgid << XEND;
  }

  if (data.has("effectid"))
     MsgManager::sendWorldMsg(data.getTableInt("effectid"), MsgParams(), EMESSAGETYPE_MIDDLE_SHOW); 

  return true;
}

bool GMCommandManager::replace_reward(const xLuaData& data)
{
  if (!data.has("id") || !data.has("newid"))
    return false;

  DWORD id = data.getTableInt("id");
  DWORD newid = data.getTableInt("newid");
  if (RewardConfig::getMe().addReplace(id, newid) == false)
    XERR << "[session-gm], reward替换失败, id:" << id << "newid:" << newid <<XEND;

  // send to scene
  stringstream str;
  str.str("");
  data.toJsonString(str);

  SessionGMCmd cmd;
  cmd.set_data(str.str());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllScene(send, len);

  return true;
}

bool GMCommandManager::recover_reward(const xLuaData& data)
{
  if (!data.has("id"))
    return false;
  DWORD id = data.getTableInt("id");
  if (RewardConfig::getMe().delReplace(id) == false)
    XERR << "[session-gm], reward恢复失败, id:" << id <<XEND;

  // send to scene
  stringstream str;
  str.str("");
  data.toJsonString(str);

  SessionGMCmd cmd;
  cmd.set_data(str.str());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToAllScene(send, len);

  return true;
}

bool GMCommandManager::set_wantedactive(const xLuaData& data)
{
  DWORD add = data.getTableInt("add");
  DWORD maxCount = data.getTableInt("max");
  if (add)
  {
    QuestConfig::getMe().setWantedActive(true, maxCount);
    QuestConfig::getMe().randomWantedQuest(true);
  }
  else
  {
    QuestConfig::getMe().setWantedActive(false, maxCount);
  }

  QueryWantedInfoQuestCmd cmd;
  cmd.set_maxcount(add != 0 ? maxCount : MiscConfig::getMe().getQuestCFG().getMaxWantedCount(EWANTEDTYPE_TOTAL));
  PROTOBUF(cmd, send, len);
  MsgManager::sendWorldCmd(send, len);

  WantedInfoSyncSessionCmd scmd;
  scmd.set_active(QuestConfig::getMe().getActiveWanted());
  scmd.set_maxcount(cmd.maxcount());
  PROTOBUF(scmd, ssend, slen);
  thisServer->sendCmdToAllScene(ssend, slen);

  XLOG << "[Session-GM] 看板任务变更" << ((add != 0) ? "开启" : "关闭") << "最大数量 :" << cmd.maxcount() << XEND;
  return true;
}

void GMCommandManager::pushtyrantdb(QWORD accid, QWORD charId, std::string orderid, DWORD amount, DWORD itemCount, string productId, string chargeType)
{
  if (!xServer::isOuter())
    return;
  if (accid == 0)
  {
    GCharReader gCharReader(thisServer->getRegionID(), charId);
    if (!gCharReader.get())
    {
      XERR << "[tyrantdb-推送] 查找玩家accid 失败,找不到玩家的charreader, charId" << charId << orderid << amount << itemCount << productId << chargeType << XEND;
      return;
    }
    accid = gCharReader.getAccID();
  }

  PushTyrantDbGateSuperCmd cmd;
  cmd.set_accid(accid);
  cmd.set_charid(charId);
  cmd.set_orderid(orderid);
  cmd.set_amount(amount);
  cmd.set_itemcount(itemCount);
  cmd.set_productid(productId);
  cmd.set_chargetype(chargeType);
  PROTOBUF(cmd, send, len);
  if (thisServer->sendCmdToServer(send, len, "SuperServer") == false)
  {
    XERR << "[tyrantdb-推送] charId,发送到superserver 失败" << charId << orderid << amount << itemCount << productId << chargeType << XEND;
  }
  else
  {
    XLOG << "[tyrantdb-推送] charId,发送到superserver 成功" << charId << orderid << amount << itemCount << productId << chargeType << XEND;
  }
}

bool GMCommandManager::canBuy(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym, bool bFromWeb, const SGlobalActCFG** pActCfg, bool&bUpdateLimit)
{
  bUpdateLimit = false;
  if (!pCFG)
    return false;
  if (!bFromWeb) //网页充值不算在内
  {
    if (SessionActivityMgr::getMe().checkUserGlobalActCnt(GACTIVITY_CHARGE_DISCOUNT, pCFG, accid, charId, pActCfg))
    {
      return true;
    }
    if (SessionActivityMgr::getMe().checkUserGlobalActCnt(GACTIVITY_CHARGE_EXTRA_COUNT, pCFG, accid, charId, pActCfg))
    {
      return true;
    }
    SessionActivityMgr::getMe().checkUserGlobalActCnt(GACTIVITY_CHARGE_EXTRA_REWARD, pCFG, accid, charId, pActCfg);
  }

  if (pCFG->limitType == ELimitType_Account || pCFG->limitType == ELimitType_Char || pCFG->limitType == ELimitType_Branch)
  {
    if (canBuyLimit(accid, charId, pCFG, outCount, ym, bFromWeb) == true)
    {
      bUpdateLimit = true;
      return true;
    }
    else
      return false;
  }
  else if (pCFG->limitType == ELimitType_ActivityOpen)
  {
    return false;
  }
  //XLOG << "[充值-充值次数查询] accid" << accid << " charid" << charId << "ym" << ym << "已经充值次数" << outCount << "每月最大次数" << pCFG->monthLimit << "ret" << bRet << "column" << columnName << "tablename" << tableName << XEND;
  return true;
}

bool GMCommandManager::canBuyLimit(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym, bool bFromWeb)
{ 
  if (!pCFG)
    return false;
  string tableName;
  string columnName = pCFG->strDbColumn;
  if (pCFG->limitType == ELimitType_Account)
    tableName = "charge_accid";
  else if (pCFG->limitType == ELimitType_Char)
    tableName = "charge_card";
  else if (pCFG->limitType == ELimitType_Branch)
  {
    if (pCFG->dwActivityDiscount)
    {
      if (bFromWeb)
      {
        XERR << "[充值-限购] 网页充值不可购买ep特典卡 accid" << accid << "charid" << charId << "dataid" << pCFG->id << XEND;
        return false;
      }
      if (pCFG->getFromId() == pCFG->id)
      {
        XERR << "[充值-限购] ep特典卡尚未打折出售 accid" << accid << "charid" << charId << "dataid" << pCFG->id << XEND;
        return false;
      }
    }
    tableName = "charge_epcard";
    columnName = "epcard";
  }
  else 
    return true;

  if (columnName.empty() || tableName.empty())
    return false;

  outCount = 0;
  if (pCFG == nullptr)
    return false;

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, tableName.c_str());
  if (field == nullptr)
    return false;
  DWORD curSec = now() - 5 * 3600;    //5点才算
  ym = xTime::getYear(curSec) * 100 + xTime::getMonth(curSec);
  
  char where[512];
  bzero(where, sizeof(where));
  if (pCFG->limitType == ELimitType_Account)
    snprintf(where, sizeof(where), "accid=%lld and ym=%d", accid, ym);
  else if (pCFG->limitType == ELimitType_Char)
    snprintf(where, sizeof(where), "charid=%lld and ym=%d", charId, ym);
  else if (pCFG->limitType == ELimitType_Branch)
    snprintf(where ,sizeof(where), "accid=%lld and dataid=%d", accid, pCFG->getFromId());

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
  if (QWORD_MAX == ret)
  {
    return false;
  }
  else if (1 == ret)
  {
    outCount = set[0].get<DWORD>(columnName);
  }

  DWORD limit = pCFG->monthLimit;
  SessionUser* pUser = SessionUserManager::getMe().getUserByID(charId);
  if (pUser)
  {
    DWORD dynLimit = pUser->getDepositLimit(pCFG->id);
    if (dynLimit)
      limit = dynLimit;
  }
  else
  {
    xLuaData data;
    if (getDepositLimit(charId, ym, data))
    {
      stringstream ss;
      ss << pCFG->id;
      DWORD dynLimit = data.getTableInt(ss.str());
      if (dynLimit)
        limit = dynLimit;
    }
  }

  bool bRet = true;
  if (outCount < limit)
    bRet = true;
  else
    bRet = false;

  XLOG << "[充值-充值次数查询] accid" <<accid <<" charid" << charId << "ym" << ym << "已经充值次数" << outCount << "每月最大次数" << limit << "ret" << bRet << "column" << columnName <<"tablename"<<tableName <<"充值id"<<pCFG->id <<"fromid"<< pCFG->getFromId() << XEND;
  return bRet;
}

bool GMCommandManager::updateBuyLimitDb(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD ym, DWORD count)
{
  if (!pCFG)
    return false;
   string tableName;
   string columnName = pCFG->strDbColumn;
   if (pCFG->limitType == ELimitType_Account)
     tableName = "charge_accid";
   else if (pCFG->limitType == ELimitType_Char)
     tableName = "charge_card";
   else if (pCFG->limitType == ELimitType_Branch)
   {
     tableName = "charge_epcard";
     columnName = "epcard";
   }
   else
     return false;

  if (columnName.empty())
    return false;

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, tableName.c_str());
  if (field == nullptr)
    return false;
  xRecord record(field);
  switch (pCFG->limitType)
  {
  case ELimitType_Account:
  {
    record.put("accid", accid);
    record.put("ym", ym);
    record.put(columnName.c_str(), count);
    break;
  }
  case ELimitType_Char:
  {
    record.put("charid", charId);
    record.put("ym", ym);
    record.put(columnName.c_str(), count);
    break;
  }
  case ELimitType_Branch:
  {
    record.put("accid", accid);
    record.put("dataid", pCFG->getFromId());
    record.put(columnName.c_str(), count);
    break;
  }
  default:
    break;
  }

  QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
  if (ret == QWORD_MAX)
  {
    XERR << "[充值-购买] 插入数据失败，accid" << accid <<"charid" << charId << "ym" << ym << "count" << count << "dataid" << pCFG->id << "column" << columnName <<tableName << pCFG->getFromId() << XEND;
    return false;
  } 
  XLOG << "[充值-购买] 插入数据成功，accid" << accid << "charid" << charId << "ym" << ym << "count" << count << "dataid" << pCFG->id << "column" << columnName << tableName << pCFG->getFromId() << XEND;
  return true;
}
//
//bool GMCommandManager::canBuyFuDai(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD& outCount, DWORD &ym)
//{
//  outCount = 0;
//  if (pCFG == nullptr)
//    return false;
//  //fdcnt
//  string columnName = pCFG->strDbColumn;
//  if (columnName.empty())
//    return false;
//
//  xField *field = xFieldsM::getMe().getField(REGION_DB, "charge_accid");
//  if (field == nullptr)
//    return false;
//  DWORD curSec = now() - 5 * 3600;    //5点才算
//  ym = xTime::getYear(curSec) * 100 + xTime::getMonth(curSec);
//  
//  char where[512];
//  bzero(where, sizeof(where));
//  snprintf(where, sizeof(where), "accid=%lld and ym=%d", accid, ym);
//  xRecordSet set;
//  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, where, NULL);
//  if (QWORD_MAX == ret)
//  {
//    return false;
//  }
//  else if (1 == ret)
//  {
//    outCount = set[0].get<DWORD>(columnName);
//  }
//
//  bool bRet = true;
//  if (outCount >= pCFG->monthLimit)
//    bRet = false;
//
//  XLOG << "[充值-账号限购次数查询] accid" << accid << " charid" << charId << "ym" << ym << "已经充值次数" << outCount << "每月最大次数" << pCFG->monthLimit << "ret" << bRet << "column" << columnName << XEND;
//  return bRet;
//}
//
//bool GMCommandManager::updateFuDaiDb(QWORD accid, QWORD charId, const SDeposit* pCFG, DWORD ym, DWORD count)
//{
//  if (pCFG == nullptr)
//    return false;
//  string columnName = pCFG->strDbColumn;
//  if (columnName.empty())
//    return false;
//
//  xField *field = xFieldsM::getMe().getField(REGION_DB, "charge_accid");
//  if (field == nullptr)
//    return false;
//  xRecord record(field);
//  record.put("accid", accid);
//  record.put("ym", ym);
//  record.put(columnName.c_str(), count);
//
//  QWORD ret = thisServer->getDBConnPool().exeInsert(record, false, true);
//  if (ret == QWORD_MAX)
//  {
//    XERR << "[充值-购买账号限购次数] 插入数据失败，accid" << accid << "charid" << charId << "ym" << ym << "count" << count << "dataid" << pCFG->id <<"column"<<columnName << XEND;
//    return false;
//  }
//  XLOG << "[充值-购买账号限购次数] 插入数据成功，accid" << accid << "charid" << charId << "ym" << ym << "count" << count << "dataid" << pCFG->id << "column" << columnName << XEND;
//  return true;
//}

bool GMCommandManager::startglobalactivity(const xLuaData& data)
{
  if (data.has("id") == false)
    return false;

  DWORD id = data.getTableInt("id");
  DWORD timerId = data.getTableInt("Timer_Id");   //TimerM自己透传进来的

  if(SessionActivityMgr::getMe().addGlobalActivity(id, timerId))
  {
    XLOG << "[全服活动-Session] 开启成功" << id << XEND;
    return true;
  }

  XERR << "[全服活动-Session] 开启失败" << id << XEND;
  return false;
}

bool GMCommandManager::stopglobalactivity(const xLuaData& data)
{
  if (data.has("id") == false)
    return false;

  DWORD id = data.getTableInt("id");

  if(SessionActivityMgr::getMe().delGlobalActivity(id))
  {
    XLOG << "[全服活动-Session] 结束成功" << id << XEND;
    return true;
  }

  XERR << "[全服活动-Session] 结束失败" << id << XEND;
  return false;
}

bool GMCommandManager::add_ntfactprogress(const xLuaData& data)
{
  if (data.has("activityid") == false || data.has("progress") == false)
    return false;
  DWORD dwActID = data.getTableInt("activityid");
  DWORD dwProgress = data.getTableInt("progress");
  DWORD dwStartTime = 0;
  const string& starttime = data.getTableString("starttime");
  if (starttime.empty() == false)
    parseTime(starttime.c_str(), dwStartTime);
  DWORD dwEndTime = 0;
  const string& endtime = data.getTableString("endtime");
  if (endtime.empty() == false)
    parseTime(endtime.c_str(), dwEndTime);
  SessionUserManager::getMe().addActivityProgress(dwActID, dwProgress, dwStartTime, dwEndTime);
  XLOG<<"[活动进度] 配置读取:" << "id: " << dwActID << "progress:" << dwProgress << "start:" << starttime << "end:" << endtime << XEND;

  return true;
}

bool GMCommandManager::del_ntfactprogress(const xLuaData& data)
{
  if (data.has("activityid") == false || data.has("progress") == false)
    return false;
  DWORD dwActID = data.getTableInt("activityid");
  DWORD dwProgress = data.getTableInt("progress");
  SessionUserManager::getMe().delActivityProgress(dwActID, dwProgress);
  XLOG<<"[活动进度] 配置删除:" << "id: " << dwActID << "progress:" << dwProgress << "nowtime:" << now() << XEND;

  return true;
}

bool GMCommandManager::getDepositLimit(QWORD charid, DWORD ym, xLuaData& data)
{
  const string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_DEPOSIT_LIMIT, charid, ym);
  if (RedisManager::getMe().getHashAll(key, data) == false)
  {
    XDBG << "[GM-获取充值上限]" << charid << "key:" << key << "查询redis失败或者数据为空" << XEND;
    return true;
  }
  return true;
}

const TVecString& GMCommandManager::clearMailTemplate(const Value& root, const ExecGMCmd& cmd)
{
  MailTemplateManager::getMe().clear();
  XLOG << "[GM-清除邮件模板] 模板缓存清理成功" << XEND;

  if (cmd.serverid() == thisServer->getZoneID())
  {
    //forward to other line
    ExecGMCmd newCmd = cmd;
    PROTOBUF(newCmd, send, len);

    ForwardToAllSessionSocialCmd cmd2;
    cmd2.set_data((const char*)send);
    cmd2.set_len(len);
    cmd2.set_except(thisServer->getZoneID());
    PROTOBUF(cmd2, send2, len2);
    thisServer->sendCmd(ClientType::global_server, send2, len2);
  }

  return m_vecResult;
}

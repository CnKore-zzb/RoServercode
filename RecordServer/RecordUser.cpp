#include "RecordUser.h"
#include "xDBFields.h"
#include "RecordServer.h"
#include "RegCmd.h"
#include "RedisManager.h"
#include "ClientCmd.h"
#include "PlatLogManager.h"
#include "GCharManager.h"
#include "BaseConfig.h"
#include "ShopConfig.h"
#include "AchieveConfig.h"
#include "CommonConfig.h"

// record user
RecordUser::RecordUser(QWORD qwAccID, QWORD qwCharID)
{
  m_oBase.set_accid(qwAccID);
  m_oBase.set_charid(qwCharID);
}

RecordUser::~RecordUser()
{

}

void RecordUser::toData(RecordUserData& rData)
{
  rData.mutable_acc()->CopyFrom(m_oAcc);
  rData.mutable_base()->CopyFrom(m_oBase);
  rData.set_char_data(m_oCharData.c_str(), m_oCharData.size());
  rData.set_acc_data(m_oAccData.c_str(), m_oAccData.size());
  //rData.set_store(m_oStore.c_str(), m_oStore.size());
}

/*void RecordUser::toData(BlobAccData& rData)
{
  rData.Clear();
  if (m_oOldQuest.version() >= ACC_UNCOMPRESS_VERSION)
  {
    string acc_data = m_oAccData;
    if (uncompress(acc_data, acc_data) == false)
    {
      XERR << "[玩家-数据]" << m_oBase.accid() << m_oBase.charid() << "提取acc_data失败" << XEND;
      return;
    }
    rData.ParseFromString(acc_data);
    return;
  }
  rData.ParseFromString(m_oAccData);
}*/

bool RecordUser::loadData()
{
  if (loadAccData() == false)
    return false;
  if (loadCharData() == false)
    return false;
  //if (loadStore() == false)
  //  return false;
  return true;
}

bool RecordUser::saveData(bool isOffline)
{
  bool bResult = false;
  if (saveAccData(isOffline) == false)
    bResult = false;
  if (saveCharData(isOffline) == false)
    bResult = false;
  if (saveRedis() == false)
    bResult = false;
  return bResult;
}

bool RecordUser::updateData(const UserDataRecordCmd& rCmd)
{
  if (rCmd.first() == true)
    m_oTransData.clear();

  if (rCmd.over() == false)
  {
    m_oTransData.append(rCmd.data().c_str(), rCmd.data().size());
    return false;
  }
  m_oTransData.append(rCmd.data().c_str(), rCmd.data().size());

  RecordUserData oData;
  if (oData.ParseFromString(m_oTransData) == false)
  {
    XERR << "[玩家-数据更新]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "RecordUserData反序列化失败" << XEND;
    return false;
  }

  m_oBase.CopyFrom(oData.base());
  m_oRedis.CopyFrom(oData.redis());

  m_oAccData.assign(oData.acc_data().c_str(), oData.acc_data().size());
  m_oCharData.assign(oData.char_data().c_str(), oData.char_data().size());

  XLOG << "[玩家-数据更新]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "更新成功" << "accdata size :" << m_oAccData.size() << "chardata size :" << m_oCharData.size() << "total size :" << m_oTransData.size() << XEND;
  m_oTransData.clear();
  return true;
}

bool RecordUser::exchangeData()
{
  xField* pAccField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (pAccField == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,未找到accbase数据库表" << XEND;
    return false;
  }
  xField* pCharField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pCharField == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 未找到charbase数据库表" << XEND;
    return false;
  }

  // select acc data
  stringstream sstr;
  sstr << "accid = " << m_oBase.accid();

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pAccField, set, sstr.str().c_str(), nullptr);
  if (QWORD_MAX == ret || 0 == ret)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,查询accbase失败,ret :" << ret << XEND;
    return false;
  }

  m_oAccData.clear();
  m_oAccData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));

  BlobCredit oCredit;
  if (oCredit.ParseFromString(m_oAccData) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,反序列化BlobCredit失败" << ret << XEND;
    return false;
  }

  // select char data
  set.clear();
  ret = thisServer->getDBConnPool().exeSelect(pCharField, set, sstr.str().c_str(), nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 查询charbase失败" << XEND;
    return false;
  }
  if (ret == 0)
  {
    // set version
    m_oOldQuest.set_version(1);

    // save acc data
    if (saveAccData() == false)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 无数据迁移,保存AccData失败" << XEND;
      return false;
    }

    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移成功, 无数据迁移" << XEND;
    return true;
  }

  TMapExchangeUser mapUser;
  for (DWORD d = 0; d < set.size(); ++d)
  {
    BlobData oData;
    string data;
    data.assign((const char*)(set[d].getBin("data")), set[d].getBinSize("data"));
    if (uncompress(data, data) == false)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 解压BlobData失败" << XEND;
      return false;
    }
    if (oData.ParseFromString(data) == false)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 反序列化BlobData失败" << XEND;
      return false;
    }
    SExchangeUser& rData = mapUser[set[d].get<QWORD>("charid")];
    rData.oBase.set_charid(set[d].get<QWORD>("charid"));
    rData.oBase.set_rolelv(set[d].get<DWORD>("rolelv"));
    rData.oBase.set_name(set[d].getString("name"));
    rData.oData.CopyFrom(oData);
  }

  BlobAccData oAccData;

  // acc - user
  BlobAccUser* pAccUser = oAccData.mutable_user();
  if (pAccUser == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->AccUser为空" << XEND;
    return false;
  }
  pAccUser->set_auguryreward(oCredit.auguryreward());
  pAccUser->set_maxbaselv(oCredit.maxbaselv());
  pAccUser->set_maxbaselv_resettime(oCredit.maxbaselv_resettime());

  // acc - credit
  BlobNewCredit* pNewCredit = oAccData.mutable_credit();
  if (pNewCredit == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->credit为空" << XEND;
    return false;
  }
  pNewCredit->set_version(oCredit.version());
  pNewCredit->set_credit(oCredit.credit());
  pNewCredit->set_monster_value(oCredit.monster_value());
  pNewCredit->set_savedtime(oCredit.savedtime());
  pNewCredit->set_forbidtime(oCredit.forbidtime());

  // acc - shopgot
  BlobShopGotItem* pAccItem = oAccData.mutable_shop();
  if (pAccItem == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->shop为空" << XEND;
    return false;
  }
  pAccItem->CopyFrom(oCredit.shop());

  // acc - var
  BlobAccVar* pAccVar = oAccData.mutable_var();
  if (pAccVar == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->var为空" << XEND;
    return false;
  }
  pAccVar->CopyFrom(oCredit.var());

  // acc - scenery
  if (exchangeScenery(mapUser, oAccData.mutable_scenery()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->scenery失败" << XEND;
    return false;
  }

  // acc - manual
  if (exchangeManual(mapUser, oAccData.mutable_manual(), oAccData.mutable_photo()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->manual失败" << XEND;
    return false;
  }

  // acc - food
  if (exchangeFood(mapUser, oAccData.mutable_food(), oAccData.mutable_manual()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->food失败" << XEND;
    return false;
  }

  // acc - quest
  if (exchangeQuest(mapUser, oAccData.mutable_quest()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->quest失败" << XEND;
    return false;
  }

  // acc - title
  if (exchangeTitle(mapUser, oAccData.mutable_title(), oAccData.mutable_quest()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->title失败" << XEND;
    return false;
  }

  // acc - achieve
  if (exchangeAchieve(mapUser, oAccData.mutable_achieve(), oAccData.mutable_manual()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->achieve失败" << XEND;
    return false;
  }

  // acc - menu
  if (exchangeMenu(mapUser, oAccData.mutable_menu()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->menu失败" << XEND;
    return false;
  }

  // acc - portrait
  if (exchangePortrait(mapUser, oAccData.mutable_portrait()) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->portrait失败" << XEND;
    return false;
  }

  // set version
  m_oOldQuest.set_version(1);

  // serial NEW acc data
  m_oAccData.clear();
  if (oAccData.SerializeToString(&m_oAccData) == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 序列化AccData失败" << XEND;
    return false;
  }

  // save acc data
  if (saveAccData() == false)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 保存AccData失败" << XEND;
    return false;
  }

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移成功" << XEND;
  return true;
}

#include "QuestConfig.h"
bool RecordUser::exchangeFoodQuest()
{
  xField* pAccField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (pAccField == nullptr)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,未找到accbase数据库表" << XEND;
    return false;
  }
  xField* pCharField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pCharField == nullptr)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 未找到charbase数据库表" << XEND;
    return false;
  }

  // select acc data
  stringstream sstr;
  sstr << "accid = " << m_oBase.accid();

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pAccField, set, sstr.str().c_str(), nullptr);
  if (QWORD_MAX == ret || 0 == ret)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,查询accbase失败,ret :" << ret << XEND;
    return false;
  }

  m_oAccData.clear();
  m_oAccData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));

  BlobAccData oAccData;
  if (oAccData.ParseFromString(m_oAccData) == false)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 反序列化BlobAccData失败" << XEND;
    return false;
  }

  // select char data
  set.clear();
  ret = thisServer->getDBConnPool().exeSelect(pCharField, set, sstr.str().c_str(), nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 查询charbase失败" << XEND;
    return false;
  }
  if (ret == 0)
  {
    // set version
    m_oOldQuest.set_version(2);

    // save acc data
    if (saveAccData() == false)
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 无数据迁移,保存AccData失败" << XEND;
      return false;
    }

    XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移成功, 无数据迁移" << XEND;
    return true;
  }

  TMapExchangeUser mapUser;
  for (DWORD d = 0; d < set.size(); ++d)
  {
    BlobData oData;
    string data;
    data.assign((const char*)(set[d].getBin("data")), set[d].getBinSize("data"));
    if (uncompress(data, data) == false)
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 解压BlobData失败" << XEND;
      return false;
    }
    if (oData.ParseFromString(data) == false)
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 反序列化BlobData失败" << XEND;
      return false;
    }
    SExchangeUser& rData = mapUser[set[d].get<QWORD>("charid")];
    rData.oBase.set_charid(set[d].get<QWORD>("charid"));
    rData.oBase.set_rolelv(set[d].get<DWORD>("rolelv"));
    rData.oBase.set_name(set[d].getString("name"));
    rData.oData.CopyFrom(oData);
  }

  // collect all quest
  map<DWORD, QuestData> mapAccept;
  map<DWORD, QuestData> mapSubmit;

  const BlobQuest& rAccQuest = oAccData.quest();
  for (int i = 0; i < rAccQuest.accept_size(); ++i)
  {
    const QuestData& rAccept = rAccQuest.accept(i);
    mapAccept[rAccept.id()].CopyFrom(rAccept);
  }
  for (int i = 0; i < rAccQuest.submit_size(); ++i)
  {
    const QuestData& rSubmit = rAccQuest.submit(i);
    mapSubmit[rSubmit.id()].CopyFrom(rSubmit);
  }

  for (auto &m : mapUser)
  {
    const BlobQuest& rQuest = m.second.oData.quest();
    for (int i = 0; i < rQuest.accept_size(); ++i)
    {
      const QuestData& rAccept = rQuest.accept(i);
      const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(rAccept.id());
      if (pCFG == nullptr)
      {
        XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败, CharData->quest accept questid :" << rAccept.id() << "未在Table_Quest.txt表中找到" << XEND;
        continue;
      }
      if (QuestConfig::getMe().isShareQuest(pCFG->eType) == false)
        continue;
      QuestData& rCur = mapAccept[rAccept.id()];
      if (rAccept.step() >= rCur.step())
      {
        rCur.CopyFrom(rAccept);
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "CharData->quest accept 自身" << rCur.ShortDebugString() << "使用 charid :" << m.first << rAccept.ShortDebugString() << XEND;
      }
    }
    for (int i = 0; i < rQuest.submit_size(); ++i)
    {
      const QuestData& rData = rQuest.submit(i);
      const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(rData.id());
      if (pCFG == nullptr)
      {
        XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败, CharData->quest 自身 submit questid :" << rData.id() << "未在Table_Quest.txt表中找到" << XEND;
        continue;
      }
      if (QuestConfig::getMe().isShareQuest(pCFG->eType) == false)
        continue;
      mapSubmit[rData.id()].CopyFrom(rData);

      auto accept = mapAccept.find(rData.id());
      if (accept != mapAccept.end())
      {
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "CharData->quest submit 自身" << accept->second.ShortDebugString() << "charid :" << m.first << "已完成 删除可接列表" << XEND;
        mapAccept.erase(accept);
      }
    }
  }

  // reset quest
  map<DWORD, TSetDWORD> mapCookQuest;
  mapCookQuest[1] = TSetDWORD{600930002, 600930001};
  mapCookQuest[2] = TSetDWORD{600930002, 600930001};
  mapCookQuest[3] = TSetDWORD{600930002, 600930001, 600990001};
  mapCookQuest[4] = TSetDWORD{600930002, 600930001, 600990001, 601000001};
  mapCookQuest[5] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001};
  mapCookQuest[6] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001, 601020001};
  mapCookQuest[7] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001, 601020001, 601030001};
  mapCookQuest[8] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001, 601020001, 601030001, 601040001};
  mapCookQuest[9] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001, 601020001, 601030001, 601040001, 601050001};
  mapCookQuest[10] = TSetDWORD{600930002, 600930001, 600990001, 601000001, 601010001, 601020001, 601030001, 601040001, 601050001, 601060001};

  map<DWORD, TSetDWORD> mapTasteQuest;
  mapTasteQuest[1] = TSetDWORD{};
  mapTasteQuest[2] = TSetDWORD{};
  mapTasteQuest[3] = TSetDWORD{601080001};
  mapTasteQuest[4] = TSetDWORD{601080001, 601090001};
  mapTasteQuest[5] = TSetDWORD{601080001, 601090001, 601100001};
  mapTasteQuest[6] = TSetDWORD{601080001, 601090001, 601100001, 601110001};
  mapTasteQuest[7] = TSetDWORD{601080001, 601090001, 601100001, 601110001, 601120001};
  mapTasteQuest[8] = TSetDWORD{601080001, 601090001, 601100001, 601110001, 601120001, 601130001};
  mapTasteQuest[9] = TSetDWORD{601080001, 601090001, 601100001, 601110001, 601120001, 601130001, 601140001};
  mapTasteQuest[10] = TSetDWORD{601080001, 601090001, 601100001, 601110001, 601120001, 601130001, 601140001, 601150001};

  // get cook lv
  if (oAccData.food().cookerlv() != 0)
  {
    auto cook = mapCookQuest.find(oAccData.food().cookerlv());
    if (cook == mapCookQuest.end())
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败, 未找到 cooklv :" << oAccData.food().cookerlv() << "处理列表" << XEND;
      return false;
    }
    for (auto &s : cook->second)
    {
      auto accept = mapAccept.find(s);
      if (accept != mapAccept.end())
      {
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移, cooklv到达" << oAccData.food().cookerlv() << "questid :" << s << "从accept列表移除" << XEND;
        mapAccept.erase(accept);
      }

      auto submit = mapSubmit.find(s);
      if (submit == mapSubmit.end())
      {
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移, cooklv到达" << oAccData.food().cookerlv() << "questid :" << s << "添加submit列表" << XEND;
        mapSubmit[s].set_id(s);
      }
    }
  }

  // get taste lv
  if (oAccData.food().tasterlv() != 0)
  {
    auto taste = mapTasteQuest.find(oAccData.food().tasterlv());
    if (taste == mapTasteQuest.end())
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败, 未找到 tastelv :" << oAccData.food().cookerlv() << "处理列表" << XEND;
      return false;
    }
    for (auto &s : taste->second)
    {
      auto accept = mapAccept.find(s);
      if (accept != mapAccept.end())
      {
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移, tastelv到达" << oAccData.food().cookerlv() << "questid :" << s << "从accept列表移除" << XEND;
        mapAccept.erase(accept);
      }

      auto submit = mapSubmit.find(s);
      if (submit == mapSubmit.end())
      {
        XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移, tastelv到达" << oAccData.food().cookerlv() << "questid :" << s << "添加submit列表" << XEND;
        mapSubmit[s].set_id(s);
      }
    }
  }

  // init data
  oAccData.mutable_quest()->clear_accept();
  oAccData.mutable_quest()->clear_submit();

  for (auto &m : mapAccept)
  {
    const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移成功, 初始化acc_accept id :" << m.first << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }
    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      oAccData.mutable_quest()->add_accept()->CopyFrom(m.second);
      XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移成功, 初始化acc_accept id :" << m.first << "进入共享accept任务列表" << XEND;
    }
  }
  for (auto &m : mapSubmit)
  {
    const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移成功, 初始化acc_submit id :" << m.first << "未在 Table_Quest.txt 表中找到" << XEND;
      continue;
    }
    if (QuestConfig::getMe().isShareQuest(pCFG->eType) == true)
    {
      oAccData.mutable_quest()->add_submit()->CopyFrom(m.second);
      XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移成功, 初始化acc_submit id :" << m.first << "进入共享submit任务列表" << XEND;
    }
  }

  // set version
  m_oOldQuest.set_version(2);

  // serial NEW acc data
  m_oAccData.clear();
  if (oAccData.SerializeToString(&m_oAccData) == false)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 序列化AccData失败" << XEND;
    return false;
  }

  // save acc data
  if (saveAccData() == false)
  {
    XERR << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, 保存AccData失败" << XEND;
    return false;
  }

  XLOG << "[玩家-料理任务迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移成功" << XEND;
  return true;
}

bool RecordUser::exchangeAccData()
{
  xField* pAccField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (pAccField == nullptr)
  {
    XERR << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩失败,未找到accbase数据库表" << XEND;
    return false;
  }

  // select acc data
  stringstream sstr;
  sstr << "accid = " << m_oBase.accid();

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pAccField, set, sstr.str().c_str(), nullptr);
  if (QWORD_MAX == ret || 0 == ret)
  {
    XERR << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩失败,查询accbase数据库表失败,ret :" << ret << XEND;
    return false;
  }

  m_oAccData.clear();
  m_oAccData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));

  XLOG << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩前大小" << m_oAccData.size() << XEND;
  if (compress(m_oAccData, m_oAccData) == false)
  {
    XERR << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩失败" << ret << XEND;
    return false;
  }
  XLOG << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩后大小" << m_oAccData.size() << XEND;

  // clear quest data
  XLOG << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "quest清空前大小" << m_oOldQuest.ByteSize() << XEND;
  m_oOldQuest.Clear();
  m_oOldQuest.set_version(3);
  XLOG << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "quest清空后大小" << m_oOldQuest.ByteSize() << XEND;

  // save acc data
  if (saveAccData() == false)
  {
    XERR << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩失败, 保存AccData失败" << XEND;
    return false;
  }

  XLOG << "[玩家-acc压缩]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "压缩成功" << XEND;
  return true;
}

bool RecordUser::achieve_patch_1()
{
  xField* pAccField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (pAccField == nullptr)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,未找到accbase数据库表" << XEND;
    return false;
  }
  xField* pCharField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pCharField == nullptr)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败, 未找到charbase数据库表" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "accid = " << m_oBase.accid();

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(pCharField, set, sstr.str().c_str(), nullptr);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败, 查询charbase失败" << XEND;
    return false;
  }
  if (ret == 0)
  {
    m_oOldQuest.set_version(4);

    if (saveAccData() == false)
    {
      XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败, 保存AccData失败" << XEND;
      return false;
    }

    XLOG << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行成功,未有角色需要刷新成就" << XEND;
    return true;
  }
  TMapExchangeUser mapUser;
  for (DWORD d = 0; d < set.size(); ++d)
  {
    BlobData oData;
    string data;
    data.assign((const char*)(set[d].getBin("data")), set[d].getBinSize("data"));
    if (uncompress(data, data) == false)
    {
      XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行成功, 解压BlobData失败" << XEND;
      return false;
    }
    if (oData.ParseFromString(data) == false)
    {
      XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行成功, 反序列化BlobData失败" << XEND;
      return false;
    }
    SExchangeUser& rData = mapUser[set[d].get<QWORD>("charid")];
    rData.oBase.set_charid(set[d].get<QWORD>("charid"));
    rData.oBase.set_rolelv(set[d].get<DWORD>("rolelv"));
    rData.oBase.set_name(set[d].getString("name"));
    rData.oData.CopyFrom(oData);
  }

  set.clear();
  ret = thisServer->getDBConnPool().exeSelect(pAccField, set, sstr.str().c_str(), nullptr);
  if (QWORD_MAX == ret || 0 == ret)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,查询accbase数据库表失败,ret :" << ret << XEND;
    return false;
  }

  m_oAccData.clear();
  m_oAccData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));

  if (uncompress(m_oAccData, m_oAccData) == false)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,解压acc失败" << ret << XEND;
    return false;
  }

  BlobAccData oAccData;
  if (oAccData.ParseFromString(m_oAccData) == false)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,反序列化acc失败" << ret << XEND;
    return false;
  }

  map<DWORD, AchieveDBItem> mapAchieveItem;
  for (int i = 0; i < oAccData.achieve().items_size(); ++i)
  {
    const AchieveDBItem& rItem = oAccData.achieve().items(i);
    mapAchieveItem[rItem.id()].CopyFrom(rItem);
  }
  TSetDWORD setSubmitQuest;
  for (int i = 0; i < oAccData.quest().submit_size(); ++i)
    setSubmitQuest.insert(oAccData.quest().submit(i).id());
  for (auto &m : mapUser)
  {
    const BlobQuest& rQuest = m.second.oData.quest();
    for (int i = 0; i < rQuest.submit_size(); ++i)
      setSubmitQuest.insert(rQuest.submit(i).id());
  }

  DWORD dwNow = xTime::getCurSec();
  const TVecAchieveCFG& vecCFG = AchieveConfig::getMe().getAchieveCond(EACHIEVECOND_QUEST_SUBMIT);
  for (auto &v : vecCFG)
  {
    AchieveDBItem& rItem = mapAchieveItem[v.dwID];
    rItem.set_id(v.dwID);

    DWORD dwProcess = 0;
    for (auto &quest : v.stCondition.vecParams)
    {
      if (setSubmitQuest.find(quest) != setSubmitQuest.end())
        ++dwProcess;
    }
    rItem.set_process(dwProcess);
    if (dwProcess >= v.getProcess())
    {
      rItem.set_finishtime(dwNow);
      rItem.set_process(v.getProcess());
    }
    else
    {
      rItem.set_finishtime(0);
    }
    XLOG << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "id :" << v.dwID << "刷新状态" << rItem.ShortDebugString() << XEND;
  }

  BlobAchieve* pAchieve = oAccData.mutable_achieve();
  pAchieve->clear_items();
  for (auto &m : mapAchieveItem)
    pAchieve->add_items()->CopyFrom(m.second);

  if (oAccData.SerializeToString(&m_oAccData) == false)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,序列化acc数据失败" << XEND;
    return false;
  }

  if (compress(m_oAccData, m_oAccData) == false)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败,压缩acc数据失败" << XEND;
    return false;
  }

  // clear quest data
  m_oOldQuest.set_version(4);

  // save acc data
  if (saveAccData() == false)
  {
    XERR << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行失败, 保存AccData失败" << XEND;
    return false;
  }

  XLOG << "[玩家-成就补丁1]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "执行成功" << XEND;
  return true;
}

bool RecordUser::exchangeStore()
{
  xTime frameTime;
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
  if (pField == nullptr)
  {
    XLOG << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "未获取到数据库表char_store,直接完成本次补丁" << XEND;
    return true;
  }

  stringstream sstr;
  sstr << "accid=" << m_oBase.accid();
  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str(), NULL);
  if (QWORD_MAX == retNum)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,查询失败, ret :" << retNum << XEND;
    return false;
  }
  if (retNum == 0)
  {
    XLOG << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具中未包含任何道具,直接完成本地补丁" << XEND;
    return true;
  }

  string accdata;
  if (uncompress(m_oAccData, accdata) == false)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,解压accdata失败" << XEND;
    return false;
  }

  BlobAccData oAccData;
  if (oAccData.ParseFromString(accdata) == false)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,反序列化accdata失败" << XEND;
    return false;
  }

  PackageData* pStoreData = nullptr;
  BlobPack* pAccPack = oAccData.mutable_pack();
  for (int i = 0; i < pAccPack->datas_size(); ++i)
  {
    PackageData* pData = pAccPack->mutable_datas(i);
    if (pData->type() == EPACKTYPE_STORE)
    {
      pStoreData = pData;
      break;
    }
  }
  if (pStoreData != nullptr)
  {
    pStoreData->Clear();
  }
  else
  {
    pStoreData = pAccPack->add_datas();
    pStoreData->set_type(EPACKTYPE_STORE);
  }

  for (QWORD q = 0; q < retNum; ++q)
  {
    ItemData* pData = pStoreData->add_items();
    string item;
    item.assign((const char *)set[q].getBin("item"), set[q].getBinSize("item"));
    if (pData->ParseFromString(item) == false)
    {
      XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,item反序列化失败" << XEND;
      continue;
    }
    pData->mutable_base()->set_isnew(false);
    XLOG << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具,被转换道具" << pData->ShortDebugString() << XEND;
  }

  retNum = thisServer->getDBConnPool().exeDelete(pField, sstr.str().c_str());
  if (retNum == QWORD_MAX)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,删除通用仓库道具失败" << XEND;
    return false;
  }

  if (oAccData.SerializeToString(&accdata) == false)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,序列化accdata失败" << XEND;
    return false;
  }
  if (compress(accdata, m_oAccData) == false)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,压缩accdata失败" << XEND;
    return false;
  }

  m_oOldQuest.set_version(5);
  if (saveAccData() == false)
  {
    XERR << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具失败,保存accdata失败" << XEND;
    return false;
  }

  XLOG << "[玩家-通用仓库转移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "转移char_store中道具成功,耗时" << frameTime.uElapse() << "微秒" << XEND;
  return true;
}

bool RecordUser::loadCharData()
{
  QWORD accid = m_oBase.accid();
  QWORD id = m_oBase.charid();

  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (field == nullptr)
  {
    XERR << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,未找到charbase数据库表" << XEND;
    return false;
  }
  char where[256] = {0};
  snprintf(where, sizeof(where), "accid=%llu AND charid=%llu", accid, id);

  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
  if (QWORD_MAX == retNum || 1 != retNum)
  {
    XERR << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,未找到该角色" << XEND;
    return false;
  }

  if (set[0].get<DWORD>("zoneid") != thisServer->getZoneID())
  {
    GCharWriter gChar(thisServer->getRegionID(), id);
    gChar.setZoneID(set[0].get<DWORD>("zoneid"));
    gChar.save();

    Cmd::ReconnectClientUserCmd send;
    send.charid = id;
    thisServer->sendCmdToSession(&send, sizeof(send));

    XERR << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "加载失败,角色zoneid :" << set[0].get<DWORD>("zoneid") << "和本区zoneid :" << thisServer->getZoneID() << "不一致" << XEND;
    return false;
  }

  m_dwSequence = set[0].get<DWORD>("sequence");

  // base data
  m_oBase.set_platformid(set[0].get<DWORD>("platformid"));
  m_oBase.set_zoneid(set[0].get<DWORD>("zoneid"));
  m_oBase.set_destzoneid(set[0].get<DWORD>("destzoneid"));
  m_oBase.set_originalzoneid(set[0].get<DWORD>("originalzoneid"));
  m_oBase.set_accid(set[0].get<QWORD>("accid"));
  m_oBase.set_charid(set[0].get<QWORD>("charid"));
  m_oBase.set_name(set[0].getString("name"));
  m_oBase.set_createtime(set[0].get<DWORD>("createtime"));
  m_oBase.set_mapid(set[0].get<DWORD>("mapid"));
  m_oBase.set_gender(static_cast<EGender>(set[0].get<WORD>("gender")));
  m_oBase.set_profession(static_cast<EProfession>(set[0].get<DWORD>("profession")));
  m_oBase.set_destprofession(static_cast<EProfession>(set[0].get<DWORD>("destprofession")));
  m_oBase.set_rolelv(set[0].get<WORD>("rolelv"));
  m_oBase.set_roleexp(set[0].get<QWORD>("roleexp"));
  m_oBase.set_charge(set[0].get<DWORD>("charge"));
  m_oBase.set_diamond(set[0].get<DWORD>("diamond"));
  m_oBase.set_silver(set[0].get<QWORD>("silver"));
  m_oBase.set_gold(set[0].get<DWORD>("gold"));
  m_oBase.set_garden(set[0].get<DWORD>("garden"));
  m_oBase.set_friendship(set[0].get<DWORD>("friendship"));
  m_oBase.set_onlinetime(xTime::getCurSec());//set[0].get<DWORD>("onlinetime"));
  m_oBase.set_offlinetime(set[0].get<DWORD>("offlinetime"));
  m_oBase.set_addict(set[0].get<WORD>("addict"));
  m_oBase.set_battletime(set[0].get<DWORD>("battletime"));
  m_oBase.set_rebattletime(set[0].get<DWORD>("rebattletime"));
  m_oBase.set_usedbattletime(set[0].get<DWORD>("usedbattletime"));
  m_oBase.set_addicttipstime(set[0].get<DWORD>("addicttipstime"));
  m_oBase.set_body(set[0].get<DWORD>("body"));
  m_oBase.set_gagtime(set[0].get<DWORD>("gagtime"));
  m_oBase.set_nologintime(set[0].get<DWORD>("nologintime"));
  m_oBase.set_hair(set[0].get<DWORD>("hair"));
  m_oBase.set_haircolor(set[0].get<DWORD>("haircolor"));
  m_oBase.set_lefthand(set[0].get<DWORD>("lefthand"));
  m_oBase.set_righthand(set[0].get<DWORD>("righthand"));
  m_oBase.set_head(set[0].get<DWORD>("head"));
  m_oBase.set_back(set[0].get<DWORD>("back"));
  m_oBase.set_face(set[0].get<DWORD>("face"));
  m_oBase.set_tail(set[0].get<DWORD>("tail"));
  m_oBase.set_mount(set[0].get<DWORD>("mount"));
  m_oBase.set_title(set[0].get<DWORD>("title"));
  m_oBase.set_eye(set[0].get<DWORD>("eye"));
  m_oBase.set_partnerid(set[0].get<DWORD>("partnerid"));
  m_oBase.set_portrait(set[0].get<DWORD>("portrait"));
  m_oBase.set_mouth(set[0].get<DWORD>("mouth"));
  m_oBase.set_maxpro(set[0].get<DWORD>("maxpro"));

  m_oCharData.assign((const char *)set[0].getBin("data"), set[0].getBinSize("data"));
  XLOG << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "data :" << m_oCharData.size() << "dbdata :" << set[0].getBinSize("data")<< XEND;

  // // 账号下所有角色最大等级
  // {
  //   xField field(REGION_DB, "charbase");
  //   field.m_list["maxbaselv"] = MYSQL_TYPE_NEWDECIMAL;
  //   stringstream maxlvsql;
  //   maxlvsql << "select max(rolelv) as maxbaselv from " << field.m_strDatabase.c_str() << "." << field.m_strTable.c_str() << " where accid=" << accid << " and charid!=" << m_qwDeleteCharID;
  //   xRecordSet maxlvset;
  //   QWORD maxlvret = thisServer->getDBConnPool().exeRawSelect(&field, maxlvset, maxlvsql.str());
  //   if (maxlvret != 1)
  //   {
  //     if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
  //     {
  //       XERR << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "查询最大角色等级失败" << XEND;
  //       return false;
  //     }
  //   }
  //   else
  //   {
  //     m_oBase.set_maxbaselv(maxlvset[0].get<DWORD>("maxbaselv"));
  //     XLOG << "[玩家-角色加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "查询最大角色等级" << m_oBase.maxbaselv() << XEND;
  //     /*BlobCredit credit;
  //     if (credit.ParseFromString(creditData) == false)
  //     {
  //       XERR << "[帐号登录]" << accid << id << "反序列化最大角色等级失败" << XEND;
  //       if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
  //         return false;
  //     }
  //     else
  //     {
  //       DWORD maxbaselv = maxlvset[0].get<DWORD>("maxbaselv");
  //       DWORD cur = now();
  //       if (xTime::getDayStart(cur, MAXBASELV_RESETTIME_OFFSET) >= xTime::getDayStart(credit.maxbaselv_resettime(), MAXBASELV_RESETTIME_OFFSET) + MAXBASELV_RESETTIME)
  //       {
  //         credit.set_maxbaselv(maxbaselv);
  //         credit.set_maxbaselv_resettime(cur);
  //         if (credit.SerializeToString(&creditData) == false)
  //         {
  //           XERR << "[帐号登录]" << accid << id << "序列化最大角色等级失败" << XEND;
  //           if (BaseConfig::getMe().getBranch() != BRANCH_PUBLISH)
  //             return false;
  //         }
  //       }
  //     }*/
  //   }
  // }

  // save online quick for social
  if (m_oBase.onlinetime() == m_oBase.offlinetime())
    m_oBase.set_onlinetime(m_oBase.offlinetime() + 1);
  saveRedis(EUSERDATATYPE_ONLINETIME);
  m_dwTimeTick = m_oBase.onlinetime() + USER_SAVE_TICK;

  // delete online map redis key
  string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ONLINE_MAPID, id);
  RedisManager::getMe().delData(key);
  return true;
}

bool RecordUser::loadAccData()
{
  xField *field = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (field == nullptr)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,未找到accbase数据库表" << XEND;
    return false;
  }

  QWORD accid = m_oBase.accid();
  QWORD id = m_oBase.charid();

  char where[256] = {0};
  snprintf(where, sizeof(where), "accid=%llu", accid);

  xRecordSet set;
  QWORD ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
  if (QWORD_MAX == ret)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,查询失败" << XEND;
    return false;
  }
  if (0 == ret)
  {
    xRecord record(field);
    record.put("accid", accid);
    QWORD insertRet = thisServer->getDBConnPool().exeInsert(record);
    if (QWORD_MAX == insertRet)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,初始化插入数据失败" << XEND;
      return false;
    }
    ret = thisServer->getDBConnPool().exeSelect(field, set, (const char *)where, NULL);
    if (QWORD_MAX == ret)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,初始化后查询数据失败" << XEND;
      return false;
    }
  }
  if (0 == ret)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,找不到账号数据" << XEND;
    return false;
  }

  xRecord record(field);
  QWORD charid = set[0].get<QWORD>("charid");
  if (charid)
  {
    if (charid != id)
    {
      Cmd::ReconnectClientUserCmd send;
      send.charid = id;
      thisServer->sendCmdToSession(&send, sizeof(send));
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,其他角色正在登陆,等待退出" << XEND;
      return false;
    }
    if (set[0].get<DWORD>("zoneid") != thisServer->getZoneID())
    {
      char updateWhere[256] = {0};
      snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=%llu", accid, charid);
      record.put("zoneid", thisServer->getZoneID());
      QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
      if (QWORD_MAX == updateRet)
      {
        XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,更新账号失败" << XEND;
        return false;
      }
      if (0==updateRet)
      {
        XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,更新账号失败,无该条件数据" << XEND;
        return false;
      }
    }
  }
  else
  {
    char updateWhere[256] = {0};
    snprintf(updateWhere, sizeof(updateWhere), "accid=%llu and charid=0", accid);
    record.put("charid", id);
    record.put("lastselect", id);
    record.put("zoneid", thisServer->getZoneID());
    QWORD updateRet = thisServer->getDBConnPool().exeUpdate(record, updateWhere);
    if (QWORD_MAX == updateRet)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,更新账号失败" << XEND;
      return false;
    }
    if (0==updateRet)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,更新账号失败,无该条件数据" << XEND;
      return false;
    }
  }
  {
    xRecordSet vSet;
    QWORD vRet = thisServer->getDBConnPool().exeSelect(field, vSet, (const char *)where, NULL);
    if (QWORD_MAX == vRet)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,查询数据库失败" << XEND;
      return false;
    }
    if (0 == vRet)
    {
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,查询数据库失败" << XEND;
      return false;
    }
    if (vSet[0].get<QWORD>("charid") != id)
    {
      Cmd::ReconnectClientUserCmd send;
      send.charid = id;
      thisServer->sendCmdToSession(&send, sizeof(send));
      XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,其他角色正在登陆,等待退出" << XEND;
      return false;
    }
  }
  m_qwDeleteCharID = set[0].get<QWORD>("deletecharid");
  if (m_qwDeleteCharID == id)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,正在登陆准备删除的角色" << XEND;
    return false;
  }

  m_oAcc.set_nologintime(set[0].get<DWORD>("nologintime"));

  string oldquest;
  oldquest.assign((const char*)set[0].getBin("quest"), set[0].getBinSize("quest"));
  m_oAccData.assign((const char*)set[0].getBin("credit"), set[0].getBinSize("credit"));

  m_oBase.set_maincharid(set[0].get<QWORD>("maincharid"));

  if (m_oOldQuest.ParseFromString(oldquest) == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,反序列化quest数据失败" << XEND;
    return false;
  }

  if (m_oOldQuest.version() == 0 && exchangeData() == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,首次转换数据失败" << XEND;
    return false;
  }
  if (m_oOldQuest.version() == 1 && exchangeFoodQuest() == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,首次转换料理任务失败" << XEND;
    return false;
  }
  if (m_oOldQuest.version() == 2 && exchangeAccData() == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,首次转换acc数据失败" << XEND;
    return false;
  }
  if (m_oOldQuest.version() == 3 && achieve_patch_1() == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,achieve_patch_1执行失败" << XEND;
    return false;
  }
  if (m_oOldQuest.version() == 4 && exchangeStore() == false)
  {
    XERR << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "加载失败,首次转换通用仓库数据执行失败" << XEND;
    return false;
  }

  XLOG << "[玩家-账号加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "加载成功, 数据大小 quest :" << oldquest.size() << "credit :" << m_oAccData.size() << XEND;
  return true;
}

/*bool RecordUser::loadStore()
{
  m_oStore.clear();

  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "char_store");
  if (pField == nullptr)
  {
    XERR << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "获取char_store失败" << XEND;
    return false;
  }

  stringstream sstr;
  sstr << "accid=" << m_oBase.accid();
  xRecordSet set;
  QWORD retNum = thisServer->getDBConnPool().exeSelect(pField, set, sstr.str().c_str(), NULL);
  if (QWORD_MAX == retNum)
  {
    XERR << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "查询失败 ret :" << retNum << XEND;
    return false;
  }

  PackageData oStoreData;
  oStoreData.set_type(EPACKTYPE_STORE);

  for (QWORD q = 0; q < retNum; ++q)
  {
    ItemData* pData = oStoreData.add_items();
    if (pData == nullptr)
    {
      XERR << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "创建ItemData protobuf失败" << XEND;
      continue;
    }

    string item;
    item.assign((const char *)set[q].getBin("item"), set[q].getBinSize("item"));
    if (pData->ParseFromString(item) == false)
    {
      XERR << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "序列化ItemData失败" << XEND;
      continue;
    }
    pData->mutable_base()->set_isnew(false);
  }
  if (oStoreData.SerializeToString(&m_oStore) == false)
  {
    XERR << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "序列化PackageData失败" << XEND;
    return false;
  }

  XLOG << "[玩家-仓库加载]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "成功加载" << XEND;
  return true;
}*/

bool RecordUser::saveCharData(bool isOffline)
{
  // get field - charbase
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "charbase");
  if (pField == NULL)
  {
    XERR << "[玩家-角色存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "获取数据库charbase失败" << XEND;
    return false;
  }

  // get sql record
  char where[32] = {0};
  snprintf(where, sizeof(where), "charid=%llu", static_cast<QWORD>(m_oBase.charid()));

  xRecord record(pField);
  record.put("guildid", m_oBase.guildid());
  record.put("mapid", m_oBase.mapid());
  record.put("gender", m_oBase.gender());
  record.put("profession", m_oBase.profession());
  record.put("rolelv", m_oBase.rolelv());
  record.put("roleexp", m_oBase.roleexp());
  record.put("charge", m_oBase.charge());
  record.put("diamond", m_oBase.diamond());
  record.put("silver", m_oBase.silver());
  record.put("gold", m_oBase.gold());
  record.put("garden", m_oBase.garden());
  record.put("friendship", m_oBase.friendship());
  record.put("onlinetime", m_oBase.onlinetime());
  record.put("offlinetime", m_oBase.offlinetime());
  record.put("createtime", m_oBase.createtime());
  record.put("addict", m_oBase.addict());
  record.put("battletime", m_oBase.battletime());
  record.put("addicttipstime", m_oBase.addicttipstime());
  record.put("rebattletime", m_oBase.rebattletime());
  record.put("usedbattletime", m_oBase.usedbattletime());
  record.put("hair", m_oBase.hair());
  record.put("haircolor", m_oBase.haircolor());
  record.put("lefthand", m_oBase.lefthand());
  record.put("righthand", m_oBase.righthand());
  record.put("body", m_oBase.body());
  record.put("head", m_oBase.head());
  record.put("back", m_oBase.back());
  record.put("face", m_oBase.face());
  record.put("tail", m_oBase.tail());
  record.put("mount", m_oBase.mount());
  record.put("title", m_oBase.title());
  record.put("eye", m_oBase.eye());
  record.put("partnerid", m_oBase.partnerid());
  record.put("portrait", m_oBase.portrait());
  record.put("clothcolor", m_oBase.clothcolor());
  record.put("mouth", m_oBase.mouth());
  record.put("maxpro", m_oBase.maxpro());
  record.put("gagtime", m_oBase.gagtime());
  record.put("nologintime", m_oBase.nologintime());
  if (m_oBase.zoneid())
    record.put("zoneid", m_oBase.zoneid());
  record.put("destzoneid", m_oBase.destzoneid());
  record.put("originalzoneid", m_oBase.originalzoneid());
  record.putBin("data", (unsigned char *)(m_oCharData.c_str()), m_oCharData.size());

  // update to db
  xTime frameTimer;
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家管理-角色存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储失败 ret :" << ret << XEND;
    return false;
  }

  XLOG << "[玩家管理-角色存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "存储成功 ret :" << ret << "data:" << m_oCharData.size() << "dbdata :" << record.getBinSize("data") << "耗时" << frameTimer.uElapse() << "微秒" << XEND;

  if (isOffline)
  {
    std::string update_sql; 
    if (thisServer->getDBConnPool().getUpdateString(record, where, update_sql))
    {
      addRollback(m_oBase.accid(), m_oBase.charid(), now(), ROLLBACK_TYPE::charbase, update_sql);
    }
  }
  return true;
}

bool RecordUser::saveAccData(bool isOffline)
{
  xField* pField = thisServer->getDBConnPool().getField(REGION_DB, "accbase");
  if (pField == nullptr)
  {
    XERR << "[玩家-账号存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储失败, 未找到 accbase 数据库表" << XEND;
    return false;
  }

  string oldquest;
  if (m_oOldQuest.SerializeToString(&oldquest) == false)
  {
    XERR << "[玩家-账号存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储失败, 原quest序列化失败" << XEND;
    return false;
  }

  char where[32] = {0};
  snprintf(where, sizeof(where), "accid=%llu", static_cast<QWORD>(m_oBase.accid()));

  xRecord record(pField);
  record.putBin("credit", (unsigned char*)(m_oAccData.c_str()), m_oAccData.size());
  record.putBin("quest", (unsigned char*)(oldquest.c_str()), oldquest.size());
  record.put("maincharid", m_oBase.maincharid());

  xTime frameTimer;
  QWORD ret = thisServer->getDBConnPool().exeUpdate(record, where);
  if (ret == QWORD_MAX)
  {
    XERR << "[玩家-账号存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储失败, ret :" << ret << XEND;
    return false;
  }
  XLOG << "[玩家-账号存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "存储成功 ret :" << ret << " 数据大小 quest :" << oldquest.size() << "credit :" << m_oAccData.size() << "耗时:" << frameTimer.uElapse() << "微妙" << XEND;

  if (isOffline && CommonConfig::m_bOpenRollback)
  {
    std::string update_sql; 
    if (thisServer->getDBConnPool().getUpdateString(record, where, update_sql))
    {
      addRollback(m_oBase.accid(), m_oBase.charid(), now(), ROLLBACK_TYPE::accbase, update_sql);
    }
  }
  return true;
}

void RecordUser::addRollback(QWORD accid, QWORD charid, DWORD timestamps, ROLLBACK_TYPE datatype, std::string &data)
{
  if (!CommonConfig::m_bOpenRollback) return;

  xField* pField = thisServer->m_oRollbackThread.getDBConnPool().getField(REGION_DB, "log_usr");
  if (pField == nullptr)
  {
    XERR << "[回档-保存]" << accid << charid << "未取到log_usr数据库表" << XEND;
    return;
  }

  xSQLAction *pAct = xSQLThread::create(pField);
  if (!pAct)
  {
    XERR << "[回档-保存]" << accid << charid << "创建xSQLAction失败" << XEND;
    return;
  }
  xRecord &record = pAct->m_oRecord;
  record.put("charid", charid);
  record.put("accid", accid);
  record.put("timestamps", timestamps);
  record.put("datatype", (DWORD)datatype);
  record.putBin("data", (unsigned char *)(data.c_str()), data.size());

  pAct->m_eType = xSQLType_Insert;
  thisServer->m_oRollbackThread.add(pAct);

  XLOG << "[回档-保存]" << accid << charid << "更新数据成功" << XEND;
}

bool RecordUser::exchangeScenery(const TMapExchangeUser& mapUser, BlobScenery* pAccScenery)
{
  if (pAccScenery == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->scenery为空" << XEND;
    return false;
  }

  map<DWORD, map<DWORD, SceneryItem>> mapMapItem;
  map<DWORD, SceneryItem> mapTotalItem;
  for (auto &m : mapUser)
  {
    const BlobScenery& rScenery = m.second.oData.scenery();

    if (rScenery.version() > pAccScenery->version())
      pAccScenery->set_version(rScenery.version());

    for (int i = 0; i < rScenery.list_size(); ++i)
    {
      const SceneryMapItem& rItem = rScenery.list(i);
      map<DWORD, SceneryItem>& mapItem = mapMapItem[rItem.mapid()];

      for (int j = 0; j < rItem.scenerys_size(); ++j)
      {
        const SceneryItem& rSItem = rItem.scenerys(j);
        mapItem[rSItem.sceneryid()].CopyFrom(rSItem);
      }
    }

    for (int i = 0; i < rScenery.items_size(); ++i)
    {
      const SceneryItem& rItem = rScenery.items(i);
      mapTotalItem[rItem.sceneryid()].CopyFrom(rItem);
    }
  }

  for (auto &m : mapMapItem)
  {
    SceneryMapItem* pItem = pAccScenery->add_list();
    pItem->set_mapid(m.first);
    for (auto &s : m.second)
      pItem->add_scenerys()->CopyFrom(s.second);
  }

  for (auto &m : mapTotalItem)
    pAccScenery->add_items()->CopyFrom(m.second);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->scenery迁移完成" << XEND;
  return true;
}

#include "ItemConfig.h"
#include "NpcConfig.h"
#include "MapConfig.h"
#include "ManualConfig.h"
#include "MiscConfig.h"
#include "MailManager.h"
bool RecordUser::exchangeManual(const TMapExchangeUser& mapUser, BlobManual* pAccManual, BlobUnsolvedPhoto* pAccPhoto)
{
  if (pAccManual == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->manual为空" << XEND;
    return false;
  }

  ManualData* pAccManualData = pAccManual->mutable_data();

  // collct max lv char
  QWORD qwManualCharID = 0;
  for (auto &m : mapUser)
  {
    const BlobManual& rManual = m.second.oData.manual();
    if (pAccManualData->level() > rManual.data().level())
      continue;
    if (pAccManualData->level() < rManual.data().level())
    {
      qwManualCharID = m.second.oBase.charid();
      pAccManualData->set_level(rManual.data().level());
      pAccManualData->set_point(rManual.data().point());
      continue;
    }
    if (pAccManualData->point() < rManual.data().point())
    {
      qwManualCharID = m.second.oBase.charid();
      pAccManualData->set_level(rManual.data().level());
      pAccManualData->set_point(rManual.data().point());
      continue;
    }

    if (qwManualCharID == 0)
    {
      qwManualCharID = m.second.oBase.charid();
      pAccManualData->set_level(rManual.data().level());
      pAccManualData->set_point(rManual.data().point());
    }

    auto max_user = mapUser.find(qwManualCharID);
    if (max_user == mapUser.end())
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->manual未找到最大等级玩家数据" << XEND;
      return false;
    }
    if (max_user->second.oBase.rolelv() < m.second.oBase.rolelv())
    {
      qwManualCharID = m.second.oBase.charid();
      pAccManualData->set_level(rManual.data().level());
      pAccManualData->set_point(rManual.data().point());
      continue;
    }
  }
  auto max_user = mapUser.find(qwManualCharID);
  if (max_user == mapUser.end())
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->manual未选择到最高等级冒险手册数据" << XEND;
    return false;
  }

  for (auto &m : mapUser)
  {
    const ManualData& rData = m.second.oData.manual().data();
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移AccData->manual charid :" << m.first << "baselv :" << m.second.oBase.rolelv() << "level :" << rData.level() << "point :" << rData.point() << XEND;
  }

  const BlobManual& rMaxManual = max_user->second.oData.manual();
  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "迁移AccData->manual 取charid :" << qwManualCharID << "level :" << rMaxManual.data().level() << "point :" << rMaxManual.data().point() << "作为主数据" << XEND;

  // set skill point
  pAccManualData->set_skillpoint(rMaxManual.data().skillpoint());

  // group
  TSetDWORD setGroupIDs;
  for (int i = 0; i < rMaxManual.data().groups_size(); ++i)
    setGroupIDs.insert(rMaxManual.data().groups(i).id());
  for (auto &m : mapUser)
  {
    if (m.first == max_user->first)
      continue;
    for (int i = 0; i < m.second.oData.manual().data().groups_size(); ++i)
    {
      DWORD dwID = m.second.oData.manual().data().groups(i).id();
      auto s = setGroupIDs.find(dwID);
      if (s != setGroupIDs.end())
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移AccData->manual groupid :" << *s << "重复被合并" << XEND;
        continue;
      }
      setGroupIDs.insert(dwID);
    }
  }
  pAccManual->mutable_data()->clear_groups();
  for (auto &s : setGroupIDs)
    pAccManual->mutable_data()->add_groups()->set_id(s);

  // collect max user manual item
  map<EManualType, map<DWORD, ManualSubItem>> mapManual;
  //map<EManualType, map<DWORD, ManualQuest>> mapQuest;
  map<QWORD, UnsolvedUserPhoto> mapPhoto;
  for (int i = 0; i < rMaxManual.data().items_size(); ++i)
  {
    const ManualItem& rItem = rMaxManual.data().items(i);

    map<DWORD, ManualSubItem>& mapSubItem = mapManual[rItem.type()];
    for (int j = 0; j < rItem.items_size(); ++j)
    {
      const ManualSubItem& rSubItem = rItem.items(j);
      ManualSubItem& rCurItem = mapSubItem[rSubItem.id()];
      rCurItem.CopyFrom(rSubItem);

      if (rItem.type() == EMANUALTYPE_SCENERY)
      {
        rCurItem.clear_data_params();
        for (int i = 0; i < rCurItem.params_size(); ++i)
        {
          stringstream sstr;
          sstr << rCurItem.params(i);
          rCurItem.add_data_params(sstr.str());
        }
        if (rCurItem.params_size() == 0)
        {
          rCurItem.add_data_params();
          rCurItem.add_data_params();
        }
        stringstream sstr;
        sstr << max_user->first;
        rCurItem.add_data_params(sstr.str());
        rCurItem.clear_params();
      }
    }

    /*map<DWORD, ManualQuest>& mapSubQuest = mapQuest[rItem.type()];
    for (int j = 0; j < rItem.quests_size(); ++j)
    {
      const ManualQuest& rQuest = rItem.quests(j);
      mapSubQuest[rQuest.id()].CopyFrom(rQuest);
    }*/
  }

  // combine manual item to max user
  map<DWORD, ItemData> mapLostItem;
  map<DWORD, DWORD> mapHeadUnlock;
  map<DWORD, DWORD> mapCardUnlock;
  for (auto &m : mapUser)
  {
    if (m.first == max_user->first)
      continue;

    const BlobManual& rManual = m.second.oData.manual();
    for (int i = 0; i < rManual.data().items_size(); ++i)
    {
      const ManualItem& rItem = rManual.data().items(i);

      /*map<DWORD, ManualQuest>& mapSubQuest = mapQuest[rItem.type()];
      for (int i = 0; i < rItem.quests_size(); ++i)
      {
        const ManualQuest& rQuest = rItem.quests(i);
        ManualQuest& r = mapSubQuest[rQuest.id()];

        r.set_process(r.process() + rQuest.process());
        if (rQuest.finish() == true)
          r.set_finish(rQuest.finish());
        if (rQuest.rewardget() == true)
          r.set_rewardget(rQuest.rewardget());
      }*/

      map<DWORD, ManualSubItem>& mapSubItem = mapManual[rItem.type()];
      for (int j = 0; j < rItem.items_size(); ++j)
      {
        const ManualSubItem& rSubItem = rItem.items(j);
        ManualSubItem& rCurItem = mapSubItem[rSubItem.id()];
        if (rCurItem.id() == 0) rCurItem.set_id(rSubItem.id());
        if (rCurItem.status() == EMANUALSTATUS_MIN) rCurItem.set_status(EMANUALSTATUS_DISPLAY);

        // collect manual quest
        map<DWORD, ManualQuest> mapTmpQuest;
        for (int i = 0; i < rCurItem.quests_size(); ++i)
        {
          const ManualQuest& rQuest = rCurItem.quests(i);
          mapTmpQuest[rQuest.id()].CopyFrom(rQuest);
        }

        // combine manual quest
        for (int i = 0; i < rSubItem.quests_size(); ++i)
        {
          const ManualQuest& rQuest = rSubItem.quests(i);
          const SManualQuestCFG* pCFG = ManualConfig::getMe().getManualQuestCFG(rQuest.id());
          if (pCFG == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移失败, AccData->manual 迁移questid :" << rQuest.id() << "未在 Table_AdventureAppend.txt 表中找到" << XEND;
            return false;
          }

          ManualQuest& rCur = mapTmpQuest[rQuest.id()];
          rCur.set_id(rQuest.id());
          rCur.set_process(rCur.process() + rQuest.process());

          if (rQuest.finish() == true)
            rCur.set_finish(rQuest.finish());
          if (rQuest.rewardget() == true)
            rCur.set_rewardget(rQuest.rewardget());

          if (pCFG->eQuestType != EMANUALQUEST_KILL || rCur.process() < pCFG->vecParams[0])
            continue;

          DWORD dwLeft = rCur.process() - pCFG->vecParams[0];
          rCur.set_process(pCFG->vecParams[0]);
          rCur.set_finish(true);

          for (DWORD id = rCur.id() + 1; id < rCur.id() + 3; ++id)
          {
            if (dwLeft <= 0)
              break;

            const SManualQuestCFG* pNextCFG = ManualConfig::getMe().getManualQuestCFG(id);
            if (pNextCFG == nullptr || pNextCFG->eQuestType != EMANUALQUEST_KILL || pNextCFG->dwTargetID != pCFG->dwTargetID)
            {
              XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
                << "迁移AccData->manual 迁移questid :" << id - 1 << "无后续追加,剩余" << dwLeft << "自动丢弃" << XEND;
              break;
            }

            ManualQuest& rNewQuest = mapTmpQuest[id];
            rNewQuest.set_id(id);

            if (rNewQuest.process() != 0)
            {
              XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
                << "迁移AccData->manual 迁移questid :" << id - 1 << "后续questid :" << id << "包含" << rNewQuest.process() << "剩余" << dwLeft << "->" << dwLeft + rNewQuest.process() << XEND;
              dwLeft += rNewQuest.process();
            }

            rNewQuest.set_process(dwLeft > pNextCFG->vecParams[0] ? pNextCFG->vecParams[0] : dwLeft);
            rNewQuest.set_finish(dwLeft > pNextCFG->vecParams[0]);

            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移AccData->manual 迁移questid :" << id - 1 << "新增加后续追加" << rNewQuest.ShortDebugString() << "剩余" << dwLeft << XEND;

            dwLeft = dwLeft > pNextCFG->vecParams[0] ? dwLeft - pNextCFG->vecParams[0] : 0;
          }
        }

        // init quest to proto
        rCurItem.clear_quests();
        for (auto &m : mapTmpQuest)
          rCurItem.add_quests()->CopyFrom(m.second);

        // manual item
        if (rItem.type() == EMANUALTYPE_SCENERY)
        {
          if (rCurItem.status() < rSubItem.status())
          {
            if (rCurItem.status() != EMANUALSTATUS_DISPLAY)
            {
              UnsolvedPhoto* pInfo = mapPhoto[max_user->first].add_photos();
              pInfo->set_id(rCurItem.id());
              pInfo->set_anglez(rCurItem.params_size() > 0 ? rCurItem.params(0) : 0);
              pInfo->set_time(rCurItem.params_size() > 1 ? rCurItem.params(1) : 0);
            }

            rCurItem.clear_data_params();
            for (int i = 0; i < rSubItem.params_size(); ++i)
            {
              stringstream sstr;
              sstr << rSubItem.params(i);
              rCurItem.add_data_params(sstr.str());
            }
            if (rSubItem.params_size() == 0)
            {
              rCurItem.add_data_params();
              rCurItem.add_data_params();
            }
            stringstream sstr;
            sstr << m.first;
            rCurItem.add_data_params(sstr.str());
            rCurItem.clear_params();
          }
          else
          {
            UnsolvedPhoto* pInfo = mapPhoto[m.first].add_photos();
            pInfo->set_id(rSubItem.id());
            pInfo->set_anglez(rSubItem.params_size() > 0 ? rSubItem.params(0) : 0);
            pInfo->set_time(rSubItem.params_size() > 1 ? rSubItem.params(1) : 0);
          }
        }
        else if (rItem.type() == EMANUALTYPE_FASHION || rItem.type() == EMANUALTYPE_CARD)
        {
          if (rCurItem.store() == true && rSubItem.store() == true)
          {
            ItemData& rData = mapLostItem[rSubItem.id()];
            rData.mutable_base()->set_count(rData.base().id() == 0 ? 1 : rData.base().count() + 1);
            rData.mutable_base()->set_id(rSubItem.id());
          }
          if (rCurItem.store() == false && rSubItem.store() == true)
            rCurItem.set_store(true);

          if (rItem.type() == EMANUALTYPE_CARD)
          {
            const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rCurItem.id());
            if (pCFG == nullptr || pCFG->eItemType < EITEMTYPE_CARD_WEAPON || pCFG->eItemType > EITEMTYPE_CARD_HEAD)
            {
              XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
                << "迁移失败, AccData->manual 迁移id :" << rCurItem.id() << "不是卡片" << XEND;
              return false;
            }
            if (rCurItem.status() >= EMANUALSTATUS_UNLOCK_CLIENT && rSubItem.status() >= EMANUALSTATUS_UNLOCK_CLIENT)
              mapCardUnlock[rCurItem.id()]++;
          }
          else if (rItem.type() == EMANUALTYPE_FASHION)
          {
            if (rCurItem.status() >= EMANUALSTATUS_UNLOCK_CLIENT && rSubItem.status() >= EMANUALSTATUS_UNLOCK_CLIENT)
              mapHeadUnlock[rCurItem.id()]++;
          }
        }
        else
        {
          if (rSubItem.unlock() == true)
            rCurItem.set_unlock(rSubItem.unlock());
        }

        bool bLost = rCurItem.status() < EMANUALSTATUS_UNLOCK_CLIENT && rSubItem.status() > EMANUALSTATUS_UNLOCK_CLIENT;
        if (rCurItem.status() < rSubItem.status())
          rCurItem.set_status(rSubItem.status());

        if (!bLost)
          continue;

        EManualType eType = rItem.type();
        if (eType == EMANUALTYPE_FASHION || eType == EMANUALTYPE_CARD || eType == EMANUALTYPE_EQUIP || eType == EMANUALTYPE_ITEM || eType == EMANUALTYPE_MOUNT || eType == EMANUALTYPE_COLLECTION)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rSubItem.id());
          if (pCFG == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移失败, AccData->manual补偿Item" << rSubItem.id() << "失败,未在 Table_Item.txt 表中找到" << XEND;
            return false;
          }
          if (pCFG->swAdventureValue > 0)
          {
            pAccManualData->set_point(pAccManualData->point() + pCFG->swAdventureValue);
            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移AccData->manual补偿Item" << rSubItem.id() << "point :" << pCFG->swAdventureValue << XEND;
          }
        }
        else if (eType == EMANUALTYPE_MONSTER || eType == EMANUALTYPE_NPC || eType == EMANUALTYPE_PET)
        {
          const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(rSubItem.id());
          if (pCFG == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移失败, AccData->manual补偿Npc" << rSubItem.id() << "失败,未在 Table_Npc.txt 表中找到" << XEND;
            return false;
          }
          if (pCFG->swAdventureValue > 0)
          {
            pAccManualData->set_point(pAccManualData->point() + pCFG->swAdventureValue);
            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移AccData->manual补偿Npc" << rSubItem.id() << "point :" << pCFG->swAdventureValue << XEND;
          }
        }
        else if (eType == EMANUALTYPE_MAP)
        {
          const SMapCFG* pBase = MapConfig::getMe().getMapCFG(rSubItem.id());
          if (pBase == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移失败, AccData->manual补偿Map" << rSubItem.id() << "失败,未在 Table_Map.txt 表中找到" << XEND;
            return false;
          }
          if (pBase->swAdventureValue > 0)
          {
            pAccManualData->set_point(pAccManualData->point() + pBase->swAdventureValue);
            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移AccData->manual补偿Map" << rSubItem.id() << "point :" << pBase->swAdventureValue << XEND;
          }
        }
        else if (eType == EMANUALTYPE_SCENERY)
        {
          const SceneryBase* pBase = TableManager::getMe().getSceneryCFG(rSubItem.id());
          if (pBase == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移失败, AccData->manual补偿Scenery" << rSubItem.id() << "失败,未在 Table_Viewspot.txt 表中找到" << XEND;
            return false;
          }
          if (pBase->getAdvectureValue() > 0)
          {
            pAccManualData->set_point(pAccManualData->point() + pBase->getAdvectureValue());
            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "迁移AccData->manual补偿Scenery" << rSubItem.id() << "point :" << pBase->getAdvectureValue() << XEND;
          }
        }
      }
    }
  }

  // init item to proto
  for (auto &m : mapManual)
  {
    ManualItem* pItem = pAccManualData->add_items();
    pItem->set_type(m.first);
    for (auto &sub : m.second)
      pItem->add_items()->CopyFrom(sub.second);

    /*map<DWORD, ManualQuest>& mapSubQuest = mapQuest[pItem->type()];
    for (auto &m : mapSubQuest)
      pItem->add_quests()->CopyFrom(m.second);*/
  }

  // init unsolved photo
  for (auto &m : mapPhoto)
  {
    auto user = mapUser.find(m.first);
    if (user == mapUser.end())
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败,AccData->manual合并未处理照片未找到 charid :" << m.first << XEND;
      continue;
    }
    UnsolvedUserPhoto* pPhoto = pAccPhoto->add_photos();
    pPhoto->set_charid(m.first);
    pPhoto->set_name(user->second.oBase.name());
    for (int i = 0; i < m.second.photos_size(); ++i)
    {
      const UnsolvedPhoto& rInfo = m.second.photos(i);
      pPhoto->add_photos()->CopyFrom(rInfo);
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->manual迁移中 charid :" << m.first << "景点" << rInfo.ShortDebugString() << "存入临时相册" << XEND;
    }
  }

  const SManualMiscCFG& rCFG = MiscConfig::getMe().getManualCFG();
  DWORD dwApology = 0;

  // send lost mail
  if (mapLostItem.empty() == false)
  {
    const MailBase* pHeadBase = TableManager::getMe().getMailCFG(rCFG.dwHeadReturnMailID);
    const MailBase* pCardBase = TableManager::getMe().getMailCFG(rCFG.dwCardReturnMailID);
    if (pHeadBase == nullptr || pCardBase == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 存储邮件补偿 mailid :" << rCFG.dwHeadReturnMailID << "or" << rCFG.dwHeadReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
      return false;
    }

    for (auto &m : mapLostItem)
    {
      const SManualReturnCFG* pCFG = ManualConfig::getMe().getManualReturnCFG(m.first);
      if (pCFG == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败 AccData->manual 存储邮件补偿 id :" << m.first << "未在 Table_ManualReturn.txt 表中找到" << XEND;
        if (thisServer->isOuter() == true)
          continue;
        else
          return false;
      }

      TVecItemInfo vecReturnItems = pCFG->vecItems;
      for (auto &v : vecReturnItems)
      {
        v.set_count(v.count() * m.second.base().count());
        if (v.id() == ITEM_APOLOGY)
          dwApology += v.count();
      }

      const SItemCFG* pItemCFG = ItemConfig::getMe().getItemCFG(m.first);
      if (pItemCFG == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败 AccData->manual 存储邮件补偿 id :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
        return false;
      }

      if (vecReturnItems.empty() == true)
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->manual迁移完成 存储邮件补偿" << m.second.ShortDebugString() << "无补偿道具" << XEND;
        continue;
      }

      stringstream sstr;
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->manual迁移完成 存储邮件补偿" << m.second.ShortDebugString() << "补偿道具";
      for (size_t i = 0; i < vecReturnItems.size(); ++i)
      {
        const ItemInfo& rItem = vecReturnItems[i];
        const SItemCFG* p = ItemConfig::getMe().getItemCFG(rItem.id());
        if (p == nullptr)
        {
          XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "迁移失败 AccData->manual 存储邮件补偿 补偿道具" << rItem.id() << "未在 Table_Item.txt 表中找到" << XEND;
          return false;
        }
        sstr << p->strNameZh << "x" << rItem.count();
        if (i != vecReturnItems.size() - 1)
          sstr << ", ";
        XLOG << rItem.ShortDebugString();
      }
      XLOG << XEND;

      const MailBase* pBase = pItemCFG->eItemType >= EITEMTYPE_CARD_WEAPON && pItemCFG->eItemType <= EITEMTYPE_CARD_HEAD ? pCardBase: pHeadBase;
      MailManager::getMe().sendMail(m_oBase.charid(), pBase->id, vecReturnItems, MsgParams(pItemCFG->strNameZh, sstr.str()), true, false);
    }
  }

  // send fashion unlock
  const MailBase* pHeadBase = TableManager::getMe().getMailCFG(rCFG.dwHeadUnlockReturnMailID);
  if (pHeadBase == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移失败 AccData->manual 头饰图鉴邮件补偿 mailid :" << rCFG.dwHeadUnlockReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
    return false;
  }
  for (auto &m : mapHeadUnlock)
  {
    const SManualReturnCFG* pCFG = ManualConfig::getMe().getManualReturnCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 头饰图鉴邮件补偿 id :" << m.first << "未在 Table_ManualReturn.txt 表中找到" << XEND;
      if (thisServer->isOuter() == true)
        continue;
      else
        return false;
    }
    const SItemCFG* pHeadCFG = ItemConfig::getMe().getItemCFG(m.first);
    if (pHeadCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 头饰图鉴邮件补偿 id :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    TVecItemInfo vecItems;
    if (m.second == 1)
      vecItems = pCFG->vecUnlock1Items;
    else if (m.second == 2)
      vecItems = pCFG->vecUnlock2Items;
    else
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 头饰图鉴邮件补偿 id :" << m.first << "count :" << m.second << "未在 Table_ManualReturn.txt 表中处理" << XEND;
      return false;
    }

    if (vecItems.empty() == true)
    {
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->manual迁移完成 头饰图鉴邮件补偿 id :" << m.first << "无补偿道具" << XEND;
      continue;
    }

    for (auto &v : vecItems)
    {
      if (v.id() == ITEM_APOLOGY)
        dwApology += v.count();
    }

    stringstream sstr;
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->manual迁移完成 头饰图鉴邮件补偿 id :" << m.first << "补偿道具";
    for (size_t i = 0; i < vecItems.size(); ++i)
    {
      const ItemInfo& rItem = vecItems[i];
      const SItemCFG* p = ItemConfig::getMe().getItemCFG(rItem.id());
      if (p == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败 AccData->manual 头饰图鉴邮件补偿 补偿道具" << rItem.id() << "未在 Table_Item.txt 表中找到" << XEND;
        return false;
      }
      sstr << p->strNameZh << "x" << rItem.count();
      if (i != vecItems.size() - 1)
        sstr << ", ";
      XLOG << rItem.ShortDebugString();
    }
    XLOG << XEND;

    MsgParams oParams;
    oParams.addString(pHeadCFG->strNameZh);
    oParams.addNumber(m.second);
    oParams.addString(sstr.str());

    MailManager::getMe().sendMail(m_oBase.charid(), pHeadBase->id, vecItems, oParams, true, false);
  }

  // send card unlock
  const MailBase* pCardBase = TableManager::getMe().getMailCFG(rCFG.dwCardUnlockReturnMailID);
  if (pCardBase == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移失败 AccData->manual 卡片图鉴邮件补偿 mailid :" << rCFG.dwHeadUnlockReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
    return false;
  }
  for (auto &m : mapCardUnlock)
  {
    const SManualReturnCFG* pCFG = ManualConfig::getMe().getManualReturnCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 卡片图鉴邮件补偿 id :" << m.first << "未在 Table_ManualReturn.txt 表中找到" << XEND;
      return false;
    }
    const SItemCFG* pCardCFG = ItemConfig::getMe().getItemCFG(m.first);
    if (pCardCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 卡片图鉴邮件补偿 id :" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    TVecItemInfo vecItems;
    if (m.second == 1)
      vecItems = pCFG->vecUnlock1Items;
    else if (m.second == 2)
      vecItems = pCFG->vecUnlock2Items;
    else
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 卡片图鉴邮件补偿 id :" << m.first << "count :" << m.second << "未在 Table_ManualReturn.txt 表中处理" << XEND;
      return false;
    }

    if (vecItems.empty() == true)
    {
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->manual迁移完成 卡片图鉴邮件补偿 id :" << m.first << "无补偿道具" << XEND;
      continue;
    }

    for (auto &v : vecItems)
    {
      if (v.id() == ITEM_APOLOGY)
        dwApology += v.count();
    }

    stringstream sstr;
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->manual迁移完成 卡片图鉴邮件补偿 id :" << m.first << "补偿道具";
    for (size_t i = 0; i < vecItems.size(); ++i)
    {
      const ItemInfo& rItem = vecItems[i];
      const SItemCFG* p = ItemConfig::getMe().getItemCFG(rItem.id());
      if (p == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败 AccData->manual 卡片图鉴邮件补偿 补偿道具" << rItem.id() << "未在 Table_Item.txt 表中找到" << XEND;
        return false;
      }
      sstr << p->strNameZh << "x" << rItem.count();
      if (i != vecItems.size() - 1)
        sstr << ", ";
      XLOG << rItem.ShortDebugString();
    }
    XLOG << XEND;

    MsgParams oParams;
    oParams.addString(pCardCFG->strNameZh);
    oParams.addNumber(m.second);
    oParams.addString(sstr.str());

    MailManager::getMe().sendMail(m_oBase.charid(), pCardBase->id, vecItems, oParams, true, false);
  }

  // send quality mail
  const MailBase* pQualityBase = TableManager::getMe().getMailCFG(rCFG.dwQualityReturnMailID);
  if (pQualityBase == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移失败 AccData->manual 品质邮件补偿 mailid :" << rCFG.dwQualityReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
    return false;
  }
  map<EQualityType, DWORD> mapCardQuality;
  for (auto &m : mapCardUnlock)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m.first);
    if (pCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 品质邮件补偿 卡片" << m.first << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    DWORD& rCount = mapCardQuality[pCFG->eQualityType];
    ++rCount;
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "AccData->manual 品质邮件补偿 卡片" << m.first << "品质" << pCFG->eQualityType << "数量" << rCount << XEND;
  }
  for (auto &m : mapCardQuality)
  {
    const ItemInfo* pItem = rCFG.getQualityReturn(m.first, m.second);
    if (pItem == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 品质邮件补偿 补偿道具" << m.first << m.second << "失败,未在 ServerGame -> quality_return 表中找到,不补偿" << XEND;
      continue;
    }

    if (pItem->id() == ITEM_APOLOGY)
      dwApology += pItem->count();

    const SItemCFG* p = ItemConfig::getMe().getItemCFG(pItem->id());
    if (p == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败 AccData->manual 品质邮件补偿 补偿道具" << pItem->id() << "未在 Table_Item.txt 表中找到" << XEND;
      return false;
    }

    stringstream sstr;
    sstr << p->strNameZh << "x" << pItem->count();

    MsgParams oParams;
    oParams.addString(rCFG.getQualityName(m.first));
    oParams.addNumber(m.second);
    oParams.addString(sstr.str());

    MailManager::getMe().sendMail(m_oBase.charid(), pQualityBase->id, TVecItemInfo{*pItem}, oParams, true, false);
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "AccData->manual迁移完成 品质邮件补偿 quality" << m.first << "count :" << m.second << "补偿:" << pItem->ShortDebugString() << XEND;
  }

  // send level mail
  const MailBase* pLevelBase = TableManager::getMe().getMailCFG(rCFG.dwLevelReturnMailID);
  if (pLevelBase == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移失败 AccData->manual 邮件等级补偿 mailid :" << rCFG.dwLevelReturnMailID << "未在 Table_Mail.txt 表中找到" << XEND;
    return false;
  }
  for (auto &m : mapUser)
  {
    if (m.first == max_user->first)
      continue;
    const ManualData& rData = m.second.oData.manual().data();
    DWORD dwNum = rCFG.getLevelReturn(rData.level());
    if (dwNum > 0)
    {
      ItemInfo oItem;
      oItem.set_id(ITEM_APOLOGY);
      oItem.set_count(dwNum);
      dwApology += oItem.count();

      const SItemCFG* p = ItemConfig::getMe().getItemCFG(ITEM_APOLOGY);
      if (p == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败 AccData->manual 邮件等级补偿 补偿道具" << ITEM_APOLOGY << "未在 Table_Item.txt 表中找到" << XEND;
        return false;
      }

      stringstream sstr;
      sstr << p->strNameZh << "x" << dwNum;
      MailManager::getMe().sendMail(m_oBase.charid(), pLevelBase->id, TVecItemInfo{oItem}, MsgParams(m.second.oBase.name(), sstr.str()), true, false);
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->manual迁移完成 邮件等级补偿 charid :" << m.first << "level :" << rData.level() << "补偿:" << dwNum << "个" << p->strNameZh << XEND;
    }
  }

  pAccManualData->set_exchange_time(xTime::getCurSec());
  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->manual迁移完成"
    << "level :" << pAccManualData->level() << "point :" << pAccManualData->point() << "apology :" << dwApology << XEND;
  return true;
}

bool RecordUser::exchangeTitle(const TMapExchangeUser& mapUser, BlobTitle* pAccTitle, BlobQuest* pAccQuest)
{
  if (pAccTitle == nullptr || pAccQuest == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->title为空" << XEND;
    return false;
  }

  map<DWORD, TitleData> mapTitle;
  for (auto &m : mapUser)
  {
    const BlobTitle& rTitle = m.second.oData.title();
    for (int i = 0; i < rTitle.datas_size(); ++i)
    {
      const TitleData& rData = rTitle.datas(i);
      mapTitle[rData.id()].CopyFrom(rData);
    }
  }

  map<DWORD, DWORD> mapQuestTitle;
  mapQuestTitle[390010001] = 1002;
  mapQuestTitle[390020001] = 1003;
  mapQuestTitle[390030007] = 1004;
  mapQuestTitle[390040005] = 1005;
  mapQuestTitle[390060011] = 1006;

  map<DWORD, TSetDWORD> mapTitleQuest;
  mapTitleQuest[1002] = TSetDWORD{390010001};
  mapTitleQuest[1003] = TSetDWORD{390020001};
  mapTitleQuest[1004] = TSetDWORD{390030001, 390030002, 390030003, 390030004, 390030005, 390030006, 390030007};
  mapTitleQuest[1005] = TSetDWORD{390040001, 390040002, 390040003, 390040004, 390040005};
  mapTitleQuest[1006] = TSetDWORD{390060001, 390060002, 390060003, 390060004, 390060005, 390060006, 390060007, 390060008, 390060009, 390060010, 390060011};

  // collect quest
  map<DWORD, QuestData> mapAccept;
  map<DWORD, QuestData> mapSubmit;
  for (int i = 0; i < pAccQuest->accept_size(); ++i)
  {
    const QuestData& rData = pAccQuest->accept(i);
    mapAccept[rData.id()].CopyFrom(rData);
  }

  for (int i = 0; i < pAccQuest->submit_size(); ++i)
  {
    const QuestData& rData = pAccQuest->submit(i);
    mapSubmit[rData.id()].CopyFrom(rData);
  }

  // collect NEW title
  DWORD dwNow = xTime::getCurSec();
  for (auto &m : mapQuestTitle)
  {
    if (mapSubmit.find(m.first) != mapSubmit.end() && mapTitle.find(m.second) == mapTitle.end())
    {
      TitleData& rData = mapTitle[m.second];
      rData.set_id(m.second);
      rData.set_createtime(dwNow);
    }
  }

  // init title to data
  for (auto &m : mapTitle)
  {
    pAccTitle->add_datas()->CopyFrom(m.second);
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->title迁移后含有称号:" << m.first << XEND;
  }

  // refresh quest
  for (auto &m : mapTitleQuest)
  {
    if (mapTitle.find(m.first) != mapTitle.end())
    {
      TSetDWORD& setIDs = mapTitleQuest[m.first];
      for (auto accept = mapAccept.begin(); accept != mapAccept.end();)
      {
        if (setIDs.find(accept->first) != setIDs.end())
        {
          XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->title 已包含" << m.first << "acceptid :" << accept->first << "被移除" << XEND;
          accept = mapAccept.erase(accept);
          continue;
        }
        ++accept;
      }
      for (auto &s : setIDs)
      {
        auto submit = mapSubmit.find(s);
        if (submit == mapSubmit.end())
        {
          XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->title 已包含" << m.first << "submitid :" << s << "被添加" << XEND;
          mapSubmit[s].set_id(s);
        }
      }
    }
  }

  // init quest
  pAccQuest->clear_accept();
  for (auto &m : mapAccept)
    pAccQuest->add_accept()->CopyFrom(m.second);
  pAccQuest->clear_submit();
  for (auto &m : mapSubmit)
    pAccQuest->add_submit()->CopyFrom(m.second);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->title迁移完成" << XEND;
  return true;
}

#include "RecipeConfig.h"
bool RecordUser::exchangeFood(const TMapExchangeUser& mapUser, BlobFood* pAccFood, BlobManual* pAccManual)
{
  if (pAccFood == nullptr || pAccManual == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->food为空" << XEND;
    return false;
  }

  // select max user by cookerlv
  QWORD qwMaxCharID = 0;
  for (auto &m : mapUser)
  {
    const BlobFood& rFood = m.second.oData.food();
    if (pAccFood->cookerlv() > rFood.cookerlv())
      continue;

    if (pAccFood->cookerlv() < rFood.cookerlv())
    {
      qwMaxCharID = m.first;
      pAccFood->set_cookerlv(rFood.cookerlv());
      pAccFood->set_cookerexp(rFood.cookerexp());
      continue;
    }
    if (pAccFood->cookerexp() < rFood.cookerexp())
    {
      qwMaxCharID = m.first;
      pAccFood->set_cookerlv(rFood.cookerlv());
      pAccFood->set_cookerexp(rFood.cookerexp());
      continue;
    }
    if (qwMaxCharID == 0)
    {
      qwMaxCharID = m.first;
      pAccFood->set_cookerlv(rFood.cookerlv());
      pAccFood->set_cookerexp(rFood.cookerexp());
    }
  }
  auto max_user = mapUser.find(qwMaxCharID);
  if (max_user == mapUser.end())
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->food未找到最大等级玩家数据" << XEND;
    return false;
  }

  for (auto &m : mapUser)
  {
    const BlobFood& rFood = m.second.oData.food();
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "迁移AccData->food charid :" << m.first << "cookerlv :" << rFood.cookerlv() << "cookerexp :" << rFood.cookerexp() << XEND;
  }
  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
    << "迁移AccData->food 取charid :" << max_user->first << "cookerlv :" << pAccFood->cookerlv() << "cookerexp :" << pAccFood->cookerexp() << "作为主数据" << XEND;

  // select max taster lv
  for (auto &m : mapUser)
  {
    const BlobFood& rFood = m.second.oData.food();
    if (pAccFood->tasterlv() > rFood.tasterlv())
      continue;

    if (pAccFood->tasterlv() < rFood.tasterlv())
    {
      pAccFood->set_tasterlv(rFood.tasterlv());
      pAccFood->set_tasterexp(rFood.tasterexp());
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移AccData->food charid :" << max_user->first << "tasterlv :" << pAccFood->tasterlv() << "tasterexp :" << pAccFood->tasterexp()
        << "被 charid :" << m.first << "tasterlv :" << rFood.tasterlv() << "tasterexp :" << rFood.tasterexp() << "覆盖" << XEND;
      continue;
    }
    if (pAccFood->tasterexp() < rFood.tasterexp())
    {
      pAccFood->set_tasterlv(rFood.tasterlv());
      pAccFood->set_tasterexp(rFood.tasterexp());
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移AccData->food charid :" << max_user->first << "tasterlv :" << pAccFood->tasterlv() << "tasterexp :" << pAccFood->tasterexp()
        << "被 charid :" << m.first << "tasterlv :" << rFood.tasterlv() << "tasterexp :" << rFood.tasterexp() << "覆盖" << XEND;
      continue;
    }
  }

  // combine recipes
  TSetDWORD setRecipes;
  for (int i = 0; i < max_user->second.oData.food().recipes_size(); ++i)
    setRecipes.insert(max_user->second.oData.food().recipes(i));
  for (auto &m : mapUser)
  {
    if (qwMaxCharID == m.first)
      continue;

    const BlobFood& rFood = m.second.oData.food();
    for (int i = 0; i < rFood.recipes_size(); ++i)
    {
      auto s = setRecipes.find(rFood.recipes(i));
      if (s == setRecipes.end())
      {
        setRecipes.insert(rFood.recipes(i));

        const SRecipeCFG* pCFG = RecipeConfig::getMe().getRecipeCFG(rFood.recipes(i));
        if (pCFG == nullptr)
        {
          XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "迁移失败, AccData->food补偿 charid :" << m.first << "recipe :" << rFood.recipes(i) << "未在Table_Recipe.txt表中找到" << XEND;
          return false;
        }

        pAccFood->set_cookerexp(pAccFood->cookerexp() + pCFG->m_dwUnlockExp);
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->food补偿 charid :" << m.first << "recipe :" << rFood.recipes(i) << "exp :" << pCFG->m_dwUnlockExp << XEND;

        while (true)
        {
          const SCookerLevel* pNextCFG = TableManager::getMe().getCookerLevelCFG(pAccFood->cookerlv() + 1);
          if (pNextCFG == nullptr)
            break;
          if (pAccFood->cookerexp() < pNextCFG->getNeedExp())
            break;
          pAccFood->set_cookerexp(pAccFood->cookerexp() - pNextCFG->getNeedExp());
          pAccFood->set_cookerlv(pAccFood->cookerlv() + 1);

          XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->food补偿 charid :" << m.first << "cookerlv" << pAccFood->cookerlv() - 1 << "->" << pAccFood->cookerlv() << "cookerexp :" << pAccFood->cookerexp() << XEND;
        }
      }
    }
  }

  // combine manual data
  map<EFoodDataType, map<DWORD, FoodSubData>> mapManualData;
  for (int i = 0; i < max_user->second.oData.food().manualdata_size(); ++i)
  {
    const FoodManualData& data = max_user->second.oData.food().manualdata(i);
    map<DWORD, FoodSubData>& mapSub = mapManualData[data.type()];

    for (int j = 0; j < data.datas_size(); ++j)
    {
      const FoodSubData& rSub = data.datas(j);
      mapSub[rSub.itemid()].CopyFrom(rSub);
      XDBG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->food charid :" << max_user->first << "type :" << data.type() << "item :" << rSub.ShortDebugString() << XEND;
    }
  }
  for (auto &m : mapUser)
  {
    if (qwMaxCharID == m.first)
      continue;
    const BlobFood& rFood = m.second.oData.food();
    for (int i = 0; i < rFood.manualdata_size(); ++i)
    {
      const FoodManualData& data = rFood.manualdata(i);

      map<DWORD, FoodSubData>& mapSub = mapManualData[data.type()];
      for (int j = 0; j < data.datas_size(); ++j)
      {
        const FoodSubData& rSub = data.datas(j);
        FoodSubData& rCur = mapSub[rSub.itemid()];
        rCur.set_itemid(rSub.itemid());

        bool bLost = rCur.status() < EFOODSTATUS_CLICKED && rSub.status() >= EFOODSTATUS_CLICKED;
        if (bLost)
        {
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rCur.itemid());
          if (pCFG == nullptr)
          {
            XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "AccData->food补偿 type :" << data.type() << "itemid :" << rCur.itemid() << "未在 Table_Item.txt 表中找到" << XEND;
            return false;
          }
          if (pCFG->swAdventureValue > 0)
          {
            pAccManual->mutable_data()->set_point(pAccManual->data().point() + pCFG->swAdventureValue);
            XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
              << "AccData->food补偿 type :" << data.type() << "itemid :" << rCur.itemid() << "冒险经验" << pCFG->swAdventureValue << "total :" << pAccManual->data().point() << XEND;
          }
        }

        if (rSub.status() > rCur.status())
        {
          XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->food type :" << data.type() << "自身" << rCur.ShortDebugString() << "使用 charid :" << m.first << rSub.ShortDebugString() << XEND;
          rCur.CopyFrom(rSub);
        }
      }
    }
  }
  pAccFood->clear_manualdata();
  for (auto &m : mapManualData)
  {
    FoodManualData* pData = pAccFood->add_manualdata();
    pData->set_type(m.first);
    for (auto &item : m.second)
      pData->add_datas()->CopyFrom(item.second);
  }

  pAccFood->clear_recipes();
  for (auto &s : setRecipes)
    pAccFood->add_recipes(s);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->food迁移完成" << XEND;
  return true;
}

bool RecordUser::exchangeQuest(const TMapExchangeUser& mapUser, BlobQuest* pAccQuest)
{
  if (pAccQuest == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->quest为空" << XEND;
    return false;
  }

  // combine other acc
  for (int i = 0; i < m_oOldQuest.accept_ids_size(); ++i)
    pAccQuest->add_process_acc(m_oOldQuest.accept_ids(i));
  for (int i = 0; i < m_oOldQuest.choice_ids_size(); ++i)
    pAccQuest->add_detail()->set_id(m_oOldQuest.choice_ids(i));

  // combine share quest
  map<DWORD, QuestData> mapAccept;
  map<DWORD, QuestData> mapSubmit;

  for (auto &m : mapUser)
  {
    const BlobQuest& rQuest = m.second.oData.quest();
    for (int i = 0; i < rQuest.accept_size(); ++i)
    {
      const QuestData& rData = rQuest.accept(i);
      const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(rData.id());
      if (pCFG == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败, AccData->quest accept questid :" << rData.id() << "未在Table_Quest.txt表中找到" << XEND;
          continue;
      }
      if (QuestConfig::getMe().isShareQuest(pCFG->eType) == false)
        continue;
      QuestData& rCur = mapAccept[rData.id()];
      if (rData.step() >= rCur.step())
      {
        rCur.CopyFrom(rData);
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->quest accept 自身" << rCur.ShortDebugString() << "使用 charid :" << m.first << rData.ShortDebugString() << XEND;
      }
    }

    for (int i = 0; i < rQuest.submit_size(); ++i)
    {
      const QuestData& rData = rQuest.submit(i);
      const SQuestCFG* pCFG = QuestConfig::getMe().getQuestCFG(rData.id());
      if (pCFG == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败, AccData->quest 自身 submit questid :" << rData.id() << "未在Table_Quest.txt表中找到" << XEND;
        continue;
      }
      if (QuestConfig::getMe().isShareQuest(pCFG->eType) == false)
        continue;
      mapSubmit[rData.id()].CopyFrom(rData);

      auto accept = mapAccept.find(rData.id());
      if (accept != mapAccept.end())
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->quest submit 自身" << accept->second.ShortDebugString() << "使用 charid :" << m.first << rData.ShortDebugString() << XEND;
        mapAccept.erase(accept);
      }
    }
  }

  pAccQuest->clear_accept();
  for (auto &m : mapAccept)
    pAccQuest->add_accept()->CopyFrom(m.second);
  pAccQuest->clear_submit();
  for (auto &m : mapSubmit)
    pAccQuest->add_submit()->CopyFrom(m.second);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->quest迁移完成" << XEND;
  return true;
}

#include "AchieveConfig.h"
bool RecordUser::exchangeAchieve(const TMapExchangeUser& mapUser, BlobAchieve* pAccAchieve, BlobManual* pAccManual)
{
  if (pAccAchieve == nullptr || pAccManual == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->achieve为空" << XEND;
    return false;
  }

  set<EAchieveCond> setAddConds = set<EAchieveCond>{
    EACHIEVECOND_ADDFRIEND, EACHIEVECOND_USER_CHAT, EACHIEVECOND_PLAYMUSIC, EACHIEVECOND_FERRISWHEEL, EACHIEVECOND_HAND, EACHIEVECOND_EXPRESSION, EACHIEVECOND_USER_RUNE,
    EACHIEVECOND_PET_CAPTURE_SUCCESS, EACHIEVECOND_PET_FEED, EACHIEVECOND_PET_DEAD, EACHIEVECOND_PET_TOUCH, EACHIEVECOND_PET_CAPTURE_FAIL, EACHIEVECOND_PET_GIFT, EACHIEVECOND_PET_HANDWALK,
    EACHIEVECOND_PET_TIME, EACHIEVECOND_PET_ADVENTURE_COUNT, EACHIEVECOND_COOKFOOD, EACHIEVECOND_EATFOOD, EACHIEVECOND_COSTSAVEHP, EACHIEVECOND_COSTSAVESP, EACHIEVECOND_TUTOR_GUIDE,
    EACHIEVECOND_PHOTO, EACHIEVECOND_GHOST_PHOTO, EACHIEVECOND_KILL_MONSTER, EACHIEVECOND_USEITEM, EACHIEVECOND_USE_SKILL, EACHIEVECOND_USER_DEAD, EACHIEVECOND_BATTLE_TIME,
    EACHIEVECOND_HELP_QUEST, EACHIEVECOND_HELP_QUEST, EACHIEVECOND_USER_TRANSFER, EACHIEVECOND_PVP, EACHIEVECOND_COMPOSE, EACHIEVECOND_REFINE_EQUIP, EACHIEVECOND_STRENGTH,
    EACHIEVECOND_ENCHANT, EACHIEVECOND_EQUIP_UPGRADE, EACHIEVECOND_GET_ITEM, EACHIEVECOND_WANTEDQUEST, EACHIEVECOND_REPAIR_SEAL, EACHIEVECOND_KPL_CONSUME, EACHIEVECOND_ITEM_GET,
    EACHIEVECOND_VEHICLE, EACHIEVECOND_USER_DAMAGE, EACHIEVECOND_MONEY_SHOP_BUY, EACHIEVECOND_MONEY_SHOP_SELL, EACHIEVECOND_MONEY_TRADE_BUY, EACHIEVECOND_MONEY_GET,
    EACHIEVECOND_MONEY_TRADE_SELL, EACHIEVECOND_TRADE_RECORD, EACHIEVECOND_MONEY_CHARGE
  };

  //TSetDWORD setUnsolvedIDs;

  TSetDWORD setExpressionIDs;
  TSetDWORD setPassDojoIDs;
  TSetDWORD setCatUnlockIDs;
  TSetDWORD setCollectionIDs;
  for (auto &m : mapUser)
  {
    const BlobShow& rShow = m.second.oData.show();
    for (int i = 0; i < rShow.expressions_size(); ++i)
      setExpressionIDs.insert(rShow.expressions(i));

    const BlobDojo& rDojo = m.second.oData.dojo();
    for (int i = 0; i < rDojo.completedid_size(); ++i)
      setPassDojoIDs.insert(rDojo.completedid(i));

    const BlobWeaponPet& rWeaponPet = m.second.oData.weaponpet();
    for (int i = 0; i < rWeaponPet.unlockids_size(); ++i)
      setCatUnlockIDs.insert(rWeaponPet.unlockids(i));

    for (int i = 0; i < pAccManual->data().items_size(); ++i)
    {
      const ManualItem& rItem = pAccManual->data().items(i);
      if (rItem.type() == EMANUALTYPE_COLLECTION)
      {
        for (int j = 0; j < rItem.items_size(); ++j)
        {
          const ManualSubItem& rSubItem = rItem.items(j);
          const SAchieveItemCFG* pCFG = AchieveConfig::getMe().getAchieveItemCFG(rSubItem.id());
          if (pCFG != nullptr && pCFG->eType == EACHIEVEITEMTYPE_CCRASTEHAM)
            setCollectionIDs.insert(rSubItem.id());
        }
        break;
      }
    }
  }

  auto manual_count = [&](EManualType eType, EManualStatus eStatus, EItemType eItemType) -> DWORD
  {
    DWORD dwCount = 0;
    for (int i = 0; i < pAccManual->data().items_size(); ++i)
    {
      const ManualItem& rItem = pAccManual->data().items(i);
      if (rItem.type() == eType)
      {
        for (int j = 0; j < rItem.items_size(); ++j)
        {
          const ManualSubItem& rSubItem = rItem.items(j);
          if (rSubItem.status() < eStatus)
            continue;

          if (eType != EMANUALTYPE_COLLECTION)
          {
            ++dwCount;
            continue;
          }
          const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(rSubItem.id());
          if (pCFG != nullptr)
          {
            if (eItemType != EITEMTYPE_MIN && pCFG->eItemType == eItemType)
              ++dwCount;
          }
        }
        break;
      }
    }
    return dwCount;
  };
  auto collection_count = [&](DWORD dwType) -> DWORD
  {
    DWORD dwCount = 0;
    for (int i = 0; i < pAccManual->data().items_size(); ++i)
    {
      const ManualItem& rItem = pAccManual->data().items(i);
      if (rItem.type() == EMANUALTYPE_COLLECTION)
      {
        for (int j = 0; j < rItem.items_size(); ++j)
        {
          const ManualSubItem& rSubItem = rItem.items(j);
          const SAchieveItemCFG* pCFG = AchieveConfig::getMe().getAchieveItemCFG(rSubItem.id());
          if (pCFG != nullptr && pCFG->eType == dwType)
            ++dwCount;
        }
        break;
      }
    }
    return dwCount;
  };

  auto func = [&](AchieveDBItem& rDest, const AchieveDBItem& rOri) -> bool
  {
    const SAchieveCFG* pCFG = AchieveConfig::getMe().getAchieveCFG(rOri.id());
    if (pCFG == nullptr)
    {
      XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "迁移失败, AccData->achieve, 采集id :" << rOri.id() << "未在 Table_Achievement.txt 表中找到" << XEND;
      return true;
    }

    if (rDest.id() == 0)
    {
      rDest.CopyFrom(rOri);
      return true;
    }

    if (rOri.reward_get() == true)
      rDest.set_reward_get(rOri.reward_get());

    if (setAddConds.find(pCFG->stCondition.eCond) != setAddConds.end())
    {
      if (rDest.process() != rDest.process() + rOri.process())
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "累加" << rDest.process() << "->" << rDest.process() + rOri.process() << XEND;
      }
      rDest.set_process(rDest.process() + rOri.process());
      return true;
    }

    if (pCFG->stCondition.eCond == EACHIEVECOND_EMOJI)
    {
      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->achieve, 采集id :" << rOri.id() << "表情总" << rDest.process() << "->" << setExpressionIDs.size() << XEND;
      rDest.set_process(setExpressionIDs.size());
      //setUnsolvedIDs.insert(rDest.id());
    }
    else if (pCFG->stCondition.eCond == EACHIEVECOND_DOJO)
    {
      if (pCFG->stCondition.vecParams.size() != 3)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "道场,参数数量不足" << XEND;
        return false;
      }

      if (pCFG->stCondition.vecParams[0] == 0)
      {
        if (rDest.process() != rDest.process() + rOri.process())
        {
          XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->achieve, 采集id :" << rOri.id() << "道场累加" << rDest.process() << "->" << rDest.process() + rOri.process() << XEND;
        }
        rDest.set_process(rDest.process() + rOri.process());
      }
      else if (pCFG->stCondition.vecParams[0] == 1)
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "道场总" << rDest.process() << "->" << setPassDojoIDs.size() << XEND;
        rDest.set_process(setPassDojoIDs.size());
        //setUnsolvedIDs.insert(rDest.id());
      }
    }
    else if (pCFG->stCondition.eCond == EACHIEVECOND_CAT)
    {
      if (pCFG->stCondition.vecParams.size() != 2)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败,AccData->achieve, 采集id :" << rOri.id() << "佣兵猫总,参数数量不足" << XEND;
        return false;
      }
      if (pCFG->stCondition.vecParams[0] == 2)
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "佣兵猫总" << rDest.process() << "->" << setCatUnlockIDs.size() << XEND;
        rDest.set_process(setCatUnlockIDs.size());
        //setUnsolvedIDs.insert(rDest.id());
      }
    }
    if (pCFG->stCondition.eCond == EACHIEVECOND_COLLECTION)
    {
      if (pCFG->stCondition.vecParams.size() != 2)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败,AccData->achieve, 采集id :" << rOri.id() << "珍藏品,参数数量不足" << XEND;
        return false;
      }
      rDest.set_process(collection_count(pCFG->stCondition.vecParams[0]));
    }
    if (pCFG->stCondition.eCond == EACHIEVECOND_MANUAL)
    {
      if (pCFG->stCondition.vecParams.size() != 3)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败,AccData->achieve, 采集id :" << rOri.id() << "珍藏品,参数数量不足" << XEND;
        return false;
      }
      rDest.set_process(manual_count(static_cast<EManualType>(pCFG->stCondition.vecParams[0]), EMANUALSTATUS_UNLOCK, static_cast<EItemType>(pCFG->stCondition.vecParams[1])));
    }
    else if (pCFG->stCondition.eCond == EACHIEVECOND_USER_HAIR)
    {
      if (pCFG->stCondition.vecParams.size() != 2)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "迁移失败,AccData->achieve, 采集id :" << rOri.id() << "发型总,参数数量不足" << XEND;
        return false;
      }
      if (pCFG->stCondition.vecParams[0] == 1)
      {
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "发型总" << rDest.process() << "->" << setCollectionIDs.size() << XEND;
        rDest.set_process(rDest.process() + rOri.process());
      }
    }
    else if (pCFG->stCondition.eCond == EACHIEVECOND_SEAT)
    {
      if (rOri.finishtime() != 0)
      {
        rDest.set_finishtime(rOri.finishtime());
        XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->achieve, 采集id :" << rOri.id() << "完成" << XEND;
      }
    }
    else
    {
      if (rDest.process() < rOri.process())
      {
        rDest.set_process(rOri.process());
        //setUnsolvedIDs.insert(rDest.id());
      }
    }

    return true;
  };

  // collect achieve data
  map<DWORD, AchieveDBItem> mapAchieve;
  for (auto &m : mapUser)
  {
    const BlobAchieve& rAchieve = m.second.oData.achieve();
    if (rAchieve.version() == 0)
    {
      for (int i = 0; i < rAchieve.data_size(); ++i)
      {
        const AchieveData& rData = rAchieve.data(i);
        for (int j = 0; j < rData.datas_size(); ++j)
        {
          const AchieveSubData& rSubData = rData.datas(j);
          for (int k = 0; k < rSubData.items_size(); ++k)
          {
            const AchieveItem& rItem = rSubData.items(k);

            AchieveDBItem rDBItem;
            rDBItem.set_id(rItem.id());
            rDBItem.set_process(rItem.process());
            rDBItem.set_finishtime(rItem.finishtime());
            rDBItem.set_reward_get(rItem.reward_get());

            if (func(mapAchieve[rDBItem.id()], rDBItem) == false)
            {
              XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
                << "迁移失败, AccData->achieve, 采集id :" << rDBItem.id() << "失败" << XEND;
              return false;
            }
          }
        }
      }
    }
    else
    {
      for (int i = 0; i < rAchieve.items_size(); ++i)
      {
        const AchieveDBItem& rDBItem = rAchieve.items(i);
        if (func(mapAchieve[rDBItem.id()], rDBItem) == false)
        {
          XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "迁移失败, AccData->achieve, 采集id :" << rDBItem.id() << "失败" << XEND;
          return false;
        }
      }
    }

    if (rAchieve.version() > pAccAchieve->version())
      pAccAchieve->set_version(rAchieve.version());
  }

  // refresh achieve process
  DWORD dwNow = xTime::getCurSec();
  const TMapCondAchCFG& mapCFG = AchieveConfig::getMe().getCondAchCFGList();
  for (auto &m : mapCFG)
  {
    const TVecAchieveCFG& vecCFG = m.second;
    DWORD dwLeft = 0;
    for (auto &v : vecCFG)
    {
      const SAchieveCFG& rCFG = v;

      auto item = mapAchieve.find(rCFG.dwID);
      if (item == mapAchieve.end())
      {
        //if (setUnsolvedIDs.find(v.dwID) != setUnsolvedIDs.end())
        //  continue;

        if (dwLeft != 0)
        {
          AchieveDBItem& rItem = mapAchieve[rCFG.dwID];
          rItem.set_id(rCFG.dwID);
          rItem.set_process(dwLeft);
          dwLeft = 0;
        }
      }
      item = mapAchieve.find(rCFG.dwID);
      if (item == mapAchieve.end())
        continue;
      if (item->second.finishtime() != 0 || rCFG.getProcess() > item->second.process())
        continue;

      if (rCFG.stCondition.eCond == EACHIEVECOND_DOJO)
      {
        if (rCFG.stCondition.vecParams.size() != 3)
        {
          XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "AccData->achieve, 刷新id :" << rCFG.dwID << "道场,参数数量不足" << XEND;
          return false;
        }
        if (rCFG.stCondition.vecParams[0] == 0)
          dwLeft = item->second.process() > rCFG.getProcess() ? item->second.process() - rCFG.getProcess() : dwLeft;
        else if (rCFG.stCondition.vecParams[0] == 1)
          dwLeft = item->second.process();

        if (item->second.process() > rCFG.getProcess())
          item->second.set_process(rCFG.getProcess());

        if (item->second.process() >= rCFG.getProcess() && item->second.finishtime() == 0)
          item->second.set_finishtime(dwNow);
      }
      else if (rCFG.stCondition.eCond == EACHIEVECOND_USER_HAIR)
      {
        if (rCFG.stCondition.vecParams.size() != 2)
        {
          XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
            << "迁移失败,AccData->achieve, 刷新id :" << rCFG.dwID << "发型总,参数数量不足" << XEND;
          return false;
        }
        if (rCFG.stCondition.vecParams[0] == 1)
          dwLeft = item->second.process() > rCFG.getProcess() ? item->second.process() - rCFG.getProcess() : dwLeft;
        else
          dwLeft = item->second.process();

        if (item->second.process() > rCFG.getProcess())
          item->second.set_process(rCFG.getProcess());

        if (item->second.process() >= rCFG.getProcess() && item->second.finishtime() == 0)
          item->second.set_finishtime(dwNow);
      }
      else if (rCFG.stCondition.eCond == EACHIEVECOND_SEAT)
      {
      }
      else
      {
        if (setAddConds.find(rCFG.stCondition.eCond) != setAddConds.end())
          dwLeft = item->second.process() > rCFG.getProcess() ? item->second.process() - rCFG.getProcess() : dwLeft;
        else
          dwLeft = item->second.process();

        if (item->second.process() > rCFG.getProcess())
          item->second.set_process(rCFG.getProcess());

        if (item->second.process() >= rCFG.getProcess() && item->second.finishtime() == 0)
          item->second.set_finishtime(dwNow);
      }

      XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
        << "AccData->achieve迁移,id :" << rCFG.dwID << "成功" << item->second.ShortDebugString() << XEND;
    }
  }

  // init achieve blob
  for (auto &m :mapAchieve)
    pAccAchieve->add_items()->CopyFrom(m.second);

  if (pAccAchieve->version() == 0)
    pAccAchieve->set_version(1);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->achieve迁移完成" << XEND;
  return true;
}

#include "MenuConfig.h"
bool RecordUser::exchangeMenu(const TMapExchangeUser& mapUser, BlobMenu* pAccMenu)
{
  if (pAccMenu == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->menu为空" << XEND;
    return false;
  }

  TSetDWORD setIDs;
  for (auto &m : mapUser)
  {
    const BlobMenu& rMenu = m.second.oData.menu();
    for (int i = 0; i < rMenu.list_size(); ++i)
    {
      const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(rMenu.list(i));
      if (pCFG == nullptr)
      {
        XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
          << "AccData->menu迁移失败 id :" << rMenu.list(i) << "未在 Table_Menu.txt 表中找到" << XEND;
        continue;
      }
      if (pCFG->acc)
        setIDs.insert(rMenu.list(i));
    }
  }

  for (auto &s : setIDs)
  {
    pAccMenu->add_list(s);
    XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name()
      << "AccData->menu迁移成功 id :" << s << "添加到acc_menu中" << XEND;
  }

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->menu迁移完成" << XEND;
  return true;
}

bool RecordUser::exchangePortrait(const TMapExchangeUser& mapUser, BlobPortrait* pAccPortrait)
{
  if (pAccPortrait == nullptr)
  {
    XERR << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "迁移失败, AccData->portrait为空" << XEND;
    return false;
  }

  TSetDWORD setPortraitIDs;
  TSetDWORD setFrameIDs;
  for (auto &m : mapUser)
  {
    const BlobPortrait& rPortrait = m.second.oData.portrait();
    for (int i = 0; i < rPortrait.unlockportrait_size(); ++i)
      setPortraitIDs.insert(rPortrait.unlockportrait(i));
    for (int i = 0; i < rPortrait.unlockframe_size(); ++i)
      setFrameIDs.insert(rPortrait.unlockframe(i));
  }

  pAccPortrait->clear_unlockportrait();
  for (auto &s : setPortraitIDs)
    pAccPortrait->add_unlockportrait(s);
  pAccPortrait->clear_unlockframe();
  for (auto &s : setFrameIDs)
    pAccPortrait->add_unlockframe(s);

  XLOG << "[玩家-数据迁移]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "AccData->portrait迁移完成" << XEND;
  return true;
}

bool RecordUser::saveRedis(EUserDataType eType /*= EUSERDATATYPE_MIN*/)
{
  GCharWriter gChar(thisServer->getRegionID(), m_oBase.charid());

  gChar.setVersion();

  if (eType == EUSERDATATYPE_ONLINETIME)
  {
    gChar.setOnlineTime(m_oBase.onlinetime());
  }
  else
  {
    gChar.setName(m_oBase.name());
    gChar.setAccID(m_oBase.accid());
    gChar.setCharID(m_oBase.charid());
    gChar.setBaseLevel(m_oBase.rolelv());
    gChar.setMapID(m_oBase.mapid());
    gChar.setPortrait(m_oRedis.portrait());
    gChar.setBody(m_oBase.body());
    gChar.setHead(m_oBase.head());
    gChar.setFace(m_oBase.face());
    gChar.setBack(m_oBase.back());
    gChar.setTail(m_oBase.tail());
    gChar.setHair(m_oBase.hair());
    gChar.setHairColor(m_oBase.haircolor());
    gChar.setClothColor(m_oRedis.clothcolor());
    gChar.setLeftHand(m_oBase.lefthand());
    gChar.setRightHand(m_oBase.righthand());
    gChar.setOnlineTime(m_oBase.onlinetime());
    gChar.setOfflineTime(m_oBase.offlinetime());
    gChar.setEye(m_oBase.eye());
    gChar.setMouth(m_oBase.mouth());
    gChar.setZoneID(m_oBase.zoneid());
    gChar.setOriginalZoneID(m_oBase.originalzoneid());
    gChar.setManualLv(m_oRedis.manuallv());
    gChar.setManualExp(m_oRedis.manualexp());
    gChar.setTitleID(m_oBase.title());
    DWORD dwQueryType = m_oRedis.querytype();
    if (dwQueryType > EQUERYTYPE_MIN && dwQueryType << EQUERYTYPE_MAX && EQueryType_IsValid(dwQueryType) == true)
      gChar.setQueryType(dwQueryType);
    gChar.setProfession(m_oBase.profession());
    gChar.setGender(m_oBase.gender());
    gChar.setBlink(m_oRedis.blink());
    gChar.setTutor(m_oRedis.canbetutor());
    gChar.setProfic(m_oRedis.profic());

    // 选角界面格子编号
    if (m_dwSequence)
    {
      gChar.setSequence(m_dwSequence);
    }
    // 坐骑
    gChar.setMount(m_oBase.mount());
    // 称号 冒险登记
    //gChar.setTitle(m_oBase.title());
    // 宠物
    gChar.setPartnerID(m_oBase.partnerid());
    // 封号时间
    gChar.setNologinTime(m_oBase.nologintime());
  }

  xTime frameTimer;
  if (gChar.save() == false)
  {
    XERR << "[玩家-redis存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储失败" << XEND;
    return false;
  }

  XLOG << "[玩家-redis存储]" << m_oBase.accid() << m_oBase.charid() << m_oBase.profession() << m_oBase.name() << "存储成功,耗时 :" << frameTimer.uElapse() << "微秒" << XEND;
  return true;
}

bool RecordUser::updataCharPrimaryId()
{

  //todo
  return true;
}


#include "PveCardConfig.h"
#include "MiscConfig.h"

PveCardConfig::PveCardConfig()
{

}

PveCardConfig::~PveCardConfig()
{

}

bool PveCardConfig::loadConfig()
{
  bool bCorrect = true;
  if (loadPveRaidConfig() == false)
    bCorrect = false;
  if (loadPveCardConfig() == false)
    bCorrect = false;
  if (loadPveCardEffectConfig() == false)
    bCorrect = false;
  if (loadPveCardGroupConfig() == false)
    bCorrect = false;

  return bCorrect;
}

bool PveCardConfig::loadPveRaidConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_PveRaid.txt"))
  {
    XERR << "[Table_PveRaid],加载配置Table_PveRaid.txt失败" << XEND;
      return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PveRaid", table);

  m_mapPveRaidCFG.clear();
  for (auto &m : table)
  {
    if (m_mapPveRaidCFG.find(m.first) != m_mapPveRaidCFG.end())
    {
      bCorrect = false;
      XERR << "[PveRaid], id duplicate!" << m.first << XEND;
      continue;
    }
    SPveRaidCFG& sCFG = m_mapPveRaidCFG[m.first];
    sCFG.dwID = m.first;
    sCFG.dwRaidID = m.second.getTableInt("RaidID");
    sCFG.dwMonsterNum = m.second.getTableInt("MonsterNum");
    sCFG.dwDifficulty = m.second.getTableInt("Difficulty");
    m.second.getMutableData("Reward").getIDList(sCFG.setRewardID);
  }

  if (bCorrect)
    XLOG << "[PveRaid], 加载Table_PveRaid.txt成功" << XEND;

  return bCorrect;
}

bool PveCardConfig::loadPveCardConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_PveCard.txt"))
  {
    XERR << "[Table_PveCard],加载配置Table_PveCard.txt失败" << XEND;
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PveCard", table);

  m_mapPveCardCFG.clear();
  m_mapPveCardType2IDs.clear();
  m_mapPveCardType2AllIDs.clear();
  for (auto &m : table)
  {
    // 不使用的配置
    if (m.second.getTableInt("Weight") == 0)
      continue;

    if (m_mapPveCardCFG.find(m.first) != m_mapPveCardCFG.end())
    {
      XERR << "[PveCard], id duplicate!" << m.first << XEND;
      bCorrect = false;
      continue;
    }

    SPveCardCFG& sCFG = m_mapPveCardCFG[m.first];
    sCFG.dwCardID = m.first;
    EPveCardType eType = getCardType(m.second.getTableString("Type"));
    if (eType == EPVECARDTYPE_MIN)
    {
      XERR << "[PveCard], 类型配置错误, id:" << m.first << XEND;
      bCorrect = false;
      continue;
    }
    sCFG.eType = eType;
    m.second.getMutableData("Difficulty").getIDList(sCFG.setDifficulty);
    m.second.getMutableData("Effect").getIDList(sCFG.vecEffectIDs);

    auto iter = m_mapPveCardType2IDs.find(eType);
    if (iter == m_mapPveCardType2IDs.end())
    {
      m_mapPveCardType2IDs[eType];
      iter = m_mapPveCardType2IDs.find(eType);
    }
    for (auto &s : sCFG.setDifficulty)
    {
      auto iter2 = iter->second.find(s);
      if (iter2 == iter->second.end())
      {
        iter->second[s];
        iter2 = iter->second.find(s);
      }
      iter2->second.insert(m.first);
    }

    m_mapPveCardType2AllIDs[eType].insert(m.first);
  }

  if (bCorrect)
    XLOG << "[PveCard], 加载Table_PveCard.txt成功" << XEND;

  return bCorrect;
}

bool PveCardConfig::loadPveCardEffectConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_PveCardEffect.txt"))
  {
    XERR << "[Table_PveCardEffect],加载配置Table_PveCardEffect.txt失败" << XEND;
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PveCardEffect", table);

  m_mapPveCardEffectCFG.clear();
  for (auto &m : table)
  {
    if (m_mapPveCardEffectCFG.find(m.first) != m_mapPveCardEffectCFG.end())
    {
      XERR << "[PveCardEffect], id duplicate!" << m.first << XEND;
      bCorrect = false;
      continue;
    }

    SPveCardEffectCFG& sCFG = m_mapPveCardEffectCFG[m.first];
    sCFG.dwEffectID = m.first;

    EPveCardTargetType eTargetType = getCardTargetType(m.second.getTableString("TargetType"));
    if (eTargetType == EPVECARDTARGET_MAX)
    {
      XERR << "[PveCardEffect], 未识别的TargetType, id:" << m.first << XEND;
      bCorrect = false;
      continue;
    }
    sCFG.eTargetType = eTargetType;
    sCFG.oTargetParam = m.second.getMutableData("TargetParam");

    EPveCardEffectType eEffectType = getCardEffectType(m.second.getTableString("EffectType"));
    if (eEffectType == EPVECAREFFEECT_MAX)
    {
      XERR << "[PveCardEffect], 未识别的EffctType, id:" << m.first << XEND;
      bCorrect = false;
      continue;
    }
    sCFG.eEffectType = eEffectType;
    sCFG.oEffectParam = m.second.getMutableData("EffectParam");
  }

  if (bCorrect)
    XLOG << "[PveCardEffect], 加载Table_PveCardEffect.txt成功" << XEND;

  return bCorrect;
}

bool PveCardConfig::loadPveCardGroupConfig()
{
  bool bCorrect = true;
  if (!xLuaTable::getMe().open("Lua/Table/Table_PveCardGroup.txt"))
  {
    XERR << "[Table_PveCardGroup],加载配置Table_PveCardGroup.txt失败" << XEND;
    return false;
  }
  xLuaTableData table;
  xLuaTable::getMe().getLuaTable("Table_PveCardGroup", table);

  m_mapCardGrpCFG.clear();
  m_mapPveGrpDif2Croups.clear();
  for (auto &m : table)
  {
    SPveCardGrpCFG& stCFG = m_mapCardGrpCFG[m.first];
    stCFG.dwGroupID = m.first;
    m.second.getMutableData("Difficulty").getIDList(stCFG.setDifficulty);
    m.second.getMutableData("CardIDs").getIDList(stCFG.vecCardIDs);

    for (auto &s : stCFG.setDifficulty)
    {
      auto it = m_mapPveGrpDif2Croups.find(s);
      if (it != m_mapPveGrpDif2Croups.end())
        it->second.insert(m.first);
      else
        m_mapPveGrpDif2Croups[s].insert(m.first);
    }
  }
  return bCorrect;
}

bool PveCardConfig::checkConfig()
{
  return true;
}

EPveCardType PveCardConfig::getCardType(const string& sType) const
{
  if (sType == "Monster")
    return EPVECARDTYPE_MONSTER;
  else if (sType == "Boss")
    return EPVECARDTYPE_BOSS;
  else if (sType == "Environment")
    return EPVECARDTYPE_ENV;
  else if (sType == "Item")
    return EPVECARDTYPE_ITEM;
  else if (sType == "Friend")
    return EPVECARDTYPE_FRIEND;

  return EPVECARDTYPE_MIN;
}

EPveCardTargetType PveCardConfig::getCardTargetType(const string& sType) const
{
  if (sType == "random_user")
    return EPVECARDTARGET_RANDOMUSER;
  else if (sType == "all_user")
    return EPVECARDTARGET_ALLUSER;
  return EPVECARDTARGET_MIN;
}

EPveCardEffectType PveCardConfig::getCardEffectType(const string& sType) const
{
  if (sType == "gm")
    return EPVECARDEFFECT_GM;
  else if (sType == "scenegm")
    return EPVECARDEFFECT_SCENEGM;
  else if (sType == "summon")
    return EPVECARDEFFECT_SUMMON;
  else if (sType == "addbuff")
    return EPVECARDEFFECT_ADDBUFF;
  else if (sType == "pvesummon")
    return EPVECARDEFFECT_PVESUMMON;

  return EPVECARDEFFECT_MIN;
}

const SPveRaidCFG* PveCardConfig::getPveRaidCFGByID(DWORD configid) const
{
  auto it = m_mapPveRaidCFG.find(configid);
  if (it != m_mapPveRaidCFG.end())
    return &(it->second);
  return nullptr;
}

const SPveCardCFG* PveCardConfig::getPveCardCFG(DWORD cardid) const
{
  auto it = m_mapPveCardCFG.find(cardid);
  if (it != m_mapPveCardCFG.end())
    return &(it->second);
  return nullptr;
}

void PveCardConfig::getIDsByTypeAndDif(EPveCardType eType, DWORD difficulty, TSetDWORD& ids)
{
  auto it = m_mapPveCardType2IDs.find(eType);
  if (it != m_mapPveCardType2IDs.end())
  {
    auto it2 = it->second.find(difficulty);
    if (it2 != it->second.end())
      ids.insert(it2->second.begin(), it2->second.end());
  }
}

const TSetDWORD& PveCardConfig::getAllCardByType(EPveCardType eType) const
{
  auto it = m_mapPveCardType2AllIDs.find(eType);
  if (it != m_mapPveCardType2AllIDs.end())
    return it->second;
  static const TSetDWORD emptyset;
  return emptyset;
}

void PveCardConfig::randSystemCard(bool bIgnoreTime /*=false*/)
{
  DWORD time = now();
  if (!bIgnoreTime && time < m_dwNextCreatTime)
    return;

  XLOG << "[Pve-卡牌创建], 开始创建" << XEND;

  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();

  DWORD weektime = xTime::getWeekStart(time, 3600 * 5);
  m_dwNextCreatTime = weektime + WEEK_T + rPveMiscCFG.dwWeekResetTime;
  srand(weektime);

  DWORD systemCardSuitNum = rPveMiscCFG.dwRaidCardSuitNum;//每个副本卡牌套数
  auto getRandomIDs = [&](const TSetDWORD& sourceIDs, DWORD num, bool bDiff, TVecDWORD& vecResult, const TSetDWORD& exclueIDs = TSetDWORD{}) -> bool
  {
    if (bDiff || !exclueIDs.empty())
    {
      TSetDWORD allIDs = sourceIDs;
      if (!exclueIDs.empty())
        for (auto &s : exclueIDs)
          allIDs.erase(s);

      for (DWORD i = 0; i < num; ++i)
      {
        auto s = randomStlContainer(allIDs);
        if (!s)
          return false;
        vecResult.push_back(*s);
        if (bDiff)
          allIDs.erase(*s);
      }
    }
    else
    {
      for (DWORD i = 0; i < num; ++i)
      {
        auto s = randomStlContainer(sourceIDs);
        if (!s)
          return false;
        vecResult.push_back(*s);
      }
    }
    return true;
  };
  auto getRandomIDsLimitNum = [&](const TSetDWORD& sourceIDs, DWORD num, DWORD limitSameNum, TVecDWORD& vecResult) -> bool
  {
    TSetDWORD allIDs = sourceIDs;
    map<DWORD, DWORD> tempID2AlreadyNum;

    for (DWORD i = 0; i < num; ++i)
    {
      if (allIDs.empty())
        return false;
      auto s = randomStlContainer(allIDs);
      if (!s)
        return false;
      vecResult.push_back(*s);

      auto it = tempID2AlreadyNum.find(*s);
      if (it == tempID2AlreadyNum.end())
        tempID2AlreadyNum[*s] = 1;
      else
      {
        it->second ++;
        if (it->second >= limitSameNum)
          allIDs.erase(*s);
      }
    }
    return true;
  };

  auto randCardByRaid = [&](SPveRaidCFG& stRaidCFG) -> bool
  {
    stRaidCFG.vecCardInfo.clear();

    TSetDWORD allBossCardIDs;
    TSetDWORD allMonsterIDs;
    TSetDWORD allItemIDs;
    TSetDWORD allEnvIDs;
    TSetDWORD allGroupIDs;

    getIDsByTypeAndDif(EPVECARDTYPE_BOSS, stRaidCFG.dwDifficulty, allBossCardIDs);
    getIDsByTypeAndDif(EPVECARDTYPE_MONSTER, stRaidCFG.dwDifficulty, allMonsterIDs);
    getIDsByTypeAndDif(EPVECARDTYPE_ITEM, stRaidCFG.dwDifficulty, allItemIDs);
    getIDsByTypeAndDif(EPVECARDTYPE_ENV, stRaidCFG.dwDifficulty, allEnvIDs);

    auto itg = m_mapPveGrpDif2Croups.find(stRaidCFG.dwDifficulty);
    if (itg != m_mapPveGrpDif2Croups.end())
      allGroupIDs.insert(itg->second.begin(), itg->second.end());

    // select unique boss
    TVecDWORD vecUniqueBossIDs;
    if (getRandomIDs(allBossCardIDs, systemCardSuitNum, true, vecUniqueBossIDs) == false || vecUniqueBossIDs.size() != systemCardSuitNum)
    {
      XERR << "[Pve-Card创建], 获取唯一boss卡随机失败" << XEND;
      return false;
    }
    TVecDWORD vecUniqueGroupIDs;
    if (getRandomIDs(allGroupIDs, systemCardSuitNum, true, vecUniqueGroupIDs) == false || vecUniqueGroupIDs.size() != systemCardSuitNum)
    {
      XERR << "[Pve-Card创建], 获取唯一小怪组合随机失败" << XEND;
      return false;
    }

    for (DWORD index = 0; index < systemCardSuitNum; ++index) // systemCardSuitNum 套卡牌
    {
      SCardRecordData stOneSuitCard;

      DWORD uniqBossID = vecUniqueBossIDs[index];
      stOneSuitCard.vecBossID.push_back(uniqBossID); //每套, 添加一张unique boss卡
      XLOG << "[Pve-Card创建],卡牌添加唯一boss, 卡牌index:" << index << "唯一boss卡:" << uniqBossID << XEND;

      TVecDWORD vecOtherBossIDs; //每套, 补足3张boss卡
      if (getRandomIDs(allBossCardIDs, 2, true, vecOtherBossIDs, TSetDWORD{uniqBossID}) == false)
      {
        XERR << "[Pve-Card创建], 补足boss卡随机失败, 唯一boss卡:" << uniqBossID << XEND;
        return false;
      }
      for (auto &v : vecOtherBossIDs)
      {
        stOneSuitCard.vecBossID.push_back(v);
        XLOG << "[Pve-Card创建],卡牌补足boss, 卡牌index:" << index << "boss卡:" << v << XEND;
      }

      // 每套, 添加唯一小怪组合 * 2
      DWORD grpid = vecUniqueGroupIDs[index];
      auto itg = m_mapCardGrpCFG.find(grpid);
      if (itg == m_mapCardGrpCFG.end())
      {
        XERR << "[Pve-Card创建], 小怪组合异常" << XEND;
        return false;
      }
      for (DWORD i = 0; i < 2; ++i)
      {
        stOneSuitCard.vecGroupID.push_back(grpid);
        for (auto v : itg->second.vecCardIDs)
          stOneSuitCard.vecGroupMonsterID.push_back(v);
      }
      XLOG << "[Pve-Card创建], 卡牌添加小怪组合, 卡牌index:" << index << "组合ID:" << grpid << XEND;

      // 每套, 添加小怪卡
      DWORD monstercardnum = rPveMiscCFG.dwAllMonsterCardNum - 2 * 3; // 小怪总数-小怪组合数*3
      if (getRandomIDsLimitNum(allMonsterIDs, monstercardnum, rPveMiscCFG.dwMaxSameMonsterNum, stOneSuitCard.vecMonsterID) == false || stOneSuitCard.vecMonsterID.size() != monstercardnum)
      {
        XERR << "[Pve-Card创建], 卡牌随机小怪失败, 卡牌index:" << index << XEND;
        return false;
      }

      // 21张环境卡, 至少8张环境， 至少4张道具, 相同卡牌最多3张
      DWORD sparenum = rPveMiscCFG.dwAllItemAndEnvNum - rPveMiscCFG.dwMinEnvCardNum - rPveMiscCFG.dwMinItemCardNum;
      DWORD randEnvNum = randBetween(0, sparenum);
      DWORD envCardNum = rPveMiscCFG.dwMinEnvCardNum  + randEnvNum; // 随机后的环境卡数量
      DWORD itemCardNum = rPveMiscCFG.dwMinItemCardNum  + sparenum - randEnvNum; //随机后的道具卡数量
      DWORD maxSameItemOrEnvNum = rPveMiscCFG.dwMaxSameEnvOrItemNum; // 环境卡和道具卡最多重复出现的数量

      // 选择环境卡
      if (getRandomIDsLimitNum(allEnvIDs, envCardNum, maxSameItemOrEnvNum, stOneSuitCard.vecEnvID) == false || stOneSuitCard.vecEnvID.size() != envCardNum)
      {
        XERR << "[Pve-Card创建], 卡牌随机环境卡失败, 卡牌index:" << index << XEND;
        return false;
      }
      // 选择道具卡
      if (getRandomIDsLimitNum(allItemIDs,itemCardNum, maxSameItemOrEnvNum, stOneSuitCard.vecItemID) == false || stOneSuitCard.vecItemID.size() != itemCardNum)
      {
        XERR << "[Pve-Card创建], 卡牌随机环境卡失败, 卡牌index:" << index << XEND;
        return false;
      }

      // print log
      {
        XLOG << "[Pve-Card创建], 卡牌创建成功, 卡牌index:" << index << "详细信息: "<< XEND;
        XLOG << "Boss卡:";
        for (auto &v : stOneSuitCard.vecBossID)
          XLOG << v << ", ";
        XLOG << XEND;
        XLOG << "小怪组合:" << grpid << XEND;
        XLOG << "小怪卡:";
        for (auto &v : stOneSuitCard.vecMonsterID)
          XLOG << v << ", ";
        XLOG << XEND;
        XLOG << "道具卡:";
        for (auto &v : stOneSuitCard.vecItemID)
          XLOG << v << ", ";
        XLOG << XEND;
        XLOG << "环境卡:";
        for (auto &v : stOneSuitCard.vecEnvID)
          XLOG << v << ", ";
        XLOG << XEND;
      }

      stOneSuitCard.vecAllCardIDs.insert(stOneSuitCard.vecAllCardIDs.end(), stOneSuitCard.vecBossID.begin(), stOneSuitCard.vecBossID.end());
      stOneSuitCard.vecAllCardIDs.insert(stOneSuitCard.vecAllCardIDs.end(), stOneSuitCard.vecGroupMonsterID.begin(), stOneSuitCard.vecGroupMonsterID.end());
      stOneSuitCard.vecAllCardIDs.insert(stOneSuitCard.vecAllCardIDs.end(), stOneSuitCard.vecMonsterID.begin(), stOneSuitCard.vecMonsterID.end());
      stOneSuitCard.vecAllCardIDs.insert(stOneSuitCard.vecAllCardIDs.end(), stOneSuitCard.vecEnvID.begin(), stOneSuitCard.vecEnvID.end());
      stOneSuitCard.vecAllCardIDs.insert(stOneSuitCard.vecAllCardIDs.end(), stOneSuitCard.vecItemID.begin(), stOneSuitCard.vecItemID.end());

      stRaidCFG.vecCardInfo.push_back(stOneSuitCard);
    }

    return true;
  };

  for (auto &m : m_mapPveRaidCFG)
  {
    if (randCardByRaid(m.second) == false)
    {
      XERR << "[Pve-卡牌创建], 创建失败, ID:" << m.first << XEND;
      continue;
    }
  }

  XLOG << "[Pve-卡牌创建], 创建成功" << XEND;
  srand(xTime::getCurUSec());
}

bool PveCardConfig::shuffleCard(DWORD pveRaidIndex, DWORD index, TVecDWORD& vecCardIDs) const
{
  auto it = m_mapPveRaidCFG.find(pveRaidIndex);
  if (it == m_mapPveRaidCFG.end())
    return false;
  if (index + 1 > it->second.vecCardInfo.size())
    return false;
  SCardRecordData stStaticCard = it->second.vecCardInfo[index];

  const SPveMiscCFG& rPveMiscCFG = MiscConfig::getMe().getMiscPveCFG();
  /*
  共60张卡牌，3张牌一组，共20组, 记为S={1..20}。
  3个boss卡分别在策划配置的指定组中(5,12,18), 记为boss组B1, B2, B3(1<= Bx < 20),
  boss组后一个组必放环境卡，记为环境组， B1+1, B2+2, B3+1
  S1 = S - {B1,B1+1, B2, B2+1,B3, B3+1}
  若一共有n个环境卡(6<=n<=9)，则再从S1中随机选取n-3个环境组， E1,E2,...,En-3作为环境组，每组放置一张环境卡。
  S2 = S1 - {E1,E2,..En-3}
  从S2中随机选择两组， M1, M2 作为小怪组合组,小怪组合组M1,M2已有3只怪物, 不再填充其他卡。
  剩余的组S3 = S2 - {M1,M2}, 作为自由组，自由组可以分配0~2张道具卡
  若一共有m个道具卡(12<=n<=15), m+n=21,其中boss组，B1,B2,B3一定放置一张道具卡
  剩余m-3张道具卡。
  S3自由组可以放置0~2张道具，boss组与环境组可以再放置0~1张道具，boss组与环境组合并记为S4
  将m-3张道具卡按权重分配到S3与S4中，遍历所有组，未满的组填充小怪卡。
  */

  std::random_shuffle(stStaticCard.vecMonsterID.begin(), stStaticCard.vecMonsterID.end());
  std::random_shuffle(stStaticCard.vecItemID.begin(), stStaticCard.vecItemID.end());
  std::random_shuffle(stStaticCard.vecEnvID.begin(), stStaticCard.vecEnvID.end());

  // check system card valid ..todo

  DWORD grpnum = (rPveMiscCFG.dwBossCardNum + rPveMiscCFG.dwAllMonsterCardNum + rPveMiscCFG.dwAllItemAndEnvNum) / 3; // : 60 / 3;
  std::vector<TVecDWORD> vecGroups;
  vecGroups.resize(grpnum);

  TSetDWORD allGroups;
  for (DWORD i = 0; i < grpnum; ++i)
    allGroups.insert(i);

  auto getRandomGrp = [&](const TSetDWORD& sourceSet, TSetDWORD& setResult, DWORD num, const TSetDWORD& exSet1, const TSetDWORD& exSet2 = TSetDWORD{}, const TSetDWORD& exSet3 = TSetDWORD{}) -> bool
  {
    TSetDWORD allset = sourceSet;
    for (auto &s : exSet1)
      allset.erase(s);
    for (auto &s : exSet2)
      allset.erase(s);
    for (auto &s : exSet3)
      allset.erase(s);

    if (allset.size() < num)
      return false;

    for (DWORD i = 0; i < num; ++i)
    {
      auto s = randomStlContainer(allset);
      if (!s)
        return false;
      setResult.insert(*s);
      allset.erase(*s);
    }

    return true;
  };

  auto addGroupValue = [&](DWORD grpIndex, const TVecDWORD& vecValue, DWORD valueIndex) -> bool
  {
    if (grpIndex >= vecGroups.size())
    {
      XERR << "[Pve-卡牌打乱], group index error" << index << XEND;
      return false;
    }
    if (valueIndex >= vecValue.size())
    {
      XERR << "[Pve-卡牌打乱], value index error" << valueIndex << "size:" << vecValue.size() << XEND;
      return false;
    }
    vecGroups[grpIndex].push_back(vecValue[valueIndex]);
    return true;
  };

  // 指定3个boss组
  TSetDWORD bossSet;
  for (auto &s : rPveMiscCFG.setBossGroups)
    bossSet.insert(s - 1); // 策划配置1作为开始

  if (stStaticCard.vecBossID.size() != bossSet.size() || bossSet.empty())
  {
    XERR << "[Pve-Card打乱], boss卡数量不匹配" << pveRaidIndex << index << XEND;
    return false;
  }
  DWORD tmpindex = 0;
  {
    DWORD uniqboss = *(bossSet.rbegin());
    if (addGroupValue(uniqboss, stStaticCard.vecBossID, 0) == false)//最后boss组放置uniqu boss
      return false;

    TVecDWORD vecOtherBoss;
    for (auto &s : bossSet)
      vecOtherBoss.push_back(s);
    std::random_shuffle(vecOtherBoss.begin(), vecOtherBoss.end());

    tmpindex = 1;
    for (auto &v : vecOtherBoss)
    {
      if (v == uniqboss)
        continue;
      if (addGroupValue(v, stStaticCard.vecBossID, tmpindex) == false)
        return false;
      tmpindex ++;
    }
  }

  // boss 组后一定环境
  if (stStaticCard.vecEnvID.size() < bossSet.size())
  {
    XERR << "[Pve-Card打乱], 环境卡数量错误" << pveRaidIndex << index << XEND;
    return false;
  }
  DWORD evnSpareSize = stStaticCard.vecEnvID.size() - bossSet.size();
  TSetDWORD envSet; //环境组 size = envsize
  TSetDWORD envSet2;
  for (auto &s : bossSet)
    envSet.insert(s + 1);

  allGroups.erase(0); // 第一组只能放小怪
  if (getRandomGrp(allGroups, envSet2, evnSpareSize, bossSet, envSet) == false)
  {
    XERR << "[Pve-Card打乱], 选择环境组失败" << pveRaidIndex << index << XEND;
    return false;
  }
  envSet.insert(envSet2.begin(), envSet2.end()); // 选择所有环境组

  // 环境组添加环境卡
  {
    if (stStaticCard.vecEnvID.size() < envSet.size())
    {
      XERR << "[Pve-Card打乱], 环境卡数量不匹配" << pveRaidIndex << index << XEND;
      return false;
    }
    tmpindex = 0;
    for (auto &s : envSet)
    {
      if (addGroupValue(s, stStaticCard.vecEnvID, tmpindex) == false) // add env card
        return false;
      tmpindex ++;
    }
  }

  TSetDWORD grpSet; // 小怪组合 size = 2
  if (getRandomGrp(allGroups, grpSet, 2, envSet, bossSet) == false)
  {
    XERR << "[Pve-Card打乱], 选择小怪组合组失败" << pveRaidIndex << index << XEND;
    return false;
  }
  // 添加小怪组合
  {
    if (stStaticCard.vecGroupMonsterID.size() < grpSet.size() * 3)
    {
      XERR << "[Pve-Card打乱], 小怪组合数量不匹配" << pveRaidIndex << index << XEND;
      return false;
    }
    tmpindex = 0;
    for (auto &s : grpSet)
    {
      while(vecGroups[s].size() < 3)
      {
        if (addGroupValue(s, stStaticCard.vecGroupMonsterID, tmpindex) == false)
          return false;
        tmpindex ++;
      }
    }
  }

  // boss 组里一定有道具
  // boss组添加道具卡
  {
    if (stStaticCard.vecItemID.size() < bossSet.size())
    {
      XERR << "[Pve-Card打乱], 道具卡数量不匹配" << pveRaidIndex << index << XEND;
      return false;
    }
    tmpindex = 0;
    for (auto &s : bossSet)
    {
      if (addGroupValue(s, stStaticCard.vecItemID, tmpindex) == false)// add item card
        return false;
      tmpindex ++;
    }
  }
  DWORD spareItemNum = stStaticCard.vecItemID.size() - bossSet.size();

  // 分配剩余道具卡到各组
  if (spareItemNum)
  {
    TSetDWORD allowOneItemGrpSet; // 可放置一个item的set
    allowOneItemGrpSet.insert(bossSet.begin(), bossSet.end());
    allowOneItemGrpSet.insert(envSet.begin(), envSet.end());
    TSetDWORD allowTwoItemGrpSet; // 可放置2个item的set
    for (DWORD i = 0; i < grpnum; ++i)
      allowTwoItemGrpSet.insert(i);
    for (auto &s : grpSet) // all - 删除小怪组合
      allowTwoItemGrpSet.erase(s);
    for (auto &s : allowOneItemGrpSet) //  - allowOneItemGrpSet
      allowTwoItemGrpSet.erase(s);

    // 分配道具卡
    while(spareItemNum)
    {
      DWORD cardindex = stStaticCard.vecItemID.size() - spareItemNum;
      if (stStaticCard.vecItemID.size() <= cardindex)
      {
        XERR << "[Pve-Card打乱], 道具卡数量不匹配" << pveRaidIndex << index << XEND;
        return false;
      }
      DWORD oneRate = allowOneItemGrpSet.size();
      DWORD twoRate = allowTwoItemGrpSet.size() * 2;
      DWORD rate = oneRate + twoRate;
      DWORD rand = randBetween(1, rate);
      if (rand <= oneRate)
      {
        auto s = randomStlContainer(allowOneItemGrpSet);
        if (!s)
          return false;
        if (addGroupValue(*s, stStaticCard.vecItemID, cardindex) == false)
          return false;
        allowOneItemGrpSet.erase(*s);
      }
      else
      {
        auto s = randomStlContainer(allowTwoItemGrpSet);
        if (!s)
          return false;
        if (addGroupValue(*s, stStaticCard.vecItemID, cardindex) == false)
          return false;
        allowOneItemGrpSet.insert(*s);
        allowTwoItemGrpSet.erase(*s);
      }
      spareItemNum --;
    }
  }

  tmpindex = 0;
  for (auto &v : vecGroups)
  {
    // 填充小怪
    while (v.size() < 3)
    {
      if (stStaticCard.vecMonsterID.size() <= tmpindex)
      {
        XERR << "[Pve-Card打乱], 小怪卡数量不匹配" << pveRaidIndex << index << XEND;
        return false;
      }
      v.push_back(stStaticCard.vecMonsterID[tmpindex]);
      tmpindex ++;
    }

    for (auto &va : v)
      vecCardIDs.push_back(va);
  }

  return true;
}

void PveCardConfig::formatCardInfo(QueryCardInfoCmd& cmd) const
{
  for (auto &m : m_mapPveRaidCFG)
  {
    PveCardInfo* pInfo = cmd.add_cards();
    if (pInfo == nullptr)
      continue;
    if (m.second.vecCardInfo.empty())
      continue;
    pInfo->set_index(m.first);
    for (auto &v : m.second.vecCardInfo[0].vecAllCardIDs) //原每套副本3套牌, 改为一套了
      pInfo->add_cardids(v);
  }
}


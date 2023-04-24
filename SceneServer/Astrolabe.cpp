#include "Astrolabe.h"
#include "SceneUser.h"
#include "MsgManager.h"
#include "UserConfig.h"
#include "Menu.h"

Astrolabes::Astrolabes(SceneUser* pUser) : m_pUser(pUser)
{
}

Astrolabes::~Astrolabes()
{
}

bool Astrolabes::load(const BlobAstrolabe& rData)
{
  m_mapType2Astr.clear();
  for (int i = 0; i < rData.datas_size(); ++i) {
    TMapAstrolabe& mapAstrolabe = m_mapType2Astr[rData.datas(i).type()];
    for (int j = 0; j < rData.datas(i).astrolabes_size(); ++j) {
      DWORD id = rData.datas(i).astrolabes(j).id();
      auto it = mapAstrolabe.find(id);
      if (it == mapAstrolabe.end()) {
        mapAstrolabe.insert(TMapAstrolabe::value_type(id, Astrolabe(m_pUser, id)));
        it = mapAstrolabe.find(id);
        if (it == mapAstrolabe.end())
          continue;
      }
      it->second.load(rData.datas(i).astrolabes(j));
    }
  }

  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  return true;
}

// 重置星盘返回点位列表，不直接给返回物品
bool Astrolabes::resetAll(std::vector<std::pair<DWORD, DWORD>>& ids)
{
  for(auto& t : m_mapType2Astr)
  {
    for(auto& m : t.second)
    {
      m.second.getIds(ids);
    }
  }

  return resetAll(true, false);
}

bool Astrolabes::loadProfessionData(const BlobAstrolabe& rData, const std::vector<std::pair<DWORD, DWORD>>& ids_old, bool& isReset, bool isNeedReset)
{
  ntfReset();
  m_mapType2Astr.clear();

  if(isNeedReset)
    isReset = true;
  else
  {
    // 激活目标星盘
    vector<pair<DWORD, DWORD>> ids_new;
    for(int i=0; i<rData.datas_size(); ++i)
    {
      for(int j=0; j<rData.datas(i).astrolabes_size(); ++j)
      {
        const AstrolabeData& aData = rData.datas(i).astrolabes(j);
        for(int k=0; k<aData.stars_size(); ++k)
        {
          ids_new.push_back(pair<DWORD, DWORD>(aData.id(), aData.stars(k).id()));
        }
      }
    }

    if(changeCost(ids_old, ids_new))
    {
      isReset = false;
      activateStar(ids_new, false, false);
    }
    else
      isReset = true;

    XLOG << "[星盘-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->name;
    XLOG << "old_ids:";
    for(auto& v1 : ids_old)
      XLOG << "(" << v1.first << "," << v1.second << ")";
    XLOG << "new_ids:";
    for(auto& v2 : ids_new)
      XLOG << "(" << v2.first << "," << v2.second << ")";
    XLOG << XEND;
  }

  // 返回物品
  if(isReset)
  {
    TVecItemInfo itemAdds;
    getCost(ids_old, itemAdds);
    if(!itemAdds.empty())
    {
      XLOG << "[星盘-多职业加载] 星盘重置返回道具:" << m_pUser->accid << m_pUser->id << m_pUser->name;
      for (auto& v : itemAdds) 
      {
        v.set_source(ESOURCE_ASTROLABE_RESET); // 设置source, 用于公会服判断该贡献不计算到玩家的周贡献和总贡献中
        XLOG << "id:" << v.id() << "count:" << v.count();
      }
      XLOG << XEND;
      m_pUser->getPackage().addItem(itemAdds, EPACKMETHOD_NOCHECK);
    }
  }

  sendAstrolabeData();
  XLOG << "[星盘-多职业加载]" << m_pUser->accid << m_pUser->id << m_pUser->name << "load astrolabe data success!" << XEND;
  return true;
}

// 检测并扣除切换星盘消耗
bool Astrolabes::changeCost(const vector<pair<DWORD, DWORD>>& ids_old, vector<pair<DWORD, DWORD>>& ids_new)
{
  TVecItemInfo itemAdds;
  TVecItemInfo itemDels;

  getCost(ids_old, itemAdds);
  getCost(ids_new, itemDels);

  auto subItem = [&](ItemInfo& info, TVecItemInfo& vecItem)
  {
    for(auto& v : vecItem)
    {
      if(info.id() != v.id())
        continue;

      if(info.count() < v.count())
        continue;

      info.set_count(info.count() - v.count());
      v.set_count(0);
    }
  };

  for(auto& v : itemAdds)
    subItem(v, itemDels);

  for(auto& v : itemDels)
    subItem(v, itemAdds);

  // 移除数量为0的元素
  TVecItemInfo itemValidAdds;
  for(auto& v : itemAdds)
  {
    if(v.count())
      itemValidAdds.push_back(v);
  }
  TVecItemInfo itemValidDels;
  for(auto& v : itemDels)
  {
    if(v.count())
      itemValidDels.push_back(v);
  }

  BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (!pMainPack)
  {
    XERR << "[星盘-检测切换职业消耗]" << m_pUser->accid << m_pUser->id << m_pUser->name << "get mainpack failed!" << XEND;
    return false;
  }

  if(!itemValidDels.empty())
  {
    if(!pMainPack->checkItemCount(itemValidDels))
      return false;

    pMainPack->reduceItem(itemValidDels, ESOURCE_ASTROLABE_ACTIVATE);
  }

  if(!itemValidAdds.empty())
  {
    XLOG << "[星盘-多职业加载] 星盘重置返回差额道具:" << m_pUser->accid << m_pUser->id << m_pUser->name;
    for (auto& v : itemValidAdds) 
    {
      v.set_source(ESOURCE_ASTROLABE_RESET); // 设置source, 用于公会服判断该贡献不计算到玩家的周贡献和总贡献中
      XLOG << "id:" << v.id() << "count:" << v.count();
    }
    XLOG << XEND;
    m_pUser->getPackage().addItem(itemValidAdds, EPACKMETHOD_NOCHECK);
  }
  return true;
}

bool Astrolabes::save(BlobAstrolabe* pBlob)
{
  if (pBlob == nullptr)
    return false;

  pBlob->Clear();

  for (auto &it : m_mapType2Astr) {
    AstrolabeMainData* pMainData = nullptr;

    for (auto &itAstr : it.second) {
      if (!itAstr.second.isNeedSave())
        continue;

      if (pMainData == nullptr) {
        pMainData = pBlob->add_datas();
        if (pMainData == nullptr) {
          XERR << "[星盘-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "星盘type:" << it.first << "protobuf error" << XEND;
          continue;
        }
      }

      // pMainData->set_type(it.first);
      pMainData->set_type(EASTROLABETYPE_PROFESSION);

      AstrolabeData* pAstrData = pMainData->add_astrolabes();
      if (pAstrData == nullptr) {
        XERR << "[星盘-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "星盘:" << itAstr.second.getId() << "protobuf error" << XEND;
        continue;
      }

      itAstr.second.save(pAstrData);
    }
  }

  XDBG << "[星盘-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "数据大小:" << pBlob->ByteSize() << XEND;
  return true;
}

Astrolabe* Astrolabes::getAstrolabe(DWORD id)
{
  auto it = m_mapType2Astr.find(getAstrolabeType(id));
  if (it == m_mapType2Astr.end())
    return nullptr;
  auto itAstr = it->second.find(id);
  if (itAstr == it->second.end())
    return nullptr;
  return &itAstr->second;
}

DWORD Astrolabes::getTypeBranch()
{
  DWORD tb = m_pUser->getProfesTypeBranch();
  if (tb == 0) {
    tb = MiscConfig::getMe().getAstrolabeCFG().getDefaultTypeBranch(m_pUser->getProfessionType());
  }
  return tb;
}

// 获取解锁特殊效果数量
DWORD Astrolabes::getEffectCnt(DWORD specId)
{
  DWORD cnt = 0;
  for (auto& it : m_mapType2EffectCnt) {
    auto v = it.second.find(specId);
    if (v != it.second.end())
      cnt += v->second;
  }
  return cnt;
}

// 增加特殊效果解锁数量
void Astrolabes::chgEffectCnt(EAstrolabeType type, DWORD specId, SDWORD cnt)
{
  auto it = m_mapType2EffectCnt.find(type);
  if (it == m_mapType2EffectCnt.end()) {
    if (cnt <= 0) return;
    m_mapType2EffectCnt[type];
    it = m_mapType2EffectCnt.find(type);
    if (it == m_mapType2EffectCnt.end())
      return;
  }

  if (it->second.find(specId) == it->second.end()) {
    if (cnt > 0) it->second[specId] = cnt;
  } else {
    if (cnt > 0)
      it->second[specId] += cnt;
    else {
      if (it->second[specId] < DWORD(-cnt))
        it->second[specId] = 0;
      else
        it->second[specId] += cnt;
    }
  }
}

// 获取已解锁的特殊效果id
void Astrolabes::getEffectIDs(TSetDWORD& set)
{
  set.clear();
  for (auto& it : m_mapType2EffectCnt) {
    for (auto& v : it.second) {
      set.insert(v.first);
    }
  }
}

void Astrolabes::collectRuneCount(DWORD& dwTotal, DWORD& dwSpecial)
{
  for (auto& it : m_mapType2Astr)
  {
    for (auto& itAstr : it.second)
    {
      const TMapAstrolabeStar& mapStar = itAstr.second.getStarList();
      for (auto &v : mapStar)
      {
        ++dwTotal;
        const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(itAstr.second.getId(), v.second.dwId);
        if (pStarCfg == nullptr) {
          continue;
        }
        const SAstrolabeAttrCFG* pAttrCfg = pStarCfg->getAttrByProfessionType(getTypeBranch());
        if (pAttrCfg == nullptr) {
          continue;
        }
        if (!pAttrCfg->vecEffect.empty())
          ++dwSpecial;
      }
    }
  }
}

// 星位是否已激活
bool Astrolabes::isStarActive(DWORD id, DWORD starid)
{
  Astrolabe* pAstr = getAstrolabe(id);
  if (pAstr == nullptr)
    return false;
  return pAstr->isStarActive(starid);
}

// 星盘是否可激活
bool Astrolabes::canActivate(DWORD id)
{
  // 已激活
  if (isActive(id))
    return true;

  // 等级/职业限制
  const SAstrolabeCFG* pCfg = AstrolabeConfig::getMe().getAstrolabe(id);
  if (pCfg == nullptr)
    return false;
  if (pCfg->dwMenuID && m_pUser->getMenu().isOpen(pCfg->dwMenuID) == false)
    return false;
  return pCfg->canActivate(m_pUser->getLevel(), m_pUser->getEvo());
}

// 创建星盘
bool Astrolabes::create(DWORD id)
{
  TMapAstrolabe& mapAstrolabe = m_mapType2Astr[getAstrolabeType(id)];
  if (mapAstrolabe.find(id) != mapAstrolabe.end())
    return true;
  mapAstrolabe.insert(TMapAstrolabe::value_type(id, Astrolabe(m_pUser, id)));
  return mapAstrolabe.find(id) != mapAstrolabe.end();
}

// 激活星盘
bool Astrolabes::activate(DWORD id)
{
  if (!canActivate(id)) {
    return false;
  }
  return create(id);
}

// 激活星位
bool Astrolabes::activateStar(DWORD id, DWORD starid)
{
  vector<pair<DWORD, DWORD>> ids;
  ids.push_back(pair<DWORD, DWORD>(id, starid));
  return activateStar(ids);
}

bool Astrolabes::getCost(const vector<pair<DWORD, DWORD>>& ids, TVecItemInfo& cost)
{
  for (auto& id : ids)
  {
    const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(id.first, id.second);
    if (!pStarCfg)
      continue;
    if (!pStarCfg->vecCost.empty())
      combinItemInfo(cost, pStarCfg->vecCost);
  }

  return true;
}

bool Astrolabes::getTotalCost(TVecItemInfo& cost)
{
  vector<pair<DWORD, DWORD>> ids;
  for (auto& it : m_mapType2Astr) {
    for (auto& itAstr : it.second) {
      auto func = [&ids, &itAstr](SAstrolabeStar& rStar) {
        ids.push_back(pair<DWORD, DWORD>(itAstr.second.getId(), rStar.dwId));
      };
      itAstr.second.foreach(func);
    }
  }

  return getCost(ids, cost);
}

bool Astrolabes::activateStar(const vector<pair<DWORD, DWORD>>& ids, bool isConnectCheck, bool isReturnItems)
{
  if (ids.empty())
    return false;

  TVecItemInfo cost;
  set<pair<DWORD, DWORD>> temp;

  for (auto& id : ids) {
    // 已激活
    if (isStarActive(id.first, id.second))
      return false;
    // 不可激活
    if (!canActivate(id.first))
      return false;
    // 初始点
    bool isInitStar = MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(id.first), id.first, id.second);
    const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(id.first, id.second);
    if (pStarCfg == nullptr)
      return false;
    const SAstrolabeAttrCFG* pAttrCfg = pStarCfg->getAttrByProfessionType(getTypeBranch());
    if (!isInitStar && pAttrCfg == nullptr)
      return false;
    if (isConnectCheck && !isInitStar) {
      bool isOk = false;
      for (auto& connId : pStarCfg->vecConn) {
        if (isStarActive(connId.first, connId.second) || temp.find(connId) != temp.end()) {
          isOk = true;
          break;
        }
      }
      if (!isOk)
        return false;
    }
    // 重复检查
    if (temp.find(id) != temp.end())
      return false;
    temp.insert(id);
    if (!pStarCfg->vecCost.empty())
      combinItemInfo(cost, pStarCfg->vecCost);
  }

  // 消耗道具
  if(isReturnItems)
  {
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr || !pMainPack->checkItemCount(cost)) {
      XERR << "[星盘-激活星位]" << m_pUser->accid << m_pUser->id << m_pUser->name << "激活星盘失败";
      return false;
    }
    pMainPack->reduceItem(cost, ESOURCE_ASTROLABE_ACTIVATE);
  }

  for (auto &s : cost)
  {
    if(s.id() == 5261)
      m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GOLD_MEDAL, s.count());
  }

  for (auto& id : temp) {
    Astrolabe* pAstr = getAstrolabe(id.first);
    if (pAstr == nullptr) {
      // 激活星盘
      if (!activate(id.first))
        continue;
      pAstr = getAstrolabe(id.first);
      if (pAstr == nullptr)
        continue;
    }
    // 激活
    pAstr->activateStar(id.second, true);
    XLOG << "[星盘-激活星位]" << m_pUser->accid << m_pUser->id << m_pUser->name << "激活星盘:" << id.first << "星位:" << id.second
         << "成功" << XEND;
  }
  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  return true;
}

// 强制激活, GM用
bool Astrolabes::activateStarForce(DWORD id, DWORD starid)
{
  Astrolabe* pAstr = getAstrolabe(id);
  if (pAstr == nullptr) {
    if (AstrolabeConfig::getMe().getAstrolabeStar(id, starid) == nullptr)
      return false;
    if (!create(id))
      return false;
    pAstr = getAstrolabe(id);
    if (pAstr == nullptr)
      return false;
  }
  if (!pAstr->isStarActive(starid))
    pAstr->activateStar(starid, true);
  return true;
}

// 删除星位
bool Astrolabes::delStarForce(DWORD id, DWORD starid)
{
  if (!isStarActive(id, starid))
    return false;
  Astrolabe* pAstr = getAstrolabe(id);
  if (pAstr == nullptr)
    return false;
  pAstr->delStar(starid);
  return true;
}

bool Astrolabes::resetAll(bool free, bool isReturnItems)
{
  set<pair<DWORD, DWORD>> ids;
  for(auto& m : MiscConfig::getMe().getAstrolabeCFG().mapInitStar)
  {
    ids.clear();
    ids.insert(pair<DWORD, DWORD>(m.second.first, m.second.second));
    reset(ids, free, isReturnItems);
  }

  return true;
}

// 重置, 会修改ids
bool Astrolabes::reset(set<pair<DWORD, DWORD>>& ids, bool free/* = false*/, bool isReturnItems)
{
  if (ids.empty())
    return false;

  TVecItemInfo cost, retitem;
  DWORD cnt = 0;

  auto checkfunc = [&](DWORD id, DWORD starid, bool checkInitStar, bool checkConn) -> bool {
    // 未激活
    if (!isStarActive(id, starid))
      return false;
    // 初始点
    if (checkInitStar && MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(id), id, starid))
      return false;

    const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(id, starid);
    if (pStarCfg == nullptr)
      return false;

    if (checkConn) {
      for (auto &it : pStarCfg->vecConn) {
        if (ids.find(it) == ids.end() && isStarActive(it.first, it.second)) {
          if (++cnt > 1)
            return false;
          break;
        }
      }
    }

    if (!pStarCfg->vecResetCost.empty())
      combinItemInfo(cost, pStarCfg->vecResetCost);
    if (!pStarCfg->vecCost.empty())
      combinItemInfo(retitem, pStarCfg->vecCost);

    return true;
  };

  if (ids.size() == 1) {        // 重置1个星位
    bool isInitStar = false;
    EAstrolabeType type = EASTROLABETYPE_MIN;
    for (auto& id : ids) {
      isInitStar = MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(id.first), id.first, id.second);
      type = getAstrolabeType(id.first);
    }
    if (isInitStar) {           // 重置初始点则表示重置整个星盘
      auto it = m_mapType2Astr.find(type);
      if (it == m_mapType2Astr.end())
        return false;
      ids.clear();
      for (auto& itAstr : it->second) {
        for (auto& itStar : itAstr.second.m_mapStar) {
          if (!checkfunc(itAstr.second.m_dwId, itStar.second.dwId, false, false))
            return false;
          else
            ids.insert(pair<DWORD, DWORD>(itAstr.second.m_dwId, itStar.second.dwId));
        }
      }
    } else {                    // 重置单个星位
      for (auto& id : ids) {
        if (!checkfunc(id.first, id.second, true, false))
          return false;
        const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(id.first, id.second);
        if (pStarCfg == nullptr)
          return false;
        DWORD num = 0;
        for (auto& v : pStarCfg->vecConn)
          if (isStarActive(v.first, v.second))
            ++num;
        if (num > 1) {
          for (auto& v : pStarCfg->vecConn)
            if (isStarActive(v.first, v.second) && !canConnectInitStar(v.first, v.second, id.first, id.second))
              return false;
        }
      }
    }
  } else {                      // 重置指定多个星位
    for (auto& id : ids) {
      if (!checkfunc(id.first, id.second, true, true))
        return false;
    }
  }

  if (!free) {
    // 重置消耗上限修正
    for (auto& v : cost) {
      DWORD limit = MiscConfig::getMe().getAstrolabeCFG().getResetCostLimitById(v.id());
      if (v.count() > limit)
        v.set_count(limit);
    }

    // 扣除道具
    BasePackage* pMainPack = m_pUser->getPackage().getPackage(EPACKTYPE_MAIN);
    if (pMainPack == nullptr || !pMainPack->checkItemCount(cost)) {
      XERR << "[星盘-重置星盘]" << m_pUser->accid << m_pUser->id << m_pUser->name << "失败";
      return false;
    }
    pMainPack->reduceItem(cost, ESOURCE_ASTROLABE_RESET);
  }

  // 重置
  for (auto& id : ids) {
    Astrolabe* pAstr = getAstrolabe(id.first);
    if (pAstr != nullptr) {
      pAstr->delStar(id.second);
    }
  }
  XLOG << "[星盘-重置]" << m_pUser->accid << m_pUser->id << m_pUser->name;
  for (auto& id : ids) {
    XLOG << id.first * 10000 + id.second;
  }
  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  // 返还道具
  if(isReturnItems)
  {
    XLOG << "返回道具:";
    for (auto& v : retitem) {
      v.set_source(ESOURCE_ASTROLABE_RESET); // 设置source, 用于公会服判断该贡献不计算到玩家的周贡献和总贡献中
      XLOG << "id:" << v.id() << "count:" << v.count();
    }
    XLOG << XEND;
    m_pUser->getPackage().addItem(retitem, EPACKMETHOD_AVAILABLE);
  }
  else
    XLOG << "未返回道具" << XEND;

  return true;
}

// // 执行重置
// void Astrolabes::resetForce(EAstrolabeType type)
// {
//   // 重置
//   m_mapType2Astr[type].clear();

//   // 清除特殊效果
//   if (m_mapType2EffectCnt.find(type) != m_mapType2EffectCnt.end()) {
//     for (auto& it : m_mapType2EffectCnt[type]) {
//       it.second = 0;            // 事件中会读取数量, 故先设为0
//       m_pUser->getEvent().onResetRuneSpecial(it.first);
//     }
//     m_mapType2EffectCnt[type].clear();
//   }

//   // 重置后自动激活初始星位
//   auto it = MiscConfig::getMe().getAstrolabeCFG().mapInitStar.find(type);
//   if (it == MiscConfig::getMe().getAstrolabeCFG().mapInitStar.end()) {
//     XERR << "[星盘-重置]" << m_pUser->accid << m_pUser->id << m_pUser->name << "type:" << type << "初始星位未配置" << XEND;
//   } else {
//     activateStar(it->second.first, it->second.second);
//   }

//   // 刷新属性
//   m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

//   // 更新客户端数据
//   sendAstrolabeData();
// }

// 计算属性加成
//void Astrolabes::collectAttr(TVecAttrSvrs& attrs)
void Astrolabes::collectAttr()
{
  for (auto& it : m_mapType2Astr)
    for (auto& itAstr : it.second)
      //itAstr.second..collectAttr(attrs);
      itAstr.second.collectAttr();
}

// 检查星位是否连接到初始星位
bool Astrolabes::canConnectInitStar(DWORD id, DWORD starid, set<DWORD>& ignores)
{
  if (MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(id), id, starid))
    return true;

  ignores.insert(id * 10000 + starid);
  const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(id, starid);
  if (pStarCfg == nullptr)
    return false;

  for (auto& it : pStarCfg->vecConn) {
    if (!isStarActive(it.first, it.second))
      continue;
    if (ignores.find(it.first * 10000 + it.second) != ignores.end())
      continue;
    if (canConnectInitStar(it.first, it.second, ignores))
      return true;
  }

  return false;
}

// 一转到二转时部分职业须重置所有星盘
void Astrolabes::onProfesChange(EProfession oldProfes)
{
  if (m_pUser->getEvo() != 2)
    return;
  const SRoleBaseCFG* pOldRoleCfg = RoleConfig::getMe().getRoleBase(oldProfes);
  if (pOldRoleCfg == nullptr)
    return;
  DWORD deftb = MiscConfig::getMe().getAstrolabeCFG().getDefaultTypeBranch(pOldRoleCfg->dwType);
  if (deftb == 0)
    return;
  if (deftb == m_pUser->getProfesTypeBranch())
    return;

  for (auto& it : m_mapType2Astr) {
    DWORD cnt = 0;
    for (auto& itAstr : it.second) {
      cnt += itAstr.second.m_mapStar.size();
      if (cnt > 1)
        break;
    }
    if (cnt <= 1)
      continue;

    auto itInitStar = MiscConfig::getMe().getAstrolabeCFG().mapInitStar.find(it.first);
    if (itInitStar == MiscConfig::getMe().getAstrolabeCFG().mapInitStar.end())
      continue;

    set<pair<DWORD, DWORD>> ids;
    ids.insert(pair<DWORD, DWORD>(itInitStar->second.first, itInitStar->second.second));

    if (!reset(ids, true)) {
      XERR << "[星盘-转职重置]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << oldProfes << "重置失败,星位:";
      for (auto& id : ids)
        XERR << id.first * 10000 + id.second;
      XERR << XEND;
      continue;
    }

    // 客户端更新星盘数据
    Cmd::AstrolabeResetCmd cmd;
    cmd.add_stars(itInitStar->second.first * 10000 + itInitStar->second.second);
    cmd.set_success(true);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);

    MsgManager::sendMsg(m_pUser->id, 2853);

    XLOG << "[星盘-转职重置]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << oldProfes << it.first << "重置成功" << XEND;
  }
}

// 通知客户端重置星盘
void Astrolabes::ntfReset()
{
  if(!m_pUser)
    return;

  for(auto& m : m_mapType2Astr)
  {
    auto it_init = MiscConfig::getMe().getAstrolabeCFG().mapInitStar.find(m.first);
    if(MiscConfig::getMe().getAstrolabeCFG().mapInitStar.end() == it_init)
      continue;

    Cmd::AstrolabeResetCmd cmd;
    cmd.add_stars(it_init->second.first * 10000 + it_init->second.second);
    cmd.set_success(true);
    PROTOBUF(cmd, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

//--------------------------------------------------------------------------------
// 消息处理

// 发送星盘数据
void Astrolabes::sendAstrolabeData()
{
  AstrolabeQueryCmd cmd;
  for (auto& it : m_mapType2Astr) {
    for (auto& itAstr : it.second) {
      auto func = [&cmd, &itAstr](SAstrolabeStar& rStar) {
        cmd.add_stars(itAstr.second.getId() * 10000 + rStar.dwId);
      };
      itAstr.second.foreach(func);
    }
  }
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

// 激活星位
void Astrolabes::handleActivateStar(Cmd::AstrolabeActivateStarCmd& rev)
{
  if (!m_pUser->getMenu().isOpen(MiscConfig::getMe().getAstrolabeCFG().dwMenuId))
    return;
  vector<pair<DWORD, DWORD>> ids;
  for (auto& star : rev.stars())
    ids.push_back(pair<DWORD, DWORD>(star / 10000, star % 10000));
  if (activateStar(ids)) {
    rev.set_success(true);
    PROTOBUF(rev, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

// 重置星盘
void Astrolabes::handleReset(Cmd::AstrolabeResetCmd& rev)
{
  set<pair<DWORD, DWORD>> ids;
  for (auto& star : rev.stars())
    ids.insert(pair<DWORD, DWORD>(star / 10000, star % 10000));

  if (reset(ids)) {
    rev.set_success(true);
    PROTOBUF(rev, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

////////////////////////////////////////////////////////////////////////////////
// 星盘
Astrolabe::Astrolabe(SceneUser* pUser, DWORD id) : m_dwId(id), m_pUser(pUser)
{
}

Astrolabe::~Astrolabe()
{
}

bool Astrolabe::load(const AstrolabeData& rData)
{
  bool ok = true;

  m_mapStar.clear();
  for (int i = 0; i < rData.stars_size(); ++i)
    activateStar(rData.stars(i).id(), false);

  return ok;
}

bool Astrolabe::save(AstrolabeData* pData)
{
  if (pData == nullptr)
    return false;

  pData->Clear();
  pData->set_id(m_dwId);

  bool ok = true;
  auto func = [this, pData, &ok](SAstrolabeStar& rStar) {
    AstrolabeStarData* pStarData = pData->add_stars();
    if (pStarData == nullptr) {
      XERR << "[星盘-保存星位]" << m_pUser->accid << m_pUser->id << m_pUser->name << "星盘:" << m_dwId << "星位:" << rStar.dwId << "star protobuf error" << XEND;
      ok = false;
      return;
    }
    pStarData->set_id(rStar.dwId);
  };
  foreach(func);

  return ok;
}

// 是否需要保存
bool Astrolabe::isNeedSave()
{
  return m_mapStar.size() > 0;
}

// 计算属性加成
//void Astrolabe::collectAttr(TVecAttrSvrs& attrs)
void Astrolabe::collectAttr()
{
  Attribute* pAttr = m_pUser->getAttribute();
  if (pAttr == nullptr)
    return;
  //if (attrs.empty())
  //  return;

  auto func = [this, pAttr/*&attrs*/](SAstrolabeStar& rStar) {
    DWORD id = rStar.dwId;

    if (MiscConfig::getMe().getAstrolabeCFG().isInitStar(getAstrolabeType(m_dwId), m_dwId, id))
      return;

    const SAstrolabeStarCFG* pCfg = AstrolabeConfig::getMe().getAstrolabeStar(m_dwId, id);
    if (pCfg == nullptr) {
      XERR << "[星盘-计算属性]" << m_pUser->accid << m_pUser->id << m_pUser->name << "星盘:" << m_dwId << "星位:" << id << "配置未找到" << XEND;
      return;
    }
    const SAstrolabeAttrCFG* pAttrCfg = pCfg->getAttrByProfessionType(m_pUser->getAstrolabes().getTypeBranch());
    if (pAttrCfg == nullptr) {
      XERR << "[星盘-计算属性]" << m_pUser->accid << m_pUser->id << m_pUser->name << m_pUser->getProfession() << "星盘:" << m_dwId << "星位:" << id << "系别分支:" << m_pUser->getAstrolabes().getTypeBranch() << "星位效果未找到" << XEND;
      return;
    }

    for (auto &it : pAttrCfg->vecAttr) {
      pAttr->modifyCollect(ECOLLECTTYPE_ASTROLABE, it);
      /*float value = attrs[it.type()].value() + it.value();
      attrs[it.type()].set_value(value);*/
    }
  };
  foreach(func);
}

// 激活
void Astrolabe::activateStar(DWORD id, bool event)
{
  m_mapStar[id].dwId = id;
  // 刷新属性
  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  bool bSpecial = false;
  const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(m_dwId, id);
  if (pStarCfg != nullptr) {
    const SAstrolabeAttrCFG* pAttrCfg = pStarCfg->getAttrByProfessionType(m_pUser->getAstrolabes().getTypeBranch());
    if (pAttrCfg != nullptr) {
      for (auto specId : pAttrCfg->vecEffect) {
        m_pUser->getAstrolabes().chgEffectCnt(getAstrolabeType(m_dwId), specId, 1);
        if (event)
          m_pUser->getEvent().onUnlockRuneSpecial(specId);
      }
      if(pAttrCfg->vecEffect.empty() == false)
        bSpecial = true;
    }
  }

  if (event)
    m_pUser->getEvent().onUnlockRune(id, bSpecial);
}

// 删除星位
void Astrolabe::delStar(DWORD id)
{
  auto it = m_mapStar.find(id);
  if (it == m_mapStar.end())
    return;
  m_mapStar.erase(it);
  m_pUser->setCollectMark(ECOLLECTTYPE_ASTROLABE);

  const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(m_dwId, id);
  if (pStarCfg != nullptr) {
    const SAstrolabeAttrCFG* pAttrCfg = pStarCfg->getAttrByProfessionType(m_pUser->getAstrolabes().getTypeBranch());
    if (pAttrCfg != nullptr) {
      for (auto specId : pAttrCfg->vecEffect) {
        m_pUser->getAstrolabes().chgEffectCnt(getAstrolabeType(m_dwId), specId, -1);
        m_pUser->getEvent().onResetRuneSpecial(specId);
      }
    }
  }
}

DWORD Astrolabes::getPetAdventureScore()
{
  DWORD score = 0;
  for (auto &m : m_mapType2Astr)
  {
    for (auto &it : m.second)
    {
      for (auto &s : it.second.m_mapStar)
      {
        const SAstrolabeStarCFG* pStarCfg = AstrolabeConfig::getMe().getAstrolabeStar(it.first, s.first);
        if (pStarCfg)
          score += pStarCfg->dwPetAdventureScore;
      }
    }
  }

  return score;
}

void Astrolabe::getIds(vector<pair<DWORD, DWORD>>& ids)
{
  for(auto& v : m_mapStar)
  {
    ids.push_back(make_pair(m_dwId, v.second.dwId));
  }
}

#include "TutorTask.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "MailManager.h"
#include "RedisManager.h"
#include "Menu.h"

bool STutorTaskItem::fromData(const TutorTaskItem& rData)
{
  dwID = rData.id();
  dwProgress = rData.progress();
  bReward = rData.reward();
  return true;
}

bool STutorTaskItem::toData(TutorTaskItem* pData)
{
  if (pData == nullptr)
    return false;
  pData->Clear();
  pData->set_id(dwID);
  pData->set_progress(dwProgress);
  pData->set_reward(bReward);
  return true;
}

bool STutorRewardItem::fromData(const TutorReward& data)
{
  qwCharID = data.charid();
  strName = data.name();
  for (int i = 0; i < data.reward_size(); ++i)
    vecReward.push_back(data.reward(i));
  for (int i = 0; i < data.item_size(); ++i)
    vecTaskID2Time.push_back(pair<DWORD, DWORD>(data.item(i).taskid(), data.item(i).time()));
  return true;
}

bool STutorRewardItem::toData(TutorReward* data)
{
  if (data == nullptr)
    return false;
  data->set_charid(qwCharID);
  data->set_name(strName);
  for (auto id : vecReward)
    data->add_reward(id);
  for (auto& v : vecTaskID2Time)
  {
    TutorRewardItem* p = data->add_item();
    if (p)
    {
      p->set_taskid(v.first);
      p->set_time(v.second);
    }
  }
  return true;
}

void STutorRewardItem::addReward(const TutorReward& data)
{
  for (int i = 0; i < data.reward_size(); ++i)
  {
    vecReward.push_back(data.reward(i));
  }
  for (int i = 0; i < data.item_size(); ++i)
  {
    // 当天学生产生的奖励, taskid不可重复
    auto it = find_if(vecTaskID2Time.begin(), vecTaskID2Time.end(), [&](const pair<DWORD, DWORD>& p) -> bool {
        return p.first == data.item(i).taskid() && p.second == data.item(i).time();
      });
    if (it == vecTaskID2Time.end())
      vecTaskID2Time.push_back(pair<DWORD, DWORD>(data.item(i).taskid(), data.item(i).time()));
  }
}

TutorTask::TutorTask(SceneUser* pUser) : m_pUser(pUser), m_oOneMinTimer(60), m_oFiveMinTimer(300)
{
}

TutorTask::~TutorTask()
{
}

bool TutorTask::load(const BlobTutorTask& rData)
{
  m_mapType2Items.clear();
  for (int i = 0; i < rData.items_size(); ++i)
  {
    const STutorTaskCFG* pCfg = TutorConfig::getMe().getTutorTaskCFG(rData.items(i).id());
    if (pCfg == nullptr)
      continue;

    STutorTaskItem& item = m_mapType2Items[pCfg->eType][rData.items(i).id()];
    item.fromData(rData.items(i));
  }

  m_dwProficiency = rData.proficiency();

  for (int i = 0; i < rData.growreward_size(); ++i)
    m_vecGrowReward.push_back(rData.growreward(i));
  // 设置初始值, 防止刚更新出去时给已获奖玩家重复发奖
  // setGrowRewardGot(m_pUser->getLevel(), false);
  for (int i = 0; i < rData.tutorgrowreward_size(); ++i)
    m_vecTutorGrowReward.push_back(rData.tutorgrowreward(i));
  if (m_vecTutorGrowReward.empty() && !m_vecGrowReward.empty())
    m_vecTutorGrowReward.insert(m_vecTutorGrowReward.end(), m_vecGrowReward.begin(), m_vecGrowReward.end());

  if (MiscConfig::getMe().getTutorCFG().bGrowRewardPatchSwitch)
  {
    // 以当前代码第一次更新出去时的玩家等级为最小可领取成长奖励的等级
    m_dwGrowRewardLv = rData.growrewardlv() ? rData.growrewardlv() : m_pUser->getLevel();
  }

  for (int i = 0; i < rData.tutorrewards_size(); ++i)
    m_mapCharID2Reward[rData.tutorrewards(i).charid()].fromData(rData.tutorrewards(i));

  return true;
}

bool TutorTask::save(BlobTutorTask* pData)
{
  if (pData == nullptr)
    return false;

  pData->Clear();
  for (auto& v : m_mapType2Items)
  {
    for (auto& m : v.second)
      m.second.toData(pData->add_items());
  }

  pData->set_proficiency(m_dwProficiency);

  for (auto v : m_vecGrowReward)
    pData->add_growreward(v);
  for (auto v : m_vecTutorGrowReward)
    pData->add_tutorgrowreward(v);
  pData->set_growrewardlv(m_dwGrowRewardLv);

  for (auto& v : m_mapCharID2Reward)
    v.second.toData(pData->add_tutorrewards());

  XDBG << "[导师-冒险任务-保存]" << m_pUser->accid << m_pUser->id << m_pUser->name << "数据大小:" << pData->ByteSize() << XEND;
  return true;
}

// 是否可以成为导师
bool TutorTask::canBeTutor()
{
  return m_pUser->getMenu().isOpen(MiscConfig::getMe().getTutorCFG().dwTutorMenuID);
}

// 冒险熟练度由动态的任务奖励熟练度和静态的等级奖励熟练度构成
// 等级奖励部分不清零, 随版本迭代而变化
DWORD TutorTask::getProficiency()
{
  return m_dwProficiency + TutorConfig::getMe().getProficiencyByLv(m_pUser->getLevel());
}

void TutorTask::addProficiency(DWORD value)
{
  DWORD pre = m_dwProficiency;
  m_dwProficiency += value;
  m_pUser->setDataMark(EUSERDATATYPE_TUTOR_PROFIC);
  m_pUser->refreshDataAtonce();
  XLOG << "[导师-熟练度增加]" << m_pUser->accid << m_pUser->id << m_pUser->name << "增加前:" << pre << "增加:" << value << "增加后:" << m_dwProficiency << XEND;
}

void TutorTask::clearProficiency()
{
  m_dwProficiency = 0;
  m_pUser->setDataMark(EUSERDATATYPE_TUTOR_PROFIC);
  m_pUser->refreshDataAtonce();
  XLOG << "[导师-熟练度清零]" << m_pUser->accid << m_pUser->id << m_pUser->name << "熟练度清零" << XEND;
}

// 毕业处理
bool TutorTask::graduation()
{
  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    XERR << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "意外成为导师但未完成毕业任务,不拦截" << XEND;

  TVecItemInfo items;
  if (MiscConfig::getMe().getTutorCFG().dwTeacherGraduationReward != 0 && MiscConfig::getMe().getTutorCFG().dwTeacherGraduationMailID != 0)
  {
    if (RewardManager::roll(MiscConfig::getMe().getTutorCFG().dwTeacherGraduationReward, m_pUser, items, ESOURCE_TUTOR_GRADUATION) == false)
    {
      XERR << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师:" << tutor << "奖励:" << MiscConfig::getMe().getTutorCFG().dwTeacherGraduationReward << "获取失败" << XEND;
      return false;
    }
  }

  if (m_pUser->getPackage().rollReward(MiscConfig::getMe().getTutorCFG().dwStudentGraduationReward, EPACKMETHOD_AVAILABLE, true, true, true, 0, false, ESOURCE_TUTOR_GRADUATION) == false)
  {
    XERR << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生毕业奖励:" << MiscConfig::getMe().getTutorCFG().dwStudentGraduationReward << "发送失败" << XEND;
    return false;
  }
  XLOG << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生毕业奖励:" << MiscConfig::getMe().getTutorCFG().dwStudentGraduationReward << "发送成功" << XEND;

  if (!items.empty())
  {
    MailManager::getMe().sendMail(tutor, MiscConfig::getMe().getTutorCFG().dwTeacherGraduationMailID, items, MsgParams(m_pUser->getName()));
    XLOG << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师:" << tutor << "导师奖励:";
    for (auto& item : items)
      XLOG << item.id() << item.count();
    XLOG << "发送成功" << XEND;
  }

  // 冒险熟练度清零
  clearProficiency();

  // 成就
  m_pUser->getAchieve().onStudentGraduation();

  // 发送未领取的成长奖励
  sendGrowRewardMail();

  // 移除关系
  RemoveRelationSocialCmd cmd;
  m_pUser->toData(cmd.mutable_user());
  cmd.set_destid(tutor);
  cmd.set_relation(ESOCIALRELATION_TUTOR);
  cmd.set_to_global(false);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  // 添加曾经学生
  AddRelationSocialCmd addcmd;
  addcmd.mutable_user()->set_charid(tutor);
  addcmd.set_destid(m_pUser->id);
  addcmd.set_relation(ESOCIALRELATION_STUDENT_RECENT);
  addcmd.set_to_global(false);
  PROTOBUF(addcmd, addsend, addlen);
  thisServer->sendCmdToSession(addsend, addlen);

  m_pUser->getServant().onGrowthFinishEvent(ETRIGGER_GRADUATE);
  XLOG << "[导师-毕业]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师:" << tutor << "毕业成功" << XEND;
  return true;
}

void TutorTask::resetProgress()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_TUTOR_TASK_DAY) == 0)
  {
    for (auto& v : m_mapType2Items)
      for (auto& m : v.second)
      {
        const STutorTaskCFG* pCfg = TutorConfig::getMe().getTutorTaskCFG(m.second.dwID);
        if (pCfg == nullptr || pCfg->dwResetTime != 1)
          continue;
        m.second.reset();
      }
    m_pUser->getVar().setVarValue(EVARTYPE_TUTOR_TASK_DAY, 1);
    m_bProgressUpdate = true;
  }

  if (m_pUser->getVar().getVarValue(EVARTYPE_TUTOR_TASK_WEEK) == 0)
  {
    for (auto& v : m_mapType2Items)
      for (auto& m : v.second)
      {
        const STutorTaskCFG* pCfg = TutorConfig::getMe().getTutorTaskCFG(m.second.dwID);
        if (pCfg == nullptr || pCfg->dwResetTime != 7)
          continue;
        m.second.reset();
      }
    m_pUser->getVar().setVarValue(EVARTYPE_TUTOR_TASK_WEEK, 1);
    m_bProgressUpdate = true;
  }
}

void TutorTask::sendTaskData()
{
  resetProgress();

  TutorTaskUpdateNtf ntf;
  for (auto& v : m_mapType2Items)
    for (auto& m : v.second)
      m.second.toData(ntf.add_items());
  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void TutorTask::sendGrowRewardData()
{
  TutorGrowRewardUpdateNtf ntf;
  DWORD cnt = 1;
  for (auto v : m_vecGrowReward)
  {
    if (m_dwGrowRewardLv)
    {
      for (DWORD i = cnt; i <= cnt + 64 && i < m_dwGrowRewardLv; ++i)
        v |= QWORD(1) << ((i - 1) % 64);
      cnt += 64;
    }
    ntf.add_growrewards(v & 0xffffffff);
    ntf.add_growrewards(v >> 32);
  }
  PROTOBUF(ntf, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void TutorTask::updateProgress(ETutorTaskType type, DWORD progress/* = 1*/, QWORD value/* = 0*/)
{
  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    return;
  if (isProficiencyMax())
    return;

  // 重置所有进度
  resetProgress();

  DWORD today = xTime::getDayStart(now(), 5 * 3600);
  const TVecTutorTaskCFG& tasks = TutorConfig::getMe().getTutorTaskCFGByType(type);
  TMapTutorTaskItem& items = m_mapType2Items[type];
  TutorTaskUpdateNtf ntf;
  vector<DWORD> finishtasks;
  for (auto& task : tasks)
  {
    STutorTaskItem& item = items[task.dwID];
    item.dwID = task.dwID;

    if (item.bReward)
      continue;
    if (!task.checkValid(value))
      continue;

    item.dwProgress += progress;
    m_bProgressUpdate = true;
    XLOG << "[导师-冒险任务-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "任务:" << task.dwID << "进度:" << progress << "当前进度:" << item.dwProgress << "学生冒险进度更新成功" << XEND;
    if (item.dwProgress >= task.dwTotalProgress)
    {
      // 任务完成
      item.dwProgress = task.dwTotalProgress;
      item.bReward = true;

      addProficiency(task.dwProficiency);
      XLOG << "[导师-冒险任务-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "任务:" << task.dwID << "熟练度:" << task.dwProficiency << "学生冒险熟练度获得成功" << XEND;

      DWORD stdReward = task.getStudentReward(m_pUser->getLevel());
      if (m_pUser->getPackage().rollReward(stdReward, EPACKMETHOD_AVAILABLE, true, true, true, 0, false, ESOURCE_TUTOR_TASK) == false)
        XERR << "[导师-冒险任务-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生奖励:" << stdReward << "发送失败" << XEND;
      else
        XLOG << "[导师-冒险任务-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "任务:" << task.dwID << "学生奖励:" << stdReward << "发送成功" << XEND;

      if (task.dwTeacherReward != 0)
        finishtasks.push_back(task.dwID);

      thisServer->sendSysMsgToWorldUser(tutor, 3244, MsgParams{m_pUser->name, task.strName});
    }

    item.toData(ntf.add_items());
  }

  // 同步导师奖励, 导师在线则直接同步给导师(发到global跨线查找), 否则存到离线数据
  if (!finishtasks.empty())
  {
    SyncTutorRewardSocialCmd cmd;
    cmd.set_searchuser(true);
    cmd.set_charid(tutor);
    TutorReward* p = cmd.mutable_reward();
    if (p != nullptr)
    {
      // p->set_type(EOFFLINEMSG_TUTOR_REWARD);
      // p->set_targetid(tutor);
      p->set_charid(m_pUser->id);
      p->set_name(m_pUser->name);
      for (auto id : finishtasks)
      {
        TutorRewardItem* i = p->add_item();
        if (i)
        {
          i->set_taskid(id);
          i->set_time(today);
        }
      }
    }
    SceneUser* pTutor = SceneUserManager::getMe().getUserByID(tutor);
    if (pTutor)
    {
      if (p)
      {
        pTutor->getTutorTask().addTutorReward(*p);
      }
    }
    else
    {
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
      XLOG << "[导师-冒险任务-进度]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师:" << tutor << "导师完成任务:";
      for (auto taskid : finishtasks)
        XLOG << taskid;
      XLOG << "成功发送到SessionServer" << XEND;
    }

    m_bRedPoint = true;
  }

  if (ntf.items_size() > 0)
  {
    PROTOBUF(ntf, send, len);
    m_pUser->sendCmdToMe(send, len);
  }
}

void TutorTask::timer(DWORD cur)
{
  if (m_oOneMinTimer.timeUp(cur))
  {
    sendTutorRewardMail();
  }

  if (m_oFiveMinTimer.timeUp(cur) && m_bProgressUpdate)
  {
    // 保存进度到redis
    saveTaskToRedis();
    // 同步红点给导师
    sendRedPointToTutor();

    m_bProgressUpdate = false;
  }
}

void TutorTask::onEnterScene()
{
  refreshTutorRewardRedPoint();
  refreshGrowRewardRedPoint();
}

void TutorTask::onLeaveScene()
{
  if (m_bProgressUpdate)
  {
    // 保存进度到redis
    saveTaskToRedis();
    // 同步红点给导师
    sendRedPointToTutor();

    m_bProgressUpdate = false;
  }
}

// 学生主动领取的成长奖励
bool TutorTask::isGrowRewardGot(DWORD lv)
{
  if (lv <= 0 || lv < m_dwGrowRewardLv) return true;
  --lv;
  if (m_vecGrowReward.size() < lv / 64 + 1)
    for (DWORD i = m_vecGrowReward.size(); i < lv / 64 + 1; ++i)
      m_vecGrowReward.push_back(0);
  return (m_vecGrowReward[lv / 64] & QWORD(1) << (lv % 64)) != 0;
}

void TutorTask::setGrowRewardGot(DWORD lv, bool ntf/* = true*/)
{
  if (lv <= 0) return;
  --lv;
  if (m_vecGrowReward.size() < lv / 64 + 1)
    for (DWORD i = m_vecGrowReward.size(); i < lv / 64 + 1; ++i)
      m_vecGrowReward.push_back(0);
  m_vecGrowReward[lv / 64] |= QWORD(1) << (lv % 64);
  if (ntf)
    sendGrowRewardData();
}

// 学生升级时自动发给导师的成长奖励
bool TutorTask::isTutorGrowRewardGot(DWORD lv)
{
  if (lv <= 0) return true;
  --lv;
  if (m_vecTutorGrowReward.size() < lv / 64 + 1)
    for (DWORD i = m_vecTutorGrowReward.size(); i < lv / 64 + 1; ++i)
      m_vecTutorGrowReward.push_back(0);
  return (m_vecTutorGrowReward[lv / 64] & QWORD(1) << (lv % 64)) != 0;
}

void TutorTask::setTutorGrowRewardGot(DWORD lv)
{
  if (lv <= 0) return;
  --lv;
  if (m_vecTutorGrowReward.size() < lv / 64 + 1)
    for (DWORD i = m_vecTutorGrowReward.size(); i < lv / 64 + 1; ++i)
      m_vecTutorGrowReward.push_back(0);
  m_vecTutorGrowReward[lv / 64] |= QWORD(1) << (lv % 64);
}

// 发送成长奖励
void TutorTask::onLevelUp()
{
  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    return;

  refreshGrowRewardRedPoint();

  DWORD lv = m_pUser->getLevel();
  if (isTutorGrowRewardGot(lv))
    return;

  const STutorGrowRewardCFG* pCfg = TutorConfig::getMe().getTutorGrowRewardCFG(lv);
  if (pCfg == nullptr)
    return;

  // if (pCfg->dwStudentReward != 0)
  // {
  //   if (m_pUser->getPackage().rollReward(pCfg->dwStudentReward, EPACKMETHOD_AVAILABLE, true, true, true, 0, false, ESOURCE_TUTOR_GROW) == false)
  //     XERR << "[导师-成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生奖励:" << pCfg->dwStudentReward << "发送失败" << XEND;
  //   else
  //     XLOG << "[导师-成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生奖励:" << pCfg->dwStudentReward << "发送成功" << XEND;
  // }

  if (pCfg->dwTeacherReward != 0)
  {
    TVecItemInfo items;
    // 导师奖励与其职业无关
    if (RewardManager::roll(pCfg->dwTeacherReward, m_pUser, items, ESOURCE_TUTOR_GROW) == false)
      XERR << "[导师-成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师奖励:" << pCfg->dwTeacherReward << "获取失败" << XEND;
    else
    {
      MailManager::getMe().sendMail(tutor, MiscConfig::getMe().getTutorCFG().dwTeacherGrowMailID, items, MsgParams(m_pUser->getName(), lv));
      XLOG << "[导师-成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "导师:" << tutor << "导师奖励:";
      for (auto& item : items)
        XLOG << item.id() << item.count();
      XLOG << "发送成功" << XEND;
    }
  }

  // 通知前端熟练度更新
  addProficiency(0);

  setTutorGrowRewardGot(lv);
}

void TutorTask::onUpdateSocial()
{
  bool redpoint = false;
  for (auto v = m_mapCharID2Reward.begin(); v != m_mapCharID2Reward.end();)
  {
    if (m_pUser->getSocial().checkRelation(v->first, ESOCIALRELATION_STUDENT) == false && m_pUser->getSocial().checkRelation(v->first, ESOCIALRELATION_STUDENT_RECENT) == false)
    {
      XLOG << "[导师-社交关系更新]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << v->first << "关系解除,清除奖励" << XEND;
      v = m_mapCharID2Reward.erase(v);
      redpoint = true;
      continue;
    }
    ++v;
  }
  if (redpoint)
    refreshTutorRewardRedPoint();
}

void TutorTask::onTutorChanged(QWORD oldtutor, QWORD newtutor)
{
  if (oldtutor && oldtutor != newtutor)
    sendGrowRewardMail();
}

// 完成n个看板任务
void TutorTask::onQuestSubmit(DWORD questid)
{
  const SQuestCFGEx* pCfg = QuestManager::getMe().getQuestCFG(questid);
  if (pCfg == nullptr)
    return;
  if (pCfg->eType == EQUESTTYPE_WANTED)
    updateProgress(ETUTORTASKTYPE_BOARD_QUEST);
}

// 完成n次封印裂隙
void TutorTask::onRepairSeal()
{
  updateProgress(ETUTORTASKTYPE_REPAIR_SEAL);
}

// 使用道具
void TutorTask::onItemUsed(DWORD itemid)
{
  updateProgress(ETUTORTASKTYPE_USE_ITEM, 1, itemid);
}

// 接受导师[冒险教程]的指导
void TutorTask::onItemBeUsed(DWORD itemid)
{
  updateProgress(ETUTORTASKTYPE_BE_USED_ITEM, 1, itemid);
}

// 累加完成40层无限塔
void TutorTask::onPassTower(DWORD curlayer, DWORD premaxlayer)
{
  if (curlayer > premaxlayer)
    updateProgress(ETUTORTASKTYPE_PASS_TOWER);
  updateProgress(ETUTORTASKTYPE_PASS_TOWER_LAYER, 1, curlayer);
}

// 通过n次公会副本
void TutorTask::onGuildRaidFinish()
{
  updateProgress(ETUTORTASKTYPE_GUILD_RAID_FINISH);
}

// 每日通关n次训练场
void TutorTask::onLaboratoryFinish()
{
  updateProgress(ETUTORTASKTYPE_LABORATORY_FINISH);
}

void TutorTask::addTutorReward(const TutorReward& reward)
{
  auto it = m_mapCharID2Reward.find(reward.charid());
  if (it == m_mapCharID2Reward.end())
    m_mapCharID2Reward[reward.charid()].fromData(reward);
  else
    it->second.addReward(reward);
}

void TutorTask::sendTutorRewardMail()
{
  if (m_pUser->getSocial().isInit() == false)
    return;

  if (m_pUser->getVar().getVarValue(EVARTYPE_TUTOR_TASK_REWARD) != 0)
    return;
  m_pUser->getVar().setVarValue(EVARTYPE_TUTOR_TASK_REWARD, 1);

  DWORD today = xTime::getDayStart(now(), 5 * 3600);
  for (auto v = m_mapCharID2Reward.begin(); v != m_mapCharID2Reward.end();)
  {
    if (m_pUser->getSocial().checkRelation(v->first, ESOCIALRELATION_STUDENT) == false && m_pUser->getSocial().checkRelation(v->first, ESOCIALRELATION_STUDENT_RECENT) == false)
    {
      XLOG << "[导师-发送任务导师奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << v->first << "关系解除,清除奖励" << XEND;
      v = m_mapCharID2Reward.erase(v);
      continue;
    }

    map<DWORD, DWORD> id2cnt;
    for (auto rewardid : v->second.vecReward)
    {
      auto it = id2cnt.find(rewardid);
      if (it == id2cnt.end())
        id2cnt[rewardid] = 0;
      id2cnt[rewardid] += 1;
    }
    v->second.vecReward.clear();

    TVecItemInfo items;
    for (auto& m : id2cnt)
    {
      TVecItemInfo rollitem;
      if (RewardManager::roll(m.first, m_pUser, rollitem, ESOURCE_TUTOR_TASK) == false)
      {
        XERR << "[导师-发送任务导师奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "奖励:" << m.first << "获取失败" << XEND;
        continue;
      }
      for (auto& item : rollitem)
        item.set_count(item.count() * m.second);
      combinItemInfo(items, rollitem);
    }

    for (auto m = v->second.vecTaskID2Time.begin(); m != v->second.vecTaskID2Time.end();)
    {
      if (m->second >= today)
      {
        ++m;
        continue;
      }
      const STutorTaskCFG* cfg = TutorConfig::getMe().getTutorTaskCFG(m->first);
      if (cfg == nullptr)
      {
        XERR << "[导师-发送任务导师奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "任务:" << m->first << "配置找不到" << XEND;
        ++m;
        continue;
      }
      TVecItemInfo rollitem;
      if (RewardManager::roll(cfg->dwTeacherReward, m_pUser, rollitem, ESOURCE_TUTOR_TASK) == false)
      {
        XERR << "[导师-发送任务导师奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "奖励:" << cfg->dwTeacherReward << "获取失败" << XEND;
        ++m;
        continue;
      }
      combinItemInfo(items, rollitem);
      m = v->second.vecTaskID2Time.erase(m);
    }

    if (!items.empty())
    {
      MailManager::getMe().sendMail(m_pUser->id, MiscConfig::getMe().getTutorCFG().dwTeacherTaskMailID, items, MsgParams(v->second.strName));
      XLOG << "[导师-发送任务导师奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "奖励:";
      for (auto& item : items)
        XLOG << item.id() << item.count();
      XLOG << "发送成功" << XEND;
    }

    ++v;
  }

  refreshTutorRewardRedPoint();
}

void TutorTask::saveTaskToRedis()
{
  resetProgress();

  const string key1 = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_TASK, m_pUser->id, 1);
  const string key7 = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_TASK, m_pUser->id, 7);

  TutorTaskQueryCmd cmd1, cmd7;
  cmd1.set_charid(m_pUser->id);
  cmd7.set_charid(m_pUser->id);

  for (auto& v : m_mapType2Items)
    for (auto& m : v.second)
    {
      const STutorTaskCFG* pCfg = TutorConfig::getMe().getTutorTaskCFG(m.second.dwID);
      if (pCfg == nullptr)
        continue;
      switch (pCfg->dwResetTime)
      {
      case 1:
        m.second.toData(cmd1.add_items());
        break;
      case 7:
        m.second.toData(cmd7.add_items());
        break;
      default:
        continue;
      }
    }

  DWORD cur = now();
  RedisManager::getMe().setProtoData(key1, &cmd1, xTime::getDayStart(cur,  5 * 3600) + DAY_T + 5 * 3600 - cur);
  RedisManager::getMe().setProtoData(key7, &cmd7, xTime::getWeekStart(cur,  5 * 3600) + WEEK_T + 5 * 3600 - cur);
}

void TutorTask::queryStudentTask(TutorTaskQueryCmd& cmd)
{
  sendTutorRewardMail();

  const string key1 = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_TASK, cmd.charid(), 1);
  const string key7 = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_TASK, cmd.charid(), 7);

  TutorTaskQueryCmd cmd1, cmd7;
  if (RedisManager::getMe().getProtoData(key1, &cmd1))
    cmd.MergeFrom(cmd1);
  if (RedisManager::getMe().getProtoData(key7, &cmd7))
    cmd.MergeFrom(cmd7);

  DWORD today = xTime::getDayStart(now(), 5 * 3600);
  auto it = m_mapCharID2Reward.find(cmd.charid());
  if (it != m_mapCharID2Reward.end())
  {
    for (auto& v : it->second.vecTaskID2Time)
      if (v.second == today)
        cmd.add_finishtaskids(v.first);
  }

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

// 领取导师奖励
bool TutorTask::getTutorReward(QWORD studentid, DWORD taskid)
{
  sendTutorRewardMail();

  auto it = m_mapCharID2Reward.find(studentid);
  if (it == m_mapCharID2Reward.end())
  {
    XERR << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "没有奖励" << XEND;
    return false;
  }

  if (m_pUser->getSocial().checkRelation(studentid, ESOCIALRELATION_STUDENT) == false)
  {
    XERR << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "不是师徒关系" << XEND;
    return false;
  }

  DWORD punishtime = 0;
  if (RedisManager::getMe().getData<DWORD>(RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_TUTOR_REWARD_PUNISH, m_pUser->id), punishtime) &&
      punishtime && punishtime <= m_pUser->getSocial().getRelationTime(studentid, ESOCIALRELATION_STUDENT))
  {
    MsgManager::sendMsg(m_pUser->id, 3245);
    return false;
  }

  DWORD today = xTime::getDayStart(now(), 5 * 3600);
  for (auto v = it->second.vecTaskID2Time.begin(); v != it->second.vecTaskID2Time.end();)
  {
    if (v->first == taskid && v->second == today)
    {
      const STutorTaskCFG* cfg = TutorConfig::getMe().getTutorTaskCFG(taskid);
      if (cfg == nullptr)
      {
        XERR << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "任务:" << taskid << "配置找不到" << XEND;
        return false;
      }

      if (m_pUser->getPackage().rollReward(cfg->dwTeacherReward, EPACKMETHOD_AVAILABLE) == false)
      {
        XERR << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "奖励:" << cfg->dwTeacherReward << "获取失败" << XEND;
        return false;
      }

      it->second.vecTaskID2Time.erase(v);
      XLOG << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "奖励:" << cfg->dwTeacherReward << "成功" << XEND;
      return true;
    }

    ++v;
  }

  XERR << "[导师-领取任务导师奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "学生:" << studentid << "任务:" << taskid << "找不到" << XEND;
  return false;
}

void TutorTask::sendRedPointToTutor()
{
  if (!m_bRedPoint)
    return;
  m_bRedPoint = false;

  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    return;
  SyncTutorRewardSocialCmd cmd;
  cmd.set_searchuser(true);
  cmd.set_charid(tutor);
  cmd.set_redpointtip(m_pUser->id);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
}

void TutorTask::addRedPointTip(QWORD tip)
{
  m_pUser->getTip().addRedTip(EREDSYS_TUTOR_TASK, tip);

  TutorTaskQueryCmd cmd;
  cmd.set_charid(tip);
  cmd.set_refresh(true);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToMe(send, len);
}

void TutorTask::refreshTutorRewardRedPoint()
{
  m_pUser->getTip().removeRedTip(EREDSYS_TUTOR_TASK);

  DWORD today = xTime::getDayStart(now(), 5 * 3600);
  for (auto& v : m_mapCharID2Reward)
  {
    for (auto& m : v.second.vecTaskID2Time)
    {
      if (m.second == today)
      {
        m_pUser->getTip().addRedTip(EREDSYS_TUTOR_TASK, v.first);
        break;
      }
    }
  }
}

bool TutorTask::getGrowReward(DWORD level/* = 0*/)
{
  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    return false;

  bool ntf = false;
  TVecItemInfo rewards;
  const TMapTutorGrowRewardCFG& cfgs = TutorConfig::getMe().getAllGrowRewardCFG();
  for (auto& v : cfgs)
  {
    if (v.first > m_pUser->getLevel())
      break;

    if (isGrowRewardGot(v.first) || v.second.dwStudentReward == 0 || (level && v.first != level))
      continue;

    TVecItemInfo items;
    if (RewardManager::roll(v.second.dwStudentReward, m_pUser, items, ESOURCE_TUTOR_GROW) == false)
    {
      XERR << "[导师-领取成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "等级:" << v.first << "学生奖励:" << v.second.dwStudentReward << "导师:" << tutor << "发送失败" << XEND;
      continue;
    }

    combinItemInfo(rewards, items);
    setGrowRewardGot(v.first, false);
    ntf = true;

    XLOG << "[导师-领取成长奖励]" << m_pUser->accid << m_pUser->id << m_pUser->name << "等级:" << v.first << "学生奖励:" << v.second.dwStudentReward << "导师:" << tutor << "发送成功" << XEND;
  }

  if (ntf)
  {
    m_pUser->getPackage().addItem(rewards, EPACKMETHOD_AVAILABLE);
    sendGrowRewardData();
    if (level)
      refreshGrowRewardRedPoint();
    else
      m_pUser->getTip().removeRedTip(EREDSYS_TUTOR_GROW_REWARD);
    return true;
  }
  return false;
}

void TutorTask::sendGrowRewardMail()
{
  bool ntf = false;
  TVecItemInfo items;
  TVecDWORD lvs;
  const TMapTutorGrowRewardCFG& cfgs = TutorConfig::getMe().getAllGrowRewardCFG();
  for (auto& v : cfgs)
  {
    if (v.first > m_pUser->getLevel())
      break;
    if (v.second.dwStudentReward == 0 || isGrowRewardGot(v.first))
      continue;

    TVecItemInfo rewards;
    if (RewardManager::roll(v.second.dwStudentReward, m_pUser, rewards, ESOURCE_TUTOR_GROW) == false)
    {
      XERR << "[导师-发送成长奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "等级:" << v.first << "奖励:" << v.second.dwStudentReward << "获取道具失败" << XEND;
      continue;
    }
    if (!rewards.empty())
      combinItemInfo(items, rewards);

    lvs.push_back(v.first);
    setGrowRewardGot(v.first, false);
    ntf = true;
  }
  if (!items.empty())
  {
    MailManager::getMe().sendMail(m_pUser->id, 12108, items);
    XLOG << "[导师-发送成长奖励邮件]" << m_pUser->accid << m_pUser->id << m_pUser->name << "玩家当前等级:" << m_pUser->getLevel() << "奖励等级:";
    for (auto lv : lvs)
      XLOG << lv;
    XLOG << "奖励:";
    for (auto& item : items)
      XLOG << item.id() << item.count();
    XLOG << "发送成功" << XEND;
  }
  if (ntf)
  {
    sendGrowRewardData();
    m_pUser->getTip().removeRedTip(EREDSYS_TUTOR_GROW_REWARD);
  }
}

void TutorTask::refreshGrowRewardRedPoint()
{
  QWORD tutor = m_pUser->getTutorCharID();
  if (tutor == 0)
    return;

  const TMapTutorGrowRewardCFG& cfgs = TutorConfig::getMe().getAllGrowRewardCFG();
  for (auto& v : cfgs)
  {
    if (v.first > m_pUser->getLevel())
      break;
    if (v.second.dwStudentReward == 0 || isGrowRewardGot(v.first))
      continue;

    m_pUser->getTip().addRedTip(EREDSYS_TUTOR_GROW_REWARD);
    return;
  }
  m_pUser->getTip().removeRedTip(EREDSYS_TUTOR_GROW_REWARD);
}

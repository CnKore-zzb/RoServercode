#include "TestQuest.h"
#include "config/QuestConfig.h"
#include "QuestStep.h"

TestQuest::TestQuest()
{
}

TestQuest::~TestQuest()
{
}

void TestQuest::print()
{
  for (auto &it : QuestManager::getMe().m_mapQuestCFG)
  {
    SQuestCFGEx &cfg = it.second;

    XLOG << "[任务测试],id:" << cfg.id << "等级:" << cfg.lv << "类型:" << cfg.eType << "职业:" << cfg.eDestProfession << XEND;
    /*
    for (auto &iter : cfg.vecPreQuest)
    {
      XLOG("[任务测试],id:%u,前置任务:%u", cfg.id, iter);
    }
    for (auto &iter : cfg.vecMustPreQuest)
    {
      XLOG("[任务测试],id:%u,必要前置任务:%u", cfg.id, iter);
    }
    for (auto &iter : it.vecSteps)
    {
      BaseStep *pStep = &(*iter);
      XLOG("[任务测试],id:%u,Content:%s", cfg.id, pStep->m_strContent.c_str());
      XLOG("[任务测试],id:%u,内容:%s,逻辑分组:%u,成功跳转:%u,失败跳转:%u,离线跳转:%u", iter->getQuestID(), iter->m_strContent.c_str(), iter->getSubGroup(), iter->getFinishJump(), iter->getFailJump(), iter->getResetJump());
      for (auto &s_it : pStep->m_vecClientSelects)
      {
        XLOG("[任务测试],id:%u,逻辑跳转:%s,client:%u", cfg.id, pStep->m_strContent.c_str(), s_it);
      }
    }
    */
    //test(cfg.vecSteps);
  }
}

void TestQuest::test(const TVecQuestStep &vec)
{
  if (vec.empty()) return;

  std::vector<TestItem> v;
  v.resize(vec.size());

  DWORD i = 0;

  for (auto &iter : vec)
  {
    TestItem item;
    item.m_dwID = i++;
    item.m_dwQuestID = iter->getQuestID();
    item.m_dwSubGroup = iter->getSubGroup();
    item.m_dwFinishJump = iter->getFinishJump();
    item.m_dwFailJump = iter->getFailJump();
    item.m_dwResetJump = iter->getResetJump();
    item.m_strContent = iter->m_strContent;
    item.m_vecSelects = iter->m_vecClientSelects;
    v[item.m_dwID] = item;
    XLOG << "[任务测试],quest:" << item.m_dwQuestID << ",id:" << item.m_dwID << ",内容:" << iter->m_strContent << ",逻辑分组:" << iter->getSubGroup()
      << ",成功跳转:" << iter->getFinishJump() << ",失败跳转:" << iter->getFailJump() << ",离线跳转:" << iter->getResetJump() << XEND;
  }

 // return;
  DWORD dwCurID = 0;
  doTestItem(v[dwCurID], v, nullptr);

  /*
  if ((*iter)->getSubGroup() != 0)
  {
    const SQuestCFGEx* pCFG = QuestManager::getMe().getQuestCFG(m_dwQuestID);
    if (pCFG != nullptr)
    {
      DWORD step = pCFG->getStepBySubGroup(subGroup);
      pUser->getQuest().setQuestStep(m_dwQuestID, step);
    }
  }
  */
}

#define GO_TO_GROUP(groupID) { TestItem i; if (getSubGroup(groupID, i, vec)) { doTestItem(i, vec, node); } else { XERR << "[任务测试],id:" << item.m_dwID << "找不到SubGroup:" << item.m_dwFinishJump << XEND; } }

void TestQuest::doTestItem(const TestItem &item, std::vector<TestItem> &vec, TestItemTreeNode *father)
{
  if (father)
  {
    if (father->m_treelist.find(item.m_dwID)!=father->m_treelist.end())
    {
      XLOG << "[任务测试],quest:" << item.m_dwQuestID << ",闭环任务结束:{" << father->m_stream.str() << "," << father->m_act << "->" << item.m_strContent << "(" << item.m_dwID << ")}" << XEND;
      return;
    }
  }

  TestItemTreeNode *node = NEW TestItemTreeNode;
  node->m_dwID = item.m_dwID;
  if (father)
  {
    node->m_pFather = father;
    father->m_children.push_back(node);
    node->m_stream << father->m_stream.str() << "," << father->m_act << "->";
    node->m_treelist = father->m_treelist;
  }
  node->m_stream << item.m_strContent << "(" << item.m_dwID << ")";
  node->m_treelist.insert(item.m_dwID);

  DWORD dwNextID = item.m_dwID + 1;

  if (item.m_dwFinishJump)
  {
    node->m_act = "成功";
    GO_TO_GROUP(item.m_dwFinishJump);
  }
  if (item.m_dwFailJump)
  {
    node->m_act = "失败";
    GO_TO_GROUP(item.m_dwFailJump);
  }
  bool goNext = false;
  for (auto &it : item.m_vecSelects)
  {
    if (it)
    {
      node->m_act = "选择";
      GO_TO_GROUP(it);
    }
    else
    {
      goNext = true;
    }
  }
  if (!item.m_dwFinishJump || !item.m_dwFailJump || item.m_vecSelects.empty() || goNext)
  {
    if (dwNextID < vec.size())
    {
      if (vec[dwNextID].m_dwSubGroup == item.m_dwSubGroup)
      {
        node->m_act = "顺序";
        doTestItem(vec[dwNextID], vec, node);
      }
      else
      {
        XLOG << "[任务测试],quest:" << item.m_dwQuestID << ",逻辑分组结束:{" << node->m_stream.str() << "}" << XEND;
      }
    }
    else
    {
      XLOG << "[任务测试],quest:" << item.m_dwQuestID << ",没有后续结束:{" << node->m_stream.str() << "}" << XEND;
    }
  }
}

bool TestQuest::getSubGroup(DWORD subGroupID, TestItem &item, std::vector<TestItem> &vec)
{
  for (auto &it : vec)
  {
    if (it.m_dwSubGroup == subGroupID)
    {
      item = it;
      return true;
    }
  }
  return false;
}

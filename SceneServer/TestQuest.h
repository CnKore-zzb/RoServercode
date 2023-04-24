#pragma once
#include <list>
#include "xDefine.h"
#include "Quest.h"

struct TestItemTreeNode
{
  DWORD m_dwID = 0;
  TestItemTreeNode *m_pFather = nullptr;
  std::list<TestItemTreeNode *> m_children;
  std::stringstream m_stream;
  std::string m_act;
  std::set<DWORD> m_treelist;
};

struct TestItem
{
  DWORD m_dwQuestID = 0;
  DWORD m_dwID = 0;
  DWORD m_dwSubGroup = 0;
  DWORD m_dwFinishJump = 0;
  DWORD m_dwFailJump = 0;
  DWORD m_dwResetJump = 0;
  std::string m_strContent;
  std::vector<DWORD> m_vecSelects;
};

typedef std::vector<TestItem> TVecTestItem;

class TestQuest
{
  public:
    TestQuest();
    ~TestQuest();

  public:
    static void print();
    static void test(const TVecQuestStep &vec);
    static void doTestItem(const TestItem &item, std::vector<TestItem> &vec, TestItemTreeNode *father);
    static bool getSubGroup(DWORD subGroupID, TestItem &item, TVecTestItem &vec);
};

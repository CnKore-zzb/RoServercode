#include "gtest/gtest.h"
#include "Guild.h"

// 需加载Table_Item.txt, GameConfig.txt
// cd bin/Test; ln -s ../Debug/Lua
TEST(GvgClearScore, clear)
{
  // 加载相关配置
  ItemConfig::getMe().loadConfig();
  MiscConfig::getMe().loadConfig();

  Guild* pGuild1 = new Guild(1, 1001, "testguild");
  auto& gvg1 = pGuild1->getMisc().getGvg();
  gvg1.checkVersion();
  ASSERT_EQ(pGuild1->getPack().getItemCount(5543), 0);
  gvg1.setFireCntAndScore(1,1);
  ASSERT_EQ(pGuild1->getPack().getItemCount(5543), 0);
  delete pGuild1;


  Guild* pGuild2 = new Guild(2, 1002, "testguild2");
  auto& gvg2 = pGuild2->getMisc().getGvg();
  gvg2.setFireCntAndScore(1,3);
  gvg2.checkVersion();
  ASSERT_EQ(pGuild2->getPack().getItemCount(5543), 100);
  delete pGuild2;

  Guild* pGuild3 = new Guild(3, 1003, "testguild3");
  auto& gvg3 = pGuild3->getMisc().getGvg();
  gvg3.setFireCntAndScore(2,7);
  gvg3.checkVersion();
  ASSERT_EQ(pGuild3->getPack().getItemCount(5543), 200);
  gvg3.checkVersion();
  ASSERT_EQ(pGuild3->getPack().getItemCount(5543), 200);
  delete pGuild3;

  Guild* pGuild4 = new Guild(4, 1004, "testguild4");
  auto& gvg4 = pGuild4->getMisc().getGvg();

  //1 / 1 = 1
  ASSERT_EQ(gvg4.getSuperGvgLv(), 1);
  gvg4.checkVersion();
  ASSERT_EQ(gvg4.getSuperGvgLv(), 1);

  //1+7/2 = 4
  gvg4.finishSuperGvg(1);
  ASSERT_EQ(gvg4.getSuperGvgLv(), 2);
  // 1+7+7/3=5
  gvg4.finishSuperGvg(1);
  ASSERT_EQ(gvg4.getSuperGvgLv(), 3);
}

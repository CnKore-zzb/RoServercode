#include <memory>
#include "gtest/gtest.h"

#include "xLuaTable.h"

TEST(xLuaDataTest, parseFromeString)
{
  std::string str = ""
      "Root = {"
      "  ID = 1,"
      "}";
  xLuaData data;
  data.parseFromString(str, "Root");

  ASSERT_EQ(1, data.getTableInt("ID"));
  ASSERT_STREQ("1", data.getTableString("ID"));
}

TEST(xLuaTableTest, open)
{
  xLuaData data;
  ASSERT_TRUE(
      xLuaTable::getMe().open(
          "../../tests/base/data/xLuaTableTest_open.lua"));
  xLuaTable::getMe().getLuaData("Root", data);

  ASSERT_EQ(1, data.getTableInt("ID"));
  ASSERT_STREQ("1", data.getTableString("ID"));
}


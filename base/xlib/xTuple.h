#pragma once
#include <tuple>
#include <string>
#include "xDefine.h"

/* 使用例子
  TUP_3U aa()
  {
    TUP_3U tup;
    std::get<0>(tup) = 1;
    return tup;
  }

  TUP_3U bb()
  {
    UINT a, b, c;
    return std::make_tuple(a, b, c);
  }

  void cc()
  {
    UINT a, b, c;
    std::tie(a, b, c) = aa();
  }

  void dd()
  {
    TUP_3U tup = aa();
    UINT a = std::get<1>(tup);
  }
// */

class SceneUser;
class SceneNpc;

typedef std::tuple <UINT, UINT> TUP_2U;
typedef std::tuple <UINT, UINT, UINT> TUP_3U;
typedef std::tuple <UINT, UINT, UINT, UINT> TUP_4U;
typedef std::tuple <UINT, UINT, UINT, UINT, UINT> TUP_5U;

typedef std::tuple <UINT, const char *> TUP_UC;
typedef std::tuple <UINT, UINT, const char *> TUP_2UC;
typedef std::tuple <UINT, UINT, UINT, const char *> TUP_3UC;
typedef std::tuple <UINT, UINT, const char *, const char *> TUP_2U2C;

typedef std::tuple <bool, UINT> TUP_BU;
typedef std::tuple <bool, UINT, UINT> TUP_B2U;
typedef std::tuple <bool, UINT, UINT, UINT> TUP_B3U;

typedef std::tuple <const char *, const char *> TUP_2C;

typedef std::tuple <UINT, std::string> TUP_US;

typedef std::tuple <std::string, std::string> TUP_2S;

typedef std::tuple <SceneUser*, SceneUser*> TUP_2User;
typedef std::tuple <SceneUser*, SceneNpc*> TUP_UserNpc;
typedef std::tuple <SceneNpc*, SceneNpc*> TUP_2Npc;


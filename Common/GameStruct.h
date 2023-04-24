#pragma once

#include "xDefine.h"

enum GUILD_BLOB_INIT_STATUS
{
  GUILD_BLOB_INIT_NULL,
  GUILD_BLOB_INIT_COPY_DATA,
  GUILD_BLOB_INIT_OK,
};

struct SPetWearStat
{
  QWORD charid = 0;
  std::map<Cmd::EQualityType, DWORD> mapQualityCount;
};
typedef std::map<QWORD, SPetWearStat> TMapPetWearStat;


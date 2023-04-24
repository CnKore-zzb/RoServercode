#pragma once

#include "xSingleton.h"

using namespace Cmd;
using std::map;
using std::string;

struct SDateLandCFG
{
  DWORD dwId = 0;
  string sName;
  DWORD dwInviteOverTime = 0;
  DWORD dwCountDown = 0;
  DWORD dwInviteMaxCount = 0;
  DWORD dwEnterWaitTime = 0;
  DWORD dwTicketItem = 0;
  DWORD dwRaidID = 0;
  bool bGender = false;
};

class DateLandConfig : public xSingleton<DateLandConfig>
{
  friend class xSingleton<DateLandConfig>;
private:
  DateLandConfig();
public:
  virtual ~DateLandConfig();

  bool loadConfig();
  bool checkConfig();

  const SDateLandCFG* getDateLandCFG(DWORD id) const;

private:
  map<DWORD, SDateLandCFG> m_mapDateLand;
};

#pragma once

#include <set>
#include <list>
#include "xNoncopyable.h"
#include "xDefine.h"

class SceneUser;
namespace Cmd
{
  class BlobLaboratory;
};

class UserLaboratory : private xNoncopyable
{
  public:
    UserLaboratory(SceneUser *u);
    ~UserLaboratory();

  public:
    void save(Cmd::BlobLaboratory *data);
    void load(const Cmd::BlobLaboratory &data);

  public:
    DWORD getTodayMaxPoint();

    void setTodayPoint(DWORD point);
    //DWORD getPoint();

  public:
    SceneUser *m_pUser = nullptr;

    DWORD m_dwTodayMaxPoint = 0;
    DWORD m_dwDays = 0;
};

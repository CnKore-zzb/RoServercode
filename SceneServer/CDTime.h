#pragma once
#include <map>
#include <set>
#include "xDefine.h"
#include "SceneUser.pb.h"
#include "RecordCmd.pb.h"
class xSceneEntryDynamic;
using namespace Cmd;

namespace Cmd
{
  class BlobCDTime;
};

class CDTimeM
{
  friend class Skill;
  public:
    CDTimeM(xSceneEntryDynamic *e);
    ~CDTimeM();
    void fixID(CD_TYPE t, DWORD &id);
    bool done(CD_TYPE t, DWORD id);
    bool skilldone(DWORD id);
    void add(DWORD id, DWORD elapse, CD_TYPE type);
    void clear(DWORD id, CD_TYPE type);
    void clear();
    void load(const Cmd::BlobCDTime &data);
    void save(Cmd::BlobCDTime *data);
    void send(CD_TYPE t, DWORD id, QWORD time);
    void send();
    QWORD getSkillCDEndTime(DWORD id);
    QWORD getCDTime(CD_TYPE type, DWORD id);
  private:
    std::map<DWORD, QWORD> m_list[CD_TYPE_ARRAYSIZE];  // <id,截止时间(毫秒)>
    typedef std::map<DWORD, QWORD>::iterator m_iter;

  private:
    xSceneEntryDynamic *m_pEntry;
};

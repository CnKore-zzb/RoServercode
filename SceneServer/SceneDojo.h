#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "Dojo.pb.h"

using std::string;
using std::vector;

// tower
class SceneUser;

class SceneDojo
{
  public:
    SceneDojo(SceneUser* pUser);
    ~SceneDojo();

    bool save(Cmd::BlobDojo *data);
    bool load(const Cmd::BlobDojo &data);
    
    bool isOpen(DWORD dwDojoGroup);
    bool isPassed(DWORD dwDojoId);
    bool isGroupPassed(DWORD dwID);
    bool passDojo(DWORD dwDojoId);
    void getPrivateInfo(Cmd::DojoPrivateInfoCmd& rev);

    DWORD getCompleteCount() const { return static_cast<DWORD>(m_setCompleted.size()); }
  private:
    SceneUser* m_pUser = nullptr;
    
    std::set<DWORD/*dojo id*/> m_setCompleted;
};


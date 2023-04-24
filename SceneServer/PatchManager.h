/**
 * @file PatchManager.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-02-18
 */

#include "xSingleton.h"

using std::map;

class SceneUser;
class PatchManager : public xSingleton<PatchManager>
{
  friend class xSingleton<PatchManager>;
  private:
    PatchManager();
  public:
    virtual ~PatchManager();

    // patch : 修正冒险任务配置错误,导致玩家无需提交材料完成任务,需完成额外任务
    // time  : 2017-02-18
    // file  : quest.h quest.cpp Character.lua
    bool loadPatchList();
    bool isPatchChar(QWORD qwCharID) const { return m_setQuestCharIDs.find(qwCharID) != m_setQuestCharIDs.end(); }

    bool canPatchAccept(SceneUser* pUser, DWORD id);
    bool submitPatch(SceneUser* pUser, DWORD id);
  private:
    TSetQWORD m_setQuestCharIDs;
};


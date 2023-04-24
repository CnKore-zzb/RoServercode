/**
 * @file Action.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-09-26
 */

#pragma once

#include <list>
#include "ActionItem.h"
#include "xNoncopyable.h"

using std::list;
using std::map;

class SceneUser;
class xSceneEntryDynamic;

typedef list<BaseAction*> TListAction;
typedef map<DWORD, TListAction> TMapAction;

class Action : private xNoncopyable
{
  public:
    Action(SceneUser* pUser);
    ~Action();

    bool hasAction(DWORD dwID) { return m_mapAction.find(dwID) != m_mapAction.end(); }
    bool addAction(DWORD dwID, DWORD dwActionID);
    bool addAction(DWORD dwID, const SActionCFG& rCFG);

    BaseAction* getActionItem(DWORD dwID, DWORD dwActionID);

    void timer(DWORD curSec);
  private:
    SceneUser* m_pUser = nullptr;

    TMapAction m_mapAction;
};


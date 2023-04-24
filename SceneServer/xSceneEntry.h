#pragma once
#include <unordered_set>
#include "xEntry.h"
#include "SceneDefine.h"
#include "xPos.h"

class Scene;

class xSceneEntry : public xEntry
{
  public:
    xSceneEntry();
    virtual ~xSceneEntry();
    virtual SCENE_ENTRY_TYPE getEntryType()const=0;
    xPos getPos() const { return pos; }
    void setPos(xPos p);
    xPos getOldPos() const { return oldpos; }

    xPosI getPosI() const { return posI; }
    xPosI getOldPosI() const { return oldposI; }

    static xPos getDirectByPos(const xPos from, const xPos tar) { return from - tar; }

    virtual void sendMeToNine()=0;
    virtual void delMeToNine()=0;
    virtual bool enterScene(Scene *s) = 0;

  private:
    xPos pos;
    xPos oldpos;
    xPosI posI;
    xPosI oldposI;

  public:
    inline void setScene(Scene *s)
    {
      m_pScene = s;
    }
    inline Scene* getScene() const
    {
      return m_pScene;
    }
  private:
    Scene *m_pScene = nullptr;
};

typedef std::unordered_set<xSceneEntry *> xSceneEntrySet;

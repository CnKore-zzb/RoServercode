#ifndef _NPC_STATE
#define _NPC_STATE

#include "xState.h"

class SceneNpc;

class NpcState : public xState<SceneNpc>
{
  public:
    virtual ~NpcState(){}
    virtual void enter(SceneNpc *npc){}
    virtual void execute(SceneNpc *npc){}
    virtual void exit(SceneNpc *npc){}
};

#define _InstanceFunction(ClassName)\
  static ClassName *instance()\
  {\
    static ClassName _instance;\
    return &_instance;\
  }

class NormalNpcState : public NpcState
{
  public:
    NormalNpcState(){}
    virtual ~NormalNpcState(){}

    _InstanceFunction(NormalNpcState);

    virtual void execute(SceneNpc *npc);
};

class AttackNpcState : public NpcState
{
  public:
    AttackNpcState(){}
    virtual ~AttackNpcState(){}

    _InstanceFunction(AttackNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class MoveToPosNpcState : public NpcState
{
  public:
   MoveToPosNpcState(){}
   virtual ~MoveToPosNpcState(){}

   _InstanceFunction(MoveToPosNpcState);

   virtual void execute(SceneNpc *npc);
   virtual void exit(SceneNpc *npc);
};

class PickupNpcState : public NpcState
{
  public:
    PickupNpcState() {}
    virtual ~PickupNpcState() {}

    _InstanceFunction(PickupNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class WaitNpcState : public NpcState
{
  public:
    WaitNpcState() {}
    virtual ~WaitNpcState() {}

    _InstanceFunction(WaitNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class CameraNpcState : public NpcState
{
  public:
    CameraNpcState() {}
    virtual ~CameraNpcState() {}

    _InstanceFunction(CameraNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class NaughtyNpcState : public NpcState
{
  public:
    NaughtyNpcState() {}
    virtual ~NaughtyNpcState() {}

    _InstanceFunction(NaughtyNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class SmileNpcState : public NpcState
{
  public:
    SmileNpcState() {}
    virtual ~SmileNpcState() {}

    _InstanceFunction(SmileNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class SeeDeadNpcState : public NpcState
{
  public:
    SeeDeadNpcState() {}
    virtual ~SeeDeadNpcState() {}

    _InstanceFunction(SeeDeadNpcState);

    virtual void execute(SceneNpc *npc);
    virtual void exit(SceneNpc *npc);
};

class SleepNpcState : public NpcState
{
public:
  SleepNpcState() {}
  virtual ~SleepNpcState() {}

  _InstanceFunction(SleepNpcState);
  virtual void enter(SceneNpc *npc);
  virtual void execute(SceneNpc *npc);
};

class GoBackNpcState : public NpcState
{
public:
  GoBackNpcState() {}
  virtual ~GoBackNpcState() {}

  _InstanceFunction(GoBackNpcState);
  virtual void enter(SceneNpc *npc);
  virtual void execute(SceneNpc *npc);
  virtual void exit(SceneNpc *npc);
};

class RunAwayNpcState : public NpcState
{
  public:
    RunAwayNpcState() {}
    virtual ~RunAwayNpcState() {}

  _InstanceFunction(RunAwayNpcState);
  virtual void enter(SceneNpc *npc);
  virtual void execute(SceneNpc *npc);
};

#endif

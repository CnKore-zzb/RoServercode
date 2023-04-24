#pragma once
#include <queue>
#include "xThread.h"
#include "xDefine.h"
#include "xSingleton.h"
#include "SceneUserData.h"
#include "xQueue.h"

class SceneUserLoadThread : public xThread, public xSingleton<SceneUserLoadThread>
{
  public:
    SceneUserLoadThread();
    virtual ~SceneUserLoadThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    void add(Cmd::RecordUserData &data);
    SceneUserDataLoad* get();
    void pop();

  private:
    xQueue<SceneUserDataLoad> m_oPrepareQueue;
    xQueue<SceneUserDataLoad> m_oFinishQueue;

  private:
    void check();
    void exec(SceneUserDataLoad *data);
};

class SceneUserSaveThread : public xThread, public xSingleton<SceneUserSaveThread>
{
  public:
    SceneUserSaveThread();
    virtual ~SceneUserSaveThread();

  public:
    bool thread_init();
    void thread_proc();
    virtual void thread_stop();
    void final();

  public:
    void add(SceneUserDataSave *pData);

  private:
    xQueue<SceneUserDataSave> m_oPrepareQueue;

  private:
    void check();
    void exec(SceneUserDataSave *data);
};

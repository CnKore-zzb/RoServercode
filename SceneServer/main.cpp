#include <signal.h>
#include "SceneServer.h"
#include "SceneManager.h"
#include "BaseConfig.h"

void kill_handler(int s)
{
  if(thisServer)
    thisServer->stop();
}

int main(int argc,char*argv[])
{
  signal(SIGTERM, kill_handler);
  signal(SIGSTOP, kill_handler);
  signal(SIGINT, kill_handler);

  OptArgs args;
  args.get(argc, argv);

  thisServer = NEW SceneServer(args);
  XLOG << "[进程启动]" << args.m_strServerName << args.m_strPlatName << args.m_strZoneName << XEND;

  if (thisServer)
  {
    thisServer->test();
    xLuaTable::getMe().test();

    if (xServer::s_oOptArgs.m_blBuild)
    {
      BaseConfig::getMe().loadConfig();
      thisServer->loadConfig();
      SceneManager::getMe().init();
      system("./update_client_export.sh");
      return 0;
    }

    //thisServer->loadConfig();//test
    thisServer->run();

    SAFE_DELETE(thisServer);
  }
  else
  {
    XERR << "命令行参数错误，请指定 -n ServerName" << XEND;
  }

#ifdef _DEBUG
  MemSta::printLeakMem();
#endif
  return 0;
}

#include <signal.h>
#include "SocialServer.h"
#include "BaseConfig.h"

void kill_handler(int s)
{
  if (thisServer)
    thisServer->stop();
}

int main(int argc,char*argv[])
{
  signal(SIGTERM, kill_handler);
  signal(SIGSTOP, kill_handler);
  signal(SIGINT, kill_handler);

  OptArgs args;
  args.get(argc, argv);

  thisServer = NEW SocialServer(args);
  XLOG << "[进程启动]" << args.m_strServerName.c_str() << args.m_strPlatName.c_str() << XEND;

  if (thisServer)
  {
    if (xServer::s_oOptArgs.m_blBuild)
    {
      BaseConfig::getMe().loadConfig();
      thisServer->addDataBase(REGION_DB, false);
      thisServer->load();
      thisServer->gcharPatch();
      return 0;
    }
    thisServer->run();

    SAFE_DELETE(thisServer);
  }
  else
  {
    XERR << "命令行参数错误" << XEND;
  }

#ifdef _DEBUG
  MemSta::printLeakMem();
#endif
  return 0;
}

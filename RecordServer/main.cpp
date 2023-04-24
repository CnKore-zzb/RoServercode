#include <signal.h>
#include "RecordServer.h"
#include "BaseConfig.h"
#include "RecordTool.h"

void kill_handler(int s)
{
  thisServer->stop();
}

int main(int argc,char*argv[])
{
  signal(SIGTERM, kill_handler);
  signal(SIGSTOP, kill_handler);
  signal(SIGINT, kill_handler);

  OptArgs args;
  args.get(argc, argv);

  thisServer = NEW RecordServer(args);

  if (thisServer)
  {
    if (xServer::s_oOptArgs.m_blBuild)
    {
      BaseConfig::getMe().loadConfig();
      thisServer->addDataBase(REGION_DB, false);
      RecordTool::getMe().init();
      return 0;
    }

    thisServer->run();
    SAFE_DELETE(thisServer);
  }
  else
    XERR << "命令行参数错误，请指定 -n ServerName" << XEND;

#ifdef _DEBUG
  MemSta::printLeakMem();
#endif
  return 0;
}

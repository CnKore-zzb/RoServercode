#include <signal.h>
#include "SessionServer.h"

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

  thisServer = NEW SessionServer(args);
  if (thisServer)
  {
    thisServer->run();

    SAFE_DELETE(thisServer);
  }
  else
    XERR << "命令行参数错误 请指定 -n ServerName" << XEND;

#ifdef _DEBUG
  MemSta::printLeakMem();
#endif
  return 0;
}

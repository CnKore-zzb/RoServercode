#include <signal.h>
#include "GlobalServer.h"

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

  thisServer = NEW GlobalServer(args);
  XLOG << "[进程启动]" << args.m_strServerName.c_str() << args.m_strPlatName.c_str() << XEND;

  if (thisServer)
  {
    thisServer->run();

    SAFE_DELETE(thisServer);
  }
  else
  {
    XERR << "命令行参数错误" << XEND;
  }
  return 0;
}

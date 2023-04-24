#include <signal.h>
#include "StatServer.h"

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

  thisServer = NEW StatServer(args);
  XLOG << "[进程启动]," << args.m_strServerName.c_str() << "," << args.m_strPlatName.c_str() << "," << args.m_strZoneName.c_str() << XEND;

  //fprintf(stderr, "%c:%s,%s", oc, serverName.c_str(), configFile.c_str());
  //test();
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

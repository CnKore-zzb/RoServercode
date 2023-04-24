#include "xNewMem.h"
#include "xLog.h"

std::map<const void*, Meminfo> MemSta::Mems;
void MemSta::add_use_mem(const void* p, const char* file, int line)
{
  Meminfo info;
  info.file = file;
  info.line = line;
  Mems[p]=info;
}

void MemSta::del_use_mem(const void* p)
{
  if(Mems.find(p)==Mems.end()) return;
  Mems.erase(p);
}

void MemSta::printLeakMem()
{
  MEM::iterator it=Mems.begin(),end=Mems.end();
  for(;it!=end;it++)
  {
    //printf("[memory],leak:%p, file:%s, line:%d \n", it->first, it->second.file.c_str(), it->second.line);
    XLOG << "[memory],leak:" << (unsigned long long)it->first << ",file:" << it->second.file.c_str() << ",line:" << it->second.line << XEND;
  }
}

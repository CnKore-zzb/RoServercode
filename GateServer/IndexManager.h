#ifndef _INDEX_MANAGER
#define _INDEX_MANAGER
#include <map>
#include <set>
#include "xSingleton.h"
#include "xCommand.h"
//#include "Define.h"
#include "GatewayCmd.h"

class GateUser;
class OneLevelIndexManager : public xSingleton<OneLevelIndexManager>
{
  public:
    OneLevelIndexManager() {}
    ~OneLevelIndexManager() {}

    void addIndex(ONE_LEVEL_INDEX_TYPE t, QWORD i, GateUser *);
    void remove(GateUser *);
    void remove(ONE_LEVEL_INDEX_TYPE t, QWORD i, GateUser *);

    void broadcastCmd(ONE_LEVEL_INDEX_TYPE t, QWORD i, unsigned char *buf, unsigned short len, QWORD exclude=0, DWORD ip=0);

  private:
    typedef std::set<GateUser *> UserSet;
    typedef std::map<QWORD, UserSet> OneLevelIndex;
    OneLevelIndex indexlist[ONE_LEVEL_INDEX_TYPE_MAX];
};

class TwoLevelIndexManager : public xSingleton<TwoLevelIndexManager>
{
  public:
    TwoLevelIndexManager() {}
    ~TwoLevelIndexManager() {}

    void addIndex(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, GateUser *pUser);
    void remove(GateUser *);
    void remove(TWO_LEVEL_INDEX_TYPE t, GateUser *);
    void remove(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, GateUser *);

    void broadcastCmd(TWO_LEVEL_INDEX_TYPE t, DWORD i, DWORD i2, unsigned char *buf, unsigned short len, GateIndexFilter &filer);

  private:
    typedef std::set<GateUser *> UserSet;
    typedef std::map<DWORD, UserSet> OneLevelIndex;
    typedef std::map<DWORD, OneLevelIndex> TwoLevelIndex;
    TwoLevelIndex indexlist[TWO_LEVEL_INDEX_TYPE_MAX];
};
#endif

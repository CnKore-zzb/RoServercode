#ifndef BASE_XLIB_XCMDQUEUE_H_
#define BASE_XLIB_XCMDQUEUE_H_
#include <ext/mt_allocator.h>
#include <vector>
#include <queue>
#include <utility>
#include "xlib/xDefine.h"
#include "xlib/xCommand.h"

typedef std::pair<DWORD, BYTE *> CmdPair;

const DWORD QUEUE_SIZE = 102400;

class xCmdQueue
{
  public:
    xCmdQueue();
    ~xCmdQueue();

  public:
    bool put(unsigned char *cmd, unsigned short len);
    CmdPair* get();
    void erase();

    bool is_valid() { return is_valid_; }

  private:
    void clear();
    bool putQueueToArray();

  private:
    std::pair<volatile bool, CmdPair> m_oCmdArray[QUEUE_SIZE];
    std::queue<CmdPair> m_oCmdQueue;
    DWORD m_dwWriteIndex;
    DWORD m_dwReadIndex;

    volatile bool is_valid_;

    __mt_alloc<BYTE> _mt_alloc;
};
#endif  // BASE_XLIB_XCMDQUEUE_H_

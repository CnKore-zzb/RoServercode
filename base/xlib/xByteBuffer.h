#ifndef BASE_XLIB_XBYTEBUFFER_H_
#define BASE_XLIB_XBYTEBUFFER_H_
#include <vector>
#include "xlib/xDefine.h"
#include "xLog.h"

// 单线程
class xByteBuffer
{
  public:
    xByteBuffer();
    ~xByteBuffer();

  public:
    inline DWORD size() const
    {
      return _size;
    }
    inline void reset()
    {
      _write_offset = 0;
      _read_offset = 0;
    }
    inline void set_wr_offset(DWORD offset)
    {
      _write_offset = offset;
    }
    inline DWORD wr_offset() const
    {
      return _write_offset;
    }
    inline void set_rd_offset(DWORD offset)
    {
      _read_offset = offset;
    }
    inline DWORD rd_offset() const
    {
      return _read_offset;
    }

    void resize(DWORD len);

    void write(const void *data, DWORD len);

    void wr_reserve(DWORD len);
    DWORD write(DWORD len);

    void read(DWORD len);
    void read_flip();

    // 剩余的可写大小
    inline DWORD getWriteLeft() const
    {
      return _size - _write_offset;
    }
    // 可读数据大小
    inline DWORD getReadSize() const
    {
      if (_write_offset > _read_offset)
      {
        return _write_offset - _read_offset;
      }
      return 0;
    }

    BYTE* getWriteBuf() { return &(_buffer[_write_offset]); }
    BYTE* getReadBuf() { return &(_buffer[_read_offset]); }
    BYTE* getReadBuf(DWORD offset) { return &(_buffer[_read_offset + offset]); }
    BYTE *getBegin() { return &(_buffer[0]); }
    BYTE *getBegin(DWORD offset) { return &(_buffer[offset]); }

    void copy(xByteBuffer& buffer);

  public:
    std::string m_strName;
    std::string m_strType;

  private:
    DWORD _size = 0;
    DWORD _write_offset = 0;
    DWORD _read_offset = 0;

    std::vector<BYTE> _buffer;
};
#endif  // BASE_XLIB_XBYTEBUFFER_H_
